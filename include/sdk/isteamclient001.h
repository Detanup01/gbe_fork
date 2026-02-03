
#ifndef ISTEAMCLIENT001_H
#define ISTEAMCLIENT001_H
#ifdef STEAM_WIN32
#pragma once
#endif

// this interface version is not found in public SDK archives, it is based on reversing old Linux binaries

#include "isteammasterserverupdater.h"

class ISteamClient001
{
public:
	virtual HSteamUser CreateGlobalInstance() = 0;
	virtual HSteamUser ConnectToGlobalInstance() = 0;
	virtual HSteamUser CreateLocalInstance() = 0;
	virtual void ReleaseInstance( HSteamUser hSteamUser ) = 0;
	virtual ISteamUser *GetISteamUser( HSteamUser hSteamUser, const char *pchVersion ) = 0;
	virtual void *GetIVAC( HSteamUser hSteamUser ) = 0;
	virtual bool BMainLoop( uint64 time, bool unk ) = 0;
	virtual void Test_SetSpew( const char *unk1, int unk2 ) = 0;
	virtual void Test_SetSpewFunc( void *unk ) = 0;
	virtual void Test_OverrideIPs( uint32 unIPPublic, uint32 unIPPrivate ) = 0;
	virtual void Test_SetServerLoadState( bool unk1, bool unk2 ) = 0;
	virtual void Test_SetStressMode( bool unk ) = 0;
	virtual int Test_GetStatsVConn() = 0;
};

#endif // ISTEAMCLIENT001_H
