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
    
    // log everything
    dbg_log::write(L"SteamClient64Dll: " + Client64Path);
    dbg_log::write(L"SteamClient: " + ClientPath);
    dbg_log::write(L"Exe: " + ExeFile);
    dbg_log::write(L"ExeRunDir: " + ExeRunDir);
    dbg_log::write(L"ExeCommandLine: " + ExeCommandLine);
    dbg_log::write(L"AppId: " + AppId);

    if (AppId.size() && AppId[0]) {
        SetEnvironmentVariableW(L"SteamAppId", AppId.c_str());
        SetEnvironmentVariableW(L"SteamGameId", AppId.c_str());
        SetEnvironmentVariableW(L"SteamOverlayGameId", AppId.c_str());
    } else {
        dbg_log::write("You forgot to set the AppId");
        MessageBoxA(NULL, "You forgot to set the AppId.", "ColdClientLoader", MB_ICONERROR);
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

    if (!common_helpers::file_exist(ExeFile)) {
        dbg_log::write("Couldn't find the requested Exe file");
        MessageBoxA(NULL, "Couldn't find the requested Exe file.", "ColdClientLoader", MB_ICONERROR);
        dbg_log::close();
        return 1;
    }

    if (!common_helpers::dir_exist(ExeRunDir)) {
        dbg_log::write("Couldn't find the requested Exe run dir");
        MessageBoxA(NULL, "Couldn't find the requested Exe run dir.", "ColdClientLoader", MB_ICONERROR);
        dbg_log::close();
        return 1;
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

    // dll to inject
    bool inject_extra_dll = false;
    std::wstring extra_dll = common_helpers::to_absolute(
        get_ini_value(L"Extra", L"InjectDll"),
        pe_helpers::get_current_exe_path_w()
    );
    if (extra_dll.size()) {
        dbg_log::write(L"InjectDll: " + extra_dll);
        if (!common_helpers::file_exist(extra_dll)) {
            dbg_log::write("Couldn't find the requested dll to inject");
            MessageBoxA(NULL, "Couldn't find the requested dll to inject.", "ColdClientLoader", MB_ICONERROR);
            dbg_log::close();
            return 1;
        }
        inject_extra_dll = true;
    }

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
    
    if (inject_extra_dll) {
        const char *err_inject = nullptr;
        DWORD code = pe_helpers::loadlib_remote(processInfo.hProcess, extra_dll, &err_inject);
        if (code != ERROR_SUCCESS) {
            TerminateProcess(processInfo.hProcess, 1);
            std::string err_full =
                "Failed to inject the requested dll:\n" +
                std::string(err_inject) + "\n" +
                pe_helpers::get_err_string(code) + "\n" +
                "Error code = " + std::to_string(code) + "\n";
            dbg_log::write(err_full);
            MessageBoxA(NULL, err_full.c_str(), "ColdClientLoader", MB_ICONERROR);
            dbg_log::close();
            return 1;
        }
    }

    bool run_exe = true;
#ifndef EMU_RELEASE_BUILD
    std::wstring resume_by_dbg = get_ini_value(L"Debug", L"ResumeByDebugger");
    dbg_log::write(L"Debug::ResumeByDebugger: " + resume_by_dbg);
    for (auto &c : resume_by_dbg) {
        c = (wchar_t)std::tolower((int)c);
    }
    if (resume_by_dbg == L"1" || resume_by_dbg == L"y" || resume_by_dbg == L"yes" || resume_by_dbg == L"true") {
        run_exe = false;
        std::string msg = "Attach a debugger now to PID " + std::to_string(processInfo.dwProcessId) + " and resume its main thread";
        dbg_log::write(msg);
        MessageBoxA(NULL, msg.c_str(), "ColdClientLoader", MB_OK);
    }
#endif

    // run
    if (run_exe) {
        ResumeThread(processInfo.hThread);
    }
    // wait
    WaitForSingleObject(processInfo.hThread, INFINITE);

    CloseHandle(processInfo.hThread);
    CloseHandle(processInfo.hProcess);

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
