
#ifndef ISTEAMUSER011_H
#define ISTEAMUSER011_H
#ifdef STEAM_WIN32
#pragma once
#endif


class ISteamUser011
{
public:
	// returns the HSteamUser this interface represents
	// this is only used internally by the API, and by a few select interfaces that support multi-user
	virtual HSteamUser GetHSteamUser() = 0;

	// returns true if the Steam client current has a live connection to the Steam servers. 
	// If false, it means there is no active connection due to either a networking issue on the local machine, or the Steam server is down/busy.
	// The Steam client will automatically be trying to recreate the connection as often as possible.
	virtual bool BLoggedOn() = 0;

	// returns the CSteamID of the account currently logged into the Steam client
	// a CSteamID is a unique identifier for an account, and used to differentiate users in all parts of the Steamworks API
	virtual CSteamID GetSteamID() = 0;

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
	virtual int InitiateGameConnection( void *pAuthBlob, int cbMaxAuthBlob, CSteamID steamIDGameServer, uint32 unIPServer, uint16 usPortServer, bool bSecure ) = 0;

	// notify of disconnect
	// needs to occur when the game client leaves the specified game server, needs to match with the InitiateGameConnection() call
	virtual void TerminateGameConnection( uint32 unIPServer, uint16 usPortServer ) = 0;

	// Legacy functions

	// used by only a few games to track usage events
	virtual void TrackAppUsageEvent( CGameID gameID, int eAppUsageEvent, const char *pchExtraInfo = "" ) = 0;

	// get the local storage folder for current Steam account to write application data, e.g. save games, configs etc.
	// this will usually be something like "C:\Progam Files\Steam\userdata\<SteamID>\<AppID>\local"
	virtual bool GetUserDataFolder( char *pchBuffer, int cubBuffer ) = 0;

	// Starts voice recording. Once started, use GetCompressedVoice() to get the data
	virtual void StartVoiceRecording( ) = 0;

	// Stops voice recording. Because people often release push-to-talk keys early, the system will keep recording for
	// a little bit after this function is called. GetCompressedVoice() should continue to be called until it returns
	// k_eVoiceResultNotRecording
	virtual void StopVoiceRecording( ) = 0;

	// Gets the latest voice data. It should be called as often as possible once recording has started.
	// nBytesWritten is set to the number of bytes written to pDestBuffer. 
	virtual EVoiceResult GetCompressedVoice( void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten ) = 0;

	// Decompresses a chunk of data produced by GetCompressedVoice(). nBytesWritten is set to the 
	// number of bytes written to pDestBuffer. The output format of the data is 16-bit signed at 
	// 11025 samples per second.
	virtual EVoiceResult DecompressVoice( void *pCompressed, uint32 cbCompressed, void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten ) = 0;
};


#endif // ISTEAMUSER011_H
