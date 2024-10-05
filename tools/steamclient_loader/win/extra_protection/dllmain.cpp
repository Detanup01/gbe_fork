#include "extra_protection/stubdrm.hpp"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <condition_variable>
#include <mutex>
#include <chrono>
#include <thread>


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
#ifdef _DEBUG
        std::chrono::minutes(5)
#else
        std::chrono::seconds(5)
#endif
    ;

    {
        std::unique_lock lock(dll_unload_mtx);
        dll_unload_cv.wait_for(lock, UNLOAD_TIMEOUT, [](){ return unload_dll; });
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
        if (!stubdrm::patch()) {
            // https://learn.microsoft.com/en-us/windows/win32/dlls/dllmain
            // "The system immediately calls your entry-point function with DLL_PROCESS_DETACH and unloads the DLL"
            unload_dll = true;
            return FALSE;
        }
        my_hModule = hModule;
        stubdrm::set_cleanup_cb(send_unload_signal);
        unload_thread_handle = CreateThread(nullptr, 0, self_unload, nullptr, 0, nullptr);
        break;
        
    case DLL_PROCESS_DETACH:
        if (!unload_dll) { // not unloaded yet, just an early exit, or thread timed out
            stubdrm::restore();
            if (unload_thread_handle != INVALID_HANDLE_VALUE && unload_thread_handle != NULL) {
                TerminateThread(unload_thread_handle, 0);
            }
        }
        break;
    }

    return TRUE;
}
