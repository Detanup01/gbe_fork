
#ifndef ISTEAMUSER004_OLD_H
#define ISTEAMUSER004_OLD_H
#ifdef STEAM_WIN32
#pragma once
#endif

// this interface version is not found in public SDK archives, it is based on reversing old Linux binaries

#include "isteamfriends.h"

class ISteamUser004_old
{
public:
	virtual bool BGetCallback( int *piCallback, uint8 **ppubParam, int *unk ) = 0;
	virtual void FreeLastCallback() = 0;
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
	virtual int NClientGameIDAdd( int nGameID ) = 0;
	virtual void RemoveClientGame( int nClientGameID ) = 0;
	virtual void SetClientGameServer( int nClientGameID, uint32 unIPServer, uint16 usPortServer ) = 0;

	virtual const char *GetPlayerName() = 0;
	virtual void SetPlayerName( const char *pchPersonaName ) = 0;
	virtual EPersonaState GetFriendStatus() = 0;
	virtual void SetFriendStatus( EPersonaState ePersonaState ) = 0;
	virtual bool AddFriend( CSteamID steamIDFriend ) = 0;
	virtual bool RemoveFriend( CSteamID steamIDFriend ) = 0;
	virtual bool HasFriend( CSteamID steamIDFriend ) = 0;
	virtual EFriendRelationship GetFriendRelationship( CSteamID steamIDFriend ) = 0;
	virtual EPersonaState GetFriendStatus( CSteamID steamIDFriend ) = 0;
	virtual bool GetFriendGamePlayed( CSteamID steamIDFriend, int32 *pnGameID, uint32 *punGameIP, uint16 *pusGamePort ) = 0;
	virtual const char *GetPlayerName( CSteamID steamIDFriend ) = 0;
	virtual int32 AddFriendByName( const char *pchEmailOrAccountName ) = 0;

	virtual void SetSteam2Ticket( uint8 *pubTicket, int cubTicket ) = 0;
	virtual void AddServerNetAddress( uint32 unIP, uint16 unPort ) = 0;
	virtual bool SetEmail( const char *pchEmail ) = 0;
	virtual int GetSteamTicket( void *pBlob, int cbMaxBlob ) = 0;
};

#endif // ISTEAMUSER004_OLD_H
