/* Copyright (C) 2019 Mr Goldberg
   This file is part of the Goldberg Emulator

   The Goldberg Emulator is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   The Goldberg Emulator is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the Goldberg Emulator; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef __INCLUDED_STEAM_FRIENDS_H__
#define __INCLUDED_STEAM_FRIENDS_H__

#include "base.h"
#include "overlay/steam_overlay.h"

struct Avatar_Numbers {
    int smallest{};
    int medium{};
    int large{};
};

class Steam_Friends : 
public ISteamFriends001,
public ISteamFriends002,
public ISteamFriends003,
public ISteamFriends004,
public ISteamFriends005,
public ISteamFriends006,
public ISteamFriends007,
public ISteamFriends008,
public ISteamFriends009,
public ISteamFriends010,
public ISteamFriends011,
public ISteamFriends012,
public ISteamFriends013,
public ISteamFriends014,
public ISteamFriends015,
public ISteamFriends016,
public ISteamFriends
{
    class Settings *settings{};
    class Local_Storage* local_storage{};
    class Networking *network{};
    class SteamCallBacks *callbacks{};
    class SteamCallResults *callback_results{};
    class RunEveryRunCB *run_every_runcb{};
    class Steam_Overlay* overlay{};

    Friend us{};
    bool modified{};
    std::vector<Friend> friends{};

    std::map<uint64, struct Avatar_Numbers> avatars{};
    CSteamID lobby_id{};

    std::chrono::high_resolution_clock::time_point last_sent_friends{};
    std::map<std::string, std::string> reg{};
    std::string reg_nullptr{};

    Friend *find_friend(CSteamID id);

    void persona_change(CSteamID id, EPersonaChange flags);

    void rich_presence_updated(CSteamID id, AppId_t appid);

    bool is_appid_compatible(Friend *f);

    struct Avatar_Numbers add_friend_avatars(CSteamID id);

    static bool ok_friend_flags(int iFriendFlags);

    static void steam_friends_callback(void *object, Common_Message *msg);
    static void steam_friends_run_every_runcb(void *object);

    void RunCallbacks();
    void Callback(Common_Message *msg);

public:
    Steam_Friends(Settings* settings, class Local_Storage* local_storage, Networking* network, SteamCallResults* callback_results, SteamCallBacks* callbacks, RunEveryRunCB* run_every_runcb, Steam_Overlay* overlay);
    ~Steam_Friends();

    void resend_friend_data();

    // returns the local players name - guaranteed to not be NULL.
    // this is the same name as on the users community profile page
    // this is stored in UTF-8 format
    // like all the other interface functions that return a char *, it's important that this pointer is not saved
    // off; it will eventually be free'd or re-allocated
    const char *GetPersonaName();

    // Sets the player name, stores it on the server and publishes the changes to all friends who are online.
    // Changes take place locally immediately, and a PersonaStateChange_t is posted, presuming success.
    //
    // The final results are available through the return value SteamAPICall_t, using SetPersonaNameResponse_t.
    //
    // If the name change fails to happen on the server, then an additional global PersonaStateChange_t will be posted
    // to change the name back, in addition to the SetPersonaNameResponse_t callback.
    STEAM_CALL_RESULT( SetPersonaNameResponse_t )
    SteamAPICall_t SetPersonaName( const char *pchPersonaName );

    void SetPersonaName_old( const char *pchPersonaName );

    // gets the status of the current user
    EPersonaState GetPersonaState();

	void SetPersonaState( EPersonaState ePersonaState );
	bool AddFriend( CSteamID steamIDFriend );
	bool RemoveFriend( CSteamID steamIDFriend );
	bool HasFriend( CSteamID steamIDFriend );


    // friend iteration
    // takes a set of k_EFriendFlags, and returns the number of users the client knows about who meet that criteria
    // then GetFriendByIndex() can then be used to return the id's of each of those users
    int GetFriendCount( int iFriendFlags );

    int GetFriendCount( EFriendFlags eFriendFlags );

    // returns the steamID of a user
    // iFriend is a index of range [0, GetFriendCount())
    // iFriendsFlags must be the same value as used in GetFriendCount()
    // the returned CSteamID can then be used by all the functions below to access details about the user
    CSteamID GetFriendByIndex( int iFriend, int iFriendFlags );

    void GetFriendByIndex(CSteamID& res, int iFriend, int iFriendFlags );

    CSteamID GetFriendByIndex( int iFriend, EFriendFlags eFriendFlags );

    void GetFriendByIndex(CSteamID& result, int iFriend, EFriendFlags eFriendFlags);

    // returns a relationship to a user
    EFriendRelationship GetFriendRelationship( CSteamID steamIDFriend );


    // returns the current status of the specified user
    // this will only be known by the local user if steamIDFriend is in their friends list; on the same game server; in a chat room or lobby; or in a small group with the local user
    EPersonaState GetFriendPersonaState( CSteamID steamIDFriend );

    bool Deprecated_GetFriendGamePlayed( CSteamID steamIDFriend, int32 *pnGameID, uint32 *punGameIP, uint16 *pusGamePort );


    // returns the name another user - guaranteed to not be NULL.
    // same rules as GetFriendPersonaState() apply as to whether or not the user knowns the name of the other user
    // note that on first joining a lobby, chat room or game server the local user will not known the name of the other users automatically; that information will arrive asyncronously
    // 
    const char *GetFriendPersonaName( CSteamID steamIDFriend );

	int32 AddFriendByName( const char *pchEmailOrAccountName );
	int GetFriendCount();
	CSteamID GetFriendByIndex( int iFriend );
	void SendMsgToFriend( CSteamID steamIDFriend, EChatEntryType eChatEntryType, const char *pchMsgBody );
	void SetFriendRegValue( CSteamID steamIDFriend, const char *pchKey, const char *pchValue );
	const char *GetFriendRegValue( CSteamID steamIDFriend, const char *pchKey );
	int GetChatMessage( CSteamID steamIDFriend, int iChatID, void *pvData, int cubData, EChatEntryType *peChatEntryType );

	bool SendMsgToFriend( CSteamID steamIDFriend, EChatEntryType eChatEntryType, const void *pvMsgBody, int cubMsgBody );
	int GetChatIDOfChatHistoryStart( CSteamID steamIDFriend );
	void SetChatHistoryStart( CSteamID steamIDFriend, int iChatID );
	void ClearChatHistory( CSteamID steamIDFriend );
	bool InviteFriendByEmail( const char *pchEmailAccount );
	int GetBlockedFriendCount();
	bool GetFriendGamePlayed( CSteamID steamIDFriend, uint64 *pulGameID, uint32 *punGameIP, uint16 *pusGamePort );
	bool GetFriendGamePlayed2( CSteamID steamIDFriend, uint64 *pulGameID, uint32 *punGameIP, uint16 *pusGamePort, uint16 *pusQueryPort );


    // returns true if the friend is actually in a game, and fills in pFriendGameInfo with an extra details 
    bool GetFriendGamePlayed( CSteamID steamIDFriend, STEAM_OUT_STRUCT() FriendGameInfo_t *pFriendGameInfo );

    bool GetFriendGamePlayed( CSteamID steamIDFriend, uint64 *pulGameID, uint32 *punGameIP, uint16 *pusGamePort, uint16 *pusQueryPort );

    // accesses old friends names - returns an empty string when their are no more items in the history
    const char *GetFriendPersonaNameHistory( CSteamID steamIDFriend, int iPersonaName );

    // friends steam level
    int GetFriendSteamLevel( CSteamID steamIDFriend );


    // Returns nickname the current user has set for the specified player. Returns NULL if the no nickname has been set for that player.
    const char *GetPlayerNickname( CSteamID steamIDPlayer );


    // friend grouping (tag) apis
    // returns the number of friends groups
    int GetFriendsGroupCount();

    // returns the friends group ID for the given index (invalid indices return k_FriendsGroupID_Invalid)
    FriendsGroupID_t GetFriendsGroupIDByIndex( int iFG );

    // returns the name for the given friends group (NULL in the case of invalid friends group IDs)
    const char *GetFriendsGroupName( FriendsGroupID_t friendsGroupID );

    // returns the number of members in a given friends group
    int GetFriendsGroupMembersCount( FriendsGroupID_t friendsGroupID );

    // gets up to nMembersCount members of the given friends group, if fewer exist than requested those positions' SteamIDs will be invalid
    void GetFriendsGroupMembersList( FriendsGroupID_t friendsGroupID, STEAM_OUT_ARRAY_CALL(nMembersCount, GetFriendsGroupMembersCount, friendsGroupID ) CSteamID *pOutSteamIDMembers, int nMembersCount );


    // returns true if the specified user meets any of the criteria specified in iFriendFlags
    // iFriendFlags can be the union (binary or, |) of one or more k_EFriendFlags values
    bool HasFriend( CSteamID steamIDFriend, int iFriendFlags );

    bool HasFriend( CSteamID steamIDFriend, EFriendFlags eFriendFlags );

    // clan (group) iteration and access functions
    int GetClanCount();

    CSteamID GetClanByIndex( int iClan );

    void GetClanByIndex(CSteamID& result, int iClan );

    const char *GetClanName( CSteamID steamIDClan );

	bool InviteFriendToClan( CSteamID steamIDFriend, CSteamID steamIDClan );
	bool AcknowledgeInviteToClan( CSteamID steamIDClan, bool bAcceptOrDenyClanInvite );

    const char *GetClanTag( CSteamID steamIDClan );

    // returns the most recent information we have about what's happening in a clan
    bool GetClanActivityCounts( CSteamID steamIDClan, int *pnOnline, int *pnInGame, int *pnChatting );

    // for clans a user is a member of, they will have reasonably up-to-date information, but for others you'll have to download the info to have the latest
    SteamAPICall_t DownloadClanActivityCounts( STEAM_ARRAY_COUNT(cClansToRequest) CSteamID *psteamIDClans, int cClansToRequest );


    // iterators for getting users in a chat room, lobby, game server or clan
    // note that large clans that cannot be iterated by the local user
    // note that the current user must be in a lobby to retrieve CSteamIDs of other users in that lobby
    // steamIDSource can be the steamID of a group, game server, lobby or chat room
    int GetFriendCountFromSource( CSteamID steamIDSource );

    CSteamID GetFriendFromSourceByIndex( CSteamID steamIDSource, int iFriend );

    void GetFriendFromSourceByIndex(CSteamID& res, CSteamID steamIDSource, int iFriend);


    // returns true if the local user can see that steamIDUser is a member or in steamIDSource
    bool IsUserInSource( CSteamID steamIDUser, CSteamID steamIDSource );


    // User is in a game pressing the talk button (will suppress the microphone for all voice comms from the Steam friends UI)
    void SetInGameVoiceSpeaking( CSteamID steamIDUser, bool bSpeaking );


    // activates the game overlay, with an optional dialog to open 
    // valid options are "Friends", "Community", "Players", "Settings", "OfficialGameGroup", "Stats", "Achievements"
    void ActivateGameOverlay( const char *pchDialog );


    // activates game overlay to a specific place
    // valid options are
    //		"steamid" - opens the overlay web browser to the specified user or groups profile
    //		"chat" - opens a chat window to the specified user, or joins the group chat 
    //		"jointrade" - opens a window to a Steam Trading session that was started with the ISteamEconomy/StartTrade Web API
    //		"stats" - opens the overlay web browser to the specified user's stats
    //		"achievements" - opens the overlay web browser to the specified user's achievements
    //		"friendadd" - opens the overlay in minimal mode prompting the user to add the target user as a friend
    //		"friendremove" - opens the overlay in minimal mode prompting the user to remove the target friend
    //		"friendrequestaccept" - opens the overlay in minimal mode prompting the user to accept an incoming friend invite
    //		"friendrequestignore" - opens the overlay in minimal mode prompting the user to ignore an incoming friend invite
    void ActivateGameOverlayToUser( const char *pchDialog, CSteamID steamID );


    // activates game overlay web browser directly to the specified URL
    // full address with protocol type is required, e.g. http://www.steamgames.com/
    void ActivateGameOverlayToWebPage( const char *pchURL, EActivateGameOverlayToWebPageMode eMode = k_EActivateGameOverlayToWebPageMode_Default );

    void ActivateGameOverlayToWebPage( const char *pchURL );

    // activates game overlay to store page for app
    void ActivateGameOverlayToStore( AppId_t nAppID, EOverlayToStoreFlag eFlag );

    void ActivateGameOverlayToStore( AppId_t nAppID);

    // Mark a target user as 'played with'. This is a client-side only feature that requires that the calling user is 
    // in game 
    void SetPlayedWith( CSteamID steamIDUserPlayedWith );


    // activates game overlay to open the invite dialog. Invitations will be sent for the provided lobby.
    void ActivateGameOverlayInviteDialog( CSteamID steamIDLobby );

    // gets the small (32x32) avatar of the current user, which is a handle to be used in IClientUtils::GetImageRGBA(), or 0 if none set
    int GetSmallFriendAvatar( CSteamID steamIDFriend );


    // gets the medium (64x64) avatar of the current user, which is a handle to be used in IClientUtils::GetImageRGBA(), or 0 if none set
    int GetMediumFriendAvatar( CSteamID steamIDFriend );


    // gets the large (184x184) avatar of the current user, which is a handle to be used in IClientUtils::GetImageRGBA(), or 0 if none set
    // returns -1 if this image has yet to be loaded, in this case wait for a AvatarImageLoaded_t callback and then call this again
    int GetLargeFriendAvatar( CSteamID steamIDFriend );

    int GetFriendAvatar( CSteamID steamIDFriend, int eAvatarSize );

    int GetFriendAvatar(CSteamID steamIDFriend);

    // requests information about a user - persona name & avatar
    // if bRequireNameOnly is set, then the avatar of a user isn't downloaded 
    // - it's a lot slower to download avatars and churns the local cache, so if you don't need avatars, don't request them
    // if returns true, it means that data is being requested, and a PersonaStateChanged_t callback will be posted when it's retrieved
    // if returns false, it means that we already have all the details about that user, and functions can be called immediately
    bool RequestUserInformation( CSteamID steamIDUser, bool bRequireNameOnly );


    // requests information about a clan officer list
    // when complete, data is returned in ClanOfficerListResponse_t call result
    // this makes available the calls below
    // you can only ask about clans that a user is a member of
    // note that this won't download avatars automatically; if you get an officer,
    // and no avatar image is available, call RequestUserInformation( steamID, false ) to download the avatar
    STEAM_CALL_RESULT( ClanOfficerListResponse_t )
    SteamAPICall_t RequestClanOfficerList( CSteamID steamIDClan );


    // iteration of clan officers - can only be done when a RequestClanOfficerList() call has completed

    // returns the steamID of the clan owner
    CSteamID GetClanOwner( CSteamID steamIDClan );

    void GetClanOwner(CSteamID& res, CSteamID steamIDClan );

    // returns the number of officers in a clan (including the owner)
    int GetClanOfficerCount( CSteamID steamIDClan );

    // returns the steamID of a clan officer, by index, of range [0,GetClanOfficerCount)
    CSteamID GetClanOfficerByIndex( CSteamID steamIDClan, int iOfficer );

    void GetClanOfficerByIndex(CSteamID& res, CSteamID steamIDClan, int iOfficer );

    // if current user is chat restricted, he can't send or receive any text/voice chat messages.
    // the user can't see custom avatars. But the user can be online and send/recv game invites.
    // a chat restricted user can't add friends or join any groups.
    uint32 GetUserRestrictions();

    EUserRestriction GetUserRestrictions_old();

    // Rich Presence data is automatically shared between friends who are in the same game
    // Each user has a set of Key/Value pairs
    // Note the following limits: k_cchMaxRichPresenceKeys, k_cchMaxRichPresenceKeyLength, k_cchMaxRichPresenceValueLength
    // There are two magic keys:
    //		"status"  - a UTF-8 string that will show up in the 'view game info' dialog in the Steam friends list
    //		"connect" - a UTF-8 string that contains the command-line for how a friend can connect to a game
    // GetFriendRichPresence() returns an empty string "" if no value is set
    // SetRichPresence() to a NULL or an empty string deletes the key
    // You can iterate the current set of keys for a friend with GetFriendRichPresenceKeyCount()
    // and GetFriendRichPresenceKeyByIndex() (typically only used for debugging)
    bool SetRichPresence( const char *pchKey, const char *pchValue );

    void ClearRichPresence();

    // the overlay will keep calling GetFriendRichPresence() and spam the debug log, hence this function
    const char *get_friend_rich_presence_silent( CSteamID steamIDFriend, const char *pchKey );

    const char *GetFriendRichPresence( CSteamID steamIDFriend, const char *pchKey );

    int GetFriendRichPresenceKeyCount( CSteamID steamIDFriend );

    const char *GetFriendRichPresenceKeyByIndex( CSteamID steamIDFriend, int iKey );

    // Requests rich presence for a specific user.
    void RequestFriendRichPresence( CSteamID steamIDFriend );


    // rich invite support
    // if the target accepts the invite, the pchConnectString gets added to the command-line for launching the game
    // if the game is already running, a GameRichPresenceJoinRequested_t callback is posted containing the connect string
    // invites can only be sent to friends
    bool InviteUserToGame( CSteamID steamIDFriend, const char *pchConnectString );


    // recently-played-with friends iteration
    // this iterates the entire list of users recently played with, across games
    // GetFriendCoplayTime() returns as a unix time
    int GetCoplayFriendCount();

    CSteamID GetCoplayFriend( int iCoplayFriend );
    
    void GetCoplayFriend(CSteamID& res, int iCoplayFriend );

    int GetFriendCoplayTime( CSteamID steamIDFriend );

    AppId_t GetFriendCoplayGame( CSteamID steamIDFriend );


    // chat interface for games
    // this allows in-game access to group (clan) chats from in the game
    // the behavior is somewhat sophisticated, because the user may or may not be already in the group chat from outside the game or in the overlay
    // use ActivateGameOverlayToUser( "chat", steamIDClan ) to open the in-game overlay version of the chat
    STEAM_CALL_RESULT( JoinClanChatRoomCompletionResult_t )
    SteamAPICall_t JoinClanChatRoom( CSteamID steamIDClan );

    bool LeaveClanChatRoom( CSteamID steamIDClan );

    int GetClanChatMemberCount( CSteamID steamIDClan );

    CSteamID GetChatMemberByIndex( CSteamID steamIDClan, int iUser );
    
    void GetChatMemberByIndex(CSteamID& res, CSteamID steamIDClan, int iUser );

    bool SendClanChatMessage( CSteamID steamIDClanChat, const char *pchText );

    int GetClanChatMessage( CSteamID steamIDClanChat, int iMessage, void *prgchText, int cchTextMax, EChatEntryType *peChatEntryType, STEAM_OUT_STRUCT() CSteamID *psteamidChatter );

    bool IsClanChatAdmin( CSteamID steamIDClanChat, CSteamID steamIDUser );


    // interact with the Steam (game overlay / desktop)
    bool IsClanChatWindowOpenInSteam( CSteamID steamIDClanChat );

    bool OpenClanChatWindowInSteam( CSteamID steamIDClanChat );

    bool CloseClanChatWindowInSteam( CSteamID steamIDClanChat );


    // peer-to-peer chat interception
    // this is so you can show P2P chats inline in the game
    bool SetListenForFriendsMessages( bool bInterceptEnabled );

    bool ReplyToFriendMessage( CSteamID steamIDFriend, const char *pchMsgToSend );

    int GetFriendMessage( CSteamID steamIDFriend, int iMessageID, void *pvData, int cubData, EChatEntryType *peChatEntryType );


    // following apis
    STEAM_CALL_RESULT( FriendsGetFollowerCount_t )
    SteamAPICall_t GetFollowerCount( CSteamID steamID );

    STEAM_CALL_RESULT( FriendsIsFollowing_t )
    SteamAPICall_t IsFollowing( CSteamID steamID );

    STEAM_CALL_RESULT( FriendsEnumerateFollowingList_t )
    SteamAPICall_t EnumerateFollowingList( uint32 unStartIndex );


    bool IsClanPublic( CSteamID steamIDClan );

    bool IsClanOfficialGameGroup( CSteamID steamIDClan );

    int GetNumChatsWithUnreadPriorityMessages();

    void ActivateGameOverlayRemotePlayTogetherInviteDialog( CSteamID steamIDLobby );

    // Call this before calling ActivateGameOverlayToWebPage() to have the Steam Overlay Browser block navigations
    // to your specified protocol (scheme) uris and instead dispatch a OverlayBrowserProtocolNavigation_t callback to your game.
    // ActivateGameOverlayToWebPage() must have been called with k_EActivateGameOverlayToWebPageMode_Modal
    bool RegisterProtocolInOverlayBrowser( const char *pchProtocol );

    // Activates the game overlay to open an invite dialog that will send the provided Rich Presence connect string to selected friends
    void ActivateGameOverlayInviteDialogConnectString( const char *pchConnectString );

    // Steam Community items equipped by a user on their profile
    // You can register for EquippedProfileItemsChanged_t to know when a friend has changed their equipped profile items
    STEAM_CALL_RESULT( EquippedProfileItems_t )
    SteamAPICall_t RequestEquippedProfileItems( CSteamID steamID );

    bool BHasEquippedProfileItem( CSteamID steamID, ECommunityProfileItemType itemType );

    const char *GetProfileItemPropertyString( CSteamID steamID, ECommunityProfileItemType itemType, ECommunityProfileItemProperty prop );

    uint32 GetProfileItemPropertyUint( CSteamID steamID, ECommunityProfileItemType itemType, ECommunityProfileItemProperty prop );

};

#endif //__INCLUDED_STEAM_FRIENDS_H__
