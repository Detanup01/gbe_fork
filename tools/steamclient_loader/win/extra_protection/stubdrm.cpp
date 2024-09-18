#include "pe_helpers/pe_helpers.hpp"
#include "common_helpers/common_helpers.hpp"
#include "extra_protection/stubdrm.hpp"
#include "detours/detours.h"
#include <tuple>
#include <mutex>
#include <intrin.h>

// MinGW doesn't implement _AddressOfReturnAddress(), throws linker error
// https://gcc.gnu.org/onlinedocs/gcc/Return-Address.html
#if defined(__GNUC__) && (defined(__MINGW32__) || defined(__MINGW64__))
    #define ADDR_OF_RET_ADDR() ((void*)((char*)__builtin_frame_address(0) + sizeof(void*)))
#else // regular windows
    #define ADDR_OF_RET_ADDR() _AddressOfReturnAddress()
#endif

typedef struct SnrUnit {
    std::string search_patt{};
    std::string replace_patt{};
} SnrUnit_t;

typedef struct SnrDetails {
    std::string detection_patt{};
    bool change_mem_access = false;
    std::vector<SnrUnit_t> snr_units{};
} SnrDetails_t;

// x64
#if defined(_WIN64)
    static const std::vector<SnrDetails> snr_patts {
        {
            // detection_patt
            "FF 94 24 ?? ?? ?? ?? 88 44 24 ?? 0F BE 44 24 ?? 83 ?? 30 74 ?? E9",
            // change memory pages access to r/w/e
            false,
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
            // change memory pages access to r/w/e
            false,
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

    static const std::vector<SnrDetails> snr_patts {
        {
            // detection_patt
            "FF 95 ?? ?? ?? ?? 88 45 ?? 0F BE 4D ?? 83 ?? 30 74 ?? E9",
            // change memory pages access to r/w/e
            false,
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
            // change memory pages access to r/w/e
            true,
            // snr_units
            {
                {
                    "F6 C? 02 0F 85 ?? ?? ?? ?? 5? FF ?? 6?",
                    "?? ?? ?? 90 E9 00 03",
                },
                {
                    "F6 C? 02 89 ?? ?? ?? ?? ?? A3 ?? ?? ?? ?? 0F 85",
                    "?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 90 E9 00 03 00 00",
                },
                {
                    "F6 05 ?? ?? ?? ?? 02 89 ?? ?? 0F 85 ?? ?? ?? ?? 5? FF ?? 6?",
                    "?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 90 E9 03 03",
                },
            },
        },
        
        {
            // detection_patt
            "FF D? 88 45 ?? 3C 30 0F 85 ?? ?? ?? ?? B8 4D 5A",
            // change memory pages access to r/w/e
            false,
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


static size_t current_snr_details = static_cast<size_t>(-1);

static std::recursive_mutex mtx_win32_api{};
static uint8_t *exe_addr_base = (uint8_t *)GetModuleHandleW(NULL);
static uint8_t *bind_addr_base = nullptr;
static uint8_t *bind_addr_end = nullptr;

static bool restore_win32_apis();

// stub v2 needs manual change for .text, section must have write access
static void change_mem_pages_access()
{
    auto sections = pe_helpers::get_section_headers((HMODULE)exe_addr_base);
    if (!sections.count) return;

    for (size_t i = 0; i < sections.count; ++i) {
        auto section = sections.ptr[i];
        uint8_t *section_base_addr = exe_addr_base + section.VirtualAddress;
        MEMORY_BASIC_INFORMATION mbi{};
        constexpr const static auto ANY_EXECUTE_RIGHT = PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;
        if (VirtualQuery((LPCVOID)section_base_addr, &mbi, sizeof(mbi)) && // function succeeded
            (mbi.Protect & ANY_EXECUTE_RIGHT)) { // this page (not entire section) has execute rights
            DWORD current_protection = 0;
            auto res = VirtualProtect(section_base_addr, mbi.RegionSize, PAGE_EXECUTE_READWRITE, &current_protection);
            // if (!res) {
            //     MessageBoxA(
            //         nullptr,
            //         (std::string("Failed to change access of page '") + (char *)section.Name + "' ").c_str(),
            //         "Failed",
            //         MB_OK
            //     );
            // }
        }
    }


}

static void patch_if_possible(void *ret_addr)
{
    if (!ret_addr) return;
    
    auto page_details = pe_helpers::get_mem_page_details(ret_addr);
    if (!page_details.BaseAddress || page_details.AllocationProtect != PAGE_READWRITE) return;

    bool anything_found = false;
    for (const auto &snr_unit : snr_patts[current_snr_details].snr_units) {
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
        if (snr_patts[current_snr_details].change_mem_access) change_mem_pages_access();
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
        void* *ret_ptr = (void**)ADDR_OF_RET_ADDR();
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
        void* *ret_ptr = (void**)ADDR_OF_RET_ADDR();
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
        void* *ret_ptr = (void**)ADDR_OF_RET_ADDR();
        patch_if_possible(*ret_ptr);
    }

    return actual_GetModuleHandleExA(dwFlags, lpModuleName, phModule);
}

static bool redirect_win32_apis()
{
    if (DetourTransactionBegin() != NO_ERROR) return false;
    if (DetourUpdateThread(GetCurrentThread()) != NO_ERROR) return false;

    if (DetourAttach((PVOID *)&actual_GetTickCount, (PVOID)GetTickCount_hook) != NO_ERROR) return false;
    if (DetourAttach((PVOID *)&actual_GetModuleHandleA, (PVOID)GetModuleHandleA_hook) != NO_ERROR) return false;
    if (DetourAttach((PVOID *)&actual_GetModuleHandleExA, (PVOID)GetModuleHandleExA_hook) != NO_ERROR) return false;
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

    DetourDetach((PVOID *)&actual_GetTickCount, (PVOID)GetTickCount_hook);
    DetourDetach((PVOID *)&actual_GetModuleHandleA, (PVOID)GetModuleHandleA_hook);
    DetourDetach((PVOID *)&actual_GetModuleHandleExA, (PVOID)GetModuleHandleExA_hook);
    return DetourTransactionCommit() == NO_ERROR;
}

static std::vector<uint8_t> get_pe_header_disk()
{
    const std::string filepath = pe_helpers::get_current_exe_path() + pe_helpers::get_current_exe_name();
    try {
        std::ifstream file(std::filesystem::u8path(filepath), std::ios::in | std::ios::binary);
        if (!file) return {};

        // 2MB is enough
        std::vector<uint8_t> data(2 * 1024 * 1024, 0);
        file.read((char *)&data[0], data.size());
        file.close();

        return data;
    } catch(...) { }

    return {};
}

static bool calc_bind_section_boundaries()
{
    auto bind_section = pe_helpers::get_section_header_with_name(((HMODULE)exe_addr_base), ".bind");
    if (bind_section) {
        bind_addr_base = exe_addr_base + bind_section->VirtualAddress;
        MEMORY_BASIC_INFORMATION mbi{};
        if (VirtualQuery((LPVOID)bind_addr_base, &mbi, sizeof(mbi)) && mbi.RegionSize > 0) {
            bind_addr_end = bind_addr_base + mbi.RegionSize;
        } else if (bind_section->Misc.VirtualSize > 0) {
            bind_addr_end = bind_addr_base + bind_section->Misc.VirtualSize;
        } else {
            return false;
        }
        return true;
    }
    
    // we don't *seem* to have .bind section *in memory*
    // appid 1732190 changes the PIMAGE_OPTIONAL_HEADER->SizeOfHeaders to a size less than the actual,
    // subtracting the size of the last section, i.e ".bind" section (original size = 0x600 >>> decreased to 0x400)
    // that way whenever the .exe is loaded in memory, the Windows loader will ignore populating the PE header with the info
    // of that section *in memory* since it is not taken into consideration, but the PE header *on disk* still contains the info
    // 
    // also the PIMAGE_FILE_HEADER->NumberOfSections is kept intact, otherwise the PIMAGE_OPTIONAL_HEADER->AddressOfEntryPoint
    // would be pointing at a non-existent section and the .exe won't work
    auto disk_header = get_pe_header_disk();
    if (disk_header.empty()) return false;

    bind_section = pe_helpers::get_section_header_with_name(((HMODULE)&disk_header[0]), ".bind");
    if (!bind_section) return false;

    bind_addr_base = exe_addr_base + bind_section->VirtualAddress;
    if (!bind_section->Misc.VirtualSize) return false;

    bind_addr_end = bind_addr_base + bind_section->Misc.VirtualSize;

    return true;
}

bool stubdrm::patch()
{
    if (!calc_bind_section_boundaries()) return false;
    
    auto addrOfEntry = exe_addr_base + pe_helpers::get_optional_header((HMODULE)exe_addr_base)->AddressOfEntryPoint;
    if (addrOfEntry < bind_addr_base || addrOfEntry >= bind_addr_end) return false; // entry addr is not inside .bind

    for (const auto &patt : snr_patts) {
        auto mem = pe_helpers::search_memory(
            bind_addr_base,
            static_cast<size_t>(bind_addr_end - bind_addr_base),
            patt.detection_patt);
        
        if (mem) {
            current_snr_details = &patt - &snr_patts[0];
            return redirect_win32_apis();
        }
    }
    
    return false;
}

bool stubdrm::restore()
{
    return restore_win32_apis();
}
