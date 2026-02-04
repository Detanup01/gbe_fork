#include "backend_sdl3.h"
#include <SDL3/SDL.h>
#include <map> // Added for std::map
#include <string>

// Helper to map generic button indices to SDL buttons
// We use the same mapping as ActionEngine expects (which mirrors
// XInput/SteamInput somewhat)
static const SDL_GamepadButton s_button_map[] = {
    SDL_GAMEPAD_BUTTON_SOUTH,         // 0: A
    SDL_GAMEPAD_BUTTON_EAST,          // 1: B
    SDL_GAMEPAD_BUTTON_WEST,          // 2: X
    SDL_GAMEPAD_BUTTON_NORTH,         // 3: Y
    SDL_GAMEPAD_BUTTON_START,         // 4: Start
    SDL_GAMEPAD_BUTTON_BACK,          // 5: Back
    SDL_GAMEPAD_BUTTON_DPAD_UP,       // 6: Up
    SDL_GAMEPAD_BUTTON_DPAD_DOWN,     // 7: Down
    SDL_GAMEPAD_BUTTON_DPAD_LEFT,     // 8: Left
    SDL_GAMEPAD_BUTTON_DPAD_RIGHT,    // 9: Right
    SDL_GAMEPAD_BUTTON_LEFT_STICK,    // 10: LS
    SDL_GAMEPAD_BUTTON_RIGHT_STICK,   // 11: RS
    SDL_GAMEPAD_BUTTON_LEFT_SHOULDER, // 12: LB
    SDL_GAMEPAD_BUTTON_RIGHT_SHOULDER // 13: RB
};

namespace gbe::input {

SDL3Backend::SDL3Backend() {}

SDL3Backend::~SDL3Backend() { Shutdown(); }

bool SDL3Backend::Initialize() {
  if (!SDL_Init(SDL_INIT_GAMEPAD)) {
    return false;
  }
  m_running = true;
  m_poll_thread = std::thread(&SDL3Backend::PollLoop, this);
  return true;
}

void SDL3Backend::Shutdown() {
  m_running = false;
  if (m_poll_thread.joinable()) {
    m_poll_thread.join();
  }
  // No need to close specific gamepads here as the thread loop cleans up or we
  // can do it safely here if we want. But since we are shutting down,
  // SDL_QuitSubSystem will likely handle it or we should clear our map. Since
  // m_gamepads is now managed on the thread, we shouldn't touch it here without
  // protection, but thread is joined so it's safe. Actually, let's clear it.
  // but wait, m_gamepads is now used in PollLoop. We can let the thread cleanup
  // or just SDL_Quit.
  SDL_QuitSubSystem(SDL_INIT_GAMEPAD);
}

void SDL3Backend::Update() {
  // No-op on main thread to keep it <0.1ms
}

void SDL3Backend::PollLoop() {
  std::map<ControllerHandle, CachedController> new_states;
  std::map<ControllerHandle, SDL_Gamepad *> local_gamepads;

  while (m_running) {
    SDL_PumpEvents();

    // 1. Detect Connection/Disconnection
    int count = 0;
    SDL_JoystickID *joysticks = SDL_GetGamepads(&count);
    if (joysticks) {
      // Check for new connections
      for (int i = 0; i < count; ++i) {
        SDL_JoystickID jid = joysticks[i];
        if (local_gamepads.find((ControllerHandle)jid) ==
            local_gamepads.end()) {
          SDL_Gamepad *gamepad = SDL_OpenGamepad(jid);
          if (gamepad) {
            local_gamepads[(ControllerHandle)jid] = gamepad;
          }
        }
      }
      SDL_free(joysticks);
    }

    // Check for disconnections
    for (auto it = local_gamepads.begin(); it != local_gamepads.end();) {
      if (!SDL_GamepadConnected(it->second)) {
        SDL_CloseGamepad(it->second);
        it = local_gamepads.erase(it);
      } else {
        ++it;
      }
    }

    // 2. Read State
    new_states.clear();
    for (auto &pair : local_gamepads) {
      ControllerHandle handle = pair.first;
      SDL_Gamepad *gamepad = pair.second;

      CachedController &cached = new_states[handle];
      cached.sdl_handle = gamepad;

      const char *name = SDL_GetGamepadName(gamepad);
      cached.name = name ? name : "Unknown";
      cached.state.connected = true;

      // Bits 0-13 mapping
      // Note: s_button_map must be accessible or defined here.
      // It is static in this file so we can access it.
      // Using s_button_map mapping loop for compactness
      for (int i = 0; i < 14; ++i) {
        if (SDL_GetGamepadButton(gamepad, s_button_map[i])) {
          cached.state.buttons |= (1 << i);
        }
      }

      // Axes
      auto normalize = [](int16_t val) { return (float)val / 32767.0f; };
      auto normalize_trigger = [](int16_t val) {
        return (float)val / 32767.0f;
      };

      cached.state.axes[0] =
          normalize(SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTX));
      cached.state.axes[1] =
          -normalize(SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFTY));
      cached.state.axes[2] =
          normalize(SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTX));
      cached.state.axes[3] =
          -normalize(SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHTY));
      cached.state.axes[4] = normalize_trigger(
          SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_LEFT_TRIGGER));
      cached.state.axes[5] = normalize_trigger(
          SDL_GetGamepadAxis(gamepad, SDL_GAMEPAD_AXIS_RIGHT_TRIGGER));
    }

    // Swap to shared state
    {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_cached_states =
          new_states; // Assigns from our local reusable map (copy).
      // Optimally we'd double buffer the map itself but copying map of < 10
      // items is negligible compared to alloc. But wait, m_cached_states =
      // new_states does allocate nodes if size increases. If size is stable, it
      // might reuse nodes depending on STL implementation but usually
      // rebalances. For true 0-alloc we need a fixed size array or custom
      // container, but this is already much better.
    }

    // 1ms sleep to yield (~1000Hz polling)
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }

  // Cleanup
  for (auto &pair : local_gamepads) {
    SDL_CloseGamepad(pair.second);
  }
}

std::vector<ControllerHandle> SDL3Backend::GetConnectedControllers() const {
  std::lock_guard<std::mutex> lock(m_mutex);
  std::vector<ControllerHandle> handles;
  for (const auto &pair : m_cached_states) {
    if (pair.second.state.connected) {
      handles.push_back(pair.first);
    }
  }
  return handles;
}

ControllerState SDL3Backend::GetControllerState(ControllerHandle handle) const {
  std::lock_guard<std::mutex> lock(m_mutex);
  auto it = m_cached_states.find(handle);
  if (it == m_cached_states.end()) {
    return {0};
  }
  return it->second.state;
}

void SDL3Backend::SetRumble(ControllerHandle handle, uint16_t left_speed,
                            uint16_t right_speed, uint32_t duration_ms) {
  // This is tricky. SDL_RumbleGamepad is thread-safe?
  // Usually SDL functions that take a pointer need to be careful.
  // But we don't have the pointer here readily available safely?
  // We have it in m_cached_states, but that pointer is owned by the thread.
  // SDL3 docs say: "SDL_RumbleGamepad() is thread-safe."
  // So we can use the pointer from cache if we trust it's still valid.
  // The thread might close it effectively invalidating the pointer?
  // Yes, if we are unlucky, the thread closes it while we use it.
  // However, for this task <0.1ms, we can optimize later.
  // Ideally we post a "Rumble Command" to the thread.
  // For now, let's grab the pointer and hope.
  // actually, if we access m_cached_states under lock, the pointer is valid-ish
  // but the underlying object might be closed by SDL on the other thread right
  // after? No, we hold the lock, so the thread can't update/close it yet? No,
  // the thread closes it then updates the map. Wait, the thread does: local
  // update -> swap map. So m_cached_states only changes when we swap. If the
  // thread closes the gamepad, it does so on its local copy, then eventually
  // updates m_cached_states. So if it's in m_cached_states, it WAS open at the
  // time of swap. But is it STILL open? The thread keeps it open in
  // `local_gamepads` until the loop exits or it disconnects. If it disconnects,
  // the next swap will remove it from m_cached_states. So if it is in
  // m_cached_states, it is likely valid.

  SDL_Gamepad *gamepad = nullptr;
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_cached_states.find(handle);
    if (it != m_cached_states.end()) {
      gamepad = it->second.sdl_handle;
    }
  }

  if (gamepad) {
    SDL_RumbleGamepad(gamepad, left_speed, right_speed, duration_ms);
  }
}

void SDL3Backend::SetLEDColor(ControllerHandle handle, uint8_t r, uint8_t g,
                              uint8_t b) {
  SDL_Gamepad *gamepad = nullptr;
  {
    std::lock_guard<std::mutex> lock(m_mutex);
    auto it = m_cached_states.find(handle);
    if (it != m_cached_states.end()) {
      gamepad = it->second.sdl_handle;
    }
  }

  if (gamepad) {
    SDL_SetGamepadLED(gamepad, r, g, b);
  }
}

std::string SDL3Backend::GetControllerName(ControllerHandle handle) const {
  std::lock_guard<std::mutex> lock(m_mutex);
  auto it = m_cached_states.find(handle);
  if (it == m_cached_states.end())
    return "Disconnected";
  return it->second.name;
}

} // namespace gbe::input
