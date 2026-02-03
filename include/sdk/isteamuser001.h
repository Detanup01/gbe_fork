
#ifndef ISTEAMUSER001_H
#define ISTEAMUSER001_H
#ifdef STEAM_WIN32
#pragma once
#endif

// this interface version is not found in public SDK archives, it is based on reversing old Linux binaries

class ICMCallback001
{
public:
	virtual ~ICMCallback001() {}

	virtual void OnLogonSuccess() = 0;
	virtual void OnLogonFailure( EResult result ) = 0;
	virtual void OnLoggedOff() = 0;
	virtual void HandleVACChallenge( int unk1, void *unk2, int unk3 ) = 0;
	virtual void GSHandleClientApprove( CSteamID *steamID ) = 0;
	virtual void GSHandleClientDeny( CSteamID *steamID, EDenyReason reason ) = 0;
	virtual void GSHandleClientKick( CSteamID *steamID, EDenyReason reason ) = 0;
};

class ISteam2Auth
{
public:
	virtual int GetValue( const char *var, char *buf, int bufsize ) = 0;
	virtual int GetServerReadableTicket( uint32 unk1, uint32 unk2, void *unk3, uint32 unk4, uint32 *unk5 ) = 0;
};

class ISteamUser001
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
	virtual bool GSSetStatus( int32 nAppIdServed, uint32 unServerFlags, int cPlayers, int cPlayersMax ) = 0;
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
	virtual void Test_Heartbeat() = 0;
	virtual void Test_FakeDisconnect() = 0;
	virtual EUniverse Test_GetEUniverse() = 0;
};

#endif // ISTEAMUSER001_H
