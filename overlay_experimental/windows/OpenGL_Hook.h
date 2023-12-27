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

class OpenGL_Hook :
    public ingame_overlay::Renderer_Hook,
    public Base_Hook
{
public:
    static constexpr const char *DLL_NAME = "opengl32.dll";

    using wglSwapBuffers_t = BOOL(WINAPI*)(HDC);

private:
    static OpenGL_Hook* _inst;

    // Variables
    bool _Hooked;
    bool _WindowsHooked;
    bool _Initialized;
    HWND _LastWindow;
    std::set<std::shared_ptr<uint64_t>> _ImageResources;

    // Functions
    OpenGL_Hook();

    void _ResetRenderState();
    void _PrepareForOverlay(HDC hDC);

    // Hook to render functions
    static BOOL WINAPI MywglSwapBuffers(HDC hDC);

    wglSwapBuffers_t wglSwapBuffers;

public:
    std::string LibraryName;

    virtual ~OpenGL_Hook();

    virtual bool StartHook(std::function<bool(bool)> key_combination_callback, std::set<ingame_overlay::ToggleKey> toggle_keys);
    virtual bool IsStarted();
    static OpenGL_Hook* Inst();
    virtual std::string GetLibraryName() const;
    void LoadFunctions(wglSwapBuffers_t pfnwglSwapBuffers);

    virtual std::weak_ptr<uint64_t> CreateImageResource(const void* image_data, uint32_t width, uint32_t height);
    virtual void ReleaseImageResource(std::weak_ptr<uint64_t> resource);
};
