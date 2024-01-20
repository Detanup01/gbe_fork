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

#include "base.h"
#ifndef CONTROLLER_SUPPORT
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

struct Controller_Map {
    std::map<ControllerDigitalActionHandle_t, std::set<int>> active_digital;
    std::map<ControllerAnalogActionHandle_t, std::pair<std::set<int>, enum EInputSourceMode>> active_analog;
};

struct Controller_Action {
    ControllerHandle_t controller_handle;
    struct Controller_Map active_map;
    ControllerDigitalActionHandle_t active_set;

    Controller_Action(ControllerHandle_t controller_handle) {
        this->controller_handle = controller_handle;
    }

    void activate_action_set(ControllerDigitalActionHandle_t active_set, std::map<ControllerActionSetHandle_t, struct Controller_Map> &controller_maps) {
        auto map = controller_maps.find(active_set);
        if (map == controller_maps.end()) return;
        this->active_set = active_set;
        this->active_map = map->second;
    }

    std::set<int> button_id(ControllerDigitalActionHandle_t handle) {
        auto a = active_map.active_digital.find(handle);
        if (a == active_map.active_digital.end()) return {};
        return a->second;
    }

    std::pair<std::set<int>, enum EInputSourceMode> analog_id(ControllerAnalogActionHandle_t handle) {
        auto a = active_map.active_analog.find(handle);
        if (a == active_map.active_analog.end()) return std::pair<std::set<int>, enum EInputSourceMode>({}, k_EInputSourceMode_None);
        return a->second;
    }
};


struct Rumble_Thread_Data {
    std::condition_variable rumble_thread_cv;
    bool kill_rumble_thread;
    std::mutex rumble_mutex;

    struct Rumble_Data {
        unsigned short left, right, last_left, last_right;
        unsigned int rumble_length_ms;
        bool new_data;
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

#define JOY_ID_START 10
#define STICK_DPAD 3
#define DEADZONE_BUTTON_STICK 0.3

class Steam_Controller :
public ISteamController001,
public ISteamController003,
public ISteamController004,
public ISteamController005,
public ISteamController006,
public ISteamController007,
public ISteamController,
public ISteamInput001,
public ISteamInput002,
public ISteamInput005,
public ISteamInput
{
    class Settings *settings;
    class SteamCallResults *callback_results;
    class SteamCallBacks *callbacks;
    class RunEveryRunCB *run_every_runcb;

    std::map<std::string, int> button_strings = {
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

    std::map<std::string, int> analog_strings = {
        {"LTRIGGER", TRIGGER_LEFT},
        {"RTRIGGER", TRIGGER_RIGHT},
        {"LJOY", STICK_LEFT + JOY_ID_START},
        {"RJOY", STICK_RIGHT + JOY_ID_START},
        {"DPAD", STICK_DPAD + JOY_ID_START},
    };

    std::map<std::string, enum EInputSourceMode> analog_input_modes = {
        {"joystick_move", k_EInputSourceMode_JoystickMove},
        {"joystick_camera", k_EInputSourceMode_JoystickCamera},
        {"trigger", k_EInputSourceMode_Trigger},
    };

    std::map<std::string, ControllerActionSetHandle_t> action_handles;
    std::map<std::string, ControllerDigitalActionHandle_t> digital_action_handles;
    std::map<std::string, ControllerAnalogActionHandle_t> analog_action_handles;

    std::map<ControllerActionSetHandle_t, struct Controller_Map> controller_maps;
    std::map<ControllerHandle_t, struct Controller_Action> controllers;

    std::map<EInputActionOrigin, std::string> steaminput_glyphs;
    std::map<EControllerActionOrigin, std::string> steamcontroller_glyphs;

    std::thread background_rumble_thread;
    Rumble_Thread_Data *rumble_thread_data;

    bool disabled;
    bool initialized;
    bool explicitly_call_run_frame;

    void set_handles(std::map<std::string, std::map<std::string, std::pair<std::set<std::string>, std::string>>> action_sets) {
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
                            PRINT_DEBUG("Did not recognize controller button %s\n", button_string.c_str());
                            continue;
                        }
                    }
                }
            }
        }
    }

public:

static void background_rumble(Rumble_Thread_Data *data)
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

        GamepadSetRumble((GAMEPAD_DEVICE)gamepad, ((double)left) / 65535.0, ((double)right) / 65535.0, rumble_length_ms);
    }
}

static void steam_run_every_runcb(void *object)
{
    PRINT_DEBUG("steam_controller_run_every_runcb\n");

    Steam_Controller *steam_controller = (Steam_Controller *)object;
    steam_controller->RunCallbacks();
}

Steam_Controller(class Settings *settings, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb)
{
    this->settings = settings;
    this->run_every_runcb = run_every_runcb;
    this->run_every_runcb->add(&Steam_Controller::steam_run_every_runcb, this);

    this->callback_results = callback_results;
    this->callbacks = callbacks;

    set_handles(settings->controller_settings.action_sets);
    disabled = !action_handles.size();
    initialized = false;
}

~Steam_Controller()
{
    //TODO rm network callbacks
    //TODO rumble thread
    this->run_every_runcb->remove(&Steam_Controller::steam_run_every_runcb, this);
}

// Init and Shutdown must be called when starting/ending use of this interface
bool Init(bool bExplicitlyCallRunFrame)
{
    PRINT_DEBUG("Steam_Controller::Init() %u\n", bExplicitlyCallRunFrame);
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

bool Init( const char *pchAbsolutePathToControllerConfigVDF )
{
    PRINT_DEBUG("Steam_Controller::Init() old\n");
    return Init();
}

bool Init()
{
    return Init(true);
}

bool Shutdown()
{
    PRINT_DEBUG("Steam_Controller::Shutdown()\n");
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
    GamepadShutdown();
    initialized = false;
    return true;
}

void SetOverrideMode( const char *pchMode )
{
    PRINT_DEBUG("Steam_Controller::SetOverrideMode\n");
}

// Set the absolute path to the Input Action Manifest file containing the in-game actions
// and file paths to the official configurations. Used in games that bundle Steam Input
// configurations inside of the game depot instead of using the Steam Workshop
bool SetInputActionManifestFilePath( const char *pchInputActionManifestAbsolutePath )
{
    PRINT_DEBUG("TODO Steam_Controller::%s\n", __FUNCTION__);
    //TODO SteamInput005
    return false;
}

bool BWaitForData( bool bWaitForever, uint32 unTimeout )
{
    PRINT_DEBUG("TODO Steam_Controller::%s\n", __FUNCTION__);
    //TODO SteamInput005
    return false;
}

// Returns true if new data has been received since the last time action data was accessed
// via GetDigitalActionData or GetAnalogActionData. The game will still need to call
// SteamInput()->RunFrame() or SteamAPI_RunCallbacks() before this to update the data stream
bool BNewDataAvailable()
{
    PRINT_DEBUG("TODO Steam_Controller::%s\n", __FUNCTION__);
    //TODO SteamInput005
    return false;
}

// Enable SteamInputDeviceConnected_t and SteamInputDeviceDisconnected_t callbacks.
// Each controller that is already connected will generate a device connected
// callback when you enable them
void EnableDeviceCallbacks()
{
    PRINT_DEBUG("TODO Steam_Controller::%s\n", __FUNCTION__);
    //TODO SteamInput005
    return;
}

// Enable SteamInputActionEvent_t callbacks. Directly calls your callback function
// for lower latency than standard Steam callbacks. Supports one callback at a time.
// Note: this is called within either SteamInput()->RunFrame or by SteamAPI_RunCallbacks
void EnableActionEventCallbacks( SteamInputActionEventCallbackPointer pCallback )
{
    PRINT_DEBUG("TODO Steam_Controller::%s\n", __FUNCTION__);
    //TODO SteamInput005
    return;
}

// Synchronize API state with the latest Steam Controller inputs available. This
// is performed automatically by SteamAPI_RunCallbacks, but for the absolute lowest
// possible latency, you call this directly before reading controller state.
void RunFrame(bool bReservedValue)
{
    PRINT_DEBUG("Steam_Controller::RunFrame()\n");
    if (disabled || !initialized) {
        return;
    }

    GamepadUpdate();
}

void RunFrame()
{
    RunFrame(true);
}

bool GetControllerState( uint32 unControllerIndex, SteamControllerState001_t *pState )
{
    PRINT_DEBUG("Steam_Controller::GetControllerState()\n");
    return false;
}

// Enumerate currently connected controllers
// handlesOut should point to a STEAM_CONTROLLER_MAX_COUNT sized array of ControllerHandle_t handles
// Returns the number of handles written to handlesOut
int GetConnectedControllers( ControllerHandle_t *handlesOut )
{
    PRINT_DEBUG("Steam_Controller::GetConnectedControllers\n");
    if (!handlesOut) return 0;
    if (disabled) {
        return 0;
    }

    int count = 0;
    if (GamepadIsConnected(GAMEPAD_0)) {*handlesOut = GAMEPAD_0 + 1; ++handlesOut; ++count;};
    if (GamepadIsConnected(GAMEPAD_1)) {*handlesOut = GAMEPAD_1 + 1; ++handlesOut; ++count;};
    if (GamepadIsConnected(GAMEPAD_2)) {*handlesOut = GAMEPAD_2 + 1; ++handlesOut; ++count;};
    if (GamepadIsConnected(GAMEPAD_3)) {*handlesOut = GAMEPAD_3 + 1; ++handlesOut; ++count;};

    PRINT_DEBUG("returned %i connected controllers\n", count);
    return count;
}


// Invokes the Steam overlay and brings up the binding screen
// Returns false is overlay is disabled / unavailable, or the user is not in Big Picture mode
bool ShowBindingPanel( ControllerHandle_t controllerHandle )
{
    PRINT_DEBUG("Steam_Controller::ShowBindingPanel\n");
    return false;
}


// ACTION SETS
// Lookup the handle for an Action Set. Best to do this once on startup, and store the handles for all future API calls.
ControllerActionSetHandle_t GetActionSetHandle( const char *pszActionSetName )
{
    PRINT_DEBUG("Steam_Controller::GetActionSetHandle %s\n", pszActionSetName);
    if (!pszActionSetName) return 0;
    std::string upper_action_name(pszActionSetName);
    std::transform(upper_action_name.begin(), upper_action_name.end(), upper_action_name.begin(),[](unsigned char c){ return std::toupper(c); });

    auto set_handle = action_handles.find(upper_action_name);
    if (set_handle == action_handles.end()) return 0;

    PRINT_DEBUG("Steam_Controller::GetActionSetHandle %s ret %llu\n", pszActionSetName, set_handle->second);
    return set_handle->second;
}


// Reconfigure the controller to use the specified action set (ie 'Menu', 'Walk' or 'Drive')
// This is cheap, and can be safely called repeatedly. It's often easier to repeatedly call it in
// your state loops, instead of trying to place it in all of your state transitions.
void ActivateActionSet( ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetHandle )
{
    PRINT_DEBUG("Steam_Controller::ActivateActionSet %llu %llu\n", controllerHandle, actionSetHandle);
    if (controllerHandle == STEAM_CONTROLLER_HANDLE_ALL_CONTROLLERS) {
        for (auto & c: controllers) {
            c.second.activate_action_set(actionSetHandle, controller_maps);
        }
    }

    auto controller = controllers.find(controllerHandle);
    if (controller == controllers.end()) return;

    controller->second.activate_action_set(actionSetHandle, controller_maps);
}

ControllerActionSetHandle_t GetCurrentActionSet( ControllerHandle_t controllerHandle )
{
    //TODO: should return zero if no action set specifically activated with ActivateActionSet
    PRINT_DEBUG("Steam_Controller::GetCurrentActionSet %llu\n", controllerHandle);
    auto controller = controllers.find(controllerHandle);
    if (controller == controllers.end()) return 0;

    return controller->second.active_set;
}


void ActivateActionSetLayer( ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetLayerHandle )
{
    PRINT_DEBUG("Steam_Controller::ActivateActionSetLayer\n");
}

void DeactivateActionSetLayer( ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetLayerHandle )
{
    PRINT_DEBUG("Steam_Controller::DeactivateActionSetLayer\n");
}

void DeactivateAllActionSetLayers( ControllerHandle_t controllerHandle )
{
    PRINT_DEBUG("Steam_Controller::DeactivateAllActionSetLayers\n");
}

int GetActiveActionSetLayers( ControllerHandle_t controllerHandle, ControllerActionSetHandle_t *handlesOut )
{
    PRINT_DEBUG("Steam_Controller::GetActiveActionSetLayers\n");
    return 0;
}



// ACTIONS
// Lookup the handle for a digital action. Best to do this once on startup, and store the handles for all future API calls.
ControllerDigitalActionHandle_t GetDigitalActionHandle( const char *pszActionName )
{
    PRINT_DEBUG("Steam_Controller::GetDigitalActionHandle %s\n", pszActionName);
    if (!pszActionName) return 0;
    std::string upper_action_name(pszActionName);
    std::transform(upper_action_name.begin(), upper_action_name.end(), upper_action_name.begin(),[](unsigned char c){ return std::toupper(c); });

    auto handle = digital_action_handles.find(upper_action_name);
    if (handle == digital_action_handles.end()) {
        //apparently GetDigitalActionHandle also works with analog handles
        handle = analog_action_handles.find(upper_action_name);
        if (handle == analog_action_handles.end()) return 0;
    }

    PRINT_DEBUG("Steam_Controller::GetDigitalActionHandle %s ret %llu\n", pszActionName, handle->second);
    return handle->second;
}


// Returns the current state of the supplied digital game action
ControllerDigitalActionData_t GetDigitalActionData( ControllerHandle_t controllerHandle, ControllerDigitalActionHandle_t digitalActionHandle )
{
    PRINT_DEBUG("Steam_Controller::GetDigitalActionData %llu %llu\n", controllerHandle, digitalActionHandle);
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
int GetDigitalActionOrigins( ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetHandle, ControllerDigitalActionHandle_t digitalActionHandle, EControllerActionOrigin *originsOut )
{
    PRINT_DEBUG("Steam_Controller::GetDigitalActionOrigins\n");
    EInputActionOrigin origins[STEAM_CONTROLLER_MAX_ORIGINS];
    int ret = GetDigitalActionOrigins(controllerHandle, actionSetHandle, digitalActionHandle, origins );
    for (int i = 0; i < ret; ++i) {
        originsOut[i] = (EControllerActionOrigin)(origins[i] - (k_EInputActionOrigin_XBox360_A - k_EControllerActionOrigin_XBox360_A));
    }

    return ret;
}

int GetDigitalActionOrigins( InputHandle_t inputHandle, InputActionSetHandle_t actionSetHandle, InputDigitalActionHandle_t digitalActionHandle, EInputActionOrigin *originsOut )
{
    PRINT_DEBUG("Steam_Controller::GetDigitalActionOrigins steaminput\n");
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
const char *GetStringForDigitalActionName( InputDigitalActionHandle_t eActionHandle )
{
    PRINT_DEBUG("TODO Steam_Controller::%s\n", __FUNCTION__);
    //TODO SteamInput005
    return "Button String";
}

// Lookup the handle for an analog action. Best to do this once on startup, and store the handles for all future API calls.
ControllerAnalogActionHandle_t GetAnalogActionHandle( const char *pszActionName )
{
    PRINT_DEBUG("Steam_Controller::GetAnalogActionHandle %s\n", pszActionName);
    if (!pszActionName) return 0;
    std::string upper_action_name(pszActionName);
    std::transform(upper_action_name.begin(), upper_action_name.end(), upper_action_name.begin(),[](unsigned char c){ return std::toupper(c); });

    auto handle = analog_action_handles.find(upper_action_name);
    if (handle == analog_action_handles.end()) return 0;

    return handle->second;
}


// Returns the current state of these supplied analog game action
ControllerAnalogActionData_t GetAnalogActionData( ControllerHandle_t controllerHandle, ControllerAnalogActionHandle_t analogActionHandle )
{
    PRINT_DEBUG("Steam_Controller::GetAnalogActionData %llu %llu\n", controllerHandle, analogActionHandle);
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
                    data.x = mov_x;
                    data.y = mov_y;
                    double length = 1.0 / std::sqrt(data.x * data.x + data.y * data.y);
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
int GetAnalogActionOrigins( ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetHandle, ControllerAnalogActionHandle_t analogActionHandle, EControllerActionOrigin *originsOut )
{
    PRINT_DEBUG("Steam_Controller::GetAnalogActionOrigins\n");
    EInputActionOrigin origins[STEAM_CONTROLLER_MAX_ORIGINS];
    int ret = GetAnalogActionOrigins(controllerHandle, actionSetHandle, analogActionHandle, origins );
    for (int i = 0; i < ret; ++i) {
        originsOut[i] = (EControllerActionOrigin)(origins[i] - (k_EInputActionOrigin_XBox360_A - k_EControllerActionOrigin_XBox360_A));
    }

    return ret;
}

int GetAnalogActionOrigins( InputHandle_t inputHandle, InputActionSetHandle_t actionSetHandle, InputAnalogActionHandle_t analogActionHandle, EInputActionOrigin *originsOut )
{
    PRINT_DEBUG("Steam_Controller::GetAnalogActionOrigins steaminput\n");
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

    
void StopAnalogActionMomentum( ControllerHandle_t controllerHandle, ControllerAnalogActionHandle_t eAction )
{
    PRINT_DEBUG("Steam_Controller::StopAnalogActionMomentum %llu %llu\n", controllerHandle, eAction);
}


// Trigger a haptic pulse on a controller
void TriggerHapticPulse( ControllerHandle_t controllerHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec )
{
    PRINT_DEBUG("Steam_Controller::TriggerHapticPulse\n");
}

// Trigger a haptic pulse on a controller
void Legacy_TriggerHapticPulse( InputHandle_t inputHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
    TriggerHapticPulse(inputHandle, eTargetPad, usDurationMicroSec );
}

void TriggerHapticPulse( uint32 unControllerIndex, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec )
{
    PRINT_DEBUG("Steam_Controller::TriggerHapticPulse old\n");
    TriggerHapticPulse(unControllerIndex, eTargetPad, usDurationMicroSec );
}

// Trigger a pulse with a duty cycle of usDurationMicroSec / usOffMicroSec, unRepeat times.
// nFlags is currently unused and reserved for future use.
void TriggerRepeatedHapticPulse( ControllerHandle_t controllerHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec, unsigned short usOffMicroSec, unsigned short unRepeat, unsigned int nFlags )
{
    PRINT_DEBUG("Steam_Controller::TriggerRepeatedHapticPulse\n");
}

void Legacy_TriggerRepeatedHapticPulse( InputHandle_t inputHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec, unsigned short usOffMicroSec, unsigned short unRepeat, unsigned int nFlags )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
    TriggerRepeatedHapticPulse(inputHandle, eTargetPad, usDurationMicroSec, usOffMicroSec, unRepeat, nFlags);
}


// Send a haptic pulse, works on Steam Deck and Steam Controller devices
void TriggerSimpleHapticEvent( InputHandle_t inputHandle, EControllerHapticLocation eHapticLocation, uint8 nIntensity, char nGainDB, uint8 nOtherIntensity, char nOtherGainDB )
{
    PRINT_DEBUG("TODO Steam_Controller::%s\n", __FUNCTION__);
}

// Tigger a vibration event on supported controllers.  
void TriggerVibration( ControllerHandle_t controllerHandle, unsigned short usLeftSpeed, unsigned short usRightSpeed )
{
    PRINT_DEBUG("Steam_Controller::TriggerVibration %hu %hu\n", usLeftSpeed, usRightSpeed);
    auto controller = controllers.find(controllerHandle);
    if (controller == controllers.end()) return;

    unsigned int rumble_length_ms = 0;
#if defined(__linux__)
    //FIXME: shadow of the tomb raider on linux doesn't seem to turn off the rumble so I made it expire after 100ms. Need to check if this is how linux steam actually behaves.
    rumble_length_ms = 100;
#endif

    unsigned gamepad_device = (controllerHandle - 1);
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
void TriggerVibrationExtended( InputHandle_t inputHandle, unsigned short usLeftSpeed, unsigned short usRightSpeed, unsigned short usLeftTriggerSpeed, unsigned short usRightTriggerSpeed )
{
    PRINT_DEBUG("TODO Steam_Controller::%s\n", __FUNCTION__);
    TriggerVibration(inputHandle, usLeftSpeed, usRightSpeed);
    //TODO trigger impulse rumbles
}

// Set the controller LED color on supported controllers.  
void SetLEDColor( ControllerHandle_t controllerHandle, uint8 nColorR, uint8 nColorG, uint8 nColorB, unsigned int nFlags )
{
    PRINT_DEBUG("Steam_Controller::SetLEDColor\n");
}


// Returns the associated gamepad index for the specified controller, if emulating a gamepad
int GetGamepadIndexForController( ControllerHandle_t ulControllerHandle )
{
    PRINT_DEBUG("Steam_Controller::GetGamepadIndexForController\n");
    auto controller = controllers.find(ulControllerHandle);
    if (controller == controllers.end()) return -1;

    return ulControllerHandle - 1;
}


// Returns the associated controller handle for the specified emulated gamepad
ControllerHandle_t GetControllerForGamepadIndex( int nIndex )
{
    PRINT_DEBUG("Steam_Controller::GetControllerForGamepadIndex %i\n", nIndex);
    ControllerHandle_t out = nIndex + 1;
    auto controller = controllers.find(out);
    if (controller == controllers.end()) return 0;
    return out;
}


// Returns raw motion data from the specified controller
ControllerMotionData_t GetMotionData( ControllerHandle_t controllerHandle )
{
    PRINT_DEBUG("Steam_Controller::GetMotionData\n");
    ControllerMotionData_t data = {};
    return data;
}


// Attempt to display origins of given action in the controller HUD, for the currently active action set
// Returns false is overlay is disabled / unavailable, or the user is not in Big Picture mode
bool ShowDigitalActionOrigins( ControllerHandle_t controllerHandle, ControllerDigitalActionHandle_t digitalActionHandle, float flScale, float flXPosition, float flYPosition )
{
    PRINT_DEBUG("Steam_Controller::ShowDigitalActionOrigins\n");
    return true;
}

bool ShowAnalogActionOrigins( ControllerHandle_t controllerHandle, ControllerAnalogActionHandle_t analogActionHandle, float flScale, float flXPosition, float flYPosition )
{
    PRINT_DEBUG("Steam_Controller::ShowAnalogActionOrigins\n");
    return true;
}


// Returns a localized string (from Steam's language setting) for the specified origin
const char *GetStringForActionOrigin( EControllerActionOrigin eOrigin )
{
    PRINT_DEBUG("Steam_Controller::GetStringForActionOrigin\n");
    return "Button String";
}

const char *GetStringForActionOrigin( EInputActionOrigin eOrigin )
{
    PRINT_DEBUG("Steam_Controller::GetStringForActionOrigin steaminput\n");
    return "Button String";
}

// Returns a localized string (from Steam's language setting) for the user-facing action name corresponding to the specified handle
const char *GetStringForAnalogActionName( InputAnalogActionHandle_t eActionHandle )
{
    PRINT_DEBUG("TODO Steam_Controller::%s\n", __FUNCTION__);
    //TODO SteamInput005
    return "Button String";
}

// Get a local path to art for on-screen glyph for a particular origin 
const char *GetGlyphForActionOrigin( EControllerActionOrigin eOrigin )
{
    PRINT_DEBUG("Steam_Controller::GetGlyphForActionOrigin %i\n", eOrigin);

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

const char *GetGlyphForActionOrigin( EInputActionOrigin eOrigin )
{
    PRINT_DEBUG("Steam_Controller::GetGlyphForActionOrigin steaminput %i\n", eOrigin);
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
const char *GetGlyphPNGForActionOrigin( EInputActionOrigin eOrigin, ESteamInputGlyphSize eSize, uint32 unFlags )
{
    PRINT_DEBUG("TODO Steam_Controller::%s\n", __FUNCTION__);
    //TODO SteamInput005
    return GetGlyphForActionOrigin(eOrigin);
}

// Get a local path to a SVG file for the provided origin's glyph. 
const char *GetGlyphSVGForActionOrigin( EInputActionOrigin eOrigin, uint32 unFlags )
{
    PRINT_DEBUG("TODO Steam_Controller::%s\n", __FUNCTION__);
    //TODO SteamInput005
    return "";
}

// Get a local path to an older, Big Picture Mode-style PNG file for a particular origin
const char *GetGlyphForActionOrigin_Legacy( EInputActionOrigin eOrigin )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
    return GetGlyphForActionOrigin(eOrigin);
}

// Returns the input type for a particular handle
ESteamInputType GetInputTypeForHandle( ControllerHandle_t controllerHandle )
{
    PRINT_DEBUG("Steam_Controller::GetInputTypeForHandle %llu\n", controllerHandle);
    auto controller = controllers.find(controllerHandle);
    if (controller == controllers.end()) return k_ESteamInputType_Unknown;
    return k_ESteamInputType_XBox360Controller;
}

const char *GetStringForXboxOrigin( EXboxOrigin eOrigin )
{
    PRINT_DEBUG("Steam_Controller::GetStringForXboxOrigin\n");
    return "";
}

const char *GetGlyphForXboxOrigin( EXboxOrigin eOrigin )
{
    PRINT_DEBUG("Steam_Controller::GetGlyphForXboxOrigin\n");
    return "";
}

EControllerActionOrigin GetActionOriginFromXboxOrigin_( ControllerHandle_t controllerHandle, EXboxOrigin eOrigin )
{
    PRINT_DEBUG("Steam_Controller::GetActionOriginFromXboxOrigin\n");
    return k_EControllerActionOrigin_None;
}

EInputActionOrigin GetActionOriginFromXboxOrigin( InputHandle_t inputHandle, EXboxOrigin eOrigin )
{
    PRINT_DEBUG("Steam_Controller::GetActionOriginFromXboxOrigin steaminput\n");
    return k_EInputActionOrigin_None;
}

EControllerActionOrigin TranslateActionOrigin( ESteamInputType eDestinationInputType, EControllerActionOrigin eSourceOrigin )
{
    PRINT_DEBUG("Steam_Controller::TranslateActionOrigin\n");
    return k_EControllerActionOrigin_None;
}

EInputActionOrigin TranslateActionOrigin( ESteamInputType eDestinationInputType, EInputActionOrigin eSourceOrigin )
{
    PRINT_DEBUG("Steam_Controller::TranslateActionOrigin steaminput destinationinputtype %d sourceorigin %d\n", eDestinationInputType, eSourceOrigin );
 
    if (eDestinationInputType == k_ESteamInputType_XBox360Controller)
        return eSourceOrigin;
 
    return k_EInputActionOrigin_None;
}

bool GetControllerBindingRevision( ControllerHandle_t controllerHandle, int *pMajor, int *pMinor )
{
    PRINT_DEBUG("Steam_Controller::GetControllerBindingRevision\n");
    return false;
}

bool GetDeviceBindingRevision( InputHandle_t inputHandle, int *pMajor, int *pMinor )
{
    PRINT_DEBUG("Steam_Controller::GetDeviceBindingRevision\n");
    return false;
}

uint32 GetRemotePlaySessionID( InputHandle_t inputHandle )
{
    PRINT_DEBUG("Steam_Controller::GetRemotePlaySessionID\n");
    return 0;
}

// Get a bitmask of the Steam Input Configuration types opted in for the current session. Returns ESteamInputConfigurationEnableType values.?	
// Note: user can override the settings from the Steamworks Partner site so the returned values may not exactly match your default configuration
uint16 GetSessionInputConfigurationSettings()
{
    PRINT_DEBUG("TODO Steam_Controller::%s\n", __FUNCTION__);
    return 0;
}

// Set the trigger effect for a DualSense controller
void SetDualSenseTriggerEffect( InputHandle_t inputHandle, const ScePadTriggerEffectParam *pParam )
{
    PRINT_DEBUG("TODO Steam_Controller::%s\n", __FUNCTION__);
}

void RunCallbacks()
{
    if (explicitly_call_run_frame) {
        RunFrame();
    }
}

};
