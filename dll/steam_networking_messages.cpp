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

#include "dll/steam_networking_messages.h"


#define NETWORKING_MESSAGES_TIMEOUT 30.0


void Steam_Networking_Messages::steam_callback(void *object, Common_Message *msg)
{
    // PRINT_DEBUG_ENTRY();

    Steam_Networking_Messages *steam_networking_messages = (Steam_Networking_Messages *)object;
    steam_networking_messages->Callback(msg);
}

void Steam_Networking_Messages::steam_run_every_runcb(void *object)
{
    // PRINT_DEBUG_ENTRY();

    Steam_Networking_Messages *steam_networking_messages = (Steam_Networking_Messages *)object;
    steam_networking_messages->RunCallbacks();
}

void Steam_Networking_Messages::free_steam_message_data(SteamNetworkingMessage_t *pMsg)
{
    free(pMsg->m_pData);
    pMsg->m_pData = NULL;
}

void Steam_Networking_Messages::delete_steam_message(SteamNetworkingMessage_t *pMsg)
{
    if (pMsg->m_pfnFreeData) pMsg->m_pfnFreeData(pMsg);
    delete pMsg;
}

void Steam_Networking_Messages::end_connection(CSteamID steam_id)
{
    auto conn = connections.find(steam_id);
    if (conn != connections.end()) {
        conn->second.dead = true;
    }
}

std::map<CSteamID, Steam_Message_Connection>::iterator Steam_Networking_Messages::find_or_create_message_connection(SteamNetworkingIdentity identityRemote, bool incoming, bool restartbroken)
{
    auto conn = connections.find(identityRemote.GetSteamID());
    if (conn == connections.end() || (conn->second.dead && restartbroken)) {
        ++id_counter;
        struct Steam_Message_Connection con;
        con.remote_identity = identityRemote;
        con.id = id_counter;
        connections[identityRemote.GetSteamID()] = con;

        Common_Message msg;
        msg.set_source_id(settings->get_local_steam_id().ConvertToUint64());
        msg.set_dest_id(con.remote_identity.GetSteamID64());
        msg.set_allocated_networking_messages(new Networking_Messages);
        if (incoming) {
            msg.mutable_networking_messages()->set_type(Networking_Messages::CONNECTION_ACCEPT);
        } else {
            msg.mutable_networking_messages()->set_type(Networking_Messages::CONNECTION_NEW);
        }
        msg.mutable_networking_messages()->set_channel(0);
        msg.mutable_networking_messages()->set_id_from(con.id);
        network->sendTo(&msg, true);

        conn = connections.find(identityRemote.GetSteamID());

        if (incoming) {
            SteamNetworkingMessagesSessionRequest_t data;
            data.m_identityRemote = con.remote_identity;
            callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
        }
    }

    if (!incoming) {
        conn->second.accepted = true;
    }

    return conn;
}


Steam_Networking_Messages::Steam_Networking_Messages(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb)
{
    this->settings = settings;
    this->network = network;
    this->callback_results = callback_results;
    this->callbacks = callbacks;
    this->run_every_runcb = run_every_runcb;

    this->created = std::chrono::steady_clock::now();

    this->network->setCallback(CALLBACK_ID_NETWORKING_MESSAGES, settings->get_local_steam_id(), &Steam_Networking_Messages::steam_callback, this);
    this->network->setCallback(CALLBACK_ID_USER_STATUS, settings->get_local_steam_id(), &Steam_Networking_Messages::steam_callback, this);
    this->run_every_runcb->add(&Steam_Networking_Messages::steam_run_every_runcb, this);
}

Steam_Networking_Messages::~Steam_Networking_Messages()
{
    this->network->rmCallback(CALLBACK_ID_NETWORKING_MESSAGES, settings->get_local_steam_id(), &Steam_Networking_Messages::steam_callback, this);
    this->network->rmCallback(CALLBACK_ID_USER_STATUS, settings->get_local_steam_id(), &Steam_Networking_Messages::steam_callback, this);
    this->run_every_runcb->remove(&Steam_Networking_Messages::steam_run_every_runcb, this);
}


/// Sends a message to the specified host.  If we don't already have a session with that user,
/// a session is implicitly created.  There might be some handshaking that needs to happen
/// before we can actually begin sending message data.  If this handshaking fails and we can't
/// get through, an error will be posted via the callback SteamNetworkingMessagesSessionFailed_t.
/// There is no notification when the operation succeeds.  (You should have the peer send a reply
/// for this purpose.)
///
/// Sending a message to a host will also implicitly accept any incoming connection from that host.
///
/// nSendFlags is a bitmask of k_nSteamNetworkingSend_xxx options
///
/// nRemoteChannel is a routing number you can use to help route message to different systems.
/// You'll have to call ReceiveMessagesOnChannel() with the same channel number in order to retrieve
/// the data on the other end.
///
/// Using different channels to talk to the same user will still use the same underlying
/// connection, saving on resources.  If you don't need this feature, use 0.
/// Otherwise, small integers are the most efficient.
///
/// It is guaranteed that reliable messages to the same host on the same channel
/// will be be received by the remote host (if they are received at all) exactly once,
/// and in the same order that they were send.
///
/// NO other order guarantees exist!  In particular, unreliable messages may be dropped,
/// received out of order with respect to each other and with respect to reliable data,
/// or may be received multiple times.  Messages on different channels are *not* guaranteed
/// to be received in the order they were sent.
///
/// A note for those familiar with TCP/IP ports, or converting an existing codebase that
/// opened multiple sockets:  You might notice that there is only one channel, and with
/// TCP/IP each endpoint has a port number.  You can think of the channel number as the
/// *destination* port.  If you need each message to also include a "source port" (so the
/// recipient can route the reply), then just put that in your message.  That is essentially
/// how UDP works!
///
/// Returns:
/// - k_EREsultOK on success.
/// - k_EResultNoConnection will be returned if the session has failed or was closed by the peer,
///   and k_nSteamNetworkingSend_AutoRestartBrokwnSession is not used.  (You can use
///   GetSessionConnectionInfo to get the details.)  In order to acknowledge the broken session
///   and start a new one, you must call CloseSessionWithUser
/// - See SendMessageToConnection::SendMessageToConnection for more
EResult Steam_Networking_Messages::SendMessageToUser( const SteamNetworkingIdentity &identityRemote, const void *pubData, uint32 cubData, int nSendFlags, int nRemoteChannel )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    const SteamNetworkingIPAddr *ip = identityRemote.GetIPAddr();
    bool reliable = false;
    if (nSendFlags & k_nSteamNetworkingSend_Reliable) {
        reliable = true;
    }

    bool restart_broken = false;
    if (nSendFlags & k_nSteamNetworkingSend_AutoRestartBrokenSession) {
        restart_broken = true;
    }

    if (identityRemote.m_eType == k_ESteamNetworkingIdentityType_SteamID) {
        PRINT_DEBUG("%llu", identityRemote.GetSteamID64());
        //steam id identity
    } else if (ip) {
        PRINT_DEBUG("%u:%u ipv4? %u", ip->GetIPv4(), ip->m_port, ip->IsIPv4());
        //ip addr
        return k_EResultNoConnection; //TODO
    } else {
        return k_EResultNoConnection;
    }

    auto conn = find_or_create_message_connection(identityRemote, false, restart_broken);
    if (conn->second.dead) {
        return k_EResultNoConnection;
    }

    Common_Message msg;
    msg.set_source_id(settings->get_local_steam_id().ConvertToUint64());
    msg.set_dest_id(conn->second.remote_identity.GetSteamID64());
    msg.set_allocated_networking_messages(new Networking_Messages);
    msg.mutable_networking_messages()->set_type(Networking_Messages::DATA);
    msg.mutable_networking_messages()->set_channel(nRemoteChannel);
    msg.mutable_networking_messages()->set_id_from(conn->second.id);
    msg.mutable_networking_messages()->set_data(pubData, cubData);

    network->sendTo(&msg, reliable);
    return k_EResultOK;
}

/// Reads the next message that has been sent from another user via SendMessageToUser() on the given channel.
/// Returns number of messages returned into your list.  (0 if no message are available on that channel.)
///
/// When you're done with the message object(s), make sure and call Release()!
int Steam_Networking_Messages::ReceiveMessagesOnChannel( int nLocalChannel, SteamNetworkingMessage_t **ppOutMessages, int nMaxMessages )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    int message_counter = 0;

    for (auto & conn : connections) {
        auto chan = conn.second.data.find(nLocalChannel);
        if (chan != conn.second.data.end()) {
            while (!chan->second.empty() && message_counter < nMaxMessages) {
                SteamNetworkingMessage_t *pMsg = new SteamNetworkingMessage_t(); //TODO size is wrong
                unsigned long size = static_cast<unsigned long>(chan->second.front().size());
                pMsg->m_pData = malloc(size);
                pMsg->m_cbSize = size;
                memcpy(pMsg->m_pData, chan->second.front().data(), size);
                pMsg->m_conn = conn.second.id;
                pMsg->m_identityPeer = conn.second.remote_identity;
                pMsg->m_nConnUserData = -1;
                pMsg->m_usecTimeReceived = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - created).count();
                //TODO: messagenumber?
                // pMsg->m_nMessageNumber = connect_socket->second.packet_receive_counter;
                // ++connect_socket->second.packet_receive_counter;

                pMsg->m_pfnFreeData = &free_steam_message_data;
                pMsg->m_pfnRelease = &delete_steam_message;
                pMsg->m_nChannel = nLocalChannel;
                ppOutMessages[message_counter] = pMsg;
                ++message_counter;
                chan->second.pop();
            }
        }

        if (message_counter >= nMaxMessages) {
            break;
        }
    }

    PRINT_DEBUG("got %i", message_counter);
    return message_counter;
}

/// AcceptSessionWithUser() should only be called in response to a SteamP2PSessionRequest_t callback
/// SteamP2PSessionRequest_t will be posted if another user tries to send you a message, and you haven't
/// tried to talk to them.  If you don't want to talk to them, just ignore the request.
/// If the user continues to send you messages, SteamP2PSessionRequest_t callbacks will continue to
/// be posted periodically.  This may be called multiple times for a single user.
///
/// Calling SendMessage() on the other user, this implicitly accepts any pending session request.
bool Steam_Networking_Messages::AcceptSessionWithUser( const SteamNetworkingIdentity &identityRemote )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    auto conn = connections.find(identityRemote.GetSteamID());
    if (conn == connections.end()) {
        return false;
    }

    conn->second.accepted = true;
    return true;
}

/// Call this when you're done talking to a user to immediately free up resources under-the-hood.
/// If the remote user tries to send data to you again, another P2PSessionRequest_t callback will
/// be posted.
///
/// Note that sessions that go unused for a few minutes are automatically timed out.
bool Steam_Networking_Messages::CloseSessionWithUser( const SteamNetworkingIdentity &identityRemote )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    auto conn = connections.find(identityRemote.GetSteamID());
    if (conn == connections.end()) {
        return false;
    }

    Common_Message msg;
    msg.set_source_id(settings->get_local_steam_id().ConvertToUint64());
    msg.set_dest_id(conn->second.remote_identity.GetSteamID64());
    msg.set_allocated_networking_messages(new Networking_Messages);
    msg.mutable_networking_messages()->set_type(Networking_Messages::CONNECTION_END);
    msg.mutable_networking_messages()->set_channel(0);
    msg.mutable_networking_messages()->set_id_from(conn->second.id);
    network->sendTo(&msg, true);

    connections.erase(conn);
    return true;
}

/// Call this  when you're done talking to a user on a specific channel.  Once all
/// open channels to a user have been closed, the open session to the user will be
/// closed, and any new data from this user will trigger a SteamP2PSessionRequest_t
/// callback
bool Steam_Networking_Messages::CloseChannelWithUser( const SteamNetworkingIdentity &identityRemote, int nLocalChannel )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    //TODO
    return false;
}

/// Returns information about the latest state of a connection, if any, with the given peer.
/// Primarily intended for debugging purposes, but can also be used to get more detailed
/// failure information.  (See SendMessageToUser and k_nSteamNetworkingSend_AutoRestartBrokwnSession.)
///
/// Returns the value of SteamNetConnectionInfo_t::m_eState, or k_ESteamNetworkingConnectionState_None
/// if no connection exists with specified peer.  You may pass nullptr for either parameter if
/// you do not need the corresponding details.  Note that sessions time out after a while,
/// so if a connection fails, or SendMessageToUser returns SendMessageToUser, you cannot wait
/// indefinitely to obtain the reason for failure.
ESteamNetworkingConnectionState Steam_Networking_Messages::GetSessionConnectionInfo( const SteamNetworkingIdentity &identityRemote, SteamNetConnectionInfo_t *pConnectionInfo, SteamNetConnectionRealTimeStatus_t *pQuickStatus )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    auto conn = connections.find(identityRemote.GetSteamID());
    if (conn == connections.end()) {
        return k_ESteamNetworkingConnectionState_None;
    }

    ESteamNetworkingConnectionState state = k_ESteamNetworkingConnectionState_Connected;
    if (conn->second.remote_id == 0 || !conn->second.accepted) {
        state = k_ESteamNetworkingConnectionState_Connecting;
    } else if (conn->second.dead) {
        state = k_ESteamNetworkingConnectionState_ClosedByPeer;
    }

    if (pConnectionInfo) {
        memset(pConnectionInfo, 0, sizeof(SteamNetConnectionInfo_t));
        pConnectionInfo->m_eState = state;
        pConnectionInfo->m_identityRemote = conn->second.remote_identity;
        //TODO
    }

    if (pQuickStatus) {
        memset(pQuickStatus, 0, sizeof(SteamNetConnectionRealTimeStatus_t));
        pQuickStatus->m_eState = state;
        pQuickStatus->m_nPing = 10; //TODO: calculate real numbers?
        pQuickStatus->m_flConnectionQualityLocal = 1.0;
        pQuickStatus->m_flConnectionQualityRemote = 1.0;
        //TODO
    }

    return k_ESteamNetworkingConnectionState_Connected;
}

void Steam_Networking_Messages::RunCallbacks()
{
    auto msg = std::begin(incoming_data);
    while (msg != std::end(incoming_data)) {
        CSteamID source_id((uint64)msg->source_id());

        auto conn = connections.find(source_id);
        if (conn != connections.end()) {
            if (conn->second.remote_id == msg->networking_messages().id_from())
                conn->second.data[msg->networking_messages().channel()].push(msg->networking_messages().data());
        }

        msg = incoming_data.erase(msg);
    }

    auto conn = std::begin(connections);
    while (conn != std::end(connections)) {
        if (!conn->second.accepted && check_timedout(conn->second.created, NETWORKING_MESSAGES_TIMEOUT)) {
            conn = connections.erase(conn);
        } else {
            ++conn;
        }
    }
}

void Steam_Networking_Messages::Callback(Common_Message *msg)
{
    if (msg->has_low_level()) {
        if (msg->low_level().type() == Low_Level::CONNECT) {
            
        }

        if (msg->low_level().type() == Low_Level::DISCONNECT) {
            end_connection((uint64)msg->source_id());
        }
    }

    if (msg->has_networking_messages()) {
        PRINT_DEBUG("got network socket msg %u", msg->networking_messages().type());
        if (msg->networking_messages().type() == Networking_Messages::CONNECTION_NEW) {
            SteamNetworkingIdentity identity;
            identity.SetSteamID64(msg->source_id());
            auto conn = find_or_create_message_connection(identity, true, false);
            conn->second.remote_id = msg->networking_messages().id_from();
            conn->second.dead = false;
        }

        if (msg->networking_messages().type() == Networking_Messages::CONNECTION_ACCEPT) {
            auto conn = connections.find((uint64)msg->source_id());
            if (conn != connections.end()) {
                conn->second.remote_id = msg->networking_messages().id_from();
            }
        }

        if (msg->networking_messages().type() == Networking_Messages::CONNECTION_END) {
            end_connection((uint64)msg->source_id());
        }

        if (msg->networking_messages().type() == Networking_Messages::DATA) {
            incoming_data.push_back(Common_Message(*msg));
        }
    }
}
