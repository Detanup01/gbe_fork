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

#ifndef __INCLUDED_STEAM_NETWORKING_H__
#define __INCLUDED_STEAM_NETWORKING_H__

#include "base.h"

struct Steam_Networking_Connection {
    CSteamID remote{};
    std::set<int> open_channels{};
};

struct steam_listen_socket {
    SNetListenSocket_t id{};
    int nVirtualP2PPort{};
    uint32 nIP{};
    uint16 nPort{};
};

enum steam_socket_connection_status {
    SOCKET_CONNECTING,
    SOCKET_CONNECTED,
    SOCKET_DISCONNECTED,
    SOCKET_KILLED,
};

struct steam_connection_socket {
    SNetSocket_t id{};
    SNetListenSocket_t listen_id{};
    enum steam_socket_connection_status status{};
    CSteamID target{};
    int nVirtualPort{};
    uint32 nIP{};
    uint16 nPort{};
    SNetSocket_t other_id{};
    std::vector<Network_Old> data_packets{};
};

class Steam_Networking :
public ISteamNetworking001,
public ISteamNetworking002,
public ISteamNetworking003,
public ISteamNetworking004,
public ISteamNetworking005,
public ISteamNetworking
{
    class Settings *settings{};
    class Networking *network{};
    class SteamCallBacks *callbacks{};
    class RunEveryRunCB *run_every_runcb{};

    std::recursive_mutex messages_mutex{};
    std::list<Common_Message> messages{};
    std::list<Common_Message> unprocessed_messages{};

    std::recursive_mutex connections_edit_mutex{};
    std::vector<struct Steam_Networking_Connection> connections{};

    std::vector<struct steam_listen_socket> listen_sockets{};
    std::vector<struct steam_connection_socket> connection_sockets{};

    std::map<CSteamID, std::chrono::high_resolution_clock::time_point> new_connection_times{};
    std::queue<CSteamID> new_connections_to_call_cb{};
    
    SNetListenSocket_t socket_number = 0;

    bool connection_exists(CSteamID id);
    struct Steam_Networking_Connection *get_or_create_connection(CSteamID id);
    void remove_connection(CSteamID id);
    SNetSocket_t create_connection_socket(CSteamID target, int nVirtualPort, uint32 nIP, uint16 nPort, SNetListenSocket_t id=0, enum steam_socket_connection_status status=SOCKET_CONNECTING, SNetSocket_t other_id=0);
    struct steam_connection_socket *get_connection_socket(SNetSocket_t id);
    void remove_killed_connection_sockets();

    static void steam_networking_callback(void *object, Common_Message *msg);
    static void steam_networking_run_every_runcp(void *object);

public:
    Steam_Networking(class Settings *settings, class Networking *network, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb);
    ~Steam_Networking();

    ////////////////////////////////////////////////////////////////////////////////////////////
    // Session-less connection functions
    //    automatically establishes NAT-traversing or Relay server connections

    // Sends a P2P packet to the specified user
    // UDP-like, unreliable and a max packet size of 1200 bytes
    // the first packet send may be delayed as the NAT-traversal code runs
    // if we can't get through to the user, an error will be posted via the callback P2PSessionConnectFail_t
    // see EP2PSend enum above for the descriptions of the different ways of sending packets
    //
    // nChannel is a routing number you can use to help route message to different systems 	- you'll have to call ReadP2PPacket() 
    // with the same channel number in order to retrieve the data on the other end
    // using different channels to talk to the same user will still use the same underlying p2p connection, saving on resources
    bool SendP2PPacket( CSteamID steamIDRemote, const void *pubData, uint32 cubData, EP2PSend eP2PSendType, int nChannel);

    bool SendP2PPacket( CSteamID steamIDRemote, const void *pubData, uint32 cubData, EP2PSend eP2PSendType );

    // returns true if any data is available for read, and the amount of data that will need to be read
    bool IsP2PPacketAvailable( uint32 *pcubMsgSize, int nChannel);

    bool IsP2PPacketAvailable( uint32 *pcubMsgSize);

    // reads in a packet that has been sent from another user via SendP2PPacket()
    // returns the size of the message and the steamID of the user who sent it in the last two parameters
    // if the buffer passed in is too small, the message will be truncated
    // this call is not blocking, and will return false if no data is available
    bool ReadP2PPacket( void *pubDest, uint32 cubDest, uint32 *pcubMsgSize, CSteamID *psteamIDRemote, int nChannel);

    bool ReadP2PPacket( void *pubDest, uint32 cubDest, uint32 *pcubMsgSize, CSteamID *psteamIDRemote);

    // AcceptP2PSessionWithUser() should only be called in response to a P2PSessionRequest_t callback
    // P2PSessionRequest_t will be posted if another user tries to send you a packet that you haven't talked to yet
    // if you don't want to talk to the user, just ignore the request
    // if the user continues to send you packets, another P2PSessionRequest_t will be posted periodically
    // this may be called multiple times for a single user
    // (if you've called SendP2PPacket() on the other user, this implicitly accepts the session request)
    bool AcceptP2PSessionWithUser( CSteamID steamIDRemote );


    // call CloseP2PSessionWithUser() when you're done talking to a user, will free up resources under-the-hood
    // if the remote user tries to send data to you again, another P2PSessionRequest_t callback will be posted
    bool CloseP2PSessionWithUser( CSteamID steamIDRemote );


    // call CloseP2PChannelWithUser() when you're done talking to a user on a specific channel. Once all channels
    // open channels to a user have been closed, the open session to the user will be closed and new data from this
    // user will trigger a P2PSessionRequest_t callback
    bool CloseP2PChannelWithUser( CSteamID steamIDRemote, int nChannel );


    // fills out P2PSessionState_t structure with details about the underlying connection to the user
    // should only needed for debugging purposes
    // returns false if no connection exists to the specified user
    bool GetP2PSessionState( CSteamID steamIDRemote, P2PSessionState_t *pConnectionState );


    // Allow P2P connections to fall back to being relayed through the Steam servers if a direct connection
    // or NAT-traversal cannot be established. Only applies to connections created after setting this value,
    // or to existing connections that need to automatically reconnect after this value is set.
    //
    // P2P packet relay is allowed by default
    bool AllowP2PPacketRelay( bool bAllow );



    ////////////////////////////////////////////////////////////////////////////////////////////
    // LISTEN / CONNECT style interface functions
    //
    // This is an older set of functions designed around the Berkeley TCP sockets model
    // it's preferential that you use the above P2P functions, they're more robust
    // and these older functions will be removed eventually
    //
    ////////////////////////////////////////////////////////////////////////////////////////////

    // creates a socket and listens others to connect
    // will trigger a SocketStatusCallback_t callback on another client connecting
    // nVirtualP2PPort is the unique ID that the client will connect to, in case you have multiple ports
    //		this can usually just be 0 unless you want multiple sets of connections
    // unIP is the local IP address to bind to
    //		pass in 0 if you just want the default local IP
    // unPort is the port to use
    //		pass in 0 if you don't want users to be able to connect via IP/Port, but expect to be always peer-to-peer connections only
    SNetListenSocket_t CreateListenSocket( int nVirtualP2PPort, uint32 nIP, uint16 nPort, bool bAllowUseOfPacketRelay );

    SNetListenSocket_t CreateListenSocket( int nVirtualP2PPort, SteamIPAddress_t nIP, uint16 nPort, bool bAllowUseOfPacketRelay );

    SNetListenSocket_t CreateListenSocket( int nVirtualP2PPort, uint32 nIP, uint16 nPort );

    // creates a socket and begin connection to a remote destination
    // can connect via a known steamID (client or game server), or directly to an IP
    // on success will trigger a SocketStatusCallback_t callback
    // on failure or timeout will trigger a SocketStatusCallback_t callback with a failure code in m_eSNetSocketState
    SNetSocket_t CreateP2PConnectionSocket( CSteamID steamIDTarget, int nVirtualPort, int nTimeoutSec, bool bAllowUseOfPacketRelay );

    SNetSocket_t CreateP2PConnectionSocket( CSteamID steamIDTarget, int nVirtualPort, int nTimeoutSec );

    SNetSocket_t CreateConnectionSocket( uint32 nIP, uint16 nPort, int nTimeoutSec );

    SNetSocket_t CreateConnectionSocket( SteamIPAddress_t nIP, uint16 nPort, int nTimeoutSec );

    // disconnects the connection to the socket, if any, and invalidates the handle
    // any unread data on the socket will be thrown away
    // if bNotifyRemoteEnd is set, socket will not be completely destroyed until the remote end acknowledges the disconnect
    bool DestroySocket( SNetSocket_t hSocket, bool bNotifyRemoteEnd );

    // destroying a listen socket will automatically kill all the regular sockets generated from it
    bool DestroyListenSocket( SNetListenSocket_t hSocket, bool bNotifyRemoteEnd );


    // sending data
    // must be a handle to a connected socket
    // data is all sent via UDP, and thus send sizes are limited to 1200 bytes; after this, many routers will start dropping packets
    // use the reliable flag with caution; although the resend rate is pretty aggressive,
    // it can still cause stalls in receiving data (like TCP)
    bool SendDataOnSocket( SNetSocket_t hSocket, void *pubData, uint32 cubData, bool bReliable );


    // receiving data
    // returns false if there is no data remaining
    // fills out *pcubMsgSize with the size of the next message, in bytes
    bool IsDataAvailableOnSocket( SNetSocket_t hSocket, uint32 *pcubMsgSize );


    // fills in pubDest with the contents of the message
    // messages are always complete, of the same size as was sent (i.e. packetized, not streaming)
    // if *pcubMsgSize < cubDest, only partial data is written
    // returns false if no data is available
    bool RetrieveDataFromSocket( SNetSocket_t hSocket, void *pubDest, uint32 cubDest, uint32 *pcubMsgSize );


    // checks for data from any socket that has been connected off this listen socket
    // returns false if there is no data remaining
    // fills out *pcubMsgSize with the size of the next message, in bytes
    // fills out *phSocket with the socket that data is available on
    bool IsDataAvailable( SNetListenSocket_t hListenSocket, uint32 *pcubMsgSize, SNetSocket_t *phSocket );


    // retrieves data from any socket that has been connected off this listen socket
    // fills in pubDest with the contents of the message
    // messages are always complete, of the same size as was sent (i.e. packetized, not streaming)
    // if *pcubMsgSize < cubDest, only partial data is written
    // returns false if no data is available
    // fills out *phSocket with the socket that data is available on
    bool RetrieveData( SNetListenSocket_t hListenSocket, void *pubDest, uint32 cubDest, uint32 *pcubMsgSize, SNetSocket_t *phSocket );


    // returns information about the specified socket, filling out the contents of the pointers
    bool GetSocketInfo( SNetSocket_t hSocket, CSteamID *pSteamIDRemote, int *peSocketStatus, uint32 *punIPRemote, uint16 *punPortRemote );

    bool GetSocketInfo( SNetSocket_t hSocket, CSteamID *pSteamIDRemote, int *peSocketStatus, SteamIPAddress_t *punIPRemote, uint16 *punPortRemote );

    // returns which local port the listen socket is bound to
    // *pnIP and *pnPort will be 0 if the socket is set to listen for P2P connections only
    bool GetListenSocketInfo( SNetListenSocket_t hListenSocket, uint32 *pnIP, uint16 *pnPort );

    bool GetListenSocketInfo( SNetListenSocket_t hListenSocket, SteamIPAddress_t *pnIP, uint16 *pnPort );

    // returns true to describe how the socket ended up connecting
    ESNetSocketConnectionType GetSocketConnectionType( SNetSocket_t hSocket );


    // max packet size, in bytes
    int GetMaxPacketSize( SNetSocket_t hSocket );

    void RunCallbacks();

    void Callback(Common_Message *msg);

};

#endif // __INCLUDED_STEAM_NETWORKING_H__
