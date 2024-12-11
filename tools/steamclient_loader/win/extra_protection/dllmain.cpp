#include "extra_protection/stubdrm.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <condition_variable>
#include <mutex>
#include <chrono>
#include <thread>
#include <string>

#if defined(DEBUG) || defined(_DEBUG)
    #define STUB_EXTRA_DEBUG
#endif

static std::mutex dll_unload_mtx{};
static std::condition_variable dll_unload_cv{};
static bool unload_dll = false;

static HMODULE my_hModule = nullptr;
static HANDLE unload_thread_handle = INVALID_HANDLE_VALUE;


static void send_unload_signal()
{
    {
        std::lock_guard lock(dll_unload_mtx);
        unload_dll = true;
    }
    dll_unload_cv.notify_one();
}

DWORD WINAPI self_unload(LPVOID lpParameter)
{
    constexpr const auto UNLOAD_TIMEOUT =
#ifdef STUB_EXTRA_DEBUG
        std::chrono::minutes(5)
#else
        std::chrono::seconds(5)
#endif
    ;

    {
#ifdef STUB_EXTRA_DEBUG
        auto t1 = std::chrono::high_resolution_clock::now();
#endif

        std::unique_lock lock(dll_unload_mtx);
        dll_unload_cv.wait_for(lock, UNLOAD_TIMEOUT, []{ return unload_dll; });
        
#ifdef STUB_EXTRA_DEBUG
        if (!unload_dll) { // flag was not raised, means we timed out
            auto t2 = std::chrono::high_resolution_clock::now();
            auto dd = std::chrono::duration_cast<std::chrono::seconds>(t2 - t1);
            std::string msg = "Unloading after " + std::to_string(dd.count()) + " seconds, due to timeout";
            MessageBoxA(nullptr, msg.c_str(), "Self-unload thread", MB_OK | MB_ICONERROR);
        }
#endif
    }
    unload_thread_handle = INVALID_HANDLE_VALUE;
    FreeLibraryAndExitThread(my_hModule, 0);
}

BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD  reason,
    LPVOID lpReserved)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        stubdrm::set_cleanup_cb(send_unload_signal);
        my_hModule = hModule;
        if (!stubdrm::patch()) {
#ifdef STUB_EXTRA_DEBUG
            MessageBoxA(nullptr, "Failed to detect .bind", "Main", MB_OK | MB_ICONERROR);
#endif

            // https://learn.microsoft.com/en-us/windows/win32/dlls/dllmain
            // "The system immediately calls your entry-point function with DLL_PROCESS_DETACH and unloads the DLL"
            unload_dll = true;
            return FALSE;
        }
        unload_thread_handle = CreateThread(nullptr, 0, self_unload, nullptr, 0, nullptr);
        break;
        
    case DLL_PROCESS_DETACH:
        if (!unload_dll) { // not unloaded yet, just an early exit, or thread timed out
#ifdef STUB_EXTRA_DEBUG
            MessageBoxA(nullptr, "Unclean exit", "Main", MB_OK | MB_ICONERROR);
#endif

            stubdrm::restore();
            if (unload_thread_handle != INVALID_HANDLE_VALUE && unload_thread_handle != NULL) {
                TerminateThread(unload_thread_handle, 0);
            }
        }
        break;
    }

    return TRUE;
}
