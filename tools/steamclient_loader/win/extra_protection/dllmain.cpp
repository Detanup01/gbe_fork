#include "pe_helpers/pe_helpers.hpp"
#include "extra_protection/stubdrm_v3.hpp"


BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD  reason,
    LPVOID lpReserved)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        stubdrm_v3::patch();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        stubdrm_v3::restore();
        break;
    }
    return TRUE;
}
