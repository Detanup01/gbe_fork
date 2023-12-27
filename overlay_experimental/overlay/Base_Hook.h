#ifndef __INCLUDED_BASE_HOOK_H__
#define __INCLUDED_BASE_HOOK_H__

#include "dll/base.h"

#ifdef EMU_OVERLAY

class Base_Hook
{
protected:
    std::vector<std::pair<void**, void*>> _hooked_funcs;

    void* _library;

    Base_Hook(const Base_Hook&) = delete;
    Base_Hook(Base_Hook&&) = delete;
    Base_Hook& operator =(const Base_Hook&) = delete;
    Base_Hook& operator =(Base_Hook&&) = delete;

public:
    Base_Hook();
    virtual ~Base_Hook();

    void BeginHook();
    void EndHook();
    void UnhookAll();

    virtual const char* get_lib_name() const;
    void HookFunc(std::pair<void**, void*> hook);

    template<typename T>
    void HookFuncs(std::pair<T*, T> funcs)
    {
        HookFunc(funcs);
    }

    template<typename T, typename ...Args>
    void HookFuncs(std::pair<T*, T> funcs, Args... args)
    {
        HookFunc(funcs);
        HookFuncs(args...);
    }
};

#endif//EMU_OVERLAY

#endif//__INCLUDED_BASE_HOOK_H__