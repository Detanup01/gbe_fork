#include <stdio.h>
#define NETWORKSOCKETS_DLL
#define STEAM_API_EXPORTS
#include "steam/steam_gameserver.h"

#if defined(WIN32) || defined(_WIN32)
#include <Windows.h>
#ifndef EMU_RELEASE_BUILD
#define PRINT_DEBUG(a, ...) do {FILE *t = fopen("STEAM_LOG.txt", "a"); fprintf(t, "%u " a, GetCurrentThreadId(), __VA_ARGS__); fclose(t);} while (0)
#endif
#else
#define PATH_SEPARATOR "/" 
#ifndef EMU_RELEASE_BUILD
#define PRINT_DEBUG(...) {FILE *t = fopen("STEAM_LOG.txt", "a"); fprintf(t, __VA_ARGS__); fclose(t);}
#endif
#endif
//#define PRINT_DEBUG(...) fprintf(stdout, __VA_ARGS__)
#ifdef EMU_RELEASE_BUILD
#define PRINT_DEBUG(...)
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

extern "C" __declspec( dllexport ) void *SteamNetworkingSocketsGameServer()
{
    PRINT_DEBUG("SteamNetworkingSocketsGameServer\n");
    return networking_sockets_gameserver;
}

extern "C" __declspec( dllexport ) void *SteamNetworkingSockets()
{
    PRINT_DEBUG("SteamNetworkingSockets\n");
    return networking_sockets;
}

const int k_cchMaxSteamDatagramErrMsg = 1024;
typedef char SteamDatagramErrMsg[ k_cchMaxSteamDatagramErrMsg ];
typedef void * ( S_CALLTYPE *FSteamInternal_CreateInterface )( const char *);

extern "C" __declspec( dllexport ) bool SteamDatagramClient_Init_InternalV6( SteamDatagramErrMsg &errMsg, FSteamInternal_CreateInterface fnCreateInterface, HSteamUser hSteamUser, HSteamPipe hSteamPipe )
{
    PRINT_DEBUG("SteamDatagramClient_Init_InternalV6 %u %u\n", hSteamUser, hSteamPipe);
    ISteamClient *client = (ISteamClient *)fnCreateInterface(STEAMCLIENT_INTERFACE_VERSION);
    networking_sockets = client->GetISteamGenericInterface(hSteamUser, hSteamPipe, "SteamNetworkingSockets001");
    networking_utils = new Networking_Utils_DLL<ISteamNetworkingUtilsDll>( (ISteamNetworkingUtils *)client->GetISteamGenericInterface(hSteamUser, hSteamPipe, "SteamNetworkingUtils001") );
    networking_p2p = new Networking_P2P_DLL<ISteamNetworkingP2P>();
    return true;
}

extern "C" __declspec( dllexport ) bool SteamDatagramClient_Init_InternalV9( SteamDatagramErrMsg &errMsg, FSteamInternal_CreateInterface fnCreateInterface, HSteamUser hSteamUser, HSteamPipe hSteamPipe )
{
    PRINT_DEBUG("SteamDatagramClient_Init_InternalV9 %u %u\n", hSteamUser, hSteamPipe);
    return SteamDatagramClient_Init_InternalV6(errMsg, fnCreateInterface, hSteamUser, hSteamPipe );
}

extern "C" __declspec( dllexport ) void SteamDatagramServer_Kill( )
{
    PRINT_DEBUG("SteamDatagramServer_Kill\n");
}

extern "C" __declspec( dllexport ) void SteamNetworkingSockets_SetDebugOutputFunction( ESteamNetworkingSocketsDebugOutputType eDetailLevel, FSteamNetworkingSocketsDebugOutput pfnFunc )
{
    PRINT_DEBUG("SteamNetworkingSockets_SetDebugOutputFunction %i\n", eDetailLevel);
    if (networking_utils) ((Networking_Utils_DLL<ISteamNetworkingUtilsDll> *)networking_utils)->networking_utils->SetDebugOutputFunction(eDetailLevel, pfnFunc);
}

typedef void ( S_CALLTYPE *FSteamAPI_RegisterCallback)( class CCallbackBase *pCallback, int iCallback );
typedef void ( S_CALLTYPE *FSteamAPI_UnregisterCallback)( class CCallbackBase *pCallback );
typedef void ( S_CALLTYPE *FSteamAPI_RegisterCallResult)( class CCallbackBase *pCallback, SteamAPICall_t hAPICall );
typedef void ( S_CALLTYPE *FSteamAPI_UnregisterCallResult)( class CCallbackBase *pCallback, SteamAPICall_t hAPICall );
extern "C" __declspec( dllexport ) void SteamDatagramClient_Internal_SteamAPIKludge( FSteamAPI_RegisterCallback fnRegisterCallback, FSteamAPI_UnregisterCallback fnUnregisterCallback, FSteamAPI_RegisterCallResult fnRegisterCallResult, FSteamAPI_UnregisterCallResult fnUnregisterCallResult )
{
    PRINT_DEBUG("SteamDatagramClient_Internal_SteamAPIKludge\n");
}

extern "C" __declspec( dllexport ) void SteamDatagramClient_Kill()
{
    PRINT_DEBUG("SteamDatagramClient_Kill\n");
}

extern "C" __declspec( dllexport ) bool SteamDatagramServer_Init_Internal( SteamDatagramErrMsg &errMsg, FSteamInternal_CreateInterface fnCreateInterface, HSteamUser hSteamUser, HSteamPipe hSteamPipe )
{
    PRINT_DEBUG("SteamDatagramServer_Init_Internal %u %u\n", hSteamUser, hSteamPipe);
    ISteamClient *client = (ISteamClient *)fnCreateInterface(STEAMCLIENT_INTERFACE_VERSION);
    networking_sockets_gameserver = client->GetISteamGenericInterface(hSteamUser, hSteamPipe, "SteamNetworkingSockets001");
    networking_utils = new Networking_Utils_DLL<ISteamNetworkingUtilsDll>( (ISteamNetworkingUtils *)client->GetISteamGenericInterface(hSteamUser, hSteamPipe, "SteamNetworkingUtils001"));
    return true;
}



extern "C" __declspec( dllexport ) void *SteamNetworkingUtils()
{
    PRINT_DEBUG("SteamNetworkingUtils\n");
    return networking_utils;
}

extern "C" __declspec( dllexport ) void *SteamNetworkingP2P()
{
    PRINT_DEBUG("SteamNetworkingP2P\n");
    return networking_p2p;
}

extern "C" __declspec( dllexport ) void *SteamNetworkingP2PGameServer()
{
    PRINT_DEBUG("SteamNetworkingP2PGameServer\n");
    return NULL;
}
