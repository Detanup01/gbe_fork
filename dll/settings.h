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
    UGCHandle_t handleFile = k_UGCHandleInvalid;
    UGCHandle_t handlePreviewFile = k_UGCHandleInvalid;
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

    //subscribed lobby/group ids
    std::set<uint64> subscribed_groups;
    std::vector<Group_Clans> subscribed_groups_clans;

    //images
    std::map<int, struct Image_Data> images;
    int add_image(std::string data, uint32 width, uint32 height);
    bool disable_account_avatar = false;

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

    //app build id
    int build_id = 10;

    //supported languages
    std::set<std::string> supported_languages;

    //make lobby creation fail in the matchmaking interface
    bool disable_lobby_creation = false;

    //warn people who use force_ settings
    bool warn_forced = false;

    //warn people who use local save
    bool warn_local_save = false;

    //steamhttp external download support
    bool http_online = false;
};

#endif
