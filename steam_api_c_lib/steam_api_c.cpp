// dll/steam_api_c.cpp
//
// Flat C wrapper (steam_api_c.dll, 32-bit Windows only) for the 2010
// Steamworks SDK. Every export forwards to gbe_fork's steam_api.dll.
//
// Design:
//   * Most SteamAPI_ISteam* flat entry points are exported by steam_api.dll
//     and forwarded under the legacy 2010-era names expected by older games.
//     Symbol that are not present in gbe_fork’s modern API surface are kept
//     as compatibility stubs.
//   * SteamAPI_ManualDispatch_* are exported by steam_api.dll and are used
//     to reimplement Steam_BGetCallback / Steam_FreeLastCallback without
//     needing ColdClientLoader's steamclient.dll.

#include "steam_api_c.h"
#include "dll/steam_masterserver_updater.h"

#include <windows.h>

// ---------------------------------------------------------------------------
// Dynamic loader for steam_api.dll
// ---------------------------------------------------------------------------
static HMODULE steam_api = nullptr;

static void EnsureSteamApiLoaded()
{
    if (!steam_api) steam_api = LoadLibraryA("steam_api.dll");
}

template <typename T>
static T GetProc(const char* name)
{
    EnsureSteamApiLoaded();
    return steam_api ? reinterpret_cast<T>(GetProcAddress(steam_api, name)) : nullptr;
}

// ---------------------------------------------------------------------------
// Instance accessors (lazy, one cache per interface family).
// ---------------------------------------------------------------------------
static intptr_t GetUser()
{
    using PFN = intptr_t(*)();
    static PFN p = GetProc<PFN>("SteamUser");
    return p ? p() : 0;
}

static intptr_t GetFriends()
{
    using PFN = intptr_t(*)();
    static PFN p = GetProc<PFN>("SteamFriends");
    return p ? p() : 0;
}

static intptr_t GetUtils()
{
    using PFN = intptr_t(*)();
    static PFN p = GetProc<PFN>("SteamUtils");
    return p ? p() : 0;
}

static intptr_t GetApps()
{
    using PFN = intptr_t(*)();
    static PFN p = GetProc<PFN>("SteamApps");
    return p ? p() : 0;
}

static intptr_t GetGameServer()
{
    using PFN = intptr_t(*)();
    static PFN p = GetProc<PFN>("SteamGameServer");
    return p ? p() : 0;
}

// ---------------------------------------------------------------------------
// ISteamMasterServerUpdater
// ---------------------------------------------------------------------------
static void* GetMSSUpdater()
{
    using PFN = void*(*)();
    static PFN p = GetProc<PFN>("SteamMasterServerUpdater");
    return p ? p() : nullptr;
}

// ---------------------------------------------------------------------------
// Top-level SteamAPI_* / SteamGameServer_*
// ---------------------------------------------------------------------------
S_API_EXPORT bool SteamAPI_Init()
{
    using PFN = bool(*)();
    static PFN p = GetProc<PFN>("SteamAPI_Init");
    return p ? p() : false;
}

S_API_EXPORT void SteamAPI_Shutdown()
{
    using PFN = void(*)();
    static PFN p = GetProc<PFN>("SteamAPI_Shutdown");
    if (p) p();
}

S_API_EXPORT HSteamPipe SteamAPI_GetHSteamPipe()
{
    using PFN = HSteamPipe(*)();
    static PFN p = GetProc<PFN>("SteamAPI_GetHSteamPipe");
    return p ? p() : 0;
}

S_API_EXPORT bool SteamGameServer_Init(uint32 unIP, uint16 usPort, uint16 usGamePort, uint16 usSpectatorPort, uint16 usQueryPort, EServerMode eServerMode, const char* pchGameDir, const char* pchVersionString)
{
    using PFN = bool(*)(uint32, uint16, uint16, uint16, uint16, EServerMode, const char*, const char*);
    static PFN p = GetProc<PFN>("SteamGameServer_Init");
    return p ? p(unIP, usPort, usGamePort, usSpectatorPort, usQueryPort, eServerMode, pchGameDir, pchVersionString) : false;
}

S_API_EXPORT void SteamGameServer_Shutdown()
{
    using PFN = void(*)();
    static PFN p = GetProc<PFN>("SteamGameServer_Shutdown");
    if (p) p();
}

S_API_EXPORT HSteamPipe SteamGameServer_GetHSteamPipe()
{
    using PFN = HSteamPipe(*)();
    static PFN p = GetProc<PFN>("SteamGameServer_GetHSteamPipe");
    return p ? p() : 0;
}

S_API_EXPORT void SteamGameServer_RunCallbacks()
{
    using PFN = void(*)();
    static PFN p = GetProc<PFN>("SteamGameServer_RunCallbacks");
    if (p) p();
}

// ---------------------------------------------------------------------------
// ISteamGameServer_*
// ---------------------------------------------------------------------------
S_API_EXPORT bool ISteamGameServer_SendUserConnectAndAuthenticate(uint32 unIPClient, const void* pvAuthBlob, uint32 cubAuthBlobSize, uint64* pSteamIDUser)
{
    using PFN = bool(*)(intptr_t, uint32, const void*, uint32, uint64*);
    static PFN p = GetProc<PFN>("SteamAPI_ISteamGameServer_SendUserConnectAndAuthenticate");
    return p ? p(GetGameServer(), unIPClient, pvAuthBlob, cubAuthBlobSize, pSteamIDUser) : false;
}

S_API_EXPORT void ISteamGameServer_SendUserDisconnect(uint64 steamIDUser)
{
    using PFN = void(*)(intptr_t, uint64);
    static PFN p = GetProc<PFN>("SteamAPI_ISteamGameServer_SendUserDisconnect");
    if (p) p(GetGameServer(), steamIDUser);
}

// ISteamGameServer_UpdateStatus is a legacy Steamworks entry point whose
// fields map to several modern setters. The current player count (cPlayers)
// has no equivalent setter and is tracked through game-server connections.
S_API_EXPORT bool ISteamGameServer_UpdateStatus(int cPlayers, int cPlayersMax, int cBotPlayers, const char* pchServerName, const char* pSpectatorServerName, const char* pchMapName)
{
    (void)cPlayers; // Current players are tracked through game-server connections.

    const intptr_t gameServer = GetGameServer();
    if (!gameServer) return false;

    using SetMaxPlayerCountFn = void(*)(intptr_t, int);
    using SetBotPlayerCountFn = void(*)(intptr_t, int);
    using SetServerNameFn = void(*)(intptr_t, const char*);
    using SetSpectatorServerNameFn = void(*)(intptr_t, const char*);
    using SetMapNameFn = void(*)(intptr_t, const char*);

    static auto setMaxPlayerCount =
        GetProc<SetMaxPlayerCountFn>("SteamAPI_ISteamGameServer_SetMaxPlayerCount");
    static auto setBotPlayerCount =
        GetProc<SetBotPlayerCountFn>("SteamAPI_ISteamGameServer_SetBotPlayerCount");
    static auto setServerName =
        GetProc<SetServerNameFn>("SteamAPI_ISteamGameServer_SetServerName");
    static auto setSpectatorServerName =
        GetProc<SetSpectatorServerNameFn>("SteamAPI_ISteamGameServer_SetSpectatorServerName");
    static auto setMapName =
        GetProc<SetMapNameFn>("SteamAPI_ISteamGameServer_SetMapName");

    if (!setMaxPlayerCount || !setBotPlayerCount || !setServerName || !setSpectatorServerName || !setMapName)
        return false;

    setMaxPlayerCount(gameServer, cPlayersMax);
    setBotPlayerCount(gameServer, cBotPlayers);
    setServerName(gameServer, pchServerName ? pchServerName : "");
    setSpectatorServerName(gameServer, pSpectatorServerName ? pSpectatorServerName : "");
    setMapName(gameServer, pchMapName ? pchMapName : "");
    return true;
}

S_API_EXPORT uint64 ISteamGameServer_GetSteamID()
{
    using PFN = uint64(*)(intptr_t);
    static PFN p = GetProc<PFN>("SteamAPI_ISteamGameServer_GetSteamID");
    return p ? p(GetGameServer()) : 0;
}

S_API_EXPORT bool ISteamGameServer_BSecure()
{
    using PFN = bool(*)(intptr_t);
    static PFN p = GetProc<PFN>("SteamAPI_ISteamGameServer_BSecure");
    return p ? p(GetGameServer()) : false;
}

S_API_EXPORT bool ISteamGameServer_BLoggedOn()
{
    using PFN = bool(*)(intptr_t);
    static PFN p = GetProc<PFN>("SteamAPI_ISteamGameServer_BLoggedOn");
    return p ? p(GetGameServer()) : false;
}

S_API_EXPORT uint64 ISteamGameServer_CreateUnauthenticatedUserConnection()
{
    using PFN = uint64(*)(intptr_t);
    static PFN p = GetProc<PFN>("SteamAPI_ISteamGameServer_CreateUnauthenticatedUserConnection");
    return p ? p(GetGameServer()) : 0;
}

S_API_EXPORT bool ISteamGameServer_BUpdateUserData(uint64 steamIDUser, const char* pchPlayerName, uint32 uScore)
{
    using PFN = bool(*)(intptr_t, uint64, const char*, uint32);
    static PFN p = GetProc<PFN>("SteamAPI_ISteamGameServer_BUpdateUserData");
    return p ? p(GetGameServer(), steamIDUser, pchPlayerName, uScore) : false;
}

// taken info from Steamworks SDK 1.16, stubbed since currently there's no modern equivalent
// Thought, BSetServerType do exist and are mentioned in dll/dll/steam_gameserver.h,
// dll/steam_gameserver.cpp and dll/dll.cpp. But I'm not gonna touch anything outside of
// this folder.
S_API_EXPORT bool ISteamGameServer_SetServerType(uint32, uint32, uint16, uint16, uint16, const char*, const char*, bool)
{
    return false;
}

// ---------------------------------------------------------------------------
// ISteamUser_*
// ---------------------------------------------------------------------------
S_API_EXPORT int ISteamUser_InitiateGameConnection(void* pAuthBlob, int cbMaxAuthBlob, uint64 steamIDGameServer, uint32 unIPServer, uint16 usPortServer, bool bSecure)
{
    using PFN = int(*)(intptr_t, void*, int, uint64, uint32, uint16, bool);
    static PFN p = GetProc<PFN>("SteamAPI_ISteamUser_InitiateGameConnection");
    return p ? p(GetUser(), pAuthBlob, cbMaxAuthBlob, steamIDGameServer, unIPServer, usPortServer, bSecure) : 0;
}

S_API_EXPORT void ISteamUser_TerminateGameConnection(uint32 unIPServer, uint16 usPortServer)
{
    using PFN = void(*)(intptr_t, uint32, uint16);
    static PFN p = GetProc<PFN>("SteamAPI_ISteamUser_TerminateGameConnection");
    if (p) p(GetUser(), unIPServer, usPortServer);
}

// ---------------------------------------------------------------------------
// ISteamApps_*
// ---------------------------------------------------------------------------
S_API_EXPORT bool ISteamApps_BIsLowViolence()
{
    using PFN = bool(*)(intptr_t);
    static PFN p = GetProc<PFN>("SteamAPI_ISteamApps_BIsLowViolence");
    return p ? p(GetApps()) : false;
}

S_API_EXPORT const char* ISteamApps_GetCurrentGameLanguage()
{
    using PFN = const char*(*)(intptr_t);
    static PFN p = GetProc<PFN>("SteamAPI_ISteamApps_GetCurrentGameLanguage");
    return p ? p(GetApps()) : "english";
}

S_API_EXPORT bool ISteamApps_BIsSubscribedApp(uint32 appID)
{
    using PFN = bool(*)(intptr_t, uint32);
    static PFN p = GetProc<PFN>("SteamAPI_ISteamApps_BIsSubscribedApp");
    return p ? p(GetApps(), appID) : false;
}

// ---------------------------------------------------------------------------
// ISteamUtils_*
// ---------------------------------------------------------------------------
S_API_EXPORT uint32 ISteamUtils_GetAppID()
{
    using PFN = uint32(*)(intptr_t);
    static PFN p = GetProc<PFN>("SteamAPI_ISteamUtils_GetAppID");
    return p ? p(GetUtils()) : 0;
}

// ---------------------------------------------------------------------------
// SteamFriends() / SteamUser() accessors
// ---------------------------------------------------------------------------
S_API_EXPORT ISteamFriends* SteamFriends()
{
    return reinterpret_cast<ISteamFriends*>(GetFriends());
}

S_API_EXPORT void* SteamUser()
{
    return reinterpret_cast<void*>(GetUser());
}

// ---------------------------------------------------------------------------
// Steam_BGetCallback / Steam_FreeLastCallback
// gbe_fork implements these on the steamclient.dll side as thin wrappers around
// SteamAPI_ManualDispatch_Init() + SteamAPI_ManualDispatch_GetNextCallback()
// and SteamAPI_ManualDispatch_FreeLastCallback().
// ---------------------------------------------------------------------------
S_API_EXPORT bool Steam_BGetCallback(HSteamPipe hSteamPipe, CallbackMsg_t* pCallbackMsg, SteamAPICall_t* phSteamCall)
{
    if (phSteamCall) *phSteamCall = 0;

    using PFN_Init = void(*)();
    static PFN_Init pInit = GetProc<PFN_Init>("SteamAPI_ManualDispatch_Init");
    static bool inited = false;
    if (!inited && pInit) { pInit(); inited = true; }

    using PFN_Get = bool(*)(HSteamPipe, CallbackMsg_t*);
    static PFN_Get pGet = GetProc<PFN_Get>("SteamAPI_ManualDispatch_GetNextCallback");
    return pGet ? pGet(hSteamPipe, pCallbackMsg) : false;
}

S_API_EXPORT void Steam_FreeLastCallback(HSteamPipe hSteamPipe)
{
    using PFN = void(*)(HSteamPipe);
    static PFN p = GetProc<PFN>("SteamAPI_ManualDispatch_FreeLastCallback");
    if (p) p(hSteamPipe);
}

// ---------------------------------------------------------------------------
// Crash-dump helpers – forwarded when present, otherwise no-ops.
// ---------------------------------------------------------------------------
S_API_EXPORT void SteamAPI_WriteMiniDump(uint32 uStructuredExceptionCode, void* pvExceptionInfo, uint32 uBuildID)
{
    using PFN = void(*)(uint32, void*, uint32);
    static PFN p = GetProc<PFN>("SteamAPI_WriteMiniDump");
    if (p) p(uStructuredExceptionCode, pvExceptionInfo, uBuildID);
}

S_API_EXPORT void SteamAPI_SetMiniDumpComment(const char* pchMsg)
{
    using PFN = void(*)(const char*);
    static PFN p = GetProc<PFN>("SteamAPI_SetMiniDumpComment");
    if (p) p(pchMsg);
}

S_API_EXPORT void SteamAPI_UseBreakpadCrashHandler(char const* pchVersion, char const* pchDate, char const* pchTime, bool bFullMemoryDumps, void* pvContext, PFNPreMinidumpCallback m_pfnPreMinidumpCallback)
{
    using PFN = void(*)(char const*, char const*, char const*, bool, void*, PFNPreMinidumpCallback);
    static PFN p = GetProc<PFN>("SteamAPI_UseBreakpadCrashHandler");
    if (p) p(pchVersion, pchDate, pchTime, bFullMemoryDumps, pvContext, m_pfnPreMinidumpCallback);
}

S_API_EXPORT void SteamAPI_SetBreakpadAppID(uint32 unAppID)
{
    using PFN = void(*)(uint32);
    static PFN p = GetProc<PFN>("SteamAPI_SetBreakpadAppID");
    if (p) p(unAppID);
}

// ---------------------------------------------------------------------------
// ISteamMasterServerUpdater_* – dispatch through the real gbe_fork interface.
// ---------------------------------------------------------------------------
S_API_EXPORT void ISteamMasterServerUpdater_SetActive(bool bActive)
{
    auto* p = reinterpret_cast<Steam_Masterserver_Updater*>(GetMSSUpdater());
    if (p) p->SetActive(bActive);
}

S_API_EXPORT void ISteamMasterServerUpdater_SetHeartbeatInterval(int i)
{
    auto* p = reinterpret_cast<Steam_Masterserver_Updater*>(GetMSSUpdater());
    if (p) p->SetHeartbeatInterval(i);
}

S_API_EXPORT bool ISteamMasterServerUpdater_HandleIncomingPacket(const void* pData, int cbData, uint32 srcIP, uint16 srcPort)
{
    auto* p = reinterpret_cast<Steam_Masterserver_Updater*>(GetMSSUpdater());
    return p ? p->HandleIncomingPacket(pData, cbData, srcIP, srcPort) : false;
}

S_API_EXPORT int ISteamMasterServerUpdater_GetNextOutgoingPacket(void* pOut, int cbMaxOut, uint32* pNetAdr, uint16* pPort)
{
    auto* p = reinterpret_cast<Steam_Masterserver_Updater*>(GetMSSUpdater());
    return p ? p->GetNextOutgoingPacket(pOut, cbMaxOut, pNetAdr, pPort) : 0;
}

S_API_EXPORT void ISteamMasterServerUpdater_SetBasicServerData(unsigned short nProtocolVersion, bool bDedicatedServer, const char* pRegionName, const char* pProductName, unsigned short nMaxReportedClients, bool bPasswordProtected, const char* pGameDescription)
{
    if (auto* p = reinterpret_cast<Steam_Masterserver_Updater*>(GetMSSUpdater()))
        p->SetBasicServerData(nProtocolVersion, bDedicatedServer, pRegionName, pProductName, nMaxReportedClients, bPasswordProtected, pGameDescription);
}

S_API_EXPORT void ISteamMasterServerUpdater_ClearAllKeyValues()
{
    if (auto* p = reinterpret_cast<Steam_Masterserver_Updater*>(GetMSSUpdater()))
        p->ClearAllKeyValues();
}

S_API_EXPORT void ISteamMasterServerUpdater_SetKeyValue(const char* pKey, const char* pValue)
{
    if (auto* p = reinterpret_cast<Steam_Masterserver_Updater*>(GetMSSUpdater()))
        p->SetKeyValue(pKey, pValue);
}

S_API_EXPORT void ISteamMasterServerUpdater_NotifyShutdown()
{
    if (auto* p = reinterpret_cast<Steam_Masterserver_Updater*>(GetMSSUpdater()))
        p->NotifyShutdown();
}

S_API_EXPORT bool ISteamMasterServerUpdater_WasRestartRequested()
{
    auto* p = reinterpret_cast<Steam_Masterserver_Updater*>(GetMSSUpdater());
    return p ? p->WasRestartRequested() : false;
}

S_API_EXPORT void ISteamMasterServerUpdater_ForceHeartbeat()
{
    auto* p = reinterpret_cast<Steam_Masterserver_Updater*>(GetMSSUpdater());
    if (p) p->ForceHeartbeat();
}

S_API_EXPORT int ISteamMasterServerUpdater_GetNumMasterServers()
{
    auto* p = reinterpret_cast<Steam_Masterserver_Updater*>(GetMSSUpdater());
    return p ? p->GetNumMasterServers() : 0;
}

S_API_EXPORT int ISteamMasterServerUpdater_GetMasterServerAddress(int iServer, char* pOut, int outBufferSize)
{
    auto* p = reinterpret_cast<Steam_Masterserver_Updater*>(GetMSSUpdater());
    return p ? p->GetMasterServerAddress(iServer, pOut, outBufferSize) : 0;
}

S_API_EXPORT bool ISteamMasterServerUpdater_AddMasterServer(const char* pServerAddress)
{
    auto* p = reinterpret_cast<Steam_Masterserver_Updater*>(GetMSSUpdater());
    return p ? p->AddMasterServer(pServerAddress) : false;
}

S_API_EXPORT bool ISteamMasterServerUpdater_RemoveMasterServer(const char* pServerAddress)
{
    auto* p = reinterpret_cast<Steam_Masterserver_Updater*>(GetMSSUpdater());
    return p ? p->RemoveMasterServer(pServerAddress) : false;
}
