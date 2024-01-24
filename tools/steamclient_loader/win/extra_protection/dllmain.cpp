#include "pe_helpers/pe_helpers.hpp"
#include "extra_protection/stubdrm.hpp"

BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD  reason,
    LPVOID lpReserved)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        stubdrm::patch();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;
    case DLL_PROCESS_DETACH:
        stubdrm::restore();
        break;
    }
    return TRUE;
}
