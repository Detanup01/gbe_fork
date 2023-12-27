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
#include <string>
#include <memory>
#include <cstdint>
#include <set>

namespace ingame_overlay {

enum class ToggleKey
{
    SHIFT, CTRL, ALT,
    TAB,
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
};

class Renderer_Hook
{
public:
    Renderer_Hook():
        OverlayProc(&DefaultOverlayProc),
        OverlayHookReady(&DefaultOverlayHookReady),
        ImGuiFontAtlas(nullptr)
    {}

    static void DefaultOverlayProc() {}
    static void DefaultOverlayHookReady(bool) {}
    std::function<void()> OverlayProc;
    std::function<void(bool)> OverlayHookReady;

    void *ImGuiFontAtlas;
    virtual bool StartHook(std::function<bool(bool)> key_combination_callback, std::set<ToggleKey> toggle_keys) = 0;
    virtual bool IsStarted() = 0;
    // Returns a Handle to the renderer image ressource or nullptr if it failed to create the resource, the handle can be used in ImGui's Image calls, image_buffer must be RGBA ordered
    virtual std::weak_ptr<uint64_t> CreateImageResource(const void* image_data, uint32_t width, uint32_t height) = 0;
    virtual void ReleaseImageResource(std::weak_ptr<uint64_t> resource) = 0;

    virtual std::string GetLibraryName() const = 0;
};

}