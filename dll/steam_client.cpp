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

#include "dll/steam_client.h"
#include "dll/settings_parser.h"


void Steam_Client::background_thread_proc()
{
    auto now_ms = (unsigned long long)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();

    // if our time exceeds last run time of callbacks and it wasn't processing already
    const auto runcallbacks_timeout_ms = last_cb_run + max_stall_ms.count();
    if (!cb_run_active && (now_ms >= runcallbacks_timeout_ms)) {
        std::lock_guard lock(global_mutex);

        PRINT_DEBUG("run @@@@@@@@@@@@@@@@@@@@@@@@@@@");
        last_cb_run = now_ms; // update the time counter just to avoid overlap
        network->Run(); // networking must run first since it receives messages used by each run_callback()
        run_every_runcb->run(); // call each run_callback()
    }
}

Steam_Client::Steam_Client()
{
    PRINT_DEBUG("start ----------");
    uint32 appid = create_localstorage_settings(&settings_client, &settings_server, &local_storage);
    local_storage->update_save_filenames(Local_Storage::remote_storage_folder);

    background_thread = new common_helpers::KillableWorker(
        [this](void *){background_thread_proc(); return false;},
        std::chrono::duration_cast<std::chrono::milliseconds>(initial_delay),
        std::chrono::duration_cast<std::chrono::milliseconds>(max_stall_ms)
    );
    network = new Networking(settings_server->get_local_steam_id(), appid, settings_server->get_port(), &(settings_server->custom_broadcasts), settings_server->disable_networking);

    run_every_runcb = new RunEveryRunCB();

    PRINT_DEBUG(
        "init: id: %llu server id: %llu, appid: %u, port: %u",
        settings_client->get_local_steam_id().ConvertToUint64(), settings_server->get_local_steam_id().ConvertToUint64(), appid, settings_server->get_port()
    );

    if (appid) {
        auto appid_str = std::to_string(appid);
        set_env_variable("SteamAppId", appid_str);
        set_env_variable("SteamGameId", appid_str);
        
        if (!settings_client->disable_steamoverlaygameid_env_var) {
            set_env_variable("SteamOverlayGameId", appid_str);
        }
    }

    {
        const char *user_name = settings_client->get_local_name();
        if (user_name) {
            set_env_variable("SteamAppUser", user_name);
            set_env_variable("SteamUser", user_name);
        }
    }

    set_env_variable("SteamClientLaunch", "1");
    set_env_variable("SteamEnv", "1");
    
    {
        std::string steam_path(get_env_variable("SteamPath"));
        if (steam_path.empty()) {
            set_env_variable("SteamPath", get_full_program_path());
        }
    }

    // client
    PRINT_DEBUG("init client");
    callback_results_client = new SteamCallResults();
    callbacks_client = new SteamCallBacks(callback_results_client);
    steam_overlay = new Steam_Overlay(settings_client, local_storage, callback_results_client, callbacks_client, run_every_runcb, network);

    steam_user = new Steam_User(settings_client, local_storage, network, callback_results_client, callbacks_client);
    steam_friends = new Steam_Friends(settings_client, local_storage, network, callback_results_client, callbacks_client, run_every_runcb, steam_overlay);
    steam_utils = new Steam_Utils(settings_client, callback_results_client, callbacks_client, steam_overlay);
    
    ugc_bridge = new Ugc_Remote_Storage_Bridge(settings_client);

    steam_matchmaking = new Steam_Matchmaking(settings_client, local_storage, network, callback_results_client, callbacks_client, run_every_runcb);
    steam_matchmaking_servers = new Steam_Matchmaking_Servers(settings_client, local_storage, network);
    steam_user_stats = new Steam_User_Stats(settings_client, network, local_storage, callback_results_client, callbacks_client, run_every_runcb, steam_overlay);
    steam_apps = new Steam_Apps(settings_client, callback_results_client, callbacks_client);
    steam_networking = new Steam_Networking(settings_client, network, callbacks_client, run_every_runcb);
    steam_remote_storage = new Steam_Remote_Storage(settings_client, ugc_bridge, local_storage, callback_results_client, callbacks_client, run_every_runcb);
    steam_screenshots = new Steam_Screenshots(local_storage, callbacks_client);
    steam_http = new Steam_HTTP(settings_client, network, callback_results_client, callbacks_client);
    steam_controller = new Steam_Controller(settings_client, callback_results_client, callbacks_client, run_every_runcb);
    steam_ugc = new Steam_UGC(settings_client, ugc_bridge, local_storage, callback_results_client, callbacks_client);
    steam_applist = new Steam_Applist();
    steam_music = new Steam_Music(callbacks_client);
    steam_musicremote = new Steam_MusicRemote();
    steam_HTMLsurface = new Steam_HTMLsurface(settings_client, network, callback_results_client, callbacks_client);
    steam_inventory = new Steam_Inventory(settings_client, callback_results_client, callbacks_client, run_every_runcb, local_storage);
    steam_video = new Steam_Video();
    steam_parental = new Steam_Parental();
    steam_networking_sockets = new Steam_Networking_Sockets(settings_client, network, callback_results_client, callbacks_client, run_every_runcb, NULL);
    steam_networking_sockets_serialized = new Steam_Networking_Sockets_Serialized(settings_client, network, callback_results_client, callbacks_client, run_every_runcb);
    steam_networking_messages = new Steam_Networking_Messages(settings_client, network, callback_results_client, callbacks_client, run_every_runcb);
    steam_game_coordinator = new Steam_Game_Coordinator(settings_client, network, callback_results_client, callbacks_client, run_every_runcb);
    steam_networking_utils = new Steam_Networking_Utils(settings_client, network, callback_results_client, callbacks_client, run_every_runcb);
    steam_unified_messages = new Steam_Unified_Messages(settings_client, network, callback_results_client, callbacks_client, run_every_runcb);
    steam_game_search = new Steam_Game_Search(settings_client, network, callback_results_client, callbacks_client, run_every_runcb);
    steam_parties = new Steam_Parties(settings_client, network, callback_results_client, callbacks_client, run_every_runcb);
    steam_remoteplay = new Steam_RemotePlay(settings_client, network, callback_results_client, callbacks_client, run_every_runcb);
    steam_tv = new Steam_TV(settings_client, network, callback_results_client, callbacks_client, run_every_runcb);
    steam_gamestats = new Steam_GameStats(settings_client, network, callback_results_client, callbacks_client, run_every_runcb);
    steam_timeline = new Steam_Timeline(settings_client, network, callback_results_client, callbacks_client, run_every_runcb);
    steam_app_disable_update = new Steam_App_Disable_Update(settings_client, network, callback_results_client, callbacks_client, run_every_runcb);
    steam_billing = new Steam_Billing(settings_client, network, callback_results_client, callbacks_client, run_every_runcb);

    // server
    PRINT_DEBUG("init gameserver");
    callback_results_server = new SteamCallResults();
    callbacks_server = new SteamCallBacks(callback_results_server);

    steam_gameserver = new Steam_GameServer(settings_server, network, callbacks_server);
    steam_gameserver_utils = new Steam_Utils(settings_server, callback_results_server, callbacks_server, steam_overlay);
    steam_gameserverstats = new Steam_GameServerStats(settings_server, network, callback_results_server, callbacks_server, run_every_runcb);
    steam_gameserver_networking = new Steam_Networking(settings_server, network, callbacks_server, run_every_runcb);
    steam_gameserver_http = new Steam_HTTP(settings_server, network, callback_results_server, callbacks_server);
    steam_gameserver_inventory = new Steam_Inventory(settings_server, callback_results_server, callbacks_server, run_every_runcb, local_storage);
    steam_gameserver_ugc = new Steam_UGC(settings_server, ugc_bridge, local_storage, callback_results_server, callbacks_server);
    steam_gameserver_apps = new Steam_Apps(settings_server, callback_results_server, callbacks_server);
    steam_gameserver_networking_sockets = new Steam_Networking_Sockets(settings_server, network, callback_results_server, callbacks_server, run_every_runcb, steam_networking_sockets->get_shared_between_client_server());
    steam_gameserver_networking_sockets_serialized = new Steam_Networking_Sockets_Serialized(settings_server, network, callback_results_server, callbacks_server, run_every_runcb);
    steam_gameserver_networking_messages = new Steam_Networking_Messages(settings_server, network, callback_results_server, callbacks_server, run_every_runcb);
    steam_gameserver_game_coordinator = new Steam_Game_Coordinator(settings_server, network, callback_results_server, callbacks_server, run_every_runcb);
    steam_masterserver_updater = new Steam_Masterserver_Updater(settings_server, network, callback_results_server, callbacks_server, run_every_runcb, steam_gameserver);
    steam_gameserver_gamestats = new Steam_GameStats(settings_server, network, callback_results_server, callbacks_server, run_every_runcb);

    PRINT_DEBUG("init AppTicket");
    steam_app_ticket = new Steam_AppTicket(settings_client);

    gameserver_has_ipv6_functions = false;

    last_cb_run = 0;
    PRINT_DEBUG("end *********");

    reset_LastError();
}

Steam_Client::~Steam_Client()
{
    #define DEL_INST(_obj_ins) do if (_obj_ins) { delete _obj_ins; _obj_ins = nullptr; } while(0)

    DEL_INST(background_thread);

    DEL_INST(steam_gameserver_utils);
    DEL_INST(steam_gameserverstats);
    DEL_INST(steam_gameserver_networking);
    DEL_INST(steam_gameserver_http);
    DEL_INST(steam_gameserver_inventory);
    DEL_INST(steam_gameserver_ugc);
    DEL_INST(steam_gameserver_apps);
    DEL_INST(steam_gameserver_networking_sockets);
    DEL_INST(steam_gameserver_networking_sockets_serialized);
    DEL_INST(steam_gameserver_networking_messages);
    DEL_INST(steam_gameserver_game_coordinator);
    DEL_INST(steam_masterserver_updater);
    DEL_INST(steam_gameserver);
    DEL_INST(steam_gameserver_gamestats);

    DEL_INST(steam_matchmaking);
    DEL_INST(steam_matchmaking_servers);
    DEL_INST(steam_user_stats);
    DEL_INST(steam_apps);
    DEL_INST(steam_networking);
    DEL_INST(steam_remote_storage);
    DEL_INST(steam_screenshots);
    DEL_INST(steam_http);
    DEL_INST(steam_controller);
    DEL_INST(steam_ugc);
    DEL_INST(steam_applist);
    DEL_INST(steam_music);
    DEL_INST(steam_musicremote);
    DEL_INST(steam_HTMLsurface);
    DEL_INST(steam_inventory);
    DEL_INST(steam_video);
    DEL_INST(steam_parental);
    DEL_INST(steam_networking_sockets);
    DEL_INST(steam_networking_sockets_serialized);
    DEL_INST(steam_networking_messages);
    DEL_INST(steam_game_coordinator);
    DEL_INST(steam_networking_utils);
    DEL_INST(steam_unified_messages);
    DEL_INST(steam_game_search);
    DEL_INST(steam_parties);
    DEL_INST(steam_remoteplay);
    DEL_INST(steam_tv);
    DEL_INST(steam_gamestats);
    DEL_INST(steam_timeline);
    DEL_INST(steam_app_disable_update);
    DEL_INST(steam_billing);

    DEL_INST(steam_utils);
    DEL_INST(steam_friends);
    DEL_INST(steam_user);
    DEL_INST(steam_overlay);
    
    DEL_INST(steam_app_ticket);

    DEL_INST(ugc_bridge);

    DEL_INST(callbacks_server);
    DEL_INST(callbacks_client);
    DEL_INST(callback_results_server);
    DEL_INST(callback_results_client);

    DEL_INST(network);
    DEL_INST(run_every_runcb);

    #undef DEL_INST
}

void Steam_Client::userLogIn()
{
    network->addListenId(settings_client->get_local_steam_id());
    user_logged_in = true;
}

void Steam_Client::serverInit()
{
    server_init = true;
}

bool Steam_Client::IsServerInit()
{
    return server_init;
}

bool Steam_Client::IsUserLogIn()
{
    return user_logged_in;
}

void Steam_Client::serverShutdown()
{
    server_init = false;
}

void Steam_Client::clientShutdown()
{
    user_logged_in = false;
}

void Steam_Client::setAppID(uint32 appid)
{
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (appid && !settings_client->get_local_game_id().AppID()) {
        settings_client->set_game_id(CGameID(appid));
        settings_server->set_game_id(CGameID(appid));
        local_storage->setAppId(appid);
        network->setAppID(appid);

        std::string appid_str(std::to_string(appid));
        set_env_variable("SteamAppId", appid_str);
        set_env_variable("SteamGameId", appid_str);

        if (!settings_client->disable_steamoverlaygameid_env_var) {
            set_env_variable("SteamOverlayGameId", appid_str);
        }
    }

    
}

    // Creates a communication pipe to the Steam client.
// NOT THREADSAFE - ensure that no other threads are accessing Steamworks API when calling
HSteamPipe Steam_Client::CreateSteamPipe()
{
    PRINT_DEBUG_ENTRY();
    if (!steam_pipe_counter) ++steam_pipe_counter;
    HSteamPipe pipe = steam_pipe_counter;
    ++steam_pipe_counter;
    PRINT_DEBUG("  returned pipe handle %i", pipe);

    steam_pipes[pipe] = Steam_Pipe::NO_USER;
    
    return pipe;
}

// Releases a previously created communications pipe
// NOT THREADSAFE - ensure that no other threads are accessing Steamworks API when calling
// "true if the pipe was valid and released successfully; otherwise, false"
// https://partner.steamgames.com/doc/api/ISteamClient
bool Steam_Client::BReleaseSteamPipe( HSteamPipe hSteamPipe )
{
    PRINT_DEBUG("%i", hSteamPipe);
    if (steam_pipes.count(hSteamPipe)) {
        return steam_pipes.erase(hSteamPipe) > 0;
    }

    return false;
}

// connects to an existing global user, failing if none exists
// used by the game to coordinate with the steamUI
// NOT THREADSAFE - ensure that no other threads are accessing Steamworks API when calling
HSteamUser Steam_Client::ConnectToGlobalUser( HSteamPipe hSteamPipe )
{
    PRINT_DEBUG("%i", hSteamPipe);
    if (!steam_pipes.count(hSteamPipe)) {
        return 0;
    }

    userLogIn();
    
    // games like appid 1740720 and 2379780 do not call SteamAPI_RunCallbacks() or SteamAPI_ManualDispatch_RunFrame() or Steam_BGetCallback()
    // hence all run_callbacks() will never run, which might break the assumption that these callbacks are always run
    // also networking callbacks won't run
    // hence we spawn the background thread here which trigger all run_callbacks() and run networking callbacks
    PRINT_DEBUG("started background thread");
    background_thread->start(this);

    steam_overlay->SetupOverlay();
    
    steam_pipes[hSteamPipe] = Steam_Pipe::CLIENT;
    return CLIENT_HSTEAMUSER;
}

// used by game servers, create a steam user that won't be shared with anyone else
// NOT THREADSAFE - ensure that no other threads are accessing Steamworks API when calling
HSteamUser Steam_Client::CreateLocalUser( HSteamPipe *phSteamPipe, EAccountType eAccountType )
{
    PRINT_DEBUG("%p %i", phSteamPipe, eAccountType);
    //if (eAccountType == k_EAccountTypeIndividual) {
        //Is this actually used?
        //if (phSteamPipe) *phSteamPipe = CLIENT_STEAM_PIPE;
        //return CLIENT_HSTEAMUSER;
    //} else { //k_EAccountTypeGameServer
    serverInit();

    // gameservers don't call ConnectToGlobalUser(), instead they call this function
    PRINT_DEBUG("started background thread");
    background_thread->start(this);

    HSteamPipe pipe = CreateSteamPipe();
    if (phSteamPipe) *phSteamPipe = pipe;
    steam_pipes[pipe] = Steam_Pipe::SERVER;
    steamclient_server_inited = true;
    return SERVER_HSTEAMUSER;
    //}
}

HSteamUser Steam_Client::CreateLocalUser( HSteamPipe *phSteamPipe )
{
    return CreateLocalUser(phSteamPipe, k_EAccountTypeGameServer);
}

// removes an allocated user
// NOT THREADSAFE - ensure that no other threads are accessing Steamworks API when calling
void Steam_Client::ReleaseUser( HSteamPipe hSteamPipe, HSteamUser hUser )
{
    PRINT_DEBUG_ENTRY();
    if (hUser == SERVER_HSTEAMUSER && steam_pipes.count(hSteamPipe)) {
        steamclient_server_inited = false;
    }
}

// set the local IP and Port to bind to
// this must be set before CreateLocalUser()
void Steam_Client::SetLocalIPBinding( uint32 unIP, uint16 usPort )
{
    PRINT_DEBUG("old %u %hu // TODO", unIP, usPort);
}

void Steam_Client::SetLocalIPBinding( const SteamIPAddress_t &unIP, uint16 usPort )
{
    PRINT_DEBUG("%i %u %hu // TODO", unIP.m_eType, unIP.m_unIPv4, usPort);
}


// Deprecated. Applications should use SteamAPI_RunCallbacks() or SteamGameServer_RunCallbacks() instead.
void Steam_Client::RunFrame()
{
    PRINT_DEBUG_TODO();
}

// returns the number of IPC calls made since the last time this function was called
// Used for perf debugging so you can understand how many IPC calls your game makes per frame
// Every IPC call is at minimum a thread context switch if not a process one so you want to rate
// control how often you do them.
uint32 Steam_Client::GetIPCCallCount()
{
    PRINT_DEBUG_ENTRY();
    return steam_utils->GetIPCCallCount();
}

// API warning handling
// 'int' is the severity; 0 for msg, 1 for warning
// 'const char *' is the text of the message
// callbacks will occur directly after the API function is called that generated the warning or message.
void Steam_Client::SetWarningMessageHook( SteamAPIWarningMessageHook_t pFunction )
{
    PRINT_DEBUG("%p // TODO", pFunction);
}

// Trigger global shutdown for the DLL
bool Steam_Client::BShutdownIfAllPipesClosed()
{
    PRINT_DEBUG_ENTRY();
    if (steam_pipes.size()) return false; // not all pipes are released via BReleaseSteamPipe() yet
    
    PRINT_DEBUG("killing background thread...");
    background_thread->kill();
    PRINT_DEBUG("killed background thread");

    steam_controller->Shutdown();
    steam_overlay->UnSetupOverlay();

    PRINT_DEBUG("all pipes closed");
    return true;
}

// Helper functions for internal Steam usage
void Steam_Client::DEPRECATED_Set_SteamAPI_CPostAPIResultInProcess( void (*)() )
{
    PRINT_DEBUG_TODO();
}

void Steam_Client::DEPRECATED_Remove_SteamAPI_CPostAPIResultInProcess( void (*)() )
{
    PRINT_DEBUG_TODO();
}

void Steam_Client::Set_SteamAPI_CCheckCallbackRegisteredInProcess( SteamAPI_CheckCallbackRegistered_t func )
{
    PRINT_DEBUG("%p // TODO", func);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}

void Steam_Client::Set_SteamAPI_CPostAPIResultInProcess( SteamAPI_PostAPIResultInProcess_t func )
{
    PRINT_DEBUG_TODO();
}

void Steam_Client::Remove_SteamAPI_CPostAPIResultInProcess( SteamAPI_PostAPIResultInProcess_t func )
{
    PRINT_DEBUG_TODO();
}

void Steam_Client::RegisterCallback( class CCallbackBase *pCallback, int iCallback)
{
    int base_callback = (iCallback / 100) * 100;
    int callback_id = iCallback % 100;
    bool isGameServer = CCallbackMgr::isServer(pCallback);
    PRINT_DEBUG("isGameServer %u %i %i", isGameServer, iCallback, base_callback);

    switch (base_callback) {
        case k_iSteamUserCallbacks:
            PRINT_DEBUG("k_iSteamUserCallbacks %i", callback_id);
            break;

        case k_iSteamGameServerCallbacks:
            PRINT_DEBUG("k_iSteamGameServerCallbacks %i", callback_id);
            break;

        case k_iSteamFriendsCallbacks:
            PRINT_DEBUG("k_iSteamFriendsCallbacks %i", callback_id);
            break;

        case k_iSteamBillingCallbacks:
            PRINT_DEBUG("k_iSteamBillingCallbacks %i", callback_id);
            break;

        case k_iSteamMatchmakingCallbacks:
            PRINT_DEBUG("k_iSteamMatchmakingCallbacks %i", callback_id);
            break;

        case k_iSteamContentServerCallbacks:
            PRINT_DEBUG("k_iSteamContentServerCallbacks %i", callback_id);
            break;

        case k_iSteamUtilsCallbacks:
            PRINT_DEBUG("k_iSteamUtilsCallbacks %i", callback_id);
            break;

        case k_iClientFriendsCallbacks:
            PRINT_DEBUG("k_iClientFriendsCallbacks %i", callback_id);
            break;

        case k_iClientUserCallbacks:
            PRINT_DEBUG("k_iClientUserCallbacks %i", callback_id);
            break;

        case k_iSteamAppsCallbacks:
            PRINT_DEBUG("k_iSteamAppsCallbacks %i", callback_id);
            break;

        case k_iSteamUserStatsCallbacks:
            PRINT_DEBUG("k_iSteamUserStatsCallbacks %i", callback_id);
            break;

        case k_iSteamNetworkingCallbacks:
            PRINT_DEBUG("k_iSteamNetworkingCallbacks %i", callback_id);
            break;

        case k_iClientRemoteStorageCallbacks:
            PRINT_DEBUG("k_iClientRemoteStorageCallbacks %i", callback_id);
            break;

        case k_iClientDepotBuilderCallbacks:
            PRINT_DEBUG("k_iClientDepotBuilderCallbacks %i", callback_id);
            break;

        case k_iSteamGameServerItemsCallbacks:
            PRINT_DEBUG("k_iSteamGameServerItemsCallbacks %i", callback_id);
            break;

        case k_iClientUtilsCallbacks:
            PRINT_DEBUG("k_iClientUtilsCallbacks %i", callback_id);
            break;

        case k_iSteamGameCoordinatorCallbacks:
            PRINT_DEBUG("k_iSteamGameCoordinatorCallbacks %i", callback_id);
            break;

        case k_iSteamGameServerStatsCallbacks:
            PRINT_DEBUG("k_iSteamGameServerStatsCallbacks %i", callback_id);
            break;

        case k_iSteam2AsyncCallbacks:
            PRINT_DEBUG("k_iSteam2AsyncCallbacks %i", callback_id);
            break;

        case k_iSteamGameStatsCallbacks:
            PRINT_DEBUG("k_iSteamGameStatsCallbacks %i", callback_id);
            break;

        case k_iClientHTTPCallbacks:
            PRINT_DEBUG("k_iClientHTTPCallbacks %i", callback_id);
            break;

        case k_iClientScreenshotsCallbacks:
            PRINT_DEBUG("k_iClientScreenshotsCallbacks %i", callback_id);
            break;

        case k_iSteamScreenshotsCallbacks:
            PRINT_DEBUG("k_iSteamScreenshotsCallbacks %i", callback_id);
            break;

        case k_iClientAudioCallbacks:
            PRINT_DEBUG("k_iClientAudioCallbacks %i", callback_id);
            break;

        case k_iClientUnifiedMessagesCallbacks:
            PRINT_DEBUG("k_iClientUnifiedMessagesCallbacks %i", callback_id);
            break;

        case k_iSteamStreamLauncherCallbacks:
            PRINT_DEBUG("k_iSteamStreamLauncherCallbacks %i", callback_id);
            break;

        case k_iClientControllerCallbacks:
            PRINT_DEBUG("k_iClientControllerCallbacks %i", callback_id);
            break;

        case k_iSteamControllerCallbacks:
            PRINT_DEBUG("k_iSteamControllerCallbacks %i", callback_id);
            break;

        case k_iClientParentalSettingsCallbacks:
            PRINT_DEBUG("k_iClientParentalSettingsCallbacks %i", callback_id);
            break;

        case k_iClientDeviceAuthCallbacks:
            PRINT_DEBUG("k_iClientDeviceAuthCallbacks %i", callback_id);
            break;

        case k_iClientNetworkDeviceManagerCallbacks:
            PRINT_DEBUG("k_iClientNetworkDeviceManagerCallbacks %i", callback_id);
            break;

        case k_iClientMusicCallbacks:
            PRINT_DEBUG("k_iClientMusicCallbacks %i", callback_id);
            break;

        case k_iClientRemoteClientManagerCallbacks:
            PRINT_DEBUG("k_iClientRemoteClientManagerCallbacks %i", callback_id);
            break;

        case k_iClientUGCCallbacks:
            PRINT_DEBUG("k_iClientUGCCallbacks %i", callback_id);
            break;

        case k_iSteamStreamClientCallbacks:
            PRINT_DEBUG("k_iSteamStreamClientCallbacks %i", callback_id);
            break;

        case k_IClientProductBuilderCallbacks:
            PRINT_DEBUG("k_IClientProductBuilderCallbacks %i", callback_id);
            break;

        case k_iClientShortcutsCallbacks:
            PRINT_DEBUG("k_iClientShortcutsCallbacks %i", callback_id);
            break;

        case k_iClientRemoteControlManagerCallbacks:
            PRINT_DEBUG("k_iClientRemoteControlManagerCallbacks %i", callback_id);
            break;

        case k_iSteamAppListCallbacks:
            PRINT_DEBUG("k_iSteamAppListCallbacks %i", callback_id);
            break;

        case k_iSteamMusicCallbacks:
            PRINT_DEBUG("k_iSteamMusicCallbacks %i", callback_id);
            break;

        case k_iSteamMusicRemoteCallbacks:
            PRINT_DEBUG("k_iSteamMusicRemoteCallbacks %i", callback_id);
            break;

        case k_iClientVRCallbacks:
            PRINT_DEBUG("k_iClientVRCallbacks %i", callback_id);
            break;

        case k_iClientGameNotificationCallbacks:
            PRINT_DEBUG("k_iClientGameNotificationCallbacks %i", callback_id);
            break;
 
        case k_iSteamGameNotificationCallbacks:
            PRINT_DEBUG("k_iSteamGameNotificationCallbacks %i", callback_id);
            break;
 
        case k_iSteamHTMLSurfaceCallbacks:
            PRINT_DEBUG("k_iSteamHTMLSurfaceCallbacks %i", callback_id);
            break;

        case k_iClientVideoCallbacks:
            PRINT_DEBUG("k_iClientVideoCallbacks %i", callback_id);
            break;

        case k_iClientInventoryCallbacks:
            PRINT_DEBUG("k_iClientInventoryCallbacks %i", callback_id);
            break;

        case k_iClientBluetoothManagerCallbacks:
            PRINT_DEBUG("k_iClientBluetoothManagerCallbacks %i", callback_id);
            break;

        case k_iClientSharedConnectionCallbacks:
            PRINT_DEBUG("k_iClientSharedConnectionCallbacks %i", callback_id);
            break;

        case k_ISteamParentalSettingsCallbacks:
            PRINT_DEBUG("k_ISteamParentalSettingsCallbacks %i", callback_id);
            break;

        case k_iClientShaderCallbacks:
            PRINT_DEBUG("k_iClientShaderCallbacks %i", callback_id);
            break;
        
        default:
            PRINT_DEBUG("Unknown callback base %i", base_callback);
    };

    if (isGameServer) {
        callbacks_server->addCallBack(iCallback, pCallback);
    } else {
        callbacks_client->addCallBack(iCallback, pCallback);
    }
}

void Steam_Client::UnregisterCallback( class CCallbackBase *pCallback)
{
    int iCallback = pCallback->GetICallback();
    int base_callback = (iCallback / 100) * 100;
    int callback_id = iCallback % 100;
    bool isGameServer = CCallbackMgr::isServer(pCallback);
    PRINT_DEBUG("isGameServer %u %i", isGameServer, base_callback);

    switch (base_callback) {
        case k_iSteamUserCallbacks:
            PRINT_DEBUG("k_iSteamUserCallbacks %i", callback_id);
            break;

        case k_iSteamGameServerCallbacks:
            PRINT_DEBUG("k_iSteamGameServerCallbacks %i", callback_id);
            break;

        case k_iSteamFriendsCallbacks:
            PRINT_DEBUG("k_iSteamFriendsCallbacks %i", callback_id);
            break;

        case k_iSteamBillingCallbacks:
            PRINT_DEBUG("k_iSteamBillingCallbacks %i", callback_id);
            break;

        case k_iSteamMatchmakingCallbacks:
            PRINT_DEBUG("k_iSteamMatchmakingCallbacks %i", callback_id);
            break;

        case k_iSteamContentServerCallbacks:
            PRINT_DEBUG("k_iSteamContentServerCallbacks %i", callback_id);
            break;

        case k_iSteamUtilsCallbacks:
            PRINT_DEBUG("k_iSteamUtilsCallbacks %i", callback_id);
            break;

        case k_iClientFriendsCallbacks:
            PRINT_DEBUG("k_iClientFriendsCallbacks %i", callback_id);
            break;

        case k_iClientUserCallbacks:
            PRINT_DEBUG("k_iClientUserCallbacks %i", callback_id);
            break;

        case k_iSteamAppsCallbacks:
            PRINT_DEBUG("k_iSteamAppsCallbacks %i", callback_id);
            break;

        case k_iSteamUserStatsCallbacks:
            PRINT_DEBUG("k_iSteamUserStatsCallbacks %i", callback_id);
            break;

        case k_iSteamNetworkingCallbacks:
            PRINT_DEBUG("k_iSteamNetworkingCallbacks %i", callback_id);
            break;

        case k_iClientRemoteStorageCallbacks:
            PRINT_DEBUG("k_iClientRemoteStorageCallbacks %i", callback_id);
            break;

        case k_iClientDepotBuilderCallbacks:
            PRINT_DEBUG("k_iClientDepotBuilderCallbacks %i", callback_id);
            break;

        case k_iSteamGameServerItemsCallbacks:
            PRINT_DEBUG("k_iSteamGameServerItemsCallbacks %i", callback_id);
            break;

        case k_iClientUtilsCallbacks:
            PRINT_DEBUG("k_iClientUtilsCallbacks %i", callback_id);
            break;

        case k_iSteamGameCoordinatorCallbacks:
            PRINT_DEBUG("k_iSteamGameCoordinatorCallbacks %i", callback_id);
            break;

        case k_iSteamGameServerStatsCallbacks:
            PRINT_DEBUG("k_iSteamGameServerStatsCallbacks %i", callback_id);
            break;

        case k_iSteam2AsyncCallbacks:
            PRINT_DEBUG("k_iSteam2AsyncCallbacks %i", callback_id);
            break;

        case k_iSteamGameStatsCallbacks:
            PRINT_DEBUG("k_iSteamGameStatsCallbacks %i", callback_id);
            break;

        case k_iClientHTTPCallbacks:
            PRINT_DEBUG("k_iClientHTTPCallbacks %i", callback_id);
            break;

        case k_iClientScreenshotsCallbacks:
            PRINT_DEBUG("k_iClientScreenshotsCallbacks %i", callback_id);
            break;

        case k_iSteamScreenshotsCallbacks:
            PRINT_DEBUG("k_iSteamScreenshotsCallbacks %i", callback_id);
            break;

        case k_iClientAudioCallbacks:
            PRINT_DEBUG("k_iClientAudioCallbacks %i", callback_id);
            break;

        case k_iClientUnifiedMessagesCallbacks:
            PRINT_DEBUG("k_iClientUnifiedMessagesCallbacks %i", callback_id);
            break;

        case k_iSteamStreamLauncherCallbacks:
            PRINT_DEBUG("k_iSteamStreamLauncherCallbacks %i", callback_id);
            break;

        case k_iClientControllerCallbacks:
            PRINT_DEBUG("k_iClientControllerCallbacks %i", callback_id);
            break;

        case k_iSteamControllerCallbacks:
            PRINT_DEBUG("k_iSteamControllerCallbacks %i", callback_id);
            break;

        case k_iClientParentalSettingsCallbacks:
            PRINT_DEBUG("k_iClientParentalSettingsCallbacks %i", callback_id);
            break;

        case k_iClientDeviceAuthCallbacks:
            PRINT_DEBUG("k_iClientDeviceAuthCallbacks %i", callback_id);
            break;

        case k_iClientNetworkDeviceManagerCallbacks:
            PRINT_DEBUG("k_iClientNetworkDeviceManagerCallbacks %i", callback_id);
            break;

        case k_iClientMusicCallbacks:
            PRINT_DEBUG("k_iClientMusicCallbacks %i", callback_id);
            break;

        case k_iClientRemoteClientManagerCallbacks:
            PRINT_DEBUG("k_iClientRemoteClientManagerCallbacks %i", callback_id);
            break;

        case k_iClientUGCCallbacks:
            PRINT_DEBUG("k_iClientUGCCallbacks %i", callback_id);
            break;

        case k_iSteamStreamClientCallbacks:
            PRINT_DEBUG("k_iSteamStreamClientCallbacks %i", callback_id);
            break;

        case k_IClientProductBuilderCallbacks:
            PRINT_DEBUG("k_IClientProductBuilderCallbacks %i", callback_id);
            break;

        case k_iClientShortcutsCallbacks:
            PRINT_DEBUG("k_iClientShortcutsCallbacks %i", callback_id);
            break;

        case k_iClientRemoteControlManagerCallbacks:
            PRINT_DEBUG("k_iClientRemoteControlManagerCallbacks %i", callback_id);
            break;

        case k_iSteamAppListCallbacks:
            PRINT_DEBUG("k_iSteamAppListCallbacks %i", callback_id);
            break;

        case k_iSteamMusicCallbacks:
            PRINT_DEBUG("k_iSteamMusicCallbacks %i", callback_id);
            break;

        case k_iSteamMusicRemoteCallbacks:
            PRINT_DEBUG("k_iSteamMusicRemoteCallbacks %i", callback_id);
            break;

        case k_iClientVRCallbacks:
            PRINT_DEBUG("k_iClientVRCallbacks %i", callback_id);
            break;

        case k_iClientGameNotificationCallbacks:
            PRINT_DEBUG("k_iClientGameNotificationCallbacks %i", callback_id);
            break;
 
        case k_iSteamGameNotificationCallbacks:
            PRINT_DEBUG("k_iSteamGameNotificationCallbacks %i", callback_id);
            break;
 
        case k_iSteamHTMLSurfaceCallbacks:
            PRINT_DEBUG("k_iSteamHTMLSurfaceCallbacks %i", callback_id);
            break;

        case k_iClientVideoCallbacks:
            PRINT_DEBUG("k_iClientVideoCallbacks %i", callback_id);
            break;

        case k_iClientInventoryCallbacks:
            PRINT_DEBUG("k_iClientInventoryCallbacks %i", callback_id);
            break;

        case k_iClientBluetoothManagerCallbacks:
            PRINT_DEBUG("k_iClientBluetoothManagerCallbacks %i", callback_id);
            break;

        case k_iClientSharedConnectionCallbacks:
            PRINT_DEBUG("k_iClientSharedConnectionCallbacks %i", callback_id);
            break;

        case k_ISteamParentalSettingsCallbacks:
            PRINT_DEBUG("k_ISteamParentalSettingsCallbacks %i", callback_id);
            break;

        case k_iClientShaderCallbacks:
            PRINT_DEBUG("k_iClientShaderCallbacks %i", callback_id);
            break;
        
        default:
            PRINT_DEBUG("Unknown callback base %i", base_callback);
    };

    if (isGameServer) {
        callbacks_server->rmCallBack(iCallback, pCallback);
    } else {
        callbacks_client->rmCallBack(iCallback, pCallback);
    }
}

void Steam_Client::RegisterCallResult( class CCallbackBase *pCallback, SteamAPICall_t hAPICall)
{
    PRINT_DEBUG("%llu %i", hAPICall, pCallback->GetICallback());
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    callback_results_client->addCallBack(hAPICall, pCallback);
    callback_results_server->addCallBack(hAPICall, pCallback);
    
}

void Steam_Client::UnregisterCallResult( class CCallbackBase *pCallback, SteamAPICall_t hAPICall)
{
    PRINT_DEBUG("%llu %i", hAPICall, pCallback->GetICallback());
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    callback_results_client->rmCallBack(hAPICall, pCallback);
    callback_results_server->rmCallBack(hAPICall, pCallback);
}

void Steam_Client::RunCallbacks(bool runClientCB, bool runGameserverCB)
{
    PRINT_DEBUG("begin ------------------------------------------------------");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    cb_run_active = true;

    // PRINT_DEBUG("network *********");
    network->Run(); // networking must run first since it receives messages use by each run_callback()

    // PRINT_DEBUG("steam_matchmaking_servers *********");
    steam_matchmaking_servers->RunCallbacks();
    
    // PRINT_DEBUG("run_every_runcb *********");
    run_every_runcb->run();

    // PRINT_DEBUG("steam_gameserver *********");
    steam_gameserver->RunCallbacks();

    if (runClientCB) {
        // PRINT_DEBUG("callback_results_client *********");
        callback_results_client->runCallResults();
    }

    if (runGameserverCB) {
        // PRINT_DEBUG("callback_results_server *********");
        callback_results_server->runCallResults();
    }

    // PRINT_DEBUG("callbacks_server *********");
    callbacks_server->runCallBacks();

    // PRINT_DEBUG("callbacks_client *********");
    callbacks_client->runCallBacks();

    last_cb_run = (unsigned long long)std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
    cb_run_active = false;
    PRINT_DEBUG("done ******************************************************");
}

void Steam_Client::DestroyAllInterfaces()
{
    PRINT_DEBUG_TODO();
}

// older sdk ----------------------------------------------------------

// creates a global instance of a steam user, so that other processes can share it
// used by the steam UI, to share it's account info/connection with any games it launches
// fails (returns NULL) if an existing instance already exists
HSteamUser Steam_Client::CreateGlobalUser( HSteamPipe *phSteamPipe )
{
    // TODO not sure if this implementation is correct
    PRINT_DEBUG_TODO();
    for (const auto& [pipe_handle, pipe_type] : steam_pipes) {
        if (pipe_type == Steam_Pipe::CLIENT) {
            if (phSteamPipe) *phSteamPipe = pipe_handle;
            return 0;
        }
    }

    HSteamPipe pipe = CreateSteamPipe();
    if (phSteamPipe) *phSteamPipe = pipe;

    steam_pipes[pipe] = Steam_Pipe::CLIENT;
    return CLIENT_HSTEAMUSER;
}

// retrieves the IVac interface associated with the handle
// there is normally only one instance of VAC running, but using this connects it to the right user/account
void *Steam_Client::GetIVAC( HSteamUser hSteamUser )
{
    PRINT_DEBUG_ENTRY();
    // actual value from steamclient64.dll
    return nullptr;
}

// returns the name of a universe
const char *Steam_Client::GetUniverseName( EUniverse eUniverse )
{
    PRINT_DEBUG("%i", (int)eUniverse);
    // actual values returned by steamclient64.dll
    switch (eUniverse)
    {
    case EUniverse::k_EUniverseInvalid: return "Invalid";
    case EUniverse::k_EUniversePublic: return "Public";
    case EUniverse::k_EUniverseBeta: return "Beta";
    case EUniverse::k_EUniverseInternal: return "Internal";
    case EUniverse::k_EUniverseDev: return "Dev";
    }

    return "Unknown";
}
// older sdk ----------------------------------------------------------
