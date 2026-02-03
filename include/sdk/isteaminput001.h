
#ifndef ISTEAMINPUT001_H
#define ISTEAMINPUT001_H
#ifdef STEAM_WIN32
#pragma once
#endif

class ISteamInput001
{
public:
	
	// Init and Shutdown must be called when starting/ending use of this interface
	virtual bool Init() = 0;
	virtual bool Shutdown() = 0;
	
	// Synchronize API state with the latest Steam Controller inputs available. This
	// is performed automatically by SteamAPI_RunCallbacks, but for the absolute lowest
	// possible latency, you call this directly before reading controller state. This must
	// be called from somewhere before GetConnectedControllers will return any handles
	virtual void RunFrame() = 0;

	// Enumerate currently connected Steam Input enabled devices - developers can opt in controller by type (ex: Xbox/Playstation/etc) via
	// the Steam Input settings in the Steamworks site or users can opt-in in their controller settings in Steam.
	// handlesOut should point to a STEAM_INPUT_MAX_COUNT sized array of InputHandle_t handles
	// Returns the number of handles written to handlesOut
	virtual int GetConnectedControllers( STEAM_OUT_ARRAY_COUNT( STEAM_INPUT_MAX_COUNT, Receives list of connected controllers ) InputHandle_t *handlesOut ) = 0;
	
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
	// handlesOut should point to a STEAM_INPUT_MAX_ACTIVE_LAYERS sized array of ControllerActionSetHandle_t handles
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
	
	// Lookup the handle for an analog action. Best to do this once on startup, and store the handles for all future API calls.
	virtual InputAnalogActionHandle_t GetAnalogActionHandle( const char *pszActionName ) = 0;
	
	// Returns the current state of these supplied analog game action
	virtual InputAnalogActionData_t GetAnalogActionData( InputHandle_t inputHandle, InputAnalogActionHandle_t analogActionHandle ) = 0;

	// Get the origin(s) for an analog action within an action set. Returns the number of origins supplied in originsOut. Use this to display the appropriate on-screen prompt for the action.
	// originsOut should point to a STEAM_INPUT_MAX_ORIGINS sized array of EInputActionOrigin handles. The EInputActionOrigin enum will get extended as support for new controller controllers gets added to
	// the Steam client and will exceed the values from this header, please check bounds if you are using a look up table.
	virtual int GetAnalogActionOrigins( InputHandle_t inputHandle, InputActionSetHandle_t actionSetHandle, InputAnalogActionHandle_t analogActionHandle, STEAM_OUT_ARRAY_COUNT( STEAM_INPUT_MAX_ORIGINS, Receives list of action origins ) EInputActionOrigin *originsOut ) = 0;
	
	// Get a local path to art for on-screen glyph for a particular origin
	virtual const char *GetGlyphForActionOrigin( EInputActionOrigin eOrigin ) = 0;
	
	// Returns a localized string (from Steam's language setting) for the specified origin.
	virtual const char *GetStringForActionOrigin( EInputActionOrigin eOrigin ) = 0;

	// Stop analog momentum for the action if it is a mouse action in trackball mode
	virtual void StopAnalogActionMomentum( InputHandle_t inputHandle, InputAnalogActionHandle_t eAction ) = 0;

	// Returns raw motion data from the specified device
	virtual InputMotionData_t GetMotionData( InputHandle_t inputHandle ) = 0;

	//-----------------------------------------------------------------------------
	// OUTPUTS
	//-----------------------------------------------------------------------------

	// Trigger a vibration event on supported controllers - Steam will translate these commands into haptic pulses for Steam Controllers
	virtual void TriggerVibration( InputHandle_t inputHandle, unsigned short usLeftSpeed, unsigned short usRightSpeed ) = 0;

	// Set the controller LED color on supported controllers. nFlags is a bitmask of values from ESteamInputLEDFlag - 0 will default to setting a color. Steam will handle
	// the behavior on exit of your program so you don't need to try restore the default as you are shutting down
	virtual void SetLEDColor( InputHandle_t inputHandle, uint8 nColorR, uint8 nColorG, uint8 nColorB, unsigned int nFlags ) = 0;

	// Trigger a haptic pulse on a Steam Controller - if you are approximating rumble you may want to use TriggerVibration instead.
	// Good uses for Haptic pulses include chimes, noises, or directional gameplay feedback (taking damage, footstep locations, etc).
	virtual void TriggerHapticPulse( InputHandle_t inputHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec ) = 0;

	// Trigger a haptic pulse with a duty cycle of usDurationMicroSec / usOffMicroSec, unRepeat times. If you are approximating rumble you may want to use TriggerVibration instead.
	// nFlags is currently unused and reserved for future use.
	virtual void TriggerRepeatedHapticPulse( InputHandle_t inputHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec, unsigned short usOffMicroSec, unsigned short unRepeat, unsigned int nFlags ) = 0;

	//-----------------------------------------------------------------------------
	// Utility functions availible without using the rest of Steam Input API
	//-----------------------------------------------------------------------------

	// Invokes the Steam overlay and brings up the binding screen if the user is using Big Picture Mode
	// If the user is not in Big Picture Mode it will open up the binding in a new window
	virtual bool ShowBindingPanel( InputHandle_t inputHandle ) = 0;

	// Returns the input type for a particular handle
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
};

#endif //ISTEAMINPUT001_H
