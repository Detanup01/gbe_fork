
#ifndef ISTEAMFRIENDS006_H
#define ISTEAMFRIENDS006_H
#ifdef STEAM_WIN32
#pragma once
#endif


//-----------------------------------------------------------------------------
// Purpose: interface to accessing information about individual users,
//			that can be a friend, in a group, on a game server or in a lobby with the local user
//-----------------------------------------------------------------------------
class ISteamFriends006
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

	// friend iteration
	// takes a set of k_EFriendFlags, and returns the number of users the client knows about who meet that criteria
	// then GetFriendByIndex() can then be used to return the id's of each of those users
	virtual int GetFriendCount( EFriendFlags eFriendFlags ) = 0;

	// returns the steamID of a user
	// iFriend is a index of range [0, GetFriendCount())
	// iFriendsFlags must be the same value as used in GetFriendCount()
	// the returned CSteamID can then be used by all the functions below to access details about the user
	STEAMWORKS_STRUCT_RETURN_2(CSteamID, GetFriendByIndex, int, iFriend, int, iFriendFlags) /*virtual CSteamID GetFriendByIndex( int iFriend, int iFriendFlags ) = 0;*/

	// returns a relationship to a user
	virtual EFriendRelationship GetFriendRelationship( CSteamID steamIDFriend ) = 0;

	// returns the current status of the specified user
	// this will only be known by the local user if steamIDFriend is in their friends list; on the same game server; in a chat room or lobby; or in a small group with the local user
	virtual EPersonaState GetFriendPersonaState( CSteamID steamIDFriend ) = 0;

	// returns the name another user - guaranteed to not be NULL.
	// same rules as GetFriendPersonaState() apply as to whether or not the user knowns the name of the other user
	// note that on first joining a lobby, chat room or game server the local user will not known the name of the other users automatically; that information will arrive asyncronously
	virtual const char *GetFriendPersonaName( CSteamID steamIDFriend ) = 0;

	// gets the avatar of the current user, which is a handle to be used in IClientUtils::GetImageRGBA(), or 0 if none set
	virtual int GetFriendAvatar( CSteamID steamIDFriend, int eAvatarSize ) = 0;
	// returns true if the friend is actually in a game, and fills in pFriendGameInfo with an extra details 
	virtual bool GetFriendGamePlayed( CSteamID steamIDFriend, FriendGameInfo_t *pFriendGameInfo ) = 0;
	// accesses old friends names - returns an empty string when their are no more items in the history
	virtual const char *GetFriendPersonaNameHistory( CSteamID steamIDFriend, int iPersonaName ) = 0;

	// returns true if the specified user meets any of the criteria specified in iFriendFlags
	// iFriendFlags can be the union (binary or, |) of one or more k_EFriendFlags values
	virtual bool HasFriend( CSteamID steamIDFriend, EFriendFlags eFriendFlags ) = 0;

	// clan (group) iteration and access functions
	virtual int GetClanCount() = 0;
	STEAMWORKS_STRUCT_RETURN_1(CSteamID, GetClanByIndex, int, iClan) /*virtual CSteamID GetClanByIndex( int iClan ) = 0;*/
	virtual const char *GetClanName( CSteamID steamIDClan ) = 0;
	virtual const char *GetClanTag( CSteamID steamIDClan ) = 0;

	// iterators for getting users in a chat room, lobby, game server or clan
	// note that large clans that cannot be iterated by the local user
	// steamIDSource can be the steamID of a group, game server, lobby or chat room
	virtual int GetFriendCountFromSource( CSteamID steamIDSource ) = 0;
	STEAMWORKS_STRUCT_RETURN_2(CSteamID, GetFriendFromSourceByIndex, CSteamID, steamIDSource, int, iFriend) /*virtual CSteamID GetFriendFromSourceByIndex( CSteamID steamIDSource, int iFriend ) = 0;*/

	// returns true if the local user can see that steamIDUser is a member or in steamIDSource
	virtual bool IsUserInSource( CSteamID steamIDUser, CSteamID steamIDSource ) = 0;

	// User is in a game pressing the talk button (will suppress the microphone for all voice comms from the Steam friends UI)
	virtual void SetInGameVoiceSpeaking( CSteamID steamIDUser, bool bSpeaking ) = 0;

	// activates the game overlay, with an optional dialog to open 
	// valid options are "Friends", "Community", "Players", "Settings", "LobbyInvite", "OfficialGameGroup"
	virtual void ActivateGameOverlay( const char *pchDialog ) = 0;

	// activates game overlay to a specific place
	// valid options are
	//		"steamid" - opens the overlay web browser to the specified user or groups profile
	//		"chat" - opens a chat window to the specified user, or joins the group chat 
	virtual void ActivateGameOverlayToUser( const char *pchDialog, CSteamID steamID ) = 0;

	// activates game overlay web browser directly to the specified URL
	// full address with protocol type is required, e.g. http://www.steamgames.com/
	virtual void ActivateGameOverlayToWebPage( const char *pchURL ) = 0;

	// activates game overlay to store page for app
	virtual void ActivateGameOverlayToStore( AppId_t nAppID ) = 0;

	// Mark a target user as 'played with'. This is a client-side only feature that requires that the calling user is 
	// in game 
	virtual void SetPlayedWith( CSteamID steamIDUserPlayedWith ) = 0;

	// activates game overlay to open the invite dialog. Invitations will be sent for the provided lobby.
	// You can also use ActivateGameOverlay( "LobbyInvite" ) to allow the user to create invitations for their current public lobby.
	virtual void ActivateGameOverlayInviteDialog( CSteamID steamIDLobby ) = 0;
};



#endif // ISTEAMFRIENDS006_H
