
#ifndef ISTEAMUTILS008_H
#define ISTEAMUTILS008_H
#ifdef STEAM_WIN32
#pragma once
#endif

class ISteamUtils008
{
public:
	// return the number of seconds since the user 
	virtual uint32 GetSecondsSinceAppActive() = 0;
	virtual uint32 GetSecondsSinceComputerActive() = 0;

	// the universe this client is connecting to
	virtual EUniverse GetConnectedUniverse() = 0;

	// Steam server time.  Number of seconds since January 1, 1970, GMT (i.e unix time)
	virtual uint32 GetServerRealTime() = 0;

	// returns the 2 digit ISO 3166-1-alpha-2 format country code this client is running in (as looked up via an IP-to-location database)
	// e.g "US" or "UK".
	virtual const char *GetIPCountry() = 0;

	// returns true if the image exists, and valid sizes were filled out
	virtual bool GetImageSize( int iImage, uint32 *pnWidth, uint32 *pnHeight ) = 0;

	// returns true if the image exists, and the buffer was successfully filled out
	// results are returned in RGBA format
	// the destination buffer size should be 4 * height * width * sizeof(char)
	virtual bool GetImageRGBA( int iImage, uint8 *pubDest, int nDestBufferSize ) = 0;

	// returns the IP of the reporting server for valve - currently only used in Source engine games
	virtual bool GetCSERIPPort( uint32 *unIP, uint16 *usPort ) = 0;

	// return the amount of battery power left in the current system in % [0..100], 255 for being on AC power
	virtual uint8 GetCurrentBatteryPower() = 0;

	// returns the appID of the current process
	virtual uint32 GetAppID() = 0;

	// Sets the position where the overlay instance for the currently calling game should show notifications.
	// This position is per-game and if this function is called from outside of a game context it will do nothing.
	virtual void SetOverlayNotificationPosition( ENotificationPosition eNotificationPosition ) = 0;

	// API asynchronous call results
	// can be used directly, but more commonly used via the callback dispatch API (see steam_api.h)
	virtual bool IsAPICallCompleted( SteamAPICall_t hSteamAPICall, bool *pbFailed ) = 0;
	virtual ESteamAPICallFailure GetAPICallFailureReason( SteamAPICall_t hSteamAPICall ) = 0;
	virtual bool GetAPICallResult( SteamAPICall_t hSteamAPICall, void *pCallback, int cubCallback, int iCallbackExpected, bool *pbFailed ) = 0;

	// Deprecated. Applications should use SteamAPI_RunCallbacks() instead. Game servers do not need to call this function.
	STEAM_PRIVATE_API( virtual void RunFrame() = 0; )

	// returns the number of IPC calls made since the last time this function was called
	// Used for perf debugging so you can understand how many IPC calls your game makes per frame
	// Every IPC call is at minimum a thread context switch if not a process one so you want to rate
	// control how often you do them.
	virtual uint32 GetIPCCallCount() = 0;

	// API warning handling
	// 'int' is the severity; 0 for msg, 1 for warning
	// 'const char *' is the text of the message
	// callbacks will occur directly after the API function is called that generated the warning or message
	virtual void SetWarningMessageHook( SteamAPIWarningMessageHook_t pFunction ) = 0;

	// Returns true if the overlay is running & the user can access it. The overlay process could take a few seconds to
	// start & hook the game process, so this function will initially return false while the overlay is loading.
	virtual bool IsOverlayEnabled() = 0;

	// Normally this call is unneeded if your game has a constantly running frame loop that calls the 
	// D3D Present API, or OGL SwapBuffers API every frame.
	//
	// However, if you have a game that only refreshes the screen on an event driven basis then that can break 
	// the overlay, as it uses your Present/SwapBuffers calls to drive it's internal frame loop and it may also
	// need to Present() to the screen any time an even needing a notification happens or when the overlay is
	// brought up over the game by a user.  You can use this API to ask the overlay if it currently need a present
	// in that case, and then you can check for this periodically (roughly 33hz is desirable) and make sure you
	// refresh the screen with Present or SwapBuffers to allow the overlay to do it's work.
	virtual bool BOverlayNeedsPresent() = 0;

	// Asynchronous call to check if an executable file has been signed using the public key set on the signing tab
	// of the partner site, for example to refuse to load modified executable files.  
	// The result is returned in CheckFileSignature_t.
	//   k_ECheckFileSignatureNoSignaturesFoundForThisApp - This app has not been configured on the signing tab of the partner site to enable this function.
	//   k_ECheckFileSignatureNoSignaturesFoundForThisFile - This file is not listed on the signing tab for the partner site.
	//   k_ECheckFileSignatureFileNotFound - The file does not exist on disk.
	//   k_ECheckFileSignatureInvalidSignature - The file exists, and the signing tab has been set for this file, but the file is either not signed or the signature does not match.
	//   k_ECheckFileSignatureValidSignature - The file is signed and the signature is valid.
	STEAM_CALL_RESULT( CheckFileSignature_t )
	virtual SteamAPICall_t CheckFileSignature( const char *szFileName ) = 0;

	// Activates the Big Picture text input dialog which only supports gamepad input
	virtual bool ShowGamepadTextInput( EGamepadTextInputMode eInputMode, EGamepadTextInputLineMode eLineInputMode, const char *pchDescription, uint32 unCharMax, const char *pchExistingText ) = 0;

	// Returns previously entered text & length
	virtual uint32 GetEnteredGamepadTextLength() = 0;
	virtual bool GetEnteredGamepadTextInput( char *pchText, uint32 cchText ) = 0;

	// returns the language the steam client is running in, you probably want ISteamApps::GetCurrentGameLanguage instead, this is for very special usage cases
	virtual const char *GetSteamUILanguage() = 0;

	// returns true if Steam itself is running in VR mode
	virtual bool IsSteamRunningInVR() = 0;
	
	// Sets the inset of the overlay notification from the corner specified by SetOverlayNotificationPosition.
	virtual void SetOverlayNotificationInset( int nHorizontalInset, int nVerticalInset ) = 0;

	// returns true if Steam & the Steam Overlay are running in Big Picture mode
	// Games much be launched through the Steam client to enable the Big Picture overlay. During development,
	// a game can be added as a non-steam game to the developers library to test this feature
	virtual bool IsSteamInBigPictureMode() = 0;

	// ask SteamUI to create and render its OpenVR dashboard
	virtual void StartVRDashboard() = 0;
};

#endif // ISTEAMUTILS008_H
