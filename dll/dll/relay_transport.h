#ifndef RELAY_TRANSPORT_INCLUDE
#define RELAY_TRANSPORT_INCLUDE

#include "network.h"

class Relay_Transport
{
    struct InboundPacket {
        Common_Message message{};
        IP_PORT ip_port{};
        bool reliable = false;
    };

    struct DisconnectEvent {
        std::vector<CSteamID> ids{};
        uint32 virtual_ip = 0;
        uint16 virtual_port = 0;
    };

    std::string host{};
    uint16 tcp_port{};
    uint16 udp_port{};
    uint16 listen_port{};
    uint32 appid{};
    std::vector<CSteamID> local_ids{};

    sock_t tcp_socket = static_cast<sock_t>(~0);
    sock_t udp_socket = static_cast<sock_t>(~0);
    bool tcp_connecting = false;
    bool tcp_connected = false;
    bool hello_sent = false;
    bool welcomed = false;
    bool registration_dirty = true;
    uint64 session_token = 0;
    uint32 assigned_virtual_ip = 0;
    uint16 assigned_virtual_port = 0;
    std::vector<char> tcp_recv_buffer{};
    std::vector<char> tcp_send_buffer{};
    std::queue<InboundPacket> inbound_packets{};
    std::queue<DisconnectEvent> disconnect_events{};
    std::chrono::steady_clock::time_point last_tcp_heartbeat{};
    std::chrono::steady_clock::time_point last_udp_heartbeat{};
    std::chrono::steady_clock::time_point next_connect_attempt{};
    std::recursive_mutex mutex{};

    void schedule_reconnect_locked(std::chrono::seconds delay = std::chrono::seconds(2));
    void disconnect_locked();
    bool ensure_connected_locked();
    bool resolve_host_locked(sockaddr_in &addr, uint16 port);
    bool open_udp_locked(const sockaddr_in &addr);
    bool open_tcp_locked(const sockaddr_in &addr);
    bool finish_tcp_connect_locked();
    void flush_tcp_send_locked();
    void read_tcp_locked();
    void read_udp_locked();
    void send_periodic_heartbeats_locked();
    void send_registration_locked(bool hello);
    bool send_udp_message_locked(uint16 type, uint32 flags, uint64 source_id, uint64 dest_id, uint32 dest_ip, uint16 dest_port, const std::vector<char> &payload);
    bool queue_tcp_message_locked(uint16 type, uint32 flags, uint64 source_id, uint64 dest_id, uint32 dest_ip, uint16 dest_port, const std::vector<char> &payload);
    void handle_incoming_locked(const std::vector<char> &packet, bool reliable);
    std::vector<char> encode_registration_payload_locked() const;

public:
    Relay_Transport(const std::string &host, uint16 tcp_port, uint16 udp_port, uint16 listen_port, uint32 appid, CSteamID initial_id);
    ~Relay_Transport();

    bool enabled() const;
    bool ready();
    void Run();
    void set_appid(uint32 appid);
    void add_listen_id(CSteamID id);
    bool Send(Common_Message *msg, bool reliable);
    bool SendToEndpoint(Common_Message *msg, uint32 ip, uint16 port, bool reliable);
    bool SendBroadcast(Common_Message *msg);
    bool PollPacket(Common_Message *msg, IP_PORT *ip_port, bool *reliable);
    bool PollDisconnect(std::vector<CSteamID> &ids, uint32 &virtual_ip, uint16 &virtual_port);
    uint32 virtual_ip();
    uint16 virtual_port();
};

#endif
