//====== Copyright Valve Corporation, All rights reserved. ====================
//
// This header includes *all* of the interfaces and callback structures
// in the Steamworks SDK, and some high level functions to control the SDK
// (init, shutdown, etc) that you probably only need in one or two files.
//
// To save your compile times, we recommend that you not include this file
// in header files.  Instead, include the specific headers for the interfaces
// and callback structures you need.  The one file you might consider including
// in your precompiled header (e.g. stdafx.h) is steam_api_common.h
//
//=============================================================================

#ifndef STEAM_API_H
#define STEAM_API_H
#ifdef STEAM_WIN32
#pragma once
#endif

// Basic stuff
#include "steam_api_common.h"

// All of the interfaces
#include "isteamappdisableupdate.h"
#include "isteambilling.h"
#include "isteamclient.h"
#include "isteamclient006.h"
#include "isteamclient007.h"
#include "isteamclient008.h"
#include "isteamclient009.h"
#include "isteamclient010.h"
#include "isteamclient011.h"
#include "isteamclient012.h"
#include "isteamclient013.h"
#include "isteamclient014.h"
#include "isteamclient015.h"
#include "isteamclient016.h"
#include "isteamclient017.h"
#include "isteamclient018.h"
#include "isteamclient019.h"
#include "isteamclient020.h"
#include "isteamuser.h"
#include "isteamuser004.h"
#include "isteamuser005.h"
#include "isteamuser006.h"
#include "isteamuser007.h"
#include "isteamuser008.h"
#include "isteamuser009.h"
#include "isteamuser010.h"
#include "isteamuser011.h"
#include "isteamuser012.h"
#include "isteamuser013.h"
#include "isteamuser014.h"
#include "isteamuser015.h"
#include "isteamuser016.h"
#include "isteamuser017.h"
#include "isteamuser018.h"
#include "isteamuser019.h"
#include "isteamuser020.h"
#include "isteamuser021.h"
#include "isteamuser022.h"
#include "isteamfriends.h"
#include "isteamfriends001.h"
#include "isteamfriends002.h"
#include "isteamfriends003.h"
#include "isteamfriends004.h"
#include "isteamfriends005.h"
#include "isteamfriends006.h"
#include "isteamfriends007.h"
#include "isteamfriends008.h"
#include "isteamfriends009.h"
#include "isteamfriends010.h"
#include "isteamfriends011.h"
#include "isteamfriends012.h"
#include "isteamfriends013.h"
#include "isteamfriends014.h"
#include "isteamfriends015.h"
#include "isteamfriends016.h"
#include "isteamutils.h"
#include "isteamutils001.h"
#include "isteamutils002.h"
#include "isteamutils003.h"
#include "isteamutils004.h"
#include "isteamutils005.h"
#include "isteamutils006.h"
#include "isteamutils007.h"
#include "isteamutils008.h"
#include "isteamutils009.h"
#include "isteammatchmaking.h"
#include "isteammatchmaking001.h"
#include "isteammatchmaking002.h"
#include "isteammatchmaking003.h"
#include "isteammatchmaking004.h"
#include "isteammatchmaking005.h"
#include "isteammatchmaking006.h"
#include "isteammatchmaking007.h"
#include "isteammatchmaking008.h"
#include "isteamuserstats.h"
#include "isteamuserstats012.h"
#include "isteamuserstats011.h"
#include "isteamuserstats010.h"
#include "isteamuserstats009.h"
#include "isteamuserstats008.h"
#include "isteamuserstats007.h"
#include "isteamuserstats006.h"
#include "isteamuserstats005.h"
#include "isteamuserstats004.h"
#include "isteamuserstats003.h"
#include "isteamuserstats002.h"
#include "isteamuserstats001.h"
#include "isteamapps.h"
#include "isteamapps007.h"
#include "isteamapps006.h"
#include "isteamapps005.h"
#include "isteamapps004.h"
#include "isteamapps003.h"
#include "isteamapps002.h"
#include "isteamapps001.h"
#include "isteamnetworking.h"
#include "isteamnetworking005.h"
#include "isteamnetworking004.h"
#include "isteamnetworking003.h"
#include "isteamnetworking002.h"
#include "isteamnetworking001.h"
#include "isteamnetworkingsockets.h"
#include "isteamnetworkingsocketsserialized.h"
#include "isteamnetworkingutils.h"
#include "isteamnetworkingutils001.h"
#include "isteamnetworkingutils002.h"
#include "isteamnetworkingutils003.h"
#include "isteamnetworkingsockets001.h"
#include "isteamnetworkingsockets002.h"
#include "isteamnetworkingsockets003.h"
#include "isteamnetworkingsockets004.h"
#include "isteamnetworkingsockets006.h"
#include "isteamnetworkingsockets008.h"
#include "isteamnetworkingsockets009.h"
#include "isteamnetworkingsockets010.h"
#include "isteamnetworkingsockets011.h"
#include "isteamremotestorage.h"
#include "isteamremotestorage001.h"
#include "isteamremotestorage002.h"
#include "isteamremotestorage003.h"
#include "isteamremotestorage004.h"
#include "isteamremotestorage005.h"
#include "isteamremotestorage006.h"
#include "isteamremotestorage007.h"
#include "isteamremotestorage008.h"
#include "isteamremotestorage009.h"
#include "isteamremotestorage010.h"
#include "isteamremotestorage011.h"
#include "isteamremotestorage012.h"
#include "isteamremotestorage013.h"
#include "isteamremotestorage014.h"
#include "isteamremotestorage015.h"
#include "isteamscreenshots.h"
#include "isteamscreenshots001.h"
#include "isteamscreenshots002.h"
#include "isteammusic.h"
#include "isteammusicremote.h"
#include "isteamhttp.h"
#include "isteamhttp001.h"
#include "isteamhttp002.h"
#include "isteamcontroller.h"
#include "isteamcontroller001.h"
#include "isteamcontroller003.h"
#include "isteamcontroller004.h"
#include "isteamcontroller005.h"
#include "isteamcontroller006.h"
#include "isteamcontroller007.h"
#include "isteamugc.h"
#include "isteamugc001.h"
#include "isteamugc002.h"
#include "isteamugc003.h"
#include "isteamugc004.h"
#include "isteamugc005.h"
#include "isteamugc006.h"
#include "isteamugc007.h"
#include "isteamugc008.h"
#include "isteamugc009.h"
#include "isteamugc010.h"
#include "isteamugc011.h"
#include "isteamugc012.h"
#include "isteamugc013.h"
#include "isteamugc014.h"
#include "isteamugc015.h"
#include "isteamugc016.h"
#include "isteamugc017.h"
#include "isteamugc018.h"
#include "isteamugc019.h"
#include "isteamapplist.h"
#include "isteamhtmlsurface.h"
#include "isteamhtmlsurface001.h"
#include "isteamhtmlsurface002.h"
#include "isteamhtmlsurface003.h"
#include "isteamhtmlsurface004.h"
#include "isteaminventory.h"
#include "isteaminventory001.h"
#include "isteaminventory002.h"
#include "isteamtimeline.h"
#include "isteamtimeline003.h"
#include "isteamtimeline002.h"
#include "isteamtimeline001.h"
#include "isteamvideo.h"
#include "isteamvideo001.h"
#include "isteamvideo002.h"
#include "isteamparentalsettings.h"
#include "isteamgamecoordinator.h"
#include "isteammasterserverupdater.h"
#include "isteamunifiedmessages.h"
#include "isteaminput.h"
#include "isteaminput001.h"
#include "isteaminput002.h"
#include "isteaminput005.h"
#include "isteamremoteplay.h"
#include "isteamremoteplay001.h"
#include "isteamnetworkingmessages.h"
#include "isteamnetworkingsockets.h"
#include "isteamnetworkingutils.h"
#include "isteamtv.h"
#include "steamnetworkingfakeip.h"
#include "isteamgameserver.h"
#include "isteamgameserver014.h"
#include "isteamgameserver013.h"
#include "isteamgameserver012.h"
#include "isteamgameserver011.h"
#include "isteamgameserver010.h"
#include "isteamgameserver009.h"
#include "isteamgameserver008.h"
#include "isteamgameserver005.h"
#include "isteamgameserver004.h"
#include "isteamgameserver003.h"
#include "isteamgameserver002.h"
#include "isteamgameserverstats.h"
#include "isteamgamestats.h"


//----------------------------------------------------------------------------------------------------------------------------------------------------------//
//	Steam API setup & shutdown
//
//	These functions manage loading, initializing and shutdown of the steamclient.dll
//
//----------------------------------------------------------------------------------------------------------------------------------------------------------//

enum ESteamAPIInitResult
{
	k_ESteamAPIInitResult_OK = 0,
	k_ESteamAPIInitResult_FailedGeneric = 1, // Some other failure
	k_ESteamAPIInitResult_NoSteamClient = 2, // We cannot connect to Steam, steam probably isn't running
	k_ESteamAPIInitResult_VersionMismatch = 3, // Steam client appears to be out of date
};


// Initializing the Steamworks SDK
// -----------------------------
// 
// There are three different methods you can use to initialize the Steamworks SDK, depending on
// your project's environment. You should only use one method in your project.
// 
// If you are able to include this C++ header in your project, we recommend using the following
// initialization methods. They will ensure that all ISteam* interfaces defined in other
// C++ header files have versions that are supported by the user's Steam Client:
// - SteamAPI_InitEx() for new projects so you can show a detailed error message to the user
// - SteamAPI_Init() for existing projects that only display a generic error message
// 
// If you are unable to include this C++ header in your project and are dynamically loading
// Steamworks SDK methods from dll/so, you can use the following method:
// - SteamAPI_InitFlat()


// See "Initializing the Steamworks SDK" above for how to choose an init method.
// On success k_ESteamAPIInitResult_OK is returned. Otherwise, returns a value that can be used
// to create a localized error message for the user. If pOutErrMsg is non-NULL,
// it will receive an example error message, in English, that explains the reason for the failure.
//
// Example usage:
// 
//   SteamErrMsg errMsg;
//   if ( SteamAPI_Init(&errMsg) != k_ESteamAPIInitResult_OK )
//       FatalError( "Failed to init Steam.  %s", errMsg );
inline ESteamAPIInitResult SteamAPI_InitEx( SteamErrMsg *pOutErrMsg );

// See "Initializing the Steamworks SDK" above for how to choose an init method.
// Same usage as SteamAPI_InitEx(), however does not verify ISteam* interfaces are
// supported by the user's client and is exported from the dll
S_API ESteamAPIInitResult S_CALLTYPE SteamAPI_InitFlat( SteamErrMsg *pOutErrMsg );

S_API ESteamAPIInitResult S_CALLTYPE SteamInternal_SteamAPI_Init( const char *pszInternalCheckInterfaceVersions, SteamErrMsg *pOutErrMsg );

// See "Initializing the Steamworks SDK" above for how to choose an init method.
// Returns true on success
S_API steam_bool S_CALLTYPE SteamAPI_Init();

// SteamAPI_Shutdown should be called during process shutdown if possible.
S_API void S_CALLTYPE SteamAPI_Shutdown();

// SteamAPI_RestartAppIfNecessary ensures that your executable was launched through Steam.
//
// Returns true if the current process should terminate. Steam is now re-launching your application.
//
// Returns false if no action needs to be taken. This means that your executable was started through
// the Steam client, or a steam_appid.txt file is present in your game's directory (for development).
// Your current process should continue if false is returned.
//
// NOTE: If you use the Steam DRM wrapper on your primary executable file, this check is unnecessary
// since the DRM wrapper will ensure that your application was launched properly through Steam.
S_API steam_bool S_CALLTYPE SteamAPI_RestartAppIfNecessary( uint32 unOwnAppID );

// Many Steam API functions allocate a small amount of thread-local memory for parameter storage.
// SteamAPI_ReleaseCurrentThreadMemory() will free API memory associated with the calling thread.
// This function is also called automatically by SteamAPI_RunCallbacks(), so a single-threaded
// program never needs to explicitly call this function.
S_API void S_CALLTYPE SteamAPI_ReleaseCurrentThreadMemory();


// crash dump recording functions
S_API void S_CALLTYPE SteamAPI_WriteMiniDump( uint32 uStructuredExceptionCode, void* pvExceptionInfo, uint32 uBuildID );
S_API void S_CALLTYPE SteamAPI_SetMiniDumpComment( const char *pchMsg );

//----------------------------------------------------------------------------------------------------------------------------------------------------------//
//	steamclient.dll private wrapper functions
//
//	The following functions are part of abstracting API access to the steamclient.dll, but should only be used in very specific cases
//----------------------------------------------------------------------------------------------------------------------------------------------------------//

// SteamAPI_IsSteamRunning() returns true if Steam is currently running
S_API steam_bool S_CALLTYPE SteamAPI_IsSteamRunning();

// Pumps out all the steam messages, calling registered callbacks.
// NOT THREADSAFE - do not call from multiple threads simultaneously.
S_API void Steam_RunCallbacks( HSteamPipe hSteamPipe, bool bGameServerCallbacks );

// register the callback funcs to use to interact with the steam dll
S_API void Steam_RegisterInterfaceFuncs( void *hModule );

// returns the HSteamUser of the last user to dispatch a callback
S_API HSteamUser Steam_GetHSteamUserCurrent();

// returns the filename path of the current running Steam process, used if you need to load an explicit steam dll by name.
// DEPRECATED - implementation is Windows only, and the path returned is a UTF-8 string which must be converted to UTF-16 for use with Win32 APIs
S_API const char *SteamAPI_GetSteamInstallPath();

// sets whether or not Steam_RunCallbacks() should do a try {} catch (...) {} around calls to issuing callbacks
// This is ignored if you are using the manual callback dispatch method
S_API void SteamAPI_SetTryCatchCallbacks( bool bTryCatchCallbacks );

// backwards compat export, passes through to SteamAPI_ variants
S_API HSteamPipe GetHSteamPipe();
S_API HSteamUser GetHSteamUser();


#if defined( VERSION_SAFE_STEAM_API_INTERFACES )
// exists only for backwards compat with code written against older SDKs
S_API steam_bool S_CALLTYPE SteamAPI_InitSafe();
#endif

#if defined(USE_BREAKPAD_HANDLER) || defined(STEAM_API_EXPORTS)
// this should be called before the game initialized the steam APIs
// pchDate should be of the format "Mmm dd yyyy" (such as from the __ DATE __ macro )
// pchTime should be of the format "hh:mm:ss" (such as from the __ TIME __ macro )
// bFullMemoryDumps (Win32 only) -- writes out a uuid-full.dmp in the client/dumps folder
// pvContext-- can be NULL, will be the void * context passed into m_pfnPreMinidumpCallback
// PFNPreMinidumpCallback m_pfnPreMinidumpCallback   -- optional callback which occurs just before a .dmp file is written during a crash.  Applications can hook this to allow adding additional information into the .dmp comment stream.
S_API void S_CALLTYPE SteamAPI_UseBreakpadCrashHandler( char const *pchVersion, char const *pchDate, char const *pchTime, bool bFullMemoryDumps, void *pvContext, PFNPreMinidumpCallback m_pfnPreMinidumpCallback );
S_API void S_CALLTYPE SteamAPI_SetBreakpadAppID( uint32 unAppID );
#endif

//----------------------------------------------------------------------------------------------------------------------------------------------------------//
//
// Manual callback loop
//
// An alternative method for dispatching callbacks.  Similar to a windows message loop.
//
// If you use the manual callback dispatch, you must NOT use:
//
// - SteamAPI_RunCallbacks or SteamGameServer_RunCallbacks
// - STEAM_CALLBACK, CCallResult, CCallback, or CCallbackManual
//
// Here is the basic template for replacing SteamAPI_RunCallbacks() with manual dispatch
/*

	HSteamPipe hSteamPipe = SteamAPI_GetHSteamPipe(); // See also SteamGameServer_GetHSteamPipe()
	SteamAPI_ManualDispatch_RunFrame( hSteamPipe )
	CallbackMsg_t callback;
	while ( SteamAPI_ManualDispatch_GetNextCallback( hSteamPipe, &callback ) )
	{
		// Check for dispatching API call results
		if ( callback.m_iCallback == SteamAPICallCompleted_t::k_iCallback )
		{
			SteamAPICallCompleted_t *pCallCompleted = (SteamAPICallCompleted_t *)callback.
			void *pTmpCallResult = malloc( pCallback->m_cubParam );
			bool bFailed;
			if ( SteamAPI_ManualDispatch_GetAPICallResult( hSteamPipe, pCallCompleted->m_hAsyncCall, pTmpCallResult, pCallback->m_cubParam, pCallback->m_iCallback, &bFailed ) )
			{
				// Dispatch the call result to the registered handler(s) for the
				// call identified by pCallCompleted->m_hAsyncCall
			}
			free( pTmpCallResult );
		}
		else
		{
			// Look at callback.m_iCallback to see what kind of callback it is,
			// and dispatch to appropriate handler(s)
		}
		SteamAPI_ManualDispatch_FreeLastCallback( hSteamPipe );
	}

*/
//----------------------------------------------------------------------------------------------------------------------------------------------------------//

/// Inform the API that you wish to use manual event dispatch.  This must be called after SteamAPI_Init, but before
/// you use any of the other manual dispatch functions below.
S_API void S_CALLTYPE SteamAPI_ManualDispatch_Init();

/// Perform certain periodic actions that need to be performed.
S_API void S_CALLTYPE SteamAPI_ManualDispatch_RunFrame( HSteamPipe hSteamPipe );

/// Fetch the next pending callback on the given pipe, if any.  If a callback is available, true is returned
/// and the structure is populated.  In this case, you MUST call SteamAPI_ManualDispatch_FreeLastCallback
/// (after dispatching the callback) before calling SteamAPI_ManualDispatch_GetNextCallback again.
S_API steam_bool S_CALLTYPE SteamAPI_ManualDispatch_GetNextCallback( HSteamPipe hSteamPipe, CallbackMsg_t *pCallbackMsg );

/// You must call this after dispatching the callback, if SteamAPI_ManualDispatch_GetNextCallback returns true.
S_API void S_CALLTYPE SteamAPI_ManualDispatch_FreeLastCallback( HSteamPipe hSteamPipe );

/// Return the call result for the specified call on the specified pipe.  You really should
/// only call this in a handler for SteamAPICallCompleted_t callback.
S_API steam_bool S_CALLTYPE SteamAPI_ManualDispatch_GetAPICallResult( HSteamPipe hSteamPipe, SteamAPICall_t hSteamAPICall, void *pCallback, int cubCallback, int iCallbackExpected, bool *pbFailed );

//----------------------------------------------------------------------------------------------------------------------------------------------------------//
//
// CSteamAPIContext
//
// Deprecated!  This is not necessary any more.  Please use the global accessors directly
//
//----------------------------------------------------------------------------------------------------------------------------------------------------------//

#ifndef STEAM_API_EXPORTS

inline bool CSteamAPIContext::Init()
{
	m_pSteamClient = ::SteamClient();
	if ( !m_pSteamClient )
		return false;

	m_pSteamUser = ::SteamUser();
	if ( !m_pSteamUser )
		return false;

	m_pSteamFriends = ::SteamFriends();
	if ( !m_pSteamFriends )
		return false;

	m_pSteamUtils = ::SteamUtils();
	if ( !m_pSteamUtils )
		return false;

	m_pSteamMatchmaking = ::SteamMatchmaking();
	if ( !m_pSteamMatchmaking )
		return false;

	m_pSteamGameSearch = ::SteamGameSearch();
	if ( !m_pSteamGameSearch )
		return false;

#if !defined( IOSALL) // Not yet supported on iOS.
	m_pSteamMatchmakingServers = ::SteamMatchmakingServers();
	if ( !m_pSteamMatchmakingServers )
		return false;
#endif

	m_pSteamUserStats = ::SteamUserStats();
	if ( !m_pSteamUserStats )
		return false;

	m_pSteamApps = ::SteamApps();
	if ( !m_pSteamApps )
		return false;

	m_pSteamNetworking = ::SteamNetworking();
	if ( !m_pSteamNetworking )
		return false;

	m_pSteamRemoteStorage = ::SteamRemoteStorage();
	if ( !m_pSteamRemoteStorage )
		return false;

	m_pSteamScreenshots = ::SteamScreenshots();
	if ( !m_pSteamScreenshots )
		return false;

	m_pSteamHTTP = ::SteamHTTP();
	if ( !m_pSteamHTTP )
		return false;

	m_pController = ::SteamController();
	if ( !m_pController )
		return false;

	m_pSteamUGC = ::SteamUGC();
	if ( !m_pSteamUGC )
		return false;

	m_pSteamAppList = ::SteamAppList();
	if ( !m_pSteamAppList )
		return false;

	m_pSteamMusic = ::SteamMusic();
	if ( !m_pSteamMusic )
		return false;

	m_pSteamMusicRemote = ::SteamMusicRemote();
	if ( !m_pSteamMusicRemote )
		return false;

#if !defined( ANDROID ) && !defined( IOSALL) // Not yet supported on Android or ios.
	m_pSteamHTMLSurface = ::SteamHTMLSurface();
	if ( !m_pSteamHTMLSurface )
	return false;
#endif

	m_pSteamInventory = ::SteamInventory();
	if ( !m_pSteamInventory )
		return false;

	m_pSteamVideo = ::SteamVideo();
	if ( !m_pSteamVideo )
		return false;

	m_pSteamParentalSettings = ::SteamParentalSettings();
	if ( !m_pSteamParentalSettings )
		return false;

	m_pSteamInput = ::SteamInput();
	if ( !m_pSteamInput )
		return false;

	return true;
}

#endif

// Internal implementation of SteamAPI_InitEx.  This is done in a way that checks
// all of the versions of interfaces from headers being compiled into this code.
// If you are not using any of the C++ interfaces and do not need this version checking
// (for example if you are only using the "flat" interfaces, which have a different type
// of version checking), you can pass a NULL interface version string.
inline ESteamAPIInitResult SteamAPI_InitEx( SteamErrMsg *pOutErrMsg )
{
	const char *pszInternalCheckInterfaceVersions = 
		STEAMUTILS_INTERFACE_VERSION "\0"
		STEAMNETWORKINGUTILS_INTERFACE_VERSION "\0"

		STEAMAPPLIST_INTERFACE_VERSION "\0"
		STEAMAPPS_INTERFACE_VERSION "\0"
		STEAMCONTROLLER_INTERFACE_VERSION "\0"
		STEAMFRIENDS_INTERFACE_VERSION "\0"
		STEAMGAMESEARCH_INTERFACE_VERSION "\0"
		STEAMHTMLSURFACE_INTERFACE_VERSION "\0"
		STEAMHTTP_INTERFACE_VERSION "\0"
		STEAMINPUT_INTERFACE_VERSION "\0"
		STEAMINVENTORY_INTERFACE_VERSION "\0"
		STEAMMATCHMAKINGSERVERS_INTERFACE_VERSION "\0"
		STEAMMATCHMAKING_INTERFACE_VERSION "\0"
		STEAMMUSICREMOTE_INTERFACE_VERSION "\0"
		STEAMMUSIC_INTERFACE_VERSION "\0"
		STEAMNETWORKINGMESSAGES_INTERFACE_VERSION "\0"
		STEAMNETWORKINGSOCKETS_INTERFACE_VERSION "\0"
		STEAMNETWORKING_INTERFACE_VERSION "\0"
		STEAMPARENTALSETTINGS_INTERFACE_VERSION "\0"
		STEAMPARTIES_INTERFACE_VERSION "\0"
		STEAMREMOTEPLAY_INTERFACE_VERSION "\0"
		STEAMREMOTESTORAGE_INTERFACE_VERSION "\0"
		STEAMSCREENSHOTS_INTERFACE_VERSION "\0"
		STEAMUGC_INTERFACE_VERSION "\0"
		STEAMUSERSTATS_INTERFACE_VERSION "\0"
		STEAMUSER_INTERFACE_VERSION "\0"
		STEAMVIDEO_INTERFACE_VERSION "\0"

		"\0";

	return SteamInternal_SteamAPI_Init( pszInternalCheckInterfaceVersions, pOutErrMsg );
}

#endif // STEAM_API_H
