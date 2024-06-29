#include <stdio.h>
#include <chrono>

#define STEAMNETWORKINGSOCKETS_STANDALONELIB
#define STEAMNETWORKINGSOCKETS_STEAMAPI
#define STEAMNETWORKINGSOCKETS_FOREXPORT
#define STEAM_API_EXPORTS
#include "steam/steam_gameserver.h"

// OS detection
#include "common_helpers/os_detector.h"

#ifndef EMU_RELEASE_BUILD
#include "dbg_log/dbg_log.hpp"
dbg_log dbg_logger("STEAM_LOG.txt");
#endif

#if defined(__LINUX__) || defined(GNUC) || defined(__MINGW32__) || defined(__MINGW64__) // MinGw
    #define EMU_FUNC_NAME __PRETTY_FUNCTION__
#else
    #define EMU_FUNC_NAME __FUNCTION__##"()"
#endif

// PRINT_DEBUG definition
#ifndef EMU_RELEASE_BUILD
    #include "dbg_log/dbg_log.hpp"
    // we need this for printf specifiers for intptr_t such as PRIdPTR
    #include <inttypes.h>
    
    #if defined(__WINDOWS__)
        #define WIN32_LEAN_AND_MEAN
        #include <windows.h>

        #define PRINT_DEBUG_TID() (long long)GetCurrentThreadId()
    #elif defined(__LINUX__)
        #ifndef _GNU_SOURCE
            #define _GNU_SOURCE
        #endif // _GNU_SOURCE
        #include <unistd.h>
        #include <sys/types.h>

        #include <sys/syscall.h> // syscall

        #define PRINT_DEBUG_TID() (long long)syscall(SYS_gettid)
    #else
        #warning  "Unrecognized OS"

        #define PRINT_DEBUG_TID() (long long)0
    #endif

    extern dbg_log dbg_logger;

    #define PRINT_DEBUG(a, ...) do {                                                                \
        dbg_logger.write("[tid %lld] %s " a, PRINT_DEBUG_TID(), EMU_FUNC_NAME, ##__VA_ARGS__);      \
    } while (0)

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
    //PRINT_DEBUG("steamclient CreateInterface %s", pName);
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
void a() { PRINT_DEBUG("Networking_Utils_DLL::a"); }
void b() { PRINT_DEBUG("Networking_Utils_DLL::b"); }
void c() { PRINT_DEBUG("Networking_Utils_DLL::c"); }
void d() { PRINT_DEBUG("Networking_Utils_DLL::d"); }
void e() { PRINT_DEBUG("Networking_Utils_DLL::e"); }
void f() { PRINT_DEBUG("Networking_Utils_DLL::f"); }
void g() { PRINT_DEBUG("Networking_Utils_DLL::g"); }
void h() { PRINT_DEBUG("Networking_Utils_DLL::h"); }
void i() { PRINT_DEBUG("Networking_Utils_DLL::i"); }
void j() { PRINT_DEBUG("Networking_Utils_DLL::j"); }
};

template <class steam_networkingp2p_class>
class Networking_P2P_DLL : public steam_networkingp2p_class
{
    public:
void a() { PRINT_DEBUG("Networking_P2P_DLL::a"); }
void b() { PRINT_DEBUG("Networking_P2P_DLL::b"); }
void c() { PRINT_DEBUG("Networking_P2P_DLL::c"); }
void d() { PRINT_DEBUG("Networking_P2P_DLL::d"); }
void e() { PRINT_DEBUG("Networking_P2P_DLL::e"); }
void f() { PRINT_DEBUG("Networking_P2P_DLL::f"); }
void g() { PRINT_DEBUG("Networking_P2P_DLL::g"); }
void h() { PRINT_DEBUG("Networking_P2P_DLL::h"); }
void i() { PRINT_DEBUG("Networking_P2P_DLL::i"); }
void j() { PRINT_DEBUG("Networking_P2P_DLL::j"); }
};

static void *networking_sockets_gameserver;
static void *networking_sockets;
static void *networking_utils;
static void *networking_p2p;
static void *networking_p2p_gameserver;

NETWORKING_SOCKET_LIB_API class ISteamNetworkingSockets *SteamNetworkingSockets()
{
    PRINT_DEBUG("SteamNetworkingSocketsLib::SteamNetworkingSockets");
    return (class ISteamNetworkingSockets *)networking_sockets;
}

NETWORKING_SOCKET_LIB_API class ISteamNetworkingSockets *SteamNetworkingSockets_LibV12()
{
    PRINT_DEBUG("SteamNetworkingSocketsLib::SteamNetworkingSockets_LibV12");
    return SteamNetworkingSockets();
}

NETWORKING_SOCKET_LIB_API class ISteamNetworkingSockets *SteamGameServerNetworkingSockets()
{
    PRINT_DEBUG("SteamNetworkingSocketsLib::SteamGameServerNetworkingSockets");
    return (class ISteamNetworkingSockets *)networking_sockets_gameserver;
}

NETWORKING_SOCKET_LIB_API class ISteamNetworkingSockets *SteamGameServerNetworkingSockets_LibV12()
{
    PRINT_DEBUG("SteamNetworkingSocketsLib::SteamGameServerNetworkingSockets_LibV12");
    return SteamGameServerNetworkingSockets();
}


const int k_cchMaxSteamDatagramErrMsg = 1024;
typedef char SteamDatagramErrMsg[ k_cchMaxSteamDatagramErrMsg ];
typedef void * ( S_CALLTYPE *FSteamInternal_CreateInterface )( const char *);

NETWORKING_SOCKET_LIB_API bool SteamDatagramClient_Init_InternalV6( SteamDatagramErrMsg &errMsg, FSteamInternal_CreateInterface fnCreateInterface, HSteamUser hSteamUser, HSteamPipe hSteamPipe )
{
    PRINT_DEBUG("SteamNetworkingSocketsLib::SteamDatagramClient_Init_InternalV6 %u %u", hSteamUser, hSteamPipe);
    ISteamClient *client = (ISteamClient *)fnCreateInterface(STEAMCLIENT_INTERFACE_VERSION);
    networking_sockets = client->GetISteamGenericInterface(hSteamUser, hSteamPipe, "SteamNetworkingSockets001");
    networking_utils = new Networking_Utils_DLL<ISteamNetworkingUtilsDll>( (ISteamNetworkingUtils *)client->GetISteamGenericInterface(hSteamUser, hSteamPipe, "SteamNetworkingUtils001") );
    networking_p2p = new Networking_P2P_DLL<ISteamNetworkingP2P>();
    return true;
}

NETWORKING_SOCKET_LIB_API bool SteamDatagramClient_Init_InternalV9( SteamDatagramErrMsg &errMsg, FSteamInternal_CreateInterface fnCreateInterface, HSteamUser hSteamUser, HSteamPipe hSteamPipe )
{
    PRINT_DEBUG("SteamNetworkingSocketsLib::SteamDatagramClient_Init_InternalV9 %u %u", hSteamUser, hSteamPipe);
    return SteamDatagramClient_Init_InternalV6(errMsg, fnCreateInterface, hSteamUser, hSteamPipe );
}

NETWORKING_SOCKET_LIB_API void SteamDatagramServer_Kill( )
{
    PRINT_DEBUG("SteamNetworkingSocketsLib::SteamDatagramServer_Kill");
}

NETWORKING_SOCKET_LIB_API void SteamNetworkingSockets_SetDebugOutputFunction( ESteamNetworkingSocketsDebugOutputType eDetailLevel, FSteamNetworkingSocketsDebugOutput pfnFunc )
{
    PRINT_DEBUG("SteamNetworkingSocketsLib::SteamNetworkingSockets_SetDebugOutputFunction %i", eDetailLevel);
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
    PRINT_DEBUG("SteamNetworkingSocketsLib::SteamDatagramClient_Internal_SteamAPIKludge");
}

NETWORKING_SOCKET_LIB_API void SteamDatagramClient_Kill()
{
    PRINT_DEBUG("SteamNetworkingSocketsLib::SteamDatagramClient_Kill");
}

NETWORKING_SOCKET_LIB_API bool SteamDatagramServer_Init_Internal( SteamDatagramErrMsg &errMsg, FSteamInternal_CreateInterface fnCreateInterface, HSteamUser hSteamUser, HSteamPipe hSteamPipe )
{
    PRINT_DEBUG("SteamNetworkingSocketsLib::SteamDatagramServer_Init_Internal %u %u", hSteamUser, hSteamPipe);
    ISteamClient *client = (ISteamClient *)fnCreateInterface(STEAMCLIENT_INTERFACE_VERSION);
    networking_sockets_gameserver = client->GetISteamGenericInterface(hSteamUser, hSteamPipe, "SteamNetworkingSockets001");
    networking_utils = new Networking_Utils_DLL<ISteamNetworkingUtilsDll>( (ISteamNetworkingUtils *)client->GetISteamGenericInterface(hSteamUser, hSteamPipe, "SteamNetworkingUtils001"));
    return true;
}



NETWORKING_SOCKET_LIB_API class ISteamNetworkingUtils *SteamNetworkingUtils()
{
    PRINT_DEBUG("SteamNetworkingSocketsLib::SteamNetworkingUtils");
    return (class ISteamNetworkingUtils *)networking_utils;
}

NETWORKING_SOCKET_LIB_API void *SteamNetworkingP2P()
{
    PRINT_DEBUG("SteamNetworkingSocketsLib::SteamNetworkingP2P");
    return networking_p2p;
}

NETWORKING_SOCKET_LIB_API void *SteamNetworkingP2PGameServer()
{
    PRINT_DEBUG("SteamNetworkingSocketsLib::SteamNetworkingP2PGameServer");
    return NULL;
}
