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

#include <X11/X.h> // XEvent types
#include <X11/Xlib.h> // XEvent structure
#include <X11/Xutil.h> // XEvent keysym

class X11_Hook :
    public Base_Hook
{
public:
    static constexpr const char* DLL_NAME = "libX11.so";

private:
    static X11_Hook* _inst;

    // Variables
    bool _Hooked;
    bool _Initialized;
    Window _GameWnd;

    // In (bool): Is toggle wanted
    // Out(bool): Is the overlay visible, if true, inputs will be disabled
    std::function<bool(bool)> _KeyCombinationCallback;
    std::set<uint32_t> _NativeKeyCombination;
    bool _KeyCombinationPushed;

    // Functions
    X11_Hook();
    int _CheckForOverlay(Display *d, int num_events);

    // Hook to X11 window messages
    decltype(::XEventsQueued)* XEventsQueued;
    decltype(::XPending)* XPending;

    static int MyXEventsQueued(Display * display, int mode);
    static int MyXPending(Display* display);

public:
    std::string LibraryName;

    virtual ~X11_Hook();

    void ResetRenderState();
    void SetInitialWindowSize(Display* display, Window wnd);
    bool PrepareForOverlay(Display *display, Window wnd);

    Window GetGameWnd() const{ return _GameWnd; }

    bool StartHook(std::function<bool(bool)>& key_combination_callback, std::set<ingame_overlay::ToggleKey> const& toggle_keys);
    static X11_Hook* Inst();
    virtual std::string GetLibraryName() const;
};
