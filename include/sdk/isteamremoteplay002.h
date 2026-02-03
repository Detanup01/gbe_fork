
#ifndef ISTEAMREMOTEPLAY002_H
#define ISTEAMREMOTEPLAY002_H
#ifdef STEAM_WIN32
#pragma once
#endif

//-----------------------------------------------------------------------------
// Purpose: Functions to provide information about Steam Remote Play sessions
//-----------------------------------------------------------------------------
class ISteamRemotePlay002
{
public:
	// Get the number of currently connected Steam Remote Play sessions
	virtual uint32 GetSessionCount() = 0;
	
	// Get the currently connected Steam Remote Play session ID at the specified index. Returns zero if index is out of bounds.
	virtual RemotePlaySessionID_t GetSessionID( int iSessionIndex ) = 0;

	// Get the SteamID of the connected user
	virtual CSteamID GetSessionSteamID( RemotePlaySessionID_t unSessionID ) = 0;

	// Get the name of the session client device
	// This returns NULL if the sessionID is not valid
	virtual const char *GetSessionClientName( RemotePlaySessionID_t unSessionID ) = 0;

	// Get the form factor of the session client device
	virtual ESteamDeviceFormFactor GetSessionClientFormFactor( RemotePlaySessionID_t unSessionID ) = 0;

	// Get the resolution, in pixels, of the session client device
	// This is set to 0x0 if the resolution is not available
	virtual bool BGetSessionClientResolution( RemotePlaySessionID_t unSessionID, int *pnResolutionX, int *pnResolutionY ) = 0;

	// Start Remote Play Together and optionally show the UI in the overlay
	// This returns false if Remote Play Together can't be started or your game is not configured for Remote Play Together
	virtual bool BStartRemotePlayTogether( bool bShowOverlay = true ) = 0;

	// Invite a friend to Remote Play Together, or create a guest invite if steamIDFriend is empty
	// This will automatically start Remote Play Together if it hasn't already been started
	// This returns false if the invite can't be sent or your game is not configured for Remote Play Together
	virtual bool BSendRemotePlayTogetherInvite( CSteamID steamIDFriend ) = 0;
};

#endif // #define ISTEAMREMOTEPLAY002_H
