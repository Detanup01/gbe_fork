// a Modified version of ColdClientLoader originally written by Rat431
// https://github.com/Rat431/ColdAPI_Steam/tree/master/src/ColdClientLoader

#include "common_helpers/common_helpers.hpp"
#include "pe_helpers/pe_helpers.hpp"
#include "dbg_log/dbg_log.hpp"

#define SI_CONVERT_GENERIC
#define SI_SUPPORT_IOSTREAMS
#define SI_NO_MBCS
#include "simpleini/SimpleIni.h"
#include "utfcpp/utf8.h"

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <stdio.h>
#include <string>
#include <string_view>
#include <filesystem>
#include <set>
#include <thread>
#include <algorithm>
#include <iterator>
#include <cstdio>
#include <cstdlib>
#include <sstream>


static CSimpleIniA local_ini{true};
static dbg_log logger{pe_helpers::get_current_exe_path() + pe_helpers::get_current_exe_name() + ".log"};
constexpr static const wchar_t STEAM_UNIVERSE[] = L"Public";
constexpr static const wchar_t STEAM_URL_PROTOCOL[] = L"URL:steam protocol";

const static bool loader_is_32 = pe_helpers::is_module_32(GetModuleHandleW(NULL));

const static std::wstring hklm_path(loader_is_32
    ? L"SOFTWARE\\Valve\\Steam"
    : L"SOFTWARE\\WOW6432Node\\Valve\\Steam"
);

// Declare some variables to be used for Steam registry.
const static DWORD UserId = 0x03100004771F810D & 0xffffffff;
const static DWORD ProcessID = GetCurrentProcessId();

static std::string IniFile{};
static std::string ClientPath{};
static std::string Client64Path{};
static std::string ExeFile{};
static std::string ExeRunDir{};
static std::string ExeCommandLine{};
static std::string AppId{};
static std::string DllsToInjectFolder{};

static bool ForceInjectSteamClient{};
static bool ForceInjectGameOverlayRenderer{};
static bool IgnoreInjectionError{};
static bool IgnoreLoaderArchDifference{};
static bool ResumeByDebugger{};

static long PersistentMode{};

static std::vector<uint8_t> exe_header{};
static std::vector<std::string> dlls_to_inject{};


static std::vector<uint8_t> get_pe_header(const std::string &filepath)
{
    try {
        std::ifstream file(std::filesystem::u8path(filepath), std::ios::in | std::ios::binary);
        if (!file.is_open()) throw;

        file.seekg(0, std::ios::beg);

        // 2MB is enough
        std::vector<uint8_t> data(2 * 1024 * 1024, 0);
        file.read((char *)&data[0], data.size());
        file.close();

        return data;
    } catch(const std::exception& e) {
        logger.write(std::string("Error reading PE header: ") + e.what());
        return std::vector<uint8_t>();
    }
}

static std::vector<std::string> collect_dlls_to_inject(const bool is_exe_32, std::string &failed_dlls)
{
    const auto load_order_file = std::filesystem::u8path(DllsToInjectFolder) / "load_order.txt";
    std::vector<std::string> dlls_to_inject{};
    for (const auto &dir_entry :
        std::filesystem::recursive_directory_iterator(std::filesystem::u8path(DllsToInjectFolder), std::filesystem::directory_options::follow_directory_symlink)
    ) {
        if (std::filesystem::is_directory(dir_entry.path())) continue;

        auto dll_path = dir_entry.path().u8string();
        // ignore this file if it is the load order file
        if (common_helpers::str_cmp_insensitive(dll_path, load_order_file.u8string())) continue;
        
        auto dll_header = get_pe_header(dll_path);
        if (dll_header.empty()) {
            logger.write("Failed to get PE header of dll: '" + dll_path + "'");
            failed_dlls += dll_path + "\n";
            continue;
        }
        
        const bool is_dll_32 = pe_helpers::is_module_32((HMODULE)&dll_header[0]);
        const bool is_dll_64 = pe_helpers::is_module_64((HMODULE)&dll_header[0]);
        if (is_dll_32 == is_dll_64) { // ARM, or just a regular file
            logger.write("Dll '" + dll_path + "' is neither 32 nor 64 bit and will be ignored");
            failed_dlls += dll_path + "\n";
            continue;
        }

        if (is_exe_32 == is_dll_32) { // same arch
            dlls_to_inject.push_back(dll_path);
            logger.write("Dll '" + dll_path + "' is valid");
        } else {
            logger.write("Dll '" + dll_path + "' has a different arch than the exe and will be ignored");
            failed_dlls += dll_path + "\n";
        }
    }

    bool load_only_specified_dlls = false;
    std::vector<std::string> ordered_dlls_to_inject{};
    {
        logger.write("Searching for load order file: '" + load_order_file.u8string() + "'");
        auto f_order = std::ifstream(load_order_file, std::ios::in);
        if (f_order.is_open()) {
            load_only_specified_dlls = true;
            logger.write(L"Reading load order file...");
            std::string line{};
            while (std::getline(f_order, line)) {
                auto abs = common_helpers::to_absolute(line, DllsToInjectFolder);
                logger.write("Load order line: '" + abs + "'");
                auto it = std::find_if(dlls_to_inject.begin(), dlls_to_inject.end(), [&abs](const std::string &dll_to_inject) {
                    return common_helpers::str_cmp_insensitive(dll_to_inject, abs);
                });
                if (dlls_to_inject.end() != it) {
                    logger.write("Found the dll specified by the load order line");
                    ordered_dlls_to_inject.push_back(*it);
                }
            }
            f_order.close();
        }
    }

    if (load_only_specified_dlls) {
        return ordered_dlls_to_inject;
    } else {
        return dlls_to_inject;
    }
}

static bool orig_steam_hkcu = false;
static WCHAR OrgSteamCDir_hkcu[8192] = { 0 };
static DWORD Size1_hkcu = sizeof(OrgSteamCDir_hkcu);
static WCHAR OrgSteamCDir64_hkcu[8192] = { 0 };
static DWORD Size2_hkcu = sizeof(OrgSteamCDir64_hkcu);
static DWORD OrgSteamActiveUser_hkcu = 0;
static WCHAR OrgSteamUniverse_hkcu[8192] = { 0 };
static DWORD Size4_hkcu = sizeof(OrgSteamUniverse_hkcu);
static bool patch_registry_hkcu()
{
    HKEY Registrykey = { 0 };
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Valve\\Steam\\ActiveProcess", 0, KEY_ALL_ACCESS, &Registrykey) == ERROR_SUCCESS) {
        orig_steam_hkcu = true;
        // Get original values to restore later.
        DWORD keyType = REG_SZ;
        RegQueryValueExW(Registrykey, L"SteamClientDll", 0, &keyType, (LPBYTE)OrgSteamCDir_hkcu, &Size1_hkcu);
        RegQueryValueExW(Registrykey, L"SteamClientDll64", 0, &keyType, (LPBYTE)OrgSteamCDir64_hkcu, &Size2_hkcu);
        DWORD SizeActiveUser_hkcu = sizeof(DWORD);
        RegQueryValueExW(Registrykey, L"ActiveUser", 0, &keyType, (LPBYTE)&OrgSteamActiveUser_hkcu, &SizeActiveUser_hkcu);
        RegQueryValueExW(Registrykey, L"Universe", 0, &keyType, (LPBYTE)OrgSteamUniverse_hkcu, &Size4_hkcu);
        logger.write("Found previous registry entry (HKCU) for Steam");
    } else if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Valve\\Steam\\ActiveProcess", 0, 0, REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS, NULL, &Registrykey, NULL) == ERROR_SUCCESS) {
        logger.write("Created new registry entry (HKCU) for Steam");
    } else {
        logger.write("Unable to patch Registry (HKCU), error = " + std::to_string(GetLastError()));
        return false;
    }

    auto client_path = common_helpers::to_wstr(ClientPath);
    auto client64_path = common_helpers::to_wstr(Client64Path);
    RegSetValueExW(Registrykey, L"pid", NULL, REG_DWORD, (const BYTE *)&ProcessID, sizeof(DWORD));
    RegSetValueExW(Registrykey, L"SteamClientDll", NULL, REG_SZ, (const BYTE*)(client_path).c_str(), static_cast<DWORD>((client_path.size() + 1) * sizeof(client_path[0])));
    RegSetValueExW(Registrykey, L"SteamClientDll64", NULL, REG_SZ, (const BYTE*)client64_path.c_str(), static_cast<DWORD>((client64_path.size() + 1) * sizeof(client64_path[0])));
    RegSetValueExW(Registrykey, L"ActiveUser", NULL, REG_DWORD, (const BYTE *)&UserId, sizeof(DWORD));
    RegSetValueExW(Registrykey, L"Universe", NULL, REG_SZ, (const BYTE *)STEAM_UNIVERSE, (DWORD)sizeof(STEAM_UNIVERSE));
    RegCloseKey(Registrykey);
    return true;
}

static void cleanup_registry_hkcu()
{
    if (!orig_steam_hkcu) return;

    logger.write("restoring registry entries (HKCU)");
    HKEY Registrykey = { 0 };
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Valve\\Steam\\ActiveProcess", 0, KEY_ALL_ACCESS, &Registrykey) == ERROR_SUCCESS) {
        // Restore the values.
        RegSetValueExW(Registrykey, L"SteamClientDll", NULL, REG_SZ, (const BYTE *)OrgSteamCDir_hkcu, Size1_hkcu);
        RegSetValueExW(Registrykey, L"SteamClientDll64", NULL, REG_SZ, (const BYTE *)OrgSteamCDir64_hkcu, Size2_hkcu);
        RegSetValueExW(Registrykey, L"ActiveUser", NULL, REG_DWORD, (const BYTE *)&OrgSteamActiveUser_hkcu, sizeof(DWORD));
        RegSetValueExW(Registrykey, L"Universe", NULL, REG_SZ, (const BYTE *)OrgSteamUniverse_hkcu, Size4_hkcu);

        // Close the HKEY Handle.
        RegCloseKey(Registrykey);
    } else {
        logger.write("Unable to restore the original registry entry (HKCU), error = " + std::to_string(GetLastError()));
    }
}


static bool orig_steam_hkcu_2 = false;
static WCHAR OrgSteamModDir_hkcu_2[8192] = { 0 };
static DWORD Size1_hkcu_2 = sizeof(OrgSteamModDir_hkcu_2);
static WCHAR OrgSteamPath_hkcu_2[8192] = { 0 };
static DWORD Size2_hkcu_2 = sizeof(OrgSteamPath_hkcu_2);
static WCHAR OrgSteamExe_2[8192] = { 0 };
static DWORD Size3_hkcu_2 = sizeof(OrgSteamExe_2);
static bool patch_registry_hkcu_2()
{
    HKEY Registrykey = { 0 };
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Valve\\Steam", 0, KEY_ALL_ACCESS, &Registrykey) == ERROR_SUCCESS) {
        orig_steam_hkcu_2 = true;
        // Get original values to restore later.
        DWORD keyType = REG_SZ;
        RegQueryValueExW(Registrykey, L"SourceModInstallPath", 0, &keyType, (LPBYTE)OrgSteamModDir_hkcu_2, &Size1_hkcu_2);
        RegQueryValueExW(Registrykey, L"SteamPath", 0, &keyType, (LPBYTE)OrgSteamPath_hkcu_2, &Size2_hkcu_2);
        RegQueryValueExW(Registrykey, L"SteamExe", 0, &keyType, (LPBYTE)OrgSteamExe_2, &Size3_hkcu_2);
        logger.write("Found previous registry entry (HKCU #2) for Steam");
    } else if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Valve\\Steam", 0, 0, REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS, NULL, &Registrykey, NULL) == ERROR_SUCCESS) {
        logger.write("Created new registry entry (HKCU #2) for Steam");
    } else {
        logger.write("Unable to patch Registry (HKCU #2), error = " + std::to_string(GetLastError()));
        return false;
    }

    auto my_path = common_helpers::to_wstr(pe_helpers::get_current_exe_path());
    my_path.pop_back(); // remove last '\\'
    const auto my_exe = common_helpers::to_wstr(pe_helpers::get_current_exe_path() + pe_helpers::get_current_exe_name());
    if (AppId.size()) { // empty in persistent mode = 2
        const auto appid_dword = std::stoul(AppId);
        RegSetValueExW(Registrykey, L"RunningAppID", NULL, REG_DWORD, (const BYTE*)&appid_dword, sizeof(DWORD));
    }
    RegSetValueExW(Registrykey, L"SourceModInstallPath", NULL, REG_SZ, (const BYTE*)my_path.c_str(), static_cast<DWORD>((my_path.size() + 1) * sizeof(my_path[0])));
    RegSetValueExW(Registrykey, L"SteamPath", NULL, REG_SZ, (const BYTE*)my_path.c_str(), static_cast<DWORD>((my_path.size() + 1) * sizeof(my_path[0])));
    RegSetValueExW(Registrykey, L"SteamExe", NULL, REG_SZ, (const BYTE*)my_exe.c_str(), static_cast<DWORD>((my_exe.size() + 1) * sizeof(my_exe[0])));
    RegCloseKey(Registrykey);
    return true;
}

static void cleanup_registry_hkcu_2()
{
    if (!orig_steam_hkcu_2) return;

    logger.write("restoring registry entries (HKCU #2)");
    HKEY Registrykey = { 0 };
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Valve\\Steam", 0, KEY_ALL_ACCESS, &Registrykey) == ERROR_SUCCESS) {
        // Restore the values.
        RegSetValueExW(Registrykey, L"SourceModInstallPath", NULL, REG_SZ, (LPBYTE)OrgSteamModDir_hkcu_2, Size1_hkcu_2);
        RegSetValueExW(Registrykey, L"SteamPath", NULL, REG_SZ, (LPBYTE)OrgSteamPath_hkcu_2, Size2_hkcu_2);
        RegSetValueExW(Registrykey, L"SteamExe", NULL, REG_SZ, (LPBYTE)OrgSteamExe_2, Size3_hkcu_2);

        // Close the HKEY Handle.
        RegCloseKey(Registrykey);
    } else {
        logger.write("Unable to restore the original registry entry (HKCU #2), error = " + std::to_string(GetLastError()));
    }
}


static bool orig_steam_hklm = false;
static WCHAR OrgInstallPath_hklm[8192] = { 0 };
static DWORD Size1_hklm = sizeof(OrgInstallPath_hklm);
static WCHAR OrgUniverse_hklm[8192] = { 0 };
static DWORD Size2_hklm = sizeof(OrgUniverse_hklm);
static bool patch_registry_hklm()
{
    HKEY Registrykey = { 0 };
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, hklm_path.c_str(), 0, KEY_ALL_ACCESS, &Registrykey) == ERROR_SUCCESS) {
        orig_steam_hklm = true;
        // Get original values to restore later.
        DWORD keyType = REG_SZ;
        RegQueryValueExW(Registrykey, L"InstallPath", 0, &keyType, (LPBYTE)OrgInstallPath_hklm, &Size1_hklm);
        RegQueryValueExW(Registrykey, L"Universe", 0, &keyType, (LPBYTE)OrgUniverse_hklm, &Size2_hklm);
        logger.write("Found previous registry entry (HKLM) for Steam");
    } else if (RegCreateKeyExW(HKEY_LOCAL_MACHINE, hklm_path.c_str(), 0, 0, REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS, NULL, &Registrykey, NULL) == ERROR_SUCCESS) {
        logger.write("Created new registry entry (HKLM) for Steam");
    } else {
        logger.write("Unable to patch Registry (HKLM), error = " + std::to_string(GetLastError()));
        return false;
    }

    auto my_path = common_helpers::to_wstr(pe_helpers::get_current_exe_path());
    my_path.pop_back(); // remove last '\\'
    RegSetValueExW(Registrykey, L"SteamPID", NULL, REG_DWORD, (const BYTE *)&ProcessID, sizeof(DWORD));
    RegSetValueExW(Registrykey, L"InstallPath", NULL, REG_SZ, (const BYTE*)my_path.c_str(), static_cast<DWORD>((my_path.size() + 1) * sizeof(my_path[0])));
    RegSetValueExW(Registrykey, L"Universe", NULL, REG_SZ, (const BYTE *)STEAM_UNIVERSE, (DWORD)sizeof(STEAM_UNIVERSE));
    RegCloseKey(Registrykey);
    return true;
}

static void cleanup_registry_hklm()
{
    if (!orig_steam_hklm) return;

    logger.write("restoring registry entries (HKLM)");
    HKEY Registrykey = { 0 };
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, hklm_path.c_str(), 0, KEY_ALL_ACCESS, &Registrykey) == ERROR_SUCCESS) {
        RegSetValueExW(Registrykey, L"InstallPath", NULL, REG_SZ, (const BYTE *)OrgInstallPath_hklm, Size1_hklm);
        RegSetValueExW(Registrykey, L"Universe", NULL, REG_SZ, (const BYTE *)OrgUniverse_hklm, Size2_hklm);
        RegCloseKey(Registrykey);
    } else {
        logger.write("Unable to restore the original registry entry (HKLM), error = " + std::to_string(GetLastError()));
    }
}


static bool orig_steam_hkcs_1 = false;
static bool orig_steam_hkcs_2 = false;
static WCHAR OrgCommand_hkcs[8192] = { 0 };
static DWORD Size1_hkcs = sizeof(OrgCommand_hkcs);
static WCHAR OrgUrlProtocol_hkcs[8192] = { 0 };
static DWORD Size2_hkcs = sizeof(OrgUrlProtocol_hkcs);
static bool patch_registry_hkcs()
{
    HKEY Registrykey = { 0 };
    if (RegOpenKeyExW(HKEY_CLASSES_ROOT, L"steam", 0, KEY_ALL_ACCESS, &Registrykey) == ERROR_SUCCESS) {
        orig_steam_hkcs_1 = true;
        // Get original values to restore later.
        DWORD keyType = REG_SZ;
        RegQueryValueExW(Registrykey, L"URL Protocol", 0, &keyType, (LPBYTE)&OrgUrlProtocol_hkcs, &Size2_hkcs);
        logger.write("Found previous registry entry (HKCS) #1 for Steam");
    } else if (RegCreateKeyExW(HKEY_CLASSES_ROOT, L"steam", 0, 0, REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS, NULL, &Registrykey, NULL) == ERROR_SUCCESS) {
        logger.write("Created new registry entry (HKCS) #1 for Steam");
    } else {
        logger.write("Unable to patch Registry (HKCS) #1, error = " + std::to_string(GetLastError()));
        return false;
    }
    
    HKEY Registrykey_2 = { 0 };
    if (RegOpenKeyExW(HKEY_CLASSES_ROOT, L"steam\\Shell\\Open\\Command", 0, KEY_ALL_ACCESS, &Registrykey_2) == ERROR_SUCCESS) {
        orig_steam_hkcs_2 = true;
        // Get original values to restore later.
        DWORD keyType = REG_SZ;
        RegQueryValueExW(Registrykey_2, L"", 0, &keyType, (LPBYTE)&OrgCommand_hkcs, &Size1_hkcs);
        logger.write("Found previous registry entry (HKCS) #2 for Steam");
    } else if (RegCreateKeyExW(HKEY_CLASSES_ROOT, L"steam\\Shell\\Open\\Command", 0, 0, REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS, NULL, &Registrykey_2, NULL) == ERROR_SUCCESS) {
        logger.write("Created new registry entry (HKCS) #2 for Steam");
    } else {
        logger.write("Unable to patch Registry (HKCS) #2, error = " + std::to_string(GetLastError()));
        RegCloseKey(Registrykey);
        return false;
    }

    RegSetValueExW(Registrykey, L"", NULL, REG_SZ, (const BYTE *)STEAM_URL_PROTOCOL, (DWORD)sizeof(STEAM_URL_PROTOCOL));
    RegSetValueExW(Registrykey, L"URL Protocol", NULL, REG_SZ, (const BYTE *)L"", (DWORD)sizeof(L""));
    RegCloseKey(Registrykey);

    const auto cmd = common_helpers::to_wstr("\"" + pe_helpers::get_current_exe_path() + pe_helpers::get_current_exe_name() + "\" -- \"%1\"");
    RegSetValueExW(Registrykey_2, L"", NULL, REG_SZ, (const BYTE*)cmd.c_str(), static_cast<DWORD>((cmd.size() + 1) * sizeof(cmd[0])));
    RegCloseKey(Registrykey_2);
    return true;
}

static void cleanup_registry_hkcs()
{
    if (orig_steam_hkcs_2) {
        logger.write("restoring registry entries (HKCS) #2");
        HKEY Registrykey = { 0 };
        if (RegOpenKeyExW(HKEY_CLASSES_ROOT, L"steam\\Shell\\Open\\Command", 0, KEY_ALL_ACCESS, &Registrykey) == ERROR_SUCCESS) {
            RegSetValueExW(Registrykey, L"", NULL, REG_SZ, (const BYTE *)OrgCommand_hkcs, Size1_hkcs);
            RegCloseKey(Registrykey);
        } else {
            logger.write("Unable to restore the original registry entry (HKCS) #2, error = " + std::to_string(GetLastError()));
        }
    }

    if (orig_steam_hkcs_1) {
        logger.write("restoring registry entries (HKCS) #1");
        HKEY Registrykey = { 0 };
        if (RegOpenKeyExW(HKEY_CLASSES_ROOT, L"steam", 0, KEY_ALL_ACCESS, &Registrykey) == ERROR_SUCCESS) {
            RegSetValueExW(Registrykey, L"URL Protocol", NULL, REG_SZ, (const BYTE *)OrgUrlProtocol_hkcs, Size2_hkcs);
            RegCloseKey(Registrykey);
        } else {
            logger.write("Unable to restore the original registry entry (HKCS) #1, error = " + std::to_string(GetLastError()));
        }
    } else {
        logger.write("removing registry entries (HKCS) #2 (added by loader)");
        HKEY Registrykey = { 0 };
        RegDeleteKeyW(HKEY_CLASSES_ROOT, L"steam");
    }
}



static void set_steam_env_vars(const std::string &AppId)
{
    SetEnvironmentVariableA("SteamAppId", AppId.c_str());
    SetEnvironmentVariableA("SteamGameId", AppId.c_str());
    // this env var conflicts with Steam Input
    // SetEnvironmentVariableA("SteamOverlayGameId", AppId.c_str());

    // these 2 wil be overridden by the emu
    SetEnvironmentVariableW(L"SteamAppUser", L"cold_player");
    SetEnvironmentVariableW(L"SteamUser", L"cold_player");

    SetEnvironmentVariableW(L"SteamClientLaunch", L"1");
    SetEnvironmentVariableW(L"SteamEnv", L"1");
    SetEnvironmentVariableW(L"SteamPath", common_helpers::to_wstr(pe_helpers::get_current_exe_path()).c_str());
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PWSTR lpCmdLine, _In_ int nCmdShow)
{
    IniFile = pe_helpers::get_current_exe_path() + std::filesystem::u8path(pe_helpers::get_current_exe_name()).stem().u8string() + ".ini";
    logger.write("Searching for configuration file: " + IniFile);
    if (!common_helpers::file_exist(IniFile)) {
        IniFile = pe_helpers::get_current_exe_path() + "ColdClientLoader.ini";
        logger.write("Searching for configuration file: " + IniFile);
    }

    if (!common_helpers::file_exist(IniFile)) {
        logger.write("Couldn't find the configuration file");
        MessageBoxA(NULL, "Couldn't find the configuration file.", "ColdClientLoader", MB_ICONERROR);
        return 1;
    }

    {
        std::ifstream ini_stream( std::filesystem::u8path(IniFile), std::ios::binary | std::ios::in);
        if (!ini_stream.is_open()) {
            logger.write("Failed to open the configuration file");
            MessageBoxA(NULL, "Failed to open the configuration file.", "ColdClientLoader", MB_ICONERROR);
            return 1;
        }

        auto err = local_ini.LoadData(ini_stream);
        ini_stream.close();
        logger.write("result of parsing ini '%s' %i (success == 0)", IniFile.c_str(), (int)err);
        if (err != SI_OK) {
            logger.write("Failed to load the configuration file");
            MessageBoxA(NULL, "Failed to load the configuration file.", "ColdClientLoader", MB_ICONERROR);
            return 1;
        }
    }

    ClientPath = common_helpers::to_absolute(
        local_ini.GetValue("SteamClient", "SteamClientDll", ""),
        pe_helpers::get_current_exe_path()
    );
    Client64Path = common_helpers::to_absolute(
        local_ini.GetValue("SteamClient", "SteamClient64Dll", ""),
        pe_helpers::get_current_exe_path()
    );
    ExeFile = common_helpers::to_absolute(
        local_ini.GetValue("SteamClient", "Exe", ""),
        pe_helpers::get_current_exe_path()
    );
    ExeRunDir = common_helpers::to_absolute(
        local_ini.GetValue("SteamClient", "ExeRunDir", ""),
        pe_helpers::get_current_exe_path()
    );
    ExeCommandLine = local_ini.GetValue("SteamClient", "ExeCommandLine", "");
    AppId = local_ini.GetValue("SteamClient", "AppId", "");
    
    // dlls to inject
    ForceInjectSteamClient = local_ini.GetBoolValue("Injection", "ForceInjectSteamClient", false);
    ForceInjectGameOverlayRenderer = local_ini.GetBoolValue("Injection", "ForceInjectGameOverlayRenderer", false);
    DllsToInjectFolder = common_helpers::to_absolute(
        local_ini.GetValue("Injection", "DllsToInjectFolder", ""),
        pe_helpers::get_current_exe_path()
    );
    IgnoreInjectionError = local_ini.GetBoolValue("Injection", "IgnoreInjectionError", true);
    IgnoreLoaderArchDifference = local_ini.GetBoolValue("Injection", "IgnoreLoaderArchDifference", false);

    PersistentMode = local_ini.GetLongValue("Persistence", "Mode", 0);

    // debug
    ResumeByDebugger = local_ini.GetBoolValue("Debug", "ResumeByDebugger", false);

    if (PersistentMode != 0 && PersistentMode != 1 && PersistentMode != 2) PersistentMode = 0;

    // log everything
    logger.write("SteamClient::Exe: " + ExeFile);
    logger.write("SteamClient::ExeRunDir: " + ExeRunDir);
    logger.write("SteamClient::ExeCommandLine: " + ExeCommandLine);
    logger.write("SteamClient::AppId: " + AppId);
    logger.write("SteamClient::SteamClient: " + ClientPath);
    logger.write("SteamClient::SteamClient64Dll: " + Client64Path);
    logger.write("SteamClient::PersistentMode: " + std::to_string(PersistentMode));
    logger.write("SteamClient::ForceInjectSteamClient: " + std::to_string(ForceInjectSteamClient));
    logger.write("SteamClient::ForceInjectGameOverlayRenderer: " + std::to_string(ForceInjectGameOverlayRenderer));
    logger.write("Injection::DllsToInjectFolder: " + DllsToInjectFolder);
    logger.write("Injection::IgnoreInjectionError: " + std::to_string(IgnoreInjectionError));
    logger.write("Injection::IgnoreLoaderArchDifference: " + std::to_string(IgnoreLoaderArchDifference));
    logger.write("Debug::ResumeByDebugger: " + std::to_string(ResumeByDebugger));

    if (!common_helpers::file_exist(Client64Path)) {
        logger.write("Couldn't find the requested SteamClient64Dll");
        MessageBoxA(NULL, "Couldn't find the requested SteamClient64Dll.", "ColdClientLoader", MB_ICONERROR);
        return 1;
    }

    if (!common_helpers::file_exist(ClientPath)) {
        logger.write("Couldn't find the requested SteamClientDll");
        MessageBoxA(NULL, "Couldn't find the requested SteamClientDll.", "ColdClientLoader", MB_ICONERROR);
        return 1;
    }

    if (!common_helpers::file_exist(ExeFile)) {
        logger.write("Couldn't find the requested Exe file");
        MessageBoxA(NULL, "Couldn't find the requested Exe file.", "ColdClientLoader", MB_ICONERROR);
        return 1;
    }

    if (ExeRunDir.empty()) {
        ExeRunDir = std::filesystem::u8path(ExeFile).parent_path().u8string();
        logger.write("Setting ExeRunDir to: " + ExeRunDir);
    }
    if (!common_helpers::dir_exist(ExeRunDir)) {
        logger.write("Couldn't find the requested Exe run dir");
        MessageBoxA(NULL, "Couldn't find the requested Exe run dir.", "ColdClientLoader", MB_ICONERROR);
        return 1;
    }
        
    bool isDllsToInjectFolderFound = false;
    if (DllsToInjectFolder.size()) {
        isDllsToInjectFolderFound = common_helpers::dir_exist(DllsToInjectFolder);
        if (!isDllsToInjectFolderFound) {
            logger.write("Couldn't find the requested folder of dlls to inject");
            MessageBoxA(NULL, "Couldn't find the requested folder of dlls to inject.", "ColdClientLoader", MB_ICONWARNING);
            // it was requested to make this a non intrusive error, don't shutdown the whole thing because the folder is missing
            // return 1;
        }
    }
    
    exe_header = get_pe_header(ExeFile);
    if (exe_header.empty()) {
        logger.write("Couldn't read the exe header");
        MessageBoxA(NULL, "Couldn't read the exe header.", "ColdClientLoader", MB_ICONERROR);
        return 1;
    }

    bool is_exe_32 = pe_helpers::is_module_32((HMODULE)&exe_header[0]);
    bool is_exe_64 = pe_helpers::is_module_64((HMODULE)&exe_header[0]);
    if (is_exe_32 == is_exe_64) { // ARM, or just a regular file
        logger.write("The requested exe is invalid (neither 32 nor 64 bit)");
        MessageBoxA(NULL, "The requested exe is invalid (neither 32 nor 64 bit)", "ColdClientLoader", MB_ICONERROR);
        return 1;
    }
    if (is_exe_32) {
        logger.write("Detected exe arch: x32");
    } else {
        logger.write("Detected exe arch: x64");
    }

    if (loader_is_32) {
        logger.write("Detected loader arch: x32");
    } else {
        logger.write("Detected loader arch: x64");
    }

    if (loader_is_32 != is_exe_32) {
        logger.write("Arch of loader and requested exe are different, it is advised to use the appropriate one");
        if (!IgnoreLoaderArchDifference) {
            MessageBoxA(NULL, "Arch of loader and requested exe are different,\nit is advised to use the appropriate one.", "ColdClientLoader", MB_OK);
        }
    }

    if (isDllsToInjectFolderFound) {
        std::string failed_dlls{};
        dlls_to_inject = collect_dlls_to_inject(is_exe_32, failed_dlls);
        if (failed_dlls.size() && !IgnoreInjectionError) {
            int choice = MessageBoxA(
                NULL,
                ("The following dlls cannot be injected:\n" + failed_dlls + "\nContinue ?").c_str(),
                "ColdClientLoader",
                MB_YESNO);
            if (choice != IDYES) {
                return 1;
            }
        }
    }
    if (ForceInjectGameOverlayRenderer) {
        if (is_exe_32) {
            auto GameOverlayPath = common_helpers::to_absolute(
                "GameOverlayRenderer.dll",
                std::filesystem::u8path(ClientPath).parent_path().u8string()
            );
            if (!common_helpers::file_exist(GameOverlayPath)) {
                logger.write("Couldn't find GameOverlayRenderer.dll");
                MessageBoxA(NULL, "Couldn't find GameOverlayRenderer.dll to inject.", "ColdClientLoader", MB_ICONWARNING);
            } else {
                dlls_to_inject.insert(dlls_to_inject.begin(), GameOverlayPath);
            }
        } else { // 64
            auto GameOverlay64Path = common_helpers::to_absolute(
                "GameOverlayRenderer64.dll",
                std::filesystem::u8path(Client64Path).parent_path().u8string()
            );
            if (!common_helpers::file_exist(GameOverlay64Path)) {
                logger.write("Couldn't find GameOverlayRenderer64.dll");
                MessageBoxA(NULL, "Couldn't find GameOverlayRenderer64.dll to inject.", "ColdClientLoader", MB_ICONWARNING);
            } else {
                dlls_to_inject.insert(dlls_to_inject.begin(), GameOverlay64Path);
            }
        }
    }
    if (ForceInjectSteamClient) {
        if (is_exe_32) {
            dlls_to_inject.insert(dlls_to_inject.begin(), ClientPath);
        } else {
            dlls_to_inject.insert(dlls_to_inject.begin(), Client64Path);
        }
    }

    if (PersistentMode != 2) {
        if (AppId.size() && AppId[0]) {
            try {
                (void)std::stoul(AppId);
            } catch(...) {
                logger.write("AppId is not a valid number");
                MessageBoxA(NULL, "AppId is not a valid number.", "ColdClientLoader", MB_ICONERROR);
                return 1;
            }
            set_steam_env_vars(AppId);
        } else {
            logger.write("You forgot to set the AppId");
            MessageBoxA(NULL, "You forgot to set the AppId.", "ColdClientLoader", MB_ICONERROR);
            return 1;
        }
    } else { // steam://run/
        constexpr const static wchar_t STEAM_LAUNCH_CMD_1[] = L"-- \"steam://run/";
        constexpr const static wchar_t STEAM_LAUNCH_CMD_2[] = L"-- \"steam://rungameid/";
        AppId.clear(); // we don't care about the app id in the ini
        auto my_cmd = lpCmdLine && lpCmdLine[0]
            ? std::wstring(lpCmdLine)
            : std::wstring();
        //MessageBoxW(NULL, (my_cmd + L" ||| " + std::to_wstring(my_cmd.size())).c_str(), L"DEBUG ME", MB_OK);
        logger.write(L"persistent mode 2 detecting steam launch cmd from: '" + my_cmd + L"'");
        if (my_cmd.find(STEAM_LAUNCH_CMD_1) == 0) {
            AppId = common_helpers::to_str( my_cmd.substr(sizeof(STEAM_LAUNCH_CMD_1) / sizeof(STEAM_LAUNCH_CMD_1[0]), my_cmd.find_first_of(L" \t")) );
            logger.write("persistent mode 2 got steam launch cmd #1");
        } else if (my_cmd.find(STEAM_LAUNCH_CMD_2) == 0) {
            AppId = common_helpers::to_str( my_cmd.substr(sizeof(STEAM_LAUNCH_CMD_2) / sizeof(STEAM_LAUNCH_CMD_2[0]), my_cmd.find_first_of(L" \t")) );
            logger.write("persistent mode 2 got steam launch cmd #2");
        } else {
            logger.write("persistent mode 2 didn't detect a valid steam launch cmd");
        }

        if (AppId.size()) {
            set_steam_env_vars(AppId);
            logger.write("persistent mode 2 will use app id = " + AppId);
        } else {
            logger.write("persistent mode 2 didn't find app id");
        }
    }

    if (!patch_registry_hkcu()) {
        cleanup_registry_hkcu();
        cleanup_registry_hkcu_2();
        cleanup_registry_hklm();
        cleanup_registry_hkcs();
    
        logger.write("Unable to patch Registry (HKCU).");
        MessageBoxA(NULL, "Unable to patch Registry (HKCU).", "ColdClientLoader", MB_ICONERROR);
        return 1;
    }

    if (!patch_registry_hkcu_2()) {
        cleanup_registry_hkcu();
        cleanup_registry_hkcu_2();
        cleanup_registry_hklm();
        cleanup_registry_hkcs();
    
        logger.write("Unable to patch Registry (HKCU #2).");
        MessageBoxA(NULL, "Unable to patch Registry (HKCU #2).", "ColdClientLoader", MB_ICONERROR);
        return 1;
    }

    // this fails due to admin rights when Steam isn't installed, not a big deal
    // ----------------------------------------------
    patch_registry_hklm();
    // if (!patch_registry_hklm()) {
    //     cleanup_registry_hkcu();
    //     cleanup_registry_hkcu_2();
    //     cleanup_registry_hklm();
    //     cleanup_registry_hkcs();
    //
    //     logger.write("Unable to patch Registry (HKLM).");
    //     MessageBoxA(NULL, "Unable to patch Registry (HKLM).", "ColdClientLoader", MB_ICONERROR);
    //     return 1;
    // }

    // this fails due to admin rights, not a big deal
    // ----------------------------------------------
    patch_registry_hkcs();
    // if (!patch_registry_hkcs()) {
    //     cleanup_registry_hkcu();
    //     cleanup_registry_hkcu_2();
    //     cleanup_registry_hklm();
    //     cleanup_registry_hkcs();
        
    //     logger.write("Unable to patch Registry (HKCS).");
    //     MessageBoxA(NULL, "Unable to patch Registry (HKCS).", "ColdClientLoader", MB_ICONERROR);
    //     return 1;
    // }
    
    if (PersistentMode != 2 || AppId.size()) { // persistent mode 0 or 1, or mode 2 with defined app id   
        STARTUPINFOW info = { 0 };

        SecureZeroMemory(&info, sizeof(info));
        info.cb = sizeof(info);

        PROCESS_INFORMATION processInfo = { 0 };
        SecureZeroMemory(&processInfo, sizeof(processInfo));

        logger.write("spawning the requested EXE file");
        const auto exe_file = common_helpers::to_wstr(ExeFile);
        std::wstringstream cmdline{};
        cmdline << L"\"" << exe_file << L"\" " << common_helpers::to_wstr(ExeCommandLine) << L" " << lpCmdLine;
        auto CommandLine = cmdline.str();
        if (!CreateProcessW(exe_file.c_str(), (LPWSTR)CommandLine.c_str(), NULL, NULL, TRUE, CREATE_SUSPENDED, NULL, common_helpers::to_wstr(ExeRunDir).c_str(), &info, &processInfo)) {
            logger.write("Unable to load the requested EXE file, error = " + std::to_string(GetLastError()));
            cleanup_registry_hkcu();
            cleanup_registry_hkcu_2();
            cleanup_registry_hklm();
            cleanup_registry_hkcs();
            MessageBoxA(NULL, "Unable to load the requested EXE file.", "ColdClientLoader", MB_ICONERROR);
            return 1;
        }
        
        logger.write("injecting " + std::to_string(dlls_to_inject.size()) + " dlls");
        for (const auto &dll : dlls_to_inject) {
            logger.write("Injecting dll: '" + dll + "' ...");
            const char *err_inject = nullptr;
            DWORD code = pe_helpers::loadlib_remote(processInfo.hProcess, dll, &err_inject);
            if (code != ERROR_SUCCESS) {
                auto err_full =
                    std::string("Failed to inject the dll: ") + dll + "\n" +
                    err_inject + "\n" +
                    pe_helpers::get_err_string(code) + "\n" +
                    "Error code = " + std::to_string(code) + "\n";
                logger.write(err_full);
                if (!IgnoreInjectionError) {
                    TerminateProcess(processInfo.hProcess, 1);
                    CloseHandle(processInfo.hProcess);
                    CloseHandle(processInfo.hThread);

                    cleanup_registry_hkcu();
                    cleanup_registry_hkcu_2();
                    cleanup_registry_hklm();
                    cleanup_registry_hkcs();

                    MessageBoxA(NULL, err_full.c_str(), "ColdClientLoader", MB_ICONERROR);
                    return 1;
                }
            } else {
                logger.write("Injected!");
            }
        }

        // run
        if (!ResumeByDebugger) {
            logger.write("resuming the main thread of the exe");
            // MessageBoxA(nullptr, "Wait then press ok", "Wait some seconds", MB_OK);
            // games like house flipper 2 won't launch until the previous exe has exited, which takes some seconds
            if (PersistentMode == 2) {
                std::this_thread::sleep_for(std::chrono::seconds(5));
            }
            ResumeThread(processInfo.hThread);
        } else {
            std::string msg = "Attach a debugger now to PID " + std::to_string(processInfo.dwProcessId) + " and resume its main thread";
            logger.write(msg);
            MessageBoxA(NULL, msg.c_str(), "ColdClientLoader", MB_OK);
        }
        
        // wait
        WaitForSingleObject(processInfo.hThread, INFINITE);

        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);
    }

    if (PersistentMode == 1 ) {
        MessageBoxA(NULL, "Press OK when you have finished playing to close the loader", "Cold Client Loader (waiting)", MB_OK);
    } else if (PersistentMode == 2 && AppId.empty()) {
        MessageBoxA(NULL, "Start the game, then press OK when you have finished playing to close the loader", "Cold Client Loader (waiting)", MB_OK);
    }

    if (PersistentMode != 2 || AppId.empty()) { // persistent mode 0 or 1, or mode 2 without app id
        cleanup_registry_hkcu();
        cleanup_registry_hkcu_2();
        cleanup_registry_hklm();
        cleanup_registry_hkcs();
    }

    return 0;
}
