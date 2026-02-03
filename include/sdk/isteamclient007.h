
#ifndef ISTEAMCLIENT007_H
#define ISTEAMCLIENT007_H
#ifdef STEAM_WIN32
#pragma once
#endif

#include "isteammasterserverupdater.h"

class ISteamClient007
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
	virtual HSteamUser CreateLocalUser( HSteamPipe *phSteamPipe ) = 0;

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

	// returns the ISteamContentServer interface
	virtual ISteamContentServer *GetISteamContentServer( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion ) = 0;

	// returns the ISteamMasterServerUpdater interface
	virtual ISteamMasterServerUpdater *GetISteamMasterServerUpdater( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion ) = 0;

	// returns the ISteamMatchmakingServers interface
	virtual ISteamMatchmakingServers *GetISteamMatchmakingServers( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion ) = 0;

	// returns the a generic interface
	virtual void *GetISteamGenericInterface( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion ) = 0;

	// this needs to be called every frame to process matchmaking results
	// redundant if you're already calling SteamAPI_RunCallbacks()
	virtual void RunFrame() = 0;

	// returns the number of IPC calls made since the last time this function was called
	// Used for perf debugging so you can understand how many IPC calls your game makes per frame
	// Every IPC call is at minimum a thread context switch if not a process one so you want to rate
	// control how often you do them.
	virtual uint32 GetIPCCallCount() = 0;

	// returns the ISteamUserStats interface
	virtual ISteamUserStats *GetISteamUserStats( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion ) = 0;

	// returns apps interface
	virtual ISteamApps *GetISteamApps( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion ) = 0;

	// networking
	virtual ISteamNetworking *GetISteamNetworking( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion ) = 0;

	// API warning handling
	// 'int' is the severity; 0 for msg, 1 for warning
	// 'const char *' is the text of the message
	// callbacks will occur directly after the API function is called that generated the warning or message
	virtual void SetWarningMessageHook( SteamAPIWarningMessageHook_t pFunction ) = 0;
};


#endif // ISTEAMCLIENT007_H
