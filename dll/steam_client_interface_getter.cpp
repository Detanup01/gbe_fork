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

#include "dll/steam_client.h"

// retrieves the ISteamUser interface associated with the handle
ISteamUser *Steam_Client::GetISteamUser( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("%s", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamUser) return NULL;

    if (strcmp(pchVersion, "SteamUser009") == 0) {
        return reinterpret_cast<ISteamUser *>(static_cast<ISteamUser009 *>(steam_user));
    } else if (strcmp(pchVersion, "SteamUser010") == 0) {
        return reinterpret_cast<ISteamUser *>(static_cast<ISteamUser010 *>(steam_user));
    } else if (strcmp(pchVersion, "SteamUser011") == 0) {
        return reinterpret_cast<ISteamUser *>(static_cast<ISteamUser011 *>(steam_user));
    } else if (strcmp(pchVersion, "SteamUser012") == 0) {
        return reinterpret_cast<ISteamUser *>(static_cast<ISteamUser012 *>(steam_user));
    } else if (strcmp(pchVersion, "SteamUser013") == 0) {
        return reinterpret_cast<ISteamUser *>(static_cast<ISteamUser013 *>(steam_user));
    } else if (strcmp(pchVersion, "SteamUser014") == 0) {
        return reinterpret_cast<ISteamUser *>(static_cast<ISteamUser014 *>(steam_user));
    } else if (strcmp(pchVersion, "SteamUser015") == 0) {
        return reinterpret_cast<ISteamUser *>(static_cast<ISteamUser015 *>(steam_user));
    } else if (strcmp(pchVersion, "SteamUser016") == 0) {
        return reinterpret_cast<ISteamUser *>(static_cast<ISteamUser016 *>(steam_user));
    } else if (strcmp(pchVersion, "SteamUser017") == 0) {
        return reinterpret_cast<ISteamUser *>(static_cast<ISteamUser017 *>(steam_user));
    } else if (strcmp(pchVersion, "SteamUser018") == 0) {
        return reinterpret_cast<ISteamUser *>(static_cast<ISteamUser018 *>(steam_user));
    } else if (strcmp(pchVersion, "SteamUser019") == 0) {
        return reinterpret_cast<ISteamUser *>(static_cast<ISteamUser019 *>(steam_user));
    } else if (strcmp(pchVersion, "SteamUser020") == 0) {
        return reinterpret_cast<ISteamUser *>(static_cast<ISteamUser020 *>(steam_user));
    } else if (strcmp(pchVersion, "SteamUser021") == 0) {
        return reinterpret_cast<ISteamUser *>(static_cast<ISteamUser021 *>(steam_user));
    } else if (strcmp(pchVersion, "SteamUser022") == 0) {
        return reinterpret_cast<ISteamUser *>(static_cast<ISteamUser022 *>(steam_user));
    } else if (strcmp(pchVersion, STEAMUSER_INTERFACE_VERSION) == 0) {
        return reinterpret_cast<ISteamUser *>(static_cast<ISteamUser *>(steam_user));
    } else {
        return reinterpret_cast<ISteamUser *>(static_cast<ISteamUser *>(steam_user));
    }
    
    return reinterpret_cast<ISteamUser *>(static_cast<ISteamUser *>(steam_user));
}

// retrieves the ISteamGameServer interface associated with the handle
ISteamGameServer *Steam_Client::GetISteamGameServer( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("%s", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamUser) return NULL;

    if (strcmp(pchVersion, "SteamGameServer004") == 0) {
        return reinterpret_cast<ISteamGameServer *>(static_cast<ISteamGameServer004 *>(steam_gameserver));
    } else if (strcmp(pchVersion, "SteamGameServer005") == 0) {
        return reinterpret_cast<ISteamGameServer *>(static_cast<ISteamGameServer005 *>(steam_gameserver));
    } else if (strcmp(pchVersion, "SteamGameServer006") == 0) {
        return reinterpret_cast<ISteamGameServer *>(static_cast<ISteamGameServer008 *>(steam_gameserver));
    } else if (strcmp(pchVersion, "SteamGameServer007") == 0) {
        return reinterpret_cast<ISteamGameServer *>(static_cast<ISteamGameServer008 *>(steam_gameserver));
    } else if (strcmp(pchVersion, "SteamGameServer008") == 0) {
        return reinterpret_cast<ISteamGameServer *>(static_cast<ISteamGameServer008 *>(steam_gameserver));
    } else if (strcmp(pchVersion, "SteamGameServer009") == 0) {
        return reinterpret_cast<ISteamGameServer *>(static_cast<ISteamGameServer009 *>(steam_gameserver));
    } else if (strcmp(pchVersion, "SteamGameServer010") == 0) {
        return reinterpret_cast<ISteamGameServer *>(static_cast<ISteamGameServer010 *>(steam_gameserver));
    } else if (strcmp(pchVersion, "SteamGameServer011") == 0) {
        return reinterpret_cast<ISteamGameServer *>(static_cast<ISteamGameServer011 *>(steam_gameserver));
    } else if (strcmp(pchVersion, "SteamGameServer012") == 0) {
        return reinterpret_cast<ISteamGameServer *>(static_cast<ISteamGameServer012 *>(steam_gameserver));
    } else if (strcmp(pchVersion, "SteamGameServer013") == 0) {
        gameserver_has_ipv6_functions = true;
        return reinterpret_cast<ISteamGameServer *>(static_cast<ISteamGameServer013 *>(steam_gameserver));
    } else if (strcmp(pchVersion, "SteamGameServer014") == 0) {
        gameserver_has_ipv6_functions = true;
        return reinterpret_cast<ISteamGameServer *>(static_cast<ISteamGameServer014 *>(steam_gameserver));
    } else if (strcmp(pchVersion, STEAMGAMESERVER_INTERFACE_VERSION) == 0) {
        gameserver_has_ipv6_functions = true;
        return reinterpret_cast<ISteamGameServer *>(static_cast<ISteamGameServer *>(steam_gameserver));
    } else {
        gameserver_has_ipv6_functions = true;
        return reinterpret_cast<ISteamGameServer *>(static_cast<ISteamGameServer *>(steam_gameserver));
    }
    
    return reinterpret_cast<ISteamGameServer *>(static_cast<ISteamGameServer *>(steam_gameserver));
}

// returns the ISteamFriends interface
ISteamFriends *Steam_Client::GetISteamFriends( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("%s", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamUser) return NULL;

    if (strcmp(pchVersion, "SteamFriends003") == 0) {
        return reinterpret_cast<ISteamFriends *>(static_cast<ISteamFriends003 *>(steam_friends));
    } else if (strcmp(pchVersion, "SteamFriends004") == 0) {
        return reinterpret_cast<ISteamFriends *>(static_cast<ISteamFriends004 *>(steam_friends));
    } else if (strcmp(pchVersion, "SteamFriends005") == 0) {
        return reinterpret_cast<ISteamFriends *>(static_cast<ISteamFriends005 *>(steam_friends));
    } else if (strcmp(pchVersion, "SteamFriends006") == 0) {
        return reinterpret_cast<ISteamFriends *>(static_cast<ISteamFriends006 *>(steam_friends));
    } else if (strcmp(pchVersion, "SteamFriends007") == 0) {
        return reinterpret_cast<ISteamFriends *>(static_cast<ISteamFriends007 *>(steam_friends));
    } else if (strcmp(pchVersion, "SteamFriends008") == 0) {
        return reinterpret_cast<ISteamFriends *>(static_cast<ISteamFriends008 *>(steam_friends));
    } else if (strcmp(pchVersion, "SteamFriends009") == 0) {
        return reinterpret_cast<ISteamFriends *>(static_cast<ISteamFriends009 *>(steam_friends));
    } else if (strcmp(pchVersion, "SteamFriends010") == 0) {
        return reinterpret_cast<ISteamFriends *>(static_cast<ISteamFriends010 *>(steam_friends));
    } else if (strcmp(pchVersion, "SteamFriends011") == 0) {
        return reinterpret_cast<ISteamFriends *>(static_cast<ISteamFriends011 *>(steam_friends));
    } else if (strcmp(pchVersion, "SteamFriends012") == 0) {
        return reinterpret_cast<ISteamFriends *>(static_cast<ISteamFriends012 *>(steam_friends));
    } else if (strcmp(pchVersion, "SteamFriends013") == 0) {
        return reinterpret_cast<ISteamFriends *>(static_cast<ISteamFriends013 *>(steam_friends));
    } else if (strcmp(pchVersion, "SteamFriends014") == 0) {
        return reinterpret_cast<ISteamFriends *>(static_cast<ISteamFriends014 *>(steam_friends));
    } else if (strcmp(pchVersion, "SteamFriends015") == 0) {
        return reinterpret_cast<ISteamFriends *>(static_cast<ISteamFriends015 *>(steam_friends));
    } else if (strcmp(pchVersion, "SteamFriends016") == 0) {
        return reinterpret_cast<ISteamFriends *>(static_cast<ISteamFriends016 *>(steam_friends));
    } else if (strcmp(pchVersion, STEAMFRIENDS_INTERFACE_VERSION) == 0) {
        return reinterpret_cast<ISteamFriends *>(static_cast<ISteamFriends *>(steam_friends));
    } else {
        return reinterpret_cast<ISteamFriends *>(static_cast<ISteamFriends *>(steam_friends));
    }
    
    // we can get here if one of the if-statements didn't return in all paths
    PRINT_DEBUG("Missing handling for interface: %s", pchVersion);
    return reinterpret_cast<ISteamFriends *>(static_cast<ISteamFriends *>(steam_friends));
}

// returns the ISteamUtils interface
ISteamUtils *Steam_Client::GetISteamUtils( HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("%s", pchVersion);
    if (!steam_pipes.count(hSteamPipe)) return NULL;

    Steam_Utils *steam_utils_temp;

    if (steam_pipes[hSteamPipe] == Steam_Pipe::SERVER) {
        steam_utils_temp = steam_gameserver_utils;
    } else {
        steam_utils_temp = steam_utils;
    }

    if (strcmp(pchVersion, "SteamUtils002") == 0) {
        return reinterpret_cast<ISteamUtils *>(static_cast<ISteamUtils002 *>(steam_utils_temp));
    } else if (strcmp(pchVersion, "SteamUtils003") == 0) {
        return reinterpret_cast<ISteamUtils *>(static_cast<ISteamUtils003 *>(steam_utils_temp));
    } else if (strcmp(pchVersion, "SteamUtils004") == 0) {
        return reinterpret_cast<ISteamUtils *>(static_cast<ISteamUtils004 *>(steam_utils_temp));
    } else if (strcmp(pchVersion, "SteamUtils005") == 0) {
        return reinterpret_cast<ISteamUtils *>(static_cast<ISteamUtils005 *>(steam_utils_temp));
    } else if (strcmp(pchVersion, "SteamUtils006") == 0) {
        return reinterpret_cast<ISteamUtils *>(static_cast<ISteamUtils006 *>(steam_utils_temp));
    } else if (strcmp(pchVersion, "SteamUtils007") == 0) {
        return reinterpret_cast<ISteamUtils *>(static_cast<ISteamUtils007 *>(steam_utils_temp));
    } else if (strcmp(pchVersion, "SteamUtils008") == 0) {
        return reinterpret_cast<ISteamUtils *>(static_cast<ISteamUtils008 *>(steam_utils_temp));
    } else if (strcmp(pchVersion, "SteamUtils009") == 0) {
        return reinterpret_cast<ISteamUtils *>(static_cast<ISteamUtils009 *>(steam_utils_temp));
    } else if (strcmp(pchVersion, STEAMUTILS_INTERFACE_VERSION) == 0) {
        return reinterpret_cast<ISteamUtils *>(static_cast<ISteamUtils *>(steam_utils_temp));
    } else {
        return reinterpret_cast<ISteamUtils *>(static_cast<ISteamUtils *>(steam_utils_temp));
    }
    
    // we can get here if one of the if-statements didn't return in all paths
    PRINT_DEBUG("Missing handling for interface: %s", pchVersion);
    return reinterpret_cast<ISteamUtils *>(static_cast<ISteamUtils *>(steam_utils_temp));
}

// returns the ISteamMatchmaking interface
ISteamMatchmaking *Steam_Client::GetISteamMatchmaking( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("%s", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamUser) return NULL;

    if (strcmp(pchVersion, "SteamMatchMaking001") == 0) {
        //TODO
        return reinterpret_cast<ISteamMatchmaking *>(static_cast<ISteamMatchmaking002 *>(steam_matchmaking));
    } else if (strcmp(pchVersion, "SteamMatchMaking002") == 0) {
        return reinterpret_cast<ISteamMatchmaking *>(static_cast<ISteamMatchmaking002 *>(steam_matchmaking));
    } else if (strcmp(pchVersion, "SteamMatchMaking003") == 0) {
        return reinterpret_cast<ISteamMatchmaking *>(static_cast<ISteamMatchmaking003 *>(steam_matchmaking));
    } else if (strcmp(pchVersion, "SteamMatchMaking004") == 0) {
        return reinterpret_cast<ISteamMatchmaking *>(static_cast<ISteamMatchmaking004 *>(steam_matchmaking));
    } else if (strcmp(pchVersion, "SteamMatchMaking005") == 0) {
        return reinterpret_cast<ISteamMatchmaking *>(static_cast<ISteamMatchmaking005 *>(steam_matchmaking));
    } else if (strcmp(pchVersion, "SteamMatchMaking006") == 0) {
        return reinterpret_cast<ISteamMatchmaking *>(static_cast<ISteamMatchmaking006 *>(steam_matchmaking));
    } else if (strcmp(pchVersion, "SteamMatchMaking007") == 0) {
        return reinterpret_cast<ISteamMatchmaking *>(static_cast<ISteamMatchmaking007 *>(steam_matchmaking));
    } else if (strcmp(pchVersion, "SteamMatchMaking008") == 0) {
        return reinterpret_cast<ISteamMatchmaking *>(static_cast<ISteamMatchmaking008 *>(steam_matchmaking));
    } else if (strcmp(pchVersion, STEAMMATCHMAKING_INTERFACE_VERSION) == 0) {
        return reinterpret_cast<ISteamMatchmaking *>(static_cast<ISteamMatchmaking *>(steam_matchmaking));
    } else {
        return reinterpret_cast<ISteamMatchmaking *>(static_cast<ISteamMatchmaking *>(steam_matchmaking));
    }
    
    // we can get here if one of the if-statements didn't return in all paths
    PRINT_DEBUG("Missing handling for interface: %s", pchVersion);
    return reinterpret_cast<ISteamMatchmaking *>(static_cast<ISteamMatchmaking *>(steam_matchmaking));
}

// returns the ISteamMatchmakingServers interface
ISteamMatchmakingServers *Steam_Client::GetISteamMatchmakingServers( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("%s", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamUser) return NULL;

    if (strcmp(pchVersion, "SteamMatchMakingServers001") == 0) {
        return reinterpret_cast<ISteamMatchmakingServers *>(static_cast<ISteamMatchmakingServers001 *>(steam_matchmaking_servers));
    } else if (strcmp(pchVersion, STEAMMATCHMAKINGSERVERS_INTERFACE_VERSION) == 0) {
        return reinterpret_cast<ISteamMatchmakingServers *>(static_cast<ISteamMatchmakingServers *>(steam_matchmaking_servers));
    } else {
        return reinterpret_cast<ISteamMatchmakingServers *>(static_cast<ISteamMatchmakingServers *>(steam_matchmaking_servers));
    }
    
    // we can get here if one of the if-statements didn't return in all paths
    PRINT_DEBUG("Missing handling for interface: %s", pchVersion);
    return reinterpret_cast<ISteamMatchmakingServers *>(static_cast<ISteamMatchmakingServers *>(steam_matchmaking_servers));
}

// returns the a generic interface
void *Steam_Client::GetISteamGenericInterface( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("%s", pchVersion);
    if (!steam_pipes.count(hSteamPipe)) return NULL;

    bool server = false;
    if (steam_pipes[hSteamPipe] == Steam_Pipe::SERVER) {
        server = true;
    } else {
        if ((strstr(pchVersion, "SteamNetworkingUtils") != pchVersion) && (strstr(pchVersion, "SteamUtils") != pchVersion)) {
            if (!hSteamUser) return NULL;
        }
    }

    // NOTE: you must try to read the one with the most characters first

    if (strstr(pchVersion, "SteamNetworkingSocketsSerialized") == pchVersion) {
        Steam_Networking_Sockets_Serialized *steam_networking_sockets_serialized_temp;
        if (server) {
            steam_networking_sockets_serialized_temp = steam_gameserver_networking_sockets_serialized;
        } else {
            steam_networking_sockets_serialized_temp = steam_networking_sockets_serialized;
        }

        if (strcmp(pchVersion, "SteamNetworkingSocketsSerialized002") == 0) {
            return (void *)(ISteamNetworkingSocketsSerialized002 *)steam_networking_sockets_serialized_temp;
        } else if (strcmp(pchVersion, "SteamNetworkingSocketsSerialized003") == 0) {
            return (void *)(ISteamNetworkingSocketsSerialized003 *)steam_networking_sockets_serialized_temp;
        } else if (strcmp(pchVersion, "SteamNetworkingSocketsSerialized004") == 0) {
            return (void *)(ISteamNetworkingSocketsSerialized004 *)steam_networking_sockets_serialized_temp;
        } else if (strcmp(pchVersion, "SteamNetworkingSocketsSerialized005") == 0) {
            return (void *)(ISteamNetworkingSocketsSerialized005 *)steam_networking_sockets_serialized_temp;
        } else {
            return (void *)(ISteamNetworkingSocketsSerialized005 *)steam_networking_sockets_serialized_temp;
        }
    } else if (strstr(pchVersion, "SteamNetworkingSockets") == pchVersion) {
        Steam_Networking_Sockets *steam_networking_sockets_temp;
        if (server) {
            steam_networking_sockets_temp = steam_gameserver_networking_sockets;
        } else {
            steam_networking_sockets_temp = steam_networking_sockets;
        }

        if (strcmp(pchVersion, "SteamNetworkingSockets001") == 0) {
            return (void *)(ISteamNetworkingSockets001 *) steam_networking_sockets_temp;
        } else if (strcmp(pchVersion, "SteamNetworkingSockets002") == 0) {
            return (void *)(ISteamNetworkingSockets002 *) steam_networking_sockets_temp;
        } else if (strcmp(pchVersion, "SteamNetworkingSockets003") == 0) {
            return (void *)(ISteamNetworkingSockets003 *) steam_networking_sockets_temp;
        } else if (strcmp(pchVersion, "SteamNetworkingSockets004") == 0) {
            return (void *)(ISteamNetworkingSockets004 *) steam_networking_sockets_temp;
        } else if (strcmp(pchVersion, "SteamNetworkingSockets006") == 0) {
            return (void *)(ISteamNetworkingSockets006 *) steam_networking_sockets_temp;
        } else if (strcmp(pchVersion, "SteamNetworkingSockets008") == 0) {
            return (void *)(ISteamNetworkingSockets008 *) steam_networking_sockets_temp;
        } else if (strcmp(pchVersion, "SteamNetworkingSockets009") == 0) {
            return (void *)(ISteamNetworkingSockets009 *) steam_networking_sockets_temp;
        } else if (strcmp(pchVersion, STEAMNETWORKINGSOCKETS_INTERFACE_VERSION) == 0) {
            return (void *)(ISteamNetworkingSockets *) steam_networking_sockets_temp;
        } else {
            return (void *)(ISteamNetworkingSockets *) steam_networking_sockets_temp;
        }
    } else if (strstr(pchVersion, "SteamNetworkingMessages") == pchVersion) {
        Steam_Networking_Messages *steam_networking_messages_temp;
        if (server) {
            steam_networking_messages_temp = steam_gameserver_networking_messages;
        } else {
            steam_networking_messages_temp = steam_networking_messages;
        }

        if (strcmp(pchVersion, STEAMNETWORKINGMESSAGES_INTERFACE_VERSION) == 0) {
            return (void *)(ISteamNetworkingMessages *)steam_networking_messages_temp;
        } else {
            return (void *)(ISteamNetworkingMessages *)steam_networking_messages_temp;
        }
    } else if (strstr(pchVersion, "SteamNetworkingUtils") == pchVersion) {
        if (strcmp(pchVersion, "SteamNetworkingUtils001") == 0) {
            return (void *)(ISteamNetworkingUtils001 *)steam_networking_utils;
        } else if (strcmp(pchVersion, "SteamNetworkingUtils002") == 0) {
            return (void *)(ISteamNetworkingUtils002 *)steam_networking_utils;
        } else if (strcmp(pchVersion, "SteamNetworkingUtils003") == 0) {
            return (void *)(ISteamNetworkingUtils003 *)steam_networking_utils;
        } else if (strcmp(pchVersion, STEAMNETWORKINGUTILS_INTERFACE_VERSION) == 0) {
            return (void *)(ISteamNetworkingUtils *)steam_networking_utils;
        } else {
            return (void *)(ISteamNetworkingUtils *)steam_networking_utils;
        }
    } else if (strstr(pchVersion, "SteamNetworking") == pchVersion) {
        return GetISteamNetworking(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "SteamGameCoordinator") == pchVersion) {
        Steam_Game_Coordinator *steam_game_coordinator_temp;
        if (server) {
            steam_game_coordinator_temp = steam_gameserver_game_coordinator;
        } else {
            steam_game_coordinator_temp = steam_game_coordinator;
        }

        if (strcmp(pchVersion, STEAMGAMECOORDINATOR_INTERFACE_VERSION) == 0) {
            return (void *)(ISteamGameCoordinator *)steam_game_coordinator_temp;
        } else {
            return (void *)(ISteamGameCoordinator *)steam_game_coordinator_temp;
        }
    } else if (strstr(pchVersion, "STEAMTV_INTERFACE_V") == pchVersion) {
        if (strcmp(pchVersion, STEAMTV_INTERFACE_VERSION) == 0) {
            return (void *)(ISteamTV *)steam_tv;
        } else {
            return (void *)(ISteamTV *)steam_tv;
        }
    } else if (strstr(pchVersion, "STEAMREMOTESTORAGE_INTERFACE_VERSION") == pchVersion) {
        return GetISteamRemoteStorage(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "SteamGameServerStats") == pchVersion) {
        return GetISteamGameServerStats(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "SteamGameServer") == pchVersion) {
        return GetISteamGameServer(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "SteamMatchMakingServers") == pchVersion) {
        return GetISteamMatchmakingServers(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "SteamMatchMaking") == pchVersion) {
        return GetISteamMatchmaking(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "SteamFriends") == pchVersion) {
        return GetISteamFriends(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "SteamController") == pchVersion || strstr(pchVersion, "STEAMCONTROLLER_INTERFACE_VERSION") == pchVersion) {
        return GetISteamController(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "STEAMUGC_INTERFACE_VERSION") == pchVersion) {
        return GetISteamUGC(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "STEAMINVENTORY_INTERFACE") == pchVersion) {
        return GetISteamInventory(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "STEAMUSERSTATS_INTERFACE_VERSION") == pchVersion) {
        return GetISteamUserStats(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "SteamUser") == pchVersion) {
        return GetISteamUser(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "SteamUtils") == pchVersion) {
        return GetISteamUtils(hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "STEAMAPPS_INTERFACE_VERSION") == pchVersion) {
        return GetISteamApps(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "STEAMSCREENSHOTS_INTERFACE_VERSION") == pchVersion) {
        return GetISteamScreenshots(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "STEAMHTTP_INTERFACE_VERSION") == pchVersion) {
        return GetISteamHTTP(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "STEAMUNIFIEDMESSAGES_INTERFACE_VERSION") == pchVersion) {
        return DEPRECATED_GetISteamUnifiedMessages(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "STEAMAPPLIST_INTERFACE_VERSION") == pchVersion) {
        return GetISteamAppList(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "STEAMMUSIC_INTERFACE_VERSION") == pchVersion) {
        return GetISteamMusic(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "STEAMMUSICREMOTE_INTERFACE_VERSION") == pchVersion) {
        return GetISteamMusicRemote(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "STEAMHTMLSURFACE_INTERFACE_VERSION") == pchVersion) {
        return GetISteamHTMLSurface(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "STEAMVIDEO_INTERFACE") == pchVersion) {
        return GetISteamVideo(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "SteamMasterServerUpdater") == pchVersion) {
        return GetISteamMasterServerUpdater(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "SteamMatchGameSearch") == pchVersion) {
        return GetISteamGameSearch(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "SteamParties") == pchVersion) {
        return GetISteamParties(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "SteamInput") == pchVersion) {
        return GetISteamInput(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "STEAMREMOTEPLAY_INTERFACE_VERSION") == pchVersion) {
        return GetISteamRemotePlay(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "STEAMPARENTALSETTINGS_INTERFACE_VERSION") == pchVersion) {
        return GetISteamParentalSettings(hSteamUser, hSteamPipe, pchVersion);
    } else if (strstr(pchVersion, "STEAMAPPTICKET_INTERFACE_VERSION") == pchVersion) {
        return GetAppTicket(hSteamUser, hSteamPipe, pchVersion);
    } else {
        PRINT_DEBUG("No interface: %s", pchVersion);
        //TODO: all the interfaces
        return NULL;
    }
    
    // we can get here if one of the if-statements didn't return in all paths
    PRINT_DEBUG("Invalid handling for interface: %s", pchVersion);
    return NULL;
}

// returns the ISteamUserStats interface
ISteamUserStats *Steam_Client::GetISteamUserStats( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("%s", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamUser) return NULL;

    if (strcmp(pchVersion, "STEAMUSERSTATS_INTERFACE_VERSION001") == 0) {
        //TODO
        return reinterpret_cast<ISteamUserStats *>(static_cast<ISteamUserStats003 *>(steam_user_stats));
    } else if (strcmp(pchVersion, "STEAMUSERSTATS_INTERFACE_VERSION002") == 0) {
        //TODO
        return reinterpret_cast<ISteamUserStats *>(static_cast<ISteamUserStats003 *>(steam_user_stats));
    } else if (strcmp(pchVersion, "STEAMUSERSTATS_INTERFACE_VERSION003") == 0) {
        return reinterpret_cast<ISteamUserStats *>(static_cast<ISteamUserStats003 *>(steam_user_stats));
    } else if (strcmp(pchVersion, "STEAMUSERSTATS_INTERFACE_VERSION004") == 0) {
        return reinterpret_cast<ISteamUserStats *>(static_cast<ISteamUserStats004 *>(steam_user_stats));
    } else if (strcmp(pchVersion, "STEAMUSERSTATS_INTERFACE_VERSION005") == 0) {
        return reinterpret_cast<ISteamUserStats *>(static_cast<ISteamUserStats005 *>(steam_user_stats));
    } else if (strcmp(pchVersion, "STEAMUSERSTATS_INTERFACE_VERSION006") == 0) {
        return reinterpret_cast<ISteamUserStats *>(static_cast<ISteamUserStats006 *>(steam_user_stats));
    } else if (strcmp(pchVersion, "STEAMUSERSTATS_INTERFACE_VERSION007") == 0) {
        return reinterpret_cast<ISteamUserStats *>(static_cast<ISteamUserStats007 *>(steam_user_stats));
    } else if (strcmp(pchVersion, "STEAMUSERSTATS_INTERFACE_VERSION008") == 0) {
        return reinterpret_cast<ISteamUserStats *>(static_cast<ISteamUserStats008 *>(steam_user_stats));
    } else if (strcmp(pchVersion, "STEAMUSERSTATS_INTERFACE_VERSION009") == 0) {
        return reinterpret_cast<ISteamUserStats *>(static_cast<ISteamUserStats009 *>(steam_user_stats));
    } else if (strcmp(pchVersion, "STEAMUSERSTATS_INTERFACE_VERSION010") == 0) {
        return reinterpret_cast<ISteamUserStats *>(static_cast<ISteamUserStats010 *>(steam_user_stats));
    } else if (strcmp(pchVersion, "STEAMUSERSTATS_INTERFACE_VERSION011") == 0) {
        return reinterpret_cast<ISteamUserStats *>(static_cast<ISteamUserStats011 *>(steam_user_stats));
    } else if (strcmp(pchVersion, STEAMUSERSTATS_INTERFACE_VERSION) == 0) {
        return reinterpret_cast<ISteamUserStats *>(static_cast<ISteamUserStats *>(steam_user_stats));
    } else {
        return reinterpret_cast<ISteamUserStats *>(static_cast<ISteamUserStats *>(steam_user_stats));
    }

    // we can get here if one of the if-statements didn't return in all paths
    PRINT_DEBUG("Missing handling for interface: %s", pchVersion);
    return reinterpret_cast<ISteamUserStats *>(static_cast<ISteamUserStats *>(steam_user_stats));
}

// returns the ISteamGameServerStats interface
ISteamGameServerStats *Steam_Client::GetISteamGameServerStats( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("%s", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamuser) return NULL;
    
    if (strcmp(pchVersion, STEAMGAMESERVERSTATS_INTERFACE_VERSION) == 0) {
        return reinterpret_cast<ISteamGameServerStats *>(static_cast<ISteamGameServerStats *>(steam_gameserverstats));
    } else {
        return reinterpret_cast<ISteamGameServerStats *>(static_cast<ISteamGameServerStats *>(steam_gameserverstats));
    }

    // we can get here if one of the if-statements didn't return in all paths
    PRINT_DEBUG("Missing handling for interface: %s", pchVersion);
    return reinterpret_cast<ISteamGameServerStats *>(static_cast<ISteamGameServerStats *>(steam_gameserverstats));
}

// returns apps interface
ISteamApps *Steam_Client::GetISteamApps( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("%s", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamUser) return NULL;

    Steam_Apps *steam_apps_temp;

    if (steam_pipes[hSteamPipe] == Steam_Pipe::SERVER) {
        steam_apps_temp = steam_gameserver_apps;
    } else {
        steam_apps_temp = steam_apps;
    }
    if (strcmp(pchVersion, "STEAMAPPS_INTERFACE_VERSION001") == 0) {
        return reinterpret_cast<ISteamApps *>(static_cast<ISteamApps001 *>(steam_apps_temp));
    } else if (strcmp(pchVersion, "STEAMAPPS_INTERFACE_VERSION002") == 0) {
        return reinterpret_cast<ISteamApps *>(static_cast<ISteamApps002 *>(steam_apps_temp));
    } else if (strcmp(pchVersion, "STEAMAPPS_INTERFACE_VERSION003") == 0) {
        return reinterpret_cast<ISteamApps *>(static_cast<ISteamApps003 *>(steam_apps_temp));
    } else if (strcmp(pchVersion, "STEAMAPPS_INTERFACE_VERSION004") == 0) {
        return reinterpret_cast<ISteamApps *>(static_cast<ISteamApps004 *>(steam_apps_temp));
    } else if (strcmp(pchVersion, "STEAMAPPS_INTERFACE_VERSION005") == 0) {
        return reinterpret_cast<ISteamApps *>(static_cast<ISteamApps005 *>(steam_apps_temp));
    } else if (strcmp(pchVersion, "STEAMAPPS_INTERFACE_VERSION006") == 0) {
        return reinterpret_cast<ISteamApps *>(static_cast<ISteamApps006 *>(steam_apps_temp));
    } else if (strcmp(pchVersion, "STEAMAPPS_INTERFACE_VERSION007") == 0) {
        return reinterpret_cast<ISteamApps *>(static_cast<ISteamApps007 *>(steam_apps_temp));
    } else if (strcmp(pchVersion, STEAMAPPS_INTERFACE_VERSION) == 0) {
        return reinterpret_cast<ISteamApps *>(static_cast<ISteamApps *>(steam_apps_temp));
    } else {
        return reinterpret_cast<ISteamApps *>(static_cast<ISteamApps *>(steam_apps_temp));
    }

    // we can get here if one of the if-statements didn't return in all paths
    PRINT_DEBUG("Missing handling for interface: %s", pchVersion);
    return reinterpret_cast<ISteamApps *>(static_cast<ISteamApps *>(steam_apps_temp));
}

// networking
ISteamNetworking *Steam_Client::GetISteamNetworking( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("%s", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamUser) return NULL;

    Steam_Networking *steam_networking_temp;

    if (steam_pipes[hSteamPipe] == Steam_Pipe::SERVER) {
        steam_networking_temp = steam_gameserver_networking;
    } else {
        steam_networking_temp = steam_networking;
    }

    if (strcmp(pchVersion, "SteamNetworking001") == 0) {
        return reinterpret_cast<ISteamNetworking *>(static_cast<ISteamNetworking001 *>(steam_networking_temp));
    } else if (strcmp(pchVersion, "SteamNetworking002") == 0) {
        return reinterpret_cast<ISteamNetworking *>(static_cast<ISteamNetworking002 *>(steam_networking_temp));
    } else if (strcmp(pchVersion, "SteamNetworking003") == 0) {
        return reinterpret_cast<ISteamNetworking *>(static_cast<ISteamNetworking003 *>(steam_networking_temp));
    } else if (strcmp(pchVersion, "SteamNetworking004") == 0) {
        return reinterpret_cast<ISteamNetworking *>(static_cast<ISteamNetworking004 *>(steam_networking_temp));
    } else if (strcmp(pchVersion, "SteamNetworking005") == 0) {
        return reinterpret_cast<ISteamNetworking *>(static_cast<ISteamNetworking005 *>(steam_networking_temp));
    } else if (strcmp(pchVersion, STEAMNETWORKING_INTERFACE_VERSION) == 0) {
        return reinterpret_cast<ISteamNetworking *>(static_cast<ISteamNetworking *>(steam_networking_temp));
    } else {
        return reinterpret_cast<ISteamNetworking *>(static_cast<ISteamNetworking *>(steam_networking_temp));
    }

    // we can get here if one of the if-statements didn't return in all paths
    PRINT_DEBUG("Missing handling for interface: %s", pchVersion);
    return reinterpret_cast<ISteamNetworking *>(static_cast<ISteamNetworking *>(steam_networking_temp));
}

// remote storage
ISteamRemoteStorage *Steam_Client::GetISteamRemoteStorage( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("%s", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamuser) return NULL;

    if (strcmp(pchVersion, "STEAMREMOTESTORAGE_INTERFACE_VERSION001") == 0) {
        return reinterpret_cast<ISteamRemoteStorage *>(static_cast<ISteamRemoteStorage001 *>(steam_remote_storage));
    } else if (strcmp(pchVersion, "STEAMREMOTESTORAGE_INTERFACE_VERSION002") == 0) {
        return reinterpret_cast<ISteamRemoteStorage *>(static_cast<ISteamRemoteStorage002 *>(steam_remote_storage));
    } else if (strcmp(pchVersion, "STEAMREMOTESTORAGE_INTERFACE_VERSION003") == 0) {
        return reinterpret_cast<ISteamRemoteStorage *>(static_cast<ISteamRemoteStorage003 *>(steam_remote_storage));
    } else if (strcmp(pchVersion, "STEAMREMOTESTORAGE_INTERFACE_VERSION004") == 0) {
        return reinterpret_cast<ISteamRemoteStorage *>(static_cast<ISteamRemoteStorage004 *>(steam_remote_storage));
    } else if (strcmp(pchVersion, "STEAMREMOTESTORAGE_INTERFACE_VERSION005") == 0) {
        return reinterpret_cast<ISteamRemoteStorage *>(static_cast<ISteamRemoteStorage005 *>(steam_remote_storage));
    } else if (strcmp(pchVersion, "STEAMREMOTESTORAGE_INTERFACE_VERSION006") == 0) {
        return reinterpret_cast<ISteamRemoteStorage *>(static_cast<ISteamRemoteStorage006 *>(steam_remote_storage));
    } else if (strcmp(pchVersion, "STEAMREMOTESTORAGE_INTERFACE_VERSION007") == 0) {
        return reinterpret_cast<ISteamRemoteStorage *>(static_cast<ISteamRemoteStorage007 *>(steam_remote_storage));
    } else if (strcmp(pchVersion, "STEAMREMOTESTORAGE_INTERFACE_VERSION008") == 0) {
        return reinterpret_cast<ISteamRemoteStorage *>(static_cast<ISteamRemoteStorage008 *>(steam_remote_storage));
    } else if (strcmp(pchVersion, "STEAMREMOTESTORAGE_INTERFACE_VERSION009") == 0) {
        return reinterpret_cast<ISteamRemoteStorage *>(static_cast<ISteamRemoteStorage009 *>(steam_remote_storage));
    } else if (strcmp(pchVersion, "STEAMREMOTESTORAGE_INTERFACE_VERSION010") == 0) {
        return reinterpret_cast<ISteamRemoteStorage *>(static_cast<ISteamRemoteStorage010 *>(steam_remote_storage));
    } else if (strcmp(pchVersion, "STEAMREMOTESTORAGE_INTERFACE_VERSION011") == 0) {
        return reinterpret_cast<ISteamRemoteStorage *>(static_cast<ISteamRemoteStorage011 *>(steam_remote_storage));
    } else if (strcmp(pchVersion, "STEAMREMOTESTORAGE_INTERFACE_VERSION012") == 0) {
        return reinterpret_cast<ISteamRemoteStorage *>(static_cast<ISteamRemoteStorage012 *>(steam_remote_storage));
    } else if (strcmp(pchVersion, "STEAMREMOTESTORAGE_INTERFACE_VERSION013") == 0) {
        return reinterpret_cast<ISteamRemoteStorage *>(static_cast<ISteamRemoteStorage013 *>(steam_remote_storage));
    } else if (strcmp(pchVersion, "STEAMREMOTESTORAGE_INTERFACE_VERSION014") == 0) {
        return reinterpret_cast<ISteamRemoteStorage *>(static_cast<ISteamRemoteStorage014 *>(steam_remote_storage));
    } else if (strcmp(pchVersion, STEAMREMOTESTORAGE_INTERFACE_VERSION) == 0) {
        return reinterpret_cast<ISteamRemoteStorage *>(static_cast<ISteamRemoteStorage *>(steam_remote_storage));
    } else {
        return reinterpret_cast<ISteamRemoteStorage *>(static_cast<ISteamRemoteStorage *>(steam_remote_storage));
    }

    // we can get here if one of the if-statements didn't return in all paths
    PRINT_DEBUG("Missing handling for interface: %s", pchVersion);
    return reinterpret_cast<ISteamRemoteStorage *>(static_cast<ISteamRemoteStorage *>(steam_remote_storage));
}

// user screenshots
ISteamScreenshots *Steam_Client::GetISteamScreenshots( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("%s", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamuser) return NULL;


    if (strcmp(pchVersion, STEAMSCREENSHOTS_INTERFACE_VERSION) == 0) {
        return reinterpret_cast<ISteamScreenshots *>(static_cast<ISteamScreenshots *>(steam_screenshots));
    } else {
        return reinterpret_cast<ISteamScreenshots *>(static_cast<ISteamScreenshots *>(steam_screenshots));
    }
    
    // we can get here if one of the if-statements didn't return in all paths
    PRINT_DEBUG("Missing handling for interface: %s", pchVersion);
    return reinterpret_cast<ISteamScreenshots *>(static_cast<ISteamScreenshots *>(steam_screenshots));
}


// Expose HTTP interface
ISteamHTTP *Steam_Client::GetISteamHTTP( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("%s", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamuser) return NULL;
    Steam_HTTP *steam_http_temp;

    if (steam_pipes[hSteamPipe] == Steam_Pipe::SERVER) {
        steam_http_temp = steam_gameserver_http;
    } else {
        steam_http_temp = steam_http;
    }

    if (strcmp(pchVersion, "STEAMHTTP_INTERFACE_VERSION001") == 0) {
        return reinterpret_cast<ISteamHTTP *>(static_cast<ISteamHTTP001 *>(steam_http_temp));
    } else if (strcmp(pchVersion, "STEAMHTTP_INTERFACE_VERSION002") == 0) {
        return reinterpret_cast<ISteamHTTP *>(static_cast<ISteamHTTP002 *>(steam_http_temp));
    } else if (strcmp(pchVersion, STEAMHTTP_INTERFACE_VERSION) == 0) {
        return reinterpret_cast<ISteamHTTP *>(static_cast<ISteamHTTP *>(steam_http_temp));
    } else {
        return reinterpret_cast<ISteamHTTP *>(static_cast<ISteamHTTP *>(steam_http_temp));
    }

    // we can get here if one of the if-statements didn't return in all paths
    PRINT_DEBUG("Missing handling for interface: %s", pchVersion);
    return reinterpret_cast<ISteamHTTP *>(static_cast<ISteamHTTP *>(steam_http_temp));
}

// Deprecated - the ISteamUnifiedMessages interface is no longer intended for public consumption.
void *Steam_Client::DEPRECATED_GetISteamUnifiedMessages( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion ) 
{
    PRINT_DEBUG("%s", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamuser) return NULL;

    if (strcmp(pchVersion, STEAMUNIFIEDMESSAGES_INTERFACE_VERSION) == 0) {
        return (void *)(ISteamUnifiedMessages *)steam_unified_messages;
    } else {
        return (void *)(ISteamUnifiedMessages *)steam_unified_messages;
    }

    // we can get here if one of the if-statements didn't return in all paths
    PRINT_DEBUG("Missing handling for interface: %s", pchVersion);
    return (void *)(ISteamUnifiedMessages *)steam_unified_messages;
}

ISteamUnifiedMessages *Steam_Client::GetISteamUnifiedMessages( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("%s", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamuser) return NULL;

    if (strcmp(pchVersion, STEAMUNIFIEDMESSAGES_INTERFACE_VERSION) == 0) {
        return reinterpret_cast<ISteamUnifiedMessages *>(static_cast<ISteamUnifiedMessages *>(steam_unified_messages));
    } else {
        return reinterpret_cast<ISteamUnifiedMessages *>(static_cast<ISteamUnifiedMessages *>(steam_unified_messages));
    }

    // we can get here if one of the if-statements didn't return in all paths
    PRINT_DEBUG("Missing handling for interface: %s", pchVersion);
    return steam_unified_messages;
}

// Exposes the ISteamController interface
ISteamController *Steam_Client::GetISteamController( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("%s", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamUser) return NULL;

    if (strcmp(pchVersion, "STEAMCONTROLLER_INTERFACE_VERSION") == 0) {
        return reinterpret_cast<ISteamController *>(static_cast<ISteamController001 *>(steam_controller));
    } else if (strcmp(pchVersion, "STEAMCONTROLLER_INTERFACE_VERSION_002") == 0) {
        //I'm pretty sure this interface is never actually used
        return reinterpret_cast<ISteamController *>(static_cast<ISteamController003 *>(steam_controller));
    } else if (strcmp(pchVersion, "SteamController003") == 0) {
        return reinterpret_cast<ISteamController *>(static_cast<ISteamController003 *>(steam_controller));
    } else if (strcmp(pchVersion, "SteamController004") == 0) {
        return reinterpret_cast<ISteamController *>(static_cast<ISteamController004 *>(steam_controller));
    } else if (strcmp(pchVersion, "SteamController005") == 0) {
        return reinterpret_cast<ISteamController *>(static_cast<ISteamController005 *>(steam_controller));
    } else if (strcmp(pchVersion, "SteamController006") == 0) {
        return reinterpret_cast<ISteamController *>(static_cast<ISteamController006 *>(steam_controller));
    } else if (strcmp(pchVersion, "SteamController007") == 0) {
        return reinterpret_cast<ISteamController *>(static_cast<ISteamController007 *>(steam_controller));
    } else if (strcmp(pchVersion, STEAMCONTROLLER_INTERFACE_VERSION) == 0) {
        return reinterpret_cast<ISteamController *>(static_cast<ISteamController *>(steam_controller));
    } else {
        return reinterpret_cast<ISteamController *>(static_cast<ISteamController *>(steam_controller));
    }

    // we can get here if one of the if-statements didn't return in all paths
    PRINT_DEBUG("Missing handling for interface: %s", pchVersion);
    return reinterpret_cast<ISteamController *>(static_cast<ISteamController *>(steam_controller));
}

// Exposes the ISteamUGC interface
ISteamUGC *Steam_Client::GetISteamUGC( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("%s", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamUser) return NULL;
    Steam_UGC *steam_ugc_temp;

    if (steam_pipes[hSteamPipe] == Steam_Pipe::SERVER) {
        steam_ugc_temp = steam_gameserver_ugc;
    } else {
        steam_ugc_temp = steam_ugc;
    }

    if (strcmp(pchVersion, "STEAMUGC_INTERFACE_VERSION") == 0) {
        //Is this actually a valid interface version?
        return reinterpret_cast<ISteamUGC *>(static_cast<ISteamUGC001 *>(steam_ugc_temp));
    } else if (strcmp(pchVersion, "STEAMUGC_INTERFACE_VERSION001") == 0) {
        return reinterpret_cast<ISteamUGC *>(static_cast<ISteamUGC001 *>(steam_ugc_temp));
    } else if (strcmp(pchVersion, "STEAMUGC_INTERFACE_VERSION002") == 0) {
        return reinterpret_cast<ISteamUGC *>(static_cast<ISteamUGC002 *>(steam_ugc_temp));
    } else if (strcmp(pchVersion, "STEAMUGC_INTERFACE_VERSION003") == 0) {
        return reinterpret_cast<ISteamUGC *>(static_cast<ISteamUGC003 *>(steam_ugc_temp));
    } else if (strcmp(pchVersion, "STEAMUGC_INTERFACE_VERSION004") == 0) {
        return reinterpret_cast<ISteamUGC *>(static_cast<ISteamUGC004 *>(steam_ugc_temp));
    } else if (strcmp(pchVersion, "STEAMUGC_INTERFACE_VERSION005") == 0) {
        return reinterpret_cast<ISteamUGC *>(static_cast<ISteamUGC005 *>(steam_ugc_temp));
    } else if (strcmp(pchVersion, "STEAMUGC_INTERFACE_VERSION006") == 0) {
        return reinterpret_cast<ISteamUGC *>(static_cast<ISteamUGC006 *>(steam_ugc_temp));
    } else if (strcmp(pchVersion, "STEAMUGC_INTERFACE_VERSION007") == 0) {
        return reinterpret_cast<ISteamUGC *>(static_cast<ISteamUGC007 *>(steam_ugc_temp));
    } else if (strcmp(pchVersion, "STEAMUGC_INTERFACE_VERSION008") == 0) {
        return reinterpret_cast<ISteamUGC *>(static_cast<ISteamUGC008 *>(steam_ugc_temp));
    } else if (strcmp(pchVersion, "STEAMUGC_INTERFACE_VERSION009") == 0) {
        return reinterpret_cast<ISteamUGC *>(static_cast<ISteamUGC009 *>(steam_ugc_temp));
    } else if (strcmp(pchVersion, "STEAMUGC_INTERFACE_VERSION010") == 0) {
        return reinterpret_cast<ISteamUGC *>(static_cast<ISteamUGC010 *>(steam_ugc_temp));
    } else if (strcmp(pchVersion, "STEAMUGC_INTERFACE_VERSION011") == 0) {
        //TODO ?
        return reinterpret_cast<ISteamUGC *>(static_cast<ISteamUGC012 *>(steam_ugc_temp));
    } else if (strcmp(pchVersion, "STEAMUGC_INTERFACE_VERSION012") == 0) {
        return reinterpret_cast<ISteamUGC *>(static_cast<ISteamUGC012 *>(steam_ugc_temp));
    } else if (strcmp(pchVersion, "STEAMUGC_INTERFACE_VERSION013") == 0) {
        return reinterpret_cast<ISteamUGC *>(static_cast<ISteamUGC013 *>(steam_ugc_temp));
    } else if (strcmp(pchVersion, "STEAMUGC_INTERFACE_VERSION014") == 0) {
        return reinterpret_cast<ISteamUGC *>(static_cast<ISteamUGC014 *>(steam_ugc_temp));
    } else if (strcmp(pchVersion, "STEAMUGC_INTERFACE_VERSION015") == 0) {
        return reinterpret_cast<ISteamUGC *>(static_cast<ISteamUGC015 *>(steam_ugc_temp));
    } else if (strcmp(pchVersion, "STEAMUGC_INTERFACE_VERSION016") == 0) {
        return reinterpret_cast<ISteamUGC *>(static_cast<ISteamUGC016 *>(steam_ugc_temp));
    } else if (strcmp(pchVersion, "STEAMUGC_INTERFACE_VERSION017") == 0) {
        return reinterpret_cast<ISteamUGC *>(static_cast<ISteamUGC017 *>(steam_ugc_temp));
    } else if (strcmp(pchVersion, STEAMUGC_INTERFACE_VERSION) == 0) {
        return reinterpret_cast<ISteamUGC *>(static_cast<ISteamUGC *>(steam_ugc_temp));
    } else {
        return reinterpret_cast<ISteamUGC *>(static_cast<ISteamUGC *>(steam_ugc_temp));
    }

    // we can get here if one of the if-statements didn't return in all paths
    PRINT_DEBUG("Missing handling for interface: %s", pchVersion);
    return reinterpret_cast<ISteamUGC *>(static_cast<ISteamUGC *>(steam_ugc_temp));
}

// returns app list interface, only available on specially registered apps
ISteamAppList *Steam_Client::GetISteamAppList( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("%s", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamUser) return NULL;
    
    if (strcmp(pchVersion, STEAMAPPLIST_INTERFACE_VERSION) == 0) {
        return reinterpret_cast<ISteamAppList *>(static_cast<ISteamAppList *>(steam_applist));
    } else {
        return reinterpret_cast<ISteamAppList *>(static_cast<ISteamAppList *>(steam_applist));
    }

    // we can get here if one of the if-statements didn't return in all paths
    PRINT_DEBUG("Missing handling for interface: %s", pchVersion);
    return reinterpret_cast<ISteamAppList *>(static_cast<ISteamAppList *>(steam_applist));
}

// Music Player
ISteamMusic *Steam_Client::GetISteamMusic( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("%s", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamuser) return NULL;

    if (strcmp(pchVersion, STEAMMUSIC_INTERFACE_VERSION) == 0) {
        return reinterpret_cast<ISteamMusic *>(static_cast<ISteamMusic *>(steam_music));
    } else {
        return reinterpret_cast<ISteamMusic *>(static_cast<ISteamMusic *>(steam_music));
    }

    // we can get here if one of the if-statements didn't return in all paths
    PRINT_DEBUG("Missing handling for interface: %s", pchVersion);
    return reinterpret_cast<ISteamMusic *>(static_cast<ISteamMusic *>(steam_music));
}

// Music Player Remote
ISteamMusicRemote *Steam_Client::GetISteamMusicRemote(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion)
{
    PRINT_DEBUG("%s", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamuser) return NULL;
    
    if (strcmp(pchVersion, STEAMMUSICREMOTE_INTERFACE_VERSION) == 0) {
        return reinterpret_cast<ISteamMusicRemote *>(static_cast<ISteamMusicRemote *>(steam_musicremote));
    } else {
        return reinterpret_cast<ISteamMusicRemote *>(static_cast<ISteamMusicRemote *>(steam_musicremote));
    }

    // we can get here if one of the if-statements didn't return in all paths
    PRINT_DEBUG("Missing handling for interface: %s", pchVersion);
    return reinterpret_cast<ISteamMusicRemote *>(static_cast<ISteamMusicRemote *>(steam_musicremote));
}

// html page display
ISteamHTMLSurface *Steam_Client::GetISteamHTMLSurface(HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion)
{
    PRINT_DEBUG("%s", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamuser) return NULL;

    if (strcmp(pchVersion, "STEAMHTMLSURFACE_INTERFACE_VERSION_001") == 0) {
        return reinterpret_cast<ISteamHTMLSurface *>(static_cast<ISteamHTMLSurface001 *>(steam_HTMLsurface));
    } else if (strcmp(pchVersion, "STEAMHTMLSURFACE_INTERFACE_VERSION_002") == 0) {
        return reinterpret_cast<ISteamHTMLSurface *>(static_cast<ISteamHTMLSurface002 *>(steam_HTMLsurface));
    } else if (strcmp(pchVersion, "STEAMHTMLSURFACE_INTERFACE_VERSION_003") == 0) {
        return reinterpret_cast<ISteamHTMLSurface *>(static_cast<ISteamHTMLSurface003 *>(steam_HTMLsurface));
    } else if (strcmp(pchVersion, "STEAMHTMLSURFACE_INTERFACE_VERSION_004") == 0) {
        return reinterpret_cast<ISteamHTMLSurface *>(static_cast<ISteamHTMLSurface004 *>(steam_HTMLsurface));
    } else if (strcmp(pchVersion, STEAMHTMLSURFACE_INTERFACE_VERSION) == 0) {
        return reinterpret_cast<ISteamHTMLSurface *>(static_cast<ISteamHTMLSurface *>(steam_HTMLsurface));
    } else {
        return reinterpret_cast<ISteamHTMLSurface *>(static_cast<ISteamHTMLSurface *>(steam_HTMLsurface));
    }

    // we can get here if one of the if-statements didn't return in all paths
    PRINT_DEBUG("Missing handling for interface: %s", pchVersion);
    return reinterpret_cast<ISteamHTMLSurface *>(static_cast<ISteamHTMLSurface *>(steam_HTMLsurface));
}

// inventory
ISteamInventory *Steam_Client::GetISteamInventory( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("%s", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamuser) return NULL;
    Steam_Inventory *steam_inventory_temp;
    Settings *settings_temp;
    SteamCallBacks *callbacks_temp;
    SteamCallResults *callback_results_temp;

    if (steam_pipes[hSteamPipe] == Steam_Pipe::SERVER) {
        steam_inventory_temp = steam_gameserver_inventory;
    } else {
        steam_inventory_temp = steam_inventory;
    }

    if (strcmp(pchVersion, "STEAMINVENTORY_INTERFACE_V001") == 0) {
        return reinterpret_cast<ISteamInventory *>(static_cast<ISteamInventory001 *>(steam_inventory_temp));
    } else if (strcmp(pchVersion, "STEAMINVENTORY_INTERFACE_V002") == 0) {
        return reinterpret_cast<ISteamInventory *>(static_cast<ISteamInventory002 *>(steam_inventory_temp));
    } else if (strcmp(pchVersion, STEAMINVENTORY_INTERFACE_VERSION) == 0) {
        return reinterpret_cast<ISteamInventory *>(static_cast<ISteamInventory *>(steam_inventory_temp));
    } else {
        return reinterpret_cast<ISteamInventory *>(static_cast<ISteamInventory *>(steam_inventory_temp));
    }

    // we can get here if one of the if-statements didn't return in all paths
    PRINT_DEBUG("Missing handling for interface: %s", pchVersion);
    return reinterpret_cast<ISteamInventory *>(static_cast<ISteamInventory *>(steam_inventory_temp));
}

// Video
ISteamVideo *Steam_Client::GetISteamVideo( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("%s", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamuser) return NULL;
    
    if (strcmp(pchVersion, STEAMVIDEO_INTERFACE_VERSION) == 0) {
        return reinterpret_cast<ISteamVideo *>(static_cast<ISteamVideo *>(steam_video));
    } else {
        return reinterpret_cast<ISteamVideo *>(static_cast<ISteamVideo *>(steam_video));
    }

    // we can get here if one of the if-statements didn't return in all paths
    PRINT_DEBUG("Missing handling for interface: %s", pchVersion);
    return reinterpret_cast<ISteamVideo *>(static_cast<ISteamVideo *>(steam_video));
}

// Parental controls
ISteamParentalSettings *Steam_Client::GetISteamParentalSettings( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("%s", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamuser) return NULL;
    
    if (strcmp(pchVersion, STEAMPARENTALSETTINGS_INTERFACE_VERSION) == 0) {
        return reinterpret_cast<ISteamParentalSettings *>(static_cast<ISteamParentalSettings *>(steam_parental));
    } else {
        return reinterpret_cast<ISteamParentalSettings *>(static_cast<ISteamParentalSettings *>(steam_parental));
    }

    // we can get here if one of the if-statements didn't return in all paths
    PRINT_DEBUG("Missing handling for interface: %s", pchVersion);
    return reinterpret_cast<ISteamParentalSettings *>(static_cast<ISteamParentalSettings *>(steam_parental));
}

ISteamMasterServerUpdater *Steam_Client::GetISteamMasterServerUpdater( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("%s", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamUser) return NULL;
    
    if (strcmp(pchVersion, STEAMMASTERSERVERUPDATER_INTERFACE_VERSION) == 0) {
        return reinterpret_cast<ISteamMasterServerUpdater *>(static_cast<ISteamMasterServerUpdater *>(steam_masterserver_updater));
    } else {
        return reinterpret_cast<ISteamMasterServerUpdater *>(static_cast<ISteamMasterServerUpdater *>(steam_masterserver_updater));
    }

    // we can get here if one of the if-statements didn't return in all paths
    PRINT_DEBUG("Missing handling for interface: %s", pchVersion);
    return reinterpret_cast<ISteamMasterServerUpdater *>(static_cast<ISteamMasterServerUpdater *>(steam_masterserver_updater));
}

ISteamContentServer *Steam_Client::GetISteamContentServer( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("%s", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamUser) return NULL;
    return NULL;
}

// game search
ISteamGameSearch *Steam_Client::GetISteamGameSearch( HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("%s", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamuser) return NULL;
    
    if (strcmp(pchVersion, STEAMGAMESEARCH_INTERFACE_VERSION) == 0) {
        return reinterpret_cast<ISteamGameSearch *>(static_cast<ISteamGameSearch *>(steam_game_search));
    } else {
        return reinterpret_cast<ISteamGameSearch *>(static_cast<ISteamGameSearch *>(steam_game_search));
    }

    // we can get here if one of the if-statements didn't return in all paths
    PRINT_DEBUG("Missing handling for interface: %s", pchVersion);
    return reinterpret_cast<ISteamGameSearch *>(static_cast<ISteamGameSearch *>(steam_game_search));
}

// Exposes the Steam Input interface for controller support
ISteamInput *Steam_Client::GetISteamInput( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("%s", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamUser) return NULL;

    if (strcmp(pchVersion, "SteamInput001") == 0) {
        return reinterpret_cast<ISteamInput *>(static_cast<ISteamInput001 *>(steam_controller));
    } else if (strcmp(pchVersion, "SteamInput002") == 0) {
        return reinterpret_cast<ISteamInput *>(static_cast<ISteamInput002 *>(steam_controller));
    } else if (strcmp(pchVersion, "SteamInput005") == 0) {
        return reinterpret_cast<ISteamInput *>(static_cast<ISteamInput005 *>(steam_controller));
    } else if (strcmp(pchVersion, STEAMINPUT_INTERFACE_VERSION) == 0) {
        return reinterpret_cast<ISteamInput *>(static_cast<ISteamInput *>(steam_controller));
    } else {
        return reinterpret_cast<ISteamInput *>(static_cast<ISteamInput *>(steam_controller));
    }

    // we can get here if one of the if-statements didn't return in all paths
    PRINT_DEBUG("Missing handling for interface: %s", pchVersion);
    return reinterpret_cast<ISteamInput *>(static_cast<ISteamInput *>(steam_controller));
}

// Steam Parties interface
ISteamParties *Steam_Client::GetISteamParties( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("%s", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamUser) return NULL;
    
    if (strcmp(pchVersion, STEAMPARTIES_INTERFACE_VERSION) == 0) {
        return reinterpret_cast<ISteamParties *>(static_cast<ISteamParties *>(steam_parties));
    } else {
        return reinterpret_cast<ISteamParties *>(static_cast<ISteamParties *>(steam_parties));
    }

    // we can get here if one of the if-statements didn't return in all paths
    PRINT_DEBUG("Missing handling for interface: %s", pchVersion);
    return reinterpret_cast<ISteamParties *>(static_cast<ISteamParties *>(steam_parties));
}

ISteamRemotePlay *Steam_Client::GetISteamRemotePlay( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("%s", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamUser) return NULL;

    if (strcmp(pchVersion, "STEAMREMOTEPLAY_INTERFACE_VERSION001") == 0) {
        return reinterpret_cast<ISteamRemotePlay *>(static_cast<ISteamRemotePlay001 *>(steam_remoteplay));
    } else if (strcmp(pchVersion, STEAMREMOTEPLAY_INTERFACE_VERSION) == 0) {
        return reinterpret_cast<ISteamRemotePlay *>(static_cast<ISteamRemotePlay *>(steam_remoteplay));
    } else {
        return reinterpret_cast<ISteamRemotePlay *>(static_cast<ISteamRemotePlay *>(steam_remoteplay));
    }

    // we can get here if one of the if-statements didn't return in all paths
    PRINT_DEBUG("Missing handling for interface: %s", pchVersion);
    return reinterpret_cast<ISteamRemotePlay *>(static_cast<ISteamRemotePlay *>(steam_remoteplay));
}

ISteamAppTicket *Steam_Client::GetAppTicket( HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char *pchVersion )
{
    PRINT_DEBUG("%s", pchVersion);
    if (!steam_pipes.count(hSteamPipe) || !hSteamUser) return NULL;

    if (strcmp(pchVersion, STEAMAPPTICKET_INTERFACE_VERSION) == 0) {
        return reinterpret_cast<ISteamAppTicket *>(static_cast<ISteamAppTicket *>(steam_app_ticket));
    } else {
        return reinterpret_cast<ISteamAppTicket *>(static_cast<ISteamAppTicket *>(steam_app_ticket));
    }

    // we can get here if one of the if-statements didn't return in all paths
    PRINT_DEBUG("Missing handling for interface: %s", pchVersion);
    return reinterpret_cast<ISteamAppTicket *>(static_cast<ISteamAppTicket *>(steam_app_ticket));
}
