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

#ifndef __INCLUDED_STEAM_CONTROLLER_H__
#define __INCLUDED_STEAM_CONTROLLER_H__

#include "base.h"


struct Controller_Map {
    std::map<ControllerDigitalActionHandle_t, std::set<int>> active_digital{};
    std::map<ControllerAnalogActionHandle_t, std::pair<std::set<int>, enum EInputSourceMode>> active_analog{};
};

struct Controller_Action {
    ControllerHandle_t controller_handle{};
    struct Controller_Map active_map{};
    ControllerDigitalActionHandle_t active_set{};

    Controller_Action(ControllerHandle_t controller_handle);

    void activate_action_set(ControllerDigitalActionHandle_t active_set, std::map<ControllerActionSetHandle_t, struct Controller_Map> &controller_maps);
    std::set<int> button_id(ControllerDigitalActionHandle_t handle);
    std::pair<std::set<int>, enum EInputSourceMode> analog_id(ControllerAnalogActionHandle_t handle);
};


struct Rumble_Thread_Data {
    std::condition_variable rumble_thread_cv{};
    bool kill_rumble_thread{};
    std::mutex rumble_mutex{};

    struct Rumble_Data {
        unsigned short left{}, right{}, last_left{}, last_right{};
        unsigned int rumble_length_ms{};
        bool new_data{};
    } data[GAMEPAD_COUNT];
};

enum EXTRA_GAMEPAD_BUTTONS {
    BUTTON_LTRIGGER = BUTTON_COUNT + 1,
    BUTTON_RTRIGGER = BUTTON_COUNT + 2,
    BUTTON_STICK_LEFT_UP = BUTTON_COUNT + 3,
    BUTTON_STICK_LEFT_DOWN = BUTTON_COUNT + 4,
    BUTTON_STICK_LEFT_LEFT = BUTTON_COUNT + 5,
    BUTTON_STICK_LEFT_RIGHT = BUTTON_COUNT + 6,
    BUTTON_STICK_RIGHT_UP = BUTTON_COUNT + 7,
    BUTTON_STICK_RIGHT_DOWN = BUTTON_COUNT + 8,
    BUTTON_STICK_RIGHT_LEFT = BUTTON_COUNT + 9,
    BUTTON_STICK_RIGHT_RIGHT = BUTTON_COUNT + 10,
};

class Steam_Controller :
// --- ISteamController
public ISteamController001,
public ISteamController003,
public ISteamController004,
public ISteamController005,
public ISteamController006,
public ISteamController007,
public ISteamController,
// ---

// --- ISteamInput
public ISteamInput001,
public ISteamInput002,
public ISteamInput005,
public ISteamInput
// ---
{
    static const std::map<std::string, int> button_strings;
    static const std::map<std::string, int> analog_strings;
    static const std::map<std::string, enum EInputSourceMode> analog_input_modes;

    class Settings *settings{};
    class SteamCallResults *callback_results{};
    class SteamCallBacks *callbacks{};
    class RunEveryRunCB *run_every_runcb{};

    std::map<std::string, ControllerActionSetHandle_t> action_handles{};
    std::map<std::string, ControllerDigitalActionHandle_t> digital_action_handles{};
    std::map<std::string, ControllerAnalogActionHandle_t> analog_action_handles{};

    std::map<ControllerActionSetHandle_t, struct Controller_Map> controller_maps{};
    std::map<ControllerHandle_t, struct Controller_Action> controllers{};

    std::map<EInputActionOrigin, std::string> steaminput_glyphs{};
    std::map<EControllerActionOrigin, std::string> steamcontroller_glyphs{};

    std::thread background_rumble_thread{};
    Rumble_Thread_Data *rumble_thread_data{};

    bool disabled{};
    bool initialized{};
    bool explicitly_call_run_frame{};

    void set_handles(std::map<std::string, std::map<std::string, std::pair<std::set<std::string>, std::string>>> action_sets);

    void RunCallbacks();

    static void background_rumble(Rumble_Thread_Data *data);
    static void steam_run_every_runcb(void *object);

public:
    Steam_Controller(class Settings *settings, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb);
    ~Steam_Controller();

    // Init and Shutdown must be called when starting/ending use of this interface
    bool Init(bool bExplicitlyCallRunFrame);

    bool Init( const char *pchAbsolutePathToControllerConfigVDF );

    bool Init();

    bool Shutdown();

    void SetOverrideMode( const char *pchMode );

    // Set the absolute path to the Input Action Manifest file containing the in-game actions
    // and file paths to the official configurations. Used in games that bundle Steam Input
    // configurations inside of the game depot instead of using the Steam Workshop
    bool SetInputActionManifestFilePath( const char *pchInputActionManifestAbsolutePath );

    bool BWaitForData( bool bWaitForever, uint32 unTimeout );

    // Returns true if new data has been received since the last time action data was accessed
    // via GetDigitalActionData or GetAnalogActionData. The game will still need to call
    // SteamInput()->RunFrame() or SteamAPI_RunCallbacks() before this to update the data stream
    bool BNewDataAvailable();

    // Enable SteamInputDeviceConnected_t and SteamInputDeviceDisconnected_t callbacks.
    // Each controller that is already connected will generate a device connected
    // callback when you enable them
    void EnableDeviceCallbacks();

    // Enable SteamInputActionEvent_t callbacks. Directly calls your callback function
    // for lower latency than standard Steam callbacks. Supports one callback at a time.
    // Note: this is called within either SteamInput()->RunFrame or by SteamAPI_RunCallbacks
    void EnableActionEventCallbacks( SteamInputActionEventCallbackPointer pCallback );

    // Synchronize API state with the latest Steam Controller inputs available. This
    // is performed automatically by SteamAPI_RunCallbacks, but for the absolute lowest
    // possible latency, you call this directly before reading controller state.
    void RunFrame(bool bReservedValue);

    void RunFrame();

    bool GetControllerState( uint32 unControllerIndex, SteamControllerState001_t *pState );

    // Enumerate currently connected controllers
    // handlesOut should point to a STEAM_CONTROLLER_MAX_COUNT sized array of ControllerHandle_t handles
    // Returns the number of handles written to handlesOut
    int GetConnectedControllers( ControllerHandle_t *handlesOut );


    // Invokes the Steam overlay and brings up the binding screen
    // Returns false is overlay is disabled / unavailable, or the user is not in Big Picture mode
    bool ShowBindingPanel( ControllerHandle_t controllerHandle );


    // ACTION SETS
    // Lookup the handle for an Action Set. Best to do this once on startup, and store the handles for all future API calls.
    ControllerActionSetHandle_t GetActionSetHandle( const char *pszActionSetName );


    // Reconfigure the controller to use the specified action set (ie 'Menu', 'Walk' or 'Drive')
    // This is cheap, and can be safely called repeatedly. It's often easier to repeatedly call it in
    // your state loops, instead of trying to place it in all of your state transitions.
    void ActivateActionSet( ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetHandle );

    ControllerActionSetHandle_t GetCurrentActionSet( ControllerHandle_t controllerHandle );


    void ActivateActionSetLayer( ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetLayerHandle );

    void DeactivateActionSetLayer( ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetLayerHandle );

    void DeactivateAllActionSetLayers( ControllerHandle_t controllerHandle );

    int GetActiveActionSetLayers( ControllerHandle_t controllerHandle, ControllerActionSetHandle_t *handlesOut );



    // ACTIONS
    // Lookup the handle for a digital action. Best to do this once on startup, and store the handles for all future API calls.
    ControllerDigitalActionHandle_t GetDigitalActionHandle( const char *pszActionName );


    // Returns the current state of the supplied digital game action
    ControllerDigitalActionData_t GetDigitalActionData( ControllerHandle_t controllerHandle, ControllerDigitalActionHandle_t digitalActionHandle );


    // Get the origin(s) for a digital action within an action set. Returns the number of origins supplied in originsOut. Use this to display the appropriate on-screen prompt for the action.
    // originsOut should point to a STEAM_CONTROLLER_MAX_ORIGINS sized array of EControllerActionOrigin handles
    int GetDigitalActionOrigins( ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetHandle, ControllerDigitalActionHandle_t digitalActionHandle, EControllerActionOrigin *originsOut );

    int GetDigitalActionOrigins( InputHandle_t inputHandle, InputActionSetHandle_t actionSetHandle, InputDigitalActionHandle_t digitalActionHandle, EInputActionOrigin *originsOut );

    // Returns a localized string (from Steam's language setting) for the user-facing action name corresponding to the specified handle
    const char *GetStringForDigitalActionName( InputDigitalActionHandle_t eActionHandle );

    // Lookup the handle for an analog action. Best to do this once on startup, and store the handles for all future API calls.
    ControllerAnalogActionHandle_t GetAnalogActionHandle( const char *pszActionName );


    // Returns the current state of these supplied analog game action
    ControllerAnalogActionData_t GetAnalogActionData( ControllerHandle_t controllerHandle, ControllerAnalogActionHandle_t analogActionHandle );


    // Get the origin(s) for an analog action within an action set. Returns the number of origins supplied in originsOut. Use this to display the appropriate on-screen prompt for the action.
    // originsOut should point to a STEAM_CONTROLLER_MAX_ORIGINS sized array of EControllerActionOrigin handles
    int GetAnalogActionOrigins( ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetHandle, ControllerAnalogActionHandle_t analogActionHandle, EControllerActionOrigin *originsOut );

    int GetAnalogActionOrigins( InputHandle_t inputHandle, InputActionSetHandle_t actionSetHandle, InputAnalogActionHandle_t analogActionHandle, EInputActionOrigin *originsOut );

        
    void StopAnalogActionMomentum( ControllerHandle_t controllerHandle, ControllerAnalogActionHandle_t eAction );


    // Trigger a haptic pulse on a controller
    void TriggerHapticPulse( ControllerHandle_t controllerHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec );

    // Trigger a haptic pulse on a controller
    void Legacy_TriggerHapticPulse( InputHandle_t inputHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec );

    void TriggerHapticPulse( uint32 unControllerIndex, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec );

    // Trigger a pulse with a duty cycle of usDurationMicroSec / usOffMicroSec, unRepeat times.
    // nFlags is currently unused and reserved for future use.
    void TriggerRepeatedHapticPulse( ControllerHandle_t controllerHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec, unsigned short usOffMicroSec, unsigned short unRepeat, unsigned int nFlags );

    void Legacy_TriggerRepeatedHapticPulse( InputHandle_t inputHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec, unsigned short usOffMicroSec, unsigned short unRepeat, unsigned int nFlags );


    // Send a haptic pulse, works on Steam Deck and Steam Controller devices
    void TriggerSimpleHapticEvent( InputHandle_t inputHandle, EControllerHapticLocation eHapticLocation, uint8 nIntensity, char nGainDB, uint8 nOtherIntensity, char nOtherGainDB );

    // Tigger a vibration event on supported controllers.  
    void TriggerVibration( ControllerHandle_t controllerHandle, unsigned short usLeftSpeed, unsigned short usRightSpeed );

    // Trigger a vibration event on supported controllers including Xbox trigger impulse rumble - Steam will translate these commands into haptic pulses for Steam Controllers
    void TriggerVibrationExtended( InputHandle_t inputHandle, unsigned short usLeftSpeed, unsigned short usRightSpeed, unsigned short usLeftTriggerSpeed, unsigned short usRightTriggerSpeed );

    // Set the controller LED color on supported controllers.  
    void SetLEDColor( ControllerHandle_t controllerHandle, uint8 nColorR, uint8 nColorG, uint8 nColorB, unsigned int nFlags );


    // Returns the associated gamepad index for the specified controller, if emulating a gamepad
    int GetGamepadIndexForController( ControllerHandle_t ulControllerHandle );


    // Returns the associated controller handle for the specified emulated gamepad
    ControllerHandle_t GetControllerForGamepadIndex( int nIndex );


    // Returns raw motion data from the specified controller
    ControllerMotionData_t GetMotionData( ControllerHandle_t controllerHandle );


    // Attempt to display origins of given action in the controller HUD, for the currently active action set
    // Returns false is overlay is disabled / unavailable, or the user is not in Big Picture mode
    bool ShowDigitalActionOrigins( ControllerHandle_t controllerHandle, ControllerDigitalActionHandle_t digitalActionHandle, float flScale, float flXPosition, float flYPosition );

    bool ShowAnalogActionOrigins( ControllerHandle_t controllerHandle, ControllerAnalogActionHandle_t analogActionHandle, float flScale, float flXPosition, float flYPosition );


    // Returns a localized string (from Steam's language setting) for the specified origin
    const char *GetStringForActionOrigin( EControllerActionOrigin eOrigin );

    const char *GetStringForActionOrigin( EInputActionOrigin eOrigin );

    // Returns a localized string (from Steam's language setting) for the user-facing action name corresponding to the specified handle
    const char *GetStringForAnalogActionName( InputAnalogActionHandle_t eActionHandle );

    // Get a local path to art for on-screen glyph for a particular origin 
    const char *GetGlyphForActionOrigin( EControllerActionOrigin eOrigin );

    const char *GetGlyphForActionOrigin( EInputActionOrigin eOrigin );

    // Get a local path to a PNG file for the provided origin's glyph. 
    const char *GetGlyphPNGForActionOrigin( EInputActionOrigin eOrigin, ESteamInputGlyphSize eSize, uint32 unFlags );

    // Get a local path to a SVG file for the provided origin's glyph. 
    const char *GetGlyphSVGForActionOrigin( EInputActionOrigin eOrigin, uint32 unFlags );

    // Get a local path to an older, Big Picture Mode-style PNG file for a particular origin
    const char *GetGlyphForActionOrigin_Legacy( EInputActionOrigin eOrigin );

    // Returns the input type for a particular handle
    ESteamInputType GetInputTypeForHandle( ControllerHandle_t controllerHandle );

    const char *GetStringForXboxOrigin( EXboxOrigin eOrigin );

    const char *GetGlyphForXboxOrigin( EXboxOrigin eOrigin );

    EControllerActionOrigin GetActionOriginFromXboxOrigin_( ControllerHandle_t controllerHandle, EXboxOrigin eOrigin );

    EInputActionOrigin GetActionOriginFromXboxOrigin( InputHandle_t inputHandle, EXboxOrigin eOrigin );

    EControllerActionOrigin TranslateActionOrigin( ESteamInputType eDestinationInputType, EControllerActionOrigin eSourceOrigin );

    EInputActionOrigin TranslateActionOrigin( ESteamInputType eDestinationInputType, EInputActionOrigin eSourceOrigin );

    bool GetControllerBindingRevision( ControllerHandle_t controllerHandle, int *pMajor, int *pMinor );

    bool GetDeviceBindingRevision( InputHandle_t inputHandle, int *pMajor, int *pMinor );

    uint32 GetRemotePlaySessionID( InputHandle_t inputHandle );

    // Get a bitmask of the Steam Input Configuration types opted in for the current session. Returns ESteamInputConfigurationEnableType values.?	
    // Note: user can override the settings from the Steamworks Partner site so the returned values may not exactly match your default configuration
    uint16 GetSessionInputConfigurationSettings();

    // Set the trigger effect for a DualSense controller
    void SetDualSenseTriggerEffect( InputHandle_t inputHandle, const ScePadTriggerEffectParam *pParam );

};



#endif // __INCLUDED_STEAM_CONTROLLER_H__
