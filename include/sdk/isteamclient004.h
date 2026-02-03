
#ifndef ISTEAMCLIENT004_H
#define ISTEAMCLIENT004_H
#ifdef STEAM_WIN32
#pragma once
#endif

// this interface version is not found in public SDK archives, it is based on reversing old Linux binaries

#include "isteammasterserverupdater.h"

class ISteamClient004
{
public:
	virtual HSteamUser CreateGlobalInstance() = 0;
	virtual HSteamUser ConnectToGlobalInstance() = 0;
	virtual HSteamUser CreateLocalInstance() = 0;
	virtual void ReleaseInstance( HSteamUser hSteamUser ) = 0;
	virtual ISteamUser *GetISteamUser( HSteamUser hSteamUser, const char *pchVersion ) = 0;
	virtual void *GetIVAC( HSteamUser hSteamUser ) = 0;
	virtual ISteamGameServer *GetISteamGameServer( HSteamUser hSteamUser, const char *pchVersion ) = 0;
	virtual void SetLocalIPBinding( uint32 unIP, uint16 usPort ) = 0;
};

#endif // ISTEAMCLIENT004_H
