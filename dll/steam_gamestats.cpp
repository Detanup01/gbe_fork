/* Copyright (C) 2019 Mr Goldberg
   This file is part of the Goldberg Emulator

   The Goldberg Emulator is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   The Goldberg Emulator is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the Goldberg Emulator; if not, see
   <http://www.gnu.org/licenses/>.
*/

/*
    There are no public docs about this interface, this implementation
    is an imagination of how it might have been implemented
*/

#include "dll/steam_gamestats.h"


Steam_GameStats::Attribute_t::Attribute_t(AttributeType_t type)
    :type(type)
{
    switch (type)
    {
    case AttributeType_t::Float: f_data = 0; break;
    case AttributeType_t::Int64: ll_data = 0; break;
    case AttributeType_t::Int: n_data = 0; break;
    case AttributeType_t::Str: new (&s_data) std::string{}; break;
    
    default: PRINT_DEBUG("[X] invalid type %i", (int)type); break;
    }
}

Steam_GameStats::Attribute_t::Attribute_t(const Attribute_t &other)
    :type(type)
{
    switch (other.type)
    {
    case AttributeType_t::Int: n_data = other.n_data; break;
    case AttributeType_t::Str: new (&s_data) std::string(other.s_data); break;
    case AttributeType_t::Float: f_data = other.f_data; break;
    case AttributeType_t::Int64: ll_data = other.ll_data; break;
    
    default: PRINT_DEBUG("[X] invalid type %i", (int)other.type); break;
    }
}

Steam_GameStats::Attribute_t::Attribute_t(Attribute_t &&other)
    :type(type)
{
    switch (other.type)
    {
    case AttributeType_t::Int: n_data = other.n_data; break;
    case AttributeType_t::Str: new (&s_data) std::string(std::move(other.s_data)); break;
    case AttributeType_t::Float: f_data = other.f_data; break;
    case AttributeType_t::Int64: ll_data = other.ll_data; break;
    
    default: PRINT_DEBUG("[X] invalid type %i", (int)other.type); break;
    }
}

Steam_GameStats::Attribute_t::~Attribute_t()
{
    if (type == AttributeType_t::Str) {
        s_data.~basic_string();
    }
}


void Steam_GameStats::steam_gamestats_network_low_level(void *object, Common_Message *msg)
{
    //PRINT_DEBUG_ENTRY();

    auto inst = (Steam_GameStats *)object;
    inst->network_callback_low_level(msg);
}

void Steam_GameStats::steam_gamestats_run_every_runcb(void *object)
{
    //PRINT_DEBUG_ENTRY();

    auto inst = (Steam_GameStats *)object;
    inst->steam_run_callback();
}

Steam_GameStats::Steam_GameStats(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb)
{
    this->settings = settings;
    this->network = network;
    this->callback_results = callback_results;
    this->callbacks = callbacks;
    this->run_every_runcb = run_every_runcb;
    
    this->network->setCallback(CALLBACK_ID_USER_STATUS, settings->get_local_steam_id(), &Steam_GameStats::steam_gamestats_network_low_level, this);
    this->run_every_runcb->add(&Steam_GameStats::steam_gamestats_run_every_runcb, this);

}

Steam_GameStats::~Steam_GameStats()
{
    this->network->rmCallback(CALLBACK_ID_USER_STATUS, settings->get_local_steam_id(), &Steam_GameStats::steam_gamestats_network_low_level, this);
    this->run_every_runcb->remove(&Steam_GameStats::steam_gamestats_run_every_runcb, this);
}


bool Steam_GameStats::valid_stats_account_type(int8 nAccountType)
{
    switch ((EGameStatsAccountType)nAccountType) {
        case EGameStatsAccountType::k_EGameStatsAccountType_Steam:
        case EGameStatsAccountType::k_EGameStatsAccountType_Xbox:
        case EGameStatsAccountType::k_EGameStatsAccountType_SteamGameServer:
            return true;
    }

    return false;
}

Steam_GameStats::Table_t *Steam_GameStats::get_or_create_session_table(Session_t &session, const char *table_name)
{
    Table_t *table{};
    {
        auto table_it = std::find_if(session.tables.rbegin(), session.tables.rend(), [=](const std::pair<std::string, Table_t> &item){ return item.first == table_name; });
        if (session.tables.rend() == table_it) {
            session.tables.emplace_back(std::pair<std::string, Table_t>{});
            table = &session.tables.back().second;
        } else {
            table = &table_it->second;
        }
    }

    return table;
}

Steam_GameStats::Attribute_t *Steam_GameStats::get_or_create_session_att(const char *att_name, Session_t &session, AttributeType_t type_if_create)
{
    auto [ele_it, _] = session.attributes.emplace(att_name, type_if_create);
    return &ele_it->second;
}

Steam_GameStats::Attribute_t *Steam_GameStats::get_or_create_row_att(uint64 ulRowID, const char *att_name, Table_t &table, AttributeType_t type_if_create)
{
    auto &row = table.rows[static_cast<unsigned>(ulRowID)];
    auto [ele_it, _] = row.attributes.emplace(att_name, type_if_create);
    return &ele_it->second;
}


SteamAPICall_t Steam_GameStats::GetNewSession( int8 nAccountType, uint64 ulAccountID, int32 nAppID, RTime32 rtTimeStarted )
{
    PRINT_DEBUG("%i, %llu, %i, %u", (int)nAccountType, ulAccountID, nAppID, rtTimeStarted);
    std::lock_guard lock(global_mutex);

    if ((settings->get_local_steam_id().ConvertToUint64() != ulAccountID) ||
        (nAppID < 0) ||
        (settings->get_local_game_id().AppID() != (uint32)nAppID) ||
        !valid_stats_account_type(nAccountType)) {
        GameStatsSessionIssued_t data_invalid{};
        data_invalid.m_bCollectingAny = false;
        data_invalid.m_bCollectingDetails = false;
        data_invalid.m_eResult = EResult::k_EResultInvalidParam;
        data_invalid.m_ulSessionID = 0;

        auto ret = callback_results->addCallResult(data_invalid.k_iCallback, &data_invalid, sizeof(data_invalid));
        // the function returns SteamAPICall_t (call result), but in isteamstats.h you can see that a callback is also expected
        callbacks->addCBResult(data_invalid.k_iCallback, &data_invalid, sizeof(data_invalid));
        return ret;
    }

    Session_t new_session{};
    new_session.nAccountType = (EGameStatsAccountType)nAccountType;
    new_session.rtTimeStarted = rtTimeStarted;
    sessions.emplace_back(new_session);

    GameStatsSessionIssued_t data{};
    data.m_bCollectingAny = true; // TODO is this correct?
    data.m_bCollectingDetails = true; // TODO is this correct?
    data.m_eResult = EResult::k_EResultOK;
    data.m_ulSessionID = (uint64)sessions.size(); // I don't know if 0 is a bad value, so always send count (index + 1)
    
    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    // the function returns SteamAPICall_t (call result), but in isteamstats.h you can see that a callback is also expected
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return ret;
}

SteamAPICall_t Steam_GameStats::EndSession( uint64 ulSessionID, RTime32 rtTimeEnded, int nReasonCode )
{
    PRINT_DEBUG("%llu, %u, %i", ulSessionID, rtTimeEnded, nReasonCode);
    std::lock_guard lock(global_mutex);

    if (ulSessionID == 0 || ulSessionID > sessions.size()) {
        GameStatsSessionClosed_t data_invalid{};
        data_invalid.m_eResult = EResult::k_EResultInvalidParam;
        data_invalid.m_ulSessionID = ulSessionID;

        auto ret = callback_results->addCallResult(data_invalid.k_iCallback, &data_invalid, sizeof(data_invalid));
        // the function returns SteamAPICall_t (call result), but in isteamstats.h you can see that a callback is also expected
        callbacks->addCBResult(data_invalid.k_iCallback, &data_invalid, sizeof(data_invalid));
        return ret;
    }

    auto& session = sessions[static_cast<unsigned>(ulSessionID - 1)];
    if (session.ended) {
        GameStatsSessionClosed_t data_invalid{};
        data_invalid.m_eResult = EResult::k_EResultExpired; // TODO is this correct?
        data_invalid.m_ulSessionID = ulSessionID;

        auto ret = callback_results->addCallResult(data_invalid.k_iCallback, &data_invalid, sizeof(data_invalid));
        // the function returns SteamAPICall_t (call result), but in isteamstats.h you can see that a callback is also expected
        callbacks->addCBResult(data_invalid.k_iCallback, &data_invalid, sizeof(data_invalid));
        return ret;
    }

    session.ended = true;
    session.rtTimeEnded = rtTimeEnded;
    session.nReasonCode = nReasonCode;

    GameStatsSessionClosed_t data{};
    data.m_eResult = EResult::k_EResultOK;
    data.m_ulSessionID = ulSessionID;

    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    // the function returns SteamAPICall_t (call result), but in isteamstats.h you can see that a callback is also expected
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return ret;
}

EResult Steam_GameStats::AddSessionAttributeInt( uint64 ulSessionID, const char* pstrName, int32 nData )
{
    PRINT_DEBUG("%llu, '%s', %i", ulSessionID, pstrName, nData);
    std::lock_guard lock(global_mutex);

    if (ulSessionID == 0 || ulSessionID > sessions.size() || !pstrName) return EResult::k_EResultInvalidParam; // TODO is this correct?
    
    auto& session = sessions[static_cast<unsigned>(ulSessionID - 1)];
    if (session.ended) return EResult::k_EResultExpired; // TODO is this correct?

    auto att = get_or_create_session_att(pstrName, session, AttributeType_t::Int);
    if (att->type != AttributeType_t::Int) return EResult::k_EResultFail; // TODO is this correct?

    att->n_data = nData;
    return EResult::k_EResultOK;
}

EResult Steam_GameStats::AddSessionAttributeString( uint64 ulSessionID, const char* pstrName, const char *pstrData )
{
    PRINT_DEBUG("%llu, '%s', '%s'", ulSessionID, pstrName, pstrData);
    std::lock_guard lock(global_mutex);

    if (ulSessionID == 0 || ulSessionID > sessions.size() || !pstrName || !pstrData) return EResult::k_EResultInvalidParam; // TODO is this correct?
    
    auto& session = sessions[static_cast<unsigned>(ulSessionID - 1)];
    if (session.ended) return EResult::k_EResultExpired; // TODO is this correct?

    auto att = get_or_create_session_att(pstrName, session, AttributeType_t::Str);
    if (att->type != AttributeType_t::Str) return EResult::k_EResultFail; // TODO is this correct?
    
    att->s_data = pstrData;
    return EResult::k_EResultOK;
}

EResult Steam_GameStats::AddSessionAttributeFloat( uint64 ulSessionID, const char* pstrName, float fData )
{
    PRINT_DEBUG("%llu, '%s', %f", ulSessionID, pstrName, fData);
    std::lock_guard lock(global_mutex);

    if (ulSessionID == 0 || ulSessionID > sessions.size() || !pstrName) return EResult::k_EResultInvalidParam; // TODO is this correct?
    
    auto& session = sessions[static_cast<unsigned>(ulSessionID - 1)];
    if (session.ended) return EResult::k_EResultExpired; // TODO is this correct?

    auto att = get_or_create_session_att(pstrName, session, AttributeType_t::Float);
    if (att->type != AttributeType_t::Float) return EResult::k_EResultFail; // TODO is this correct?

    att->f_data = fData;
    return EResult::k_EResultOK;
}


EResult Steam_GameStats::AddNewRow( uint64 *pulRowID, uint64 ulSessionID, const char *pstrTableName )
{
    PRINT_DEBUG("%p, %llu, '%s'", pulRowID, ulSessionID, pstrTableName);
    std::lock_guard lock(global_mutex);

    if (ulSessionID == 0 || ulSessionID > sessions.size() || !pstrTableName) return EResult::k_EResultInvalidParam; // TODO is this correct?
    
    auto& session = sessions[static_cast<unsigned>(ulSessionID - 1)];
    if (session.ended) return EResult::k_EResultExpired; // TODO is this correct?

    auto table = get_or_create_session_table(session, pstrTableName);
    table->rows.emplace_back(Row_t{});
    if (pulRowID) *pulRowID = (uint64)(table->rows.size() - 1);
    return EResult::k_EResultOK;
}

EResult Steam_GameStats::CommitRow( uint64 ulRowID )
{
    PRINT_DEBUG("%llu", ulRowID);
    std::lock_guard lock(global_mutex);
    
    auto active_session = std::find_if(sessions.rbegin(), sessions.rend(), [](const Session_t &item){ return !item.ended; });
    if (sessions.rend() == active_session) return EResult::k_EResultFail; // TODO is this correct?
    if (active_session->tables.empty()) return EResult::k_EResultFail; // TODO is this correct?

    auto &table = active_session->tables.back().second;
    if (ulRowID >= table.rows.size()) return EResult::k_EResultInvalidParam; // TODO is this correct?

    // TODO what if it was already committed ?
    auto& row = table.rows[static_cast<unsigned>(ulRowID)];
    row.committed = true;
    
    return EResult::k_EResultOK;
}

EResult Steam_GameStats::CommitOutstandingRows( uint64 ulSessionID )
{
    PRINT_DEBUG("%llu", ulSessionID);
    std::lock_guard lock(global_mutex);
    
    if (ulSessionID == 0 || ulSessionID > sessions.size()) return EResult::k_EResultInvalidParam; // TODO is this correct?
    
    auto& session = sessions[static_cast<unsigned>(ulSessionID - 1)];
    if (session.ended) return EResult::k_EResultExpired; // TODO is this correct?

    if (session.tables.size()) {
        for (auto &row : session.tables.back().second.rows) {
            row.committed = true;
        }
    }
    return EResult::k_EResultOK;
}

EResult Steam_GameStats::AddRowAttributeInt( uint64 ulRowID, const char *pstrName, int32 nData )
{
    PRINT_DEBUG("%llu, '%s', %i", ulRowID, pstrName, nData);
    std::lock_guard lock(global_mutex);
    
    if (!pstrName) return EResult::k_EResultInvalidParam; // TODO is this correct?
    
    auto active_session = std::find_if(sessions.rbegin(), sessions.rend(), [](const Session_t &item){ return !item.ended; });
    if (sessions.rend() == active_session) return EResult::k_EResultFail; // TODO is this correct?
    if (active_session->tables.empty()) return EResult::k_EResultFail; // TODO is this correct?

    auto &table = active_session->tables.back().second;
    if (ulRowID >= table.rows.size()) return EResult::k_EResultInvalidParam; // TODO is this correct?

    auto att = get_or_create_row_att(ulRowID, pstrName, table, AttributeType_t::Int);
    if (att->type != AttributeType_t::Int) return EResult::k_EResultFail; // TODO is this correct?

    att->n_data = nData;
    return EResult::k_EResultOK;
}

EResult Steam_GameStats::AddRowAtributeString( uint64 ulRowID, const char *pstrName, const char *pstrData )
{
    PRINT_DEBUG("%llu, '%s', '%s'", ulRowID, pstrName, pstrData);
    std::lock_guard lock(global_mutex);
    
    if (!pstrName || !pstrData) return EResult::k_EResultInvalidParam; // TODO is this correct?
    
    auto active_session = std::find_if(sessions.rbegin(), sessions.rend(), [](const Session_t &item){ return !item.ended; });
    if (sessions.rend() == active_session) return EResult::k_EResultFail; // TODO is this correct?

    auto &table = active_session->tables.back().second;
    if (ulRowID >= table.rows.size()) return EResult::k_EResultInvalidParam; // TODO is this correct?

    auto att = get_or_create_row_att(ulRowID, pstrName, table, AttributeType_t::Str);
    if (att->type != AttributeType_t::Str) return EResult::k_EResultFail; // TODO is this correct?

    att->s_data = pstrData;
    return EResult::k_EResultOK;
}

EResult Steam_GameStats::AddRowAttributeFloat( uint64 ulRowID, const char *pstrName, float fData )
{
    PRINT_DEBUG("%llu, '%s', %f", ulRowID, pstrName, fData);
    std::lock_guard lock(global_mutex);
    
    if (!pstrName) return EResult::k_EResultInvalidParam; // TODO is this correct?
    
    auto active_session = std::find_if(sessions.rbegin(), sessions.rend(), [](const Session_t &item){ return !item.ended; });
    if (sessions.rend() == active_session) return EResult::k_EResultFail; // TODO is this correct?

    auto &table = active_session->tables.back().second;
    if (ulRowID >= table.rows.size()) return EResult::k_EResultInvalidParam; // TODO is this correct?

    auto att = get_or_create_row_att(ulRowID, pstrName, table, AttributeType_t::Float);
    if (att->type != AttributeType_t::Float) return EResult::k_EResultFail; // TODO is this correct?

    att->f_data = fData;
    return EResult::k_EResultOK;
}


EResult Steam_GameStats::AddSessionAttributeInt64( uint64 ulSessionID, const char *pstrName, int64 llData )
{
    PRINT_DEBUG("%llu, '%s', %lli", ulSessionID, pstrName, llData);
    std::lock_guard lock(global_mutex);

    if (ulSessionID == 0 || ulSessionID > sessions.size() || !pstrName) return EResult::k_EResultInvalidParam; // TODO is this correct?
    
    auto& session = sessions[static_cast<unsigned>(ulSessionID - 1)];
    if (session.ended) return EResult::k_EResultExpired; // TODO is this correct?

    auto att = get_or_create_session_att(pstrName, session, AttributeType_t::Int64);
    if (att->type != AttributeType_t::Int64) return EResult::k_EResultFail; // TODO is this correct?

    att->ll_data = llData;
    return EResult::k_EResultOK;
}

EResult Steam_GameStats::AddRowAttributeInt64( uint64 ulRowID, const char *pstrName, int64 llData )
{
    PRINT_DEBUG("%llu, '%s', %lli", ulRowID, pstrName, llData);
    std::lock_guard lock(global_mutex);
    
    if (!pstrName) return EResult::k_EResultInvalidParam; // TODO is this correct?
    
    auto active_session = std::find_if(sessions.rbegin(), sessions.rend(), [](const Session_t &item){ return !item.ended; });
    if (sessions.rend() == active_session) return EResult::k_EResultFail; // TODO is this correct?

    auto &table = active_session->tables.back().second;
    if (ulRowID >= table.rows.size()) return EResult::k_EResultInvalidParam; // TODO is this correct?

    auto att = get_or_create_row_att(ulRowID, pstrName, table, AttributeType_t::Int64);
    if (att->type != AttributeType_t::Int64) return EResult::k_EResultFail; // TODO is this correct?

    att->ll_data = llData;
    return EResult::k_EResultOK;
}



// --- steam callbacks

void Steam_GameStats::steam_run_callback()
{
    // nothing
}



// --- networking callbacks

// user connect/disconnect
void Steam_GameStats::network_callback_low_level(Common_Message *msg)
{
    switch (msg->low_level().type())
    {
    case Low_Level::CONNECT:
        // nothing
    break;
    
    case Low_Level::DISCONNECT:
        // nothing
    break;
    
    default:
        PRINT_DEBUG("unknown type %i", (int)msg->low_level().type());
    break;
    }
}
