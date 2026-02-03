
#ifndef ISTEAMGAMESERVER012_H
#define ISTEAMGAMESERVER012_H
#ifdef STEAM_WIN32
#pragma once
#endif

class ISteamGameServer012
{
public:
public:

//
// Basic server data.  These properties, if set, must be set before before calling LogOn.  They
// may not be changed after logged in.
//

	/// This is called by SteamGameServer_Init, and you will usually not need to call it directly
	virtual bool InitGameServer( uint32 unIP, uint16 usGamePort, uint16 usQueryPort, uint32 unFlags, AppId_t nGameAppId, const char *pchVersionString ) = 0;

	/// Game product identifier.  This is currently used by the master server for version checking purposes.
	/// It's a required field, but will eventually will go away, and the AppID will be used for this purpose.
	virtual void SetProduct( const char *pszProduct ) = 0;

	/// Description of the game.  This is a required field and is displayed in the steam server browser....for now.
	/// This is a required field, but it will go away eventually, as the data should be determined from the AppID.
	virtual void SetGameDescription( const char *pszGameDescription ) = 0;

	/// If your game is a "mod," pass the string that identifies it.  The default is an empty string, meaning
	/// this application is the original game, not a mod.
	///
	/// @see k_cbMaxGameServerGameDir
	virtual void SetModDir( const char *pszModDir ) = 0;

	/// Is this is a dedicated server?  The default value is false.
	virtual void SetDedicatedServer( bool bDedicated ) = 0;

//
// Login
//

	/// Begin process to login to a persistent game server account
	///
	/// You need to register for callbacks to determine the result of this operation.
	/// @see SteamServersConnected_t
	/// @see SteamServerConnectFailure_t
	/// @see SteamServersDisconnected_t
	virtual void LogOn( const char *pszToken ) = 0;

	/// Login to a generic, anonymous account.
	///
	/// Note: in previous versions of the SDK, this was automatically called within SteamGameServer_Init,
	/// but this is no longer the case.
	virtual void LogOnAnonymous() = 0;

	/// Begin process of logging game server out of steam
	virtual void LogOff() = 0;
	
	// status functions
	virtual bool BLoggedOn() = 0;
	virtual bool BSecure() = 0; 
	virtual CSteamID GetSteamID() = 0;

	/// Returns true if the master server has requested a restart.
	/// Only returns true once per request.
	virtual bool WasRestartRequested() = 0;

//
// Server state.  These properties may be changed at any time.
//

	/// Max player count that will be reported to server browser and client queries
	virtual void SetMaxPlayerCount( int cPlayersMax ) = 0;

	/// Number of bots.  Default value is zero
	virtual void SetBotPlayerCount( int cBotplayers ) = 0;

	/// Set the name of server as it will appear in the server browser
	///
	/// @see k_cbMaxGameServerName
	virtual void SetServerName( const char *pszServerName ) = 0;

	/// Set name of map to report in the server browser
	///
	/// @see k_cbMaxGameServerName
	virtual void SetMapName( const char *pszMapName ) = 0;

	/// Let people know if your server will require a password
	virtual void SetPasswordProtected( bool bPasswordProtected ) = 0;

	/// Spectator server.  The default value is zero, meaning the service
	/// is not used.
	virtual void SetSpectatorPort( uint16 unSpectatorPort ) = 0;

	/// Name of the spectator server.  (Only used if spectator port is nonzero.)
	///
	/// @see k_cbMaxGameServerMapName
	virtual void SetSpectatorServerName( const char *pszSpectatorServerName ) = 0;

	/// Call this to clear the whole list of key/values that are sent in rules queries.
	virtual void ClearAllKeyValues() = 0;
	
	/// Call this to add/update a key/value pair.
	virtual void SetKeyValue( const char *pKey, const char *pValue ) = 0;

	/// Sets a string defining the "gametags" for this server, this is optional, but if it is set
	/// it allows users to filter in the matchmaking/server-browser interfaces based on the value
	///
	/// @see k_cbMaxGameServerTags
	virtual void SetGameTags( const char *pchGameTags ) = 0;

	/// Sets a string defining the "gamedata" for this server, this is optional, but if it is set
	/// it allows users to filter in the matchmaking/server-browser interfaces based on the value
	/// don't set this unless it actually changes, its only uploaded to the master once (when
	/// acknowledged)
	///
	/// @see k_cbMaxGameServerGameData
	virtual void SetGameData( const char *pchGameData ) = 0;

	/// Region identifier.  This is an optional field, the default value is empty, meaning the "world" region
	virtual void SetRegion( const char *pszRegion ) = 0;

//
// Player list management / authentication
//

	// Handles receiving a new connection from a Steam user.  This call will ask the Steam
	// servers to validate the users identity, app ownership, and VAC status.  If the Steam servers 
	// are off-line, then it will validate the cached ticket itself which will validate app ownership 
	// and identity.  The AuthBlob here should be acquired on the game client using SteamUser()->InitiateGameConnection()
	// and must then be sent up to the game server for authentication.
	//
	// Return Value: returns true if the users ticket passes basic checks. pSteamIDUser will contain the Steam ID of this user. pSteamIDUser must NOT be NULL
	// If the call succeeds then you should expect a GSClientApprove_t or GSClientDeny_t callback which will tell you whether authentication
	// for the user has succeeded or failed (the steamid in the callback will match the one returned by this call)
	virtual bool SendUserConnectAndAuthenticate( uint32 unIPClient, const void *pvAuthBlob, uint32 cubAuthBlobSize, CSteamID *pSteamIDUser ) = 0;

	// Creates a fake user (ie, a bot) which will be listed as playing on the server, but skips validation.  
	// 
	// Return Value: Returns a SteamID for the user to be tracked with, you should call HandleUserDisconnect()
	// when this user leaves the server just like you would for a real user.
	virtual CSteamID CreateUnauthenticatedUserConnection() = 0;

	// Should be called whenever a user leaves our game server, this lets Steam internally
	// track which users are currently on which servers for the purposes of preventing a single
	// account being logged into multiple servers, showing who is currently on a server, etc.
	virtual void SendUserDisconnect( CSteamID steamIDUser ) = 0;

	// Update the data to be displayed in the server browser and matchmaking interfaces for a user
	// currently connected to the server.  For regular users you must call this after you receive a
	// GSUserValidationSuccess callback.
	// 
	// Return Value: true if successful, false if failure (ie, steamIDUser wasn't for an active player)
	virtual bool BUpdateUserData( CSteamID steamIDUser, const char *pchPlayerName, uint32 uScore ) = 0;

	// New auth system APIs - do not mix with the old auth system APIs.
	// ----------------------------------------------------------------

	// Retrieve ticket to be sent to the entity who wishes to authenticate you ( using BeginAuthSession API ). 
	// pcbTicket retrieves the length of the actual ticket.
	virtual HAuthTicket GetAuthSessionTicket( void *pTicket, int cbMaxTicket, uint32 *pcbTicket ) = 0;

	// Authenticate ticket ( from GetAuthSessionTicket ) from entity steamID to be sure it is valid and isnt reused
	// Registers for callbacks if the entity goes offline or cancels the ticket ( see ValidateAuthTicketResponse_t callback and EAuthSessionResponse )
	virtual EBeginAuthSessionResult BeginAuthSession( const void *pAuthTicket, int cbAuthTicket, CSteamID steamID ) = 0;

	// Stop tracking started by BeginAuthSession - called when no longer playing game with this entity
	virtual void EndAuthSession( CSteamID steamID ) = 0;

	// Cancel auth ticket from GetAuthSessionTicket, called when no longer playing game with the entity you gave the ticket to
	virtual void CancelAuthTicket( HAuthTicket hAuthTicket ) = 0;

	// After receiving a user's authentication data, and passing it to SendUserConnectAndAuthenticate, use this function
	// to determine if the user owns downloadable content specified by the provided AppID.
	virtual EUserHasLicenseForAppResult UserHasLicenseForApp( CSteamID steamID, AppId_t appID ) = 0;

	// Ask if a user in in the specified group, results returns async by GSUserGroupStatus_t
	// returns false if we're not connected to the steam servers and thus cannot ask
	virtual bool RequestUserGroupStatus( CSteamID steamIDUser, CSteamID steamIDGroup ) = 0;


	// these two functions s are deprecated, and will not return results
	// they will be removed in a future version of the SDK
	virtual void GetGameplayStats( ) = 0;
	STEAM_CALL_RESULT( GSReputation_t )
	virtual SteamAPICall_t GetServerReputation() = 0;

	// Returns the public IP of the server according to Steam, useful when the server is 
	// behind NAT and you want to advertise its IP in a lobby for other clients to directly
	// connect to
	virtual uint32 GetPublicIP_old() = 0;

// These are in GameSocketShare mode, where instead of ISteamGameServer creating its own
// socket to talk to the master server on, it lets the game use its socket to forward messages
// back and forth. This prevents us from requiring server ops to open up yet another port
// in their firewalls.
//
// the IP address and port should be in host order, i.e 127.0.0.1 == 0x7f000001
	
	// These are used when you've elected to multiplex the game server's UDP socket
	// rather than having the master server updater use its own sockets.
	// 
	// Source games use this to simplify the job of the server admins, so they 
	// don't have to open up more ports on their firewalls.
	
	// Call this when a packet that starts with 0xFFFFFFFF comes in. That means
	// it's for us.
	virtual bool HandleIncomingPacket( const void *pData, int cbData, uint32 srcIP, uint16 srcPort ) = 0;

	// AFTER calling HandleIncomingPacket for any packets that came in that frame, call this.
	// This gets a packet that the master server updater needs to send out on UDP.
	// It returns the length of the packet it wants to send, or 0 if there are no more packets to send.
	// Call this each frame until it returns 0.
	virtual int GetNextOutgoingPacket( void *pOut, int cbMaxOut, uint32 *pNetAdr, uint16 *pPort ) = 0;

//
// Control heartbeats / advertisement with master server
//

	// Call this as often as you like to tell the master server updater whether or not
	// you want it to be active (default: off).
	virtual void EnableHeartbeats( bool bActive ) = 0;

	// You usually don't need to modify this.
	// Pass -1 to use the default value for iHeartbeatInterval.
	// Some mods change this.
	virtual void SetHeartbeatInterval( int iHeartbeatInterval ) = 0;

	// Force a heartbeat to steam at the next opportunity
	virtual void ForceHeartbeat() = 0;

	// associate this game server with this clan for the purposes of computing player compat
	STEAM_CALL_RESULT( AssociateWithClanResult_t )
	virtual SteamAPICall_t AssociateWithClan( CSteamID steamIDClan ) = 0;
	
	// ask if any of the current players dont want to play with this new player - or vice versa
	STEAM_CALL_RESULT( ComputeNewPlayerCompatibilityResult_t )
	virtual SteamAPICall_t ComputeNewPlayerCompatibility( CSteamID steamIDNewPlayer ) = 0;

};

#endif // ISTEAMGAMESERVER012_H
