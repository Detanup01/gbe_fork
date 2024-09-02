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

#include "dll/steam_user_stats.h"
#include <random>



// change stats without sending back to server
bool Steam_User_Stats::clear_stats_internal()
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    bool notify_server = false;
    
    for (const auto &stat : settings->getStats()) {
        std::string stat_name(common_helpers::to_lower(stat.first));

        switch (stat.second.type)
        {
        case GameServerStats_Messages::StatInfo::STAT_TYPE_INT: {
            auto data = stat.second.default_value_int;

            bool needs_disk_write = false;
            auto it_res = stats_cache_int.find(stat_name);
            if (stats_cache_int.end() == it_res || it_res->second != data) {
                needs_disk_write = true;
                notify_server = true;
            }

            stats_cache_int[stat_name] = data;
            
            if (needs_disk_write) local_storage->store_data(Local_Storage::stats_storage_folder, stat_name, (char *)&data, sizeof(data));
        }
        break;

        case GameServerStats_Messages::StatInfo::STAT_TYPE_FLOAT:
        case GameServerStats_Messages::StatInfo::STAT_TYPE_AVGRATE: {
            auto data = stat.second.default_value_float;

            bool needs_disk_write = false;
            auto it_res = stats_cache_float.find(stat_name);
            if (stats_cache_float.end() == it_res || it_res->second != data) {
                needs_disk_write = true;
                notify_server = true;
            }

            stats_cache_float[stat_name] = data;
            
            if (needs_disk_write) local_storage->store_data(Local_Storage::stats_storage_folder, stat_name, (char *)&data, sizeof(data));
        }
        break;
        
        default: PRINT_DEBUG("unhandled type %i", (int)stat.second.type); break;
        }
    }

    return notify_server;
}

Steam_User_Stats::InternalSetResult<int32> Steam_User_Stats::set_stat_internal( const char *pchName, int32 nData )
{
    PRINT_DEBUG("<int32> '%s' = %i", pchName, nData);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    Steam_User_Stats::InternalSetResult<int32> result{};

    if (!pchName) return result;
    std::string stat_name(common_helpers::to_lower(pchName));

    const auto &stats_config = settings->getStats();
    auto stats_data = stats_config.find(stat_name);
    if (stats_config.end() == stats_data && settings->allow_unknown_stats) {
        Stat_config cfg{};
        cfg.type = GameServerStats_Messages::StatInfo::STAT_TYPE_INT;
        cfg.default_value_int = 0;
        stats_data = settings->setStatDefiniton(stat_name, cfg);
    }

    if (stats_config.end() == stats_data) return result;
    if (stats_data->second.type != GameServerStats_Messages::StatInfo::STAT_TYPE_INT) return result;

    result.internal_name = stat_name;
    result.current_val = nData;

    auto cached_stat = stats_cache_int.find(stat_name);
    if (stats_cache_int.end() != cached_stat) {
        if (cached_stat->second == nData) {
            result.success = true;
            return result;
        }
    }

    auto stat_trigger = achievement_stat_trigger.find(stat_name);
    if (stat_trigger != achievement_stat_trigger.end()) {
        for (auto &t : stat_trigger->second) {
            if (t.should_unlock_ach(nData)) {
                set_achievement_internal(t.name.c_str());
            }
            if (settings->stat_achievement_progress_functionality && t.should_indicate_progress(nData)) {
                bool indicate_progress = true;
                // appid 1482380 needs that otherwise it will spam progress indications while driving
                if (settings->save_only_higher_stat_achievement_progress) {
                    try {
                        auto user_ach_it =  user_achievements.find(t.name);
                        if (user_achievements.end() != user_ach_it) {
                            auto user_progress_it = user_ach_it->find("progress");
                            if (user_ach_it->end() != user_progress_it) {
                                int32 user_progress = *user_progress_it;
                                if (nData <= user_progress) {
                                    indicate_progress = false;
                                }
                            }
                        }
                    } catch(...){}
                }

                if (indicate_progress) {
                    IndicateAchievementProgress(t.name.c_str(), nData, std::stoi(t.max_value));
                }
            }
        }
    }

    if (local_storage->store_data(Local_Storage::stats_storage_folder, stat_name, (char* )&nData, sizeof(nData)) == sizeof(nData)) {
        stats_cache_int[stat_name] = nData;
        result.success = true;
        result.notify_server = !settings->disable_sharing_stats_with_gameserver;
        return result;
    }

    return result;
}

Steam_User_Stats::InternalSetResult<std::pair<GameServerStats_Messages::StatInfo::Stat_Type, float>> Steam_User_Stats::set_stat_internal( const char *pchName, float fData )
{
    PRINT_DEBUG("<float> '%s' = %f", pchName, fData);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    Steam_User_Stats::InternalSetResult<std::pair<GameServerStats_Messages::StatInfo::Stat_Type, float>> result{};

    if (!pchName) return result;
    std::string stat_name(common_helpers::to_lower(pchName));

    const auto &stats_config = settings->getStats();
    auto stats_data = stats_config.find(stat_name);
    if (stats_config.end() == stats_data && settings->allow_unknown_stats) {
        Stat_config cfg{};
        cfg.type = GameServerStats_Messages::StatInfo::STAT_TYPE_FLOAT;
        cfg.default_value_float = 0;
        stats_data = settings->setStatDefiniton(stat_name, cfg);
    }

    if (stats_config.end() == stats_data) return result;
    if (stats_data->second.type == GameServerStats_Messages::StatInfo::STAT_TYPE_INT) return result;

    result.internal_name = stat_name;
    result.current_val.first = stats_data->second.type;
    result.current_val.second = fData;

    auto cached_stat = stats_cache_float.find(stat_name);
    if (stats_cache_float.end() != cached_stat) {
        if (cached_stat->second == fData) {
            result.success = true;
            return result;
        }
    }

    auto stat_trigger = achievement_stat_trigger.find(stat_name);
    if (stat_trigger != achievement_stat_trigger.end()) {
        for (auto &t : stat_trigger->second) {
            if (t.should_unlock_ach(fData)) {
                set_achievement_internal(t.name.c_str());
            }
            if (settings->stat_achievement_progress_functionality && t.should_indicate_progress(fData)) {
                bool indicate_progress = true;
                // appid 1482380 needs that otherwise it will spam progress indications while driving
                if (settings->save_only_higher_stat_achievement_progress) {
                    try {
                        auto user_ach_it =  user_achievements.find(t.name);
                        if (user_achievements.end() != user_ach_it) {
                            auto user_progress_it = user_ach_it->find("progress");
                            if (user_ach_it->end() != user_progress_it) {
                                float user_progress = *user_progress_it;
                                if (fData <= user_progress) {
                                    indicate_progress = false;
                                }
                            }
                        }
                    } catch(...){}
                }

                if (indicate_progress) {
                    IndicateAchievementProgress(t.name.c_str(), (uint32)fData, (uint32)std::stof(t.max_value));
                }
            }
        }
    }

    if (local_storage->store_data(Local_Storage::stats_storage_folder, stat_name, (char* )&fData, sizeof(fData)) == sizeof(fData)) {
        stats_cache_float[stat_name] = fData;
        result.success = true;
        result.notify_server = !settings->disable_sharing_stats_with_gameserver;
        return result;
    }

    return result;
}

Steam_User_Stats::InternalSetResult<std::pair<GameServerStats_Messages::StatInfo::Stat_Type, float>> Steam_User_Stats::update_avg_rate_stat_internal( const char *pchName, float flCountThisSession, double dSessionLength )
{
    PRINT_DEBUG("%s", pchName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    Steam_User_Stats::InternalSetResult<std::pair<GameServerStats_Messages::StatInfo::Stat_Type, float>> result{};

    if (!pchName) return result;
    std::string stat_name(common_helpers::to_lower(pchName));

    const auto &stats_config = settings->getStats();
    auto stats_data = stats_config.find(stat_name);
    if (stats_config.end() == stats_data && settings->allow_unknown_stats) {
        Stat_config cfg{};
        cfg.type = GameServerStats_Messages::StatInfo::STAT_TYPE_AVGRATE;
        cfg.default_value_float = 0;
        stats_data = settings->setStatDefiniton(stat_name, cfg);
    }

    if (stats_config.end() == stats_data) return result;
    if (stats_data->second.type == GameServerStats_Messages::StatInfo::STAT_TYPE_INT) return result;

    result.internal_name = stat_name;

    char data[sizeof(float) + sizeof(float) + sizeof(double)];
    int read_data = local_storage->get_data(Local_Storage::stats_storage_folder, stat_name, (char* )data, sizeof(*data));
    float oldcount = 0;
    double oldsessionlength = 0;
    if (read_data == sizeof(data)) {
        memcpy(&oldcount, data + sizeof(float), sizeof(oldcount));
        memcpy(&oldsessionlength, data + sizeof(float) + sizeof(float), sizeof(oldsessionlength));
    }

    oldcount += flCountThisSession;
    oldsessionlength += dSessionLength;

    float average = static_cast<float>(oldcount / oldsessionlength);
    memcpy(data, &average, sizeof(average));
    memcpy(data + sizeof(float), &oldcount, sizeof(oldcount));
    memcpy(data + sizeof(float) * 2, &oldsessionlength, sizeof(oldsessionlength));

    result.current_val.first = stats_data->second.type;
    result.current_val.second = average;

    if (local_storage->store_data(Local_Storage::stats_storage_folder, stat_name, data, sizeof(data)) == sizeof(data)) {
        stats_cache_float[stat_name] = average;
        result.success = true;
        result.notify_server = !settings->disable_sharing_stats_with_gameserver;
        return result;
    }

    return result;
}


void Steam_User_Stats::steam_user_stats_network_stats(void *object, Common_Message *msg)
{
    // PRINT_DEBUG_ENTRY();

    auto inst = (Steam_User_Stats *)object;
    inst->network_callback_stats(msg);
}


// Ask the server to send down this user's data and achievements for this game
STEAM_CALL_BACK( UserStatsReceived_t )
bool Steam_User_Stats::RequestCurrentStats()
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    UserStatsReceived_t data{};
    data.m_nGameID = settings->get_local_game_id().ToUint64();
    data.m_eResult = k_EResultOK;
    data.m_steamIDUser = settings->get_local_steam_id();
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data), 0.1);
    return true;
}


// Data accessors
bool Steam_User_Stats::GetStat( const char *pchName, int32 *pData )
{
    PRINT_DEBUG("<int32> '%s' %p", pchName, pData);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pchName) return false;
    std::string stat_name = common_helpers::to_lower(pchName);

    const auto &stats_config = settings->getStats();
    auto stats_data = stats_config.find(stat_name);
    if (stats_config.end() == stats_data && settings->allow_unknown_stats) {
        Stat_config cfg{};
        cfg.type = GameServerStats_Messages::StatInfo::STAT_TYPE_INT;
        cfg.default_value_int = 0;
        stats_data = settings->setStatDefiniton(stat_name, cfg);
    }

    if (stats_config.end() == stats_data) return false;
    if (stats_data->second.type != GameServerStats_Messages::StatInfo::STAT_TYPE_INT) return false;

    auto cached_stat = stats_cache_int.find(stat_name);
    if (cached_stat != stats_cache_int.end()) {
        if (pData) *pData = cached_stat->second;
        return true;
    }

    int32 output = 0;
    int read_data = local_storage->get_data(Local_Storage::stats_storage_folder, stat_name, (char* )&output, sizeof(output));
    if (read_data == sizeof(int32)) {
        stats_cache_int[stat_name] = output;
        if (pData) *pData = output;
        return true;
    }

    stats_cache_int[stat_name] = stats_data->second.default_value_int;
    if (pData) *pData = stats_data->second.default_value_int;
    return true;
}

bool Steam_User_Stats::GetStat( const char *pchName, float *pData )
{
    PRINT_DEBUG("<float> '%s' %p", pchName, pData);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pchName) return false;
    std::string stat_name = common_helpers::to_lower(pchName);

    const auto &stats_config = settings->getStats();
    auto stats_data = stats_config.find(stat_name);
    if (stats_config.end() == stats_data && settings->allow_unknown_stats) {
        Stat_config cfg{};
        cfg.type = GameServerStats_Messages::StatInfo::STAT_TYPE_FLOAT;
        cfg.default_value_float = 0;
        stats_data = settings->setStatDefiniton(stat_name, cfg);
    }

    if (stats_config.end() == stats_data) return false;
    if (stats_data->second.type == GameServerStats_Messages::StatInfo::STAT_TYPE_INT) return false;

    auto cached_stat = stats_cache_float.find(stat_name);
    if (cached_stat != stats_cache_float.end()) {
        if (pData) *pData = cached_stat->second;
        return true;
    }

    float output = 0.0;
    int read_data = local_storage->get_data(Local_Storage::stats_storage_folder, stat_name, (char* )&output, sizeof(output));
    if (read_data == sizeof(float)) {
        stats_cache_float[stat_name] = output;
        if (pData) *pData = output;
        return true;
    }

    stats_cache_float[stat_name] = stats_data->second.default_value_float;
    if (pData) *pData = stats_data->second.default_value_float;
    return true;
}


// Set / update data
bool Steam_User_Stats::SetStat( const char *pchName, int32 nData )
{
    PRINT_DEBUG("<int32> '%s' = %i", pchName, nData);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    auto ret = set_stat_internal(pchName, nData );
    if (ret.success && ret.notify_server ) {
        auto &new_stat = (*pending_server_updates.mutable_user_stats())[ret.internal_name];
        new_stat.set_stat_type(GameServerStats_Messages::StatInfo::STAT_TYPE_INT);
        new_stat.set_value_int(ret.current_val);

        if (settings->immediate_gameserver_stats) send_updated_stats();
    }

    return ret.success;
}

bool Steam_User_Stats::SetStat( const char *pchName, float fData )
{
    PRINT_DEBUG("<float> '%s' = %f", pchName, fData);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    auto ret = set_stat_internal(pchName, fData);
    if (ret.success && ret.notify_server) {
        auto &new_stat = (*pending_server_updates.mutable_user_stats())[ret.internal_name];
        new_stat.set_stat_type(ret.current_val.first);
        new_stat.set_value_float(ret.current_val.second);

        if (settings->immediate_gameserver_stats) send_updated_stats();
    }
    
    return ret.success;
}

bool Steam_User_Stats::UpdateAvgRateStat( const char *pchName, float flCountThisSession, double dSessionLength )
{
    PRINT_DEBUG("'%s'", pchName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    auto ret = update_avg_rate_stat_internal(pchName, flCountThisSession, dSessionLength);
    if (ret.success && ret.notify_server) {
        auto &new_stat = (*pending_server_updates.mutable_user_stats())[ret.internal_name];
        new_stat.set_stat_type(ret.current_val.first);
        new_stat.set_value_float(ret.current_val.second);

        if (settings->immediate_gameserver_stats) send_updated_stats();
    }
    
    return ret.success;
}


// Store the current data on the server, will get a callback when set
// And one callback for every new achievement
//
// If the callback has a result of k_EResultInvalidParam, one or more stats 
// uploaded has been rejected, either because they broke constraints
// or were out of date. In this case the server sends back updated values.
// The stats should be re-iterated to keep in sync.
bool Steam_User_Stats::StoreStats()
{
    // no need to exchange data with gameserver, we already do that in run_callback() and on each stat/ach update (immediate mode)
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    UserStatsStored_t data{};
    data.m_eResult = k_EResultOK;
    data.m_nGameID = settings->get_local_game_id().ToUint64();
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data), 0.01);

    for (auto &kv : store_stats_trigger) {
        callbacks->addCBResult(kv.second.k_iCallback, &kv.second, sizeof(kv.second));
    }
    store_stats_trigger.clear();

    return true;
}


// Friends stats

// downloads stats for the user
// returns a UserStatsReceived_t received when completed
// if the other user has no stats, UserStatsReceived_t.m_eResult will be set to k_EResultFail
// these stats won't be auto-updated; you'll need to call RequestUserStats() again to refresh any data
STEAM_CALL_RESULT( UserStatsReceived_t )
SteamAPICall_t Steam_User_Stats::RequestUserStats( CSteamID steamIDUser )
{
    PRINT_DEBUG("%llu", steamIDUser.ConvertToUint64());
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    // Enable this to allow hot reload achievements status
    //if (steamIDUser == settings->get_local_steam_id()) {
    //    load_achievements();
    //}

    UserStatsReceived_t data{};
    data.m_nGameID = settings->get_local_game_id().ToUint64();
    data.m_eResult = k_EResultOK;
    data.m_steamIDUser = steamIDUser;
    // appid 756800 expects both: a callback (global event occurring in the Steam environment),
    // and a callresult (the specific result of this function call)
    // otherwise it will lock-up and hang at startup
    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data), 0.1);
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data), 0.1);
    return ret;
}


// requests stat information for a user, usable after a successful call to RequestUserStats()
bool Steam_User_Stats::GetUserStat( CSteamID steamIDUser, const char *pchName, int32 *pData )
{
    PRINT_DEBUG("'%s' %llu", pchName, steamIDUser.ConvertToUint64());
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pchName) return false;

    if (steamIDUser == settings->get_local_steam_id()) {
        GetStat(pchName, pData);
    } else {
        *pData = 0;
    }

    return true;
}

bool Steam_User_Stats::GetUserStat( CSteamID steamIDUser, const char *pchName, float *pData )
{
    PRINT_DEBUG("%s %llu", pchName, steamIDUser.ConvertToUint64());
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pchName) return false;

    if (steamIDUser == settings->get_local_steam_id()) {
        GetStat(pchName, pData);
    } else {
        *pData = 0;
    }

    return true;
}


// Reset stats 
bool Steam_User_Stats::ResetAllStats( bool bAchievementsToo )
{
    PRINT_DEBUG("bAchievementsToo = %i", (int)bAchievementsToo);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    clear_stats_internal(); // this will save stats to disk if necessary
    if (!settings->disable_sharing_stats_with_gameserver) {
        for (const auto &stat : settings->getStats()) {
            std::string stat_name(common_helpers::to_lower(stat.first));

            auto &new_stat = (*pending_server_updates.mutable_user_stats())[stat_name];
            new_stat.set_stat_type(stat.second.type);

            switch (stat.second.type)
            {
            case GameServerStats_Messages::StatInfo::STAT_TYPE_INT:
                new_stat.set_value_int(stat.second.default_value_int);
            break;

            case GameServerStats_Messages::StatInfo::STAT_TYPE_AVGRATE:
            case GameServerStats_Messages::StatInfo::STAT_TYPE_FLOAT:
                new_stat.set_value_float(stat.second.default_value_float);
            break;
            
            default: PRINT_DEBUG("unhandled type %i", (int)stat.second.type); break;
            }
        }
    }

    if (bAchievementsToo) {
        bool needs_disk_write = false;
        for (auto &kv : user_achievements.items()) {
            try {
                auto &name = kv.key();
                auto &item = kv.value();
                // assume "earned" is true in case the json obj exists, but the key is absent
                if (item.value("earned", true) == true) needs_disk_write = true;

                item["earned"] = false;
                item["earned_time"] = static_cast<uint32>(0);

                try {
                    auto defined_ach_it = defined_achievements_find(name);
                    if (defined_achievements.end() != defined_ach_it) {
                        auto defined_progress_it = defined_ach_it->find("progress");
                        if (defined_ach_it->end() != defined_progress_it) { // if the schema had "progress"
                            uint32 val = 0;
                            try {
                                auto defined_min_val = defined_progress_it->value("min_val", std::string("0"));
                                val = std::stoul(defined_min_val);
                            } catch(...){}
                            item["progress"] = val;
                        }
                    }
                }catch(...){}
                
                // this won't actually trigger a notification, just updates the data
                overlay->AddAchievementNotification(name, item, false);
            } catch(const std::exception& e) {
                const char *errorMessage = e.what();
                PRINT_DEBUG("ERROR: %s", errorMessage);
            }
        }
        if (needs_disk_write) save_achievements();

        if (!settings->disable_sharing_stats_with_gameserver) {
            for (const auto &item : user_achievements.items()) {
                auto &new_ach = (*pending_server_updates.mutable_user_achievements())[item.key()];
                new_ach.set_achieved(false);
            }
        }
    }

    if (!settings->disable_sharing_stats_with_gameserver && settings->immediate_gameserver_stats) send_updated_stats();

    return true;
}


// Requests global stats data, which is available for stats marked as "aggregated".
// This call is asynchronous, with the results returned in GlobalStatsReceived_t.
// nHistoryDays specifies how many days of day-by-day history to retrieve in addition
// to the overall totals. The limit is 60.
STEAM_CALL_RESULT( GlobalStatsReceived_t )
SteamAPICall_t Steam_User_Stats::RequestGlobalStats( int nHistoryDays )
{
    PRINT_DEBUG("%i", nHistoryDays);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    GlobalStatsReceived_t data{};
    data.m_nGameID = settings->get_local_game_id().ToUint64();
    data.m_eResult = k_EResultOK;
    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return ret;
}


// Gets the lifetime totals for an aggregated stat
bool Steam_User_Stats::GetGlobalStat( const char *pchStatName, int64 *pData )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return false;
}

bool Steam_User_Stats::GetGlobalStat( const char *pchStatName, double *pData )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return false;
}


// Gets history for an aggregated stat. pData will be filled with daily values, starting with today.
// So when called, pData[0] will be today, pData[1] will be yesterday, and pData[2] will be two days ago, 
// etc. cubData is the size in bytes of the pubData buffer. Returns the number of 
// elements actually set.
int32 Steam_User_Stats::GetGlobalStatHistory( const char *pchStatName, STEAM_ARRAY_COUNT(cubData) int64 *pData, uint32 cubData )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return 0;
}

int32 Steam_User_Stats::GetGlobalStatHistory( const char *pchStatName, STEAM_ARRAY_COUNT(cubData) double *pData, uint32 cubData )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return 0;
}



// --- steam callbacks

void Steam_User_Stats::send_updated_stats()
{
    if (pending_server_updates.user_stats().empty() && pending_server_updates.user_achievements().empty()) return;
    if (settings->disable_sharing_stats_with_gameserver) return;

    auto new_updates_msg = new GameServerStats_Messages::AllStats(pending_server_updates);
    pending_server_updates.clear_user_stats();
    pending_server_updates.clear_user_achievements();

    auto gameserverstats_msg = new GameServerStats_Messages();
    gameserverstats_msg->set_type(GameServerStats_Messages::UpdateUserStatsFromUser);
    gameserverstats_msg->set_allocated_update_user_stats(new_updates_msg);
    
    Common_Message msg{};
    // https://protobuf.dev/reference/cpp/cpp-generated/#string
    // set_allocated_xxx() takes ownership of the allocated object, no need to delete
    msg.set_allocated_gameserver_stats_messages(gameserverstats_msg);
    msg.set_source_id(settings->get_local_steam_id().ConvertToUint64());
    // here we send to all gameservers on the network because we don't know the server steamid
    network->sendToAllGameservers(&msg, true);

    PRINT_DEBUG("sent updated stats: %zu stats, %zu achievements",
        new_updates_msg->user_stats().size(), new_updates_msg->user_achievements().size()
    );
}



// --- networking callbacks
// only triggered when we have a message

// server wants all stats
void Steam_User_Stats::network_stats_initial(Common_Message *msg)
{
    if (!msg->gameserver_stats_messages().has_initial_user_stats()) {
        PRINT_DEBUG("error empty msg");
        return;
    }

    uint64 server_steamid = msg->source_id();

    auto all_stats_msg = new GameServerStats_Messages::AllStats();

    // get all stats
    auto &stats_map = *all_stats_msg->mutable_user_stats();
    const auto &current_stats = settings->getStats();
    for (const auto &stat : current_stats) {
        auto &this_stat = stats_map[stat.first];
        this_stat.set_stat_type(stat.second.type);
        switch (stat.second.type)
        {
        case GameServerStats_Messages::StatInfo::STAT_TYPE_INT: {
            int32 val = 0;
            GetStat(stat.first.c_str(), &val);
            this_stat.set_value_int(val);
        }
        break;

        case GameServerStats_Messages::StatInfo::STAT_TYPE_AVGRATE: // we set the float value also for avg
        case GameServerStats_Messages::StatInfo::STAT_TYPE_FLOAT: {
            float val = 0;
            GetStat(stat.first.c_str(), &val);
            this_stat.set_value_float(val);
        }
        break;
        
        default:
            PRINT_DEBUG("Request_AllUserStats unhandled stat type %i", (int)stat.second.type);
        break;
        }
    }

    // get all achievements
    auto &achievements_map = *all_stats_msg->mutable_user_achievements();
    for (const auto &ach : defined_achievements) {
        const std::string &name = static_cast<const std::string &>( ach.value("name", std::string()) );
        auto &this_ach = achievements_map[name];

        // achieved or not
        bool achieved = false;
        GetAchievement(name.c_str(), &achieved);
        this_ach.set_achieved(achieved);
    }

    auto initial_stats_msg = new GameServerStats_Messages::InitialAllStats();
    // send back same api call id
    initial_stats_msg->set_steam_api_call(msg->gameserver_stats_messages().initial_user_stats().steam_api_call());
    initial_stats_msg->set_allocated_all_data(all_stats_msg);

    auto gameserverstats_msg = new GameServerStats_Messages();
    gameserverstats_msg->set_type(GameServerStats_Messages::Response_AllUserStats);
    gameserverstats_msg->set_allocated_initial_user_stats(initial_stats_msg);
    
    Common_Message new_msg{};
    // https://protobuf.dev/reference/cpp/cpp-generated/#string
    // set_allocated_xxx() takes ownership of the allocated object, no need to delete
    new_msg.set_allocated_gameserver_stats_messages(gameserverstats_msg);
    new_msg.set_source_id(settings->get_local_steam_id().ConvertToUint64());
    new_msg.set_dest_id(server_steamid);
    network->sendTo(&new_msg, true);

    PRINT_DEBUG("server requested all stats, sent %zu stats, %zu achievements",
        initial_stats_msg->all_data().user_stats().size(), initial_stats_msg->all_data().user_achievements().size()
    );


}

// server has updated/new stats
void Steam_User_Stats::network_stats_updated(Common_Message *msg)
{
    if (!msg->gameserver_stats_messages().has_update_user_stats()) {
        PRINT_DEBUG("error empty msg");
        return;
    }

    auto &new_user_data = msg->gameserver_stats_messages().update_user_stats();

    // update our stats
    for (auto &new_stat : new_user_data.user_stats()) {
        switch (new_stat.second.stat_type())
        {
        case GameServerStats_Messages::StatInfo::STAT_TYPE_INT: {
            set_stat_internal(new_stat.first.c_str(), new_stat.second.value_int());
        }
        break;
        
        case GameServerStats_Messages::StatInfo::STAT_TYPE_AVGRATE:
        case GameServerStats_Messages::StatInfo::STAT_TYPE_FLOAT: {
            set_stat_internal(new_stat.first.c_str(), new_stat.second.value_float());
            // non-INT values could have avg values
            if (new_stat.second.has_value_avg()) {
                auto &avg_val = new_stat.second.value_avg();
                update_avg_rate_stat_internal(new_stat.first.c_str(), avg_val.count_this_session(), avg_val.session_length());
            }
        }
        break;
        
        default:
            PRINT_DEBUG("UpdateUserStats unhandled stat type %i", (int)new_stat.second.stat_type());
        break;
        }
    }

    // update achievements
    for (auto &new_ach : new_user_data.user_achievements()) {
        if (new_ach.second.achieved()) {
            set_achievement_internal(new_ach.first.c_str());
        } else {
            clear_achievement_internal(new_ach.first.c_str());
        }
    }
    
    PRINT_DEBUG("server sent updated user stats, %zu stats, %zu achievements",
        new_user_data.user_stats().size(), new_user_data.user_achievements().size()
    );
}

void Steam_User_Stats::network_callback_stats(Common_Message *msg)
{
    // network->sendToAll() sends to current user also
    if (msg->source_id() == settings->get_local_steam_id().ConvertToUint64()) return; 

    uint64 server_steamid = msg->source_id();

    switch (msg->gameserver_stats_messages().type())
    {
    // server wants all stats
    case GameServerStats_Messages::Request_AllUserStats:
        network_stats_initial(msg);
    break;

    // server has updated/new stats
    case GameServerStats_Messages::UpdateUserStatsFromServer:
        network_stats_updated(msg);
    break;
    
    // a user has updated/new stats
    case GameServerStats_Messages::UpdateUserStatsFromUser:
        // nothing
    break;
    
    default:
        PRINT_DEBUG("unhandled type %i", (int)msg->gameserver_stats_messages().type());
    break;
    }
}
