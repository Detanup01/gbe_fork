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
#include "dll/dll.h"
#include "dll/settings_parser.h"
#include "dll/client_known_interfaces.h"


static char old_client[128] = STEAMCLIENT_INTERFACE_VERSION; //"SteamClient017";
static char old_gameserver_stats[128] = STEAMGAMESERVERSTATS_INTERFACE_VERSION; //"SteamGameServerStats001";
static char old_gameserver[128] = STEAMGAMESERVER_INTERFACE_VERSION; //"SteamGameServer012";
static char old_matchmaking_servers[128] = STEAMMATCHMAKINGSERVERS_INTERFACE_VERSION; //"SteamMatchMakingServers002";
static char old_matchmaking[128] = STEAMMATCHMAKING_INTERFACE_VERSION; //"SteamMatchMaking009";
static char old_user[128] = STEAMUSER_INTERFACE_VERSION; //"SteamUser018";
static char old_friends[128] = STEAMFRIENDS_INTERFACE_VERSION; //"SteamFriends015";
static char old_utils[128] = STEAMUTILS_INTERFACE_VERSION; //"SteamUtils007";
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

static ISteamUser *old_user_instance{};
static ISteamFriends *old_friends_interface{};
static ISteamUtils *old_utils_interface{};
static ISteamMatchmaking *old_matchmaking_instance{};
static ISteamUserStats *old_userstats_instance{};
static ISteamApps *old_apps_instance{};
static ISteamMatchmakingServers *old_matchmakingservers_instance{};
static ISteamNetworking *old_networking_instance{};
static ISteamRemoteStorage *old_remotestorage_instance{};
static ISteamScreenshots *old_screenshots_instance{};
static ISteamHTTP *old_http_instance{};
static ISteamController *old_controller_instance{};
static ISteamUGC *old_ugc_instance{};
static ISteamAppList *old_applist_instance{};
static ISteamMusic *old_music_instance{};
static ISteamMusicRemote *old_musicremote_instance{};
static ISteamHTMLSurface *old_htmlsurface_instance{};
static ISteamInventory *old_inventory_instance{};
static ISteamVideo *old_video_instance{};
static ISteamParentalSettings *old_parental_instance{};
static ISteamUnifiedMessages *old_unified_instance{};
static ISteamGameServer *old_gameserver_instance{};
static ISteamUtils *old_gamserver_utils_instance{};
static ISteamNetworking  *old_gamserver_networking_instance{};
static ISteamGameServerStats *old_gamserver_stats_instance{};
static ISteamHTTP *old_gamserver_http_instance{};
static ISteamInventory *old_gamserver_inventory_instance{};
static ISteamUGC *old_gamserver_ugc_instance{};
static ISteamApps *old_gamserver_apps_instance{};
static ISteamMasterServerUpdater *old_gamserver_masterupdater_instance{};



static void load_old_steam_interfaces()
{
    static bool loaded = false;
    
    std::lock_guard lck(global_mutex);
    if (loaded) return;
    loaded = true;

    const auto &old_itfs_map = settings_old_interfaces();

#define SET_OLD_ITF(sitf, var) do {                                     \
    auto itr = old_itfs_map.find(sitf);                                 \
    if (old_itfs_map.end() != itr && itr->second.size()) {              \
        memset(var, 0, sizeof(var));                                    \
        itr->second.copy(var, sizeof(var) - 1);                         \
        PRINT_DEBUG("set old interface: '%s'", itr->second.c_str());    \
    }                                                                   \
} while (0)

    SET_OLD_ITF(SettingsItf::CLIENT, old_client);
    SET_OLD_ITF(SettingsItf::GAMESERVER_STATS, old_gameserver_stats);
    SET_OLD_ITF(SettingsItf::GAMESERVER, old_gameserver);
    SET_OLD_ITF(SettingsItf::MATCHMAKING_SERVERS, old_matchmaking_servers);
    SET_OLD_ITF(SettingsItf::MATCHMAKING, old_matchmaking);
    SET_OLD_ITF(SettingsItf::USER, old_user);
    SET_OLD_ITF(SettingsItf::FRIENDS, old_friends);
    SET_OLD_ITF(SettingsItf::UTILS, old_utils);
    SET_OLD_ITF(SettingsItf::USER_STATS, old_userstats);
    SET_OLD_ITF(SettingsItf::APPS, old_apps);
    SET_OLD_ITF(SettingsItf::NETWORKING, old_networking);
    SET_OLD_ITF(SettingsItf::REMOTE_STORAGE, old_remote_storage_interface);
    SET_OLD_ITF(SettingsItf::SCREENSHOTS, old_screenshots);
    SET_OLD_ITF(SettingsItf::HTTP, old_http);
    SET_OLD_ITF(SettingsItf::UNIFIED_MESSAGES, old_unified_messages);
    SET_OLD_ITF(SettingsItf::CONTROLLER, old_controller);
    SET_OLD_ITF(SettingsItf::UGC, old_ugc_interface);
    SET_OLD_ITF(SettingsItf::APPLIST, old_applist);
    SET_OLD_ITF(SettingsItf::MUSIC, old_music);
    SET_OLD_ITF(SettingsItf::MUSIC_REMOTE, old_music_remote);
    SET_OLD_ITF(SettingsItf::HTML_SURFACE, old_html_surface);
    SET_OLD_ITF(SettingsItf::INVENTORY, old_inventory);
    SET_OLD_ITF(SettingsItf::VIDEO, old_video);
    SET_OLD_ITF(SettingsItf::MASTERSERVER_UPDATER, old_masterserver_updater);

#undef SET_OLD_ITF

    PRINT_DEBUG("Old interfaces:");
    PRINT_DEBUG("  client: %s", old_client);
    PRINT_DEBUG("  gameserver stats: %s", old_gameserver_stats);
    PRINT_DEBUG("  gameserver: %s", old_gameserver);
    PRINT_DEBUG("  matchmaking servers: %s", old_matchmaking_servers);
    PRINT_DEBUG("  matchmaking: %s", old_matchmaking);
    PRINT_DEBUG("  user: %s", old_user);
    PRINT_DEBUG("  friends: %s", old_friends);
    PRINT_DEBUG("  utils: %s", old_utils);
    PRINT_DEBUG("  userstats: %s", old_userstats);
    PRINT_DEBUG("  apps: %s", old_apps);
    PRINT_DEBUG("  networking: %s", old_networking);
    PRINT_DEBUG("  remote storage: %s", old_remote_storage_interface);
    PRINT_DEBUG("  screenshots: %s", old_screenshots);
    PRINT_DEBUG("  http: %s", old_http);
    PRINT_DEBUG("  unified messages: %s", old_unified_messages);
    PRINT_DEBUG("  controller %s", old_controller);
    PRINT_DEBUG("  ugc: %s", old_ugc_interface);
    PRINT_DEBUG("  applist: %s", old_applist);
    PRINT_DEBUG("  music: %s", old_music);
    PRINT_DEBUG("  music remote: %s", old_music_remote);
    PRINT_DEBUG("  html surface: %s", old_html_surface);
    PRINT_DEBUG("  inventory: %s", old_inventory);
    PRINT_DEBUG("  video: %s", old_video);
    PRINT_DEBUG("  masterserver updater: %s", old_masterserver_updater);
    
    reset_LastError();
}

//steam_api_internal.h
STEAMAPI_API HSteamUser SteamAPI_GetHSteamUser()
{
    PRINT_DEBUG_ENTRY();
    if (!get_steam_client()->IsUserLogIn()) return 0;
    return CLIENT_HSTEAMUSER;
}


// declare "g_pSteamClientGameServer" as an export for API library, then actually define it
#if !defined(STEAMCLIENT_DLL) // api
STEAMAPI_API ISteamClient *g_pSteamClientGameServer;
#endif
ISteamClient *g_pSteamClientGameServer{};

static Steam_Client *steamclient_instance{};
Steam_Client *get_steam_client()
{
    if (!steamclient_instance) {
        std::lock_guard<std::recursive_mutex> lock(global_mutex);
        // if we win the thread arbitration for the first time, this will still be null
        if (!steamclient_instance) {
            load_old_steam_interfaces();
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
        steamclient_instance = nullptr;
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

static bool steamclient_has_ipv6_functions_flag{};
bool steamclient_has_ipv6_functions()
{
    return get_steam_client()->gameserver_has_ipv6_functions || steamclient_has_ipv6_functions_flag;
}

static void *create_client_interface(const char *ver)
{
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    Steam_Client *client_ptr = get_steam_client();
    if (strstr(ver, "SteamClient") == ver) {
        if (strcmp(ver, "SteamClient006") == 0) {
            return static_cast<ISteamClient006 *>(client_ptr);
        } else if (strcmp(ver, "SteamClient007") == 0) {
            return static_cast<ISteamClient007 *>(client_ptr);
        } else if (strcmp(ver, "SteamClient008") == 0) {
            return static_cast<ISteamClient008 *>(client_ptr);
        } else if (strcmp(ver, "SteamClient009") == 0) {
            return static_cast<ISteamClient009 *>(client_ptr);
        } else if (strcmp(ver, "SteamClient010") == 0) {
            return static_cast<ISteamClient010 *>(client_ptr);
        } else if (strcmp(ver, "SteamClient011") == 0) {
            return static_cast<ISteamClient011 *>(client_ptr);
        } else if (strcmp(ver, "SteamClient012") == 0) {
            return static_cast<ISteamClient012 *>(client_ptr);
        } else if (strcmp(ver, "SteamClient013") == 0) {
            return static_cast<ISteamClient013 *>(client_ptr);
        } else if (strcmp(ver, "SteamClient014") == 0) {
            return static_cast<ISteamClient014 *>(client_ptr);
        } else if (strcmp(ver, "SteamClient015") == 0) {
            return static_cast<ISteamClient015 *>(client_ptr);
        } else if (strcmp(ver, "SteamClient016") == 0) {
            return static_cast<ISteamClient016 *>(client_ptr);
        } else if (strcmp(ver, "SteamClient017") == 0) {
            return static_cast<ISteamClient017 *>(client_ptr);
        } else if (strcmp(ver, "SteamClient018") == 0) {
            return static_cast<ISteamClient018 *>(client_ptr);
        } else if (strcmp(ver, "SteamClient019") == 0) {
            return static_cast<ISteamClient019 *>(client_ptr);
        }
        
        steamclient_has_ipv6_functions_flag = true;
        if (strcmp(ver, "SteamClient020") == 0) {
            return static_cast<ISteamClient020 *>(client_ptr);
        } else if (strcmp(ver, STEAMCLIENT_INTERFACE_VERSION) == 0) {
            return static_cast<ISteamClient *>(client_ptr);
        }
    }
    
    client_ptr->report_missing_impl_and_exit(ver, EMU_FUNC_NAME);
}

STEAMAPI_API void * S_CALLTYPE SteamInternal_CreateInterface( const char *ver )
{
    PRINT_DEBUG("%s", ver);
    if (!get_steam_client()->IsUserLogIn() && !get_steam_client()->IsServerInit()) return NULL;

    return create_client_interface(ver);
}

static uintp global_counter{};
struct ContextInitData {
    void (*pFn)(void* pCtx) = nullptr;
    uintp counter{};
    CSteamAPIContext ctx{};
};

STEAMAPI_API void * S_CALLTYPE SteamInternal_ContextInit( void *pContextInitData )
{
    //PRINT_DEBUG_ENTRY();
    struct ContextInitData *contextInitData = (struct ContextInitData *)pContextInitData;
    if (contextInitData->counter != global_counter) {
        PRINT_DEBUG("initializing");
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
    PRINT_DEBUG("%s", pszInternalCheckInterfaceVersions);
    if (SteamAPI_Init()) {
        return ESteamAPIInitResult::k_ESteamAPIInitResult_OK;
    }

    if (pOutErrMsg) {
        constexpr const static char err[] = "SteamInitEx failed";
        memcpy(*pOutErrMsg, err, sizeof(err));
    }
    return ESteamAPIInitResult::k_ESteamAPIInitResult_FailedGeneric;
}

STEAMAPI_API ESteamAPIInitResult S_CALLTYPE SteamAPI_InitFlat( SteamErrMsg *pOutErrMsg )
{
    PRINT_DEBUG_ENTRY();
    if (SteamAPI_Init()) {
        return ESteamAPIInitResult::k_ESteamAPIInitResult_OK;
    }

    if (pOutErrMsg) {
        constexpr const static char err[] = "SteamAPI_InitFlat failed";
        memcpy(*pOutErrMsg, err, sizeof(err));
    }
    return ESteamAPIInitResult::k_ESteamAPIInitResult_FailedGeneric;
}

// SteamAPI_Init must be called before using any other API functions. If it fails, an
// error message will be output to the debugger (or stderr) with further information.
static HSteamPipe user_steam_pipe = 0;
STEAMAPI_API steam_bool S_CALLTYPE SteamAPI_Init()
{
    PRINT_DEBUG_ENTRY();
    if (user_steam_pipe) return true;
    
    // call this first since it loads old interfaces
    Steam_Client* client = get_steam_client();

#ifdef EMU_EXPERIMENTAL_BUILD
    crack_SteamAPI_Init();
#endif

    user_steam_pipe = client->CreateSteamPipe();
    client->ConnectToGlobalUser(user_steam_pipe);
    global_counter++;
    return true;
}

//TODO: not sure if this is the right signature for this function.
STEAMAPI_API steam_bool S_CALLTYPE SteamAPI_InitAnonymousUser()
{
    PRINT_DEBUG_ENTRY();
    return SteamAPI_Init();
}

// SteamAPI_Shutdown should be called during process shutdown if possible.
STEAMAPI_API void S_CALLTYPE SteamAPI_Shutdown()
{
    PRINT_DEBUG_ENTRY();
    
    // if nothing is initialized just return, see the note in SteamGameServer_Shutdown()
    // guard against sloppy games programming
    // don't shutdown if this isn't a client app (maybe gameserver?)
    if (!user_steam_pipe &&
        !(steamclient_instance && steamclient_instance->IsUserLogIn())) {
        PRINT_DEBUG("[WARNING] app is trying to shutdown as client, but it isn't initialzed (is this a gameserver?)");
        return;
    }

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

// ------------------------
// --- older notes/docs ---
// note that this usually means we're dealing with CEG!
// ------------------------

// Detects if your executable was launched through the Steam client, and restarts your game through 
// the client if necessary. The Steam client will be started if it is not running.
//
// Returns: true if your executable was NOT launched through the Steam client. This function will
//          then start your application through the client. Your current process should exit.
//
//          false if your executable was started through the Steam client or a steam_appid.txt file
//          is present in your game's directory (for development). Your current process should continue.
//
// NOTE: This function should be used only if you are using CEG or not using Steam's DRM. Once applied
//       to your executable, Steam's DRM will handle restarting through Steam if necessary.

STEAMAPI_API steam_bool S_CALLTYPE SteamAPI_RestartAppIfNecessary( uint32 unOwnAppID )
{
    PRINT_DEBUG("%u", unOwnAppID);
    
    // call this first since it loads old interfaces
    Steam_Client* client = get_steam_client();
    
#ifdef EMU_EXPERIMENTAL_BUILD
    crack_SteamAPI_RestartAppIfNecessary(unOwnAppID);
#endif
    client->setAppID(unOwnAppID);
    return false;
}

// Many Steam API functions allocate a small amount of thread-local memory for parameter storage.
// SteamAPI_ReleaseCurrentThreadMemory() will free API memory associated with the calling thread.
// This function is also called automatically by SteamAPI_RunCallbacks(), so a single-threaded
// program never needs to explicitly call this function.
STEAMAPI_API void S_CALLTYPE SteamAPI_ReleaseCurrentThreadMemory()
{
    PRINT_DEBUG_TODO();
}

// crash dump recording functions
STEAMAPI_API void S_CALLTYPE SteamAPI_WriteMiniDump( uint32 uStructuredExceptionCode, void* pvExceptionInfo, uint32 uBuildID )
{
    PRINT_DEBUG_TODO();
    PRINT_DEBUG("  app is writing a crash dump! [XXXXXXXXXXXXXXXXXXXXXXXXXXX]");
}

STEAMAPI_API void S_CALLTYPE SteamAPI_SetMiniDumpComment( const char *pchMsg )
{
    PRINT_DEBUG_TODO();
    PRINT_DEBUG(  "%s", pchMsg);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------//
//    steam callback and call-result helpers
//
//    The following macros and classes are used to register your application for
//    callbacks and call-results, which are delivered in a predictable manner.
//
//    STEAM_CALLBACK macros are meant for use inside of a C++ class definition.
//    They map a Steam notification callback directly to a class member function
//    which is automatically prototyped as "void func( callback_type *pParam )".
//
//    CCallResult is used with specific Steam APIs that return "result handles".
//    The handle can be passed to a CCallResult object's Set function, along with
//    an object pointer and member-function pointer. The member function will
//    be executed once the results of the Steam API call are available.
//
//    CCallback and CCallbackManual classes can be used instead of STEAM_CALLBACK
//    macros if you require finer control over registration and unregistration.
//
//    Callbacks and call-results are queued automatically and are only
//    delivered/executed when your application calls SteamAPI_RunCallbacks().
//----------------------------------------------------------------------------------------------------------------------------------------------------------//

// SteamAPI_RunCallbacks is safe to call from multiple threads simultaneously,
// but if you choose to do this, callback code could be executed on any thread.
// One alternative is to call SteamAPI_RunCallbacks from the main thread only,
// and call SteamAPI_ReleaseCurrentThreadMemory regularly on other threads.
STEAMAPI_API void S_CALLTYPE SteamAPI_RunCallbacks()
{
    // PRINT_DEBUG_ENTRY();
    get_steam_client()->RunCallbacks(true, false);
    //std::this_thread::sleep_for(std::chrono::microseconds(1)); //fixes resident evil revelations lagging. (Seems to work fine without this right now, commenting out)
}


// Declares a callback member function plus a helper member variable which
// registers the callback on object creation and unregisters on destruction.
// The optional fourth 'var' param exists only for backwards-compatibility
// and can be ignored.
//#define STEAM_CALLBACK( thisclass, func, .../*callback_type, [deprecated] var*/ ) \
//    _STEAM_CALLBACK_SELECT( ( __VA_ARGS__, 4, 3 ), ( /**/, thisclass, func, __VA_ARGS__ ) )

// Declares a callback function and a named CCallbackManual variable which
// has Register and Unregister functions instead of automatic registration.
//#define STEAM_CALLBACK_MANUAL( thisclass, func, callback_type, var )    \
//    CCallbackManual< thisclass, callback_type > var; void func( callback_type *pParam )


// Internal functions used by the utility CCallback objects to receive callbacks
STEAMAPI_API void S_CALLTYPE SteamAPI_RegisterCallback( class CCallbackBase *pCallback, int iCallback )
{
    PRINT_DEBUG("%p %u funct:%u", pCallback, iCallback, pCallback->GetICallback());
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    get_steam_client()->RegisterCallback(pCallback, iCallback);
}

STEAMAPI_API void S_CALLTYPE SteamAPI_UnregisterCallback( class CCallbackBase *pCallback )
{
    PRINT_DEBUG("%p", pCallback);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!steamclient_instance) return;
    get_steam_client()->UnregisterCallback(pCallback);
}

// Internal functions used by the utility CCallResult objects to receive async call results
STEAMAPI_API void S_CALLTYPE SteamAPI_RegisterCallResult( class CCallbackBase *pCallback, SteamAPICall_t hAPICall )
{
    PRINT_DEBUG("%llu %p", hAPICall, pCallback);
    if (!hAPICall)
        return;

    get_steam_client()->RegisterCallResult(pCallback, hAPICall);
}

STEAMAPI_API void S_CALLTYPE SteamAPI_UnregisterCallResult( class CCallbackBase *pCallback, SteamAPICall_t hAPICall )
{
    PRINT_DEBUG_ENTRY();
    if (!hAPICall)
        return;

    if (!steamclient_instance) return;
    get_steam_client()->UnregisterCallResult(pCallback, hAPICall);
}

STEAMAPI_API void *S_CALLTYPE SteamInternal_FindOrCreateUserInterface( HSteamUser hSteamUser, const char *pszVersion )
{
    PRINT_DEBUG("%i %s", hSteamUser, pszVersion);
    return get_steam_client()->GetISteamGenericInterface(hSteamUser, SteamAPI_GetHSteamPipe(), pszVersion);
}

STEAMAPI_API void *S_CALLTYPE SteamInternal_FindOrCreateGameServerInterface( HSteamUser hSteamUser, const char *pszVersion )
{
    PRINT_DEBUG("%i %s", hSteamUser, pszVersion);
    return get_steam_client()->GetISteamGenericInterface(hSteamUser, SteamGameServer_GetHSteamPipe(), pszVersion);
}

//----------------------------------------------------------------------------------------------------------------------------------------------------------//
//    steamclient.dll private wrapper functions
//
//    The following functions are part of abstracting API access to the steamclient.dll, but should only be used in very specific cases
//----------------------------------------------------------------------------------------------------------------------------------------------------------//

// SteamAPI_IsSteamRunning() returns true if Steam is currently running
STEAMAPI_API steam_bool S_CALLTYPE SteamAPI_IsSteamRunning()
{
    PRINT_DEBUG_ENTRY();
    return true;
}

// Pumps out all the steam messages, calling registered callbacks.
// NOT THREADSAFE - do not call from multiple threads simultaneously.
STEAMAPI_API void Steam_RunCallbacks( HSteamPipe hSteamPipe, bool bGameServerCallbacks )
{
    // PRINT_DEBUG_ENTRY();

    SteamAPI_RunCallbacks();

    if (bGameServerCallbacks)
        SteamGameServer_RunCallbacks();
}

// register the callback funcs to use to interact with the steam dll
STEAMAPI_API void Steam_RegisterInterfaceFuncs( void *hModule )
{
    PRINT_DEBUG_TODO();
}

// returns the HSteamUser of the last user to dispatch a callback
STEAMAPI_API HSteamUser Steam_GetHSteamUserCurrent()
{
    PRINT_DEBUG_ENTRY();
    //TODO
    return SteamAPI_GetHSteamUser();
}

// returns the filename path of the current running Steam process, used if you need to load an explicit steam dll by name.
// DEPRECATED - implementation is Windows only, and the path returned is a UTF-8 string which must be converted to UTF-16 for use with Win32 APIs
STEAMAPI_API const char *SteamAPI_GetSteamInstallPath()
{
    PRINT_DEBUG_ENTRY();
    static char steam_folder[4096]{};
    std::string path(get_env_variable("SteamPath"));
    if (path.empty()) {
        path = Local_Storage::get_program_path();
    }

    auto count = path.copy(steam_folder, sizeof(steam_folder) - 1);
    steam_folder[count] = '\0';

    PRINT_DEBUG("returned path '%s'", steam_folder);
    return steam_folder;
}

// returns the pipe we are communicating to Steam with
STEAMAPI_API HSteamPipe SteamAPI_GetHSteamPipe()
{
    PRINT_DEBUG_ENTRY();
    return user_steam_pipe;
}

// sets whether or not Steam_RunCallbacks() should do a try {} catch (...) {} around calls to issuing callbacks
STEAMAPI_API void SteamAPI_SetTryCatchCallbacks( bool bTryCatchCallbacks )
{
    PRINT_DEBUG_TODO();
}

// backwards compat export, passes through to SteamAPI_ variants
STEAMAPI_API HSteamPipe GetHSteamPipe()
{
    PRINT_DEBUG_ENTRY();
    return SteamAPI_GetHSteamPipe();
}

STEAMAPI_API HSteamUser GetHSteamUser()
{
    PRINT_DEBUG_ENTRY();
    return SteamAPI_GetHSteamUser();
}


// exists only for backwards compat with code written against older SDKs
STEAMAPI_API steam_bool S_CALLTYPE SteamAPI_InitSafe()
{
    PRINT_DEBUG_ENTRY();
    return SteamAPI_Init();
}

STEAMAPI_API ISteamClient *SteamClient()
{
    PRINT_DEBUG("old");
    // call this first since it loads old interfaces
    Steam_Client* client = get_steam_client();
    if (!client->IsUserLogIn()) return NULL;
    return (ISteamClient *)SteamInternal_CreateInterface(old_client);
}

#define CACHE_OLDSTEAM_INSTANCE(variable, get_func) { if (variable) return variable; else return variable = (get_func); }

STEAMAPI_API ISteamUser *SteamUser()
{
    PRINT_DEBUG("old");
    CACHE_OLDSTEAM_INSTANCE(old_user_instance, get_steam_client_old()->GetISteamUser(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_user))
}

STEAMAPI_API ISteamFriends *SteamFriends()
{
    PRINT_DEBUG("old");
    CACHE_OLDSTEAM_INSTANCE(old_friends_interface, get_steam_client_old()->GetISteamFriends(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_friends ))
}

STEAMAPI_API ISteamUtils *SteamUtils()
{
    PRINT_DEBUG("old");
    CACHE_OLDSTEAM_INSTANCE(old_utils_interface, get_steam_client_old()->GetISteamUtils(SteamAPI_GetHSteamPipe(), old_utils))
}

STEAMAPI_API ISteamMatchmaking *SteamMatchmaking()
{
    PRINT_DEBUG("old");
    CACHE_OLDSTEAM_INSTANCE(old_matchmaking_instance, get_steam_client_old()->GetISteamMatchmaking(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_matchmaking))
}

STEAMAPI_API ISteamUserStats *SteamUserStats()
{
    PRINT_DEBUG("old");
    CACHE_OLDSTEAM_INSTANCE(old_userstats_instance, get_steam_client_old()->GetISteamUserStats(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_userstats))
}

STEAMAPI_API ISteamApps *SteamApps()
{
    PRINT_DEBUG("old");
    CACHE_OLDSTEAM_INSTANCE(old_apps_instance, get_steam_client_old()->GetISteamApps(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_apps))
}

STEAMAPI_API ISteamMatchmakingServers *SteamMatchmakingServers()
{
    PRINT_DEBUG("old");
    CACHE_OLDSTEAM_INSTANCE(old_matchmakingservers_instance, get_steam_client_old()->GetISteamMatchmakingServers(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_matchmaking_servers))
}

STEAMAPI_API ISteamNetworking *SteamNetworking()
{
    PRINT_DEBUG("old");
    CACHE_OLDSTEAM_INSTANCE(old_networking_instance, get_steam_client_old()->GetISteamNetworking(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_networking))
}

STEAMAPI_API ISteamRemoteStorage *SteamRemoteStorage()
{
    PRINT_DEBUG("old");
    CACHE_OLDSTEAM_INSTANCE(old_remotestorage_instance, get_steam_client_old()->GetISteamRemoteStorage(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_remote_storage_interface))
}

STEAMAPI_API ISteamScreenshots *SteamScreenshots()
{
    PRINT_DEBUG("old");
    CACHE_OLDSTEAM_INSTANCE(old_screenshots_instance, get_steam_client_old()->GetISteamScreenshots(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_screenshots))
}

STEAMAPI_API ISteamHTTP *SteamHTTP()
{
    PRINT_DEBUG("old");
    CACHE_OLDSTEAM_INSTANCE(old_http_instance, get_steam_client_old()->GetISteamHTTP(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_http))
}

STEAMAPI_API ISteamController *SteamController()
{
    PRINT_DEBUG("old");
    CACHE_OLDSTEAM_INSTANCE(old_controller_instance, get_steam_client_old()->GetISteamController(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_controller))
}

STEAMAPI_API ISteamUGC *SteamUGC()
{
    PRINT_DEBUG("old");
    CACHE_OLDSTEAM_INSTANCE(old_ugc_instance, get_steam_client_old()->GetISteamUGC(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_ugc_interface ))
}

STEAMAPI_API ISteamAppList *SteamAppList()
{
    PRINT_DEBUG("old");
    CACHE_OLDSTEAM_INSTANCE(old_applist_instance, get_steam_client_old()->GetISteamAppList(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_applist))
}

STEAMAPI_API ISteamMusic *SteamMusic()
{
    PRINT_DEBUG("old");
    CACHE_OLDSTEAM_INSTANCE(old_music_instance, get_steam_client_old()->GetISteamMusic(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_music))
}

STEAMAPI_API ISteamMusicRemote *SteamMusicRemote()
{
    PRINT_DEBUG("old");
    CACHE_OLDSTEAM_INSTANCE(old_musicremote_instance, get_steam_client_old()->GetISteamMusicRemote(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_music_remote))
}

STEAMAPI_API ISteamHTMLSurface *SteamHTMLSurface()
{
    PRINT_DEBUG("old");
    CACHE_OLDSTEAM_INSTANCE(old_htmlsurface_instance, get_steam_client_old()->GetISteamHTMLSurface(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_html_surface))
}

STEAMAPI_API ISteamInventory *SteamInventory()
{
    PRINT_DEBUG("old");
    CACHE_OLDSTEAM_INSTANCE(old_inventory_instance, get_steam_client_old()->GetISteamInventory(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_inventory))
}

STEAMAPI_API ISteamVideo *SteamVideo()
{
    PRINT_DEBUG("old");
    CACHE_OLDSTEAM_INSTANCE(old_video_instance, get_steam_client_old()->GetISteamVideo(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_video))
}

STEAMAPI_API ISteamParentalSettings *SteamParentalSettings()
{
    PRINT_DEBUG("old");
    CACHE_OLDSTEAM_INSTANCE(old_parental_instance, get_steam_client_old()->GetISteamParentalSettings(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), ""))
}

STEAMAPI_API ISteamUnifiedMessages *SteamUnifiedMessages()
{
    PRINT_DEBUG("old");
    CACHE_OLDSTEAM_INSTANCE(old_unified_instance, get_steam_client_old()->GetISteamUnifiedMessages(SteamAPI_GetHSteamUser(), SteamAPI_GetHSteamPipe(), old_unified_messages))
}

STEAMAPI_API ISteamGameServer *SteamGameServer()
{
    PRINT_DEBUG("old");
    CACHE_OLDSTEAM_INSTANCE(old_gameserver_instance, get_steam_clientserver_old()->GetISteamGameServer(SteamGameServer_GetHSteamUser(), SteamGameServer_GetHSteamPipe(), old_gameserver ))
}

STEAMAPI_API ISteamUtils *SteamGameServerUtils()
{
    PRINT_DEBUG("old");
    CACHE_OLDSTEAM_INSTANCE(old_gamserver_utils_instance, get_steam_clientserver_old()->GetISteamUtils(SteamGameServer_GetHSteamPipe(), old_utils ))
}

STEAMAPI_API ISteamNetworking *SteamGameServerNetworking()
{
    PRINT_DEBUG("old");
    CACHE_OLDSTEAM_INSTANCE(old_gamserver_networking_instance, get_steam_clientserver_old()->GetISteamNetworking(SteamGameServer_GetHSteamUser(), SteamGameServer_GetHSteamPipe(), old_networking ))
}

STEAMAPI_API ISteamGameServerStats *SteamGameServerStats()
{
    PRINT_DEBUG("old");
    CACHE_OLDSTEAM_INSTANCE(old_gamserver_stats_instance, get_steam_clientserver_old()->GetISteamGameServerStats(SteamGameServer_GetHSteamUser(), SteamGameServer_GetHSteamPipe(), old_gameserver_stats ))
}

STEAMAPI_API ISteamHTTP *SteamGameServerHTTP()
{
    PRINT_DEBUG("old");
    CACHE_OLDSTEAM_INSTANCE(old_gamserver_http_instance, get_steam_clientserver_old()->GetISteamHTTP(SteamGameServer_GetHSteamUser(), SteamGameServer_GetHSteamPipe(), old_http ))
}

STEAMAPI_API ISteamInventory *SteamGameServerInventory()
{
    PRINT_DEBUG("old");
    CACHE_OLDSTEAM_INSTANCE(old_gamserver_inventory_instance, get_steam_clientserver_old()->GetISteamInventory(SteamGameServer_GetHSteamUser(), SteamGameServer_GetHSteamPipe(), old_inventory ))
}

STEAMAPI_API ISteamUGC *SteamGameServerUGC()
{
    PRINT_DEBUG("old");
    CACHE_OLDSTEAM_INSTANCE(old_gamserver_ugc_instance, get_steam_clientserver_old()->GetISteamUGC(SteamGameServer_GetHSteamUser(), SteamGameServer_GetHSteamPipe(), old_ugc_interface ))
}

STEAMAPI_API ISteamApps *SteamGameServerApps()
{
    PRINT_DEBUG("old");
    CACHE_OLDSTEAM_INSTANCE(old_gamserver_apps_instance, get_steam_clientserver_old()->GetISteamApps(SteamGameServer_GetHSteamUser(), SteamGameServer_GetHSteamPipe(), old_apps ))
}

STEAMAPI_API ISteamMasterServerUpdater *SteamMasterServerUpdater()
{
    PRINT_DEBUG("old");
    CACHE_OLDSTEAM_INSTANCE(old_gamserver_masterupdater_instance, get_steam_clientserver_old()->GetISteamMasterServerUpdater(SteamGameServer_GetHSteamUser(), SteamGameServer_GetHSteamPipe(), old_masterserver_updater))
}

#undef CACHE_OLDSTEAM_INSTANCE


//Gameserver stuff

STEAMAPI_API void * S_CALLTYPE SteamGameServerInternal_CreateInterface( const char *ver )
{
    PRINT_DEBUG("%s", ver);
    return SteamInternal_CreateInterface(ver);
}

static HSteamPipe server_steam_pipe = 0;
STEAMAPI_API HSteamPipe S_CALLTYPE SteamGameServer_GetHSteamPipe()
{
    PRINT_DEBUG_ENTRY();
    return server_steam_pipe;
}

STEAMAPI_API HSteamUser S_CALLTYPE SteamGameServer_GetHSteamUser()
{
    PRINT_DEBUG_ENTRY();
    if (!get_steam_client()->IsServerInit()) return 0;
    return SERVER_HSTEAMUSER;
}

//See: SteamGameServer_Init
//STEAMAPI_API steam_bool S_CALLTYPE SteamGameServer_InitSafe(uint32 unIP, uint16 usSteamPort, uint16 usGamePort, uint16 usQueryPort, EServerMode eServerMode, const char *pchVersionString )
STEAMAPI_API steam_bool S_CALLTYPE SteamGameServer_InitSafe( uint32 unIP, uint16 usSteamPort, uint16 usGamePort, uint16 unknown, EServerMode eServerMode, void *unknown1, void *unknown2, void *unknown3 )
{
    PRINT_DEBUG_ENTRY();
    const char *pchVersionString{};
    EServerMode serverMode{};
    uint16 usQueryPort{};
    // call this first since it loads old interfaces
    Steam_Client* client = get_steam_client();
    bool logon_anon = false;
    if (strcmp(old_gameserver, "SteamGameServer010") == 0 || strstr(old_gameserver, "SteamGameServer00") == old_gameserver) {
        PRINT_DEBUG("Old game server init safe");
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
        client->steam_gameserver->LogOnAnonymous();
    }

    return ret;
}

STEAMAPI_API ISteamClient *SteamGameServerClient();

STEAMAPI_API steam_bool S_CALLTYPE SteamInternal_GameServer_Init( uint32 unIP, uint16 usPort, uint16 usGamePort, uint16 usQueryPort, EServerMode eServerMode, const char *pchVersionString )
{
    PRINT_DEBUG("%X %hu %hu %hu %u %s", unIP, usPort, usGamePort, usQueryPort, eServerMode, pchVersionString);
    // call this first since it loads old interfaces
    Steam_Client* client = get_steam_client();
    if (!server_steam_pipe) {
        client->CreateLocalUser(&server_steam_pipe, k_EAccountTypeGameServer);
        ++global_counter;
        //g_pSteamClientGameServer is only used in pre 1.37 (where the interface versions are not provided by the game)
        g_pSteamClientGameServer = SteamGameServerClient();
    }

    uint32 unFlags = 0;
    if (eServerMode == eServerModeAuthenticationAndSecure) unFlags = k_unServerFlagSecure;
    return client->steam_gameserver->InitGameServer(unIP, usGamePort, usQueryPort, unFlags, 0, pchVersionString);
}

STEAMAPI_API ESteamAPIInitResult S_CALLTYPE SteamInternal_GameServer_Init_V2( uint32 unIP, uint16 usGamePort, uint16 usQueryPort, EServerMode eServerMode, const char *pchVersionString, const char *pszInternalCheckInterfaceVersions, SteamErrMsg *pOutErrMsg )
{
    PRINT_DEBUG("%u %hu %hu %u %s %s", unIP, usGamePort, usQueryPort, eServerMode, pchVersionString, pszInternalCheckInterfaceVersions);
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
    PRINT_DEBUG_ENTRY();
    const char *pchVersionString{};
    EServerMode serverMode{};
    uint16 usQueryPort{};
    // call this first since it loads old interfaces
    Steam_Client* client = get_steam_client();
    bool logon_anon = false;
    if (strcmp(old_gameserver, "SteamGameServer010") == 0 || strstr(old_gameserver, "SteamGameServer00") == old_gameserver) {
        PRINT_DEBUG("Old game server init");
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
        client->steam_gameserver->LogOnAnonymous();
    }

    return ret;
}

STEAMAPI_API void SteamGameServer_Shutdown()
{
    PRINT_DEBUG_ENTRY();

    // appid 35140 despite being a regular game (not a server) will still call this function
    // its steam_api.dll (old api) will check first if "g_pSteamClientGameServer" is null, and if so it just returns
    // hence avoiding sloppy programming
    
    // if nothing is initialized just return
    // don't shutdown if this isn't a gameserver app (maybe client?)
    if (!server_steam_pipe &&
        !g_pSteamClientGameServer && // old steam_api.dll checks for this
        !(steamclient_instance && steamclient_instance->IsServerInit())) {
        PRINT_DEBUG("[WARNING] app is trying to shutdown as gameserver, but it isn't initialzed (is this a client?)");
        return;
    }

    get_steam_client()->serverShutdown();
    get_steam_client()->BReleaseSteamPipe(server_steam_pipe);
    get_steam_client()->BShutdownIfAllPipesClosed();

    server_steam_pipe = 0;
    --global_counter;
    g_pSteamClientGameServer = NULL; // old steam_api.dll sets this to null when SteamGameServer_Shutdown is called
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
    // PRINT_DEBUG_ENTRY();
    get_steam_client()->RunCallbacks(false, true);
}

STEAMAPI_API steam_bool SteamGameServer_BSecure()
{
    PRINT_DEBUG_ENTRY();
    return get_steam_client()->steam_gameserver->BSecure();
}

STEAMAPI_API uint64 SteamGameServer_GetSteamID()
{
    PRINT_DEBUG_ENTRY();
    return get_steam_client()->steam_gameserver->GetSteamID().ConvertToUint64();
}

STEAMAPI_API ISteamClient *SteamGameServerClient()
{
    PRINT_DEBUG("old");
    if (!get_steam_clientserver_old()->IsServerInit()) return NULL;
    return reinterpret_cast<ISteamClient *>(SteamInternal_CreateInterface(old_client)); 
}

STEAMAPI_API uint32 SteamGameServer_GetIPCCallCount()
{
    return get_steam_client()->GetIPCCallCount();
}


STEAMAPI_API void S_CALLTYPE SteamAPI_UseBreakpadCrashHandler( char const *pchVersion, char const *pchDate, char const *pchTime, bool bFullMemoryDumps, void *pvContext, PFNPreMinidumpCallback m_pfnPreMinidumpCallback )
{
    PRINT_DEBUG_TODO();
}

STEAMAPI_API void S_CALLTYPE SteamAPI_SetBreakpadAppID( uint32 unAppID )
{
    PRINT_DEBUG_TODO();
}

struct cb_data {
    int cb_id{};
    std::vector<char> result{};
};
static std::queue<struct cb_data> client_cb{};
static std::queue<struct cb_data> server_cb{};

static void cb_add_queue_server(std::vector<char> result, int callback)
{
    PRINT_DEBUG("adding callback=%i, size=%zu", callback, result.size());
    struct cb_data cb{};
    cb.cb_id = callback;
    cb.result = result;
    server_cb.push(cb);
}

static void cb_add_queue_client(std::vector<char> result, int callback)
{
    PRINT_DEBUG("adding callback=%i, m_iCallback=%i", callback, ((SteamAPICallCompleted_t *)&result[0])->m_iCallback);
    struct cb_data cb{};
    cb.cb_id = callback;
    cb.result = result;
    client_cb.push(cb);
}

/// Inform the API that you wish to use manual event dispatch.  This must be called after SteamAPI_Init, but before
/// you use any of the other manual dispatch functions below.
STEAMAPI_API void S_CALLTYPE SteamAPI_ManualDispatch_Init()
{
    static std::atomic_bool manual_dispatch_called = false;
    bool not_yet = false;
    if (manual_dispatch_called.compare_exchange_weak(not_yet, true)) {
        PRINT_DEBUG_ENTRY();
        Steam_Client *steam_client = get_steam_client();
        steam_client->callback_results_server->setCbAll(&cb_add_queue_server);
        steam_client->callback_results_client->setCbAll(&cb_add_queue_client);
    }
}

/// Perform certain periodic actions that need to be performed.
STEAMAPI_API void S_CALLTYPE SteamAPI_ManualDispatch_RunFrame( HSteamPipe hSteamPipe )
{
    PRINT_DEBUG("%i", hSteamPipe);
    Steam_Client *steam_client = get_steam_client();
    auto it = steam_client->steam_pipes.find(hSteamPipe);
    if (steam_client->steam_pipes.end() == it) {
        return;
    }

    if (it->second == Steam_Pipe::SERVER) {
        steam_client->RunCallbacks(false, true);
    } else if (it->second == Steam_Pipe::CLIENT) {
        steam_client->RunCallbacks(true, false);
    }
}

/// Fetch the next pending callback on the given pipe, if any.  If a callback is available, true is returned
/// and the structure is populated.  In this case, you MUST call SteamAPI_ManualDispatch_FreeLastCallback
/// (after dispatching the callback) before calling SteamAPI_ManualDispatch_GetNextCallback again.
STEAMAPI_API steam_bool S_CALLTYPE SteamAPI_ManualDispatch_GetNextCallback( HSteamPipe hSteamPipe, CallbackMsg_t *pCallbackMsg )
{
    PRINT_DEBUG("%i %p", hSteamPipe, pCallbackMsg);
    Steam_Client *steam_client = get_steam_client();
    if (!steam_client->steamclient_server_inited) {
        while(!server_cb.empty()) server_cb.pop();
    }

    auto it = steam_client->steam_pipes.find(hSteamPipe);
    if (steam_client->steam_pipes.end() == it) {
        PRINT_DEBUG("error invalid hSteamPipe");
        return false;
    }

    std::queue<struct cb_data> *q = NULL;
    HSteamUser m_hSteamUser = 0;
    if (it->second == Steam_Pipe::SERVER) {
        q = &server_cb;
        m_hSteamUser = SERVER_HSTEAMUSER;
    } else if (it->second == Steam_Pipe::CLIENT) {
        q = &client_cb;
        m_hSteamUser = CLIENT_HSTEAMUSER;
    } else {
        PRINT_DEBUG("error invalid steam pipe type");
        return false;
    }

    if (q->empty()) {
        //PRINT_DEBUG("error queue is empty");
        return false;
    }

    if (pCallbackMsg) {
        pCallbackMsg->m_hSteamUser = m_hSteamUser;
        pCallbackMsg->m_iCallback = q->front().cb_id;
        pCallbackMsg->m_pubParam = (uint8 *)&(q->front().result[0]);
        pCallbackMsg->m_cubParam = static_cast<unsigned long>(q->front().result.size());
        PRINT_DEBUG("cb number %i", q->front().cb_id);
        return true;
    }

    PRINT_DEBUG("error nullptr pCallbackMsg");
    return false;
}

/// You must call this after dispatching the callback, if SteamAPI_ManualDispatch_GetNextCallback returns true.
STEAMAPI_API void S_CALLTYPE SteamAPI_ManualDispatch_FreeLastCallback( HSteamPipe hSteamPipe )
{
    PRINT_DEBUG("%i", hSteamPipe);
    std::queue<struct cb_data> *q = NULL;
    Steam_Client *steam_client = get_steam_client();
    auto it = steam_client->steam_pipes.find(hSteamPipe);
    if (steam_client->steam_pipes.end() == it) {
        return;
    }

    if (it->second == Steam_Pipe::SERVER) {
        q = &server_cb;
    } else if (it->second == Steam_Pipe::CLIENT) {
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
    PRINT_DEBUG("%i %llu %i %i", hSteamPipe, hSteamAPICall, cubCallback, iCallbackExpected);
    Steam_Client *steam_client = get_steam_client();
    if (!steam_client->steam_pipes.count(hSteamPipe)) {
        return false;
    }

    if (steam_client->steam_pipes[hSteamPipe] == Steam_Pipe::SERVER) {
        return steam_client->steam_gameserver_utils->GetAPICallResult(hSteamAPICall, pCallback, cubCallback, iCallbackExpected, pbFailed);
    } else if (steam_client->steam_pipes[hSteamPipe] == Steam_Pipe::CLIENT) {
        return steam_client->steam_utils->GetAPICallResult(hSteamAPICall, pCallback, cubCallback, iCallbackExpected, pbFailed);
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
    PRINT_DEBUG_TODO();
    if (error) *error = 108; //HmdError_Init_HmdNotFound
    return NULL;
}

STEAMAPI_API void *VR_GetGenericInterface( const char *pchInterfaceVersion, int *peError )
{
    PRINT_DEBUG_TODO();
    return NULL;
}

STEAMAPI_API const char *VR_GetStringForHmdError( int error )
{
    PRINT_DEBUG_TODO();
    return "";
}

STEAMAPI_API steam_bool VR_IsHmdPresent()
{
    PRINT_DEBUG_TODO();
    return false;
}

STEAMAPI_API void VR_Shutdown()
{
    PRINT_DEBUG_TODO();
}

STEAMAPI_API steam_bool SteamAPI_RestartApp( uint32 appid )
{
    PRINT_DEBUG("%u", appid);
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
    PRINT_DEBUG("%i", hSteamPipe);
    SteamAPI_ManualDispatch_Init();
    Steam_Client *steam_client = get_steam_client();
    steam_client->RunCallbacks(true, true);
    return SteamAPI_ManualDispatch_GetNextCallback( hSteamPipe, pCallbackMsg );
}

STEAMCLIENT_API void Steam_FreeLastCallback( HSteamPipe hSteamPipe )
{
    //PRINT_DEBUG("%i", hSteamPipe);
    SteamAPI_ManualDispatch_FreeLastCallback( hSteamPipe );
}

STEAMCLIENT_API steam_bool Steam_GetAPICallResult( HSteamPipe hSteamPipe, SteamAPICall_t hSteamAPICall, void* pCallback, int cubCallback, int iCallbackExpected, bool* pbFailed )
{
    //PRINT_DEBUG("%i %llu %i %i", hSteamPipe, hSteamAPICall, cubCallback, iCallbackExpected);
    return SteamAPI_ManualDispatch_GetAPICallResult(hSteamPipe, hSteamAPICall, pCallback, cubCallback, iCallbackExpected, pbFailed);
}

STEAMCLIENT_API void* CreateInterface( const char *pName, int *pReturnCode )
{
    PRINT_DEBUG("%s %p", pName, pReturnCode);
    auto ptr = create_client_interface(pName);
    if (ptr) {
        if (pReturnCode) *pReturnCode = 1;
    } else {
        if (pReturnCode) *pReturnCode = 0;
    }
    return ptr;
}

STEAMCLIENT_API void Breakpad_SteamMiniDumpInit( uint32 a, const char *b, const char *c )
{
    PRINT_DEBUG_TODO();
}

STEAMCLIENT_API void Breakpad_SteamSendMiniDump( void *a, uint32 b )
{
    PRINT_DEBUG_TODO();
    PRINT_DEBUG("  app is sending a crash dump! [XXXXXXXXXXXXXXXXXXXXXXXXXXX]");
}

STEAMCLIENT_API void Breakpad_SteamSetAppID( uint32 unAppID )
{
    PRINT_DEBUG_TODO();
}

STEAMCLIENT_API void Breakpad_SteamSetSteamID( uint64 ulSteamID )
{
    PRINT_DEBUG_TODO();
}

STEAMCLIENT_API void Breakpad_SteamWriteMiniDumpSetComment( const char *pchMsg )
{
    PRINT_DEBUG("'%s'", pchMsg);
    PRINT_DEBUG("  app is writing a crash dump comment! [XXXXXXXXXXXXXXXXXXXXXXXXXXX]");
}

STEAMCLIENT_API void Breakpad_SteamWriteMiniDumpUsingExceptionInfoWithBuildId( int a, int b )
{
    PRINT_DEBUG("%i -- %i", a, b);
    PRINT_DEBUG("  app is writing a crash dump! [XXXXXXXXXXXXXXXXXXXXXXXXXXX]");
}

STEAMCLIENT_API steam_bool Steam_BConnected( HSteamUser hUser, HSteamPipe hSteamPipe )
{
    PRINT_DEBUG_ENTRY();
    return true;
}

STEAMCLIENT_API steam_bool Steam_BLoggedOn( HSteamUser hUser, HSteamPipe hSteamPipe )
{
    PRINT_DEBUG("%i %i", hUser, hSteamPipe);
    Steam_Client *steam_client = get_steam_client();

    auto pipe_it = steam_client->steam_pipes.find(hSteamPipe);
    if (steam_client->steam_pipes.end() == pipe_it) {
        return false;
    }

    class Settings *settings_tmp{};
    if (pipe_it->second == Steam_Pipe::SERVER) {
        settings_tmp = steam_client->settings_server;
    } else if (pipe_it->second == Steam_Pipe::CLIENT) {
        settings_tmp = steam_client->settings_client;
    } else {
        return false;
    }

    return !settings_tmp->is_offline();
}

STEAMCLIENT_API steam_bool Steam_BReleaseSteamPipe( HSteamPipe hSteamPipe )
{
    PRINT_DEBUG_TODO();
    return false;
}

STEAMCLIENT_API HSteamUser Steam_ConnectToGlobalUser( HSteamPipe hSteamPipe )
{
    PRINT_DEBUG_TODO();
    return 0;
}

STEAMCLIENT_API HSteamUser Steam_CreateGlobalUser( HSteamPipe *phSteamPipe )
{
    PRINT_DEBUG_TODO();
    return 0;
}

STEAMCLIENT_API HSteamUser Steam_CreateLocalUser( HSteamPipe *phSteamPipe, EAccountType eAccountType )
{
    PRINT_DEBUG_TODO();
    return 0;
}

STEAMCLIENT_API HSteamPipe Steam_CreateSteamPipe()
{
    PRINT_DEBUG_TODO();
    return 0;
}

STEAMCLIENT_API steam_bool Steam_GSBLoggedOn( void *phSteamHandle )
{
    PRINT_DEBUG_TODO();
    return false;
}

STEAMCLIENT_API steam_bool Steam_GSBSecure( void *phSteamHandle)
{
    PRINT_DEBUG_TODO();
    return false;
}

// latest steamclient
STEAMCLIENT_API steam_bool Steam_GSGetSteam2GetEncryptionKeyToSendToNewClient(void* phSteamHandle)
//STEAMCLIENT_API steam_bool Steam_GSGetSteam2GetEncryptionKeyToSendToNewClient( void *phSteamHandle, void *pvEncryptionKey, uint32 *pcbEncryptionKey, uint32 cbMaxEncryptionKey )
{
    PRINT_DEBUG_TODO();
    return false;
}

STEAMCLIENT_API uint64 Steam_GSGetSteamID()
{
    PRINT_DEBUG_TODO();
    return 0;
}

STEAMCLIENT_API void Steam_GSLogOff( void *phSteamHandle )
{
    PRINT_DEBUG_TODO();
}

STEAMCLIENT_API void Steam_GSLogOn( void *phSteamHandle )
{
    PRINT_DEBUG_TODO();
}

STEAMCLIENT_API steam_bool Steam_GSRemoveUserConnect(void* phSteamHandle)
//STEAMCLIENT_API steam_bool Steam_GSRemoveUserConnect( void *phSteamHandle, uint32 unUserID )
{
    PRINT_DEBUG_TODO();
    return false;
}

STEAMCLIENT_API steam_bool Steam_GSSendSteam2UserConnect(void* phSteamHandle)
//STEAMCLIENT_API steam_bool Steam_GSSendSteam2UserConnect( void *phSteamHandle, uint32 unUserID, const void *pvRawKey, uint32 unKeyLen, uint32 unIPPublic, uint16 usPort, const void *pvCookie, uint32 cubCookie )
{
    PRINT_DEBUG_TODO();
    return false;
}

STEAMCLIENT_API steam_bool Steam_GSSendSteam3UserConnect(void* phSteamHandle)
//STEAMCLIENT_API steam_bool Steam_GSSendSteam3UserConnect( void *phSteamHandle, uint64 steamID, uint32 unIPPublic, const void *pvCookie, uint32 cubCookie )
{
    PRINT_DEBUG_TODO();
    return false;
}

STEAMCLIENT_API steam_bool Steam_GSSendUserDisconnect(void* phSteamHandle)
//STEAMCLIENT_API steam_bool Steam_GSSendUserDisconnect( void *phSteamHandle, uint64 ulSteamID, uint32 unUserID )
{
    PRINT_DEBUG_TODO();
    return false;
}

STEAMCLIENT_API steam_bool Steam_GSSendUserStatusResponse()
//STEAMCLIENT_API steam_bool Steam_GSSendUserStatusResponse( void *phSteamHandle, uint64 ulSteamID, int nSecondsConnected, int nSecondsSinceLast )
{
    PRINT_DEBUG_TODO();
    return false;
}

// Changed later, idk when.
STEAMCLIENT_API steam_bool Steam_GSSetServerType(void* phSteamHandle, int32 a2)
//STEAMCLIENT_API steam_bool Steam_GSSetServerType( void *phSteamHandle, int32 nAppIdServed, uint32 unServerFlags, uint32 unGameIP, uint32 unGamePort, const char *pchGameDir, const char *pchVersion )
{
    PRINT_DEBUG_TODO();
    return false;
}

STEAMCLIENT_API int64 Steam_GSSetSpawnCount(void* phSteamHandle)
{
    PRINT_DEBUG_TODO();
    return 0;
}

/*
STEAMCLIENT_API void Steam_GSSetSpawnCount( void *phSteamHandle, uint32 ucSpawn )
{
    PRINT_DEBUG_TODO();
}
*/
STEAMCLIENT_API steam_bool Steam_GSUpdateStatus(void* phSteamHandle)
//STEAMCLIENT_API steam_bool Steam_GSUpdateStatus( void *phSteamHandle, int cPlayers, int cPlayersMax, int cBotPlayers, const char *pchServerName, const char *pchMapName )
{
    PRINT_DEBUG_TODO();
    return false;
}

STEAMCLIENT_API void* Steam_GetGSHandle( HSteamUser hUser, HSteamPipe hSteamPipe )
{
    PRINT_DEBUG_TODO();
    return nullptr;
}

STEAMCLIENT_API int Steam_InitiateGameConnection( HSteamUser hUser, HSteamPipe hSteamPipe, void *pBlob, int cbMaxBlob, uint64 steamID, int nGameAppID, uint32 unIPServer, uint16 usPortServer, bool bSecure )
{
    PRINT_DEBUG_TODO();
    return 0;
}

// https://github.com/ValveSoftware/Proton/blob/962bbc4e74dde0643a6edab7c845bc628601f23f/lsteamclient/steamclient_main.c#L579-L586
STEAMCLIENT_API steam_bool Steam_IsKnownInterface( const char *pchVersion )
{
    PRINT_DEBUG("'%s'", pchVersion);
    
    // real client doesn't validate if the arg was null
    if (!pchVersion) return false;

    bool found = client_known_interfaces.count(pchVersion);
    
    PRINT_DEBUG("  result=%i", (int)found);
    if (!found) {
        get_steam_client()->report_missing_impl(pchVersion, EMU_FUNC_NAME);
    }

    return found;
}

STEAMCLIENT_API void Steam_LogOff( HSteamUser hUser, HSteamPipe hSteamPipe )
{
    PRINT_DEBUG_TODO();
}

STEAMCLIENT_API void Steam_LogOn( HSteamUser hUser, HSteamPipe hSteamPipe, uint64 ulSteamID )
{
    PRINT_DEBUG_TODO();
}

// https://github.com/ValveSoftware/Proton/blob/962bbc4e74dde0643a6edab7c845bc628601f23f/lsteamclient/steamclient_main.c#L588-L594
STEAMCLIENT_API void Steam_NotifyMissingInterface(HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("XXXXXXXXXXXXX '%s' %i", pchVersion, hSteamPipe);
    get_steam_client()->report_missing_impl(pchVersion, EMU_FUNC_NAME);
}

STEAMCLIENT_API void Steam_ReleaseThreadLocalMemory(bool thread_exit)
{
    PRINT_DEBUG_TODO();
}

STEAMCLIENT_API void Steam_ReleaseUser( HSteamPipe hSteamPipe, HSteamUser hUser )
{
    PRINT_DEBUG_TODO();
}

STEAMCLIENT_API void Steam_SetLocalIPBinding( uint32 unIP, uint16 usLocalPort )
{
    PRINT_DEBUG_TODO();
}

STEAMCLIENT_API void Steam_TerminateGameConnection( HSteamUser hUser, HSteamPipe hSteamPipe, uint32 unIPServer, uint16 usPortServer )
{
    PRINT_DEBUG_TODO();
}
