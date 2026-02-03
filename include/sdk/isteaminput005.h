
#ifndef ISTEAMINPUT005_H
#define ISTEAMINPUT005_H
#ifdef STEAM_WIN32
#pragma once
#endif

class ISteamInput005
{
public:
	
	// Init and Shutdown must be called when starting/ending use of this interface.
	// if bExplicitlyCallRunFrame is called then you will need to manually call RunFrame
	// each frame, otherwise Steam Input will updated when SteamAPI_RunCallbacks() is called
	virtual bool Init( bool bExplicitlyCallRunFrame ) = 0;
	virtual bool Shutdown() = 0;
	
	// Set the absolute path to the Input Action Manifest file containing the in-game actions
	// and file paths to the official configurations. Used in games that bundle Steam Input
	// configurations inside of the game depot instead of using the Steam Workshop
	virtual bool SetInputActionManifestFilePath( const char *pchInputActionManifestAbsolutePath ) = 0;

	// Synchronize API state with the latest Steam Input action data available. This
	// is performed automatically by SteamAPI_RunCallbacks, but for the absolute lowest
	// possible latency, you call this directly before reading controller state. 
	// Note: This must be called from somewhere before GetConnectedControllers will
	// return any handles
	virtual void RunFrame( bool bReservedValue = true ) = 0;

	// Waits on an IPC event from Steam sent when there is new data to be fetched from
	// the data drop. Returns true when data was recievied before the timeout expires.
	// Useful for games with a dedicated input thread
	virtual bool BWaitForData( bool bWaitForever, uint32 unTimeout ) = 0;

	// Returns true if new data has been received since the last time action data was accessed
	// via GetDigitalActionData or GetAnalogActionData. The game will still need to call
	// SteamInput()->RunFrame() or SteamAPI_RunCallbacks() before this to update the data stream
	virtual bool BNewDataAvailable() = 0;

	// Enumerate currently connected Steam Input enabled devices - developers can opt in controller by type (ex: Xbox/Playstation/etc) via
	// the Steam Input settings in the Steamworks site or users can opt-in in their controller settings in Steam.
	// handlesOut should point to a STEAM_INPUT_MAX_COUNT sized array of InputHandle_t handles
	// Returns the number of handles written to handlesOut
	virtual int GetConnectedControllers( STEAM_OUT_ARRAY_COUNT( STEAM_INPUT_MAX_COUNT, Receives list of connected controllers ) InputHandle_t *handlesOut ) = 0;
	
	//-----------------------------------------------------------------------------
	// CALLBACKS
	//-----------------------------------------------------------------------------
	
	// Controller configuration loaded - these callbacks will always fire if you have
	// a handler. Note: this is called within either SteamInput()->RunFrame or by SteamAPI_RunCallbacks
	STEAM_CALL_BACK( SteamInputConfigurationLoaded_t )

	// Enable SteamInputDeviceConnected_t and SteamInputDeviceDisconnected_t callbacks.
	// Each controller that is already connected will generate a device connected
	// callback when you enable them
	virtual void EnableDeviceCallbacks() = 0;

	// Controller Connected - provides info about a single newly connected controller
	// Note: this is called within either SteamInput()->RunFrame or by SteamAPI_RunCallbacks
	STEAM_CALL_BACK( SteamInputDeviceConnected_t )

	// Controller Disconnected - provides info about a single disconnected controller
	// Note: this is called within either SteamInput()->RunFrame or by SteamAPI_RunCallbacks
	STEAM_CALL_BACK( SteamInputDeviceDisconnected_t )

	// Enable SteamInputActionEvent_t callbacks. Directly calls your callback function
	// for lower latency than standard Steam callbacks. Supports one callback at a time.
	// Note: this is called within either SteamInput()->RunFrame or by SteamAPI_RunCallbacks
	virtual void EnableActionEventCallbacks( SteamInputActionEventCallbackPointer pCallback ) = 0;

	//-----------------------------------------------------------------------------
	// ACTION SETS
	//-----------------------------------------------------------------------------

	// Lookup the handle for an Action Set. Best to do this once on startup, and store the handles for all future API calls.
	virtual InputActionSetHandle_t GetActionSetHandle( const char *pszActionSetName ) = 0;
	
	// Reconfigure the controller to use the specified action set (ie 'Menu', 'Walk' or 'Drive')
	// This is cheap, and can be safely called repeatedly. It's often easier to repeatedly call it in
	// your state loops, instead of trying to place it in all of your state transitions.
	virtual void ActivateActionSet( InputHandle_t inputHandle, InputActionSetHandle_t actionSetHandle ) = 0;
	virtual InputActionSetHandle_t GetCurrentActionSet( InputHandle_t inputHandle ) = 0;

	// ACTION SET LAYERS
	virtual void ActivateActionSetLayer( InputHandle_t inputHandle, InputActionSetHandle_t actionSetLayerHandle ) = 0;
	virtual void DeactivateActionSetLayer( InputHandle_t inputHandle, InputActionSetHandle_t actionSetLayerHandle ) = 0;
	virtual void DeactivateAllActionSetLayers( InputHandle_t inputHandle ) = 0;

	// Enumerate currently active layers.
	// handlesOut should point to a STEAM_INPUT_MAX_ACTIVE_LAYERS sized array of InputActionSetHandle_t handles
	// Returns the number of handles written to handlesOut
	virtual int GetActiveActionSetLayers( InputHandle_t inputHandle, STEAM_OUT_ARRAY_COUNT( STEAM_INPUT_MAX_ACTIVE_LAYERS, Receives list of active layers ) InputActionSetHandle_t *handlesOut ) = 0;

	//-----------------------------------------------------------------------------
	// ACTIONS
	//-----------------------------------------------------------------------------

	// Lookup the handle for a digital action. Best to do this once on startup, and store the handles for all future API calls.
	virtual InputDigitalActionHandle_t GetDigitalActionHandle( const char *pszActionName ) = 0;
	
	// Returns the current state of the supplied digital game action
	virtual InputDigitalActionData_t GetDigitalActionData( InputHandle_t inputHandle, InputDigitalActionHandle_t digitalActionHandle ) = 0;
	
	// Get the origin(s) for a digital action within an action set. Returns the number of origins supplied in originsOut. Use this to display the appropriate on-screen prompt for the action.
	// originsOut should point to a STEAM_INPUT_MAX_ORIGINS sized array of EInputActionOrigin handles. The EInputActionOrigin enum will get extended as support for new controller controllers gets added to
	// the Steam client and will exceed the values from this header, please check bounds if you are using a look up table.
	virtual int GetDigitalActionOrigins( InputHandle_t inputHandle, InputActionSetHandle_t actionSetHandle, InputDigitalActionHandle_t digitalActionHandle, STEAM_OUT_ARRAY_COUNT( STEAM_INPUT_MAX_ORIGINS, Receives list of action origins ) EInputActionOrigin *originsOut ) = 0;
	
	// Returns a localized string (from Steam's language setting) for the user-facing action name corresponding to the specified handle
	virtual const char *GetStringForDigitalActionName( InputDigitalActionHandle_t eActionHandle ) = 0;

	// Lookup the handle for an analog action. Best to do this once on startup, and store the handles for all future API calls.
	virtual InputAnalogActionHandle_t GetAnalogActionHandle( const char *pszActionName ) = 0;
	
	// Returns the current state of these supplied analog game action
	virtual InputAnalogActionData_t GetAnalogActionData( InputHandle_t inputHandle, InputAnalogActionHandle_t analogActionHandle ) = 0;

	// Get the origin(s) for an analog action within an action set. Returns the number of origins supplied in originsOut. Use this to display the appropriate on-screen prompt for the action.
	// originsOut should point to a STEAM_INPUT_MAX_ORIGINS sized array of EInputActionOrigin handles. The EInputActionOrigin enum will get extended as support for new controller controllers gets added to
	// the Steam client and will exceed the values from this header, please check bounds if you are using a look up table.
	virtual int GetAnalogActionOrigins( InputHandle_t inputHandle, InputActionSetHandle_t actionSetHandle, InputAnalogActionHandle_t analogActionHandle, STEAM_OUT_ARRAY_COUNT( STEAM_INPUT_MAX_ORIGINS, Receives list of action origins ) EInputActionOrigin *originsOut ) = 0;

	// Get a local path to a PNG file for the provided origin's glyph. 
	virtual const char *GetGlyphPNGForActionOrigin( EInputActionOrigin eOrigin, ESteamInputGlyphSize eSize, uint32 unFlags ) = 0;

	// Get a local path to a SVG file for the provided origin's glyph. 
	virtual const char *GetGlyphSVGForActionOrigin( EInputActionOrigin eOrigin, uint32 unFlags ) = 0;

	// Get a local path to an older, Big Picture Mode-style PNG file for a particular origin
	virtual const char *GetGlyphForActionOrigin_Legacy( EInputActionOrigin eOrigin ) = 0;
	
	// Returns a localized string (from Steam's language setting) for the specified origin.
	virtual const char *GetStringForActionOrigin( EInputActionOrigin eOrigin ) = 0;

	// Returns a localized string (from Steam's language setting) for the user-facing action name corresponding to the specified handle
	virtual const char *GetStringForAnalogActionName( InputAnalogActionHandle_t eActionHandle ) = 0;

	// Stop analog momentum for the action if it is a mouse action in trackball mode
	virtual void StopAnalogActionMomentum( InputHandle_t inputHandle, InputAnalogActionHandle_t eAction ) = 0;

	// Returns raw motion data from the specified device
	virtual InputMotionData_t GetMotionData( InputHandle_t inputHandle ) = 0;

	//-----------------------------------------------------------------------------
	// OUTPUTS
	//-----------------------------------------------------------------------------

	// Trigger a vibration event on supported controllers - Steam will translate these commands into haptic pulses for Steam Controllers
	virtual void TriggerVibration( InputHandle_t inputHandle, unsigned short usLeftSpeed, unsigned short usRightSpeed ) = 0;

	// Trigger a vibration event on supported controllers including Xbox trigger impulse rumble - Steam will translate these commands into haptic pulses for Steam Controllers
	virtual void TriggerVibrationExtended( InputHandle_t inputHandle, unsigned short usLeftSpeed, unsigned short usRightSpeed, unsigned short usLeftTriggerSpeed, unsigned short usRightTriggerSpeed ) = 0;

	// Send a haptic pulse, works on Steam Deck and Steam Controller devices
	virtual void TriggerSimpleHapticEvent( InputHandle_t inputHandle, EControllerHapticLocation eHapticLocation, uint8 nIntensity, char nGainDB, uint8 nOtherIntensity, char nOtherGainDB ) = 0;

	// Set the controller LED color on supported controllers. nFlags is a bitmask of values from ESteamInputLEDFlag - 0 will default to setting a color. Steam will handle
	// the behavior on exit of your program so you don't need to try restore the default as you are shutting down
	virtual void SetLEDColor( InputHandle_t inputHandle, uint8 nColorR, uint8 nColorG, uint8 nColorB, unsigned int nFlags ) = 0;

	// Trigger a haptic pulse on a Steam Controller - if you are approximating rumble you may want to use TriggerVibration instead.
	// Good uses for Haptic pulses include chimes, noises, or directional gameplay feedback (taking damage, footstep locations, etc).
	virtual void Legacy_TriggerHapticPulse( InputHandle_t inputHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec ) = 0;

	// Trigger a haptic pulse with a duty cycle of usDurationMicroSec / usOffMicroSec, unRepeat times. If you are approximating rumble you may want to use TriggerVibration instead.
	// nFlags is currently unused and reserved for future use.
	virtual void Legacy_TriggerRepeatedHapticPulse( InputHandle_t inputHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec, unsigned short usOffMicroSec, unsigned short unRepeat, unsigned int nFlags ) = 0;

	//-----------------------------------------------------------------------------
	// Utility functions available without using the rest of Steam Input API
	//-----------------------------------------------------------------------------

	// Invokes the Steam overlay and brings up the binding screen if the user is using Big Picture Mode
	// If the user is not in Big Picture Mode it will open up the binding in a new window
	virtual bool ShowBindingPanel( InputHandle_t inputHandle ) = 0;

	// Returns the input type for a particular handle - unlike EInputActionOrigin which update with Steam and may return unrecognized values
	// ESteamInputType will remain static and only return valid values from your SDK version 
	virtual ESteamInputType GetInputTypeForHandle( InputHandle_t inputHandle ) = 0;

	// Returns the associated controller handle for the specified emulated gamepad - can be used with the above 2 functions
	// to identify controllers presented to your game over Xinput. Returns 0 if the Xinput index isn't associated with Steam Input
	virtual InputHandle_t GetControllerForGamepadIndex( int nIndex ) = 0;

	// Returns the associated gamepad index for the specified controller, if emulating a gamepad or -1 if not associated with an Xinput index
	virtual int GetGamepadIndexForController( InputHandle_t ulinputHandle ) = 0;
	
	// Returns a localized string (from Steam's language setting) for the specified Xbox controller origin.
	virtual const char *GetStringForXboxOrigin( EXboxOrigin eOrigin ) = 0;

	// Get a local path to art for on-screen glyph for a particular Xbox controller origin
	virtual const char *GetGlyphForXboxOrigin( EXboxOrigin eOrigin ) = 0;

	// Get the equivalent ActionOrigin for a given Xbox controller origin this can be chained with GetGlyphForActionOrigin to provide future proof glyphs for
	// non-Steam Input API action games. Note - this only translates the buttons directly and doesn't take into account any remapping a user has made in their configuration
	virtual EInputActionOrigin GetActionOriginFromXboxOrigin( InputHandle_t inputHandle, EXboxOrigin eOrigin ) = 0;

	// Convert an origin to another controller type - for inputs not present on the other controller type this will return k_EInputActionOrigin_None
	// When a new input type is added you will be able to pass in k_ESteamInputType_Unknown and the closest origin that your version of the SDK recognized will be returned
	// ex: if a Playstation 5 controller was released this function would return Playstation 4 origins.
	virtual EInputActionOrigin TranslateActionOrigin( ESteamInputType eDestinationInputType, EInputActionOrigin eSourceOrigin ) = 0;

	// Get the binding revision for a given device. Returns false if the handle was not valid or if a mapping is not yet loaded for the device
	virtual bool GetDeviceBindingRevision( InputHandle_t inputHandle, int *pMajor, int *pMinor ) = 0;

	// Get the Steam Remote Play session ID associated with a device, or 0 if there is no session associated with it
	// See isteamremoteplay.h for more information on Steam Remote Play sessions
	virtual uint32 GetRemotePlaySessionID( InputHandle_t inputHandle ) = 0;

	// Get a bitmask of the Steam Input Configuration types opted in for the current session. Returns ESteamInputConfigurationEnableType values.?	
	// Note: user can override the settings from the Steamworks Partner site so the returned values may not exactly match your default configuration
	virtual uint16 GetSessionInputConfigurationSettings() = 0;
};

#endif //ISTEAMINPUT005_H
