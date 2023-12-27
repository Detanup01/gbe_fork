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

#include "overlay/internal_includes.h"

#include <vulkan/vulkan.h>

class Vulkan_Hook :
    public ingame_overlay::Renderer_Hook,
    public Base_Hook
{
public:
    static constexpr const char *DLL_NAME = "vulkan-1.dll";

private:
    static Vulkan_Hook* _inst;

    // Variables
    bool _Hooked;
    bool _WindowsHooked;
    bool _Initialized;

    // Functions
    Vulkan_Hook();

    void _ResetRenderState();
    void _PrepareForOverlay();

    // Hook to render functions
    static VKAPI_ATTR VkResult VKAPI_CALL MyvkQueuePresentKHR(VkQueue queue, const VkPresentInfoKHR* pPresentInfo);

    decltype(::vkQueuePresentKHR)* vkQueuePresentKHR;

public:
    std::string LibraryName;

    virtual ~Vulkan_Hook();

    virtual bool StartHook(std::function<bool(bool)> key_combination_callback, std::set<ingame_overlay::ToggleKey> toggle_keys);
    virtual bool IsStarted();
    static Vulkan_Hook* Inst();
    virtual std::string GetLibraryName() const;
    void LoadFunctions(decltype(::vkQueuePresentKHR)* _vkQueuePresentKHR);

    virtual std::weak_ptr<uint64_t> CreateImageResource(const void* image_data, uint32_t width, uint32_t height);
    virtual void ReleaseImageResource(std::weak_ptr<uint64_t> resource);
};
