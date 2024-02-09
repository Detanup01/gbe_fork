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

#include <d3d12.h>
#include <dxgi1_4.h>

class DX12_Hook : 
    public ingame_overlay::Renderer_Hook,
    public Base_Hook
{
public:
    static constexpr const char *DLL_NAME = "d3d12.dll";

private:
    static DX12_Hook* _inst;
    std::set<std::shared_ptr<uint64_t>> _ImageResources;

    struct texture_t{
        using gpu_heap_t = decltype(D3D12_GPU_DESCRIPTOR_HANDLE::ptr);

        gpu_heap_t gpu_handle; // This must be the first member, ImGui will use the content of the pointer as a D3D12_GPU_DESCRIPTOR_HANDLE::ptr
        ID3D12Resource* pTexture;
        int64_t heap_id;
    };
	
    struct DX12Frame_t
    {
        D3D12_CPU_DESCRIPTOR_HANDLE RenderTarget = {};
        ID3D12CommandAllocator* pCmdAlloc = nullptr;
        ID3D12Resource* pBackBuffer = nullptr;

        inline void Reset()
        {
            pCmdAlloc = nullptr;
            pBackBuffer = nullptr;
        }

        DX12Frame_t() {}
        DX12Frame_t(DX12Frame_t const&) = delete;
        DX12Frame_t& operator=(DX12Frame_t const&) = delete;

        DX12Frame_t(D3D12_CPU_DESCRIPTOR_HANDLE RenderTarget, ID3D12CommandAllocator* pCmdAlloc, ID3D12Resource* pBackBuffer):
            RenderTarget(RenderTarget), pCmdAlloc(pCmdAlloc), pBackBuffer(pBackBuffer)
        {}

        DX12Frame_t(DX12Frame_t&& other) noexcept:
            RenderTarget(other.RenderTarget), pCmdAlloc(other.pCmdAlloc), pBackBuffer(other.pBackBuffer)
        {
            other.Reset();
        }

        DX12Frame_t& operator=(DX12Frame_t&& other) noexcept
        {
            DX12Frame_t tmp(std::move(other));
            RenderTarget = tmp.RenderTarget;
            pCmdAlloc = tmp.pCmdAlloc;
            pBackBuffer = tmp.pBackBuffer;
            tmp.Reset();

            return *this;
        }

        ~DX12Frame_t()
        {
            if (pCmdAlloc != nullptr) pCmdAlloc->Release();
            if (pBackBuffer != nullptr) pBackBuffer->Release();
        }
    };

    struct heap_t
    {
       D3D12_GPU_DESCRIPTOR_HANDLE gpu_handle;
       D3D12_CPU_DESCRIPTOR_HANDLE cpu_handle;
       int64_t id;
    };
	
    // Variables
    bool _Hooked;
    bool _WindowsHooked;
    bool _Initialized;

    size_t CommandQueueOffset;
    ID3D12CommandQueue* pCmdQueue;
    ID3D12Device* pDevice;
    std::vector<DX12Frame_t> OverlayFrames;
	std::vector<bool> srvDescHeapBitmap;
    ID3D12DescriptorHeap* pSrvDescHeap;
    ID3D12GraphicsCommandList* pCmdList;
    ID3D12DescriptorHeap* pRtvDescHeap;

    // Functions
    DX12_Hook();

    heap_t get_free_texture_heap();
    bool release_texture_heap(int64_t heap_id);

    ID3D12CommandQueue* _FindCommandQueueFromSwapChain(IDXGISwapChain* pSwapChain);

    void _ResetRenderState();
    void _ResetStatesOnly();
    void _PrepareForOverlay(IDXGISwapChain* pSwapChain, ID3D12CommandQueue* pCommandQueue);

    // Hook to render functions
    static HRESULT STDMETHODCALLTYPE MyPresent(IDXGISwapChain* _this, UINT SyncInterval, UINT Flags);
    static HRESULT STDMETHODCALLTYPE MyResizeTarget(IDXGISwapChain* _this, const DXGI_MODE_DESC* pNewTargetParameters);
    static HRESULT STDMETHODCALLTYPE MyResizeBuffers(IDXGISwapChain* _this, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
    static void STDMETHODCALLTYPE MyExecuteCommandLists(ID3D12CommandQueue *_this, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists);
    static HRESULT STDMETHODCALLTYPE MyPresent1(IDXGISwapChain1* _this, UINT SyncInterval, UINT Flags, const DXGI_PRESENT_PARAMETERS* pPresentParameters);

    decltype(&IDXGISwapChain::Present)       Present;
    decltype(&IDXGISwapChain::ResizeBuffers) ResizeBuffers;
    decltype(&IDXGISwapChain::ResizeTarget)  ResizeTarget;
    decltype(&ID3D12CommandQueue::ExecuteCommandLists) ExecuteCommandLists;
    decltype(&IDXGISwapChain1::Present1)     Present1;

public:
    std::string LibraryName;

    virtual ~DX12_Hook();

    virtual bool StartHook(std::function<bool(bool)> key_combination_callback, std::set<ingame_overlay::ToggleKey> toggle_keys);
    virtual bool IsStarted();
    static DX12_Hook* Inst();
    virtual std::string GetLibraryName() const;

    void LoadFunctions(
        decltype(Present) PresentFcn,
        decltype(ResizeBuffers) ResizeBuffersFcn,
        decltype(ResizeTarget) ResizeTargetFcn,
        decltype(ExecuteCommandLists) ExecuteCommandListsFcn,
        decltype(Present1) Present1Fcn1);

    virtual std::weak_ptr<uint64_t> CreateImageResource(const void* image_data, uint32_t width, uint32_t height);
    virtual void ReleaseImageResource(std::weak_ptr<uint64_t> resource);
};
