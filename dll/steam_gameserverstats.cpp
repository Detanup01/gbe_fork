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
   <http://www.gnu.org/licenses/>.  */

#include "dll/steam_gameserverstats.h"

// TODO not sure what's the real value
#define PENDING_RequestUserStats_TIMEOUT 7.0


void Steam_GameServerStats::steam_gameserverstats_network_callback(void *object, Common_Message *msg)
{
    // PRINT_DEBUG("Steam_GameServerStats::steam_gameserverstats_network_callback\n");

    auto steam_gameserverstats = (Steam_GameServerStats *)object;
    steam_gameserverstats->network_callback(msg);
}

void Steam_GameServerStats::steam_gameserverstats_run_every_runcb(void *object)
{
    // PRINT_DEBUG("Steam_GameServerStats::steam_gameserverstats_run_every_runcb\n");

    auto steam_gameserverstats = (Steam_GameServerStats *)object;
    steam_gameserverstats->steam_run_callback();
}

Steam_GameServerStats::CachedStat* Steam_GameServerStats::find_stat(CSteamID steamIDUser, const std::string &key)
{
    auto it_data = all_users_data.find(steamIDUser.ConvertToUint64());
    if (all_users_data.end() == it_data) return {}; // no user

    auto it_stat = std::find_if(
        it_data->second.stats.begin(), it_data->second.stats.end(),
        [&key](std::pair<const std::string, CachedStat> &item) {
            const std::string &name = item.first;
            return key.size() == name.size() &&
                std::equal(
                    name.begin(), name.end(), key.begin(),
                    [](char a, char b) { return std::tolower(a) == std::tolower(b); }
                );
        }
    );

    if (it_data->second.stats.end() == it_stat) return {}; // no stat

    return &it_stat->second;
}

Steam_GameServerStats::CachedAchievement* Steam_GameServerStats::find_ach(CSteamID steamIDUser, const std::string &key)
{
    auto it_data = all_users_data.find(steamIDUser.ConvertToUint64());
    if (all_users_data.end() == it_data) return {}; // no user

    auto it_ach = std::find_if(
        it_data->second.achievements.begin(), it_data->second.achievements.end(),
        [&key](std::pair<const std::string, CachedAchievement> &item) {
            const std::string &name = item.first;
            return key.size() == name.size() &&
                std::equal(
                    name.begin(), name.end(), key.begin(),
                    [](char a, char b) { return std::tolower(a) == std::tolower(b); }
                );
        }
    );

    if (it_data->second.achievements.end() == it_ach) return {}; // no user

    return &it_ach->second;
}

Steam_GameServerStats::Steam_GameServerStats(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb)
{
    this->settings = settings;
    this->network = network;
    this->callback_results = callback_results;
    this->callbacks = callbacks;
    this->run_every_runcb = run_every_runcb;

    this->network->setCallback(CALLBACK_ID_GAMESERVER_STATS, settings->get_local_steam_id(), &Steam_GameServerStats::steam_gameserverstats_network_callback, this);
    this->run_every_runcb->add(&Steam_GameServerStats::steam_gameserverstats_run_every_runcb, this);

}

Steam_GameServerStats::~Steam_GameServerStats()
{
    this->network->rmCallback(CALLBACK_ID_GAMESERVER_STATS, settings->get_local_steam_id(), &Steam_GameServerStats::steam_gameserverstats_network_callback, this);
    this->run_every_runcb->remove(&Steam_GameServerStats::steam_gameserverstats_run_every_runcb, this);
}


// downloads stats for the user
// returns a GSStatsReceived_t callback when completed
// if the user has no stats, GSStatsReceived_t.m_eResult will be set to k_EResultFail
// these stats will only be auto-updated for clients playing on the server. For other
// users you'll need to call RequestUserStats() again to refresh any data
STEAM_CALL_RESULT( GSStatsReceived_t )
SteamAPICall_t Steam_GameServerStats::RequestUserStats( CSteamID steamIDUser )
{
    PRINT_DEBUG("Steam_GameServerStats::RequestUserStats %llu\n", (uint64)steamIDUser.ConvertToUint64());
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    struct RequestAllStats new_request{};
    new_request.created = std::chrono::high_resolution_clock::now();
    new_request.steamAPICall = callback_results->reserveCallResult();
    new_request.steamIDUser = steamIDUser;

    pending_RequestUserStats.push_back(new_request);

    auto initial_stats_msg = new GameServerStats_Messages::InitialAllStats();
    initial_stats_msg->set_steam_api_call(new_request.steamAPICall);

    auto gameserverstats_messages = new GameServerStats_Messages();
    gameserverstats_messages->set_type(GameServerStats_Messages::Request_AllUserStats);
    gameserverstats_messages->set_allocated_initial_user_stats(initial_stats_msg);
    
    Common_Message msg{};
    // https://protobuf.dev/reference/cpp/cpp-generated/#string
    // set_allocated_xxx() takes ownership of the allocated object, no need to delete
    msg.set_allocated_gameserver_stats_messages(gameserverstats_messages);
    msg.set_source_id(settings->get_local_steam_id().ConvertToUint64());
    msg.set_dest_id(new_request.steamIDUser.ConvertToUint64());
    network->sendTo(&msg, true);

    return new_request.steamAPICall;
}


// requests stat information for a user, usable after a successful call to RequestUserStats()
bool Steam_GameServerStats::GetUserStat( CSteamID steamIDUser, const char *pchName, int32 *pData )
{
    PRINT_DEBUG("Steam_GameServerStats::GetUserStat <int32> %llu '%s' %p\n", (uint64)steamIDUser.ConvertToUint64(), pchName, pData);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pchName) return false;

    auto stat = find_stat(steamIDUser, pchName);
    if (!stat) return false;
    if (stat->stat.stat_type() != GameServerStats_Messages::StatInfo::STAT_TYPE_INT) return false;

    if (pData) *pData = stat->stat.value_int();
    return true;
}

bool Steam_GameServerStats::GetUserStat( CSteamID steamIDUser, const char *pchName, float *pData )
{
    PRINT_DEBUG("Steam_GameServerStats::GetUserStat <float> %llu '%s' %p\n", (uint64)steamIDUser.ConvertToUint64(), pchName, pData);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pchName) return false;

    auto stat = find_stat(steamIDUser, pchName);
    if (!stat) return false;
    if (stat->stat.stat_type() == GameServerStats_Messages::StatInfo::STAT_TYPE_INT) return false;

    if (pData) *pData = stat->stat.value_float();
    return true;
}

bool Steam_GameServerStats::GetUserAchievement( CSteamID steamIDUser, const char *pchName, bool *pbAchieved )
{
    PRINT_DEBUG("Steam_GameServerStats::GetUserAchievement %llu '%s' %p\n", (uint64)steamIDUser.ConvertToUint64(), pchName, pbAchieved);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pchName) return false;

    auto ach = find_ach(steamIDUser, pchName);
    if (!ach) return false;
    if (pbAchieved) *pbAchieved = ach->ach.achieved();

    return true;
}


// Set / update stats and achievements. 
// Note: These updates will work only on stats game servers are allowed to edit and only for 
// game servers that have been declared as officially controlled by the game creators. 
// Set the IP range of your official servers on the Steamworks page
bool Steam_GameServerStats::SetUserStat( CSteamID steamIDUser, const char *pchName, int32 nData )
{
    PRINT_DEBUG("Steam_GameServerStats::SetUserStat <int32> %llu '%s' %i\n", (uint64)steamIDUser.ConvertToUint64(), pchName, nData);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pchName) return false;

    auto stat = find_stat(steamIDUser, pchName);
    if (!stat) return false;
    if (stat->stat.stat_type() != GameServerStats_Messages::StatInfo::STAT_TYPE_INT) return false;
    if (stat->stat.value_int() == nData) return true; // don't waste time

    stat->dirty = true;
    stat->stat.set_value_int(nData);
    return true;
}

bool Steam_GameServerStats::SetUserStat( CSteamID steamIDUser, const char *pchName, float fData )
{
    PRINT_DEBUG("Steam_GameServerStats::SetUserStat <float> %llu '%s' %f\n", (uint64)steamIDUser.ConvertToUint64(), pchName, fData);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pchName) return false;

    auto stat = find_stat(steamIDUser, pchName);
    if (!stat) return false;
    if (stat->stat.stat_type() == GameServerStats_Messages::StatInfo::STAT_TYPE_INT) return false;
    if (stat->stat.value_float() == fData) return true; // don't waste time

    stat->dirty = true;
    stat->stat.set_value_float(fData); // we set the float field in case it's float or avg
    return true;
}

bool Steam_GameServerStats::UpdateUserAvgRateStat( CSteamID steamIDUser, const char *pchName, float flCountThisSession, double dSessionLength )
{
    PRINT_DEBUG("Steam_GameServerStats::UpdateUserAvgRateStat %llu '%s'\n", (uint64)steamIDUser.ConvertToUint64(), pchName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pchName) return false;

    auto stat = find_stat(steamIDUser, pchName);
    if (!stat) return false;
    if (stat->stat.stat_type() == GameServerStats_Messages::StatInfo::STAT_TYPE_INT) return false;
    // don't waste time
    if (stat->stat.has_value_avg() &&
        stat->stat.value_avg().count_this_session() == flCountThisSession &&
        stat->stat.value_avg().session_length() == dSessionLength) {
        return true;
    }

    stat->dirty = true;

    // https://protobuf.dev/reference/cpp/cpp-generated/#string
    // set_allocated_xxx() takes ownership of the allocated object, no need to delete
    auto avg_info = new GameServerStats_Messages::StatInfo::AvgStatInfo();
    avg_info->set_count_this_session(flCountThisSession);
    avg_info->set_session_length(dSessionLength);
    stat->stat.set_allocated_value_avg(avg_info);

    return true;
}


bool Steam_GameServerStats::SetUserAchievement( CSteamID steamIDUser, const char *pchName )
{
    PRINT_DEBUG("Steam_GameServerStats::SetUserAchievement %llu '%s'\n", (uint64)steamIDUser.ConvertToUint64(), pchName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pchName) return false;

    auto ach = find_ach(steamIDUser, pchName);
    if (!ach) return false;
    if (ach->ach.achieved() == true) return true; // don't waste time
    
    ach->dirty = true;
    ach->ach.set_achieved(true);
    return true;
}

bool Steam_GameServerStats::ClearUserAchievement( CSteamID steamIDUser, const char *pchName )
{
    PRINT_DEBUG("Steam_GameServerStats::ClearUserAchievement %llu '%s'\n", (uint64)steamIDUser.ConvertToUint64(), pchName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pchName) return false;

    auto ach = find_ach(steamIDUser, pchName);
    if (!ach) return false;
    if (ach->ach.achieved() == false) return true; // don't waste time
    
    ach->dirty = true;
    ach->ach.set_achieved(false);
    return true;
}


// Store the current data on the server, will get a GSStatsStored_t callback when set.
//
// If the callback has a result of k_EResultInvalidParam, one or more stats 
// uploaded has been rejected, either because they broke constraints
// or were out of date. In this case the server sends back updated values.
// The stats should be re-iterated to keep in sync.
STEAM_CALL_RESULT( GSStatsStored_t )
SteamAPICall_t Steam_GameServerStats::StoreUserStats( CSteamID steamIDUser )
{
    // it's not necessary to send all data here
    PRINT_DEBUG("Steam_GameServerStats::StoreUserStats\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    GSStatsStored_t data{};

    if (all_users_data.count(steamIDUser.ConvertToUint64())) {
        data.m_eResult = EResult::k_EResultOK;
    } else {
        data.m_eResult = EResult::k_EResultFail;
    }
    data.m_steamIDUser = steamIDUser;
    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data), 0.01);
}



// --- steam callbacks

void Steam_GameServerStats::remove_timedout_userstats_requests()
{
    if (pending_RequestUserStats.empty()) return;

    // send all pending RequestUserStats() requests
    for (auto &pendReq : pending_RequestUserStats) {
        if (check_timedout(pendReq.created, PENDING_RequestUserStats_TIMEOUT)) {
            pendReq.timeout = true;

            GSStatsReceived_t data{};
            data.m_eResult = k_EResultTimeout;
            data.m_steamIDUser = pendReq.steamIDUser;
            callback_results->addCallResult(pendReq.steamAPICall, data.k_iCallback, &data, sizeof(data));
            
            PRINT_DEBUG(
                "Steam_GameServerStats::steam_run_callback RequestUserStats timeout, %llu\n",
                pendReq.steamIDUser.ConvertToUint64()
            );
        }
    }

    // remove all timedout requests
    auto itrm = std::remove_if(
        pending_RequestUserStats.begin(), pending_RequestUserStats.end(),
        [](const struct RequestAllStats &item) { return item.timeout; }
    );
    pending_RequestUserStats.erase(itrm, pending_RequestUserStats.end());
}

void Steam_GameServerStats::collect_and_send_updated_user_stats()
{
	std::map<uint64, UserData> updated_users_data{}; // new data to send

    // collect new data
    for (auto &this_user : all_users_data) { // foreach user
        uint64 user_steamid = this_user.first;

        // collect changed stats
        for (auto &user_stat : this_user.second.stats) {
            if (user_stat.second.dirty) {
                user_stat.second.dirty = false;
                updated_users_data[user_steamid].stats[user_stat.first] = user_stat.second;
                // clear this to avoid sending it to the user next time
                if (user_stat.second.stat.has_value_avg()) user_stat.second.stat.clear_value_avg();
            }
        }

        // collect changed achievements
        for (auto &user_ach : this_user.second.achievements) {
            if (user_ach.second.dirty) {
                user_ach.second.dirty = false;
                updated_users_data[user_steamid].achievements[user_ach.first] = user_ach.second;
            }
        }
    }

    // send new user stats
    for (auto &user_new_data : updated_users_data) { // foreach user
        uint64 user_steamid = user_new_data.first;
        const auto &new_data = user_new_data.second;
        auto updated_stats_msg = new GameServerStats_Messages::AllStats();

        // copy new stats
        auto &updated_stats_map = *updated_stats_msg->mutable_user_stats();
        for (auto &new_stat : new_data.stats) {
            updated_stats_map[new_stat.first] = new_stat.second.stat;
            
        }

        // copy new achievements
        auto &updated_achs_map = *updated_stats_msg->mutable_user_achievements();
        for (auto &new_ach : new_data.achievements) {
            updated_achs_map[new_ach.first] = new_ach.second.ach;
            
        }
        
        auto gameserverstats_msg = new GameServerStats_Messages();
        gameserverstats_msg->set_type(GameServerStats_Messages::UpdateUserStats);
        gameserverstats_msg->set_allocated_update_user_stats(updated_stats_msg);
        
        Common_Message msg{};
        // https://protobuf.dev/reference/cpp/cpp-generated/#string
        // set_allocated_xxx() takes ownership of the allocated object, no need to delete
        msg.set_allocated_gameserver_stats_messages(gameserverstats_msg);
        msg.set_source_id(settings->get_local_steam_id().ConvertToUint64());
        msg.set_dest_id(user_steamid);
        network->sendTo(&msg, true);

        PRINT_DEBUG(
            "Steam_GameServerStats::collect_and_send_updated_user_stats server sent updated stats %llu: %zu stats, %zu achievements\n",
            user_steamid, updated_stats_msg->user_stats().size(), updated_stats_msg->user_achievements().size()
        );
    }

}

void Steam_GameServerStats::steam_run_callback()
{
    remove_timedout_userstats_requests();
    collect_and_send_updated_user_stats();
}



// --- networking callbacks

void Steam_GameServerStats::network_callback_initial_stats(Common_Message *msg)
{
    uint64 user_steamid = msg->source_id();

    PRINT_DEBUG("Steam_GameServerStats::network_callback_initial_stats player sent all their stats %llu\n", user_steamid);
    if (!msg->gameserver_stats_messages().has_initial_user_stats() ||
        !msg->gameserver_stats_messages().initial_user_stats().has_all_data()) {
        PRINT_DEBUG("Steam_GameServerStats::network_callback_initial_stats error empty msg\n");
        return;
    }

    const auto &new_data = msg->gameserver_stats_messages().initial_user_stats();
    
    // find this pending request
    auto it = std::find_if(
        pending_RequestUserStats.begin(), pending_RequestUserStats.end(),
        [=](const RequestAllStats &item) { 
            return item.steamAPICall == new_data.steam_api_call() &&
                item.steamIDUser == user_steamid;
        }
    );
    if (pending_RequestUserStats.end()  == it) { // timeout and already removed
        PRINT_DEBUG("Steam_GameServerStats::network_callback_initial_stats error got all player stats but pending request timedout and removed\n");
        return;
    }

    // remove this pending request
    pending_RequestUserStats.erase(it);
    
    // copy new stats
    auto &current_stats = all_users_data[user_steamid].stats;
    current_stats.clear();
    for (const auto &new_stat : new_data.all_data().user_stats()) {
        current_stats[new_stat.first].stat = new_stat.second;
    }
    
    // copy new achievements
    auto &current_achievements = all_users_data[user_steamid].achievements;
    current_achievements.clear();
    for (const auto &new_ach : new_data.all_data().user_achievements()) {
        current_achievements[new_ach.first].ach = new_ach.second;
    }

    GSStatsReceived_t data{};
    data.m_eResult = EResult::k_EResultOK;
    data.m_steamIDUser = user_steamid;
    callback_results->addCallResult(it->steamAPICall, data.k_iCallback, &data, sizeof(data));
    
    PRINT_DEBUG(
        "Steam_GameServerStats::network_callback_initial_stats server got all player stats %llu: %zu stats, %zu achievements\n",
        user_steamid, all_users_data[user_steamid].stats.size(), all_users_data[user_steamid].achievements.size()
    );

    
}

void Steam_GameServerStats::network_callback_updated_stats(Common_Message *msg)
{
    uint64 user_steamid = msg->source_id();

    PRINT_DEBUG("Steam_GameServerStats::network_callback_updated_stats player sent updated stats %llu\n", user_steamid);
    if (!msg->gameserver_stats_messages().has_update_user_stats()) {
        PRINT_DEBUG("Steam_GameServerStats::network_callback_updated_stats error empty msg\n");
        return;
    }

    auto &current_user_data = all_users_data[user_steamid];
    auto &new_user_data =msg->gameserver_stats_messages().update_user_stats();

    // update stats
    for (auto &new_stat : new_user_data.user_stats()) {
        auto &current_stat = current_user_data.stats[new_stat.first];
        current_stat.dirty = false;
        current_stat.stat = new_stat.second;
    }

    // update achievements
    for (auto &new_ach : new_user_data.user_achievements()) {
        auto &current_ach = current_user_data.achievements[new_ach.first];
        current_ach.dirty = false;
        current_ach.ach = new_ach.second;
    }
    
    PRINT_DEBUG(
        "Steam_GameServerStats::network_callback got updated user stats %llu: %zu stats, %zu achievements\n",
        user_steamid, new_user_data.user_stats().size(), new_user_data.user_achievements().size()
    );
}

// only triggered when we have a message
void Steam_GameServerStats::network_callback(Common_Message *msg)
{
    switch (msg->gameserver_stats_messages().type())
    {
    // user sent all their stats
    case GameServerStats_Messages::Response_AllUserStats:
        network_callback_initial_stats(msg);
    break;

    // user has updated/new stats
    case GameServerStats_Messages::UpdateUserStats:
        network_callback_updated_stats(msg);
    break;
    
    default:
        PRINT_DEBUG("Steam_GameServerStats::network_callback unhandled type %i\n", (int)msg->gameserver_stats_messages().type());
    break;
    }
}
