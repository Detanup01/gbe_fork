#include "dll/relay_transport.h"

namespace {
constexpr uint32 RELAY_MAGIC = 0x4C524247; // GBRL
constexpr uint16 RELAY_VERSION = 1;
constexpr uint16 RELAY_MSG_HELLO = 1;
constexpr uint16 RELAY_MSG_REGISTER = 2;
constexpr uint16 RELAY_MSG_WELCOME = 3;
constexpr uint16 RELAY_MSG_HEARTBEAT = 4;
constexpr uint16 RELAY_MSG_RELIABLE = 5;
constexpr uint16 RELAY_MSG_UNRELIABLE = 6;
constexpr uint16 RELAY_MSG_DISCONNECT = 7;

constexpr uint32 RELAY_FLAG_BROADCAST = 1u << 0;
constexpr uint32 RELAY_FLAG_HAS_DEST_STEAMID = 1u << 1;
constexpr uint32 RELAY_FLAG_HAS_DEST_ENDPOINT = 1u << 2;

static const char *relay_msg_name(uint16 type)
{
    switch (type) {
    case RELAY_MSG_HELLO: return "hello";
    case RELAY_MSG_REGISTER: return "register";
    case RELAY_MSG_WELCOME: return "welcome";
    case RELAY_MSG_HEARTBEAT: return "heartbeat";
    case RELAY_MSG_RELIABLE: return "reliable";
    case RELAY_MSG_UNRELIABLE: return "unreliable";
    case RELAY_MSG_DISCONNECT: return "disconnect";
    default: return "unknown";
    }
}

static bool relay_socket_valid(sock_t sock)
{
#if defined(STEAM_WIN32)
    return sock != (sock_t)INVALID_SOCKET && sock != (sock_t)~0;
#else
    return sock >= 0;
#endif
}

static void relay_close_socket(sock_t &sock)
{
    if (!relay_socket_valid(sock)) return;
#if defined(STEAM_WIN32)
    closesocket(sock);
#else
    close(sock);
#endif
    sock = static_cast<sock_t>(~0);
}

static bool relay_set_nonblocking(sock_t sock)
{
#if defined(STEAM_WIN32)
    u_long mode = 1;
    return ioctlsocket(sock, FIONBIO, &mode) == 0;
#else
    return fcntl(sock, F_SETFL, O_NONBLOCK, 1) == 0;
#endif
}

static bool relay_last_error_is_would_block()
{
#if defined(STEAM_WIN32)
    int err = WSAGetLastError();
    return err == WSAEWOULDBLOCK || err == WSAEINPROGRESS || err == WSAEALREADY;
#else
    return errno == EWOULDBLOCK || errno == EAGAIN || errno == EINPROGRESS || errno == EALREADY;
#endif
}

static void append_u16(std::vector<char> &out, uint16 value)
{
    out.push_back(static_cast<char>(value & 0xFF));
    out.push_back(static_cast<char>((value >> 8) & 0xFF));
}

static void append_u32(std::vector<char> &out, uint32 value)
{
    out.push_back(static_cast<char>(value & 0xFF));
    out.push_back(static_cast<char>((value >> 8) & 0xFF));
    out.push_back(static_cast<char>((value >> 16) & 0xFF));
    out.push_back(static_cast<char>((value >> 24) & 0xFF));
}

static void append_u64(std::vector<char> &out, uint64 value)
{
    for (int i = 0; i < 8; ++i) {
        out.push_back(static_cast<char>((value >> (i * 8)) & 0xFF));
    }
}

static bool read_u16(const std::vector<char> &in, size_t &offset, uint16 &value)
{
    if (offset + 2 > in.size()) return false;
    value = static_cast<uint16>(static_cast<uint8_t>(in[offset])) |
            (static_cast<uint16>(static_cast<uint8_t>(in[offset + 1])) << 8);
    offset += 2;
    return true;
}

static bool read_u32(const std::vector<char> &in, size_t &offset, uint32 &value)
{
    if (offset + 4 > in.size()) return false;
    value =
        static_cast<uint32>(static_cast<uint8_t>(in[offset])) |
        (static_cast<uint32>(static_cast<uint8_t>(in[offset + 1])) << 8) |
        (static_cast<uint32>(static_cast<uint8_t>(in[offset + 2])) << 16) |
        (static_cast<uint32>(static_cast<uint8_t>(in[offset + 3])) << 24);
    offset += 4;
    return true;
}

static bool read_u64(const std::vector<char> &in, size_t &offset, uint64 &value)
{
    if (offset + 8 > in.size()) return false;
    value = 0;
    for (int i = 0; i < 8; ++i) {
        value |= (static_cast<uint64>(static_cast<uint8_t>(in[offset + i])) << (i * 8));
    }
    offset += 8;
    return true;
}

struct RelayEnvelope {
    uint16 type = 0;
    uint32 flags = 0;
    uint32 appid = 0;
    uint64 source_id = 0;
    uint64 dest_id = 0;
    uint32 source_virtual_ip = 0;
    uint16 source_virtual_port = 0;
    uint32 dest_virtual_ip = 0;
    uint16 dest_virtual_port = 0;
    uint64 session_token = 0;
    std::vector<char> payload{};
};

static std::vector<char> serialize_envelope(const RelayEnvelope &env)
{
    std::vector<char> out{};
    out.reserve(56 + env.payload.size());
    append_u32(out, RELAY_MAGIC);
    append_u16(out, RELAY_VERSION);
    append_u16(out, env.type);
    append_u32(out, env.flags);
    append_u32(out, env.appid);
    append_u64(out, env.source_id);
    append_u64(out, env.dest_id);
    append_u32(out, env.source_virtual_ip);
    append_u16(out, env.source_virtual_port);
    append_u32(out, env.dest_virtual_ip);
    append_u16(out, env.dest_virtual_port);
    append_u64(out, env.session_token);
    append_u32(out, static_cast<uint32>(env.payload.size()));
    out.insert(out.end(), env.payload.begin(), env.payload.end());
    return out;
}

static bool deserialize_envelope(const std::vector<char> &bytes, RelayEnvelope &env)
{
    size_t offset = 0;
    uint32 magic = 0;
    uint16 version = 0;
    uint32 payload_len = 0;
    if (!read_u32(bytes, offset, magic) || magic != RELAY_MAGIC) return false;
    if (!read_u16(bytes, offset, version) || version != RELAY_VERSION) return false;
    if (!read_u16(bytes, offset, env.type)) return false;
    if (!read_u32(bytes, offset, env.flags)) return false;
    if (!read_u32(bytes, offset, env.appid)) return false;
    if (!read_u64(bytes, offset, env.source_id)) return false;
    if (!read_u64(bytes, offset, env.dest_id)) return false;
    if (!read_u32(bytes, offset, env.source_virtual_ip)) return false;
    if (!read_u16(bytes, offset, env.source_virtual_port)) return false;
    if (!read_u32(bytes, offset, env.dest_virtual_ip)) return false;
    if (!read_u16(bytes, offset, env.dest_virtual_port)) return false;
    if (!read_u64(bytes, offset, env.session_token)) return false;
    if (!read_u32(bytes, offset, payload_len)) return false;
    if (offset + payload_len > bytes.size()) return false;
    env.payload.assign(bytes.begin() + static_cast<std::ptrdiff_t>(offset), bytes.begin() + static_cast<std::ptrdiff_t>(offset + payload_len));
    return true;
}

static std::vector<char> serialize_ids_payload(const std::vector<CSteamID> &ids, uint16 listen_port)
{
    std::vector<char> payload{};
    append_u16(payload, listen_port);
    append_u16(payload, static_cast<uint16>(ids.size()));
    for (const auto &id : ids) {
        append_u64(payload, id.ConvertToUint64());
    }
    return payload;
}

static bool deserialize_ids_payload(const std::vector<char> &payload, std::vector<CSteamID> &ids, uint16 &listen_port)
{
    size_t offset = 0;
    uint16 count = 0;
    if (!read_u16(payload, offset, listen_port)) return false;
    if (!read_u16(payload, offset, count)) return false;
    ids.clear();
    ids.reserve(count);
    for (uint16 i = 0; i < count; ++i) {
        uint64 raw = 0;
        if (!read_u64(payload, offset, raw)) return false;
        ids.emplace_back(raw);
    }
    return offset == payload.size();
}
}

Relay_Transport::Relay_Transport(const std::string &host, uint16 tcp_port, uint16 udp_port, uint16 listen_port, uint32 appid, CSteamID initial_id)
    : host(host)
    , tcp_port(tcp_port)
    , udp_port(udp_port)
    , listen_port(listen_port)
    , appid(appid)
{
    PRINT_DEBUG("relay transport created host='%s' tcp_port=%u udp_port=%u listen_port=%u appid=%u", host.c_str(), tcp_port, udp_port, listen_port, appid);
    if (initial_id.IsValid()) {
        local_ids.push_back(initial_id);
    }
    last_tcp_heartbeat = std::chrono::steady_clock::now();
    last_udp_heartbeat = std::chrono::steady_clock::now();
    next_connect_attempt = std::chrono::steady_clock::now();
}

Relay_Transport::~Relay_Transport()
{
    std::lock_guard<std::recursive_mutex> lock(mutex);
    disconnect_locked();
}

bool Relay_Transport::enabled() const
{
    return !host.empty() && tcp_port != 0 && udp_port != 0;
}

bool Relay_Transport::ready()
{
    std::lock_guard<std::recursive_mutex> lock(mutex);
    return welcomed && relay_socket_valid(tcp_socket) && relay_socket_valid(udp_socket);
}

void Relay_Transport::schedule_reconnect_locked(std::chrono::seconds delay)
{
    PRINT_DEBUG("relay reconnect scheduled delay=%llds", static_cast<long long>(delay.count()));
    disconnect_locked();
    next_connect_attempt = std::chrono::steady_clock::now() + delay;
    hello_sent = false;
    welcomed = false;
    session_token = 0;
    assigned_virtual_ip = 0;
    assigned_virtual_port = 0;
}

void Relay_Transport::disconnect_locked()
{
    relay_close_socket(tcp_socket);
    relay_close_socket(udp_socket);
    tcp_connecting = false;
    tcp_connected = false;
    tcp_recv_buffer.clear();
    tcp_send_buffer.clear();
}

bool Relay_Transport::resolve_host_locked(sockaddr_in &addr, uint16 port)
{
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    struct addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo *result = nullptr;
    if (getaddrinfo(host.c_str(), nullptr, &hints, &result) != 0 || result == nullptr) {
        PRINT_DEBUG("relay resolve failed for '%s'", host.c_str());
        if (result) freeaddrinfo(result);
        return false;
    }

    auto *ipv4 = reinterpret_cast<sockaddr_in *>(result->ai_addr);
    addr.sin_addr = ipv4->sin_addr;
    PRINT_DEBUG("relay resolved host '%s' port=%u ip=%s", host.c_str(), port, inet_ntoa(addr.sin_addr));
    freeaddrinfo(result);
    return true;
}

bool Relay_Transport::open_udp_locked(const sockaddr_in &addr)
{
    udp_socket = static_cast<sock_t>(socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP));
    if (!relay_socket_valid(udp_socket)) {
        PRINT_DEBUG("relay udp socket creation failed");
        return false;
    }
    if (!relay_set_nonblocking(udp_socket)) {
        PRINT_DEBUG("relay udp set nonblocking failed");
        relay_close_socket(udp_socket);
        return false;
    }

    if (connect(udp_socket, reinterpret_cast<const sockaddr *>(&addr), sizeof(addr)) != 0 && !relay_last_error_is_would_block()) {
        PRINT_DEBUG("relay udp connect failed");
        relay_close_socket(udp_socket);
        return false;
    }

    PRINT_DEBUG("relay udp socket connected");
    return true;
}

bool Relay_Transport::open_tcp_locked(const sockaddr_in &addr)
{
    tcp_socket = static_cast<sock_t>(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP));
    if (!relay_socket_valid(tcp_socket)) {
        PRINT_DEBUG("relay tcp socket creation failed");
        return false;
    }
    if (!relay_set_nonblocking(tcp_socket)) {
        PRINT_DEBUG("relay tcp set nonblocking failed");
        relay_close_socket(tcp_socket);
        return false;
    }

    int one = 1;
    setsockopt(tcp_socket, IPPROTO_TCP, TCP_NODELAY, reinterpret_cast<char *>(&one), sizeof(one));

    int res = connect(tcp_socket, reinterpret_cast<const sockaddr *>(&addr), sizeof(addr));
    if (res == 0) {
        tcp_connected = true;
        tcp_connecting = false;
        hello_sent = false;
        PRINT_DEBUG("relay tcp connect completed immediately");
        return true;
    }

    if (!relay_last_error_is_would_block()) {
        PRINT_DEBUG("relay tcp connect failed immediately");
        relay_close_socket(tcp_socket);
        return false;
    }

    tcp_connecting = true;
    tcp_connected = false;
    PRINT_DEBUG("relay tcp connect in progress");
    return true;
}

bool Relay_Transport::finish_tcp_connect_locked()
{
    if (!tcp_connecting || !relay_socket_valid(tcp_socket)) return tcp_connected;

    fd_set writefds;
    FD_ZERO(&writefds);
    FD_SET(tcp_socket, &writefds);
    timeval timeout{};
    int res = select(static_cast<int>(tcp_socket + 1), nullptr, &writefds, nullptr, &timeout);
    if (res == 0) return false;
    if (res < 0) {
        schedule_reconnect_locked();
        return false;
    }

    int so_error = 0;
#if defined(STEAM_WIN32)
    int len = sizeof(so_error);
#else
    socklen_t len = sizeof(so_error);
#endif
    if (getsockopt(tcp_socket, SOL_SOCKET, SO_ERROR, reinterpret_cast<char *>(&so_error), &len) != 0 || so_error != 0) {
        PRINT_DEBUG("relay tcp connect failed %i", so_error);
        schedule_reconnect_locked();
        return false;
    }

    tcp_connecting = false;
    tcp_connected = true;
    hello_sent = false;
    registration_dirty = true;
    PRINT_DEBUG("relay tcp connect completed");
    return true;
}

bool Relay_Transport::ensure_connected_locked()
{
    if (!enabled()) return false;
    auto now = std::chrono::steady_clock::now();
    if (relay_socket_valid(tcp_socket) && relay_socket_valid(udp_socket)) {
        if (tcp_connecting) {
            finish_tcp_connect_locked();
        }
        if (tcp_connected && !hello_sent) {
            send_registration_locked(true);
        }
        return tcp_connected;
    }

    if (now < next_connect_attempt) return false;

    PRINT_DEBUG("relay opening new tcp/udp connection");
    sockaddr_in tcp_addr{};
    sockaddr_in udp_addr{};
    if (!resolve_host_locked(tcp_addr, tcp_port) || !resolve_host_locked(udp_addr, udp_port)) {
        schedule_reconnect_locked(std::chrono::seconds(5));
        return false;
    }

    disconnect_locked();
    if (!open_udp_locked(udp_addr) || !open_tcp_locked(tcp_addr)) {
        schedule_reconnect_locked();
        return false;
    }

    if (tcp_connected) {
        send_registration_locked(true);
    }

    return tcp_connected;
}

void Relay_Transport::flush_tcp_send_locked()
{
    while (tcp_connected && relay_socket_valid(tcp_socket) && !tcp_send_buffer.empty()) {
        int sent = send(tcp_socket, tcp_send_buffer.data(), static_cast<int>(tcp_send_buffer.size()), 0);
        if (sent > 0) {
            PRINT_DEBUG("relay tcp sent bytes=%d pending=%zu", sent, tcp_send_buffer.size() - static_cast<size_t>(sent));
            tcp_send_buffer.erase(tcp_send_buffer.begin(), tcp_send_buffer.begin() + sent);
            continue;
        }
        if (sent == 0 || !relay_last_error_is_would_block()) {
            PRINT_DEBUG("relay tcp send failed");
            schedule_reconnect_locked();
        }
        return;
    }
}

void Relay_Transport::handle_incoming_locked(const std::vector<char> &packet, bool reliable)
{
    RelayEnvelope env{};
    if (!deserialize_envelope(packet, env)) {
        PRINT_DEBUG("relay incoming packet parse failed reliable=%u bytes=%zu", reliable ? 1u : 0u, packet.size());
        return;
    }

    PRINT_DEBUG("relay incoming type=%s reliable=%u appid=%u source_id=%llu dest_id=%llu payload_bytes=%zu", relay_msg_name(env.type), reliable ? 1u : 0u, env.appid, static_cast<unsigned long long>(env.source_id), static_cast<unsigned long long>(env.dest_id), env.payload.size());

    if (env.type == RELAY_MSG_WELCOME) {
        welcomed = true;
        session_token = env.session_token;
        assigned_virtual_ip = env.source_virtual_ip;
        assigned_virtual_port = env.source_virtual_port ? env.source_virtual_port : listen_port;
        registration_dirty = true;
        PRINT_DEBUG("relay welcome received virtual_ip=%u virtual_port=%u token=%llu", assigned_virtual_ip, assigned_virtual_port, static_cast<unsigned long long>(session_token));
        return;
    }

    if (env.type == RELAY_MSG_HEARTBEAT) {
        return;
    }

    if (env.type == RELAY_MSG_DISCONNECT) {
        DisconnectEvent ev{};
        uint16 peer_port = 0;
        if (!deserialize_ids_payload(env.payload, ev.ids, peer_port)) {
            PRINT_DEBUG("relay disconnect payload parse failed");
            return;
        }
        ev.virtual_ip = env.source_virtual_ip;
        ev.virtual_port = env.source_virtual_port ? env.source_virtual_port : peer_port;
        size_t disconnected_ids = ev.ids.size();
        uint32 disconnected_ip = ev.virtual_ip;
        uint16 disconnected_port = ev.virtual_port;
        disconnect_events.push(std::move(ev));
        PRINT_DEBUG("relay disconnect queued ids=%zu virtual_ip=%u virtual_port=%u", disconnected_ids, disconnected_ip, disconnected_port);
        return;
    }

    if (env.type != RELAY_MSG_RELIABLE && env.type != RELAY_MSG_UNRELIABLE) return;

    Common_Message msg{};
    if (!msg.ParseFromArray(env.payload.data(), static_cast<int>(env.payload.size()))) {
        PRINT_DEBUG("relay payload parse failed");
        return;
    }

    InboundPacket inbound{};
    inbound.message = std::move(msg);
    inbound.ip_port.ip = htonl(env.source_virtual_ip);
    inbound.ip_port.port = htons(env.source_virtual_port);
    inbound.reliable = reliable;
    inbound_packets.push(std::move(inbound));
}

void Relay_Transport::read_tcp_locked()
{
    if (!tcp_connected || !relay_socket_valid(tcp_socket)) return;

    char buffer[4096];
    while (true) {
        int received = recv(tcp_socket, buffer, sizeof(buffer), 0);
        if (received > 0) {
            tcp_recv_buffer.insert(tcp_recv_buffer.end(), buffer, buffer + received);
            continue;
        }

        if (received == 0) {
            PRINT_DEBUG("relay tcp closed");
            schedule_reconnect_locked();
            return;
        }

        if (!relay_last_error_is_would_block()) {
            PRINT_DEBUG("relay tcp recv failed");
            schedule_reconnect_locked();
        }
        break;
    }

    while (tcp_recv_buffer.size() >= sizeof(uint32)) {
        size_t offset = 0;
        uint32 frame_len = 0;
        if (!read_u32(tcp_recv_buffer, offset, frame_len)) break;
        if (tcp_recv_buffer.size() < sizeof(uint32) + frame_len) break;

        std::vector<char> frame(
            tcp_recv_buffer.begin() + static_cast<std::ptrdiff_t>(sizeof(uint32)),
            tcp_recv_buffer.begin() + static_cast<std::ptrdiff_t>(sizeof(uint32) + frame_len)
        );
        tcp_recv_buffer.erase(
            tcp_recv_buffer.begin(),
            tcp_recv_buffer.begin() + static_cast<std::ptrdiff_t>(sizeof(uint32) + frame_len)
        );
        handle_incoming_locked(frame, true);
    }
}

void Relay_Transport::read_udp_locked()
{
    if (!welcomed || !relay_socket_valid(udp_socket)) return;

    char buffer[20480];
    while (true) {
        int received = recv(udp_socket, buffer, sizeof(buffer), 0);
        if (received > 0) {
            std::vector<char> packet(buffer, buffer + received);
            handle_incoming_locked(packet, false);
            continue;
        }

        if (received == 0) return;
        if (!relay_last_error_is_would_block()) {
            PRINT_DEBUG("relay udp recv failed");
            schedule_reconnect_locked();
        }
        return;
    }
}

std::vector<char> Relay_Transport::encode_registration_payload_locked() const
{
    return serialize_ids_payload(local_ids, listen_port);
}

void Relay_Transport::send_registration_locked(bool hello)
{
    if (!tcp_connected) return;
    std::vector<char> payload = encode_registration_payload_locked();
    uint16 type = hello ? RELAY_MSG_HELLO : RELAY_MSG_REGISTER;
    uint64 source_id = local_ids.empty() ? 0 : local_ids.front().ConvertToUint64();
    bool queued = queue_tcp_message_locked(type, 0, source_id, 0, 0, 0, payload);
    PRINT_DEBUG("relay %s queued=%u appid=%u source_id=%llu ids=%zu listen_port=%u", relay_msg_name(type), queued ? 1u : 0u, appid, static_cast<unsigned long long>(source_id), local_ids.size(), listen_port);
    if (hello) {
        hello_sent = true;
    } else {
        registration_dirty = false;
    }
}

bool Relay_Transport::send_udp_message_locked(uint16 type, uint32 flags, uint64 source_id, uint64 dest_id, uint32 dest_ip, uint16 dest_port, const std::vector<char> &payload)
{
    if (!welcomed || !relay_socket_valid(udp_socket)) {
        PRINT_DEBUG("relay udp send skipped type=%s welcomed=%u udp_valid=%u", relay_msg_name(type), welcomed ? 1u : 0u, relay_socket_valid(udp_socket) ? 1u : 0u);
        return false;
    }

    RelayEnvelope env{};
    env.type = type;
    env.flags = flags;
    env.appid = appid;
    env.source_id = source_id;
    env.dest_id = dest_id;
    env.source_virtual_ip = assigned_virtual_ip;
    env.source_virtual_port = assigned_virtual_port;
    env.dest_virtual_ip = dest_ip;
    env.dest_virtual_port = dest_port;
    env.session_token = session_token;
    env.payload = payload;

    std::vector<char> bytes = serialize_envelope(env);
    int sent = send(udp_socket, bytes.data(), static_cast<int>(bytes.size()), 0);
    if (sent == static_cast<int>(bytes.size())) {
        PRINT_DEBUG("relay udp sent type=%s bytes=%d flags=%u dest_id=%llu dest_ip=%u dest_port=%u", relay_msg_name(type), sent, flags, static_cast<unsigned long long>(dest_id), dest_ip, dest_port);
        return true;
    }
    if (sent >= 0) {
        PRINT_DEBUG("relay udp partial send type=%s sent=%d expected=%zu", relay_msg_name(type), sent, bytes.size());
        return false;
    }
    if (!relay_last_error_is_would_block()) {
        PRINT_DEBUG("relay udp send failed");
        schedule_reconnect_locked();
    }
    return false;
}

bool Relay_Transport::queue_tcp_message_locked(uint16 type, uint32 flags, uint64 source_id, uint64 dest_id, uint32 dest_ip, uint16 dest_port, const std::vector<char> &payload)
{
    if (!tcp_connected || !relay_socket_valid(tcp_socket)) {
        PRINT_DEBUG("relay tcp queue skipped type=%s connected=%u tcp_valid=%u", relay_msg_name(type), tcp_connected ? 1u : 0u, relay_socket_valid(tcp_socket) ? 1u : 0u);
        return false;
    }

    RelayEnvelope env{};
    env.type = type;
    env.flags = flags;
    env.appid = appid;
    env.source_id = source_id;
    env.dest_id = dest_id;
    env.source_virtual_ip = assigned_virtual_ip;
    env.source_virtual_port = assigned_virtual_port;
    env.dest_virtual_ip = dest_ip;
    env.dest_virtual_port = dest_port;
    env.payload = payload;

    std::vector<char> bytes = serialize_envelope(env);
    append_u32(tcp_send_buffer, static_cast<uint32>(bytes.size()));
    tcp_send_buffer.insert(tcp_send_buffer.end(), bytes.begin(), bytes.end());
    PRINT_DEBUG("relay tcp queued type=%s payload_bytes=%zu frame_bytes=%zu pending=%zu flags=%u dest_id=%llu dest_ip=%u dest_port=%u", relay_msg_name(type), payload.size(), bytes.size(), tcp_send_buffer.size(), flags, static_cast<unsigned long long>(dest_id), dest_ip, dest_port);
    return true;
}

void Relay_Transport::send_periodic_heartbeats_locked()
{
    auto now = std::chrono::steady_clock::now();
    if (tcp_connected && std::chrono::duration_cast<std::chrono::seconds>(now - last_tcp_heartbeat).count() >= 10) {
        queue_tcp_message_locked(RELAY_MSG_HEARTBEAT, 0, local_ids.empty() ? 0 : local_ids.front().ConvertToUint64(), 0, 0, 0, {});
        last_tcp_heartbeat = now;
    }

    if (welcomed && std::chrono::duration_cast<std::chrono::seconds>(now - last_udp_heartbeat).count() >= 5) {
        send_udp_message_locked(RELAY_MSG_HEARTBEAT, 0, local_ids.empty() ? 0 : local_ids.front().ConvertToUint64(), 0, 0, 0, {});
        last_udp_heartbeat = now;
    }
}

void Relay_Transport::Run()
{
    std::lock_guard<std::recursive_mutex> lock(mutex);
    if (!enabled()) return;

    ensure_connected_locked();
    if (tcp_connecting) {
        finish_tcp_connect_locked();
    }
    if (!tcp_connected) return;

    if (!hello_sent) {
        send_registration_locked(true);
    }
    if (welcomed && registration_dirty) {
        send_registration_locked(false);
    }

    flush_tcp_send_locked();
    read_tcp_locked();
    read_udp_locked();
    send_periodic_heartbeats_locked();
    flush_tcp_send_locked();
}

void Relay_Transport::set_appid(uint32 next_appid)
{
    std::lock_guard<std::recursive_mutex> lock(mutex);
    if (appid == next_appid) return;
    appid = next_appid;
    schedule_reconnect_locked(std::chrono::seconds(0));
}

void Relay_Transport::add_listen_id(CSteamID id)
{
    std::lock_guard<std::recursive_mutex> lock(mutex);
    if (!id.IsValid()) return;
    auto found = std::find(local_ids.begin(), local_ids.end(), id);
    if (found != local_ids.end()) return;
    local_ids.push_back(id);
    registration_dirty = true;
}

bool Relay_Transport::Send(Common_Message *msg, bool reliable)
{
    if (!msg) return false;
    std::lock_guard<std::recursive_mutex> lock(mutex);
    if (!welcomed) {
        PRINT_DEBUG("relay send skipped reliable=%u because welcome not received yet", reliable ? 1u : 0u);
        return false;
    }

    size_t size = msg->ByteSizeLong();
    std::vector<char> payload(size);
    msg->SerializeToArray(payload.data(), static_cast<int>(payload.size()));

    uint32 flags = 0;
    uint64 dest_id = msg->dest_id();
    if (dest_id != 0) flags |= RELAY_FLAG_HAS_DEST_STEAMID;
    uint64 source_id = msg->source_id();

    if (reliable) {
        return queue_tcp_message_locked(RELAY_MSG_RELIABLE, flags, source_id, dest_id, 0, 0, payload);
    }
    return send_udp_message_locked(RELAY_MSG_UNRELIABLE, flags, source_id, dest_id, 0, 0, payload);
}

bool Relay_Transport::SendToEndpoint(Common_Message *msg, uint32 ip, uint16 port, bool reliable)
{
    if (!msg) return false;
    std::lock_guard<std::recursive_mutex> lock(mutex);
    if (!welcomed) {
        PRINT_DEBUG("relay send-to-endpoint skipped reliable=%u ip=%u port=%u because welcome not received yet", reliable ? 1u : 0u, ip, port);
        return false;
    }

    size_t size = msg->ByteSizeLong();
    std::vector<char> payload(size);
    msg->SerializeToArray(payload.data(), static_cast<int>(payload.size()));

    uint32 flags = RELAY_FLAG_HAS_DEST_ENDPOINT;
    uint64 source_id = msg->source_id();
    if (msg->dest_id() != 0) flags |= RELAY_FLAG_HAS_DEST_STEAMID;

    if (reliable) {
        return queue_tcp_message_locked(RELAY_MSG_RELIABLE, flags, source_id, msg->dest_id(), ip, port, payload);
    }
    return send_udp_message_locked(RELAY_MSG_UNRELIABLE, flags, source_id, msg->dest_id(), ip, port, payload);
}

bool Relay_Transport::SendBroadcast(Common_Message *msg)
{
    if (!msg) return false;
    std::lock_guard<std::recursive_mutex> lock(mutex);
    if (!welcomed) {
        PRINT_DEBUG("relay broadcast skipped because welcome not received yet");
        return false;
    }

    size_t size = msg->ByteSizeLong();
    std::vector<char> payload(size);
    msg->SerializeToArray(payload.data(), static_cast<int>(payload.size()));
    return send_udp_message_locked(RELAY_MSG_UNRELIABLE, RELAY_FLAG_BROADCAST, msg->source_id(), 0, 0, 0, payload);
}

bool Relay_Transport::PollPacket(Common_Message *msg, IP_PORT *ip_port, bool *reliable)
{
    std::lock_guard<std::recursive_mutex> lock(mutex);
    if (inbound_packets.empty()) return false;
    InboundPacket packet = std::move(inbound_packets.front());
    inbound_packets.pop();
    if (msg) *msg = std::move(packet.message);
    if (ip_port) *ip_port = packet.ip_port;
    if (reliable) *reliable = packet.reliable;
    return true;
}

bool Relay_Transport::PollDisconnect(std::vector<CSteamID> &ids, uint32 &virtual_ip, uint16 &virtual_port)
{
    std::lock_guard<std::recursive_mutex> lock(mutex);
    if (disconnect_events.empty()) return false;
    DisconnectEvent event = std::move(disconnect_events.front());
    disconnect_events.pop();
    ids = std::move(event.ids);
    virtual_ip = event.virtual_ip;
    virtual_port = event.virtual_port;
    return true;
}

uint32 Relay_Transport::virtual_ip()
{
    std::lock_guard<std::recursive_mutex> lock(mutex);
    return assigned_virtual_ip;
}

uint16 Relay_Transport::virtual_port()
{
    std::lock_guard<std::recursive_mutex> lock(mutex);
    return assigned_virtual_port;
}
