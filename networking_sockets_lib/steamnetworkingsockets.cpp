#include <stdio.h>
#include <chrono>

#define STEAMNETWORKINGSOCKETS_STANDALONELIB
#define STEAMNETWORKINGSOCKETS_STEAMAPI
#define STEAMNETWORKINGSOCKETS_FOREXPORT
#define STEAM_API_EXPORTS
#include "steam/steam_gameserver.h"

const std::chrono::time_point<std::chrono::high_resolution_clock> startup_counter = std::chrono::high_resolution_clock::now();
const std::chrono::time_point<std::chrono::system_clock> startup_time = std::chrono::system_clock::now();

#ifndef EMU_RELEASE_BUILD

    // we need this for printf specifiers for intptr_t such as PRIdPTR
    #include <inttypes.h>

    #if defined(WIN32) || defined(_WIN32)
        #define WIN32_LEAN_AND_MEAN
        #include <Windows.h>
        
        #define PRINT_DEBUG(a, ...) do {                                                                                                                     \
            auto __prnt_dbg_ctr = std::chrono::high_resolution_clock::now();                                                                                 \
            auto __prnt_dbg_duration = __prnt_dbg_ctr - startup_counter;                                                                                     \
            auto __prnt_dbg_micro = std::chrono::duration_cast<std::chrono::duration<unsigned long long, std::micro>>(__prnt_dbg_duration);                  \
            auto __prnt_dbg_ms = std::chrono::duration_cast<std::chrono::duration<unsigned long long, std::milli>>(__prnt_dbg_duration);                     \
            auto __prnt_dbg_f = fopen("NETWORKING_SOCKET_LIB_LOG.txt", "a");                                                                                 \
            fprintf(__prnt_dbg_f, "[%llu ms, %llu us] [tid %lu] " a, __prnt_dbg_ms.count(), __prnt_dbg_micro.count(), GetCurrentThreadId(), __VA_ARGS__);    \
            fclose(__prnt_dbg_f);                                                                                                                            \
        } while (0)
    #else
        #ifndef _GNU_SOURCE
            #define _GNU_SOURCE
        #endif // _GNU_SOURCE

        #include <unistd.h>
        #include <sys/syscall.h>
        #include <sys/types.h>

        #define PRINT_DEBUG(a, ...) do {                                                                                                                      \
            auto __prnt_dbg_ctr = std::chrono::high_resolution_clock::now();                                                                                  \
            auto __prnt_dbg_duration = __prnt_dbg_ctr - startup_counter;                                                                                      \
            auto __prnt_dbg_micro = std::chrono::duration_cast<std::chrono::duration<unsigned long long, std::micro>>(__prnt_dbg_duration);                   \
            auto __prnt_dbg_ms = std::chrono::duration_cast<std::chrono::duration<unsigned long long, std::milli>>(__prnt_dbg_duration);                      \
            auto __prnt_dbg_f = fopen("NETWORKING_SOCKET_LIB_LOG.txt", "a");                                                                                  \
            fprintf(__prnt_dbg_f, "[%llu ms, %llu us] [tid %ld] " a, __prnt_dbg_ms.count(), __prnt_dbg_micro.count(), syscall(SYS_gettid), ##__VA_ARGS__);    \
            fclose(__prnt_dbg_f);                                                                                                                             \
        } while (0)
    #endif

#else // EMU_RELEASE_BUILD

    #define PRINT_DEBUG(...)

#endif // EMU_RELEASE_BUILD

#if defined(WIN32) || defined(_WIN32)
	#define NETWORKING_SOCKET_LIB_API extern "C" __declspec( dllexport ) 
#else // !WIN32
	#define NETWORKING_SOCKET_LIB_API extern "C" __attribute__ ((visibility("default"))) 
#endif


/*
extern "C" __declspec( dllexport )  void *CreateInterface( const char *pName, int *pReturnCode )
{
    //PRINT_DEBUG("steamclient CreateInterface %s\n", pName);
    HMODULE steam_api = LoadLibraryA(DLL_NAME);
    void *(__stdcall* create_interface)(const char*) = (void * (__stdcall *)(const char*))GetProcAddress(steam_api, "SteamInternal_CreateInterface");

    return create_interface(pName);
}

*/

class ISteamNetworkingUtilsDll
{
    virtual SteamNetworkingMicroseconds GetLocalTimestamp() = 0;
    //not sure if these are the correct functions
    virtual bool CheckPingDataUpToDate( float flMaxAgeSeconds ) = 0;
    virtual void a() = 0;
    virtual void b() = 0;
    virtual void c() = 0;
    virtual void d() = 0;
    virtual void e() = 0;
    virtual void f() = 0;
    virtual void g() = 0;
    virtual void h() = 0;
    virtual void i() = 0;
    virtual void j() = 0;
};

class ISteamNetworkingP2P
{
    virtual void a() = 0;
    virtual void b() = 0;
    virtual void c() = 0;
    virtual void d() = 0;
    virtual void e() = 0;
    virtual void f() = 0;
    virtual void g() = 0;
    virtual void h() = 0;
    virtual void i() = 0;
    virtual void j() = 0;
};

template <class steam_networkingutils_class>
class Networking_Utils_DLL : public steam_networkingutils_class
{
    public:
    ISteamNetworkingUtils *networking_utils;

    Networking_Utils_DLL(ISteamNetworkingUtils *networking_utils)
    {
        this->networking_utils = networking_utils;
    }

SteamNetworkingMicroseconds GetLocalTimestamp() { return networking_utils->GetLocalTimestamp(); }
bool CheckPingDataUpToDate( float flMaxAgeSeconds ) { return networking_utils->CheckPingDataUpToDate(flMaxAgeSeconds); }
void a() { PRINT_DEBUG("Networking_Utils_DLL::a\n"); }
void b() { PRINT_DEBUG("Networking_Utils_DLL::b\n"); }
void c() { PRINT_DEBUG("Networking_Utils_DLL::c\n"); }
void d() { PRINT_DEBUG("Networking_Utils_DLL::d\n"); }
void e() { PRINT_DEBUG("Networking_Utils_DLL::e\n"); }
void f() { PRINT_DEBUG("Networking_Utils_DLL::f\n"); }
void g() { PRINT_DEBUG("Networking_Utils_DLL::g\n"); }
void h() { PRINT_DEBUG("Networking_Utils_DLL::h\n"); }
void i() { PRINT_DEBUG("Networking_Utils_DLL::i\n"); }
void j() { PRINT_DEBUG("Networking_Utils_DLL::j\n"); }
};

template <class steam_networkingp2p_class>
class Networking_P2P_DLL : public steam_networkingp2p_class
{
    public:
void a() { PRINT_DEBUG("Networking_P2P_DLL::a\n"); }
void b() { PRINT_DEBUG("Networking_P2P_DLL::b\n"); }
void c() { PRINT_DEBUG("Networking_P2P_DLL::c\n"); }
void d() { PRINT_DEBUG("Networking_P2P_DLL::d\n"); }
void e() { PRINT_DEBUG("Networking_P2P_DLL::e\n"); }
void f() { PRINT_DEBUG("Networking_P2P_DLL::f\n"); }
void g() { PRINT_DEBUG("Networking_P2P_DLL::g\n"); }
void h() { PRINT_DEBUG("Networking_P2P_DLL::h\n"); }
void i() { PRINT_DEBUG("Networking_P2P_DLL::i\n"); }
void j() { PRINT_DEBUG("Networking_P2P_DLL::j\n"); }
};

static void *networking_sockets_gameserver;
static void *networking_sockets;
static void *networking_utils;
static void *networking_p2p;
static void *networking_p2p_gameserver;

NETWORKING_SOCKET_LIB_API class ISteamNetworkingSockets *SteamNetworkingSockets()
{
    PRINT_DEBUG("SteamNetworkingSocketsLib::SteamNetworkingSockets\n");
    return (class ISteamNetworkingSockets *)networking_sockets;
}

NETWORKING_SOCKET_LIB_API class ISteamNetworkingSockets *SteamNetworkingSockets_LibV12()
{
    PRINT_DEBUG("SteamNetworkingSocketsLib::SteamNetworkingSockets_LibV12\n");
    return SteamNetworkingSockets();
}

NETWORKING_SOCKET_LIB_API class ISteamNetworkingSockets *SteamGameServerNetworkingSockets()
{
    PRINT_DEBUG("SteamNetworkingSocketsLib::SteamGameServerNetworkingSockets\n");
    return (class ISteamNetworkingSockets *)networking_sockets_gameserver;
}

NETWORKING_SOCKET_LIB_API class ISteamNetworkingSockets *SteamGameServerNetworkingSockets_LibV12()
{
    PRINT_DEBUG("SteamNetworkingSocketsLib::SteamGameServerNetworkingSockets_LibV12\n");
    return SteamGameServerNetworkingSockets();
}


const int k_cchMaxSteamDatagramErrMsg = 1024;
typedef char SteamDatagramErrMsg[ k_cchMaxSteamDatagramErrMsg ];
typedef void * ( S_CALLTYPE *FSteamInternal_CreateInterface )( const char *);

NETWORKING_SOCKET_LIB_API bool SteamDatagramClient_Init_InternalV6( SteamDatagramErrMsg &errMsg, FSteamInternal_CreateInterface fnCreateInterface, HSteamUser hSteamUser, HSteamPipe hSteamPipe )
{
    PRINT_DEBUG("SteamNetworkingSocketsLib::SteamDatagramClient_Init_InternalV6 %u %u\n", hSteamUser, hSteamPipe);
    ISteamClient *client = (ISteamClient *)fnCreateInterface(STEAMCLIENT_INTERFACE_VERSION);
    networking_sockets = client->GetISteamGenericInterface(hSteamUser, hSteamPipe, "SteamNetworkingSockets001");
    networking_utils = new Networking_Utils_DLL<ISteamNetworkingUtilsDll>( (ISteamNetworkingUtils *)client->GetISteamGenericInterface(hSteamUser, hSteamPipe, "SteamNetworkingUtils001") );
    networking_p2p = new Networking_P2P_DLL<ISteamNetworkingP2P>();
    return true;
}

NETWORKING_SOCKET_LIB_API bool SteamDatagramClient_Init_InternalV9( SteamDatagramErrMsg &errMsg, FSteamInternal_CreateInterface fnCreateInterface, HSteamUser hSteamUser, HSteamPipe hSteamPipe )
{
    PRINT_DEBUG("SteamNetworkingSocketsLib::SteamDatagramClient_Init_InternalV9 %u %u\n", hSteamUser, hSteamPipe);
    return SteamDatagramClient_Init_InternalV6(errMsg, fnCreateInterface, hSteamUser, hSteamPipe );
}

NETWORKING_SOCKET_LIB_API void SteamDatagramServer_Kill( )
{
    PRINT_DEBUG("SteamNetworkingSocketsLib::SteamDatagramServer_Kill\n");
}

NETWORKING_SOCKET_LIB_API void SteamNetworkingSockets_SetDebugOutputFunction( ESteamNetworkingSocketsDebugOutputType eDetailLevel, FSteamNetworkingSocketsDebugOutput pfnFunc )
{
    PRINT_DEBUG("SteamNetworkingSocketsLib::SteamNetworkingSockets_SetDebugOutputFunction %i\n", eDetailLevel);
    if (networking_utils) {
        ((Networking_Utils_DLL<ISteamNetworkingUtilsDll> *)networking_utils)->networking_utils->SetDebugOutputFunction(eDetailLevel, pfnFunc);
    }
}

typedef void ( S_CALLTYPE *FSteamAPI_RegisterCallback)( class CCallbackBase *pCallback, int iCallback );
typedef void ( S_CALLTYPE *FSteamAPI_UnregisterCallback)( class CCallbackBase *pCallback );
typedef void ( S_CALLTYPE *FSteamAPI_RegisterCallResult)( class CCallbackBase *pCallback, SteamAPICall_t hAPICall );
typedef void ( S_CALLTYPE *FSteamAPI_UnregisterCallResult)( class CCallbackBase *pCallback, SteamAPICall_t hAPICall );
NETWORKING_SOCKET_LIB_API void SteamDatagramClient_Internal_SteamAPIKludge( FSteamAPI_RegisterCallback fnRegisterCallback, FSteamAPI_UnregisterCallback fnUnregisterCallback, FSteamAPI_RegisterCallResult fnRegisterCallResult, FSteamAPI_UnregisterCallResult fnUnregisterCallResult )
{
    PRINT_DEBUG("SteamNetworkingSocketsLib::SteamDatagramClient_Internal_SteamAPIKludge\n");
}

NETWORKING_SOCKET_LIB_API void SteamDatagramClient_Kill()
{
    PRINT_DEBUG("SteamNetworkingSocketsLib::SteamDatagramClient_Kill\n");
}

NETWORKING_SOCKET_LIB_API bool SteamDatagramServer_Init_Internal( SteamDatagramErrMsg &errMsg, FSteamInternal_CreateInterface fnCreateInterface, HSteamUser hSteamUser, HSteamPipe hSteamPipe )
{
    PRINT_DEBUG("SteamNetworkingSocketsLib::SteamDatagramServer_Init_Internal %u %u\n", hSteamUser, hSteamPipe);
    ISteamClient *client = (ISteamClient *)fnCreateInterface(STEAMCLIENT_INTERFACE_VERSION);
    networking_sockets_gameserver = client->GetISteamGenericInterface(hSteamUser, hSteamPipe, "SteamNetworkingSockets001");
    networking_utils = new Networking_Utils_DLL<ISteamNetworkingUtilsDll>( (ISteamNetworkingUtils *)client->GetISteamGenericInterface(hSteamUser, hSteamPipe, "SteamNetworkingUtils001"));
    return true;
}



NETWORKING_SOCKET_LIB_API class ISteamNetworkingUtils *SteamNetworkingUtils()
{
    PRINT_DEBUG("SteamNetworkingSocketsLib::SteamNetworkingUtils\n");
    return (class ISteamNetworkingUtils *)networking_utils;
}

NETWORKING_SOCKET_LIB_API void *SteamNetworkingP2P()
{
    PRINT_DEBUG("SteamNetworkingSocketsLib::SteamNetworkingP2P\n");
    return networking_p2p;
}

NETWORKING_SOCKET_LIB_API void *SteamNetworkingP2PGameServer()
{
    PRINT_DEBUG("SteamNetworkingSocketsLib::SteamNetworkingP2PGameServer\n");
    return NULL;
}
