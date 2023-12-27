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

#ifndef __INCLUDED_STEAM_USER_STATS_H__
#define __INCLUDED_STEAM_USER_STATS_H__

#include <limits>
#include "base.h"
#include "overlay_experimental/steam_overlay.h"

struct Steam_Leaderboard_Score {
    CSteamID steam_id;
    int32 score;
    std::vector<int32> score_details;
};

struct Steam_Leaderboard {
    std::string name;
    ELeaderboardSortMethod sort_method;
    ELeaderboardDisplayType display_type;
    Steam_Leaderboard_Score self_score;
};

struct achievement_trigger {
    std::string name;
    std::string value_operation;
    std::string min_value;
    std::string max_value;

    bool check_triggered(float stat) {
        try {
            if (std::stof(max_value) <= stat) return true;
        } catch (...) {}

        return false;
    }

    bool check_triggered(int32 stat) {
        try {
            if (std::stoi(max_value) <= stat) return true;
        } catch (...) {}

        return false;
    }
};

class Steam_User_Stats :
public ISteamUserStats003,
public ISteamUserStats004,
public ISteamUserStats005,
public ISteamUserStats006,
public ISteamUserStats007,
public ISteamUserStats008,
public ISteamUserStats009,
public ISteamUserStats010,
public ISteamUserStats011,
public ISteamUserStats
{
public:
    static constexpr auto achievements_user_file = "achievements.json";

private:

    Local_Storage *local_storage;
    Settings *settings;
    SteamCallResults *callback_results;
    class SteamCallBacks *callbacks;
    class Steam_Overlay* overlay;

    std::vector<struct Steam_Leaderboard> leaderboards;

    nlohmann::json defined_achievements;
    nlohmann::json user_achievements;
    std::vector<std::string> sorted_achievement_names;
    std::map<std::string, int32> stats_cache_int;
    std::map<std::string, float> stats_cache_float;

    std::map<std::string, std::vector<achievement_trigger>> achievement_stat_trigger;

unsigned int find_leaderboard(std::string name)
{
    unsigned index = 1;
    for (auto &leaderboard : leaderboards) {
        if (leaderboard.name == name) return index;
        ++index;
    }

    return 0;
}

nlohmann::detail::iter_impl<nlohmann::json> defined_achievements_find(std::string key)
{
    return std::find_if(defined_achievements.begin(), defined_achievements.end(), [key](nlohmann::json& item) {
            std::string name = static_cast<std::string const&>(item["name"]);
            return key.size() == name.size() && std::equal(name.begin(), name.end(), key.begin(),
                                                            [](char a, char b) {
                                                                return tolower(a) == tolower(b);
                                                            });
        });
}

void load_achievements_db()
{
    std::string file_path = Local_Storage::get_game_settings_path() + achievements_user_file;
    local_storage->load_json(file_path, defined_achievements);
}

void load_achievements()
{
    local_storage->load_json_file("", achievements_user_file, user_achievements);
}

void save_achievements()
{
    local_storage->write_json_file("", achievements_user_file, user_achievements);
}

void save_leaderboard_score(Steam_Leaderboard *leaderboard)
{
    std::vector<uint32_t> output;
    uint64_t steam_id = leaderboard->self_score.steam_id.ConvertToUint64();
    output.push_back(steam_id & 0xFFFFFFFF);
    output.push_back(steam_id >> 32);

    output.push_back(leaderboard->self_score.score);
    output.push_back(leaderboard->self_score.score_details.size());
    for (auto &s : leaderboard->self_score.score_details) {
        output.push_back(s);
    }

    std::string leaderboard_name = ascii_to_lowercase(leaderboard->name);
    local_storage->store_data(Local_Storage::leaderboard_storage_folder, leaderboard_name, (char* )output.data(), sizeof(uint32_t) * output.size());
}

std::vector<Steam_Leaderboard_Score> load_leaderboard_scores(std::string name)
{
    std::vector<Steam_Leaderboard_Score> out;

    std::string leaderboard_name = ascii_to_lowercase(name);
    unsigned size = local_storage->file_size(Local_Storage::leaderboard_storage_folder, leaderboard_name);
    if (size == 0 || (size % sizeof(uint32_t)) != 0) return out;

    std::vector<uint32_t> output(size / sizeof(uint32_t));
    if (local_storage->get_data(Local_Storage::leaderboard_storage_folder, leaderboard_name, (char* )output.data(), size) != size) return out;

    unsigned i = 0;
    while (true) {
        if ((i + 4) > output.size()) break;

        Steam_Leaderboard_Score score;
        score.steam_id = CSteamID((uint64)output[i] + (((uint64)output[i + 1]) << 32));
        i += 2;
        score.score = output[i];
        i += 1;
        unsigned count = output[i];
        i += 1;

        if ((i + count) > output.size()) break;

        for (unsigned j = 0; j < count; ++j) {
            score.score_details.push_back(output[i]);
            i += 1;
        }

        PRINT_DEBUG("loaded leaderboard score %llu %u\n", score.steam_id.ConvertToUint64(), score.score);
        out.push_back(score);
    }

    return out;
}

std::string get_value_for_language(nlohmann::json &json, std::string key, std::string language)
{
    auto x = json.find(key);
    if (x == json.end()) return "";
    if (x.value().is_string()) {
        return x.value().get<std::string>();
    } else if (x.value().is_object()) {
        auto l = x.value().find(language);
        if (l != x.value().end()) {
            return l.value().get<std::string>();
        }

        l = x.value().find("english");
        if (l != x.value().end()) {
            return l.value().get<std::string>();
        }

        l = x.value().begin();
        if (l != x.value().end()) {
            if (l.key() == "token") {
                std::string token_value = l.value().get<std::string>();
                l++;
                if (l != x.value().end()) {
                    return l.value().get<std::string>();
                }

                return token_value;
            }

            return l.value().get<std::string>();
        }
    }

    return "";
}

public:
Steam_User_Stats(Settings *settings, Local_Storage *local_storage, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, Steam_Overlay* overlay):
    settings(settings),
    local_storage(local_storage),
    callback_results(callback_results),
    callbacks(callbacks),
    defined_achievements(nlohmann::json::object()),
    user_achievements(nlohmann::json::object()),
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
            if (user_achievements.find(name) == user_achievements.end()) {
                user_achievements[name]["earned"] = false;
                user_achievements[name]["earned_time"] = static_cast<uint32>(0);
            }

            achievement_trigger trig;
            trig.name = name;
            trig.value_operation = static_cast<std::string const&>(it["progress"]["value"]["operation"]);
            std::string stat_name = ascii_to_lowercase(static_cast<std::string const&>(it["progress"]["value"]["operand1"]));
            trig.min_value = static_cast<std::string const&>(it["progress"]["min_val"]);
            trig.max_value = static_cast<std::string const&>(it["progress"]["max_val"]);
            achievement_stat_trigger[stat_name].push_back(trig);
        } catch (...) {}

        try {
            it["hidden"] = std::to_string(it["hidden"].get<int>());
        } catch (...) {}

        it["displayName"] = get_value_for_language(it, "displayName", settings->get_language());
        it["description"] = get_value_for_language(it, "description", settings->get_language());
    }

    //TODO: not sure if the sort is actually case insensitive, ach names seem to be treated by steam as case insensitive so I assume they are.
    //need to find a game with achievements of different case names to confirm
    std::sort(sorted_achievement_names.begin(), sorted_achievement_names.end(), [](const std::string lhs, const std::string rhs){
        const auto result = std::mismatch(lhs.cbegin(), lhs.cend(), rhs.cbegin(), rhs.cend(), [](const unsigned char lhs, const unsigned char rhs){return std::tolower(lhs) == std::tolower(rhs);});
        return result.second != rhs.cend() && (result.first == lhs.cend() || std::tolower(*result.first) < std::tolower(*result.second));}
    );
}

// Ask the server to send down this user's data and achievements for this game
STEAM_CALL_BACK( UserStatsReceived_t )
bool RequestCurrentStats()
{
    PRINT_DEBUG("Steam_User_Stats::RequestCurrentStats\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    UserStatsReceived_t data;
    data.m_nGameID = settings->get_local_game_id().ToUint64();
    data.m_eResult = k_EResultOK;
    data.m_steamIDUser = settings->get_local_steam_id();
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data), 0.1);
    return true;
}


// Data accessors
bool GetStat( const char *pchName, int32 *pData )
{
    PRINT_DEBUG("GetStat int32 %s\n", pchName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pchName || !pData) return false;
    std::string stat_name = ascii_to_lowercase(pchName);

    auto stats_config = settings->getStats();
    auto stats_data = stats_config.find(stat_name);
    if (stats_data != stats_config.end()) {
        if (stats_data->second.type != Stat_Type::STAT_TYPE_INT) return false;
    }

    auto cached_stat = stats_cache_int.find(stat_name);
    if (cached_stat != stats_cache_int.end()) {
        *pData = cached_stat->second;
        return true;
    }

    int32 output = 0;
    int read_data = local_storage->get_data(Local_Storage::stats_storage_folder, stat_name, (char* )&output, sizeof(output));
    if (read_data == sizeof(int32)) {
        stats_cache_int[stat_name] = output;
        *pData = output;
        return true;
    }

    if (stats_data != stats_config.end()) {
        stats_cache_int[stat_name] = stats_data->second.default_value_int;
        *pData = stats_data->second.default_value_int;
        return true;
    }

    return false;
}

bool GetStat( const char *pchName, float *pData )
{
    PRINT_DEBUG("GetStat float %s\n", pchName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pchName || !pData) return false;
    std::string stat_name = ascii_to_lowercase(pchName);

    auto stats_config = settings->getStats();
    auto stats_data = stats_config.find(stat_name);
    if (stats_data != stats_config.end()) {
        if (stats_data->second.type == Stat_Type::STAT_TYPE_INT) return false;
    }

    auto cached_stat = stats_cache_float.find(stat_name);
    if (cached_stat != stats_cache_float.end()) {
        *pData = cached_stat->second;
        return true;
    }

    float output = 0.0;
    int read_data = local_storage->get_data(Local_Storage::stats_storage_folder, stat_name, (char* )&output, sizeof(output));
    if (read_data == sizeof(float)) {
        stats_cache_float[stat_name] = output;
        *pData = output;
        return true;
    }

    if (stats_data != stats_config.end()) {
        stats_cache_float[stat_name] = stats_data->second.default_value_float;
        *pData = stats_data->second.default_value_float;
        return true;
    }

    return false;
}


// Set / update data
bool SetStat( const char *pchName, int32 nData )
{
    PRINT_DEBUG("SetStat int32 %s\n", pchName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pchName) return false;
    std::string stat_name = ascii_to_lowercase(pchName);

    auto cached_stat = stats_cache_int.find(stat_name);
    if (cached_stat != stats_cache_int.end()) {
        if (cached_stat->second == nData) return true;
    }

    auto stat_trigger = achievement_stat_trigger.find(stat_name);
    if (stat_trigger != achievement_stat_trigger.end()) {
        for (auto &t : stat_trigger->second) {
            if (t.check_triggered(nData)) {
                SetAchievement(t.name.c_str());
            }
        }
    }

    if (local_storage->store_data(Local_Storage::stats_storage_folder, stat_name, (char* )&nData, sizeof(nData)) == sizeof(nData)) {
        stats_cache_int[stat_name] = nData;
        return true;
    }

    return false;
}

bool SetStat( const char *pchName, float fData )
{
    PRINT_DEBUG("SetStat float %s\n", pchName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pchName) return false;
    std::string stat_name = ascii_to_lowercase(pchName);

    auto cached_stat = stats_cache_float.find(stat_name);
    if (cached_stat != stats_cache_float.end()) {
        if (cached_stat->second == fData) return true;
    }

    auto stat_trigger = achievement_stat_trigger.find(stat_name);
    if (stat_trigger != achievement_stat_trigger.end()) {
        for (auto &t : stat_trigger->second) {
            if (t.check_triggered(fData)) {
                SetAchievement(t.name.c_str());
            }
        }
    }

    if (local_storage->store_data(Local_Storage::stats_storage_folder, stat_name, (char* )&fData, sizeof(fData)) == sizeof(fData)) {
        stats_cache_float[stat_name] = fData;
        return true;
    }

    return false;
}

bool UpdateAvgRateStat( const char *pchName, float flCountThisSession, double dSessionLength )
{
    PRINT_DEBUG("UpdateAvgRateStat %s\n", pchName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pchName) return false;
    std::string stat_name = ascii_to_lowercase(pchName);

    char data[sizeof(float) + sizeof(float) + sizeof(double)];
    int read_data = local_storage->get_data(Local_Storage::stats_storage_folder, stat_name, (char* )data, sizeof(*data));
    float oldcount = 0;
    double oldsessionlength = 0;
    if (read_data == sizeof(data)) {
        memcpy(&oldcount, data + sizeof(float), sizeof(oldcount));
        memcpy(&oldsessionlength, data + sizeof(float) + sizeof(double), sizeof(oldsessionlength));
    }

    oldcount += flCountThisSession;
    oldsessionlength += dSessionLength;

    float average = oldcount / oldsessionlength;
    memcpy(data, &average, sizeof(average));
    memcpy(data + sizeof(float), &oldcount, sizeof(oldcount));
    memcpy(data + sizeof(float) * 2, &oldsessionlength, sizeof(oldsessionlength));

    if (local_storage->store_data(Local_Storage::stats_storage_folder, stat_name, data, sizeof(data)) == sizeof(data)) {
        stats_cache_float[stat_name] = average;
        return true;
    }

    return false;
}


// Achievement flag accessors
bool GetAchievement( const char *pchName, bool *pbAchieved )
{
    PRINT_DEBUG("GetAchievement %s\n", pchName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (pchName == nullptr) return false;

    try {
        auto it = defined_achievements_find(pchName);
        if (it == defined_achievements.end()) return false;
        std::string pch_name = it->value("name", std::string());

        auto ach = user_achievements.find(pch_name);
        if (it != defined_achievements.end() && ach != user_achievements.end()) {
            if(pbAchieved != nullptr) *pbAchieved = (*ach)["earned"];
            return true;
        }
    } catch (...) {}

    if (pbAchieved != nullptr)* pbAchieved = false;

    return false;
}

bool SetAchievement( const char *pchName )
{
    PRINT_DEBUG("SetAchievement %s\n", pchName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (pchName == nullptr) return false;
    
    if (settings->achievement_bypass) return true;

    try {
        auto it = defined_achievements_find(pchName);
        if (it == defined_achievements.end()) return false;
        std::string pch_name = it->value("name", std::string());

        if (it != defined_achievements.end()) {
            if (user_achievements.find(pch_name) == user_achievements.end() || user_achievements[pch_name].value("earned", false) == false) {
                user_achievements[pch_name]["earned"] = true;
                user_achievements[pch_name]["earned_time"] = std::chrono::duration_cast<std::chrono::duration<uint32>>(std::chrono::system_clock::now().time_since_epoch()).count();
#ifdef EMU_OVERLAY
                overlay->AddAchievementNotification(it.value());
#endif
                save_achievements();
            }

            return true;
        }
    } catch (...) {}

    return false;
}

bool ClearAchievement( const char *pchName )
{
    PRINT_DEBUG("ClearAchievement %s\n", pchName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (pchName == nullptr) return false;

    try {
        auto it = defined_achievements_find(pchName);
        if (it == defined_achievements.end()) return false;
        std::string pch_name = it->value("name", std::string());

        if (it != defined_achievements.end()) {
            user_achievements[pch_name]["earned"] = false;
            user_achievements[pch_name]["earned_time"] = static_cast<uint32>(0);
            save_achievements();
            return true;
        }
    } catch (...) {}

    return false;
}


// Get the achievement status, and the time it was unlocked if unlocked.
// If the return value is true, but the unlock time is zero, that means it was unlocked before Steam 
// began tracking achievement unlock times (December 2009). Time is seconds since January 1, 1970.
bool GetAchievementAndUnlockTime( const char *pchName, bool *pbAchieved, uint32 *punUnlockTime )
{
    PRINT_DEBUG("GetAchievementAndUnlockTime\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (pchName == nullptr) return false;

    try {
        auto it = defined_achievements_find(pchName);
        if (it == defined_achievements.end()) return false;
        std::string pch_name = it->value("name", std::string());

        auto ach = user_achievements.find(pch_name);
        if (it != defined_achievements.end() && ach != user_achievements.end()) {
            if(pbAchieved != nullptr) *pbAchieved = (*ach)["earned"];
            if(punUnlockTime != nullptr) *punUnlockTime = (*ach)["earned_time"];
            return true;
        }
    } catch (...) {}

    if(pbAchieved != nullptr) *pbAchieved = false;
    if(punUnlockTime != nullptr) *punUnlockTime = 0;
    return false;
}


// Store the current data on the server, will get a callback when set
// And one callback for every new achievement
//
// If the callback has a result of k_EResultInvalidParam, one or more stats 
// uploaded has been rejected, either because they broke constraints
// or were out of date. In this case the server sends back updated values.
// The stats should be re-iterated to keep in sync.
bool StoreStats()
{
    PRINT_DEBUG("StoreStats\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    UserStatsStored_t data;
    data.m_nGameID = settings->get_local_game_id().ToUint64();
    data.m_eResult = k_EResultOK;
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data), 0.01);
    return true;
}


// Achievement / GroupAchievement metadata

// Gets the icon of the achievement, which is a handle to be used in ISteamUtils::GetImageRGBA(), or 0 if none set. 
// A return value of 0 may indicate we are still fetching data, and you can wait for the UserAchievementIconFetched_t callback
// which will notify you when the bits are ready. If the callback still returns zero, then there is no image set for the
// specified achievement.
int GetAchievementIcon( const char *pchName )
{
    PRINT_DEBUG("GetAchievementIcon\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (pchName == nullptr) return 0;

    return 0;
}

std::string get_achievement_icon_name( const char *pchName, bool pbAchieved )
{
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (pchName == nullptr) return "";

    try {
        auto it = defined_achievements_find(pchName);
        if (it != defined_achievements.end()) {
            if (pbAchieved) return it.value()["icon"].get<std::string>();
            else return it.value()["icon_gray"].get<std::string>();
        }
    } catch (...) {}

    return "";
}


// Get general attributes for an achievement. Accepts the following keys:
// - "name" and "desc" for retrieving the localized achievement name and description (returned in UTF8)
// - "hidden" for retrieving if an achievement is hidden (returns "0" when not hidden, "1" when hidden)
const char * GetAchievementDisplayAttribute( const char *pchName, const char *pchKey )
{
    PRINT_DEBUG("GetAchievementDisplayAttribute %s %s\n", pchName, pchKey);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (pchName == nullptr) return "";
    if (pchKey == nullptr) return "";


    if (strcmp (pchKey, "name") == 0) {
        try {
            auto it = defined_achievements_find(pchName);
            if (it != defined_achievements.end()) {
                return it.value()["displayName"].get_ptr<std::string*>()->c_str();
            }
        } catch (...) {}
    }

    if (strcmp (pchKey, "desc") == 0) {
        try {
            auto it = defined_achievements_find(pchName);
            if (it != defined_achievements.end()) {
                return it.value()["description"].get_ptr<std::string*>()->c_str();
            }
        } catch (...) {}
    }

    if (strcmp (pchKey, "hidden") == 0) {
        try {
            auto it = defined_achievements_find(pchName);
            if (it != defined_achievements.end()) {
                return it.value()["hidden"].get_ptr<std::string*>()->c_str();
            }
        } catch (...) {}
    }

    return "";
}


// Achievement progress - triggers an AchievementProgress callback, that is all.
// Calling this w/ N out of N progress will NOT set the achievement, the game must still do that.
bool IndicateAchievementProgress( const char *pchName, uint32 nCurProgress, uint32 nMaxProgress )
{
    PRINT_DEBUG("IndicateAchievementProgress %s\n", pchName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (pchName == nullptr) return false;

    try {
        auto it = defined_achievements_find(pchName);
        if (it == defined_achievements.end()) return false;

        std::string pch_name = it->value("name", std::string());

        auto ach = user_achievements.find(pch_name);
        if (it != defined_achievements.end()) {
            bool achieved = false;
            if ( ach != user_achievements.end()) {
                bool achieved = ach->value("earned", false);
            }

            UserAchievementStored_t data = {};
            data.m_nGameID = settings->get_local_game_id().ToUint64();
            data.m_bGroupAchievement = false;
            strncpy(data.m_rgchAchievementName, pch_name.c_str(), k_cchStatNameMax);

            if (achieved) {
                data.m_nCurProgress = 0;
                data.m_nMaxProgress = 0;
            } else {
                user_achievements[pch_name]["progress"] = nCurProgress;
                user_achievements[pch_name]["max_progress"] = nMaxProgress;
                data.m_nCurProgress = nCurProgress;
                data.m_nMaxProgress = nMaxProgress;
            }

            save_achievements();
            callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
            return true;
        }
    } catch (...) {}

    return false;
}


// Used for iterating achievements. In general games should not need these functions because they should have a
// list of existing achievements compiled into them
uint32 GetNumAchievements()
{
    PRINT_DEBUG("GetNumAchievements\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return defined_achievements.size();
}

// Get achievement name iAchievement in [0,GetNumAchievements)
const char * GetAchievementName( uint32 iAchievement )
{
    PRINT_DEBUG("GetAchievementName\n");
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
SteamAPICall_t RequestUserStats( CSteamID steamIDUser )
{
    PRINT_DEBUG("Steam_User_Stats::RequestUserStats %llu\n", steamIDUser.ConvertToUint64());
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    // Enable this to allow hot reload achievements status
    //if (steamIDUser == settings->get_local_steam_id()) {
    //    load_achievements();
    //}
    

    UserStatsReceived_t data;
    data.m_nGameID = settings->get_local_game_id().ToUint64();
    data.m_eResult = k_EResultOK;
    data.m_steamIDUser = steamIDUser;
    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data), 0.1);
}


// requests stat information for a user, usable after a successful call to RequestUserStats()
bool GetUserStat( CSteamID steamIDUser, const char *pchName, int32 *pData )
{
    PRINT_DEBUG("GetUserStat %s %llu\n", pchName, steamIDUser.ConvertToUint64());
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (pchName == nullptr) return false;

    if (steamIDUser == settings->get_local_steam_id()) {
        GetStat(pchName, pData);
    } else {
        *pData = 0;
    }

    return true;
}

bool GetUserStat( CSteamID steamIDUser, const char *pchName, float *pData )
{
    PRINT_DEBUG("GetUserStat %s %llu\n", pchName, steamIDUser.ConvertToUint64());
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (pchName == nullptr) return false;

    if (steamIDUser == settings->get_local_steam_id()) {
        GetStat(pchName, pData);
    } else {
        *pData = 0;
    }

    return true;
}

bool GetUserAchievement( CSteamID steamIDUser, const char *pchName, bool *pbAchieved )
{
    PRINT_DEBUG("GetUserAchievement %s\n", pchName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    if (pchName == nullptr) return false;

    if (steamIDUser == settings->get_local_steam_id()) {
        return GetAchievement(pchName, pbAchieved);
    }

    return false;
}

// See notes for GetAchievementAndUnlockTime above
bool GetUserAchievementAndUnlockTime( CSteamID steamIDUser, const char *pchName, bool *pbAchieved, uint32 *punUnlockTime )
{
    PRINT_DEBUG("GetUserAchievementAndUnlockTime %s\n", pchName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (pchName == nullptr) return false;

    if (steamIDUser == settings->get_local_steam_id()) {
        return GetAchievementAndUnlockTime(pchName, pbAchieved, punUnlockTime);
    }
    return false;
}


// Reset stats 
bool ResetAllStats( bool bAchievementsToo )
{
    PRINT_DEBUG("ResetAllStats\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    //TODO
    if (bAchievementsToo) {
        std::for_each(user_achievements.begin(), user_achievements.end(), [](nlohmann::json& v) {
            v["earned"] = false;
            v["earned_time"] = 0;
        });
    }

    return true;
}


// Leaderboard functions

// asks the Steam back-end for a leaderboard by name, and will create it if it's not yet
// This call is asynchronous, with the result returned in LeaderboardFindResult_t
STEAM_CALL_RESULT(LeaderboardFindResult_t)
SteamAPICall_t FindOrCreateLeaderboard( const char *pchLeaderboardName, ELeaderboardSortMethod eLeaderboardSortMethod, ELeaderboardDisplayType eLeaderboardDisplayType )
{
    PRINT_DEBUG("FindOrCreateLeaderboard %s\n", pchLeaderboardName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!pchLeaderboardName) {
        LeaderboardFindResult_t data;
        data.m_hSteamLeaderboard = 0;
        data.m_bLeaderboardFound = 0;
        return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    }

    unsigned int leader = find_leaderboard(pchLeaderboardName);
    if (!leader) {
        struct Steam_Leaderboard leaderboard;
        leaderboard.name = std::string(pchLeaderboardName);
        leaderboard.sort_method = eLeaderboardSortMethod;
        leaderboard.display_type = eLeaderboardDisplayType;
        leaderboard.self_score.score = eLeaderboardSortMethod == k_ELeaderboardSortMethodAscending ? INT_MAX : INT_MIN;

        std::vector<Steam_Leaderboard_Score> scores = load_leaderboard_scores(pchLeaderboardName);
        for (auto &s : scores) {
            if (s.steam_id == settings->get_local_steam_id()) {
                leaderboard.self_score = s;
            }
        }

        leaderboards.push_back(leaderboard);
        leader = leaderboards.size();
    }

    LeaderboardFindResult_t data;
    data.m_hSteamLeaderboard = leader;
    data.m_bLeaderboardFound = 1;
    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}


// as above, but won't create the leaderboard if it's not found
// This call is asynchronous, with the result returned in LeaderboardFindResult_t
STEAM_CALL_RESULT( LeaderboardFindResult_t )
SteamAPICall_t FindLeaderboard( const char *pchLeaderboardName )
{
    PRINT_DEBUG("FindLeaderboard %s\n", pchLeaderboardName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!pchLeaderboardName) {
        LeaderboardFindResult_t data;
        data.m_hSteamLeaderboard = 0;
        data.m_bLeaderboardFound = 0;
        return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    }

    auto settings_Leaderboards = settings->getLeaderboards();
    if (settings_Leaderboards.count(pchLeaderboardName)) {
        auto config = settings_Leaderboards[pchLeaderboardName];
        return FindOrCreateLeaderboard(pchLeaderboardName, config.sort_method, config.display_type);
    } else if (settings->createUnknownLeaderboards()) {
        return FindOrCreateLeaderboard(pchLeaderboardName, k_ELeaderboardSortMethodDescending, k_ELeaderboardDisplayTypeNumeric);
    } else {
        LeaderboardFindResult_t data;
        data.m_hSteamLeaderboard = find_leaderboard(pchLeaderboardName);;
        data.m_bLeaderboardFound = !!data.m_hSteamLeaderboard;
        return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    }
}


// returns the name of a leaderboard
const char * GetLeaderboardName( SteamLeaderboard_t hSteamLeaderboard )
{
    PRINT_DEBUG("GetLeaderboardName\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (hSteamLeaderboard > leaderboards.size() || hSteamLeaderboard <= 0) return "";
    return leaderboards[hSteamLeaderboard - 1].name.c_str();
}


// returns the total number of entries in a leaderboard, as of the last request
int GetLeaderboardEntryCount( SteamLeaderboard_t hSteamLeaderboard )
{
    PRINT_DEBUG("GetLeaderboardEntryCount\n");
    return 0;
}


// returns the sort method of the leaderboard
ELeaderboardSortMethod GetLeaderboardSortMethod( SteamLeaderboard_t hSteamLeaderboard )
{
    PRINT_DEBUG("GetLeaderboardSortMethod\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (hSteamLeaderboard > leaderboards.size() || hSteamLeaderboard <= 0) return k_ELeaderboardSortMethodNone;
    return leaderboards[hSteamLeaderboard - 1].sort_method; 
}


// returns the display type of the leaderboard
ELeaderboardDisplayType GetLeaderboardDisplayType( SteamLeaderboard_t hSteamLeaderboard )
{
    PRINT_DEBUG("GetLeaderboardDisplayType\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (hSteamLeaderboard > leaderboards.size() || hSteamLeaderboard <= 0) return k_ELeaderboardDisplayTypeNone;
    return leaderboards[hSteamLeaderboard - 1].display_type; 
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
SteamAPICall_t DownloadLeaderboardEntries( SteamLeaderboard_t hSteamLeaderboard, ELeaderboardDataRequest eLeaderboardDataRequest, int nRangeStart, int nRangeEnd )
{
    PRINT_DEBUG("DownloadLeaderboardEntries %llu %i %i %i\n", hSteamLeaderboard, eLeaderboardDataRequest, nRangeStart, nRangeEnd);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (hSteamLeaderboard > leaderboards.size() || hSteamLeaderboard <= 0) return k_uAPICallInvalid; //might return callresult even if hSteamLeaderboard is invalid

    LeaderboardScoresDownloaded_t data;
    data.m_hSteamLeaderboard = hSteamLeaderboard;
    data.m_hSteamLeaderboardEntries = hSteamLeaderboard;
    data.m_cEntryCount = leaderboards[hSteamLeaderboard - 1].self_score.steam_id.IsValid();
    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}

// as above, but downloads leaderboard entries for an arbitrary set of users - ELeaderboardDataRequest is k_ELeaderboardDataRequestUsers
// if a user doesn't have a leaderboard entry, they won't be included in the result
// a max of 100 users can be downloaded at a time, with only one outstanding call at a time
STEAM_METHOD_DESC(Downloads leaderboard entries for an arbitrary set of users - ELeaderboardDataRequest is k_ELeaderboardDataRequestUsers)
STEAM_CALL_RESULT( LeaderboardScoresDownloaded_t )
SteamAPICall_t DownloadLeaderboardEntriesForUsers( SteamLeaderboard_t hSteamLeaderboard,
                                                            STEAM_ARRAY_COUNT_D(cUsers, Array of users to retrieve) CSteamID *prgUsers, int cUsers )
{
    PRINT_DEBUG("DownloadLeaderboardEntriesForUsers %i %llu\n", cUsers, cUsers > 0 ? prgUsers[0].ConvertToUint64() : 0);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (hSteamLeaderboard > leaderboards.size() || hSteamLeaderboard <= 0) return k_uAPICallInvalid; //might return callresult even if hSteamLeaderboard is invalid

    bool get_for_current_id = false;
    for (int i = 0; i < cUsers; ++i) {
        if (prgUsers[i] == settings->get_local_steam_id()) {
            get_for_current_id = true;
        }
    }

    LeaderboardScoresDownloaded_t data;
    data.m_hSteamLeaderboard = hSteamLeaderboard;
    data.m_hSteamLeaderboardEntries = hSteamLeaderboard;
    data.m_cEntryCount = get_for_current_id && leaderboards[hSteamLeaderboard - 1].self_score.steam_id.IsValid();
    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
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
bool GetDownloadedLeaderboardEntry( SteamLeaderboardEntries_t hSteamLeaderboardEntries, int index, LeaderboardEntry_t *pLeaderboardEntry, int32 *pDetails, int cDetailsMax )
{
    PRINT_DEBUG("GetDownloadedLeaderboardEntry\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (hSteamLeaderboardEntries > leaderboards.size() || hSteamLeaderboardEntries <= 0) return false;
    if (index > 0) return false;

    LeaderboardEntry_t entry = {};
    entry.m_steamIDUser = leaderboards[hSteamLeaderboardEntries - 1].self_score.steam_id;
    entry.m_nGlobalRank = 1;
    entry.m_nScore = leaderboards[hSteamLeaderboardEntries - 1].self_score.score;
    for (int i = 0; i < leaderboards[hSteamLeaderboardEntries - 1].self_score.score_details.size() && i < cDetailsMax; ++i) {
        pDetails[i] = leaderboards[hSteamLeaderboardEntries - 1].self_score.score_details[i];
    }

    *pLeaderboardEntry = entry;
    return true;
}


// Uploads a user score to the Steam back-end.
// This call is asynchronous, with the result returned in LeaderboardScoreUploaded_t
// Details are extra game-defined information regarding how the user got that score
// pScoreDetails points to an array of int32's, cScoreDetailsCount is the number of int32's in the list
STEAM_CALL_RESULT( LeaderboardScoreUploaded_t )
SteamAPICall_t UploadLeaderboardScore( SteamLeaderboard_t hSteamLeaderboard, ELeaderboardUploadScoreMethod eLeaderboardUploadScoreMethod, int32 nScore, const int32 *pScoreDetails, int cScoreDetailsCount )
{
    PRINT_DEBUG("UploadLeaderboardScore %i\n", nScore);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (hSteamLeaderboard > leaderboards.size() || hSteamLeaderboard <= 0) return k_uAPICallInvalid; //TODO: might return callresult even if hSteamLeaderboard is invalid

    Steam_Leaderboard_Score score;
    score.score = nScore;
    score.steam_id = settings->get_local_steam_id();
    for (int i = 0; i < cScoreDetailsCount; ++i) {
        score.score_details.push_back(pScoreDetails[i]);
    }

    bool changed = false;
    if (eLeaderboardUploadScoreMethod == k_ELeaderboardUploadScoreMethodKeepBest) {
        if (leaderboards[hSteamLeaderboard - 1].sort_method == k_ELeaderboardSortMethodAscending
            ? leaderboards[hSteamLeaderboard - 1].self_score.score >= score.score
            : leaderboards[hSteamLeaderboard - 1].self_score.score <= score.score) {
            leaderboards[hSteamLeaderboard - 1].self_score = score;
            changed = true;
        }
    } else {
        if (leaderboards[hSteamLeaderboard - 1].self_score.score != score.score) changed = true;
        leaderboards[hSteamLeaderboard - 1].self_score = score;
    }

    if (changed) {
        save_leaderboard_score(&(leaderboards[hSteamLeaderboard - 1]));
    }

    LeaderboardScoreUploaded_t data;
    data.m_bSuccess = 1; //needs to be success or DOA6 freezes when uploading score.
    //data.m_bSuccess = 0;
    data.m_hSteamLeaderboard = hSteamLeaderboard;
    data.m_nScore = nScore;
    data.m_bScoreChanged = changed;
    data.m_nGlobalRankNew = 1;
    data.m_nGlobalRankPrevious = 0;
    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}

SteamAPICall_t UploadLeaderboardScore( SteamLeaderboard_t hSteamLeaderboard, int32 nScore, int32 *pScoreDetails, int cScoreDetailsCount )
{
	PRINT_DEBUG("UploadLeaderboardScore old\n");
	return UploadLeaderboardScore(hSteamLeaderboard, k_ELeaderboardUploadScoreMethodKeepBest, nScore, pScoreDetails, cScoreDetailsCount);
}


// Attaches a piece of user generated content the user's entry on a leaderboard.
// hContent is a handle to a piece of user generated content that was shared using ISteamUserRemoteStorage::FileShare().
// This call is asynchronous, with the result returned in LeaderboardUGCSet_t.
STEAM_CALL_RESULT( LeaderboardUGCSet_t )
SteamAPICall_t AttachLeaderboardUGC( SteamLeaderboard_t hSteamLeaderboard, UGCHandle_t hUGC )
{
    PRINT_DEBUG("AttachLeaderboardUGC\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    LeaderboardUGCSet_t data = {};
    if (hSteamLeaderboard > leaderboards.size() || hSteamLeaderboard <= 0) {
        data.m_eResult = k_EResultFail;
    } else {
        data.m_eResult = k_EResultOK;
    }

    data.m_hSteamLeaderboard = hSteamLeaderboard;
    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}


// Retrieves the number of players currently playing your game (online + offline)
// This call is asynchronous, with the result returned in NumberOfCurrentPlayers_t
STEAM_CALL_RESULT( NumberOfCurrentPlayers_t )
SteamAPICall_t GetNumberOfCurrentPlayers()
{
    PRINT_DEBUG("GetNumberOfCurrentPlayers\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    NumberOfCurrentPlayers_t data;
    data.m_bSuccess = 1;
    data.m_cPlayers = 69;
    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}


// Requests that Steam fetch data on the percentage of players who have received each achievement
// for the game globally.
// This call is asynchronous, with the result returned in GlobalAchievementPercentagesReady_t.
STEAM_CALL_RESULT( GlobalAchievementPercentagesReady_t )
SteamAPICall_t RequestGlobalAchievementPercentages()
{
    PRINT_DEBUG("RequestGlobalAchievementPercentages\n");
    return 0;
}


// Get the info on the most achieved achievement for the game, returns an iterator index you can use to fetch
// the next most achieved afterwards.  Will return -1 if there is no data on achievement 
// percentages (ie, you haven't called RequestGlobalAchievementPercentages and waited on the callback).
int GetMostAchievedAchievementInfo( char *pchName, uint32 unNameBufLen, float *pflPercent, bool *pbAchieved )
{
    PRINT_DEBUG("GetMostAchievedAchievementInfo\n");
    return -1;
}


// Get the info on the next most achieved achievement for the game. Call this after GetMostAchievedAchievementInfo or another
// GetNextMostAchievedAchievementInfo call passing the iterator from the previous call. Returns -1 after the last
// achievement has been iterated.
int GetNextMostAchievedAchievementInfo( int iIteratorPrevious, char *pchName, uint32 unNameBufLen, float *pflPercent, bool *pbAchieved )
{
    PRINT_DEBUG("GetNextMostAchievedAchievementInfo\n");
    return -1;
}


// Returns the percentage of users who have achieved the specified achievement.
bool GetAchievementAchievedPercent( const char *pchName, float *pflPercent )
{
    PRINT_DEBUG("GetAchievementAchievedPercent\n");
    return false;
}


// Requests global stats data, which is available for stats marked as "aggregated".
// This call is asynchronous, with the results returned in GlobalStatsReceived_t.
// nHistoryDays specifies how many days of day-by-day history to retrieve in addition
// to the overall totals. The limit is 60.
STEAM_CALL_RESULT( GlobalStatsReceived_t )
SteamAPICall_t RequestGlobalStats( int nHistoryDays )
{
    PRINT_DEBUG("RequestGlobalStats %i\n", nHistoryDays);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    GlobalStatsReceived_t data;
    data.m_nGameID = settings->get_local_game_id().ToUint64();
    data.m_eResult = k_EResultOK;
    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}


// Gets the lifetime totals for an aggregated stat
bool GetGlobalStat( const char *pchStatName, int64 *pData )
{
    PRINT_DEBUG("GetGlobalStat %s\n", pchStatName);
    return false;
}

bool GetGlobalStat( const char *pchStatName, double *pData )
{
    PRINT_DEBUG("GetGlobalStat %s\n", pchStatName);
    return false;
}


// Gets history for an aggregated stat. pData will be filled with daily values, starting with today.
// So when called, pData[0] will be today, pData[1] will be yesterday, and pData[2] will be two days ago, 
// etc. cubData is the size in bytes of the pubData buffer. Returns the number of 
// elements actually set.
int32 GetGlobalStatHistory( const char *pchStatName, STEAM_ARRAY_COUNT(cubData) int64 *pData, uint32 cubData )
{
    PRINT_DEBUG("GetGlobalStatHistory int64 %s\n", pchStatName);
    return 0;
}

int32 GetGlobalStatHistory( const char *pchStatName, STEAM_ARRAY_COUNT(cubData) double *pData, uint32 cubData )
{
    PRINT_DEBUG("GetGlobalStatHistory double %s\n", pchStatName);
    return 0;
}

// For achievements that have related Progress stats, use this to query what the bounds of that progress are.
// You may want this info to selectively call IndicateAchievementProgress when appropriate milestones of progress
// have been made, to show a progress notification to the user.
bool GetAchievementProgressLimits( const char *pchName, int32 *pnMinProgress, int32 *pnMaxProgress )
{
    PRINT_DEBUG("GetAchievementProgressLimits int\n");
    return false;
}

bool GetAchievementProgressLimits( const char *pchName, float *pfMinProgress, float *pfMaxProgress )
{
    PRINT_DEBUG("GetAchievementProgressLimits float\n");
    return false;
}


};

#endif//__INCLUDED_STEAM_USER_STATS_H__
