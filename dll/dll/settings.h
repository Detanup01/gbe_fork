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

#ifndef SETTINGS_INCLUDE
#define SETTINGS_INCLUDE

#include "base.h"

struct IP_PORT;

struct DLC_entry {
    AppId_t appID;
    std::string name;
    bool available;
};

struct Mod_entry {
    PublishedFileId_t id;
    std::string title;
    std::string path;

    std::string previewURL;
    EWorkshopFileType fileType;
    std::string description;
    uint64 steamIDOwner;
    uint32 timeCreated;
    uint32 timeUpdated;
    uint32 timeAddedToUserList;
    ERemoteStoragePublishedFileVisibility visibility;
    bool banned;
    bool acceptedForUse;
    bool tagsTruncated;
    std::string tags; 
    // file/url information
    UGCHandle_t handleFile = generate_file_handle();
    UGCHandle_t handlePreviewFile = generate_file_handle();
    std::string primaryFileName;
    int32 primaryFileSize;
    std::string previewFileName;
    int32 previewFileSize;
    std::string workshopItemURL;
    // voting information
    uint32 votesUp;
    uint32 votesDown;
    float score;
    // collection details
    uint32 numChildren;
    
private:
    UGCHandle_t generate_file_handle()
    {
        static UGCHandle_t val = 0;

        ++val;
        if (val == 0 || val == k_UGCHandleInvalid) val = 1;
        
        return val;
    }

};

struct Leaderboard_config {
    enum ELeaderboardSortMethod sort_method;
    enum ELeaderboardDisplayType display_type;
};

enum Stat_Type {
    STAT_TYPE_INT,
    STAT_TYPE_FLOAT,
    STAT_TYPE_AVGRATE
};

struct Stat_config {
    enum Stat_Type type;
    union {
        float default_value_float;
        uint32 default_value_int;
    };
};

struct Image_Data {
    uint32 width;
    uint32 height;
    std::string data;
};

struct Controller_Settings {
    std::map<std::string, std::map<std::string, std::pair<std::set<std::string>, std::string>>> action_sets;
    std::map<std::string, std::string> action_set_layer_parents;
    std::map<std::string, std::map<std::string, std::pair<std::set<std::string>, std::string>>> action_set_layers;
};

struct Group_Clans {
    CSteamID id;
    std::string name;
    std::string tag;
};

struct Overlay_Appearance {
    enum NotificationPosition {
        top_left, top_center, top_right,
        bot_left, bot_center, bot_right,
    };

    constexpr const static NotificationPosition default_pos = NotificationPosition::top_right;

    float font_size = 16.0;
    float icon_size = 64.0;
    float notification_r = 0.16;
    float notification_g = 0.29;
    float notification_b = 0.48;
    float notification_a = 1.0;
    float background_r = -1.0;
    float background_g = -1.0;
    float background_b = -1.0;
    float background_a = -1.0;
    float element_r = -1.0;
    float element_g = -1.0;
    float element_b = -1.0;
    float element_a = -1.0;
    float element_hovered_r = -1.0;
    float element_hovered_g = -1.0;
    float element_hovered_b = -1.0;
    float element_hovered_a = -1.0;
    float element_active_r = -1.0;
    float element_active_g = -1.0;
    float element_active_b = -1.0;
    float element_active_a = -1.0;

    NotificationPosition ach_earned_pos = default_pos; // achievement earned
    NotificationPosition invite_pos = default_pos; // lobby/game invitation
    NotificationPosition chat_msg_pos = default_pos; // chat message from a friend

    static NotificationPosition translate_notification_position(const std::string &str)
    {
        if (str == "top_left") return NotificationPosition::top_left;
        else if (str == "top_center") return NotificationPosition::top_center;
        else if (str == "top_right") return NotificationPosition::top_right;
        else if (str == "bot_left") return NotificationPosition::bot_left;
        else if (str == "bot_center") return NotificationPosition::bot_center;
        else if (str == "bot_right") return NotificationPosition::bot_right;

        PRINT_DEBUG("Invalid position '%s'\n", str.c_str());
        return default_pos;
    }
};

class Settings {
    CSteamID steam_id;
    CGameID game_id;
    std::string name, language;
    CSteamID lobby_id;

    bool unlockAllDLCs;
    bool offline;
    std::vector<struct DLC_entry> DLCs;
    std::vector<struct Mod_entry> mods;
    std::map<AppId_t, std::string> app_paths;
    std::map<std::string, Leaderboard_config> leaderboards;
    std::map<std::string, Stat_config> stats;
    bool create_unknown_leaderboards;
    uint16 port;

    // whether to auto accept any overlay invites
    bool auto_accept_any_overlay_invites = false;
    // list of user steam IDs to auto-accept invites from
    std::set<uint64_t> auto_accept_overlay_invites_friends{};

public:

#ifdef LOBBY_CONNECT
    static const bool is_lobby_connect = true;
#else
    static const bool is_lobby_connect = false;
#endif

    static std::string sanitize(std::string name);
    Settings(CSteamID steam_id, CGameID game_id, std::string name, std::string language, bool offline);
    CSteamID get_local_steam_id();
    CGameID get_local_game_id();
    const char *get_local_name();
    void set_local_name(char *name);
    const char *get_language();
    void set_language(char *language);

    void set_game_id(CGameID game_id);
    void set_lobby(CSteamID lobby_id);
    CSteamID get_lobby();
    bool is_offline() {return offline; }
    uint16 get_port() {return port;}
    void set_port(uint16 port) { this->port = port;}

    //DLC stuff
    void unlockAllDLC(bool value);
    void addDLC(AppId_t appID, std::string name, bool available);
    unsigned int DLCCount();
    bool hasDLC(AppId_t appID);
    bool getDLC(unsigned int index, AppId_t &appID, bool &available, std::string &name);
    bool allDLCUnlocked() const;

    //Depots
    std::vector<DepotId_t> depots;

    //App Install paths
    void setAppInstallPath(AppId_t appID, std::string path);
    std::string getAppInstallPath(AppId_t appID);

    //mod stuff
    void addMod(PublishedFileId_t id, std::string title, std::string path);
    void addModDetails(PublishedFileId_t id, Mod_entry details);
    Mod_entry getMod(PublishedFileId_t id);
    bool isModInstalled(PublishedFileId_t id);
    std::set<PublishedFileId_t> modSet();

    //leaderboards
    void setLeaderboard(std::string leaderboard, enum ELeaderboardSortMethod sort_method, enum ELeaderboardDisplayType display_type);
    std::map<std::string, Leaderboard_config> getLeaderboards() { return leaderboards; }
    void setCreateUnknownLeaderboards(bool enable) {create_unknown_leaderboards = enable;}
    bool createUnknownLeaderboards() { return create_unknown_leaderboards; }

    //custom broadcasts
    std::set<IP_PORT> custom_broadcasts;

    //stats
    std::map<std::string, Stat_config> getStats() { return stats; }
    void setStatDefiniton(std::string name, struct Stat_config stat_config) {stats[ascii_to_lowercase(name)] = stat_config; }
    // bypass to make SetAchievement() always return true, prevent some games from breaking
    bool achievement_bypass = false;

    //subscribed lobby/group ids
    std::set<uint64> subscribed_groups;
    std::vector<Group_Clans> subscribed_groups_clans;

    //images
    std::map<int, struct Image_Data> images;
    int add_image(std::string data, uint32 width, uint32 height);
    bool disable_account_avatar = false;

    //installed app ids, Steam_Apps::BIsAppInstalled()
    std::set<AppId_t> installed_app_ids;
    bool assume_any_app_installed = true;
    bool appIsInstalled(AppId_t appID);

    //is playing on beta branch + current/forced branch name
    bool is_beta_branch = false;
    std::string current_branch_name = "public";

    //controller
    struct Controller_Settings controller_settings;
    std::string glyphs_directory;

    //networking
    bool disable_networking = false;

    //gameserver source query
    bool disable_source_query = false;

    //overlay
    bool disable_overlay = false;
    bool disable_overlay_achievement_notification = false;
    bool disable_overlay_friend_notification = false;
    //warn people who use force_ settings
    bool overlay_warn_forced_setting = false;
    //warn people who use local save
    bool overlay_warn_local_save = false;
    //disable overlay warning for force_ settings
    bool disable_overlay_warning_forced_setting = false;
    //disable overlay warning for local save
    bool disable_overlay_warning_local_save = false;
    //disable overlay warning for bad app ID (= 0)
    bool disable_overlay_warning_bad_appid = false;
    // disable all overlay warnings
    bool disable_overlay_warning_any = false;
    Overlay_Appearance overlay_appearance{};

    //app build id
    int build_id = 10;

    //supported languages
    std::set<std::string> supported_languages;

    //make lobby creation fail in the matchmaking interface
    bool disable_lobby_creation = false;

    // path to local save
    std::string local_save;

    //steamhttp external download support
    bool http_online = false;

    //steam deck flag
    bool steam_deck = false;
    
    // use new app_ticket auth instead of old one
    bool enable_new_app_ticket = false;
    // can use GC token for generation
    bool use_gc_token = false;

    // overlay auto accept stuff
    void acceptAnyOverlayInvites(bool value);
    void addFriendToOverlayAutoAccept(uint64_t friend_id);
    bool hasOverlayAutoAcceptInviteFromFriend(uint64_t friend_id) const;
    size_t overlayAutoAcceptInvitesCount() const;

    // get the alpha-2 code from: https://www.iban.com/country-codes 
    std::string ip_country = "US";
};

#endif
