#include "pe_helpers/pe_helpers.hpp"
#include "common_helpers/common_helpers.hpp"
#include "extra_protection/stubdrm.hpp"
#include "detours/detours.h"
#include <tuple>
#include <mutex>
#include <intrin.h>

typedef struct SnrUnit {
    std::string search_patt{};
    std::string replace_patt{};
} SnrUnit_t;

typedef struct SnrDetails {
    std::string detection_patt{};
    std::vector<SnrUnit_t> snr_units{};
} SnrDetails_t;

// x64
#if defined(_WIN64)
    const std::vector<SnrDetails> snr_patts {
        {
            // detection_patt
            "FF 94 24 ?? ?? ?? ?? 88 44 24 ?? 0F BE 44 24 ?? 83 ?? 30 74 ?? E9",
            // snr_units
            {
                // patt 1 is a bunch of checks for registry + files validity (including custom DOS stub)
                // patt 2 is again a bunch of checks + creates some interfaces via steamclient + calls getappownershipticket()
                {
                    "E8 ?? ?? ?? ?? 84 C0 75 ?? B0 33 E9",
                    "B8 01 00 00 00 ?? ?? EB",
                },
                {
                    "E8 ?? ?? ?? ?? 44 0F B6 ?? 3C 30 0F 84 ?? ?? ?? ?? 3C 35 0F 85",
                    "B8 30 00 00 00 ?? ?? ?? ?? ?? ?? 90 E9",
                },
            },
        },

        {
            // detection_patt
            "FF D? 44 0F B6 ?? 3C 30 0F 85",
            // snr_units
            {
                {
                    "E8 ?? ?? ?? ?? 44 0F B6 ?? 3C 30 0F 84 ?? ?? ?? ?? 3C ?? 0F 85",
                    "B8 30 00 00 00 ?? ?? ?? ?? ?? ?? 90 E9 ?? ?? ?? ?? ?? ?? ?? ??",
                },
            },
        },
    };

#endif

// x32
#if !defined(_WIN64)

    const std::vector<SnrDetails> snr_patts {
        {
            // detection_patt
            "FF 95 ?? ?? ?? ?? 88 45 ?? 0F BE 4D ?? 83 ?? 30 74 ?? E9",
            // snr_units
            {
                // patt 1 is a bunch of checks for registry + files validity (including custom DOS stub)
                // patt 2 is again a bunch of checks + creates some interfaces via steamclient + calls getappownershipticket()
                {
                    "5? 5? E8 ?? ?? ?? ?? 83 C4 08 84 C0 75 ?? B0 33",
                    "?? ?? B8 01 00 00 00 ?? ?? ?? ?? ?? EB",
                },
                {
                    "E8 ?? ?? ?? ?? 83 C4 ?? 88 45 ?? 3C 30 0F 84 ?? ?? ?? ?? 3C 35 75",
                    "B8 30 00 00 00 ?? ?? ?? ?? ?? ?? ?? ?? 90 E9",
                },
            },
        },

        {
            // detection_patt
            "FF 95 ?? ?? ?? ?? 89 85 ?? ?? ?? ?? 8B ?? ?? ?? ?? ?? 89 ?? ?? ?? ?? ?? 8B ?? ?? 89 ?? ?? ?? ?? ?? 83 A5 ?? ?? ?? ?? ?? EB",
            // snr_units
            {
                {
                    "F6 C? 02 0F 85 ?? ?? ?? ?? 5? FF ?? 6?",
                    "?? ?? ?? 90 E9 00 03",
                },
            },
        },
        
        {
            // detection_patt
            "FF D? 88 45 ?? 3C 30 0F 85 ?? ?? ?? ?? B8 4D 5A",
            // snr_units
            {
                {
                    "5? E8 ?? ?? ?? ?? 83 C4 ?? 88 45 ?? 3C 30 0F 84",
                    "?? B8 30 00 00 00 ?? ?? ?? ?? ?? ?? ?? ?? 90 E9",
                },
                {
                    "5? E8 ?? ?? ?? ?? 8? ?? 83 C4 ?? 8? F? 30 74 ?? 8?",
                    "?? B8 30 00 00 00 ?? ?? ?? ?? ?? ?? ?? ?? EB",
                },
            },
        },
        
    };

#endif // _WIN64

const std::vector<SnrUnit_t> *current_snr_units = nullptr;

static std::recursive_mutex mtx_win32_api{};
static uint8_t *proc_addr_base = (uint8_t *)GetModuleHandleW(NULL);
static uint8_t *bind_addr_base = nullptr;
static uint8_t *bind_addr_end = nullptr;

bool restore_win32_apis();

static void patch_if_possible(void *ret_addr)
{
    if (!ret_addr) return;
    
    auto page_details = pe_helpers::get_mem_page_details(ret_addr);
    if (!page_details.BaseAddress || page_details.AllocationProtect != PAGE_READWRITE) return;

    bool anything_found = false;
    for (const auto &snr_unit : *current_snr_units) {
        auto mem = pe_helpers::search_memory(
            (uint8_t *)page_details.BaseAddress,
            page_details.RegionSize,
            snr_unit.search_patt);
        
        if (mem) {
            anything_found = true;
            
            auto size_until_match = (uint8_t *)mem - (uint8_t *)page_details.BaseAddress;
            bool ok = pe_helpers::replace_memory(
                (uint8_t *)mem,
                page_details.RegionSize - size_until_match,
                snr_unit.replace_patt,
                GetCurrentProcess());
            // if (!ok) return;
        }
    }

    if (anything_found) {
        restore_win32_apis();
    }

    // MessageBoxA(NULL, ("ret addr = " + std::to_string((size_t)ret_addr)).c_str(), "Patched", MB_OK);
}

// https://learn.microsoft.com/en-us/cpp/intrinsics/addressofreturnaddress
static bool GetTickCount_hooked = false;
static decltype(GetTickCount) *actual_GetTickCount = GetTickCount;
__declspec(noinline)
static DWORD WINAPI GetTickCount_hook()
{
    std::lock_guard lk(mtx_win32_api);

    if (GetTickCount_hooked) { // american truck doesn't call GetModuleHandleA
        void* *ret_ptr = (void**)_AddressOfReturnAddress();
        patch_if_possible(*ret_ptr);
    }

    return actual_GetTickCount();
}

static bool GetModuleHandleA_hooked = false;
static decltype(GetModuleHandleA) *actual_GetModuleHandleA = GetModuleHandleA;
__declspec(noinline)
static HMODULE WINAPI GetModuleHandleA_hook(
  LPCSTR lpModuleName
)
{
    std::lock_guard lk(mtx_win32_api);

    if (GetModuleHandleA_hooked &&
        lpModuleName  &&
        common_helpers::ends_with_i(lpModuleName, "ntdll.dll")) {
        void* *ret_ptr = (void**)_AddressOfReturnAddress();
        patch_if_possible(*ret_ptr);
    }

    return actual_GetModuleHandleA(lpModuleName);
}

static bool GetModuleHandleExA_hooked = false;
static decltype(GetModuleHandleExA) *actual_GetModuleHandleExA = GetModuleHandleExA;
__declspec(noinline)
static BOOL WINAPI GetModuleHandleExA_hook(
  DWORD   dwFlags,
  LPCSTR  lpModuleName,
  HMODULE *phModule
)
{
    std::lock_guard lk(mtx_win32_api);

    if (GetModuleHandleExA_hooked &&
        (dwFlags == (GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT)) &&
        ((uint8_t *)lpModuleName >= bind_addr_base && (uint8_t *)lpModuleName < bind_addr_end)) {
        void* *ret_ptr = (void**)_AddressOfReturnAddress();
        patch_if_possible(*ret_ptr);
    }

    return actual_GetModuleHandleExA(dwFlags, lpModuleName, phModule);
}

static bool redirect_win32_apis()
{
    if (DetourTransactionBegin() != NO_ERROR) return false;
    if (DetourUpdateThread(GetCurrentThread()) != NO_ERROR) return false;

    if (DetourAttach((PVOID *)&actual_GetTickCount, GetTickCount_hook) != NO_ERROR) return false;
    if (DetourAttach((PVOID *)&actual_GetModuleHandleA, GetModuleHandleA_hook) != NO_ERROR) return false;
    if (DetourAttach((PVOID *)&actual_GetModuleHandleExA, GetModuleHandleExA_hook) != NO_ERROR) return false;
    bool ret = DetourTransactionCommit() == NO_ERROR;
    if (ret) {
        GetTickCount_hooked = true;
        GetModuleHandleA_hooked = true;
        GetModuleHandleExA_hooked = true;
    }
    return ret;
}

static bool restore_win32_apis()
{
    GetTickCount_hooked = false;
    GetModuleHandleA_hooked = false;
    GetModuleHandleExA_hooked = false;

    if (DetourTransactionBegin() != NO_ERROR) return false;
    if (DetourUpdateThread(GetCurrentThread()) != NO_ERROR) return false;

    DetourDetach((PVOID *)&actual_GetTickCount, GetTickCount_hook);
    DetourDetach((PVOID *)&actual_GetModuleHandleA, GetModuleHandleA_hook);
    DetourDetach((PVOID *)&actual_GetModuleHandleExA, GetModuleHandleExA_hook);
    return DetourTransactionCommit() == NO_ERROR;
}

bool stubdrm::patch()
{
    auto bind_section = pe_helpers::get_section_header_with_name(((HMODULE)proc_addr_base), ".bind");
    if (!bind_section) return false; // we don't have .bind section

    bind_addr_base = proc_addr_base + bind_section->VirtualAddress;
    MEMORY_BASIC_INFORMATION mbi{};
    if (!VirtualQuery((LPVOID)bind_addr_base, &mbi, sizeof(mbi))) return false;

    bind_addr_end = bind_addr_base + mbi.RegionSize;
    auto addrOfEntry = proc_addr_base + pe_helpers::get_optional_header((HMODULE)proc_addr_base)->AddressOfEntryPoint;
    if (addrOfEntry < bind_addr_base || addrOfEntry >= bind_addr_end) return false; // entry addr is not inside .bind

    for (const auto &patt : snr_patts) {
        auto mem = pe_helpers::search_memory(
            bind_addr_base,
            bind_section->Misc.VirtualSize,
            patt.detection_patt);
        
        if (mem) {
            current_snr_units = &patt.snr_units;
            return redirect_win32_apis();
        }
    }
    
    return false;
}

bool stubdrm::restore()
{
    return restore_win32_apis();
}
