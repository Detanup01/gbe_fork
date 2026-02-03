
#ifndef ISTEAMCLIENT015_H
#define ISTEAMCLIENT015_H
#ifdef STEAM_WIN32
#pragma once
#endif

class ISteamClient015
{
public:
	// Creates a communication pipe to the Steam client
	virtual HSteamPipe CreateSteamPipe() = 0;

	// Releases a previously created communications pipe
	virtual bool BReleaseSteamPipe( HSteamPipe hSteamPipe ) = 0;

	// connects to an existing global user, failing if none exists
	// used by the game to coordinate with the steamUI
	virtual HSteamUser ConnectToGlobalUser( HSteamPipe hSteamPipe ) = 0;

	// used by game servers, create a steam user that won't be shared with anyone else
	virtual HSteamUser CreateLocalUser( HSteamPipe *phSteamPipe, EAccountType eAccountType ) = 0;

	// removes an allocated user
	virtual void ReleaseUser( HSteamPipe hSteamPipe, HSteamUser hUser ) = 0;

	// retrieves the ISteamUser interface associated with the handle
	virtual ISteamUser *GetISteamUser( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion ) = 0;

	// retrieves the ISteamGameServer interface associated with the handle
	virtual ISteamGameServer *GetISteamGameServer( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion ) = 0;

	// set the local IP and Port to bind to
	// this must be set before CreateLocalUser()
	virtual void SetLocalIPBinding( uint32 unIP, uint16 usPort ) = 0; 

	// returns the ISteamFriends interface
	virtual ISteamFriends *GetISteamFriends( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion ) = 0;

	// returns the ISteamUtils interface
	virtual ISteamUtils *GetISteamUtils( HSteamPipe hSteamPipe, const char *pchVersion ) = 0;

	// returns the ISteamMatchmaking interface
	virtual ISteamMatchmaking *GetISteamMatchmaking( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion ) = 0;

	// returns the ISteamMatchmakingServers interface
	virtual ISteamMatchmakingServers *GetISteamMatchmakingServers( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion ) = 0;

	// returns the a generic interface
	virtual void *GetISteamGenericInterface( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion ) = 0;

	// returns the ISteamUserStats interface
	virtual ISteamUserStats *GetISteamUserStats( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion ) = 0;

	// returns the ISteamGameServerStats interface
	virtual ISteamGameServerStats *GetISteamGameServerStats( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion ) = 0;

	// returns apps interface
	virtual ISteamApps *GetISteamApps( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion ) = 0;

	// networking
	virtual ISteamNetworking *GetISteamNetworking( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion ) = 0;

	// remote storage
	virtual ISteamRemoteStorage *GetISteamRemoteStorage( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion ) = 0;

	// user screenshots
	virtual ISteamScreenshots *GetISteamScreenshots( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion ) = 0;


	// this needs to be called every frame to process matchmaking results
	// redundant if you're already calling SteamAPI_RunCallbacks()
	virtual void RunFrame() = 0;

	// returns the number of IPC calls made since the last time this function was called
	// Used for perf debugging so you can understand how many IPC calls your game makes per frame
	// Every IPC call is at minimum a thread context switch if not a process one so you want to rate
	// control how often you do them.
	virtual uint32 GetIPCCallCount() = 0;

	// API warning handling
	// 'int' is the severity; 0 for msg, 1 for warning
	// 'const char *' is the text of the message
	// callbacks will occur directly after the API function is called that generated the warning or message
	virtual void SetWarningMessageHook( SteamAPIWarningMessageHook_t pFunction ) = 0;

	// Trigger global shutdown for the DLL
	virtual bool BShutdownIfAllPipesClosed() = 0;

#ifdef _PS3
	virtual ISteamPS3OverlayRender *GetISteamPS3OverlayRender() = 0;
#endif

	// Expose HTTP interface
	virtual ISteamHTTP *GetISteamHTTP( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion ) = 0;

	// Exposes the ISteamUnifiedMessages interface
	virtual ISteamUnifiedMessages *GetISteamUnifiedMessages( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion ) = 0;

	// Exposes the ISteamController interface
	virtual ISteamController *GetISteamController( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion ) = 0;

	// Exposes the ISteamUGC interface
	virtual ISteamUGC *GetISteamUGC( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion ) = 0;

	// returns app list interface, only available on specially registered apps
	virtual ISteamAppList *GetISteamAppList( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion ) = 0;
	
	// Music Player
	virtual ISteamMusic *GetISteamMusic( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion ) = 0;

	// Music Player Remote
	virtual ISteamMusicRemote *GetISteamMusicRemote(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion) = 0;
};


#endif // ISTEAMCLIENT015_H
