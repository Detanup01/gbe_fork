
#ifndef ISTEAMFRIENDS002_H
#define ISTEAMFRIENDS002_H
#ifdef STEAM_WIN32
#pragma once
#endif


//-----------------------------------------------------------------------------
// Purpose: interface to accessing information about individual users,
//			that can be a friend, in a group, on a game server or in a lobby with the local user
//-----------------------------------------------------------------------------
class ISteamFriends002
{
public:
	// returns the local players name - guaranteed to not be NULL.
	// this is the same name as on the users community profile page
	// this is stored in UTF-8 format
	// like all the other interface functions that return a char *, it's important that this pointer is not saved
	// off; it will eventually be free'd or re-allocated
	virtual const char *GetPersonaName() = 0;
	
	// sets the player name, stores it on the server and publishes the changes to all friends who are online
	virtual void SetPersonaName_old( const char *pchPersonaName ) = 0;

	// gets the status of the current user
	virtual EPersonaState GetPersonaState() = 0;

	virtual void SetPersonaState( EPersonaState ePersonaState ) = 0;

	// friend iteration
	// takes a set of k_EFriendFlags, and returns the number of users the client knows about who meet that criteria
	// then GetFriendByIndex() can then be used to return the id's of each of those users
	virtual int GetFriendCount( int iFriendFlags ) = 0;

	// returns the steamID of a user
	// iFriend is a index of range [0, GetFriendCount())
	// iFriendsFlags must be the same value as used in GetFriendCount()
	// the returned CSteamID can then be used by all the functions below to access details about the user
	virtual CSteamID GetFriendByIndex( int iFriend, int iFriendFlags ) = 0;

	// returns a relationship to a user
	virtual EFriendRelationship GetFriendRelationship( CSteamID steamIDFriend ) = 0;

	// returns the current status of the specified user
	// this will only be known by the local user if steamIDFriend is in their friends list; on the same game server; in a chat room or lobby; or in a small group with the local user
	virtual EPersonaState GetFriendPersonaState( CSteamID steamIDFriend ) = 0;

	// returns the name another user - guaranteed to not be NULL.
	// same rules as GetFriendPersonaState() apply as to whether or not the user knowns the name of the other user
	// note that on first joining a lobby, chat room or game server the local user will not known the name of the other users automatically; that information will arrive asyncronously
	// 
	virtual const char *GetFriendPersonaName( CSteamID steamIDFriend ) = 0;

	virtual void SetFriendRegValue( CSteamID steamIDFriend, const char *pchKey, const char *pchValue ) = 0;
	virtual const char *GetFriendRegValue( CSteamID steamIDFriend, const char *pchKey ) = 0;

	// returns true if the friend is actually in a game
	virtual bool GetFriendGamePlayed( CSteamID steamIDFriend, uint64 *pulGameID, uint32 *punGameIP, uint16 *pusGamePort, uint16 *pusQueryPort ) = 0;
	// accesses old friends names - returns an empty string when their are no more items in the history
	virtual const char *GetFriendPersonaNameHistory( CSteamID steamIDFriend, int iPersonaName ) = 0;

	// returns true if the specified user meets any of the criteria specified in iFriendFlags
	// iFriendFlags can be the union (binary or, |) of one or more k_EFriendFlags values
	virtual bool AddFriend( CSteamID steamIDFriend ) = 0;
	virtual bool RemoveFriend( CSteamID steamIDFriend ) = 0;
	virtual bool HasFriend( CSteamID steamIDFriend, int iFriendFlags ) = 0;

	virtual int32 AddFriendByName( const char *pchEmailOrAccountName ) = 0;
	virtual bool InviteFriendByEmail( const char *pchEmailAccount ) = 0;
	virtual int GetChatMessage( CSteamID steamIDFriend, int iChatID, void *pvData, int cubData, EChatEntryType *peChatEntryType ) = 0;
	virtual bool SendMsgToFriend( CSteamID steamIDFriend, EChatEntryType eChatEntryType, const void *pvMsgBody, int cubMsgBody ) = 0;
	virtual int GetChatIDOfChatHistoryStart( CSteamID steamIDFriend ) = 0;
	virtual void SetChatHistoryStart( CSteamID steamIDFriend, int iChatID ) = 0;
	virtual void ClearChatHistory( CSteamID steamIDFriend ) = 0;

	// clan (group) iteration and access functions
	virtual int GetClanCount() = 0;
	virtual CSteamID GetClanByIndex( int iClan ) = 0;
	virtual const char *GetClanName( CSteamID steamIDClan ) = 0;

	virtual bool InviteFriendToClan( CSteamID steamIDFriend, CSteamID steamIDClan ) = 0;
	virtual bool AcknowledgeInviteToClan( CSteamID steamIDClan, bool bAcceptOrDenyClanInvite ) = 0;

	// iterators for getting users in a chat room, lobby, game server or clan
	// note that large clans that cannot be iterated by the local user
	// steamIDSource can be the steamID of a group, game server, lobby or chat room
	virtual int GetFriendCountFromSource( CSteamID steamIDSource ) = 0;
	virtual CSteamID GetFriendFromSourceByIndex( CSteamID steamIDSource, int iFriend ) = 0;
};


#endif // ISTEAMFRIENDS002_H
