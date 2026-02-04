#include "service.h"
#include "backend_sdl3.h"

namespace gbe::input {

InputService& InputService::GetInstance() {
    static InputService instance;
    return instance;
}

InputService::InputService() 
    : m_backend(std::make_unique<SDL3Backend>()),
      m_action_engine(std::make_unique<ActionEngine>()) {
}

InputService::~InputService() {
    Shutdown();
}

bool InputService::Initialize() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_initialized) return true;

    if (m_backend && m_backend->Initialize()) {
        m_initialized = true;
        return true;
    }
    return false;
}

void InputService::Shutdown() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_initialized) return;

    m_backend->Shutdown();
    m_backend.reset();
    m_initialized = false;
}

void InputService::Update() {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (!m_initialized) return;

    m_backend->Update();
}

} // namespace gbe::input
