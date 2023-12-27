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

#include <d3d11.h>
#include <dxgi1_2.h>

class DX11_Hook : 
    public ingame_overlay::Renderer_Hook,
    public Base_Hook
{
public:
    static constexpr const char *DLL_NAME = "d3d11.dll";

private:
    static DX11_Hook* _inst;

    // Variables
    bool _Hooked;
    bool _WindowsHooked;
    bool _Initialized;
    ID3D11Device* pDevice;
    ID3D11DeviceContext* pContext;
    ID3D11RenderTargetView* mainRenderTargetView;
    std::set<std::shared_ptr<uint64_t>> _ImageResources;

    // Functions
    DX11_Hook();

    void _ResetRenderState();
    void _PrepareForOverlay(IDXGISwapChain* pSwapChain);

    // Hook to render functions
    static HRESULT STDMETHODCALLTYPE MyPresent(IDXGISwapChain* _this, UINT SyncInterval, UINT Flags);
    static HRESULT STDMETHODCALLTYPE MyResizeTarget(IDXGISwapChain* _this, const DXGI_MODE_DESC* pNewTargetParameters);
    static HRESULT STDMETHODCALLTYPE MyResizeBuffers(IDXGISwapChain* _this, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
    static HRESULT STDMETHODCALLTYPE MyPresent1(IDXGISwapChain1* _this, UINT SyncInterval, UINT Flags, const DXGI_PRESENT_PARAMETERS* pPresentParameters);

    decltype(&IDXGISwapChain::Present)       Present;
    decltype(&IDXGISwapChain::ResizeBuffers) ResizeBuffers;
    decltype(&IDXGISwapChain::ResizeTarget)  ResizeTarget;
    decltype(&IDXGISwapChain1::Present1)     Present1;

public:
    std::string LibraryName;

    virtual ~DX11_Hook();

    virtual bool StartHook(std::function<bool(bool)> key_combination_callback, std::set<ingame_overlay::ToggleKey> toggle_keys);
    virtual bool IsStarted();
    static DX11_Hook* Inst();
    virtual std::string GetLibraryName() const;

    void LoadFunctions(
        decltype(Present) PresentFcn,
        decltype(ResizeBuffers) ResizeBuffersFcn,
        decltype(ResizeTarget) ResizeTargetFcn,
        decltype(Present1) Present1Fcn);

    virtual std::weak_ptr<uint64_t> CreateImageResource(const void* image_data, uint32_t width, uint32_t height);
    virtual void ReleaseImageResource(std::weak_ptr<uint64_t> resource);
};
