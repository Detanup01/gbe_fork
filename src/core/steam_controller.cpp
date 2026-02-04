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

#include "gbe/steam_controller.h"
#include "settings.h"
#include "input/service.h"

using namespace gbe::input;

void Steam_Controller::steam_run_every_runcb(void *object)
{
    // PRINT_DEBUG_ENTRY();

    Steam_Controller *steam_controller = (Steam_Controller *)object;
    steam_controller->RunCallbacks();
}

Steam_Controller::Steam_Controller(class Settings *settings, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb)
{
    this->settings = settings;
    this->callback_results = callback_results;
    this->callbacks = callbacks;
    this->run_every_runcb = run_every_runcb;

    // set_handles(settings->controller_settings.action_sets); // REMOVED
    disabled = settings->controller_settings.action_sets.empty();
    initialized = false;
    
    this->run_every_runcb->add(&Steam_Controller::steam_run_every_runcb, this);
}

Steam_Controller::~Steam_Controller()
{
    this->run_every_runcb->remove(&Steam_Controller::steam_run_every_runcb, this);
    Shutdown();
}

// Init and Shutdown must be called when starting/ending use of this interface
bool Steam_Controller::Init(bool bExplicitlyCallRunFrame)
{
    PRINT_DEBUG("%u", bExplicitlyCallRunFrame);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (disabled || initialized) {
        return true;
    }

    // Initialize the new InputService
    if (!InputService::GetInstance().Initialize()) {
        PRINT_DEBUG("Failed to initialize InputService");
        return false;
    }

    // Load action manifest from Settings (Legacy Format)
    if (!settings->controller_settings.action_sets.empty()) {
        InputService::GetInstance().GetActionEngine().LoadFromSettings(settings->controller_settings);
    }
    
    initialized = true;
    explicitly_call_run_frame = bExplicitlyCallRunFrame;
    return true;
}

bool Steam_Controller::Init( const char *pchAbsolutePathToControllerConfigVDF )
{
    PRINT_DEBUG("old");
    return Init();
}

bool Steam_Controller::Init()
{
    return Init(true);
}

bool Steam_Controller::Shutdown()
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (disabled || !initialized) {
        return true;
    }

    InputService::GetInstance().Shutdown();
    initialized = false;
    return true;
}

void Steam_Controller::SetOverrideMode( const char *pchMode )
{
    PRINT_DEBUG_TODO();
}

// Set the absolute path to the Input Action Manifest file containing the in-game actions
// and file paths to the official configurations. Used in games that bundle Steam Input
// configurations inside of the game depot instead of using the Steam Workshop
bool Steam_Controller::SetInputActionManifestFilePath( const char *pchInputActionManifestAbsolutePath )
{
    PRINT_DEBUG("%s", pchInputActionManifestAbsolutePath);
    return InputService::GetInstance().GetActionEngine().LoadFromVDF(pchInputActionManifestAbsolutePath);
}

bool Steam_Controller::BWaitForData( bool bWaitForever, uint32 unTimeout )
{
    PRINT_DEBUG_TODO();
    //TODO SteamInput005
    return false;
}

// Returns true if new data has been received since the last time action data was accessed
// via GetDigitalActionData or GetAnalogActionData. The game will still need to call
// SteamInput()->RunFrame() or SteamAPI_RunCallbacks() before this to update the data stream
bool Steam_Controller::BNewDataAvailable()
{
    PRINT_DEBUG_TODO();
    //TODO SteamInput005
    return false;
}

// Enable SteamInputDeviceConnected_t and SteamInputDeviceDisconnected_t callbacks.
// Each controller that is already connected will generate a device connected
// callback when you enable them
void Steam_Controller::EnableDeviceCallbacks()
{
    PRINT_DEBUG_TODO();
    //TODO SteamInput005
    return;
}

// Enable SteamInputActionEvent_t callbacks. Directly calls your callback function
// for lower latency than standard Steam callbacks. Supports one callback at a time.
// Note: this is called within either SteamInput()->RunFrame or by SteamAPI_RunCallbacks
void Steam_Controller::EnableActionEventCallbacks( SteamInputActionEventCallbackPointer pCallback )
{
    PRINT_DEBUG_TODO();
    //TODO SteamInput005
    return;
}

// Synchronize API state with the latest Steam Controller inputs available. This
// is performed automatically by SteamAPI_RunCallbacks, but for the absolute lowest
// possible latency, you call this directly before reading controller state.
void Steam_Controller::RunFrame(bool bReservedValue)
{
    if (disabled || !initialized) {
        return;
    }
    PRINT_DEBUG_ENTRY();

    InputService::GetInstance().Update();
}

void Steam_Controller::RunFrame()
{
    RunFrame(true);
}

bool Steam_Controller::GetControllerState( uint32 unControllerIndex, SteamControllerState001_t *pState )
{
    PRINT_DEBUG_TODO();
    return false;
}

// Enumerate currently connected controllers
// handlesOut should point to a STEAM_CONTROLLER_MAX_COUNT sized array of ControllerHandle_t handles
// Returns the number of handles written to handlesOut
int Steam_Controller::GetConnectedControllers( ControllerHandle_t *handlesOut )
{
    PRINT_DEBUG_ENTRY();
    if (!handlesOut) return 0;
    if (disabled) {
        return 0;
    }

    auto connected = InputService::GetInstance().GetBackend()->GetConnectedControllers();
    int count = 0;
    for (auto h : connected) {
        if (count >= STEAM_CONTROLLER_MAX_COUNT) break;
        handlesOut[count++] = h;
    }

    PRINT_DEBUG("returned %i connected controllers", count);
    return count;
}

// Invokes the Steam overlay and brings up the binding screen
// Returns false if overlay is disabled / unavailable, or the user is not in Big Picture mode
bool Steam_Controller::ShowBindingPanel( ControllerHandle_t controllerHandle )
{
    PRINT_DEBUG_TODO();
    return false;
}

// ACTION SETS
// Lookup the handle for an Action Set. Best to do this once on startup, and store the handles for all future API calls.
ControllerActionSetHandle_t Steam_Controller::GetActionSetHandle( const char *pszActionSetName )
{
    PRINT_DEBUG("%s", pszActionSetName);
    return InputService::GetInstance().GetActionEngine().GetActionSetHandle(pszActionSetName);
}

// Reconfigure the controller to use the specified action set (ie 'Menu', 'Walk' or 'Drive')
// This is cheap, and can be safely called repeatedly. It's often easier to repeatedly call it in
// your state loops, instead of trying to place it in all of your state transitions.
void Steam_Controller::ActivateActionSet( ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetHandle )
{
    PRINT_DEBUG("%llu %llu", controllerHandle, actionSetHandle);
    if (controllerHandle == STEAM_CONTROLLER_HANDLE_ALL_CONTROLLERS) {
        for (auto handle : InputService::GetInstance().GetBackend()->GetConnectedControllers()) {
             InputService::GetInstance().GetActionEngine().ActivateActionSet(handle, actionSetHandle);
        }
    } else {
        InputService::GetInstance().GetActionEngine().ActivateActionSet(controllerHandle, actionSetHandle);
    }
}

ControllerActionSetHandle_t Steam_Controller::GetCurrentActionSet( ControllerHandle_t controllerHandle )
{
    // PRINT_DEBUG("%llu", controllerHandle);
    return InputService::GetInstance().GetActionEngine().GetCurrentActionSet(controllerHandle);
}

void Steam_Controller::ActivateActionSetLayer( ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetLayerHandle )
{
    PRINT_DEBUG("%llu %llu", controllerHandle, actionSetLayerHandle);
    if (controllerHandle == STEAM_CONTROLLER_HANDLE_ALL_CONTROLLERS) {
        for (auto handle : InputService::GetInstance().GetBackend()->GetConnectedControllers()) {
             InputService::GetInstance().GetActionEngine().ActivateActionSetLayer(handle, actionSetLayerHandle);
        }
    } else {
        InputService::GetInstance().GetActionEngine().ActivateActionSetLayer(controllerHandle, actionSetLayerHandle);
    }
}

void Steam_Controller::DeactivateActionSetLayer( ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetLayerHandle )
{
    PRINT_DEBUG("%llu %llu", controllerHandle, actionSetLayerHandle);
     if (controllerHandle == STEAM_CONTROLLER_HANDLE_ALL_CONTROLLERS) {
        for (auto handle : InputService::GetInstance().GetBackend()->GetConnectedControllers()) {
             InputService::GetInstance().GetActionEngine().DeactivateActionSetLayer(handle, actionSetLayerHandle);
        }
    } else {
        InputService::GetInstance().GetActionEngine().DeactivateActionSetLayer(controllerHandle, actionSetLayerHandle);
    }
}

void Steam_Controller::DeactivateAllActionSetLayers( ControllerHandle_t controllerHandle )
{
    PRINT_DEBUG("%llu", controllerHandle);
     if (controllerHandle == STEAM_CONTROLLER_HANDLE_ALL_CONTROLLERS) {
        for (auto handle : InputService::GetInstance().GetBackend()->GetConnectedControllers()) {
             InputService::GetInstance().GetActionEngine().DeactivateAllActionSetLayers(handle);
        }
    } else {
        InputService::GetInstance().GetActionEngine().DeactivateAllActionSetLayers(controllerHandle);
    }
}

int Steam_Controller::GetActiveActionSetLayers( ControllerHandle_t controllerHandle, ControllerActionSetHandle_t *handlesOut )
{
    std::vector<ControllerActionSetHandle_t> layers = InputService::GetInstance().GetActionEngine().GetActiveActionSetLayers(controllerHandle);
    int count = 0;
    for (auto h : layers) {
        if (count >= STEAM_CONTROLLER_MAX_ACTIVE_LAYERS) break;
        handlesOut[count++] = h;
    }
    return count;
}

// ACTIONS
// Lookup the handle for a digital action. Best to do this once on startup, and store the handles for all future API calls.
ControllerDigitalActionHandle_t Steam_Controller::GetDigitalActionHandle( const char *pszActionName )
{
    PRINT_DEBUG("%s", pszActionName);
    return InputService::GetInstance().GetActionEngine().GetDigitalActionHandle(pszActionName);
}

// Returns the current state of the supplied digital game action
ControllerDigitalActionData_t Steam_Controller::GetDigitalActionData( ControllerHandle_t controllerHandle, ControllerDigitalActionHandle_t digitalActionHandle )
{
    PRINT_DEBUG("%llu %llu", controllerHandle, digitalActionHandle);
    return InputService::GetInstance().GetActionEngine().GetDigitalActionData(controllerHandle, digitalActionHandle);
}

// Get the origin(s) for a digital action within an action set. Returns the number of origins supplied in originsOut. Use this to display the appropriate on-screen prompt for the action.
// originsOut should point to a STEAM_CONTROLLER_MAX_ORIGINS sized array of EControllerActionOrigin handles
int Steam_Controller::GetDigitalActionOrigins( ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetHandle, ControllerDigitalActionHandle_t digitalActionHandle, EControllerActionOrigin *originsOut )
{
    PRINT_DEBUG_ENTRY();
    if (actionSetHandle == 0) return 0;
    
    std::vector<int> buttons = InputService::GetInstance().GetActionEngine().GetDigitalActionOrigins(actionSetHandle, digitalActionHandle);
    
    int count = 0;
    for (int button : buttons) {
        if (count >= STEAM_CONTROLLER_MAX_ORIGINS) break;
        
        switch (button) {
            case BUTTON_A: originsOut[count] = k_EControllerActionOrigin_XBox360_A; break;
            case BUTTON_B: originsOut[count] = k_EControllerActionOrigin_XBox360_B; break;
            case BUTTON_X: originsOut[count] = k_EControllerActionOrigin_XBox360_X; break;
            case BUTTON_Y: originsOut[count] = k_EControllerActionOrigin_XBox360_Y; break;
            case BUTTON_LEFT_SHOULDER: originsOut[count] = k_EControllerActionOrigin_XBox360_LeftBumper; break;
            case BUTTON_RIGHT_SHOULDER: originsOut[count] = k_EControllerActionOrigin_XBox360_RightBumper; break;
            case BUTTON_START: originsOut[count] = k_EControllerActionOrigin_XBox360_Start; break;
            case BUTTON_BACK: originsOut[count] = k_EControllerActionOrigin_XBox360_Back; break;
            case BUTTON_LTRIGGER: originsOut[count] = k_EControllerActionOrigin_XBox360_LeftTrigger_Click; break;
            case BUTTON_RTRIGGER: originsOut[count] = k_EControllerActionOrigin_XBox360_RightTrigger_Click; break;
            case BUTTON_LEFT_THUMB: originsOut[count] = k_EControllerActionOrigin_XBox360_LeftStick_Click; break;
            case BUTTON_RIGHT_THUMB: originsOut[count] = k_EControllerActionOrigin_XBox360_RightStick_Click; break;
            case BUTTON_STICK_LEFT_UP: originsOut[count] = k_EControllerActionOrigin_XBox360_LeftStick_DPadNorth; break;
            case BUTTON_STICK_LEFT_DOWN: originsOut[count] = k_EControllerActionOrigin_XBox360_LeftStick_DPadSouth; break;
            case BUTTON_STICK_LEFT_LEFT: originsOut[count] = k_EControllerActionOrigin_XBox360_LeftStick_DPadWest; break;
            case BUTTON_STICK_LEFT_RIGHT: originsOut[count] = k_EControllerActionOrigin_XBox360_LeftStick_DPadEast; break;
            case BUTTON_STICK_RIGHT_UP: originsOut[count] = k_EControllerActionOrigin_XBox360_RightStick_DPadNorth; break;
            case BUTTON_STICK_RIGHT_DOWN: originsOut[count] = k_EControllerActionOrigin_XBox360_RightStick_DPadSouth; break;
            case BUTTON_STICK_RIGHT_LEFT: originsOut[count] = k_EControllerActionOrigin_XBox360_RightStick_DPadWest; break;
            case BUTTON_STICK_RIGHT_RIGHT: originsOut[count] = k_EControllerActionOrigin_XBox360_RightStick_DPadEast; break;
            case BUTTON_DPAD_UP: originsOut[count] = k_EControllerActionOrigin_XBox360_DPad_North; break;
            case BUTTON_DPAD_DOWN: originsOut[count] = k_EControllerActionOrigin_XBox360_DPad_South; break;
            case BUTTON_DPAD_LEFT: originsOut[count] = k_EControllerActionOrigin_XBox360_DPad_West; break;
            case BUTTON_DPAD_RIGHT: originsOut[count] = k_EControllerActionOrigin_XBox360_DPad_East; break;
            default: originsOut[count] = k_EControllerActionOrigin_None; break;
        }
        
        if (originsOut[count] != k_EControllerActionOrigin_None) {
            count++;
        }
    }
    return count;
}

int Steam_Controller::GetDigitalActionOrigins( InputHandle_t inputHandle, InputActionSetHandle_t actionSetHandle, InputDigitalActionHandle_t digitalActionHandle, EInputActionOrigin *originsOut )
{
    // Stubbed
    return 0;
}

// Returns a localized string (from Steam's language setting) for the user-facing action name corresponding to the specified handle
const char* Steam_Controller::GetStringForDigitalActionName( InputDigitalActionHandle_t eActionHandle )
{
    PRINT_DEBUG_TODO();
    //TODO SteamInput005
    return "Button String";
}

// Lookup the handle for an analog action. Best to do this once on startup, and store the handles for all future API calls.
ControllerAnalogActionHandle_t Steam_Controller::GetAnalogActionHandle( const char *pszActionName )
{
    PRINT_DEBUG("%s", pszActionName);
    return InputService::GetInstance().GetActionEngine().GetAnalogActionHandle(pszActionName);
}

// Returns the current state of these supplied analog game action
ControllerAnalogActionData_t Steam_Controller::GetAnalogActionData( ControllerHandle_t controllerHandle, ControllerAnalogActionHandle_t analogActionHandle )
{
    PRINT_DEBUG("%llu %llu", controllerHandle, analogActionHandle);
    return InputService::GetInstance().GetActionEngine().GetAnalogActionData(controllerHandle, analogActionHandle);
}

// Get the origin(s) for an analog action within an action set. Returns the number of origins supplied in originsOut. Use this to display the appropriate on-screen prompt for the action.
// originsOut should point to a STEAM_CONTROLLER_MAX_ORIGINS sized array of EControllerActionOrigin handles
int Steam_Controller::GetAnalogActionOrigins( ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetHandle, ControllerAnalogActionHandle_t analogActionHandle, EControllerActionOrigin *originsOut )
{
    PRINT_DEBUG_ENTRY();
    if (actionSetHandle == 0) return 0;

    std::vector<int> axes = InputService::GetInstance().GetActionEngine().GetAnalogActionOrigins(actionSetHandle, analogActionHandle);
    
    int count = 0;
    for (int axis : axes) {
        if (count >= STEAM_CONTROLLER_MAX_ORIGINS) break;
        
        switch (axis) {
            case 0: // Left Stick X
            case 1: // Left Stick Y
                originsOut[count] = k_EControllerActionOrigin_XBox360_LeftStick_Move; 
                break;
            case 2: // Right Stick X
            case 3: // Right Stick Y
                originsOut[count] = k_EControllerActionOrigin_XBox360_RightStick_Move; 
                break;
            case 4: // Left Trigger
                originsOut[count] = k_EControllerActionOrigin_XBox360_LeftTrigger_Pull; 
                break;
            case 5: // Right Trigger
                originsOut[count] = k_EControllerActionOrigin_XBox360_RightTrigger_Pull; 
                break;
            default: 
                originsOut[count] = k_EControllerActionOrigin_None; 
                break;
        }
        
        if (originsOut[count] != k_EControllerActionOrigin_None) {
            count++;
        }
    }
    return count;
}

int Steam_Controller::GetAnalogActionOrigins( InputHandle_t inputHandle, InputActionSetHandle_t actionSetHandle, InputAnalogActionHandle_t analogActionHandle, EInputActionOrigin *originsOut )
{
    // Stubbed
    return 0;
}
    
void Steam_Controller::StopAnalogActionMomentum( ControllerHandle_t controllerHandle, ControllerAnalogActionHandle_t eAction )
{
    PRINT_DEBUG("%llu %llu", controllerHandle, eAction);
}

// Trigger a haptic pulse on a controller
void Steam_Controller::TriggerHapticPulse( ControllerHandle_t controllerHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec )
{
    PRINT_DEBUG_TODO();
}

// Trigger a haptic pulse on a controller
void Steam_Controller::Legacy_TriggerHapticPulse( InputHandle_t inputHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec )
{
    PRINT_DEBUG_TODO();
    TriggerHapticPulse(inputHandle, eTargetPad, usDurationMicroSec );
}

void Steam_Controller::TriggerHapticPulse( uint32 unControllerIndex, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec )
{
    PRINT_DEBUG("old");
    TriggerHapticPulse(unControllerIndex, eTargetPad, usDurationMicroSec );
}

// Trigger a pulse with a duty cycle of usDurationMicroSec / usOffMicroSec, unRepeat times.
// nFlags is currently unused and reserved for future use.
void Steam_Controller::TriggerRepeatedHapticPulse( ControllerHandle_t controllerHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec, unsigned short usOffMicroSec, unsigned short unRepeat, unsigned int nFlags )
{
    PRINT_DEBUG_TODO();
}

void Steam_Controller::Legacy_TriggerRepeatedHapticPulse( InputHandle_t inputHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec, unsigned short usOffMicroSec, unsigned short unRepeat, unsigned int nFlags )
{
    PRINT_DEBUG_TODO();
    TriggerRepeatedHapticPulse(inputHandle, eTargetPad, usDurationMicroSec, usOffMicroSec, unRepeat, nFlags);
}

// Send a haptic pulse, works on Steam Deck and Steam Controller devices
void Steam_Controller::TriggerSimpleHapticEvent( InputHandle_t inputHandle, EControllerHapticLocation eHapticLocation, uint8 nIntensity, char nGainDB, uint8 nOtherIntensity, char nOtherGainDB )
{
    PRINT_DEBUG_TODO();
}

// Tigger a vibration event on supported controllers.  
void Steam_Controller::TriggerVibration( ControllerHandle_t controllerHandle, unsigned short usLeftSpeed, unsigned short usRightSpeed )
{
    PRINT_DEBUG("%hu %hu", usLeftSpeed, usRightSpeed);
    // Forward to backend
    InputService::GetInstance().GetBackend()->SetRumble(controllerHandle, usLeftSpeed, usRightSpeed, 0xFFFF);
}

// Trigger a vibration event on supported controllers including Xbox trigger impulse rumble - Steam will translate these commands into haptic pulses for Steam Controllers
void Steam_Controller::TriggerVibrationExtended( InputHandle_t inputHandle, unsigned short usLeftSpeed, unsigned short usRightSpeed, unsigned short usLeftTriggerSpeed, unsigned short usRightTriggerSpeed )
{
    PRINT_DEBUG_TODO();
    TriggerVibration(inputHandle, usLeftSpeed, usRightSpeed);
    //TODO trigger impulse rumbles
}

// Set the controller LED color on supported controllers.  
void Steam_Controller::SetLEDColor( ControllerHandle_t controllerHandle, uint8 nColorR, uint8 nColorG, uint8 nColorB, unsigned int nFlags )
{
    PRINT_DEBUG_TODO();
}

// Returns the associated gamepad index for the specified controller, if emulating a gamepad
int Steam_Controller::GetGamepadIndexForController( ControllerHandle_t ulControllerHandle )
{
    PRINT_DEBUG_ENTRY();
    PRINT_DEBUG_ENTRY();
    // Replaces legacy loop. Handles are 1-based, index 0-based.
    if (ulControllerHandle < 1) return -1;
    return static_cast<int>(ulControllerHandle) - 1;
}

// Returns the associated controller handle for the specified emulated gamepad
ControllerHandle_t Steam_Controller::GetControllerForGamepadIndex( int nIndex )
{
    PRINT_DEBUG("%i", nIndex);
    // Valid check? We can check backend if we want strict correctness
    return nIndex + 1;
}

// Returns raw motion data from the specified controller
ControllerMotionData_t Steam_Controller::GetMotionData( ControllerHandle_t controllerHandle )
{
    PRINT_DEBUG_TODO();
    ControllerMotionData_t data = {};
    return data;
}

// Attempt to display origins of given action in the controller HUD, for the currently active action set
// Returns false is overlay is disabled / unavailable, or the user is not in Big Picture mode
bool Steam_Controller::ShowDigitalActionOrigins( ControllerHandle_t controllerHandle, ControllerDigitalActionHandle_t digitalActionHandle, float flScale, float flXPosition, float flYPosition )
{
    PRINT_DEBUG_TODO();
    return true;
}

bool Steam_Controller::ShowAnalogActionOrigins( ControllerHandle_t controllerHandle, ControllerAnalogActionHandle_t analogActionHandle, float flScale, float flXPosition, float flYPosition )
{
    PRINT_DEBUG_TODO();
    return true;
}

// Returns a localized string (from Steam's language setting) for the specified origin
const char* Steam_Controller::GetStringForActionOrigin( EControllerActionOrigin eOrigin )
{
    PRINT_DEBUG_TODO();
    return "Button String";
}

const char* Steam_Controller::GetStringForActionOrigin( EInputActionOrigin eOrigin )
{
    PRINT_DEBUG_TODO();
    return "Button String";
}

// Returns a localized string (from Steam's language setting) for the user-facing action name corresponding to the specified handle
const char* Steam_Controller::GetStringForAnalogActionName( InputAnalogActionHandle_t eActionHandle )
{
    PRINT_DEBUG_TODO();
    //TODO SteamInput005
    return "Button String";
}

// Get a local path to art for on-screen glyph for a particular origin 
const char* Steam_Controller::GetGlyphForActionOrigin( EControllerActionOrigin eOrigin )
{
    static std::map<EControllerActionOrigin, std::string> s_glyphs;

    if (s_glyphs.empty()) {
        std::string dir = settings->glyphs_directory;
        s_glyphs[k_EControllerActionOrigin_XBox360_A] = dir + "button_a.png";
        s_glyphs[k_EControllerActionOrigin_XBox360_B] = dir + "button_b.png";
        s_glyphs[k_EControllerActionOrigin_XBox360_X] = dir + "button_x.png";
        s_glyphs[k_EControllerActionOrigin_XBox360_Y] = dir + "button_y.png";
        s_glyphs[k_EControllerActionOrigin_XBox360_LeftBumper] = dir + "shoulder_l.png";
        s_glyphs[k_EControllerActionOrigin_XBox360_RightBumper] = dir + "shoulder_r.png";
        s_glyphs[k_EControllerActionOrigin_XBox360_Start] = dir + "xbox_button_start.png";
        s_glyphs[k_EControllerActionOrigin_XBox360_Back] = dir + "xbox_button_select.png";
        s_glyphs[k_EControllerActionOrigin_XBox360_LeftTrigger_Pull] = dir + "trigger_l_pull.png";
        s_glyphs[k_EControllerActionOrigin_XBox360_LeftTrigger_Click] = dir + "trigger_l_click.png";
        s_glyphs[k_EControllerActionOrigin_XBox360_RightTrigger_Pull] = dir + "trigger_r_pull.png";
        s_glyphs[k_EControllerActionOrigin_XBox360_RightTrigger_Click] = dir + "trigger_r_click.png";
        s_glyphs[k_EControllerActionOrigin_XBox360_LeftStick_Move] = dir + "stick_l_move.png";
        s_glyphs[k_EControllerActionOrigin_XBox360_LeftStick_Click] = dir + "stick_l_click.png";
        s_glyphs[k_EControllerActionOrigin_XBox360_LeftStick_DPadNorth] = dir + "stick_dpad_n.png";
        s_glyphs[k_EControllerActionOrigin_XBox360_LeftStick_DPadSouth] = dir + "stick_dpad_s.png";
        s_glyphs[k_EControllerActionOrigin_XBox360_LeftStick_DPadWest] = dir + "stick_dpad_w.png";
        s_glyphs[k_EControllerActionOrigin_XBox360_LeftStick_DPadEast] = dir + "stick_dpad_e.png";
        s_glyphs[k_EControllerActionOrigin_XBox360_RightStick_Move] = dir + "stick_r_move.png";
        s_glyphs[k_EControllerActionOrigin_XBox360_RightStick_Click] = dir + "stick_r_click.png";
        s_glyphs[k_EControllerActionOrigin_XBox360_RightStick_DPadNorth] = dir + "stick_dpad_n.png";
        s_glyphs[k_EControllerActionOrigin_XBox360_RightStick_DPadSouth] = dir + "stick_dpad_s.png";
        s_glyphs[k_EControllerActionOrigin_XBox360_RightStick_DPadWest] = dir + "stick_dpad_w.png";
        s_glyphs[k_EControllerActionOrigin_XBox360_RightStick_DPadEast] = dir + "stick_dpad_e.png";
        s_glyphs[k_EControllerActionOrigin_XBox360_DPad_North] = dir + "xbox_button_dpad_n.png";
        s_glyphs[k_EControllerActionOrigin_XBox360_DPad_South] = dir + "xbox_button_dpad_s.png";
        s_glyphs[k_EControllerActionOrigin_XBox360_DPad_West] = dir + "xbox_button_dpad_w.png";
        s_glyphs[k_EControllerActionOrigin_XBox360_DPad_East] = dir + "xbox_button_dpad_e.png";
        s_glyphs[k_EControllerActionOrigin_XBox360_DPad_Move] = dir + "xbox_button_dpad_move.png";
    }

    auto glyph = s_glyphs.find(eOrigin);
    if (glyph == s_glyphs.end()) return "";
    return glyph->second.c_str();
}

const char* Steam_Controller::GetGlyphForActionOrigin( EInputActionOrigin eOrigin )
{
    // Stubbed
    return "";
}

// Get a local path to a PNG file for the provided origin's glyph. 
const char* Steam_Controller::GetGlyphPNGForActionOrigin( EInputActionOrigin eOrigin, ESteamInputGlyphSize eSize, uint32 unFlags )
{
    PRINT_DEBUG_TODO();
    //TODO SteamInput005
    return GetGlyphForActionOrigin(eOrigin);
}

// Get a local path to a SVG file for the provided origin's glyph. 
const char* Steam_Controller::GetGlyphSVGForActionOrigin( EInputActionOrigin eOrigin, uint32 unFlags )
{
    PRINT_DEBUG_TODO();
    //TODO SteamInput005
    return "";
}

// Get a local path to an older, Big Picture Mode-style PNG file for a particular origin
const char* Steam_Controller::GetGlyphForActionOrigin_Legacy( EInputActionOrigin eOrigin )
{
    PRINT_DEBUG_ENTRY();
    return GetGlyphForActionOrigin(eOrigin);
}

const char* Steam_Controller::GetStringForXboxOrigin( EXboxOrigin eOrigin )
{
    PRINT_DEBUG_TODO();
    return "";
}

const char* Steam_Controller::GetGlyphForXboxOrigin( EXboxOrigin eOrigin )
{
    PRINT_DEBUG_TODO();
    return "";
}

EControllerActionOrigin Steam_Controller::GetActionOriginFromXboxOrigin_( ControllerHandle_t controllerHandle, EXboxOrigin eOrigin )
{
    PRINT_DEBUG_TODO();
    return k_EControllerActionOrigin_None;
}

EInputActionOrigin Steam_Controller::GetActionOriginFromXboxOrigin( InputHandle_t inputHandle, EXboxOrigin eOrigin )
{
    PRINT_DEBUG_TODO();
    return k_EInputActionOrigin_None;
}

EControllerActionOrigin Steam_Controller::TranslateActionOrigin( ESteamInputType eDestinationInputType, EControllerActionOrigin eSourceOrigin )
{
    PRINT_DEBUG_TODO();
    return k_EControllerActionOrigin_None;
}

EInputActionOrigin Steam_Controller::TranslateActionOrigin( ESteamInputType eDestinationInputType, EInputActionOrigin eSourceOrigin )
{
    PRINT_DEBUG("steaminput destinationinputtype %d sourceorigin %d", eDestinationInputType, eSourceOrigin );
 
    if (eDestinationInputType == k_ESteamInputType_XBox360Controller)
        return eSourceOrigin;
 
    return k_EInputActionOrigin_None;
}

bool Steam_Controller::GetControllerBindingRevision( ControllerHandle_t controllerHandle, int *pMajor, int *pMinor )
{
    PRINT_DEBUG_TODO();
    return false;
}

bool Steam_Controller::GetDeviceBindingRevision( InputHandle_t inputHandle, int *pMajor, int *pMinor )
{
    PRINT_DEBUG_TODO();
    return false;
}

uint32 Steam_Controller::GetRemotePlaySessionID( InputHandle_t inputHandle )
{
    PRINT_DEBUG_TODO();
    return 0;
}

// Get a bitmask of the Steam Input Configuration types opted in for the current session. Returns ESteamInputConfigurationEnableType values.?	
// Note: user can override the settings from the Steamworks Partner site so the returned values may not exactly match your default configuration
uint16 Steam_Controller::GetSessionInputConfigurationSettings()
{
    PRINT_DEBUG_TODO();
    return 0;
}

// Set the trigger effect for a DualSense controller
void Steam_Controller::SetDualSenseTriggerEffect( InputHandle_t inputHandle, const ScePadTriggerEffectParam *pParam )
{
    PRINT_DEBUG_TODO();
}

void Steam_Controller::RunCallbacks()
{
    if (explicitly_call_run_frame) {
        RunFrame();
    }
}

ESteamInputType Steam_Controller::GetInputTypeForHandle( ControllerHandle_t controllerHandle )
{
    PRINT_DEBUG("%llu", controllerHandle);
    return k_ESteamInputType_XBox360Controller;
}
