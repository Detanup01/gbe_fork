// dear imgui: Platform Binding for Linux (standard X11 API for 32 and 64 bits applications)
// This needs to be used along with a Renderer (e.g. OpenGL3, Vulkan..)

// Implemented features:
//  [X] Platform: Clipboard support
//  [ ] Platform: Mouse cursor shape and visibility. Disable with 'io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange'.
//  [X] Platform: Keyboard arrays indexed using XK_* Virtual Key Codes, e.g. ImGui::IsKeyPressed(XK_space).
//  [ ] Platform: Gamepad support. Enabled with 'io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad'.

#pragma once

IMGUI_IMPL_API bool     ImGui_ImplX11_Init(void* display, void* window);
IMGUI_IMPL_API void     ImGui_ImplX11_Shutdown();
IMGUI_IMPL_API void     ImGui_ImplX11_NewFrame();

// Configuration
// - Disable gamepad support
#define IMGUI_IMPL_X11_DISABLE_GAMEPAD

// X11 message handler your application need to call.
// - Intentionally commented out in a '#if 0' block to avoid dragging dependencies on <X11/*> from this helper.
// - You should COPY the line below into your .cpp code to forward declare the function and then you can call it.
#if 0
extern IMGUI_IMPL_API int ImGui_ImplX11_EventHandler(XEvent &event);
#endif
