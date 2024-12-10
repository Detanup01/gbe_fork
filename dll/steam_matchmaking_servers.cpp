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

#include "dll/dll.h"

#define SERVER_TIMEOUT 10.0
#define DIRECT_IP_DELAY 0.05


static HServerQuery new_server_query()
{
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    static unsigned int a = 0;
    ++a;
    if (!a) ++a;
    return a;
}


void Steam_Matchmaking_Servers::network_callback(void *object, Common_Message *msg)
{
    // PRINT_DEBUG_ENTRY();

    Steam_Matchmaking_Servers *obj = (Steam_Matchmaking_Servers *)object;
    obj->Callback(msg);
}


Steam_Matchmaking_Servers::Steam_Matchmaking_Servers(class Settings *settings, class Local_Storage *local_storage, class Networking *network)
{
    this->settings = settings;
    this->local_storage = local_storage;
    this->network = network;

    this->network->setCallback(CALLBACK_ID_GAMESERVER, (uint64)0, &network_callback, this);
}

Steam_Matchmaking_Servers::~Steam_Matchmaking_Servers()
{
    this->network->rmCallback(CALLBACK_ID_GAMESERVER, (uint64)0, &network_callback, this);
}


HServerListRequest Steam_Matchmaking_Servers::RequestServerList(AppId_t iApp, ISteamMatchmakingServerListResponse *pRequestServersResponse, EMatchMakingType type)
{
    PRINT_DEBUG("%u %p, %i", iApp, pRequestServersResponse, (int)type);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    static unsigned server_list_request = 0;

    ++server_list_request;
    if (!server_list_request) server_list_request = 1;
    HServerListRequest id = (char *)0 + server_list_request; // (char *)0 silences the compiler warning

    if (settings->matchmaking_server_list_always_lan_type) {
        PRINT_DEBUG("forcing request type to LAN");
        type = EMatchMakingType::eLANServer;
    }

    struct Steam_Matchmaking_Request request{};
    request.appid = iApp;
    request.callbacks = pRequestServersResponse;
    request.old_callbacks = NULL;
    request.cancelled = false;
    request.completed = false;
    request.type = type;
    request.id = id;
    requests.push_back(request);
    PRINT_DEBUG("pushed new request with id: %p", request.id);

    if (type == eLANServer) return id;

    if (type == eFriendsServer) {
        for (auto &g : gameservers_friends) {
            if (g.source_id != settings->get_local_steam_id().ConvertToUint64()) {
                Gameserver server{};
                server.set_ip(g.ip);
                server.set_port(g.port);
                server.set_query_port(g.port);
                server.set_appid(iApp);

                struct Steam_Matchmaking_Servers_Gameserver g2{};
                g2.last_recv = std::chrono::high_resolution_clock::now();
                g2.server = server;
                g2.type = type;
                gameservers.push_back(g2);
                PRINT_DEBUG("  eFriendsServer SERVER ADDED");
            }
        }
        return id;
    }

    std::string file_path{};
    unsigned int file_size{};
    if (type == eInternetServer || type == eSpectatorServer) {
        file_path = local_storage->get_current_save_directory() + "7" + PATH_SEPARATOR + Local_Storage::remote_storage_folder + PATH_SEPARATOR + "serverbrowser.txt";
        file_size = file_size_(file_path);
    } else if (type == eFavoritesServer) {
        file_path = local_storage->get_current_save_directory() + "7" + PATH_SEPARATOR + Local_Storage::remote_storage_folder + PATH_SEPARATOR + "serverbrowser_favorites.txt";
        file_size = file_size_(file_path);
    } else if (type == eHistoryServer) {
        file_path = local_storage->get_current_save_directory() + "7" + PATH_SEPARATOR + Local_Storage::remote_storage_folder + PATH_SEPARATOR + "serverbrowser_history.txt";
        file_size = file_size_(file_path);
    }

    PRINT_DEBUG("list file '%s' [%u bytes]", file_path.c_str(), file_size);
    std::string list{};
    if (file_size) {
        list.resize(file_size);
        Local_Storage::get_file_data(file_path, (char *)&list[0], file_size, 0);
    } else {
        return id;
    }

    std::istringstream list_ss(list);
    std::string list_ip{};
    while (std::getline(list_ss, list_ip)) {
        if (list_ip.length() <= 0) continue;

        unsigned int byte4{}, byte3{}, byte2{}, byte1{}, byte0{};
        uint32 ip_int{};
        uint16 port_int{};
        char newip[24]{};
        if (sscanf(list_ip.c_str(), "%u.%u.%u.%u:%u", &byte3, &byte2, &byte1, &byte0, &byte4) == 5) {
            ip_int = (byte3 << 24) + (byte2 << 16) + (byte1 << 8) + byte0;
            port_int = byte4;

            unsigned char ip_tmp[4]{};
            ip_tmp[0] = ip_int & 0xFF;
            ip_tmp[1] = (ip_int >> 8) & 0xFF;
            ip_tmp[2] = (ip_int >> 16) & 0xFF;
            ip_tmp[3] = (ip_int >> 24) & 0xFF;
            snprintf(newip, sizeof(newip), "%d.%d.%d.%d", ip_tmp[3], ip_tmp[2], ip_tmp[1], ip_tmp[0]);
        } else {
            continue;
        }

        Gameserver server{};
        server.set_ip(ip_int);
        server.set_port(port_int);
        server.set_query_port(port_int);
        server.set_appid(iApp);

        struct Steam_Matchmaking_Servers_Gameserver g{};
        g.last_recv = std::chrono::high_resolution_clock::now();
        g.server = server;
        g.type = type;
        gameservers.push_back(g);
        PRINT_DEBUG("  SERVER ADDED %i", (int)g.type);

        list_ip = "";
    }

    return id;
}

// Request a new list of servers of a particular type.  These calls each correspond to one of the EMatchMakingType values.
// Each call allocates a new asynchronous request object.
// Request object must be released by calling ReleaseRequest( hServerListRequest )
HServerListRequest Steam_Matchmaking_Servers::RequestInternetServerList( AppId_t iApp, STEAM_ARRAY_COUNT(nFilters) MatchMakingKeyValuePair_t **ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse *pRequestServersResponse )
{
    PRINT_DEBUG_ENTRY();
    return RequestServerList(iApp, pRequestServersResponse, eInternetServer);
}

HServerListRequest Steam_Matchmaking_Servers::RequestLANServerList( AppId_t iApp, ISteamMatchmakingServerListResponse *pRequestServersResponse )
{
    PRINT_DEBUG_ENTRY();
    return RequestServerList(iApp, pRequestServersResponse, eLANServer);
}

HServerListRequest Steam_Matchmaking_Servers::RequestFriendsServerList( AppId_t iApp, STEAM_ARRAY_COUNT(nFilters) MatchMakingKeyValuePair_t **ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse *pRequestServersResponse )
{
    PRINT_DEBUG_ENTRY();
    return RequestServerList(iApp, pRequestServersResponse, eFriendsServer);
}

HServerListRequest Steam_Matchmaking_Servers::RequestFavoritesServerList( AppId_t iApp, STEAM_ARRAY_COUNT(nFilters) MatchMakingKeyValuePair_t **ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse *pRequestServersResponse )
{
    PRINT_DEBUG_ENTRY();
    return RequestServerList(iApp, pRequestServersResponse, eFavoritesServer);
}

HServerListRequest Steam_Matchmaking_Servers::RequestHistoryServerList( AppId_t iApp, STEAM_ARRAY_COUNT(nFilters) MatchMakingKeyValuePair_t **ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse *pRequestServersResponse )
{
    PRINT_DEBUG_ENTRY();
    return RequestServerList(iApp, pRequestServersResponse, eHistoryServer);
}

HServerListRequest Steam_Matchmaking_Servers::RequestSpectatorServerList( AppId_t iApp, STEAM_ARRAY_COUNT(nFilters) MatchMakingKeyValuePair_t **ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse *pRequestServersResponse )
{
    PRINT_DEBUG_ENTRY();
    return RequestServerList(iApp, pRequestServersResponse, eSpectatorServer);
}

// old server list request

void Steam_Matchmaking_Servers::RequestOldServerList(AppId_t iApp, ISteamMatchmakingServerListResponse001 *pRequestServersResponse, EMatchMakingType type)
{
    PRINT_DEBUG("%u", iApp);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    auto g = std::begin(requests);
    while (g != std::end(requests)) {
        if (g->id == (void *)type) {
            return;
        }

        ++g;
    }

    struct Steam_Matchmaking_Request request{};
    request.appid = iApp;
    request.callbacks = NULL;
    request.old_callbacks = pRequestServersResponse;
    request.cancelled = false;
    request.completed = false;
    request.type = type;
    request.id = (void *)type;
    requests.push_back(request);
    PRINT_DEBUG("pushed new request with id: %p", request.id);
}

void Steam_Matchmaking_Servers::RequestInternetServerList( AppId_t iApp, MatchMakingKeyValuePair_t **ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse001 *pRequestServersResponse )
{
    PRINT_DEBUG("old");
    //TODO
    RequestOldServerList(iApp, pRequestServersResponse, eInternetServer);
}

void Steam_Matchmaking_Servers::RequestLANServerList( AppId_t iApp, ISteamMatchmakingServerListResponse001 *pRequestServersResponse )
{
    PRINT_DEBUG("old");
    //TODO
    RequestOldServerList(iApp, pRequestServersResponse, eLANServer);
}

void Steam_Matchmaking_Servers::RequestFriendsServerList( AppId_t iApp, MatchMakingKeyValuePair_t **ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse001 *pRequestServersResponse )
{
    PRINT_DEBUG("old");
    //TODO
    RequestOldServerList(iApp, pRequestServersResponse, eFriendsServer);
}

void Steam_Matchmaking_Servers::RequestFavoritesServerList( AppId_t iApp, MatchMakingKeyValuePair_t **ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse001 *pRequestServersResponse )
{
    PRINT_DEBUG("old");
    //TODO
    RequestOldServerList(iApp, pRequestServersResponse, eFavoritesServer);
}

void Steam_Matchmaking_Servers::RequestHistoryServerList( AppId_t iApp, MatchMakingKeyValuePair_t **ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse001 *pRequestServersResponse )
{
    PRINT_DEBUG("old");
    //TODO
    RequestOldServerList(iApp, pRequestServersResponse, eHistoryServer);
}

void Steam_Matchmaking_Servers::RequestSpectatorServerList( AppId_t iApp, MatchMakingKeyValuePair_t **ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse001 *pRequestServersResponse )
{
    PRINT_DEBUG("old");
    //TODO
    RequestOldServerList(iApp, pRequestServersResponse, eSpectatorServer);
}


// Releases the asynchronous request object and cancels any pending query on it if there's a pending query in progress.
// RefreshComplete callback is not posted when request is released.
void Steam_Matchmaking_Servers::ReleaseRequest( HServerListRequest hServerListRequest )
{
    PRINT_DEBUG("%p", hServerListRequest);
    auto g = std::begin(requests);
    while (g != std::end(requests)) {
        if (g->id == hServerListRequest) {
            //NOTE: some garbage games release the request before getting server details from it.
            //     g = requests.erase(g);
            // } else {
            //TODO: eventually delete the released request.
            g->cancelled = true;
            g->released = true;
            PRINT_DEBUG("released request with id: %p", g->id);
        }

        ++g;
    }
}


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
gameserveritem_t *Steam_Matchmaking_Servers::GetServerDetails( HServerListRequest hRequest, int iServer )
{
    PRINT_DEBUG("%p %i", hRequest, iServer);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    std::vector <struct Steam_Matchmaking_Servers_Gameserver> gameservers_filtered;
    auto g = std::begin(requests);
    while (g != std::end(requests)) {
        PRINT_DEBUG("  equal? %p %p", hRequest, g->id);
        if (g->id == hRequest) {
            gameservers_filtered = g->gameservers_filtered;
            PRINT_DEBUG("  found %zu", gameservers_filtered.size());
            break;
        }

        ++g;
    }

    if (iServer < 0 || static_cast<size_t>(iServer) >= gameservers_filtered.size()) {
        return NULL;
    }

    Gameserver *gs = &gameservers_filtered[iServer].server;
    auto &server = requests_from_GetServerDetails.create(std::chrono::hours(1));
    server_details(gs, &server);
    PRINT_DEBUG("  Returned server details");
    return &server;
}


// Cancel an request which is operation on the given list type.  You should call this to cancel
// any in-progress requests before destructing a callback object that may have been passed 
// to one of the above list request calls.  Not doing so may result in a crash when a callback
// occurs on the destructed object.
// Canceling a query does not release the allocated request handle.
// The request handle must be released using ReleaseRequest( hRequest )
void Steam_Matchmaking_Servers::CancelQuery( HServerListRequest hRequest )
{
    PRINT_DEBUG("%p", hRequest);
    auto g = std::begin(requests);
    while (g != std::end(requests)) {
        if (g->id == hRequest) {
            g->cancelled = true;
            PRINT_DEBUG("canceled request with id: %p", g->id);
        }

        ++g;
    }
}
 

// Ping every server in your list again but don't update the list of servers
// Query callback installed when the server list was requested will be used
// again to post notifications and RefreshComplete, so the callback must remain
// valid until another RefreshComplete is called on it or the request
// is released with ReleaseRequest( hRequest )
void Steam_Matchmaking_Servers::RefreshQuery( HServerListRequest hRequest )
{
    PRINT_DEBUG("%p", hRequest);
}
 

// Returns true if the list is currently refreshing its server list
bool Steam_Matchmaking_Servers::IsRefreshing( HServerListRequest hRequest )
{
    PRINT_DEBUG("%p", hRequest);
    return false;
}
 

// How many servers in the given list, GetServerDetails above takes 0... GetServerCount() - 1
int Steam_Matchmaking_Servers::GetServerCount( HServerListRequest hRequest )
{
    PRINT_DEBUG("%p", hRequest);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    int size = 0;
    auto g = std::begin(requests);
    while (g != std::end(requests)) {
        if (g->id == hRequest) {
            size = static_cast<int>(g->gameservers_filtered.size());
            break;
        }

        ++g;
    }

    PRINT_DEBUG("final count = %i", size);
    return size;
}


// Refresh a single server inside of a query (rather than all the servers )
void Steam_Matchmaking_Servers::RefreshServer( HServerListRequest hRequest, int iServer )
{
    PRINT_DEBUG("%p", hRequest);
    //TODO
}



// Get details on a given server in the list, you can get the valid range of index
// values by calling GetServerCount().  You will also receive index values in 
// ISteamMatchmakingServerListResponse::ServerResponded() callbacks
gameserveritem_t* Steam_Matchmaking_Servers::GetServerDetails( EMatchMakingType eType, int iServer )
{
    PRINT_DEBUG_ENTRY();
    return GetServerDetails((HServerListRequest) eType , iServer );
}

// Cancel an request which is operation on the given list type.  You should call this to cancel
// any in-progress requests before destructing a callback object that may have been passed 
// to one of the above list request calls.  Not doing so may result in a crash when a callback
// occurs on the destructed object.
void Steam_Matchmaking_Servers::CancelQuery( EMatchMakingType eType )
{
    PRINT_DEBUG_ENTRY();
    return CancelQuery((HServerListRequest) eType);
}

// Ping every server in your list again but don't update the list of servers
void Steam_Matchmaking_Servers::RefreshQuery( EMatchMakingType eType )
{
    PRINT_DEBUG_ENTRY();
    return RefreshQuery((HServerListRequest) eType);
}

// Returns true if the list is currently refreshing its server list
bool Steam_Matchmaking_Servers::IsRefreshing( EMatchMakingType eType )
{
    return IsRefreshing((HServerListRequest) eType);
}

// How many servers in the given list, GetServerDetails above takes 0... GetServerCount() - 1
int Steam_Matchmaking_Servers::GetServerCount( EMatchMakingType eType )
{
    PRINT_DEBUG_ENTRY();
    return GetServerCount((HServerListRequest) eType);
}

// Refresh a single server inside of a query (rather than all the servers )
void Steam_Matchmaking_Servers::RefreshServer( EMatchMakingType eType, int iServer )
{
    PRINT_DEBUG_ENTRY();
    return RefreshServer((HServerListRequest) eType, iServer);
}


//-----------------------------------------------------------------------------
// Queries to individual servers directly via IP/Port
//-----------------------------------------------------------------------------

// Request updated ping time and other details from a single server
HServerQuery Steam_Matchmaking_Servers::PingServer( uint32 unIP, uint16 usPort, ISteamMatchmakingPingResponse *pRequestServersResponse )
{
    PRINT_DEBUG("%hhu.%hhu.%hhu.%hhu:%hu", ((unsigned char *)&unIP)[3], ((unsigned char *)&unIP)[2], ((unsigned char *)&unIP)[1], ((unsigned char *)&unIP)[0], usPort);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    Steam_Matchmaking_Servers_Direct_IP_Request r;
    r.id = new_server_query();
    r.ip = unIP;
    r.port = usPort;
    r.ping_response = pRequestServersResponse;
    r.created = std::chrono::high_resolution_clock::now();
    direct_ip_requests.push_back(r);
    return r.id;
}

// Request the list of players currently playing on a server
HServerQuery Steam_Matchmaking_Servers::PlayerDetails( uint32 unIP, uint16 usPort, ISteamMatchmakingPlayersResponse *pRequestServersResponse )
{
    PRINT_DEBUG("%hhu.%hhu.%hhu.%hhu:%hu", ((unsigned char *)&unIP)[3], ((unsigned char *)&unIP)[2], ((unsigned char *)&unIP)[1], ((unsigned char *)&unIP)[0], usPort);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    Steam_Matchmaking_Servers_Direct_IP_Request r;
    r.id = new_server_query();
    r.ip = unIP;
    r.port = usPort;
    r.players_response = pRequestServersResponse;
    r.created = std::chrono::high_resolution_clock::now();
    direct_ip_requests.push_back(r);
    return r.id;
}


// Request the list of rules that the server is running (See ISteamGameServer::SetKeyValue() to set the rules server side)
HServerQuery Steam_Matchmaking_Servers::ServerRules( uint32 unIP, uint16 usPort, ISteamMatchmakingRulesResponse *pRequestServersResponse )
{
    PRINT_DEBUG("%hhu.%hhu.%hhu.%hhu:%hu", ((unsigned char *)&unIP)[3], ((unsigned char *)&unIP)[2], ((unsigned char *)&unIP)[1], ((unsigned char *)&unIP)[0], usPort);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    Steam_Matchmaking_Servers_Direct_IP_Request r;
    r.id = new_server_query();
    r.ip = unIP;
    r.port = usPort;
    r.rules_response = pRequestServersResponse;
    r.created = std::chrono::high_resolution_clock::now();
    direct_ip_requests.push_back(r);
    return r.id;
}


// Cancel an outstanding Ping/Players/Rules query from above.  You should call this to cancel
// any in-progress requests before destructing a callback object that may have been passed 
// to one of the above calls to avoid crashing when callbacks occur.
void Steam_Matchmaking_Servers::CancelServerQuery( HServerQuery hServerQuery )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    auto r = std::find_if(direct_ip_requests.begin(), direct_ip_requests.end(), [&hServerQuery](Steam_Matchmaking_Servers_Direct_IP_Request const& item) { return item.id == hServerQuery; });
    if (direct_ip_requests.end() == r) return;
    direct_ip_requests.erase(r);
}



void Steam_Matchmaking_Servers::server_details(Gameserver *g, gameserveritem_t *server)
{
    PRINT_DEBUG_ENTRY();
    constexpr const static int MIN_LATENCY = 2;

    int latency = MIN_LATENCY;

    if (settings->matchmaking_server_details_via_source_query && !(g->ip() < 0) && !(g->query_port() < 0)) {
        unsigned char ip[4]{};
        char newip[24]{};
        ip[0] = g->ip() & 0xFF;
        ip[1] = (g->ip() >> 8) & 0xFF;
        ip[2] = (g->ip() >> 16) & 0xFF;
        ip[3] = (g->ip() >> 24) & 0xFF;
        snprintf(newip, sizeof(newip), "%d.%d.%d.%d", ip[3], ip[2], ip[1], ip[0]);

        PRINT_DEBUG("  connecting to ssq server on %s:%u", newip, g->query_port());
        SSQ_SERVER *ssq = ssq_server_new(newip, g->query_port());
        if (ssq != NULL && ssq_server_eok(ssq)) {
            PRINT_DEBUG("  ssq server connection ok");
            ssq_server_timeout(ssq, (SSQ_TIMEOUT_SELECTOR)(SSQ_TIMEOUT_RECV | SSQ_TIMEOUT_SEND), 1200);

            std::chrono::high_resolution_clock::time_point t1 = std::chrono::high_resolution_clock::now();
            A2S_INFO *ssq_a2s_info = ssq_info(ssq);
            std::chrono::high_resolution_clock::time_point t2 = std::chrono::high_resolution_clock::now();
            latency = (int)std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

            // TODO I don't know if low latency is problematic or not, hence this artificial latency
            if (latency < MIN_LATENCY) latency = MIN_LATENCY;

            if (ssq_server_eok(ssq)) {
                PRINT_DEBUG("  ssq server info ok");
                if (ssq_info_has_steamid(ssq_a2s_info)) g->set_id(ssq_a2s_info->steamid);
                g->set_game_description(ssq_a2s_info->game);
                g->set_mod_dir(ssq_a2s_info->folder);
                if (ssq_a2s_info->server_type == A2S_SERVER_TYPE_DEDICATED) g->set_dedicated_server(true);
                else if (ssq_a2s_info->server_type == A2S_SERVER_TYPE_STV_RELAY) g->set_dedicated_server(true);
                else g->set_dedicated_server(false);
                g->set_max_player_count(ssq_a2s_info->max_players);
                g->set_bot_player_count(ssq_a2s_info->bots);
                g->set_server_name(ssq_a2s_info->name);
                g->set_map_name(ssq_a2s_info->map);
                if (ssq_a2s_info->visibility) g->set_password_protected(true);
                else g->set_password_protected(false);
                if (ssq_info_has_stv(ssq_a2s_info)) {
                    g->set_spectator_port(ssq_a2s_info->stv_port);
                    g->set_spectator_server_name(ssq_a2s_info->stv_name);
                }
                //g->set_tags(ssq_a2s_info->keywords);
                //g->set_gamedata();
                //g->set_region();
                g->set_product(ssq_a2s_info->game);
                if (ssq_a2s_info->vac) g->set_secure(true);
                else g->set_secure(false);
                g->set_num_players(ssq_a2s_info->players);
                g->set_version(static_cast<uint32_t>(std::stoull(ssq_a2s_info->version, NULL, 0)));
                if (ssq_info_has_port(ssq_a2s_info)) g->set_port(ssq_a2s_info->port);

                if (ssq_info_has_gameid(ssq_a2s_info)) g->set_appid(CGameID((uint64)ssq_a2s_info->gameid).AppID());
                else g->set_appid(ssq_a2s_info->id);
                
                g->set_offline(false);
            } else {
                PRINT_DEBUG("  ssq server info failed: %s", ssq_server_emsg(ssq));
            }

            if (ssq_a2s_info != NULL) ssq_info_free(ssq_a2s_info);
        } else {
            PRINT_DEBUG("  ssq server connection failed: %s", (ssq ? ssq_server_emsg(ssq) : "NULL instance"));
        }

        if (ssq != NULL) ssq_server_free(ssq);
    }

    uint16 query_port = g->query_port();
    if (g->query_port() == 0xFFFF) {
        query_port = g->port();
    }

    server->m_NetAdr.Init(g->ip(), query_port, g->port());
    server->m_nPing = latency;
    server->m_bHadSuccessfulResponse = true;
    server->m_bDoNotRefresh = false;

    server->m_nAppID = g->appid();
    server->m_nPlayers = g->num_players();
    server->m_nMaxPlayers = g->max_player_count();
    server->m_nBotPlayers = g->bot_player_count();
    server->m_bPassword = g->password_protected();
    server->m_bSecure = g->secure();
    server->m_ulTimeLastPlayed = 0;
    server->m_nServerVersion = g->version();
    server->SetName(g->server_name().c_str());
    server->m_steamID = CSteamID((uint64)g->id());
    
    memset(server->m_szGameDir, 0, sizeof(server->m_szGameDir));
    g->mod_dir().copy(server->m_szGameDir, sizeof(server->m_szGameDir) - 1);

    memset(server->m_szMap, 0, sizeof(server->m_szMap));
    g->map_name().copy(server->m_szMap, sizeof(server->m_szMap) - 1);

    memset(server->m_szGameDescription, 0, sizeof(server->m_szGameDescription));
    g->game_description().copy(server->m_szGameDescription, sizeof(server->m_szGameDescription) - 1);

    memset(server->m_szGameTags, 0, sizeof(server->m_szGameTags));
    g->tags().copy(server->m_szGameTags, sizeof(server->m_szGameTags) - 1);

    PRINT_DEBUG("  " "%" PRIu64 "", g->id());
}

void Steam_Matchmaking_Servers::server_details_players(Gameserver *g, Steam_Matchmaking_Servers_Direct_IP_Request *r)
{
    if (settings->matchmaking_server_details_via_source_query && !(g->ip() < 0) && !(g->query_port() < 0)) {
        unsigned char ip[4]{};
        char newip[24];
        ip[0] = g->ip() & 0xFF;
        ip[1] = (g->ip() >> 8) & 0xFF;
        ip[2] = (g->ip() >> 16) & 0xFF;
        ip[3] = (g->ip() >> 24) & 0xFF;
        snprintf(newip, sizeof(newip), "%d.%d.%d.%d", ip[3], ip[2], ip[1], ip[0]);

        PRINT_DEBUG("  connecting to ssq server on  %s:%u", newip, g->query_port());
        SSQ_SERVER *ssq = ssq_server_new(newip, g->query_port());
        if (ssq != NULL && ssq_server_eok(ssq)) {
            PRINT_DEBUG("  ssq server connection ok");
            ssq_server_timeout(ssq, (SSQ_TIMEOUT_SELECTOR)(SSQ_TIMEOUT_RECV | SSQ_TIMEOUT_SEND), 1200);

            uint8_t ssq_a2s_player_count = 0;
            A2S_PLAYER *ssq_a2s_player = ssq_player(ssq, &ssq_a2s_player_count);

            if (ssq_server_eok(ssq)) {
                PRINT_DEBUG("  ssq server players ok");
                for (int i = 0; i < ssq_a2s_player_count; i++) {
                    r->players_response->AddPlayerToList(ssq_a2s_player[i].name, ssq_a2s_player[i].score, ssq_a2s_player[i].duration);
                }
            } else {
                PRINT_DEBUG("  ssq server players failed: %s", ssq_server_emsg(ssq));
            }

            if (ssq_a2s_player != NULL) ssq_player_free(ssq_a2s_player, ssq_a2s_player_count);
        } else {
            PRINT_DEBUG("  ssq server connection failed: %s", (ssq ? ssq_server_emsg(ssq) : "NULL instance"));
        }

        if (ssq != NULL) ssq_server_free(ssq);
    } else if (!settings->matchmaking_server_details_via_source_query) { // original behavior
        uint32_t number_players = g->num_players();
        PRINT_DEBUG("  players: %u", number_players);
        const auto &players = get_steam_client()->steam_gameserver->get_players();
        auto player = players->cbegin();
        for (uint32_t i = 0; i < number_players && player != players->end(); ++i, ++player) {
            auto duration = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::steady_clock::now() - player->second.join_time);
            float playtime = static_cast<float>(duration.count());
            PRINT_DEBUG("  PLAYER [%u] '%s' %u %f", i, player->second.name.c_str(), player->second.score, playtime);
            r->players_response->AddPlayerToList(player->second.name.c_str(), player->second.score, playtime);
        }
    }

    PRINT_DEBUG("  " "%" PRIu64 "", g->id());
}

void Steam_Matchmaking_Servers::server_details_rules(Gameserver *g, Steam_Matchmaking_Servers_Direct_IP_Request *r)
{
    if (settings->matchmaking_server_details_via_source_query && !(g->ip() < 0) && !(g->query_port() < 0)) {
        unsigned char ip[4]{};
        char newip[24];
        ip[0] = g->ip() & 0xFF;
        ip[1] = (g->ip() >> 8) & 0xFF;
        ip[2] = (g->ip() >> 16) & 0xFF;
        ip[3] = (g->ip() >> 24) & 0xFF;
        snprintf(newip, sizeof(newip), "%d.%d.%d.%d", ip[3], ip[2], ip[1], ip[0]);

        PRINT_DEBUG("  connecting to ssq server on %s:%u", newip, g->query_port());
        SSQ_SERVER *ssq = ssq_server_new(newip, g->query_port());
        if (ssq != NULL && ssq_server_eok(ssq)) {
            ssq_server_timeout(ssq, (SSQ_TIMEOUT_SELECTOR)(SSQ_TIMEOUT_RECV | SSQ_TIMEOUT_SEND), 1200);

            uint16_t ssq_a2s_rules_count = 0;
            A2S_RULES *ssq_a2s_rules = ssq_rules(ssq, &ssq_a2s_rules_count);

            if (ssq_server_eok(ssq)) {
                PRINT_DEBUG("  ssq server rules ok");
                for (int i = 0; i < ssq_a2s_rules_count; i++) {
                    r->rules_response->RulesResponded(ssq_a2s_rules[i].name, ssq_a2s_rules[i].value);
                }
            } else {
                PRINT_DEBUG("  ssq server rules failed: %s", ssq_server_emsg(ssq));
            }

            if (ssq_a2s_rules != NULL) ssq_rules_free(ssq_a2s_rules, ssq_a2s_rules_count);
        } else {
            PRINT_DEBUG("  ssq server connection failed: %s", (ssq ? ssq_server_emsg(ssq) : "NULL instance"));
        }

        if (ssq != NULL) ssq_server_free(ssq);
    } else if (!settings->matchmaking_server_details_via_source_query) { // original behavior
        int number_rules = (int)g->values().size();
        PRINT_DEBUG("  rules: %i", number_rules);
        auto rule = g->values().begin();
        for (int i = 0; i < number_rules; ++i) {
            PRINT_DEBUG("  RULE '%s'='%s'", rule->first.c_str(), rule->second.c_str());
            r->rules_response->RulesResponded(rule->first.c_str(), rule->second.c_str());
            ++rule;
        }
    }

    PRINT_DEBUG("  " "%" PRIu64 "", g->id());
}

void Steam_Matchmaking_Servers::RunCallbacks()
{
    // PRINT_DEBUG_ENTRY();

    {
        auto g = std::begin(gameservers);
        while (g != std::end(gameservers)) {
            if (check_timedout(g->last_recv, SERVER_TIMEOUT)) {
                g = gameservers.erase(g);
                PRINT_DEBUG("SERVER REMOVED, TIMEOUT");
            } else {
                ++g;
            }
        }
    }

    if (requests.size() || gameservers.size()) {
        PRINT_DEBUG("requests count = %zu, servers count = %zu", requests.size(), gameservers.size());
    }

    for (auto &r : requests) {
        if (r.cancelled || r.completed) continue;

        r.gameservers_filtered.clear();
        for (auto &g : gameservers) {
            PRINT_DEBUG("%u==%u | %i==%i", g.server.appid(), r.appid, (int)g.type, (int)r.type);
            if ((g.server.appid() == r.appid) && (g.type == r.type || settings->matchmaking_server_list_always_lan_type)) {
                PRINT_DEBUG("server found");
                r.gameservers_filtered.push_back(g);
            }
        }
    }

    std::vector <struct Steam_Matchmaking_Request> requests_temp(requests);
    for (auto &r : requests) {
        r.completed = true;
    }

    for (auto &r : requests_temp) {
        if (r.cancelled || r.completed) continue;
        int i = 0;

        if (r.callbacks) {
            for (auto &g : r.gameservers_filtered) {
                PRINT_DEBUG("server responded cb %p", r.id);
                r.callbacks->ServerResponded(r.id, i);
                ++i;
            }

            if (i) {
                r.callbacks->RefreshComplete(r.id, eServerResponded);
            } else {
                r.callbacks->RefreshComplete(r.id, eNoServersListedOnMasterServer);
            }
        }

        if (r.old_callbacks) {
            for (auto &g : r.gameservers_filtered) {
                PRINT_DEBUG("REQUESTS server responded cb %p", r.id);
                r.old_callbacks->ServerResponded(i);
                ++i;
            }

            if (i) {
                r.old_callbacks->RefreshComplete(eServerResponded);
            } else {
                r.old_callbacks->RefreshComplete(eNoServersListedOnMasterServer);
            }
        }
    }

    std::vector <struct Steam_Matchmaking_Servers_Direct_IP_Request> direct_ip_requests_temp;
    auto dip = std::begin(direct_ip_requests);
    while (dip != std::end(direct_ip_requests)) {
        if (check_timedout(dip->created, DIRECT_IP_DELAY)) {
            direct_ip_requests_temp.push_back(*dip);
            dip = direct_ip_requests.erase(dip);
        } else {
            ++dip;
        }
    }

    for (auto &r : direct_ip_requests_temp) {
        PRINT_DEBUG("request: %u:%hu", r.ip, r.port);
        for (auto &g : gameservers) {
            PRINT_DEBUG("%u:%u", g.server.ip(), g.server.query_port());
            uint16 query_port = g.server.query_port();
            if (query_port == 0xFFFF) {
                query_port = g.server.port();
            }

            if (query_port == r.port && g.server.ip() == r.ip) {
                if (r.rules_response) {
                    server_details_rules(&(g.server), &r);
                    r.rules_response->RulesRefreshComplete();
                    r.rules_response = NULL;
                }

                if (r.players_response) {
                    server_details_players(&(g.server), &r);
                    r.players_response->PlayersRefreshComplete();
                    r.players_response = NULL;
                }

                if (r.ping_response) {
                    gameserveritem_t server{};
                    server_details(&(g.server), &server);
                    r.ping_response->ServerResponded(server);
                    r.ping_response = NULL;
                }
            }
        }

        if (r.rules_response) r.rules_response->RulesRefreshComplete();
        if (r.players_response) r.players_response->PlayersRefreshComplete();
        if (r.ping_response) r.ping_response->ServerFailedToRespond();
    }

    requests_from_GetServerDetails.cleanup();
}

void Steam_Matchmaking_Servers::Callback(Common_Message *msg)
{
    if (msg->has_gameserver() && msg->gameserver().type() != eFriendsServer) {
        PRINT_DEBUG("got SERVER " "%" PRIu64 ", offline:%u", msg->gameserver().id(), msg->gameserver().offline());
        if (msg->gameserver().offline()) {
            for (auto &g : gameservers) {
                if (g.server.id() == msg->gameserver().id()) {
                    g.last_recv = std::chrono::high_resolution_clock::time_point();
                    g.type = eLANServer;
                }
            }
        } else {
            bool already = false;
            for (auto &g : gameservers) {
                if (g.server.id() == msg->gameserver().id()) {
                    g.last_recv = std::chrono::high_resolution_clock::now();
                    g.server = msg->gameserver();
                    g.server.set_ip(msg->source_ip());
                    g.type = eLANServer;
                    already = true;
                }
            }

            if (!already) {
                struct Steam_Matchmaking_Servers_Gameserver g{};
                g.last_recv = std::chrono::high_resolution_clock::now();
                g.server = msg->gameserver();
                g.server.set_ip(msg->source_ip());
                g.type = eLANServer;
                gameservers.push_back(g);
                PRINT_DEBUG("  eLANServer SERVER ADDED");
            }
        }
    }

    if (msg->has_gameserver() && msg->gameserver().type() == eFriendsServer) {
        PRINT_DEBUG("got eFriendsServer SERVER " "%" PRIu64 "", msg->gameserver().id());
        bool addserver = true;
        for (auto &g : gameservers_friends) {
            if (g.source_id == msg->source_id()) {
                g.ip = msg->gameserver().ip();
                g.port = msg->gameserver().port();
                g.last_recv = std::chrono::high_resolution_clock::now();
                addserver = false;
            }
        }

        if (addserver) {
            struct Steam_Matchmaking_Servers_Gameserver_Friends gameserver_friend;
            gameserver_friend.source_id = msg->source_id();
            gameserver_friend.ip = msg->gameserver().ip();
            gameserver_friend.port = msg->gameserver().port();
            gameserver_friend.last_recv = std::chrono::high_resolution_clock::now();
            gameservers_friends.push_back(gameserver_friend);
        }
    }
}
