
#ifndef ISTEAMUSER016_H
#define ISTEAMUSER016_H
#ifdef STEAM_WIN32
#pragma once
#endif


class ISteamUser016
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

	// Starts voice recording. Once started, use GetVoice() to get the data
	virtual void StartVoiceRecording( ) = 0;

	// Stops voice recording. Because people often release push-to-talk keys early, the system will keep recording for
	// a little bit after this function is called. GetVoice() should continue to be called until it returns
	// k_eVoiceResultNotRecording
	virtual void StopVoiceRecording( ) = 0;

	// Determine the amount of captured audio data that is available in bytes.
	// This provides both the compressed and uncompressed data. Please note that the uncompressed
	// data is not the raw feed from the microphone: data may only be available if audible 
	// levels of speech are detected.
	// nUncompressedVoiceDesiredSampleRate is necessary to know the number of bytes to return in pcbUncompressed - can be set to 0 if you don't need uncompressed (the usual case)
	// If you're upgrading from an older Steamworks API, you'll want to pass in 11025 to nUncompressedVoiceDesiredSampleRate
	virtual EVoiceResult GetAvailableVoice(uint32 *pcbCompressed, uint32 *pcbUncompressed, uint32 nUncompressedVoiceDesiredSampleRate) = 0;

	// Gets the latest voice data from the microphone. Compressed data is an arbitrary format, and is meant to be handed back to 
	// DecompressVoice() for playback later as a binary blob. Uncompressed data is 16-bit, signed integer, 11025Hz PCM format.
	// Please note that the uncompressed data is not the raw feed from the microphone: data may only be available if audible 
	// levels of speech are detected, and may have passed through denoising filters, etc.
	// This function should be called as often as possible once recording has started; once per frame at least.
	// nBytesWritten is set to the number of bytes written to pDestBuffer. 
	// nUncompressedBytesWritten is set to the number of bytes written to pUncompressedDestBuffer. 
	// You must grab both compressed and uncompressed here at the same time, if you want both.
	// Matching data that is not read during this call will be thrown away.
	// GetAvailableVoice() can be used to determine how much data is actually available.
	// If you're upgrading from an older Steamworks API, you'll want to pass in 11025 to nUncompressedVoiceDesiredSampleRate
	virtual EVoiceResult GetVoice( bool bWantCompressed, void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten, bool bWantUncompressed, void *pUncompressedDestBuffer, uint32 cbUncompressedDestBufferSize, uint32 *nUncompressBytesWritten, uint32 nUncompressedVoiceDesiredSampleRate ) = 0;

	// Decompresses a chunk of compressed data produced by GetVoice().
	// nBytesWritten is set to the number of bytes written to pDestBuffer unless the return value is k_EVoiceResultBufferTooSmall.
	// In that case, nBytesWritten is set to the size of the buffer required to decompress the given
	// data. The suggested buffer size for the destination buffer is 22 kilobytes.
	// The output format of the data is 16-bit signed at the requested samples per second.
	// If you're upgrading from an older Steamworks API, you'll want to pass in 11025 to nDesiredSampleRate
	virtual EVoiceResult DecompressVoice( const void *pCompressed, uint32 cbCompressed, void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten, uint32 nDesiredSampleRate ) = 0;

	// This returns the frequency of the voice data as it's stored internally; calling DecompressVoice() with this size will yield the best results
	virtual uint32 GetVoiceOptimalSampleRate() = 0;

	// Retrieve ticket to be sent to the entity who wishes to authenticate you. 
	// pcbTicket retrieves the length of the actual ticket.
	virtual HAuthTicket GetAuthSessionTicket( void *pTicket, int cbMaxTicket, uint32 *pcbTicket ) = 0;

	// Authenticate ticket from entity steamID to be sure it is valid and isnt reused
	// Registers for callbacks if the entity goes offline or cancels the ticket ( see ValidateAuthTicketResponse_t callback and EAuthSessionResponse )
	virtual EBeginAuthSessionResult BeginAuthSession( const void *pAuthTicket, int cbAuthTicket, CSteamID steamID ) = 0;

	// Stop tracking started by BeginAuthSession - called when no longer playing game with this entity
	virtual void EndAuthSession( CSteamID steamID ) = 0;

	// Cancel auth ticket from GetAuthSessionTicket, called when no longer playing game with the entity you gave the ticket to
	virtual void CancelAuthTicket( HAuthTicket hAuthTicket ) = 0;

	// After receiving a user's authentication data, and passing it to BeginAuthSession, use this function
	// to determine if the user owns downloadable content specified by the provided AppID.
	virtual EUserHasLicenseForAppResult UserHasLicenseForApp( CSteamID steamID, AppId_t appID ) = 0;
	
	// returns true if this users looks like they are behind a NAT device. Only valid once the user has connected to steam 
	// (i.e a SteamServersConnected_t has been issued) and may not catch all forms of NAT.
	virtual bool BIsBehindNAT() = 0;

	// set data to be replicated to friends so that they can join your game
	// CSteamID steamIDGameServer - the steamID of the game server, received from the game server by the client
	// uint32 unIPServer, uint16 usPortServer - the IP address of the game server
	virtual void AdvertiseGame( CSteamID steamIDGameServer, uint32 unIPServer, uint16 usPortServer ) = 0;

	// Requests a ticket encrypted with an app specific shared key
	// pDataToInclude, cbDataToInclude will be encrypted into the ticket
	// ( This is asynchronous, you must wait for the ticket to be completed by the server )
	virtual SteamAPICall_t RequestEncryptedAppTicket( void *pDataToInclude, int cbDataToInclude ) = 0;

	// retrieve a finished ticket
	virtual bool GetEncryptedAppTicket( void *pTicket, int cbMaxTicket, uint32 *pcbTicket ) = 0;
};


#endif // ISTEAMUSER016_H
