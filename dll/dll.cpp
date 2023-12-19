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

#define STEAM_API_FUNCTIONS_IMPL
#include "dll.h"


static char old_client[128] = STEAMCLIENT_INTERFACE_VERSION; //"SteamClient017";
static char old_gameserver[128] = STEAMGAMESERVER_INTERFACE_VERSION; //"SteamGameServer012";
static char old_gameserver_stats[128] = STEAMGAMESERVERSTATS_INTERFACE_VERSION; //"SteamGameServerStats001";
static char old_user[128] = STEAMUSER_INTERFACE_VERSION; //"SteamUser018";
static char old_friends[128] = STEAMFRIENDS_INTERFACE_VERSION; //"SteamFriends015";
static char old_utils[128] = STEAMUTILS_INTERFACE_VERSION; //"SteamUtils007";
static char old_matchmaking[128] = STEAMMATCHMAKING_INTERFACE_VERSION; //"SteamMatchMaking009";
static char old_matchmaking_servers[128] = STEAMMATCHMAKINGSERVERS_INTERFACE_VERSION; //"SteamMatchMakingServers002";
static char old_userstats[128] = STEAMUSERSTATS_INTERFACE_VERSION; //"STEAMUSERSTATS_INTERFACE_VERSION011";
static char old_apps[128] = STEAMAPPS_INTERFACE_VERSION; //"STEAMAPPS_INTERFACE_VERSION007";
static char old_networking[128] = STEAMNETWORKING_INTERFACE_VERSION; //"SteamNetworking005";
static char old_remote_storage_interface[128] = STEAMREMOTESTORAGE_INTERFACE_VERSION; //"STEAMREMOTESTORAGE_INTERFACE_VERSION013";
static char old_screenshots[128] = STEAMSCREENSHOTS_INTERFACE_VERSION; //"STEAMSCREENSHOTS_INTERFACE_VERSION002";
static char old_http[128] = STEAMHTTP_INTERFACE_VERSION; //"STEAMHTTP_INTERFACE_VERSION002";
static char old_unified_messages[128] = STEAMUNIFIEDMESSAGES_INTERFACE_VERSION; //"STEAMUNIFIEDMESSAGES_INTERFACE_VERSION001";
static char old_controller[128] = STEAMCONTROLLER_INTERFACE_VERSION; //"SteamController003";
static char old_ugc_interface[128] = STEAMUGC_INTERFACE_VERSION; //"STEAMUGC_INTERFACE_VERSION007";
static char old_applist[128] = STEAMAPPLIST_INTERFACE_VERSION; //"STEAMAPPLIST_INTERFACE_VERSION001";
static char old_music[128] = STEAMMUSIC_INTERFACE_VERSION; //"STEAMMUSIC_INTERFACE_VERSION001";
static char old_music_remote[128] = STEAMMUSICREMOTE_INTERFACE_VERSION; //"STEAMMUSICREMOTE_INTERFACE_VERSION001";
static char old_html_surface[128] = STEAMHTMLSURFACE_INTERFACE_VERSION; //"STEAMHTMLSURFACE_INTERFACE_VERSION_003";
static char old_inventory[128] = STEAMINVENTORY_INTERFACE_VERSION; //"STEAMINVENTORY_INTERFACE_V001";
static char old_video[128] = STEAMVIDEO_INTERFACE_VERSION; //"STEAMVIDEO_INTERFACE_V001";
static char old_masterserver_updater[128] = STEAMMASTERSERVERUPDATER_INTERFACE_VERSION; //"SteamMasterServerUpdater001";

static bool try_load_steam_interfaces(std::string interfaces_path)
{
    std::ifstream input( utf8_decode(interfaces_path) );
    if (!input.is_open()) {
        return false;
    }

    PRINT_DEBUG("load from: %s\n", interfaces_path.c_str());

    for( std::string line; getline( input, line ); )
    {
        PRINT_DEBUG("line: %s\n", line.c_str());
        line.erase(std::remove(line.begin(), line.end(), ' '), line.end());
        line.erase(std::remove(line.begin(), line.end(), '\n'), line.end());
        line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
        line.erase(std::remove(line.begin(), line.end(), '\t'), line.end());
#define REPLACE_WITH_FILE(s, f) {if (line.find(s) != std::string::npos) {strncpy(f, line.c_str(), sizeof(f) - 1); continue;}}
        REPLACE_WITH_FILE("SteamClient", old_client);
        REPLACE_WITH_FILE("SteamGameServer", old_gameserver);
        REPLACE_WITH_FILE("SteamGameServerStats", old_gameserver_stats);
        REPLACE_WITH_FILE("SteamUser", old_user);
        REPLACE_WITH_FILE("SteamFriends", old_friends);
        REPLACE_WITH_FILE("SteamUtils", old_utils);
        REPLACE_WITH_FILE("SteamMatchMaking", old_matchmaking);
        REPLACE_WITH_FILE("SteamMatchMakingServers", old_matchmaking_servers);
        REPLACE_WITH_FILE("STEAMUSERSTATS_INTERFACE_VERSION", old_userstats);
        REPLACE_WITH_FILE("STEAMAPPS_INTERFACE_VERSION", old_apps);
        REPLACE_WITH_FILE("SteamNetworking", old_networking);
        REPLACE_WITH_FILE("STEAMREMOTESTORAGE_INTERFACE_VERSION", old_remote_storage_interface);
        REPLACE_WITH_FILE("STEAMSCREENSHOTS_INTERFACE_VERSION", old_screenshots);
        REPLACE_WITH_FILE("STEAMHTTP_INTERFACE_VERSION", old_http);
        REPLACE_WITH_FILE("STEAMUNIFIEDMESSAGES_INTERFACE_VERSION", old_unified_messages);
        REPLACE_WITH_FILE("STEAMCONTROLLER_INTERFACE_VERSION", old_controller);
        REPLACE_WITH_FILE("SteamController", old_controller);
        REPLACE_WITH_FILE("STEAMUGC_INTERFACE_VERSION", old_ugc_interface);
        REPLACE_WITH_FILE("STEAMAPPLIST_INTERFACE_VERSION", old_applist);
        REPLACE_WITH_FILE("STEAMMUSIC_INTERFACE_VERSION", old_music);
        REPLACE_WITH_FILE("STEAMMUSICREMOTE_INTERFACE_VERSION", old_music_remote);
        REPLACE_WITH_FILE("STEAMHTMLSURFACE_INTERFACE_VERSION", old_html_surface);
        REPLACE_WITH_FILE("STEAMVIDEO_INTERFACE", old_video);
        REPLACE_WITH_FILE("STEAMINVENTORY_INTERFACE", old_inventory);
        REPLACE_WITH_FILE("SteamMasterServerUpdater", old_masterserver_updater);

        PRINT_DEBUG("NOT REPLACED %s\n", line.c_str());
#undef REPLACE_WITH_FILE
    }

    return true;
}

static void load_old_interface_versions()
{
    static bool loaded = false;
    if (loaded) return;

    if (!try_load_steam_interfaces(Local_Storage::get_game_settings_path() + "steam_interfaces.txt"))
        try_load_steam_interfaces(Local_Storage::get_program_path() + "steam_interfaces.txt");

    PRINT_DEBUG("client: %s\n", old_client);
    PRINT_DEBUG("gameserver: %s\n", old_gameserver);
    PRINT_DEBUG("gameserver stats: %s\n", old_gameserver_stats);
    PRINT_DEBUG("user: %s\n", old_user);
    PRINT_DEBUG("friends: %s\n", old_friends);
    PRINT_DEBUG("matchmaking: %s\n", old_matchmaking);
    PRINT_DEBUG("remote: %s\n", old_remote_storage_interface);
    PRINT_DEBUG("screenshots: %s\n", old_screenshots);
    PRINT_DEBUG("http: %s\n", old_http);
    PRINT_DEBUG("controller %s\n", old_controller);
    PRINT_DEBUG("ugc: %s\n", old_ugc_interface);
    PRINT_DEBUG("inventory: %s\n", old_inventory);
    PRINT_DEBUG("userstats: %s\n", old_userstats);
    PRINT_DEBUG("apps: %s\n", old_apps);
    PRINT_DEBUG("networking: %s\n", old_networking);
    PRINT_DEBUG("html surface: %s\n", old_html_surface);
    PRINT_DEBUG("utils: %s\n", old_utils);
    reset_LastError();
    loaded = true;
}

//steam_api_internal.h
STEAMAPI_API HSteamUser SteamAPI_GetHSteamUser()
{
    PRINT_DEBUG("SteamAPI_GetHSteamUser\n");
    if (!get_steam_client()->user_logged_in) return 0;
    return CLIENT_HSTEAMUSER;
}

STEAMAPI_API ISteamClient *g_pSteamClientGameServer;
#ifndef STEAMCLIENT_DLL
ISteamClient *g_pSteamClientGameServer;
#endif

static Steam_Client *steamclient_instance;
Steam_Client *get_steam_client()
{
    if (!steamclient_instance) {
        std::lock_guard<std::recursive_mutex> lock(global_mutex);
        // if we win the thread arbitration for the first time, this will still be null
        if (!steamclient_instance) {
            steamclient_instance = new Steam_Client();
        }
    }

    return steamclient_instance;
}

void destroy_client()
{
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (steamclient_instance) {
        delete steamclient_instance;
        steamclient_instance = NULL;
    }
}

Steam_Client *get_steam_client_old()
{
    return get_steam_client();
}

Steam_Client *get_steam_clientserver_old()
{
    return get_steam_client();
}

static bool steamclient_has_ipv6_functions_flag;
bool steamclient_has_ipv6_functions()
{
    return steamclient_has_ipv6_functions_flag || get_steam_client()->gameserver_has_ipv6_functions;
}

static void *create_client_interface(const char *ver)
{
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (strstr(ver, "SteamClient") == ver) {
        void *steam_client;

        if (strcmp(ver, "SteamClient007") == 0) {
            steam_client = (ISteamClient007 *)get_steam_client();
        } else if (strcmp(ver, "SteamClient008") == 0) {
            steam_client = (ISteamClient008 *)get_steam_client();
        } else if (strcmp(ver, "SteamClient009") == 0) {
            steam_client = (ISteamClient009 *)get_steam_client();
        } else if (strcmp(ver, "SteamClient010") == 0) {
            steam_client = (ISteamClient010 *)get_steam_client();
        } else if (strcmp(ver, "SteamClient011") == 0) {
            steam_client = (ISteamClient011 *)get_steam_client();
        } else if (strcmp(ver, "SteamClient012") == 0) {
            steam_client = (ISteamClient012 *)get_steam_client();
        } else if (strcmp(ver, "SteamClient013") == 0) {
            steam_client = (ISteamClient013 *)get_steam_client();
        } else if (strcmp(ver, "SteamClient014") == 0) {
            steam_client = (ISteamClient014 *)get_steam_client();
        } else if (strcmp(ver, "SteamClient015") == 0) {
            steam_client = (ISteamClient015 *)get_steam_client();
        } else if (strcmp(ver, "SteamClient016") == 0) {
            steam_client = (ISteamClient016 *)get_steam_client();
        } else if (strcmp(ver, "SteamClient017") == 0) {
            steam_client = (ISteamClient017 *)get_steam_client();
        } else if (strcmp(ver, "SteamClient018") == 0) {
            steam_client = (ISteamClient018 *)get_steam_client();
        } else if (strcmp(ver, "SteamClient019") == 0) {
            steam_client = (ISteamClient019 *)get_steam_client();
        } else if (strcmp(ver, STEAMCLIENT_INTERFACE_VERSION) == 0) {
            steam_client = (ISteamClient *)get_steam_client();
            steamclient_has_ipv6_functions_flag = true;
        } else {
            steam_client = (ISteamClient *)get_steam_client();
            steamclient_has_ipv6_functions_flag = true;
        }

        return steam_client;
    } else {
        return NULL;
    }
}

STEAMAPI_API void * S_CALLTYPE SteamInternal_CreateInterface( const char *ver )
{
    PRINT_DEBUG("SteamInternal_CreateInterface %s\n", ver);
    if (!get_steam_client()->user_logged_in && !get_steam_client()->IsServerInit()) return NULL;

    return create_client_interface(ver);
}

static uintp global_counter;
struct ContextInitData { void (*pFn)(void* pCtx); uintp counter; CSteamAPIContext ctx; };

STEAMAPI_API void * S_CALLTYPE SteamInternal_ContextInit( void *pContextInitData )
{
    //PRINT_DEBUG("SteamInternal_ContextInit\n");
    struct ContextInitData *contextInitData = (struct ContextInitData *)pContextInitData;
    if (contextInitData->counter != global_counter) {
        PRINT_DEBUG("SteamInternal_ContextInit initializing\n");
        contextInitData->pFn(&contextInitData->ctx);
        contextInitData->counter = global_counter;
    }

    return &contextInitData->ctx;
}

//steam_api.h

// Initialize the Steamworks SDK.
// On success k_ESteamAPIInitResult_OK is returned.  Otherwise, if pOutErrMsg is non-NULL,
// it will receive a non-localized message that explains the reason for the failure
//
// Example usage:
// 
//   SteamErrMsg errMsg;
//   if ( SteamAPI_Init(&errMsg) != k_ESteamAPIInitResult_OK )
//       FatalError( "Failed to init Steam.  %s", errMsg );
STEAMAPI_API ESteamAPIInitResult S_CALLTYPE SteamInternal_SteamAPI_Init( const char *pszInternalCheckInterfaceVersions, SteamErrMsg *pOutErrMsg )
{
    PRINT_DEBUG("SteamInternal_SteamAPI_Init: %s\n", pszInternalCheckInterfaceVersions);
    if (SteamAPI_Init()) {
        return ESteamAPIInitResult::k_ESteamAPIInitResult_OK;
    }

    if (pOutErrMsg) {
        memcpy(*pOutErrMsg, "SteamInitEx failed", 18);
        (*pOutErrMsg)[18] = 0;
        (*pOutErrMsg)[19] = 0;
    }
    return ESteamAPIInitResult::k_ESteamAPIInitResult_FailedGeneric;
}

// SteamAPI_Init must be called before using any other API functions. If it fails, an
// error message will be output to the debugger (or stderr) with further information.
static HSteamPipe user_steam_pipe;
STEAMAPI_API steam_bool S_CALLTYPE SteamAPI_Init()
{
    PRINT_DEBUG("SteamAPI_Init called\n");
    if (user_steam_pipe) return true;
#ifdef EMU_EXPERIMENTAL_BUILD
    crack_SteamAPI_Init();
#endif
    load_old_interface_versions();
    Steam_Client* client = get_steam_client();
    user_steam_pipe = client->CreateSteamPipe();
    client->ConnectToGlobalUser(user_steam_pipe);
    global_counter++;
    return true;
}

//TODO: not sure if this is the right signature for this function.
STEAMAPI_API steam_bool S_CALLTYPE SteamAPI_InitAnonymousUser()
{
    PRINT_DEBUG("SteamAPI_InitAnonymousUser called\n");
    return SteamAPI_Init();
}

static ISteamUser *old_user_instance;
static ISteamFriends *old_friends_interface;
static ISteamUtils *old_utils_interface;
static ISteamMatchmaking *old_matchmaking_instance;
static ISteamUserStats *old_userstats_instance;
static ISteamApps *old_apps_instance;
static ISteamMatchmakingServers *old_matchmakingservers_instance;
static ISteamNetworking *old_networking_instance;
static ISteamRemoteStorage *old_remotestorage_instance;
static ISteamScreenshots *old_screenshots_instance;
static ISteamHTTP *old_http_instance;
static ISteamController *old_controller_instance;
static ISteamUGC *old_ugc_instance;
static ISteamAppList *old_applist_instance;
static ISteamMusic *old_music_instance;
static ISteamMusicRemote *old_musicremote_instance;
static ISteamHTMLSurface *old_htmlsurface_instance;
static ISteamInventory *old_inventory_instance;
static ISteamVideo *old_video_instance;
static ISteamParentalSettings *old_parental_instance;
static ISteamUnifiedMessages *old_unified_instance;

// SteamAPI_Shutdown should be called during process shutdown if possible.
STEAMAPI_API void S_CALLTYPE SteamAPI_Shutdown()
{
    PRINT_DEBUG("SteamAPI_Shutdown\n");
    get_steam_client()->clientShutdown();
    get_steam_client()->BReleaseSteamPipe(user_steam_pipe);
    get_steam_client()->BShutdownIfAllPipesClosed();
    user_steam_pipe = 0;
    --global_counter;
    old_user_instance = NULL;
    old_friends_interface = NULL;
    old_utils_interface = NULL;
    old_matchmaking_instance = NULL;
    old_userstats_instance = NULL;
    old_apps_instance = NULL;
    old_matchmakingservers_instance = NULL;
    old_networking_instance = NULL;
    old_remotestorage_instance = NULL;
    old_screenshots_instance = NULL;
    old_http_instance = NULL;
    old_controller_instance = NULL;
    old_ugc_instance = NULL;
    old_applist_instance = NULL;
    old_music_instance = NULL;
    old_musicremote_instance = NULL;
    old_htmlsurface_instance = NULL;
    old_inventory_instance = NULL;
    old_video_instance = NULL;
    old_parental_instance = NULL;
    old_unified_instance = NULL;
    if (global_counter == 0) {
        destroy_client();
    }
}

// SteamAPI_RestartAppIfNecessary ensures that your executable was launched through Steam.
//
// Returns true if the current process should terminate. Steam is now re-launching your application.
//
// Returns false if no action needs to be taken. This means that your executable was started through
// the Steam client, or a steam_appid.txt file is present in your game's directory (for development).
// Your current process should continue if false is returned.
//
// NOTE: If you use the Steam DRM wrapper on your primary executable file, this check is unnecessary
// since the DRM wrapper will ensure that your application was launched properly through Steam.
STEAMAPI_API steam_bool S_CALLTYPE SteamAPI_RestartAppIfNecessary( uint32 unOwnAppID )
{
    PRINT_DEBUG("SteamAPI_RestartAppIfNecessary %u\n", unOwnAppID);
#ifdef EMU_EXPERIMENTAL_BUILD
    crack_SteamAPI_RestartAppIfNecessary(unOwnAppID);
#endif
    get_steam_client()->setAppID(unOwnAppID);
    return false;
}

// Many Steam API functions allocate a small amount of thread-local memory for parameter storage.
// SteamAPI_ReleaseCurrentThreadMemory() will free API memory associated with the calling thread.
// This function is also called automatically by SteamAPI_RunCallbacks(), so a single-threaded
// program never needs to explicitly call this function.
STEAMAPI_API void S_CALLTYPE SteamAPI_ReleaseCurrentThreadMemory()
{
    PRINT_DEBUG("SteamAPI_ReleaseCurrentThreadMemory\n");
}

// crash dump recording functions
STEAMAPI_API void S_CALLTYPE SteamAPI_WriteMiniDump( uint32 uStructuredExceptionCode, void* pvExceptionInfo, uint32 uBuildID )
{
    PRINT_DEBUG("SteamAPI_WriteMiniDump\n");
}

STEAMAPI_API void S_CALLTYPE SteamAPI_SetMiniDumpComment( const char *pchMsg )
{
    PRINT_DEBUG("SteamAPI_SetMiniDumpComment: %s\n", pchMsg);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------//
//	steam callback and call-result helpers
//
//	The following macros and classes are used to register your application for
//	callbacks and call-results, which are delivered in a predictable manner.
//
//	STEAM_CALLBACK macros are meant for use inside of a C++ class definition.
//	They map a Steam notification callback directly to a class member function
//	which is automatically prototyped as "void func( callback_type *pParam )".
//
//	CCallResult is used with specific Steam APIs that return "result handles".
//	The handle can be passed to a CCallResult object's Set function, along with
//	an object pointer and member-function pointer. The member function will
//	be executed once the results of the Steam API call are available.
//
//	CCallback and CCallbackManual classes can be used instead of STEAM_CALLBACK
//	macros if you require finer control over registration and unregistration.
//
//	Callbacks and call-results are queued automatically and are only
//	delivered/executed when your application calls SteamAPI_RunCallbacks().
//----------------------------------------------------------------------------------------------------------------------------------------------------------//

// SteamAPI_RunCallbacks is safe to call from multiple threads simultaneously,
// but if you choose to do this, callback code could be executed on any thread.
// One alternative is to call SteamAPI_RunCallbacks from the main thread only,
// and call SteamAPI_ReleaseCurrentThreadMemory regularly on other threads.
STEAMAPI_API void S_CALLTYPE SteamAPI_RunCallbacks()
{
    PRINT_DEBUG("SteamAPI_RunCallbacks\n");
    get_steam_client()->RunCallbacks(true, false);
    //std::this_thread::sleep_for(std::chrono::microseconds(1)); //fixes resident evil revelations lagging. (Seems to work fine without this right now, commenting out)
}


// Declares a callback member function plus a helper member variable which
// registers the callback on object creation and unregisters on destruction.
// The optional fourth 'var' param exists only for backwards-compatibility
// and can be ignored.
//#define STEAM_CALLBACK( thisclass, func, .../*callback_type, [deprecated] var*/ ) \
//	_STEAM_CALLBACK_SELECT( ( __VA_ARGS__, 4, 3 ), ( /**/, thisclass, func, __VA_ARGS__ ) )

// Declares a callback function and a named CCallbackManual variable which
// has Register and Unregister functions instead of automatic registration.
//#define STEAM_CALLBACK_MANUAL( thisclass, func, callback_type, var )	\
//	CCallbackManual< thisclass, callback_type > var; void func( callback_type *pParam )


// Internal functions used by the utility CCallback objects to receive callbacks
STEAMAPI_API void S_CALLTYPE SteamAPI_RegisterCallback( class CCallbackBase *pCallback, int iCallback )
{
    PRINT_DEBUG("SteamAPI_RegisterCallback %p %u funct:%u\n", pCallback, iCallback, pCallback->GetICallback());
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    get_steam_client()->RegisterCallback(pCallback, iCallback);
}

STEAMAPI_API void S_CALLTYPE SteamAPI_UnregisterCallback( class CCallbackBase *pCallback )
{
    PRINT_DEBUG("SteamAPI_UnregisterCallback %p\n", pCallback);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!steamclient_instance) return;
    get_steam_client()->UnregisterCallback(pCallback);
}

// Internal functions used by the utility CCallResult objects to receive async call results
STEAMAPI_API void S_CALLTYPE SteamAPI_RegisterCallResult( class CCallbackBase *pCallback, SteamAPICall_t hAPICall )
{
    PRINT_DEBUG("SteamAPI_RegisterCallResult\n");
    if (!hAPICall)
        return;

    get_steam_client()->RegisterCallResult(pCallback, hAPICall);
}

STEAMAPI_API void S_CALLTYPE SteamAPI_UnregisterCallResult( class CCallbackBase *pCallback, SteamAPICall_t hAPICall )
{
    PRINT_DEBUG("SteamAPI_UnregisterCallResult\n");
    if (!hAPICall)
        return;

    if (!steamclient_instance) return;
    get_steam_client()->UnregisterCallResult(pCallback, hAPICall);
}

STEAMAPI_API void *S_CALLTYPE SteamInternal_FindOrCreateUserInterface( HSteamUser hSteamUser, const char *pszVersion )
{
    PRINT_DEBUG("SteamInternal_FindOrCreateUserInterface %i %s\n", hSteamUser, pszVersion);
    return get_steam_client()->GetISteamGenericInterface(hSteamUser, SteamAPI_GetHSteamPipe(), pszVersion);
}

STEAMAPI_API void *S_CALLTYPE SteamInternal_FindOrCreateGameServerInterface( HSteamUser hSteamUser, const char *pszVersion )
{
    PRINT_DEBUG("SteamInternal_FindOrCreateGameServerInterface %i %s\n", hSteamUser, pszVersion);
    return get_steam_client()->GetISteamGenericInterface(hSteamUser, SteamGameServer_GetHSteamPipe(), pszVersion);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------//
//	steamclient.dll private wrapper functions
//
//	The following functions are part of abstracting API access to the steamclient.dll, but should only be used in very specific cases
//----------------------------------------------------------------------------------------------------------------------------------------------------------//

// SteamAPI_IsSteamRunning() returns true if Steam is currently running
STEAMAPI_API steam_bool S_CALLTYPE SteamAPI_IsSteamRunning()
{
    PRINT_DEBUG("SteamAPI_IsSteamRunning\n");
    return true;
}

// Pumps out all the steam messages, calling registered callbacks.
// NOT THREADSAFE - do not call from multiple threads simultaneously.
STEAMAPI_API void Steam_RunCallbacks( HSteamPipe hSteamPipe, bool bGameServerCallbacks )
{
    PRINT_DEBUG("Steam_RunCallbacks\n");

    SteamAPI_RunCallbacks();

    if (bGameServerCallbacks)
        SteamGameServer_RunCallbacks();
}

// register the callback funcs to use to interact with the steam dll
STEAMAPI_API void Steam_RegisterInterfaceFuncs( void *hModule )
{
    PRINT_DEBUG("Steam_RegisterInterfaceFuncs\n");
}

// returns the HSteamUser of the last user to dispatch a callback
STEAMAPI_API HSteamUser Steam_GetHSteamUserCurrent()
{
    PRINT_DEBUG("Steam_GetHSteamUserCurrent\n");
    //TODO
    return SteamAPI_GetHSteamUser();
}

// returns the filename path of the current running Steam process, used if you need to load an explicit steam dll by name.
// DEPRECATED - implementation is Windows only, and the path returned is a UTF-8 string which must be converted to UTF-16 for use with Win32 APIs
STEAMAPI_API const char *SteamAPI_GetSteamInstallPath()
{
    PRINT_DEBUG("SteamAPI_GetSteamInstallPath\n");
    static char steam_folder[1024];
    std::string path = Local_Storage::get_program_path();
    strcpy(steam_folder, path.c_str());
    steam_folder[path.length() - 1] = 0;
    return steam_folder;
}

// returns the pipe we are communicating to Steam with
STEAMAPI_API HSteamPipe SteamAPI_GetHSteamPipe()
{
    PRINT_DEBUG("SteamAPI_GetHSteamPipe\n");
    return user_steam_pipe;
}

// sets whether or not Steam_RunCallbacks() should do a try {} catch (...) {} around calls to issuing callbacks
STEAMAPI_API void SteamAPI_SetTryCatchCallbacks( bool bTryCatchCallbacks )
{
    PRINT_DEBUG("SteamAPI_SetTryCatchCallbacks\n");
}

// backwards compat export, passes through to SteamAPI_ variants
STEAMAPI_API HSteamPipe GetHSteamPipe()
{
    PRINT_DEBUG("GetHSteamPipe\n");
    return SteamAPI_GetHSteamPipe();
}

STEAMAPI_API HSteamUser GetHSteamUser()
{
    PRINT_DEBUG("GetHSteamUser\n");
    return SteamAPI_GetHSteamUser();
}


// exists only for backwards compat with code written against older SDKs
STEAMAPI_API steam_bool S_CALLTYPE SteamAPI_InitSafe()
{
    PRINT_DEBUG("SteamAPI_InitSafe\n");
    SteamAPI_Init();
    return true;
}

STEAMAPI_API ISteamClient *SteamClient() {
    PRINT_DEBUG("SteamClient()\n");
    load_old_interface_versions();
    if (!get_steam_client()->user_logged_in) return NULL;
    return (ISteamClient *)SteamInternal_CreateInterface(old_client);
}

#define CACHE_OLDSTEAM_INSTANCE(variable, get_func) { if (variable) return variable; else return variable = (get_func); }

STEAMAPI_API ISteamUser *SteamUser()								{ PRINT_DEBUG("SteamUser()\n"); CACHE_OLDSTEAM_INSTANCE(old_user_instance, get_steam_client_old()->GetISteamUser(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_user)) }
STEAMAPI_API ISteamFriends *SteamFriends()						{ PRINT_DEBUG("SteamFriends()\n"); CACHE_OLDSTEAM_INSTANCE(old_friends_interface, get_steam_client_old()->GetISteamFriends(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_friends )) }
STEAMAPI_API ISteamUtils *SteamUtils()							{ PRINT_DEBUG("SteamUtils()\n"); CACHE_OLDSTEAM_INSTANCE(old_utils_interface, get_steam_client_old()->GetISteamUtils(SteamAPI_GetHSteamPipe(), old_utils)) }
STEAMAPI_API ISteamMatchmaking *SteamMatchmaking()				{ PRINT_DEBUG("SteamMatchmaking()\n"); CACHE_OLDSTEAM_INSTANCE(old_matchmaking_instance, get_steam_client_old()->GetISteamMatchmaking(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_matchmaking)) }
STEAMAPI_API ISteamUserStats *SteamUserStats()					{ PRINT_DEBUG("SteamUserStats()\n"); CACHE_OLDSTEAM_INSTANCE(old_userstats_instance, get_steam_client_old()->GetISteamUserStats(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_userstats)) }
STEAMAPI_API ISteamApps *SteamApps()								{ PRINT_DEBUG("SteamApps()\n"); CACHE_OLDSTEAM_INSTANCE(old_apps_instance, get_steam_client_old()->GetISteamApps(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_apps)) }
STEAMAPI_API ISteamMatchmakingServers *SteamMatchmakingServers()	{ PRINT_DEBUG("SteamMatchmakingServers()\n"); CACHE_OLDSTEAM_INSTANCE(old_matchmakingservers_instance, get_steam_client_old()->GetISteamMatchmakingServers(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_matchmaking_servers)) }
STEAMAPI_API ISteamNetworking *SteamNetworking()					{ PRINT_DEBUG("SteamNetworking()\n"); CACHE_OLDSTEAM_INSTANCE(old_networking_instance, get_steam_client_old()->GetISteamNetworking(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_networking)) }
STEAMAPI_API ISteamRemoteStorage *SteamRemoteStorage()			{ PRINT_DEBUG("SteamRemoteStorage()\n"); CACHE_OLDSTEAM_INSTANCE(old_remotestorage_instance, get_steam_client_old()->GetISteamRemoteStorage(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_remote_storage_interface)) }
STEAMAPI_API ISteamScreenshots *SteamScreenshots()				{ PRINT_DEBUG("SteamScreenshots()\n"); CACHE_OLDSTEAM_INSTANCE(old_screenshots_instance, get_steam_client_old()->GetISteamScreenshots(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_screenshots)) }
STEAMAPI_API ISteamHTTP *SteamHTTP()								{ PRINT_DEBUG("SteamHTTP()\n"); CACHE_OLDSTEAM_INSTANCE(old_http_instance, get_steam_client_old()->GetISteamHTTP(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_http)) }
STEAMAPI_API ISteamController *SteamController()					{ PRINT_DEBUG("SteamController()\n"); CACHE_OLDSTEAM_INSTANCE(old_controller_instance, get_steam_client_old()->GetISteamController(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_controller)) }
STEAMAPI_API ISteamUGC *SteamUGC()								{ PRINT_DEBUG("SteamUGC()\n"); CACHE_OLDSTEAM_INSTANCE(old_ugc_instance, get_steam_client_old()->GetISteamUGC(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_ugc_interface )) }
STEAMAPI_API ISteamAppList *SteamAppList()						{ PRINT_DEBUG("SteamAppList()\n"); CACHE_OLDSTEAM_INSTANCE(old_applist_instance, get_steam_client_old()->GetISteamAppList(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_applist)) }
STEAMAPI_API ISteamMusic *SteamMusic()							{ PRINT_DEBUG("SteamMusic()\n"); CACHE_OLDSTEAM_INSTANCE(old_music_instance, get_steam_client_old()->GetISteamMusic(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_music)) }
STEAMAPI_API ISteamMusicRemote *SteamMusicRemote()				{ PRINT_DEBUG("SteamMusicRemote()\n"); CACHE_OLDSTEAM_INSTANCE(old_musicremote_instance, get_steam_client_old()->GetISteamMusicRemote(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_music_remote)) }
STEAMAPI_API ISteamHTMLSurface *SteamHTMLSurface()				{ PRINT_DEBUG("SteamHTMLSurface()\n"); CACHE_OLDSTEAM_INSTANCE(old_htmlsurface_instance, get_steam_client_old()->GetISteamHTMLSurface(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_html_surface)) }
STEAMAPI_API ISteamInventory *SteamInventory()					{ PRINT_DEBUG("SteamInventory()\n"); CACHE_OLDSTEAM_INSTANCE(old_inventory_instance, get_steam_client_old()->GetISteamInventory(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_inventory)) }
STEAMAPI_API ISteamVideo *SteamVideo()							{ PRINT_DEBUG("SteamVideo()\n"); CACHE_OLDSTEAM_INSTANCE(old_video_instance, get_steam_client_old()->GetISteamVideo(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_video)) }
STEAMAPI_API ISteamParentalSettings *SteamParentalSettings()		{ PRINT_DEBUG("SteamParentalSettings()\n"); CACHE_OLDSTEAM_INSTANCE(old_parental_instance, get_steam_client_old()->GetISteamParentalSettings(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), "")) }
STEAMAPI_API ISteamUnifiedMessages *SteamUnifiedMessages()      { PRINT_DEBUG("SteamUnifiedMessages()\n"); CACHE_OLDSTEAM_INSTANCE(old_unified_instance, get_steam_client_old()->GetISteamUnifiedMessages(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_unified_messages)) }


//Gameserver stuff

STEAMAPI_API void * S_CALLTYPE SteamGameServerInternal_CreateInterface( const char *ver )
{
    PRINT_DEBUG("SteamGameServerInternal_CreateInterface %s\n", ver);
    return SteamInternal_CreateInterface(ver);
}

static HSteamPipe server_steam_pipe;
STEAMAPI_API HSteamPipe S_CALLTYPE SteamGameServer_GetHSteamPipe()
{
    PRINT_DEBUG("SteamGameServer_GetHSteamPipe\n");
    return server_steam_pipe;
}

STEAMAPI_API HSteamUser S_CALLTYPE SteamGameServer_GetHSteamUser()
{
    PRINT_DEBUG("SteamGameServer_GetHSteamUser\n");
    if (!get_steam_client()->server_init) return 0;
    return SERVER_HSTEAMUSER;
}

//See: SteamGameServer_Init
//STEAMAPI_API steam_bool S_CALLTYPE SteamGameServer_InitSafe(uint32 unIP, uint16 usSteamPort, uint16 usGamePort, uint16 usQueryPort, EServerMode eServerMode, const char *pchVersionString )
STEAMAPI_API steam_bool S_CALLTYPE SteamGameServer_InitSafe( uint32 unIP, uint16 usSteamPort, uint16 usGamePort, uint16 unknown, EServerMode eServerMode, void *unknown1, void *unknown2, void *unknown3 )
{
    const char *pchVersionString;
    EServerMode serverMode;
    uint16 usQueryPort;
    load_old_interface_versions();
    bool logon_anon = false;
    if (strcmp(old_gameserver, "SteamGameServer010") == 0 || strstr(old_gameserver, "SteamGameServer00") == old_gameserver) {
        PRINT_DEBUG("Old game server init safe\n");
        pchVersionString = (char *)unknown3;
        memcpy(&serverMode, &unknown1, sizeof(serverMode));
        memcpy(&usQueryPort, (char *)&eServerMode, sizeof(usQueryPort));
        logon_anon = true;
    } else {
        pchVersionString = (char *)unknown1;
        serverMode = eServerMode;
        usQueryPort = unknown;
    }

    bool ret = SteamInternal_GameServer_Init( unIP, usSteamPort, usGamePort, usQueryPort, serverMode, pchVersionString );
    if (logon_anon) {
        get_steam_client()->steam_gameserver->LogOnAnonymous();
    }

    return ret;
}

STEAMAPI_API ISteamClient *SteamGameServerClient();

STEAMAPI_API steam_bool S_CALLTYPE SteamInternal_GameServer_Init( uint32 unIP, uint16 usPort, uint16 usGamePort, uint16 usQueryPort, EServerMode eServerMode, const char *pchVersionString )
{
    PRINT_DEBUG("SteamInternal_GameServer_Init %u %hu %hu %hu %u %s\n", unIP, usPort, usGamePort, usQueryPort, eServerMode, pchVersionString);
    if (!server_steam_pipe) {
        load_old_interface_versions();
        get_steam_client()->CreateLocalUser(&server_steam_pipe, k_EAccountTypeGameServer);
        ++global_counter;
        //g_pSteamClientGameServer is only used in pre 1.37 (where the interface versions are not provided by the game)
        g_pSteamClientGameServer = SteamGameServerClient();
    }

    uint32 unFlags = 0;
    if (eServerMode == eServerModeAuthenticationAndSecure) unFlags = k_unServerFlagSecure;
    return get_steam_client()->steam_gameserver->InitGameServer(unIP, usGamePort, usQueryPort, unFlags, 0, pchVersionString);
}

STEAMAPI_API ESteamAPIInitResult S_CALLTYPE SteamInternal_GameServer_Init_V2( uint32 unIP, uint16 usGamePort, uint16 usQueryPort, EServerMode eServerMode, const char *pchVersionString, const char *pszInternalCheckInterfaceVersions, SteamErrMsg *pOutErrMsg )
{
    PRINT_DEBUG("SteamInternal_GameServer_Init_V2 %u %hu %hu %u %s %s\n", unIP, usGamePort, usQueryPort, eServerMode, pchVersionString, pszInternalCheckInterfaceVersions);
    if (SteamInternal_GameServer_Init(unIP, 0, usGamePort, usQueryPort, eServerMode, pchVersionString)) {
        return ESteamAPIInitResult::k_ESteamAPIInitResult_OK;
    }
    if (pOutErrMsg) {
        memcpy(*pOutErrMsg, "GameServer_V2 failed", 20);
        (*pOutErrMsg)[20] = 0;
        (*pOutErrMsg)[21] = 0;
    }
    return ESteamAPIInitResult::k_ESteamAPIInitResult_FailedGeneric;
}

//SteamGameServer004 and before:
//STEAMAPI_API steam_bool SteamGameServer_Init( uint32 unIP, uint16 usPort, uint16 usGamePort, uint16 usSpectatorPort, uint16 usQueryPort, EServerMode eServerMode, int nGameAppId, const char *pchGameDir, const char *pchVersionString );
//SteamGameServer010 and before:
//STEAMAPI_API steam_bool SteamGameServer_Init( uint32 unIP, uint16 usPort, uint16 usGamePort, uint16 usSpectatorPort, uint16 usQueryPort, EServerMode eServerMode, const char *pchGameDir, const char *pchVersionString );
//SteamGameServer011 and later:
//STEAMAPI_API steam_bool SteamGameServer_Init( uint32 unIP, uint16 usSteamPort, uint16 usGamePort, uint16 usQueryPort, EServerMode eServerMode, const char *pchVersionString );
//SteamGameServer015 and later:
//STEAMAPI_API steam_bool SteamGameServer_Init( uint32 unIP, uint16 usGamePort, uint16 usQueryPort, EServerMode eServerMode, const char *pchVersionString );
STEAMAPI_API steam_bool SteamGameServer_Init( uint32 unIP, uint16 usSteamPort, uint16 usGamePort, uint16 unknown, EServerMode eServerMode, void *unknown1, void *unknown2, void *unknown3 )
{
    const char *pchVersionString;
    EServerMode serverMode;
    uint16 usQueryPort;
    load_old_interface_versions();
    bool logon_anon = false;
    if (strcmp(old_gameserver, "SteamGameServer010") == 0 || strstr(old_gameserver, "SteamGameServer00") == old_gameserver) {
        PRINT_DEBUG("Old game server init\n");
        pchVersionString = (char *)unknown3;
        memcpy(&serverMode, &unknown1, sizeof(serverMode));
        memcpy(&usQueryPort, (char *)&eServerMode, sizeof(usQueryPort));
        logon_anon = true;
    } else {
        pchVersionString = (char *)unknown1;
        serverMode = eServerMode;
        usQueryPort = unknown;
    }

    bool ret = SteamInternal_GameServer_Init( unIP, usSteamPort, usGamePort, usQueryPort, serverMode, pchVersionString );
    if (logon_anon) {
        get_steam_client()->steam_gameserver->LogOnAnonymous();
    }

    return ret;
}

static ISteamGameServer *old_gameserver_instance;
static ISteamUtils *old_gamserver_utils_instance;
static ISteamNetworking  *old_gamserver_networking_instance;
static ISteamGameServerStats *old_gamserver_stats_instance;
static ISteamHTTP *old_gamserver_http_instance;
static ISteamInventory *old_gamserver_inventory_instance;
static ISteamUGC *old_gamserver_ugc_instance;
static ISteamApps *old_gamserver_apps_instance;
static ISteamMasterServerUpdater *old_gamserver_masterupdater_instance;

STEAMAPI_API void SteamGameServer_Shutdown()
{
    PRINT_DEBUG("SteamGameServer_Shutdown\n");
    get_steam_client()->serverShutdown();
    get_steam_client()->BReleaseSteamPipe(server_steam_pipe);
    get_steam_client()->BShutdownIfAllPipesClosed();
    server_steam_pipe = 0;
    --global_counter;
    g_pSteamClientGameServer = NULL; //TODO: check if this actually gets nulled when SteamGameServer_Shutdown is called
    old_gameserver_instance = NULL;
    old_gamserver_utils_instance = NULL;
    old_gamserver_networking_instance = NULL;
    old_gamserver_stats_instance = NULL;
    old_gamserver_http_instance = NULL;
    old_gamserver_inventory_instance = NULL;
    old_gamserver_ugc_instance = NULL;
    old_gamserver_apps_instance = NULL;
    old_gamserver_masterupdater_instance = NULL;

    if (global_counter == 0) {
        destroy_client();
    }
}

STEAMAPI_API void SteamGameServer_RunCallbacks()
{
    PRINT_DEBUG("SteamGameServer_RunCallbacks\n");
    get_steam_client()->RunCallbacks(false, true);
}

STEAMAPI_API steam_bool SteamGameServer_BSecure()
{
    PRINT_DEBUG("SteamGameServer_BSecure\n");
    return get_steam_client()->steam_gameserver->BSecure();
}

STEAMAPI_API uint64 SteamGameServer_GetSteamID()
{
    PRINT_DEBUG("SteamGameServer_GetSteamID\n");
    return get_steam_client()->steam_gameserver->GetSteamID().ConvertToUint64();
}

STEAMAPI_API ISteamClient *SteamGameServerClient() { 
    PRINT_DEBUG("SteamGameServerClient()\n");
    if (!get_steam_clientserver_old()->IsServerInit()) return NULL;
    return (ISteamClient *)SteamInternal_CreateInterface(old_client); 
}

STEAMAPI_API ISteamGameServer *SteamGameServer() { PRINT_DEBUG("SteamGameServer()\n"); CACHE_OLDSTEAM_INSTANCE(old_gameserver_instance, get_steam_clientserver_old()->GetISteamGameServer(SteamGameServer_GetHSteamUser(), SteamGameServer_GetHSteamPipe(), old_gameserver )) }
STEAMAPI_API ISteamUtils *SteamGameServerUtils() { PRINT_DEBUG("SteamGameServerUtils()\n"); CACHE_OLDSTEAM_INSTANCE(old_gamserver_utils_instance, get_steam_clientserver_old()->GetISteamUtils(SteamGameServer_GetHSteamPipe(), old_utils )) }
STEAMAPI_API ISteamNetworking *SteamGameServerNetworking() { PRINT_DEBUG("SteamGameServerNetworking()\n"); CACHE_OLDSTEAM_INSTANCE(old_gamserver_networking_instance, get_steam_clientserver_old()->GetISteamNetworking(SteamGameServer_GetHSteamUser(), SteamGameServer_GetHSteamPipe(), old_networking )) }
STEAMAPI_API ISteamGameServerStats *SteamGameServerStats() { PRINT_DEBUG("SteamGameServerStats()\n"); CACHE_OLDSTEAM_INSTANCE(old_gamserver_stats_instance, get_steam_clientserver_old()->GetISteamGameServerStats(SteamGameServer_GetHSteamUser(), SteamGameServer_GetHSteamPipe(), old_gameserver_stats )) }
STEAMAPI_API ISteamHTTP *SteamGameServerHTTP() { PRINT_DEBUG("SteamGameServerHTTP()\n"); CACHE_OLDSTEAM_INSTANCE(old_gamserver_http_instance, get_steam_clientserver_old()->GetISteamHTTP(SteamGameServer_GetHSteamUser(), SteamGameServer_GetHSteamPipe(), old_http )) }
STEAMAPI_API ISteamInventory *SteamGameServerInventory() { PRINT_DEBUG("SteamGameServerInventory()\n"); CACHE_OLDSTEAM_INSTANCE(old_gamserver_inventory_instance, get_steam_clientserver_old()->GetISteamInventory(SteamGameServer_GetHSteamUser(), SteamGameServer_GetHSteamPipe(), old_inventory )) }
STEAMAPI_API ISteamUGC *SteamGameServerUGC() { PRINT_DEBUG("SteamGameServerUGC()\n"); CACHE_OLDSTEAM_INSTANCE(old_gamserver_ugc_instance, get_steam_clientserver_old()->GetISteamUGC(SteamGameServer_GetHSteamUser(), SteamGameServer_GetHSteamPipe(), old_ugc_interface )) }
STEAMAPI_API ISteamApps *SteamGameServerApps() { PRINT_DEBUG("SteamGameServerApps()\n"); CACHE_OLDSTEAM_INSTANCE(old_gamserver_apps_instance, get_steam_clientserver_old()->GetISteamApps(SteamGameServer_GetHSteamUser(), SteamGameServer_GetHSteamPipe(), old_apps )) }

STEAMAPI_API ISteamMasterServerUpdater *SteamMasterServerUpdater() {PRINT_DEBUG("SteamMasterServerUpdater()\n"); CACHE_OLDSTEAM_INSTANCE(old_gamserver_masterupdater_instance, get_steam_clientserver_old()->GetISteamMasterServerUpdater(SteamGameServer_GetHSteamUser(), SteamGameServer_GetHSteamPipe(), old_masterserver_updater)) }

STEAMAPI_API uint32 SteamGameServer_GetIPCCallCount()
{
    return get_steam_client()->GetIPCCallCount();
}


STEAMAPI_API void S_CALLTYPE SteamAPI_UseBreakpadCrashHandler( char const *pchVersion, char const *pchDate, char const *pchTime, bool bFullMemoryDumps, void *pvContext, PFNPreMinidumpCallback m_pfnPreMinidumpCallback )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
}

STEAMAPI_API void S_CALLTYPE SteamAPI_SetBreakpadAppID( uint32 unAppID )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
}

struct cb_data {
    int cb_id;
    std::vector<char> result;
};
static std::queue<struct cb_data> client_cb;
static std::queue<struct cb_data> server_cb;

static void cb_add_queue_server(std::vector<char> result, int callback)
{
    struct cb_data cb;
    cb.cb_id = callback;
    cb.result = result;
    server_cb.push(cb);
}

static void cb_add_queue_client(std::vector<char> result, int callback)
{
    struct cb_data cb;
    cb.cb_id = callback;
    cb.result = result;
    client_cb.push(cb);
}

/// Inform the API that you wish to use manual event dispatch.  This must be called after SteamAPI_Init, but before
/// you use any of the other manual dispatch functions below.
STEAMAPI_API void S_CALLTYPE SteamAPI_ManualDispatch_Init()
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
    Steam_Client *steam_client = get_steam_client();
    steam_client->callback_results_server->setCbAll(&cb_add_queue_server);
    steam_client->callback_results_client->setCbAll(&cb_add_queue_client);
}

/// Perform certain periodic actions that need to be performed.
STEAMAPI_API void S_CALLTYPE SteamAPI_ManualDispatch_RunFrame( HSteamPipe hSteamPipe )
{
    PRINT_DEBUG("%s %i\n", __FUNCTION__, hSteamPipe);
    Steam_Client *steam_client = get_steam_client();
    if (!steam_client->steam_pipes.count(hSteamPipe)) {
        return;
    }

    if (steam_client->steam_pipes[hSteamPipe] == Steam_Pipe::SERVER) {
        steam_client->RunCallbacks(false, true);
    } else if (steam_client->steam_pipes[hSteamPipe] == Steam_Pipe::CLIENT) {
        steam_client->RunCallbacks(true, false);
    }
}

/// Fetch the next pending callback on the given pipe, if any.  If a callback is available, true is returned
/// and the structure is populated.  In this case, you MUST call SteamAPI_ManualDispatch_FreeLastCallback
/// (after dispatching the callback) before calling SteamAPI_ManualDispatch_GetNextCallback again.
STEAMAPI_API steam_bool S_CALLTYPE SteamAPI_ManualDispatch_GetNextCallback( HSteamPipe hSteamPipe, CallbackMsg_t *pCallbackMsg )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
    std::queue<struct cb_data> *q = NULL;
    HSteamUser m_hSteamUser = 0;
    Steam_Client *steam_client = get_steam_client();
    if (!steam_client->steamclient_server_inited) {
        while(!server_cb.empty()) server_cb.pop();
    }

    if (!steam_client->steam_pipes.count(hSteamPipe)) {
        return false;
    }

    if (steam_client->steam_pipes[hSteamPipe] == Steam_Pipe::SERVER) {
        q = &server_cb;
        m_hSteamUser = SERVER_HSTEAMUSER;
    } else if (steam_client->steam_pipes[hSteamPipe] == Steam_Pipe::CLIENT) {
        q = &client_cb;
        m_hSteamUser = CLIENT_HSTEAMUSER;
    } else {
        return false;
    }

    if (q->empty()) return false;
    if (pCallbackMsg) {
        pCallbackMsg->m_hSteamUser = m_hSteamUser;
        pCallbackMsg->m_iCallback = q->front().cb_id;
        pCallbackMsg->m_pubParam = (uint8 *)&(q->front().result[0]);
        pCallbackMsg->m_cubParam = q->front().result.size();
        PRINT_DEBUG("Steam_BGetCallback cb number %i\n", q->front().cb_id);
        return true;
    }

    return false;
}

/// You must call this after dispatching the callback, if SteamAPI_ManualDispatch_GetNextCallback returns true.
STEAMAPI_API void S_CALLTYPE SteamAPI_ManualDispatch_FreeLastCallback( HSteamPipe hSteamPipe )
{
    PRINT_DEBUG("%s %i\n", __FUNCTION__, hSteamPipe);
    std::queue<struct cb_data> *q = NULL;
    Steam_Client *steam_client = get_steam_client();
    if (!steam_client->steam_pipes.count(hSteamPipe)) {
        return;
    }

    if (steam_client->steam_pipes[hSteamPipe] == Steam_Pipe::SERVER) {
        q = &server_cb;
    } else if (steam_client->steam_pipes[hSteamPipe] == Steam_Pipe::CLIENT) {
        q = &client_cb;
    } else {
        return;
    }

    if (!q->empty()) q->pop();
}

/// Return the call result for the specified call on the specified pipe.  You really should
/// only call this in a handler for SteamAPICallCompleted_t callback.
STEAMAPI_API steam_bool S_CALLTYPE SteamAPI_ManualDispatch_GetAPICallResult( HSteamPipe hSteamPipe, SteamAPICall_t hSteamAPICall, void *pCallback, int cubCallback, int iCallbackExpected, bool *pbFailed )
{
    PRINT_DEBUG("SteamAPI_ManualDispatch_GetAPICallResult %i %llu %i %i\n", hSteamPipe, hSteamAPICall, cubCallback, iCallbackExpected);
    Steam_Client *steam_client = get_steam_client();
    if (!steam_client->steam_pipes.count(hSteamPipe)) {
        return false;
    }

    if (steam_client->steam_pipes[hSteamPipe] == Steam_Pipe::SERVER) {
        return get_steam_client()->steam_gameserver_utils->GetAPICallResult(hSteamAPICall, pCallback, cubCallback, iCallbackExpected, pbFailed);
    } else if (steam_client->steam_pipes[hSteamPipe] == Steam_Pipe::CLIENT) {
        return get_steam_client()->steam_utils->GetAPICallResult(hSteamAPICall, pCallback, cubCallback, iCallbackExpected, pbFailed);
    } else {
        return false;
    }
}

HSteamUser flat_hsteamuser()
{
    return SteamAPI_GetHSteamUser();
}

HSteamPipe flat_hsteampipe()
{
    return SteamAPI_GetHSteamPipe();
}

HSteamUser flat_gs_hsteamuser()
{
    return SteamGameServer_GetHSteamUser();
}

HSteamPipe flat_gs_hsteampipe()
{
    return SteamGameServer_GetHSteamPipe();
}

//VR stuff
STEAMAPI_API void *VR_Init(int *error, int type)
{
    if (error) *error = 108; //HmdError_Init_HmdNotFound
    return NULL;
}

STEAMAPI_API void *VR_GetGenericInterface( const char *pchInterfaceVersion, int *peError )
{
    return NULL;
}

STEAMAPI_API const char *VR_GetStringForHmdError( int error )
{
    return "";
}

STEAMAPI_API steam_bool VR_IsHmdPresent()
{
    return false;
}

STEAMAPI_API void VR_Shutdown()
{

}

STEAMAPI_API steam_bool SteamAPI_RestartApp( uint32 appid )
{
    PRINT_DEBUG("SteamAPI_RestartApp %u\n", appid);
    return SteamAPI_RestartAppIfNecessary(appid);
}

//OLD steam_c stuff
/*

ISteamApps_BIsCybercafe
ISteamApps_BIsLowViolence
ISteamApps_BIsSubscribed
ISteamApps_BIsSubscribedApp
ISteamApps_GetAvailableGameLanguages
ISteamApps_GetCurrentGameLanguage
ISteamClient_SetLocalIPBinding
ISteamGameServer_BLoggedOn
ISteamGameServer_BSecure
ISteamGameServer_BUpdateUserData
ISteamGameServer_CreateUnauthenticatedUserConnection
ISteamGameServer_GetSteamID
ISteamGameServer_SendUserConnectAndAuthenticate
ISteamGameServer_SendUserDisconnect
ISteamGameServer_SetGameType
ISteamGameServer_SetServerType
ISteamGameServer_UpdateSpectatorPort
ISteamGameServer_UpdateStatus
STEAMAPI_API bool ISteamMasterServerUpdater_AddMasterServer
ISteamMasterServerUpdater_ClearAllKeyValues
ISteamMasterServerUpdater_ForceHeartbeat
ISteamMasterServerUpdater_GetMasterServerAddress
ISteamMasterServerUpdater_GetNextOutgoingPacket
ISteamMasterServerUpdater_GetNumMasterServers
ISteamMasterServerUpdater_HandleIncomingPacket
ISteamMasterServerUpdater_NotifyShutdown
ISteamMasterServerUpdater_RemoveMasterServer
ISteamMasterServerUpdater_SetActive
ISteamMasterServerUpdater_SetBasicServerData
ISteamMasterServerUpdater_SetHeartbeatInterval
ISteamMasterServerUpdater_SetKeyValue
ISteamMasterServerUpdater_WasRestartRequested
ISteamUser_BLoggedOn
ISteamUser_InitiateGameConnection
ISteamUser_TerminateGameConnection
ISteamUtils_GetAppID

SteamContentServer
SteamContentServerUtils
SteamContentServer_Init
SteamContentServer_RunCallbacks
SteamContentServer_Shutdown


SteamGameServer_BSecure
SteamGameServer_GetHSteamPipe
SteamGameServer_GetHSteamUser
SteamGameServer_GetIPCCallCount
SteamGameServer_GetSteamID
SteamGameServer_Init
SteamGameServer_InitSafe
SteamGameServer_RunCallbacks
SteamGameServer_Shutdown

SteamMasterServerUpdater

*/


STEAMCLIENT_API steam_bool Steam_BGetCallback( HSteamPipe hSteamPipe, CallbackMsg_t *pCallbackMsg )
{
    PRINT_DEBUG("%s %i\n", __FUNCTION__, hSteamPipe);
    SteamAPI_ManualDispatch_Init();
    Steam_Client *steam_client = get_steam_client();
    steam_client->RunCallbacks(true, true);
    return SteamAPI_ManualDispatch_GetNextCallback( hSteamPipe, pCallbackMsg );
}

STEAMCLIENT_API void Steam_FreeLastCallback( HSteamPipe hSteamPipe )
{
    PRINT_DEBUG("%s %i\n", __FUNCTION__, hSteamPipe);
    SteamAPI_ManualDispatch_FreeLastCallback( hSteamPipe );
}

STEAMCLIENT_API steam_bool Steam_GetAPICallResult( HSteamPipe hSteamPipe, SteamAPICall_t hSteamAPICall, void* pCallback, int cubCallback, int iCallbackExpected, bool* pbFailed )
{
    PRINT_DEBUG("Steam_GetAPICallResult %i %llu %i %i\n", hSteamPipe, hSteamAPICall, cubCallback, iCallbackExpected);
    return SteamAPI_ManualDispatch_GetAPICallResult(hSteamPipe, hSteamAPICall, pCallback, cubCallback, iCallbackExpected, pbFailed);
}

STEAMCLIENT_API void *CreateInterface( const char *pName, int *pReturnCode )
{
    PRINT_DEBUG("CreateInterface %s %p\n", pName, pReturnCode);
    return create_client_interface(pName);
}

STEAMCLIENT_API void Breakpad_SteamMiniDumpInit( uint32 a, const char *b, const char *c )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
}

STEAMCLIENT_API void Breakpad_SteamSetAppID( uint32 unAppID )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
}

STEAMCLIENT_API void Breakpad_SteamSetSteamID( uint64 ulSteamID )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
}

STEAMCLIENT_API void Breakpad_SteamWriteMiniDumpSetComment( const char *pchMsg )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
}

STEAMCLIENT_API void Breakpad_SteamWriteMiniDumpUsingExceptionInfoWithBuildId( int a, int b )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
}

STEAMCLIENT_API bool Steam_BConnected( HSteamUser hUser, HSteamPipe hSteamPipe )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
    return true;
}

STEAMCLIENT_API bool Steam_BLoggedOn( HSteamUser hUser, HSteamPipe hSteamPipe )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
    return true;
}

STEAMCLIENT_API bool Steam_BReleaseSteamPipe( HSteamPipe hSteamPipe )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
    return false;
}

STEAMCLIENT_API HSteamUser Steam_ConnectToGlobalUser( HSteamPipe hSteamPipe )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
    return 0;
}

STEAMCLIENT_API HSteamUser Steam_CreateGlobalUser( HSteamPipe *phSteamPipe )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
    return 0;
}

STEAMCLIENT_API HSteamUser Steam_CreateLocalUser( HSteamPipe *phSteamPipe, EAccountType eAccountType )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
    return 0;
}

STEAMCLIENT_API HSteamPipe Steam_CreateSteamPipe()
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
    return 0;
}

STEAMCLIENT_API bool Steam_GSBLoggedOn( void *phSteamHandle )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
    return false;
}

STEAMCLIENT_API bool Steam_GSBSecure( void *phSteamHandle)
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
    return false;
}

STEAMCLIENT_API bool Steam_GSGetSteam2GetEncryptionKeyToSendToNewClient( void *phSteamHandle, void *pvEncryptionKey, uint32 *pcbEncryptionKey, uint32 cbMaxEncryptionKey )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
    return false;
}

STEAMCLIENT_API uint64 Steam_GSGetSteamID()
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
    return 0;
}

STEAMCLIENT_API void Steam_GSLogOff( void *phSteamHandle )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
}

STEAMCLIENT_API void Steam_GSLogOn( void *phSteamHandle )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
}

STEAMCLIENT_API bool Steam_GSRemoveUserConnect( void *phSteamHandle, uint32 unUserID )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
    return false;
}

STEAMCLIENT_API bool Steam_GSSendSteam2UserConnect( void *phSteamHandle, uint32 unUserID, const void *pvRawKey, uint32 unKeyLen, uint32 unIPPublic, uint16 usPort, const void *pvCookie, uint32 cubCookie )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
    return false;
}

STEAMCLIENT_API bool Steam_GSSendSteam3UserConnect( void *phSteamHandle, uint64 steamID, uint32 unIPPublic, const void *pvCookie, uint32 cubCookie )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
    return false;
}

STEAMCLIENT_API bool Steam_GSSendUserDisconnect( void *phSteamHandle, uint64 ulSteamID, uint32 unUserID )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
    return false;
}

STEAMCLIENT_API bool Steam_GSSendUserStatusResponse( void *phSteamHandle, uint64 ulSteamID, int nSecondsConnected, int nSecondsSinceLast )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
    return false;
}

STEAMCLIENT_API bool Steam_GSSetServerType( void *phSteamHandle, int32 nAppIdServed, uint32 unServerFlags, uint32 unGameIP, uint32 unGamePort, const char *pchGameDir, const char *pchVersion )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
    return false;
}

STEAMCLIENT_API void Steam_GSSetSpawnCount( void *phSteamHandle, uint32 ucSpawn )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
}

STEAMCLIENT_API bool Steam_GSUpdateStatus( void *phSteamHandle, int cPlayers, int cPlayersMax, int cBotPlayers, const char *pchServerName, const char *pchMapName )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
    return false;
}

STEAMCLIENT_API void* Steam_GetGSHandle( HSteamUser hUser, HSteamPipe hSteamPipe )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
    return NULL;
}

STEAMCLIENT_API int Steam_InitiateGameConnection( HSteamUser hUser, HSteamPipe hSteamPipe, void *pBlob, int cbMaxBlob, uint64 steamID, int nGameAppID, uint32 unIPServer, uint16 usPortServer, bool bSecure )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
    return 0;
}

STEAMCLIENT_API void Steam_LogOff( HSteamUser hUser, HSteamPipe hSteamPipe )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
}

STEAMCLIENT_API void Steam_LogOn( HSteamUser hUser, HSteamPipe hSteamPipe, uint64 ulSteamID )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
}

STEAMCLIENT_API void Steam_ReleaseThreadLocalMemory(bool thread_exit)
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
}

STEAMCLIENT_API void Steam_ReleaseUser( HSteamPipe hSteamPipe, HSteamUser hUser )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
}

STEAMCLIENT_API void Steam_SetLocalIPBinding( uint32 unIP, uint16 usLocalPort )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
}

STEAMCLIENT_API void Steam_TerminateGameConnection( HSteamUser hUser, HSteamPipe hSteamPipe, uint32 unIPServer, uint16 usPortServer )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
}
