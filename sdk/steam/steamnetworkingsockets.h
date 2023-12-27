//====== Copyright Valve Corporation, All rights reserved. ====================
//
// Kludge adapter that's used for the transition standalone library.
// This is not the opensource code, or the one in the Steamworks SDK.
//
//=============================================================================

#ifndef STEAMNETWORKINGSOCKETS
#define STEAMNETWORKINGSOCKETS
#ifdef _WIN32
#pragma once
#endif

#include "isteamnetworkingsockets.h"
struct SteamRelayNetworkStatus_t;
struct P2PSessionRequest_t;
struct P2PSessionConnectFail_t;

#if !defined( STEAMNETWORKINGSOCKETS_PARTNER ) && !defined( STEAMNETWORKINGSOCKETS_STREAMINGCLIENT )
	#error "Shouldn't be including this!"
#endif

#ifdef STEAMNETWORKINGSOCKETS_STEAM

// #KLUDGE! So we don't have to link with steam_api.lib
#include <steam/steam_api.h>
#include <steam/steam_gameserver.h>

/////////////////////////////////////////////////////////////////////////////
// Temp internal gross stuff you should ignore

	typedef void * ( S_CALLTYPE *FSteamInternal_CreateInterface )( const char *);
	typedef void ( S_CALLTYPE *FSteamAPI_RegisterCallback)( class CCallbackBase *pCallback, int iCallback );
	typedef void ( S_CALLTYPE *FSteamAPI_UnregisterCallback)( class CCallbackBase *pCallback );
	typedef void ( S_CALLTYPE *FSteamAPI_RegisterCallResult)( class CCallbackBase *pCallback, SteamAPICall_t hAPICall );
	typedef void ( S_CALLTYPE *FSteamAPI_UnregisterCallResult)( class CCallbackBase *pCallback, SteamAPICall_t hAPICall );
	STEAMNETWORKINGSOCKETS_INTERFACE void SteamDatagramClient_SetLauncher( const char *pszLauncher ); // Call this before SteamDatagramClient_Init
	STEAMNETWORKINGSOCKETS_INTERFACE void SteamDatagramClient_SetUniverse( EUniverse eUniverse ); // Call this before SteamDatagramClient_Init, to run without steam support
	STEAMNETWORKINGSOCKETS_INTERFACE void SteamDatagramClient_Internal_SteamAPIKludge( FSteamAPI_RegisterCallback fnRegisterCallback, FSteamAPI_UnregisterCallback fnUnregisterCallback, FSteamAPI_RegisterCallResult fnRegisterCallResult, FSteamAPI_UnregisterCallResult fnUnregisterCallResult );
	STEAMNETWORKINGSOCKETS_INTERFACE bool SteamDatagramClient_Init_InternalV9( bool bNoSteamSupport, SteamNetworkingErrMsg &errMsg, FSteamInternal_CreateInterface fnCreateInterface, HSteamUser hSteamUser, HSteamPipe hSteamPipe );
	STEAMNETWORKINGSOCKETS_INTERFACE bool SteamDatagramServer_Init_InternalV2( bool bNoSteamSupport, SteamNetworkingErrMsg &errMsg, FSteamInternal_CreateInterface fnCreateInterface, HSteamUser hSteamUser, HSteamPipe hSteamPipe );

/////////////////////////////////////////////////////////////////////////////

/// Initialize the user interface.
inline bool SteamDatagramClient_Init( bool bNoSteamSupport, SteamNetworkingErrMsg &errMsg )
{
	SteamDatagramClient_Internal_SteamAPIKludge( &::SteamAPI_RegisterCallback, &::SteamAPI_UnregisterCallback, &::SteamAPI_RegisterCallResult, &::SteamAPI_UnregisterCallResult );
	return SteamDatagramClient_Init_InternalV9( bNoSteamSupport, errMsg, ::SteamInternal_CreateInterface, ::SteamAPI_GetHSteamUser(), ::SteamAPI_GetHSteamPipe() );
}

/// Initialize the game server interface
inline bool SteamDatagramServer_Init( bool bNoSteamSupport, SteamNetworkingErrMsg &errMsg )
{
	SteamDatagramClient_Internal_SteamAPIKludge( &::SteamAPI_RegisterCallback, &::SteamAPI_UnregisterCallback, &::SteamAPI_RegisterCallResult, &::SteamAPI_UnregisterCallResult );
	return SteamDatagramServer_Init_InternalV2( bNoSteamSupport, errMsg, &SteamInternal_CreateInterface, ::SteamGameServer_GetHSteamUser(), ::SteamGameServer_GetHSteamPipe() );
}

#else // #ifdef STEAMNETWORKINGSOCKETS_STEAM

	/// Initialize client interface
	STEAMNETWORKINGSOCKETS_INTERFACE bool SteamDatagramClient_Init( bool /* ignored */, SteamNetworkingErrMsg &errMsg );

	/// Initialize gameserver interface
	STEAMNETWORKINGSOCKETS_INTERFACE bool SteamDatagramServer_Init( bool /* ignored */, SteamNetworkingErrMsg &errMsg );

#endif

/// Shutdown the client interface
STEAMNETWORKINGSOCKETS_INTERFACE void SteamDatagramClient_Kill();

/// Shutdown the game server interface
STEAMNETWORKINGSOCKETS_INTERFACE void SteamDatagramServer_Kill();

/// Call before initializing the library, to set the AppID.  This is only needed
/// when running without Steam support.  (Either on other platforms, or when
/// intentionally not using steam client.)
STEAMNETWORKINGSOCKETS_INTERFACE void SteamDatagramClient_SetAppID( AppId_t nAppID );

/// Manual polling mode.  You should call this before initialize the lib.
/// This will prevent the library from opening up its own service thread,
/// allowing you to pump sockets and stuff from your own thread.
STEAMNETWORKINGSOCKETS_INTERFACE void SteamNetworkingSockets_SetManualPollMode( bool bFlag );

/// If you call SteamNetworkingSockets_SetManualPollMode, then you need to
/// call this frequently.  Any time spent between calls is essentially
/// guaranteed to delay time-sensitive processing, so whatever you are
/// doing, make it quick.  If you pass a nonzero wait time, then this
/// function will sleep efficiently, waiting for incoming packets,
/// up to the maximum time you specify.  It may return prematurely
/// if packets arrive earlier than your timeout.
STEAMNETWORKINGSOCKETS_INTERFACE void SteamNetworkingSockets_Poll( int msMaxWaitTime );

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
	virtual void OnAuthenticationStatusChanged( SteamNetAuthenticationStatus_t * ) {}
	virtual void OnRelayNetworkStatusChanged( SteamRelayNetworkStatus_t * ) {}
protected:
	inline ~ISteamNetworkingSocketsCallbacks() {}
};

#endif // ISTEAMNETWORKINGSOCKETS
