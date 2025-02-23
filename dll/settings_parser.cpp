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

#define SI_CONVERT_GENERIC
#define SI_SUPPORT_IOSTREAMS
#define SI_NO_MBCS
#include "simpleini/SimpleIni.h"


constexpr const static char config_ini_app[]     = "configs.app.ini";
constexpr const static char config_ini_main[]    = "configs.main.ini";
constexpr const static char config_ini_overlay[] = "configs.overlay.ini";
constexpr const static char config_ini_user[]    = "configs.user.ini";

static CSimpleIniA ini{};

typedef struct IniValue {
    enum class Type {
        STR,
        BOOL,
        DOUBLE,
        LONG,
    };

    explicit IniValue(const char *new_val): type(Type::STR),     val_str(new_val) {}
    explicit IniValue(bool new_val):        type(Type::BOOL),    val_bool(new_val) {}
    explicit IniValue(double new_val):      type(Type::DOUBLE),  val_double(new_val) {}
    explicit IniValue(long new_val):        type(Type::LONG),    val_long(new_val) {}

    Type type;
    union {
        const char *val_str;
        bool val_bool;
        double val_double;
        long val_long;

    };
} IniValue;

static void save_global_ini_value(class Local_Storage *local_storage, const char *filename, const char *section, const char *key, IniValue val, const char *comment = nullptr) {
    CSimpleIniA new_ini{};
    new_ini.SetUnicode();
    new_ini.SetSpaces(false);

    auto sz = local_storage->data_settings_size(filename);
    if (sz > 0) {
        std::vector<char> ini_file_data(sz);
        auto read = local_storage->get_data_settings(filename, &ini_file_data[0], static_cast<unsigned int>(ini_file_data.size()));
        if (read == sz) {
            new_ini.LoadData(&ini_file_data[0], ini_file_data.size());
        }
    }

    std::string comment_str{};
    if (comment && comment[0]) {
        comment_str.append("# ").append(comment);
        comment = comment_str.c_str();
    }
    
    switch (val.type)
    {
    case IniValue::Type::STR:
        new_ini.SetValue(section, key, val.val_str, comment);
    break;
    
    case IniValue::Type::BOOL:
        new_ini.SetBoolValue(section, key, val.val_bool, comment);
    break;

    case IniValue::Type::DOUBLE:
        new_ini.SetDoubleValue(section, key, val.val_double, comment);
    break;

    case IniValue::Type::LONG:
        new_ini.SetLongValue(section, key, val.val_long, comment);
    break;

    default: break;
    }

    std::string ini_buff{};
    if (new_ini.Save(ini_buff, false) == SI_OK) {
        local_storage->store_data_settings(filename, &ini_buff[0], static_cast<unsigned int>(ini_buff.size()));
    }
    
}

static void merge_ini(const CSimpleIniA &new_ini, bool overwrite = false) {
    std::list<CSimpleIniA::Entry> sections{};
    new_ini.GetAllSections(sections);
    for (auto const &sec : sections) {
        std::list<CSimpleIniA::Entry> keys{};
        new_ini.GetAllKeys(sec.pItem, keys);
        for (auto const &key : keys) {
            // only add the key if it didn't exist already
            if (!ini.KeyExists(sec.pItem, key.pItem) || overwrite) {
                std::list<CSimpleIniA::Entry> vals{};
                new_ini.GetAllValues(sec.pItem, key.pItem, vals);
                for (const auto &val : vals) {
                    ini.SetValue(sec.pItem, key.pItem, val.pItem);
                }
            }
        }
    }
}


Overlay_Appearance::NotificationPosition Overlay_Appearance::translate_notification_position(const std::string &str)
{
    if (str == "top_left") return NotificationPosition::top_left;
    else if (str == "top_center") return NotificationPosition::top_center;
    else if (str == "top_right") return NotificationPosition::top_right;
    else if (str == "bot_left") return NotificationPosition::bot_left;
    else if (str == "bot_center") return NotificationPosition::bot_center;
    else if (str == "bot_right") return NotificationPosition::bot_right;

    PRINT_DEBUG("Invalid position '%s'", str.c_str());
    return default_pos;
}


// custom_broadcasts.txt
static void load_custom_broadcasts(const std::string &base_path, std::set<IP_PORT> &custom_broadcasts)
{
    const std::string broadcasts_filepath(base_path + "custom_broadcasts.txt");
    std::ifstream broadcasts_file(std::filesystem::u8path(broadcasts_filepath));
    if (broadcasts_file.is_open()) {
        common_helpers::consume_bom(broadcasts_file);
        PRINT_DEBUG("loading broadcasts file '%s'", broadcasts_filepath.c_str());
        std::string line{};
        while (std::getline(broadcasts_file, line)) {
            if (line.length() <= 0) continue;

            std::set<IP_PORT> ips = Networking::resolve_ip(line);
            custom_broadcasts.insert(ips.begin(), ips.end());
            PRINT_DEBUG("added ip/port to broadcast list '%s'", line.c_str());
        }
    }
}

// subscribed_groups_clans.txt
static void load_subscribed_groups_clans(const std::string &base_path, Settings *settings_client, Settings *settings_server)
{
    const std::string clans_filepath(base_path + "subscribed_groups_clans.txt");
    std::ifstream clans_file(std::filesystem::u8path(clans_filepath));
    if (clans_file.is_open()) {
        common_helpers::consume_bom(clans_file);
        PRINT_DEBUG("loading group clans file '%s'", clans_filepath.c_str());
        std::string line{};
        while (std::getline(clans_file, line)) {
            if (line.length() <= 0) continue;

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
                PRINT_DEBUG("Added clan %s", clan_name.c_str());
            } catch (...) {}
        }
    }
}

// overlay::appearance
static void load_overlay_appearance(class Settings *settings_client, class Settings *settings_server, class Local_Storage *local_storage)
{
    std::list<CSimpleIniA::Entry> names{};
    if (!ini.GetAllKeys("overlay::appearance", names) || names.empty()) return;

    for (const auto &name_ent : names) {
        auto val_ptr = ini.GetValue("overlay::appearance", name_ent.pItem);
        if (!val_ptr || !val_ptr[0]) continue;

        std::string name(name_ent.pItem);
        std::string value(val_ptr);
        PRINT_DEBUG("  Overlay appearance line '%s'='%s'", name.c_str(), value.c_str());
        try {
            if (name.compare("Font_Override") == 0) {
                value = common_helpers::string_strip(value);
                // first try the local settings folder
                std::string nfont_override(common_helpers::to_absolute(value, Local_Storage::get_game_settings_path() + "fonts"));
                if (!common_helpers::file_exist(nfont_override)) {
                    nfont_override.clear();
                }

                // then try the global settings folder
                if (nfont_override.empty()) {
                    nfont_override = common_helpers::to_absolute(value, local_storage->get_global_settings_path() + "fonts");
                    if (!common_helpers::file_exist(nfont_override)) {
                        nfont_override.clear();
                    }
                }

                if (nfont_override.size()) {
                    settings_client->overlay_appearance.font_override = nfont_override;
                    settings_server->overlay_appearance.font_override = nfont_override;
                    PRINT_DEBUG("  loaded font '%s'", nfont_override.c_str());
                } else {
                    PRINT_DEBUG("  ERROR font file '%s' doesn't exist and will be ignored", value.c_str());
                }
            } else if (name.compare("Font_Size") == 0) {
                float nfont_size = std::stof(value, NULL);
                settings_client->overlay_appearance.font_size = nfont_size;
                settings_server->overlay_appearance.font_size = nfont_size;
            } else if (name.compare("Icon_Size") == 0) {
                float nicon_size = std::stof(value, NULL);
                settings_client->overlay_appearance.icon_size = nicon_size;
                settings_server->overlay_appearance.icon_size = nicon_size;
            } else if (name.compare("Font_Glyph_Extra_Spacing_x") == 0) {
                float size = std::stof(value, NULL);
                settings_client->overlay_appearance.font_glyph_extra_spacing_x = size;
                settings_server->overlay_appearance.font_glyph_extra_spacing_x = size;
            } else if (name.compare("Font_Glyph_Extra_Spacing_y") == 0) {
                float size = std::stof(value, NULL);
                settings_client->overlay_appearance.font_glyph_extra_spacing_y = size;
                settings_server->overlay_appearance.font_glyph_extra_spacing_y = size;
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
            } else if (name.compare("Notification_Rounding") == 0) {
                float nnotification_rounding = std::stof(value, NULL);
                settings_client->overlay_appearance.notification_rounding = nnotification_rounding;
                settings_server->overlay_appearance.notification_rounding = nnotification_rounding;
            } else if (name.compare("Notification_Margin_x") == 0) {
                float val = std::stof(value, NULL);
                settings_client->overlay_appearance.notification_margin_x = val;
                settings_server->overlay_appearance.notification_margin_x = val;
            } else if (name.compare("Notification_Margin_y") == 0) {
                float val = std::stof(value, NULL);
                settings_client->overlay_appearance.notification_margin_y = val;
                settings_server->overlay_appearance.notification_margin_y = val;
            } else if (name.compare("Notification_Animation") == 0) {
                uint32 nnotification_animation = (uint32)(std::stof(value, NULL) * 1000.0f); // convert sec to milli
                settings_client->overlay_appearance.notification_animation = nnotification_animation;
                settings_server->overlay_appearance.notification_animation = nnotification_animation;
            } else if (name.compare("Notification_Duration_Progress") == 0) {
                uint32 time = (uint32)(std::stof(value, NULL) * 1000.0f); // convert sec to milli
                settings_client->overlay_appearance.notification_duration_progress = time;
                settings_server->overlay_appearance.notification_duration_progress = time;
            } else if (name.compare("Notification_Duration_Achievement") == 0) {
                uint32 time = (uint32)(std::stof(value, NULL) * 1000.0f); // convert sec to milli
                settings_client->overlay_appearance.notification_duration_achievement = time;
                settings_server->overlay_appearance.notification_duration_achievement = time;
            } else if (name.compare("Notification_Duration_Invitation") == 0) {
                uint32 time = (uint32)(std::stof(value, NULL) * 1000.0f); // convert sec to milli
                settings_client->overlay_appearance.notification_duration_invitation = time;
                settings_server->overlay_appearance.notification_duration_invitation = time;
            } else if (name.compare("Notification_Duration_Chat") == 0) {
                uint32 time = (uint32)(std::stof(value, NULL) * 1000.0f); // convert sec to milli
                settings_client->overlay_appearance.notification_duration_chat = time;
                settings_server->overlay_appearance.notification_duration_chat = time;
            } else if (name.compare("Achievement_Unlock_Datetime_Format") == 0) {
                settings_client->overlay_appearance.ach_unlock_datetime_format = value;
                settings_server->overlay_appearance.ach_unlock_datetime_format = value;
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
            } else if (name.compare("PosAchievement") == 0) {
                auto pos = Overlay_Appearance::translate_notification_position(value);
                settings_client->overlay_appearance.ach_earned_pos = pos;
                settings_server->overlay_appearance.ach_earned_pos = pos;
            } else if (name.compare("PosInvitation") == 0) {
                auto pos = Overlay_Appearance::translate_notification_position(value);
                settings_client->overlay_appearance.invite_pos = pos;
                settings_server->overlay_appearance.invite_pos = pos;
            } else if (name.compare("PosChatMsg") == 0) {
                auto pos = Overlay_Appearance::translate_notification_position(value);
                settings_client->overlay_appearance.chat_msg_pos = pos;
                settings_server->overlay_appearance.chat_msg_pos = pos;
            // >>> FPS background
            } else if (name.compare("Stats_Background_R") == 0) {
                float val = std::stof(value, NULL);
                settings_client->overlay_appearance.stats_background_r = val;
                settings_server->overlay_appearance.stats_background_r = val;
            } else if (name.compare("Stats_Background_G") == 0) {
                float val = std::stof(value, NULL);
                settings_client->overlay_appearance.stats_background_g = val;
                settings_server->overlay_appearance.stats_background_g = val;
            } else if (name.compare("Stats_Background_B") == 0) {
                float val = std::stof(value, NULL);
                settings_client->overlay_appearance.stats_background_b = val;
                settings_server->overlay_appearance.stats_background_b = val;
            } else if (name.compare("Stats_Background_A") == 0) {
                float val = std::stof(value, NULL);
                settings_client->overlay_appearance.stats_background_a = val;
                settings_server->overlay_appearance.stats_background_a = val;
            // FPS background END <<<
            // >>> FPS text color
            } else if (name.compare("Stats_Text_R") == 0) {
                float val = std::stof(value, NULL);
                settings_client->overlay_appearance.stats_text_r = val;
                settings_server->overlay_appearance.stats_text_r = val;
            } else if (name.compare("Stats_Text_G") == 0) {
                float val = std::stof(value, NULL);
                settings_client->overlay_appearance.stats_text_g = val;
                settings_server->overlay_appearance.stats_text_g = val;
            } else if (name.compare("Stats_Text_B") == 0) {
                float val = std::stof(value, NULL);
                settings_client->overlay_appearance.stats_text_b = val;
                settings_server->overlay_appearance.stats_text_b = val;
            } else if (name.compare("Stats_Text_A") == 0) {
                float val = std::stof(value, NULL);
                settings_client->overlay_appearance.stats_text_a = val;
                settings_server->overlay_appearance.stats_text_a = val;
            // FPS text color END <<<
            // >>> FPS position
            } else if (name.compare("Stats_Pos_x") == 0) {
                auto pos = std::stof(value);
                if (pos < 0) {
                    pos = 0;
                } else if (pos > 1.0f) {
                    pos = 1.0f;
                }
                settings_client->overlay_stats_pos_x = pos;
                settings_server->overlay_stats_pos_x = pos;
            } else if (name.compare("Stats_Pos_y") == 0) {
                auto pos = std::stof(value);
                if (pos < 0) {
                    pos = 0;
                } else if (pos > 1.0f) {
                    pos = 1.0f;
                }
                settings_client->overlay_stats_pos_y = pos;
                settings_server->overlay_stats_pos_y = pos;
            // FPS position END <<<
            } else {
                PRINT_DEBUG("unknown overlay appearance setting");
            }

        } catch (...) { }
    }
}

template<typename Out>
static void split_string(const std::string &s, char delim, Out result) {
    std::stringstream ss(s);
    std::string item{};
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

// folder "controller"
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

        PRINT_DEBUG("controller config %s", p.c_str());
        std::string action_set_name = p.substr(0, length - 4);
        std::transform(action_set_name.begin(), action_set_name.end(), action_set_name.begin(),[](unsigned char c){ return std::toupper(c); });

        std::string controller_config_path = path + PATH_SEPARATOR + p;
        std::ifstream input( std::filesystem::u8path(controller_config_path) );
        if (input.is_open()) {
            common_helpers::consume_bom(input);
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
                PRINT_DEBUG("Added %s %s %s", action_name.c_str(), button_name.c_str(), source_mode.c_str());
            }

            settings->controller_settings.action_sets[action_set_name] = button_pairs;
            PRINT_DEBUG("Added %zu action names to %s", button_pairs.size(), action_set_name.c_str());
        }
    }

    settings->glyphs_directory = path + (PATH_SEPARATOR "glyphs" PATH_SEPARATOR);
}

// steam_appid.txt
static uint32 parse_steam_app_id(const std::string &program_path)
{
    uint32 appid = 0;

    // try steam_settings folder
    char array[10] = {};
    array[0] = '0';
    Local_Storage::get_file_data(Local_Storage::get_game_settings_path() + "steam_appid.txt", array, sizeof(array) - 1);
    try {
        appid = std::stoul(array);
    } catch (...) {}

    // try current dir
    if (!appid) {
        memset(array, 0, sizeof(array));
        array[0] = '0';
        Local_Storage::get_file_data("steam_appid.txt", array, sizeof(array) - 1);
        try {
            appid = std::stoul(array);
        } catch (...) {}
    }

    // try exe dir
    if (!appid) {
        memset(array, 0, sizeof(array));
        array[0] = '0';
        Local_Storage::get_file_data(program_path + "steam_appid.txt", array, sizeof(array) - 1);
        try {
            appid = std::stoul(array);
        } catch (...) {}
    }

    // try env vars
    if (!appid) {
        std::string str_appid = get_env_variable("SteamAppId");
        std::string str_gameid = get_env_variable("SteamGameId");
        std::string str_overlay_gameid = get_env_variable("SteamOverlayGameId");
        
        PRINT_DEBUG("str_appid %s str_gameid: %s str_overlay_gameid: %s", str_appid.c_str(), str_gameid.c_str(), str_overlay_gameid.c_str());
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

        PRINT_DEBUG("appid_env %u gameid_env: %u overlay_gameid: %u", appid_env, gameid_env, overlay_gameid);
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

    PRINT_DEBUG("final appid = %u", appid);
    return appid;
}

// user::saves::local_save_path
static bool parse_local_save(std::string &save_path)
{
    auto ptr = ini.GetValue("user::saves", "local_save_path");
    if (!ptr || !ptr[0]) return false;
    
    save_path = common_helpers::to_absolute(common_helpers::string_strip(ptr), Local_Storage::get_program_path());
    if (save_path.size() && save_path.back() != *PATH_SEPARATOR) {
        save_path.push_back(*PATH_SEPARATOR);
    }
    PRINT_DEBUG("using local save path '%s'", save_path.c_str());
    return true;
}

// main::connectivity::listen_port
static uint16 parse_listen_port(class Local_Storage *local_storage)
{
    uint16 port = static_cast<uint16>(ini.GetLongValue("main::connectivity", "listen_port"));
    if (port == 0) {
        port = DEFAULT_PORT;
    }
    return port;
}

// user::general::account_name
static std::string parse_account_name(class Local_Storage *local_storage)
{
    auto name = ini.GetValue("user::general", "account_name");
    if (!name || !name[0]) {
        name = DEFAULT_NAME;
        save_global_ini_value(
            local_storage,
            config_ini_user,
            "user::general", "account_name", IniValue(name),
            "user account name"
        );
    }
    return std::string(name);
}

// user::general::account_steamid
static CSteamID parse_user_steam_id(class Local_Storage *local_storage)
{
    char array_steam_id[32] = {};
    CSteamID user_id((uint64)std::atoll(ini.GetValue("user::general", "account_steamid", "0")));
    if (!user_id.IsValid()) {
        user_id = generate_steam_id_user();
        char temp_text[32]{};
        snprintf(temp_text, sizeof(temp_text), "%llu", (uint64)user_id.ConvertToUint64());
        save_global_ini_value(
            local_storage,
            config_ini_user,
            "user::general", "account_steamid", IniValue(temp_text),
            "Steam64 format"
        );
    }

    return user_id;
}

// user::general::language
// valid list: https://partner.steamgames.com/doc/store/localization/languages
static std::string parse_current_language(class Local_Storage *local_storage)
{
    auto lang = ini.GetValue("user::general", "language");
    if (!lang || !lang[0]) {
        lang = DEFAULT_LANGUAGE;
        save_global_ini_value(
            local_storage,
            config_ini_user,
            "user::general", "language", IniValue(lang),
            "the language reported to the game, default is 'english', check 'API language code' in https://partner.steamgames.com/doc/store/localization/languages"
        );
    }

    return common_helpers::to_lower(std::string(lang));
}

// supported_languages.txt
// valid list: https://partner.steamgames.com/doc/store/localization/languages
static std::set<std::string> parse_supported_languages(class Local_Storage *local_storage, std::string &language)
{
    std::set<std::string> supported_languages{};

    std::string lang_config_path = Local_Storage::get_game_settings_path() + "supported_languages.txt";
    std::ifstream input( std::filesystem::u8path(lang_config_path) );

    std::string first_language{};
    if (input.is_open()) {
        common_helpers::consume_bom(input);
        for( std::string line; getline( input, line ); ) {
            if (!line.empty() && line.back() == '\n') {
                line.pop_back();
            }

            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            try {
                std::string lang = common_helpers::to_lower(line);
                if (!first_language.size()) first_language = lang;
                supported_languages.insert(lang);
                PRINT_DEBUG("Added supported_language %s", lang.c_str());
            } catch (...) {}
        }
    }

    // if the current emu language is not in the supported languages list
    if (!supported_languages.count(language)) {
        if (first_language.size()) { // get the first supported language if the list wasn't empty
            PRINT_DEBUG("[?] Your language '%s' isn't found in supported_languages.txt, using '%s' instead", language.c_str(), first_language.c_str());
            language = first_language;
        } else { // otherwise just lie and add it then!
            supported_languages.insert(language);
            PRINT_DEBUG("Forced current language '%s' into supported_languages", language.c_str());
        }
    }

    return supported_languages;
}

// app::dlcs
static void parse_dlc(class Settings *settings_client, class Settings *settings_server)
{
    constexpr static const char unlock_all_key[] = "unlock_all";

    bool unlock_all = ini.GetBoolValue("app::dlcs", unlock_all_key, true);
    if (unlock_all) {
        PRINT_DEBUG("unlocking all DLCs");
        settings_client->unlockAllDLC(true);
        settings_server->unlockAllDLC(true);
    } else {
        PRINT_DEBUG("locking all DLCs");
        settings_client->unlockAllDLC(false);
        settings_server->unlockAllDLC(false);
    }

    std::list<CSimpleIniA::Entry> dlcs_keys{};
    if (!ini.GetAllKeys("app::dlcs", dlcs_keys) || dlcs_keys.empty()) return;

    // remove the unlock all key so we can iterate through the DLCs
    dlcs_keys.remove_if([](const CSimpleIniA::Entry &item){
        return common_helpers::str_cmp_insensitive(item.pItem, unlock_all_key);
    });

    for (const auto &dlc_key : dlcs_keys) {
        AppId_t appid = (AppId_t)std::stoul(dlc_key.pItem);
        if (!appid) continue;
        
        auto name = ini.GetValue("app::dlcs", dlc_key.pItem, "unknown DLC");
        PRINT_DEBUG("adding DLC: [%u] = '%s'", appid, name);
        settings_client->addDLC(appid, name, true);
        settings_server->addDLC(appid, name, true);
    }
}

// app::paths
static void parse_app_paths(class Settings *settings_client, Settings *settings_server, const std::string &program_path)
{
    std::list<CSimpleIniA::Entry> ids{};
    if (!ini.GetAllKeys("app::paths", ids) || ids.empty()) return;

    for (const auto &id : ids) {
        auto val_ptr = ini.GetValue("app::paths", id.pItem);
        if (!val_ptr || !val_ptr[0]) continue;

        AppId_t appid = (AppId_t)std::stoul(id.pItem);
        std::string rel_path(val_ptr);
        std::string path = canonical_path(program_path + rel_path);

        if (appid) {
            if (path.size()) {
                PRINT_DEBUG("Adding app path: %u|%s|", appid, path.c_str());
                settings_client->setAppInstallPath(appid, path);
                settings_server->setAppInstallPath(appid, path);
            } else {
                PRINT_DEBUG("Error adding app path for: %u does this path exist? |%s|", appid, rel_path.c_str());
            }
        }
    }
}

// leaderboards.txt
static void parse_leaderboards(class Settings *settings_client, class Settings *settings_server)
{
    std::string dlc_config_path = Local_Storage::get_game_settings_path() + "leaderboards.txt";
    std::ifstream input( std::filesystem::u8path(dlc_config_path) );
    if (input.is_open()) {
        common_helpers::consume_bom(input);

        for( std::string line; getline( input, line ); ) {
            if (!line.empty() && line[line.length()-1] == '\n') {
                line.pop_back();
            }

            if (!line.empty() && line[line.length()-1] == '\r') {
                line.pop_back();
            }

            std::string leaderboard{};
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
                PRINT_DEBUG("Adding leaderboard: %s|%u|%u", leaderboard.c_str(), sort_method, display_type);
                settings_client->setLeaderboard(leaderboard, (ELeaderboardSortMethod)sort_method, (ELeaderboardDisplayType)display_type);
                settings_server->setLeaderboard(leaderboard, (ELeaderboardSortMethod)sort_method, (ELeaderboardDisplayType)display_type);
            } else {
                PRINT_DEBUG("Error adding leaderboard for: '%s', are sort method %u or display type %u valid?", leaderboard.c_str(), sort_method, display_type);
            }
        }
    }

}

// stats.txt
static void parse_stats(class Settings *settings_client, class Settings *settings_server)
{
    std::string stats_config_path = Local_Storage::get_game_settings_path() + "stats.txt";
    std::ifstream input( std::filesystem::u8path(stats_config_path) );
    if (input.is_open()) {
        common_helpers::consume_bom(input);
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
                    config.type = GameServerStats_Messages::StatInfo::STAT_TYPE_FLOAT;
                    config.default_value_float = std::stof(stat_default_value);
                } else if (stat_type == "int") {
                    config.type = GameServerStats_Messages::StatInfo::STAT_TYPE_INT;
                    config.default_value_int = std::stol(stat_default_value);
                } else if (stat_type == "avgrate") {
                    config.type = GameServerStats_Messages::StatInfo::STAT_TYPE_AVGRATE;
                    config.default_value_float = std::stof(stat_default_value);
                } else {
                    PRINT_DEBUG("Error adding stat %s, type %s isn't valid", stat_name.c_str(), stat_type.c_str());
                    continue;
                }
            } catch (...) {
                PRINT_DEBUG("Error adding stat %s, default value %s isn't valid", stat_name.c_str(), stat_default_value.c_str());
                continue;
            }

            if (stat_name.size()) {
                PRINT_DEBUG("Adding stat type: %s|%u|%f|%i", stat_name.c_str(), config.type, config.default_value_float, config.default_value_int);
                settings_client->setStatDefiniton(stat_name, config);
                settings_server->setStatDefiniton(stat_name, config);
            } else {
                PRINT_DEBUG("Error adding stat for: %s, empty name", stat_name.c_str());
            }
        }
    }

}

// depots.txt
static void parse_depots(class Settings *settings_client, class Settings *settings_server)
{
    std::string depots_config_path = Local_Storage::get_game_settings_path() + "depots.txt";
    std::ifstream input( std::filesystem::u8path(depots_config_path) );
    if (input.is_open()) {
        common_helpers::consume_bom(input);
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
                PRINT_DEBUG("Added depot %u", depot_id);
            } catch (...) {}
        }
    }

}

// subscribed_groups.txt
static void parse_subscribed_groups(class Settings *settings_client, class Settings *settings_server)
{
    std::string depots_config_path = Local_Storage::get_game_settings_path() + "subscribed_groups.txt";
    std::ifstream input( std::filesystem::u8path(depots_config_path) );
    if (input.is_open()) {
        common_helpers::consume_bom(input);
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
                PRINT_DEBUG("Added source %llu", source_id);
            } catch (...) {}
        }
    }

}

// installed_app_ids.txt
static void parse_installed_app_Ids(class Settings *settings_client, class Settings *settings_server)
{
    std::string installed_apps_list_path = Local_Storage::get_game_settings_path() + "installed_app_ids.txt";
    std::ifstream input( std::filesystem::u8path(installed_apps_list_path) );
    if (input.is_open()) {
        settings_client->assumeAnyAppInstalled(false);
        settings_server->assumeAnyAppInstalled(false);
        PRINT_DEBUG("Limiting/Locking installed apps");
        common_helpers::consume_bom(input);
        for( std::string line; getline( input, line ); ) {
            if (!line.empty() && line[line.length()-1] == '\n') {
                line.pop_back();
            }

            if (!line.empty() && line[line.length()-1] == '\r') {
                line.pop_back();
            }

            try {
                AppId_t app_id = std::stoul(line);
                settings_client->addInstalledApp(app_id);
                settings_server->addInstalledApp(app_id);
                PRINT_DEBUG("Added installed app with ID %u", app_id);
            } catch (...) {}
        }
    } else {
        settings_client->assumeAnyAppInstalled(true);
        settings_server->assumeAnyAppInstalled(true);
        PRINT_DEBUG("Assuming any app is installed");
    }

}



// steam_settings/mods
static const auto one_week_ago_epoch = std::chrono::duration_cast<std::chrono::seconds>(
    ( startup_time - std::chrono::hours(24 * 7) ).time_since_epoch()
).count();
static const auto two_week_ago_epoch = std::chrono::duration_cast<std::chrono::seconds>(
    ( startup_time - std::chrono::hours(24 * 7 * 2) ).time_since_epoch()
).count();
static const auto three_week_ago_epoch = std::chrono::duration_cast<std::chrono::seconds>(
    ( startup_time - std::chrono::hours(24 * 7 * 3) ).time_since_epoch()
).count();

static size_t get_file_size_safe(const std::string &filepath, const std::string &basepath, int32 default_val = 0)
{
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
}

static std::string get_mod_preview_url(const std::string &previewFileName, const std::string &mod_id)
{
    if (previewFileName.empty()) {
        return std::string();
    } else {
        auto settings_folder = std::string(Local_Storage::get_game_settings_path());
        std::replace(settings_folder.begin(), settings_folder.end(), '\\', '/');
        
        return

#if defined(__WINDOWS__)
            "file:///"
#else // on Linux absolute paths start like this: /my/path, so the 3rd slash is already appended
            "file://"
#endif

            + settings_folder + "mod_images/" + mod_id + "/" + previewFileName;
    }
    
}

static void try_parse_mods_file(class Settings *settings_client, Settings *settings_server, nlohmann::json &mod_items, const std::string &mods_folder)
{
    for (auto mod = mod_items.begin(); mod != mod_items.end(); ++mod) {
        try {
            std::string mod_images_fullpath = Local_Storage::get_game_settings_path() + "mod_images" + PATH_SEPARATOR + std::string(mod.key());
            Mod_entry newMod;
            newMod.id = std::stoull(mod.key());
            newMod.title = mod.value().value("title", std::string(mod.key()));

            // make sure this is never empty
            newMod.path = mod.value().value("path", std::string(""));
            if (newMod.path.empty()) {
                newMod.path = mods_folder + PATH_SEPARATOR + std::string(mod.key());
            } else {
                // make sure the path is normalized for current OS, and absolute
                newMod.path = common_helpers::to_absolute(
                    newMod.path,
                    get_full_program_path()
                );
            }

            newMod.fileType = k_EWorkshopFileTypeCommunity;
            newMod.description = mod.value().value("description", std::string(""));
            newMod.steamIDOwner = mod.value().value("steam_id_owner", settings_client->get_local_steam_id().ConvertToUint64());
            newMod.timeCreated = mod.value().value("time_created", (uint32)three_week_ago_epoch);
            newMod.timeUpdated = mod.value().value("time_updated", (uint32)two_week_ago_epoch);
            newMod.timeAddedToUserList = mod.value().value("time_added", (uint32)one_week_ago_epoch);
            newMod.visibility = k_ERemoteStoragePublishedFileVisibilityPublic;
            newMod.banned = false;
            newMod.acceptedForUse = true;
            newMod.tagsTruncated = false;
            newMod.tags = mod.value().value("tags", std::string(""));

            newMod.primaryFileName = mod.value().value("primary_filename", std::string(""));
            int32 primary_filesize = 0;
            if (!newMod.primaryFileName.empty()) {
                primary_filesize = (int32)get_file_size_safe(newMod.primaryFileName, newMod.path, primary_filesize);
            }
            newMod.primaryFileSize = mod.value().value("primary_filesize", primary_filesize);
            
            newMod.previewFileName = mod.value().value("preview_filename", std::string(""));
            int32 preview_filesize = 0;
            if (!newMod.previewFileName.empty()) {
                preview_filesize = (int32)get_file_size_safe(newMod.previewFileName, mod_images_fullpath, preview_filesize);
            }
            newMod.previewFileSize = mod.value().value("preview_filesize", preview_filesize);

            newMod.total_files_sizes = mod.value().value("total_files_sizes", newMod.primaryFileSize);
            newMod.min_game_branch = mod.value().value("min_game_branch", "");
            newMod.max_game_branch = mod.value().value("max_game_branch", "");
            
            newMod.workshopItemURL = mod.value().value("workshop_item_url", "https://steamcommunity.com/sharedfiles/filedetails/?id=" + std::string(mod.key()));
            newMod.votesUp = mod.value().value("upvotes", (uint32)500);
            newMod.votesDown = mod.value().value("downvotes", (uint32)12);

            float score = 0.97f;
            try
            {
                score = newMod.votesUp / (float)(newMod.votesUp + newMod.votesDown);
            } catch(...) {}
            newMod.score = mod.value().value("score", score);
            
            newMod.numChildren = mod.value().value("num_children", (uint32)0);
            newMod.previewURL = mod.value().value("preview_url", get_mod_preview_url(newMod.previewFileName, std::string(mod.key())));
            
            settings_client->addMod(newMod.id, newMod.title, newMod.path);
            settings_server->addMod(newMod.id, newMod.title, newMod.path);
            settings_client->addModDetails(newMod.id, newMod);
            settings_server->addModDetails(newMod.id, newMod);
            
            PRINT_DEBUG("  parsed mod '%s':", std::string(mod.key()).c_str());
            PRINT_DEBUG("    path (will be used for primary file): '%s'", newMod.path.c_str());
            PRINT_DEBUG("    images path (will be used for preview file): '%s'", mod_images_fullpath.c_str());
            PRINT_DEBUG("    primary_filename: '%s'", newMod.primaryFileName.c_str());
            PRINT_DEBUG("    primary_filesize: %i bytes", newMod.primaryFileSize);
            PRINT_DEBUG("    primary file handle: %llu", settings_client->getMod(newMod.id).handleFile);
            PRINT_DEBUG("    preview_filename: '%s'", newMod.previewFileName.c_str());
            PRINT_DEBUG("    preview_filesize: %i bytes", newMod.previewFileSize);
            PRINT_DEBUG("    preview file handle: %llu", settings_client->getMod(newMod.id).handlePreviewFile);
            PRINT_DEBUG("    total_files_sizes: %llu", settings_client->getMod(newMod.id).total_files_sizes);
            PRINT_DEBUG("    min_game_branch: '%s'", settings_client->getMod(newMod.id).min_game_branch.c_str());
            PRINT_DEBUG("    max_game_branch: '%s'", settings_client->getMod(newMod.id).max_game_branch.c_str());
            PRINT_DEBUG("    workshop_item_url: '%s'", newMod.workshopItemURL.c_str());
            PRINT_DEBUG("    preview_url: '%s'", newMod.previewURL.c_str());
        } catch (std::exception& e) {
            const char *errorMessage = e.what();
            PRINT_DEBUG("MODLOADER ERROR: %s", errorMessage);
        }
    }
}

// called if mods.json doesn't exist or invalid
static void try_detect_mods_folder(class Settings *settings_client, Settings *settings_server, const std::string &mods_folder)
{
    std::vector<std::string> all_mods = Local_Storage::get_folders_path(mods_folder);
    for (auto & mod_folder: all_mods) {
        std::string mod_images_fullpath = Local_Storage::get_game_settings_path() + "mod_images" + PATH_SEPARATOR + mod_folder;
        try {
            Mod_entry newMod;
            newMod.id = std::stoull(mod_folder);
            newMod.title = mod_folder;

            // make sure this is never empty
            newMod.path = mods_folder + PATH_SEPARATOR + mod_folder;

            newMod.fileType = k_EWorkshopFileTypeCommunity;
            newMod.description = "mod #" + mod_folder;
            newMod.steamIDOwner = settings_client->get_local_steam_id().ConvertToUint64();
            newMod.timeCreated = (uint32)three_week_ago_epoch;
            newMod.timeUpdated = (uint32)two_week_ago_epoch;
            newMod.timeAddedToUserList = (uint32)one_week_ago_epoch;
            newMod.visibility = k_ERemoteStoragePublishedFileVisibilityPublic;
            newMod.banned = false;
            newMod.acceptedForUse = true;
            newMod.tagsTruncated = false;
            newMod.tags = "";

            std::vector<std::string> mod_primary_files = Local_Storage::get_filenames_path(newMod.path);
            newMod.primaryFileName = mod_primary_files.size() ? mod_primary_files[0] : "";
            newMod.primaryFileSize = (int32)get_file_size_safe(newMod.primaryFileName, newMod.path);

            std::vector<std::string> mod_preview_files = Local_Storage::get_filenames_path(mod_images_fullpath);
            newMod.previewFileName = mod_preview_files.size() ? mod_preview_files[0] : "";
            newMod.previewFileSize = (int32)get_file_size_safe(newMod.previewFileName, mod_images_fullpath);

            newMod.total_files_sizes = newMod.primaryFileSize;

            newMod.workshopItemURL =  "https://steamcommunity.com/sharedfiles/filedetails/?id=" + mod_folder;
            newMod.votesUp = (uint32)500;
            newMod.votesDown = (uint32)12;
            newMod.score = 0.97f;
            newMod.numChildren = (uint32)0;
            newMod.previewURL = get_mod_preview_url(newMod.previewFileName, mod_folder);
            
            settings_client->addMod(newMod.id, newMod.title, newMod.path);
            settings_server->addMod(newMod.id, newMod.title, newMod.path);
            settings_client->addModDetails(newMod.id, newMod);
            settings_server->addModDetails(newMod.id, newMod);
            PRINT_DEBUG("  detected mod '%s':", mod_folder.c_str());
            PRINT_DEBUG("    path (will be used for primary file): '%s'", newMod.path.c_str());
            PRINT_DEBUG("    images path (will be used for preview file): '%s'", mod_images_fullpath.c_str());
            PRINT_DEBUG("    primary_filename: '%s'", newMod.primaryFileName.c_str());
            PRINT_DEBUG("    primary_filesize: %i bytes", newMod.primaryFileSize);
            PRINT_DEBUG("    primary file handle: %llu", settings_client->getMod(newMod.id).handleFile);
            PRINT_DEBUG("    preview_filename: '%s'", newMod.previewFileName.c_str());
            PRINT_DEBUG("    preview_filesize: %i bytes", newMod.previewFileSize);
            PRINT_DEBUG("    preview file handle: %llu", settings_client->getMod(newMod.id).handlePreviewFile);
            PRINT_DEBUG("    total_files_sizes: '%llu'", newMod.total_files_sizes);
            PRINT_DEBUG("    min_game_branch: '%s'", newMod.min_game_branch.c_str());
            PRINT_DEBUG("    max_game_branch: '%s'", newMod.max_game_branch.c_str());
            PRINT_DEBUG("    workshop_item_url: '%s'", newMod.workshopItemURL.c_str());
            PRINT_DEBUG("    preview_url: '%s'", newMod.previewURL.c_str());
        } catch (...) {}
    }
}

static void parse_mods_folder(class Settings *settings_client, Settings *settings_server, class Local_Storage *local_storage)
{
    std::string mods_folder = Local_Storage::get_game_settings_path() + "mods";
    nlohmann::json mod_items = nlohmann::json::object();
    static constexpr auto mods_json_file = "mods.json";
    std::string mods_json_path = Local_Storage::get_game_settings_path() + mods_json_file;
    if (local_storage->load_json(mods_json_path, mod_items)) {
        PRINT_DEBUG("Attempting to parse mods.json");
        try_parse_mods_file(settings_client, settings_server, mod_items, mods_folder);
    } else { // invalid mods.json or doesn't exist
        PRINT_DEBUG("Failed to load mods.json, attempting to auto detect mods folder");
        try_detect_mods_folder(settings_client, settings_server, mods_folder);
    }

}



// main::general::crash_printer_location
static void parse_crash_printer_location()
{
    std::string line(common_helpers::string_strip(ini.GetValue("main::general", "crash_printer_location", "")));
    if (line.size()) {
        auto crash_path = common_helpers::to_absolute(line, get_full_program_path());
        if (crash_path.size()) {
            if (crash_printer::init(crash_path)) {
                PRINT_DEBUG("Unhandled crashes will be saved to '%s'", crash_path.c_str());
            } else {
                PRINT_DEBUG("Failed to setup unhandled crash printer with path: '%s'", crash_path.c_str());
            }
        }
    }
}

// auto_accept_invite.txt
static void parse_auto_accept_invite(class Settings *settings_client, class Settings *settings_server)
{
    std::string auto_accept_list_path = Local_Storage::get_game_settings_path() + "auto_accept_invite.txt";
    std::ifstream input( std::filesystem::u8path(auto_accept_list_path) );
    if (input.is_open()) {
        bool accept_any_invite = true;
        common_helpers::consume_bom(input);
        for( std::string line; getline( input, line ); ) {
            line = common_helpers::string_strip(line);
            if (!line.empty()) {
                accept_any_invite = false;
                try {
                    auto friend_id = std::stoull(line);
                    settings_client->addFriendToOverlayAutoAccept((uint64_t)friend_id);
                    settings_server->addFriendToOverlayAutoAccept((uint64_t)friend_id);
                    PRINT_DEBUG("Auto accepting overlay invitations from user with ID (SteamID64) = %llu", friend_id);
                } catch (...) {}
            }
        }

        if (accept_any_invite) {
            PRINT_DEBUG("Auto accepting any overlay invitation");
            settings_client->acceptAnyOverlayInvites(true);
            settings_server->acceptAnyOverlayInvites(true);
        } else {
            settings_client->acceptAnyOverlayInvites(false);
            settings_server->acceptAnyOverlayInvites(false);
        }
    }
}

// branches.json
static bool parse_branches_file(
    const std::string &base_path, const bool force_load,
    class Settings *settings_client, class Settings *settings_server, class Local_Storage *local_storage)
{
    static constexpr auto branches_json_file = "branches.json";

    std::vector<Branch_Info> result{};
    long public_branch_idx = -1;
    long user_branch_idx = -1;

    std::string branches_file = base_path + branches_json_file;
    auto branches = nlohmann::json{};
    if (!local_storage->load_json(branches_file, branches) && !force_load) {
        return false;
    }

    settings_client->is_beta_branch = ini.GetBoolValue("app::general", "is_beta_branch", settings_client->is_beta_branch);
    settings_server->is_beta_branch = ini.GetBoolValue("app::general", "is_beta_branch", settings_server->is_beta_branch);


    // app::general::branch_name
    std::string selected_branch = common_helpers::string_strip(ini.GetValue("app::general", "branch_name", ""));
    if (selected_branch.empty()) {
        selected_branch = "public";
        PRINT_DEBUG("no branch name specified, defaulting to 'public'");
    }
    PRINT_DEBUG("selected branch name '%s'", selected_branch.c_str());

    PRINT_DEBUG("loaded %zu branches from file '%s'", branches.size(), branches_file.c_str());
    auto current_epoch = (uint32)std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    for (const auto &branch_data : branches) {
        auto &new_banch = result.emplace_back(Branch_Info{});
        
        new_banch.name = branch_data.value("name", new_banch.name);
        new_banch.description = branch_data.value("description", new_banch.description);
        new_banch.branch_protected = branch_data.value("protected", new_banch.branch_protected);
        new_banch.build_id = branch_data.value("build_id", new_banch.build_id);
        new_banch.time_updated_epoch = branch_data.value("time_updated", new_banch.time_updated_epoch);
        
        new_banch.flags = EBetaBranchFlags::k_EBetaBranch_Available;
        if (new_banch.branch_protected) {
            new_banch.flags = static_cast<EBetaBranchFlags>(new_banch.flags | EBetaBranchFlags::k_EBetaBranch_Private);
        }
        if (public_branch_idx < 0 && common_helpers::str_cmp_insensitive("public", new_banch.name)) {
            public_branch_idx = static_cast<long>(result.size() - 1);
            PRINT_DEBUG("found default 'public' branch [%li]", public_branch_idx);
        }
        if (user_branch_idx < 0 && common_helpers::str_cmp_insensitive(selected_branch, new_banch.name)) {
            user_branch_idx = static_cast<long>(result.size() - 1);
            PRINT_DEBUG("found your branch '%s' [%li]", selected_branch.c_str(), user_branch_idx);
        }

        PRINT_DEBUG("added branch '%s'", new_banch.name.c_str());
        PRINT_DEBUG("    description '%s'", new_banch.description.c_str());
        PRINT_DEBUG("    branch_protected %i", (int)new_banch.branch_protected);
        PRINT_DEBUG("    build_id %u", new_banch.build_id);
        PRINT_DEBUG("    time_updated_epoch %u", new_banch.time_updated_epoch);
    }

    if (public_branch_idx < 0) {
        PRINT_DEBUG("[?] 'public' branch not found, adding it");
        auto &public_branch = result.emplace_back(Branch_Info{});
        public_branch_idx = static_cast<long>(result.size() - 1);
    }

    if (user_branch_idx < 0) {
        PRINT_DEBUG("[?] selected branch '%s' wasn't loaded, forcing selection to the default 'public'", selected_branch.c_str());
        user_branch_idx = public_branch_idx;
    }

    {
        auto& public_branch = result[public_branch_idx];
        public_branch.name = "public";
        public_branch.flags = static_cast<EBetaBranchFlags>(public_branch.flags | EBetaBranchFlags::k_EBetaBranch_Default | EBetaBranchFlags::k_EBetaBranch_Available);
    }

    {
        auto& user_branch = result[user_branch_idx];
        user_branch.active = true;
        user_branch.flags = static_cast<EBetaBranchFlags>(user_branch.flags | EBetaBranchFlags::k_EBetaBranch_Available | EBetaBranchFlags::k_EBetaBranch_Installed | EBetaBranchFlags::k_EBetaBranch_Selected);
    }

    settings_client->branches = result;
    settings_server->branches = result;

    settings_client->selected_branch_idx = user_branch_idx;
    settings_server->selected_branch_idx = user_branch_idx;

    PRINT_DEBUG("selected branch index in the list [%li]", user_branch_idx);

    return true;
}

// user::general::ip_country
static void parse_ip_country(class Local_Storage *local_storage, class Settings *settings_client, class Settings *settings_server)
{
    std::string line(common_helpers::to_upper(common_helpers::string_strip(ini.GetValue("user::general", "ip_country", ""))));
    if (line.empty()) {
            line = DEFAULT_IP_COUNTRY;
            save_global_ini_value(
                local_storage,
                config_ini_user,
                "user::general", "ip_country", IniValue(line.c_str()),
                "ISO 3166-1-alpha-2 format, use this link to get the 'Alpha-2' country code: https://www.iban.com/country-codes"
            );
    }

    // ISO 3166-1-alpha-2 format is 2 chars only
    if (line.size() == 2) {
        settings_client->ip_country = line;
        settings_server->ip_country = line;
        PRINT_DEBUG("Setting IP country to: '%s'", line.c_str());
    }
}

// overlay::general
static void parse_overlay_general_config(class Settings *settings_client, class Settings *settings_server)
{
    settings_client->disable_overlay = !ini.GetBoolValue("overlay::general", "enable_experimental_overlay", !settings_client->disable_overlay);
    settings_server->disable_overlay = !ini.GetBoolValue("overlay::general", "enable_experimental_overlay", !settings_server->disable_overlay);

    {
        auto val = ini.GetLongValue("overlay::general", "hook_delay_sec", -1);
        if (val >= 0) {
        settings_client->overlay_hook_delay_sec = val;
        settings_server->overlay_hook_delay_sec = val;
        PRINT_DEBUG("Setting overlay hook delay to %i seconds", (int)val);
    }
    }

    {
        auto val = ini.GetLongValue("overlay::general", "renderer_detector_timeout_sec", -1);
        if (val > 0) {
        settings_client->overlay_renderer_detector_timeout_sec = val;
        settings_server->overlay_renderer_detector_timeout_sec = val;
        PRINT_DEBUG("Setting overlay renderer detector timeout to %i seconds", (int)val);
    }
    }

    settings_client->disable_overlay_achievement_notification = ini.GetBoolValue("overlay::general", "disable_achievement_notification", settings_client->disable_overlay_achievement_notification);
    settings_server->disable_overlay_achievement_notification = ini.GetBoolValue("overlay::general", "disable_achievement_notification", settings_server->disable_overlay_achievement_notification);

    settings_client->disable_overlay_friend_notification = ini.GetBoolValue("overlay::general", "disable_friend_notification", settings_client->disable_overlay_friend_notification);
    settings_server->disable_overlay_friend_notification = ini.GetBoolValue("overlay::general", "disable_friend_notification", settings_server->disable_overlay_friend_notification);

    settings_client->disable_overlay_achievement_progress = ini.GetBoolValue("overlay::general", "disable_achievement_progress", settings_client->disable_overlay_achievement_progress);
    settings_server->disable_overlay_achievement_progress = ini.GetBoolValue("overlay::general", "disable_achievement_progress", settings_server->disable_overlay_achievement_progress);

    settings_client->disable_overlay_warning_any = ini.GetBoolValue("overlay::general", "disable_warning_any", settings_client->disable_overlay_warning_any);
    settings_server->disable_overlay_warning_any = ini.GetBoolValue("overlay::general", "disable_warning_any", settings_server->disable_overlay_warning_any);

    settings_client->disable_overlay_warning_bad_appid = ini.GetBoolValue("overlay::general", "disable_warning_bad_appid", settings_client->disable_overlay_warning_bad_appid);
    settings_server->disable_overlay_warning_bad_appid = ini.GetBoolValue("overlay::general", "disable_warning_bad_appid", settings_server->disable_overlay_warning_bad_appid);

    settings_client->disable_overlay_warning_local_save = ini.GetBoolValue("overlay::general", "disable_warning_local_save", settings_client->disable_overlay_warning_local_save);
    settings_server->disable_overlay_warning_local_save = ini.GetBoolValue("overlay::general", "disable_warning_local_save", settings_server->disable_overlay_warning_local_save);

    settings_client->overlay_upload_achs_icons_to_gpu = ini.GetBoolValue("overlay::general", "upload_achievements_icons_to_gpu", settings_client->overlay_upload_achs_icons_to_gpu);
    settings_server->overlay_upload_achs_icons_to_gpu = ini.GetBoolValue("overlay::general", "upload_achievements_icons_to_gpu", settings_server->overlay_upload_achs_icons_to_gpu);

    settings_client->overlay_always_show_user_info = ini.GetBoolValue("overlay::general", "overlay_always_show_user_info", settings_client->overlay_always_show_user_info);
    settings_server->overlay_always_show_user_info = ini.GetBoolValue("overlay::general", "overlay_always_show_user_info", settings_server->overlay_always_show_user_info);

    settings_client->overlay_always_show_fps = ini.GetBoolValue("overlay::general", "overlay_always_show_fps", settings_client->overlay_always_show_fps);
    settings_server->overlay_always_show_fps = ini.GetBoolValue("overlay::general", "overlay_always_show_fps", settings_server->overlay_always_show_fps);
    
    settings_client->overlay_always_show_frametime = ini.GetBoolValue("overlay::general", "overlay_always_show_frametime", settings_client->overlay_always_show_frametime);
    settings_server->overlay_always_show_frametime = ini.GetBoolValue("overlay::general", "overlay_always_show_frametime", settings_server->overlay_always_show_frametime);

    settings_client->overlay_always_show_playtime = ini.GetBoolValue("overlay::general", "overlay_always_show_playtime", settings_client->overlay_always_show_playtime);
    settings_server->overlay_always_show_playtime = ini.GetBoolValue("overlay::general", "overlay_always_show_playtime", settings_server->overlay_always_show_playtime);

    {
        auto val = ini.GetLongValue("overlay::general", "fps_averaging_window", settings_client->overlay_fps_avg_window);
        if (val > 0) {
            settings_client->overlay_fps_avg_window = val;        
        }
    }

    {
        auto val = ini.GetLongValue("overlay::general", "fps_averaging_window", settings_server->overlay_fps_avg_window);
        if (val > 0) {
            settings_server->overlay_fps_avg_window = val;        
        }
    }
    
}

// main::misc::steam_game_stats_reports_dir
static void parse_steam_game_stats_reports_dir(class Settings *settings_client, class Settings *settings_server)
{
    std::string line(common_helpers::string_strip(ini.GetValue("main::misc", "steam_game_stats_reports_dir", "")));
    if (line.size()) {
        auto folder = common_helpers::to_absolute(line, get_full_program_path());
        if (folder.size()) {
            PRINT_DEBUG("ISteamGameStats reports will be saved to '%s'", folder.c_str());
            settings_client->steam_game_stats_reports_dir = folder;
            settings_server->steam_game_stats_reports_dir = folder;
        }
    }
}

// mainly enable/disable features
static void parse_simple_features(class Settings *settings_client, class Settings *settings_server)
{
    // [main::general]
    settings_client->enable_new_app_ticket = ini.GetBoolValue("main::general", "new_app_ticket", settings_client->enable_new_app_ticket);
    settings_server->enable_new_app_ticket = ini.GetBoolValue("main::general", "new_app_ticket", settings_server->enable_new_app_ticket);

    settings_client->use_gc_token = ini.GetBoolValue("main::general", "gc_token", settings_client->use_gc_token);
    settings_server->use_gc_token = ini.GetBoolValue("main::general", "gc_token", settings_server->use_gc_token);

    settings_client->disable_account_avatar = !ini.GetBoolValue("main::general", "enable_account_avatar", !settings_client->disable_account_avatar);
    settings_server->disable_account_avatar = !ini.GetBoolValue("main::general", "enable_account_avatar", !settings_server->disable_account_avatar);

    settings_client->steam_deck = ini.GetBoolValue("main::general", "steam_deck", settings_client->steam_deck);
    settings_server->steam_deck = ini.GetBoolValue("main::general", "steam_deck", settings_server->steam_deck);

    settings_client->immediate_gameserver_stats = ini.GetBoolValue("main::general", "immediate_gameserver_stats", settings_client->immediate_gameserver_stats);
    settings_server->immediate_gameserver_stats = ini.GetBoolValue("main::general", "immediate_gameserver_stats", settings_server->immediate_gameserver_stats);

    settings_client->matchmaking_server_details_via_source_query = ini.GetBoolValue("main::general", "matchmaking_server_details_via_source_query", settings_client->matchmaking_server_details_via_source_query);
    settings_server->matchmaking_server_details_via_source_query = ini.GetBoolValue("main::general", "matchmaking_server_details_via_source_query", settings_server->matchmaking_server_details_via_source_query);

    settings_client->matchmaking_server_list_always_lan_type = !ini.GetBoolValue("main::general", "matchmaking_server_list_actual_type", !settings_client->matchmaking_server_list_always_lan_type);
    settings_server->matchmaking_server_list_always_lan_type = !ini.GetBoolValue("main::general", "matchmaking_server_list_actual_type", !settings_server->matchmaking_server_list_always_lan_type);


    // [main::connectivity]
    settings_client->disable_networking = ini.GetBoolValue("main::connectivity", "disable_networking", settings_client->disable_networking);
    settings_server->disable_networking = ini.GetBoolValue("main::connectivity", "disable_networking", settings_server->disable_networking);

    settings_client->disable_sharing_stats_with_gameserver = ini.GetBoolValue("main::connectivity", "disable_sharing_stats_with_gameserver", settings_client->disable_sharing_stats_with_gameserver);
    settings_server->disable_sharing_stats_with_gameserver = ini.GetBoolValue("main::connectivity", "disable_sharing_stats_with_gameserver", settings_server->disable_sharing_stats_with_gameserver);
    
    settings_client->disable_source_query = ini.GetBoolValue("main::connectivity", "disable_source_query", settings_client->disable_source_query);
    settings_server->disable_source_query = ini.GetBoolValue("main::connectivity", "disable_source_query", settings_server->disable_source_query);

    settings_client->share_leaderboards_over_network = ini.GetBoolValue("main::connectivity", "share_leaderboards_over_network", settings_client->share_leaderboards_over_network);
    settings_server->share_leaderboards_over_network = ini.GetBoolValue("main::connectivity", "share_leaderboards_over_network", settings_server->share_leaderboards_over_network);

    settings_client->disable_lobby_creation = ini.GetBoolValue("main::connectivity", "disable_lobby_creation", settings_client->disable_lobby_creation);
    settings_server->disable_lobby_creation = ini.GetBoolValue("main::connectivity", "disable_lobby_creation", settings_server->disable_lobby_creation);

    settings_client->download_steamhttp_requests = ini.GetBoolValue("main::connectivity", "download_steamhttp_requests", settings_client->download_steamhttp_requests);
    settings_server->download_steamhttp_requests = ini.GetBoolValue("main::connectivity", "download_steamhttp_requests", settings_server->download_steamhttp_requests);


    // [main::misc]
    settings_client->achievement_bypass = ini.GetBoolValue("main::misc", "achievements_bypass", settings_client->achievement_bypass);
    settings_server->achievement_bypass = ini.GetBoolValue("main::misc", "achievements_bypass", settings_server->achievement_bypass);

    settings_client->force_steamhttp_success = ini.GetBoolValue("main::misc", "force_steamhttp_success", settings_client->force_steamhttp_success);
    settings_server->force_steamhttp_success = ini.GetBoolValue("main::misc", "force_steamhttp_success", settings_server->force_steamhttp_success);

    settings_client->disable_steamoverlaygameid_env_var = ini.GetBoolValue("main::misc", "disable_steamoverlaygameid_env_var", settings_client->disable_steamoverlaygameid_env_var);
    settings_server->disable_steamoverlaygameid_env_var = ini.GetBoolValue("main::misc", "disable_steamoverlaygameid_env_var", settings_server->disable_steamoverlaygameid_env_var);

    settings_client->enable_builtin_preowned_ids = ini.GetBoolValue("main::misc", "enable_steam_preowned_ids", settings_client->enable_builtin_preowned_ids);
    settings_server->enable_builtin_preowned_ids = ini.GetBoolValue("main::misc", "enable_steam_preowned_ids", settings_server->enable_builtin_preowned_ids);
}

// [main::stats]
static void parse_stats_features(class Settings *settings_client, class Settings *settings_server)
{
    settings_client->disable_leaderboards_create_unknown = ini.GetBoolValue("main::stats", "disable_leaderboards_create_unknown", settings_client->disable_leaderboards_create_unknown);
    settings_server->disable_leaderboards_create_unknown = ini.GetBoolValue("main::stats", "disable_leaderboards_create_unknown", settings_server->disable_leaderboards_create_unknown);

    settings_client->allow_unknown_stats = ini.GetBoolValue("main::stats", "allow_unknown_stats", settings_client->allow_unknown_stats);
    settings_server->allow_unknown_stats = ini.GetBoolValue("main::stats", "allow_unknown_stats", settings_server->allow_unknown_stats);

    settings_client->stat_achievement_progress_functionality = ini.GetBoolValue("main::stats", "stat_achievement_progress_functionality", settings_client->stat_achievement_progress_functionality);
    settings_server->stat_achievement_progress_functionality = ini.GetBoolValue("main::stats", "stat_achievement_progress_functionality", settings_server->stat_achievement_progress_functionality);

    settings_client->save_only_higher_stat_achievement_progress = ini.GetBoolValue("main::stats", "save_only_higher_stat_achievement_progress", settings_client->save_only_higher_stat_achievement_progress);
    settings_server->save_only_higher_stat_achievement_progress = ini.GetBoolValue("main::stats", "save_only_higher_stat_achievement_progress", settings_server->save_only_higher_stat_achievement_progress);

    {
        long val_client = ini.GetLongValue("main::stats", "paginated_achievements_icons", settings_client->paginated_achievements_icons);
        settings_client->paginated_achievements_icons = static_cast<int>(val_client);

        long val_server = ini.GetLongValue("main::stats", "paginated_achievements_icons", settings_server->paginated_achievements_icons);
        settings_server->paginated_achievements_icons = static_cast<int>(val_server);
    }
}



static std::map<SettingsItf, std::string> old_itfs_map{};

static bool try_parse_old_steam_interfaces_file(std::string interfaces_path)
{
    std::ifstream input( std::filesystem::u8path(interfaces_path) );
    if (!input.is_open()) return false;

    PRINT_DEBUG("Trying to parse old steam interfaces from '%s'", interfaces_path.c_str());
    for( std::string line; std::getline( input, line ); ) {
        line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
        line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        line.erase(std::remove(line.begin(), line.end(), '\t'), line.end());
        PRINT_DEBUG("  valid line: |%s|", line.c_str());

#define OLD_ITF_LINE(istr, itype) {                 \
    if (line.find(istr) != std::string::npos) {     \
        old_itfs_map[itype] = line;                 \
        continue;                                   \
    }                                               \
}

        OLD_ITF_LINE("SteamClient", SettingsItf::CLIENT);

        // NOTE: you must try to read the one with the most characters first
        OLD_ITF_LINE("SteamGameServerStats", SettingsItf::GAMESERVER_STATS);
        OLD_ITF_LINE("SteamGameServer", SettingsItf::GAMESERVER);

        // NOTE: you must try to read the one with the most characters first
        OLD_ITF_LINE("SteamMatchMakingServers", SettingsItf::MATCHMAKING_SERVERS);
        OLD_ITF_LINE("SteamMatchMaking", SettingsItf::MATCHMAKING);

        OLD_ITF_LINE("SteamUser", SettingsItf::USER);
        OLD_ITF_LINE("SteamFriends", SettingsItf::FRIENDS);
        OLD_ITF_LINE("SteamUtils", SettingsItf::UTILS);
        OLD_ITF_LINE("STEAMUSERSTATS_INTERFACE_VERSION", SettingsItf::USER_STATS);
        OLD_ITF_LINE("STEAMAPPS_INTERFACE_VERSION", SettingsItf::APPS);
        OLD_ITF_LINE("SteamNetworking", SettingsItf::NETWORKING);
        OLD_ITF_LINE("STEAMREMOTESTORAGE_INTERFACE_VERSION", SettingsItf::REMOTE_STORAGE);
        OLD_ITF_LINE("STEAMSCREENSHOTS_INTERFACE_VERSION", SettingsItf::SCREENSHOTS);
        OLD_ITF_LINE("STEAMHTTP_INTERFACE_VERSION", SettingsItf::HTTP);
        OLD_ITF_LINE("STEAMUNIFIEDMESSAGES_INTERFACE_VERSION", SettingsItf::UNIFIED_MESSAGES);

        OLD_ITF_LINE("STEAMCONTROLLER_INTERFACE_VERSION", SettingsItf::CONTROLLER);
        OLD_ITF_LINE("SteamController", SettingsItf::CONTROLLER);

        OLD_ITF_LINE("STEAMUGC_INTERFACE_VERSION", SettingsItf::UGC);
        OLD_ITF_LINE("STEAMAPPLIST_INTERFACE_VERSION", SettingsItf::APPLIST);
        OLD_ITF_LINE("STEAMMUSIC_INTERFACE_VERSION", SettingsItf::MUSIC);
        OLD_ITF_LINE("STEAMMUSICREMOTE_INTERFACE_VERSION", SettingsItf::MUSIC_REMOTE);
        OLD_ITF_LINE("STEAMHTMLSURFACE_INTERFACE_VERSION", SettingsItf::HTML_SURFACE);
        OLD_ITF_LINE("STEAMINVENTORY_INTERFACE", SettingsItf::INVENTORY);
        OLD_ITF_LINE("STEAMVIDEO_INTERFACE", SettingsItf::VIDEO);
        OLD_ITF_LINE("SteamMasterServerUpdater", SettingsItf::MASTERSERVER_UPDATER);

#undef OLD_ITF_LINE

        PRINT_DEBUG("  NOT REPLACED |%s|", line.c_str());
    }

    return true;
}

static void parse_old_steam_interfaces()
{
    if (!try_parse_old_steam_interfaces_file(Local_Storage::get_game_settings_path() + "steam_interfaces.txt") &&
        !try_parse_old_steam_interfaces_file(Local_Storage::get_program_path() + "steam_interfaces.txt")) {
        PRINT_DEBUG("Couldn't load steam_interfaces.txt");
    }
}

static void load_all_config_settings()
{
    static std::recursive_mutex ini_mtx{};
    static bool loaded = false;
    
    std::lock_guard lck(ini_mtx);
    if (loaded) return;
    loaded = true;

    constexpr const static char* config_files[] = {
        config_ini_app,
        config_ini_main,
        config_ini_overlay,
        config_ini_user,
    };

    ini.SetUnicode();
    ini.SetSpaces(false);

    // we have to load the local one first, since it might change base saves_folder_name
    {
        CSimpleIniA local_ini{};
        local_ini.SetUnicode();

        for (const auto &config_file : config_files) {
            std::ifstream local_ini_file( std::filesystem::u8path(Local_Storage::get_game_settings_path() + config_file), std::ios::binary | std::ios::in);
            if (!local_ini_file.is_open()) continue;

            auto err = local_ini.LoadData(local_ini_file);
            local_ini_file.close();
            PRINT_DEBUG("result of parsing ini in local settings '%s' %i (success == 0)", config_file, (int)err);
            if (err == SI_OK) {
                merge_ini(local_ini);
            }
        }
        
        std::string saves_folder_name(common_helpers::string_strip(Settings::sanitize(local_ini.GetValue("user::saves", "saves_folder_name", ""))));
        if (saves_folder_name.size()) {
            Local_Storage::set_saves_folder_name(saves_folder_name);
            PRINT_DEBUG("changed base folder for save data to '%s'", saves_folder_name.c_str());
        }
    }

    std::string local_save_folder{};
    if (parse_local_save(local_save_folder) && local_save_folder.size()) {
        CSimpleIniA local_ini{};
        local_ini.SetUnicode();

        for (const auto &config_file : config_files) {
            std::ifstream local_ini_file( std::filesystem::u8path(local_save_folder + Local_Storage::settings_storage_folder + PATH_SEPARATOR + config_file), std::ios::binary | std::ios::in);
            if (!local_ini_file.is_open()) continue;

            auto err = local_ini.LoadData(local_ini_file);
            local_ini_file.close();
            PRINT_DEBUG("result of parsing ini in local save '%s' %i (success == 0)", config_file, (int)err);
            if (err == SI_OK) {
                merge_ini(local_ini, true);
            }
        }
        
        std::string saves_folder_name(common_helpers::string_strip(Settings::sanitize(local_ini.GetValue("user::saves", "saves_folder_name", ""))));
        if (saves_folder_name.size()) {
            Local_Storage::set_saves_folder_name(saves_folder_name);
            PRINT_DEBUG("changed base folder for save data to '%s'", saves_folder_name.c_str());
        }
        
        PRINT_DEBUG("global settings will be ignored since local save is being used");

    } else { // only read global folder if we're not using local save
        CSimpleIniA global_ini{};
        global_ini.SetUnicode();
        
        // now we can access get_user_appdata_path() which might have been changed by the above code
        for (const auto &config_file : config_files) {
            std::ifstream ini_file( std::filesystem::u8path(Local_Storage::get_user_appdata_path() + Local_Storage::settings_storage_folder + PATH_SEPARATOR + config_file), std::ios::binary | std::ios::in);
            if (!ini_file.is_open()) continue;
            
            auto err = global_ini.LoadData(ini_file);
            ini_file.close();
            PRINT_DEBUG("result of parsing global ini '%s' %i (success == 0)", config_file, (int)err);
            
            if (err == SI_OK) {
                merge_ini(global_ini);
            }
        }
    }

    parse_old_steam_interfaces();

#ifndef EMU_RELEASE_BUILD
    // dump the final ini file
    {
        PRINT_DEBUG("final ini start ---------");
        std::list<CSimpleIniA::Entry> sections{};
        ini.GetAllSections(sections);
        for (auto const &sec : sections) {
            PRINT_DEBUG("[%s]", sec.pItem);
            std::list<CSimpleIniA::Entry> keys{};
            ini.GetAllKeys(sec.pItem, keys);
            for (auto const &key : keys) {
                std::list<CSimpleIniA::Entry> vals{};
                ini.GetAllValues(sec.pItem, key.pItem, vals);
                for (const auto &val : vals) {
                    PRINT_DEBUG("%s=%s", key.pItem, val.pItem);
                }
            }
            PRINT_DEBUG("");
        }
        PRINT_DEBUG("final ini end *********");
    }
#endif // EMU_RELEASE_BUILD

    reset_LastError();

}


uint32 create_localstorage_settings(Settings **settings_client_out, Settings **settings_server_out, Local_Storage **local_storage_out)
{
    PRINT_DEBUG("start ----------");

    load_all_config_settings();

#if defined(EMU_BUILD_STRING)
    PRINT_DEBUG("emu build '%s'", EXPAND_AS_STR(EMU_BUILD_STRING));
#else
    PRINT_DEBUG("<unspecified emu build>");
#endif

    parse_crash_printer_location();

    const std::string program_path(Local_Storage::get_program_path());
    const std::string steam_settings_path(Local_Storage::get_game_settings_path());
    
    std::string save_path(Local_Storage::get_user_appdata_path());
    bool local_save = parse_local_save(save_path);
    PRINT_DEBUG("program path: '%s', base path for saves: '%s'", program_path.c_str(), save_path.c_str());

    Local_Storage *local_storage = new Local_Storage(save_path);
    PRINT_DEBUG("global settings path: '%s'", local_storage->get_global_settings_path().c_str());
    uint32 appid = parse_steam_app_id(program_path);
    local_storage->setAppId(appid);

    // Custom broadcasts
    std::set<IP_PORT> custom_broadcasts{};
    load_custom_broadcasts(local_storage->get_global_settings_path(), custom_broadcasts);
    load_custom_broadcasts(steam_settings_path, custom_broadcasts);

    // Listen port
    uint16 port = parse_listen_port(local_storage);
    // Acount name
    std::string name(parse_account_name(local_storage));
    // Steam ID
    CSteamID user_id = parse_user_steam_id(local_storage);
    // Language
    std::string language(parse_current_language(local_storage));
    // Supported languages, this will change the current language if needed
    std::set<std::string> supported_languages(parse_supported_languages(local_storage, language));

    bool steam_offline_mode = ini.GetBoolValue("main::connectivity", "offline", false);
    if (steam_offline_mode) {
        PRINT_DEBUG("setting emu to offline mode");
    }
    Settings *settings_client = new Settings(user_id, CGameID(appid), name, language, steam_offline_mode);
    Settings *settings_server = new Settings(generate_steam_id_server(), CGameID(appid), name, language, steam_offline_mode);

    // listen port
    settings_client->set_port(port);
    settings_server->set_port(port);
    // broadcasts list
    settings_client->custom_broadcasts = custom_broadcasts;
    settings_server->custom_broadcasts = custom_broadcasts;
    // overlay warning for local save
    settings_client->overlay_warn_local_save = local_save;
    settings_server->overlay_warn_local_save = local_save;
    // supported languages list
    settings_client->set_supported_languages(supported_languages);
    settings_server->set_supported_languages(supported_languages);

    parse_simple_features(settings_client, settings_server);
    parse_stats_features(settings_client, settings_server);

    parse_dlc(settings_client, settings_server);
    parse_installed_app_Ids(settings_client, settings_server);
    parse_app_paths(settings_client, settings_server, program_path);

    parse_leaderboards(settings_client, settings_server);
    parse_stats(settings_client, settings_server);
    parse_depots(settings_client, settings_server);
    parse_subscribed_groups(settings_client, settings_server);
    load_subscribed_groups_clans(local_storage->get_global_settings_path(), settings_client, settings_server);
    load_subscribed_groups_clans(steam_settings_path, settings_client, settings_server);

    parse_mods_folder(settings_client, settings_server, local_storage);
    load_gamecontroller_settings(settings_client);
    parse_auto_accept_invite(settings_client, settings_server);
    parse_ip_country(local_storage, settings_client, settings_server);
    
    // try local "steam_settings" then saves path, on second trial force load defaults
    if (!parse_branches_file(steam_settings_path, false, settings_client, settings_server, local_storage)) {
        parse_branches_file(local_storage->get_global_settings_path(), true, settings_client, settings_server, local_storage);
    }

    parse_overlay_general_config(settings_client, settings_server);
    load_overlay_appearance(settings_client, settings_server, local_storage);
    parse_steam_game_stats_reports_dir(settings_client, settings_server);

    *settings_client_out = settings_client;
    *settings_server_out = settings_server;
    *local_storage_out = local_storage;

    PRINT_DEBUG("end *********");
    reset_LastError();
    return appid;
}

void save_global_settings(class Local_Storage *local_storage, const char *name, const char *language)
{
    save_global_ini_value(
        local_storage,
        config_ini_user,
        "user::general", "account_name", IniValue(name),
        "user account name"
    );
    
    save_global_ini_value(
        local_storage,
        config_ini_user,
        "user::general", "language", IniValue(language),
        "the language reported to the game, default is 'english', check 'API language code' in https://partner.steamgames.com/doc/store/localization/languages"
    );
}

bool settings_disable_lan_only()
{
    load_all_config_settings();
    return ini.GetBoolValue("main::connectivity", "disable_lan_only", false);
}

const std::map<SettingsItf, std::string>& settings_old_interfaces()
{
    load_all_config_settings();
    return old_itfs_map;
}
