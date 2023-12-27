/*
 * Copyright (C) 2019-2020 Nemirtingas
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

#include <ingame_overlay/Renderer_Hook.h>

#include "overlay/internal_includes.h"

#include <GL/glx.h>

class OpenGLX_Hook :
    public ingame_overlay::Renderer_Hook,
    public Base_Hook
{
public:
    static constexpr const char *DLL_NAME = "libGLX.so";

private:
    static OpenGLX_Hook* _inst;

    // Variables
    bool _Hooked;
    bool _X11Hooked;
    bool _Initialized;
    Display *_Display;
    GLXContext _Context;
    std::set<std::shared_ptr<uint64_t>> _ImageResources;

    // Functions
    OpenGLX_Hook();

    void _ResetRenderState();
    void _PrepareForOverlay(Display* display, GLXDrawable drawable);

    // Hook to render functions
    decltype(::glXSwapBuffers)* glXSwapBuffers;

public:
    std::string LibraryName;

    static void MyglXSwapBuffers(Display* display, GLXDrawable drawable);

    virtual ~OpenGLX_Hook();

    virtual bool StartHook(std::function<bool(bool)> key_combination_callback, std::set<ingame_overlay::ToggleKey> toggle_keys);
    virtual bool IsStarted();
    static OpenGLX_Hook* Inst();
    virtual std::string GetLibraryName() const;
    void LoadFunctions(decltype(::glXSwapBuffers)* pfnglXSwapBuffers);

    virtual std::weak_ptr<uint64_t> CreateImageResource(const void* image_data, uint32_t width, uint32_t height);
    virtual void ReleaseImageResource(std::weak_ptr<uint64_t> resource);
};
