
#ifndef ISTEAMGAMESERVER002_H
#define ISTEAMGAMESERVER002_H
#ifdef STEAM_WIN32
#pragma once
#endif

// this interface version is not found in public SDK archives, it is based on proton src: https://github.com/ValveSoftware/Proton/tree/proton_9.0/lsteamclient

class ISteamGameServer002
{
public:
	virtual void LogOn() = 0;
	virtual void LogOff() = 0;
	virtual bool BLoggedOn() = 0;
	virtual void GSSetSpawnCount( uint32 ucSpawn ) = 0;
	virtual bool GSGetSteam2GetEncryptionKeyToSendToNewClient( void *pvEncryptionKey, uint32 *pcbEncryptionKey, uint32 cbMaxEncryptionKey ) = 0;
	virtual bool GSSendSteam2UserConnect(  uint32 unUserID, const void *pvRawKey, uint32 unKeyLen, uint32 unIPPublic, uint16 usPort, const void *pvCookie, uint32 cubCookie ) = 0;
	virtual bool GSSendSteam3UserConnect( CSteamID steamID, uint32 unIPPublic, const void *pvCookie, uint32 cubCookie ) = 0;
	virtual bool GSRemoveUserConnect( uint32 unUserID ) = 0;
	virtual bool GSSendUserDisconnect( CSteamID steamID, uint32 unUserID ) = 0;
	virtual bool GSSendUserStatusResponse( CSteamID steamID, int nSecondsConnected, int nSecondsSinceLast ) = 0;
	virtual bool Obsolete_GSSetStatus( int32 nAppIdServed, uint32 unServerFlags, int cPlayers, int cPlayersMax, int cBotPlayers, int unGamePort, const char *pchServerName, const char *pchGameDir, const char *pchMapName, const char *pchVersion ) = 0;
	virtual bool GSUpdateStatus( int cPlayers, int cPlayersMax, int cBotPlayers, const char *pchServerName, const char *pchMapName ) = 0;
	virtual bool BSecure() = 0;
	virtual CSteamID GetSteamID() = 0;
	virtual bool GSSetServerType( int32 nGameAppId, uint32 unServerFlags, uint32 unGameIP, uint32 unGamePort, const char *pchGameDir, const char *pchVersion ) = 0;
	virtual bool GSSetServerType2( int32 nGameAppId, uint32 unServerFlags, uint32 unGameIP, uint16 unGamePort, uint16 unSpectatorPort, uint16 usQueryPort, const char *pchGameDir, const char *pchVersion, bool bLANMode ) = 0;
	virtual bool GSUpdateStatus2( int cPlayers, int cPlayersMax, int cBotPlayers, const char *pchServerName, const char *pSpectatorServerName, const char *pchMapName ) = 0;
	virtual bool GSCreateUnauthenticatedUser( CSteamID *pSteamID ) = 0;
	virtual bool GSSetUserData( CSteamID steamID, const char *pPlayerName, uint32 nFrags ) = 0;
	virtual void GSUpdateSpectatorPort( uint16 unSpectatorPort ) = 0;
	virtual void GSSetGameType( const char *pchType ) = 0;
};

#endif // ISTEAMGAMESERVER002_H
