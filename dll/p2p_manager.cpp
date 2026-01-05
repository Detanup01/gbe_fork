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

#include "dll/p2p_manager.hpp"
#include "dll/dll.h"


//kingdom 2 crowns doesn't work with a 0.3 delay or lower
// appid 353090 becomes unstable when joining a lobby if the time is too low,
// it takes between ~5-12 seconds to get past the message "Waiting for our local client to connect..." !!!
constexpr static double SESSION_REQUEST_DELAY = 2.0;

// https://partner.steamgames.com/doc/api/ISteamNetworking#SendP2PPacket
// "if we can't get through to the user after a timeout of 20 seconds, then an error will be posted"
constexpr static double SESSION_REQUEST_TIMEOUT = 20.0;

constexpr static double PACKET_MAX_TIME_TO_LIVE = 20.0;



const std::chrono::high_resolution_clock::time_point& P2p_Manager::Packet_t::get_time_created() const
{
    return time_created;
}


void P2p_Manager::steam_networking_callback(void *object, Common_Message *msg)
{
    // PRINT_DEBUG_ENTRY();

    auto *obj = (P2p_Manager *)object;
    obj->network_callback(msg);
}

void P2p_Manager::steam_run_every_runcb(void *object)
{
    // PRINT_DEBUG_ENTRY();

    auto *obj = (P2p_Manager *)object;
    obj->periodic_callback();
}




SteamCallBacks* P2p_Manager::get_my_callbacks(const CSteamID &my_id) const
{
    const CSteamID our_id_client = settings_client->get_local_steam_id();
    const CSteamID our_id_server = settings_server->get_local_steam_id();

    SteamCallBacks *callbacks = nullptr;
    if (my_id == our_id_client) {
        PRINT_DEBUG("  using our client callbacks");
        return callbacks_client;
    } else if (my_id == our_id_server) {
        PRINT_DEBUG("  using our server callbacks");
        return callbacks_server;
    }

    PRINT_DEBUG("[X] Id=[%llu] is not ours!", my_id.ConvertToUint64());
    return nullptr;
}

bool P2p_Manager::is_same_peer(const CSteamID &id, const CSteamID &peer_id) const
{
    if (id == peer_id) {
        return true;
    }

    return false;
}

void P2p_Manager::send_peer_session_failure(const Peer_Src_t &peer_conn)
{
    Common_Message update_msg{};
    update_msg.set_source_id(peer_conn.my_dest_id.ConvertToUint64());
    update_msg.set_dest_id(peer_conn.remote_id.ConvertToUint64());
    update_msg.set_allocated_network(new Network_pb);

    update_msg.mutable_network()->set_type(Network_pb::FAILED_CONNECT);

    PRINT_DEBUG(
        "sent a connection failue packet, src id (our client/gameserver)=[%llu], dest id (peer)=[%llu]",
        (uint64)update_msg.source_id(), (uint64)update_msg.dest_id()
    );
    network->sendTo(&update_msg, true);
}

void P2p_Manager::trigger_session_request(const CSteamID &remote_id, const CSteamID &my_id)
{
    PRINT_DEBUG(
        "triggering session request callback for source steamid=[%llu], I am=[%llu]",
        remote_id.ConvertToUint64(), my_id.ConvertToUint64()
    );
    P2PSessionRequest_t data{};
    data.m_steamIDRemote = remote_id;

    get_my_callbacks(my_id)->addCBResult(data.k_iCallback, &data, sizeof(data), SESSION_REQUEST_DELAY);
}





bool P2p_Manager::remove_connection(const CSteamID &remote_id, const CSteamID &my_id)
{
    auto rem_it = std::remove_if(connections.begin(), connections.end(), [&remote_id, &my_id, this](const Connection_t &item){
        return is_same_peer(remote_id, item.peer_conn.remote_id)
            && is_same_peer(my_id, item.peer_conn.my_dest_id)
            ;
    });

    if (connections.end() != rem_it) {
        connections.erase(rem_it, connections.end());
        return true;
    }

    return false;
}

P2p_Manager::Connection_t* P2p_Manager::create_connection(const CSteamID &remote_id, const CSteamID &my_id)
{
    auto conn = get_connection(remote_id, my_id);
    if (conn) {
        return conn;
    }

    Connection_t connection{};
    connection.peer_conn.remote_id = remote_id;
    connection.peer_conn.my_dest_id = my_id;

    auto &conn_ref = connections.emplace_back(std::move(connection));
    PRINT_DEBUG(
        "created for/them=[%llu], from/me=[%llu]",
        conn_ref.peer_conn.remote_id.ConvertToUint64(), conn_ref.peer_conn.my_dest_id.ConvertToUint64()
    );
    return &conn_ref;
}

P2p_Manager::Connection_t* P2p_Manager::get_connection(const CSteamID &remote_id, const CSteamID &my_id)
{
    auto conn = std::find_if(connections.begin(), connections.end(), [&remote_id, &my_id, this](const Connection_t &item) {
        return is_same_peer(remote_id, item.peer_conn.remote_id)
            && is_same_peer(my_id, item.peer_conn.my_dest_id)
            ;
    });

    if (connections.end() == conn) {
        return nullptr;
    }

    return &(*conn);
}





P2p_Manager::P2p_Manager(
    class Settings *settings_client, class Settings *settings_server,
    class SteamCallBacks *callbacks_client, class SteamCallBacks *callbacks_server,
    class Networking *network, class RunEveryRunCB *run_every_runcb
)
{
    this->settings_client = settings_client;
    this->settings_server = settings_server;

    this->callbacks_client = callbacks_client;
    this->callbacks_server = callbacks_server;

    this->network = network;
    this->run_every_runcb = run_every_runcb;

    for (auto settings : { settings_client, settings_server }) {
        network->setCallback(CALLBACK_ID_NETWORKING, settings->get_local_steam_id(), &P2p_Manager::steam_networking_callback, this);
        network->setCallback(CALLBACK_ID_USER_STATUS, settings->get_local_steam_id(), &P2p_Manager::steam_networking_callback, this);
    }
    
    run_every_runcb->add(&P2p_Manager::steam_run_every_runcb, this);
}

P2p_Manager::~P2p_Manager()
{
    for (auto settings : { settings_client, settings_server }) {
        network->rmCallback(CALLBACK_ID_NETWORKING, settings->get_local_steam_id(), &P2p_Manager::steam_networking_callback, this);
        network->rmCallback(CALLBACK_ID_USER_STATUS, settings->get_local_steam_id(), &P2p_Manager::steam_networking_callback, this);
    }

    run_every_runcb->remove(&P2p_Manager::steam_run_every_runcb, this);
}


bool P2p_Manager::store_packet(
    CSteamID my_id, CSteamID steamIDRemote,
    const void *pubData, uint32 cubData, int nChannel,
    EP2PSend send_type
)
{
    std::lock_guard lock(p2p_mtx);

    auto conn = create_connection(steamIDRemote, my_id);

    {
        Packet_t channel_msg{};
        channel_msg.is_processed = conn->is_accepted;
        channel_msg.send_type = send_type;
        if (pubData && cubData > 0) {
            channel_msg.data.assign((const char *)pubData, (const char *)pubData + cubData);
        }

        auto &channel = conn->channels[nChannel];
        channel.packets.emplace_back(std::move(channel_msg));
    }

    PRINT_DEBUG(
        "stored msg, size=[%u], from=[%llu] (connection accepted=%i), channel=[%i]",
        cubData, steamIDRemote.ConvertToUint64(), (int)conn->is_accepted, nChannel
    );

    return conn->is_accepted;
}

std::optional<std::tuple<
    decltype(P2p_Manager::connections)::iterator,
    decltype(P2p_Manager::Connection_t::channels)::iterator,
    decltype(P2p_Manager::Channel_t::packets)::iterator
>> P2p_Manager::get_next_packet(CSteamID my_id, int nChannel)
{
    const auto my_settings = is_same_peer(my_id, settings_client->get_local_steam_id())
        ? settings_client
        : settings_server;

    for (auto conn_it = connections.begin(); connections.end() != conn_it; ++conn_it) {
        if (!conn_it->is_accepted) {
            continue;
        }

        auto ch_it = conn_it->channels.find(nChannel);
        // channel doesn't exist for this connection
        if (conn_it->channels.end() == ch_it) {
            continue;
        }

        auto &packets = ch_it->second.packets;
        auto packet_it = packets.begin();
        // channel has no packets
        if (packets.end() == packet_it) {
            continue;
        }

        // no need to check next packets in this channel if the first one isn't processed yet
        // once the connection is accepted, all messages in all channels will be marked  as processed
        if (!packet_it->is_processed) {
            continue;
        }

        const bool is_packet_for_me = is_same_peer(my_id, conn_it->peer_conn.my_dest_id);
        if (!is_packet_for_me) {
            bool can_share_packet = false;

            switch (my_settings->old_p2p_behavior.mode) {
            default:
            case OldP2pBehavior::EPacketShareMode::NEVER_SHARE: {
                can_share_packet = false;
            }

            case OldP2pBehavior::EPacketShareMode::ALWAYS_SHARE: {
                can_share_packet = true;
            }
            break;

            case OldP2pBehavior::EPacketShareMode::MIXED: {
                // appids (353090, 301300) do this:
                // - send packet from client >>> to gameserver
                // - use the **client** to check for these packets
                // it should use the gameserver instead
                // but the client is expected to return these packets to the game
                // this seems to be an old behavior
                //
                // on the contrary appids (701160, 248390) use k_EP2PSendReliable
                // and they do not need the gameserver to share its packets with the client
                // even multiplayer in appid 248390 won't work if packets were shared
                can_share_packet =
                    EP2PSend::k_EP2PSendUnreliable == packet_it->send_type ||
                    EP2PSend::k_EP2PSendUnreliableNoDelay == packet_it->send_type;
            }
            break;
            }

            // avoid sharing packets between client and gameserver
            if (!can_share_packet) {
                continue;
            }
        }

        return std::make_tuple(conn_it, ch_it, packet_it);
    }

    return {};
}


bool P2p_Manager::send_packet(CSteamID my_id, CSteamID steamIDRemote, const void *pubData, uint32 cubData, EP2PSend eP2PSendType, int nChannel)
{
    bool reliable = false;
    if (eP2PSendType == EP2PSend::k_EP2PSendReliable || eP2PSendType == EP2PSend::k_EP2PSendReliableWithBuffering) {
        reliable = true;
    }

    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // don't lock 2 or more mutexes at the same time
    // to avoid the problem of lock ordering deadlock
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    {
        std::lock_guard lock(p2p_mtx);

        auto conn = create_connection(steamIDRemote, my_id);
        if (!conn->is_accepted) {
            // https://partner.steamgames.com/doc/api/ISteamNetworking#AcceptP2PSessionWithUser
            // "If you've called SendP2PPacket on the other user, this implicitly accepts the session request"
            conn->is_accepted = true;
            PRINT_DEBUG(
                "auto-accepting remote connections from=[%llu], I am=[%llu]",
                conn->peer_conn.remote_id.ConvertToUint64(), conn->peer_conn.my_dest_id.ConvertToUint64()
            );
        }
    }

    Common_Message msg{};
    msg.set_source_id(my_id.ConvertToUint64());
    msg.set_dest_id(steamIDRemote.ConvertToUint64());
    msg.set_allocated_network(new Network_pb);

    msg.mutable_network()->set_type(Network_pb::DATA);
    msg.mutable_network()->set_channel(nChannel);
    msg.mutable_network()->set_data(pubData, cubData);
    msg.mutable_network()->set_send_type(eP2PSendType);

    bool ret = false;
    {
        std::lock_guard lock(global_mutex);

        ret = network->sendTo(&msg, reliable);
        PRINT_DEBUG(
            "Sent remote message with size=[%zu] from=[%llu] to=[%llu], is_ok=%u",
            msg.network().data().size(), (uint64)msg.source_id(), (uint64)msg.dest_id(), ret
        );
    }

    return ret;
}

bool P2p_Manager::is_packet_available(CSteamID my_id, uint32 *pcubMsgSize, int nChannel)
{
    std::lock_guard lock(p2p_mtx);

    const CSteamID our_id_client = settings_client->get_local_steam_id();
    const CSteamID our_id_server = settings_server->get_local_steam_id();

    if (pcubMsgSize) *pcubMsgSize = 0;

    auto packet_opt = get_next_packet(my_id, nChannel);
    if (!packet_opt) {
        return false;
    }

    auto &[conn_it, ch_it, packet_it] = packet_opt.value();
    uint32 size = static_cast<uint32>(packet_it->data.size());
    if (pcubMsgSize) {
        *pcubMsgSize = size;
    }
    PRINT_DEBUG(
        "  available message from=[%llu], size=[%u]",
        conn_it->peer_conn.remote_id.ConvertToUint64(), size
    );
    
    return true;
}

bool P2p_Manager::read_packet(CSteamID my_id, void *pubDest, uint32 cubDest, uint32 *pcubMsgSize, CSteamID *psteamIDRemote, int nChannel)
{
    std::lock_guard lock(p2p_mtx);

    if (pcubMsgSize) *pcubMsgSize = 0;
    if (psteamIDRemote) *psteamIDRemote = k_steamIDNil;

    auto packet_opt = get_next_packet(my_id, nChannel);
    if (!packet_opt) {
        return false;
    }

    auto &[conn_it, ch_it, packet_it] = packet_opt.value();
    if (psteamIDRemote) {
        *psteamIDRemote = conn_it->peer_conn.remote_id;
    }

    uint32 size = static_cast<uint32>(packet_it->data.size());
    if (cubDest < size) {
        // https://partner.steamgames.com/doc/api/ISteamNetworking#ReadP2PPacket
        // "If the cubDest buffer is too small for the packet, then the message will be truncated"
        size = cubDest;
    }

    if (pcubMsgSize) {
        *pcubMsgSize = size;
    }

    if (pubDest) {
        memcpy(pubDest, packet_it->data.data(), size);
    }

    ch_it->second.packets.erase(packet_it);

    PRINT_DEBUG(
        "  copied message from=[%llu], size=[%u]",
        conn_it->peer_conn.remote_id.ConvertToUint64(), size
    );

    return true;
}

bool P2p_Manager::close_channel(CSteamID my_id, CSteamID steamIDRemote, int nChannel)
{
    std::lock_guard lock(p2p_mtx);

    auto conn = get_connection(steamIDRemote, my_id);
    if (!conn) {
        PRINT_DEBUG(
            "[X] no connection to remote user [%llu] was found, I am [%llu]",
            steamIDRemote.ConvertToUint64(), my_id.ConvertToUint64()
        );
        return false;
    }

    conn->channels.erase(nChannel);
    PRINT_DEBUG(
        "closed channel [%i] with remote user [%llu], I am [%llu]",
        nChannel, steamIDRemote.ConvertToUint64(), my_id.ConvertToUint64()
    );

    if (conn->channels.empty()) {
        // https://partner.steamgames.com/doc/api/ISteamNetworking#CloseP2PChannelWithUser
        // "Once all channels to a user have been closed,"
        // "the open session to the user will be closed and new data from this user will trigger a new P2PSessionRequest_t callback."
        PRINT_DEBUG(
            "[?] all channels with remote user [%llu] are closed, removing connection, I am [%llu]",
            steamIDRemote.ConvertToUint64(), my_id.ConvertToUint64()
        );
        remove_connection(steamIDRemote, my_id);
        remove_connection(my_id, steamIDRemote);
    }

    return true;
}

bool P2p_Manager::close_session(CSteamID my_id, CSteamID steamIDRemote)
{
    std::lock_guard lock(p2p_mtx);

    const bool res_1 = remove_connection(steamIDRemote, my_id);
    const bool res_2 = remove_connection(my_id, steamIDRemote);
    return res_1 || res_2;
}

bool P2p_Manager::get_session_state(CSteamID my_id, CSteamID steamIDRemote, P2PSessionState_t *pConnectionState)
{
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    // don't lock 2 or more mutexes at the same time
    // to avoid the problem of lock ordering deadlock
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

    {
        std::lock_guard lock(p2p_mtx);

        auto conn = get_connection(steamIDRemote, my_id);
        if (!conn) {
            if (pConnectionState) {
                pConnectionState->m_bConnectionActive = false;
                pConnectionState->m_bConnecting = false;
                pConnectionState->m_eP2PSessionError = EP2PSessionError::k_EP2PSessionErrorTimeout;
                pConnectionState->m_bUsingRelay = false;
                pConnectionState->m_nBytesQueuedForSend = 0;
                pConnectionState->m_nPacketsQueuedForSend = 0;
                pConnectionState->m_nRemoteIP = 0;
                pConnectionState->m_nRemotePort = 0;
            }

            PRINT_DEBUG(
                "no connection to remote user [%llu], I am [%llu]",
                steamIDRemote.ConvertToUint64(), my_id.ConvertToUint64()
            );
            return false;
        }

        if (pConnectionState) {
            int32 pending_packets = 0;
            int32 pending_bytes = 0;
            for (const auto& [ch_idx, channel] : conn->channels) {
                pending_packets += (int32)channel.packets.size();
                for (const auto &msg : channel.packets) {
                    pending_bytes += (int32)msg.data.size();
                }
            }

            pConnectionState->m_bConnectionActive = conn->is_accepted;
            pConnectionState->m_bConnecting = !conn->is_accepted;
            pConnectionState->m_eP2PSessionError = EP2PSessionError::k_EP2PSessionErrorNone;
            pConnectionState->m_bUsingRelay = false; // TODO
            pConnectionState->m_nPacketsQueuedForSend = pending_packets;
            pConnectionState->m_nBytesQueuedForSend = pending_bytes;
        }
    }

    if (pConnectionState) {
        std::lock_guard lock(global_mutex);

        pConnectionState->m_nRemoteIP = network->getIP(steamIDRemote);
        pConnectionState->m_nRemotePort = network->getPort(steamIDRemote);
    }

    PRINT_DEBUG(
        "remote user [%llu] has a session/connection, I am [%llu]",
        steamIDRemote.ConvertToUint64(), my_id.ConvertToUint64()
    );
    return true;
}

bool P2p_Manager::accept_session(CSteamID my_id, CSteamID steamIDRemote)
{
    std::lock_guard lock(p2p_mtx);

    auto conn = get_connection(steamIDRemote, my_id);
    if (!conn) {
        PRINT_DEBUG("[X] no connection from=[%llu], I am=[%llu]", steamIDRemote.ConvertToUint64(), my_id.ConvertToUint64());
        return false;
    }

    if (!conn->is_accepted) {
        conn->is_accepted = true;
        PRINT_DEBUG("accepted new session from=[%llu], I am=[%llu]", steamIDRemote.ConvertToUint64(), my_id.ConvertToUint64());    
    }
    return true;
}




void P2p_Manager::periodic_handle_connections(const std::chrono::high_resolution_clock::time_point &now)
{
    auto conn_it = connections.begin();
    while (connections.end() != conn_it) {
        bool is_remove = false;
        if (!conn_it->is_accepted) {
            if (check_timedout(conn_it->time_added, SESSION_REQUEST_TIMEOUT, now)) {
                is_remove = true;
                send_peer_session_failure(conn_it->peer_conn);
            }
        }

        if (is_remove) {
            conn_it = connections.erase(conn_it);
        } else {
            ++conn_it;
        }
    }
}

void P2p_Manager::periodic_handle_channels(const std::chrono::high_resolution_clock::time_point &now)
{
    for (auto &conn : connections) {
        if (!conn.is_accepted) {
            continue;
        }

        // remove channels with no packets
        auto ch_it = conn.channels.begin();
        while (conn.channels.end() != ch_it) {
            auto &channel = ch_it->second;
            if (channel.packets.empty()) {
                ch_it = conn.channels.erase(ch_it);
            } else {
                ++ch_it;
            }
        }
    }
}

void P2p_Manager::periodic_handle_packets(const std::chrono::high_resolution_clock::time_point &now)
{
    for (auto &conn : connections) {
        if (!conn.is_accepted) {
            continue;
        }

        for (auto &[ch_num, channel] : conn.channels) {
            // remove outadated packets, and mark the rest as processed
            auto packet_it = channel.packets.begin();
            while (channel.packets.end() != packet_it) {
                bool is_remove = false;
                if (check_timedout(packet_it->get_time_created(), PACKET_MAX_TIME_TO_LIVE, now)) {
                    is_remove = true;
                } else {
                    packet_it->is_processed = true;
                }

                if (is_remove) {
                    packet_it = channel.packets.erase(packet_it);
                } else {
                    ++packet_it;
                }
            }
        }
    }
}

void P2p_Manager::periodic_callback()
{
    std::lock_guard lock(p2p_mtx);

    auto now = std::chrono::high_resolution_clock::now();
    periodic_handle_connections(now);
    periodic_handle_channels(now);
    periodic_handle_packets(now);
}




void P2p_Manager::network_data_packets(Common_Message *msg)
{
    const CSteamID src_id = (uint64)msg->source_id();
    const CSteamID my_dest_id = (uint64)msg->dest_id(); // this is us

    PRINT_DEBUG("got network msg from [%llu], I am [%llu], type <%u>",
        src_id.ConvertToUint64(), my_dest_id.ConvertToUint64(), msg->network().type()
    );

    switch (msg->network().type()) {
    case Network_pb::DATA: {
        PRINT_DEBUG("got network data message");
        const bool conn_is_accepted = store_packet(
            my_dest_id, src_id,
            msg->network().data().c_str(), (uint32)msg->network().data().size(),
            (int)msg->network().channel(),
            (EP2PSend)msg->network().send_type()
        );
        if (!conn_is_accepted) {
            trigger_session_request(src_id, my_dest_id);
        }
    }
    break;

    case Network_pb::FAILED_CONNECT: {
        PRINT_DEBUG("[X] got connection failure packet");
        P2PSessionConnectFail_t data{};
        data.m_steamIDRemote = src_id;
        data.m_eP2PSessionError = EP2PSessionError::k_EP2PSessionErrorTimeout;

        get_my_callbacks(my_dest_id)->addCBResult(data.k_iCallback, &data, sizeof(data));

        {
            std::lock_guard lock(p2p_mtx);

            remove_connection(src_id, my_dest_id);
            remove_connection(my_dest_id, src_id);
        }
    }
    break;
    }
}

void P2p_Manager::network_low_level(Common_Message *msg)
{
    const CSteamID src_id = (uint64)msg->source_id();
    const CSteamID my_dest_id = (uint64)msg->dest_id(); // this is us

    switch (msg->low_level().type()) {
        case Low_Level::CONNECT: {

        }
        break;

        case Low_Level::DISCONNECT: {
            bool any_conn_removed = false;
            {
                std::lock_guard lock(p2p_mtx);

                any_conn_removed |= remove_connection(src_id, my_dest_id);
                any_conn_removed |= remove_connection(my_dest_id, src_id);
            }

            if (any_conn_removed) {
                PRINT_DEBUG(
                    "[X] remote user [%llu] disconnected, sending P2PSessionConnectFail_t, I am [%llu]",
                    src_id.ConvertToUint64(), my_dest_id.ConvertToUint64()
                );
                P2PSessionConnectFail_t data{};
                data.m_steamIDRemote = src_id;
                data.m_eP2PSessionError = k_EP2PSessionErrorDestinationNotLoggedIn;

                get_my_callbacks(my_dest_id)->addCBResult(data.k_iCallback, &data, sizeof(data));
            }

        }
        break;
    }
}

void P2p_Manager::network_callback(Common_Message *msg)
{
    if (msg->has_network()) {
        network_data_packets(msg);
    }
    
    if (msg->has_low_level()) {
        network_low_level(msg);
    }
}
