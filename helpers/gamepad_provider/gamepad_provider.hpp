#pragma once

#include <map>
#include <vector>

#include <steam/steamtypes.h>
#include <steam/isteamcontroller.h>

#if !defined(CONTROLLER_SUPPORT)

#define JOYSTICK_MAX 32767

enum GAMEPAD_BUTTON {
  BUTTON_DPAD_UP, /* UP on the direction pad */
  BUTTON_DPAD_DOWN,	/**< DOWN on the direction pad */
  BUTTON_DPAD_LEFT,	/**< LEFT on the direction pad */
  BUTTON_DPAD_RIGHT,	/**< RIGHT on the direction pad */
  BUTTON_START,	/**< START button */
  BUTTON_BACK,	/**< BACK button */
  BUTTON_LEFT_THUMB,	/**< Left analog stick button */
  BUTTON_RIGHT_THUMB,	/**< Right analog stick button */
  BUTTON_LEFT_SHOULDER,	/**< Left bumper button */
  BUTTON_RIGHT_SHOULDER,	/**< Right bumper button */
  BUTTON_A,	/**< A button */
  BUTTON_B,	/**< B button */
  BUTTON_X,	/**< X button */
  BUTTON_Y,	/**< Y button */
  BUTTON_COUNT = 16
};
#else

#include <SDL3/SDL.h>
#include <SDL3/SDL_joystick.h>

#define JOYSTICK_MAX SDL_JOYSTICK_AXIS_MAX

enum GAMEPAD_BUTTON {
  BUTTON_DPAD_UP = SDL_GAMEPAD_BUTTON_DPAD_UP,	/**< UP on the direction pad */
  BUTTON_DPAD_DOWN = SDL_GAMEPAD_BUTTON_DPAD_DOWN,	/**< DOWN on the direction pad */
  BUTTON_DPAD_LEFT = SDL_GAMEPAD_BUTTON_DPAD_LEFT,	/**< LEFT on the direction pad */
  BUTTON_DPAD_RIGHT = SDL_GAMEPAD_BUTTON_DPAD_RIGHT,	/**< RIGHT on the direction pad */
  BUTTON_START = SDL_GAMEPAD_BUTTON_START,	/**< START button */
  BUTTON_BACK = SDL_GAMEPAD_BUTTON_BACK,	/**< BACK button */
  BUTTON_LEFT_THUMB = SDL_GAMEPAD_BUTTON_LEFT_STICK,	/**< Left analog stick button */
  BUTTON_RIGHT_THUMB = SDL_GAMEPAD_BUTTON_RIGHT_STICK,	/**< Right analog stick button */
  BUTTON_LEFT_SHOULDER = SDL_GAMEPAD_BUTTON_LEFT_SHOULDER,	/**< Left bumper button */
  BUTTON_RIGHT_SHOULDER = SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER,	/**< Right bumper button */
  BUTTON_A = SDL_GAMEPAD_BUTTON_SOUTH,	/**< A button */
  BUTTON_B = SDL_GAMEPAD_BUTTON_EAST,	/**< B button */
  BUTTON_X = SDL_GAMEPAD_BUTTON_WEST,	/**< X button */
  BUTTON_Y = SDL_GAMEPAD_BUTTON_NORTH,	/**< Y button */
  BUTTON_COUNT = 16
};

#endif

/**
 * Enumeration of the possible pressure/trigger buttons.
 */
enum GAMEPAD_TRIGGER {
  TRIGGER_LEFT = 0,	/**< Left trigger */
  TRIGGER_RIGHT = 1,	/**< Right trigger */
  TRIGGER_COUNT			/**< Number of triggers */
};

/**
 * Enumeration of the analog sticks.
 */
enum GAMEPAD_STICK {
  STICK_LEFT = 0,	/**< Left stick */
  STICK_RIGHT = 1,	/**< Right stick */
  STICK_COUNT		/**< Number of analog sticks */
};

namespace gamepad_provider {
  namespace sdl
  {
      int DetectGamepads(std::vector<ControllerHandle_t> &handlesOut, std::vector<ControllerHandle_t>& newHandlesOut);
      bool GamepadInit(bool combine_joycons);
      void GamepadShutdown(void);
      void GamepadUpdate(void);
      bool GamepadButtonDown(ControllerHandle_t id, GAMEPAD_BUTTON button);
      float GamepadTriggerLength(ControllerHandle_t id, GAMEPAD_TRIGGER trigger);
      void GamepadStickXY(ControllerHandle_t id, GAMEPAD_STICK stick, float* out_x, float* out_y);
      void GamepadStickNormXY(ControllerHandle_t id, GAMEPAD_STICK stick, float* out_x, float* out_y, int inner_deadzone, int outer_deadzone);
      void GamepadSetRumble(ControllerHandle_t id, unsigned short left, unsigned short right, unsigned int rumble_length_ms);
      void GamepadSetTriggersRumble(ControllerHandle_t id, unsigned short left, unsigned short right, unsigned int rumble_length_ms);
      ESteamInputType GamepadGetType(ControllerHandle_t id);
  }
}
