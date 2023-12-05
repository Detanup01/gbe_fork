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

#include "steam_client.h"
#include "settings_parser.h"

static std::mutex kill_background_thread_mutex;
static std::condition_variable kill_background_thread_cv;
static bool kill_background_thread;
static void background_thread(Steam_Client *client)
{
    PRINT_DEBUG("background thread starting\n");

    while (1) {
        {
            std::unique_lock<std::mutex> lck(kill_background_thread_mutex);
            if (kill_background_thread || kill_background_thread_cv.wait_for(lck, std::chrono::seconds(1)) != std::cv_status::timeout) {
                if (kill_background_thread) {
                    PRINT_DEBUG("background thread exit\n");
                    return;
                }
            }
        }

        unsigned long long time = std::chrono::duration_cast<std::chrono::duration<unsigned long long>>(std::chrono::system_clock::now().time_since_epoch()).count();

        if (time > client->last_cb_run + 1) {
            global_mutex.lock();
            PRINT_DEBUG("background thread run\n");
            client->network->Run();
            client->run_every_runcb->run();
            global_mutex.unlock();
        }
    }
}

Steam_Client::Steam_Client()
{
    uint32 appid = create_localstorage_settings(&settings_client, &settings_server, &local_storage);

    network = new Networking(settings_server->get_local_steam_id(), appid, settings_server->get_port(), &(settings_server->custom_broadcasts), settings_server->disable_networking);

    callback_results_client = new SteamCallResults();
    callback_results_server = new SteamCallResults();
    callbacks_client = new SteamCallBacks(callback_results_client);
    callbacks_server = new SteamCallBacks(callback_results_server);
    run_every_runcb = new RunEveryRunCB();

    PRINT_DEBUG("steam client init: id: %llu server id: %llu appid: %u port: %u \n", settings_client->get_local_steam_id().ConvertToUint64(), settings_server->get_local_steam_id().ConvertToUint64(), appid, settings_server->get_port());

    if (appid) {
        set_env_variable("SteamAppId", std::to_string(appid));
        set_env_variable("SteamGameId", std::to_string(appid));
    }

    steam_overlay = new Steam_Overlay(settings_client, callback_results_client, callbacks_client, run_every_runcb, network);

    steam_user = new Steam_User(settings_client, local_storage, network, callback_results_client, callbacks_client);
    steam_friends = new Steam_Friends(settings_client, network, callback_results_client, callbacks_client, run_every_runcb, steam_overlay);
    steam_utils = new Steam_Utils(settings_client, callback_results_client, steam_overlay);

    steam_matchmaking = new Steam_Matchmaking(settings_client, network, callback_results_client, callbacks_client, run_every_runcb);
    steam_matchmaking_servers = new Steam_Matchmaking_Servers(settings_client, network);
    steam_user_stats = new Steam_User_Stats(settings_client, local_storage, callback_results_client, callbacks_client, steam_overlay);
    steam_apps = new Steam_Apps(settings_client, callback_results_client);
    steam_networking = new Steam_Networking(settings_client, network, callbacks_client, run_every_runcb);
    steam_remote_storage = new Steam_Remote_Storage(settings_client, local_storage, callback_results_client);
    steam_screenshots = new Steam_Screenshots(local_storage, callbacks_client);
    steam_http = new Steam_HTTP(settings_client, network, callback_results_client, callbacks_client);
    steam_controller = new Steam_Controller(settings_client, callback_results_client, callbacks_client, run_every_runcb);
    steam_ugc = new Steam_UGC(settings_client, callback_results_client, callbacks_client);
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

    PRINT_DEBUG("client init gameserver\n");
    steam_gameserver = new Steam_GameServer(settings_server, network, callbacks_server);
    steam_gameserver_utils = new Steam_Utils(settings_server, callback_results_server, steam_overlay);
    steam_gameserverstats = new Steam_GameServerStats(settings_server, network, callback_results_server, callbacks_server);
    steam_gameserver_networking = new Steam_Networking(settings_server, network, callbacks_server, run_every_runcb);
    steam_gameserver_http = new Steam_HTTP(settings_server, network, callback_results_server, callbacks_server);
    steam_gameserver_inventory = new Steam_Inventory(settings_server, callback_results_server, callbacks_server, run_every_runcb, local_storage);
    steam_gameserver_ugc = new Steam_UGC(settings_server, callback_results_server, callbacks_server);
    steam_gameserver_apps = new Steam_Apps(settings_server, callback_results_server);
    steam_gameserver_networking_sockets = new Steam_Networking_Sockets(settings_server, network, callback_results_server, callbacks_server, run_every_runcb, steam_networking_sockets->get_shared_between_client_server());
    steam_gameserver_networking_sockets_serialized = new Steam_Networking_Sockets_Serialized(settings_server, network, callback_results_server, callbacks_server, run_every_runcb);
    steam_gameserver_networking_messages = new Steam_Networking_Messages(settings_server, network, callback_results_server, callbacks_server, run_every_runcb);
    steam_gameserver_game_coordinator = new Steam_Game_Coordinator(settings_server, network, callback_results_server, callbacks_server, run_every_runcb);
    steam_masterserver_updater = new Steam_Masterserver_Updater(settings_server, network, callback_results_server, callbacks_server, run_every_runcb);

    gameserver_has_ipv6_functions = false;

    last_cb_run = 0;
    PRINT_DEBUG("client init end\n");
}

Steam_Client::~Steam_Client()
{
    delete steam_gameserver;
    delete steam_gameserver_utils;
    delete steam_gameserverstats;
    delete steam_gameserver_networking;
    delete steam_gameserver_http;
    delete steam_gameserver_inventory;
    delete steam_gameserver_ugc;
    delete steam_gameserver_apps;
    delete steam_gameserver_networking_sockets;
    delete steam_gameserver_networking_sockets_serialized;
    delete steam_gameserver_networking_messages;
    delete steam_gameserver_game_coordinator;
    delete steam_masterserver_updater;

    delete steam_matchmaking;
    delete steam_matchmaking_servers;
    delete steam_user_stats;
    delete steam_apps;
    delete steam_networking;
    delete steam_remote_storage;
    delete steam_screenshots;
    delete steam_http;
    delete steam_controller;
    delete steam_ugc;
    delete steam_applist;
    delete steam_music;
    delete steam_musicremote;
    delete steam_HTMLsurface;
    delete steam_inventory;
    delete steam_video;
    delete steam_parental;
    delete steam_networking_sockets;
    delete steam_networking_sockets_serialized;
    delete steam_networking_messages;
    delete steam_game_coordinator;
    delete steam_networking_utils;
    delete steam_unified_messages;
    delete steam_game_search;
    delete steam_parties;
    delete steam_remoteplay;
    delete steam_tv;

    delete steam_utils;
    delete steam_friends;
    delete steam_user;
    delete steam_overlay;

    delete run_every_runcb;
    delete callbacks_server;
    delete callbacks_client;
    delete callback_results_server;
    delete callback_results_client;
    delete network;
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
        set_env_variable("SteamAppId", std::to_string(appid));
        set_env_variable("SteamGameId", std::to_string(appid));
    }

    
}

    // Creates a communication pipe to the Steam client.
// NOT THREADSAFE - ensure that no other threads are accessing Steamworks API when calling
HSteamPipe Steam_Client::CreateSteamPipe()
{
    PRINT_DEBUG("CreateSteamPipe\n");
    HSteamPipe pipe = steam_pipe_counter++;
    PRINT_DEBUG("creating pipe %i\n", pipe);

    steam_pipes[pipe] = Steam_Pipe::NO_USER;
    return pipe;
}

// Releases a previously created communications pipe
// NOT THREADSAFE - ensure that no other threads are accessing Steamworks API when calling
bool Steam_Client::BReleaseSteamPipe( HSteamPipe hSteamPipe )
{
    PRINT_DEBUG("BReleaseSteamPipe %i\n", hSteamPipe);
    if (steam_pipes.count(hSteamPipe)) {
        steam_pipes.erase(hSteamPipe);
        return true;
    }

    return false;
}

// connects to an existing global user, failing if none exists
// used by the game to coordinate with the steamUI
// NOT THREADSAFE - ensure that no other threads are accessing Steamworks API when calling
HSteamUser Steam_Client::ConnectToGlobalUser( HSteamPipe hSteamPipe )
{
    PRINT_DEBUG("ConnectToGlobalUser %i\n", hSteamPipe);
    if (!steam_pipes.count(hSteamPipe)) {
        return 0;
    }

    userLogIn();
#ifdef EMU_OVERLAY
    if(!settings_client->disable_overlay)
        steam_overlay->SetupOverlay();
#endif
    steam_pipes[hSteamPipe] = Steam_Pipe::CLIENT;
    return CLIENT_HSTEAMUSER;
}

// used by game servers, create a steam user that won't be shared with anyone else
// NOT THREADSAFE - ensure that no other threads are accessing Steamworks API when calling
HSteamUser Steam_Client::CreateLocalUser( HSteamPipe *phSteamPipe, EAccountType eAccountType )
{
    PRINT_DEBUG("CreateLocalUser %p %i\n", phSteamPipe, eAccountType);
    //if (eAccountType == k_EAccountTypeIndividual) {
        //Is this actually used?
        //if (phSteamPipe) *phSteamPipe = CLIENT_STEAM_PIPE;
        //return CLIENT_HSTEAMUSER;
    //} else { //k_EAccountTypeGameServer
    serverInit();

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
    PRINT_DEBUG("ReleaseUser\n");
    if (hUser == SERVER_HSTEAMUSER && steam_pipes.count(hSteamPipe)) {
        steamclient_server_inited = false;
    }
}

// retrieves the ISteamUser interface associated with the handle
ISteamUser *Steam_Client::GetISteamUser( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("GetISteamUser %s\n", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamUser) return NULL;

    if (strcmp(pchVersion, "SteamUser009") == 0) {
        return (ISteamUser *)(void *)(ISteamUser009 *)steam_user;
    } else if (strcmp(pchVersion, "SteamUser010") == 0) {
        return (ISteamUser *)(void *)(ISteamUser010 *)steam_user;
    } else if (strcmp(pchVersion, "SteamUser011") == 0) {
        return (ISteamUser *)(void *)(ISteamUser011 *)steam_user;
    } else if (strcmp(pchVersion, "SteamUser012") == 0) {
        return (ISteamUser *)(void *)(ISteamUser012 *)steam_user;
    } else if (strcmp(pchVersion, "SteamUser013") == 0) {
        return (ISteamUser *)(void *)(ISteamUser013 *)steam_user;
    } else if (strcmp(pchVersion, "SteamUser014") == 0) {
        return (ISteamUser *)(void *)(ISteamUser014 *)steam_user;
    } else if (strcmp(pchVersion, "SteamUser015") == 0) {
        return (ISteamUser *)(void *)(ISteamUser015 *)steam_user;
    } else if (strcmp(pchVersion, "SteamUser016") == 0) {
        return (ISteamUser *)(void *)(ISteamUser016 *)steam_user;
    } else if (strcmp(pchVersion, "SteamUser017") == 0) {
        return (ISteamUser *)(void *)(ISteamUser017 *)steam_user;
    } else if (strcmp(pchVersion, "SteamUser018") == 0) {
        return (ISteamUser *)(void *)(ISteamUser018 *)steam_user;
    } else if (strcmp(pchVersion, "SteamUser019") == 0) {
        return (ISteamUser *)(void *)(ISteamUser019 *)steam_user;
    } else if (strcmp(pchVersion, "SteamUser020") == 0) {
        return (ISteamUser *)(void *)(ISteamUser020 *)steam_user;
    } else if (strcmp(pchVersion, "SteamUser021") == 0) {
        return (ISteamUser *)(void *)(ISteamUser021 *)steam_user;
    } else if (strcmp(pchVersion, "SteamUser022") == 0) {
        return (ISteamUser *)(void *)(ISteamUser022 *)steam_user;
    } else if (strcmp(pchVersion, STEAMUSER_INTERFACE_VERSION) == 0) {
        return (ISteamUser *)(void *)(ISteamUser *)steam_user;
    } else {
        return (ISteamUser *)(void *)(ISteamUser *)steam_user;
    }

    return steam_user;
}

// retrieves the ISteamGameServer interface associated with the handle
ISteamGameServer *Steam_Client::GetISteamGameServer( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("GetISteamGameServer %s\n", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamUser) return NULL;

    if (strcmp(pchVersion, "SteamGameServer004") == 0) {
        return (ISteamGameServer *)(void *)(ISteamGameServer004 *)steam_gameserver;
    } else if (strcmp(pchVersion, "SteamGameServer005") == 0) {
        return (ISteamGameServer *)(void *)(ISteamGameServer005 *)steam_gameserver;
    } else if (strcmp(pchVersion, "SteamGameServer006") == 0) {
        return (ISteamGameServer *)(void *)(ISteamGameServer008 *)steam_gameserver;
    } else if (strcmp(pchVersion, "SteamGameServer007") == 0) {
        return (ISteamGameServer *)(void *)(ISteamGameServer008 *)steam_gameserver;
    } else if (strcmp(pchVersion, "SteamGameServer008") == 0) {
        return (ISteamGameServer *)(void *)(ISteamGameServer008 *)steam_gameserver;
    } else if (strcmp(pchVersion, "SteamGameServer009") == 0) {
        return (ISteamGameServer *)(void *)(ISteamGameServer009 *)steam_gameserver;
    } else if (strcmp(pchVersion, "SteamGameServer010") == 0) {
        return (ISteamGameServer *)(void *)(ISteamGameServer010 *)steam_gameserver;
    } else if (strcmp(pchVersion, "SteamGameServer011") == 0) {
        return (ISteamGameServer *)(void *)(ISteamGameServer011 *)steam_gameserver;
    } else if (strcmp(pchVersion, "SteamGameServer012") == 0) {
        return (ISteamGameServer *)(void *)(ISteamGameServer012 *)steam_gameserver;
    } else if (strcmp(pchVersion, "SteamGameServer013") == 0) {
        gameserver_has_ipv6_functions = true;
        return (ISteamGameServer *)(void *)(ISteamGameServer013 *)steam_gameserver;
    } else if (strcmp(pchVersion, "SteamGameServer014") == 0) {
        gameserver_has_ipv6_functions = true;
        return (ISteamGameServer *)(void *)(ISteamGameServer014 *)steam_gameserver;
    } else if (strcmp(pchVersion, STEAMGAMESERVER_INTERFACE_VERSION) == 0) {
        gameserver_has_ipv6_functions = true;
        return (ISteamGameServer *)(void *)(ISteamGameServer *)steam_gameserver;
    } else {
        gameserver_has_ipv6_functions = true;
        return (ISteamGameServer *)(void *)(ISteamGameServer *)steam_gameserver;
    }

    return steam_gameserver;
}

// set the local IP and Port to bind to
// this must be set before CreateLocalUser()
void Steam_Client::SetLocalIPBinding( uint32 unIP, uint16 usPort )
{
    PRINT_DEBUG("SetLocalIPBinding old %u %hu\n", unIP, usPort);
}

void Steam_Client::SetLocalIPBinding( const SteamIPAddress_t &unIP, uint16 usPort )
{
    PRINT_DEBUG("SetLocalIPBinding %i %u %hu\n", unIP.m_eType, unIP.m_unIPv4, usPort);
}

// returns the ISteamFriends interface
ISteamFriends *Steam_Client::GetISteamFriends( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("GetISteamFriends %s\n", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamUser) return NULL;

    if (strcmp(pchVersion, "SteamFriends003") == 0) {
        return (ISteamFriends *)(void *)(ISteamFriends003 *)steam_friends;
    } else if (strcmp(pchVersion, "SteamFriends004") == 0) {
        return (ISteamFriends *)(void *)(ISteamFriends004 *)steam_friends;
    } else if (strcmp(pchVersion, "SteamFriends005") == 0) {
        return (ISteamFriends *)(void *)(ISteamFriends005 *)steam_friends;
    } else if (strcmp(pchVersion, "SteamFriends006") == 0) {
        return (ISteamFriends *)(void *)(ISteamFriends006 *)steam_friends;
    } else if (strcmp(pchVersion, "SteamFriends007") == 0) {
        return (ISteamFriends *)(void *)(ISteamFriends007 *)steam_friends;
    } else if (strcmp(pchVersion, "SteamFriends008") == 0) {
        return (ISteamFriends *)(void *)(ISteamFriends008 *)steam_friends;
    } else if (strcmp(pchVersion, "SteamFriends009") == 0) {
        return (ISteamFriends *)(void *)(ISteamFriends009 *)steam_friends;
    } else if (strcmp(pchVersion, "SteamFriends010") == 0) {
        return (ISteamFriends *)(void *)(ISteamFriends010 *)steam_friends;
    } else if (strcmp(pchVersion, "SteamFriends011") == 0) {
        return (ISteamFriends *)(void *)(ISteamFriends011 *)steam_friends;
    } else if (strcmp(pchVersion, "SteamFriends012") == 0) {
        return (ISteamFriends *)(void *)(ISteamFriends012 *)steam_friends;
    } else if (strcmp(pchVersion, "SteamFriends013") == 0) {
        return (ISteamFriends *)(void *)(ISteamFriends013 *)steam_friends;
    } else if (strcmp(pchVersion, "SteamFriends014") == 0) {
        return (ISteamFriends *)(void *)(ISteamFriends014 *)steam_friends;
    } else if (strcmp(pchVersion, "SteamFriends015") == 0) {
        return (ISteamFriends *)(void *)(ISteamFriends015 *)steam_friends;
    } else if (strcmp(pchVersion, "SteamFriends016") == 0) {
        return (ISteamFriends *)(void *)(ISteamFriends016 *)steam_friends;
    } else if (strcmp(pchVersion, STEAMFRIENDS_INTERFACE_VERSION) == 0) {
        return (ISteamFriends *)(void *)(ISteamFriends *)steam_friends;
    } else {
        return (ISteamFriends *)(void *)(ISteamFriends *)steam_friends;
    }

    return steam_friends;
}

// returns the ISteamUtils interface
ISteamUtils *Steam_Client::GetISteamUtils( HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("GetISteamUtils %s\n", pchVersion);
    if (!steam_pipes.count(hSteamPipe)) return NULL;

    Steam_Utils *steam_utils_temp;

    if (steam_pipes[hSteamPipe] == Steam_Pipe::SERVER) {
        steam_utils_temp = steam_gameserver_utils;
    } else {
        steam_utils_temp = steam_utils;
    }

    if (strcmp(pchVersion, "SteamUtils002") == 0) {
        return (ISteamUtils *)(void *)(ISteamUtils002 *)steam_utils_temp;
    } else if (strcmp(pchVersion, "SteamUtils003") == 0) {
        return (ISteamUtils *)(void *)(ISteamUtils003 *)steam_utils_temp;
    } else if (strcmp(pchVersion, "SteamUtils004") == 0) {
        return (ISteamUtils *)(void *)(ISteamUtils004 *)steam_utils_temp;
    } else if (strcmp(pchVersion, "SteamUtils005") == 0) {
        return (ISteamUtils *)(void *)(ISteamUtils005 *)steam_utils_temp;
    } else if (strcmp(pchVersion, "SteamUtils006") == 0) {
        return (ISteamUtils *)(void *)(ISteamUtils006 *)steam_utils_temp;
    } else if (strcmp(pchVersion, "SteamUtils007") == 0) {
        return (ISteamUtils *)(void *)(ISteamUtils007 *)steam_utils_temp;
    } else if (strcmp(pchVersion, "SteamUtils008") == 0) {
        return (ISteamUtils *)(void *)(ISteamUtils008 *)steam_utils_temp;
    } else if (strcmp(pchVersion, "SteamUtils009") == 0) {
        return (ISteamUtils *)(void *)(ISteamUtils009 *)steam_utils_temp;
    } else if (strcmp(pchVersion, STEAMUTILS_INTERFACE_VERSION) == 0) {
        return (ISteamUtils *)(void *)(ISteamUtils *)steam_utils_temp;
    } else {
        return (ISteamUtils *)(void *)(ISteamUtils *)steam_utils_temp;
    }

    return steam_utils_temp;
}

// returns the ISteamMatchmaking interface
ISteamMatchmaking *Steam_Client::GetISteamMatchmaking( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("GetISteamMatchmaking %s\n", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamUser) return NULL;

    if (strcmp(pchVersion, "SteamMatchMaking001") == 0) {
        //TODO
        return (ISteamMatchmaking *)(void *)(ISteamMatchmaking002 *)steam_matchmaking;
    } else if (strcmp(pchVersion, "SteamMatchMaking002") == 0) {
        return (ISteamMatchmaking *)(void *)(ISteamMatchmaking002 *)steam_matchmaking;
    } else if (strcmp(pchVersion, "SteamMatchMaking003") == 0) {
        return (ISteamMatchmaking *)(void *)(ISteamMatchmaking003 *)steam_matchmaking;
    } else if (strcmp(pchVersion, "SteamMatchMaking004") == 0) {
        return (ISteamMatchmaking *)(void *)(ISteamMatchmaking004 *)steam_matchmaking;
    } else if (strcmp(pchVersion, "SteamMatchMaking005") == 0) {
        return (ISteamMatchmaking *)(void *)(ISteamMatchmaking005 *)steam_matchmaking;
    } else if (strcmp(pchVersion, "SteamMatchMaking006") == 0) {
        return (ISteamMatchmaking *)(void *)(ISteamMatchmaking006 *)steam_matchmaking;
    } else if (strcmp(pchVersion, "SteamMatchMaking007") == 0) {
        return (ISteamMatchmaking *)(void *)(ISteamMatchmaking007 *)steam_matchmaking;
    } else if (strcmp(pchVersion, "SteamMatchMaking008") == 0) {
        return (ISteamMatchmaking *)(void *)(ISteamMatchmaking008 *)steam_matchmaking;
    } else if (strcmp(pchVersion, STEAMMATCHMAKING_INTERFACE_VERSION) == 0) {
        return (ISteamMatchmaking *)(void *)(ISteamMatchmaking *)steam_matchmaking;
    } else {
        return (ISteamMatchmaking *)(void *)(ISteamMatchmaking *)steam_matchmaking;
    }


    return steam_matchmaking;
}

// returns the ISteamMatchmakingServers interface
ISteamMatchmakingServers *Steam_Client::GetISteamMatchmakingServers( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("GetISteamMatchmakingServers %s\n", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamUser) return NULL;

    if (strcmp(pchVersion, "SteamMatchMakingServers001") == 0) {
        return (ISteamMatchmakingServers *)(void *)(ISteamMatchmakingServers001 *)steam_matchmaking_servers;
    } else if (strcmp(pchVersion, STEAMMATCHMAKINGSERVERS_INTERFACE_VERSION) == 0) {
        return steam_matchmaking_servers;
    } else {
        return steam_matchmaking_servers;
    }

    return steam_matchmaking_servers;
}

// returns the a generic interface
void *Steam_Client::GetISteamGenericInterface( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("GetISteamGenericInterface %s\n", pchVersion);
    if (!steam_pipes.count(hSteamPipe)) return NULL;

    bool server = false;
    if (steam_pipes[hSteamPipe] == Steam_Pipe::SERVER) {
        server = true;
    } else {
        if ((strstr(pchVersion, "SteamNetworkingUtils") != pchVersion) && (strstr(pchVersion, "SteamUtils") != pchVersion)) {
            if (!hSteamUser) return NULL;
        }
    }

    if (strstr(pchVersion, "SteamNetworkingSocketsSerialized") == pchVersion) {
        Steam_Networking_Sockets_Serialized *steam_networking_sockets_serialized_temp;
        if (server) {
            steam_networking_sockets_serialized_temp = steam_gameserver_networking_sockets_serialized;
        } else {
            steam_networking_sockets_serialized_temp = steam_networking_sockets_serialized;
        }

        if (strcmp(pchVersion, "SteamNetworkingSocketsSerialized002") == 0) {
            return (void *)(ISteamNetworkingSocketsSerialized002 *)steam_networking_sockets_serialized_temp;
        } else if (strcmp(pchVersion, "SteamNetworkingSocketsSerialized003") == 0) {
            return (void *)(ISteamNetworkingSocketsSerialized003 *)steam_networking_sockets_serialized_temp;
        } else if (strcmp(pchVersion, "SteamNetworkingSocketsSerialized004") == 0) {
            return (void *)(ISteamNetworkingSocketsSerialized004 *)steam_networking_sockets_serialized_temp;
        } else if (strcmp(pchVersion, "SteamNetworkingSocketsSerialized005") == 0) {
            return (void *)(ISteamNetworkingSocketsSerialized005 *)steam_networking_sockets_serialized_temp;
        } else {
            return (void *)(ISteamNetworkingSocketsSerialized005 *)steam_networking_sockets_serialized_temp;
        }
    } else if (strstr(pchVersion, "SteamNetworkingSockets") == pchVersion) {
        Steam_Networking_Sockets *steam_networking_sockets_temp;
        if (server) {
            steam_networking_sockets_temp = steam_gameserver_networking_sockets;
        } else {
            steam_networking_sockets_temp = steam_networking_sockets;
        }

        if (strcmp(pchVersion, "SteamNetworkingSockets001") == 0) {
            return (void *)(ISteamNetworkingSockets001 *) steam_networking_sockets_temp;
        } else if (strcmp(pchVersion, "SteamNetworkingSockets002") == 0) {
            return (void *)(ISteamNetworkingSockets002 *) steam_networking_sockets_temp;
        } else if (strcmp(pchVersion, "SteamNetworkingSockets003") == 0) {
            return (void *)(ISteamNetworkingSockets003 *) steam_networking_sockets_temp;
        } else if (strcmp(pchVersion, "SteamNetworkingSockets004") == 0) {
            return (void *)(ISteamNetworkingSockets004 *) steam_networking_sockets_temp;
        } else if (strcmp(pchVersion, "SteamNetworkingSockets006") == 0) {
            return (void *)(ISteamNetworkingSockets006 *) steam_networking_sockets_temp;
        } else if (strcmp(pchVersion, "SteamNetworkingSockets008") == 0) {
            return (void *)(ISteamNetworkingSockets008 *) steam_networking_sockets_temp;
        } else if (strcmp(pchVersion, "SteamNetworkingSockets009") == 0) {
            return (void *)(ISteamNetworkingSockets009 *) steam_networking_sockets_temp;
        } else {
            return (void *)(ISteamNetworkingSockets *) steam_networking_sockets_temp;
        }
    } else if (strstr(pchVersion, "SteamNetworkingMessages") == pchVersion) {
        Steam_Networking_Messages *steam_networking_messages_temp;
        if (server) {
            steam_networking_messages_temp = steam_gameserver_networking_messages;
        } else {
            steam_networking_messages_temp = steam_networking_messages;
        }

        return (void *)(ISteamNetworkingMessages *)steam_networking_messages_temp;
    } else if (strstr(pchVersion, "SteamGameCoordinator") == pchVersion) {
        Steam_Game_Coordinator *steam_game_coordinator_temp;
        if (server) {
            steam_game_coordinator_temp = steam_gameserver_game_coordinator;
        } else {
            steam_game_coordinator_temp = steam_game_coordinator;
        }

        return (void *)(ISteamGameCoordinator *)steam_game_coordinator_temp;
    } else if (strstr(pchVersion, STEAMTV_INTERFACE_VERSION) == pchVersion) {
        return (void *)(ISteamTV *)steam_tv;
    } else if (strstr(pchVersion, "SteamNetworkingUtils") == pchVersion) {
            if (strcmp(pchVersion, "SteamNetworkingUtils001") == 0) {
                return (void *)(ISteamNetworkingUtils001 *)steam_networking_utils;
            } else if (strcmp(pchVersion, "SteamNetworkingUtils002") == 0) {
                return (void *)(ISteamNetworkingUtils002 *)steam_networking_utils;
            } else if (strcmp(pchVersion, "SteamNetworkingUtils003") == 0) {
                return (void *)(ISteamNetworkingUtils003 *)steam_networking_utils;
            } else if (strcmp(pchVersion, STEAMNETWORKINGUTILS_INTERFACE_VERSION) == 0) {
                return (void *)(ISteamNetworkingUtils *)steam_networking_utils;
            } else {
                return (void *)(ISteamNetworkingUtils *)steam_networking_utils;
            }
    } else if (strstr(pchVersion, "STEAMREMOTESTORAGE_INTERFACE_VERSION") == pchVersion) {
        return GetISteamRemoteStorage(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "SteamGameServerStats") == pchVersion) {
        return GetISteamGameServerStats(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "SteamMatchMakingServers") == pchVersion) {
        return GetISteamMatchmakingServers(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "SteamFriends") == pchVersion) {
        return GetISteamFriends(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "SteamMatchMaking") == pchVersion) {
        return GetISteamMatchmaking(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "SteamController") == pchVersion || strstr(pchVersion, "STEAMCONTROLLER_INTERFACE_VERSION") == pchVersion) {
        return GetISteamController(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "STEAMUGC_INTERFACE_VERSION") == pchVersion) {
        return GetISteamUGC(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "STEAMINVENTORY_INTERFACE") == pchVersion) {
        return GetISteamInventory(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "STEAMUSERSTATS_INTERFACE_VERSION") == pchVersion) {
        return GetISteamUserStats(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "SteamUser") == pchVersion) {
        return GetISteamUser(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "SteamNetworking") == pchVersion) {
        return GetISteamNetworking(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "SteamGameServer") == pchVersion) {
        return GetISteamGameServer(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "SteamUtils") == pchVersion) {
        return GetISteamUtils(hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "STEAMAPPS_INTERFACE_VERSION") == pchVersion) {
        return GetISteamApps(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "STEAMSCREENSHOTS_INTERFACE_VERSION") == pchVersion) {
        return GetISteamScreenshots(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "STEAMHTTP_INTERFACE_VERSION") == pchVersion) {
        return GetISteamHTTP(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "STEAMUNIFIEDMESSAGES_INTERFACE_VERSION") == pchVersion) {
        return DEPRECATED_GetISteamUnifiedMessages(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "STEAMAPPLIST_INTERFACE_VERSION") == pchVersion) {
        return GetISteamAppList(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "STEAMMUSIC_INTERFACE_VERSION") == pchVersion) {
        return GetISteamMusic(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "STEAMMUSICREMOTE_INTERFACE_VERSION") == pchVersion) {
        return GetISteamMusicRemote(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "STEAMHTMLSURFACE_INTERFACE_VERSION") == pchVersion) {
        return GetISteamHTMLSurface(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "STEAMVIDEO_INTERFACE") == pchVersion) {
        return GetISteamVideo(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "SteamMasterServerUpdater") == pchVersion) {
        return GetISteamMasterServerUpdater(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "SteamMatchGameSearch") == pchVersion) {
        return GetISteamGameSearch(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "SteamParties") == pchVersion) {
        return GetISteamParties(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "SteamInput") == pchVersion) {
        return GetISteamInput(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "STEAMREMOTEPLAY_INTERFACE_VERSION") == pchVersion) {
        return GetISteamRemotePlay(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "STEAMPARENTALSETTINGS_INTERFACE_VERSION") == pchVersion) {
        return GetISteamParentalSettings(hSteamUser, hSteamPipe, pchVersion);
    } else {
        PRINT_DEBUG("No interface: %s\n", pchVersion);
        //TODO: all the interfaces
        return NULL;
    }
}

// returns the ISteamUserStats interface
ISteamUserStats *Steam_Client::GetISteamUserStats( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("GetISteamUserStats %s\n", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamUser) return NULL;

    if (strcmp(pchVersion, "STEAMUSERSTATS_INTERFACE_VERSION001") == 0) {
        //TODO
        return (ISteamUserStats *)(void *)(ISteamUserStats003 *)steam_user_stats;
    } else if (strcmp(pchVersion, "STEAMUSERSTATS_INTERFACE_VERSION002") == 0) {
        //TODO
        return (ISteamUserStats *)(void *)(ISteamUserStats003 *)steam_user_stats;
    } else if (strcmp(pchVersion, "STEAMUSERSTATS_INTERFACE_VERSION003") == 0) {
        return (ISteamUserStats *)(void *)(ISteamUserStats003 *)steam_user_stats;
    } else if (strcmp(pchVersion, "STEAMUSERSTATS_INTERFACE_VERSION004") == 0) {
        return (ISteamUserStats *)(void *)(ISteamUserStats004 *)steam_user_stats;
    } else if (strcmp(pchVersion, "STEAMUSERSTATS_INTERFACE_VERSION005") == 0) {
        return (ISteamUserStats *)(void *)(ISteamUserStats005 *)steam_user_stats;
    } else if (strcmp(pchVersion, "STEAMUSERSTATS_INTERFACE_VERSION006") == 0) {
        return (ISteamUserStats *)(void *)(ISteamUserStats006 *)steam_user_stats;
    } else if (strcmp(pchVersion, "STEAMUSERSTATS_INTERFACE_VERSION007") == 0) {
        return (ISteamUserStats *)(void *)(ISteamUserStats007 *)steam_user_stats;
    } else if (strcmp(pchVersion, "STEAMUSERSTATS_INTERFACE_VERSION008") == 0) {
        return (ISteamUserStats *)(void *)(ISteamUserStats008 *)steam_user_stats;
    } else if (strcmp(pchVersion, "STEAMUSERSTATS_INTERFACE_VERSION009") == 0) {
        return (ISteamUserStats *)(void *)(ISteamUserStats009 *)steam_user_stats;
    } else if (strcmp(pchVersion, "STEAMUSERSTATS_INTERFACE_VERSION010") == 0) {
        return (ISteamUserStats *)(void *)(ISteamUserStats010 *)steam_user_stats;
    } else if (strcmp(pchVersion, "STEAMUSERSTATS_INTERFACE_VERSION011") == 0) {
        return (ISteamUserStats *)(void *)(ISteamUserStats011 *)steam_user_stats;
    } else if (strcmp(pchVersion, STEAMUSERSTATS_INTERFACE_VERSION) == 0) {
        return (ISteamUserStats *)(void *)(ISteamUserStats *)steam_user_stats;
    } else {
        return (ISteamUserStats *)(void *)(ISteamUserStats *)steam_user_stats;
    }

    return steam_user_stats;
}

// returns the ISteamGameServerStats interface
ISteamGameServerStats *Steam_Client::GetISteamGameServerStats( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("GetISteamGameServerStats %s\n", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamuser) return NULL;
    return steam_gameserverstats;
}

// returns apps interface
ISteamApps *Steam_Client::GetISteamApps( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("GetISteamApps %s\n", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamUser) return NULL;

    Steam_Apps *steam_apps_temp;

    if (steam_pipes[hSteamPipe] == Steam_Pipe::SERVER) {
        steam_apps_temp = steam_gameserver_apps;
    } else {
        steam_apps_temp = steam_apps;
    }
    if (strcmp(pchVersion, "STEAMAPPS_INTERFACE_VERSION001") == 0) {
        return (ISteamApps *)(void *)(ISteamApps001 *)steam_apps_temp;
    } else if (strcmp(pchVersion, "STEAMAPPS_INTERFACE_VERSION002") == 0) {
        return (ISteamApps *)(void *)(ISteamApps002 *)steam_apps_temp;
    } else if (strcmp(pchVersion, "STEAMAPPS_INTERFACE_VERSION003") == 0) {
        return (ISteamApps *)(void *)(ISteamApps003 *)steam_apps_temp;
    } else if (strcmp(pchVersion, "STEAMAPPS_INTERFACE_VERSION004") == 0) {
        return (ISteamApps *)(void *)(ISteamApps004 *)steam_apps_temp;
    } else if (strcmp(pchVersion, "STEAMAPPS_INTERFACE_VERSION005") == 0) {
        return (ISteamApps *)(void *)(ISteamApps005 *)steam_apps_temp;
    } else if (strcmp(pchVersion, "STEAMAPPS_INTERFACE_VERSION006") == 0) {
        return (ISteamApps *)(void *)(ISteamApps006 *)steam_apps_temp;
    } else if (strcmp(pchVersion, "STEAMAPPS_INTERFACE_VERSION007") == 0) {
        return (ISteamApps *)(void *)(ISteamApps007 *)steam_apps_temp;
    } else if (strcmp(pchVersion, STEAMAPPS_INTERFACE_VERSION) == 0) {
        return (ISteamApps *)(void *)(ISteamApps *)steam_apps_temp;
    } else {
        return (ISteamApps *)(void *)(ISteamApps *)steam_apps_temp;
    }

    return steam_apps;
}

// networking
ISteamNetworking *Steam_Client::GetISteamNetworking( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("GetISteamNetworking %s\n", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamUser) return NULL;

    Steam_Networking *steam_networking_temp;

    if (steam_pipes[hSteamPipe] == Steam_Pipe::SERVER) {
        steam_networking_temp = steam_gameserver_networking;
    } else {
        steam_networking_temp = steam_networking;
    }

    if (strcmp(pchVersion, "SteamNetworking001") == 0) {
        return (ISteamNetworking *)(void *)(ISteamNetworking001 *)steam_networking_temp;
    } else if (strcmp(pchVersion, "SteamNetworking002") == 0) {
        return (ISteamNetworking *)(void *)(ISteamNetworking002 *)steam_networking_temp;
    } else if (strcmp(pchVersion, "SteamNetworking003") == 0) {
        return (ISteamNetworking *)(void *)(ISteamNetworking003 *)steam_networking_temp;
    } else if (strcmp(pchVersion, "SteamNetworking004") == 0) {
        return (ISteamNetworking *)(void *)(ISteamNetworking004 *)steam_networking_temp;
    } else if (strcmp(pchVersion, "SteamNetworking005") == 0) {
        return (ISteamNetworking *)(void *)(ISteamNetworking005 *)steam_networking_temp;
    } else if (strcmp(pchVersion, STEAMNETWORKING_INTERFACE_VERSION) == 0) {
        return (ISteamNetworking *)(void *)(ISteamNetworking *)steam_networking_temp;
    } else {
        return (ISteamNetworking *)(void *)(ISteamNetworking *)steam_networking_temp;
    }

    return steam_networking_temp;
}

// remote storage
ISteamRemoteStorage *Steam_Client::GetISteamRemoteStorage( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("GetISteamRemoteStorage %s\n", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamuser) return NULL;

    if (strcmp(pchVersion, "STEAMREMOTESTORAGE_INTERFACE_VERSION001") == 0) {
        return (ISteamRemoteStorage *)(void *)(ISteamRemoteStorage001 *)steam_remote_storage;
    } else if (strcmp(pchVersion, "STEAMREMOTESTORAGE_INTERFACE_VERSION002") == 0) {
        return (ISteamRemoteStorage *)(void *)(ISteamRemoteStorage002 *)steam_remote_storage;
    } else if (strcmp(pchVersion, "STEAMREMOTESTORAGE_INTERFACE_VERSION003") == 0) {
        return (ISteamRemoteStorage *)(void *)(ISteamRemoteStorage003 *)steam_remote_storage;
    } else if (strcmp(pchVersion, "STEAMREMOTESTORAGE_INTERFACE_VERSION004") == 0) {
        return (ISteamRemoteStorage *)(void *)(ISteamRemoteStorage004 *)steam_remote_storage;
    } else if (strcmp(pchVersion, "STEAMREMOTESTORAGE_INTERFACE_VERSION005") == 0) {
        return (ISteamRemoteStorage *)(void *)(ISteamRemoteStorage005 *)steam_remote_storage;
    } else if (strcmp(pchVersion, "STEAMREMOTESTORAGE_INTERFACE_VERSION006") == 0) {
        return (ISteamRemoteStorage *)(void *)(ISteamRemoteStorage006 *)steam_remote_storage;
    } else if (strcmp(pchVersion, "STEAMREMOTESTORAGE_INTERFACE_VERSION007") == 0) {
        return (ISteamRemoteStorage *)(void *)(ISteamRemoteStorage007 *)steam_remote_storage;
    } else if (strcmp(pchVersion, "STEAMREMOTESTORAGE_INTERFACE_VERSION008") == 0) {
        return (ISteamRemoteStorage *)(void *)(ISteamRemoteStorage008 *)steam_remote_storage;
    } else if (strcmp(pchVersion, "STEAMREMOTESTORAGE_INTERFACE_VERSION009") == 0) {
        return (ISteamRemoteStorage *)(void *)(ISteamRemoteStorage009 *)steam_remote_storage;
    } else if (strcmp(pchVersion, "STEAMREMOTESTORAGE_INTERFACE_VERSION010") == 0) {
        return (ISteamRemoteStorage *)(void *)(ISteamRemoteStorage010 *)steam_remote_storage;
    } else if (strcmp(pchVersion, "STEAMREMOTESTORAGE_INTERFACE_VERSION011") == 0) {
        return (ISteamRemoteStorage *)(void *)(ISteamRemoteStorage011 *)steam_remote_storage;
    } else if (strcmp(pchVersion, "STEAMREMOTESTORAGE_INTERFACE_VERSION012") == 0) {
        return (ISteamRemoteStorage *)(void *)(ISteamRemoteStorage012 *)steam_remote_storage;
    } else if (strcmp(pchVersion, "STEAMREMOTESTORAGE_INTERFACE_VERSION013") == 0) {
        return (ISteamRemoteStorage *)(void *)(ISteamRemoteStorage013 *)steam_remote_storage;
    } else if (strcmp(pchVersion, "STEAMREMOTESTORAGE_INTERFACE_VERSION014") == 0) {
        return (ISteamRemoteStorage *)(void *)(ISteamRemoteStorage014 *)steam_remote_storage;
    } else if (strcmp(pchVersion, STEAMREMOTESTORAGE_INTERFACE_VERSION) == 0) {
        return (ISteamRemoteStorage *)(void *)(ISteamRemoteStorage *)steam_remote_storage;
    } else {
        return (ISteamRemoteStorage *)(void *)(ISteamRemoteStorage *)steam_remote_storage;
    }

    return steam_remote_storage;
}

// user screenshots
ISteamScreenshots *Steam_Client::GetISteamScreenshots( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("GetISteamScreenshots %s\n", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamuser) return NULL;
    return steam_screenshots;
}


// Deprecated. Applications should use SteamAPI_RunCallbacks() or SteamGameServer_RunCallbacks() instead.
void Steam_Client::RunFrame()
{
    PRINT_DEBUG("Steam_Client::RunFrame\n");
}

// returns the number of IPC calls made since the last time this function was called
// Used for perf debugging so you can understand how many IPC calls your game makes per frame
// Every IPC call is at minimum a thread context switch if not a process one so you want to rate
// control how often you do them.
uint32 Steam_Client::GetIPCCallCount()
{
    PRINT_DEBUG("Steam_Client::GetIPCCallCount\n");
    return steam_utils->GetIPCCallCount();
}

// API warning handling
// 'int' is the severity; 0 for msg, 1 for warning
// 'const char *' is the text of the message
// callbacks will occur directly after the API function is called that generated the warning or message.
void Steam_Client::SetWarningMessageHook( SteamAPIWarningMessageHook_t pFunction )
{
    PRINT_DEBUG("Steam_Client::SetWarningMessageHook\n");
}

// Trigger global shutdown for the DLL
bool Steam_Client::BShutdownIfAllPipesClosed()
{
    PRINT_DEBUG("BShutdownIfAllPipesClosed\n");
    if (!steam_pipes.size()) {
        bool joinable = background_keepalive.joinable();
        if (joinable) {
            kill_background_thread_mutex.lock();
            kill_background_thread = true;
            kill_background_thread_mutex.unlock();
            kill_background_thread_cv.notify_one();
        }

        steam_controller->Shutdown();
#ifdef EMU_OVERLAY
    if(!settings_client->disable_overlay)
        steam_overlay->UnSetupOverlay();
#endif

        if (joinable) {
            background_keepalive.join();
        }

        PRINT_DEBUG("all pipes closed\n");
        return true;
    }

    return false;
}

// Expose HTTP interface
ISteamHTTP *Steam_Client::GetISteamHTTP( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("GetISteamHTTP %s\n", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamuser) return NULL;
    Steam_HTTP *steam_http_temp;

    if (steam_pipes[hSteamPipe] == Steam_Pipe::SERVER) {
        steam_http_temp = steam_gameserver_http;
    } else {
        steam_http_temp = steam_http;
    }

    if (strcmp(pchVersion, "STEAMHTTP_INTERFACE_VERSION001") == 0) {
        return (ISteamHTTP *)(void *)(ISteamHTTP001 *)steam_http_temp;
    } else if (strcmp(pchVersion, "STEAMHTTP_INTERFACE_VERSION002") == 0) {
        return (ISteamHTTP *)(void *)(ISteamHTTP002 *)steam_http_temp;
    } else if (strcmp(pchVersion, STEAMHTTP_INTERFACE_VERSION) == 0) {
        return (ISteamHTTP *)(void *)(ISteamHTTP *)steam_http_temp;
    } else {
        return (ISteamHTTP *)(void *)(ISteamHTTP *)steam_http_temp;
    }

    return steam_http_temp;
}

// Deprecated - the ISteamUnifiedMessages interface is no longer intended for public consumption.
void *Steam_Client::DEPRECATED_GetISteamUnifiedMessages( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion ) 
{
    PRINT_DEBUG("DEPRECATED_GetISteamUnifiedMessages %s\n", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamuser) return NULL;
    return (void *)(ISteamUnifiedMessages *)steam_unified_messages;
}

ISteamUnifiedMessages *Steam_Client::GetISteamUnifiedMessages( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("GetISteamUnifiedMessages %s\n", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamuser) return NULL;
    return steam_unified_messages;
}

// Exposes the ISteamController interface
ISteamController *Steam_Client::GetISteamController( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("GetISteamController %s\n", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamUser) return NULL;

    if (strcmp(pchVersion, "STEAMCONTROLLER_INTERFACE_VERSION") == 0) {
        return (ISteamController *)(void *)(ISteamController001 *)steam_controller;
    } else if (strcmp(pchVersion, "STEAMCONTROLLER_INTERFACE_VERSION_002") == 0) {
        //I'm pretty sure this interface is never actually used
        return (ISteamController *)(void *)(ISteamController003 *)steam_controller;
    } else if (strcmp(pchVersion, "SteamController003") == 0) {
        return (ISteamController *)(void *)(ISteamController003 *)steam_controller;
    } else if (strcmp(pchVersion, "SteamController004") == 0) {
        return (ISteamController *)(void *)(ISteamController004 *)steam_controller;
    } else if (strcmp(pchVersion, "SteamController005") == 0) {
        return (ISteamController *)(void *)(ISteamController005 *)steam_controller;
    } else if (strcmp(pchVersion, "SteamController006") == 0) {
        return (ISteamController *)(void *)(ISteamController006 *)steam_controller;
    } else if (strcmp(pchVersion, "SteamController007") == 0) {
        return (ISteamController *)(void *)(ISteamController007 *)steam_controller;
    } else if (strcmp(pchVersion, STEAMCONTROLLER_INTERFACE_VERSION) == 0) {
        return (ISteamController *)(void *)(ISteamController *)steam_controller;
    } else {
        return (ISteamController *)(void *)(ISteamController *)steam_controller;
    }

    return steam_controller;
}

// Exposes the ISteamUGC interface
ISteamUGC *Steam_Client::GetISteamUGC( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("GetISteamUGC %s\n", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamUser) return NULL;
    Steam_UGC *steam_ugc_temp;

    if (steam_pipes[hSteamPipe] == Steam_Pipe::SERVER) {
        steam_ugc_temp = steam_gameserver_ugc;
    } else {
        steam_ugc_temp = steam_ugc;
    }

    if (strcmp(pchVersion, "STEAMUGC_INTERFACE_VERSION") == 0) {
        //Is this actually a valid interface version?
        return (ISteamUGC *)(void *)(ISteamUGC001 *)steam_ugc_temp;
    } else if (strcmp(pchVersion, "STEAMUGC_INTERFACE_VERSION001") == 0) {
        return (ISteamUGC *)(void *)(ISteamUGC001 *)steam_ugc_temp;
    } else if (strcmp(pchVersion, "STEAMUGC_INTERFACE_VERSION002") == 0) {
        return (ISteamUGC *)(void *)(ISteamUGC002 *)steam_ugc_temp;
    } else if (strcmp(pchVersion, "STEAMUGC_INTERFACE_VERSION003") == 0) {
        return (ISteamUGC *)(void *)(ISteamUGC003 *)steam_ugc_temp;
    } else if (strcmp(pchVersion, "STEAMUGC_INTERFACE_VERSION004") == 0) {
        return (ISteamUGC *)(void *)(ISteamUGC004 *)steam_ugc_temp;
    } else if (strcmp(pchVersion, "STEAMUGC_INTERFACE_VERSION005") == 0) {
        return (ISteamUGC *)(void *)(ISteamUGC005 *)steam_ugc_temp;
    } else if (strcmp(pchVersion, "STEAMUGC_INTERFACE_VERSION006") == 0) {
        return (ISteamUGC *)(void *)(ISteamUGC006 *)steam_ugc_temp;
    } else if (strcmp(pchVersion, "STEAMUGC_INTERFACE_VERSION007") == 0) {
        return (ISteamUGC *)(void *)(ISteamUGC007 *)steam_ugc_temp;
    } else if (strcmp(pchVersion, "STEAMUGC_INTERFACE_VERSION008") == 0) {
        return (ISteamUGC *)(void *)(ISteamUGC008 *)steam_ugc_temp;
    } else if (strcmp(pchVersion, "STEAMUGC_INTERFACE_VERSION009") == 0) {
        return (ISteamUGC *)(void *)(ISteamUGC009 *)steam_ugc_temp;
    } else if (strcmp(pchVersion, "STEAMUGC_INTERFACE_VERSION010") == 0) {
        return (ISteamUGC *)(void *)(ISteamUGC010 *)steam_ugc_temp;
    } else if (strcmp(pchVersion, "STEAMUGC_INTERFACE_VERSION011") == 0) {
        //TODO ?
        return (ISteamUGC *)(void *)(ISteamUGC012 *)steam_ugc_temp;
    } else if (strcmp(pchVersion, "STEAMUGC_INTERFACE_VERSION012") == 0) {
        return (ISteamUGC *)(void *)(ISteamUGC012 *)steam_ugc_temp;
    } else if (strcmp(pchVersion, "STEAMUGC_INTERFACE_VERSION013") == 0) {
        return (ISteamUGC *)(void *)(ISteamUGC013 *)steam_ugc_temp;
    } else if (strcmp(pchVersion, "STEAMUGC_INTERFACE_VERSION014") == 0) {
        return (ISteamUGC *)(void *)(ISteamUGC014 *)steam_ugc_temp;
    } else if (strcmp(pchVersion, "STEAMUGC_INTERFACE_VERSION015") == 0) {
        return (ISteamUGC *)(void *)(ISteamUGC015 *)steam_ugc_temp;
    } else if (strcmp(pchVersion, "STEAMUGC_INTERFACE_VERSION016") == 0) {
        return (ISteamUGC *)(void *)(ISteamUGC016 *)steam_ugc_temp;
    } else if (strcmp(pchVersion, "STEAMUGC_INTERFACE_VERSION017") == 0) {
        return (ISteamUGC *)(void *)(ISteamUGC017 *)steam_ugc_temp;
    } else if (strcmp(pchVersion, STEAMUGC_INTERFACE_VERSION) == 0) {
        return (ISteamUGC *)(void *)(ISteamUGC *)steam_ugc_temp;
    } else {
        return (ISteamUGC *)(void *)(ISteamUGC *)steam_ugc_temp;
    }

    return steam_ugc_temp;
}

// returns app list interface, only available on specially registered apps
ISteamAppList *Steam_Client::GetISteamAppList( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("GetISteamAppList %s\n", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamUser) return NULL;
    return steam_applist;
}

// Music Player
ISteamMusic *Steam_Client::GetISteamMusic( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("GetISteamMusic %s\n", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamuser) return NULL;
    return steam_music;
}

// Music Player Remote
ISteamMusicRemote *Steam_Client::GetISteamMusicRemote(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion)
{
    PRINT_DEBUG("GetISteamMusicRemote %s\n", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamuser) return NULL;
    return steam_musicremote;
}

// html page display
ISteamHTMLSurface *Steam_Client::GetISteamHTMLSurface(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion)
{
    PRINT_DEBUG("GetISteamHTMLSurface %s\n", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamuser) return NULL;

    if (strcmp(pchVersion, "STEAMHTMLSURFACE_INTERFACE_VERSION_001") == 0) {
        return (ISteamHTMLSurface *)(void *)(ISteamHTMLSurface001 *)steam_HTMLsurface;
    } else if (strcmp(pchVersion, "STEAMHTMLSURFACE_INTERFACE_VERSION_002") == 0) {
        return (ISteamHTMLSurface *)(void *)(ISteamHTMLSurface002 *)steam_HTMLsurface;
    } else if (strcmp(pchVersion, "STEAMHTMLSURFACE_INTERFACE_VERSION_003") == 0) {
        return (ISteamHTMLSurface *)(void *)(ISteamHTMLSurface003 *)steam_HTMLsurface;
    } else if (strcmp(pchVersion, "STEAMHTMLSURFACE_INTERFACE_VERSION_004") == 0) {
        return (ISteamHTMLSurface *)(void *)(ISteamHTMLSurface004 *)steam_HTMLsurface;
    } else if (strcmp(pchVersion, STEAMHTMLSURFACE_INTERFACE_VERSION) == 0) {
        return (ISteamHTMLSurface *)(void *)(ISteamHTMLSurface *)steam_HTMLsurface;
    } else {
        return (ISteamHTMLSurface *)(void *)(ISteamHTMLSurface *)steam_HTMLsurface;
    }

    return steam_HTMLsurface;
}

// Helper functions for internal Steam usage
void Steam_Client::DEPRECATED_Set_SteamAPI_CPostAPIResultInProcess( void (*)() )
{
    PRINT_DEBUG("DEPRECATED_Set_SteamAPI_CPostAPIResultInProcess\n");
}

void Steam_Client::DEPRECATED_Remove_SteamAPI_CPostAPIResultInProcess( void (*)() )
{
    PRINT_DEBUG("DEPRECATED_Remove_SteamAPI_CPostAPIResultInProcess\n");
}

void Steam_Client::Set_SteamAPI_CCheckCallbackRegisteredInProcess( SteamAPI_CheckCallbackRegistered_t func )
{
    PRINT_DEBUG("Set_SteamAPI_CCheckCallbackRegisteredInProcess\n");
}

void Steam_Client::Set_SteamAPI_CPostAPIResultInProcess( SteamAPI_PostAPIResultInProcess_t func )
{
    PRINT_DEBUG("Set_SteamAPI_CPostAPIResultInProcess\n");
}

void Steam_Client::Remove_SteamAPI_CPostAPIResultInProcess( SteamAPI_PostAPIResultInProcess_t func )
{
    PRINT_DEBUG("Remove_SteamAPI_CPostAPIResultInProcess\n");
}

// inventory
ISteamInventory *Steam_Client::GetISteamInventory( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("GetISteamInventory %s\n", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamuser) return NULL;
    Steam_Inventory *steam_inventory_temp;
    Settings *settings_temp;
    SteamCallBacks *callbacks_temp;
    SteamCallResults *callback_results_temp;

    if (steam_pipes[hSteamPipe] == Steam_Pipe::SERVER) {
        steam_inventory_temp = steam_gameserver_inventory;
    } else {
        steam_inventory_temp = steam_inventory;
    }

    if (strcmp(pchVersion, "STEAMINVENTORY_INTERFACE_V001") == 0) {
        return (ISteamInventory *)(void *)(ISteamInventory001 *)steam_inventory_temp;
    } else if (strcmp(pchVersion, "STEAMINVENTORY_INTERFACE_V002") == 0) {
        return (ISteamInventory *)(void *)(ISteamInventory002 *)steam_inventory_temp;
    } else if (strcmp(pchVersion, STEAMINVENTORY_INTERFACE_VERSION) == 0) {
        return (ISteamInventory *)(void *)(ISteamInventory *)steam_inventory_temp;
    } else {
        return (ISteamInventory *)(void *)(ISteamInventory *)steam_inventory_temp;
    }

    return steam_inventory_temp;
}

// Video
ISteamVideo *Steam_Client::GetISteamVideo( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("GetISteamVideo %s\n", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamuser) return NULL;
    return steam_video;
}

// Parental controls
ISteamParentalSettings *Steam_Client::GetISteamParentalSettings( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("GetISteamParentalSettings %s\n", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamuser) return NULL;
    return steam_parental;
}

ISteamMasterServerUpdater *Steam_Client::GetISteamMasterServerUpdater( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("GetISteamMasterServerUpdater %s\n", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamUser) return NULL;
    return steam_masterserver_updater;
}

ISteamContentServer *Steam_Client::GetISteamContentServer( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("GetISteamContentServer %s\n", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamUser) return NULL;
    return NULL;
}

// game search
ISteamGameSearch *Steam_Client::GetISteamGameSearch( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("GetISteamGameSearch %s\n", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamuser) return NULL;

    return steam_game_search;
}

// Exposes the Steam Input interface for controller support
ISteamInput *Steam_Client::GetISteamInput( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("GetISteamInput %s\n", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamUser) return NULL;

    if (strcmp(pchVersion, "SteamInput001") == 0) {
        return (ISteamInput *)(void *)(ISteamInput001 *)steam_controller;
    } else if (strcmp(pchVersion, "SteamInput002") == 0) {
        return (ISteamInput *)(void *)(ISteamInput002 *)steam_controller;
    } else if (strcmp(pchVersion, "SteamInput005") == 0) {
        return (ISteamInput *)(void *)(ISteamInput005 *)steam_controller;
    } else if (strcmp(pchVersion, STEAMINPUT_INTERFACE_VERSION) == 0) {
        return (ISteamInput *)(void *)(ISteamInput *)steam_controller;
    } else {
        return (ISteamInput *)(void *)(ISteamInput *)steam_controller;
    }

    return steam_controller;
}

// Steam Parties interface
ISteamParties *Steam_Client::GetISteamParties( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("GetISteamParties %s\n", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamUser) return NULL;

    return steam_parties;
}

ISteamRemotePlay *Steam_Client::GetISteamRemotePlay( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("GetISteamRemotePlay %s\n", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamUser) return NULL;

    if (strcmp(pchVersion, "STEAMREMOTEPLAY_INTERFACE_VERSION001") == 0) {
        return (ISteamRemotePlay *)(void *)(ISteamRemotePlay001 *)steam_remoteplay;
    } else if (strcmp(pchVersion, STEAMREMOTEPLAY_INTERFACE_VERSION) == 0) {
        return (ISteamRemotePlay *)(void *)(ISteamRemotePlay *)steam_remoteplay;
    } else {
        return (ISteamRemotePlay *)(void *)(ISteamRemotePlay *)steam_remoteplay;
    }

    return steam_remoteplay;
}

void Steam_Client::RegisterCallback( class CCallbackBase *pCallback, int iCallback)
{
    int base_callback = (iCallback / 100) * 100;
    int callback_id = iCallback % 100;
    bool isGameServer = CCallbackMgr::isServer(pCallback);
    PRINT_DEBUG("isGameServer %u\n", isGameServer);

    switch (base_callback) {
        case k_iSteamUserCallbacks:
            PRINT_DEBUG("k_iSteamUserCallbacks %i\n", callback_id);
            break;

        case k_iSteamGameServerCallbacks:
            PRINT_DEBUG("k_iSteamGameServerCallbacks %i\n", callback_id);
            break;

        case k_iSteamFriendsCallbacks:
            PRINT_DEBUG("k_iSteamFriendsCallbacks %i\n", callback_id);
            break;

        case k_iSteamBillingCallbacks:
            PRINT_DEBUG("k_iSteamBillingCallbacks %i\n", callback_id);
            break;

        case k_iSteamMatchmakingCallbacks:
            PRINT_DEBUG("k_iSteamMatchmakingCallbacks %i\n", callback_id);
            break;

        case k_iSteamContentServerCallbacks:
            PRINT_DEBUG("k_iSteamContentServerCallbacks %i\n", callback_id);
            break;

        case k_iSteamUtilsCallbacks:
            PRINT_DEBUG("k_iSteamUtilsCallbacks %i\n", callback_id);
            break;

        case k_iClientFriendsCallbacks:
            PRINT_DEBUG("k_iClientFriendsCallbacks %i\n", callback_id);
            break;

        case k_iClientUserCallbacks:
            PRINT_DEBUG("k_iClientUserCallbacks %i\n", callback_id);
            break;

        case k_iSteamAppsCallbacks:
            PRINT_DEBUG("k_iSteamAppsCallbacks %i\n", callback_id);
            break;

        case k_iSteamUserStatsCallbacks:
            PRINT_DEBUG("k_iSteamUserStatsCallbacks %i\n", callback_id);
            break;

        case k_iSteamNetworkingCallbacks:
            PRINT_DEBUG("k_iSteamNetworkingCallbacks %i\n", callback_id);
            break;

        case k_iClientRemoteStorageCallbacks:
            PRINT_DEBUG("k_iClientRemoteStorageCallbacks %i\n", callback_id);
            break;

        case k_iClientDepotBuilderCallbacks:
            PRINT_DEBUG("k_iClientDepotBuilderCallbacks %i\n", callback_id);
            break;

        case k_iSteamGameServerItemsCallbacks:
            PRINT_DEBUG("k_iSteamGameServerItemsCallbacks %i\n", callback_id);
            break;

        case k_iClientUtilsCallbacks:
            PRINT_DEBUG("k_iClientUtilsCallbacks %i\n", callback_id);
            break;

        case k_iSteamGameCoordinatorCallbacks:
            PRINT_DEBUG("k_iSteamGameCoordinatorCallbacks %i\n", callback_id);
            break;

        case k_iSteamGameServerStatsCallbacks:
            PRINT_DEBUG("k_iSteamGameServerStatsCallbacks %i\n", callback_id);
            break;

        case k_iSteam2AsyncCallbacks:
            PRINT_DEBUG("k_iSteam2AsyncCallbacks %i\n", callback_id);
            break;

        case k_iSteamGameStatsCallbacks:
            PRINT_DEBUG("k_iSteamGameStatsCallbacks %i\n", callback_id);
            break;

        case k_iClientHTTPCallbacks:
            PRINT_DEBUG("k_iClientHTTPCallbacks %i\n", callback_id);
            break;

        case k_iClientScreenshotsCallbacks:
            PRINT_DEBUG("k_iClientScreenshotsCallbacks %i\n", callback_id);
            break;

        case k_iSteamScreenshotsCallbacks:
            PRINT_DEBUG("k_iSteamScreenshotsCallbacks %i\n", callback_id);
            break;

        case k_iClientAudioCallbacks:
            PRINT_DEBUG("k_iClientAudioCallbacks %i\n", callback_id);
            break;

        case k_iClientUnifiedMessagesCallbacks:
            PRINT_DEBUG("k_iClientUnifiedMessagesCallbacks %i\n", callback_id);
            break;

        case k_iSteamStreamLauncherCallbacks:
            PRINT_DEBUG("k_iSteamStreamLauncherCallbacks %i\n", callback_id);
            break;

        case k_iClientControllerCallbacks:
            PRINT_DEBUG("k_iClientControllerCallbacks %i\n", callback_id);
            break;

        case k_iSteamControllerCallbacks:
            PRINT_DEBUG("k_iSteamControllerCallbacks %i\n", callback_id);
            break;

        case k_iClientParentalSettingsCallbacks:
            PRINT_DEBUG("k_iClientParentalSettingsCallbacks %i\n", callback_id);
            break;

        case k_iClientDeviceAuthCallbacks:
            PRINT_DEBUG("k_iClientDeviceAuthCallbacks %i\n", callback_id);
            break;

        case k_iClientNetworkDeviceManagerCallbacks:
            PRINT_DEBUG("k_iClientNetworkDeviceManagerCallbacks %i\n", callback_id);
            break;

        case k_iClientMusicCallbacks:
            PRINT_DEBUG("k_iClientMusicCallbacks %i\n", callback_id);
            break;

        case k_iClientRemoteClientManagerCallbacks:
            PRINT_DEBUG("k_iClientRemoteClientManagerCallbacks %i\n", callback_id);
            break;

        case k_iClientUGCCallbacks:
            PRINT_DEBUG("k_iClientUGCCallbacks %i\n", callback_id);
            break;

        case k_iSteamStreamClientCallbacks:
            PRINT_DEBUG("k_iSteamStreamClientCallbacks %i\n", callback_id);
            break;

        case k_IClientProductBuilderCallbacks:
            PRINT_DEBUG("k_IClientProductBuilderCallbacks %i\n", callback_id);
            break;

        case k_iClientShortcutsCallbacks:
            PRINT_DEBUG("k_iClientShortcutsCallbacks %i\n", callback_id);
            break;

        case k_iClientRemoteControlManagerCallbacks:
            PRINT_DEBUG("k_iClientRemoteControlManagerCallbacks %i\n", callback_id);
            break;

        case k_iSteamAppListCallbacks:
            PRINT_DEBUG("k_iSteamAppListCallbacks %i\n", callback_id);
            break;

        case k_iSteamMusicCallbacks:
            PRINT_DEBUG("k_iSteamMusicCallbacks %i\n", callback_id);
            break;

        case k_iSteamMusicRemoteCallbacks:
            PRINT_DEBUG("k_iSteamMusicRemoteCallbacks %i\n", callback_id);
            break;

        case k_iClientVRCallbacks:
            PRINT_DEBUG("k_iClientVRCallbacks %i\n", callback_id);
            break;

        case k_iClientGameNotificationCallbacks:
            PRINT_DEBUG("k_iClientGameNotificationCallbacks %i\n", callback_id);
            break;
 
        case k_iSteamGameNotificationCallbacks:
            PRINT_DEBUG("k_iSteamGameNotificationCallbacks %i\n", callback_id);
            break;
 
        case k_iSteamHTMLSurfaceCallbacks:
            PRINT_DEBUG("k_iSteamHTMLSurfaceCallbacks %i\n", callback_id);
            break;

        case k_iClientVideoCallbacks:
            PRINT_DEBUG("k_iClientVideoCallbacks %i\n", callback_id);
            break;

        case k_iClientInventoryCallbacks:
            PRINT_DEBUG("k_iClientInventoryCallbacks %i\n", callback_id);
            break;

        case k_iClientBluetoothManagerCallbacks:
            PRINT_DEBUG("k_iClientBluetoothManagerCallbacks %i\n", callback_id);
            break;

        case k_iClientSharedConnectionCallbacks:
            PRINT_DEBUG("k_iClientSharedConnectionCallbacks %i\n", callback_id);
            break;

        case k_ISteamParentalSettingsCallbacks:
            PRINT_DEBUG("k_ISteamParentalSettingsCallbacks %i\n", callback_id);
            break;

        case k_iClientShaderCallbacks:
            PRINT_DEBUG("k_iClientShaderCallbacks %i\n", callback_id);
            break;
        
        default:
            PRINT_DEBUG("Unknown callback base %i\n", base_callback);
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
    PRINT_DEBUG("isGameServer %u\n", isGameServer);

    switch (base_callback) {
        case k_iSteamUserCallbacks:
            PRINT_DEBUG("k_iSteamUserCallbacks %i\n", callback_id);
            break;

        case k_iSteamGameServerCallbacks:
            PRINT_DEBUG("k_iSteamGameServerCallbacks %i\n", callback_id);
            break;

        case k_iSteamFriendsCallbacks:
            PRINT_DEBUG("k_iSteamFriendsCallbacks %i\n", callback_id);
            break;

        case k_iSteamBillingCallbacks:
            PRINT_DEBUG("k_iSteamBillingCallbacks %i\n", callback_id);
            break;

        case k_iSteamMatchmakingCallbacks:
            PRINT_DEBUG("k_iSteamMatchmakingCallbacks %i\n", callback_id);
            break;

        case k_iSteamContentServerCallbacks:
            PRINT_DEBUG("k_iSteamContentServerCallbacks %i\n", callback_id);
            break;

        case k_iSteamUtilsCallbacks:
            PRINT_DEBUG("k_iSteamUtilsCallbacks %i\n", callback_id);
            break;

        case k_iClientFriendsCallbacks:
            PRINT_DEBUG("k_iClientFriendsCallbacks %i\n", callback_id);
            break;

        case k_iClientUserCallbacks:
            PRINT_DEBUG("k_iClientUserCallbacks %i\n", callback_id);
            break;

        case k_iSteamAppsCallbacks:
            PRINT_DEBUG("k_iSteamAppsCallbacks %i\n", callback_id);
            break;

        case k_iSteamUserStatsCallbacks:
            PRINT_DEBUG("k_iSteamUserStatsCallbacks %i\n", callback_id);
            break;

        case k_iSteamNetworkingCallbacks:
            PRINT_DEBUG("k_iSteamNetworkingCallbacks %i\n", callback_id);
            break;

        case k_iClientRemoteStorageCallbacks:
            PRINT_DEBUG("k_iClientRemoteStorageCallbacks %i\n", callback_id);
            break;

        case k_iClientDepotBuilderCallbacks:
            PRINT_DEBUG("k_iClientDepotBuilderCallbacks %i\n", callback_id);
            break;

        case k_iSteamGameServerItemsCallbacks:
            PRINT_DEBUG("k_iSteamGameServerItemsCallbacks %i\n", callback_id);
            break;

        case k_iClientUtilsCallbacks:
            PRINT_DEBUG("k_iClientUtilsCallbacks %i\n", callback_id);
            break;

        case k_iSteamGameCoordinatorCallbacks:
            PRINT_DEBUG("k_iSteamGameCoordinatorCallbacks %i\n", callback_id);
            break;

        case k_iSteamGameServerStatsCallbacks:
            PRINT_DEBUG("k_iSteamGameServerStatsCallbacks %i\n", callback_id);
            break;

        case k_iSteam2AsyncCallbacks:
            PRINT_DEBUG("k_iSteam2AsyncCallbacks %i\n", callback_id);
            break;

        case k_iSteamGameStatsCallbacks:
            PRINT_DEBUG("k_iSteamGameStatsCallbacks %i\n", callback_id);
            break;

        case k_iClientHTTPCallbacks:
            PRINT_DEBUG("k_iClientHTTPCallbacks %i\n", callback_id);
            break;

        case k_iClientScreenshotsCallbacks:
            PRINT_DEBUG("k_iClientScreenshotsCallbacks %i\n", callback_id);
            break;

        case k_iSteamScreenshotsCallbacks:
            PRINT_DEBUG("k_iSteamScreenshotsCallbacks %i\n", callback_id);
            break;

        case k_iClientAudioCallbacks:
            PRINT_DEBUG("k_iClientAudioCallbacks %i\n", callback_id);
            break;

        case k_iClientUnifiedMessagesCallbacks:
            PRINT_DEBUG("k_iClientUnifiedMessagesCallbacks %i\n", callback_id);
            break;

        case k_iSteamStreamLauncherCallbacks:
            PRINT_DEBUG("k_iSteamStreamLauncherCallbacks %i\n", callback_id);
            break;

        case k_iClientControllerCallbacks:
            PRINT_DEBUG("k_iClientControllerCallbacks %i\n", callback_id);
            break;

        case k_iSteamControllerCallbacks:
            PRINT_DEBUG("k_iSteamControllerCallbacks %i\n", callback_id);
            break;

        case k_iClientParentalSettingsCallbacks:
            PRINT_DEBUG("k_iClientParentalSettingsCallbacks %i\n", callback_id);
            break;

        case k_iClientDeviceAuthCallbacks:
            PRINT_DEBUG("k_iClientDeviceAuthCallbacks %i\n", callback_id);
            break;

        case k_iClientNetworkDeviceManagerCallbacks:
            PRINT_DEBUG("k_iClientNetworkDeviceManagerCallbacks %i\n", callback_id);
            break;

        case k_iClientMusicCallbacks:
            PRINT_DEBUG("k_iClientMusicCallbacks %i\n", callback_id);
            break;

        case k_iClientRemoteClientManagerCallbacks:
            PRINT_DEBUG("k_iClientRemoteClientManagerCallbacks %i\n", callback_id);
            break;

        case k_iClientUGCCallbacks:
            PRINT_DEBUG("k_iClientUGCCallbacks %i\n", callback_id);
            break;

        case k_iSteamStreamClientCallbacks:
            PRINT_DEBUG("k_iSteamStreamClientCallbacks %i\n", callback_id);
            break;

        case k_IClientProductBuilderCallbacks:
            PRINT_DEBUG("k_IClientProductBuilderCallbacks %i\n", callback_id);
            break;

        case k_iClientShortcutsCallbacks:
            PRINT_DEBUG("k_iClientShortcutsCallbacks %i\n", callback_id);
            break;

        case k_iClientRemoteControlManagerCallbacks:
            PRINT_DEBUG("k_iClientRemoteControlManagerCallbacks %i\n", callback_id);
            break;

        case k_iSteamAppListCallbacks:
            PRINT_DEBUG("k_iSteamAppListCallbacks %i\n", callback_id);
            break;

        case k_iSteamMusicCallbacks:
            PRINT_DEBUG("k_iSteamMusicCallbacks %i\n", callback_id);
            break;

        case k_iSteamMusicRemoteCallbacks:
            PRINT_DEBUG("k_iSteamMusicRemoteCallbacks %i\n", callback_id);
            break;

        case k_iClientVRCallbacks:
            PRINT_DEBUG("k_iClientVRCallbacks %i\n", callback_id);
            break;

        case k_iClientGameNotificationCallbacks:
            PRINT_DEBUG("k_iClientGameNotificationCallbacks %i\n", callback_id);
            break;
 
        case k_iSteamGameNotificationCallbacks:
            PRINT_DEBUG("k_iSteamGameNotificationCallbacks %i\n", callback_id);
            break;
 
        case k_iSteamHTMLSurfaceCallbacks:
            PRINT_DEBUG("k_iSteamHTMLSurfaceCallbacks %i\n", callback_id);
            break;

        case k_iClientVideoCallbacks:
            PRINT_DEBUG("k_iClientVideoCallbacks %i\n", callback_id);
            break;

        case k_iClientInventoryCallbacks:
            PRINT_DEBUG("k_iClientInventoryCallbacks %i\n", callback_id);
            break;

        case k_iClientBluetoothManagerCallbacks:
            PRINT_DEBUG("k_iClientBluetoothManagerCallbacks %i\n", callback_id);
            break;

        case k_iClientSharedConnectionCallbacks:
            PRINT_DEBUG("k_iClientSharedConnectionCallbacks %i\n", callback_id);
            break;

        case k_ISteamParentalSettingsCallbacks:
            PRINT_DEBUG("k_ISteamParentalSettingsCallbacks %i\n", callback_id);
            break;

        case k_iClientShaderCallbacks:
            PRINT_DEBUG("k_iClientShaderCallbacks %i\n", callback_id);
            break;
        
        default:
            PRINT_DEBUG("Unknown callback base %i\n", base_callback);
    };

    if (isGameServer) {
        callbacks_server->rmCallBack(iCallback, pCallback);
    } else {
        callbacks_client->rmCallBack(iCallback, pCallback);
    }
}

void Steam_Client::RegisterCallResult( class CCallbackBase *pCallback, SteamAPICall_t hAPICall)
{
    PRINT_DEBUG("Steam_Client::RegisterCallResult %llu %i\n", hAPICall, pCallback->GetICallback());
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    callback_results_client->addCallBack(hAPICall, pCallback);
    callback_results_server->addCallBack(hAPICall, pCallback);
    
}

void Steam_Client::UnregisterCallResult( class CCallbackBase *pCallback, SteamAPICall_t hAPICall)
{
    PRINT_DEBUG("Steam_Client::UnregisterCallResult %llu %i\n", hAPICall, pCallback->GetICallback());
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    callback_results_client->rmCallBack(hAPICall, pCallback);
    callback_results_server->rmCallBack(hAPICall, pCallback);
}

void Steam_Client::RunCallbacks(bool runClientCB, bool runGameserverCB)
{
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!background_keepalive.joinable()) background_keepalive = std::thread(background_thread, this);

    network->Run();
    PRINT_DEBUG("Steam_Client::RunCallbacks steam_matchmaking_servers\n");
    steam_matchmaking_servers->RunCallbacks();
    run_every_runcb->run();

    steam_gameserver->RunCallbacks();

    if (runClientCB) {
        PRINT_DEBUG("Steam_Client::RunCallbacks callback_results_client\n");
        callback_results_client->runCallResults();
    }

    if (runGameserverCB) {
        PRINT_DEBUG("Steam_Client::RunCallbacks callback_results_server\n");
        callback_results_server->runCallResults();
    }

    PRINT_DEBUG("Steam_Client::RunCallbacks callbacks_server\n");
    callbacks_server->runCallBacks();
    PRINT_DEBUG("Steam_Client::RunCallbacks callbacks_client\n");
    callbacks_client->runCallBacks();
    last_cb_run = std::chrono::duration_cast<std::chrono::duration<unsigned long long>>(std::chrono::system_clock::now().time_since_epoch()).count();
    PRINT_DEBUG("Steam_Client::RunCallbacks done\n");
}

void Steam_Client::DestroyAllInterfaces()
{
    PRINT_DEBUG("Steam_Client::DestroyAllInterfaces\n");
}
