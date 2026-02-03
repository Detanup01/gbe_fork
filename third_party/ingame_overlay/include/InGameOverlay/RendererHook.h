/*
 * Copyright (C) Nemirtingas
 * This file is part of the ingame overlay project
 *
 * The ingame overlay project is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 * 
 * The ingame overlay project is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with the ingame overlay project; if not, see
 * <http://www.gnu.org/licenses/>.
 */

#pragma once

#include <functional>
#include <cstdint>

#include "RendererResource.h"

namespace InGameOverlay {

enum class ToggleKey
{
    SHIFT, CTRL, ALT,
    TAB,
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
};

enum class OverlayHookState : uint8_t
{
    Ready,
    Reset,
    Removing,
};

/// <summary>
/// Only one RendererHookType_t will always be returned by RendererHook_t::GetRendererHookType
/// but you can use them as flags to limit the detection in InGameOverlay::DetectRenderer.
/// NOTE: Even if a DirectX hook has been found and DXVK is used, the renderer hook will only report the DirectX hook type.
/// There is no combination like : DirectX11 | Vulkan returned by RendererHook_t::GetRendererHookType.
/// </summary>
enum class RendererHookType_t : uint8_t
{
    DirectX9   = 1 << 0,
    DirectX10  = 1 << 1,
    DirectX11  = 1 << 2,
    DirectX12  = 1 << 3,
    OpenGL     = 1 << 4,
    Vulkan     = 1 << 5,
    Metal      = 1 << 6,
    AnyDirectX = DirectX9 | DirectX10 | DirectX11 | DirectX12,
    AnyWindows = DirectX9 | DirectX10 | DirectX11 | DirectX12 | OpenGL | Vulkan,
    AnyLinux   = OpenGL | Vulkan,
    AnyMacOS   = OpenGL | Metal,
    Any        = DirectX9 | DirectX10 | DirectX11 | DirectX12 | OpenGL | Vulkan | Metal,
};

enum class ScreenshotType_t : uint8_t
{
    None = 0,
    BeforeOverlay = 1,
    AfterOverlay = 2,
};

enum class ScreenshotDataFormat_t : uint16_t
{
    Unknown = 0,
    // 8-bit formats
    R8G8B8,
    X8R8G8B8,
    A8R8G8B8,
    B8G8R8A8,
    B8G8R8X8,
    R8G8B8A8,

    // 10-bit formats
    A2R10G10B10,
    A2B10G10R10,
    R10G10B10A2,

    // 16-bit formats
    R5G6B5,
    X1R5G5B5,
    A1R5G5B5,
    B5G6R5,
    B5G5R5A1,

    // HDR / float formats
    R16G16B16A16_FLOAT,
    R16G16B16A16_UNORM,
    R32G32B32A32_FLOAT,
};

/// <summary>
///   The screenshot pixels data and format. To copy this buffer, you can reserve Width * Height * PixelSize bytes, but you will need to
///   copy row by row because of the pitch.
/// </summary>
struct ScreenshotCallbackParameter_t
{
    uint32_t Width;
    uint32_t Height;
    uint32_t PixelSize;
    uint32_t Pitch;
    void* Data;
    ScreenshotDataFormat_t Format;
};

typedef void (*ScreenshotCallback_t)(ScreenshotCallbackParameter_t const* screenshot, void* userParameter);

/// <summary>
///   The renderer hook.
///     ResourceAutoLoad_t: Default value is ResourceAutoLoad_t::Batch
///     BatchSize: Default value is 10
/// </summary>
class RendererHook_t
{

public:
    virtual ~RendererHook_t() {}

    // TODO: Deprecated direct use of thoses and use either a setter or plain C function pointers with void* user parameter.
    std::function<void()> OverlayProc;
    std::function<void(OverlayHookState)> OverlayHookReady;

    virtual void SetScreenshotCallback(ScreenshotCallback_t callback, void* userParam) = 0;

    /// <summary>
    ///   Starts the current renderer hook procedure, allowing a user to render things on the application window.
    /// </summary>
    /// <param name="key_combination_callback">
    ///   Callback called when your toggle_keys are all pressed.
    /// </param>
    /// <param name="toggle_keys">
    ///   The key combination that must be pressed to show/hide your overlay.
    /// </param>
    /// <param name="imgui_font_atlas">
    ///   *Can be nullptr*. Fill this parameter with your own ImGuiAtlas pointer if you don't want ImGui to generate one for you.
    /// </param>
    /// <returns></returns>
    virtual bool StartHook(std::function<void()> keyCombinationCallback, ToggleKey toggleKeys[], int toggleKeysCount, /*ImFontAtlas* */ void* imguiFontAtlas = nullptr) = 0;

    /// <summary>
    ///   Change the hooked application input policy.
    /// </summary>
    /// <param name="hide">
    ///   Set to true to hide mouse and keyboards inputs from the hooked application.
    ///   Set to false to allow the hooked application to receive inputs.
    /// </param>
    /// <returns></returns>
    virtual void HideAppInputs(bool hide) = 0;
    
    /// <summary>
    ///   Change the overlay input policy.
    /// </summary>
    /// <param name="hide">
    ///   Set to true to hide mouse and keyboards inputs from the overlay.
    ///   Set to false to allow the overlay to receive inputs.
    /// </param>
    /// <returns></returns>
    virtual void HideOverlayInputs(bool hide) = 0;

    /// <summary>
    ///   Returns the hook state. If its started, then the functions are hooked (redirected to InGameOverlay) and will intercepts the application frame rendering.
    /// </summary>
    /// <returns></returns>
    virtual bool IsStarted() = 0;

    /// <summary>
    ///   Get the current renderer library name.
    /// </summary>
    /// <returns></returns>
    virtual const char* GetLibraryName() const = 0;

    /// <summary>
    ///   Get the current renderer hook type.
    /// </summary>
    /// <returns></returns>
    virtual RendererHookType_t GetRendererHookType() const = 0;

    /// <summary>
    ///   Gets the auto load batch size.
    /// </summary>
    /// <returns></returns>
    virtual uint32_t GetAutoLoadBatchSize() = 0;

    /// <summary>
    ///   Sets how many resources the renderer hook can load before rendering the frame, to not block the rendering with a hundred of resource to load.
    /// </summary>
    /// <param name="batchSize"></param>
    virtual void SetAutoLoadBatchSize(uint32_t batchSize) = 0;

    /// <summary>
    ///   Gets if the resources will be auto loaded by default. This state doesn't affect resources created before its call.
    /// </summary>
    /// <returns></returns>
    virtual ResourceAutoLoad_t GetResourceAutoLoad() const = 0;

    /// <summary>
    ///   Sets if the resources will be auto loaded by default.
    /// </summary>
    /// <param name="autoLoad"></param>
    virtual void SetResourceAutoLoad(ResourceAutoLoad_t autoLoad = ResourceAutoLoad_t::Batch) = 0;

    /// <summary>
    ///   Creates an image resource that can be setup and used later.
    /// </summary>
    /// <returns></returns>
    virtual RendererResource_t* CreateResource() = 0;

    /// <summary>
    ///   Loads an RGBA ordered buffer into GPU and returns a resource.
    /// </summary>
    /// <param name="image_data">
    ///   The RGBA buffer.
    /// </param>
    /// <param name="width">
    ///   Your RGBA image width.
    /// </param>
    /// <param name="height">
    ///   Your RGBA image height.
    /// </param>
    /// <param name="attach">
    ///   If set, the pointer, width and height will be stored in the RendererResource_t to be reloaded later if a renderer reset occurs.
    /// </param>
    /// <returns></returns>
    virtual RendererResource_t* CreateAndLoadResource(const void* image_data, uint32_t width, uint32_t height, bool attach) = 0;

    virtual void TakeScreenshot(ScreenshotType_t type) = 0;
};

}

inline InGameOverlay::RendererHookType_t operator&(InGameOverlay::RendererHookType_t l, InGameOverlay::RendererHookType_t r)
{
    return static_cast<InGameOverlay::RendererHookType_t>(static_cast<uint8_t>(l) & static_cast<uint8_t>(r));
}

inline InGameOverlay::RendererHookType_t operator|(InGameOverlay::RendererHookType_t l, InGameOverlay::RendererHookType_t r)
{
    return static_cast<InGameOverlay::RendererHookType_t>(static_cast<uint8_t>(l) | static_cast<uint8_t>(r));
}