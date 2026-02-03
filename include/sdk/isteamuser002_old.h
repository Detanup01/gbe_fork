
#ifndef ISTEAMUSER002_OLD_H
#define ISTEAMUSER002_OLD_H
#ifdef STEAM_WIN32
#pragma once
#endif

// this interface version is not found in public SDK archives, it is based on reversing old Linux binaries

class ISteamUser002_old
{
public:
	virtual void Init( ICMCallback001 *cmcallback, ISteam2Auth *steam2auth ) = 0;
	virtual int ProcessCall( int unk ) = 0;
	virtual void LogOn( CSteamID *steamID ) = 0;
	virtual void LogOff() = 0;
	virtual bool BLoggedOn() = 0;
	virtual bool BConnected() = 0;
	virtual int CreateAccount( const char *unk1, void *unk2, void *unk3, const char *unk4, int unk5, void *unk6 ) = 0;
	virtual bool IsVACBanned( int nGameID ) = 0;
	virtual bool RequireShowVACBannedMessage( int nGameID ) = 0;
	virtual void AcknowledgeVACBanning( int nGameID ) = 0;
	virtual bool GSSendLogonRequest( CSteamID *steamID ) = 0;
	virtual bool GSSendDisconnect( CSteamID *steamID ) = 0;
	virtual bool GSSendStatusResponse( CSteamID *steamID, int nSecondsConnected, int nSecondsSinceLast ) = 0;
	virtual bool GSSetStatus( int32 nAppIdServed, uint32 unServerFlags, int cPlayers, int cPlayersMax, int cBotPlayers, int unGamePort, const char *pchServerName, const char *pchGameDir, const char *pchMapName, const char *pchVersion ) = 0;
	virtual int NClientGameIDAdd( int nGameID ) = 0;
	virtual void RemoveClientGame( int nClientGameID ) = 0;
	virtual void SetClientGameServer( int nClientGameID, uint32 unIPServer, uint16 usPortServer ) = 0;
	virtual void Test_SuspendActivity() = 0;
	virtual void Test_ResumeActivity() = 0;
	virtual void Test_SendVACResponse( int unk1, void *unk2, int unk3 ) = 0;
	virtual void Test_SetFakePrivateIP( uint32 ip ) = 0;
	virtual void Test_SendBigMessage() = 0;
	virtual bool Test_BBigMessageResponseReceived() = 0;
	virtual void Test_SetPktLossPct( int unk1 ) = 0;
	virtual void Test_SetForceTCP( bool unk1 ) = 0;
	virtual void Test_SetMaxUDPConnectionAttempts( int unk1 ) = 0;
	virtual void Test_Heartbeat() = 0;
	virtual void Test_FakeDisconnect() = 0;
	virtual EUniverse Test_GetEUniverse() = 0;
};

#endif // ISTEAMUSER002_OLD_H
