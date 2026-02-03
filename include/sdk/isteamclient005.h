
#ifndef ISTEAMCLIENT005_H
#define ISTEAMCLIENT005_H
#ifdef STEAM_WIN32
#pragma once
#endif

// this interface version is not found in public SDK archives, it is based on reversing old Linux binaries

#include "isteammasterserverupdater.h"

class ISteamClient005
{
public:
	virtual HSteamPipe CreateSteamPipe() = 0;
	virtual bool BReleaseSteamPipe( HSteamPipe hSteamPipe ) = 0;
	virtual HSteamUser CreateGlobalUser( HSteamPipe *phSteamPipe ) = 0;
	virtual HSteamUser ConnectToGlobalUser( HSteamPipe hSteamPipe ) = 0;
	virtual HSteamUser CreateLocalUser( HSteamPipe *phSteamPipe ) = 0;
	virtual void ReleaseUser( HSteamPipe hSteamPipe, HSteamUser hUser ) = 0;
	virtual ISteamUser *GetISteamUser( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion ) = 0;
	virtual void *GetIVAC( HSteamUser hSteamUser ) = 0;
	virtual ISteamGameServer *GetISteamGameServer( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion ) = 0;
	virtual void SetLocalIPBinding( uint32 unIP, uint16 usPort ) = 0;
	virtual EUniverse GetConnectedUniverse() = 0;
	virtual const char *GetUniverseName( EUniverse eUniverse ) = 0;
	virtual ISteamFriends *GetISteamFriends( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion ) = 0;
	virtual bool BGetCallback( HSteamPipe hSteamPipe, CallbackMsg_t *pCallbackMsg, int *unk ) = 0;
	virtual void FreeLastCallback( HSteamPipe hSteamPipe ) = 0;
	virtual void SetEUniverse( EUniverse universe ) = 0;
};

#endif // ISTEAMCLIENT005_H
