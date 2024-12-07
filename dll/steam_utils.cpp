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

#include "dll/dll.h"
#include "dll/steam_utils.h"


Steam_Utils::Steam_Utils(Settings *settings, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, Steam_Overlay *overlay):
    settings(settings),
    callback_results(callback_results),
    callbacks(callbacks),
    overlay(overlay)
{
    
}

// return the number of seconds since the user 
uint32 Steam_Utils::GetSecondsSinceAppActive()
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return static_cast<uint32>(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - startup_time).count());
}

uint32 Steam_Utils::GetSecondsSinceComputerActive()
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return GetSecondsSinceAppActive() + 2000;
}


// the universe this client is connecting to
EUniverse Steam_Utils::GetConnectedUniverse()
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return k_EUniversePublic;
}


// Steam server time.  Number of seconds since January 1, 1970, GMT (i.e unix time)
uint32 Steam_Utils::GetServerRealTime()
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    uint32 server_time = std::chrono::duration_cast<std::chrono::duration<uint32>>(std::chrono::system_clock::now().time_since_epoch()).count();
    PRINT_DEBUG("  Time %u", server_time);
    return server_time;
}


// returns the 2 digit ISO 3166-1-alpha-2 format country code this client is running in (as looked up via an IP-to-location database)
// e.g "US" or "UK".
const char* Steam_Utils::GetIPCountry()
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return settings->ip_country.c_str();
}

// returns true if the image exists, and valid sizes were filled out
bool Steam_Utils::GetImageSize( int iImage, uint32 *pnWidth, uint32 *pnHeight )
{
    PRINT_DEBUG("%i", iImage);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!iImage || !pnWidth || !pnHeight) return false;

    auto image_info = settings->get_image(iImage);
    if (!image_info) return false;

    *pnWidth = image_info->width;
    *pnHeight = image_info->height;
    return true;
}

// returns true if the image exists, and the buffer was successfully filled out
// results are returned in RGBA format
// the destination buffer size should be 4 * height * width * sizeof(char)
bool Steam_Utils::GetImageRGBA( int iImage, uint8 *pubDest, int nDestBufferSize )
{
    PRINT_DEBUG("%i %p %i", iImage, pubDest, nDestBufferSize);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!iImage || !pubDest || nDestBufferSize <= 0) return false;

    auto image_info = settings->get_image(iImage);
    if (!image_info) return false;

    image_info->data.copy((char *)pubDest, nDestBufferSize);
    return true;
}

// returns the IP of the reporting server for valve - currently only used in Source engine games
bool Steam_Utils::GetCSERIPPort( uint32 *unIP, uint16 *usPort )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return false;
}


// return the amount of battery power left in the current system in % [0..100], 255 for being on AC power
uint8 Steam_Utils::GetCurrentBatteryPower()
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return 255; // AC
}


// returns the appID of the current process
uint32 Steam_Utils::GetAppID()
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return settings->get_local_game_id().AppID();
}


// Sets the position where the overlay instance for the currently calling game should show notifications.
// This position is per-game and if this function is called from outside of a game context it will do nothing.
void Steam_Utils::SetOverlayNotificationPosition( ENotificationPosition eNotificationPosition )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    overlay->SetNotificationPosition(eNotificationPosition);
}


// API asynchronous call results
// can be used directly, but more commonly used via the callback dispatch API (see steam_api.h)
bool Steam_Utils::IsAPICallCompleted( SteamAPICall_t hSteamAPICall, bool *pbFailed )
{
    PRINT_DEBUG("%llu", hSteamAPICall);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (pbFailed) *pbFailed = false;

    if (hSteamAPICall == 1) { //bug ? soul calibur 6 calls this function with the return value 1 of Steam_User_Stats::RequestCurrentStats and expects this function to return true
        if (pbFailed) *pbFailed = true;
        return true;
    }

    if (!callback_results->exists(hSteamAPICall)) return false;
    return true; //all api calls "complete" right away
}

ESteamAPICallFailure Steam_Utils::GetAPICallFailureReason( SteamAPICall_t hSteamAPICall )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return k_ESteamAPICallFailureNone;
}

bool Steam_Utils::GetAPICallResult( SteamAPICall_t hSteamAPICall, void *pCallback, int cubCallback, int iCallbackExpected, bool *pbFailed )
{
    PRINT_DEBUG("%llu %i %i %p", hSteamAPICall, cubCallback, iCallbackExpected, pbFailed);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (callback_results->callback_result(hSteamAPICall, pCallback, cubCallback)) {
        if (pbFailed) *pbFailed = false;
        PRINT_DEBUG("  Succeeded");
        return true;
    } else {
        return false;
    }
}

// Deprecated. Applications should use SteamAPI_RunCallbacks() instead. Game servers do not need to call this function.
STEAM_PRIVATE_API(
void Steam_Utils::RunFrame()
{
    //PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    // Steam_Client *client = get_steam_client();
    // client ->RunCallbacks(true, false, false);
}
)

// returns the number of IPC calls made since the last time this function was called
// Used for perf debugging so you can understand how many IPC calls your game makes per frame
// Every IPC call is at minimum a thread context switch if not a process one so you want to rate
// control how often you do them.
uint32 Steam_Utils::GetIPCCallCount()
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    static int i = 0;
    i += 123;
    return i; //TODO
}


// API warning handling
// 'int' is the severity; 0 for msg, 1 for warning
// 'const char *' is the text of the message
// callbacks will occur directly after the API function is called that generated the warning or message
void Steam_Utils::SetWarningMessageHook( SteamAPIWarningMessageHook_t pFunction )
{
    PRINT_DEBUG_TODO();
}


// Returns true if the overlay is running & the user can access it. The overlay process could take a few seconds to
// start & hook the game process, so this function will initially return false while the overlay is loading.
bool Steam_Utils::IsOverlayEnabled()
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return overlay->Ready();
}


// Normally this call is unneeded if your game has a constantly running frame loop that calls the 
// D3D Present API, or OGL SwapBuffers API every frame.
//
// However, if you have a game that only refreshes the screen on an event driven basis then that can break 
// the overlay, as it uses your Present/SwapBuffers calls to drive it's internal frame loop and it may also
// need to Present() to the screen any time an even needing a notification happens or when the overlay is
// brought up over the game by a user.  You can use this API to ask the overlay if it currently need a present
// in that case, and then you can check for this periodically (roughly 33hz is desirable) and make sure you
// refresh the screen with Present or SwapBuffers to allow the overlay to do it's work.
bool Steam_Utils::BOverlayNeedsPresent()
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return overlay->NeedPresent();
}


// Asynchronous call to check if an executable file has been signed using the public key set on the signing tab
// of the partner site, for example to refuse to load modified executable files.  
// The result is returned in CheckFileSignature_t.
//   k_ECheckFileSignatureNoSignaturesFoundForThisApp - This app has not been configured on the signing tab of the partner site to enable this function.
//   k_ECheckFileSignatureNoSignaturesFoundForThisFile - This file is not listed on the signing tab for the partner site.
//   k_ECheckFileSignatureFileNotFound - The file does not exist on disk.
//   k_ECheckFileSignatureInvalidSignature - The file exists, and the signing tab has been set for this file, but the file is either not signed or the signature does not match.
//   k_ECheckFileSignatureValidSignature - The file is signed and the signature is valid.
STEAM_CALL_RESULT( CheckFileSignature_t )
SteamAPICall_t Steam_Utils::CheckFileSignature( const char *szFileName )
{
    PRINT_DEBUG("'%s'", szFileName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    CheckFileSignature_t data{};
    data.m_eCheckFileSignature = k_ECheckFileSignatureValidSignature;
    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return ret;
}


// Activates the Big Picture text input dialog which only supports gamepad input
bool Steam_Utils::ShowGamepadTextInput( EGamepadTextInputMode eInputMode, EGamepadTextInputLineMode eLineInputMode, const char *pchDescription, uint32 unCharMax, const char *pchExistingText )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return false;
}

bool Steam_Utils::ShowGamepadTextInput( EGamepadTextInputMode eInputMode, EGamepadTextInputLineMode eLineInputMode, const char *pchDescription, uint32 unCharMax )
{
	PRINT_DEBUG("old");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
	return ShowGamepadTextInput(eInputMode, eLineInputMode, pchDescription, unCharMax, NULL);
}

// Returns previously entered text & length
uint32 Steam_Utils::GetEnteredGamepadTextLength()
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return 0;
}

bool Steam_Utils::GetEnteredGamepadTextInput( char *pchText, uint32 cchText )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return false;
}


// returns the language the steam client is running in, you probably want ISteamApps::GetCurrentGameLanguage instead, this is for very special usage cases
const char* Steam_Utils::GetSteamUILanguage()
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return settings->get_language();
}


// returns true if Steam itself is running in VR mode
bool Steam_Utils::IsSteamRunningInVR()
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return false;
}


// Sets the inset of the overlay notification from the corner specified by SetOverlayNotificationPosition.
void Steam_Utils::SetOverlayNotificationInset( int nHorizontalInset, int nVerticalInset )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    overlay->SetNotificationInset(nHorizontalInset, nVerticalInset);
}


// returns true if Steam & the Steam Overlay are running in Big Picture mode
// Games much be launched through the Steam client to enable the Big Picture overlay. During development,
// a game can be added as a non-steam game to the developers library to test this feature
bool Steam_Utils::IsSteamInBigPictureMode()
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return false;
}


// ask SteamUI to create and render its OpenVR dashboard
void Steam_Utils::StartVRDashboard()
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}


// Returns true if the HMD content will be streamed via Steam In-Home Streaming
bool Steam_Utils::IsVRHeadsetStreamingEnabled()
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return false;
}


// Set whether the HMD content will be streamed via Steam In-Home Streaming
// If this is set to true, then the scene in the HMD headset will be streamed, and remote input will not be allowed.
// If this is set to false, then the application window will be streamed instead, and remote input will be allowed.
// The default is true unless "VRHeadsetStreaming" "0" is in the extended appinfo for a game.
// (this is useful for games that have asymmetric multiplayer gameplay)
void Steam_Utils::SetVRHeadsetStreamingEnabled( bool bEnabled )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}

// Returns whether this steam client is a Steam China specific client, vs the global client.
bool Steam_Utils::IsSteamChinaLauncher()
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return false;
}

// Initializes text filtering.
//   Returns false if filtering is unavailable for the language the user is currently running in.
bool Steam_Utils::InitFilterText()
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return false;
}

// Initializes text filtering.
//   unFilterOptions are reserved for future use and should be set to 0
// Returns false if filtering is unavailable for the language the user is currently running in.
bool Steam_Utils::InitFilterText( uint32 unFilterOptions )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return false;
}

// Filters the provided input message and places the filtered result into pchOutFilteredText.
//   pchOutFilteredText is where the output will be placed, even if no filtering or censoring is performed
//   nByteSizeOutFilteredText is the size (in bytes) of pchOutFilteredText
//   pchInputText is the input string that should be filtered, which can be ASCII or UTF-8
//   bLegalOnly should be false if you want profanity and legally required filtering (where required) and true if you want legally required filtering only
//   Returns the number of characters (not bytes) filtered.
int Steam_Utils::FilterText( char* pchOutFilteredText, uint32 nByteSizeOutFilteredText, const char * pchInputMessage, bool bLegalOnly )
{
    PRINT_DEBUG("old");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return FilterText(k_ETextFilteringContextUnknown, CSteamID(), pchInputMessage, pchOutFilteredText, nByteSizeOutFilteredText );
}

// Filters the provided input message and places the filtered result into pchOutFilteredText, using legally required filtering and additional filtering based on the context and user settings
//   eContext is the type of content in the input string
//   sourceSteamID is the Steam ID that is the source of the input string (e.g. the player with the name, or who said the chat text)
//   pchInputText is the input string that should be filtered, which can be ASCII or UTF-8
//   pchOutFilteredText is where the output will be placed, even if no filtering is performed
//   nByteSizeOutFilteredText is the size (in bytes) of pchOutFilteredText, should be at least strlen(pchInputText)+1
// Returns the number of characters (not bytes) filtered
int Steam_Utils::FilterText( ETextFilteringContext eContext, CSteamID sourceSteamID, const char *pchInputMessage, char *pchOutFilteredText, uint32 nByteSizeOutFilteredText )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!nByteSizeOutFilteredText) return 0;
    unsigned len = static_cast<unsigned>(strlen(pchInputMessage));
    if (!len) return 0;
    len += 1;
    if (len > nByteSizeOutFilteredText) len = nByteSizeOutFilteredText;
    len -= 1;

    memcpy(pchOutFilteredText, pchInputMessage, len);
    pchOutFilteredText[len] = 0;
    return len;
}


// Return what we believe your current ipv6 connectivity to "the internet" is on the specified protocol.
// This does NOT tell you if the Steam client is currently connected to Steam via ipv6.
ESteamIPv6ConnectivityState Steam_Utils::GetIPv6ConnectivityState( ESteamIPv6ConnectivityProtocol eProtocol )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return k_ESteamIPv6ConnectivityState_Unknown;
}

// returns true if currently running on the Steam Deck device
bool Steam_Utils::IsSteamRunningOnSteamDeck()
{
    PRINT_DEBUG("%i", (int)settings->steam_deck);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return settings->steam_deck;
}

// Opens a floating keyboard over the game content and sends OS keyboard keys directly to the game.
// The text field position is specified in pixels relative the origin of the game window and is used to position the floating keyboard in a way that doesn't cover the text field
bool Steam_Utils::ShowFloatingGamepadTextInput( EFloatingGamepadTextInputMode eKeyboardMode, int nTextFieldXPosition, int nTextFieldYPosition, int nTextFieldWidth, int nTextFieldHeight )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return false;
}

// In game launchers that don't have controller support you can call this to have Steam Input translate the controller input into mouse/kb to navigate the launcher
void Steam_Utils::SetGameLauncherMode( bool bLauncherMode )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}

bool Steam_Utils::DismissFloatingGamepadTextInput()
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return true;
}

bool Steam_Utils::DismissGamepadTextInput()
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return true;
}
