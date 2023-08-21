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

#ifndef NETWORK_INCLUDE
#define NETWORK_INCLUDE

#include "base.h"
#include <curl/curl.h>

inline bool protobuf_message_equal(const google::protobuf::MessageLite& msg_a,
                const google::protobuf::MessageLite& msg_b) {
  return (msg_a.GetTypeName() == msg_b.GetTypeName()) &&
      (msg_a.SerializeAsString() == msg_b.SerializeAsString());
}


#define DEFAULT_PORT 47584

#if defined(STEAM_WIN32)
typedef unsigned int sock_t;
#else
typedef int sock_t;
#endif

struct IP_PORT {
    uint32 ip;
    uint16 port;
    bool operator <(const IP_PORT& other) const
    {
        return (ip < other.ip) || (ip == other.ip && port < other.port);
    }
};

struct Network_Callback {
    void (*message_callback)(void *object, Common_Message *msg);
    void *object;
    CSteamID steam_id;
};

enum Callback_Ids {
    CALLBACK_ID_USER_STATUS,
    CALLBACK_ID_LOBBY,
    CALLBACK_ID_NETWORKING,
    CALLBACK_ID_GAMESERVER,
    CALLBACK_ID_FRIEND,
    CALLBACK_ID_AUTH_TICKET,
    CALLBACK_ID_FRIEND_MESSAGES,
    CALLBACK_ID_NETWORKING_SOCKETS,
    CALLBACK_ID_STEAM_MESSAGES,
    CALLBACK_ID_NETWORKING_MESSAGES,

    CALLBACK_IDS_MAX
};

struct Network_Callback_Container {
    std::vector<struct Network_Callback> callbacks;
};

struct TCP_Socket {
    sock_t sock = ~0;
    bool received_data = false;
    std::vector<char> recv_buffer;
    std::vector<char> send_buffer;
    std::chrono::high_resolution_clock::time_point last_heartbeat_sent, last_heartbeat_received;
};

struct Connection {
    struct TCP_Socket tcp_socket_outgoing, tcp_socket_incoming;
    bool connected = false;
    IP_PORT udp_ip_port;
    bool udp_pinged = false;
    IP_PORT tcp_ip_port;
    std::vector<CSteamID> ids;
    uint32 appid;
    std::chrono::high_resolution_clock::time_point last_received;
};

class Networking {
    bool enabled = false;
    bool query_alive;
    std::chrono::high_resolution_clock::time_point last_run;
    sock_t query_socket, udp_socket, tcp_socket;
    uint16 udp_port, tcp_port;
    uint32 own_ip;
    std::vector<struct Connection> connections;
    struct Connection *find_connection(CSteamID id, uint32 appid = 0);
    struct Connection *new_connection(CSteamID id, uint32 appid);

    bool handle_announce(Common_Message *msg, IP_PORT ip_port);
    bool handle_low_level_udp(Common_Message *msg, IP_PORT ip_port);
    bool handle_tcp(Common_Message *msg, struct TCP_Socket &socket);
    void send_announce_broadcasts();

    std::vector<CSteamID> ids;
    uint32 appid;
    std::chrono::high_resolution_clock::time_point last_broadcast;
    std::vector<IP_PORT> custom_broadcasts;

    std::vector<struct TCP_Socket> accepted;
    std::recursive_mutex mutex;

    struct Network_Callback_Container callbacks[CALLBACK_IDS_MAX];
    std::vector<Common_Message> local_send;

    bool add_id_connection(struct Connection *connection, CSteamID steam_id);
    void run_callbacks(Callback_Ids id, Common_Message *msg);
    void run_callback_user(CSteamID steam_id, bool online, uint32 appid);
    void do_callbacks_message(Common_Message *msg);

    Common_Message create_announce(bool request);
public:
    //NOTE: for all functions ips/ports are passed/returned in host byte order
    //ex: 127.0.0.1 should be passed as 0x7F000001
    static std::set<IP_PORT> resolve_ip(std::string dns);
    Networking(CSteamID id, uint32 appid, uint16 port, std::set<IP_PORT> *custom_broadcasts, bool disable_sockets);
    ~Networking();
    void addListenId(CSteamID id);
    void setAppID(uint32 appid);
    void Run();
    bool sendTo(Common_Message *msg, bool reliable, Connection *conn = NULL);
    bool sendToAllIndividuals(Common_Message *msg, bool reliable);
    bool sendToAll(Common_Message *msg, bool reliable);
    bool sendToIPPort(Common_Message *msg, uint32 ip, uint16 port, bool reliable);

    bool setCallback(Callback_Ids id, CSteamID steam_id, void (*message_callback)(void *object, Common_Message *msg), void *object);
    uint32 getIP(CSteamID id);
    uint32 getOwnIP();

    void startQuery(IP_PORT ip_port);
    void shutDownQuery();
    bool isQueryAlive();
};

#endif
