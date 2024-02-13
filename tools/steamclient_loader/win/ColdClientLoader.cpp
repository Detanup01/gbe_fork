// a Modified version of ColdClientLoader originally written by Rat431
// https://github.com/Rat431/ColdAPI_Steam/tree/master/src/ColdClientLoader

#include "common_helpers/common_helpers.hpp"
#include "pe_helpers/pe_helpers.hpp"
#include "dbg_log/dbg_log.hpp"

// C RunTime Header Files
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>
#include <stdio.h>
#include <string>
#include <set>
#include <thread>


static const std::wstring IniFile = pe_helpers::get_current_exe_path_w() + L"ColdClientLoader.ini";
static const std::wstring dbg_file = pe_helpers::get_current_exe_path_w() + L"COLD_LDR_LOG.txt";
constexpr static const char STEAM_UNIVERSE[] = "Public";
constexpr static const char STEAM_URL_PROTOCOL[] = "URL:steam protocol";

const bool loader_is_32 = pe_helpers::is_module_32(GetModuleHandleW(NULL));

const std::wstring hklm_path(loader_is_32 ? L"SOFTWARE\\Valve\\Steam" : L"SOFTWARE\\WOW6432Node\\Valve\\Steam");

// Declare some variables to be used for Steam registry.
const DWORD UserId = 0x03100004771F810D & 0xffffffff;
const DWORD ProcessID = GetCurrentProcessId();

std::wstring ClientPath{};
std::wstring Client64Path{};

std::vector<std::wstring> dlls_to_inject{};




std::wstring get_ini_value(LPCWSTR section, LPCWSTR key, LPCWSTR default_val = NULL)
{
    std::vector<wchar_t> buff(INT16_MAX);
    DWORD read_chars = GetPrivateProfileStringW(section, key, default_val, &buff[0], (DWORD)buff.size(), IniFile.c_str());
    if (!read_chars) {
        std::wstring();
    }

    // "If neither lpAppName nor lpKeyName is NULL and the supplied destination buffer is too small to hold the requested string, the return value is equal to nSize minus one"
    int trials = 3;
    while ((read_chars == (buff.size() - 1)) && trials > 0) {
        buff.resize(buff.size() * 2);
        read_chars = GetPrivateProfileStringW(section, key, default_val, &buff[0], (DWORD)buff.size(), IniFile.c_str());
        --trials;
    }

    return std::wstring(&buff[0], read_chars);
}

static std::vector<uint8_t> get_pe_header(const std::wstring &filepath)
{
    try {
        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open()) throw;

        file.seekg(0, std::ios::beg);

        // 2MB is enough
        std::vector<uint8_t> data(2 * 1024 * 1024, 0);
        file.read((char *)&data[0], data.size());
        file.close();

        return data;
    } catch(const std::exception& e) {
        dbg_log::write(std::string("Error reading PE header: ") + e.what());
        return std::vector<uint8_t>();
    }
}

static std::vector<std::wstring> collect_dlls_to_inject(
    const std::wstring &extra_dlls_folder,
    bool is_exe_32,
    std::wstring &failed_dlls = std::wstring{})
{
    const auto load_order_file = std::filesystem::path(extra_dlls_folder) / "load_order.txt";
    std::vector<std::wstring> dlls_to_inject{};
    for (auto const& dir_entry :
        std::filesystem::recursive_directory_iterator(extra_dlls_folder, std::filesystem::directory_options::follow_directory_symlink)) {
        if (std::filesystem::is_directory(dir_entry.path())) continue;

        auto dll_path = dir_entry.path().wstring();
        // ignore this file if it is the load order file
        if (common_helpers::to_upper(dll_path) == common_helpers::to_upper(load_order_file.wstring())) continue;
        
        auto dll_header = get_pe_header(dll_path);
        if (dll_header.empty()) {
            dbg_log::write(L"Failed to get PE header of dll: " + dll_path);
            failed_dlls += dll_path + L"\n";
            continue;
        }
        
        bool is_dll_32 = pe_helpers::is_module_32((HMODULE)&dll_header[0]);
        bool is_dll_64 = pe_helpers::is_module_64((HMODULE)&dll_header[0]);
        if ((!is_dll_32 && !is_dll_64) || (is_dll_32 && is_dll_64)) { // ARM, or just a regular file
            dbg_log::write(L"Dll " + dll_path + L" is neither 32 nor 64 bit and will be ignored");
            failed_dlls += dll_path + L"\n";
            continue;
        }

        if (is_dll_32 == is_exe_32) { // same arch
            dlls_to_inject.push_back(dll_path);
            dbg_log::write(L"Dll " + dll_path + L" will be injected");
        } else {
            dbg_log::write(L"Dll " + dll_path + L" has a different arch than the exe and will be ignored");
            failed_dlls += dll_path + L"\n";
        }
    }

    std::vector<std::wstring> ordered_dlls_to_inject{};
    {
        dbg_log::write(L"Searching for load order file: " + load_order_file.wstring());
        auto f_order = std::wifstream(load_order_file, std::ios::in);
        if (f_order.is_open()) {
            dbg_log::write(L"Reading load order file: " + load_order_file.wstring());
            std::wstring line{};
            while (std::getline(f_order, line)) {
                auto abs = common_helpers::to_absolute(line, extra_dlls_folder);
                auto abs_upper = common_helpers::to_upper(abs);
                dbg_log::write(L"Load order line: " + abs_upper);
                auto it = std::find_if(dlls_to_inject.begin(), dlls_to_inject.end(), [&abs_upper](const std::wstring &dll_to_inject) {
                    return  common_helpers::to_upper(dll_to_inject) == abs_upper;
                });
                if (it != dlls_to_inject.end()) {
                    dbg_log::write("Found the dll specified by the load order line");
                    ordered_dlls_to_inject.push_back(*it);
                    // mark for deletion
                    it->clear();
                }
            }
            f_order.close();
        }
    }

    // add the remaining dlls
    for (auto &dll : dlls_to_inject) {
        if (dll.size()) {
            ordered_dlls_to_inject.push_back(dll);
        }
    }

    return ordered_dlls_to_inject;
}

static void to_bool_ini_val(std::wstring &val)
{
    for (auto &c : val) {
        c = (wchar_t)std::tolower((int)c);
    }
    if (val != L"1" && val != L"y" && val != L"yes" && val != L"true") {
        val.clear();
    }
}


bool orig_steam_hkcu = false;
WCHAR OrgSteamCDir_hkcu[8192] = { 0 };
DWORD Size1_hkcu = _countof(OrgSteamCDir_hkcu);
WCHAR OrgSteamCDir64_hkcu[8192] = { 0 };
DWORD Size2_hkcu = _countof(OrgSteamCDir64_hkcu);
bool patch_registry_hkcu()
{
    HKEY Registrykey = { 0 };
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Valve\\Steam\\ActiveProcess", 0, KEY_ALL_ACCESS, &Registrykey) == ERROR_SUCCESS) {
        orig_steam_hkcu = true;
        // Get original values to restore later.
        DWORD keyType = REG_SZ;
        RegQueryValueExW(Registrykey, L"SteamClientDll", 0, &keyType, (LPBYTE)&OrgSteamCDir_hkcu, &Size1_hkcu);
        RegQueryValueExW(Registrykey, L"SteamClientDll64", 0, &keyType, (LPBYTE)&OrgSteamCDir64_hkcu, &Size2_hkcu);
        dbg_log::write("Found previous registry entry (HKCU) for Steam");
    } else if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Valve\\Steam\\ActiveProcess", 0, 0, REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS, NULL, &Registrykey, NULL) == ERROR_SUCCESS) {
        dbg_log::write("Created new registry entry (HKCU) for Steam");
    } else {
        dbg_log::write("Unable to patch Registry (HKCU), error = " + std::to_string(GetLastError()));
        return false;
    }

    RegSetValueExA(Registrykey, "ActiveUser", NULL, REG_DWORD, (const BYTE *)&UserId, sizeof(DWORD));
    RegSetValueExA(Registrykey, "pid", NULL, REG_DWORD, (const BYTE *)&ProcessID, sizeof(DWORD));
    RegSetValueExW(Registrykey, L"SteamClientDll", NULL, REG_SZ, (const BYTE *)ClientPath.c_str(), (ClientPath.size() + 1) * sizeof(ClientPath[0]));
    RegSetValueExW(Registrykey, L"SteamClientDll64", NULL, REG_SZ, (const BYTE *)Client64Path.c_str(), (Client64Path.size() + 1) * sizeof(Client64Path[0]));
    RegSetValueExA(Registrykey, "Universe", NULL, REG_SZ, (const BYTE *)STEAM_UNIVERSE, (DWORD)sizeof(STEAM_UNIVERSE));
    RegCloseKey(Registrykey);
    return true;
}

void cleanup_registry_hkcu()
{
    if (!orig_steam_hkcu) return;

    dbg_log::write("restoring registry entries (HKCU)");
    HKEY Registrykey = { 0 };
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Valve\\Steam\\ActiveProcess", 0, KEY_ALL_ACCESS, &Registrykey) == ERROR_SUCCESS) {
        // Restore the values.
        RegSetValueExW(Registrykey, L"SteamClientDll", NULL, REG_SZ, (LPBYTE)OrgSteamCDir_hkcu, Size1_hkcu);
        RegSetValueExW(Registrykey, L"SteamClientDll64", NULL, REG_SZ, (LPBYTE)OrgSteamCDir64_hkcu, Size2_hkcu);

        // Close the HKEY Handle.
        RegCloseKey(Registrykey);
    } else {
        dbg_log::write("Unable to restore the original registry entry (HKCU), error = " + std::to_string(GetLastError()));
    }
}


bool orig_steam_hklm = false;
WCHAR OrgInstallPath_hklm[8192] = { 0 };
DWORD Size1_hklm = _countof(OrgInstallPath_hklm);
bool patch_registry_hklm()
{
    HKEY Registrykey = { 0 };
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, hklm_path.c_str(), 0, KEY_ALL_ACCESS, &Registrykey) == ERROR_SUCCESS) {
        orig_steam_hklm = true;
        // Get original values to restore later.
        DWORD keyType = REG_SZ;
        RegQueryValueExW(Registrykey, L"InstallPath", 0, &keyType, (LPBYTE)&OrgInstallPath_hklm, &Size1_hklm);
        dbg_log::write("Found previous registry entry (HKLM) for Steam");
    } else if (RegCreateKeyExW(HKEY_LOCAL_MACHINE, hklm_path.c_str(), 0, 0, REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS, NULL, &Registrykey, NULL) == ERROR_SUCCESS) {
        dbg_log::write("Created new registry entry (HKLM) for Steam");
    } else {
        dbg_log::write("Unable to patch Registry (HKLM), error = " + std::to_string(GetLastError()));
        return false;
    }

    const auto my_path = pe_helpers::get_current_exe_path_w();
    RegSetValueExW(Registrykey, L"InstallPath", NULL, REG_SZ, (const BYTE *)my_path.c_str(), (my_path.size() + 1) * sizeof(my_path[0]));
    RegSetValueExA(Registrykey, "SteamPID", NULL, REG_DWORD, (const BYTE *)&ProcessID, sizeof(DWORD));
    RegSetValueExA(Registrykey, "Universe", NULL, REG_SZ, (const BYTE *)STEAM_UNIVERSE, (DWORD)sizeof(STEAM_UNIVERSE));
    RegCloseKey(Registrykey);
    return true;
}

void cleanup_registry_hklm()
{
    if (!orig_steam_hklm) return;

    dbg_log::write("restoring registry entries (HKLM)");
    HKEY Registrykey = { 0 };
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, hklm_path.c_str(), 0, KEY_ALL_ACCESS, &Registrykey) == ERROR_SUCCESS) {
        RegSetValueExW(Registrykey, L"InstallPath", NULL, REG_SZ, (const BYTE *)OrgInstallPath_hklm, Size1_hklm);
        RegCloseKey(Registrykey);
    } else {
        dbg_log::write("Unable to restore the original registry entry (HKLM), error = " + std::to_string(GetLastError()));
    }
}


bool orig_steam_hkcs_1 = false;
bool orig_steam_hkcs_2 = false;
WCHAR OrgCommand_hkcs[8192] = { 0 };
DWORD Size1_hkcs = _countof(OrgCommand_hkcs);
bool patch_registry_hkcs()
{
    HKEY Registrykey = { 0 };
    if (RegOpenKeyExW(HKEY_CLASSES_ROOT, L"steam", 0, KEY_ALL_ACCESS, &Registrykey) == ERROR_SUCCESS) {
        orig_steam_hkcs_1 = true;
        dbg_log::write("Found previous registry entry (HKCS) #1 for Steam");
    } else if (RegCreateKeyExW(HKEY_CLASSES_ROOT, L"steam", 0, 0, REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS, NULL, &Registrykey, NULL) == ERROR_SUCCESS) {
        dbg_log::write("Created new registry entry (HKCS) #1 for Steam");
    } else {
        dbg_log::write("Unable to patch Registry (HKCS) #1, error = " + std::to_string(GetLastError()));
        return false;
    }
    
    HKEY Registrykey_2 = { 0 };
    if (RegOpenKeyExW(HKEY_CLASSES_ROOT, L"steam\\Shell\\Open\\Command", 0, KEY_ALL_ACCESS, &Registrykey_2) == ERROR_SUCCESS) {
        orig_steam_hkcs_2 = true;
        // Get original values to restore later.
        DWORD keyType = REG_SZ;
        RegQueryValueExW(Registrykey_2, L"", 0, &keyType, (LPBYTE)&OrgCommand_hkcs, &Size1_hkcs);
        dbg_log::write("Found previous registry entry (HKCS) #2 for Steam");
    } else if (RegCreateKeyExW(HKEY_CLASSES_ROOT, L"steam\\Shell\\Open\\Command", 0, 0, REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS, NULL, &Registrykey_2, NULL) == ERROR_SUCCESS) {
        dbg_log::write("Created new registry entry (HKCS) #2 for Steam");
    } else {
        dbg_log::write("Unable to patch Registry (HKCS) #2, error = " + std::to_string(GetLastError()));
        RegCloseKey(Registrykey);
        return false;
    }

    RegSetValueExA(Registrykey, "", NULL, REG_SZ, (const BYTE *)STEAM_URL_PROTOCOL, (DWORD)sizeof(STEAM_URL_PROTOCOL));
    RegSetValueExA(Registrykey, "URL Protocol", NULL, REG_SZ, (const BYTE *)"", (DWORD)sizeof(""));
    RegCloseKey(Registrykey);

    const auto cmd = pe_helpers::get_current_exe_path_w() + L"steam.exe -- \"%1\"";
    RegSetValueExW(Registrykey_2, L"", NULL, REG_SZ, (const BYTE *)cmd.c_str(), (cmd.size() + 1) * sizeof(cmd[0]));
    RegCloseKey(Registrykey_2);
    return true;
}

void cleanup_registry_hkcs()
{
    if (orig_steam_hkcs_2) {
        dbg_log::write("restoring registry entries (HKCS) #1");
        HKEY Registrykey = { 0 };
        if (RegOpenKeyExW(HKEY_CLASSES_ROOT, L"steam\\Shell\\Open\\Command", 0, KEY_ALL_ACCESS, &Registrykey) == ERROR_SUCCESS) {
            RegSetValueExW(Registrykey, L"", NULL, REG_SZ, (const BYTE *)OrgCommand_hkcs, Size1_hkcs);
            RegCloseKey(Registrykey);
        } else {
            dbg_log::write("Unable to restore the original registry entry (HKCS) #2, error = " + std::to_string(GetLastError()));
        }
    }

    if (!orig_steam_hkcs_1) {
        dbg_log::write("removing registry entries (HKCS) #2 (added by loader)");
        HKEY Registrykey = { 0 };
        RegDeleteKeyA(HKEY_CLASSES_ROOT, "steam");
    }
}



void set_steam_env_vars(const std::wstring &AppId)
{
    SetEnvironmentVariableW(L"SteamAppId", AppId.c_str());
    SetEnvironmentVariableW(L"SteamGameId", AppId.c_str());
    SetEnvironmentVariableW(L"SteamOverlayGameId", AppId.c_str());
}


int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    dbg_log::init(dbg_file.c_str());

    if (!common_helpers::file_exist(IniFile)) {
        dbg_log::write(L"Couldn't find the configuration file: " + dbg_file);
        MessageBoxA(NULL, "Couldn't find the configuration file ColdClientLoader.ini.", "ColdClientLoader", MB_ICONERROR);
        dbg_log::close();
        return 1;
    }

    ClientPath = common_helpers::to_absolute(
        get_ini_value(L"SteamClient", L"SteamClientDll"),
        pe_helpers::get_current_exe_path_w()
    );
    Client64Path = common_helpers::to_absolute(
        get_ini_value(L"SteamClient", L"SteamClient64Dll"),
        pe_helpers::get_current_exe_path_w()
    );
    std::wstring ExeFile = common_helpers::to_absolute(
        get_ini_value(L"SteamClient", L"Exe"),
        pe_helpers::get_current_exe_path_w()
    );
    std::wstring ExeRunDir = common_helpers::to_absolute(
        get_ini_value(L"SteamClient", L"ExeRunDir"),
        pe_helpers::get_current_exe_path_w()
    );
    std::wstring ExeCommandLine = get_ini_value(L"SteamClient", L"ExeCommandLine");
    std::wstring AppId = get_ini_value(L"SteamClient", L"AppId");
    
    // dlls to inject
    std::wstring ForceInjectSteamClient = get_ini_value(L"Injection", L"ForceInjectSteamClient");
    std::wstring DllsToInjectFolder = common_helpers::to_absolute(
        get_ini_value(L"Injection", L"DllsToInjectFolder"),
        pe_helpers::get_current_exe_path_w()
    );
    std::wstring IgnoreInjectionError = get_ini_value(L"Injection", L"IgnoreInjectionError", L"1");
    std::wstring IgnoreLoaderArchDifference = get_ini_value(L"Injection", L"IgnoreLoaderArchDifference", L"0");

    std::wstring PersistentMode = get_ini_value(L"Persistence", L"Mode", L"0");

    // debug
    std::wstring ResumeByDebugger = get_ini_value(L"Debug", L"ResumeByDebugger");

    to_bool_ini_val(ForceInjectSteamClient);
    to_bool_ini_val(IgnoreInjectionError);
    to_bool_ini_val(IgnoreLoaderArchDifference);
    to_bool_ini_val(ResumeByDebugger);
    if (PersistentMode != L"0" && PersistentMode != L"1" && PersistentMode != L"2") PersistentMode = L"0";

    // log everything
    dbg_log::write(L"SteamClient::Exe: " + ExeFile);
    dbg_log::write(L"SteamClient::ExeRunDir: " + ExeRunDir);
    dbg_log::write(L"SteamClient::ExeCommandLine: " + ExeCommandLine);
    dbg_log::write(L"SteamClient::AppId: " + AppId);
    dbg_log::write(L"SteamClient::SteamClient: " + ClientPath);
    dbg_log::write(L"SteamClient::SteamClient64Dll: " + Client64Path);
    dbg_log::write(L"SteamClient::PersistentMode: " + PersistentMode);
    dbg_log::write(L"SteamClient::ForceInjectSteamClient: " + ForceInjectSteamClient);
    dbg_log::write(L"Injection::DllsToInjectFolder: " + DllsToInjectFolder);
    dbg_log::write(L"Injection::IgnoreInjectionError: " + IgnoreInjectionError);
    dbg_log::write(L"Injection::IgnoreLoaderArchDifference: " + IgnoreLoaderArchDifference);
    dbg_log::write(L"Debug::ResumeByDebugger: " + ResumeByDebugger);

    if (!common_helpers::file_exist(Client64Path)) {
        dbg_log::write("Couldn't find the requested SteamClient64Dll");
        MessageBoxA(NULL, "Couldn't find the requested SteamClient64Dll.", "ColdClientLoader", MB_ICONERROR);
        dbg_log::close();
        return 1;
    }

    if (!common_helpers::file_exist(ClientPath)) {
        dbg_log::write("Couldn't find the requested SteamClientDll");
        MessageBoxA(NULL, "Couldn't find the requested SteamClientDll.", "ColdClientLoader", MB_ICONERROR);
        dbg_log::close();
        return 1;
    }

    if (!common_helpers::file_exist(ExeFile)) {
        dbg_log::write("Couldn't find the requested Exe file");
        MessageBoxA(NULL, "Couldn't find the requested Exe file.", "ColdClientLoader", MB_ICONERROR);
        dbg_log::close();
        return 1;
    }

    if (ExeRunDir.empty()) {
        ExeRunDir = std::filesystem::path(ExeFile).parent_path().wstring();
        dbg_log::write(L"Setting ExeRunDir to: " + ExeRunDir);
    }
    if (!common_helpers::dir_exist(ExeRunDir)) {
        dbg_log::write("Couldn't find the requested Exe run dir");
        MessageBoxA(NULL, "Couldn't find the requested Exe run dir.", "ColdClientLoader", MB_ICONERROR);
        dbg_log::close();
        return 1;
    }
        
    bool isDllsToInjectFolderFound = false;
    if (DllsToInjectFolder.size()) {
        isDllsToInjectFolderFound = common_helpers::dir_exist(DllsToInjectFolder);
        if (!isDllsToInjectFolderFound) {
            dbg_log::write("Couldn't find the requested folder of dlls to inject");
            MessageBoxA(NULL, "Couldn't find the requested folder of dlls to inject.", "ColdClientLoader", MB_ICONERROR);
            // it was requested to make this a non intrusive error, don't shutdown the whole thing because the folder is missing
            // dbg_log::close();
            // return 1;
        }
    }
    
    auto exe_header = get_pe_header(ExeFile);
    if (exe_header.empty()) {
        dbg_log::write("Couldn't read the exe header");
        MessageBoxA(NULL, "Couldn't read the exe header.", "ColdClientLoader", MB_ICONERROR);
        dbg_log::close();
        return 1;
    }

    bool is_exe_32 = pe_helpers::is_module_32((HMODULE)&exe_header[0]);
    bool is_exe_64 = pe_helpers::is_module_64((HMODULE)&exe_header[0]);
    if ((!is_exe_32 && !is_exe_64) || (is_exe_32 && is_exe_64)) { // ARM, or just a regular file
        dbg_log::write("The requested exe is invalid (neither 32 nor 64 bit)");
        MessageBoxA(NULL, "The requested exe is invalid (neither 32 nor 64 bit)", "ColdClientLoader", MB_ICONERROR);
        dbg_log::close();
        return 1;
    }
    if (is_exe_32) {
        dbg_log::write("Detected exe arch: x32");
    } else {
        dbg_log::write("Detected exe arch: x64");
    }

    if (loader_is_32) {
        dbg_log::write("Detected loader arch: x32");
    } else {
        dbg_log::write("Detected loader arch: x64");
    }

    if (loader_is_32 != is_exe_32) {
        dbg_log::write("Arch of loader and requested exe are different, it is advised to use the appropriate one");
        if (IgnoreLoaderArchDifference.empty()) {
            MessageBoxA(NULL, "Arch of loader and requested exe are different,\nit is advised to use the appropriate one.", "ColdClientLoader", MB_OK);
        }
    }

    if (isDllsToInjectFolderFound) {
        std::wstring failed_dlls{};
        dlls_to_inject = collect_dlls_to_inject(DllsToInjectFolder, is_exe_32, failed_dlls);
        if (failed_dlls.size() && IgnoreInjectionError.empty()) {
            int choice = MessageBoxW(
                NULL,
                (L"The following dlls cannot be injected:\n" + failed_dlls + L"\nContinue ?").c_str(),
                L"ColdClientLoader",
                MB_YESNO);
            if (choice != IDYES) {
                dbg_log::close();
                return 1;
            }
        }
    }
    if (ForceInjectSteamClient.size()) {
        if (is_exe_32) {
            dlls_to_inject.insert(dlls_to_inject.begin(), ClientPath);
        } else {
            dlls_to_inject.insert(dlls_to_inject.begin(), Client64Path);
        }
    }

    if (PersistentMode != L"2" ) {
        if (AppId.size() && AppId[0]) {
            set_steam_env_vars(AppId);
        } else {
            dbg_log::write("You forgot to set the AppId");
            MessageBoxA(NULL, "You forgot to set the AppId.", "ColdClientLoader", MB_ICONERROR);
            return 1;
        }
    } else { // steam://run/
        constexpr const static wchar_t STEAM_LAUNCH_CMD_1[] = L"steam://run/";
        constexpr const static wchar_t STEAM_LAUNCH_CMD_2[] = L"-- \"steam://rungameid/";
        AppId.clear(); // we don't care about the app id in the ini
        std::wstring my_cmd = lpCmdLine && lpCmdLine[0] ? std::wstring(lpCmdLine) : std::wstring();
        dbg_log::write(L"persistent mode 2 detecting steam launch cmd from: '" + my_cmd + L"'");
        if (my_cmd.find(STEAM_LAUNCH_CMD_1) == 0) {
            AppId = my_cmd.substr(sizeof(STEAM_LAUNCH_CMD_1) / sizeof(wchar_t), my_cmd.find_first_of(L" \t"));
            dbg_log::write("persistent mode 2 got steam launch cmd #1");
        } else if (my_cmd.find(STEAM_LAUNCH_CMD_2) == 0) {
            AppId = my_cmd.substr(sizeof(STEAM_LAUNCH_CMD_2) / sizeof(wchar_t), my_cmd.find_first_of(L" \t"));
            dbg_log::write("persistent mode 2 got steam launch cmd #2");
        } else {
            dbg_log::write("persistent mode 2 didn't detect a valid steam launch cmd");
        }

        if (AppId.size()) {
            set_steam_env_vars(AppId);
            dbg_log::write(L"persistent mode 2 will use app id = " + AppId);
        } else {
            dbg_log::write("persistent mode 2 didn't find app id");
        }
    }

    if (!patch_registry_hkcu()) {
        dbg_log::close();
        MessageBoxA(NULL, "Unable to patch Registry (HKCU).", "ColdClientLoader", MB_ICONERROR);
        return 1;
    }

    if (!patch_registry_hklm()) {
        cleanup_registry_hkcu();
        
        dbg_log::close();
        MessageBoxA(NULL, "Unable to patch Registry (HKLM).", "ColdClientLoader", MB_ICONERROR);
        return 1;
    }

    patch_registry_hkcs();
    // if (!patch_registry_hkcs()) { // this fails due to admin rights, not a big deal
    //     cleanup_registry_hkcu();
    //     cleanup_registry_hklm();
        
    //     dbg_log::close();
    //     MessageBoxA(NULL, "Unable to patch Registry (HKCS).", "ColdClientLoader", MB_ICONERROR);
    //     return 1;
    // }
    
    if (PersistentMode != L"2" || AppId.size()) { // persistent mode 0 or 1, or mode 2 with defined app id   
        STARTUPINFOW info = { 0 };

        SecureZeroMemory(&info, sizeof(info));
        info.cb = sizeof(info);

        PROCESS_INFORMATION processInfo = { 0 };
        SecureZeroMemory(&processInfo, sizeof(processInfo));

        dbg_log::write("spawning the requested EXE file");
        WCHAR CommandLine[16384] = { 0 };
        _snwprintf(CommandLine, _countof(CommandLine), L"\"%ls\" %ls %ls", ExeFile.c_str(), ExeCommandLine.c_str(), lpCmdLine);
        if (!CreateProcessW(ExeFile.c_str(), CommandLine, NULL, NULL, TRUE, CREATE_SUSPENDED, NULL, ExeRunDir.c_str(), &info, &processInfo)) {
            dbg_log::write("Unable to load the requested EXE file");
            cleanup_registry_hkcu();
            cleanup_registry_hklm();
            cleanup_registry_hkcs();

            dbg_log::close();
            MessageBoxA(NULL, "Unable to load the requested EXE file.", "ColdClientLoader", MB_ICONERROR);
            return 1;
        }
        
        dbg_log::write("injecting " + std::to_string(dlls_to_inject.size()) + " dlls");
        for (const auto &dll : dlls_to_inject) {
            dbg_log::write(L"Injecting dll: '" + dll + L"' ...");
            const char *err_inject = nullptr;
            DWORD code = pe_helpers::loadlib_remote(processInfo.hProcess, dll, &err_inject);
            if (code != ERROR_SUCCESS) {
                auto err_full =
                    L"Failed to inject the dll: " + dll + L"\n" +
                    common_helpers::str_to_w(err_inject) + L"\n" +
                    common_helpers::str_to_w(pe_helpers::get_err_string(code)) + L"\n" +
                    L"Error code = " + std::to_wstring(code) + L"\n";
                dbg_log::write(err_full);
                if (IgnoreInjectionError.empty()) {
                    TerminateProcess(processInfo.hProcess, 1);
                    CloseHandle(processInfo.hProcess);
                    CloseHandle(processInfo.hThread);

                    cleanup_registry_hkcu();
                    cleanup_registry_hklm();
                    cleanup_registry_hkcs();

                    dbg_log::close();
                    MessageBoxW(NULL, err_full.c_str(), L"ColdClientLoader", MB_ICONERROR);
                    return 1;
                }
            } else {
                dbg_log::write("Injected!");
            }
        }

        // run
        if (ResumeByDebugger.empty()) {
            dbg_log::write("resuming the main thread of the exe");
            // MessageBoxA(nullptr, "Wait then press ok", "Wait some seconds", MB_OK);
            // games like house flipper 2 won't launch until the previous exe has exited, which takes some seconds
            if (PersistentMode == L"2") std::this_thread::sleep_for(std::chrono::seconds(5));
            ResumeThread(processInfo.hThread);
        } else {
            std::string msg = "Attach a debugger now to PID " + std::to_string(processInfo.dwProcessId) + " and resume its main thread";
            dbg_log::write(msg);
            MessageBoxA(NULL, msg.c_str(), "ColdClientLoader", MB_OK);
        }
        
        // wait
        WaitForSingleObject(processInfo.hThread, INFINITE);

        CloseHandle(processInfo.hProcess);
        CloseHandle(processInfo.hThread);
    }

    if (PersistentMode == L"1" ) {
        MessageBoxA(NULL, "Press OK when you have finished playing to close the loader", "Cold Client Loader (waiting)", MB_OK);
    } else if (PersistentMode == L"2" && AppId.empty()) {
        MessageBoxA(NULL, "Start the game, then press OK when you have finished playing to close the loader", "Cold Client Loader (waiting)", MB_OK);
    }

    if (PersistentMode != L"2" || AppId.empty()) { // persistent mode 0 or 1, or mode 2 without app id   
        cleanup_registry_hkcu();
        cleanup_registry_hklm();
        cleanup_registry_hkcs();
    }

    dbg_log::close();
    return 0;
}
