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


static const std::wstring IniFile = pe_helpers::get_current_exe_path_w() + L"ColdClientLoader.ini";
static const std::wstring dbg_file = pe_helpers::get_current_exe_path_w() + L"COLD_LDR_LOG.txt";
constexpr static const char STEAM_UNIVERSE[] = "Public";

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
    try
    {
        std::ifstream file(filepath, std::ios::binary);
        if (!file.is_open()) throw;

        file.seekg(0, std::ios::beg);

        // 2MB is enough
        std::vector<uint8_t> data(2 * 1024 * 1024, 0);
        file.read((char *)&data[0], data.size());
        file.close();

        return data;
    }
    catch(const std::exception& e)
    {
        return std::vector<uint8_t>();
    }
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

    std::wstring Client64Path = common_helpers::to_absolute(
        get_ini_value(L"SteamClient", L"SteamClient64Dll"),
        pe_helpers::get_current_exe_path_w()
    );

    std::wstring ClientPath = common_helpers::to_absolute(
        get_ini_value(L"SteamClient", L"SteamClientDll"),
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
    std::wstring InjectClient = get_ini_value(L"SteamClient", L"ForceInjectSteamClient");
    
    std::wstring resume_by_dbg = get_ini_value(L"Debug", L"ResumeByDebugger");

    // dlls to inject
    std::wstring extra_dlls_folder = common_helpers::to_absolute(
        get_ini_value(L"Extra", L"DllsToInjectFolder"),
        pe_helpers::get_current_exe_path_w()
    );
    std::wstring IgnoreInjectionError = get_ini_value(L"Extra", L"IgnoreInjectionError", L"1");

    // log everything
    dbg_log::write(L"SteamClient::Exe: " + ExeFile);
    dbg_log::write(L"SteamClient::ExeRunDir: " + ExeRunDir);
    dbg_log::write(L"SteamClient::ExeCommandLine: " + ExeCommandLine);
    dbg_log::write(L"SteamClient::AppId: " + AppId);
    dbg_log::write(L"SteamClient::SteamClient: " + ClientPath);
    dbg_log::write(L"SteamClient::SteamClient64Dll: " + Client64Path);
    dbg_log::write(L"SteamClient::ForceInjectSteamClient: " + InjectClient);
    dbg_log::write(L"Debug::ResumeByDebugger: " + resume_by_dbg);
    dbg_log::write(L"Extra::DllsToInjectFolder: " + extra_dlls_folder);
    dbg_log::write(L"Extra::IgnoreInjectionError: " + IgnoreInjectionError);

    if (AppId.size() && AppId[0]) {
        SetEnvironmentVariableW(L"SteamAppId", AppId.c_str());
        SetEnvironmentVariableW(L"SteamGameId", AppId.c_str());
        SetEnvironmentVariableW(L"SteamOverlayGameId", AppId.c_str());
    } else {
        dbg_log::write("You forgot to set the AppId");
        MessageBoxA(NULL, "You forgot to set the AppId.", "ColdClientLoader", MB_ICONERROR);
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
        dbg_log::write(L"Setting exe run dir to: " + ExeRunDir);
    }
    if (!common_helpers::dir_exist(ExeRunDir)) {
        dbg_log::write("Couldn't find the requested Exe run dir");
        MessageBoxA(NULL, "Couldn't find the requested Exe run dir.", "ColdClientLoader", MB_ICONERROR);
        dbg_log::close();
        return 1;
    }

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

    for (auto &c : resume_by_dbg) {
        c = (wchar_t)std::tolower((int)c);
    }
    if (resume_by_dbg != L"1" && resume_by_dbg != L"y" && resume_by_dbg != L"yes" && resume_by_dbg != L"true") {
        resume_by_dbg.clear();
    }

    for (auto &c : IgnoreInjectionError) {
        c = (wchar_t)std::tolower((int)c);
    }
    if (IgnoreInjectionError != L"1" && IgnoreInjectionError != L"y" && IgnoreInjectionError != L"yes" && IgnoreInjectionError != L"true") {
        IgnoreInjectionError.clear();
    }

    for (auto &c : InjectClient) {
        c = (wchar_t)std::tolower((int)c);
    }
    if (InjectClient != L"1" && InjectClient != L"y" && InjectClient != L"yes" && InjectClient != L"true") {
        InjectClient.clear();
    }

    if (extra_dlls_folder.size()) {
        if (!common_helpers::dir_exist(extra_dlls_folder)) {
            dbg_log::write("Couldn't find the requested folder of dlls to inject");
            MessageBoxA(NULL, "Couldn't find the requested folder of dlls to inject.", "ColdClientLoader", MB_ICONERROR);
            dbg_log::close();
            return 1;
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

    bool loader_is_32 = pe_helpers::is_module_32(GetModuleHandleW(NULL));
    if (pe_helpers::is_module_32(GetModuleHandleW(NULL))) {
        dbg_log::write("Detected loader arch: x32");
    } else {
        dbg_log::write("Detected loader arch: x64");
    }

    if (loader_is_32 != is_exe_32) {
        dbg_log::write("Arch of loader and requested exe are different, it is advised to use the appropriate one");
        MessageBoxA(NULL, "Arch of loader and requested exe are different,\nit is advised to use the appropriate one.", "ColdClientLoader", MB_OK);
    }

    std::vector<std::wstring> dlls_to_inject{};
    if (extra_dlls_folder.size()) {
        std::wstring failed_dlls = std::wstring{};

        for (auto const& dir_entry :
             std::filesystem::recursive_directory_iterator(extra_dlls_folder, std::filesystem::directory_options::follow_directory_symlink)) {
                if (std::filesystem::is_directory(dir_entry.path())) continue;

                auto dll_path = dir_entry.path().wstring();
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

                if ((is_dll_32 && is_exe_32) || (is_dll_64 && is_exe_64)) {
                    dlls_to_inject.push_back(dll_path);
                    dbg_log::write(L"Dll " + dll_path + L" will be injected");
                } else {
                    dbg_log::write(L"Dll " + dll_path + L" has a different arch than the exe and will be ignored");
                    failed_dlls += dll_path + L"\n";
                }
        }

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

    HKEY Registrykey = { 0 };
    // Declare some variables to be used for Steam registry.
    DWORD UserId = 0x03100004771F810D & 0xffffffff;
    DWORD ProcessID = GetCurrentProcessId();

    bool orig_steam = false;
    DWORD keyType = REG_SZ;
    WCHAR OrgSteamCDir[8192] = { 0 };
    WCHAR OrgSteamCDir64[8192] = { 0 };
    DWORD Size1 = _countof(OrgSteamCDir);
    DWORD Size2 = _countof(OrgSteamCDir64);
    if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Valve\\Steam\\ActiveProcess", 0, KEY_ALL_ACCESS, &Registrykey) == ERROR_SUCCESS)
    {
        orig_steam = true;
        // Get original values to restore later.
        RegQueryValueExW(Registrykey, L"SteamClientDll", 0, &keyType, (LPBYTE)& OrgSteamCDir, &Size1);
        RegQueryValueExW(Registrykey, L"SteamClientDll64", 0, &keyType, (LPBYTE)& OrgSteamCDir64, &Size2);
    } else {
        if (RegCreateKeyExW(HKEY_CURRENT_USER, L"Software\\Valve\\Steam\\ActiveProcess", 0, 0, REG_OPTION_NON_VOLATILE,
            KEY_ALL_ACCESS, NULL, &Registrykey, NULL) != ERROR_SUCCESS)
        {
            dbg_log::write("Unable to patch Steam process informations on the Windows registry (ActiveProcess), error = " + std::to_string(GetLastError()));
            MessageBoxA(NULL, "Unable to patch Steam process informations on the Windows registry.", "ColdClientLoader", MB_ICONERROR);
            dbg_log::close();
            return 1;
        }
    }

    // Set values to Windows registry.
    RegSetValueExA(Registrykey, "ActiveUser", NULL, REG_DWORD, (const BYTE *)&UserId, sizeof(DWORD));
    RegSetValueExA(Registrykey, "pid", NULL, REG_DWORD, (const BYTE *)&ProcessID, sizeof(DWORD));
    RegSetValueExW(Registrykey, L"SteamClientDll", NULL, REG_SZ, (const BYTE *)ClientPath.c_str(), (ClientPath.size() + 1) * sizeof(ClientPath[0]));
    RegSetValueExW(Registrykey, L"SteamClientDll64", NULL, REG_SZ, (const BYTE *)Client64Path.c_str(), (Client64Path.size() + 1) * sizeof(Client64Path[0]));
    RegSetValueExA(Registrykey, "Universe", NULL, REG_SZ, (const BYTE *)STEAM_UNIVERSE, (DWORD)sizeof(STEAM_UNIVERSE));

    // Close the HKEY Handle.
    RegCloseKey(Registrykey);

    // spawn the exe
    STARTUPINFOW info = { 0 };
    SecureZeroMemory(&info, sizeof(info));
    info.cb = sizeof(info);

    PROCESS_INFORMATION processInfo = { 0 };
    SecureZeroMemory(&processInfo, sizeof(processInfo));

    WCHAR CommandLine[16384] = { 0 };
    _snwprintf(CommandLine, _countof(CommandLine), L"\"%ls\" %ls %ls", ExeFile.c_str(), ExeCommandLine.c_str(), lpCmdLine);
    if (!CreateProcessW(ExeFile.c_str(), CommandLine, NULL, NULL, TRUE, CREATE_SUSPENDED, NULL, ExeRunDir.c_str(), &info, &processInfo))
    {
        dbg_log::write("Unable to load the requested EXE file");
        MessageBoxA(NULL, "Unable to load the requested EXE file.", "ColdClientLoader", MB_ICONERROR);
        dbg_log::close();
        return 1;
    }
    
    if (InjectClient.size()) {
        if (is_exe_32) {
            dlls_to_inject.insert(dlls_to_inject.begin(), ClientPath);
        } else {
            dlls_to_inject.insert(dlls_to_inject.begin(), Client64Path);
        }
    }
    for (const auto &dll_path : dlls_to_inject) {
        const char *err_inject = nullptr;
        DWORD code = pe_helpers::loadlib_remote(processInfo.hProcess, dll_path, &err_inject);
        if (code != ERROR_SUCCESS) {
            std::wstring err_full =
                L"Failed to inject the dll: " + dll_path + L"\n" +
                common_helpers::str_to_w(err_inject) + L"\n" +
                common_helpers::str_to_w(pe_helpers::get_err_string(code)) + L"\n" +
                L"Error code = " + std::to_wstring(code) + L"\n";
            dbg_log::write(err_full);
            if (IgnoreInjectionError.empty()) {
                TerminateProcess(processInfo.hProcess, 1);
                CloseHandle(processInfo.hProcess);
                CloseHandle(processInfo.hThread);
                MessageBoxW(NULL, err_full.c_str(), L"ColdClientLoader", MB_ICONERROR);
                dbg_log::close();
                return 1;
            }
        } else {

        }
    }

    // run
    if (resume_by_dbg.empty()) {
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

    if (orig_steam) {
        if (RegOpenKeyExW(HKEY_CURRENT_USER, L"Software\\Valve\\Steam\\ActiveProcess", 0, KEY_ALL_ACCESS, &Registrykey) == ERROR_SUCCESS)
        {
            // Restore the values.
            RegSetValueExW(Registrykey, L"SteamClientDll", NULL, REG_SZ, (LPBYTE)OrgSteamCDir, Size1);
            RegSetValueExW(Registrykey, L"SteamClientDll64", NULL, REG_SZ, (LPBYTE)OrgSteamCDir64, Size2);

            // Close the HKEY Handle.
            RegCloseKey(Registrykey);
        } else {
            dbg_log::write("Unable to restore the original Steam process informations in the Windows registry, error = " + std::to_string(GetLastError()));
        }
    }

    dbg_log::close();
    return 0;
}
