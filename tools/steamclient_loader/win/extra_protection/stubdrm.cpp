#include "extra_protection/stubdrm.hpp"
#include "pe_helpers/pe_helpers.hpp"
#include "common_helpers/common_helpers.hpp"
#include "detours/detours.h"
#include <string>
#include <vector>
#include <tuple>
#include <mutex>
#include <intrin.h>

// MinGW doesn't implement _AddressOfReturnAddress(), throws linker error
// https://gcc.gnu.org/onlinedocs/gcc/Return-Address.html
// https://learn.microsoft.com/en-us/cpp/intrinsics/addressofreturnaddress
#if defined(__GNUC__) && (defined(__MINGW32__) || defined(__MINGW64__))
    #define ADDR_OF_RET_ADDR() ((void*)((char*)__builtin_frame_address(0) + sizeof(void*)))
#else // regular windows
    #define ADDR_OF_RET_ADDR() _AddressOfReturnAddress()
#endif

typedef struct _SnrUnit_t {
    std::string search_patt{};
    std::string replace_patt{};
} SnrUnit_t;

typedef struct _StubSnrDetails_t {
    std::string stub_detection_patt{}; // inside the dynamically allocated stub
    bool change_mem_access = false;
    std::vector<SnrUnit_t> stub_snr_units{};
} StubSnrDetails_t;

typedef struct _BindSnrDetails_t {
    std::vector<std::string> bind_detection_patts{}; // any pattern inside .bind
    std::vector<StubSnrDetails_t> stub_details{};
} BindSnrDetails_t;

static const std::vector<BindSnrDetails_t> all_bind_details {

// x64
#if defined(_WIN64)
    {
        // bind_detection_patts[]
        {
            "FF 94 24 ?? ?? ?? ?? 88 44 24 ?? 0F BE 44 24 ?? 83 ?? 30 74 ?? E9", // appid 1684350
        },
        // stub_details[]
        {
            {
                // stub_detection_patt
                "??",
                // change memory pages access to r/w/e
                false,
                // stub_snr_units[]
                {
                    // patt 1 is a bunch of checks for registry + files validity (including custom DOS stub)
                    // patt 2 is again a bunch of checks + creates some interfaces via steamclient + calls getappownershipticket()
                    {
                        "E8 ?? ?? ?? ?? 84 C0 75 ?? B0 3?",
                        "B8 01 00 00 00 ?? ?? EB",
                    },
                    {
                        "E8 ?? ?? ?? ?? 44 0F B6 ?? 3C 30 0F 84 ?? ?? ?? ?? 3C",
                        "B8 30 00 00 00 ?? ?? ?? ?? ?? ?? 90 E9",
                    },
                },
            },
        },
    },

    {
        // bind_detection_patts[]
        {
            "FF D? 44 0F B6 ?? 3C 30 0F 85", // appid: 537450 (rare, only found in this appid!)
        },
        // stub_details[]
        {
            {
                // stub_detection_patt
                "??",
                // change memory pages access to r/w/e
                false,
                // stub_snr_units[]
                {
                    // patt 1 is a bunch of checks for registry + files validity (including custom DOS stub)
                    // patt 2 is again a bunch of checks + creates some interfaces via steamclient + calls getappownershipticket()
                    {
                        "E8 ?? ?? ?? ?? 84 C0 75 ?? B0 3?",
                        "B8 01 00 00 00 ?? ?? EB",
                    },
                    {
                        "E8 ?? ?? ?? ?? 44 0F B6 ?? 3C 30 0F 84 ?? ?? ?? ?? 3C",
                        "B8 30 00 00 00 ?? ?? ?? ?? ?? ?? 90 E9",
                    },
                },
            },
        },
    },

#endif // x64

// x32
#if !defined(_WIN64)
    {
        // bind_detection_patts[]
        {
            "FF 95 ?? ?? ?? ?? 88 45 ?? 0F BE 4D ?? 83 ?? 30 74 ?? E9", // appid 588650
        },
        // stub_details[]
        {
            {
                // stub_detection_patt
                "??",
                // change memory pages access to r/w/e
                false,
                // stub_snr_units[]
                {
                    // patt 1 is a bunch of checks for registry + files validity (including custom DOS stub)
                    // patt 2 is again a bunch of checks + creates some interfaces via steamclient + calls getappownershipticket()
                    {
                        "5? 5? E8 ?? ?? ?? ?? 83 C4 08 84 C0 75",
                        "?? ?? B8 01 00 00 00 ?? ?? ?? ?? ?? EB",
                    },
                    {
                        "E8 ?? ?? ?? ?? 83 C4 ?? 88 45 ?? 3C 30 0F 84 ?? ?? ?? ?? 3C 3?",
                        "B8 30 00 00 00 ?? ?? ?? ?? ?? ?? ?? ?? 90 E9",
                    },
                },
            },
        },
    },

    {
        // bind_detection_patts[]
        {
            "FF 95 ?? ?? ?? ?? 89 85 ?? ?? ?? ?? 8B ?? ?? ?? ?? ?? 89 ?? ?? ?? ?? ?? 8B ?? ?? 89 ?? ?? ?? ?? ?? 83 A5 ?? ?? ?? ?? ?? EB", // appid 201790
        },
        // stub_details[]
        {
            {
                // stub_detection_patt
                "??",
                // change memory pages access to r/w/e
                true, // appid 48000
                // stub_snr_units[]
                {
                    {
                        "F6 C? 02 0F 85 ?? ?? ?? ?? 5? FF ?? 6?",
                        "?? ?? ?? 90 E9 00 03",
                    },
                    {
                        "F6 C? 02 89 ?? ?? ?? ?? ?? A3 ?? ?? ?? ?? 0F 85",
                        "?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 90 E9 00 03 00 00",
                    },
                    { // appid 250180
                        "F6 05 ?? ?? ?? ?? 02 89 ?? ?? 0F 85 ?? ?? ?? ?? 5? FF ?? 6?",
                        "?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 90 E9 03 03",
                    },
                },
            },
        },
    },
    
    {
        // bind_detection_patts[]
        {
            "FF D? 88 45 ?? 3C 30 0F 85 ?? ?? ?? ?? B8 4D 5A",
            "FF D? 88 45 ?? 3C 30 0F 84 ?? ?? ?? ?? FF 9?", // appid 251390
        },
        // stub_details[]
        {
            {
                // stub_detection_patt
                "??",
                // change memory pages access to r/w/e
                false,
                // stub_snr_units[]
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
        },
    },
        
    {
        // bind_detection_patts[]
        {
            "FF 95 ?? ?? ?? ?? 89 85 ?? ?? ?? ?? 8B ?? ?? ?? ?? ?? 89 ?? ?? ?? ?? ?? 8B ?? ?? ?? ?? ?? 89 ?? ?? ?? ?? ?? 83 A5 ?? ?? ?? ?? ?? EB", // appids: 31290, 94530, 37010
        },
        // stub_details[]
        {
            { // appid 31290, 37010
                // stub_detection_patt
                "F6 05 ?? ?? ?? ?? 04 0F 85 ?? ?? ?? ?? A1 ?? ?? ?? ?? 89",
                // change memory pages access to r/w/e
                false,
                // stub_snr_units[]
                {
                    {
                        "F6 C? 02 89 ?? ?? ?? ?? ?? A3 ?? ?? ?? ?? 0F 85",
                        "?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 90 E9 57 02 00 00",
                    },
                },
            },

            { // 94530
                // stub_detection_patt
                "84 ?? ?? ?? ?? ?? 0F 85 ?? ?? ?? ?? A1 ?? ?? ?? ?? 89",
                // change memory pages access to r/w/e
                false,
                // stub_snr_units[]
                {
                    {
                        "F6 C? 02 89 ?? ?? ?? ?? ?? A3 ?? ?? ?? ?? 0F 85",
                        "?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? ?? 90 E9 2A 02 00 00",
                    },
                    {
                        "6A 04 5? 5? 8D",
                        "?? ?? ?? E9 BF 00 00 00",
                    },
                },
            },
        },
    },
        
#endif // x32

};


static size_t current_bind_idx = static_cast<size_t>(-1);

static uint8_t *exe_addr_base = (uint8_t *)GetModuleHandleW(nullptr);
static uint8_t *bind_addr_base = nullptr;
static uint8_t *bind_addr_end = nullptr;

// this mutex is used to halt/defer the execution of threads if they tried to use the hooked functions at the same time
// just in case
static std::recursive_mutex mtx_win32_api{};
// these flags are used as a fallback in case Detours lib failed to restore the hooks
static bool GetTickCount_hooked = false;
static bool GetModuleHandleA_hooked = false;
static bool GetModuleHandleExA_hooked = false;
void (*cleanup_cb)() = nullptr;


// old stub variant (found in appid 201790) needs manual change for .text section, it must have write access
static void change_mem_pages_access()
{
    auto sections = pe_helpers::get_section_headers((HMODULE)exe_addr_base);
    if (!sections.count) return;
    
    constexpr const static unsigned ANY_EXECUTE_RIGHT = PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;
    for (size_t i = 0; i < sections.count; ++i) {
        auto section = sections.ptr[i];
        uint8_t *section_base_addr = exe_addr_base + section.VirtualAddress;
        MEMORY_BASIC_INFORMATION mbi{};
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

static void call_cleanup_cb()
{
    if (cleanup_cb) {
        cleanup_cb();
    }
}

static bool restore_win32_apis();
static void patch_if_possible(void *ret_addr)
{
    if (!ret_addr) return;
    
    auto page_details = pe_helpers::get_mem_page_details(ret_addr);
    if (!page_details.BaseAddress || page_details.AllocationProtect != PAGE_READWRITE) return;

    // find stub variant
    const StubSnrDetails_t *current_stub = nullptr;
    const auto &bind_details = all_bind_details[current_bind_idx];
    for (const auto &stub_details : bind_details.stub_details) {
        auto mem = pe_helpers::search_memory(
            (uint8_t *)page_details.BaseAddress,
            page_details.RegionSize,
            stub_details.stub_detection_patt);
        
        if (mem) {
            current_stub = &stub_details;
            break;
        }
    }

    if (!current_stub) {
        // we can't remove hooks here, the drm allocates many pages with read/write access to decrypt other parts
        // and their code also gets here, if we restore hooks then we can't patch the actual stub page (which comes much later)
        return;
    }

    // patch all snr units inside stub
    bool anything_found = false;
    for (const auto &snr_unit : current_stub->stub_snr_units) {
        auto mem = pe_helpers::search_memory(
            (uint8_t *)page_details.BaseAddress,
            page_details.RegionSize,
            snr_unit.search_patt);
        
        if (mem) {
            anything_found = true;
            
            auto size_until_match = mem - (uint8_t *)page_details.BaseAddress;
            bool ok = pe_helpers::replace_memory(
                mem,
                page_details.RegionSize - size_until_match,
                snr_unit.replace_patt,
                GetCurrentProcess());
            // if (!ok) return;
        }
    }

    if (anything_found) {
        // MessageBoxA(NULL, ("ret addr = " + std::to_string((size_t)ret_addr)).c_str(), "Patched", MB_OK);
        restore_win32_apis();
        if (current_stub->change_mem_access) {
            change_mem_pages_access();
        }
        call_cleanup_cb();
    }
}


static decltype(GetTickCount) *actual_GetTickCount = GetTickCount;
__declspec(noinline)
static DWORD WINAPI GetTickCount_hook()
{
    if (GetTickCount_hooked) { // unencrypted apps (like 270880 american truck) don't call GetModuleHandleA()
        std::lock_guard lk(mtx_win32_api);
        if (GetTickCount_hooked) { // if we win arbitration and hooks are still intact
            void* *ret_ptr = (void**)ADDR_OF_RET_ADDR();
            patch_if_possible(*ret_ptr);
        }
    }

    return actual_GetTickCount();
}

static decltype(GetModuleHandleA) *actual_GetModuleHandleA = GetModuleHandleA;
__declspec(noinline)
static HMODULE WINAPI GetModuleHandleA_hook(
  LPCSTR lpModuleName
)
{
    if (GetModuleHandleA_hooked &&
        lpModuleName && lpModuleName[0] &&
        common_helpers::ends_with_i(lpModuleName, "ntdll.dll")) {
        std::lock_guard lk(mtx_win32_api);
        if (GetModuleHandleA_hooked) { // if we win arbitration and hooks are still intact
            void* *ret_ptr = (void**)ADDR_OF_RET_ADDR();
            patch_if_possible(*ret_ptr);
        }
    }

    return actual_GetModuleHandleA(lpModuleName);
}

static decltype(GetModuleHandleExA) *actual_GetModuleHandleExA = GetModuleHandleExA;
__declspec(noinline)
static BOOL WINAPI GetModuleHandleExA_hook(
  DWORD   dwFlags,
  LPCSTR  lpModuleName,
  HMODULE *phModule
)
{
    constexpr const static unsigned HANDLE_FROM_ADDR = GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT;
    if (GetModuleHandleExA_hooked &&
        (dwFlags & HANDLE_FROM_ADDR) &&
        ((uint8_t *)lpModuleName >= bind_addr_base && (uint8_t *)lpModuleName < bind_addr_end)) {
        std::lock_guard lk(mtx_win32_api);
        if (GetModuleHandleExA_hooked) { // if we win arbitration and hooks are still intact
            void* *ret_ptr = (void**)ADDR_OF_RET_ADDR();
            patch_if_possible(*ret_ptr);
        }
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


static bool calc_bind_section_boundaries()
{
    constexpr static auto calc_bind_section_boundaries_from_mem = [] {
        auto bind_section = pe_helpers::get_section_header_with_name(reinterpret_cast<HMODULE>(exe_addr_base), ".bind");
        if (!bind_section || !bind_section->VirtualAddress) return false;

        uint8_t * const bind_start = exe_addr_base + bind_section->VirtualAddress;
        uint8_t *bind_end = nullptr;

        MEMORY_BASIC_INFORMATION mbi{};
        if (VirtualQuery((LPVOID)bind_start, &mbi, sizeof(mbi)) && mbi.RegionSize > 0) {
            bind_end = bind_start + mbi.RegionSize;
        } else if (bind_section->Misc.VirtualSize > 0) {
            bind_end = bind_start + bind_section->Misc.VirtualSize;
        } else {
            return false;
        }

        bind_addr_base = bind_start;
        bind_addr_end = bind_end;
        return true;
    };

    constexpr static auto calc_bind_section_boundaries_from_disk = [] {
        constexpr static auto get_pe_header_from_disk = []() -> std::vector<uint8_t> {
            try {
                const std::string filepath = pe_helpers::get_current_exe_path() + pe_helpers::get_current_exe_name();
                std::ifstream file(std::filesystem::u8path(filepath), std::ios::in | std::ios::binary);
                if (!file) return {};

                // 2MB is enough
                std::vector<uint8_t> data(2 * 1024 * 1024, 0);
                file.read((char *)&data[0], data.size());
                file.close();

                return data;
            } catch(...) { }

            return {};
        };

        auto disk_header = get_pe_header_from_disk();
        if (disk_header.empty()) return false;

        auto bind_section = pe_helpers::get_section_header_with_name(reinterpret_cast<HMODULE>(&disk_header[0]), ".bind");
        if (!bind_section || !bind_section->VirtualAddress) return false;
        if (!bind_section->Misc.VirtualSize) return false;

        bind_addr_base = exe_addr_base + bind_section->VirtualAddress;
        bind_addr_end = bind_addr_base + bind_section->Misc.VirtualSize;
        return true;
    };


    // appid 2677660 (build 16659541) changes the PIMAGE_OPTIONAL_HEADER->SizeOfHeaders to a size less than the actual,
    // so that the ".bind" section *looks* as if it exists in memory (IMAGE_SECTION_HEADER->Name is valid), but its data is 0/nulled
    if (calc_bind_section_boundaries_from_mem()) {
        return true;
    }
    
    // otherwise we *seem* to be missing the .bind section *in memory*, but not necessarily
    // appid 1732190 also changes the PIMAGE_OPTIONAL_HEADER->SizeOfHeaders to a size less than the actual
    // by subtracting the size of the last section, i.e ".bind" section (original size = 0x600 >>> decreased to 0x400)
    // that way whenever the .exe is loaded in memory, the Windows loader will ignore populating the PE header with the info
    // of that section *in memory* since it is not taken into consideration, but the PE header *on disk* still contains the info
    // 
    // also the PIMAGE_FILE_HEADER->NumberOfSections is kept intact, otherwise the PIMAGE_OPTIONAL_HEADER->AddressOfEntryPoint
    // would be pointing at a non-existent section and the .exe won't work
    if (calc_bind_section_boundaries_from_disk()) {
        return true;
    }
    
    return false;
}


bool stubdrm::patch()
{
    if (!calc_bind_section_boundaries()) return false;
    
    auto addrOfEntry = exe_addr_base + pe_helpers::get_optional_header((HMODULE)exe_addr_base)->AddressOfEntryPoint;
    if (addrOfEntry < bind_addr_base || addrOfEntry >= bind_addr_end) return false; // entry addr is not inside .bind

    // detect .bind variant, search for a known pattern inside the .bind section itself
    for (const auto &bind_details : all_bind_details) {
        // multiple variants of .bind sections can share the same details,
        // hence similar ones are grouped together, their detection patterns are added to the same array
        for (const auto &bind_detection_patt : bind_details.bind_detection_patts) {
            auto mem = pe_helpers::search_memory(
                bind_addr_base,
                static_cast<size_t>(bind_addr_end - bind_addr_base),
                bind_detection_patt);
            
            if (mem) {
                current_bind_idx = static_cast<size_t>(&bind_details - &all_bind_details[0]);
                return redirect_win32_apis();
            }
        }
    }
    
    return false;
}

bool stubdrm::restore()
{
    return restore_win32_apis();
}

void stubdrm::set_cleanup_cb(void (*fn)())
{
    cleanup_cb = fn;
}
