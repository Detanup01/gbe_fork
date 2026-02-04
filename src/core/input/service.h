#pragma once

#include "backend.h"
#include "action_engine.h"
#include <memory>
#include <vector>
#include <mutex>

namespace gbe::input {

/**
 * Singleton service that manages the Steam Input emulation.
 * This is the high-level orchestration layer.
 */
class InputService {
public:
    static InputService& GetInstance();

    bool Initialize();
    void Shutdown();

    /**
     * Should be called every frame to update backend states and process events.
     */
    void Update();

    IInputBackend* GetBackend() const { return m_backend.get(); }
    ActionEngine& GetActionEngine() { return *m_action_engine; }

private:
    InputService();
    ~InputService();

    InputService(const InputService&) = delete;
    InputService& operator=(const InputService&) = delete;

    std::unique_ptr<IInputBackend> m_backend;
    std::unique_ptr<ActionEngine> m_action_engine;
    bool m_initialized = false;
    mutable std::mutex m_mutex;
};

} // namespace gbe::input
