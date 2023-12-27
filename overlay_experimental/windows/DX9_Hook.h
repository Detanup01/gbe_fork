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

#include <d3d9.h>

class DX9_Hook :
    public ingame_overlay::Renderer_Hook,
    public Base_Hook
{
public:
    static constexpr const char *DLL_NAME = "d3d9.dll";

private:
    static DX9_Hook* _inst;

    // Variables
    bool _Hooked;
    bool _WindowsHooked;
    bool _Initialized;
    HWND _LastWindow;
    IDirect3DDevice9* _pDevice;
    std::set<std::shared_ptr<uint64_t>> _ImageResources;

    // Functions
    DX9_Hook();

    void _ResetRenderState();
    void _PrepareForOverlay(IDirect3DDevice9* pDevice, HWND destWindow);

    // Hook to render functions
    decltype(&IDirect3DDevice9::Reset)       Reset;
    decltype(&IDirect3DDevice9::Present)     Present;
    decltype(&IDirect3DDevice9Ex::PresentEx) PresentEx;
    decltype(&IDirect3DSwapChain9::Present)  SwapChainPresent;

    static HRESULT STDMETHODCALLTYPE MyReset(IDirect3DDevice9* _this, D3DPRESENT_PARAMETERS* pPresentationParameters);
    static HRESULT STDMETHODCALLTYPE MyPresent(IDirect3DDevice9* _this, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion);
    static HRESULT STDMETHODCALLTYPE MyPresentEx(IDirect3DDevice9Ex* _this, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags);
    static HRESULT STDMETHODCALLTYPE MySwapChainPresent(IDirect3DSwapChain9* _this, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags);

public:
    std::string LibraryName;

    virtual ~DX9_Hook();

    virtual bool StartHook(std::function<bool(bool)> key_combination_callback, std::set<ingame_overlay::ToggleKey> toggle_keys);
    virtual bool IsStarted();
    static DX9_Hook* Inst();
    virtual std::string GetLibraryName() const;

    void LoadFunctions(decltype(Present) PresentFcn, decltype(Reset) ResetFcn, decltype(PresentEx) PresentExFcn, decltype(&IDirect3DSwapChain9::Present) SwapChainPresentFcn);

    virtual std::weak_ptr<uint64_t> CreateImageResource(const void* image_data, uint32_t width, uint32_t height);
    virtual void ReleaseImageResource(std::weak_ptr<uint64_t> resource);
};
