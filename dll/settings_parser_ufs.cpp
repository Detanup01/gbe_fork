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

#include "dll/settings_parser_ufs.h"
#include <unordered_map>
#include <functional>


//https://developer.valvesoftware.com/wiki/SteamID
// given the Steam64 format:
// [Universe "X": 8-bit] [Account type: 4-bit] [Account instance: 20-bit] [Account number: 30-bit] ["Y": 1-bit]
//
// "X represents the "Universe" the steam account belongs to."
// "Y is part of the ID number for the account. Y is either 0 or 1."
// "Z is the "account number")."
// "Let X, Y and Z constants be defined by the SteamID: STEAM_X:Y:Z"
// "W=Z*2+Y"
//
// W is the Steam3 number we want
static inline uint32 convert_to_steam3(CSteamID steamID)
{
    uint32 component_y = steamID.GetAccountID() & 0x01U;
    uint32 component_z = (steamID.GetAccountID() & 0xFFFFFFFEU) >> 1;
    return component_z * 2 + component_y;
}


static std::string iden_factory_Steam3AccountID(CSimpleIniA *ini, class Settings *settings_client, class Settings *settings_server, class Local_Storage *local_storage)
{
    return std::to_string(convert_to_steam3(settings_client->get_local_steam_id()));
}

static std::string iden_factory_64BitSteamID(CSimpleIniA *ini, class Settings *settings_client, class Settings *settings_server, class Local_Storage *local_storage)
{
    return std::to_string(settings_client->get_local_steam_id().ConvertToUint64());
}

static std::string iden_factory_gameinstall(CSimpleIniA *ini, class Settings *settings_client, class Settings *settings_server, class Local_Storage *local_storage)
{
    // should be: [Steam Install]\SteamApps\common\[Game Folder]
    auto str = Local_Storage::get_exe_dir();
    str.pop_back(); // we don't want the trailing backslash
    return str;
}

static std::string iden_factory_EmuSteamInstall(CSimpleIniA *ini, class Settings *settings_client, class Settings *settings_server, class Local_Storage *local_storage)
{
    auto steam_path = get_env_variable("SteamPath");
    if (steam_path.empty()) {
        steam_path = get_env_variable("InstallPath");
    }
    if (steam_path.empty()) {
        steam_path = Local_Storage::get_program_path();
    }
    if (steam_path.empty()) {
        steam_path = Local_Storage::get_exe_dir();
    }

    if (steam_path.empty()) {
        return {};
    }

    if (steam_path.size() > 1 && PATH_SEPARATOR[0] == steam_path.back()) {
        steam_path = steam_path.substr(0, steam_path.find_last_not_of(PATH_SEPARATOR) + 1);
    }
    return steam_path;
}


#if defined(__WINDOWS__)
// https://learn.microsoft.com/en-us/windows/win32/shell/knownfolderid
static inline std::string get_winSpecialFolder(REFKNOWNFOLDERID rfid)
{
    std::string path{};

    wchar_t *pszPath = nullptr;
    HRESULT hr = SHGetKnownFolderPath(rfid, 0, NULL, &pszPath);
    if (SUCCEEDED(hr) && pszPath != nullptr) {
        path = utf8_encode(pszPath);
    }

    CoTaskMemFree(pszPath);
    return path;
}

static std::string iden_factory_WinMyDocuments(CSimpleIniA *ini, class Settings *settings_client, class Settings *settings_server, class Local_Storage *local_storage)
{
    return get_winSpecialFolder(FOLDERID_Documents);
}

static std::string iden_factory_WinAppDataLocal(CSimpleIniA *ini, class Settings *settings_client, class Settings *settings_server, class Local_Storage *local_storage)
{
    return get_winSpecialFolder(FOLDERID_LocalAppData);
}

static std::string iden_factory_WinAppDataLocalLow(CSimpleIniA *ini, class Settings *settings_client, class Settings *settings_server, class Local_Storage *local_storage)
{
    return get_winSpecialFolder(FOLDERID_LocalAppDataLow);
}

static std::string iden_factory_WinAppDataRoaming(CSimpleIniA *ini, class Settings *settings_client, class Settings *settings_server, class Local_Storage *local_storage)
{
    return get_winSpecialFolder(FOLDERID_RoamingAppData);
}

static std::string iden_factory_WinSavedGames(CSimpleIniA *ini, class Settings *settings_client, class Settings *settings_server, class Local_Storage *local_storage)
{
    return get_winSpecialFolder(FOLDERID_SavedGames);
}

#else

typedef struct _t_nix_user_info {
    std::string name{};
    std::string home_dir{};
} t_nix_user_info;

static t_nix_user_info get_nix_user_info()
{
    t_nix_user_info res{};

    // https://linux.die.net/man/3/getpwuid_r
    // https://man7.org/linux/man-pages/man2/geteuid.2.html
    struct passwd pwd{};
    struct passwd* result = nullptr;
    char buffer[1024]{};
    auto ret = getpwuid_r(getuid(), &pwd, buffer, sizeof(buffer), &result);
    if (0 == ret && result != nullptr) {
        res.name = pwd.pw_name;
        res.home_dir = pwd.pw_dir;
    }

    return res;
}

static std::string get_nix_home()
{
    auto str = get_env_variable("HOME");
    if (str.empty()) {
        str = get_nix_user_info().home_dir;
    }

    if (!str.empty()) {
        if (str.size() > 1 && PATH_SEPARATOR[0] == str.back()) {
            str = str.substr(0, str.find_last_not_of(PATH_SEPARATOR) + 1);
        }
        return str;
    }
    return {};
}

static std::string iden_factory_LinuxHome(CSimpleIniA *ini, class Settings *settings_client, class Settings *settings_server, class Local_Storage *local_storage)
{
    return get_nix_home();
}

static std::string iden_factory_SteamCloudDocuments(CSimpleIniA *ini, class Settings *settings_client, class Settings *settings_server, class Local_Storage *local_storage)
{
    std::string home = get_nix_home();

    std::string username = settings_client->get_local_name();
    if (username.empty()) {
        username = get_nix_user_info().name;
    }
    if (username.empty()) {
        username = DEFAULT_NAME;
    }

    std::string game_folder = iden_factory_gameinstall(ini, settings_client, settings_server, local_storage);
    {
        auto last_sep = game_folder.rfind(PATH_SEPARATOR);
        if (last_sep != std::string::npos) {
            game_folder = game_folder.substr(last_sep + 1);
        }
    }

    return home + PATH_SEPARATOR + ".SteamCloud" + PATH_SEPARATOR + username + PATH_SEPARATOR + game_folder;
}

static std::string iden_factory_LinuxXdgDataHome(CSimpleIniA *ini, class Settings *settings_client, class Settings *settings_server, class Local_Storage *local_storage)
{
    // https://specifications.freedesktop.org/basedir-spec/latest/#variables
    auto datadir = get_env_variable("XDG_DATA_HOME");
    if (datadir.empty()) {
        auto homedir = get_nix_home();
        if (!homedir.empty()) {
            datadir = std::move(homedir) + PATH_SEPARATOR + ".local" + PATH_SEPARATOR + "share";
        }
    }

    if (datadir.size() > 1 && PATH_SEPARATOR[0] == datadir.back()) {
        datadir = datadir.substr(0, datadir.find_last_not_of(PATH_SEPARATOR) + 1);
    }
    return datadir;
}
#endif // __WINDOWS__

static std::unordered_map<
    std::string_view,
    std::function<std::string (CSimpleIniA*, class Settings*, class Settings*, class Local_Storage*)>
> identifiers_factories {
    { "{::Steam3AccountID::}", iden_factory_Steam3AccountID, },
    { "{::64BitSteamID::}", iden_factory_64BitSteamID, },
    { "{::gameinstall::}", iden_factory_gameinstall, },
    { "{::EmuSteamInstall::}", iden_factory_EmuSteamInstall, },

#if defined(__WINDOWS__)
    { "{::WinMyDocuments::}", iden_factory_WinMyDocuments, },
    { "{::WinAppDataLocal::}", iden_factory_WinAppDataLocal, },
    { "{::WinAppDataLocalLow::}", iden_factory_WinAppDataLocalLow, },
    { "{::WinAppDataRoaming::}", iden_factory_WinAppDataRoaming, },
    { "{::WinSavedGames::}", iden_factory_WinSavedGames, },
#else
    { "{::LinuxHome::}", iden_factory_LinuxHome, },
    { "{::SteamCloudDocuments::}", iden_factory_SteamCloudDocuments, },
    { "{::LinuxXdgDataHome::}", iden_factory_LinuxXdgDataHome, },
#endif // __WINDOWS__
};

static std::filesystem::path factory_default_cloud_dir(CSimpleIniA *ini, class Settings *settings_client, class Settings *settings_server, class Local_Storage *local_storage)
{
    auto steam_path = iden_factory_EmuSteamInstall(ini, settings_client, settings_server, local_storage);
    if (steam_path.empty()) {
        return {};
    }

    auto steam3_account_id = std::to_string(convert_to_steam3(settings_client->get_local_steam_id()));
    auto app_id = std::to_string(settings_client->get_local_game_id().AppID());
    return
        std::filesystem::u8path(steam_path)
        / std::filesystem::u8path("userdata")
        / std::filesystem::u8path(steam3_account_id)
        / std::filesystem::u8path(app_id)
        ;
}

// app::cloud_save
void parse_cloud_save(CSimpleIniA *ini, class Settings *settings_client, class Settings *settings_server, class Local_Storage *local_storage)
{
    constexpr static bool DEFAULT_CREATE_DEFAULT_DIR = true;
    constexpr static bool DEFAULT_CREATE_SPECIFIC_DIRS = true;
    constexpr static const char SPECIFIC_INI_KEY[] =
        "app::cloud_save::"
        // then concat the OS specific part
#if defined(__WINDOWS__)
        "win"
#else
        "linux"
#endif
        ;

    bool create_default_dir = ini->GetBoolValue("app::cloud_save::general", "create_default_dir", DEFAULT_CREATE_DEFAULT_DIR);
    if (create_default_dir) {
        auto default_cloud_dir = factory_default_cloud_dir(ini, settings_client, settings_server, local_storage);
        if (default_cloud_dir.empty()) {
            PRINT_DEBUG("[X] cannot resolve default cloud save dir");
        } else if (!std::filesystem::is_directory(default_cloud_dir)) {
            try {
                if (std::filesystem::create_directories(default_cloud_dir)) {
                    PRINT_DEBUG(
                        "successfully created default cloud save dir '%s'",
                        default_cloud_dir.u8string().c_str()
                    );
                }
            } catch (const std::filesystem::filesystem_error& e) {
                PRINT_DEBUG(
                    "[X] failed to create default cloud save dir '%s': '%s'",
                    default_cloud_dir.u8string().c_str(),
                    e.what()
                );
                // TODO crash the program here, or just print and move on?
            }
        }
    }

    bool create_specific_dirs = ini->GetBoolValue("app::cloud_save::general", "create_specific_dirs", DEFAULT_CREATE_SPECIFIC_DIRS);
    if (!create_specific_dirs) return;

    std::list<CSimpleIniA::Entry> specific_keys{};
    if (!ini->GetAllKeys(SPECIFIC_INI_KEY, specific_keys) || specific_keys.empty()) return;

    PRINT_DEBUG("processing all cloud save dirs under [%s]", SPECIFIC_INI_KEY);
    for (const auto &dir_key : specific_keys) {
        auto dirname_raw = ini->GetValue(SPECIFIC_INI_KEY, dir_key.pItem, "");
        if (!dirname_raw || !dirname_raw[0]) continue;

        // parse specific dir
        std::string dirname = dirname_raw;
        for (auto &[iden_name, iden_factory] : identifiers_factories) {
            auto iden_val = iden_factory(ini, settings_client, settings_server, local_storage);
            if (!iden_val.empty()) {
                dirname = common_helpers::str_replace_all(dirname, iden_name, iden_val);
            } else {
                PRINT_DEBUG("  [?] cannot resolve cloud save identifier '%s'", iden_name.data());
            }
        }

        PRINT_DEBUG("  parsed cloud save dir [%s]:\n  '%s'\n  ->\n  '%s'", dir_key.pItem, dirname_raw, dirname.c_str());

        // create specific dir
        if (common_helpers::str_find(dirname, "{::") == static_cast<size_t>(-1)) {
            auto dirname_p = std::filesystem::u8path(dirname);
            if (std::filesystem::is_directory(dirname_p) || std::filesystem::create_directories(dirname_p)) {
                PRINT_DEBUG("    successfully created cloud save dir");
            } else {
                PRINT_DEBUG("    [X] failed to create cloud save dir");
            }
        } else {
            PRINT_DEBUG("    [X] cloud save dir has unprocessed identifiers, skipping");
        }
    }
}
