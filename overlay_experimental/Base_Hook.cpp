#include "overlay/Base_Hook.h"

#include <algorithm>

#ifdef EMU_OVERLAY
#ifdef STEAM_WIN32

#include "detours/detours.h"

Base_Hook::Base_Hook():
    _library(nullptr)
{}

Base_Hook::~Base_Hook()
{
    UnhookAll();
}

const char* Base_Hook::get_lib_name() const
{
    return "<no_name>";
}

void Base_Hook::BeginHook()
{
    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
}

void Base_Hook::EndHook()
{
    DetourTransactionCommit();
}

void Base_Hook::HookFunc(std::pair<void**, void*> hook)
{
    if( DetourAttach(hook.first, hook.second) == 0 )
        _hooked_funcs.emplace_back(hook);
}

void Base_Hook::UnhookAll()
{
    if (_hooked_funcs.size())
    {
        BeginHook();
        std::for_each(_hooked_funcs.begin(), _hooked_funcs.end(), [](std::pair<void**, void*>& hook) {
            DetourDetach(hook.first, hook.second);
            });
        EndHook();
        _hooked_funcs.clear();
    }
}

#else

Base_Hook::Base_Hook():
    _library(nullptr)
{}

Base_Hook::~Base_Hook()
{
}

const char* Base_Hook::get_lib_name() const
{
    return "<no_name>";
}

void Base_Hook::BeginHook()
{
}

void Base_Hook::EndHook()
{
}

void Base_Hook::HookFunc(std::pair<void**, void*> hook)
{
}

void Base_Hook::UnhookAll()
{
}

#endif

#endif//EMU_OVERLAY
