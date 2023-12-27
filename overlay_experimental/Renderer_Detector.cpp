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

#include <cassert>

#include "overlay/Renderer_Detector.h"

#include "System/Encoding.hpp"
#include "System/String.hpp"
#include "System/System.h"
#include "System/Library.h"
#include "System/ScopedLock.hpp"
#define GLAD_GL_IMPLEMENTATION
#include <glad/gl.h>

#if defined(WIN64) || defined(_WIN64) || defined(__MINGW64__) \
 || defined(WIN32) || defined(_WIN32) || defined(__MINGW32__)
    #define RENDERERDETECTOR_OS_WINDOWS
  
    #include "windows/DX12_Hook.h"
    #include "windows/DX11_Hook.h"
    #include "windows/DX10_Hook.h"
    #include "windows/DX9_Hook.h"
    #include "windows/OpenGL_Hook.h"
    #include "windows/Vulkan_Hook.h"
  
    #include "windows/DirectX_VTables.h"
  
    #include <random>
  
    #ifdef GetModuleHandle
        #undef GetModuleHandle
    #endif

#elif defined(__linux__) || defined(linux)
    #define RENDERERDETECTOR_OS_LINUX

    #include "linux/OpenGLX_Hook.h"

#elif defined(__APPLE__)
    #define RENDERERDETECTOR_OS_APPLE

    #include "macosx/OpenGL_Hook.h"

#endif

class Renderer_Detector
{
    static Renderer_Detector* instance;
public:
    static Renderer_Detector* Inst()
    {
        if (instance == nullptr)
        {
            instance = new Renderer_Detector;
        }
        return instance;
    }

#if defined(RENDERERDETECTOR_OS_WINDOWS)
#define RENDERER_HOOKS { OpenGL_Hook::DLL_NAME, &Renderer_Detector::hook_opengl },\
                       { Vulkan_Hook::DLL_NAME, &Renderer_Detector::hook_vulkan },\
                       {   DX12_Hook::DLL_NAME, &Renderer_Detector::hook_dx12   },\
                       {   DX11_Hook::DLL_NAME, &Renderer_Detector::hook_dx11   },\
                       {   DX10_Hook::DLL_NAME, &Renderer_Detector::hook_dx10   },\
                       {    DX9_Hook::DLL_NAME, &Renderer_Detector::hook_dx9    },

    ~Renderer_Detector()
    {
        stop_detection();

        delete dx9_hook;
        delete dx10_hook;
        delete dx11_hook;
        delete dx12_hook;
        delete opengl_hook;
        delete vulkan_hook;

        instance = nullptr;
    }

private:

    decltype(&IDXGISwapChain::Present)       IDXGISwapChainPresent;
    decltype(&IDXGISwapChain1::Present1)     IDXGISwapChainPresent1;
    decltype(&IDirect3DDevice9::Present)     IDirect3DDevice9Present;
    decltype(&IDirect3DDevice9Ex::PresentEx) IDirect3DDevice9ExPresentEx;
    decltype(&IDirect3DSwapChain9::Present)  IDirect3DSwapChain9Present;
    decltype(::SwapBuffers)* wglSwapBuffers;
    decltype(::vkQueuePresentKHR)* vkQueuePresentKHR;

    bool dxgi_hooked;
    bool dxgi1_2_hooked;
    bool dx12_hooked;
    bool dx11_hooked;
    bool dx10_hooked;
    bool dx9_hooked;
    bool opengl_hooked;
    bool vulkan_hooked;

    DX12_Hook* dx12_hook;
    DX11_Hook* dx11_hook;
    DX10_Hook* dx10_hook;
    DX9_Hook* dx9_hook;
    OpenGL_Hook* opengl_hook;
    Vulkan_Hook* vulkan_hook;

    HWND dummyWindow = nullptr;
    std::wstring _WindowClassName;
    std::string _SystemDir;
    ATOM atom = 0;

    Renderer_Detector() :
        dxgi_hooked(false),
        dxgi1_2_hooked(false),
        dx12_hooked(false),
        dx11_hooked(false),
        dx10_hooked(false),
        dx9_hooked(false),
        opengl_hooked(false),
        vulkan_hooked(false),
        renderer_hook(nullptr),
        dx9_hook(nullptr),
        dx10_hook(nullptr),
        dx11_hook(nullptr),
        dx12_hook(nullptr),
        opengl_hook(nullptr),
        vulkan_hook(nullptr),
        detection_done(false),
        detection_count(0),
        detection_cancelled(false)
    {
        std::wstring tmp(4096, L'\0');
        tmp.resize(GetSystemDirectoryW(&tmp[0], tmp.size()));
        _SystemDir = System::Encoding::WCharToUtf8(tmp);

        System::String::ToLower(_SystemDir);

        wchar_t random_str[] = L"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
        std::random_device rd;
        std::mt19937_64 gen(rd());

        std::uniform_int_distribution<uint64_t> dis(0, 61);
        _WindowClassName.resize(64);
        for (int i = 0; i < 64; ++i)
            _WindowClassName[i] = random_str[dis(gen)];
    }

    std::string FindPreferedModulePath(std::string const& name)
    {
        std::string res;
        std::string tmp;
        auto modules = System::GetModules();
        for (auto& item : modules)
        {
            tmp = System::String::CopyLower(item);
            if (tmp.length() >= name.length() && strcmp(tmp.c_str() + tmp.length() - name.length(), name.c_str()) == 0)
            {
                if (strncmp(tmp.c_str(), _SystemDir.c_str(), _SystemDir.length()) == 0)
                    return item;

                // I don't care which one is picked if we can't find a library in the system32 folder...
                res = std::move(item);
            }
        }

        return res;
    }

    HWND CreateHWND()
    {
        if (dummyWindow == nullptr)
        {
            HINSTANCE hInst = GetModuleHandleW(nullptr);
            if (atom == 0)
            {
                // Register a window class for creating our render window with.
                WNDCLASSEXW windowClass = {};

                windowClass.cbSize = sizeof(WNDCLASSEX);
                windowClass.style = CS_HREDRAW | CS_VREDRAW;
                windowClass.lpfnWndProc = DefWindowProc;
                windowClass.cbClsExtra = 0;
                windowClass.cbWndExtra = 0;
                windowClass.hInstance = hInst;
                windowClass.hIcon = NULL;
                windowClass.hCursor = ::LoadCursor(NULL, IDC_ARROW);
                windowClass.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
                windowClass.lpszMenuName = NULL;
                windowClass.lpszClassName = _WindowClassName.c_str();
                windowClass.hIconSm = NULL;

                atom = ::RegisterClassExW(&windowClass);
            }

            if (atom > 0)
            {
                dummyWindow = ::CreateWindowExW(
                    NULL,
                    _WindowClassName.c_str(),
                    L"",
                    WS_OVERLAPPEDWINDOW,
                    0,
                    0,
                    1,
                    1,
                    NULL,
                    NULL,
                    hInst,
                    nullptr
                );

                assert(dummyWindow && "Failed to create window");
            }
        }

        return dummyWindow;
    }

    void DestroyHWND()
    {
        if (dummyWindow != nullptr)
        {
            DestroyWindow(dummyWindow);
            UnregisterClassW(_WindowClassName.c_str(), GetModuleHandleW(nullptr));

            dummyWindow = nullptr;
            atom = 0;
        }
    }

    template<typename T>
    void HookDetected(T*& detected_renderer)
    {
        detection_hooks.UnhookAll();
        renderer_hook = static_cast<ingame_overlay::Renderer_Hook*>(detected_renderer);
        detected_renderer = nullptr;
        detection_done = true;
        DestroyHWND();
    }

    void DeduceDXVersionFromSwapChain(IDXGISwapChain* pSwapChain)
    {
        IUnknown* pDevice = nullptr;
        if (Inst()->dx12_hooked)
        {
            pSwapChain->GetDevice(IID_PPV_ARGS(reinterpret_cast<ID3D12Device**>(&pDevice)));
        }
        if (pDevice != nullptr)
        {
            HookDetected(dx12_hook);
        }
        else
        {
            if (dx11_hooked)
            {
                pSwapChain->GetDevice(IID_PPV_ARGS(reinterpret_cast<ID3D11Device**>(&pDevice)));
            }
            if (pDevice != nullptr)
            {
                HookDetected(dx11_hook);
            }
            else
            {
                if (dx10_hooked)
                {
                    pSwapChain->GetDevice(IID_PPV_ARGS(reinterpret_cast<ID3D10Device**>(&pDevice)));
                }
                if (pDevice != nullptr)
                {
                    HookDetected(dx10_hook);
                }
            }
        }
        if (pDevice != nullptr)
        {
            pDevice->Release();
        }
    }

    static HRESULT STDMETHODCALLTYPE MyIDXGISwapChain_Present(IDXGISwapChain* _this, UINT SyncInterval, UINT Flags)
    {
        auto inst = Inst();
        HRESULT res;
        bool locked;
        std::unique_lock<std::mutex> lk(inst->renderer_mutex, std::defer_lock);

        // It appears that (NVidia at least) calls IDXGISwapChain when calling OpenGL or Vulkan SwapBuffers.
        // So only lock when OpenGL or Vulkan hasn't already locked the mutex.
        locked = lk.try_lock();
        res = (_this->*inst->IDXGISwapChainPresent)(SyncInterval, Flags);
        if (!locked || inst->detection_done)
            return res;

        inst->DeduceDXVersionFromSwapChain(_this);

        return res;
    }

    static HRESULT STDMETHODCALLTYPE MyIDXGISwapChain_Present1(IDXGISwapChain1* _this, UINT SyncInterval, UINT Flags, const DXGI_PRESENT_PARAMETERS* pPresentParameters)
    {
        auto inst = Inst();
        HRESULT res;
        bool locked;
        std::unique_lock<std::mutex> lk(inst->renderer_mutex, std::defer_lock);

        // It appears that (NVidia at least) calls IDXGISwapChain when calling OpenGL or Vulkan SwapBuffers.
        // So only lock when OpenGL or Vulkan hasn't already locked the mutex.
        locked = lk.try_lock();
        res = (_this->*inst->IDXGISwapChainPresent1)(SyncInterval, Flags, pPresentParameters);
        if (!locked || inst->detection_done)
            return res;

        inst->DeduceDXVersionFromSwapChain(_this);

        return res;
    }

    static HRESULT STDMETHODCALLTYPE MyDX9Present(IDirect3DDevice9* _this, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion)
    {
        auto inst = Inst();
        std::lock_guard<std::mutex> lk(inst->renderer_mutex);

        auto res = (_this->*inst->IDirect3DDevice9Present)(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
        if (inst->detection_done)
            return res;

        inst->HookDetected(inst->dx9_hook);

        return res;
    }

    static HRESULT STDMETHODCALLTYPE MyDX9PresentEx(IDirect3DDevice9Ex* _this, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags)
    {
        auto inst = Inst();
        std::lock_guard<std::mutex> lk(inst->renderer_mutex);

        auto res = (_this->*inst->IDirect3DDevice9ExPresentEx)(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
        if (inst->detection_done)
            return res;

        inst->HookDetected(inst->dx9_hook);

        return res;
    }

    static HRESULT STDMETHODCALLTYPE MyDX9SwapChainPresent(IDirect3DSwapChain9* _this, CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags)
    {
        auto inst = Inst();
        std::lock_guard<std::mutex> lk(inst->renderer_mutex);

        auto res = (_this->*inst->IDirect3DSwapChain9Present)(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
        if (inst->detection_done)
            return res;

        inst->HookDetected(inst->dx9_hook);

        return res;
    }

    static BOOL WINAPI MywglSwapBuffers(HDC hDC)
    {
        auto inst = Inst();
        std::lock_guard<std::mutex> lk(inst->renderer_mutex);

        auto res = inst->wglSwapBuffers(hDC);
        if (inst->detection_done)
            return res;

        if (gladLoaderLoadGL() >= GLAD_MAKE_VERSION(3, 1))
        {
            inst->HookDetected(inst->opengl_hook);
        }

        return res;
    }

    static VkResult VKAPI_CALL MyvkQueuePresentKHR(VkQueue Queue, const VkPresentInfoKHR* pPresentInfo)
    {
        auto inst = Inst();
        std::lock_guard<std::mutex> lk(inst->renderer_mutex);

        auto res = inst->vkQueuePresentKHR(Queue, pPresentInfo);
        if (inst->detection_done)
            return res;

        inst->HookDetected(inst->vulkan_hook);

        return res;
    }

    void HookDXGIPresent(IDXGISwapChain* pSwapChain, decltype(&IDXGISwapChain::Present)& pfnPresent, decltype(&IDXGISwapChain::ResizeBuffers)& pfnResizeBuffers, decltype(&IDXGISwapChain::ResizeTarget)& pfnResizeTarget)
    {
        void** vTable = *reinterpret_cast<void***>(pSwapChain);
        (void*&)pfnPresent = vTable[(int)IDXGISwapChainVTable::Present];
        (void*&)pfnResizeBuffers = vTable[(int)IDXGISwapChainVTable::ResizeBuffers];
        (void*&)pfnResizeTarget = vTable[(int)IDXGISwapChainVTable::ResizeTarget];

        if (!dxgi_hooked)
        {
            dxgi_hooked = true;

            (void*&)IDXGISwapChainPresent = vTable[(int)IDXGISwapChainVTable::Present];
            detection_hooks.BeginHook();
            detection_hooks.HookFunc(std::pair<void**, void*>{ (void**)&IDXGISwapChainPresent, (void*)&MyIDXGISwapChain_Present});
            detection_hooks.EndHook();
        }
    }

    void HookDXGIPresent1(IDXGISwapChain1* pSwapChain1, decltype(&IDXGISwapChain1::Present1)& pfnPresent1)
    {
        void** vTable = *reinterpret_cast<void***>(pSwapChain1);
        (void*&)pfnPresent1 = vTable[(int)IDXGISwapChainVTable::Present1];

        if (!dxgi1_2_hooked)
        {
            dxgi1_2_hooked = true;

            (void*&)IDXGISwapChainPresent1 = vTable[(int)IDXGISwapChainVTable::Present1];
            detection_hooks.BeginHook();
            detection_hooks.HookFunc(std::pair<void**, void*>{ (void**)&IDXGISwapChainPresent1, (void*)&MyIDXGISwapChain_Present1});
            detection_hooks.EndHook();
        }
    }

    void HookDX9Present(IDirect3DDevice9* pDevice, bool ex, IDirect3DSwapChain9* pSwapChain,
        void*& pfnPresent,
        void*& pfnReset,
        void*& pfnPresentEx,
        void*& pfnSwapChainPresent)
    {
        void** vTable = *reinterpret_cast<void***>(pDevice);
        pfnPresent = vTable[(int)IDirect3DDevice9VTable::Present];
        pfnReset = vTable[(int)IDirect3DDevice9VTable::Reset];

        (void*&)IDirect3DDevice9Present = vTable[(int)IDirect3DDevice9VTable::Present];

        detection_hooks.BeginHook();
        detection_hooks.HookFunc(std::pair<void**, void*>{ (void**)&IDirect3DDevice9Present, (void*)&MyDX9Present });
        detection_hooks.EndHook();

        if (ex)
        {
            pfnPresentEx = vTable[(int)IDirect3DDevice9VTable::PresentEx];
            (void*&)IDirect3DDevice9ExPresentEx = vTable[(int)IDirect3DDevice9VTable::PresentEx];

            detection_hooks.BeginHook();
            detection_hooks.HookFunc(std::pair<void**, void*>{ (void**)&IDirect3DDevice9ExPresentEx, (void*)&MyDX9PresentEx });
            detection_hooks.EndHook();
        }
        else
        {
            pfnPresentEx = nullptr;
            IDirect3DDevice9ExPresentEx = nullptr;
        }

        if (pSwapChain != nullptr)
        {
            vTable = *reinterpret_cast<void***>(pSwapChain);
            pfnSwapChainPresent = vTable[(int)IDirect3DSwapChain9VTable::Present];
            (void*&)IDirect3DSwapChain9Present = vTable[(int)IDirect3DSwapChain9VTable::Present];

            detection_hooks.BeginHook();
            detection_hooks.HookFunc(std::pair<void**, void*>{ (void**)&IDirect3DSwapChain9Present, (void*)&MyDX9SwapChainPresent });
            detection_hooks.EndHook();
        }
        else
        {
            pfnSwapChainPresent = nullptr;
        }
    }

    void HookwglSwapBuffers(decltype(::SwapBuffers)* _wglSwapBuffers)
    {
        wglSwapBuffers = _wglSwapBuffers;

        detection_hooks.BeginHook();
        detection_hooks.HookFunc(std::pair<void**, void*>{ (void**)&wglSwapBuffers, (void*)&MywglSwapBuffers });
        detection_hooks.EndHook();
    }

    void HookvkQueuePresentKHR(decltype(::vkQueuePresentKHR)* _vkQueuePresentKHR)
    {
        vkQueuePresentKHR = _vkQueuePresentKHR;

        detection_hooks.BeginHook();
        detection_hooks.HookFuncs(
            std::pair<void**, void*>{ (void**)&vkQueuePresentKHR, (void*)&MyvkQueuePresentKHR }
        );
        detection_hooks.EndHook();
    }

    void hook_dx9(std::string const& library_path)
    {
        if (!dx9_hooked)
        {
            System::Library::Library libD3d9;
            if (!libD3d9.OpenLibrary(library_path, false))
            {
                SPDLOG_WARN("Failed to load {} to detect DX9", library_path);
                return;
            }

            IDirect3D9Ex* pD3D = nullptr;
            IDirect3DDevice9* pDevice = nullptr;
            IDirect3DSwapChain9* pSwapChain = nullptr;

            auto Direct3DCreate9Ex = libD3d9.GetSymbol<decltype(::Direct3DCreate9Ex)>("Direct3DCreate9Ex");

            D3DPRESENT_PARAMETERS params = {};
            params.BackBufferWidth = 1;
            params.BackBufferHeight = 1;
            params.hDeviceWindow = dummyWindow;
            params.BackBufferCount = 1;
            params.Windowed = TRUE;
            params.SwapEffect = D3DSWAPEFFECT_DISCARD;

            if (Direct3DCreate9Ex != nullptr)
            {
                Direct3DCreate9Ex(D3D_SDK_VERSION, &pD3D);
                pD3D->CreateDeviceEx(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, NULL, D3DCREATE_HARDWARE_VERTEXPROCESSING, &params, NULL, reinterpret_cast<IDirect3DDevice9Ex**>(&pDevice));
            }

            if (pDevice == nullptr)
            {
                Direct3DCreate9Ex = nullptr;
                auto Direct3DCreate9 = libD3d9.GetSymbol<decltype(::Direct3DCreate9)>("Direct3DCreate9");
                if (Direct3DCreate9 != nullptr)
                {
                    // D3DDEVTYPE_HAL
                    pD3D = reinterpret_cast<IDirect3D9Ex*>(Direct3DCreate9(D3D_SDK_VERSION));
                    pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, NULL, D3DCREATE_HARDWARE_VERTEXPROCESSING, &params, &pDevice);
                }
            }

            if (pDevice != nullptr)
            {
                SPDLOG_INFO("Hooked D3D9::Present to detect DX Version");

                dx9_hooked = true;

                pDevice->GetSwapChain(0, &pSwapChain);

                decltype(&IDirect3DDevice9::Present) pfnPresent;
                decltype(&IDirect3DDevice9::Reset) pfnReset;
                decltype(&IDirect3DDevice9Ex::PresentEx) pfnPresentEx;
                decltype(&IDirect3DSwapChain9::Present) pfnSwapChainPresent;

                HookDX9Present(pDevice, Direct3DCreate9Ex != nullptr, pSwapChain, (void*&)pfnPresent, (void*&)pfnReset, (void*&)pfnPresentEx, (void*&)pfnSwapChainPresent);

                dx9_hook = DX9_Hook::Inst();
                dx9_hook->LibraryName = library_path;
                dx9_hook->LoadFunctions(pfnPresent, pfnReset, pfnPresentEx, pfnSwapChainPresent);
            }
            else
            {
                SPDLOG_WARN("Failed to hook D3D9::Present to detect DX Version");
            }

            if (pSwapChain) pSwapChain->Release();
            if (pDevice) pDevice->Release();
            if (pD3D) pD3D->Release();
        }
    }

    void hook_dx10(std::string const& library_path)
    {
        if (!dx10_hooked)
        {
            System::Library::Library libD3d10;
            if (!libD3d10.OpenLibrary(library_path, false))
            {
                SPDLOG_WARN("Failed to load {} to detect DX10", library_path);
                return;
            }
            std::string dxgi_path = FindPreferedModulePath("dxgi.dll");

            IDXGISwapChain* pSwapChain = nullptr;
            ID3D10Device* pDevice = nullptr;
            int version = 0;

            if (!dxgi_path.empty())
            {
                HMODULE dxgi = GetModuleHandleA(dxgi_path.c_str());
                if (dxgi != nullptr)
                {
                    IDXGIFactory2* pDXGIFactory = nullptr;

                    auto D3D10CreateDevice = libD3d10.GetSymbol<decltype(::D3D10CreateDevice)>("D3D10CreateDevice");
                    decltype(CreateDXGIFactory1)* CreateDXGIFactory1 = (decltype(CreateDXGIFactory1))GetProcAddress(dxgi, "CreateDXGIFactory1");

                    if (D3D10CreateDevice != nullptr && CreateDXGIFactory1 != nullptr)
                    {
                        D3D10CreateDevice(NULL, D3D10_DRIVER_TYPE_NULL, NULL, 0, D3D10_SDK_VERSION, &pDevice);
                        if (pDevice != nullptr)
                        {
                            CreateDXGIFactory1(IID_PPV_ARGS(&pDXGIFactory));
                            if (pDXGIFactory != nullptr)
                            {
                                DXGI_SWAP_CHAIN_DESC1 SwapChainDesc = {};
                                SwapChainDesc.BufferCount = 2;
                                SwapChainDesc.Width = 1;
                                SwapChainDesc.Height = 1;
                                SwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                                SwapChainDesc.Stereo = FALSE;
                                SwapChainDesc.SampleDesc = { 1, 0 };
                                SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                                SwapChainDesc.Scaling = DXGI_SCALING_NONE;
                                SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
                                SwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

                                pDXGIFactory->CreateSwapChainForHwnd(pDevice, dummyWindow, &SwapChainDesc, NULL, NULL, reinterpret_cast<IDXGISwapChain1**>(&pSwapChain));
                            }
                        }
                    }

                    if (pDXGIFactory) pDXGIFactory->Release();
                }
            }

            if (pDevice != nullptr && pSwapChain != nullptr)
            {
                version = 1;
            }
            else
            {
                SPDLOG_WARN("Failed to instanciate IDXGISwapChain1, fallback to pure DX10 detection");

                auto D3D10CreateDeviceAndSwapChain = libD3d10.GetSymbol<decltype(::D3D10CreateDeviceAndSwapChain)>("D3D10CreateDeviceAndSwapChain");

                if (D3D10CreateDeviceAndSwapChain != nullptr)
                {
                    DXGI_SWAP_CHAIN_DESC SwapChainDesc = {};

                    SwapChainDesc.BufferCount = 1;
                    SwapChainDesc.BufferDesc.Width = 1;
                    SwapChainDesc.BufferDesc.Height = 1;
                    SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                    SwapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
                    SwapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
                    SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                    SwapChainDesc.OutputWindow = dummyWindow;
                    SwapChainDesc.SampleDesc.Count = 1;
                    SwapChainDesc.SampleDesc.Quality = 0;
                    SwapChainDesc.Windowed = TRUE;

                    D3D10CreateDeviceAndSwapChain(NULL, D3D10_DRIVER_TYPE_NULL, NULL, 0, D3D10_SDK_VERSION, &SwapChainDesc, &pSwapChain, &pDevice);
                }
            }

            if (pSwapChain != nullptr)
            {
                SPDLOG_INFO("Hooked IDXGISwapChain::Present to detect DX Version");

                dx10_hooked = true;

                decltype(&IDXGISwapChain::Present) pfnPresent;
                decltype(&IDXGISwapChain::ResizeBuffers) pfnResizeBuffers;
                decltype(&IDXGISwapChain::ResizeTarget) pfnResizeTarget;
                decltype(&IDXGISwapChain1::Present1) pfnPresent1 = nullptr;

                HookDXGIPresent(pSwapChain, pfnPresent, pfnResizeBuffers, pfnResizeTarget);
                if (version > 0)
                {
                    HookDXGIPresent1(reinterpret_cast<IDXGISwapChain1*>(pSwapChain), pfnPresent1);
                }

                dx10_hook = DX10_Hook::Inst();
                dx10_hook->LibraryName = library_path;
                dx10_hook->LoadFunctions(pfnPresent, pfnResizeBuffers, pfnResizeTarget, pfnPresent1);
            }
            else
            {
                SPDLOG_WARN("Failed to Hook IDXGISwapChain::Present to detect DX Version");
            }
            if (pDevice)pDevice->Release();
            if (pSwapChain)pSwapChain->Release();
        }
    }

    void hook_dx11(std::string const& library_path)
    {
        if (!dx11_hooked)
        {
            System::Library::Library libD3d11;
            if (!libD3d11.OpenLibrary(library_path, false))
            {
                SPDLOG_WARN("Failed to load {} to detect DX11", library_path);
                return;
            }
            std::string dxgi_path = FindPreferedModulePath("dxgi.dll");

            IDXGISwapChain* pSwapChain = nullptr;
            ID3D11Device* pDevice = nullptr;
            int version = 0;

            if (!dxgi_path.empty())
            {
                HMODULE dxgi = GetModuleHandleA(dxgi_path.c_str());
                if (dxgi != nullptr)
                {
                    IDXGIFactory2* pDXGIFactory = nullptr;

                    auto D3D11CreateDevice = libD3d11.GetSymbol<decltype(::D3D11CreateDevice)>("D3D11CreateDevice");
                    decltype(CreateDXGIFactory1)* CreateDXGIFactory1 = (decltype(CreateDXGIFactory1))GetProcAddress(dxgi, "CreateDXGIFactory1");

                    if (D3D11CreateDevice != nullptr && CreateDXGIFactory1 != nullptr)
                    {
                        D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_NULL, NULL, 0, NULL, 0, D3D11_SDK_VERSION, &pDevice, NULL, NULL);
                        if (pDevice != nullptr)
                        {
                            CreateDXGIFactory1(IID_PPV_ARGS(&pDXGIFactory));
                            if (pDXGIFactory != nullptr)
                            {
                                DXGI_SWAP_CHAIN_DESC1 SwapChainDesc = {};
                                SwapChainDesc.BufferCount = 2;
                                SwapChainDesc.Width = 1;
                                SwapChainDesc.Height = 1;
                                SwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                                SwapChainDesc.Stereo = FALSE;
                                SwapChainDesc.SampleDesc = { 1, 0 };
                                SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                                SwapChainDesc.Scaling = DXGI_SCALING_NONE;
                                SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
                                SwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

                                pDXGIFactory->CreateSwapChainForHwnd(pDevice, dummyWindow, &SwapChainDesc, NULL, NULL, reinterpret_cast<IDXGISwapChain1**>(&pSwapChain));
                            }
                        }
                    }

                    if (pDXGIFactory) pDXGIFactory->Release();
                }
            }

            if (pDevice != nullptr && pSwapChain != nullptr)
            {
                version = 1;
            }
            else
            {
                SPDLOG_WARN("Failed to instanciate IDXGISwapChain1, fallback to pure DX11 detection");

                auto D3D11CreateDeviceAndSwapChain = libD3d11.GetSymbol<decltype(::D3D11CreateDeviceAndSwapChain)>("D3D11CreateDeviceAndSwapChain");

                if (D3D11CreateDeviceAndSwapChain != nullptr)
                {
                    DXGI_SWAP_CHAIN_DESC SwapChainDesc = {};

                    SwapChainDesc.BufferCount = 1;
                    SwapChainDesc.BufferDesc.Width = 1;
                    SwapChainDesc.BufferDesc.Height = 1;
                    SwapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                    SwapChainDesc.BufferDesc.RefreshRate.Numerator = 0;
                    SwapChainDesc.BufferDesc.RefreshRate.Denominator = 0;
                    SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                    SwapChainDesc.OutputWindow = dummyWindow;
                    SwapChainDesc.SampleDesc.Count = 1;
                    SwapChainDesc.SampleDesc.Quality = 0;
                    SwapChainDesc.Windowed = TRUE;

                    D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_NULL, NULL, 0, NULL, NULL, D3D11_SDK_VERSION, &SwapChainDesc, &pSwapChain, &pDevice, NULL, NULL);
                }
            }

            if (pSwapChain != nullptr)
            {
                SPDLOG_INFO("Hooked IDXGISwapChain::Present to detect DX Version");

                dx11_hooked = true;

                decltype(&IDXGISwapChain::Present) pfnPresent = nullptr;
                decltype(&IDXGISwapChain::ResizeBuffers) pfnResizeBuffers = nullptr;
                decltype(&IDXGISwapChain::ResizeTarget) pfnResizeTarget = nullptr;
                decltype(&IDXGISwapChain1::Present1) pfnPresent1 = nullptr;

                HookDXGIPresent(pSwapChain, pfnPresent, pfnResizeBuffers, pfnResizeTarget);
                if (version > 0)
                {
                    HookDXGIPresent1(reinterpret_cast<IDXGISwapChain1*>(pSwapChain), pfnPresent1);
                }

                dx11_hook = DX11_Hook::Inst();
                dx11_hook->LibraryName = library_path;
                dx11_hook->LoadFunctions(pfnPresent, pfnResizeBuffers, pfnResizeTarget, pfnPresent1);
            }
            else
            {
                SPDLOG_WARN("Failed to Hook IDXGISwapChain::Present to detect DX Version");
            }

            if (pDevice) pDevice->Release();
            if (pSwapChain) pSwapChain->Release();
        }
    }

    void hook_dx12(std::string const& library_path)
    {
        if (!dx12_hooked)
        {
            System::Library::Library libD3d12;
            if (!libD3d12.OpenLibrary(library_path, false))
            {
                SPDLOG_WARN("Failed to load {} to detect DX12", library_path);
                return;
            }
            std::string dxgi_path = FindPreferedModulePath("dxgi.dll");
            if (dxgi_path.empty())
            {
                SPDLOG_WARN("Failed to load dxgi.dll to detect DX12");
                return;
            }

            IDXGIFactory4* pDXGIFactory = nullptr;
            IDXGISwapChain1* pSwapChain = nullptr;
            ID3D12CommandQueue* pCommandQueue = nullptr;
            ID3D12Device* pDevice = nullptr;

            auto D3D12CreateDevice = libD3d12.GetSymbol<decltype(::D3D12CreateDevice)>("D3D12CreateDevice");

            if (D3D12CreateDevice != nullptr)
            {
                D3D12CreateDevice(NULL, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&pDevice));

                if (pDevice != nullptr)
                {
                    DXGI_SWAP_CHAIN_DESC1 SwapChainDesc = {};
                    SwapChainDesc.BufferCount = 2;
                    SwapChainDesc.Width = 1;
                    SwapChainDesc.Height = 1;
                    SwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
                    SwapChainDesc.Stereo = FALSE;
                    SwapChainDesc.SampleDesc = { 1, 0 };
                    SwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
                    SwapChainDesc.Scaling = DXGI_SCALING_NONE;
                    SwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
                    SwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;

                    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
                    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
                    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
                    pDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&pCommandQueue));

                    if (pCommandQueue != nullptr)
                    {
                        HMODULE dxgi = GetModuleHandleA(dxgi_path.c_str());
                        if (dxgi != nullptr)
                        {
                            decltype(CreateDXGIFactory1)* CreateDXGIFactory1 = (decltype(CreateDXGIFactory1))GetProcAddress(dxgi, "CreateDXGIFactory1");
                            if (CreateDXGIFactory1 != nullptr)
                            {
                                CreateDXGIFactory1(IID_PPV_ARGS(&pDXGIFactory));
                                if (pDXGIFactory != nullptr)
                                {
                                    pDXGIFactory->CreateSwapChainForHwnd(pCommandQueue, dummyWindow, &SwapChainDesc, NULL, NULL, &pSwapChain);
                                }
                            }
                        }
                    }
                }//if (pDevice != nullptr)
            }//if (D3D12CreateDevice != nullptr)

            if (pCommandQueue != nullptr && pSwapChain != nullptr)
            {
                SPDLOG_INFO("Hooked IDXGISwapChain::Present to detect DX Version");

                dx12_hooked = true;

                decltype(&IDXGISwapChain::Present) pfnPresent = nullptr;
                decltype(&IDXGISwapChain::ResizeBuffers) pfnResizeBuffers = nullptr;
                decltype(&IDXGISwapChain::ResizeTarget) pfnResizeTarget = nullptr;
                decltype(&IDXGISwapChain1::Present1) pfnPresent1 = nullptr;

                HookDXGIPresent(pSwapChain, pfnPresent, pfnResizeBuffers, pfnResizeTarget);
                HookDXGIPresent1(reinterpret_cast<IDXGISwapChain1*>(pSwapChain), pfnPresent1);

                void** vTable = *reinterpret_cast<void***>(pCommandQueue);
                decltype(&ID3D12CommandQueue::ExecuteCommandLists) pfnExecuteCommandLists;
                (void*&)pfnExecuteCommandLists = vTable[(int)ID3D12CommandQueueVTable::ExecuteCommandLists];

                dx12_hook = DX12_Hook::Inst();
                dx12_hook->LibraryName = library_path;
                dx12_hook->LoadFunctions(pfnPresent, pfnResizeBuffers, pfnResizeTarget, pfnExecuteCommandLists, pfnPresent1);
            }
            else
            {
                SPDLOG_WARN("Failed to Hook IDXGISwapChain::Present to detect DX Version");
            }

            if (pSwapChain) pSwapChain->Release();
            if (pDXGIFactory) pDXGIFactory->Release();
            if (pCommandQueue) pCommandQueue->Release();
            if (pDevice) pDevice->Release();
        }
    }

    void hook_opengl(std::string const& library_path)
    {
        if (!opengl_hooked)
        {
            System::Library::Library libOpenGL;
            if (!libOpenGL.OpenLibrary(library_path, false))
            {
                SPDLOG_WARN("Failed to load {} to detect OpenGL", library_path);
                return;
            }

            auto wglSwapBuffers = libOpenGL.GetSymbol<decltype(::SwapBuffers)>("wglSwapBuffers");
            if (wglSwapBuffers != nullptr)
            {
                SPDLOG_INFO("Hooked wglSwapBuffers to detect OpenGL");

                opengl_hooked = true;

                opengl_hook = OpenGL_Hook::Inst();
                opengl_hook->LibraryName = library_path;
                opengl_hook->LoadFunctions(wglSwapBuffers);

                HookwglSwapBuffers(wglSwapBuffers);
            }
            else
            {
                SPDLOG_WARN("Failed to Hook wglSwapBuffers to detect OpenGL");
            }
        }
    }

    void hook_vulkan(std::string const& library_path)
    {
        // Vulkan hook disabled until proper implementation.
        return;
        if (!vulkan_hooked)
        {
            System::Library::Library libVulkan;
            if (!libVulkan.OpenLibrary(library_path, false))
            {
                SPDLOG_WARN("Failed to load {} to detect Vulkan", library_path);
                return;
            }

            auto vkCreateInstance = libVulkan.GetSymbol<decltype(::vkCreateInstance)>("vkCreateInstance");
            auto vkDestroyInstance = libVulkan.GetSymbol<decltype(::vkDestroyInstance)>("vkDestroyInstance");
            auto vkGetInstanceProcAddr = libVulkan.GetSymbol<decltype(::vkGetInstanceProcAddr)>("vkGetInstanceProcAddr");

            decltype(::vkQueuePresentKHR)* vkQueuePresentKHR = nullptr;
            decltype(::vkAcquireNextImageKHR)* vkAcquireNextImageKHR = nullptr;
            decltype(::vkAcquireNextImage2KHR)* vkAcquireNextImage2KHR = nullptr;

            VkInstanceCreateInfo instance_infos{};
            VkInstance instance{};
            std::vector<VkPhysicalDevice> phyDevices;
            VkDeviceCreateInfo create_info{};
            VkDevice pDevice{};
            uint32_t count = 0;

            instance_infos.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
            vkCreateInstance(&instance_infos, nullptr, &instance);

            auto vkCreateDevice = (decltype(::vkCreateDevice)*)vkGetInstanceProcAddr(instance, "vkCreateDevice");
            auto vkDestroyDevice = (decltype(::vkDestroyDevice)*)vkGetInstanceProcAddr(instance, "vkDestroyDevice");
            auto vkGetDeviceProcAddr = (decltype(::vkGetDeviceProcAddr)*)vkGetInstanceProcAddr(instance, "vkGetDeviceProcAddr");
            auto vkEnumeratePhysicalDevices = (decltype(::vkEnumeratePhysicalDevices)*)vkGetInstanceProcAddr(instance, "vkEnumeratePhysicalDevices");
            auto vkEnumerateDeviceExtensionProperties = (decltype(::vkEnumerateDeviceExtensionProperties)*)vkGetInstanceProcAddr(instance, "vkEnumerateDeviceExtensionProperties");
            auto vkGetPhysicalDeviceProperties = (decltype(::vkGetPhysicalDeviceProperties)*)vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties");

            vkEnumeratePhysicalDevices(instance, &count, nullptr);
            phyDevices.resize(count);
            vkEnumeratePhysicalDevices(instance, &count, phyDevices.data());

            [&]()
            {// Lambda for nested for break.
                VkPhysicalDeviceProperties props{};
                std::vector<VkExtensionProperties> ext_props;

                for (auto& device : phyDevices)
                {
                    vkGetPhysicalDeviceProperties(device, &props);
                    if (props.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU || props.deviceType == VkPhysicalDeviceType::VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU)
                    {
                        count = 0;
                        vkEnumerateDeviceExtensionProperties(device, nullptr, &count, nullptr);
                        ext_props.resize(count);
                        vkEnumerateDeviceExtensionProperties(device, nullptr, &count, ext_props.data());

                        for (auto& ext : ext_props)
                        {
                            if (strcmp(ext.extensionName, "VK_KHR_swapchain") == 0)
                            {
                                create_info.sType = VkStructureType::VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
                                create_info.enabledExtensionCount = 1;
                                const char* str = "VK_KHR_swapchain";
                                create_info.ppEnabledExtensionNames = &str;
                                vkCreateDevice(device, &create_info, nullptr, &pDevice);
                                if (pDevice != nullptr)
                                    return;
                            }
                        }
                    }
                }
            }();

            if (pDevice != nullptr)
            {
                vkQueuePresentKHR = (decltype(::vkQueuePresentKHR)*)vkGetDeviceProcAddr(pDevice, "vkQueuePresentKHR");
                vkAcquireNextImageKHR = (decltype(::vkAcquireNextImageKHR)*)vkGetDeviceProcAddr(pDevice, "vkAcquireNextImageKHR");
                vkAcquireNextImage2KHR = (decltype(::vkAcquireNextImage2KHR)*)vkGetDeviceProcAddr(pDevice, "vkAcquireNextImage2KHR");
                vkDestroyDevice(pDevice, nullptr);
            }
            vkDestroyInstance(instance, nullptr);

            if (vkQueuePresentKHR != nullptr /* && (vkAcquireNextImageKHR != nullptr || vkAcquireNextImage2KHR != nullptr)*/)
            {
                SPDLOG_INFO("Hooked vkQueuePresentKHR to detect Vulkan");

                vulkan_hooked = true;

                vulkan_hook = Vulkan_Hook::Inst();
                vulkan_hook->LibraryName = library_path;
                vulkan_hook->LoadFunctions(vkQueuePresentKHR);

                HookvkQueuePresentKHR(vkQueuePresentKHR);
            }
            else
            {
                SPDLOG_WARN("Failed to Hook vkQueuePresentKHR to detect Vulkan");
            }
        }
    }

    bool EnterDetection()
    {
        return CreateHWND() != nullptr;
    }

    void ExitDetection()
    {
        DestroyHWND();

        detection_done = true;
        detection_hooks.UnhookAll();

        dxgi_hooked    = false;
        dxgi1_2_hooked = false;
        dx12_hooked    = false;
        dx11_hooked    = false;
        dx10_hooked    = false;
        dx9_hooked     = false;
        opengl_hooked  = false;
        vulkan_hooked  = false;

        delete dx9_hook   ; dx9_hook    = nullptr;
        delete dx10_hook  ; dx10_hook   = nullptr;
        delete dx11_hook  ; dx11_hook   = nullptr;
        delete dx12_hook  ; dx12_hook   = nullptr;
        delete opengl_hook; opengl_hook = nullptr;
        delete vulkan_hook; vulkan_hook = nullptr;
    }

#elif defined(RENDERERDETECTOR_OS_LINUX)
#define RENDERER_HOOKS { OpenGLX_Hook::DLL_NAME, &Renderer_Detector::hook_openglx },

    ~Renderer_Detector()
    {
        stop_detection();

        delete openglx_hook;
        //delete vulkan_hook;

        instance = nullptr;
    }

private:
    decltype(::glXSwapBuffers)* glXSwapBuffers;

    bool openglx_hooked;
    //bool vulkan_hooked;

    OpenGLX_Hook* openglx_hook;
    //Vulkan_Hook* vulkan_hook;

    Renderer_Detector() :
        openglx_hooked(false),
        renderer_hook(nullptr),
        openglx_hook(nullptr),
        //vulkan_hook(nullptr),
        detection_done(false),
        detection_count(0),
        detection_cancelled(false)
    {}

    std::string FindPreferedModulePath(std::string const& name)
    {
        return name;
    }

    static void MyglXSwapBuffers(Display* dpy, GLXDrawable drawable)
    {
        auto inst = Inst();
        std::lock_guard<std::mutex> lk(inst->renderer_mutex);
        inst->glXSwapBuffers(dpy, drawable);
        if (inst->detection_done)
            return;

        if (gladLoaderLoadGL() >= GLAD_MAKE_VERSION(3, 1))
        {
            inst->detection_hooks.UnhookAll();
            inst->renderer_hook = static_cast<ingame_overlay::Renderer_Hook*>(Inst()->openglx_hook);
            inst->openglx_hook = nullptr;
            inst->detection_done = true;
        }
    }

    void HookglXSwapBuffers(decltype(::glXSwapBuffers)* _glXSwapBuffers)
    {
        glXSwapBuffers = _glXSwapBuffers;

        detection_hooks.BeginHook();
        detection_hooks.HookFunc(std::pair<void**, void*>{ (void**)&glXSwapBuffers, (void*)&MyglXSwapBuffers });
        detection_hooks.EndHook();
    }

    void hook_openglx(std::string const& library_path)
    {
        if (!openglx_hooked)
        {
            System::Library::Library libGLX;
            if (!libGLX.OpenLibrary(library_path, false))
            {
                SPDLOG_WARN("Failed to load {} to detect OpenGLX", library_path);
                return;
            }

            auto glXSwapBuffers = libGLX.GetSymbol<decltype(::glXSwapBuffers)>("glXSwapBuffers");
            if (glXSwapBuffers != nullptr)
            {
                SPDLOG_INFO("Hooked glXSwapBuffers to detect OpenGLX");

                openglx_hooked = true;

                openglx_hook = OpenGLX_Hook::Inst();
                openglx_hook->LibraryName = library_path;
                openglx_hook->LoadFunctions(glXSwapBuffers);

                HookglXSwapBuffers(glXSwapBuffers);
            }
            else
            {
                SPDLOG_WARN("Failed to Hook glXSwapBuffers to detect OpenGLX");
            }
        }
    }

    bool EnterDetection()
    {
        return true;
    }

    void ExitDetection()
    {
        detection_done = true;
        detection_hooks.UnhookAll();

        openglx_hooked = false;
        //vulkan_hooked = false;

        delete openglx_hook; openglx_hook = nullptr;
        //delete vulkan_hook; vulkan_hook = nullptr;
    }

#elif defined(RENDERERDETECTOR_OS_APPLE)
#define RENDERER_HOOKS { OpenGL_Hook::DLL_NAME, & Renderer_Detector::hook_opengl },

    ~Renderer_Detector()
    {
        stop_detection();

        delete opengl_hook;

        instance = nullptr;
    }

private:
    decltype(::CGLFlushDrawable)* CGLFlushDrawable;

    bool opengl_hooked;
    //bool metal_hooked;

    OpenGL_Hook* opengl_hook;
    //Metal_Hook* metal_hook;

    Renderer_Detector() :
        opengl_hooked(false),
        renderer_hook(nullptr),
        opengl_hook(nullptr),
        detection_done(false),
        detection_count(0),
        detection_cancelled(false)
    {}

    std::string FindPreferedModulePath(std::string const& name)
    {
        return name;
    }

    static int64_t MyCGLFlushDrawable(CGLDrawable_t* glDrawable)
    {
        auto inst = Inst();
        std::lock_guard<std::mutex> lk(inst->renderer_mutex);
        int64_t res = inst->CGLFlushDrawable(glDrawable);

        if (gladLoaderLoadGL() >= GLAD_MAKE_VERSION(2, 0))
        {
            inst->detection_hooks.UnhookAll();
            inst->renderer_hook = static_cast<ingame_overlay::Renderer_Hook*>(Inst()->opengl_hook);
            inst->opengl_hook = nullptr;
            inst->detection_done = true;
        }

        return res;
    }

    void HookglFlushDrawable(decltype(::CGLFlushDrawable)* _CGLFlushDrawable)
    {
        CGLFlushDrawable = _CGLFlushDrawable;

        detection_hooks.BeginHook();
        detection_hooks.HookFunc(std::pair<void**, void*>{ (void**)&CGLFlushDrawable, (void*)&MyCGLFlushDrawable });
        detection_hooks.EndHook();
    }

    void hook_opengl(std::string const& library_path)
    {
        if (!opengl_hooked)
        {
            System::Library::Library libOpenGL;
            if (!libOpenGL.OpenLibrary(library_path, false))
            {
                SPDLOG_WARN("Failed to load {} to detect OpenGL", library_path);
                return;
            }

            auto CGLFlushDrawable = libOpenGL.GetSymbol<decltype(::CGLFlushDrawable)>("CGLFlushDrawable");
            if (CGLFlushDrawable != nullptr)
            {
                SPDLOG_INFO("Hooked CGLFlushDrawable to detect OpenGL");

                opengl_hooked = true;

                opengl_hook = OpenGL_Hook::Inst();
                opengl_hook->LibraryName = library_path;
                opengl_hook->LoadFunctions(CGLFlushDrawable);

                HookglFlushDrawable(CGLFlushDrawable);
            }
            else
            {
                SPDLOG_WARN("Failed to Hook CGLFlushDrawable to detect OpenGL");
            }
        }
    }

    bool EnterDetection()
    {
        return true;
    }

    void ExitDetection()
    {
        detection_done = true;
        detection_hooks.UnhookAll();

        opengl_hooked = false;
        //metal_hooked = false;

        delete opengl_hook; opengl_hook = nullptr;
        //delete metal_hook; metal_hook = nullptr;
    }

#endif

private:
    std::timed_mutex detector_mutex;
    std::mutex renderer_mutex;

    Base_Hook detection_hooks;
    ingame_overlay::Renderer_Hook* renderer_hook;
    
    bool detection_done;
    uint32_t detection_count;
    bool detection_cancelled;
    std::condition_variable stop_detection_cv;
    std::mutex stop_detection_mutex;

public:
    std::future<ingame_overlay::Renderer_Hook*> detect_renderer(std::chrono::milliseconds timeout)
    {
        std::lock_guard<std::mutex> lk(stop_detection_mutex);

        if (detection_count == 0)
        {// If we have no detections in progress, restart detection.
            detection_cancelled = false;
        }

        ++detection_count;

        return std::async(std::launch::async, [this, timeout]() -> ingame_overlay::Renderer_Hook*
        {
            std::unique_lock<std::timed_mutex> detection_lock(detector_mutex, std::defer_lock);
            constexpr std::chrono::milliseconds infinite_timeout{ -1 };
        
            if (!detection_lock.try_lock_for(timeout))
                return nullptr;

            bool cancel = false;
            {
                System::scoped_lock lk(renderer_mutex, stop_detection_mutex);

                if (!detection_cancelled)
                {
                    if (detection_done)
                    {
                        if (renderer_hook == nullptr)
                        {// Renderer detection was run but we didn't find it, restart the detection
                            detection_done = false;
                        }
                        else
                        {// Renderer already detected, cancel detection and return the renderer.
                            cancel = true;
                        }
                    }

                    if (!EnterDetection())
                        cancel = true;
                }
                else
                {// Detection was cancelled, cancel this detection
                    cancel = true;
                }
            }

            if (cancel)
            {
                --detection_count;
                stop_detection_cv.notify_all();
                return renderer_hook;
            }

            SPDLOG_TRACE("Started renderer detection.");

            std::pair<std::string, void(Renderer_Detector::*)(std::string const&)> libraries[]{
                RENDERER_HOOKS
            };
            std::string name;

            auto start_time = std::chrono::steady_clock::now();
            do
            {
                std::unique_lock<std::mutex> lck(stop_detection_mutex);
                if (detection_cancelled || detection_done)
                    break;

                for (auto const& library : libraries)
                {
                    std::string lib_path = FindPreferedModulePath(library.first);
                    if (!lib_path.empty())
                    {
                        void* lib_handle = System::Library::GetLibraryHandle(lib_path.c_str());
                        if (lib_handle != nullptr)
                        {
                            std::lock_guard<std::mutex> lk(renderer_mutex);
                            (this->*library.second)(System::Library::GetLibraryPath(lib_handle));
                        }
                    }
                }

                stop_detection_cv.wait_for(lck, std::chrono::milliseconds{ 100 });
            } while (timeout == infinite_timeout || (std::chrono::steady_clock::now() - start_time) <= timeout);

            {
                System::scoped_lock lk(renderer_mutex, stop_detection_mutex);
                
                ExitDetection();

                --detection_count;
            }
            stop_detection_cv.notify_all();

            SPDLOG_TRACE("Renderer detection done {}.", (void*)renderer_hook);

            return renderer_hook;
        });
    }

    void stop_detection()
    {
        {
            std::lock_guard<std::mutex> lk(stop_detection_mutex);
            if (detection_count == 0)
                return;
        }
        {
            System::scoped_lock lk(renderer_mutex, stop_detection_mutex);
            detection_cancelled = true;
        }
        stop_detection_cv.notify_all();
        {
            std::unique_lock<std::mutex> lk(stop_detection_mutex);
            stop_detection_cv.wait(lk, [&]() { return detection_count == 0; });
        }
    }
};

Renderer_Detector* Renderer_Detector::instance = nullptr;

namespace ingame_overlay {

std::future<ingame_overlay::Renderer_Hook*> DetectRenderer(std::chrono::milliseconds timeout)
{
    return Renderer_Detector::Inst()->detect_renderer(timeout);
}

void StopRendererDetection()
{
    Renderer_Detector::Inst()->stop_detection();
}

void FreeDetector()
{
    delete Renderer_Detector::Inst();
}

}