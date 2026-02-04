#pragma once

#include "backend.h"
#include <SDL3/SDL.h>
#include <atomic>
#include <map>
#include <mutex>
#include <thread>
#include <vector>

namespace gbe {
namespace input {

class SDL3Backend : public IInputBackend {
public:
  SDL3Backend();
  virtual ~SDL3Backend();

  bool Initialize() override;
  void Shutdown() override;
  void Update() override;

  std::vector<ControllerHandle> GetConnectedControllers() const override;
  ControllerState GetControllerState(ControllerHandle handle) const override;
  void SetRumble(ControllerHandle handle, uint16_t left_speed,
                 uint16_t right_speed, uint32_t duration_ms) override;
  void SetLEDColor(ControllerHandle handle, uint8_t r, uint8_t g,
                   uint8_t b) override;
  std::string GetControllerName(ControllerHandle handle) const override;

private:
  std::map<ControllerHandle, SDL_Gamepad *> m_gamepads;

  // Threading support
  std::thread m_poll_thread;
  std::atomic<bool> m_running{false};
  mutable std::mutex m_mutex;

  // Double buffered state cache
  struct CachedController {
    ControllerState state;
    std::string name;
    SDL_Gamepad *sdl_handle = nullptr; // Only safe to use on thread
  };
  std::map<ControllerHandle, CachedController> m_cached_states;

  void PollLoop();
};

} // namespace input
} // namespace gbe
