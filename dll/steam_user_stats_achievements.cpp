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

int Steam_User_Stats::load_ach_icon(nlohmann::json &defined_ach, bool achieved)
{
    const char *icon_handle_key = achieved ? "icon_handle" : "icon_gray_handle";
    int current_handle = defined_ach.value(icon_handle_key, Settings::UNLOADED_IMAGE_HANDLE);
    if (Settings::UNLOADED_IMAGE_HANDLE != current_handle) { // already loaded
        return current_handle;
    }

    const char *icon_key = achieved ? "icon" : "icon_gray";
    if (!achieved && !defined_ach.contains(icon_key)) {
        icon_key = "icongray"; // old format
    }

    std::string icon_filepath = defined_ach.value(icon_key, std::string{});
    if (icon_filepath.empty()) {
        defined_ach[icon_handle_key] = Settings::INVALID_IMAGE_HANDLE;
        return Settings::INVALID_IMAGE_HANDLE;
    }

    std::string file_path(Local_Storage::get_game_settings_path() + icon_filepath);
    unsigned int file_size = file_size_(file_path);
    if (!file_size)
    {
        file_path = (Local_Storage::get_game_settings_path() + "achievement_images" + PATH_SEPARATOR + icon_filepath);
        file_size = file_size_(file_path);
        if (!file_size)
        {
            defined_ach[icon_handle_key] = Settings::INVALID_IMAGE_HANDLE;
            return Settings::INVALID_IMAGE_HANDLE;
        }
    }

    int icon_size = static_cast<int>(settings->overlay_appearance.icon_size);
    std::string img(Local_Storage::load_image_resized(file_path, "", icon_size));
    if (img.empty()) {
        defined_ach[icon_handle_key] = Settings::INVALID_IMAGE_HANDLE;
        return Settings::INVALID_IMAGE_HANDLE;
    }

    int handle = settings->add_image(img, icon_size, icon_size);
    defined_ach[icon_handle_key] = handle;
    return handle;
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



// change achievements without sending back to server
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
        trig.m_nCurProgress = 0;
        trig.m_nGameID = settings->get_local_game_id().ToUint64();
        trig.m_nMaxProgress = 0;
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
    trig.m_nCurProgress = 0;
    trig.m_nGameID = settings->get_local_game_id().ToUint64();
    trig.m_nMaxProgress = 0;
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


// Achievement / GroupAchievement metadata

// Gets the icon of the achievement, which is a handle to be used in ISteamUtils::GetImageRGBA(), or 0 if none set. 
// A return value of 0 may indicate we are still fetching data, and you can wait for the UserAchievementIconFetched_t callback
// which will notify you when the bits are ready. If the callback still returns zero, then there is no image set for the
// specified achievement.
int Steam_User_Stats::GetAchievementIcon( const char *pchName )
{
    PRINT_DEBUG("'%s'", pchName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!pchName) return Settings::INVALID_IMAGE_HANDLE;

    bool achieved = false;
    GetAchievement(pchName, &achieved);

    std::string ach_name(pchName);
    // here we force load in case the game has a lot of achievements, because otherwise some games might timeout
    // this somewhat defeats the purpose of background loading but a timeout is worse
    int handle = get_achievement_icon_handle(ach_name, achieved, true);
    if (Settings::UNLOADED_IMAGE_HANDLE == handle) { // if the background callback didn't get a chance to load this one yet
        handle = Settings::INVALID_IMAGE_HANDLE;
    }

    UserAchievementIconFetched_t data{};
    data.m_bAchieved = achieved ;
    data.m_nGameID = settings->get_local_game_id();
    data.m_nIconHandle = handle;
    ach_name.copy(data.m_rgchAchievementName, sizeof(data.m_rgchAchievementName));

    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return handle;
}

int Steam_User_Stats::get_achievement_icon_handle( const std::string &ach_name, bool achieved, bool force_load )
{
    PRINT_DEBUG("'%s', achieved=%i, force=%i", ach_name.c_str(), (int)achieved, (int)force_load);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    nlohmann::detail::iter_impl<nlohmann::json> it = defined_achievements.end();
    try {
        it = defined_achievements_find(ach_name);
    } catch(...) { }
    if (defined_achievements.end() == it) return Settings::INVALID_IMAGE_HANDLE;

    int handle = Settings::INVALID_IMAGE_HANDLE;
    if (settings->paginated_achievements_icons < 0) { // disabled functionality
        handle = Settings::INVALID_IMAGE_HANDLE;
    } else if (settings->paginated_achievements_icons == 0) { // load the icon only when requested
        handle = load_ach_icon(*it, achieved);
    } else { // depend on the periodic callback to load the icon
        if (force_load) {
            handle = load_ach_icon(*it, achieved);
        } else {
            const char *icon_handle_key = achieved ? "icon_handle" : "icon_gray_handle";
            handle = it->value(icon_handle_key, Settings::UNLOADED_IMAGE_HANDLE);
        }
    }
    
    PRINT_DEBUG("returned handle = %i", handle);
    return handle;
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


// Friends achievements

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

void Steam_User_Stats::load_achievements_icons()
{
    if (last_loaded_ach_icon >= defined_achievements.size() || settings->paginated_achievements_icons <= 0) return;

#ifndef EMU_RELEASE_BUILD
    auto now1 = std::chrono::high_resolution_clock::now();
#endif

    int idx = 0;
    for (;
        idx < settings->paginated_achievements_icons && last_loaded_ach_icon < defined_achievements.size();
        ++idx, ++last_loaded_ach_icon) {
        auto &ach = defined_achievements.at(last_loaded_ach_icon);
        load_ach_icon(ach, true);
        load_ach_icon(ach, false);
    }

#ifndef EMU_RELEASE_BUILD
    auto now2 = std::chrono::high_resolution_clock::now();
    auto dd = (unsigned)std::chrono::duration_cast<std::chrono::milliseconds>(now2 - now1).count();
    PRINT_DEBUG("attempted to load %d achievements icons in %u ms", idx * 2, dd);
#endif

}
