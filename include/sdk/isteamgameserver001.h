
#ifndef ISTEAMGAMESERVER001_H
#define ISTEAMGAMESERVER001_H
#ifdef STEAM_WIN32
#pragma once
#endif

// this interface version is not found in public SDK archives, it is based on reversing old Linux binaries

class ISteamGameServer001
{
public:
	virtual bool GSSendUserConnect( CSteamID steamID, uint32 unIPPublic, uint32 unk ) = 0;
	virtual bool GSSendUserDisconnect( CSteamID steamID ) = 0;
	virtual bool GSSendUserStatusResponse( CSteamID steamID, int nSecondsConnected, int nSecondsSinceLast ) = 0;
	virtual bool Obsolete_GSSetStatus( int32 nAppIdServed, uint32 unServerFlags, int cPlayers, int cPlayersMax, int cBotPlayers, int unGamePort, const char *pchServerName, const char *pchGameDir, const char *pchMapName, const char *pchVersion ) = 0;
};

#endif // ISTEAMGAMESERVER001_H
