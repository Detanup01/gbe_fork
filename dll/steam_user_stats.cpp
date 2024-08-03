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


// --- Steam_Leaderboard ---

Steam_Leaderboard_Entry* Steam_Leaderboard::find_recent_entry(const CSteamID &steamid) const
{
    auto my_it = std::find_if(entries.begin(), entries.end(), [&steamid](const Steam_Leaderboard_Entry &item) {
        return item.steam_id == steamid;
    });
    if (entries.end() == my_it) return nullptr;
    return const_cast<Steam_Leaderboard_Entry*>(&*my_it);
}

void Steam_Leaderboard::remove_entries(const CSteamID &steamid)
{
    auto rm_it = std::remove_if(entries.begin(), entries.end(), [&](const Steam_Leaderboard_Entry &item){
        return item.steam_id == steamid;
    });
    if (entries.end() != rm_it) entries.erase(rm_it, entries.end());
}

void Steam_Leaderboard::remove_duplicate_entries()
{
    if (entries.size() <= 1) return;

    auto rm = std::remove_if(entries.begin(), entries.end(), [&](const Steam_Leaderboard_Entry& item) {
        auto recent = find_recent_entry(item.steam_id);
        return &item != recent;
    });
    if (entries.end() != rm) entries.erase(rm, entries.end());
}

void Steam_Leaderboard::sort_entries()
{
    if (sort_method == k_ELeaderboardSortMethodNone) return;
    if (entries.size() <= 1) return;

    std::sort(entries.begin(), entries.end(), [this](const Steam_Leaderboard_Entry &item1, const Steam_Leaderboard_Entry &item2) {
        if (sort_method == k_ELeaderboardSortMethodAscending) {
            return item1.score < item2.score;
        } else { // k_ELeaderboardSortMethodDescending
            return item1.score > item2.score;
        }
    });

}

// --- Steam_Leaderboard ---



// --- achievement_trigger ---
bool achievement_trigger::should_unlock_ach(float stat) const
{
    try {
        if (std::stof(max_value) <= stat) return true;
    } catch (...) {}

    return false;
}

bool achievement_trigger::should_unlock_ach(int32 stat) const
{
    try {
        if (std::stoi(max_value) <= stat) return true;
    } catch (...) {}

    return false;
}

bool achievement_trigger::should_indicate_progress(float stat) const
{
    // show progress if number < max
    try {
        if (std::stof(max_value) > stat) return true;
    } catch (...) {}

    return false;
}

bool achievement_trigger::should_indicate_progress(int32 stat) const
{
    // show progress if number < max
    try {
        if (std::stoi(max_value) > stat) return true;
    } catch (...) {}

    return false;
}
// --- achievement_trigger ---



void Steam_User_Stats::load_achievements_db()
{
    std::string file_path = Local_Storage::get_game_settings_path() + achievements_user_file;
    local_storage->load_json(file_path, defined_achievements);
}

void Steam_User_Stats::load_achievements()
{
    local_storage->load_json_file("", achievements_user_file, user_achievements);
}

void Steam_User_Stats::save_achievements()
{
    local_storage->write_json_file("", achievements_user_file, user_achievements);
}


nlohmann::detail::iter_impl<nlohmann::json> Steam_User_Stats::defined_achievements_find(const std::string &key)
{
    return std::find_if(
        defined_achievements.begin(), defined_achievements.end(),
        [&key](const nlohmann::json& item) {
            const std::string &name = static_cast<const std::string &>( item.value("name", std::string()) );
            return common_helpers::str_cmp_insensitive(key, name);
        }
    );
}

std::string Steam_User_Stats::get_value_for_language(const nlohmann::json &json, std::string_view key, std::string_view language)
{
    auto x = json.find(key); // find "displayName", or "description", etc ...
    if (json.end() == x) return "";

    if (x.value().is_string()) { // ex: "displayName": "some description"
        return x.value().get<std::string>();
    } else if (x.value().is_object()) {
        const auto &obj_kv_pairs = x.value().items();

        // try to find target language
        auto obj_itr = std::find_if(obj_kv_pairs.begin(), obj_kv_pairs.end(), [&]( decltype(*obj_kv_pairs.begin()) item ) {
            return common_helpers::str_cmp_insensitive(item.key(), language);
        });
        if (obj_itr != obj_kv_pairs.end()) {
            return obj_itr.value().get<std::string>();
        }

        // try to find english language
        obj_itr = std::find_if(obj_kv_pairs.begin(), obj_kv_pairs.end(), [&]( decltype(*obj_kv_pairs.begin()) item ) {
            return common_helpers::str_cmp_insensitive(item.key(), "english");
        });
        if (obj_itr != obj_kv_pairs.end()) {
            return obj_itr.value().get<std::string>();
        }

        // try to find the first available language (not "token"),
        // if not languages exist, try to find "token"
        for (bool search_for_token : { false, true }) {
            obj_itr = std::find_if(obj_kv_pairs.begin(), obj_kv_pairs.end(), [&]( decltype(*obj_kv_pairs.begin()) item ) {
                return common_helpers::str_cmp_insensitive(item.key(), "token") == search_for_token;
            });
            if (obj_itr != obj_kv_pairs.end()) {
                return obj_itr.value().get<std::string>();
            }
        }
    }

    return "";
}


/*
layout of each item in the leaderboard file
| steamid - lower 32-bits | steamid - higher 32-bits | score (4 bytes) | score details count (4 bytes) | score details array (4 bytes each) ...
  [0]                     | [1]                      | [2]             | [3]                            | [4]
  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ main header ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
*/

std::vector<Steam_Leaderboard_Entry> Steam_User_Stats::load_leaderboard_entries(const std::string &name)
{
    constexpr const static unsigned int MAIN_HEADER_ELEMENTS_COUNT = 4;
    constexpr const static unsigned int ELEMENT_SIZE = (unsigned int)sizeof(uint32_t);

    std::vector<Steam_Leaderboard_Entry> out{};

    std::string leaderboard_name(common_helpers::ascii_to_lowercase(name));
    unsigned read_bytes = local_storage->file_size(Local_Storage::leaderboard_storage_folder, leaderboard_name);
    if ((read_bytes == 0) ||
        (read_bytes < (ELEMENT_SIZE * MAIN_HEADER_ELEMENTS_COUNT)) ||
        (read_bytes % ELEMENT_SIZE) != 0) {
        return out;
    }

    std::vector<uint32_t> output(read_bytes / ELEMENT_SIZE);
    if (local_storage->get_data(Local_Storage::leaderboard_storage_folder, leaderboard_name, (char* )output.data(), read_bytes) != read_bytes) return out;

    unsigned int i = 0;
    while (true) {
        if ((i + MAIN_HEADER_ELEMENTS_COUNT) > output.size()) break; // invalid main header, or end of buffer

        Steam_Leaderboard_Entry new_entry{};
        new_entry.steam_id = CSteamID((uint64)output[i] + (((uint64)output[i + 1]) << 32));
        new_entry.score = (int32)output[i + 2];
        uint32_t details_count = output[i + 3];
        i += MAIN_HEADER_ELEMENTS_COUNT; // skip main header

        if ((i + details_count) > output.size()) break; // invalid score details count

        for (uint32_t j = 0; j < details_count; ++j) {
            new_entry.score_details.push_back(output[i]);
            ++i; // move past this score detail
        }

        PRINT_DEBUG("'%s': user %llu, score %i, details count = %zu",
            name.c_str(), new_entry.steam_id.ConvertToUint64(), new_entry.score, new_entry.score_details.size()
        );
        out.push_back(new_entry);
    }

    PRINT_DEBUG("'%s' total entries = %zu", name.c_str(), out.size());
    return out;
}

void Steam_User_Stats::save_my_leaderboard_entry(const Steam_Leaderboard &leaderboard)
{
     auto my_entry = leaderboard.find_recent_entry(settings->get_local_steam_id());
     if (!my_entry) return; // we don't have a score entry

    PRINT_DEBUG("saving entries for leaderboard '%s'", leaderboard.name.c_str());

    std::vector<uint32_t> output{};

    uint64_t steam_id = my_entry->steam_id.ConvertToUint64();
    output.push_back((uint32_t)(steam_id & 0xFFFFFFFF)); // lower 4 bytes
    output.push_back((uint32_t)(steam_id >> 32)); // higher 4 bytes

    output.push_back(my_entry->score);
    output.push_back((uint32_t)my_entry->score_details.size());
    for (const auto &detail : my_entry->score_details) {
        output.push_back(detail);
    }

    std::string leaderboard_name(common_helpers::ascii_to_lowercase(leaderboard.name));
    unsigned int buffer_size = static_cast<unsigned int>(output.size() * sizeof(output[0])); // in bytes
    local_storage->store_data(Local_Storage::leaderboard_storage_folder, leaderboard_name, (char* )&output[0], buffer_size);
}

Steam_Leaderboard_Entry* Steam_User_Stats::update_leaderboard_entry(Steam_Leaderboard &leaderboard, const Steam_Leaderboard_Entry &entry, bool overwrite)
{
    bool added = false;
    auto user_entry = leaderboard.find_recent_entry(entry.steam_id);
    if (!user_entry) { // user doesn't have an entry yet, create one
        added = true;
        leaderboard.entries.push_back(entry);
        user_entry = &leaderboard.entries.back();
    } else if (overwrite) {
        added = true;
        *user_entry = entry;
    }

    if (added) { // if we added a new entry then we have to sort and find the target entry again
        leaderboard.sort_entries();
        user_entry = leaderboard.find_recent_entry(entry.steam_id);
        PRINT_DEBUG("added/updated entry for user %llu", entry.steam_id.ConvertToUint64());
    }
    
    return user_entry;
}


unsigned int Steam_User_Stats::find_cached_leaderboard(const std::string &name)
{
    unsigned index = 1;
    for (const auto &leaderboard : cached_leaderboards) {
        if (common_helpers::str_cmp_insensitive(leaderboard.name, name)) return index;

        ++index;
    }

    return 0;
}

unsigned int Steam_User_Stats::cache_leaderboard_ifneeded(const std::string &name, ELeaderboardSortMethod eLeaderboardSortMethod, ELeaderboardDisplayType eLeaderboardDisplayType)
{
    unsigned int board_handle = find_cached_leaderboard(name);
    if (board_handle) return board_handle;
    // PRINT_DEBUG("cache miss '%s'", name.c_str());

    // create a new entry in-memory and try reading the entries from disk
    struct Steam_Leaderboard new_board{};
    new_board.name = common_helpers::ascii_to_lowercase(name);
    new_board.sort_method = eLeaderboardSortMethod;
    new_board.display_type = eLeaderboardDisplayType;
    new_board.entries = load_leaderboard_entries(name);

    new_board.sort_entries();
    new_board.remove_duplicate_entries();

    // save it in memory for later
    cached_leaderboards.push_back(new_board);
    board_handle = static_cast<unsigned int>(cached_leaderboards.size());

    PRINT_DEBUG("cached a new leaderboard '%s' %i %i",
        new_board.name.c_str(), (int)eLeaderboardSortMethod, (int)eLeaderboardDisplayType
    );
    return board_handle;
}

void Steam_User_Stats::send_my_leaderboard_score(const Steam_Leaderboard &board, const CSteamID *steamid, bool want_scores_back)
{
    if (!settings->share_leaderboards_over_network) return;

    const auto my_entry = board.find_recent_entry(settings->get_local_steam_id());
    Leaderboards_Messages::UserScoreEntry *score_entry_msg = nullptr;
    
    if (my_entry) {
        score_entry_msg = new Leaderboards_Messages::UserScoreEntry();
        score_entry_msg->set_score(my_entry->score);
        score_entry_msg->mutable_score_details()->Assign(my_entry->score_details.begin(), my_entry->score_details.end());
    }

    auto board_info_msg = new Leaderboards_Messages::LeaderboardInfo();
    board_info_msg->set_allocated_board_name(new std::string(board.name));
    board_info_msg->set_sort_method(board.sort_method);
    board_info_msg->set_display_type(board.display_type);

    auto board_msg = new Leaderboards_Messages();
    if (want_scores_back) board_msg->set_type(Leaderboards_Messages::UpdateUserScoreMutual);
    else board_msg->set_type(Leaderboards_Messages::UpdateUserScore);
    board_msg->set_appid(settings->get_local_game_id().AppID());
    board_msg->set_allocated_leaderboard_info(board_info_msg);
    // if we have an entry
    if (score_entry_msg) board_msg->set_allocated_user_score_entry(score_entry_msg);

    Common_Message common_msg{};
    common_msg.set_source_id(settings->get_local_steam_id().ConvertToUint64());
    if (steamid) common_msg.set_dest_id(steamid->ConvertToUint64());
    common_msg.set_allocated_leaderboards_messages(board_msg);

    if (steamid) network->sendTo(&common_msg, false);
    else network->sendToAll(&common_msg, false);
}

void Steam_User_Stats::request_user_leaderboard_entry(const Steam_Leaderboard &board, const CSteamID &steamid)
{
    if (!settings->share_leaderboards_over_network) return;

    auto board_info_msg = new Leaderboards_Messages::LeaderboardInfo();
    board_info_msg->set_allocated_board_name(new std::string(board.name));
    board_info_msg->set_sort_method(board.sort_method);
    board_info_msg->set_display_type(board.display_type);

    auto board_msg = new Leaderboards_Messages();
    board_msg->set_type(Leaderboards_Messages::RequestUserScore);
    board_msg->set_appid(settings->get_local_game_id().AppID());
    board_msg->set_allocated_leaderboard_info(board_info_msg);

    Common_Message common_msg{};
    common_msg.set_source_id(settings->get_local_steam_id().ConvertToUint64());
    common_msg.set_dest_id(steamid.ConvertToUint64());
    common_msg.set_allocated_leaderboards_messages(board_msg);

    network->sendTo(&common_msg, false);
}


// change stats/achievements without sending back to server
bool Steam_User_Stats::clear_stats_internal()
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    bool notify_server = false;
    
    for (const auto &stat : settings->getStats()) {
        std::string stat_name(common_helpers::ascii_to_lowercase(stat.first));

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
    std::string stat_name(common_helpers::ascii_to_lowercase(pchName));

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
    std::string stat_name(common_helpers::ascii_to_lowercase(pchName));

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
    std::string stat_name(common_helpers::ascii_to_lowercase(pchName));

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

Steam_User_Stats::InternalSetResult<bool> Steam_User_Stats::set_achievement_internal( const char *pchName )
{
    PRINT_DEBUG("'%s'", pchName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    Steam_User_Stats::InternalSetResult<bool> result{};

    if (!pchName) return result;
    
    std::string org_name(pchName);

    if (settings->achievement_bypass) {
        auto &trig = store_stats_trigger[common_helpers::to_lower(org_name)];
        trig.m_bGroupAchievement = false;
        trig.m_nCurProgress = 100;
        trig.m_nGameID = settings->get_local_game_id().ToUint64();
        trig.m_nMaxProgress = 100;
        memset(trig.m_rgchAchievementName, 0, sizeof(trig.m_rgchAchievementName));
        org_name.copy(trig.m_rgchAchievementName, sizeof(trig.m_rgchAchievementName) - 1);

        result.success = true;
        return result;
    }

    nlohmann::detail::iter_impl<nlohmann::json> it = defined_achievements.end();
    try {
        it = defined_achievements_find(org_name);
    } catch(...) { }
    if (defined_achievements.end() == it) return result;

    result.current_val = true;
    result.internal_name = org_name;
    result.success = true;

    try {
        std::string internal_name = it->value("name", std::string());

        result.internal_name = internal_name;

        auto ach = user_achievements.find(internal_name);
        if (user_achievements.end() == ach || ach->value("earned", false) == false) {
            user_achievements[internal_name]["earned"] = true;
            user_achievements[internal_name]["earned_time"] =
                std::chrono::duration_cast<std::chrono::duration<uint32>>(std::chrono::system_clock::now().time_since_epoch()).count();

            save_achievements();

            result.notify_server = !settings->disable_sharing_stats_with_gameserver;

            overlay->AddAchievementNotification(internal_name, user_achievements[internal_name], false);

        }
    } catch (...) {}

    auto &trig = store_stats_trigger[common_helpers::to_lower(org_name)];
    trig.m_bGroupAchievement = false;
    trig.m_nCurProgress = 100;
    trig.m_nGameID = settings->get_local_game_id().ToUint64();
    trig.m_nMaxProgress = 100;
    memset(trig.m_rgchAchievementName, 0, sizeof(trig.m_rgchAchievementName));
    org_name.copy(trig.m_rgchAchievementName, sizeof(trig.m_rgchAchievementName) - 1);

    return result;
}

Steam_User_Stats::InternalSetResult<bool> Steam_User_Stats::clear_achievement_internal( const char *pchName )
{
    PRINT_DEBUG("'%s'", pchName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    Steam_User_Stats::InternalSetResult<bool> result{};

    if (!pchName) return result;

    std::string org_name(pchName);

    nlohmann::detail::iter_impl<nlohmann::json> it = defined_achievements.end();
    try {
        it = defined_achievements_find(org_name);
    } catch(...) { }
    if (defined_achievements.end() == it) return result;

    result.current_val = false;
    result.internal_name = org_name;
    result.success = true;

    try {
        std::string internal_name = it->value("name", std::string());

        result.internal_name = internal_name;

        auto ach = user_achievements.find(internal_name);
        // assume "earned" is true in case the json obj exists, but the key is absent
        // assume "earned_time" is UINT32_MAX in case the json obj exists, but the key is absent
        if (user_achievements.end() == ach ||
            ach->value("earned", true) == true ||
            ach->value("earned_time", static_cast<uint32>(UINT32_MAX)) == UINT32_MAX) {
            
            user_achievements[internal_name]["earned"] = false;
            user_achievements[internal_name]["earned_time"] = static_cast<uint32>(0);
            save_achievements();

            result.notify_server = !settings->disable_sharing_stats_with_gameserver;

            overlay->AddAchievementNotification(internal_name, user_achievements[internal_name], false);

        }
    } catch (...) {}

    store_stats_trigger.erase(common_helpers::to_lower(org_name));
    
    return result;
}


void Steam_User_Stats::steam_user_stats_network_low_level(void *object, Common_Message *msg)
{
    // PRINT_DEBUG_ENTRY();

    auto inst = (Steam_User_Stats *)object;
    inst->network_callback_low_level(msg);
}

void Steam_User_Stats::steam_user_stats_network_stats(void *object, Common_Message *msg)
{
    // PRINT_DEBUG_ENTRY();

    auto inst = (Steam_User_Stats *)object;
    inst->network_callback_stats(msg);
}

void Steam_User_Stats::steam_user_stats_network_leaderboards(void *object, Common_Message *msg)
{
    // PRINT_DEBUG_ENTRY();

    auto inst = (Steam_User_Stats *)object;
    inst->network_callback_leaderboards(msg);
}

void Steam_User_Stats::steam_user_stats_run_every_runcb(void *object)
{
    // PRINT_DEBUG_ENTRY();

    auto inst = (Steam_User_Stats *)object;
    inst->steam_run_callback();
}


Steam_User_Stats::Steam_User_Stats(Settings *settings, class Networking *network, Local_Storage *local_storage, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb, Steam_Overlay* overlay):
    settings(settings),
    network(network),
    local_storage(local_storage),
    callback_results(callback_results),
    callbacks(callbacks),
    defined_achievements(nlohmann::json::object()),
    user_achievements(nlohmann::json::object()),
    run_every_runcb(run_every_runcb),
    overlay(overlay)
{
    load_achievements_db(); // achievements db
    load_achievements(); // achievements per user

    auto x = defined_achievements.begin();
    while (x != defined_achievements.end()) {
        if (!x->contains("name")) {
            x = defined_achievements.erase(x);
        } else {
            ++x;
        }
    }

    for (auto & it : defined_achievements) {
        try {
            std::string name = static_cast<std::string const&>(it["name"]);
            sorted_achievement_names.push_back(name);

            achievement_trigger trig{};
            try {
                trig.name = name;
                trig.value_operation = static_cast<std::string const&>(it["progress"]["value"]["operation"]);
                std::string stat_name = common_helpers::ascii_to_lowercase(static_cast<std::string const&>(it["progress"]["value"]["operand1"]));
                trig.min_value = static_cast<std::string const&>(it["progress"]["min_val"]);
                trig.max_value = static_cast<std::string const&>(it["progress"]["max_val"]);
                achievement_stat_trigger[stat_name].push_back(trig);
            } catch(...) {}
            
            // default initial values, will only be added if they don't exist already
            auto &user_ach = user_achievements[name]; // this will create a new json entry if the key didn't exist already
            user_ach.emplace("earned", false);
            user_ach.emplace("earned_time", static_cast<uint32>(0));
            // they will throw an exception for achievements with no progress
            try {
                uint32 progress_min = std::stoul(trig.min_value);
                uint32 progress_max = std::stoul(trig.max_value);
                // if the above lines didn't throw exception then add the values
                user_ach.emplace("progress", progress_min);
                user_ach.emplace("max_progress", progress_max);
            } catch(...) {}
        } catch(...) {}

        try {
            it["hidden"] = std::to_string(it["hidden"].get<int>());
        } catch(...) {}

        it["displayName"] = get_value_for_language(it, "displayName", settings->get_language());
        it["description"] = get_value_for_language(it, "description", settings->get_language());
    }

    //TODO: not sure if the sort is actually case insensitive, ach names seem to be treated by steam as case insensitive so I assume they are.
    //need to find a game with achievements of different case names to confirm
    std::sort(sorted_achievement_names.begin(), sorted_achievement_names.end(), [](const std::string lhs, const std::string rhs){
        const auto result = std::mismatch(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend(), [](const unsigned char lhs, const unsigned char rhs){return std::tolower(lhs) == std::tolower(rhs);});
        return result.second != rhs.cend() && (result.first == lhs.cend() || std::tolower(*result.first) < std::tolower(*result.second));}
    );
    
    if (!settings->disable_sharing_stats_with_gameserver) {
        this->network->setCallback(CALLBACK_ID_GAMESERVER_STATS, settings->get_local_steam_id(), &Steam_User_Stats::steam_user_stats_network_stats, this);
    }
    if (settings->share_leaderboards_over_network) {
        this->network->setCallback(CALLBACK_ID_LEADERBOARDS_STATS, settings->get_local_steam_id(), &Steam_User_Stats::steam_user_stats_network_leaderboards, this);
    }
    this->network->setCallback(CALLBACK_ID_USER_STATUS, settings->get_local_steam_id(), &Steam_User_Stats::steam_user_stats_network_low_level, this);
    this->run_every_runcb->add(&Steam_User_Stats::steam_user_stats_run_every_runcb, this);
}

Steam_User_Stats::~Steam_User_Stats()
{
    if (!settings->disable_sharing_stats_with_gameserver) {
        this->network->rmCallback(CALLBACK_ID_GAMESERVER_STATS, settings->get_local_steam_id(), &Steam_User_Stats::steam_user_stats_network_stats, this);
    }
    if (settings->share_leaderboards_over_network) {
        this->network->rmCallback(CALLBACK_ID_LEADERBOARDS_STATS, settings->get_local_steam_id(), &Steam_User_Stats::steam_user_stats_network_leaderboards, this);
    }
    this->network->rmCallback(CALLBACK_ID_USER_STATUS, settings->get_local_steam_id(), &Steam_User_Stats::steam_user_stats_network_low_level, this);
    this->run_every_runcb->remove(&Steam_User_Stats::steam_user_stats_run_every_runcb, this);
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
    std::string stat_name = common_helpers::ascii_to_lowercase(pchName);

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
    std::string stat_name = common_helpers::ascii_to_lowercase(pchName);

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


// Achievement flag accessors
bool Steam_User_Stats::GetAchievement( const char *pchName, bool *pbAchieved )
{
    PRINT_DEBUG("'%s'", pchName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pchName) return false;

    nlohmann::detail::iter_impl<nlohmann::json> it = defined_achievements.end();
    try {
        it = defined_achievements_find(pchName);
    } catch(...) { }
    if (defined_achievements.end() == it) return false;

    // according to docs, the function returns true if the achievement was found,
    // regardless achieved or not 
    if (!pbAchieved) return true;

    *pbAchieved = false;
    try {
        std::string pch_name = it->value("name", std::string());
        auto ach = user_achievements.find(pch_name);
        if (user_achievements.end() != ach) {
            *pbAchieved = ach->value("earned", false);
        }
    } catch (...) { }

    return true;
}

bool Steam_User_Stats::SetAchievement( const char *pchName )
{
    PRINT_DEBUG("'%s'", pchName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    auto ret = set_achievement_internal(pchName);
    if (ret.success && ret.notify_server) {
        auto &new_ach = (*pending_server_updates.mutable_user_achievements())[ret.internal_name];
        new_ach.set_achieved(ret.current_val);

        if (settings->immediate_gameserver_stats) send_updated_stats();
    }
    
    return ret.success;
}

bool Steam_User_Stats::ClearAchievement( const char *pchName )
{
    PRINT_DEBUG("'%s'", pchName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    auto ret = clear_achievement_internal(pchName);
    if (ret.success && ret.notify_server) {
        auto &new_ach = (*pending_server_updates.mutable_user_achievements())[ret.internal_name];
        new_ach.set_achieved(ret.current_val);

        if (settings->immediate_gameserver_stats) send_updated_stats();
    }
    
    return ret.success;
}


// Get the achievement status, and the time it was unlocked if unlocked.
// If the return value is true, but the unlock time is zero, that means it was unlocked before Steam 
// began tracking achievement unlock times (December 2009). Time is seconds since January 1, 1970.
bool Steam_User_Stats::GetAchievementAndUnlockTime( const char *pchName, bool *pbAchieved, uint32 *punUnlockTime )
{
    PRINT_DEBUG("'%s'", pchName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pchName) return false;

    nlohmann::detail::iter_impl<nlohmann::json> it = defined_achievements.end();
    try {
        it = defined_achievements_find(pchName);
    } catch(...) { }
    if (defined_achievements.end() == it) return false;

    if (pbAchieved) *pbAchieved = false;
    if (punUnlockTime) *punUnlockTime = 0;
    
    try {
        std::string pch_name = it->value("name", std::string());
        auto ach = user_achievements.find(pch_name);
        if (user_achievements.end() != ach) {
            if (pbAchieved) *pbAchieved = ach->value("earned", false);
            if (punUnlockTime) *punUnlockTime = ach->value("earned_time", static_cast<uint32>(0));
        }
    } catch (...) {}

    return true;
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


// Achievement / GroupAchievement metadata

// Gets the icon of the achievement, which is a handle to be used in ISteamUtils::GetImageRGBA(), or 0 if none set. 
// A return value of 0 may indicate we are still fetching data, and you can wait for the UserAchievementIconFetched_t callback
// which will notify you when the bits are ready. If the callback still returns zero, then there is no image set for the
// specified achievement.
int Steam_User_Stats::GetAchievementIcon( const char *pchName )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!pchName) return 0;

    // callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return 0;
}

std::string Steam_User_Stats::get_achievement_icon_name( const char *pchName, bool pbAchieved )
{
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!pchName) return "";

    nlohmann::detail::iter_impl<nlohmann::json> it = defined_achievements.end();
    try {
        it = defined_achievements_find(pchName);
    } catch(...) { }
    if (defined_achievements.end() == it) return "";

    try {
        if (pbAchieved) return it.value()["icon"].get<std::string>();
        
        std::string locked_icon = it.value().value("icon_gray", std::string());
        if (locked_icon.size()) return locked_icon;
        else return it.value().value("icongray", std::string()); // old format
    } catch (...) {}

    return "";
}


// Get general attributes for an achievement. Accepts the following keys:
// - "name" and "desc" for retrieving the localized achievement name and description (returned in UTF8)
// - "hidden" for retrieving if an achievement is hidden (returns "0" when not hidden, "1" when hidden)
const char * Steam_User_Stats::GetAchievementDisplayAttribute( const char *pchName, const char *pchKey )
{
    PRINT_DEBUG("[%s] [%s]", pchName, pchKey);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pchName || !pchKey || !pchKey[0]) return "";

    nlohmann::detail::iter_impl<nlohmann::json> it = defined_achievements.end();
    try {
        it = defined_achievements_find(pchName);
    } catch(...) { }
    if (defined_achievements.end() == it) return "";

    if (strncmp(pchKey, "name", sizeof("name")) == 0) {
        try {
            return it.value()["displayName"].get_ptr<std::string*>()->c_str();
        } catch (...) {}
    } else if (strncmp(pchKey, "desc", sizeof("desc")) == 0) {
        try {
            return it.value()["description"].get_ptr<std::string*>()->c_str();
        } catch (...) {}
    } else if (strncmp(pchKey, "hidden", sizeof("hidden")) == 0) {
        try {
            return it.value()["hidden"].get_ptr<std::string*>()->c_str();
        } catch (...) {}
    }

    return "";
}


// Achievement progress - triggers an AchievementProgress callback, that is all.
// Calling this w/ N out of N progress will NOT set the achievement, the game must still do that.
bool Steam_User_Stats::IndicateAchievementProgress( const char *pchName, uint32 nCurProgress, uint32 nMaxProgress )
{
    PRINT_DEBUG("'%s' %u %u", pchName, nCurProgress, nMaxProgress);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pchName) return false;
    if (nCurProgress >= nMaxProgress) return false;

    std::string ach_name(pchName);

    // find in achievements.json
    nlohmann::detail::iter_impl<nlohmann::json> it = defined_achievements.end();
    try {
        it = defined_achievements_find(ach_name);
    } catch(...) { }
    if (defined_achievements.end() == it) return false;

    // get actual name from achievements.json
    std::string actual_ach_name{};
    try {
        actual_ach_name = it->value("name", std::string());
    } catch (...) { }
    if (actual_ach_name.empty()) { // fallback
        actual_ach_name = ach_name;
    }

    // check if already achieved
    bool achieved = false;
    try {
        auto ach = user_achievements.find(actual_ach_name);
        if (ach != user_achievements.end()) {
            achieved = ach->value("earned", false);
        }
    } catch (...) { }
    if (achieved) return false;

    // save new progress
    try {
        auto old_progress = user_achievements.value(actual_ach_name, nlohmann::json{}).value("progress", ~nCurProgress);
        if (old_progress != nCurProgress) {
            user_achievements[actual_ach_name]["progress"] = nCurProgress;
            user_achievements[actual_ach_name]["max_progress"] = nMaxProgress;
            
            save_achievements();
            
            overlay->AddAchievementNotification(actual_ach_name, user_achievements[actual_ach_name], true);
        }
    } catch (...) {}

    {
        UserStatsStored_t data{};
        data.m_eResult = EResult::k_EResultOK;
        data.m_nGameID = settings->get_local_game_id().ToUint64();
        callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    }

    {
        UserAchievementStored_t data{};
        data.m_nGameID = settings->get_local_game_id().ToUint64();
        data.m_bGroupAchievement = false;
        data.m_nCurProgress = nCurProgress;
        data.m_nMaxProgress = nMaxProgress;
        ach_name.copy(data.m_rgchAchievementName, sizeof(data.m_rgchAchievementName) - 1);

        callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    }

    return true;
}


// Used for iterating achievements. In general games should not need these functions because they should have a
// list of existing achievements compiled into them
uint32 Steam_User_Stats::GetNumAchievements()
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return (uint32)defined_achievements.size();
}

// Get achievement name iAchievement in [0,GetNumAchievements)
const char * Steam_User_Stats::GetAchievementName( uint32 iAchievement )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (iAchievement >= sorted_achievement_names.size()) {
        return "";
    }

    return sorted_achievement_names[iAchievement].c_str();
}


// Friends stats & achievements

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

bool Steam_User_Stats::GetUserAchievement( CSteamID steamIDUser, const char *pchName, bool *pbAchieved )
{
    PRINT_DEBUG("%s", pchName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    if (!pchName) return false;

    if (steamIDUser == settings->get_local_steam_id()) {
        return GetAchievement(pchName, pbAchieved);
    }

    return false;
}

// See notes for GetAchievementAndUnlockTime above
bool Steam_User_Stats::GetUserAchievementAndUnlockTime( CSteamID steamIDUser, const char *pchName, bool *pbAchieved, uint32 *punUnlockTime )
{
    PRINT_DEBUG("%s", pchName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pchName) return false;

    if (steamIDUser == settings->get_local_steam_id()) {
        return GetAchievementAndUnlockTime(pchName, pbAchieved, punUnlockTime);
    }
    return false;
}


// Reset stats 
bool Steam_User_Stats::ResetAllStats( bool bAchievementsToo )
{
    PRINT_DEBUG("bAchievementsToo = %i", (int)bAchievementsToo);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    clear_stats_internal(); // this will save stats to disk if necessary
    if (!settings->disable_sharing_stats_with_gameserver) {
        for (const auto &stat : settings->getStats()) {
            std::string stat_name(common_helpers::ascii_to_lowercase(stat.first));

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
                PRINT_DEBUG("ERROR: %s", e.what());
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


// Leaderboard functions

// asks the Steam back-end for a leaderboard by name, and will create it if it's not yet
// This call is asynchronous, with the result returned in LeaderboardFindResult_t
STEAM_CALL_RESULT(LeaderboardFindResult_t)
SteamAPICall_t Steam_User_Stats::FindOrCreateLeaderboard( const char *pchLeaderboardName, ELeaderboardSortMethod eLeaderboardSortMethod, ELeaderboardDisplayType eLeaderboardDisplayType )
{
    PRINT_DEBUG("'%s'", pchLeaderboardName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!pchLeaderboardName) {
        LeaderboardFindResult_t data{};
        data.m_hSteamLeaderboard = 0;
        data.m_bLeaderboardFound = 0;
        auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
        callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
        return ret;
    }

    unsigned int board_handle = cache_leaderboard_ifneeded(pchLeaderboardName, eLeaderboardSortMethod, eLeaderboardDisplayType);
    send_my_leaderboard_score(cached_leaderboards[board_handle - 1], nullptr, true);

    LeaderboardFindResult_t data{};
    data.m_hSteamLeaderboard = board_handle;
    data.m_bLeaderboardFound = 1;
    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data), 0.1); // TODO is the timing ok?
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data), 0.1);
    return ret;
}


// as above, but won't create the leaderboard if it's not found
// This call is asynchronous, with the result returned in LeaderboardFindResult_t
STEAM_CALL_RESULT( LeaderboardFindResult_t )
SteamAPICall_t Steam_User_Stats::FindLeaderboard( const char *pchLeaderboardName )
{
    PRINT_DEBUG("'%s'", pchLeaderboardName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!pchLeaderboardName) {
        LeaderboardFindResult_t data{};
        data.m_hSteamLeaderboard = 0;
        data.m_bLeaderboardFound = 0;
        auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
        callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
        return ret;
    }

    std::string name_lower(common_helpers::ascii_to_lowercase(pchLeaderboardName));
    const auto &settings_Leaderboards = settings->getLeaderboards();
    auto it = settings_Leaderboards.begin();
    for (;  settings_Leaderboards.end() != it; ++it) {
        if (common_helpers::str_cmp_insensitive(it->first, name_lower)) break;
    }
    if (settings_Leaderboards.end() != it) {
        auto &config = it->second;
        return FindOrCreateLeaderboard(pchLeaderboardName, config.sort_method, config.display_type);
    } else if (!settings->disable_leaderboards_create_unknown) {
        return FindOrCreateLeaderboard(pchLeaderboardName, k_ELeaderboardSortMethodDescending, k_ELeaderboardDisplayTypeNumeric);
    } else {
        LeaderboardFindResult_t data{};
        data.m_hSteamLeaderboard = find_cached_leaderboard(name_lower);
        data.m_bLeaderboardFound = !!data.m_hSteamLeaderboard;
        auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
        callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
        return ret;
    }
}


// returns the name of a leaderboard
const char * Steam_User_Stats::GetLeaderboardName( SteamLeaderboard_t hSteamLeaderboard )
{
    PRINT_DEBUG("%llu", hSteamLeaderboard);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (hSteamLeaderboard > cached_leaderboards.size() || hSteamLeaderboard <= 0) return "";

    return cached_leaderboards[static_cast<unsigned>(hSteamLeaderboard - 1)].name.c_str();
}


// returns the total number of entries in a leaderboard, as of the last request
int Steam_User_Stats::GetLeaderboardEntryCount( SteamLeaderboard_t hSteamLeaderboard )
{
    PRINT_DEBUG("%llu", hSteamLeaderboard);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (hSteamLeaderboard > cached_leaderboards.size() || hSteamLeaderboard <= 0) return 0;

    return (int)cached_leaderboards[static_cast<unsigned>(hSteamLeaderboard - 1)].entries.size();
}


// returns the sort method of the leaderboard
ELeaderboardSortMethod Steam_User_Stats::GetLeaderboardSortMethod( SteamLeaderboard_t hSteamLeaderboard )
{
    PRINT_DEBUG("%llu", hSteamLeaderboard);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (hSteamLeaderboard > cached_leaderboards.size() || hSteamLeaderboard <= 0) return k_ELeaderboardSortMethodNone;

    return cached_leaderboards[static_cast<unsigned>(hSteamLeaderboard - 1)].sort_method;
}


// returns the display type of the leaderboard
ELeaderboardDisplayType Steam_User_Stats::GetLeaderboardDisplayType( SteamLeaderboard_t hSteamLeaderboard )
{
    PRINT_DEBUG("%llu", hSteamLeaderboard);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (hSteamLeaderboard > cached_leaderboards.size() || hSteamLeaderboard <= 0) return k_ELeaderboardDisplayTypeNone;

    return cached_leaderboards[static_cast<unsigned>(hSteamLeaderboard - 1)].display_type;
}


// Asks the Steam back-end for a set of rows in the leaderboard.
// This call is asynchronous, with the result returned in LeaderboardScoresDownloaded_t
// LeaderboardScoresDownloaded_t will contain a handle to pull the results from GetDownloadedLeaderboardEntries() (below)
// You can ask for more entries than exist, and it will return as many as do exist.
// k_ELeaderboardDataRequestGlobal requests rows in the leaderboard from the full table, with nRangeStart & nRangeEnd in the range [1, TotalEntries]
// k_ELeaderboardDataRequestGlobalAroundUser requests rows around the current user, nRangeStart being negate
//   e.g. DownloadLeaderboardEntries( hLeaderboard, k_ELeaderboardDataRequestGlobalAroundUser, -3, 3 ) will return 7 rows, 3 before the user, 3 after
// k_ELeaderboardDataRequestFriends requests all the rows for friends of the current user 
STEAM_CALL_RESULT( LeaderboardScoresDownloaded_t )
SteamAPICall_t Steam_User_Stats::DownloadLeaderboardEntries( SteamLeaderboard_t hSteamLeaderboard, ELeaderboardDataRequest eLeaderboardDataRequest, int nRangeStart, int nRangeEnd )
{
    PRINT_DEBUG("%llu %i [%i, %i]", hSteamLeaderboard, eLeaderboardDataRequest, nRangeStart, nRangeEnd);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (hSteamLeaderboard > cached_leaderboards.size() || hSteamLeaderboard <= 0) return k_uAPICallInvalid; //might return callresult even if hSteamLeaderboard is invalid

    int entries_count = (int)cached_leaderboards[static_cast<unsigned>(hSteamLeaderboard - 1)].entries.size();
    // https://partner.steamgames.com/doc/api/ISteamUserStats#ELeaderboardDataRequest
    if (eLeaderboardDataRequest != k_ELeaderboardDataRequestFriends) {
        int required_count = nRangeEnd - nRangeStart + 1;
        if (required_count < 0) required_count = 0;
        
        if (required_count < entries_count) entries_count = required_count;
    }
    LeaderboardScoresDownloaded_t data{};
    data.m_hSteamLeaderboard = hSteamLeaderboard;
    data.m_hSteamLeaderboardEntries = hSteamLeaderboard;
    data.m_cEntryCount = entries_count;
    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data), 0.1); // TODO is this timing ok?
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data), 0.1);
    return ret;
}

// as above, but downloads leaderboard entries for an arbitrary set of users - ELeaderboardDataRequest is k_ELeaderboardDataRequestUsers
// if a user doesn't have a leaderboard entry, they won't be included in the result
// a max of 100 users can be downloaded at a time, with only one outstanding call at a time
STEAM_METHOD_DESC(Downloads leaderboard entries for an arbitrary set of users - ELeaderboardDataRequest is k_ELeaderboardDataRequestUsers)
STEAM_CALL_RESULT( LeaderboardScoresDownloaded_t )
SteamAPICall_t Steam_User_Stats::DownloadLeaderboardEntriesForUsers( SteamLeaderboard_t hSteamLeaderboard,
                                                            STEAM_ARRAY_COUNT_D(cUsers, Array of users to retrieve) CSteamID *prgUsers, int cUsers )
{
    PRINT_DEBUG("%i %llu", cUsers, cUsers > 0 ? prgUsers[0].ConvertToUint64() : 0);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (hSteamLeaderboard > cached_leaderboards.size() || hSteamLeaderboard <= 0) return k_uAPICallInvalid; //might return callresult even if hSteamLeaderboard is invalid

    auto& board = cached_leaderboards[static_cast<unsigned>(hSteamLeaderboard - 1)];
    bool ok = true;
    int total_count = 0;
    if (prgUsers && cUsers > 0) {
        for (int i = 0; i < cUsers; ++i) {
            const auto &user_steamid = prgUsers[i];
            if (!user_steamid.IsValid()) {
                ok = false;
                PRINT_DEBUG("bad userid %llu", user_steamid.ConvertToUint64());
                break;
            }
            if (board.find_recent_entry(user_steamid)) ++total_count;

            request_user_leaderboard_entry(board, user_steamid);
        }
    }

    PRINT_DEBUG("total count %i", total_count);
    // https://partner.steamgames.com/doc/api/ISteamUserStats#DownloadLeaderboardEntriesForUsers
    if (!ok || total_count > 100) return k_uAPICallInvalid;

    LeaderboardScoresDownloaded_t data{};
    data.m_hSteamLeaderboard = hSteamLeaderboard;
    data.m_hSteamLeaderboardEntries = hSteamLeaderboard;
    data.m_cEntryCount = total_count;
    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data), 0.1); // TODO is this timing ok?
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data), 0.1);
    return ret;
}


// Returns data about a single leaderboard entry
// use a for loop from 0 to LeaderboardScoresDownloaded_t::m_cEntryCount to get all the downloaded entries
// e.g.
//		void OnLeaderboardScoresDownloaded( LeaderboardScoresDownloaded_t *pLeaderboardScoresDownloaded )
//		{
//			for ( int index = 0; index < pLeaderboardScoresDownloaded->m_cEntryCount; index++ )
//			{
//				LeaderboardEntry_t leaderboardEntry;
//				int32 details[3];		// we know this is how many we've stored previously
//				GetDownloadedLeaderboardEntry( pLeaderboardScoresDownloaded->m_hSteamLeaderboardEntries, index, &leaderboardEntry, details, 3 );
//				assert( leaderboardEntry.m_cDetails == 3 );
//				...
//			}
// once you've accessed all the entries, the data will be free'd, and the SteamLeaderboardEntries_t handle will become invalid
bool Steam_User_Stats::GetDownloadedLeaderboardEntry( SteamLeaderboardEntries_t hSteamLeaderboardEntries, int index, LeaderboardEntry_t *pLeaderboardEntry, int32 *pDetails, int cDetailsMax )
{
    PRINT_DEBUG("[%i] (%i) %llu %p %p", index, cDetailsMax, hSteamLeaderboardEntries, pLeaderboardEntry, pDetails);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (hSteamLeaderboardEntries > cached_leaderboards.size() || hSteamLeaderboardEntries <= 0) return false;
    
    const auto &board = cached_leaderboards[static_cast<unsigned>(hSteamLeaderboardEntries - 1)];
    if (index < 0 || static_cast<size_t>(index) >= board.entries.size()) return false;

    const auto &target_entry = board.entries[index];
    
    if (pLeaderboardEntry) {
        LeaderboardEntry_t entry{};
        entry.m_steamIDUser = target_entry.steam_id;
        entry.m_nGlobalRank = 1 + (int)(&target_entry - &board.entries[0]);
        entry.m_nScore = target_entry.score;
        
        *pLeaderboardEntry = entry;
    }
    
    if (pDetails && cDetailsMax > 0) {
        for (unsigned i = 0; i < target_entry.score_details.size() && i < static_cast<unsigned>(cDetailsMax); ++i) {
            pDetails[i] = target_entry.score_details[i];
        }
    }

    return true;
}


// Uploads a user score to the Steam back-end.
// This call is asynchronous, with the result returned in LeaderboardScoreUploaded_t
// Details are extra game-defined information regarding how the user got that score
// pScoreDetails points to an array of int32's, cScoreDetailsCount is the number of int32's in the list
STEAM_CALL_RESULT( LeaderboardScoreUploaded_t )
SteamAPICall_t Steam_User_Stats::UploadLeaderboardScore( SteamLeaderboard_t hSteamLeaderboard, ELeaderboardUploadScoreMethod eLeaderboardUploadScoreMethod, int32 nScore, const int32 *pScoreDetails, int cScoreDetailsCount )
{
    PRINT_DEBUG("%llu %i", hSteamLeaderboard, nScore);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (hSteamLeaderboard > cached_leaderboards.size() || hSteamLeaderboard <= 0) return k_uAPICallInvalid; //TODO: might return callresult even if hSteamLeaderboard is invalid

    auto &board = cached_leaderboards[static_cast<unsigned>(hSteamLeaderboard - 1)];
    auto my_entry = board.find_recent_entry(settings->get_local_steam_id());
    int current_rank = my_entry
        ? 1 + (int)(my_entry - &board.entries[0])
        : 0;
    int new_rank = current_rank;

    bool score_updated = false;
    if (my_entry) {
        switch (eLeaderboardUploadScoreMethod)
        {
        case k_ELeaderboardUploadScoreMethodKeepBest: { // keep user's best score
            if (board.sort_method == k_ELeaderboardSortMethodAscending) { // keep user's lowest score
                score_updated = nScore < my_entry->score;
            } else { // keep user's highest score
                score_updated = nScore > my_entry->score;
            }
        }
        break;

        case k_ELeaderboardUploadScoreMethodForceUpdate: { // always replace score with specified
            score_updated = my_entry->score != nScore;
        }
        break;
        
        default: break;
        }
    } else { // no entry yet for us
        score_updated = true;
    }

    if (score_updated || (eLeaderboardUploadScoreMethod == k_ELeaderboardUploadScoreMethodForceUpdate)) {
        Steam_Leaderboard_Entry new_entry{};
        new_entry.steam_id = settings->get_local_steam_id();
        new_entry.score = nScore;
        if (pScoreDetails && cScoreDetailsCount  > 0) {
            for (int i = 0; i < cScoreDetailsCount; ++i) {
                new_entry.score_details.push_back(pScoreDetails[i]);
            }
        }
        
        update_leaderboard_entry(board, new_entry);
        new_rank = 1 + (int)(board.find_recent_entry(settings->get_local_steam_id()) - &board.entries[0]);

        // check again in case this was a forced update
        // avoid disk write if score is the same
        if (score_updated) save_my_leaderboard_entry(board);
        send_my_leaderboard_score(board);
            
    }

    LeaderboardScoreUploaded_t data{};
    data.m_bSuccess = 1; //needs to be success or DOA6 freezes when uploading score.
    data.m_hSteamLeaderboard = hSteamLeaderboard;
    data.m_nScore = nScore;
    data.m_bScoreChanged = score_updated;
    data.m_nGlobalRankNew = new_rank;
    data.m_nGlobalRankPrevious = current_rank;
    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data), 0.1); // TODO is this timing ok?
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data), 0.1);
    return ret;
}

SteamAPICall_t Steam_User_Stats::UploadLeaderboardScore( SteamLeaderboard_t hSteamLeaderboard, int32 nScore, int32 *pScoreDetails, int cScoreDetailsCount )
{
	PRINT_DEBUG("old");
	return UploadLeaderboardScore(hSteamLeaderboard, k_ELeaderboardUploadScoreMethodKeepBest, nScore, pScoreDetails, cScoreDetailsCount);
}


// Attaches a piece of user generated content the user's entry on a leaderboard.
// hContent is a handle to a piece of user generated content that was shared using ISteamUserRemoteStorage::FileShare().
// This call is asynchronous, with the result returned in LeaderboardUGCSet_t.
STEAM_CALL_RESULT( LeaderboardUGCSet_t )
SteamAPICall_t Steam_User_Stats::AttachLeaderboardUGC( SteamLeaderboard_t hSteamLeaderboard, UGCHandle_t hUGC )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    LeaderboardUGCSet_t data{};
    if (hSteamLeaderboard > cached_leaderboards.size() || hSteamLeaderboard <= 0) {
        data.m_eResult = k_EResultFail;
    } else {
        data.m_eResult = k_EResultOK;
    }

    data.m_hSteamLeaderboard = hSteamLeaderboard;
    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return ret;
}


// Retrieves the number of players currently playing your game (online + offline)
// This call is asynchronous, with the result returned in NumberOfCurrentPlayers_t
STEAM_CALL_RESULT( NumberOfCurrentPlayers_t )
SteamAPICall_t Steam_User_Stats::GetNumberOfCurrentPlayers()
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    std::random_device rd{};
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int32> distrib(117, 1017);
 
    NumberOfCurrentPlayers_t data{};
    data.m_bSuccess = 1;
    data.m_cPlayers = distrib(gen);
    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return ret;
}


// Requests that Steam fetch data on the percentage of players who have received each achievement
// for the game globally.
// This call is asynchronous, with the result returned in GlobalAchievementPercentagesReady_t.
STEAM_CALL_RESULT( GlobalAchievementPercentagesReady_t )
SteamAPICall_t Steam_User_Stats::RequestGlobalAchievementPercentages()
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    GlobalAchievementPercentagesReady_t data{};
    data.m_eResult = EResult::k_EResultOK;
    data.m_nGameID = settings->get_local_game_id().ToUint64();
    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return ret;
}


// Get the info on the most achieved achievement for the game, returns an iterator index you can use to fetch
// the next most achieved afterwards.  Will return -1 if there is no data on achievement 
// percentages (ie, you haven't called RequestGlobalAchievementPercentages and waited on the callback).
int Steam_User_Stats::GetMostAchievedAchievementInfo( char *pchName, uint32 unNameBufLen, float *pflPercent, bool *pbAchieved )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!pchName) return -1;

    std::string name(GetAchievementName(0));
    if (name.empty()) return -1;

    if (pchName && unNameBufLen) {
        memset(pchName, 0, unNameBufLen);
        name.copy(pchName, unNameBufLen - 1);
    }

    if (pflPercent) *pflPercent = 90;
    if (pbAchieved) {
        bool achieved = false;
        GetAchievement(name.c_str(), &achieved);
        *pbAchieved = achieved;
    }

    return 0;
}


// Get the info on the next most achieved achievement for the game. Call this after GetMostAchievedAchievementInfo or another
// GetNextMostAchievedAchievementInfo call passing the iterator from the previous call. Returns -1 after the last
// achievement has been iterated.
int Steam_User_Stats::GetNextMostAchievedAchievementInfo( int iIteratorPrevious, char *pchName, uint32 unNameBufLen, float *pflPercent, bool *pbAchieved )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (iIteratorPrevious < 0) return -1;
    
    unsigned iIteratorCurrent = static_cast<unsigned>(iIteratorPrevious + 1);
    if (iIteratorCurrent >= defined_achievements.size()) return -1;

    std::string name(GetAchievementName(iIteratorCurrent));
    if (name.empty()) return -1;

    if (pchName && unNameBufLen) {
        memset(pchName, 0, unNameBufLen);
        name.copy(pchName, unNameBufLen - 1);
    }

    if (pflPercent) {
        *pflPercent = (float)(90 * (defined_achievements.size() - iIteratorCurrent) / defined_achievements.size());
    }
    if (pbAchieved) {
        bool achieved = false;
        GetAchievement(name.c_str(), &achieved);
        *pbAchieved = achieved;
    }

    return static_cast<int>(iIteratorCurrent);
}


// Returns the percentage of users who have achieved the specified achievement.
bool Steam_User_Stats::GetAchievementAchievedPercent( const char *pchName, float *pflPercent )
{
    PRINT_DEBUG("'%s'", pchName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    auto it = defined_achievements_find(pchName);
    if (defined_achievements.end() == it) return false;

    size_t idx = it - defined_achievements.begin();
    if (pflPercent) {
        *pflPercent = (float)(90 * (defined_achievements.size() - idx) / defined_achievements.size());
    }
    
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

// For achievements that have related Progress stats, use this to query what the bounds of that progress are.
// You may want this info to selectively call IndicateAchievementProgress when appropriate milestones of progress
// have been made, to show a progress notification to the user.
bool Steam_User_Stats::GetAchievementProgressLimits( const char *pchName, int32 *pnMinProgress, int32 *pnMaxProgress )
{
    PRINT_DEBUG("'%s'", pchName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    float fMinProgress{};
    float fMaxProgress{};
    bool ret = GetAchievementProgressLimits(pchName, &fMinProgress, &fMaxProgress);
    if (ret) {
        if (pnMinProgress) *pnMinProgress = static_cast<int32>(fMinProgress);
        if (pnMaxProgress) *pnMaxProgress = static_cast<int32>(fMaxProgress);
    }
    return ret;
}

bool Steam_User_Stats::GetAchievementProgressLimits( const char *pchName, float *pfMinProgress, float *pfMaxProgress )
{
    PRINT_DEBUG("'%s'", pchName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pchName) return false;

    auto it = defined_achievements.end();
    try {
        it = defined_achievements_find(pchName);
    }
    catch (...) {}
    if (defined_achievements.end() == it) return false;

    if (pfMinProgress) *pfMinProgress = 0;
    if (pfMaxProgress) *pfMaxProgress = 0;

    try {
        std::string pch_name = it->value("name", std::string());
        auto ach = user_achievements.find(pch_name);
        if (user_achievements.end() != ach) {
            auto it_progress = ach->find("progress");
            auto it_max_progress = ach->find("max_progress");
            if (ach->end() == it_progress || ach->end() == it_max_progress) return false;

            if (pfMinProgress) {
                try {
                    if (it_progress->is_number()) {
                        *pfMinProgress = it_progress->get<float>();
                    } else {
                        auto s_ptr = it_progress->get_ptr<std::string*>();
                        if (s_ptr) {
                            *pfMinProgress  = std::stof(*s_ptr);
                        }
                    }
                }catch(...){}
            }
            if (pfMaxProgress) {
                try {
                    if (it_max_progress->is_number()) {
                        *pfMaxProgress = it_max_progress->get<float>();
                    } else {
                        auto s_ptr = it_max_progress->get_ptr<std::string*>();
                        if (s_ptr) {
                            *pfMaxProgress  = std::stof(*s_ptr);
                        }
                    }
                }catch(...){}
            }
            return true;
        }
    }
    catch (...) {}

    return false;
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

void Steam_User_Stats::steam_run_callback()
{
    send_updated_stats();
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


// someone updated their score
void Steam_User_Stats::network_leaderboard_update_score(Common_Message *msg, Steam_Leaderboard &board, bool send_score_back)
{
    CSteamID sender_steamid((uint64)msg->source_id());
    PRINT_DEBUG("got score for user %llu on leaderboard '%s' (send our score back=%i)",
        (uint64)msg->source_id(), board.name.c_str(), (int)send_score_back
    );
    
    // when players initally load a board, and they don't have an entry in it,
    // they send this msg but without their user score entry
    if (msg->leaderboards_messages().has_user_score_entry()) {
        const auto &user_score_msg = msg->leaderboards_messages().user_score_entry();

        Steam_Leaderboard_Entry updated_entry{};
        updated_entry.steam_id = sender_steamid;
        updated_entry.score = user_score_msg.score();
        updated_entry.score_details.reserve(user_score_msg.score_details().size());
        updated_entry.score_details.assign(user_score_msg.score_details().begin(), user_score_msg.score_details().end());
        update_leaderboard_entry(board, updated_entry);
    }

    // if the sender wants back our score, send it to all, not just them
    // in case we have 3 or more players and none of them have our data
    if (send_score_back) send_my_leaderboard_score(board);
}

// someone is requesting our score on a leaderboard
void Steam_User_Stats::network_leaderboard_send_my_score(Common_Message *msg, const Steam_Leaderboard &board)
{
    CSteamID sender_steamid((uint64)msg->source_id());
    PRINT_DEBUG("user %llu requested our score for leaderboard '%s'", (uint64)msg->source_id(), board.name.c_str());

    send_my_leaderboard_score(board, &sender_steamid);
}

void Steam_User_Stats::network_callback_leaderboards(Common_Message *msg)
{
    // network->sendToAll() sends to current user also
    if (msg->source_id() == settings->get_local_steam_id().ConvertToUint64()) return; 
    if (settings->get_local_game_id().AppID() != msg->leaderboards_messages().appid()) return;

    if (!msg->leaderboards_messages().has_leaderboard_info()) {
        PRINT_DEBUG("error empty leaderboard msg");
        return;
    }

    const auto &board_info_msg = msg->leaderboards_messages().leaderboard_info();
    
    PRINT_DEBUG("attempting to cache leaderboard '%s'", board_info_msg.board_name().c_str());
    unsigned int board_handle = cache_leaderboard_ifneeded(
        board_info_msg.board_name(),
        (ELeaderboardSortMethod)board_info_msg.sort_method(),
        (ELeaderboardDisplayType)board_info_msg.display_type()
    );

    switch (msg->leaderboards_messages().type()) {
        // someone updated their score
        case Leaderboards_Messages::UpdateUserScore:
            network_leaderboard_update_score(msg, cached_leaderboards[board_handle - 1], false);
        break;

        // someone updated their score and wants us to share back ours
        case Leaderboards_Messages::UpdateUserScoreMutual:
            network_leaderboard_update_score(msg, cached_leaderboards[board_handle - 1], true);
        break;

        // someone is requesting our score on a leaderboard
        case Leaderboards_Messages::RequestUserScore:
            network_leaderboard_send_my_score(msg, cached_leaderboards[board_handle - 1]);
        break;
        
        default:
            PRINT_DEBUG("unhandled type %i", (int)msg->leaderboards_messages().type());
        break;
    }

}


// user connect/disconnect
void Steam_User_Stats::network_callback_low_level(Common_Message *msg)
{
    CSteamID steamid((uint64)msg->source_id());
    // this should never happen, but just in case
    if (steamid == settings->get_local_steam_id()) return;

    switch (msg->low_level().type())
    {
    case Low_Level::CONNECT:
        // nothing
    break;
    
    case Low_Level::DISCONNECT: {
        for (auto &board : cached_leaderboards) {
            board.remove_entries(steamid);
        }
        
        // PRINT_DEBUG("removed user %llu", (uint64)steamid.ConvertToUint64());
    }
    break;
    
    default:
        PRINT_DEBUG("unknown type %i", (int)msg->low_level().type());
    break;
    }
}
