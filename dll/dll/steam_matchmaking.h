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

#ifndef __INCLUDED_STEAM_MATCHMAKING_H__
#define __INCLUDED_STEAM_MATCHMAKING_H__

#include "base.h"


struct Pending_Joins {
    SteamAPICall_t api_id{};
    CSteamID lobby_id{};
    std::chrono::high_resolution_clock::time_point joined{};
    bool message_sent{};
};

struct Pending_Creates {
    SteamAPICall_t api_id{};
    std::chrono::high_resolution_clock::time_point created{};
    ELobbyType eLobbyType{};
    int cMaxMembers{};
};

struct Data_Requested {
    CSteamID lobby_id{};
    std::chrono::high_resolution_clock::time_point requested{};
};

struct Filter_Values {
    std::string key{};
    std::string value_string{};
    int value_int{};
    bool is_int{};
    ELobbyComparison eComparisonType{};
};

struct Chat_Entry {
    std::string message{};
    EChatEntryType type{};
    CSteamID lobby_id, user_id{};
};


class Steam_Matchmaking :
public ISteamMatchmaking001,
public ISteamMatchmaking002,
public ISteamMatchmaking003,
public ISteamMatchmaking004,
public ISteamMatchmaking005,
public ISteamMatchmaking006,
public ISteamMatchmaking007,
public ISteamMatchmaking008,
public ISteamMatchmaking
{
    class Settings *settings{};
    class Local_Storage *local_storage{};
    class Networking *network{};
    class SteamCallResults *callback_results{};
    class SteamCallBacks *callbacks{};
    class RunEveryRunCB *run_every_runcb{};

    std::vector<Lobby> lobbies{};
    std::chrono::high_resolution_clock::time_point last_sent_lobbies{};
    std::vector<struct Pending_Joins> pending_joins{};
    std::vector<struct Pending_Creates> pending_creates{};

    std::vector<struct Filter_Values> filter_values{};
    int filter_max_results{};
    std::vector<struct Filter_Values> filter_values_copy{};
    int filter_max_results_copy{};
    std::vector<CSteamID> filtered_lobbies{};
    std::chrono::high_resolution_clock::time_point lobby_last_search{};
    SteamAPICall_t search_call_api_id{};
    bool searching{};

    std::vector<struct Chat_Entry> chat_entries{};
    std::vector<struct Data_Requested> data_requested{};

    std::map<uint64, ::google::protobuf::Map<std::string, std::string>> self_lobby_member_data{};

    google::protobuf::Map<std::string,std::string>::const_iterator caseinsensitive_find(const ::google::protobuf::Map< ::std::string, ::std::string >& map, std::string key);

    static Lobby_Member *get_lobby_member(Lobby *lobby, CSteamID user_id);
    static bool add_member_to_lobby(Lobby *lobby, CSteamID id);
    static bool leave_lobby(Lobby *lobby, CSteamID id);

    Lobby *get_lobby(CSteamID id);
    void send_lobby_data();

    void trigger_lobby_dataupdate(CSteamID lobby, CSteamID member, bool success, double cb_timeout=0.005, bool send_changed_lobby=true);
    void trigger_lobby_member_join_leave(CSteamID lobby, CSteamID member, bool leaving, bool success, double cb_timeout=0.0);

    bool send_owner_packet(CSteamID lobby_id, Lobby_Messages *message);
    bool send_clients_packet(CSteamID lobby_id, Lobby_Messages *message);
    bool send_lobby_members_packet(CSteamID lobby_id, Lobby_Messages *message);

    bool change_owner(Lobby *lobby, CSteamID new_owner);

    void send_gameservercreated_cb(uint64 room_id, uint64 server_id, uint32 ip, uint16 port);

    void remove_lobbies();
    void on_self_enter_leave_lobby(CSteamID id, int type, bool leaving);

    void create_pending_lobbies();
    void run_background();
    void RunCallbacks();
    void Callback(Common_Message *msg);

    static void steam_matchmaking_callback(void *object, Common_Message *msg);
    static void steam_matchmaking_run_every_runcb(void *object);

public:
    Steam_Matchmaking(class Settings *settings, class Local_Storage *local_storage, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb);
    ~Steam_Matchmaking();

    // game server favorites storage
    // saves basic details about a multiplayer game server locally

    // returns the number of favorites servers the user has stored
    int GetFavoriteGameCount();


    // returns the details of the game server
    // iGame is of range [0,GetFavoriteGameCount())
    // *pnIP, *pnConnPort are filled in the with IP:port of the game server
    // *punFlags specify whether the game server was stored as an explicit favorite or in the history of connections
    // *pRTime32LastPlayedOnServer is filled in the with the Unix time the favorite was added
    bool GetFavoriteGame( int iGame, AppId_t *pnAppID, uint32 *pnIP, uint16 *pnConnPort, uint16 *pnQueryPort, uint32 *punFlags, uint32 *pRTime32LastPlayedOnServer );


    // adds the game server to the local list; updates the time played of the server if it already exists in the list
    int AddFavoriteGame( AppId_t nAppID, uint32 nIP, uint16 nConnPort, uint16 nQueryPort, uint32 unFlags, uint32 rTime32LastPlayedOnServer );


    // removes the game server from the local storage; returns true if one was removed
    bool RemoveFavoriteGame( AppId_t nAppID, uint32 nIP, uint16 nConnPort, uint16 nQueryPort, uint32 unFlags );


    ///////
    // Game lobby functions

    // Get a list of relevant lobbies
    // this is an asynchronous request
    // results will be returned by LobbyMatchList_t callback & call result, with the number of lobbies found
    // this will never return lobbies that are full
    // to add more filter, the filter calls below need to be call before each and every RequestLobbyList() call
    // use the CCallResult<> object in steam_api.h to match the SteamAPICall_t call result to a function in an object, e.g.
    /*
        class CMyLobbyListManager
        {
            CCallResult<CMyLobbyListManager, LobbyMatchList_t> m_CallResultLobbyMatchList;
            void FindLobbies()
            {
                // SteamMatchmaking()->AddRequestLobbyListFilter*() functions would be called here, before RequestLobbyList();

                m_CallResultLobbyMatchList.Set( hSteamAPICall, this, &CMyLobbyListManager::OnLobbyMatchList );

            }

            void OnLobbyMatchList( LobbyMatchList_t *pLobbyMatchList, bool bIOFailure )
            {
                // lobby list has be retrieved from Steam back-end, use results
            }
        }
    */
    // 
    STEAM_CALL_RESULT( LobbyMatchList_t )
    SteamAPICall_t RequestLobbyList();

    void RequestLobbyList_OLD();

    // filters for lobbies
    // this needs to be called before RequestLobbyList() to take effect
    // these are cleared on each call to RequestLobbyList()
    void AddRequestLobbyListStringFilter( const char *pchKeyToMatch, const char *pchValueToMatch, ELobbyComparison eComparisonType );

    // numerical comparison
    void AddRequestLobbyListNumericalFilter( const char *pchKeyToMatch, int nValueToMatch, ELobbyComparison eComparisonType );

    // returns results closest to the specified value. Multiple near filters can be added, with early filters taking precedence
    void AddRequestLobbyListNearValueFilter( const char *pchKeyToMatch, int nValueToBeCloseTo );

    // returns only lobbies with the specified number of slots available
    void AddRequestLobbyListFilterSlotsAvailable( int nSlotsAvailable );

    // sets the distance for which we should search for lobbies (based on users IP address to location map on the Steam backed)
    void AddRequestLobbyListDistanceFilter( ELobbyDistanceFilter eLobbyDistanceFilter );

    // sets how many results to return, the lower the count the faster it is to download the lobby results & details to the client
    void AddRequestLobbyListResultCountFilter( int cMaxResults );


    void AddRequestLobbyListCompatibleMembersFilter( CSteamID steamIDLobby );

    void AddRequestLobbyListFilter( const char *pchKeyToMatch, const char *pchValueToMatch );

    void AddRequestLobbyListNumericalFilter( const char *pchKeyToMatch, int nValueToMatch, int nComparisonType );

    void AddRequestLobbyListSlotsAvailableFilter();

    // returns the CSteamID of a lobby, as retrieved by a RequestLobbyList call
    // should only be called after a LobbyMatchList_t callback is received
    // iLobby is of the range [0, LobbyMatchList_t::m_nLobbiesMatching)
    // the returned CSteamID::IsValid() will be false if iLobby is out of range
    CSteamID GetLobbyByIndex( int iLobby );

    void GetLobbyByIndex(CSteamID& res, int iLobby );


    // Create a lobby on the Steam servers.
    // If private, then the lobby will not be returned by any RequestLobbyList() call; the CSteamID
    // of the lobby will need to be communicated via game channels or via InviteUserToLobby()
    // this is an asynchronous request
    // results will be returned by LobbyCreated_t callback and call result; lobby is joined & ready to use at this point
    // a LobbyEnter_t callback will also be received (since the local user is joining their own lobby)
    STEAM_CALL_RESULT( LobbyCreated_t )
    SteamAPICall_t CreateLobby( ELobbyType eLobbyType, int cMaxMembers );

    SteamAPICall_t CreateLobby( ELobbyType eLobbyType );

    void CreateLobby_OLD( ELobbyType eLobbyType );

    void CreateLobby( bool bPrivate );

    // Joins an existing lobby
    // this is an asynchronous request
    // results will be returned by LobbyEnter_t callback & call result, check m_EChatRoomEnterResponse to see if was successful
    // lobby metadata is available to use immediately on this call completing
    STEAM_CALL_RESULT( LobbyEnter_t )
    SteamAPICall_t JoinLobby( CSteamID steamIDLobby );

    void JoinLobby_OLD( CSteamID steamIDLobby );

    // Leave a lobby; this will take effect immediately on the client side
    // other users in the lobby will be notified by a LobbyChatUpdate_t callback
    void LeaveLobby( CSteamID steamIDLobby );


    // Invite another user to the lobby
    // the target user will receive a LobbyInvite_t callback
    // will return true if the invite is successfully sent, whether or not the target responds
    // returns false if the local user is not connected to the Steam servers
    // if the other user clicks the join link, a GameLobbyJoinRequested_t will be posted if the user is in-game,
    // or if the game isn't running yet the game will be launched with the parameter +connect_lobby <64-bit lobby id>
    bool InviteUserToLobby( CSteamID steamIDLobby, CSteamID steamIDInvitee );


    // Lobby iteration, for viewing details of users in a lobby
    // only accessible if the lobby user is a member of the specified lobby
    // persona information for other lobby members (name, avatar, etc.) will be asynchronously received
    // and accessible via ISteamFriends interface

    // returns the number of users in the specified lobby
    int GetNumLobbyMembers( CSteamID steamIDLobby );

    // returns the CSteamID of a user in the lobby
    // iMember is of range [0,GetNumLobbyMembers())
    // note that the current user must be in a lobby to retrieve CSteamIDs of other users in that lobby
    CSteamID GetLobbyMemberByIndex( CSteamID steamIDLobby, int iMember );

    void GetLobbyMemberByIndex(CSteamID& res, CSteamID steamIDLobby, int iMember );


    // Get data associated with this lobby
    // takes a simple key, and returns the string associated with it
    // "" will be returned if no value is set, or if steamIDLobby is invalid
    const char *GetLobbyData( CSteamID steamIDLobby, const char *pchKey );

    // Sets a key/value pair in the lobby metadata
    // each user in the lobby will be broadcast this new value, and any new users joining will receive any existing data
    // this can be used to set lobby names, map, etc.
    // to reset a key, just set it to ""
    // other users in the lobby will receive notification of the lobby data change via a LobbyDataUpdate_t callback
    bool SetLobbyData( CSteamID steamIDLobby, const char *pchKey, const char *pchValue );


    // returns the number of metadata keys set on the specified lobby
    int GetLobbyDataCount( CSteamID steamIDLobby );


    // returns a lobby metadata key/values pair by index, of range [0, GetLobbyDataCount())
    bool GetLobbyDataByIndex( CSteamID steamIDLobby, int iLobbyData, char *pchKey, int cchKeyBufferSize, char *pchValue, int cchValueBufferSize );


    // removes a metadata key from the lobby
    bool DeleteLobbyData( CSteamID steamIDLobby, const char *pchKey );


    // Gets per-user metadata for someone in this lobby
    const char *GetLobbyMemberData( CSteamID steamIDLobby, CSteamID steamIDUser, const char *pchKey );

    // Sets per-user metadata (for the local user implicitly)
    void SetLobbyMemberData( CSteamID steamIDLobby, const char *pchKey, const char *pchValue );


    // Broadcasts a chat message to the all the users in the lobby
    // users in the lobby (including the local user) will receive a LobbyChatMsg_t callback
    // returns true if the message is successfully sent
    // pvMsgBody can be binary or text data, up to 4k
    // if pvMsgBody is text, cubMsgBody should be strlen( text ) + 1, to include the null terminator
    bool SendLobbyChatMsg( CSteamID steamIDLobby, const void *pvMsgBody, int cubMsgBody );

    // Get a chat message as specified in a LobbyChatMsg_t callback
    // iChatID is the LobbyChatMsg_t::m_iChatID value in the callback
    // *pSteamIDUser is filled in with the CSteamID of the member
    // *pvData is filled in with the message itself
    // return value is the number of bytes written into the buffer
    int GetLobbyChatEntry( CSteamID steamIDLobby, int iChatID, STEAM_OUT_STRUCT() CSteamID *pSteamIDUser, void *pvData, int cubData, EChatEntryType *peChatEntryType );


    // Refreshes metadata for a lobby you're not necessarily in right now
    // you never do this for lobbies you're a member of, only if your
    // this will send down all the metadata associated with a lobby
    // this is an asynchronous call
    // returns false if the local user is not connected to the Steam servers
    // results will be returned by a LobbyDataUpdate_t callback
    // if the specified lobby doesn't exist, LobbyDataUpdate_t::m_bSuccess will be set to false
    bool RequestLobbyData( CSteamID steamIDLobby );

    // sets the game server associated with the lobby
    // usually at this point, the users will join the specified game server
    // either the IP/Port or the steamID of the game server has to be valid, depending on how you want the clients to be able to connect
    void SetLobbyGameServer( CSteamID steamIDLobby, uint32 unGameServerIP, uint16 unGameServerPort, CSteamID steamIDGameServer );

    // returns the details of a game server set in a lobby - returns false if there is no game server set, or that lobby doesn't exist
    bool GetLobbyGameServer( CSteamID steamIDLobby, uint32 *punGameServerIP, uint16 *punGameServerPort, STEAM_OUT_STRUCT() CSteamID *psteamIDGameServer );


    // set the limit on the # of users who can join the lobby
    bool SetLobbyMemberLimit( CSteamID steamIDLobby, int cMaxMembers );

    // returns the current limit on the # of users who can join the lobby; returns 0 if no limit is defined
    int GetLobbyMemberLimit( CSteamID steamIDLobby );

    void SetLobbyVoiceEnabled( CSteamID steamIDLobby, bool bVoiceEnabled );

    // updates which type of lobby it is
    // only lobbies that are k_ELobbyTypePublic or k_ELobbyTypeInvisible, and are set to joinable, will be returned by RequestLobbyList() calls
    bool SetLobbyType( CSteamID steamIDLobby, ELobbyType eLobbyType );


    // sets whether or not a lobby is joinable - defaults to true for a new lobby
    // if set to false, no user can join, even if they are a friend or have been invited
    bool SetLobbyJoinable( CSteamID steamIDLobby, bool bLobbyJoinable );


    // returns the current lobby owner
    // you must be a member of the lobby to access this (Mr_Goldberg note: This is a lie)
    // there always one lobby owner - if the current owner leaves, another user will become the owner
    // it is possible (bur rare) to join a lobby just as the owner is leaving, thus entering a lobby with self as the owner
    CSteamID GetLobbyOwner( CSteamID steamIDLobby );

    void GetLobbyOwner(CSteamID& res, CSteamID steamIDLobby );

    // asks the Steam servers for a list of lobbies that friends are in
    // returns results by posting one RequestFriendsLobbiesResponse_t callback per friend/lobby pair
    // if no friends are in lobbies, RequestFriendsLobbiesResponse_t will be posted but with 0 results
    // filters don't apply to lobbies (currently)
    bool RequestFriendsLobbies();

    float GetLobbyDistance( CSteamID steamIDLobby );

    // changes who the lobby owner is
    // you must be the lobby owner for this to succeed, and steamIDNewOwner must be in the lobby
    // after completion, the local user will no longer be the owner
    bool SetLobbyOwner( CSteamID steamIDLobby, CSteamID steamIDNewOwner );


    // link two lobbies for the purposes of checking player compatibility
    // you must be the lobby owner of both lobbies
    bool SetLinkedLobby( CSteamID steamIDLobby, CSteamID steamIDLobbyDependent );


    // older sdk -------------------------------------------------------------------

    // returns the details of the game server
    // iGame is of range [0,GetFavoriteGameCount())
    // *pnIP, *pnConnPort are filled in the with IP:port of the game server
    // *punFlags specify whether the game server was stored as an explicit favorite or in the history of connections
    // *pRTime32LastPlayedOnServer is filled in the with the Unix time the favorite was added
    bool GetFavoriteGame( int iGame, uint32 *pnAppID, uint32 *pnIP, uint16 *pnConnPort, uint32 *punFlags, uint32 *pRTime32LastPlayedOnServer );

    // adds the game server to the local list; updates the time played of the server if it already exists in the list
    int AddFavoriteGame( uint32 nAppID, uint32 nIP, uint16 nConnPort, uint32 unFlags, uint32 rTime32LastPlayedOnServer );
    
    // removes the game server from the local storage; returns true if one was removed
    bool RemoveFavoriteGame( uint32 nAppID, uint32 nIP, uint16 nConnPort, uint32 unFlags );
    bool GetFavoriteGame2( int iGame, uint32 *pnAppID, uint32 *pnIP, uint16 *pnConnPort, uint16 *pnQueryPort, uint32 *punFlags, uint32 *pRTime32LastPlayedOnServer );
    int AddFavoriteGame2( uint32 nAppID, uint32 nIP, uint16 nConnPort, uint16 nQueryPort, uint32 unFlags, uint32 rTime32LastPlayedOnServer );
    bool RemoveFavoriteGame2( uint32 nAppID, uint32 nIP, uint16 nConnPort, uint16 nQueryPort, uint32 unFlags );
    void RequestLobbyList( uint64 ulGameID, MatchMakingKeyValuePair_t *pFilters, uint32 nFilters );
    void CreateLobby( uint64 ulGameID, bool bPrivate );
    bool SetLobbyMemberData_OLD( CSteamID steamIDLobby, const char *pchKey, const char *pchValue );

    // older sdk -------------------------------------------------------------------
    
};

#endif // __INCLUDED_STEAM_MATCHMAKING_H__
