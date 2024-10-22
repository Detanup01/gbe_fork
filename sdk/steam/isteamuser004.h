
#ifndef ISTEAMUSER004_H
#define ISTEAMUSER004_H
#ifdef STEAM_WIN32
#pragma once
#endif

enum EConfigSubTree {
    EConfigSubTree_stub
};

enum ELogonState {
    ELogonState_stub
};

class ISteamUser004
{
public:
	virtual HSteamUser GetHSteamUser() = 0;
	virtual void LogOn( CSteamID steamID ) = 0;
	virtual void LogOff() = 0;
	virtual bool BLoggedOn() = 0;
	virtual ELogonState GetLogonState() = 0;
	virtual bool BConnected() = 0;
	virtual CSteamID GetSteamID() = 0;
	virtual bool IsVACBanned( int nGameID ) = 0;
	virtual bool RequireShowVACBannedMessage( int nGameID ) = 0;
	virtual void AcknowledgeVACBanning( int nGameID ) = 0;

	// These are dead.
	virtual int NClientGameIDAdd( int nGameID ) = 0;
	virtual void RemoveClientGame( int nClientGameID ) = 0;
	virtual void SetClientGameServer( int nClientGameID, uint32 unIPServer, uint16 usPortServer ) = 0;

	virtual void SetSteam2Ticket( uint8 *pubTicket, int cubTicket ) = 0;
	virtual void AddServerNetAddress( uint32 unIP, uint16 unPort ) = 0;
	virtual bool SetEmail( const char *pchEmail ) = 0;

	// logon cookie - this is obsolete and never used
	virtual int GetSteamGameConnectToken( void *pBlob, int cbMaxBlob ) = 0;
	virtual bool SetRegistryString( EConfigSubTree eRegistrySubTree, const char *pchKey, const char *pchValue ) = 0;
	virtual bool GetRegistryString( EConfigSubTree eRegistrySubTree, const char *pchKey, char *pchValue, int cbValue ) = 0;
	virtual bool SetRegistryInt( EConfigSubTree eRegistrySubTree, const char *pchKey, int iValue ) = 0;
	virtual bool GetRegistryInt( EConfigSubTree eRegistrySubTree, const char *pchKey, int *piValue ) = 0;
	virtual int InitiateGameConnection( void *pBlob, int cbMaxBlob, CSteamID steamID, int nGameAppID, uint32 unIPServer, uint16 usPortServer, bool bSecure ) = 0;
	virtual void TerminateGameConnection( uint32 unIPServer, uint16 usPortServer ) = 0;
	virtual void SetSelfAsPrimaryChatDestination() = 0;
	virtual bool IsPrimaryChatDestination() = 0;
	virtual void RequestLegacyCDKey( uint32 iAppID ) = 0;
};


#endif // ISTEAMUSER004_H
