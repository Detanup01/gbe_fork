#include "gamepad_provider/gamepad_provider.hpp"
#include <array>
#include <algorithm>
#include <string>
#define GAMEPAD_COUNT 16

#if !defined(CONTROLLER_SUPPORT)
int gamepad_provider::sdl::DetectGamepads(std::vector<ControllerHandle_t>& handlesOut, std::vector<ControllerHandle_t>& newHandlesOut) { return 0; }
bool gamepad_provider::sdl::GamepadInit(bool combine_joycons) { return false; };
void gamepad_provider::sdl::GamepadShutdown(void) {};
void gamepad_provider::sdl::GamepadUpdate(void) {};
bool gamepad_provider::sdl::GamepadButtonDown(ControllerHandle_t id, GAMEPAD_BUTTON button) { return false; };
float gamepad_provider::sdl::GamepadTriggerLength(ControllerHandle_t id, GAMEPAD_TRIGGER trigger) { return 0.0f; };
void gamepad_provider::sdl::GamepadStickXY(ControllerHandle_t id, GAMEPAD_STICK stick, float* out_x, float* out_y) {};
void gamepad_provider::sdl::GamepadStickNormXY(ControllerHandle_t id, GAMEPAD_STICK stick, float* out_x, float* out_y, int inner_deadzone, int outer_deadzone) {};
void gamepad_provider::sdl::GamepadSetRumble(ControllerHandle_t id, unsigned short left, unsigned short right, unsigned int rumble_length_ms) {};
void gamepad_provider::sdl::GamepadSetTriggersRumble(ControllerHandle_t id, unsigned short left, unsigned short right, unsigned int rumble_length_ms) {};
ESteamInputType gamepad_provider::sdl::GamepadGetType(ControllerHandle_t id) { return k_ESteamInputType_Unknown; };

#else

namespace gamepad_provider {
    namespace sdl {
        namespace {
            static std::map<ControllerHandle_t, SDL_JoystickID> sdl_controller_map{};
            static std::map<std::string, std::vector<ControllerHandle_t>> guid_controller_map{};
            static ControllerHandle_t current_controller_index = 1; // Must start from 1

            inline SDL_Gamepad* GamepadGetFromHandle(ControllerHandle_t controller) {
                auto sdl_controller = sdl_controller_map.find(controller);
                if (sdl_controller == sdl_controller_map.end()) return nullptr;
                return SDL_GetGamepadFromID(sdl_controller->second);
            }
        }
    }
}

bool gamepad_provider::sdl::GamepadInit(bool combine_joycons) {
    SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");

    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS4, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_PS5, "1");

    SDL_SetHint(SDL_HINT_JOYSTICK_ENHANCED_REPORTS, "1");

    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_GAMECUBE, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_SWITCH, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_JOY_CONS, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_COMBINE_JOY_CONS, combine_joycons ? "1" : "0");  // Joycons are combined by default
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_STADIA, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_STEAM, "1");
    SDL_SetHint(SDL_HINT_JOYSTICK_HIDAPI_LUNA, "1");

    if (!SDL_Init(SDL_INIT_JOYSTICK | SDL_INIT_GAMEPAD | SDL_INIT_HAPTIC | SDL_INIT_EVENTS))
        return false;

    SDL_SetGamepadEventsEnabled(false);

    return true;
}

void gamepad_provider::sdl::GamepadShutdown(void) {
    for (auto& c : sdl_controller_map) {
        SDL_Gamepad* gamepad = SDL_GetGamepadFromID(c.second);
        if (gamepad)
            SDL_CloseGamepad(gamepad);
    }

    SDL_QuitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_GAMEPAD | SDL_INIT_HAPTIC | SDL_INIT_EVENTS);
}

void gamepad_provider::sdl::GamepadUpdate(void) {
    SDL_UpdateGamepads();
}

/* Detects gamepad connect/disconnects. */
int gamepad_provider::sdl::DetectGamepads(std::vector<ControllerHandle_t> &handlesOut, std::vector<ControllerHandle_t> &newHandlesOut) {
    SDL_UpdateGamepads();
    int count = 0;
    SDL_JoystickID* gamepads = SDL_GetGamepads(&count);

    for (int i = 0; i < count && i < GAMEPAD_COUNT; ++i) {
        bool newHandleCreated = false;
        SDL_JoystickID instance_id = gamepads[i];
        SDL_GUID guid = SDL_GetGamepadGUIDForID(instance_id);
        char guid_string[33];
        SDL_GUIDToString(guid, guid_string, 33);
        auto guid_handles = guid_controller_map.find(guid_string);

        // Always open gamepad, since a disconnected controller could be in the map
        SDL_Gamepad* gamepadHandle = SDL_OpenGamepad(instance_id);
        ControllerHandle_t curr_index = 0;

        if (guid_handles == guid_controller_map.end()) {
            newHandlesOut.push_back(current_controller_index);
            char guid_string[33];
            SDL_GUIDToString(guid, guid_string, 33);
            sdl_controller_map.insert(std::pair<ControllerHandle_t, SDL_JoystickID>(current_controller_index, instance_id));
            guid_controller_map.insert(std::pair<std::string, std::vector<ControllerHandle_t>>(guid_string, std::vector<ControllerHandle_t>{current_controller_index}));
            curr_index = current_controller_index;
            newHandleCreated = true;
            ++current_controller_index;
        }
        else {
            bool replaced = false;
            // GUID already exists
            for (auto& c : guid_handles->second) {
                SDL_Gamepad* gamepadHandle = GamepadGetFromHandle(c);
                if (!gamepadHandle || !SDL_GamepadConnected(gamepadHandle)) {
                    // Gamepad is not connected, replace this index
                    if (gamepadHandle)
                        SDL_CloseGamepad(gamepadHandle);
                    gamepadHandle = SDL_OpenGamepad(instance_id);
                    sdl_controller_map[c] = instance_id;
                    replaced = true;
                    curr_index = c;
                    break;
                }
                if (SDL_GetGamepadID(gamepadHandle) == instance_id) {
                    // Gamepad is connected and already in controller_map
                    replaced = true;
                    curr_index = c;
                    break;
                }
            }

            if (!replaced) {
                sdl_controller_map.insert(std::pair<ControllerHandle_t, SDL_JoystickID>(current_controller_index, instance_id));
                guid_handles->second.push_back(current_controller_index);
                curr_index = current_controller_index;
                newHandleCreated = true;
                ++current_controller_index;
            }
        }

        handlesOut.push_back(curr_index);

        if (newHandleCreated) {
            newHandlesOut.push_back(curr_index);
        }
    }

    SDL_free(gamepads);

    return count;
}

bool gamepad_provider::sdl::GamepadButtonDown(ControllerHandle_t id, GAMEPAD_BUTTON button) {
    SDL_Gamepad* gamepad = GamepadGetFromHandle(id);
    if (!gamepad)
        return false;
    return SDL_GetGamepadButton(gamepad, (SDL_GamepadButton)button);
}

float gamepad_provider::sdl::GamepadTriggerLength(ControllerHandle_t id, GAMEPAD_TRIGGER trigger) {
    SDL_Gamepad* gamepad = GamepadGetFromHandle(id);
    if (!gamepad)
        return 0.0f;
    if (trigger == TRIGGER_LEFT)
        return static_cast<float>(SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER));
    else if (trigger == TRIGGER_RIGHT)
        return static_cast<float>(SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER));
    return 0.0f;
}

void gamepad_provider::sdl::GamepadStickNormXY(ControllerHandle_t id, GAMEPAD_STICK stick, float* out_x, float* out_y, int inner_deadzone, int outer_deadzone) {
    float x = 0.0f, y = 0.0f;
    GamepadStickXY(id, stick, &x, &y);

    // normalize X and Y
    float length = static_cast<float>(sqrt(x * x + y * y));
    if (length > inner_deadzone) {
        // clamp length to maximum value
        length = std::min(length, 32767.f);

        // normalized X and Y values
        x /= (JOYSTICK_MAX - outer_deadzone);
        y /= (JOYSTICK_MAX - outer_deadzone);

        //fix special case
        x = std::clamp(x, -1.0f, 1.0f);
        y = std::clamp(y, -1.0f, 1.0f);
    }
    else {
        x = 0.0f;
        y = 0.0f;
    }

    *out_x = x;
    *out_y = y;
}

void gamepad_provider::sdl::GamepadStickXY(ControllerHandle_t id, GAMEPAD_STICK stick, float* out_x, float* out_y) {
    SDL_Gamepad* gamepad = GamepadGetFromHandle(id);
    if (!gamepad) {
        *out_x = 0.0f;
        *out_y = 0.0f;
        return;
    }

    if (stick == STICK_LEFT) {
        *out_x = static_cast<float>(SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTX));
        *out_y = static_cast<float>(-SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTY));
    }
    else if (stick == STICK_RIGHT) {
        *out_x = static_cast<float>(SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTX));
        *out_y = static_cast<float>(-SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTY));
    }
}

void gamepad_provider::sdl::GamepadSetRumble(ControllerHandle_t id, unsigned short left, unsigned short right, unsigned int rumble_length_ms) {
    SDL_Gamepad* gamepad = GamepadGetFromHandle(id);
    if (!gamepad)
        return;
    SDL_RumbleGamepad(gamepad, left, right, rumble_length_ms);
}

void gamepad_provider::sdl::GamepadSetTriggersRumble(ControllerHandle_t id, unsigned short left, unsigned short right, unsigned int rumble_length_ms) {
    SDL_Gamepad* gamepad = GamepadGetFromHandle(id);
    if (!gamepad)
        return;
    SDL_RumbleGamepadTriggers(gamepad, left, right, rumble_length_ms);
}

ESteamInputType gamepad_provider::sdl::GamepadGetType(ControllerHandle_t id) {
    SDL_Gamepad* gamepad = GamepadGetFromHandle(id);
    if (!gamepad)
        return k_ESteamInputType_Unknown;
    switch (SDL_GetGamepadType(gamepad)) {
    case SDL_GAMEPAD_TYPE_XBOX360:
        return k_ESteamInputType_XBox360Controller;
    case SDL_GAMEPAD_TYPE_XBOXONE:
        return k_ESteamInputType_XBoxOneController;
    case SDL_GAMEPAD_TYPE_PS3:
        return k_ESteamInputType_PS3Controller;
    case SDL_GAMEPAD_TYPE_PS4:
        return k_ESteamInputType_PS4Controller;
    case SDL_GAMEPAD_TYPE_PS5:
        return k_ESteamInputType_PS5Controller;
    case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_LEFT:
    case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_RIGHT:
        return k_ESteamInputType_SwitchJoyConSingle;
    case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_JOYCON_PAIR:
        return k_ESteamInputType_SwitchJoyConPair;
    case SDL_GAMEPAD_TYPE_NINTENDO_SWITCH_PRO:
        return k_ESteamInputType_SwitchProController;
    case SDL_GAMEPAD_TYPE_STANDARD:
        return k_ESteamInputType_GenericGamepad;
    default:
        return k_ESteamInputType_Unknown;
    }
}

#endif
