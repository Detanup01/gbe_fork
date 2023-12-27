//====== Copyright Valve Corporation, All rights reserved. ====================
//
// Networking API similar to Berkeley sockets, but for games.
// - connection-oriented API (like TCP, not UDP)
// - but unlike TCP, it's message-oriented, not stream-oriented
// - mix of reliable and unreliable messages
// - fragmentation and reassembly
// - Supports connectivity over plain UDPv4
// - Also supports SDR ("Steam Datagram Relay") connections, which are
//   addressed by SteamID.  There is a "P2P" use case and also a "hosted
//   dedicated server" use case.
//
//=============================================================================

#ifndef ISTEAMNETWORKINGSOCKETS001
#define ISTEAMNETWORKINGSOCKETS001
#ifdef STEAM_WIN32
#pragma once
#endif

const int k_nSteamNetworkingSendFlags_NoNagle = 1;
const int k_nSteamNetworkingSendFlags_NoDelay = 2;
const int k_nSteamNetworkingSendFlags_Reliable = 8;

/// Different methods that messages can be sent
enum ESteamNetworkingSendType
{
	// Send an unreliable message. Can be lost.  Messages *can* be larger than a single MTU (UDP packet), but there is no
	// retransmission, so if any piece of the message is lost, the entire message will be dropped.
	//
	// The sending API does have some knowledge of the underlying connection, so if there is no NAT-traversal accomplished or
	// there is a recognized adjustment happening on the connection, the packet will be batched until the connection is open again.
	//
	// NOTE: By default, Nagle's algorithm is applied to all outbound packets.  This means that the message will NOT be sent
	//       immediately, in case further messages are sent soon after you send this, which can be grouped together.
	//       Any time there is enough buffered data to fill a packet, the packets will be pushed out immediately, but
	//       partially-full packets not be sent until the Nagle timer expires.
	//       See k_ESteamNetworkingSendType_UnreliableNoNagle, ISteamNetworkingSockets::FlushMessagesOnConnection,
	//       ISteamNetworkingP2P::FlushMessagesToUser
	//
	// This is not exactly the same as k_EP2PSendUnreliable!  You probably want k_ESteamNetworkingSendType_UnreliableNoNagle
	k_ESteamNetworkingSendType_Unreliable = 0,

	// Send a message unreliably, bypassing Nagle's algorithm for this message and any messages currently pending on the Nagle timer.
	// This is equivalent to using k_ESteamNetworkingSendType_Unreliable,
	// and then immediately flushing the messages using ISteamNetworkingSockets::FlushMessagesOnConnection or ISteamNetworkingP2P::FlushMessagesToUser.
	// (But this is more efficient.)
	k_ESteamNetworkingSendType_UnreliableNoNagle = k_nSteamNetworkingSendFlags_NoNagle,

	// Send an unreliable message, but do not buffer it if it cannot be sent relatively quickly.
	// This is useful for messages that are not useful if they are excessively delayed, such as voice data.
	// The Nagle algorithm is not used, and if the message is not dropped, any messages waiting on the Nagle timer
	// are immediately flushed.
	//
	// A message will be dropped under the following circumstances:
	// - the connection is not fully connected.  (E.g. the "Connecting" or "FindingRoute" states)
	// - there is a sufficiently large number of messages queued up already such that the current message
	//   will not be placed on the wire in the next ~200ms or so.
	//
	// if a message is dropped for these reasons, k_EResultIgnored will be returned.
	k_ESteamNetworkingSendType_UnreliableNoDelay = k_nSteamNetworkingSendFlags_NoDelay|k_nSteamNetworkingSendFlags_NoNagle,

	// Reliable message send. Can send up to 512kb of data in a single message. 
	// Does fragmentation/re-assembly of messages under the hood, as well as a sliding window for
	// efficient sends of large chunks of data.
	//
	// The Nagle algorithm is used.  See notes on k_ESteamNetworkingSendType_Unreliable for more details.
	// See k_ESteamNetworkingSendType_ReliableNoNagle, ISteamNetworkingSockets::FlushMessagesOnConnection,
	// ISteamNetworkingP2P::FlushMessagesToUser
	//
	// This is NOT the same as k_EP2PSendReliable, it's more like k_EP2PSendReliableWithBuffering
	k_ESteamNetworkingSendType_Reliable = k_nSteamNetworkingSendFlags_Reliable,

	// Send a message reliably, but bypass Nagle's algorithm.
	// See k_ESteamNetworkingSendType_UnreliableNoNagle for more info.
	//
	// This is equivalent to k_EP2PSendReliable
	k_ESteamNetworkingSendType_ReliableNoNagle = k_nSteamNetworkingSendFlags_Reliable|k_nSteamNetworkingSendFlags_NoNagle,
};

/// Configuration values for Steam networking. 
///  
/// Most of these are for controlling extend logging or features 
/// of various subsystems 
enum ESteamNetworkingConfigurationValue
{
	/// 0-100 Randomly discard N pct of unreliable messages instead of sending
	/// Defaults to 0 (no loss).
	k_ESteamNetworkingConfigurationValue_FakeMessageLoss_Send = 0,

	/// 0-100 Randomly discard N pct of unreliable messages upon receive
	/// Defaults to 0 (no loss).
	k_ESteamNetworkingConfigurationValue_FakeMessageLoss_Recv = 1,

	/// 0-100 Randomly discard N pct of packets instead of sending
	k_ESteamNetworkingConfigurationValue_FakePacketLoss_Send = 2,

	/// 0-100 Randomly discard N pct of packets received
	k_ESteamNetworkingConfigurationValue_FakePacketLoss_Recv = 3,

	/// Globally delay all outbound packets by N ms before sending
	k_ESteamNetworkingConfigurationValue_FakePacketLag_Send = 4,

	/// Globally delay all received packets by N ms before processing
	k_ESteamNetworkingConfigurationValue_FakePacketLag_Recv = 5,

	/// Globally reorder some percentage of packets we send
	k_ESteamNetworkingConfigurationValue_FakePacketReorder_Send = 6,

	/// Globally reorder some percentage of packets we receive
	k_ESteamNetworkingConfigurationValue_FakePacketReorder_Recv = 7,

	/// Amount of delay, in ms, to apply to reordered packets.
	k_ESteamNetworkingConfigurationValue_FakePacketReorder_Time = 8,

	/// Upper limit of buffered pending bytes to be sent, if this is reached
	/// SendMessage will return k_EResultLimitExceeded
	/// Default is 512k (524288 bytes)
	k_ESteamNetworkingConfigurationValue_SendBufferSize = 9,

	/// Maximum send rate clamp, 0 is no limit
	/// This value will control the maximum allowed sending rate that congestion 
	/// is allowed to reach.  Default is 0 (no-limit)
	k_ESteamNetworkingConfigurationValue_MaxRate = 10,

	/// Minimum send rate clamp, 0 is no limit
	/// This value will control the minimum allowed sending rate that congestion 
	/// is allowed to reach.  Default is 0 (no-limit)
	k_ESteamNetworkingConfigurationValue_MinRate = 11,

	/// Set the nagle timer.  When SendMessage is called, if the outgoing message 
	/// is less than the size of the MTU, it will be queued for a delay equal to 
	/// the Nagle timer value.  This is to ensure that if the application sends
	/// several small messages rapidly, they are coalesced into a single packet.
	/// See historical RFC 896.  Value is in microseconds. 
	/// Default is 5000us (5ms).
	k_ESteamNetworkingConfigurationValue_Nagle_Time = 12,

	/// Set to true (non-zero) to enable logging of RTT's based on acks.
	/// This doesn't track all sources of RTT, just the inline ones based
	/// on acks, but those are the most common
	k_ESteamNetworkingConfigurationValue_LogLevel_AckRTT = 13,

	/// Log level of SNP packet decoding
	k_ESteamNetworkingConfigurationValue_LogLevel_Packet = 14,

	/// Log when messages are sent/received
	k_ESteamNetworkingConfigurationValue_LogLevel_Message = 15,

	/// Log level when individual packets drop
	k_ESteamNetworkingConfigurationValue_LogLevel_PacketGaps = 16,

	/// Log level for P2P rendezvous.
	k_ESteamNetworkingConfigurationValue_LogLevel_P2PRendezvous = 17,

	/// Log level for sending and receiving pings to relays
	k_ESteamNetworkingConfigurationValue_LogLevel_RelayPings = 18,

	/// If the first N pings to a port all fail, mark that port as unavailable for
	/// a while, and try a different one.  Some ISPs and routers may drop the first
	/// packet, so setting this to 1 may greatly disrupt communications.
	k_ESteamNetworkingConfigurationValue_ClientConsecutitivePingTimeoutsFailInitial = 19,

	/// If N consecutive pings to a port fail, after having received successful 
	/// communication, mark that port as unavailable for a while, and try a 
	/// different one.
	k_ESteamNetworkingConfigurationValue_ClientConsecutitivePingTimeoutsFail = 20,

	/// Minimum number of lifetime pings we need to send, before we think our estimate
	/// is solid.  The first ping to each cluster is very often delayed because of NAT,
	/// routers not having the best route, etc.  Until we've sent a sufficient number
	/// of pings, our estimate is often inaccurate.  Keep pinging until we get this
	/// many pings.
	k_ESteamNetworkingConfigurationValue_ClientMinPingsBeforePingAccurate = 21,

	/// Set all steam datagram traffic to originate from the same local port.  
	/// By default, we open up a new UDP socket (on a different local port) 
	/// for each relay.  This is not optimal, but it works around some 
	/// routers that don't implement NAT properly.  If you have intermittent 
	/// problems talking to relays that might be NAT related, try toggling 
	/// this flag
	k_ESteamNetworkingConfigurationValue_ClientSingleSocket = 22,

	/// Don't automatically fail IP connections that don't have strong auth.
	/// On clients, this means we will attempt the connection even if we don't
	/// know our SteamID or can't get a cert.  On the server, it means that we won't
	/// automatically reject a connection due to a failure to authenticate.
	/// (You can examine the incoming connection and decide whether to accept it.)
	k_ESteamNetworkingConfigurationValue_IP_Allow_Without_Auth = 23,

	/// Timeout value (in seconds) to use when first connecting
	k_ESteamNetworkingConfigurationValue_Timeout_Seconds_Initial = 24,

	/// Timeout value (in seconds) to use after connection is established
	k_ESteamNetworkingConfigurationValue_Timeout_Seconds_Connected = 25,

	/// Number of k_ESteamNetworkingConfigurationValue defines
	k_ESteamNetworkingConfigurationValue_Count,
};

/// Configuration strings for Steam networking. 
///  
/// Most of these are for controlling extend logging or features 
/// of various subsystems 
enum ESteamNetworkingConfigurationString
{
	// Code of relay cluster to use.  If not empty, we will only use relays in that cluster.  E.g. 'iad'
	k_ESteamNetworkingConfigurationString_ClientForceRelayCluster = 0,

	// For debugging, generate our own (unsigned) ticket, using the specified 
	// gameserver address.  Router must be configured to accept unsigned tickets.
	k_ESteamNetworkingConfigurationString_ClientDebugTicketAddress = 1,

	// For debugging.  Override list of relays from the config with this set
	// (maybe just one).  Comma-separated list.
	k_ESteamNetworkingConfigurationString_ClientForceProxyAddr = 2,

	// Number of k_ESteamNetworkingConfigurationString defines
	k_ESteamNetworkingConfigurationString_Count = k_ESteamNetworkingConfigurationString_ClientForceProxyAddr + 1,
};

/// Configuration values for Steam networking per connection
enum ESteamNetworkingConnectionConfigurationValue
{
	// Maximum send rate clamp, 0 is no limit
	// This value will control the maximum allowed sending rate that congestion 
	// is allowed to reach.  Default is 0 (no-limit)
	k_ESteamNetworkingConnectionConfigurationValue_SNP_MaxRate = 0,

	// Minimum send rate clamp, 0 is no limit
	// This value will control the minimum allowed sending rate that congestion 
	// is allowed to reach.  Default is 0 (no-limit)
	k_ESteamNetworkingConnectionConfigurationValue_SNP_MinRate = 1,

	// Number of k_ESteamNetworkingConfigurationValue defines
	k_ESteamNetworkingConnectionConfigurationValue_Count,
};

/// Message that has been received
typedef struct _SteamNetworkingMessage001_t
{

	/// SteamID that sent this to us.
	CSteamID m_steamIDSender;

	/// The user data associated with the connection.
	///
	/// This is *usually* the same as calling GetConnection() and then
	/// fetching the user data associated with that connection, but for
	/// the following subtle differences:
	///
	/// - This user data will match the connection's user data at the time
	///   is captured at the time the message is returned by the API.
	///   If you subsequently change the userdata on the connection,
	///   this won't be updated.
	/// - This is an inline call, so it's *much* faster.
	/// - You might have closed the connection, so fetching the user data
	///   would not be possible.
	int64 m_nConnUserData;

	/// Local timestamps when it was received
	SteamNetworkingMicroseconds m_usecTimeReceived;

	/// Message number assigned by the sender
	int64 m_nMessageNumber;

	/// Function used to clean up this object.  Normally you won't call
	/// this directly, use Release() instead.
	void (*m_pfnRelease)( struct _SteamNetworkingMessage001_t *msg );

	/// Message payload
	void *m_pData;

	/// Size of the payload.
	uint32 m_cbSize;

	/// The connection this came from.  (Not used when using the P2P calls)
	HSteamNetConnection m_conn;

	/// The channel number the message was received on.
	/// (Not used for messages received on "connections")
	int m_nChannel;

	/// Pad to multiple of 8 bytes
	int m___nPadDummy;

	#ifdef __cplusplus

		/// You MUST call this when you're done with the object,
		/// to free up memory, etc.
		inline void Release()
		{
			m_pfnRelease( this );
		}

		// For code compatibility, some accessors
		inline uint32 GetSize() const { return m_cbSize; }
		inline const void *GetData() const { return m_pData; }
		inline CSteamID GetSenderSteamID() const { return m_steamIDSender; }
		inline int GetChannel() const { return m_nChannel; }
		inline HSteamNetConnection GetConnection() const { return m_conn; }
		inline int64 GetConnectionUserData() const { return m_nConnUserData; }
		inline SteamNetworkingMicroseconds GetTimeReceived() const { return m_usecTimeReceived; }
		inline int64 GetMessageNumber() const { return m_nMessageNumber; }
	#endif
} SteamNetworkingMessage001_t;

// For code compatibility
typedef SteamNetworkingMessage001_t ISteamNetworkingMessage001;

struct SteamNetConnectionInfo001_t
{

	/// Who is on the other end.  Depending on the connection type and phase of the connection, we might not know
	CSteamID m_steamIDRemote;

	// FIXME - some sort of connection type enum?

	/// Arbitrary user data set by the local application code
	int64 m_nUserData;

	/// Handle to listen socket this was connected on, or k_HSteamListenSocket_Invalid if we initiated the connection
	HSteamListenSocket m_hListenSocket;

	/// Remote address.  Might be 0 if we don't know it
	uint32 m_unIPRemote;
	uint16 m_unPortRemote;
	uint16 m__pad1;

	/// What data center is the remote host in?  (0 if we don't know.)
	SteamNetworkingPOPID m_idPOPRemote;

	/// What relay are we using to communicate with the remote host?
	/// (0 if not applicable.)
	SteamNetworkingPOPID m_idPOPRelay;

	/// Local port that we're bound to for this connection.  Might not be applicable
	/// for all connection types.
	//uint16 m_unPortLocal;

	/// High level state of the connection
	int /* ESteamNetworkingConnectionState */ m_eState;

	/// Basic cause of the connection termination or problem.
	/// One of ESteamNetConnectionEnd
	int /* ESteamNetConnectionEnd */ m_eEndReason;

	/// Human-readable, but non-localized explanation for connection
	/// termination or problem.  This is intended for debugging /
	/// diagnostic purposes only, not to display to users.  It might
	/// have some details specific to the issue.
	char m_szEndDebug[ k_cchSteamNetworkingMaxConnectionCloseReason ];
};

/// TEMP callback dispatch mechanism.
/// You'll override this guy and hook any callbacks you are interested in,
/// and then use ISteamNetworkingSockets::RunCallbacks.  Eventually this will go away,
/// and you will register for the callbacks you want using the normal SteamWorks callback
/// mechanisms, and they will get dispatched along with other Steamworks callbacks
/// when you call SteamAPI_RunCallbacks and SteamGameServer_RunCallbacks.
class ISteamNetworkingSocketsCallbacks
{
public:
	inline ISteamNetworkingSocketsCallbacks() {}
	virtual void OnSteamNetConnectionStatusChanged( SteamNetConnectionStatusChangedCallback_t * ) {}
	virtual void OnP2PSessionRequest( P2PSessionRequest_t * ) {}
	virtual void OnP2PSessionConnectFail( P2PSessionConnectFail_t * ) {}
protected:
	inline ~ISteamNetworkingSocketsCallbacks() {}
};


//-----------------------------------------------------------------------------
/// Lower level networking interface that more closely mirrors the standard
/// Berkeley sockets model.  Sockets are hard!  You should probably only use
/// this interface under the existing circumstances:
///
/// - You have an existing socket-based codebase you want to port, or coexist with.
/// - You want to be able to connect based on IP address, rather than (just) Steam ID.
/// - You need low-level control of bandwidth utilization, when to drop packets, etc.
///
/// Note that neither of the terms "connection" and "socket" will correspond
/// one-to-one with an underlying UDP socket.  An attempt has been made to
/// keep the semantics as similar to the standard socket model when appropriate,
/// but some deviations do exist.
class ISteamNetworkingSockets001
{
public:

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
	virtual HSteamListenSocket CreateListenSocket( int nSteamConnectVirtualPort, uint32 nIP, uint16 nPort ) = 0;

	/// Creates a connection and begins talking to a remote destination.  The remote host
	/// must be listening with a matching call to CreateListenSocket.
	///
	/// Use ConnectBySteamID to connect using the SteamID (client or game server) as the network address.
	/// Use ConnectByIPv4Address to connect by IP address.
	///
	/// A SteamNetConnectionStatusChangedCallback_t callback will be triggered when we start connecting,
	/// and then another one on timeout or successful connection
//#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE
	virtual HSteamNetConnection ConnectBySteamID( CSteamID steamIDTarget, int nVirtualPort ) = 0;
//#endif
	virtual HSteamNetConnection ConnectByIPv4Address( uint32 nIP, uint16 nPort ) = 0;

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
	virtual EResult AcceptConnection( HSteamNetConnection hConn ) = 0;

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
	virtual bool CloseConnection( HSteamNetConnection hPeer, int nReason, const char *pszDebug, bool bEnableLinger ) = 0;

	/// Destroy a listen socket, and all the client sockets generated by accepting connections
	/// on the listen socket.
	///
	/// pszNotifyRemoteReason determines what cleanup actions are performed on the client
	/// sockets being destroyed.  (See DestroySocket for more details.)
	///
	/// Note that if cleanup is requested and you have requested the listen socket bound to a
	/// particular local port to facilitate direct UDP/IPv4 connections, then the underlying UDP
	/// socket must remain open until all clients have been cleaned up.
	virtual bool CloseListenSocket( HSteamListenSocket hSocket, const char *pszNotifyRemoteReason ) = 0;

	/// Set connection user data.  Returns false if the handle is invalid.
	virtual bool SetConnectionUserData( HSteamNetConnection hPeer, int64 nUserData ) = 0;

	/// Fetch connection user data.  Returns -1 if handle is invalid
	/// or if you haven't set any userdata on the connection.
	virtual int64 GetConnectionUserData( HSteamNetConnection hPeer ) = 0;

	/// Set a name for the connection, used mostly for debugging
	virtual void SetConnectionName( HSteamNetConnection hPeer, const char *pszName ) = 0;

	/// Fetch connection name.  Returns false if handle is invalid
	virtual bool GetConnectionName( HSteamNetConnection hPeer, char *pszName, int nMaxLen ) = 0;

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
	virtual EResult SendMessageToConnection( HSteamNetConnection hConn, const void *pData, uint32 cbData, ESteamNetworkingSendType eSendType ) = 0;

	/// If Nagle is enabled (its on by default) then when calling 
	/// SendMessageToConnection the message will be queued up the Nagle time
	/// before being sent to merge small messages into the same packet.
	///
	/// Call this function to flush any queued messages and send them immediately
	/// on the next transmission time (often that means right now).
	virtual EResult FlushMessagesOnConnection( HSteamNetConnection hConn ) = 0;

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
	virtual int ReceiveMessagesOnConnection( HSteamNetConnection hConn, SteamNetworkingMessage001_t **ppOutMessages, int nMaxMessages ) = 0; 

	/// Same as ReceiveMessagesOnConnection, but will return the next message available
	/// on any client socket that was accepted through the specified listen socket.  Examine
	/// SteamNetworkingMessage_t::m_conn to know which client connection.
	///
	/// Delivery order of messages among different clients is not defined.  They may
	/// be returned in an order different from what they were actually received.  (Delivery
	/// order of messages from the same client is well defined, and thus the order of the
	/// messages is relevant!)
	virtual int ReceiveMessagesOnListenSocket( HSteamListenSocket hSocket, SteamNetworkingMessage001_t **ppOutMessages, int nMaxMessages ) = 0; 

	/// Returns information about the specified connection.
	virtual bool GetConnectionInfo( HSteamNetConnection hConn, SteamNetConnectionInfo001_t *pInfo ) = 0;

	/// Returns brief set of connection status that you might want to display
	/// to the user in game.
	virtual bool GetQuickConnectionStatus( HSteamNetConnection hConn, SteamNetworkingQuickConnectionStatus *pStats ) = 0;

	/// Returns detailed connection stats in text format.  Useful
	/// for dumping to a log, etc.
	///
	/// Returns:
	/// -1 failure (bad connection handle)
	/// 0 OK, your buffer was filled in and '\0'-terminated
	/// >0 Your buffer was either nullptr, or it was too small and the text got truncated.  Try again with a buffer of at least N bytes.
	virtual int GetDetailedConnectionStatus( HSteamNetConnection hConn, char *pszBuf, int cbBuf ) = 0;

	/// Returns information about the listen socket.
	///
	/// *pnIP and *pnPort will be 0 if the socket is set to listen for connections based
	/// on SteamID only.  If your listen socket accepts connections on IPv4, then both
	/// fields will return nonzero, even if you originally passed a zero IP.  However,
	/// note that the address returned may be a private address (e.g. 10.0.0.x or 192.168.x.x),
	/// and may not be reachable by a general host on the Internet.
	virtual bool GetListenSocketInfo( HSteamListenSocket hSocket, uint32 *pnIP, uint16 *pnPort ) = 0;

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
	virtual bool CreateSocketPair( HSteamNetConnection *pOutConnection1, HSteamNetConnection *pOutConnection2, bool bUseNetworkLoopback ) = 0;

//#ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE

	//
	// Clients connecting to dedicated servers hosted in a data center,
	// using central-authority-granted tickets.
	//

	/// Called when we receive a ticket from our central matchmaking system.  Puts the
	/// ticket into a persistent cache, and optionally returns the parsed ticket.
	///
	/// See stamdatagram_ticketgen.h for more details.
	virtual bool ReceivedRelayAuthTicket( const void *pvTicket, int cbTicket, SteamDatagramRelayAuthTicket *pOutParsedTicket ) = 0;

	/// Search cache for a ticket to talk to the server on the specified virtual port.
	/// If found, returns the number of second until the ticket expires, and optionally
	/// the complete cracked ticket.  Returns 0 if we don't have a ticket.
	///
	/// Typically this is useful just to confirm that you have a ticket, before you
	/// call ConnectToHostedDedicatedServer to connect to the server.
	virtual int FindRelayAuthTicketForServer( CSteamID steamID, int nVirtualPort, SteamDatagramRelayAuthTicket *pOutParsedTicket ) = 0;

	/// Client call to connect to a server hosted in a Valve data center, on the specified virtual
	/// port.  You should have received a ticket for this server, or else this connect call will fail!
	///
	/// You may wonder why tickets are stored in a cache, instead of simply being passed as an argument
	/// here.  The reason is to make reconnection to a gameserver robust, even if the client computer loses
	/// connection to Steam or the central backend, or the app is restarted or crashes, etc.
	virtual HSteamNetConnection ConnectToHostedDedicatedServer( CSteamID steamIDTarget, int nVirtualPort ) = 0;

	//
	// Servers hosted in Valve data centers
	//

	/// Returns the value of the SDR_LISTEN_PORT environment variable.
	virtual uint16 GetHostedDedicatedServerPort() = 0;

	/// If you are running in a production data center, this will return the data
	/// center code.  Returns 0 otherwise.
	virtual SteamNetworkingPOPID GetHostedDedicatedServerPOPID() = 0;

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
	virtual bool GetHostedDedicatedServerAddress001( SteamDatagramHostedAddress *pRouting ) = 0;

	/// Create a listen socket on the specified virtual port.  The physical UDP port to use
	/// will be determined by the SDR_LISTEN_PORT environment variable.  If a UDP port is not
	/// configured, this call will fail.
	///
	/// Note that this call MUST be made through the SteamNetworkingSocketsGameServer() interface
	virtual HSteamListenSocket CreateHostedDedicatedServerListenSocket( int nVirtualPort ) = 0;

//#endif // #ifndef STEAMNETWORKINGSOCKETS_OPENSOURCE

	//
	// Gets some debug text from the connection
	//
	virtual bool GetConnectionDebugText( HSteamNetConnection hConn, char *pOut, int nOutCCH ) = 0;

	//
	// Set and get configuration values, see ESteamNetworkingConfigurationValue for individual descriptions.
	//
	// Returns the value or -1 is eConfigValue is invalid
	virtual int32 GetConfigurationValue( ESteamNetworkingConfigurationValue eConfigValue ) = 0;
	// Returns true if successfully set
	virtual bool SetConfigurationValue( ESteamNetworkingConfigurationValue eConfigValue, int32 nValue ) = 0;

	// Return the name of an int configuration value, or NULL if config value isn't known
	virtual const char *GetConfigurationValueName( ESteamNetworkingConfigurationValue eConfigValue ) = 0;

	//
	// Set and get configuration strings, see ESteamNetworkingConfigurationString for individual descriptions.
	//
	// Get the configuration string, returns length of string needed if pDest is nullpr or destSize is 0
	// returns -1 if the eConfigValue is invalid
	virtual int32 GetConfigurationString( ESteamNetworkingConfigurationString eConfigString, char *pDest, int32 destSize ) = 0;
	virtual bool SetConfigurationString( ESteamNetworkingConfigurationString eConfigString, const char *pString ) = 0;

	// Return the name of a string configuration value, or NULL if config value isn't known
	virtual const char *GetConfigurationStringName( ESteamNetworkingConfigurationString eConfigString ) = 0;

	//
	// Set and get configuration values, see ESteamNetworkingConnectionConfigurationValue for individual descriptions.
	//
	// Returns the value or -1 is eConfigValue is invalid
	virtual int32 GetConnectionConfigurationValue( HSteamNetConnection hConn, ESteamNetworkingConnectionConfigurationValue eConfigValue ) = 0;
	// Returns true if successfully set
	virtual bool SetConnectionConfigurationValue( HSteamNetConnection hConn, ESteamNetworkingConnectionConfigurationValue eConfigValue, int32 nValue ) = 0;

	// TEMP KLUDGE Call to invoke all queued callbacks.
	// Eventually this function will go away, and callwacks will be ordinary Steamworks callbacks.
	// You should call this at the same time you call SteamAPI_RunCallbacks and SteamGameServer_RunCallbacks
	// to minimize potential changes in timing when that change happens.
	virtual void RunCallbacks( ISteamNetworkingSocketsCallbacks *pCallbacks ) = 0;
protected:
	//~ISteamNetworkingSockets001(); // Silence some warnings
};
//#define STEAMNETWORKINGSOCKETS_VERSION "SteamNetworkingSockets001"


#endif // ISTEAMNETWORKINGSOCKETS001
