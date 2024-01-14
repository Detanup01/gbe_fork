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

#include "base.h"
#include "appticket.h"
#include "steam_user.h"
#include "steam_friends.h"
#include "steam_utils.h"
#include "steam_matchmaking.h"
#include "steam_matchmaking_servers.h"
#include "steam_user_stats.h"
#include "steam_apps.h"
#include "steam_networking.h"
#include "steam_remote_storage.h"
#include "steam_screenshots.h"
#include "steam_http.h"
#include "steam_controller.h"
#include "steam_ugc.h"
#include "ugc_remote_storage_bridge.h"
#include "steam_applist.h"
#include "steam_music.h"
#include "steam_musicremote.h"
#include "steam_HTMLsurface.h"
#include "steam_inventory.h"
#include "steam_video.h"
#include "steam_parental.h"
#include "steam_game_coordinator.h"
#include "steam_networking_socketsserialized.h"
#include "steam_networking_sockets.h"
#include "steam_networking_messages.h"
#include "steam_networking_utils.h"
#include "steam_unified_messages.h"
#include "steam_gamesearch.h"
#include "steam_parties.h"
#include "steam_remoteplay.h"
#include "steam_tv.h"

#include "steam_gameserver.h"
#include "steam_gameserverstats.h"
#include "steam_masterserver_updater.h"

#include "overlay/steam_overlay.h"

enum Steam_Pipe {
    NO_USER,
    CLIENT,
    SERVER
};

class Steam_Client :
public ISteamClient007,
public ISteamClient008,
public ISteamClient009,
public ISteamClient010,
public ISteamClient011,
public ISteamClient012,
public ISteamClient013,
public ISteamClient014,
public ISteamClient015,
public ISteamClient016,
public ISteamClient017,
public ISteamClient018,
public ISteamClient019,
public ISteamClient
{
public:
    Networking *network;
    SteamCallResults *callback_results_server, *callback_results_client;
    SteamCallBacks *callbacks_server, *callbacks_client;
    Settings *settings_client, *settings_server;
    Local_Storage *local_storage;
    RunEveryRunCB *run_every_runcb;

    Ugc_Remote_Storage_Bridge *ugc_bridge;
    
    Steam_User *steam_user;
    Steam_Friends *steam_friends;
    Steam_Utils *steam_utils;
    Steam_Matchmaking *steam_matchmaking;
    Steam_Matchmaking_Servers *steam_matchmaking_servers;
    Steam_User_Stats *steam_user_stats;
    Steam_Apps *steam_apps;
    Steam_Networking *steam_networking;
    Steam_Remote_Storage *steam_remote_storage;
    Steam_Screenshots *steam_screenshots;
    Steam_HTTP *steam_http;
    Steam_Controller *steam_controller;
    Steam_UGC *steam_ugc;
    Steam_Applist *steam_applist;
    Steam_Music *steam_music;
    Steam_MusicRemote *steam_musicremote;
    Steam_HTMLsurface *steam_HTMLsurface;
    Steam_Inventory *steam_inventory;
    Steam_Video *steam_video;
    Steam_Parental *steam_parental;
    Steam_Networking_Sockets *steam_networking_sockets;
    Steam_Networking_Sockets_Serialized *steam_networking_sockets_serialized;
    Steam_Networking_Messages *steam_networking_messages;
    Steam_Game_Coordinator *steam_game_coordinator;
    Steam_Networking_Utils *steam_networking_utils;
    Steam_Unified_Messages *steam_unified_messages;
    Steam_Game_Search *steam_game_search;
    Steam_Parties *steam_parties;
    Steam_RemotePlay *steam_remoteplay;
    Steam_TV *steam_tv;

    Steam_GameServer *steam_gameserver;
    Steam_Utils *steam_gameserver_utils;
    Steam_GameServerStats *steam_gameserverstats;
    Steam_Networking *steam_gameserver_networking;
    Steam_HTTP *steam_gameserver_http;
    Steam_Inventory *steam_gameserver_inventory;
    Steam_UGC *steam_gameserver_ugc;
    Steam_Apps *steam_gameserver_apps;
    Steam_Networking_Sockets *steam_gameserver_networking_sockets;
    Steam_Networking_Sockets_Serialized *steam_gameserver_networking_sockets_serialized;
    Steam_Networking_Messages *steam_gameserver_networking_messages;
    Steam_Game_Coordinator *steam_gameserver_game_coordinator;
    Steam_Masterserver_Updater *steam_masterserver_updater;
    Steam_AppTicket *steam_app_ticket;

    Steam_Overlay* steam_overlay;

    bool user_logged_in = false;
    bool server_init = false;
    std::thread background_keepalive;
    bool steamclient_server_inited = false;
    std::atomic<unsigned long long> last_cb_run;

    unsigned steam_pipe_counter = 1;
    std::map<HSteamPipe, enum Steam_Pipe> steam_pipes;

    bool gameserver_has_ipv6_functions;

    Steam_Client();
    ~Steam_Client();
    	// Creates a communication pipe to the Steam client.
	// NOT THREADSAFE - ensure that no other threads are accessing Steamworks API when calling
	HSteamPipe CreateSteamPipe();

	// Releases a previously created communications pipe
	// NOT THREADSAFE - ensure that no other threads are accessing Steamworks API when calling
	bool BReleaseSteamPipe( HSteamPipe hSteamPipe );

	// connects to an existing global user, failing if none exists
	// used by the game to coordinate with the steamUI
	// NOT THREADSAFE - ensure that no other threads are accessing Steamworks API when calling
	HSteamUser ConnectToGlobalUser( HSteamPipe hSteamPipe );

	// used by game servers, create a steam user that won't be shared with anyone else
	// NOT THREADSAFE - ensure that no other threads are accessing Steamworks API when calling
	HSteamUser CreateLocalUser( HSteamPipe *phSteamPipe, EAccountType eAccountType );
    HSteamUser CreateLocalUser( HSteamPipe *phSteamPipe );

	// removes an allocated user
	// NOT THREADSAFE - ensure that no other threads are accessing Steamworks API when calling
	void ReleaseUser( HSteamPipe hSteamPipe, HSteamUser hUser );

	// retrieves the ISteamUser interface associated with the handle
	ISteamUser *GetISteamUser( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion );

	// retrieves the ISteamGameServer interface associated with the handle
	ISteamGameServer *GetISteamGameServer( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion );

	// set the local IP and Port to bind to
	// this must be set before CreateLocalUser()
	void SetLocalIPBinding( uint32 unIP, uint16 usPort ); 
    void SetLocalIPBinding( const SteamIPAddress_t &unIP, uint16 usPort );

	// returns the ISteamFriends interface
	ISteamFriends *GetISteamFriends( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion );

	// returns the ISteamUtils interface
	ISteamUtils *GetISteamUtils( HSteamPipe hSteamPipe, const char *pchVersion );

	// returns the ISteamMatchmaking interface
	ISteamMatchmaking *GetISteamMatchmaking( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion );

	// returns the ISteamMatchmakingServers interface
	ISteamMatchmakingServers *GetISteamMatchmakingServers( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion );

	// returns the a generic interface
	void *GetISteamGenericInterface( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion );

	// returns the ISteamUserStats interface
	ISteamUserStats *GetISteamUserStats( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion );

	// returns the ISteamGameServerStats interface
	ISteamGameServerStats *GetISteamGameServerStats( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion );

	// returns apps interface
	ISteamApps *GetISteamApps( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion );

	// networking
	ISteamNetworking *GetISteamNetworking( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion );

	// remote storage
	ISteamRemoteStorage *GetISteamRemoteStorage( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion );

	// user screenshots
	ISteamScreenshots *GetISteamScreenshots( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion );


	// Deprecated. Applications should use SteamAPI_RunCallbacks() or SteamGameServer_RunCallbacks() instead.
	STEAM_PRIVATE_API( void RunFrame(); )

	// returns the number of IPC calls made since the last time this function was called
	// Used for perf debugging so you can understand how many IPC calls your game makes per frame
	// Every IPC call is at minimum a thread context switch if not a process one so you want to rate
	// control how often you do them.
	uint32 GetIPCCallCount();

	// API warning handling
	// 'int' is the severity; 0 for msg, 1 for warning
	// 'const char *' is the text of the message
	// callbacks will occur directly after the API function is called that generated the warning or message.
	void SetWarningMessageHook( SteamAPIWarningMessageHook_t pFunction );

	// Trigger global shutdown for the DLL
	bool BShutdownIfAllPipesClosed();

	// Expose HTTP interface
	ISteamHTTP *GetISteamHTTP( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion );

	// Deprecated - the ISteamUnifiedMessages interface is no longer intended for public consumption.
	STEAM_PRIVATE_API( void *DEPRECATED_GetISteamUnifiedMessages( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion ) ; )
    ISteamUnifiedMessages *GetISteamUnifiedMessages( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion );

	// Exposes the ISteamController interface
	ISteamController *GetISteamController( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion );

	// Exposes the ISteamUGC interface
	ISteamUGC *GetISteamUGC( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion );

	// returns app list interface, only available on specially registered apps
	ISteamAppList *GetISteamAppList( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion );
	
	// Music Player
	ISteamMusic *GetISteamMusic( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion );

	// Music Player Remote
	ISteamMusicRemote *GetISteamMusicRemote(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion);

	// html page display
	ISteamHTMLSurface *GetISteamHTMLSurface(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion);

	// Helper functions for internal Steam usage
	STEAM_PRIVATE_API( void DEPRECATED_Set_SteamAPI_CPostAPIResultInProcess( void (*)() ); )
	STEAM_PRIVATE_API( void DEPRECATED_Remove_SteamAPI_CPostAPIResultInProcess( void (*)() ); )
	STEAM_PRIVATE_API( void Set_SteamAPI_CCheckCallbackRegisteredInProcess( SteamAPI_CheckCallbackRegistered_t func ); )
    void Set_SteamAPI_CPostAPIResultInProcess( SteamAPI_PostAPIResultInProcess_t func );
    void Remove_SteamAPI_CPostAPIResultInProcess( SteamAPI_PostAPIResultInProcess_t func );

	// inventory
	ISteamInventory *GetISteamInventory( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion );

	// Video
	ISteamVideo *GetISteamVideo( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion );

	// Parental controls
	ISteamParentalSettings *GetISteamParentalSettings( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion );

    //
    ISteamMasterServerUpdater *GetISteamMasterServerUpdater( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion );

    ISteamContentServer *GetISteamContentServer( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion );

    // game search
    ISteamGameSearch *GetISteamGameSearch( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion );
    // Exposes the Steam Input interface for controller support
    ISteamInput *GetISteamInput( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion );

    // Steam Parties interface
    ISteamParties *GetISteamParties( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion );

    // Steam Remote Play interface
    ISteamRemotePlay *GetISteamRemotePlay( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion );
    
    ISteamAppTicket *GetAppTicket( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion );

    void RegisterCallback( class CCallbackBase *pCallback, int iCallback);
    void UnregisterCallback( class CCallbackBase *pCallback);

    void RegisterCallResult( class CCallbackBase *pCallback, SteamAPICall_t hAPICall);
    void UnregisterCallResult( class CCallbackBase *pCallback, SteamAPICall_t hAPICall);
    
    void RunCallbacks(bool runClientCB, bool runGameserverCB);
    void setAppID(uint32 appid);
    void userLogIn();
    void serverInit();
    void serverShutdown();
    void clientShutdown();
    bool IsServerInit();
    bool IsUserLogIn();

    void DestroyAllInterfaces();
};
