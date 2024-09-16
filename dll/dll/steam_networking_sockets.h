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

#ifndef __INCLUDED_STEAM_NETWORKING_SOCKETS_H__
#define __INCLUDED_STEAM_NETWORKING_SOCKETS_H__

#include "base.h"

struct Listen_Socket {
    HSteamListenSocket socket_id{};

    int virtual_port{};
    int real_port{};

    CSteamID created_by{};
};

enum connect_socket_status {
    CONNECT_SOCKET_NO_CONNECTION,
    CONNECT_SOCKET_CONNECTING,
    CONNECT_SOCKET_NOT_ACCEPTED,
    CONNECT_SOCKET_CONNECTED,
    CONNECT_SOCKET_CLOSED,
    CONNECT_SOCKET_TIMEDOUT
};

struct Connect_Socket {
    struct compare_snm_for_queue {
        bool operator()(const Networking_Sockets &left, const Networking_Sockets &right) {
            return left.message_number() > right.message_number();
        }
    };

    int virtual_port{};
    int real_port{};

    SteamNetworkingIdentity remote_identity{};
    HSteamNetConnection remote_id{};

    HSteamListenSocket listen_socket_id{};

    enum connect_socket_status status{};
    int64 user_data{};

    std::priority_queue<Networking_Sockets, std::vector<Networking_Sockets>, compare_snm_for_queue> data{};
    HSteamNetPollGroup poll_group{};

    unsigned long long packet_send_counter{};
    CSteamID created_by{};

    std::chrono::steady_clock::time_point connect_request_last_sent{};
    unsigned connect_requests_sent{};
};

struct shared_between_client_server {
    std::vector<struct Listen_Socket> listen_sockets{};
    std::map<HSteamNetConnection, struct Connect_Socket> connect_sockets{};
    std::map<HSteamNetPollGroup, std::list<HSteamNetConnection>> poll_groups{};
    unsigned used{};
};

class Steam_Networking_Sockets :
public ISteamNetworkingSockets001,
public ISteamNetworkingSockets002,
public ISteamNetworkingSockets003,
public ISteamNetworkingSockets004,
public ISteamNetworkingSockets006,
public ISteamNetworkingSockets008,
public ISteamNetworkingSockets009,
public ISteamNetworkingSockets010,
public ISteamNetworkingSockets011,
public ISteamNetworkingSockets
{
    class Settings *settings{};
    class Networking *network{};
    class SteamCallResults *callback_results{};
    class SteamCallBacks *callbacks{};
    class RunEveryRunCB *run_every_runcb{};

    struct shared_between_client_server *sbcs{};
    std::chrono::steady_clock::time_point created{};

    static const int SNS_DISABLED_PORT = -1;

    static void steam_callback(void *object, Common_Message *msg);
    static void steam_run_every_runcb(void *object);

    SteamNetworkingMessage_t *get_steam_message_connection(HSteamNetConnection hConn);
    static void free_steam_message_data(SteamNetworkingMessage_t *pMsg);
    static void delete_steam_message(SteamNetworkingMessage_t *pMsg);

    static unsigned long get_socket_id();

    HSteamNetConnection new_connect_socket(SteamNetworkingIdentity remote_identity, int virtual_port, int real_port, enum connect_socket_status status=CONNECT_SOCKET_CONNECTING, HSteamListenSocket listen_socket_id=k_HSteamListenSocket_Invalid, HSteamNetConnection remote_id=k_HSteamNetConnection_Invalid);
    struct Listen_Socket *get_connection_socket(HSteamListenSocket id);

    bool send_packet_new_connection(HSteamNetConnection m_hConn);

    HSteamListenSocket new_listen_socket(int nSteamConnectVirtualPort, int real_port);

    ESteamNetworkingConnectionState convert_status(enum connect_socket_status old_status);

    void set_steamnetconnectioninfo(std::map<HSteamNetConnection, Connect_Socket>::iterator connect_socket, SteamNetConnectionInfo_t *pInfo);

    void launch_callback(HSteamNetConnection m_hConn, enum connect_socket_status old_status);

    void Callback(Common_Message *msg);

public:
    Steam_Networking_Sockets(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb, shared_between_client_server *sbcs);
    ~Steam_Networking_Sockets();

    shared_between_client_server *get_shared_between_client_server();


    /// Creates a "server" socket that listens for clients to connect to, either by calling
    /// ConnectSocketBySteamID or ConnectSocketByIPv4Address.
    ///
    /// nSteamConnectVirtualPort specifies how clients can connect to this socket using
    /// ConnectBySteamID.  A negative value indicates that this functionality is
    /// disabled and clients must connect by IP address.  It's very common for applications
    /// to only have one listening socket; in that case, use zero.  If you need to open
    /// multiple listen sockets and have clients be able to connect to one or the other, then
    /// nSteamConnectVirtualPort should be a small integer constant unique to each listen socket
    /// you create.
    ///
    /// In the open-source version of this API, you must pass -1 for nSteamConnectVirtualPort
    ///
    /// If you want clients to connect to you by your IPv4 addresses using
    /// ConnectByIPv4Address, then you must set nPort to be nonzero.  Steam will
    /// bind a UDP socket to the specified local port, and clients will send packets using
    /// ordinary IP routing.  It's up to you to take care of NAT, protecting your server
    /// from DoS, etc.  If you don't need clients to connect to you by IP, then set nPort=0.
    /// Use nIP if you wish to bind to a particular local interface.  Typically you will use 0,
    /// which means to listen on all interfaces, and accept the default outbound IP address.
    /// If nPort is zero, then nIP must also be zero.
    ///
    /// A SocketStatusCallback_t callback when another client attempts a connection.
    HSteamListenSocket CreateListenSocket( int nSteamConnectVirtualPort, uint32 nIP, uint16 nPort );

    /// Creates a "server" socket that listens for clients to connect to by 
    /// calling ConnectByIPAddress, over ordinary UDP (IPv4 or IPv6)
    ///
    /// You must select a specific local port to listen on and set it
    /// the port field of the local address.
    ///
    /// Usually you wil set the IP portion of the address to zero, (SteamNetworkingIPAddr::Clear()).
    /// This means that you will not bind to any particular local interface.  In addition,
    /// if possible the socket will be bound in "dual stack" mode, which means that it can
    /// accept both IPv4 and IPv6 clients.  If you wish to bind a particular interface, then
    /// set the local address to the appropriate IPv4 or IPv6 IP.
    ///
    /// When a client attempts to connect, a SteamNetConnectionStatusChangedCallback_t
    /// will be posted.  The connection will be in the connecting state.
    HSteamListenSocket CreateListenSocketIP( const SteamNetworkingIPAddr &localAddress );

    HSteamListenSocket CreateListenSocketIP( const SteamNetworkingIPAddr *localAddress );

    HSteamListenSocket CreateListenSocketIP( const SteamNetworkingIPAddr &localAddress, int nOptions, const SteamNetworkingConfigValue_t *pOptions );

    /// Creates a connection and begins talking to a "server" over UDP at the
    /// given IPv4 or IPv6 address.  The remote host must be listening with a
    /// matching call to CreateListenSocketIP on the specified port.
    ///
    /// A SteamNetConnectionStatusChangedCallback_t callback will be triggered when we start
    /// connecting, and then another one on either timeout or successful connection.
    ///
    /// If the server does not have any identity configured, then their network address
    /// will be the only identity in use.  Or, the network host may provide a platform-specific
    /// identity with or without a valid certificate to authenticate that identity.  (These
    /// details will be contained in the SteamNetConnectionStatusChangedCallback_t.)  It's
    /// up to your application to decide whether to allow the connection.
    ///
    /// By default, all connections will get basic encryption sufficient to prevent
    /// casual eavesdropping.  But note that without certificates (or a shared secret
    /// distributed through some other out-of-band mechanism), you don't have any
    /// way of knowing who is actually on the other end, and thus are vulnerable to
    /// man-in-the-middle attacks.
    HSteamNetConnection ConnectByIPAddress( const SteamNetworkingIPAddr &address );

    HSteamNetConnection ConnectByIPAddress( const SteamNetworkingIPAddr *address );

    HSteamNetConnection ConnectByIPAddress( const SteamNetworkingIPAddr &address, int nOptions, const SteamNetworkingConfigValue_t *pOptions );

    /// Like CreateListenSocketIP, but clients will connect using ConnectP2P
    ///
    /// nVirtualPort specifies how clients can connect to this socket using
    /// ConnectP2P.  It's very common for applications to only have one listening socket;
    /// in that case, use zero.  If you need to open multiple listen sockets and have clients
    /// be able to connect to one or the other, then nVirtualPort should be a small integer (<1000)
    /// unique to each listen socket you create.
    ///
    /// If you use this, you probably want to call ISteamNetworkingUtils::InitializeRelayNetworkAccess()
    /// when your app initializes
    HSteamListenSocket CreateListenSocketP2P( int nVirtualPort );

    HSteamListenSocket CreateListenSocketP2P( int nVirtualPort, int nOptions, const SteamNetworkingConfigValue_t *pOptions );

    /// Begin connecting to a server that is identified using a platform-specific identifier.
    /// This requires some sort of third party rendezvous service, and will depend on the
    /// platform and what other libraries and services you are integrating with.
    ///
    /// At the time of this writing, there is only one supported rendezvous service: Steam.
    /// Set the SteamID (whether "user" or "gameserver") and Steam will determine if the
    /// client is online and facilitate a relay connection.  Note that all P2P connections on
    /// Steam are currently relayed.
    ///
    /// If you use this, you probably want to call ISteamNetworkingUtils::InitializeRelayNetworkAccess()
    /// when your app initializes
    HSteamNetConnection ConnectP2P( const SteamNetworkingIdentity &identityRemote, int nVirtualPort );

    HSteamNetConnection ConnectP2P( const SteamNetworkingIdentity *identityRemote, int nVirtualPort );

    HSteamNetConnection ConnectP2P( const SteamNetworkingIdentity &identityRemote, int nVirtualPort, int nOptions, const SteamNetworkingConfigValue_t *pOptions );

    /// Creates a connection and begins talking to a remote destination.  The remote host
    /// must be listening with a matching call to CreateListenSocket.
    ///
    /// Use ConnectBySteamID to connect using the SteamID (client or game server) as the network address.
    /// Use ConnectByIPv4Address to connect by IP address.
    ///
    /// A SteamNetConnectionStatusChangedCallback_t callback will be triggered when we start connecting,
    /// and then another one on timeout or successful connection
    //#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE
    HSteamNetConnection ConnectBySteamID( CSteamID steamIDTarget, int nVirtualPort );

    //#endif
    HSteamNetConnection ConnectByIPv4Address( uint32 nIP, uint16 nPort );


    /// Accept an incoming connection that has been received on a listen socket.
    ///
    /// When a connection attempt is received (perhaps after a few basic handshake
    /// packets have been exchanged to prevent trivial spoofing), a connection interface
    /// object is created in the k_ESteamNetworkingConnectionState_Connecting state
    /// and a SteamNetConnectionStatusChangedCallback_t is posted.  At this point, your
    /// application MUST either accept or close the connection.  (It may not ignore it.)
    /// Accepting the connection will transition it either into the connected state,
    /// of the finding route state, depending on the connection type.
    ///
    /// You should take action within a second or two, because accepting the connection is
    /// what actually sends the reply notifying the client that they are connected.  If you
    /// delay taking action, from the client's perspective it is the same as the network
    /// being unresponsive, and the client may timeout the connection attempt.  In other
    /// words, the client cannot distinguish between a delay caused by network problems
    /// and a delay caused by the application.
    ///
    /// This means that if your application goes for more than a few seconds without
    /// processing callbacks (for example, while loading a map), then there is a chance
    /// that a client may attempt to connect in that interval and fail due to timeout.
    ///
    /// If the application does not respond to the connection attempt in a timely manner,
    /// and we stop receiving communication from the client, the connection attempt will
    /// be timed out locally, transitioning the connection to the
    /// k_ESteamNetworkingConnectionState_ProblemDetectedLocally state.  The client may also
    /// close the connection before it is accepted, and a transition to the
    /// k_ESteamNetworkingConnectionState_ClosedByPeer is also possible depending the exact
    /// sequence of events.
    ///
    /// Returns k_EResultInvalidParam if the handle is invalid.
    /// Returns k_EResultInvalidState if the connection is not in the appropriate state.
    /// (Remember that the connection state could change in between the time that the
    /// notification being posted to the queue and when it is received by the application.)
    EResult AcceptConnection( HSteamNetConnection hConn );


    /// Disconnects from the remote host and invalidates the connection handle.
    /// Any unread data on the connection is discarded.
    ///
    /// nReason is an application defined code that will be received on the other
    /// end and recorded (when possible) in backend analytics.  The value should
    /// come from a restricted range.  (See ESteamNetConnectionEnd.)  If you don't need
    /// to communicate any information to the remote host, and do not want analytics to
    /// be able to distinguish "normal" connection terminations from "exceptional" ones,
    /// You may pass zero, in which case the generic value of
    /// k_ESteamNetConnectionEnd_App_Generic will be used.
    ///
    /// pszDebug is an optional human-readable diagnostic string that will be received
    /// by the remote host and recorded (when possible) in backend analytics.
    ///
    /// If you wish to put the socket into a "linger" state, where an attempt is made to
    /// flush any remaining sent data, use bEnableLinger=true.  Otherwise reliable data
    /// is not flushed.
    ///
    /// If the connection has already ended and you are just freeing up the
    /// connection interface, the reason code, debug string, and linger flag are
    /// ignored.
    bool CloseConnection( HSteamNetConnection hPeer, int nReason, const char *pszDebug, bool bEnableLinger );


    /// Destroy a listen socket, and all the client sockets generated by accepting connections
    /// on the listen socket.
    ///
    /// pszNotifyRemoteReason determines what cleanup actions are performed on the client
    /// sockets being destroyed.  (See DestroySocket for more details.)
    ///
    /// Note that if cleanup is requested and you have requested the listen socket bound to a
    /// particular local port to facilitate direct UDP/IPv4 connections, then the underlying UDP
    /// socket must remain open until all clients have been cleaned up.
    bool CloseListenSocket( HSteamListenSocket hSocket, const char *pszNotifyRemoteReason );

    /// Destroy a listen socket.  All the connections that were accepting on the listen
    /// socket are closed ungracefully.
    bool CloseListenSocket( HSteamListenSocket hSocket );

    /// Set connection user data.  Returns false if the handle is invalid.
    bool SetConnectionUserData( HSteamNetConnection hPeer, int64 nUserData );


    /// Fetch connection user data.  Returns -1 if handle is invalid
    /// or if you haven't set any userdata on the connection.
    int64 GetConnectionUserData( HSteamNetConnection hPeer );


    /// Set a name for the connection, used mostly for debugging
    void SetConnectionName( HSteamNetConnection hPeer, const char *pszName );


    /// Fetch connection name.  Returns false if handle is invalid
    bool GetConnectionName( HSteamNetConnection hPeer, char *pszName, int nMaxLen );


    /// Send a message to the remote host on the connected socket.
    ///
    /// eSendType determines the delivery guarantees that will be provided,
    /// when data should be buffered, etc.
    ///
    /// Note that the semantics we use for messages are not precisely
    /// the same as the semantics of a standard "stream" socket.
    /// (SOCK_STREAM)  For an ordinary stream socket, the boundaries
    /// between chunks are not considered relevant, and the sizes of
    /// the chunks of data written will not necessarily match up to
    /// the sizes of the chunks that are returned by the reads on
    /// the other end.  The remote host might read a partial chunk,
    /// or chunks might be coalesced.  For the message semantics 
    /// used here, however, the sizes WILL match.  Each send call 
    /// will match a successful read call on the remote host 
    /// one-for-one.  If you are porting existing stream-oriented 
    /// code to the semantics of reliable messages, your code should 
    /// work the same, since reliable message semantics are more 
    /// strict than stream semantics.  The only caveat is related to 
    /// performance: there is per-message overhead to retain the 
    /// messages sizes, and so if your code sends many small chunks 
    /// of data, performance will suffer. Any code based on stream 
    /// sockets that does not write excessively small chunks will 
    /// work without any changes. 
    EResult SendMessageToConnection( HSteamNetConnection hConn, const void *pData, uint32 cbData, ESteamNetworkingSendType eSendType );

    /// Send a message to the remote host on the specified connection.
    ///
    /// nSendFlags determines the delivery guarantees that will be provided,
    /// when data should be buffered, etc.  E.g. k_nSteamNetworkingSend_Unreliable
    ///
    /// Note that the semantics we use for messages are not precisely
    /// the same as the semantics of a standard "stream" socket.
    /// (SOCK_STREAM)  For an ordinary stream socket, the boundaries
    /// between chunks are not considered relevant, and the sizes of
    /// the chunks of data written will not necessarily match up to
    /// the sizes of the chunks that are returned by the reads on
    /// the other end.  The remote host might read a partial chunk,
    /// or chunks might be coalesced.  For the message semantics 
    /// used here, however, the sizes WILL match.  Each send call 
    /// will match a successful read call on the remote host 
    /// one-for-one.  If you are porting existing stream-oriented 
    /// code to the semantics of reliable messages, your code should 
    /// work the same, since reliable message semantics are more 
    /// strict than stream semantics.  The only caveat is related to 
    /// performance: there is per-message overhead to retain the 
    /// message sizes, and so if your code sends many small chunks 
    /// of data, performance will suffer. Any code based on stream 
    /// sockets that does not write excessively small chunks will 
    /// work without any changes. 
    ///
    /// The pOutMessageNumber is an optional pointer to receive the
    /// message number assigned to the message, if sending was successful.
    ///
    /// Returns:
    /// - k_EResultInvalidParam: invalid connection handle, or the individual message is too big.
    ///   (See k_cbMaxSteamNetworkingSocketsMessageSizeSend)
    /// - k_EResultInvalidState: connection is in an invalid state
    /// - k_EResultNoConnection: connection has ended
    /// - k_EResultIgnored: You used k_nSteamNetworkingSend_NoDelay, and the message was dropped because
    ///   we were not ready to send it.
    /// - k_EResultLimitExceeded: there was already too much data queued to be sent.
    ///   (See k_ESteamNetworkingConfig_SendBufferSize)
    EResult SendMessageToConnection( HSteamNetConnection hConn, const void *pData, uint32 cbData, int nSendFlags, int64 *pOutMessageNumber );

    EResult SendMessageToConnection( HSteamNetConnection hConn, const void *pData, uint32 cbData, int nSendFlags );

    /// Send one or more messages without copying the message payload.
    /// This is the most efficient way to send messages. To use this
    /// function, you must first allocate a message object using
    /// ISteamNetworkingUtils::AllocateMessage.  (Do not declare one
    /// on the stack or allocate your own.)
    ///
    /// You should fill in the message payload.  You can either let
    /// it allocate the buffer for you and then fill in the payload,
    /// or if you already have a buffer allocated, you can just point
    /// m_pData at your buffer and set the callback to the appropriate function
    /// to free it.  Note that if you use your own buffer, it MUST remain valid
    /// until the callback is executed.  And also note that your callback can be
    /// invoked at ant time from any thread (perhaps even before SendMessages
    /// returns!), so it MUST be fast and threadsafe.
    ///
    /// You MUST also fill in:
    /// - m_conn - the handle of the connection to send the message to
    /// - m_nFlags - bitmask of k_nSteamNetworkingSend_xxx flags.
    ///
    /// All other fields are currently reserved and should not be modified.
    ///
    /// The library will take ownership of the message structures.  They may
    /// be modified or become invalid at any time, so you must not read them
    /// after passing them to this function.
    ///
    /// pOutMessageNumberOrResult is an optional array that will receive,
    /// for each message, the message number that was assigned to the message
    /// if sending was successful.  If sending failed, then a negative EResult
    /// valid is placed into the array.  For example, the array will hold
    /// -k_EResultInvalidState if the connection was in an invalid state.
    /// See ISteamNetworkingSockets::SendMessageToConnection for possible
    /// failure codes.
    void SendMessages( int nMessages, SteamNetworkingMessage_t *const *pMessages, int64 *pOutMessageNumberOrResult );


    /// If Nagle is enabled (its on by default) then when calling 
    /// SendMessageToConnection the message will be queued up the Nagle time
    /// before being sent to merge small messages into the same packet.
    ///
    /// Call this function to flush any queued messages and send them immediately
    /// on the next transmission time (often that means right now).
    EResult FlushMessagesOnConnection( HSteamNetConnection hConn );

    /// Fetch the next available message(s) from the connection, if any.
    /// Returns the number of messages returned into your array, up to nMaxMessages.
    /// If the connection handle is invalid, -1 is returned.
    ///
    /// The order of the messages returned in the array is relevant.
    /// Reliable messages will be received in the order they were sent (and with the
    /// same sizes --- see SendMessageToConnection for on this subtle difference from a stream socket).
    ///
    /// Unreliable messages may be dropped, or delivered out of order withrespect to
    /// each other or with respect to reliable messages.  The same unreliable message
    /// may be received multiple times.
    ///
    /// If any messages are returned, you MUST call SteamNetworkingMessage_t::Release() on each
    /// of them free up resources after you are done.  It is safe to keep the object alive for
    /// a little while (put it into some queue, etc), and you may call Release() from any thread.
    int ReceiveMessagesOnConnection( HSteamNetConnection hConn, SteamNetworkingMessage_t **ppOutMessages, int nMaxMessages );

    /// Same as ReceiveMessagesOnConnection, but will return the next message available
    /// on any connection that was accepted through the specified listen socket.  Examine
    /// SteamNetworkingMessage_t::m_conn to know which client connection.
    ///
    /// Delivery order of messages among different clients is not defined.  They may
    /// be returned in an order different from what they were actually received.  (Delivery
    /// order of messages from the same client is well defined, and thus the order of the
    /// messages is relevant!)
    int ReceiveMessagesOnListenSocket( HSteamListenSocket hSocket, SteamNetworkingMessage_t **ppOutMessages, int nMaxMessages );

    /// Returns basic information about the high-level state of the connection.
    bool GetConnectionInfo( HSteamNetConnection hConn, SteamNetConnectionInfo_t *pInfo );

    /// Returns a small set of information about the real-time state of the connection
    /// and the queue status of each lane.
    /// 
    /// - pStatus may be NULL if the information is not desired.  (E.g. you are only interested
    ///   in the lane information.)
    /// - On entry, nLanes specifies the length of the pLanes array.  This may be 0
    ///   if you do not wish to receive any lane data.  It's OK for this to be smaller than
    ///   the total number of configured lanes.
    /// - pLanes points to an array that will receive lane-specific info.  It can be NULL
    ///   if this is not needed.
    /// 
    /// Return value:
    /// - k_EResultNoConnection - connection handle is invalid or connection has been closed.
    /// - k_EResultInvalidParam - nLanes is bad
    EResult GetConnectionRealTimeStatus( HSteamNetConnection hConn, SteamNetConnectionRealTimeStatus_t *pStatus, int nLanes, SteamNetConnectionRealTimeLaneStatus_t *pLanes );

    /// Fetch the next available message(s) from the socket, if any.
    /// Returns the number of messages returned into your array, up to nMaxMessages.
    /// If the connection handle is invalid, -1 is returned.
    ///
    /// The order of the messages returned in the array is relevant.
    /// Reliable messages will be received in the order they were sent (and with the
    /// same sizes --- see SendMessageToConnection for on this subtle difference from a stream socket).
    ///
    /// FIXME - We're still debating the exact set of guarantees for unreliable, so this might change.
    /// Unreliable messages may not be received.  The order of delivery of unreliable messages
    /// is NOT specified.  They may be received out of order with respect to each other or
    /// reliable messages.  They may be received multiple times!
    ///
    /// If any messages are returned, you MUST call Release() to each of them free up resources
    /// after you are done.  It is safe to keep the object alive for a little while (put it
    /// into some queue, etc), and you may call Release() from any thread.
    int ReceiveMessagesOnConnection( HSteamNetConnection hConn, SteamNetworkingMessage001_t **ppOutMessages, int nMaxMessages );
    

    /// Same as ReceiveMessagesOnConnection, but will return the next message available
    /// on any client socket that was accepted through the specified listen socket.  Examine
    /// SteamNetworkingMessage_t::m_conn to know which client connection.
    ///
    /// Delivery order of messages among different clients is not defined.  They may
    /// be returned in an order different from what they were actually received.  (Delivery
    /// order of messages from the same client is well defined, and thus the order of the
    /// messages is relevant!)
    int ReceiveMessagesOnListenSocket( HSteamListenSocket hSocket, SteamNetworkingMessage001_t **ppOutMessages, int nMaxMessages );
    

    /// Returns information about the specified connection.
    bool GetConnectionInfo( HSteamNetConnection hConn, SteamNetConnectionInfo001_t *pInfo );


    /// Returns brief set of connection status that you might want to display
    /// to the user in game.
    bool GetQuickConnectionStatus( HSteamNetConnection hConn, SteamNetworkingQuickConnectionStatus *pStats );


    /// Returns detailed connection stats in text format.  Useful
    /// for dumping to a log, etc.
    ///
    /// Returns:
    /// -1 failure (bad connection handle)
    /// 0 OK, your buffer was filled in and '\0'-terminated
    /// >0 Your buffer was either nullptr, or it was too small and the text got truncated.  Try again with a buffer of at least N bytes.
    int GetDetailedConnectionStatus( HSteamNetConnection hConn, char *pszBuf, int cbBuf );

    /// Returns local IP and port that a listen socket created using CreateListenSocketIP is bound to.
    ///
    /// An IPv6 address of ::0 means "any IPv4 or IPv6"
    /// An IPv6 address of ::ffff:0000:0000 means "any IPv4"
    bool GetListenSocketAddress( HSteamListenSocket hSocket, SteamNetworkingIPAddr *address );

    /// Returns information about the listen socket.
    ///
    /// *pnIP and *pnPort will be 0 if the socket is set to listen for connections based
    /// on SteamID only.  If your listen socket accepts connections on IPv4, then both
    /// fields will return nonzero, even if you originally passed a zero IP.  However,
    /// note that the address returned may be a private address (e.g. 10.0.0.x or 192.168.x.x),
    /// and may not be reachable by a general host on the Internet.
    bool GetListenSocketInfo( HSteamListenSocket hSocket, uint32 *pnIP, uint16 *pnPort );


    /// Create a pair of connections that are talking to each other, e.g. a loopback connection.
    /// This is very useful for testing, or so that your client/server code can work the same
    /// even when you are running a local "server".
    ///
    /// The two connections will immediately be placed into the connected state, and no callbacks
    /// will be posted immediately.  After this, if you close either connection, the other connection
    /// will receive a callback, exactly as if they were communicating over the network.  You must
    /// close *both* sides in order to fully clean up the resources!
    ///
    /// By default, internal buffers are used, completely bypassing the network, the chopping up of
    /// messages into packets, encryption, copying the payload, etc.  This means that loopback
    /// packets, by default, will not simulate lag or loss.  Passing true for bUseNetworkLoopback will
    /// cause the socket pair to send packets through the local network loopback device (127.0.0.1)
    /// on ephemeral ports.  Fake lag and loss are supported in this case, and CPU time is expended
    /// to encrypt and decrypt.
    ///
    /// The SteamID assigned to both ends of the connection will be the SteamID of this interface.
    bool CreateSocketPair( HSteamNetConnection *pOutConnection1, HSteamNetConnection *pOutConnection2, bool bUseNetworkLoopback );

    /// Create a pair of connections that are talking to each other, e.g. a loopback connection.
    /// This is very useful for testing, or so that your client/server code can work the same
    /// even when you are running a local "server".
    ///
    /// The two connections will immediately be placed into the connected state, and no callbacks
    /// will be posted immediately.  After this, if you close either connection, the other connection
    /// will receive a callback, exactly as if they were communicating over the network.  You must
    /// close *both* sides in order to fully clean up the resources!
    ///
    /// By default, internal buffers are used, completely bypassing the network, the chopping up of
    /// messages into packets, encryption, copying the payload, etc.  This means that loopback
    /// packets, by default, will not simulate lag or loss.  Passing true for bUseNetworkLoopback will
    /// cause the socket pair to send packets through the local network loopback device (127.0.0.1)
    /// on ephemeral ports.  Fake lag and loss are supported in this case, and CPU time is expended
    /// to encrypt and decrypt.
    ///
    /// If you wish to assign a specific identity to either connection, you may pass a particular
    /// identity.  Otherwise, if you pass nullptr, the respective connection will assume a generic
    /// "localhost" identity.  If you use real network loopback, this might be translated to the
    /// actual bound loopback port.  Otherwise, the port will be zero.
    bool CreateSocketPair( HSteamNetConnection *pOutConnection1, HSteamNetConnection *pOutConnection2, bool bUseNetworkLoopback, const SteamNetworkingIdentity *pIdentity1, const SteamNetworkingIdentity *pIdentity2 );

    /// Configure multiple outbound messages streams ("lanes") on a connection, and
    /// control head-of-line blocking between them.  Messages within a given lane
    /// are always sent in the order they are queued, but messages from different
    /// lanes may be sent out of order.  Each lane has its own message number
    /// sequence.  The first message sent on each lane will be assigned the number 1.
    ///
    /// Each lane has a "priority".  Lower priority lanes will only be processed
    /// when all higher-priority lanes are empty.  The magnitudes of the priority
    /// values are not relevant, only their sort order.  Higher numeric values
    /// take priority over lower numeric values.
    /// 
    /// Each lane also is assigned a weight, which controls the approximate proportion
    /// of the bandwidth that will be consumed by the lane, relative to other lanes
    /// of the same priority.  (This is assuming the lane stays busy.  An idle lane
    /// does not build up "credits" to be be spent once a message is queued.)
    /// This value is only meaningful as a proportion, relative to other lanes with
    /// the same priority.  For lanes with different priorities, the strict priority
    /// order will prevail, and their weights relative to each other are not relevant.
    /// Thus, if a lane has a unique priority value, the weight value for that lane is
    /// not relevant.  
    ///
    /// Example: 3 lanes, with priorities [ 0, 10, 10 ] and weights [ (NA), 20, 5 ].
    /// Messages sent on the first will always be sent first, before messages in the
    /// other two lanes.  Its weight value is irrelevant, since there are no other
    /// lanes with priority=0.  The other two lanes will share bandwidth, with the second
    /// and third lanes sharing bandwidth using a ratio of approximately 4:1.
    /// (The weights [ NA, 4, 1 ] would be equivalent.)
    ///
    /// Notes:
    /// - At the time of this writing, some code has performance cost that is linear
    ///   in the number of lanes, so keep the number of lanes to an absolute minimum.
    ///   3 or so is fine; >8 is a lot.  The max number of lanes on Steam is 255,
    ///   which is a very large number and not recommended!  If you are compiling this
    ///   library from source, see STEAMNETWORKINGSOCKETS_MAX_LANES.)
    /// - Lane priority values may be any int.  Their absolute value is not relevant,
    ///   only the order matters.
    /// - Weights must be positive, and due to implementation details, they are restricted
    ///   to 16-bit values.  The absolute magnitudes don't matter, just the proportions.
    /// - Messages sent on a lane index other than 0 have a small overhead on the wire,
    ///   so for maximum wire efficiency, lane 0 should be the "most common" lane, regardless
    ///   of priorities or weights.
    /// - A connection has a single lane by default.  Calling this function with
    ///   nNumLanes=1 is legal, but pointless, since the priority and weight values are
    ///   irrelevant in that case.
    /// - You may reconfigure connection lanes at any time, however reducing the number of
    ///   lanes is not allowed.
    /// - Reconfiguring lanes might restart any bandwidth sharing balancing.  Usually you
    ///   will call this function once, near the start of the connection, perhaps after
    ///   exchanging a few messages.
    /// - To assign all lanes the same priority, you may use pLanePriorities=NULL.
    /// - If you wish all lanes with the same priority to share bandwidth equally (or
    ///   if no two lanes have the same priority value, and thus priority values are
    ///   irrelevant), you may use pLaneWeights=NULL
    /// - Priorities and weights determine the order that messages are SENT on the wire.
    ///   There are NO GUARANTEES on the order that messages are RECEIVED!  Due to packet
    ///   loss, out-of-order delivery, and subtle details of packet serialization, messages
    ///   might still be received slightly out-of-order!  The *only* strong guarantee is that
    ///   *reliable* messages on the *same lane* will be delivered in the order they are sent.
    /// - Each host configures the lanes for the packets they send; the lanes for the flow
    ///   in one direction are completely unrelated to the lanes in the opposite direction.
    /// 
    /// Return value:
    /// - k_EResultNoConnection - bad hConn
    /// - k_EResultInvalidParam - Invalid number of lanes, bad weights, or you tried to reduce the number of lanes
    /// - k_EResultInvalidState - Connection is already dead, etc
    /// 
    /// See also:
    /// SteamNetworkingMessage_t::m_idxLane
    EResult ConfigureConnectionLanes( HSteamNetConnection hConn, int nNumLanes, const int *pLanePriorities, const uint16 *pLaneWeights );


    /// Get the identity assigned to this interface.
    /// E.g. on Steam, this is the user's SteamID, or for the gameserver interface, the SteamID assigned
    /// to the gameserver.  Returns false and sets the result to an invalid identity if we don't know
    /// our identity yet.  (E.g. GameServer has not logged in.  On Steam, the user will know their SteamID
    /// even if they are not signed into Steam.)
    bool GetIdentity( SteamNetworkingIdentity *pIdentity );

    /// Indicate our desire to be ready participate in authenticated communications.
    /// If we are currently not ready, then steps will be taken to obtain the necessary
    /// certificates.   (This includes a certificate for us, as well as any CA certificates
    /// needed to authenticate peers.)
    ///
    /// You can call this at program init time if you know that you are going to
    /// be making authenticated connections, so that we will be ready immediately when
    /// those connections are attempted.  (Note that essentially all connections require
    /// authentication, with the exception of ordinary UDP connections with authentication
    /// disabled using k_ESteamNetworkingConfig_IP_AllowWithoutAuth.)  If you don't call
    /// this function, we will wait until a feature is utilized that that necessitates
    /// these resources.
    ///
    /// You can also call this function to force a retry, if failure has occurred.
    /// Once we make an attempt and fail, we will not automatically retry.
    /// In this respect, the behavior of the system after trying and failing is the same
    /// as before the first attempt: attempting authenticated communication or calling
    /// this function will call the system to attempt to acquire the necessary resources.
    ///
    /// You can use GetAuthenticationStatus or listen for SteamNetAuthenticationStatus_t
    /// to monitor the status.
    ///
    /// Returns the current value that would be returned from GetAuthenticationStatus.
    ESteamNetworkingAvailability InitAuthentication();

    /// Query our readiness to participate in authenticated communications.  A
    /// SteamNetAuthenticationStatus_t callback is posted any time this status changes,
    /// but you can use this function to query it at any time.
    ///
    /// The value of SteamNetAuthenticationStatus_t::m_eAvail is returned.  If you only
    /// want this high level status, you can pass NULL for pDetails.  If you want further
    /// details, pass non-NULL to receive them.
    ESteamNetworkingAvailability GetAuthenticationStatus( SteamNetAuthenticationStatus_t *pDetails );

    /// Create a new poll group.
    ///
    /// You should destroy the poll group when you are done using DestroyPollGroup
    HSteamNetPollGroup CreatePollGroup();

    /// Destroy a poll group created with CreatePollGroup().
    ///
    /// If there are any connections in the poll group, they are removed from the group,
    /// and left in a state where they are not part of any poll group.
    /// Returns false if passed an invalid poll group handle.
    bool DestroyPollGroup( HSteamNetPollGroup hPollGroup );

    /// Assign a connection to a poll group.  Note that a connection may only belong to a
    /// single poll group.  Adding a connection to a poll group implicitly removes it from
    /// any other poll group it is in.
    ///
    /// You can pass k_HSteamNetPollGroup_Invalid to remove a connection from its current
    /// poll group without adding it to a new poll group.
    ///
    /// If there are received messages currently pending on the connection, an attempt
    /// is made to add them to the queue of messages for the poll group in approximately
    /// the order that would have applied if the connection was already part of the poll
    /// group at the time that the messages were received.
    ///
    /// Returns false if the connection handle is invalid, or if the poll group handle
    /// is invalid (and not k_HSteamNetPollGroup_Invalid).
    bool SetConnectionPollGroup( HSteamNetConnection hConn, HSteamNetPollGroup hPollGroup );

    /// Same as ReceiveMessagesOnConnection, but will return the next messages available
    /// on any connection in the poll group.  Examine SteamNetworkingMessage_t::m_conn
    /// to know which connection.  (SteamNetworkingMessage_t::m_nConnUserData might also
    /// be useful.)
    ///
    /// Delivery order of messages among different connections will usually match the
    /// order that the last packet was received which completed the message.  But this
    /// is not a strong guarantee, especially for packets received right as a connection
    /// is being assigned to poll group.
    ///
    /// Delivery order of messages on the same connection is well defined and the
    /// same guarantees are present as mentioned in ReceiveMessagesOnConnection.
    /// (But the messages are not grouped by connection, so they will not necessarily
    /// appear consecutively in the list; they may be interleaved with messages for
    /// other connections.)
    int ReceiveMessagesOnPollGroup( HSteamNetPollGroup hPollGroup, SteamNetworkingMessage_t **ppOutMessages, int nMaxMessages );


    //#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE

    //
    // Clients connecting to dedicated servers hosted in a data center,
    // using central-authority-granted tickets.
    //

    /// Called when we receive a ticket from our central matchmaking system.  Puts the
    /// ticket into a persistent cache, and optionally returns the parsed ticket.
    ///
    /// See stamdatagram_ticketgen.h for more details.
    bool ReceivedRelayAuthTicket( const void *pvTicket, int cbTicket, SteamDatagramRelayAuthTicket *pOutParsedTicket );


    /// Search cache for a ticket to talk to the server on the specified virtual port.
    /// If found, returns the number of second until the ticket expires, and optionally
    /// the complete cracked ticket.  Returns 0 if we don't have a ticket.
    ///
    /// Typically this is useful just to confirm that you have a ticket, before you
    /// call ConnectToHostedDedicatedServer to connect to the server.
    int FindRelayAuthTicketForServer( CSteamID steamID, int nVirtualPort, SteamDatagramRelayAuthTicket *pOutParsedTicket );

    /// Search cache for a ticket to talk to the server on the specified virtual port.
    /// If found, returns the number of seconds until the ticket expires, and optionally
    /// the complete cracked ticket.  Returns 0 if we don't have a ticket.
    ///
    /// Typically this is useful just to confirm that you have a ticket, before you
    /// call ConnectToHostedDedicatedServer to connect to the server.
    int FindRelayAuthTicketForServer( const SteamNetworkingIdentity *identityGameServer, int nVirtualPort, SteamDatagramRelayAuthTicket *pOutParsedTicket );

    int FindRelayAuthTicketForServer( const SteamNetworkingIdentity &identityGameServer, int nVirtualPort, SteamDatagramRelayAuthTicket *pOutParsedTicket );

    /// Client call to connect to a server hosted in a Valve data center, on the specified virtual
    /// port.  You must have placed a ticket for this server into the cache, or else this connect attempt will fail!
    ///
    /// You may wonder why tickets are stored in a cache, instead of simply being passed as an argument
    /// here.  The reason is to make reconnection to a gameserver robust, even if the client computer loses
    /// connection to Steam or the central backend, or the app is restarted or crashes, etc.
    ///
    /// If you use this, you probably want to call ISteamNetworkingUtils::InitializeRelayNetworkAccess()
    /// when your app initializes
    HSteamNetConnection ConnectToHostedDedicatedServer( const SteamNetworkingIdentity &identityTarget, int nVirtualPort );

    HSteamNetConnection ConnectToHostedDedicatedServer( const SteamNetworkingIdentity *identityTarget, int nVirtualPort );

    /// Client call to connect to a server hosted in a Valve data center, on the specified virtual
    /// port.  You should have received a ticket for this server, or else this connect call will fail!
    ///
    /// You may wonder why tickets are stored in a cache, instead of simply being passed as an argument
    /// here.  The reason is to make reconnection to a gameserver robust, even if the client computer loses
    /// connection to Steam or the central backend, or the app is restarted or crashes, etc.
    HSteamNetConnection ConnectToHostedDedicatedServer( CSteamID steamIDTarget, int nVirtualPort );

    HSteamNetConnection ConnectToHostedDedicatedServer( const SteamNetworkingIdentity &identityTarget, int nVirtualPort, int nOptions, const SteamNetworkingConfigValue_t *pOptions );

    //
    // Servers hosted in Valve data centers
    //

    /// Returns the value of the SDR_LISTEN_PORT environment variable.
    uint16 GetHostedDedicatedServerPort();


    /// If you are running in a production data center, this will return the data
    /// center code.  Returns 0 otherwise.
    SteamNetworkingPOPID GetHostedDedicatedServerPOPID();


    /// Return info about the hosted server.  You will need to send this information to your
    /// backend, and put it in tickets, so that the relays will know how to forward traffic from
    /// clients to your server.  See SteamDatagramRelayAuthTicket for more info.
    ///
    /// NOTE ABOUT DEVELOPMENT ENVIRONMENTS:
    /// In production in our data centers, these parameters are configured via environment variables.
    /// In development, the only one you need to set is SDR_LISTEN_PORT, which is the local port you
    /// want to listen on.  Furthermore, if you are running your server behind a corporate firewall,
    /// you probably will not be able to put the routing information returned by this function into
    /// tickets.   Instead, it should be a public internet address that the relays can use to send
    /// data to your server.  So you might just end up hardcoding a public address and setup port
    /// forwarding on your corporate firewall.  In that case, the port you put into the ticket
    /// needs to be the public-facing port opened on your firewall, if it is different from the
    /// actual server port.
    ///
    /// This function will fail if SteamDatagramServer_Init has not been called.
    ///
    /// Returns false if the SDR_LISTEN_PORT environment variable is not set.
    bool GetHostedDedicatedServerAddress001( SteamDatagramHostedAddress *pRouting );

    /// Return info about the hosted server.  This contains the PoPID of the server,
    /// and opaque routing information that can be used by the relays to send traffic
    /// to your server.
    ///
    /// You will need to send this information to your backend, and put it in tickets,
    /// so that the relays will know how to forward traffic from
    /// clients to your server.  See SteamDatagramRelayAuthTicket for more info.
    ///
    /// Also, note that the routing information is contained in SteamDatagramGameCoordinatorServerLogin,
    /// so if possible, it's preferred to use GetGameCoordinatorServerLogin to send this info
    /// to your game coordinator service, and also login securely at the same time.
    ///
    /// On a successful exit, k_EResultOK is returned
    ///
    /// Unsuccessful exit:
    /// - Something other than k_EResultOK is returned.
    /// - k_EResultInvalidState: We are not configured to listen for SDR (SDR_LISTEN_SOCKET
    ///   is not set.)
    /// - k_EResultPending: we do not (yet) have the authentication information needed.
    ///   (See GetAuthenticationStatus.)  If you use environment variables to pre-fetch
    ///   the network config, this data should always be available immediately.
    /// - A non-localized diagnostic debug message will be placed in m_data that describes
    ///   the cause of the failure.
    ///
    /// NOTE: The returned blob is not encrypted.  Send it to your backend, but don't
    ///       directly share it with clients.
    virtual EResult GetHostedDedicatedServerAddress( SteamDatagramHostedAddress *pRouting );

    /// Create a listen socket on the specified virtual port.  The physical UDP port to use
    /// will be determined by the SDR_LISTEN_PORT environment variable.  If a UDP port is not
    /// configured, this call will fail.
    ///
    /// Note that this call MUST be made through the SteamNetworkingSocketsGameServer() interface
    HSteamListenSocket CreateHostedDedicatedServerListenSocket( int nVirtualPort );

    /// Create a listen socket on the specified virtual port.  The physical UDP port to use
    /// will be determined by the SDR_LISTEN_PORT environment variable.  If a UDP port is not
    /// configured, this call will fail.
    ///
    /// Note that this call MUST be made through the SteamGameServerNetworkingSockets() interface
    ///
    /// If you need to set any initial config options, pass them here.  See
    /// SteamNetworkingConfigValue_t for more about why this is preferable to
    /// setting the options "immediately" after creation.
    HSteamListenSocket CreateHostedDedicatedServerListenSocket( int nVirtualPort, int nOptions, const SteamNetworkingConfigValue_t *pOptions );


    //#endif // #ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE

    //
    // Gets some debug text from the connection
    //
    bool GetConnectionDebugText( HSteamNetConnection hConn, char *pOut, int nOutCCH );


    //
    // Set and get configuration values, see ESteamNetworkingConfigurationValue for individual descriptions.
    //
    // Returns the value or -1 is eConfigValue is invalid
    int32 GetConfigurationValue( ESteamNetworkingConfigurationValue eConfigValue );

    // Returns true if successfully set
    bool SetConfigurationValue( ESteamNetworkingConfigurationValue eConfigValue, int32 nValue );


    // Return the name of an int configuration value, or NULL if config value isn't known
    const char *GetConfigurationValueName( ESteamNetworkingConfigurationValue eConfigValue );


    //
    // Set and get configuration strings, see ESteamNetworkingConfigurationString for individual descriptions.
    //
    // Get the configuration string, returns length of string needed if pDest is nullpr or destSize is 0
    // returns -1 if the eConfigValue is invalid
    int32 GetConfigurationString( ESteamNetworkingConfigurationString eConfigString, char *pDest, int32 destSize );

    bool SetConfigurationString( ESteamNetworkingConfigurationString eConfigString, const char *pString );


    // Return the name of a string configuration value, or NULL if config value isn't known
    const char *GetConfigurationStringName( ESteamNetworkingConfigurationString eConfigString );


    //
    // Set and get configuration values, see ESteamNetworkingConnectionConfigurationValue for individual descriptions.
    //
    // Returns the value or -1 is eConfigValue is invalid
    int32 GetConnectionConfigurationValue( HSteamNetConnection hConn, ESteamNetworkingConnectionConfigurationValue eConfigValue );

    // Returns true if successfully set
    bool SetConnectionConfigurationValue( HSteamNetConnection hConn, ESteamNetworkingConnectionConfigurationValue eConfigValue, int32 nValue );

    /// Generate an authentication blob that can be used to securely login with
    /// your backend, using SteamDatagram_ParseHostedServerLogin.  (See
    /// steamdatagram_gamecoordinator.h)
    ///
    /// Before calling the function:
    /// - Populate the app data in pLoginInfo (m_cbAppData and m_appData).  You can leave
    ///   all other fields uninitialized.
    /// - *pcbSignedBlob contains the size of the buffer at pBlob.  (It should be
    ///   at least k_cbMaxSteamDatagramGameCoordinatorServerLoginSerialized.)
    ///
    /// On a successful exit:
    /// - k_EResultOK is returned
    /// - All of the remaining fields of pLoginInfo will be filled out.
    /// - *pcbSignedBlob contains the size of the serialized blob that has been
    ///   placed into pBlob.
    ///
    /// Unsuccessful exit:
    /// - Something other than k_EResultOK is returned.
    /// - k_EResultNotLoggedOn: you are not logged in (yet)
    /// - See GetHostedDedicatedServerAddress for more potential failure return values.
    /// - A non-localized diagnostic debug message will be placed in pBlob that describes
    ///   the cause of the failure.
    ///
    /// This works by signing the contents of the SteamDatagramGameCoordinatorServerLogin
    /// with the cert that is issued to this server.  In dev environments, it's OK if you do
    /// not have a cert.  (You will need to enable insecure dev login in SteamDatagram_ParseHostedServerLogin.)
    /// Otherwise, you will need a signed cert.
    ///
    /// NOTE: The routing blob returned here is not encrypted.  Send it to your backend
    ///       and don't share it directly with clients.
    EResult GetGameCoordinatorServerLogin( SteamDatagramGameCoordinatorServerLogin *pLoginInfo, int *pcbSignedBlob, void *pBlob );

    //
    // Relayed connections using custom signaling protocol
    //
    // This is used if you have your own method of sending out-of-band
    // signaling / rendezvous messages through a mutually trusted channel.
    //

    /// Create a P2P "client" connection that does signaling over a custom
    /// rendezvous/signaling channel.
    ///
    /// pSignaling points to a new object that you create just for this connection.
    /// It must stay valid until Release() is called.  Once you pass the
    /// object to this function, it assumes ownership.  Release() will be called
    /// from within the function call if the call fails.  Furthermore, until Release()
    /// is called, you should be prepared for methods to be invoked on your
    /// object from any thread!  You need to make sure your object is threadsafe!
    /// Furthermore, you should make sure that dispatching the methods is done
    /// as quickly as possible.
    ///
    /// This function will immediately construct a connection in the "connecting"
    /// state.  Soon after (perhaps before this function returns, perhaps in another thread),
    /// the connection will begin sending signaling messages by calling
    /// ISteamNetworkingConnectionCustomSignaling::SendSignal.
    ///
    /// When the remote peer accepts the connection (See
    /// ISteamNetworkingCustomSignalingRecvContext::OnConnectRequest),
    /// it will begin sending signaling messages.  When these messages are received,
    /// you can pass them to the connection using ReceivedP2PCustomSignal.
    ///
    /// If you know the identity of the peer that you expect to be on the other end,
    /// you can pass their identity to improve debug output or just detect bugs.
    /// If you don't know their identity yet, you can pass NULL, and their
    /// identity will be established in the connection handshake.  
    ///
    /// If you use this, you probably want to call ISteamNetworkingUtils::InitRelayNetworkAccess()
    /// when your app initializes
    ///
    /// If you need to set any initial config options, pass them here.  See
    /// SteamNetworkingConfigValue_t for more about why this is preferable to
    /// setting the options "immediately" after creation.
    HSteamNetConnection ConnectP2PCustomSignaling( ISteamNetworkingConnectionCustomSignaling *pSignaling, const SteamNetworkingIdentity *pPeerIdentity, int nOptions, const SteamNetworkingConfigValue_t *pOptions );

    //HSteamNetConnection ConnectP2PCustomSignaling( ISteamNetworkingConnectionCustomSignaling *pSignaling, const SteamNetworkingIdentity *pPeerIdentity, int nRemoteVirtualPort, int nOptions, const SteamNetworkingConfigValue_t *pOptions )
    HSteamNetConnection ConnectP2PCustomSignaling( ISteamNetworkingConnectionSignaling *pSignaling, const SteamNetworkingIdentity *pPeerIdentity, int nRemoteVirtualPort, int nOptions, const SteamNetworkingConfigValue_t *pOptions );

    /// Called when custom signaling has received a message.  When your
    /// signaling channel receives a message, it should save off whatever
    /// routing information was in the envelope into the context object,
    /// and then pass the payload to this function.
    ///
    /// A few different things can happen next, depending on the message:
    ///
    /// - If the signal is associated with existing connection, it is dealt
    ///   with immediately.  If any replies need to be sent, they will be
    ///   dispatched using the ISteamNetworkingConnectionCustomSignaling
    ///   associated with the connection.
    /// - If the message represents a connection request (and the request
    ///   is not redundant for an existing connection), a new connection
    ///   will be created, and ReceivedConnectRequest will be called on your
    ///   context object to determine how to proceed.
    /// - Otherwise, the message is for a connection that does not
    ///   exist (anymore).  In this case, we *may* call SendRejectionReply
    ///   on your context object.
    ///
    /// In any case, we will not save off pContext or access it after this
    /// function returns.
    ///
    /// Returns true if the message was parsed and dispatched without anything
    /// unusual or suspicious happening.  Returns false if there was some problem
    /// with the message that prevented ordinary handling.  (Debug output will
    /// usually have more information.)
    ///
    /// If you expect to be using relayed connections, then you probably want
    /// to call ISteamNetworkingUtils::InitRelayNetworkAccess() when your app initializes
    bool ReceivedP2PCustomSignal( const void *pMsg, int cbMsg, ISteamNetworkingCustomSignalingRecvContext *pContext );

    bool ReceivedP2PCustomSignal( const void *pMsg, int cbMsg, ISteamNetworkingSignalingRecvContext *pContext );

    //
    // Certificate provision by the application.  On Steam, we normally handle all this automatically
    // and you will not need to use these advanced functions.
    //

    /// Get blob that describes a certificate request.  You can send this to your game coordinator.
    /// Upon entry, *pcbBlob should contain the size of the buffer.  On successful exit, it will
    /// return the number of bytes that were populated.  You can pass pBlob=NULL to query for the required
    /// size.  (256 bytes is a very conservative estimate.)
    ///
    /// Pass this blob to your game coordinator and call SteamDatagram_CreateCert.
    bool GetCertificateRequest( int *pcbBlob, void *pBlob, SteamNetworkingErrMsg &errMsg );

    /// Set the certificate.  The certificate blob should be the output of
    /// SteamDatagram_CreateCert.
    bool SetCertificate( const void *pCertificate, int cbCertificate, SteamNetworkingErrMsg &errMsg );

    /// Reset the identity associated with this instance.
    /// Any open connections are closed.  Any previous certificates, etc are discarded.
    /// You can pass a specific identity that you want to use, or you can pass NULL,
    /// in which case the identity will be invalid until you set it using SetCertificate
    ///
    /// NOTE: This function is not actually supported on Steam!  It is included
    ///       for use on other platforms where the active user can sign out and
    ///       a new user can sign in.
    void ResetIdentity( const SteamNetworkingIdentity *pIdentity );

    //
    // "FakeIP" system.
    //
    // A FakeIP is essentially a temporary, arbitrary identifier that
    // happens to be a valid IPv4 address.  The purpose of this system is to make it
    // easy to integrate with existing code that identifies hosts using IPv4 addresses.
    // The FakeIP address will never actually be used to send or receive any packets
    // on the Internet, it is strictly an identifier.
    //
    // FakeIP addresses are designed to (hopefully) pass through existing code as
    // transparently as possible, while conflicting with "real" addresses that might
    // be in use on networks (both the Internet and LANs) in the same code as little
    // as possible.  At the time this comment is being written, they come from the
    // 169.254.0.0/16 range, and the port number will always be >1024.  HOWEVER,
    // this is subject to change!  Do not make assumptions about these addresses,
    // or your code might break in the future.  In particular, you should use
    // functions such as  ISteamNetworkingUtils::IsFakeIP to determine if an IP
    // address is a "fake" one used by this system.
    //

    /// Begin asynchronous process of allocating a fake IPv4 address that other
    /// peers can use to contact us via P2P.  IP addresses returned by this
    /// function are globally unique for a given appid.
    ///
    /// nNumPorts is the numbers of ports you wish to reserve.  This is useful
    /// for the same reason that listening on multiple UDP ports is useful for
    /// different types of traffic.  Because these allocations come from a global
    /// namespace, there is a relatively strict limit on the maximum number of
    /// ports you may request.  (At the time of this writing, the limit is 4.)
    /// The Port assignments are *not* guaranteed to have any particular order
    /// or relationship!  Do *not* assume they are contiguous, even though that
    /// may often occur in practice.
    ///
    /// Returns false if a request was already in progress, true if a new request
    /// was started.  A SteamNetworkingFakeIPResult_t will be posted when the request
    /// completes.
    ///
    /// For gameservers, you *must* call this after initializing the SDK but before
    /// beginning login.  Steam needs to know in advance that FakeIP will be used.
    /// Everywhere your public IP would normally appear (such as the server browser) will be
    /// replaced by the FakeIP, and the fake port at index 0.  The request is actually queued
    /// until the logon completes, so you must not wait until the allocation completes
    /// before logging in.  Except for trivial failures that can be detected locally
    /// (e.g. invalid parameter), a SteamNetworkingFakeIPResult_t callback (whether success or
    /// failure) will not be posted until after we have logged in.  Furthermore, it is assumed
    /// that FakeIP allocation is essential for your application to function, and so failure
    /// will not be reported until *several* retries have been attempted.  This process may
    /// last several minutes.  It is *highly* recommended to treat failure as fatal.
    ///
    /// To communicate using a connection-oriented (TCP-style) API:
    /// - Server creates a listen socket using CreateListenSocketP2PFakeIP
    /// - Client connects using ConnectByIPAddress, passing in the FakeIP address.
    /// - The connection will behave mostly like a P2P connection.  The identities
    ///   that appear in SteamNetConnectionInfo_t will be the FakeIP identity until
    ///   we know the real identity.  Then it will be the real identity.  If the
    ///   SteamNetConnectionInfo_t::m_addrRemote is valid, it will be a real IPv4
    ///   address of a NAT-punched connection.  Otherwise, it will not be valid.
    /// 
    /// To communicate using an ad-hoc sendto/recv from (UDP-style) API,
    /// use CreateFakeUDPPort.
    bool BeginAsyncRequestFakeIP( int nNumPorts );

    /// Return info about the FakeIP and port(s) that we have been assigned,
    /// if any.  idxFirstPort is currently reserved and must be zero.
    /// Make sure and check SteamNetworkingFakeIPResult_t::m_eResult
    void GetFakeIP( int idxFirstPort, SteamNetworkingFakeIPResult_t *pInfo );

    /// Create a listen socket that will listen for P2P connections sent
    /// to our FakeIP.  A peer can initiate connections to this listen
    /// socket by calling ConnectByIPAddress.
    ///
    /// idxFakePort refers to the *index* of the fake port requested,
    /// not the actual port number.  For example, pass 0 to refer to the
    /// first port in the reservation.  You must call this only after calling
    /// BeginAsyncRequestFakeIP.  However, you do not need to wait for the
    /// request to complete before creating the listen socket.
    HSteamListenSocket CreateListenSocketP2PFakeIP( int idxFakePort, int nOptions, const SteamNetworkingConfigValue_t *pOptions );

    /// If the connection was initiated using the "FakeIP" system, then we
    /// we can get an IP address for the remote host.  If the remote host had
    /// a global FakeIP at the time the connection was established, this
    /// function will return that global IP.  Otherwise, a FakeIP that is
    /// unique locally will be allocated from the local FakeIP address space,
    /// and that will be returned.
    /// 
    /// The allocation of local FakeIPs attempts to assign addresses in
    /// a consistent manner.  If multiple connections are made to the
    /// same remote host, they *probably* will return the same FakeIP.
    /// However, since the namespace is limited, this cannot be guaranteed.
    ///
    /// On failure, returns:
    /// - k_EResultInvalidParam: invalid connection handle
    /// - k_EResultIPNotFound: This connection wasn't made using FakeIP system
    EResult GetRemoteFakeIPForConnection( HSteamNetConnection hConn, SteamNetworkingIPAddr *pOutAddr );

    /// Get an interface that can be used like a UDP port to send/receive
    /// datagrams to a FakeIP address.  This is intended to make it easy
    /// to port existing UDP-based code to take advantage of SDR.
    /// 
    /// idxFakeServerPort refers to the *index* of the port allocated using
    /// BeginAsyncRequestFakeIP and is used to create "server" ports.  You may
    /// call this before the allocation has completed.  However, any attempts
    /// to send packets will fail until the allocation has succeeded.  When
    /// the peer receives packets sent from this interface, the from address
    /// of the packet will be the globally-unique FakeIP.  If you call this
    /// function multiple times and pass the same (nonnegative) fake port index,
    /// the same object will be returned, and this object is not reference counted.
    /// 
    /// To create a "client" port (e.g. the equivalent of an ephemeral UDP port)
    /// pass -1.  In this case, a distinct object will be returned for each call.
    /// When the peer receives packets sent from this interface, the peer will
    /// assign a FakeIP from its own locally-controlled namespace.
    ISteamNetworkingFakeUDPPort *CreateFakeUDPPort( int idxFakeServerPort );

    // TEMP KLUDGE Call to invoke all queued callbacks.
    // Eventually this function will go away, and callwacks will be ordinary Steamworks callbacks.
    // You should call this at the same time you call SteamAPI_RunCallbacks and SteamGameServer_RunCallbacks
    // to minimize potential changes in timing when that change happens.
    void RunCallbacks( ISteamNetworkingSocketsCallbacks *pCallbacks );


    void RunCallbacks();

};

#endif // __INCLUDED_STEAM_NETWORKING_SOCKETS_H__
