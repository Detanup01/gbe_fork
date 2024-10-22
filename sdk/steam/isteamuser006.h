
#ifndef ISTEAMUSER006_H
#define ISTEAMUSER006_H
#ifdef STEAM_WIN32
#pragma once
#endif

//-----------------------------------------------------------------------------
// Purpose: Functions for accessing and manipulating a steam account
//			associated with one client instance
//-----------------------------------------------------------------------------
class ISteamUser006
{
public:
	// returns the HSteamUser this interface represents
	// this is only used internally by the API, and by a few select interfaces that support multi-user
	virtual HSteamUser GetHSteamUser() = 0;

	virtual void LogOn( CSteamID steamID ) = 0;
	virtual void LogOff() = 0;

	// returns true if the Steam client current has a live connection to the Steam servers. 
	// If false, it means there is no active connection due to either a networking issue on the local machine, or the Steam server is down/busy.
	// The Steam client will automatically be trying to recreate the connection as often as possible.
	virtual bool BLoggedOn() = 0;

	// returns the CSteamID of the account currently logged into the Steam client
	// a CSteamID is a unique identifier for an account, and used to differentiate users in all parts of the Steamworks API
	virtual CSteamID GetSteamID() = 0;

	virtual bool SetRegistryString( EConfigSubTree eRegistrySubTree, const char *pchKey, const char *pchValue ) = 0;
	virtual bool GetRegistryString( EConfigSubTree eRegistrySubTree, const char *pchKey, char *pchValue, int cbValue ) = 0;
	virtual bool SetRegistryInt( EConfigSubTree eRegistrySubTree, const char *pchKey, int iValue ) = 0;
	virtual bool GetRegistryInt( EConfigSubTree eRegistrySubTree, const char *pchKey, int *piValue ) = 0;

	// Multiplayer Authentication functions
	
	// InitiateGameConnection() starts the state machine for authenticating the game client with the game server
	// It is the client portion of a three-way handshake between the client, the game server, and the steam servers
	//
	// Parameters:
	// void *pAuthBlob - a pointer to empty memory that will be filled in with the authentication token.
	// int cbMaxAuthBlob - the number of bytes of allocated memory in pBlob. Should be at least 2048 bytes.
	// CSteamID steamIDGameServer - the steamID of the game server, received from the game server by the client
	// CGameID gameID - the ID of the current game. For games without mods, this is just CGameID( <appID> )
	// uint32 unIPServer, uint16 usPortServer - the IP address of the game server
	// bool bSecure - whether or not the client thinks that the game server is reporting itself as secure (i.e. VAC is running)
	//
	// return value - returns the number of bytes written to pBlob. If the return is 0, then the buffer passed in was too small, and the call has failed
	// The contents of pBlob should then be sent to the game server, for it to use to complete the authentication process.
	virtual int InitiateGameConnection( void *pBlob, int cbMaxBlob, CSteamID steamID, CGameID gameID, uint32 unIPServer, uint16 usPortServer, bool bSecure ) = 0;

	// notify of disconnect
	// needs to occur when the game client leaves the specified game server, needs to match with the InitiateGameConnection() call
	virtual void TerminateGameConnection( uint32 unIPServer, uint16 usPortServer ) = 0;

	// Legacy functions

	// used by only a few games to track usage events
	virtual void TrackAppUsageEvent( CGameID gameID, int eAppUsageEvent, const char *pchExtraInfo = "" ) = 0;
};


#endif // ISTEAMUSER006_H
