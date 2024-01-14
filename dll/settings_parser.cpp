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

#include "dll/settings_parser.h"

static void load_custom_broadcasts(std::string broadcasts_filepath, std::set<IP_PORT> &custom_broadcasts)
{
    PRINT_DEBUG("Broadcasts file path: %s\n", broadcasts_filepath.c_str());
    std::ifstream broadcasts_file(utf8_decode(broadcasts_filepath));
    consume_bom(broadcasts_file);
    if (broadcasts_file.is_open()) {
        std::string line;
        while (std::getline(broadcasts_file, line)) {
            std::set<IP_PORT> ips = Networking::resolve_ip(line);
            custom_broadcasts.insert(ips.begin(), ips.end());
        }
    }
}

static void load_subscribed_groups_clans(std::string clans_filepath, Settings *settings_client, Settings *settings_server)
{
    PRINT_DEBUG("Group clans file path: %s\n", clans_filepath.c_str());
    std::ifstream clans_file(utf8_decode(clans_filepath));
    consume_bom(clans_file);
    if (clans_file.is_open()) {
        std::string line;
        while (std::getline(clans_file, line)) {
            if (line.length() < 0) continue;

            std::size_t seperator1 = line.find("\t\t");
            std::size_t seperator2 = line.rfind("\t\t");
            std::string clan_id;
            std::string clan_name;
            std::string clan_tag;
            if ((seperator1 != std::string::npos) && (seperator2 != std::string::npos)) {
                clan_id = line.substr(0, seperator1);
                clan_name = line.substr(seperator1+2, seperator2-2);
                clan_tag = line.substr(seperator2+2);

                // fix persistant tabbing problem for clan name
                std::size_t seperator3 = clan_name.find("\t");
                std::string clan_name_fix = clan_name.substr(0, seperator3);
                clan_name = clan_name_fix;
            }

            Group_Clans nclan;
            nclan.id = CSteamID( std::stoull(clan_id.c_str(), NULL, 0) );
            nclan.name = clan_name;
            nclan.tag = clan_tag;

            try {
                settings_client->subscribed_groups_clans.push_back(nclan);
                settings_server->subscribed_groups_clans.push_back(nclan);
                PRINT_DEBUG("Added clan %s\n", clan_name.c_str());
            } catch (...) {}
        }
    }
}

static void load_overlay_appearance(std::string appearance_filepath, Settings *settings_client, Settings *settings_server)
{
    PRINT_DEBUG("Overlay appearance file path: %s\n", appearance_filepath.c_str());
    std::ifstream appearance_file(utf8_decode(appearance_filepath));
    consume_bom(appearance_file);
    if (appearance_file.is_open()) {
        std::string line;
        while (std::getline(appearance_file, line)) {
            if (line.length() < 0) continue;

            std::size_t seperator = line.find(" ");
            std::string name;
            std::string value;
            if (seperator != std::string::npos) {
                name = line.substr(0, seperator);
                value = line.substr(seperator);
            }

            try {
                if (name.compare("Font_Size") == 0) {
                    float nfont_size = std::stof(value, NULL);
                    settings_client->overlay_appearance.font_size = nfont_size;
                    settings_server->overlay_appearance.font_size = nfont_size;
                } else if (name.compare("Icon_Size") == 0) {
                    float nicon_size = std::stof(value, NULL);
                    settings_client->overlay_appearance.icon_size = nicon_size;
                    settings_server->overlay_appearance.icon_size = nicon_size;
                } else if (name.compare("Notification_R") == 0) {
                    float nnotification_r = std::stof(value, NULL);
                    settings_client->overlay_appearance.notification_r = nnotification_r;
                    settings_server->overlay_appearance.notification_r = nnotification_r;
                } else if (name.compare("Notification_G") == 0) {
                    float nnotification_g = std::stof(value, NULL);
                    settings_client->overlay_appearance.notification_g = nnotification_g;
                    settings_server->overlay_appearance.notification_g = nnotification_g;
                } else if (name.compare("Notification_B") == 0) {
                    float nnotification_b = std::stof(value, NULL);
                    settings_client->overlay_appearance.notification_b = nnotification_b;
                    settings_server->overlay_appearance.notification_b = nnotification_b;
                } else if (name.compare("Notification_A") == 0) {
                    float nnotification_a = std::stof(value, NULL);
                    settings_client->overlay_appearance.notification_a = nnotification_a;
                    settings_server->overlay_appearance.notification_a = nnotification_a;
                } else if (name.compare("Background_R") == 0) {
                    float nbackground_r = std::stof(value, NULL);
                    settings_client->overlay_appearance.background_r = nbackground_r;
                    settings_server->overlay_appearance.background_r = nbackground_r;
                } else if (name.compare("Background_G") == 0) {
                    float nbackground_g = std::stof(value, NULL);
                    settings_client->overlay_appearance.background_g = nbackground_g;
                    settings_server->overlay_appearance.background_g = nbackground_g;
                } else if (name.compare("Background_B") == 0) {
                    float nbackground_b = std::stof(value, NULL);
                    settings_client->overlay_appearance.background_b = nbackground_b;
                    settings_server->overlay_appearance.background_b = nbackground_b;
                } else if (name.compare("Background_A") == 0) {
                    float nbackground_a = std::stof(value, NULL);
                    settings_client->overlay_appearance.background_a = nbackground_a;
                    settings_server->overlay_appearance.background_a = nbackground_a;
                } else if (name.compare("Element_R") == 0) {
                    float nelement_r = std::stof(value, NULL);
                    settings_client->overlay_appearance.element_r = nelement_r;
                    settings_server->overlay_appearance.element_r = nelement_r;
                } else if (name.compare("Element_G") == 0) {
                    float nelement_g = std::stof(value, NULL);
                    settings_client->overlay_appearance.element_g = nelement_g;
                    settings_server->overlay_appearance.element_g = nelement_g;
                } else if (name.compare("Element_B") == 0) {
                    float nelement_b = std::stof(value, NULL);
                    settings_client->overlay_appearance.element_b = nelement_b;
                    settings_server->overlay_appearance.element_b = nelement_b;
                } else if (name.compare("Element_A") == 0) {
                    float nelement_a = std::stof(value, NULL);
                    settings_client->overlay_appearance.element_a = nelement_a;
                    settings_server->overlay_appearance.element_a = nelement_a;
                } else if (name.compare("ElementHovered_R") == 0) {
                    float nelement_hovered_r = std::stof(value, NULL);
                    settings_client->overlay_appearance.element_hovered_r = nelement_hovered_r;
                    settings_server->overlay_appearance.element_hovered_r = nelement_hovered_r;
                } else if (name.compare("ElementHovered_G") == 0) {
                    float nelement_hovered_g = std::stof(value, NULL);
                    settings_client->overlay_appearance.element_hovered_g = nelement_hovered_g;
                    settings_server->overlay_appearance.element_hovered_g = nelement_hovered_g;
                } else if (name.compare("ElementHovered_B") == 0) {
                    float nelement_hovered_b = std::stof(value, NULL);
                    settings_client->overlay_appearance.element_hovered_b = nelement_hovered_b;
                    settings_server->overlay_appearance.element_hovered_b = nelement_hovered_b;
                } else if (name.compare("ElementHovered_A") == 0) {
                    float nelement_hovered_a = std::stof(value, NULL);
                    settings_client->overlay_appearance.element_hovered_a = nelement_hovered_a;
                    settings_server->overlay_appearance.element_hovered_a = nelement_hovered_a;
                } else if (name.compare("ElementActive_R") == 0) {
                    float nelement_active_r = std::stof(value, NULL);
                    settings_client->overlay_appearance.element_active_r = nelement_active_r;
                    settings_server->overlay_appearance.element_active_r = nelement_active_r;
                } else if (name.compare("ElementActive_G") == 0) {
                    float nelement_active_g = std::stof(value, NULL);
                    settings_client->overlay_appearance.element_active_g = nelement_active_g;
                    settings_server->overlay_appearance.element_active_g = nelement_active_g;
                } else if (name.compare("ElementActive_B") == 0) {
                    float nelement_active_b = std::stof(value, NULL);
                    settings_client->overlay_appearance.element_active_b = nelement_active_b;
                    settings_server->overlay_appearance.element_active_b = nelement_active_b;
                } else if (name.compare("ElementActive_A") == 0) {
                    float nelement_active_a = std::stof(value, NULL);
                    settings_client->overlay_appearance.element_active_a = nelement_active_a;
                    settings_server->overlay_appearance.element_active_a = nelement_active_a;
                }
                PRINT_DEBUG("Overlay appearance %s %s\n", name.c_str(), value.c_str());
            } catch (...) {}
        }
    }
}

template<typename Out>
static void split_string(const std::string &s, char delim, Out result) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

static void load_gamecontroller_settings(Settings *settings)
{
    std::string path = Local_Storage::get_game_settings_path() + "controller";
    std::vector<std::string> paths = Local_Storage::get_filenames_path(path);

    for (auto & p: paths) {
        size_t length = p.length();
        if (length < 4) continue;
        if ( std::toupper(p.back()) != 'T') continue;
        if ( std::toupper(p[length - 2]) != 'X') continue;
        if ( std::toupper(p[length - 3]) != 'T') continue;
        if (p[length - 4] != '.') continue;

        PRINT_DEBUG("controller config %s\n", p.c_str());
        std::string action_set_name = p.substr(0, length - 4);
        std::transform(action_set_name.begin(), action_set_name.end(), action_set_name.begin(),[](unsigned char c){ return std::toupper(c); });

        std::string controller_config_path = path + PATH_SEPARATOR + p;
        std::ifstream input( utf8_decode(controller_config_path) );
        if (input.is_open()) {
            consume_bom(input);
            std::map<std::string, std::pair<std::set<std::string>, std::string>> button_pairs;

            for( std::string line; getline( input, line ); ) {
                if (!line.empty() && line[line.length()-1] == '\n') {
                    line.pop_back();
                }

                if (!line.empty() && line[line.length()-1] == '\r') {
                    line.pop_back();
                }

                std::string action_name;
                std::string button_name;
                std::string source_mode;

                std::size_t deliminator = line.find("=");
                if (deliminator != 0 && deliminator != std::string::npos && deliminator != line.size()) {
                    action_name = line.substr(0, deliminator);
                    std::size_t deliminator2 = line.find("=", deliminator + 1);

                    if (deliminator2 != std::string::npos && deliminator2 != line.size()) {
                        button_name = line.substr(deliminator + 1, deliminator2 - (deliminator + 1));
                        source_mode = line.substr(deliminator2 + 1);
                    } else {
                        button_name = line.substr(deliminator + 1);
                        source_mode = "";
                    }
                }

                std::transform(action_name.begin(), action_name.end(), action_name.begin(),[](unsigned char c){ return std::toupper(c); });
                std::transform(button_name.begin(), button_name.end(), button_name.begin(),[](unsigned char c){ return std::toupper(c); });
                std::pair<std::set<std::string>, std::string> button_config = {{}, source_mode};
                split_string(button_name, ',', std::inserter(button_config.first, button_config.first.begin()));
                button_pairs[action_name] = button_config;
                PRINT_DEBUG("Added %s %s %s\n", action_name.c_str(), button_name.c_str(), source_mode.c_str());
            }

            settings->controller_settings.action_sets[action_set_name] = button_pairs;
            PRINT_DEBUG("Added %zu action names to %s\n", button_pairs.size(), action_set_name.c_str());
        }
    }

    settings->glyphs_directory = path + (PATH_SEPARATOR "glyphs" PATH_SEPARATOR);
}

// steam_appid.txt
uint32 parse_steam_app_id(std::string &program_path)
{
    char array[10] = {};
    array[0] = '0';
    Local_Storage::get_file_data(Local_Storage::get_game_settings_path() + "steam_appid.txt", array, sizeof(array) - 1);
    uint32 appid = 0;
    try {
        appid = std::stoi(array);
    } catch (...) {}
    if (!appid) {
        memset(array, 0, sizeof(array));
        array[0] = '0';
        Local_Storage::get_file_data("steam_appid.txt", array, sizeof(array) - 1);
        try {
            appid = std::stoi(array);
        } catch (...) {}
        if (!appid) {
            memset(array, 0, sizeof(array));
            array[0] = '0';
            Local_Storage::get_file_data(program_path + "steam_appid.txt", array, sizeof(array) - 1);
            try {
                appid = std::stoi(array);
            } catch (...) {}
        }
    }

    if (!appid) {
        std::string str_appid = get_env_variable("SteamAppId");
        std::string str_gameid = get_env_variable("SteamGameId");
        std::string str_overlay_gameid = get_env_variable("SteamOverlayGameId");
        
        PRINT_DEBUG("str_appid %s str_gameid: %s str_overlay_gameid: %s\n", str_appid.c_str(), str_gameid.c_str(), str_overlay_gameid.c_str());
        uint32 appid_env = 0;
        uint32 gameid_env = 0;
        uint32 overlay_gameid = 0;

        if (str_appid.size() > 0) {
            try {
                appid_env = std::stoul(str_appid);
            } catch (...) {
                appid_env = 0;
            }
        }

        if (str_gameid.size() > 0) {
            try {
                gameid_env = std::stoul(str_gameid);
            } catch (...) {
                gameid_env = 0;
            }
        }

        if (str_overlay_gameid.size() > 0) {
            try {
                overlay_gameid = std::stoul(str_overlay_gameid);
            } catch (...) {
                overlay_gameid = 0;
            }
        }

        PRINT_DEBUG("appid_env %u gameid_env: %u overlay_gameid: %u\n", appid_env, gameid_env, overlay_gameid);
        if (appid_env) {
            appid = appid_env;
        }

        if (gameid_env) {
            appid = gameid_env;
        }

        if (overlay_gameid) {
            appid = overlay_gameid;
        }
    }

    return appid;
}

// local_save.txt
bool parse_local_save(std::string &program_path, std::string &save_path)
{
    bool local_save = false;

    char array[256] = {};
    if (Local_Storage::get_file_data(program_path + "local_save.txt", array, sizeof(array) - 1) != -1) {
        save_path = program_path + Settings::sanitize(array);
        local_save = true;
    }
    return local_save;
}

// listen_port.txt
uint16 parse_listen_port(class Local_Storage *local_storage)
{
    char array_port[10] = {};
    array_port[0] = '0';
    local_storage->get_data_settings("listen_port.txt", array_port, sizeof(array_port) - 1);
    uint16 port = std::stoi(array_port);
    if (port == 0) {
        port = DEFAULT_PORT;
        snprintf(array_port, sizeof(array_port), "%hu", port);
        local_storage->store_data_settings("listen_port.txt", array_port, strlen(array_port));
    }
    return port;
}

// account_name.txt
std::string parse_account_name(class Local_Storage *local_storage)
{
    char name[100] = {};
    if (local_storage->get_data_settings("account_name.txt", name, sizeof(name) - 1) <= 0) {
        strcpy(name, DEFAULT_NAME);
        local_storage->store_data_settings("account_name.txt", name, strlen(name));
    }
    return std::string(name);
}

// user_steam_id.txt
CSteamID parse_user_steam_id(class Local_Storage *local_storage)
{
    char array_steam_id[32] = {};
    CSteamID user_id;
    uint64 steam_id = 0;
    bool generate_new = false;
    //try to load steam id from game specific settings folder first
    if (local_storage->get_data(Local_Storage::settings_storage_folder, "user_steam_id.txt", array_steam_id, sizeof(array_steam_id) - 1) > 0) {
        user_id = CSteamID((uint64)std::atoll(array_steam_id));
        if (!user_id.IsValid()) {
            generate_new = true;
        }
    } else {
        generate_new = true;
    }

    if (generate_new) {
        generate_new = false;
        if (local_storage->get_data_settings("user_steam_id.txt", array_steam_id, sizeof(array_steam_id) - 1) > 0) {
            user_id = CSteamID((uint64)std::atoll(array_steam_id));
            if (!user_id.IsValid()) {
                generate_new = true;
            }
        } else {
            generate_new = true;
        }
    }

    if (generate_new) {
        user_id = generate_steam_id_user();
        uint64 steam_id = user_id.ConvertToUint64();
        char temp_text[32] = {};
        snprintf(temp_text, sizeof(temp_text), "%llu", steam_id);
        local_storage->store_data_settings("user_steam_id.txt", temp_text, strlen(temp_text));
    }

    return user_id;
}

// language.txt
std::string parse_current_language(class Local_Storage *local_storage)
{
    char language[64] = {};
    if (local_storage->get_data_settings("language.txt", language, sizeof(language) - 1) <= 0) {
        strcpy(language, DEFAULT_LANGUAGE);
        local_storage->store_data_settings("language.txt", language, strlen(language));
    }

    return std::string(language);
}

// supported_languages.txt
std::set<std::string> parse_supported_languages(class Local_Storage *local_storage, std::string &language)
{
    std::set<std::string> supported_languages;

    std::string lang_config_path = Local_Storage::get_game_settings_path() + "supported_languages.txt";
    std::ifstream input( utf8_decode(lang_config_path) );

    std::string first_language;
    if (input.is_open()) {
        consume_bom(input);
        for( std::string line; getline( input, line ); ) {
            if (!line.empty() && line[line.length()-1] == '\n') {
                line.pop_back();
            }

            if (!line.empty() && line[line.length()-1] == '\r') {
                line.pop_back();
            }

            try {
                std::string lang = line;
                if (!first_language.size()) first_language = lang;
                supported_languages.insert(lang);
                PRINT_DEBUG("Added supported_language %s\n", lang.c_str());
            } catch (...) {}
        }
    }

    // if the current emu language is not in the supported languages list
    if (!supported_languages.count(language)) {
        // and the supported languages list wasn't empty
        if (first_language.size()) {
            language = first_language;
        }
    }

    return supported_languages;
}

// DLC.txt
static void parse_dlc(class Settings *settings_client, Settings *settings_server)
{
    std::string dlc_config_path = Local_Storage::get_game_settings_path() + "DLC.txt";
    std::ifstream input( utf8_decode(dlc_config_path) );
    if (input.is_open()) {
        consume_bom(input);
        settings_client->unlockAllDLC(false);
        settings_server->unlockAllDLC(false);
        PRINT_DEBUG("Locking all DLC\n");

        for( std::string line; std::getline( input, line ); ) {
            if (!line.empty() && line.front() == '#') {
                continue;
            }
            if (!line.empty() && line.back() == '\n') {
                line.pop_back();
            }

            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            std::size_t deliminator = line.find("=");
            if (deliminator != 0 && deliminator != std::string::npos && deliminator != line.size()) {
                AppId_t appid = stol(line.substr(0, deliminator));
                std::string name = line.substr(deliminator + 1);
                bool available = true;

                if (appid) {
                    PRINT_DEBUG("Adding DLC: %u|%s| %u\n", appid, name.c_str(), available);
                    settings_client->addDLC(appid, name, available);
                    settings_server->addDLC(appid, name, available);
                }
            }
        }
    } else {
        //unlock all DLC
        PRINT_DEBUG("Unlocking all DLC\n");
        settings_client->unlockAllDLC(true);
        settings_server->unlockAllDLC(true);
    }
}

// app_paths.txt
static void parse_app_paths(class Settings *settings_client, Settings *settings_server, std::string &program_path)
{
    std::string dlc_config_path = Local_Storage::get_game_settings_path() + "app_paths.txt";
    std::ifstream input( utf8_decode(dlc_config_path) );

    if (input.is_open()) {
        consume_bom(input);
        for( std::string line; getline( input, line ); ) {
            if (!line.empty() && line[line.length()-1] == '\n') {
                line.pop_back();
            }

            if (!line.empty() && line[line.length()-1] == '\r') {
                line.pop_back();
            }

            std::size_t deliminator = line.find("=");
            if (deliminator != 0 && deliminator != std::string::npos && deliminator != line.size()) {
                AppId_t appid = stol(line.substr(0, deliminator));
                std::string rel_path = line.substr(deliminator + 1);
                std::string path = canonical_path(program_path + rel_path);

                if (appid) {
                    if (path.size()) {
                        PRINT_DEBUG("Adding app path: %u|%s|\n", appid, path.c_str());
                        settings_client->setAppInstallPath(appid, path);
                        settings_server->setAppInstallPath(appid, path);
                    } else {
                        PRINT_DEBUG("Error adding app path for: %u does this path exist? |%s|\n", appid, rel_path.c_str());
                    }
                }
            }
        }
    }

}

// leaderboards.txt
static void parse_leaderboards(class Settings *settings_client, Settings *settings_server)
{
    std::string dlc_config_path = Local_Storage::get_game_settings_path() + "leaderboards.txt";
    std::ifstream input( utf8_decode(dlc_config_path) );
    if (input.is_open()) {
        consume_bom(input);
        settings_client->setCreateUnknownLeaderboards(false);
        settings_server->setCreateUnknownLeaderboards(false);

        for( std::string line; getline( input, line ); ) {
            if (!line.empty() && line[line.length()-1] == '\n') {
                line.pop_back();
            }

            if (!line.empty() && line[line.length()-1] == '\r') {
                line.pop_back();
            }

            std::string leaderboard;
            unsigned int sort_method = 0;
            unsigned int display_type = 0;

            std::size_t deliminator = line.find("=");
            if (deliminator != 0 && deliminator != std::string::npos && deliminator != line.size()) {
                leaderboard = line.substr(0, deliminator);
                std::size_t deliminator2 = line.find("=", deliminator + 1);
                if (deliminator2 != std::string::npos && deliminator2 != line.size()) {
                    sort_method = stol(line.substr(deliminator + 1, deliminator2 - (deliminator + 1)));
                    display_type = stol(line.substr(deliminator2 + 1));
                }
            }

            if (leaderboard.size() && sort_method <= k_ELeaderboardSortMethodDescending && display_type <= k_ELeaderboardDisplayTypeTimeMilliSeconds) {
                PRINT_DEBUG("Adding leaderboard: %s|%u|%u\n", leaderboard.c_str(), sort_method, display_type);
                settings_client->setLeaderboard(leaderboard, (ELeaderboardSortMethod)sort_method, (ELeaderboardDisplayType)display_type);
                settings_server->setLeaderboard(leaderboard, (ELeaderboardSortMethod)sort_method, (ELeaderboardDisplayType)display_type);
            } else {
                PRINT_DEBUG("Error adding leaderboard for: %s, are sort method %u or display type %u valid?\n", leaderboard.c_str(), sort_method, display_type);
            }
        }
    }

}

// stats.txt
static void parse_stats(class Settings *settings_client, Settings *settings_server)
{
    std::string stats_config_path = Local_Storage::get_game_settings_path() + "stats.txt";
    std::ifstream input( utf8_decode(stats_config_path) );
    if (input.is_open()) {
        consume_bom(input);
        for( std::string line; getline( input, line ); ) {
            if (!line.empty() && line[line.length()-1] == '\n') {
                line.pop_back();
            }

            if (!line.empty() && line[line.length()-1] == '\r') {
                line.pop_back();
            }

            std::string stat_name;
            std::string stat_type;
            std::string stat_default_value;

            std::size_t deliminator = line.find("=");
            if (deliminator != 0 && deliminator != std::string::npos && deliminator != line.size()) {
                stat_name = line.substr(0, deliminator);
                std::size_t deliminator2 = line.find("=", deliminator + 1);

                if (deliminator2 != std::string::npos && deliminator2 != line.size()) {
                    stat_type = line.substr(deliminator + 1, deliminator2 - (deliminator + 1));
                    stat_default_value = line.substr(deliminator2 + 1);
                } else {
                    stat_type = line.substr(deliminator + 1);
                    stat_default_value = "0";
                }
            }

            std::transform(stat_type.begin(), stat_type.end(), stat_type.begin(),[](unsigned char c){ return std::tolower(c); });
            struct Stat_config config = {};

            try {
                if (stat_type == "float") {
                    config.type = Stat_Type::STAT_TYPE_FLOAT;
                    config.default_value_float = std::stof(stat_default_value);
                } else if (stat_type == "int") {
                    config.type = Stat_Type::STAT_TYPE_INT;
                    config.default_value_int = std::stol(stat_default_value);
                } else if (stat_type == "avgrate") {
                    config.type = Stat_Type::STAT_TYPE_AVGRATE;
                    config.default_value_float = std::stof(stat_default_value);
                } else {
                    PRINT_DEBUG("Error adding stat %s, type %s isn't valid\n", stat_name.c_str(), stat_type.c_str());
                    continue;
                }
            } catch (...) {
                PRINT_DEBUG("Error adding stat %s, default value %s isn't valid\n", stat_name.c_str(), stat_default_value.c_str());
                continue;
            }

            if (stat_name.size()) {
                PRINT_DEBUG("Adding stat type: %s|%u|%f|%u\n", stat_name.c_str(), config.type, config.default_value_float, config.default_value_int);
                settings_client->setStatDefiniton(stat_name, config);
                settings_server->setStatDefiniton(stat_name, config);
            } else {
                PRINT_DEBUG("Error adding stat for: %s, empty name\n", stat_name.c_str());
            }
        }
    }

}

// depots.txt
static void parse_depots(class Settings *settings_client, Settings *settings_server)
{
    std::string depots_config_path = Local_Storage::get_game_settings_path() + "depots.txt";
    std::ifstream input( utf8_decode(depots_config_path) );
    if (input.is_open()) {
        consume_bom(input);
        for( std::string line; getline( input, line ); ) {
            if (!line.empty() && line[line.length()-1] == '\n') {
                line.pop_back();
            }

            if (!line.empty() && line[line.length()-1] == '\r') {
                line.pop_back();
            }

            try {
                DepotId_t depot_id = std::stoul(line);
                settings_client->depots.push_back(depot_id);
                settings_server->depots.push_back(depot_id);
                PRINT_DEBUG("Added depot %u\n", depot_id);
            } catch (...) {}
        }
    }

}

// subscribed_groups.txt
static void parse_subscribed_groups(class Settings *settings_client, Settings *settings_server)
{
    std::string depots_config_path = Local_Storage::get_game_settings_path() + "subscribed_groups.txt";
    std::ifstream input( utf8_decode(depots_config_path) );
    if (input.is_open()) {
        consume_bom(input);
        for( std::string line; getline( input, line ); ) {
            if (!line.empty() && line[line.length()-1] == '\n') {
                line.pop_back();
            }

            if (!line.empty() && line[line.length()-1] == '\r') {
                line.pop_back();
            }

            try {
                uint64 source_id = std::stoull(line);
                settings_client->subscribed_groups.insert(source_id);
                settings_server->subscribed_groups.insert(source_id);
                PRINT_DEBUG("Added source %llu\n", source_id);
            } catch (...) {}
        }
    }

}

// installed_app_ids.txt
static void parse_installed_app_Ids(class Settings *settings_client, Settings *settings_server)
{
    std::string installed_apps_list_path = Local_Storage::get_game_settings_path() + "installed_app_ids.txt";
    std::ifstream input( utf8_decode(installed_apps_list_path) );
    if (input.is_open()) {
        settings_client->assume_any_app_installed = false;
        settings_server->assume_any_app_installed = false;
        PRINT_DEBUG("Limiting/Locking installed apps\n");
        consume_bom(input);
        for( std::string line; getline( input, line ); ) {
            if (!line.empty() && line[line.length()-1] == '\n') {
                line.pop_back();
            }

            if (!line.empty() && line[line.length()-1] == '\r') {
                line.pop_back();
            }

            try {
                AppId_t app_id = std::stoul(line);
                settings_client->installed_app_ids.insert(app_id);
                settings_server->installed_app_ids.insert(app_id);
                PRINT_DEBUG("Added installed app with ID %u\n", app_id);
            } catch (...) {}
        }
    } else {
        settings_client->assume_any_app_installed = true;
        settings_server->assume_any_app_installed = true;
        PRINT_DEBUG("Assuming any app is installed\n");
    }

}

// force_branch_name.txt
static void parse_force_branch_name(class Settings *settings_client, Settings *settings_server)
{
    std::string installed_apps_list_path = Local_Storage::get_game_settings_path() + "force_branch_name.txt";
    std::ifstream input( utf8_decode(installed_apps_list_path) );
    if (input.is_open()) {
        consume_bom(input);
        std::string line;
        getline( input, line );
        
        size_t start = line.find_first_not_of(whitespaces);
        size_t end = line.find_last_not_of(whitespaces);
        line = start == end
            ? std::string()
            : line.substr(start, end - start + 1);
        
        if (!line.empty()) {
            settings_client->current_branch_name = line;
            settings_server->current_branch_name = line;
            PRINT_DEBUG("Forcing current branch name to '%s'\n", line.c_str());
        }
    }
}

// steam_settings/mods
static void parse_mods_folder(class Settings *settings_client, Settings *settings_server, class Local_Storage *local_storage)
{
    std::chrono::system_clock::time_point one_week_ago = std::chrono::system_clock::now() - std::chrono::hours(24 * 7);
    auto one_week_ago_epoch = std::chrono::duration_cast<std::chrono::seconds>(one_week_ago.time_since_epoch()).count();
    std::string mod_path = Local_Storage::get_game_settings_path() + "mods";
    nlohmann::json mod_items = nlohmann::json::object();
    static constexpr auto mods_json_file = "mods.json";
    std::string mods_json_path = Local_Storage::get_game_settings_path() + mods_json_file;
    if (local_storage->load_json(mods_json_path, mod_items)) {
        for (auto mod = mod_items.begin(); mod != mod_items.end(); ++mod) {
            try {
                std::string mod_images_folder = Local_Storage::get_game_settings_path() + "mod_images" + PATH_SEPARATOR + std::string(mod.key());
                Mod_entry newMod;
                newMod.id = std::stoull(mod.key());
                newMod.title = mod.value().value("title", std::string(mod.key()));
                newMod.path = mod.value().value("path", std::string(""));
                if (newMod.path.empty()) {
                    newMod.path = mod_path + PATH_SEPARATOR + std::string(mod.key());
                }
                newMod.fileType = k_EWorkshopFileTypeCommunity;
                newMod.description = mod.value().value("description", std::string(""));
                newMod.steamIDOwner = mod.value().value("steam_id_owner", settings_client->get_local_steam_id().ConvertToUint64());
                newMod.timeCreated = mod.value().value("time_created", (uint32)std::chrono::system_clock::now().time_since_epoch().count());
                newMod.timeUpdated = mod.value().value("time_updated", (uint32)one_week_ago.time_since_epoch().count());
                newMod.timeAddedToUserList = mod.value().value("time_added", (uint32)one_week_ago_epoch);
                newMod.visibility = k_ERemoteStoragePublishedFileVisibilityPublic;
                newMod.banned = false;
                newMod.acceptedForUse = true;
                newMod.tagsTruncated = false;
                newMod.tags = mod.value().value("tags", std::string(""));

                constexpr static auto get_file_size = [](
                    const std::string &filepath,
                    const std::string &basepath,
                    int32 default_val = 0) -> size_t {
                    try
                    {
                        const auto file_p = common_helpers::to_absolute(filepath, basepath);
                        if (file_p.empty()) return default_val;

                        size_t size = 0;
                        if (common_helpers::file_size(file_p, size)) {
                            return size;
                        }
                    } catch(...) {}
                    return default_val;
                };

                newMod.primaryFileName = mod.value().value("primary_filename", std::string(""));
                int32 primary_filesize = 0;
                if (!newMod.primaryFileName.empty()) {
                    primary_filesize = (int32)get_file_size(newMod.primaryFileName, newMod.path, primary_filesize);
                }
                newMod.primaryFileSize = mod.value().value("primary_filesize", primary_filesize);
                
                newMod.previewFileName = mod.value().value("preview_filename", std::string(""));
                int32 preview_filesize = 0;
                if (!newMod.previewFileName.empty()) {
                    preview_filesize = (int32)get_file_size(
                        newMod.previewFileName,
                        mod_images_folder,
                        preview_filesize);
                }
                newMod.previewFileSize = mod.value().value("preview_filesize", preview_filesize);

                newMod.workshopItemURL = mod.value().value("workshop_item_url", "https://steamcommunity.com/sharedfiles/filedetails/?id=" + std::string(mod.key()));
                newMod.votesUp = mod.value().value("upvotes", (uint32)1);
                newMod.votesDown = mod.value().value("downvotes", (uint32)0);

                float score = 1.0f;
                try
                {
                    score = newMod.votesUp / (float)(newMod.votesUp + newMod.votesDown);
                } catch(...) {}
                newMod.score = mod.value().value("score", score);
                
                newMod.numChildren = mod.value().value("num_children", (uint32)0);

                newMod.previewURL = mod.value().value("preview_url", std::string(""));
                if (newMod.previewURL.empty()) {
                    if (newMod.previewFileName.empty()) {
                        newMod.previewURL = std::string();
                    } else {
                        auto settings_folder = std::string(Local_Storage::get_game_settings_path());
                        std::replace(settings_folder.begin(), settings_folder.end(), '\\', '/');
                        newMod.previewURL = "file://" + settings_folder + "mod_images/" + std::string(mod.key()) + "/" + newMod.previewFileName;
                    }
                }
                
                settings_client->addMod(newMod.id, newMod.title, newMod.path);
                settings_server->addMod(newMod.id, newMod.title, newMod.path);
                settings_client->addModDetails(newMod.id, newMod);
                settings_server->addModDetails(newMod.id, newMod);
                
                PRINT_DEBUG("  parsed mod '%s':\n", std::string(mod.key()).c_str());
                PRINT_DEBUG("    path (will be used for primary file): '%s'\n", newMod.path.c_str());
                PRINT_DEBUG("    images path (will be used for preview file): '%s'\n", mod_images_folder.c_str());
                PRINT_DEBUG("    primary_filename: '%s'\n", newMod.primaryFileName.c_str());
                PRINT_DEBUG("    primary_filesize: %i bytes\n", newMod.primaryFileSize);
                PRINT_DEBUG("    primary file handle: %llu\n", settings_client->getMod(newMod.id).handleFile);
                PRINT_DEBUG("    preview_filename: '%s'\n", newMod.previewFileName.c_str());
                PRINT_DEBUG("    preview_filesize: %i bytes\n", newMod.previewFileSize);
                PRINT_DEBUG("    preview file handle: %llu\n", settings_client->getMod(newMod.id).handlePreviewFile);
                PRINT_DEBUG("    workshop_item_url: '%s'\n", newMod.workshopItemURL.c_str());
                PRINT_DEBUG("    preview_url: '%s'\n", newMod.previewURL.c_str());
            } catch (std::exception& e) {
                PRINT_DEBUG("MODLOADER ERROR: %s\n", e.what());
            }
        }
    } else { // invalid mods.json or doesn't exist
        std::vector<std::string> paths = Local_Storage::get_filenames_path(mod_path);
        for (auto & p: paths) {
            PRINT_DEBUG("mod directory %s\n", p.c_str());
            try {
                Mod_entry newMod;
                newMod.id = std::stoull(p);
                newMod.title = p;
                newMod.path = mod_path + PATH_SEPARATOR + p;
                newMod.fileType = k_EWorkshopFileTypeCommunity;
                newMod.description = "";
                newMod.steamIDOwner = (uint64)0;
                newMod.timeCreated = (uint32)one_week_ago_epoch;
                newMod.timeUpdated = (uint32)one_week_ago_epoch;
                newMod.timeAddedToUserList = (uint32)one_week_ago_epoch;
                newMod.visibility = k_ERemoteStoragePublishedFileVisibilityPublic;
                newMod.banned = false;
                newMod.acceptedForUse = true;
                newMod.tagsTruncated = false;
                newMod.tags = "";
                newMod.primaryFileName = "";
                newMod.primaryFileSize = (int32)0;
                newMod.previewFileName = "";
                newMod.previewFileSize = (int32)0;
                newMod.workshopItemURL = "";
                newMod.votesUp = (uint32)1;
                newMod.votesDown = (uint32)0;
                newMod.score = 1.0f;
                newMod.numChildren = (uint32)0;
                newMod.previewURL = "";
                settings_client->addMod(newMod.id, newMod.title, newMod.path);
                settings_server->addMod(newMod.id, newMod.title, newMod.path);
                settings_client->addModDetails(newMod.id, newMod);
                settings_server->addModDetails(newMod.id, newMod);
            } catch (...) {}
        }
    }

}

// force_language.txt
static bool parse_force_language(std::string &language, std::string &steam_settings_path)
{
    bool warn_forced = false;

    char forced_language[64] = {};
    int len = Local_Storage::get_file_data(steam_settings_path + "force_language.txt", forced_language, sizeof(forced_language) - 1);
    if (len > 0) {
        forced_language[len] = 0;
        warn_forced = true;
    }
    language = std::string(forced_language);

    return warn_forced;
}

// force_steamid.txt
static bool parse_force_user_steam_id(CSteamID &user_id, std::string &steam_settings_path)
{
    bool warn_forced = false;

    char steam_id_text[32] = {};
    if (Local_Storage::get_file_data(steam_settings_path + "force_steamid.txt", steam_id_text, sizeof(steam_id_text) - 1) > 0) {
        CSteamID temp_id = CSteamID((uint64)std::atoll(steam_id_text));
        if (temp_id.IsValid()) {
            user_id = temp_id;
            warn_forced = true;
        }
    }

    return warn_forced;
}

// force_account_name.txt
static bool parse_force_account_name(std::string &name, std::string &steam_settings_path)
{
    bool warn_forced = false;

    char forced_name[100] = {};
    int len = Local_Storage::get_file_data(steam_settings_path + "force_account_name.txt", forced_name, sizeof(forced_name) - 1);
    if (len > 0) {
        forced_name[len] = 0;
        warn_forced = true;
    }
    name = std::string(forced_name);

    return warn_forced;
}

// force_listen_port.txt
static bool parse_force_listen_port(uint16 &port, std::string &steam_settings_path)
{
    bool warn_forced = false;

    char array_port[10] = {};
    int len = Local_Storage::get_file_data(steam_settings_path + "force_listen_port.txt", array_port, sizeof(array_port) - 1);
    if (len > 0) {
        port = std::stoi(array_port);
        warn_forced = true;
    }

    return warn_forced;
}

// build_id.txt
static void parse_build_id(int &build_id, std::string &steam_settings_path)
{
    char array_id[10] = {};
    int len = Local_Storage::get_file_data(steam_settings_path + "build_id.txt", array_id, sizeof(array_id) - 1);
    if (len > 0) {
        build_id = std::stoi(array_id);
    }
}

// crash_printer_location.txt
static void parse_crash_printer_location()
{
    std::string installed_apps_list_path = Local_Storage::get_game_settings_path() + "crash_printer_location.txt";
    std::ifstream input( utf8_decode(installed_apps_list_path) );
    if (input.is_open()) {
        consume_bom(input);
        std::string line;
        std::getline( input, line );
        
        size_t start = line.find_first_not_of(whitespaces);
        size_t end = line.find_last_not_of(whitespaces);
        line = start == end
            ? std::string()
            : line.substr(start, end - start + 1);
        
        if (!line.empty()) {
            auto crash_path = utf8_decode(get_full_program_path() + line);
            if (crash_printer::init(crash_path)) {
                PRINT_DEBUG("Unhandled crashes will be saved to '%s'\n", line.c_str());
            } else {
                PRINT_DEBUG("Failed to setup unhandled crash printer with path: '%s'\n", line.c_str());
            }
        }
    }
}

uint32 create_localstorage_settings(Settings **settings_client_out, Settings **settings_server_out, Local_Storage **local_storage_out)
{
    std::string program_path = Local_Storage::get_program_path();
    std::string save_path = Local_Storage::get_user_appdata_path();

    PRINT_DEBUG("Current Path %s save_path: %s\n", program_path.c_str(), save_path.c_str());

    parse_crash_printer_location();

    uint32 appid = parse_steam_app_id(program_path);

    bool local_save = parse_local_save(program_path, save_path);

    PRINT_DEBUG("Set save_path: %s\n", save_path.c_str());
    Local_Storage *local_storage = new Local_Storage(save_path);
    local_storage->setAppId(appid);

    // Listen port
    uint16 port = parse_listen_port(local_storage);

    // Custom broadcasts
    std::set<IP_PORT> custom_broadcasts;
    load_custom_broadcasts(local_storage->get_global_settings_path() + "custom_broadcasts.txt", custom_broadcasts);
    load_custom_broadcasts(Local_Storage::get_game_settings_path() + "custom_broadcasts.txt", custom_broadcasts);

    // Acount name
    std::string name = parse_account_name(local_storage);
    
    // Steam ID
    CSteamID user_id = parse_user_steam_id(local_storage);

    // Language
    std::string language = parse_current_language(local_storage);

    // Supported languages, this will change the current language if needed
    std::set<std::string> supported_languages = parse_supported_languages(local_storage, language);

    bool steam_offline_mode = false;
    bool steam_deck_mode = false;
    bool steamhttp_online_mode = false;
    bool disable_networking = false;
    bool disable_overlay = false;
    bool disable_overlay_achievement_notification = false;
    bool disable_overlay_friend_notification = false;
    bool disable_overlay_warning = false;
    bool disable_lobby_creation = false;
    bool disable_source_query = false;
    bool disable_account_avatar = false;
    bool achievement_bypass = false;
    bool is_beta_branch = false;
    bool use_gc_token = false;
    bool enable_new_app_ticket = false;
    int build_id = 10;

    bool warn_forced = false;

    // boolean flags and forced configurations
    {
        std::string steam_settings_path = Local_Storage::get_game_settings_path();

        std::vector<std::string> paths = Local_Storage::get_filenames_path(steam_settings_path);
        for (auto & p: paths) {
            PRINT_DEBUG("steam settings path %s\n", p.c_str());
            if (p == "offline.txt") {
                steam_offline_mode = true;
            } else if (p == "steam_deck.txt") {
                steam_deck_mode = true;
            } else if (p == "http_online.txt") {
                steamhttp_online_mode = true;
            } else if (p == "disable_networking.txt") {
                disable_networking = true;
            } else if (p == "disable_overlay.txt") {
                disable_overlay = true;
            } else if (p == "disable_overlay_achievement_notification.txt") {
                disable_overlay_achievement_notification = true;
            } else if (p == "disable_overlay_friend_notification.txt") {
                disable_overlay_friend_notification = true;
            } else if (p == "disable_overlay_warning.txt") {
                disable_overlay_warning = true;
            } else if (p == "disable_lobby_creation.txt") {
                disable_lobby_creation = true;
            } else if (p == "disable_source_query.txt") {
                disable_source_query = true;
            } else if (p == "disable_account_avatar.txt") {
                disable_account_avatar = true;
            } else if (p == "achievements_bypass.txt") {
                achievement_bypass = true;
            } else if (p == "is_beta_branch.txt") {
                is_beta_branch = true;
            } else if (p == "force_language.txt") {
                warn_forced = parse_force_language(language, steam_settings_path);
            } else if (p == "force_steamid.txt") {
                warn_forced = parse_force_user_steam_id(user_id, steam_settings_path);
            } else if (p == "force_account_name.txt") {
                warn_forced = parse_force_account_name(name, steam_settings_path);
            } else if (p == "force_listen_port.txt") {
                warn_forced = parse_force_listen_port(port, steam_settings_path);
            } else if (p == "build_id.txt") {
                parse_build_id(build_id, steam_settings_path);
            } else if (p == "new_app_ticket.txt") {
                enable_new_app_ticket = true;
            } else if (p == "gc_token.txt") {
                use_gc_token = true;
            }
        }
    }

    Settings *settings_client = new Settings(user_id, CGameID(appid), name, language, steam_offline_mode);
    Settings *settings_server = new Settings(generate_steam_id_server(), CGameID(appid), name, language, steam_offline_mode);
    settings_client->set_port(port);
    settings_server->set_port(port);
    settings_client->custom_broadcasts = custom_broadcasts;
    settings_server->custom_broadcasts = custom_broadcasts;
    settings_client->disable_networking = disable_networking;
    settings_server->disable_networking = disable_networking;
    settings_client->disable_overlay = disable_overlay;
    settings_server->disable_overlay = disable_overlay;
    settings_client->disable_overlay_achievement_notification = disable_overlay_achievement_notification;
    settings_server->disable_overlay_achievement_notification = disable_overlay_achievement_notification;
    settings_client->disable_overlay_friend_notification = disable_overlay_friend_notification;
    settings_server->disable_overlay_friend_notification = disable_overlay_friend_notification;
    settings_client->disable_overlay_warning = disable_overlay_warning;
    settings_server->disable_overlay_warning = disable_overlay_warning;
    settings_client->disable_lobby_creation = disable_lobby_creation;
    settings_server->disable_lobby_creation = disable_lobby_creation;
    settings_client->disable_source_query = disable_source_query;
    settings_server->disable_source_query = disable_source_query;
    settings_client->disable_account_avatar = disable_account_avatar;
    settings_server->disable_account_avatar = disable_account_avatar;
    settings_client->build_id = build_id;
    settings_server->build_id = build_id;
    settings_client->warn_forced = warn_forced;
    settings_server->warn_forced = warn_forced;
    settings_client->warn_local_save = local_save;
    settings_server->warn_local_save = local_save;
    settings_client->supported_languages = supported_languages;
    settings_server->supported_languages = supported_languages;
    settings_client->steam_deck = steam_deck_mode;
    settings_server->steam_deck = steam_deck_mode;
    settings_client->http_online = steamhttp_online_mode;
    settings_server->http_online = steamhttp_online_mode;
    settings_client->achievement_bypass = achievement_bypass;
    settings_server->achievement_bypass = achievement_bypass;
    settings_client->is_beta_branch = is_beta_branch;
    settings_server->is_beta_branch = is_beta_branch;
    settings_client->enable_new_app_ticket = enable_new_app_ticket;
    settings_server->enable_new_app_ticket = enable_new_app_ticket;
    settings_client->use_gc_token = use_gc_token;
    settings_server->use_gc_token = use_gc_token;

    if (local_save) {
        settings_client->local_save = save_path;
        settings_server->local_save = save_path;
    }

    parse_dlc(settings_client, settings_server);

    parse_app_paths(settings_client, settings_server, program_path);

    parse_leaderboards(settings_client, settings_server);

    parse_stats(settings_client, settings_server);

    parse_depots(settings_client, settings_server);

    parse_subscribed_groups(settings_client, settings_server);

    parse_installed_app_Ids(settings_client, settings_server);

    parse_force_branch_name(settings_client, settings_server);

    load_subscribed_groups_clans(local_storage->get_global_settings_path() + "subscribed_groups_clans.txt", settings_client, settings_server);
    load_subscribed_groups_clans(Local_Storage::get_game_settings_path() + "subscribed_groups_clans.txt", settings_client, settings_server);

    load_overlay_appearance(local_storage->get_global_settings_path() + "overlay_appearance.txt", settings_client, settings_server);
    load_overlay_appearance(Local_Storage::get_game_settings_path() + "overlay_appearance.txt", settings_client, settings_server);

    parse_mods_folder(settings_client, settings_server, local_storage);

    load_gamecontroller_settings(settings_client);

    *settings_client_out = settings_client;
    *settings_server_out = settings_server;
    *local_storage_out = local_storage;

    reset_LastError();
    return appid;
}

void save_global_settings(Local_Storage *local_storage, char *name, char *language)
{
    local_storage->store_data_settings("account_name.txt", name, strlen(name));
    local_storage->store_data_settings("language.txt", language, strlen(language));
}
