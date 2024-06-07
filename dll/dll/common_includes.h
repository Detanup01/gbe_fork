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

#ifndef __INCLUDED_COMMON_INCLUDES__
#define __INCLUDED_COMMON_INCLUDES__

// OS detection
#if defined(WIN64) || defined(_WIN64) || defined(__MINGW64__)
    #define __WINDOWS_64__
#elif defined(WIN32) || defined(_WIN32) || defined(__MINGW32__)
    #define __WINDOWS_32__
#endif

#if defined(__WINDOWS_32__) || defined(__WINDOWS_64__)
    #define __WINDOWS__
#endif

#if defined(__linux__) || defined(linux)
    #if defined(__x86_64__)
        #define __LINUX_64__
    #else
        #define __LINUX_32__
    #endif
#endif

#if defined(__LINUX_32__) || defined(__LINUX_64__)
    #define __LINUX__
#endif

#if defined(__WINDOWS__)
    #define STEAM_WIN32
    #ifndef NOMINMAX
        #define NOMINMAX
    #endif
#endif

// we need this otherwise 'S_API_EXPORT' will be dllimport
#define STEAM_API_EXPORTS

// C/C++ includes
#include <cstdint>
#include <algorithm>
#include <string>
#include <string_view>
#include <chrono>
#include <cctype>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <iterator>

#include <vector>
#include <map>
#include <set>
#include <queue>
#include <list>

#include <thread>
#include <mutex>
#include <condition_variable>

#include <string.h>
#include <stdio.h>
#include <filesystem>
#include <optional>

// OS specific includes + definitions
#if defined(__WINDOWS__)
    #include <winsock2.h>
    #include <windows.h>
    #include <ws2tcpip.h>
    #include <processthreadsapi.h>
    #include <direct.h>
    #include <iphlpapi.h> // Include winsock2 before this, or winsock2 iphlpapi will be unavailable
    #include <shlobj.h>

    // we need this for BCryptGenRandom() in base.cpp
    #include <bcrypt.h>
    // we need non-standard S_IFDIR for Windows
    #include <sys/stat.h>

    #define MSG_NOSIGNAL 0

    EXTERN_C IMAGE_DOS_HEADER __ImageBase;
    #define PATH_SEPARATOR "\\"

    #ifdef EMU_EXPERIMENTAL_BUILD
        #include <winhttp.h>
        #include <intsafe.h> // MinGW cannot find DWordMult()
        #include "detours/detours.h"
    #endif

    #include "crash_printer/win.hpp"

// Convert a wide Unicode string to an UTF8 string
static inline std::string utf8_encode(const std::wstring &wstr)
{
    if( wstr.empty() ) return std::string();
    int size_needed = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo( size_needed, 0 );
    WideCharToMultiByte                  (CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}

// Convert UTF8 string to a wide Unicode String
static inline std::wstring utf8_decode(const std::string &str)
{
    if( str.empty() ) return std::wstring();
    int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo( size_needed, 0 );
    MultiByteToWideChar                  (CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
    return wstrTo;
}

static inline void reset_LastError()
{
    SetLastError(0);
}

#elif defined(__LINUX__)
    #ifndef _GNU_SOURCE
        #define _GNU_SOURCE
    #endif // _GNU_SOURCE

    #include <arpa/inet.h>

    #include <sys/types.h>
    #include <sys/stat.h>
    #include <sys/ioctl.h>
    #include <sys/socket.h>
    #include <sys/mount.h>
    #include <sys/stat.h>
    #include <sys/statvfs.h>
    #include <sys/time.h>

    #include <netinet/in.h>
    #include <netinet/tcp.h>
    #include <linux/netdevice.h>

    #include <fcntl.h>
    #include <unistd.h>
    #include <dirent.h>
    #include <netdb.h>
    #include <dlfcn.h>
    #include <utime.h>

    #include "crash_printer/linux.hpp"

    #define PATH_MAX_STRING_SIZE 512
    #define PATH_SEPARATOR "/" 
    #define utf8_decode(a) a
    #define utf8_encode(a) a
    #define reset_LastError()
#endif

// Other libs includes
#include "json/json.hpp"
#include "utfcpp/utf8.h"
#include "controller/gamepad.h"

// common includes
#include "common_helpers/common_helpers.hpp"

// Steamsdk includes
#include "steam/steam_api.h"
#include "steam/steam_gameserver.h"
#include "steam/steamdatagram_tickets.h"

#define AS_STR(x) #x
#define EXPAND_AS_STR(x) AS_STR(x)

#if defined(__LINUX__) || defined(GNUC) || defined(__MINGW32__) || defined(__MINGW64__) // MinGw
    #define EMU_FUNC_NAME __PRETTY_FUNCTION__
#else
    #define EMU_FUNC_NAME __FUNCTION__##"()"
#endif

// PRINT_DEBUG definition
#ifndef EMU_RELEASE_BUILD
    // we need this for printf specifiers for intptr_t such as PRIdPTR
    #include <inttypes.h>
    
    #if defined(__WINDOWS__)
        #define PRINT_DEBUG_TID() (long long)GetCurrentThreadId()
        #define PRINT_DEBUG_CLEANUP() WSASetLastError(0)
    #elif defined(__LINUX__)
        #include <sys/syscall.h> // syscall

        #define PRINT_DEBUG_TID() (long long)syscall(SYS_gettid)
        #define PRINT_DEBUG_CLEANUP() (void)0
    #else
        #warning  "Unrecognized OS"

        #define PRINT_DEBUG_TID() (long long)0
        #define PRINT_DEBUG_CLEANUP() (void)0
    #endif

    //#define PRINT_DEBUG(...) fprintf(stdout, __VA_ARGS__)
    extern const std::string dbg_log_file;
    extern const std::chrono::time_point<std::chrono::high_resolution_clock> startup_counter;

    #define PRINT_DEBUG(a, ...) do {                                                                                                                     \
        auto __prnt_dbg_ctr = std::chrono::high_resolution_clock::now();                                                                                 \
        auto __prnt_dbg_duration = __prnt_dbg_ctr - startup_counter;                                                                                     \
        auto __prnt_dbg_micro = std::chrono::duration_cast<std::chrono::duration<unsigned long long, std::micro>>(__prnt_dbg_duration);                  \
        auto __prnt_dbg_ms = std::chrono::duration_cast<std::chrono::duration<unsigned long long, std::milli>>(__prnt_dbg_duration);                     \
        auto __prnt_dbg_f = fopen(dbg_log_file.c_str(), "a");                                                                                            \
        if (!__prnt_dbg_f) break;                                                                                                                        \
        fprintf(__prnt_dbg_f, "[%llu ms, %lld us] [tid %lld] %s " a "\n",                                                                                \
            __prnt_dbg_ms.count(), __prnt_dbg_micro.count(), PRINT_DEBUG_TID(), EMU_FUNC_NAME, ##__VA_ARGS__);                                           \
        fclose(__prnt_dbg_f);                                                                                                                            \
        PRINT_DEBUG_CLEANUP();                                                                                                                           \
    } while (0)

#else // EMU_RELEASE_BUILD
    #define PRINT_DEBUG(...)
#endif // EMU_RELEASE_BUILD

// function entry
#define PRINT_DEBUG_ENTRY() PRINT_DEBUG("")
#define PRINT_DEBUG_TODO() PRINT_DEBUG("// TODO")
#define PRINT_DEBUG_GNU_WIN() PRINT_DEBUG("GNU/Win")

// Emulator includes
// add them here after the inline functions definitions
#include "net.pb.h"
#include "settings.h"
#include "local_storage.h"
#include "network.h"

// Emulator defines
#define CLIENT_HSTEAMUSER 1
#define SERVER_HSTEAMUSER 1

#define DEFAULT_NAME "gse orca"
#define DEFAULT_LANGUAGE "english"
#define DEFAULT_IP_COUNTRY "US"

#define LOBBY_CONNECT_APPID ((uint32)-2)

#endif //__INCLUDED_COMMON_INCLUDES__
