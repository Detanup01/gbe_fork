/* Version.dll proxy with Steam injection - DEBUG VERSION */

#define WIN32_LEAN_AND_MEAN
#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <shlwapi.h>
#include <fstream>
#include <string>
#include <map>
#include <ctime>

#pragma comment(lib, "shlwapi.lib")

// =============================================================================
// GLOBALS
// =============================================================================

static HMODULE g_system_version_dll = nullptr;
static HANDLE g_injection_thread = nullptr;
static bool g_is_shutting_down = false;

// No registry backup needed - we use real Steam path from HKLM

// =============================================================================
// DEBUG HELPERS
// =============================================================================

static void WriteDebugLog(const std::string& message) {
    std::ofstream logFile("debug_log.txt", std::ios::app);
    if (logFile.is_open()) {
        time_t now = time(0);
        char* timeStr = ctime(&now);
        timeStr[strlen(timeStr) - 1] = '\0'; // Remove newline
        
        logFile << "[" << timeStr << "] " << message << std::endl;
        logFile.close();
    }
}

static void WriteEnvironmentVariablesToLog() {
    // Write environment variables to separate debug_env.txt file
    std::ofstream envFile("debug_env.txt", std::ios::out);
    if (envFile.is_open()) {
        envFile << "=== ENVIRONMENT VARIABLES (DEBUG) ===" << std::endl;
        
        // Check our custom variables
        char buffer[MAX_PATH];
        if (GetEnvironmentVariableA("SteamAppId", buffer, MAX_PATH)) {
            envFile << "SteamAppId=" << buffer << std::endl;
        }
        if (GetEnvironmentVariableA("SteamGameId", buffer, MAX_PATH)) {
            envFile << "SteamGameId=" << buffer << std::endl;
        }
        if (GetEnvironmentVariableA("SteamClientLaunch", buffer, MAX_PATH)) {
            envFile << "SteamClientLaunch=" << buffer << std::endl;
        }
        if (GetEnvironmentVariableA("SteamEnv", buffer, MAX_PATH)) {
            envFile << "SteamEnv=" << buffer << std::endl;
        }
        if (GetEnvironmentVariableA("SteamAppUser", buffer, MAX_PATH)) {
            envFile << "SteamAppUser=" << buffer << std::endl;
        }
        if (GetEnvironmentVariableA("SteamUser", buffer, MAX_PATH)) {
            envFile << "SteamUser=" << buffer << std::endl;
        }
        if (GetEnvironmentVariableA("SteamPath", buffer, MAX_PATH)) {
            envFile << "SteamPath=" << buffer << std::endl;
        }
        
        envFile.close();
    }
}

// Registry debugging removed as requested

// =============================================================================
// HELPERS
// =============================================================================

static HMODULE LoadSystemVersionDll() {
    if (g_system_version_dll) return g_system_version_dll;
    
    char system_path[MAX_PATH];
    GetSystemDirectoryA(system_path, MAX_PATH);
    strcat_s(system_path, "\\version.dll");
    g_system_version_dll = LoadLibraryA(system_path);
    
    WriteDebugLog("Loaded system version.dll from: " + std::string(system_path));
    return g_system_version_dll;
}

static void CleanupVersionDll() {
    g_is_shutting_down = true;
    WriteDebugLog("Cleanup started - shutting down");
    
    if (g_injection_thread) {
        WaitForSingleObject(g_injection_thread, 2000);
        CloseHandle(g_injection_thread);
        g_injection_thread = nullptr;
    }
    
    if (g_system_version_dll) {
        FreeLibrary(g_system_version_dll);
        g_system_version_dll = nullptr;
    }
}

static std::string ReadAppIdFromFile() {
    WriteDebugLog("Reading AppID from steam_settings\\steam_appid.txt");
    
    std::ifstream file("steam_settings\\steam_appid.txt");
    if (!file.is_open()) {
        WriteDebugLog("AppID file not found, using default 480");
        return "480"; // Default to Spacewar
    }
    
    std::string appId;
    std::getline(file, appId);
    file.close();
    
    // Remove whitespace
    appId.erase(0, appId.find_first_not_of(" \t\r\n"));
    appId.erase(appId.find_last_not_of(" \t\r\n") + 1);
    
    std::string result = appId.empty() ? "480" : appId;
    WriteDebugLog("Using AppID: " + result);
    return result;
}

static void SetEnvironmentVariables(const std::string& appId) {
    WriteDebugLog("Setting environment variables for AppID: " + appId);
    
    SetEnvironmentVariableA("SteamAppId", appId.c_str());
    SetEnvironmentVariableA("SteamGameId", appId.c_str());
    SetEnvironmentVariableA("SteamClientLaunch", "1");
    SetEnvironmentVariableA("SteamEnv", "1");
    SetEnvironmentVariableA("SteamAppUser", "cold_player");
    SetEnvironmentVariableA("SteamUser", "cold_player");
    
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    SetEnvironmentVariableA("SteamPath", exePath);
    
    WriteDebugLog("Environment variables set, SteamPath: " + std::string(exePath));
    
    // Write environment variables to log
    WriteEnvironmentVariablesToLog();
}


static void SetRegistryEntries(const std::string& appId) {
    WriteDebugLog("Setting registry entries for AppID: " + appId);
    
    HKEY hKey;
    DWORD dwDisp;
    
    // Set ActiveProcess entries
    if (RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Valve\\Steam\\ActiveProcess", 0, NULL, 
                       REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisp) == ERROR_SUCCESS) {
        
        DWORD pid = GetCurrentProcessId();
        RegSetValueExA(hKey, "pid", 0, REG_DWORD, (const BYTE*)&pid, sizeof(DWORD));
        WriteDebugLog("Set pid: " + std::to_string(pid));
        
        // Our steamclient64.dll path
        char currentDir[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, currentDir);
        std::string steamclientPath = std::string(currentDir) + "\\steamclient64.dll";
        
        RegSetValueExA(hKey, "SteamClientDll64", 0, REG_SZ, (const BYTE*)steamclientPath.c_str(), 
                      (DWORD)steamclientPath.length() + 1);
        WriteDebugLog("Set SteamClientDll64: " + steamclientPath);
        
        DWORD activeUser = 0;
        RegSetValueExA(hKey, "ActiveUser", 0, REG_DWORD, (const BYTE*)&activeUser, sizeof(DWORD));
        
        const char* universe = "Public";
        RegSetValueExA(hKey, "Universe", 0, REG_SZ, (const BYTE*)universe, (DWORD)strlen(universe) + 1);
        
        RegCloseKey(hKey);
    }
    
    // Set main Steam entries
    if (RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Valve\\Steam", 0, NULL, 
                       REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dwDisp) == ERROR_SUCCESS) {
        
        DWORD runningAppId = std::stoul(appId);
        RegSetValueExA(hKey, "RunningAppID", 0, REG_DWORD, (const BYTE*)&runningAppId, sizeof(DWORD));
        WriteDebugLog("Set RunningAppID: " + std::to_string(runningAppId));
        
        char currentDir[MAX_PATH];
        GetCurrentDirectoryA(MAX_PATH, currentDir);
        RegSetValueExA(hKey, "SteamPath", 0, REG_SZ, (const BYTE*)currentDir, (DWORD)strlen(currentDir) + 1);
        WriteDebugLog("Set SteamPath: " + std::string(currentDir));
        
        RegCloseKey(hKey);
    }
    
    WriteDebugLog("Registry entries set successfully");
}

static void RestoreRegistryValues() {
    WriteDebugLog("Restoring registry values after 7 seconds");
    
    HKEY hKey;
    char buffer[MAX_PATH];
    DWORD bufferSize = MAX_PATH;
    DWORD valueType;
    
    // Get real Steam install path from HKLM for SteamClientDll64 (keep original format)
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\WOW6432Node\\Valve\\Steam", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        if (RegQueryValueExA(hKey, "InstallPath", NULL, &valueType, (BYTE*)buffer, &bufferSize) == ERROR_SUCCESS) {
            // SteamClientDll64 uses the original path format from HKLM (with backslashes)
            std::string realSteamClientDll = std::string(buffer) + "\\steamclient64.dll";
            
            // Restore SteamClientDll64
            HKEY hKeyCU;
            if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Valve\\Steam\\ActiveProcess", 0, KEY_WRITE, &hKeyCU) == ERROR_SUCCESS) {
                RegSetValueExA(hKeyCU, "SteamClientDll64", 0, REG_SZ, (const BYTE*)realSteamClientDll.c_str(), 
                              (DWORD)realSteamClientDll.length() + 1);
                WriteDebugLog("Restored SteamClientDll64: " + realSteamClientDll);
                RegCloseKey(hKeyCU);
            }
            
            // SteamPath uses forward slashes and lowercase
            std::string realSteamPath = std::string(buffer);
            // Convert to lowercase
            for (size_t i = 0; i < realSteamPath.length(); i++) {
                realSteamPath[i] = tolower(realSteamPath[i]);
            }
            // Convert backslashes to forward slashes
            for (size_t i = 0; i < realSteamPath.length(); i++) {
                if (realSteamPath[i] == '\\') {
                    realSteamPath[i] = '/';
                }
            }
            
            // Restore SteamPath
            if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Valve\\Steam", 0, KEY_WRITE, &hKeyCU) == ERROR_SUCCESS) {
                RegSetValueExA(hKeyCU, "SteamPath", 0, REG_SZ, (const BYTE*)realSteamPath.c_str(), 
                              (DWORD)realSteamPath.length() + 1);
                WriteDebugLog("Restored SteamPath: " + realSteamPath);
                RegCloseKey(hKeyCU);
            }
        }
        RegCloseKey(hKey);
    } else {
        // Fallback: try 32-bit registry
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, "SOFTWARE\\Valve\\Steam", 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            if (RegQueryValueExA(hKey, "InstallPath", NULL, &valueType, (BYTE*)buffer, &bufferSize) == ERROR_SUCCESS) {
                // SteamClientDll64 uses the original path format from HKLM (with backslashes)
                std::string realSteamClientDll = std::string(buffer) + "\\steamclient64.dll";
                
                // Restore SteamClientDll64
                HKEY hKeyCU;
                if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Valve\\Steam\\ActiveProcess", 0, KEY_WRITE, &hKeyCU) == ERROR_SUCCESS) {
                    RegSetValueExA(hKeyCU, "SteamClientDll64", 0, REG_SZ, (const BYTE*)realSteamClientDll.c_str(), 
                                  (DWORD)realSteamClientDll.length() + 1);
                    WriteDebugLog("Restored SteamClientDll64: " + realSteamClientDll);
                    RegCloseKey(hKeyCU);
                }
                
                // SteamPath uses forward slashes and lowercase
                std::string realSteamPath = std::string(buffer);
                // Convert to lowercase
                for (size_t i = 0; i < realSteamPath.length(); i++) {
                    realSteamPath[i] = tolower(realSteamPath[i]);
                }
                // Convert backslashes to forward slashes
                for (size_t i = 0; i < realSteamPath.length(); i++) {
                    if (realSteamPath[i] == '\\') {
                        realSteamPath[i] = '/';
                    }
                }
                
                // Restore SteamPath
                if (RegOpenKeyExA(HKEY_CURRENT_USER, "Software\\Valve\\Steam", 0, KEY_WRITE, &hKeyCU) == ERROR_SUCCESS) {
                    RegSetValueExA(hKeyCU, "SteamPath", 0, REG_SZ, (const BYTE*)realSteamPath.c_str(), 
                                  (DWORD)realSteamPath.length() + 1);
                    WriteDebugLog("Restored SteamPath: " + realSteamPath);
                    RegCloseKey(hKeyCU);
                }
            }
            RegCloseKey(hKey);
        } else {
            WriteDebugLog("Could not find real Steam path - no restore needed");
        }
    }
    
    WriteDebugLog("Registry restoration completed");
}

// 7-second cleanup thread
static DWORD WINAPI RegistryCleanupThread(LPVOID lpParam) {
    WriteDebugLog("Registry cleanup thread started - waiting 7 seconds");
    Sleep(7000);
    
    if (!g_is_shutting_down) {
        RestoreRegistryValues();
    } else {
        WriteDebugLog("Skipping registry restore - shutting down");
    }
    
    return 0;
}

static DWORD WINAPI SteamInjectionThread(LPVOID lpParam) {
    WriteDebugLog("=== Steam injection thread started ===");
    
    // Read App ID
    std::string appId = ReadAppIdFromFile();
    
    // Set environment
    SetEnvironmentVariables(appId);
    
    // Set registry
    SetRegistryEntries(appId);
    
    // Load steamclient64.dll
    if (PathFileExistsA("steamclient64.dll")) {
        WriteDebugLog("Loading steamclient64.dll");
        HMODULE hSteamClient = LoadLibraryA("steamclient64.dll");
        if (hSteamClient) {
            WriteDebugLog("steamclient64.dll loaded successfully");
        } else {
            WriteDebugLog("Failed to load steamclient64.dll");
        }
        
        // Start cleanup thread
        CreateThread(NULL, 0, RegistryCleanupThread, NULL, 0, NULL);
    } else {
        WriteDebugLog("steamclient64.dll not found!");
    }
    
    WriteDebugLog("Steam injection thread completed");
    return 0;
}

// =============================================================================
// VERSION.DLL EXPORTS (simplified for debug)
// =============================================================================

extern "C" {

BOOL WINAPI GetFileVersionInfoA(LPCSTR lptstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData) {
    static HMODULE dll = LoadSystemVersionDll();
    if (dll) {
        typedef BOOL (WINAPI *Func_t)(LPCSTR, DWORD, DWORD, LPVOID);
        static Func_t func = (Func_t)GetProcAddress(dll, "GetFileVersionInfoA");
        if (func) return func(lptstrFilename, dwHandle, dwLen, lpData);
    }
    return FALSE;
}

DWORD WINAPI GetFileVersionInfoSizeA(LPCSTR lptstrFilename, LPDWORD lpdwHandle) {
    static HMODULE dll = LoadSystemVersionDll();
    if (dll) {
        typedef DWORD (WINAPI *Func_t)(LPCSTR, LPDWORD);
        static Func_t func = (Func_t)GetProcAddress(dll, "GetFileVersionInfoSizeA");
        if (func) return func(lptstrFilename, lpdwHandle);
    }
    return 0;
}

BOOL WINAPI GetFileVersionInfoW(LPCWSTR lptstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData) {
    static HMODULE dll = LoadSystemVersionDll();
    if (dll) {
        typedef BOOL (WINAPI *Func_t)(LPCWSTR, DWORD, DWORD, LPVOID);
        static Func_t func = (Func_t)GetProcAddress(dll, "GetFileVersionInfoW");
        if (func) return func(lptstrFilename, dwHandle, dwLen, lpData);
    }
    return FALSE;
}

DWORD WINAPI GetFileVersionInfoSizeW(LPCWSTR lptstrFilename, LPDWORD lpdwHandle) {
    static HMODULE dll = LoadSystemVersionDll();
    if (dll) {
        typedef DWORD (WINAPI *Func_t)(LPCWSTR, LPDWORD);
        static Func_t func = (Func_t)GetProcAddress(dll, "GetFileVersionInfoSizeW");
        if (func) return func(lptstrFilename, lpdwHandle);
    }
    return 0;
}

BOOL WINAPI GetFileVersionInfoExA(DWORD dwFlags, LPCSTR lpwstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData) {
    static HMODULE dll = LoadSystemVersionDll();
    if (dll) {
        typedef BOOL (WINAPI *Func_t)(DWORD, LPCSTR, DWORD, DWORD, LPVOID);
        static Func_t func = (Func_t)GetProcAddress(dll, "GetFileVersionInfoExA");
        if (func) return func(dwFlags, lpwstrFilename, dwHandle, dwLen, lpData);
    }
    return GetFileVersionInfoA(lpwstrFilename, dwHandle, dwLen, lpData);
}

BOOL WINAPI GetFileVersionInfoExW(DWORD dwFlags, LPCWSTR lpwstrFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData) {
    static HMODULE dll = LoadSystemVersionDll();
    if (dll) {
        typedef BOOL (WINAPI *Func_t)(DWORD, LPCWSTR, DWORD, DWORD, LPVOID);
        static Func_t func = (Func_t)GetProcAddress(dll, "GetFileVersionInfoExW");
        if (func) return func(dwFlags, lpwstrFilename, dwHandle, dwLen, lpData);
    }
    return GetFileVersionInfoW(lpwstrFilename, dwHandle, dwLen, lpData);
}

DWORD WINAPI GetFileVersionInfoSizeExA(DWORD dwFlags, LPCSTR lpwstrFilename, LPDWORD lpdwHandle) {
    static HMODULE dll = LoadSystemVersionDll();
    if (dll) {
        typedef DWORD (WINAPI *Func_t)(DWORD, LPCSTR, LPDWORD);
        static Func_t func = (Func_t)GetProcAddress(dll, "GetFileVersionInfoSizeExA");
        if (func) return func(dwFlags, lpwstrFilename, lpdwHandle);
    }
    return GetFileVersionInfoSizeA(lpwstrFilename, lpdwHandle);
}

DWORD WINAPI GetFileVersionInfoSizeExW(DWORD dwFlags, LPCWSTR lpwstrFilename, LPDWORD lpdwHandle) {
    static HMODULE dll = LoadSystemVersionDll();
    if (dll) {
        typedef DWORD (WINAPI *Func_t)(DWORD, LPCWSTR, LPDWORD);
        static Func_t func = (Func_t)GetProcAddress(dll, "GetFileVersionInfoSizeExW");
        if (func) return func(dwFlags, lpwstrFilename, lpdwHandle);
    }
    return GetFileVersionInfoSizeW(lpwstrFilename, lpdwHandle);
}

DWORD WINAPI VerFindFileA(DWORD uFlags, LPCSTR szFileName, LPCSTR szWinDir, LPCSTR szAppDir, LPSTR szCurDir, PUINT puCurDirLen, LPSTR szDestDir, PUINT puDestDirLen) {
    static HMODULE dll = LoadSystemVersionDll();
    if (dll) {
        typedef DWORD (WINAPI *Func_t)(DWORD, LPCSTR, LPCSTR, LPCSTR, LPSTR, PUINT, LPSTR, PUINT);
        static Func_t func = (Func_t)GetProcAddress(dll, "VerFindFileA");
        if (func) return func(uFlags, szFileName, szWinDir, szAppDir, szCurDir, puCurDirLen, szDestDir, puDestDirLen);
    }
    return 0x0001;
}

DWORD WINAPI VerFindFileW(DWORD uFlags, LPCWSTR szFileName, LPCWSTR szWinDir, LPCWSTR szAppDir, LPWSTR szCurDir, PUINT puCurDirLen, LPWSTR szDestDir, PUINT puDestDirLen) {
    static HMODULE dll = LoadSystemVersionDll();
    if (dll) {
        typedef DWORD (WINAPI *Func_t)(DWORD, LPCWSTR, LPCWSTR, LPCWSTR, LPWSTR, PUINT, LPWSTR, PUINT);
        static Func_t func = (Func_t)GetProcAddress(dll, "VerFindFileW");
        if (func) return func(uFlags, szFileName, szWinDir, szAppDir, szCurDir, puCurDirLen, szDestDir, puDestDirLen);
    }
    return 0x0001;
}

DWORD WINAPI VerInstallFileA(DWORD uFlags, LPCSTR szSrcFileName, LPCSTR szDestFileName, LPCSTR szSrcDir, LPCSTR szDestDir, LPCSTR szCurDir, LPSTR szTmpFile, PUINT puTmpFileLen) {
    static HMODULE dll = LoadSystemVersionDll();
    if (dll) {
        typedef DWORD (WINAPI *Func_t)(DWORD, LPCSTR, LPCSTR, LPCSTR, LPCSTR, LPCSTR, LPSTR, PUINT);
        static Func_t func = (Func_t)GetProcAddress(dll, "VerInstallFileA");
        if (func) return func(uFlags, szSrcFileName, szDestFileName, szSrcDir, szDestDir, szCurDir, szTmpFile, puTmpFileLen);
    }
    return 0x0001;
}

DWORD WINAPI VerInstallFileW(DWORD uFlags, LPCWSTR szSrcFileName, LPCWSTR szDestFileName, LPCWSTR szSrcDir, LPCWSTR szDestDir, LPCWSTR szCurDir, LPWSTR szTmpFile, PUINT puTmpFileLen) {
    static HMODULE dll = LoadSystemVersionDll();
    if (dll) {
        typedef DWORD (WINAPI *Func_t)(DWORD, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, LPCWSTR, LPWSTR, PUINT);
        static Func_t func = (Func_t)GetProcAddress(dll, "VerInstallFileW");
        if (func) return func(uFlags, szSrcFileName, szDestFileName, szSrcDir, szDestDir, szCurDir, szTmpFile, puTmpFileLen);
    }
    return 0x0001;
}

DWORD WINAPI VerLanguageNameA(DWORD wLang, LPSTR szLang, DWORD cchLang) {
    static HMODULE dll = LoadSystemVersionDll();
    if (dll) {
        typedef DWORD (WINAPI *Func_t)(DWORD, LPSTR, DWORD);
        static Func_t func = (Func_t)GetProcAddress(dll, "VerLanguageNameA");
        if (func) return func(wLang, szLang, cchLang);
    }
    return 0;
}

DWORD WINAPI VerLanguageNameW(DWORD wLang, LPWSTR szLang, DWORD cchLang) {
    static HMODULE dll = LoadSystemVersionDll();
    if (dll) {
        typedef DWORD (WINAPI *Func_t)(DWORD, LPWSTR, DWORD);
        static Func_t func = (Func_t)GetProcAddress(dll, "VerLanguageNameW");
        if (func) return func(wLang, szLang, cchLang);
    }
    return 0;
}

BOOL WINAPI VerQueryValueA(LPCVOID pBlock, LPCSTR lpSubBlock, LPVOID* lplpBuffer, PUINT puLen) {
    static HMODULE dll = LoadSystemVersionDll();
    if (dll) {
        typedef BOOL (WINAPI *Func_t)(LPCVOID, LPCSTR, LPVOID*, PUINT);
        static Func_t func = (Func_t)GetProcAddress(dll, "VerQueryValueA");
        if (func) return func(pBlock, lpSubBlock, lplpBuffer, puLen);
    }
    return FALSE;
}

BOOL WINAPI VerQueryValueW(LPCVOID pBlock, LPCWSTR lpSubBlock, LPVOID* lplpBuffer, PUINT puLen) {
    static HMODULE dll = LoadSystemVersionDll();
    if (dll) {
        typedef BOOL (WINAPI *Func_t)(LPCVOID, LPCWSTR, LPVOID*, PUINT);
        static Func_t func = (Func_t)GetProcAddress(dll, "VerQueryValueW");
        if (func) return func(pBlock, lpSubBlock, lplpBuffer, puLen);
    }
    return FALSE;
}

DWORD WINAPI GetFileVersionInfoByHandle(int hFile, LPTSTR lpszFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData) {
    static HMODULE dll = LoadSystemVersionDll();
    if (dll) {
        typedef DWORD (WINAPI *Func_t)(int, LPTSTR, DWORD, DWORD, LPVOID);
        static Func_t func = (Func_t)GetProcAddress(dll, "GetFileVersionInfoByHandle");
        if (func) return func(hFile, lpszFilename, dwHandle, dwLen, lpData);
    }
    return 0;
}

} // extern "C"

// =============================================================================
// DLL ENTRY POINT
// =============================================================================

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
    case DLL_PROCESS_ATTACH:
        {
            DisableThreadLibraryCalls(hModule);
            
            // Clear debug file
            std::ofstream clearLog("debug_log.txt", std::ios::out);
            clearLog.close();
            
            WriteDebugLog("=== VERSION.DLL DEBUG MODE LOADED ===");
            WriteDebugLog("Process attached, starting injection thread");
            
            // Create injection thread
            g_injection_thread = CreateThread(NULL, 0, SteamInjectionThread, NULL, 0, NULL);
        }
        break;
        
    case DLL_PROCESS_DETACH:
        WriteDebugLog("Process detaching, cleaning up");
        // Restore on exit
        RestoreRegistryValues();
        CleanupVersionDll();
        WriteDebugLog("=== VERSION.DLL DEBUG MODE UNLOADED ===");
        break;
    }
    return TRUE;
}