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

#include "dll/steam_controller.h"

#define JOY_ID_START 10
#define STICK_DPAD 3
#define DEADZONE_BUTTON_STICK 0.3



#if !defined(CONTROLLER_SUPPORT)

inline void GamepadInit(void) {}
inline void GamepadShutdown(void) {}
inline void GamepadUpdate(void) {}
inline GAMEPAD_BOOL GamepadIsConnected(GAMEPAD_DEVICE device) { return GAMEPAD_FALSE; }
inline GAMEPAD_BOOL GamepadButtonDown(GAMEPAD_DEVICE device, GAMEPAD_BUTTON button) { return GAMEPAD_FALSE; }
inline float GamepadTriggerLength(GAMEPAD_DEVICE device, GAMEPAD_TRIGGER trigger) { return 0.0; }
inline GAMEPAD_STICKDIR GamepadStickDir(GAMEPAD_DEVICE device, GAMEPAD_STICK stick) { return STICKDIR_CENTER; }
inline void GamepadStickNormXY(GAMEPAD_DEVICE device, GAMEPAD_STICK stick, float* outX, float* outY) {}
inline float GamepadStickLength(GAMEPAD_DEVICE device, GAMEPAD_STICK stick) { return 0.0; }
inline void GamepadSetRumble(GAMEPAD_DEVICE device, float left, float right,  unsigned int rumble_length_ms) {}

#endif



Controller_Action::Controller_Action(ControllerHandle_t controller_handle) {
    this->controller_handle = controller_handle;
}

void Controller_Action::activate_action_set(ControllerDigitalActionHandle_t active_set, std::map<ControllerActionSetHandle_t, struct Controller_Map> &controller_maps) {
    auto map = controller_maps.find(active_set);
    if (map == controller_maps.end()) return;
    this->active_set = active_set;
    this->active_map = map->second;
}

std::set<int> Controller_Action::button_id(ControllerDigitalActionHandle_t handle) {
    auto a = active_map.active_digital.find(handle);
    if (a == active_map.active_digital.end()) return {};
    return a->second;
}

std::pair<std::set<int>, enum EInputSourceMode> Controller_Action::analog_id(ControllerAnalogActionHandle_t handle) {
    auto a = active_map.active_analog.find(handle);
    if (a == active_map.active_analog.end()) return std::pair<std::set<int>, enum EInputSourceMode>({}, k_EInputSourceMode_None);
    return a->second;
}



const std::map<std::string, int> Steam_Controller::button_strings = {
    {"DUP", BUTTON_DPAD_UP},
    {"DDOWN", BUTTON_DPAD_DOWN},
    {"DLEFT", BUTTON_DPAD_LEFT},
    {"DRIGHT", BUTTON_DPAD_RIGHT},
    {"START", BUTTON_START},
    {"BACK", BUTTON_BACK},
    {"LSTICK", BUTTON_LEFT_THUMB},
    {"RSTICK", BUTTON_RIGHT_THUMB},
    {"LBUMPER", BUTTON_LEFT_SHOULDER},
    {"RBUMPER", BUTTON_RIGHT_SHOULDER},
    {"A", BUTTON_A},
    {"B", BUTTON_B},
    {"X", BUTTON_X},
    {"Y", BUTTON_Y},
    {"DLTRIGGER", BUTTON_LTRIGGER},
    {"DRTRIGGER", BUTTON_RTRIGGER},
    {"DLJOYUP", BUTTON_STICK_LEFT_UP},
    {"DLJOYDOWN", BUTTON_STICK_LEFT_DOWN},
    {"DLJOYLEFT", BUTTON_STICK_LEFT_LEFT},
    {"DLJOYRIGHT", BUTTON_STICK_LEFT_RIGHT},
    {"DRJOYUP", BUTTON_STICK_RIGHT_UP},
    {"DRJOYDOWN", BUTTON_STICK_RIGHT_DOWN},
    {"DRJOYLEFT", BUTTON_STICK_RIGHT_LEFT},
    {"DRJOYRIGHT", BUTTON_STICK_RIGHT_RIGHT},
};

const std::map<std::string, int> Steam_Controller::analog_strings = {
    {"LTRIGGER", TRIGGER_LEFT},
    {"RTRIGGER", TRIGGER_RIGHT},
    {"LJOY", STICK_LEFT + JOY_ID_START},
    {"RJOY", STICK_RIGHT + JOY_ID_START},
    {"DPAD", STICK_DPAD + JOY_ID_START},
};

const std::map<std::string, enum EInputSourceMode> Steam_Controller::analog_input_modes = {
    {"joystick_move", k_EInputSourceMode_JoystickMove},
    {"joystick_camera", k_EInputSourceMode_JoystickCamera},
    {"trigger", k_EInputSourceMode_Trigger},
};


void Steam_Controller::set_handles(std::map<std::string, std::map<std::string, std::pair<std::set<std::string>, std::string>>> action_sets)
{
    uint64 handle_num = 1;
    for (auto & set : action_sets) {
        ControllerActionSetHandle_t action_handle_num = handle_num;
        ++handle_num;

        action_handles[set.first] = action_handle_num;
        for (auto & config_key : set.second) {
            uint64 current_handle_num = handle_num;
            ++handle_num;

            for (auto & button_string : config_key.second.first) {
                auto digital = button_strings.find(button_string);
                if (digital != button_strings.end()) {
                    ControllerDigitalActionHandle_t digital_handle_num = current_handle_num;

                    if (digital_action_handles.find(config_key.first) == digital_action_handles.end()) {
                        digital_action_handles[config_key.first] = digital_handle_num;
                    } else {
                        digital_handle_num = digital_action_handles[config_key.first];
                    }

                    controller_maps[action_handle_num].active_digital[digital_handle_num].insert(digital->second);
                } else {
                    auto analog = analog_strings.find(button_string);
                    if (analog != analog_strings.end()) {
                        ControllerAnalogActionHandle_t analog_handle_num = current_handle_num;

                        enum EInputSourceMode source_mode;
                        if (analog->second == TRIGGER_LEFT || analog->second == TRIGGER_RIGHT) {
                            source_mode = k_EInputSourceMode_Trigger;
                        } else {
                            source_mode = k_EInputSourceMode_JoystickMove;
                        }

                        auto input_mode = analog_input_modes.find(config_key.second.second);
                        if (input_mode != analog_input_modes.end()) {
                            source_mode = input_mode->second;
                        }

                        if (analog_action_handles.find(config_key.first) == analog_action_handles.end()) {
                            analog_action_handles[config_key.first] = analog_handle_num;
                        } else {
                            analog_handle_num = analog_action_handles[config_key.first];
                        }

                        controller_maps[action_handle_num].active_analog[analog_handle_num].first.insert(analog->second);
                        controller_maps[action_handle_num].active_analog[analog_handle_num].second = source_mode;

                    } else {
                        PRINT_DEBUG("Did not recognize controller button %s", button_string.c_str());
                        continue;
                    }
                }
            }
        }
    }
}


void Steam_Controller::background_rumble(Rumble_Thread_Data *data)
{
    while (true) {
        unsigned short left, right;
        unsigned int rumble_length_ms;
        int gamepad = -1;
        while (gamepad == -1) {
            std::unique_lock<std::mutex> lck(data->rumble_mutex);
            if (data->kill_rumble_thread) {
                return;
            }

            data->rumble_thread_cv.wait_for(lck, std::chrono::milliseconds(1000));
            if (data->kill_rumble_thread) {
                return;
            }

            for (int i = 0; i < GAMEPAD_COUNT; ++i) {
                if (data->data[i].new_data) {
                    left = data->data[i].left;
                    right = data->data[i].right;
                    rumble_length_ms = data->data[i].rumble_length_ms;
                    data->data[i].new_data = false;
                    if (data->data[i].last_left != left || data->data[i].last_right != right) {
                        gamepad = i;
                        data->data[i].last_left = left;
                        data->data[i].last_right = right;
                        break;
                    }
                }
            }
        }

        GamepadSetRumble((GAMEPAD_DEVICE)gamepad, ((float)left) / 65535.0f, ((float)right) / 65535.0f, rumble_length_ms);
    }
}

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

    set_handles(settings->controller_settings.action_sets);
    disabled = action_handles.empty();
    initialized = false;
    
    this->run_every_runcb->add(&Steam_Controller::steam_run_every_runcb, this);
}

Steam_Controller::~Steam_Controller()
{
    //TODO rm network callbacks
    //TODO rumble thread
    Shutdown();
    this->run_every_runcb->remove(&Steam_Controller::steam_run_every_runcb, this);
}

// Init and Shutdown must be called when starting/ending use of this interface
bool Steam_Controller::Init(bool bExplicitlyCallRunFrame)
{
    PRINT_DEBUG("%u", bExplicitlyCallRunFrame);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (disabled || initialized) {
        return true;
    }

    GamepadInit();
    GamepadUpdate();

    for (int i = 1; i < 5; ++i) {
        struct Controller_Action cont_action(i);
        //Activate the first action set.
        //TODO: check exactly what decides which gets activated by default
        if (action_handles.size() >= 1) {
            cont_action.activate_action_set(action_handles.begin()->second, controller_maps);
        }

        controllers.insert(std::pair<ControllerHandle_t, struct Controller_Action>(i, cont_action));
    }

    rumble_thread_data = new Rumble_Thread_Data();
    background_rumble_thread = std::thread(background_rumble, rumble_thread_data);

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

    controllers = std::map<ControllerHandle_t, struct Controller_Action>();
    rumble_thread_data->rumble_mutex.lock();
    rumble_thread_data->kill_rumble_thread = true;
    rumble_thread_data->rumble_mutex.unlock();
    rumble_thread_data->rumble_thread_cv.notify_one();
    background_rumble_thread.join();
    delete rumble_thread_data;
    rumble_thread_data = nullptr;
    GamepadShutdown();
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
    PRINT_DEBUG_TODO();
    //TODO SteamInput005
    return false;
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

    GamepadUpdate();
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

    int count = 0;
    if (GamepadIsConnected(GAMEPAD_0)) {*handlesOut = GAMEPAD_0 + 1; ++handlesOut; ++count;};
    if (GamepadIsConnected(GAMEPAD_1)) {*handlesOut = GAMEPAD_1 + 1; ++handlesOut; ++count;};
    if (GamepadIsConnected(GAMEPAD_2)) {*handlesOut = GAMEPAD_2 + 1; ++handlesOut; ++count;};
    if (GamepadIsConnected(GAMEPAD_3)) {*handlesOut = GAMEPAD_3 + 1; ++handlesOut; ++count;};

    PRINT_DEBUG("returned %i connected controllers", count);
    return count;
}


// Invokes the Steam overlay and brings up the binding screen
// Returns false is overlay is disabled / unavailable, or the user is not in Big Picture mode
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
    if (!pszActionSetName) return 0;
    std::string upper_action_name(pszActionSetName);
    std::transform(upper_action_name.begin(), upper_action_name.end(), upper_action_name.begin(),[](unsigned char c){ return std::toupper(c); });

    auto set_handle = action_handles.find(upper_action_name);
    if (set_handle == action_handles.end()) return 0;

    PRINT_DEBUG("%s ret %llu", pszActionSetName, set_handle->second);
    return set_handle->second;
}


// Reconfigure the controller to use the specified action set (ie 'Menu', 'Walk' or 'Drive')
// This is cheap, and can be safely called repeatedly. It's often easier to repeatedly call it in
// your state loops, instead of trying to place it in all of your state transitions.
void Steam_Controller::ActivateActionSet( ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetHandle )
{
    PRINT_DEBUG("%llu %llu", controllerHandle, actionSetHandle);
    if (controllerHandle == STEAM_CONTROLLER_HANDLE_ALL_CONTROLLERS) {
        for (auto & c: controllers) {
            c.second.activate_action_set(actionSetHandle, controller_maps);
        }
    }

    auto controller = controllers.find(controllerHandle);
    if (controller == controllers.end()) return;

    controller->second.activate_action_set(actionSetHandle, controller_maps);
}

ControllerActionSetHandle_t Steam_Controller::GetCurrentActionSet( ControllerHandle_t controllerHandle )
{
    //TODO: should return zero if no action set specifically activated with ActivateActionSet
    PRINT_DEBUG("%llu", controllerHandle);
    auto controller = controllers.find(controllerHandle);
    if (controller == controllers.end()) return 0;

    return controller->second.active_set;
}


void Steam_Controller::ActivateActionSetLayer( ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetLayerHandle )
{
    PRINT_DEBUG_TODO();
}

void Steam_Controller::DeactivateActionSetLayer( ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetLayerHandle )
{
    PRINT_DEBUG_TODO();
}

void Steam_Controller::DeactivateAllActionSetLayers( ControllerHandle_t controllerHandle )
{
    PRINT_DEBUG_TODO();
}

int Steam_Controller::GetActiveActionSetLayers( ControllerHandle_t controllerHandle, ControllerActionSetHandle_t *handlesOut )
{
    PRINT_DEBUG_TODO();
    return 0;
}



// ACTIONS
// Lookup the handle for a digital action. Best to do this once on startup, and store the handles for all future API calls.
ControllerDigitalActionHandle_t Steam_Controller::GetDigitalActionHandle( const char *pszActionName )
{
    PRINT_DEBUG("%s", pszActionName);
    if (!pszActionName) return 0;
    std::string upper_action_name(pszActionName);
    std::transform(upper_action_name.begin(), upper_action_name.end(), upper_action_name.begin(),[](unsigned char c){ return std::toupper(c); });

    auto handle = digital_action_handles.find(upper_action_name);
    if (handle == digital_action_handles.end()) {
        //apparently GetDigitalActionHandle also works with analog handles
        handle = analog_action_handles.find(upper_action_name);
        if (handle == analog_action_handles.end()) return 0;
    }

    PRINT_DEBUG("%s ret %llu", pszActionName, handle->second);
    return handle->second;
}


// Returns the current state of the supplied digital game action
ControllerDigitalActionData_t Steam_Controller::GetDigitalActionData( ControllerHandle_t controllerHandle, ControllerDigitalActionHandle_t digitalActionHandle )
{
    PRINT_DEBUG("%llu %llu", controllerHandle, digitalActionHandle);
    ControllerDigitalActionData_t digitalData;
    digitalData.bActive = false;
    digitalData.bState = false;

    auto controller = controllers.find(controllerHandle);
    if (controller == controllers.end()) return digitalData;

    std::set<int> buttons = controller->second.button_id(digitalActionHandle);
    if (!buttons.size()) return digitalData;
    digitalData.bActive = true;

    GAMEPAD_DEVICE device = (GAMEPAD_DEVICE)(controllerHandle - 1);

    for (auto button : buttons) {
        bool pressed = false;
        if (button < BUTTON_COUNT) {
            pressed = GamepadButtonDown(device, (GAMEPAD_BUTTON)button);
        } else {
            switch (button) {
                case BUTTON_LTRIGGER:
                    pressed = GamepadTriggerLength(device, TRIGGER_LEFT) > 0.8;
                    break;
                case BUTTON_RTRIGGER:
                    pressed = GamepadTriggerLength(device, TRIGGER_RIGHT) > 0.8;
                    break;
                case BUTTON_STICK_LEFT_UP:
                case BUTTON_STICK_LEFT_DOWN:
                case BUTTON_STICK_LEFT_LEFT:
                case BUTTON_STICK_LEFT_RIGHT: {
                    float x = 0, y = 0, len = GamepadStickLength(device, STICK_LEFT);
                    GamepadStickNormXY(device, STICK_LEFT, &x, &y);
                    x *= len;
                    y *= len;
                    if (button == BUTTON_STICK_LEFT_UP) pressed = y > DEADZONE_BUTTON_STICK;
                    if (button == BUTTON_STICK_LEFT_DOWN) pressed = y < -DEADZONE_BUTTON_STICK;
                    if (button == BUTTON_STICK_LEFT_RIGHT) pressed = x > DEADZONE_BUTTON_STICK;
                    if (button == BUTTON_STICK_LEFT_LEFT) pressed = x < -DEADZONE_BUTTON_STICK;
                    break;
                }
                case BUTTON_STICK_RIGHT_UP:
                case BUTTON_STICK_RIGHT_DOWN:
                case BUTTON_STICK_RIGHT_LEFT:
                case BUTTON_STICK_RIGHT_RIGHT: {
                    float x = 0, y = 0, len = GamepadStickLength(device, STICK_RIGHT);
                    GamepadStickNormXY(device, STICK_RIGHT, &x, &y);
                    x *= len;
                    y *= len;
                    if (button == BUTTON_STICK_RIGHT_UP) pressed = y > DEADZONE_BUTTON_STICK;
                    if (button == BUTTON_STICK_RIGHT_DOWN) pressed = y < -DEADZONE_BUTTON_STICK;
                    if (button == BUTTON_STICK_RIGHT_RIGHT) pressed = x > DEADZONE_BUTTON_STICK;
                    if (button == BUTTON_STICK_RIGHT_LEFT) pressed = x < -DEADZONE_BUTTON_STICK;
                    break;
                }
                default:
                    break;
            }
        }

        if (pressed) {
            digitalData.bState = true;
            break;
        }
    }

    return digitalData;
}


// Get the origin(s) for a digital action within an action set. Returns the number of origins supplied in originsOut. Use this to display the appropriate on-screen prompt for the action.
// originsOut should point to a STEAM_CONTROLLER_MAX_ORIGINS sized array of EControllerActionOrigin handles
int Steam_Controller::GetDigitalActionOrigins( ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetHandle, ControllerDigitalActionHandle_t digitalActionHandle, EControllerActionOrigin *originsOut )
{
    PRINT_DEBUG_ENTRY();
    EInputActionOrigin origins[STEAM_CONTROLLER_MAX_ORIGINS];
    int ret = GetDigitalActionOrigins(controllerHandle, actionSetHandle, digitalActionHandle, origins );
    for (int i = 0; i < ret; ++i) {
        originsOut[i] = (EControllerActionOrigin)(origins[i] - ((long)k_EInputActionOrigin_XBox360_A - (long)k_EControllerActionOrigin_XBox360_A));
    }

    return ret;
}

int Steam_Controller::GetDigitalActionOrigins( InputHandle_t inputHandle, InputActionSetHandle_t actionSetHandle, InputDigitalActionHandle_t digitalActionHandle, EInputActionOrigin *originsOut )
{
    PRINT_DEBUG_ENTRY();
    auto controller = controllers.find(inputHandle);
    if (controller == controllers.end()) return 0;

    auto map = controller_maps.find(actionSetHandle);
    if (map == controller_maps.end()) return 0;

    auto a = map->second.active_digital.find(digitalActionHandle);
    if (a == map->second.active_digital.end()) return 0;

    int count = 0;
    for (auto button: a->second) {
        switch (button) {
            case BUTTON_A:
                originsOut[count] = k_EInputActionOrigin_XBox360_A;
                break;
            case BUTTON_B:
                originsOut[count] = k_EInputActionOrigin_XBox360_B;
                break;
            case BUTTON_X:
                originsOut[count] = k_EInputActionOrigin_XBox360_X;
                break;
            case BUTTON_Y:
                originsOut[count] = k_EInputActionOrigin_XBox360_Y;
                break;
            case BUTTON_LEFT_SHOULDER:
                originsOut[count] = k_EInputActionOrigin_XBox360_LeftBumper;
                break;
            case BUTTON_RIGHT_SHOULDER:
                originsOut[count] = k_EInputActionOrigin_XBox360_RightBumper;
                break;
            case BUTTON_START:
                originsOut[count] = k_EInputActionOrigin_XBox360_Start;
                break;
            case BUTTON_BACK:
                originsOut[count] = k_EInputActionOrigin_XBox360_Back;
                break;
            case BUTTON_LTRIGGER:
                originsOut[count] = k_EInputActionOrigin_XBox360_LeftTrigger_Click;
                break;
            case BUTTON_RTRIGGER:
                originsOut[count] = k_EInputActionOrigin_XBox360_RightTrigger_Click;
                break;
            case BUTTON_LEFT_THUMB:
                originsOut[count] = k_EInputActionOrigin_XBox360_LeftStick_Click;
                break;
            case BUTTON_RIGHT_THUMB:
                originsOut[count] = k_EInputActionOrigin_XBox360_RightStick_Click;
                break;

            case BUTTON_STICK_LEFT_UP:
                originsOut[count] = k_EInputActionOrigin_XBox360_LeftStick_DPadNorth;
                break;
            case BUTTON_STICK_LEFT_DOWN:
                originsOut[count] = k_EInputActionOrigin_XBox360_LeftStick_DPadSouth;
                break;
            case BUTTON_STICK_LEFT_LEFT:
                originsOut[count] = k_EInputActionOrigin_XBox360_LeftStick_DPadWest;
                break;
            case BUTTON_STICK_LEFT_RIGHT:
                originsOut[count] = k_EInputActionOrigin_XBox360_LeftStick_DPadEast;
                break;

            case BUTTON_STICK_RIGHT_UP:
                originsOut[count] = k_EInputActionOrigin_XBox360_RightStick_DPadNorth;
                break;
            case BUTTON_STICK_RIGHT_DOWN:
                originsOut[count] = k_EInputActionOrigin_XBox360_RightStick_DPadSouth;
                break;
            case BUTTON_STICK_RIGHT_LEFT:
                originsOut[count] = k_EInputActionOrigin_XBox360_RightStick_DPadWest;
                break;
            case BUTTON_STICK_RIGHT_RIGHT:
                originsOut[count] = k_EInputActionOrigin_XBox360_RightStick_DPadEast;
                break;

            case BUTTON_DPAD_UP:
                originsOut[count] = k_EInputActionOrigin_XBox360_DPad_North;
                break;
            case BUTTON_DPAD_DOWN:
                originsOut[count] = k_EInputActionOrigin_XBox360_DPad_South;
                break;
            case BUTTON_DPAD_LEFT:
                originsOut[count] = k_EInputActionOrigin_XBox360_DPad_West;
                break;
            case BUTTON_DPAD_RIGHT:
                originsOut[count] = k_EInputActionOrigin_XBox360_DPad_East;
                break;

            default:
                originsOut[count] = k_EInputActionOrigin_None;
                break;
        }

        ++count;
        if (count >= STEAM_INPUT_MAX_ORIGINS) {
            break;
        }
    }

    return count;
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
    if (!pszActionName) return 0;
    std::string upper_action_name(pszActionName);
    std::transform(upper_action_name.begin(), upper_action_name.end(), upper_action_name.begin(),[](unsigned char c){ return std::toupper(c); });

    auto handle = analog_action_handles.find(upper_action_name);
    if (handle == analog_action_handles.end()) return 0;

    return handle->second;
}


// Returns the current state of these supplied analog game action
ControllerAnalogActionData_t Steam_Controller::GetAnalogActionData( ControllerHandle_t controllerHandle, ControllerAnalogActionHandle_t analogActionHandle )
{
    PRINT_DEBUG("%llu %llu", controllerHandle, analogActionHandle);
    GAMEPAD_DEVICE device = (GAMEPAD_DEVICE)(controllerHandle - 1);

    ControllerAnalogActionData_t data;
    data.eMode = k_EInputSourceMode_None;
    data.x = data.y = 0;
    data.bActive = false;

    auto controller = controllers.find(controllerHandle);
    if (controller == controllers.end()) return data;

    auto analog = controller->second.analog_id(analogActionHandle);
    if (!analog.first.size()) return data;

    data.bActive = true;
    data.eMode = analog.second;

    for (auto a : analog.first) {
        if (a >= JOY_ID_START) {
            int joystick_id = a - JOY_ID_START;
            if (joystick_id == STICK_DPAD) {
                int mov_y = (int)GamepadButtonDown(device, BUTTON_DPAD_UP) - (int)GamepadButtonDown(device, BUTTON_DPAD_DOWN);
                int mov_x = (int)GamepadButtonDown(device, BUTTON_DPAD_RIGHT) - (int)GamepadButtonDown(device, BUTTON_DPAD_LEFT);
                if (mov_y || mov_x) {
                    data.x = static_cast<float>(mov_x);
                    data.y = static_cast<float>(mov_y);
                    float length = 1.0f / std::sqrt(data.x * data.x + data.y * data.y);
                    data.x = data.x * length;
                    data.y = data.y * length;
                }
            } else {
                GamepadStickNormXY(device, (GAMEPAD_STICK) joystick_id, &data.x, &data.y);
                float length = GamepadStickLength(device, (GAMEPAD_STICK) joystick_id);
                data.x = data.x * length;
                data.y = data.y * length;
            }
        } else {
            data.x = GamepadTriggerLength(device, (GAMEPAD_TRIGGER) a);
        }

        if (data.x || data.y) {
            break;
        }
    }

    return data;
}


// Get the origin(s) for an analog action within an action set. Returns the number of origins supplied in originsOut. Use this to display the appropriate on-screen prompt for the action.
// originsOut should point to a STEAM_CONTROLLER_MAX_ORIGINS sized array of EControllerActionOrigin handles
int Steam_Controller::GetAnalogActionOrigins( ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetHandle, ControllerAnalogActionHandle_t analogActionHandle, EControllerActionOrigin *originsOut )
{
    PRINT_DEBUG_ENTRY();
    EInputActionOrigin origins[STEAM_CONTROLLER_MAX_ORIGINS];
    int ret = GetAnalogActionOrigins(controllerHandle, actionSetHandle, analogActionHandle, origins );
    for (int i = 0; i < ret; ++i) {
        originsOut[i] = (EControllerActionOrigin)(origins[i] - ((long)k_EInputActionOrigin_XBox360_A - (long)k_EControllerActionOrigin_XBox360_A));
    }

    return ret;
}

int Steam_Controller::GetAnalogActionOrigins( InputHandle_t inputHandle, InputActionSetHandle_t actionSetHandle, InputAnalogActionHandle_t analogActionHandle, EInputActionOrigin *originsOut )
{
    PRINT_DEBUG_ENTRY();
    auto controller = controllers.find(inputHandle);
    if (controller == controllers.end()) return 0;

    auto map = controller_maps.find(actionSetHandle);
    if (map == controller_maps.end()) return 0;

    auto a = map->second.active_analog.find(analogActionHandle);
    if (a == map->second.active_analog.end()) return 0;

    int count = 0;
    for (auto b: a->second.first) {
        switch (b) {
            case TRIGGER_LEFT:
                originsOut[count] = k_EInputActionOrigin_XBox360_LeftTrigger_Pull;
                break;
            case TRIGGER_RIGHT:
                originsOut[count] = k_EInputActionOrigin_XBox360_RightTrigger_Pull;
                break;
            case STICK_LEFT + JOY_ID_START:
                originsOut[count] = k_EInputActionOrigin_XBox360_LeftStick_Move;
                break;
            case STICK_RIGHT + JOY_ID_START:
                originsOut[count] = k_EInputActionOrigin_XBox360_RightStick_Move;
                break;
            case STICK_DPAD + JOY_ID_START:
                originsOut[count] = k_EInputActionOrigin_XBox360_DPad_Move;
                break;
            default:
                originsOut[count] = k_EInputActionOrigin_None;
                break;
        }

        ++count;
        if (count >= STEAM_INPUT_MAX_ORIGINS) {
            break;
        }
    }

    return count;
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
    auto controller = controllers.find(controllerHandle);
    if (controller == controllers.end()) return;

    unsigned int rumble_length_ms = 0;

#if defined(__linux__)
    //FIXME: shadow of the tomb raider on linux doesn't seem to turn off the rumble so I made it expire after 100ms. Need to check if this is how linux steam actually behaves.
    rumble_length_ms = 100;
#endif

    unsigned gamepad_device = static_cast<unsigned int>(controllerHandle - 1);
    if (gamepad_device > GAMEPAD_COUNT) return;
    rumble_thread_data->rumble_mutex.lock();
    rumble_thread_data->data[gamepad_device].new_data = true;
    rumble_thread_data->data[gamepad_device].left = usLeftSpeed;
    rumble_thread_data->data[gamepad_device].right = usRightSpeed;
    rumble_thread_data->data[gamepad_device].rumble_length_ms = rumble_length_ms;
    rumble_thread_data->rumble_mutex.unlock();
    rumble_thread_data->rumble_thread_cv.notify_one();
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
    auto controller = controllers.find(ulControllerHandle);
    if (controller == controllers.end()) return -1;

    return static_cast<int>(ulControllerHandle) - 1;
}


// Returns the associated controller handle for the specified emulated gamepad
ControllerHandle_t Steam_Controller::GetControllerForGamepadIndex( int nIndex )
{
    PRINT_DEBUG("%i", nIndex);
    ControllerHandle_t out = nIndex + 1;
    auto controller = controllers.find(out);
    if (controller == controllers.end()) return 0;
    return out;
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
    PRINT_DEBUG("%i", eOrigin);

    if (steamcontroller_glyphs.empty()) {
        std::string dir = settings->glyphs_directory;
        steamcontroller_glyphs[k_EControllerActionOrigin_XBox360_A] = dir + "button_a.png";
        steamcontroller_glyphs[k_EControllerActionOrigin_XBox360_B] = dir + "button_b.png";
        steamcontroller_glyphs[k_EControllerActionOrigin_XBox360_X] = dir + "button_x.png";
        steamcontroller_glyphs[k_EControllerActionOrigin_XBox360_Y] = dir + "button_y.png";
        steamcontroller_glyphs[k_EControllerActionOrigin_XBox360_LeftBumper] = dir + "shoulder_l.png";
        steamcontroller_glyphs[k_EControllerActionOrigin_XBox360_RightBumper] = dir + "shoulder_r.png";
        steamcontroller_glyphs[k_EControllerActionOrigin_XBox360_Start] = dir + "xbox_button_start.png";
        steamcontroller_glyphs[k_EControllerActionOrigin_XBox360_Back] = dir + "xbox_button_select.png";
        steamcontroller_glyphs[k_EControllerActionOrigin_XBox360_LeftTrigger_Pull] = dir + "trigger_l_pull.png";
        steamcontroller_glyphs[k_EControllerActionOrigin_XBox360_LeftTrigger_Click] = dir + "trigger_l_click.png";
        steamcontroller_glyphs[k_EControllerActionOrigin_XBox360_RightTrigger_Pull] = dir + "trigger_r_pull.png";
        steamcontroller_glyphs[k_EControllerActionOrigin_XBox360_RightTrigger_Click] = dir + "trigger_r_click.png";
        steamcontroller_glyphs[k_EControllerActionOrigin_XBox360_LeftStick_Move] = dir + "stick_l_move.png";
        steamcontroller_glyphs[k_EControllerActionOrigin_XBox360_LeftStick_Click] = dir + "stick_l_click.png";
        steamcontroller_glyphs[k_EControllerActionOrigin_XBox360_LeftStick_DPadNorth] = dir + "stick_dpad_n.png";
        steamcontroller_glyphs[k_EControllerActionOrigin_XBox360_LeftStick_DPadSouth] = dir + "stick_dpad_s.png";
        steamcontroller_glyphs[k_EControllerActionOrigin_XBox360_LeftStick_DPadWest] = dir + "stick_dpad_w.png";
        steamcontroller_glyphs[k_EControllerActionOrigin_XBox360_LeftStick_DPadEast] = dir + "stick_dpad_e.png";
        steamcontroller_glyphs[k_EControllerActionOrigin_XBox360_RightStick_Move] = dir + "stick_r_move.png";
        steamcontroller_glyphs[k_EControllerActionOrigin_XBox360_RightStick_Click] = dir + "stick_r_click.png";
        steamcontroller_glyphs[k_EControllerActionOrigin_XBox360_RightStick_DPadNorth] = dir + "stick_dpad_n.png";
        steamcontroller_glyphs[k_EControllerActionOrigin_XBox360_RightStick_DPadSouth] = dir + "stick_dpad_s.png";
        steamcontroller_glyphs[k_EControllerActionOrigin_XBox360_RightStick_DPadWest] = dir + "stick_dpad_w.png";
        steamcontroller_glyphs[k_EControllerActionOrigin_XBox360_RightStick_DPadEast] = dir + "stick_dpad_e.png";
        steamcontroller_glyphs[k_EControllerActionOrigin_XBox360_DPad_North] = dir + "xbox_button_dpad_n.png";
        steamcontroller_glyphs[k_EControllerActionOrigin_XBox360_DPad_South] = dir + "xbox_button_dpad_s.png";
        steamcontroller_glyphs[k_EControllerActionOrigin_XBox360_DPad_West] = dir + "xbox_button_dpad_w.png";
        steamcontroller_glyphs[k_EControllerActionOrigin_XBox360_DPad_East] = dir + "xbox_button_dpad_e.png";
        steamcontroller_glyphs[k_EControllerActionOrigin_XBox360_DPad_Move] = dir + "xbox_button_dpad_move.png";
    }

    auto glyph = steamcontroller_glyphs.find(eOrigin);
    if (glyph == steamcontroller_glyphs.end()) return "";
    return glyph->second.c_str();
}

const char* Steam_Controller::GetGlyphForActionOrigin( EInputActionOrigin eOrigin )
{
    PRINT_DEBUG("steaminput %i", eOrigin);
    if (steaminput_glyphs.empty()) {
        std::string dir = settings->glyphs_directory;
        steaminput_glyphs[k_EInputActionOrigin_XBox360_A] = dir + "button_a.png";
        steaminput_glyphs[k_EInputActionOrigin_XBox360_B] = dir + "button_b.png";
        steaminput_glyphs[k_EInputActionOrigin_XBox360_X] = dir + "button_x.png";
        steaminput_glyphs[k_EInputActionOrigin_XBox360_Y] = dir + "button_y.png";
        steaminput_glyphs[k_EInputActionOrigin_XBox360_LeftBumper] = dir + "shoulder_l.png";
        steaminput_glyphs[k_EInputActionOrigin_XBox360_RightBumper] = dir + "shoulder_r.png";
        steaminput_glyphs[k_EInputActionOrigin_XBox360_Start] = dir + "xbox_button_start.png";
        steaminput_glyphs[k_EInputActionOrigin_XBox360_Back] = dir + "xbox_button_select.png";
        steaminput_glyphs[k_EInputActionOrigin_XBox360_LeftTrigger_Pull] = dir + "trigger_l_pull.png";
        steaminput_glyphs[k_EInputActionOrigin_XBox360_LeftTrigger_Click] = dir + "trigger_l_click.png";
        steaminput_glyphs[k_EInputActionOrigin_XBox360_RightTrigger_Pull] = dir + "trigger_r_pull.png";
        steaminput_glyphs[k_EInputActionOrigin_XBox360_RightTrigger_Click] = dir + "trigger_r_click.png";
        steaminput_glyphs[k_EInputActionOrigin_XBox360_LeftStick_Move] = dir + "stick_l_move.png";
        steaminput_glyphs[k_EInputActionOrigin_XBox360_LeftStick_Click] = dir + "stick_l_click.png";
        steaminput_glyphs[k_EInputActionOrigin_XBox360_LeftStick_DPadNorth] = dir + "stick_dpad_n.png";
        steaminput_glyphs[k_EInputActionOrigin_XBox360_LeftStick_DPadSouth] = dir + "stick_dpad_s.png";
        steaminput_glyphs[k_EInputActionOrigin_XBox360_LeftStick_DPadWest] = dir + "stick_dpad_w.png";
        steaminput_glyphs[k_EInputActionOrigin_XBox360_LeftStick_DPadEast] = dir + "stick_dpad_e.png";
        steaminput_glyphs[k_EInputActionOrigin_XBox360_RightStick_Move] = dir + "stick_r_move.png";
        steaminput_glyphs[k_EInputActionOrigin_XBox360_RightStick_Click] = dir + "stick_r_click.png";
        steaminput_glyphs[k_EInputActionOrigin_XBox360_RightStick_DPadNorth] = dir + "stick_dpad_n.png";
        steaminput_glyphs[k_EInputActionOrigin_XBox360_RightStick_DPadSouth] = dir + "stick_dpad_s.png";
        steaminput_glyphs[k_EInputActionOrigin_XBox360_RightStick_DPadWest] = dir + "stick_dpad_w.png";
        steaminput_glyphs[k_EInputActionOrigin_XBox360_RightStick_DPadEast] = dir + "stick_dpad_e.png";
        steaminput_glyphs[k_EInputActionOrigin_XBox360_DPad_North] = dir + "xbox_button_dpad_n.png";
        steaminput_glyphs[k_EInputActionOrigin_XBox360_DPad_South] = dir + "xbox_button_dpad_s.png";
        steaminput_glyphs[k_EInputActionOrigin_XBox360_DPad_West] = dir + "xbox_button_dpad_w.png";
        steaminput_glyphs[k_EInputActionOrigin_XBox360_DPad_East] = dir + "xbox_button_dpad_e.png";
        steaminput_glyphs[k_EInputActionOrigin_XBox360_DPad_Move] = dir + "xbox_button_dpad_move.png";
        //steaminput_glyphs[] = dir + "";
    }

    auto glyph = steaminput_glyphs.find(eOrigin);
    if (glyph == steaminput_glyphs.end()) return "";
    return glyph->second.c_str();
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

// Returns the input type for a particular handle
ESteamInputType Steam_Controller::GetInputTypeForHandle( ControllerHandle_t controllerHandle )
{
    PRINT_DEBUG("%llu", controllerHandle);
    auto controller = controllers.find(controllerHandle);
    if (controller == controllers.end()) return k_ESteamInputType_Unknown;
    return k_ESteamInputType_XBox360Controller;
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
