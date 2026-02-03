
#ifndef ISTEAMUSER005_H
#define ISTEAMUSER005_H
#ifdef STEAM_WIN32
#pragma once
#endif

//-----------------------------------------------------------------------------
// Purpose: Functions for accessing and manipulating a steam account
//			associated with one client instance
//-----------------------------------------------------------------------------
class ISteamUser005
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
	virtual bool RequireShowVACBannedMessage( int nAppID ) = 0;
	virtual void AcknowledgeVACBanning( int nAppID ) = 0;
	virtual void SetSteam2Ticket( uint8 *pubTicket, int cubTicket ) = 0;
	virtual void AddServerNetAddress( uint32 unIP, uint16 unPort ) = 0;
	virtual bool SetEmail( const char *pchEmail ) = 0;
	virtual bool SetRegistryString( EConfigSubTree eRegistrySubTree, const char *pchKey, const char *pchValue ) = 0;
	virtual bool GetRegistryString( EConfigSubTree eRegistrySubTree, const char *pchKey, char *pchValue, int cbValue ) = 0;
	virtual bool SetRegistryInt( EConfigSubTree eRegistrySubTree, const char *pchKey, int iValue ) = 0;
	virtual bool GetRegistryInt( EConfigSubTree eRegistrySubTree, const char *pchKey, int *piValue ) = 0;
	virtual int InitiateGameConnection( void *pBlob, int cbMaxBlob, CSteamID steamID, CGameID gameID, uint32 unIPServer, uint16 usPortServer, bool bSecure ) = 0;
	virtual void TerminateGameConnection( uint32 unIPServer, uint16 usPortServer ) = 0;
	virtual void SetSelfAsPrimaryChatDestination() = 0;
	virtual bool IsPrimaryChatDestination() = 0;
	virtual void RequestLegacyCDKey( uint32 nAppID ) = 0;
	virtual bool SendGuestPassByEmail( const char *pchEmailAccount, GID_t gidGuestPassID, bool bResending ) = 0;
	virtual bool SendGuestPassByAccountID( uint32 uAccountID, GID_t gidGuestPassID, bool bResending ) = 0;
	virtual bool AckGuestPass(const char *pchGuestPassCode) = 0;
	virtual bool RedeemGuestPass(const char *pchGuestPassCode) = 0;
	virtual uint32 GetGuestPassToGiveCount() = 0;
	virtual uint32 GetGuestPassToRedeemCount() = 0;
	virtual RTime32 GetGuestPassLastUpdateTime() = 0;
	virtual bool GetGuestPassToGiveInfo( uint32 nPassIndex, GID_t *pgidGuestPassID, PackageId_t *pnPackageID, RTime32 *pRTime32Created, RTime32 *pRTime32Expiration, RTime32 *pRTime32Sent, RTime32 *pRTime32Redeemed, char *pchRecipientAddress, int cRecipientAddressSize ) = 0;
	virtual bool GetGuestPassToRedeemInfo( uint32 nPassIndex, GID_t *pgidGuestPassID, PackageId_t *pnPackageID, RTime32 *pRTime32Created, RTime32 *pRTime32Expiration, RTime32 *pRTime32Sent, RTime32 *pRTime32Redeemed) = 0;
	virtual bool GetGuestPassToRedeemSenderAddress( uint32 nPassIndex, char* pchSenderAddress, int cSenderAddressSize ) = 0;
	virtual bool GetGuestPassToRedeemSenderName( uint32 nPassIndex, char* pchSenderName, int cSenderNameSize ) = 0;
	virtual void AcknowledgeMessageByGID( const char *pchMessageGID ) = 0;
	virtual bool SetLanguage( const char *pchLanguage ) = 0;
	virtual void TrackAppUsageEvent( CGameID gameID, int eAppUsageEvent, const char *pchExtraInfo = "" ) = 0;
	virtual void SetAccountName( const char *pchAccountName ) = 0;
	virtual void SetPassword( const char *pchPassword ) = 0;
	virtual void SetAccountCreationTime( RTime32 rt ) = 0;
};


#endif // ISTEAMUSER005_H
