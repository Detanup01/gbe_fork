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

#ifndef __INCLUDED_STEAM_MATCHMAKING_SERVERS_H__
#define __INCLUDED_STEAM_MATCHMAKING_SERVERS_H__

#include "base.h"
#include "common_helpers/forgettable_memory.hpp"
#include <ssq/a2s.h>

struct Steam_Matchmaking_Servers_Direct_IP_Request {
	HServerQuery id{};
	uint32 ip{};
	uint16 port{};

	std::chrono::high_resolution_clock::time_point created{};
	ISteamMatchmakingRulesResponse *rules_response{};
	ISteamMatchmakingPlayersResponse *players_response{};
	ISteamMatchmakingPingResponse *ping_response{};
};

struct Steam_Matchmaking_Servers_Gameserver_Friends {
    uint64 source_id{};
    uint32 ip{};
    uint16 port{};
    std::chrono::high_resolution_clock::time_point last_recv{};
};

struct Steam_Matchmaking_Servers_Gameserver {
    Gameserver server{};
    std::chrono::high_resolution_clock::time_point last_recv{};
    EMatchMakingType type{};
};

struct Steam_Matchmaking_Request {
    AppId_t appid{};
    HServerListRequest id{};
    ISteamMatchmakingServerListResponse *callbacks{};
	ISteamMatchmakingServerListResponse001 *old_callbacks{};
    bool completed{}, cancelled{}, released{};
    std::vector <struct Steam_Matchmaking_Servers_Gameserver> gameservers_filtered{};
    EMatchMakingType type{};
};

class Steam_Matchmaking_Servers :
public ISteamMatchmakingServers001,
public ISteamMatchmakingServers
{
    class Settings *settings{};
    class Local_Storage *local_storage{};
    class Networking *network{};

    std::vector <struct Steam_Matchmaking_Servers_Gameserver> gameservers{};
    std::vector <struct Steam_Matchmaking_Servers_Gameserver_Friends> gameservers_friends{};
    std::vector <struct Steam_Matchmaking_Request> requests{};
    std::vector <struct Steam_Matchmaking_Servers_Direct_IP_Request> direct_ip_requests{};

	common_helpers::ForgettableMemory<gameserveritem_t> requests_from_GetServerDetails{};

	HServerListRequest RequestServerList(AppId_t iApp, ISteamMatchmakingServerListResponse *pRequestServersResponse, EMatchMakingType type);
	void RequestOldServerList(AppId_t iApp, ISteamMatchmakingServerListResponse001 *pRequestServersResponse, EMatchMakingType type);
	
    //
	static void network_callback(void *object, Common_Message *msg);
    void server_details(Gameserver *g, gameserveritem_t *server);
    void server_details_players(Gameserver *g, Steam_Matchmaking_Servers_Direct_IP_Request *r);
    void server_details_rules(Gameserver *g, Steam_Matchmaking_Servers_Direct_IP_Request *r);
    void Callback(Common_Message *msg);

public:
    Steam_Matchmaking_Servers(class Settings *settings, class Local_Storage *local_storage, class Networking *network);
	~Steam_Matchmaking_Servers();
	
	// Request a new list of servers of a particular type.  These calls each correspond to one of the EMatchMakingType values.
	// Each call allocates a new asynchronous request object.
	// Request object must be released by calling ReleaseRequest( hServerListRequest )
	HServerListRequest RequestInternetServerList( AppId_t iApp, STEAM_ARRAY_COUNT(nFilters) MatchMakingKeyValuePair_t **ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse *pRequestServersResponse );
	HServerListRequest RequestLANServerList( AppId_t iApp, ISteamMatchmakingServerListResponse *pRequestServersResponse );
	HServerListRequest RequestFriendsServerList( AppId_t iApp, STEAM_ARRAY_COUNT(nFilters) MatchMakingKeyValuePair_t **ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse *pRequestServersResponse );
	HServerListRequest RequestFavoritesServerList( AppId_t iApp, STEAM_ARRAY_COUNT(nFilters) MatchMakingKeyValuePair_t **ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse *pRequestServersResponse );
	HServerListRequest RequestHistoryServerList( AppId_t iApp, STEAM_ARRAY_COUNT(nFilters) MatchMakingKeyValuePair_t **ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse *pRequestServersResponse );
	HServerListRequest RequestSpectatorServerList( AppId_t iApp, STEAM_ARRAY_COUNT(nFilters) MatchMakingKeyValuePair_t **ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse *pRequestServersResponse );

	void RequestInternetServerList( AppId_t iApp, MatchMakingKeyValuePair_t **ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse001 *pRequestServersResponse );
	void RequestLANServerList( AppId_t iApp, ISteamMatchmakingServerListResponse001 *pRequestServersResponse );
	void RequestFriendsServerList( AppId_t iApp, MatchMakingKeyValuePair_t **ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse001 *pRequestServersResponse );
	void RequestFavoritesServerList( AppId_t iApp, MatchMakingKeyValuePair_t **ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse001 *pRequestServersResponse );
	void RequestHistoryServerList( AppId_t iApp, MatchMakingKeyValuePair_t **ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse001 *pRequestServersResponse );
	void RequestSpectatorServerList( AppId_t iApp, MatchMakingKeyValuePair_t **ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse001 *pRequestServersResponse );

	// Releases the asynchronous request object and cancels any pending query on it if there's a pending query in progress.
	// RefreshComplete callback is not posted when request is released.
	void ReleaseRequest( HServerListRequest hServerListRequest );

	/* the filter operation codes that go in the key part of MatchMakingKeyValuePair_t should be one of these:

		"map"
			- Server passes the filter if the server is playing the specified map.
		"gamedataand"
			- Server passes the filter if the server's game data (ISteamGameServer::SetGameData) contains all of the
			specified strings.  The value field is a comma-delimited list of strings to match.
		"gamedataor"
			- Server passes the filter if the server's game data (ISteamGameServer::SetGameData) contains at least one of the
			specified strings.  The value field is a comma-delimited list of strings to match.
		"gamedatanor"
			- Server passes the filter if the server's game data (ISteamGameServer::SetGameData) does not contain any
			of the specified strings.  The value field is a comma-delimited list of strings to check.
		"gametagsand"
			- Server passes the filter if the server's game tags (ISteamGameServer::SetGameTags) contains all
			of the specified strings.  The value field is a comma-delimited list of strings to check.
		"gametagsnor"
			- Server passes the filter if the server's game tags (ISteamGameServer::SetGameTags) does not contain any
			of the specified strings.  The value field is a comma-delimited list of strings to check.
		"and" (x1 && x2 && ... && xn)
		"or" (x1 || x2 || ... || xn)
		"nand" !(x1 && x2 && ... && xn)
		"nor" !(x1 || x2 || ... || xn)
			- Performs Boolean operation on the following filters.  The operand to this filter specifies
			the "size" of the Boolean inputs to the operation, in Key/value pairs.  (The keyvalue
			pairs must immediately follow, i.e. this is a prefix logical operator notation.)
			In the simplest case where Boolean expressions are not nested, this is simply
			the number of operands.

			For example, to match servers on a particular map or with a particular tag, would would
			use these filters.

				( server.map == "cp_dustbowl" || server.gametags.contains("payload") )
				"or", "2"
				"map", "cp_dustbowl"
				"gametagsand", "payload"

			If logical inputs are nested, then the operand specifies the size of the entire
			"length" of its operands, not the number of immediate children.

				( server.map == "cp_dustbowl" || ( server.gametags.contains("payload") && !server.gametags.contains("payloadrace") ) )
				"or", "4"
				"map", "cp_dustbowl"
				"and", "2"
				"gametagsand", "payload"
				"gametagsnor", "payloadrace"

			Unary NOT can be achieved using either "nand" or "nor" with a single operand.

		"addr"
			- Server passes the filter if the server's query address matches the specified IP or IP:port.
		"gameaddr"
			- Server passes the filter if the server's game address matches the specified IP or IP:port.

		The following filter operations ignore the "value" part of MatchMakingKeyValuePair_t

		"dedicated"
			- Server passes the filter if it passed true to SetDedicatedServer.
		"secure"
			- Server passes the filter if the server is VAC-enabled.
		"notfull"
			- Server passes the filter if the player count is less than the reported max player count.
		"hasplayers"
			- Server passes the filter if the player count is greater than zero.
		"noplayers"
			- Server passes the filter if it doesn't have any players.
		"linux"
			- Server passes the filter if it's a linux server
	*/

	// Get details on a given server in the list, you can get the valid range of index
	// values by calling GetServerCount().  You will also receive index values in 
	// ISteamMatchmakingServerListResponse::ServerResponded() callbacks
	gameserveritem_t *GetServerDetails( HServerListRequest hRequest, int iServer ); 

	// Cancel an request which is operation on the given list type.  You should call this to cancel
	// any in-progress requests before destructing a callback object that may have been passed 
	// to one of the above list request calls.  Not doing so may result in a crash when a callback
	// occurs on the destructed object.
	// Canceling a query does not release the allocated request handle.
	// The request handle must be released using ReleaseRequest( hRequest )
	void CancelQuery( HServerListRequest hRequest ); 

	// Ping every server in your list again but don't update the list of servers
	// Query callback installed when the server list was requested will be used
	// again to post notifications and RefreshComplete, so the callback must remain
	// valid until another RefreshComplete is called on it or the request
	// is released with ReleaseRequest( hRequest )
	void RefreshQuery( HServerListRequest hRequest ); 

	// Returns true if the list is currently refreshing its server list
	bool IsRefreshing( HServerListRequest hRequest ); 

	// How many servers in the given list, GetServerDetails above takes 0... GetServerCount() - 1
	int GetServerCount( HServerListRequest hRequest ); 

	// Refresh a single server inside of a query (rather than all the servers )
	void RefreshServer( HServerListRequest hRequest, int iServer ); 

	// Get details on a given server in the list, you can get the valid range of index
	// values by calling GetServerCount().  You will also receive index values in 
	// ISteamMatchmakingServerListResponse::ServerResponded() callbacks
	gameserveritem_t *GetServerDetails( EMatchMakingType eType, int iServer );

	// Cancel an request which is operation on the given list type.  You should call this to cancel
	// any in-progress requests before destructing a callback object that may have been passed 
	// to one of the above list request calls.  Not doing so may result in a crash when a callback
	// occurs on the destructed object.
	void CancelQuery( EMatchMakingType eType );

	// Ping every server in your list again but don't update the list of servers
	void RefreshQuery( EMatchMakingType eType );

	// Returns true if the list is currently refreshing its server list
	bool IsRefreshing( EMatchMakingType eType );

	// How many servers in the given list, GetServerDetails above takes 0... GetServerCount() - 1
	int GetServerCount( EMatchMakingType eType );

	// Refresh a single server inside of a query (rather than all the servers )
	void RefreshServer( EMatchMakingType eType, int iServer );

	//-----------------------------------------------------------------------------
	// Queries to individual servers directly via IP/Port
	//-----------------------------------------------------------------------------

	// Request updated ping time and other details from a single server
	HServerQuery PingServer( uint32 unIP, uint16 usPort, ISteamMatchmakingPingResponse *pRequestServersResponse ); 

	// Request the list of players currently playing on a server
	HServerQuery PlayerDetails( uint32 unIP, uint16 usPort, ISteamMatchmakingPlayersResponse *pRequestServersResponse );

	// Request the list of rules that the server is running (See ISteamGameServer::SetKeyValue() to set the rules server side)
	HServerQuery ServerRules( uint32 unIP, uint16 usPort, ISteamMatchmakingRulesResponse *pRequestServersResponse ); 

	// Cancel an outstanding Ping/Players/Rules query from above.  You should call this to cancel
	// any in-progress requests before destructing a callback object that may have been passed 
	// to one of the above calls to avoid crashing when callbacks occur.
	void CancelServerQuery( HServerQuery hServerQuery ); 

	// called by steam_client::runcallbacks
    void RunCallbacks();
	
};

#endif // __INCLUDED_STEAM_MATCHMAKING_SERVERS_H__
