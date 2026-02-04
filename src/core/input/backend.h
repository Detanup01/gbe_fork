#pragma once

#include "common_includes.h"
#include <vector>
#include <string>
#include <memory>

namespace gbe::input {

/**
 * Handle for a physical controller.
 * Typically 1-indexed to match Steam's behavior.
 */
using ControllerHandle = uint64_t;

/**
 * Raw state of a controller axis or button.
 */
struct ControllerState {
    bool connected = false;
    uint32_t buttons = 0;   // Bitmask of GAMEPAD_BUTTON
    float axes[6] = {0.0f}; // LX, LY, RX, RY, L2, R2
};

/**
 * Interface for hardware input backends (e.g., SDL3, XInput).
 */
class IInputBackend {
public:
    virtual ~IInputBackend() = default;

    /**
     * Initialize the backend.
     */
    virtual bool Initialize() = 0;

    /**
     * Shutdown the backend.
     */
    virtual void Shutdown() = 0;

    /**
     * Poll for hardware events and update internal states.
     * Should be called once per frame.
     */
    virtual void Update() = 0;

    /**
     * Get the current state of a specific controller.
     */
    virtual ControllerState GetControllerState(ControllerHandle handle) const = 0;

    /**
     * Enumerate all currently connected controller handles.
     */
    virtual std::vector<ControllerHandle> GetConnectedControllers() const = 0;

    /**
     * Trigger a rumble effect on a controller.
     */
    virtual void SetRumble(ControllerHandle handle, uint16_t left, uint16_t right, uint32_t duration_ms) = 0;

    /**
     * Get the descriptive name/type of the controller.
     */
    virtual std::string GetControllerName(ControllerHandle handle) const = 0;

    /**
     * Set the LED color on a controller.
     */
    virtual void SetLEDColor(ControllerHandle handle, uint8_t r, uint8_t g, uint8_t b) = 0;
};

} // namespace gbe::input
