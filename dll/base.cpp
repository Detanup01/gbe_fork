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

#include "dll/base.h"
#include "dll/settings_parser.h"

#ifndef EMU_RELEASE_BUILD
#include "dbg_log/dbg_log.hpp"
#endif


std::recursive_mutex global_mutex{};
// some arbitrary counter/time for reference
const std::chrono::time_point<std::chrono::high_resolution_clock> startup_counter = std::chrono::high_resolution_clock::now();
const std::chrono::time_point<std::chrono::system_clock> startup_time = std::chrono::system_clock::now();

#ifndef EMU_RELEASE_BUILD
dbg_log dbg_logger(get_full_program_path() + "STEAM_LOG_" + std::to_string(common_helpers::rand_number(UINT32_MAX)) + ".log");
#endif


#ifdef __WINDOWS__

void randombytes(char *buf, size_t size)
{
    // NT_SUCCESS is: return value >= 0, including Ntdef.h causes so many errors
    while (BCryptGenRandom(NULL, (PUCHAR) buf, (ULONG) size, BCRYPT_USE_SYSTEM_PREFERRED_RNG) < 0) {
        PRINT_DEBUG("ERROR");
        Sleep(100);
    }
    
}

std::string get_env_variable(const std::string &name)
{
    wchar_t env_variable[1024]{};
    DWORD ret = GetEnvironmentVariableW(utf8_decode(name).c_str(), env_variable, _countof(env_variable));
    if (ret <= 0 || !env_variable[0]) {
        return std::string();
    }

    env_variable[ret] = 0;
    return utf8_encode(env_variable);
}

bool set_env_variable(const std::string &name, const std::string &value)
{
    return SetEnvironmentVariableW(utf8_decode(name).c_str(), utf8_decode(value).c_str());
}

#else

static int fd = -1;

void randombytes(char *buf, size_t size)
{
  int i;

  if (fd == -1) {
    for (;;) {
      fd = open("/dev/urandom",O_RDONLY);
      if (fd != -1) break;
      sleep(1);
    }
  }

  while (size > 0) {
    if (size < 1048576) i = size; else i = 1048576;

    i = read(fd,buf,i);
    if (i < 1) {
      sleep(1);
      continue;
    }

    buf += i;
    size -= i;
  }
}

std::string get_env_variable(const std::string &name)
{
    char *env = getenv(name.c_str());
    if (!env) {
        return std::string();
    }

    return std::string(env);
}

bool set_env_variable(const std::string &name, const std::string &value)
{
    return setenv(name.c_str(), value.c_str(), 1) == 0;
}

#endif


unsigned generate_account_id()
{
    int a;
    randombytes((char *)&a, sizeof(a));
    a = abs(a);
    if (!a) ++a;
    return a;
}

CSteamID generate_steam_anon_user()
{
    return CSteamID(generate_account_id(), k_unSteamUserDefaultInstance, k_EUniversePublic, k_EAccountTypeAnonUser);
}

SteamAPICall_t generate_steam_api_call_id() {
    static SteamAPICall_t a;
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    randombytes((char *)&a, sizeof(a));
    ++a;
    if (a == 0) ++a;
    return a;
}

CSteamID generate_steam_id_user()
{
    return CSteamID(generate_account_id(), k_unSteamUserDefaultInstance, k_EUniversePublic, k_EAccountTypeIndividual);
}

CSteamID generate_steam_id_server()
{
    return CSteamID(generate_account_id(), k_unSteamUserDefaultInstance, k_EUniversePublic, k_EAccountTypeGameServer);
}

CSteamID generate_steam_id_anonserver()
{
    return CSteamID(generate_account_id(), k_unSteamUserDefaultInstance, k_EUniversePublic, k_EAccountTypeAnonGameServer);
}

CSteamID generate_steam_id_lobby()
{
    return CSteamID(generate_account_id(), k_EChatInstanceFlagLobby | k_EChatInstanceFlagMMSLobby, k_EUniversePublic, k_EAccountTypeChat);
}

bool check_timedout(std::chrono::high_resolution_clock::time_point old, double timeout)
{
    if (timeout == 0.0) return true;

    std::chrono::high_resolution_clock::time_point now = std::chrono::high_resolution_clock::now();
    if (std::chrono::duration_cast<std::chrono::duration<double>>(now - old).count() > timeout) {
        return true;
    }

    return false;
}

#ifdef __LINUX__
std::string get_lib_path()
{
    Dl_info info;
    if (dladdr((void*)get_lib_path, &info))
    {
        return std::string(info.dli_fname);
    }
    return ".";
}
#endif

std::string get_full_lib_path()
{
    std::string program_path;
#if defined(__WINDOWS__)
    wchar_t   DllPath[2048] = {0};
    GetModuleFileNameW((HINSTANCE)&__ImageBase, DllPath, _countof(DllPath));
    program_path = utf8_encode(DllPath);
#else
    program_path = get_lib_path();
#endif
    return program_path;
}

std::string get_full_program_path()
{
    std::string env_program_path = get_env_variable("GseAppPath");
    if (env_program_path.length()) {
        if (env_program_path.back() != PATH_SEPARATOR[0]) {
            env_program_path = env_program_path.append(PATH_SEPARATOR);
        }

        return env_program_path;
    }

    std::string program_path{};
    program_path = get_full_lib_path();
    return program_path.substr(0, program_path.rfind(PATH_SEPARATOR)).append(PATH_SEPARATOR);
}

std::string get_current_path()
{
    std::string path;
#if defined(STEAM_WIN32)
    char *buffer = _getcwd( NULL, 0 );
#else
    char *buffer = get_current_dir_name();
#endif
    if (buffer) {
        path = buffer;
        path.append(PATH_SEPARATOR);
        free(buffer);
    }

    return path;
}

std::string canonical_path(const std::string &path)
{
    std::string output;
#if defined(STEAM_WIN32)
    wchar_t *buffer = _wfullpath(NULL, utf8_decode(path).c_str(), 0);
    if (buffer) {
        output = utf8_encode(buffer);
        free(buffer);
    }
#else
    char *buffer = canonicalize_file_name(path.c_str());
    if (buffer) {
        output = buffer;
        free(buffer);
    }
#endif

    return output;
}

bool file_exists_(const std::string &full_path)
{
#if defined(STEAM_WIN32)
    struct _stat buffer{};
    if (_wstat(utf8_decode(full_path).c_str(), &buffer) != 0)
        return false;

    if ( buffer.st_mode & S_IFDIR)
        return false;
#else
    struct stat buffer{};
    if (stat(full_path.c_str(), &buffer) != 0)
        return false;

    if (S_ISDIR(buffer.st_mode))
        return false;
#endif

    return true;
}

unsigned int file_size_(const std::string &full_path)
{
#if defined(STEAM_WIN32)
    struct _stat buffer{};
    if (_wstat(utf8_decode(full_path).c_str(), &buffer) != 0) return 0;
#else
    struct stat buffer{};
    if (stat (full_path.c_str(), &buffer) != 0) return 0;
#endif
    return buffer.st_size;
}


#ifdef EMU_EXPERIMENTAL_BUILD

static std::vector<void*> loaded_libs{};

static void load_dlls()
{
    constexpr static const char LIB_EXTENSION[] =
#ifdef __WINDOWS__
        ".dll"
#else
        ".so"
#endif
        ;

    std::string path(Local_Storage::get_game_settings_path() + "load_dlls" + PATH_SEPARATOR);

    std::vector<std::string> paths(Local_Storage::get_filenames_path(path));
    for (auto & p: paths) {
        std::string full_path(path + p);
        if (!common_helpers::ends_with_i(full_path, LIB_EXTENSION)) continue;

        PRINT_DEBUG("loading '%s'", full_path.c_str());
        auto lib_handle =
#ifdef __WINDOWS__
            LoadLibraryW(utf8_decode(full_path).c_str());
#else
            dlopen(full_path.c_str(), RTLD_NOW | RTLD_LOCAL);
#endif

        if (lib_handle != nullptr) {
            loaded_libs.push_back(reinterpret_cast<void *>(lib_handle));
            PRINT_DEBUG(" LOADED");
        } else {
#ifdef __WINDOWS__
            PRINT_DEBUG(" FAILED, error code 0x%X", GetLastError());
#else
            PRINT_DEBUG(" FAILED, error string '%s'", dlerror());
#endif
        }
    }
}

static void unload_dlls()
{
    for (auto lib_handle : loaded_libs) {
#ifdef __WINDOWS__
        FreeLibrary(reinterpret_cast<HMODULE>(lib_handle));
#else
        dlclose(lib_handle);
#endif
    }
}

#ifdef __WINDOWS__

struct ips_test {
    uint32_t ip_from;
    uint32_t ip_to;
};

static std::vector<struct ips_test> whitelist_ips;

void set_whitelist_ips(uint32_t *from, uint32_t *to, unsigned num_ips)
{
    whitelist_ips.clear();
    for (unsigned i = 0; i < num_ips; ++i) {
        struct ips_test ip_a;
        PRINT_DEBUG("from: %hhu.%hhu.%hhu.%hhu", ((unsigned char *)&from[i])[0], ((unsigned char *)&from[i])[1], ((unsigned char *)&from[i])[2], ((unsigned char *)&from[i])[3]);
        PRINT_DEBUG("to: %hhu.%hhu.%hhu.%hhu", ((unsigned char *)&to[i])[0], ((unsigned char *)&to[i])[1], ((unsigned char *)&to[i])[2], ((unsigned char *)&to[i])[3]);
        ip_a.ip_from = ntohl(from[i]);
        ip_a.ip_to = ntohl(to[i]);
        if (ip_a.ip_to < ip_a.ip_from) continue;
        if ((ip_a.ip_to - ip_a.ip_from) > (1 << 25)) continue;
        PRINT_DEBUG("added ip to whitelist");
        whitelist_ips.push_back(ip_a);
    }
}

static bool is_whitelist_ip(unsigned char *ip)
{
    uint32_t ip_temp = 0;
    memcpy(&ip_temp, ip, sizeof(ip_temp));
    ip_temp = ntohl(ip_temp);

    for (auto &i : whitelist_ips) {
        if (i.ip_from <= ip_temp && ip_temp <= i.ip_to) {
            PRINT_DEBUG("IP IS WHITELISTED %hhu.%hhu.%hhu.%hhu", ip[0], ip[1], ip[2], ip[3]);
            return true;
        }
    }

    return false;
}

static bool is_lan_ipv4(unsigned char *ip)
{
    PRINT_DEBUG("CHECK LAN IP %hhu.%hhu.%hhu.%hhu", ip[0], ip[1], ip[2], ip[3]);
    if (is_whitelist_ip(ip)) return true;
    if (ip[0] == 127) return true;
    if (ip[0] == 10) return true;
    if (ip[0] == 192 && ip[1] == 168) return true;
    if (ip[0] == 169 && ip[1] == 254 && ip[2] != 0) return true;
    if (ip[0] == 172 && ip[1] >= 16 && ip[1] <= 31) return true;
    if ((ip[0] == 100) && ((ip[1] & 0xC0) == 0x40)) return true;
    if (ip[0] == 239) return true; //multicast
    if (ip[0] == 0) return true; //Current network
    if (ip[0] == 192 && (ip[1] == 18 || ip[1] == 19)) return true; //Used for benchmark testing of inter-network communications between two separate subnets.
    if (ip[0] >= 224) return true; //ip multicast (224 - 239) future use (240.0.0.0 - 255.255.255.254) broadcast (255.255.255.255)
    return false;
}

static bool is_lan_ip(const sockaddr *addr, int namelen)
{
    if (!namelen) return false;

    if (addr->sa_family == AF_INET) {
        struct sockaddr_in *addr_in = (struct sockaddr_in *)addr;
        unsigned char ip[4];
        memcpy(ip, &addr_in->sin_addr, sizeof(ip));
        if (is_lan_ipv4(ip)) return true;
    } else if (addr->sa_family == AF_INET6) {
        struct sockaddr_in6 *addr_in6 = (struct sockaddr_in6 *)addr;
        unsigned char ip[16];
        unsigned char zeroes[16] = {};
        memcpy(ip, &addr_in6->sin6_addr, sizeof(ip));
        PRINT_DEBUG("CHECK LAN IP6 %hhu.%hhu.%hhu.%hhu.%hhu.%hhu.%hhu.%hhu.%hhu.%hhu.%hhu.%hhu.%hhu.%hhu.%hhu.%hhu", ip[0], ip[1], ip[2], ip[3], ip[4], ip[5], ip[6], ip[7], ip[8], ip[9], ip[10], ip[11], ip[12], ip[13], ip[14], ip[15]);
        if (((ip[0] == 0xFF) && (ip[1] < 3) && (ip[15] == 1)) ||
        ((ip[0] == 0xFE) && ((ip[1] & 0xC0) == 0x80))) return true;
        if (memcmp(zeroes, ip, sizeof(ip)) == 0) return true;
        if (memcmp(zeroes, ip, sizeof(ip) - 1) == 0 && ip[15] == 1) return true;
        if (ip[0] == 0xff) return true; //multicast
        if (ip[0] == 0xfc) return true; //unique local
        if (ip[0] == 0xfd) return true; //unique local

        unsigned char ipv4_mapped[12] = {};
        ipv4_mapped[10] = 0xFF;
        ipv4_mapped[11] = 0xFF;
        if (memcmp(ipv4_mapped, ip, sizeof(ipv4_mapped)) == 0) {
            if (is_lan_ipv4(ip + 12)) return true;
        }
    }

    PRINT_DEBUG("NOT LAN IP");
    return false;
}

int ( WINAPI *Real_SendTo )( SOCKET s, const char *buf, int len, int flags, const sockaddr *to, int tolen) = sendto;
int ( WINAPI *Real_Connect )( SOCKET s, const sockaddr *addr, int namelen ) = connect;
int ( WINAPI *Real_WSAConnect )( SOCKET s, const sockaddr *addr, int namelen, LPWSABUF lpCallerData, LPWSABUF lpCalleeData, LPQOS lpSQOS, LPQOS lpGQOS) = WSAConnect;

static int WINAPI Mine_SendTo( SOCKET s, const char *buf, int len, int flags, const sockaddr *to, int tolen)
{
    PRINT_DEBUG_ENTRY();
    if (is_lan_ip(to, tolen)) {
        return Real_SendTo( s, buf, len, flags, to, tolen );
    } else {
        return len;
    }
}

static int WINAPI Mine_Connect( SOCKET s, const sockaddr *addr, int namelen )
{
    PRINT_DEBUG_ENTRY();
    if (is_lan_ip(addr, namelen)) {
        return Real_Connect(s, addr, namelen);
    } else {
        WSASetLastError(WSAECONNREFUSED);
        return SOCKET_ERROR;
    }
}

static int WINAPI Mine_WSAConnect( SOCKET s, const sockaddr *addr, int namelen, LPWSABUF lpCallerData, LPWSABUF lpCalleeData, LPQOS lpSQOS, LPQOS lpGQOS)
{
    PRINT_DEBUG_ENTRY();
    if (is_lan_ip(addr, namelen)) {
        return Real_WSAConnect(s, addr, namelen, lpCallerData, lpCalleeData, lpSQOS, lpGQOS);
    } else {
        WSASetLastError(WSAECONNREFUSED);
        return SOCKET_ERROR;
    }
}

inline bool file_exists (const std::string& name)
{
  struct stat buffer;   
  return (stat (name.c_str(), &buffer) == 0); 
}

#ifdef DETOURS_64BIT
    #define DLL_NAME "steam_api64.dll"
#else
    #define DLL_NAME "steam_api.dll"
#endif

HMODULE (WINAPI *Real_GetModuleHandleA)(LPCSTR lpModuleName) = GetModuleHandleA;
HMODULE WINAPI Mine_GetModuleHandleA(LPCSTR lpModuleName)
{
    PRINT_DEBUG("%s", lpModuleName);
    if (!lpModuleName) return Real_GetModuleHandleA(lpModuleName);
    std::string in(lpModuleName);
    if (in == std::string(DLL_NAME)) {
        in = std::string("crack") + in;
    }

    return Real_GetModuleHandleA(in.c_str());
}

static void redirect_crackdll()
{
    DetourTransactionBegin();
    DetourUpdateThread( GetCurrentThread() );
    DetourAttach( reinterpret_cast<PVOID*>(&Real_GetModuleHandleA), reinterpret_cast<PVOID>(Mine_GetModuleHandleA) );
    DetourTransactionCommit();
}

static void unredirect_crackdll()
{
    DetourTransactionBegin();
    DetourUpdateThread( GetCurrentThread() );
    DetourDetach( reinterpret_cast<PVOID*>(&Real_GetModuleHandleA), reinterpret_cast<PVOID>(Mine_GetModuleHandleA) );
    DetourTransactionCommit();
}

HMODULE crack_dll_handle{};
static void load_crack_dll()
{
    std::string path(get_full_program_path() + "crack" + DLL_NAME);
    PRINT_DEBUG("searching for crack file '%s'", path.c_str());
    if (file_exists(path)) {
        redirect_crackdll();
        crack_dll_handle = LoadLibraryW(utf8_decode(path).c_str());
        unredirect_crackdll();
        PRINT_DEBUG("Loaded crack file");
    }
}

#include "dll/local_storage.h"

//For some reason when this function is optimized it breaks the shogun 2 prophet (reloaded) crack.
#pragma optimize( "", off )
bool crack_SteamAPI_RestartAppIfNecessary(uint32 unOwnAppID)
{
    if (crack_dll_handle) {
        bool (__stdcall* restart_app)(uint32) = (bool (__stdcall *)(uint32))GetProcAddress(crack_dll_handle, "SteamAPI_RestartAppIfNecessary");
        if (restart_app) {
            PRINT_DEBUG("Calling crack SteamAPI_RestartAppIfNecessary");
            redirect_crackdll();
            bool ret = restart_app(unOwnAppID);
            unredirect_crackdll();
            return ret;
        }
    }

    return false;
}
#pragma optimize( "", on )

bool crack_SteamAPI_Init()
{
    if (crack_dll_handle) {
        bool (__stdcall* init_app)() = (bool (__stdcall *)())GetProcAddress(crack_dll_handle, "SteamAPI_Init");
        if (init_app) {
            PRINT_DEBUG("Calling crack SteamAPI_Init");
            redirect_crackdll();
            bool ret = init_app();
            unredirect_crackdll();
            return ret;
        }
    }

    return false;
}

HINTERNET (WINAPI *Real_WinHttpConnect)(
  IN HINTERNET     hSession,
  IN LPCWSTR       pswzServerName,
  IN INTERNET_PORT nServerPort,
  IN DWORD         dwReserved
);

HINTERNET WINAPI Mine_WinHttpConnect(
  IN HINTERNET     hSession,
  IN LPCWSTR       pswzServerName,
  IN INTERNET_PORT nServerPort,
  IN DWORD         dwReserved
) {
    PRINT_DEBUG("%ls %u", pswzServerName, nServerPort);
    struct sockaddr_in ip4;
    struct sockaddr_in6 ip6;
    ip4.sin_family = AF_INET;
    ip6.sin6_family = AF_INET6;

    if ((InetPtonW(AF_INET, pswzServerName, &(ip4.sin_addr)) && is_lan_ip((sockaddr *)&ip4, sizeof(ip4))) || (InetPtonW(AF_INET6, pswzServerName, &(ip6.sin6_addr)) && is_lan_ip((sockaddr *)&ip6, sizeof(ip6)))) {
        return Real_WinHttpConnect(hSession, pswzServerName, nServerPort, dwReserved);
    } else {
        return Real_WinHttpConnect(hSession, L"127.1.33.7", nServerPort, dwReserved);
    }
}

HINTERNET (WINAPI *Real_WinHttpOpenRequest)(
  IN HINTERNET hConnect,
  IN LPCWSTR   pwszVerb,
  IN LPCWSTR   pwszObjectName,
  IN LPCWSTR   pwszVersion,
  IN LPCWSTR   pwszReferrer,
  IN LPCWSTR   *ppwszAcceptTypes,
  IN DWORD     dwFlags
);

HINTERNET WINAPI Mine_WinHttpOpenRequest(
  IN HINTERNET hConnect,
  IN LPCWSTR   pwszVerb,
  IN LPCWSTR   pwszObjectName,
  IN LPCWSTR   pwszVersion,
  IN LPCWSTR   pwszReferrer,
  IN LPCWSTR   *ppwszAcceptTypes,
  IN DWORD     dwFlags
) {
    PRINT_DEBUG("%ls %ls %ls %ls %i", pwszVerb, pwszObjectName, pwszVersion, pwszReferrer, dwFlags);
    if (dwFlags & WINHTTP_FLAG_SECURE) {
        dwFlags ^= WINHTTP_FLAG_SECURE;
    }

    return Real_WinHttpOpenRequest(hConnect, pwszVerb, pwszObjectName, pwszVersion, pwszReferrer, ppwszAcceptTypes, dwFlags);
}


static bool network_functions_attached = false;
BOOL WINAPI DllMain( HINSTANCE, DWORD dwReason, LPVOID )
{
    switch ( dwReason ) {
        case DLL_PROCESS_ATTACH:
            PRINT_DEBUG("experimental DLL_PROCESS_ATTACH");
            if (!settings_disable_lan_only()) {
                PRINT_DEBUG("Hooking lan only functions");
                DetourTransactionBegin();
                DetourUpdateThread( GetCurrentThread() );
                DetourAttach( reinterpret_cast<PVOID*>(&Real_SendTo), reinterpret_cast<PVOID>(Mine_SendTo) );
                DetourAttach( reinterpret_cast<PVOID*>(&Real_Connect), reinterpret_cast<PVOID>(Mine_Connect) );
                DetourAttach( reinterpret_cast<PVOID*>(&Real_WSAConnect), reinterpret_cast<PVOID>(Mine_WSAConnect) );

                HMODULE winhttp = GetModuleHandleA("winhttp.dll");
                if (winhttp) {
                    Real_WinHttpConnect = (decltype(Real_WinHttpConnect))GetProcAddress(winhttp, "WinHttpConnect");
                    DetourAttach( reinterpret_cast<PVOID*>(&Real_WinHttpConnect), reinterpret_cast<PVOID>(Mine_WinHttpConnect) );
                    // Real_WinHttpOpenRequest = (decltype(Real_WinHttpOpenRequest))GetProcAddress(winhttp, "WinHttpOpenRequest");
                    // DetourAttach( reinterpret_cast<PVOID*>(&Real_WinHttpOpenRequest), reinterpret_cast<PVOID>(Mine_WinHttpOpenRequest) );
                }
    
                DetourTransactionCommit();
                network_functions_attached = true;
            }
            load_crack_dll();
            load_dlls();
        break;

        case DLL_PROCESS_DETACH:
            PRINT_DEBUG("experimental DLL_PROCESS_DETACH");
            if (network_functions_attached) {
                DetourTransactionBegin();
                DetourUpdateThread( GetCurrentThread() );
                DetourDetach( reinterpret_cast<PVOID*>(&Real_SendTo), reinterpret_cast<PVOID>(Mine_SendTo) );
                DetourDetach( reinterpret_cast<PVOID*>(&Real_Connect), reinterpret_cast<PVOID>(Mine_Connect) );
                DetourDetach( reinterpret_cast<PVOID*>(&Real_WSAConnect), reinterpret_cast<PVOID>(Mine_WSAConnect) );
                if (Real_WinHttpConnect) {
                    DetourDetach( reinterpret_cast<PVOID*>(&Real_WinHttpConnect), reinterpret_cast<PVOID>(Mine_WinHttpConnect) );
                    // DetourDetach( reinterpret_cast<PVOID*>(&Real_WinHttpOpenRequest), reinterpret_cast<PVOID>(Mine_WinHttpOpenRequest) );
                }
                DetourTransactionCommit();
            }

            unload_dlls();
        break;
    }

    return TRUE;
}

#else

__attribute__((__constructor__)) static void lib_base_entry()
{
    load_dlls();
}

__attribute__((__destructor__)) static void lib_base_exit()
{
    unload_dlls();
}

void set_whitelist_ips(uint32_t *from, uint32_t *to, unsigned num_ips)
{

}

#endif // __WINDOWS__

#else

void set_whitelist_ips(uint32_t *from, uint32_t *to, unsigned num_ips)
{

}

#endif // EMU_EXPERIMENTAL_BUILD

