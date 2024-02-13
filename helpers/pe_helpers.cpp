#include "pe_helpers/pe_helpers.hpp"
#include "common_helpers/common_helpers.hpp"
#include <vector>
#include <utility>
#include <mutex>
#include <cwchar>

static inline bool is_hex(const char c)
{
    return (c >= '0' && c <= '9') ||
           (c >= 'a' && c <= 'f') ||
           (c >= 'A' && c <= 'F');
}

static inline uint8_t char_to_byte(const char c)
{
    if (c >= '0' && c <= '9') return (uint8_t)(c - '0');
    if (c >= 'a' && c <= 'f') return (uint8_t)(c - 'a') + 0xa;
    if (c >= 'A' && c <= 'F') return (uint8_t)(c - 'A') + 0xa;
    return (uint8_t)c;
}

PIMAGE_NT_HEADERS pe_helpers::get_nt_header(HMODULE hModule)
{
    // https://dev.to/wireless90/validating-the-pe-signature-my-av-flagged-me-windows-pe-internals-2m5o/
    PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)(char*)hModule;
    if (dosHeader->e_magic != 0x5A4D) { // "MZ" 
        return nullptr;
    }
    LONG newExeHeaderOffset = dosHeader->e_lfanew;
    return (PIMAGE_NT_HEADERS)((char*)hModule + newExeHeaderOffset);
}

PIMAGE_FILE_HEADER pe_helpers::get_file_header(HMODULE hModule)
{
    auto nt_header = get_nt_header(hModule);
    return nt_header ? &nt_header->FileHeader : nullptr;
}

PIMAGE_OPTIONAL_HEADER pe_helpers::get_optional_header(HMODULE hModule)
{
    auto nt_header = get_nt_header(hModule);
    return nt_header ? &nt_header->OptionalHeader : nullptr;
}

uint8_t* pe_helpers::search_memory(uint8_t *mem, size_t size, const std::string &search_patt)
{
    if (!mem || !size || search_patt.empty()) return nullptr;

    if (search_patt.find_first_not_of(" \t", 0) == std::string::npos) {
        return nullptr; // empty search patt
    }

    const uint8_t *end = mem + size;
    for (uint8_t *base = mem; base < end; ++base)
    {
        // incremental offset after each byte match
        size_t search_patt_offset = 0;
        bool error = false;
        for (const uint8_t *displacement = base; displacement < end; ++displacement)
        {
            uint8_t mask = 0xFF;
            uint8_t s_byte = 0;

            // skip spaces
            search_patt_offset = search_patt.find_first_not_of(" \t", search_patt_offset);
            if (search_patt_offset == std::string::npos) {
                break;
            }

            const auto this_char = search_patt[search_patt_offset];
            const auto next_char = (search_patt_offset + 1) < search_patt.size()
                ? search_patt[search_patt_offset + 1]
                : '\0';
            if (this_char == '?') {
                if (next_char == '?' || // "??"
                    next_char == ' ' || // "? "
                    next_char == 't') { // "?    "
                    mask = 0x00;
                    s_byte = 0;
                } else if (is_hex(next_char)) { // "?c"
                    mask = 0x0F;
                    s_byte = char_to_byte(next_char);
                } else { // unknown
                    return nullptr;
                }

                // skip
                search_patt_offset += 2;
            } else if (is_hex(this_char)) {
                if (next_char == '?') { // "c?"
                    mask = 0xF0;
                    s_byte = char_to_byte(this_char) << 4;
                } else if (is_hex(next_char)) { // "34"
                    mask = 0xFF;
                    s_byte = (char_to_byte(this_char) << 4) | char_to_byte(next_char);
                } else { // unknown
                    return nullptr;
                }
                
                // skip
                search_patt_offset += 2;
            } else { // unknown
                return nullptr;
            }

            if ((*displacement & mask) != (s_byte & mask)) {
                error = true;
                break;
            }
        }

        if (!error && (search_patt_offset >= search_patt.size())) {
            return base;
        }
    }

    return nullptr;
}

bool pe_helpers::replace_memory(uint8_t *mem, size_t size, const std::string &replace_patt, HANDLE hProcess)
{
    if (!mem || !size || replace_patt.empty()) return false;

    size_t replace_patt_offset = replace_patt.find_first_not_of(" \t", 0);
    if (replace_patt_offset == std::string::npos) {
        return false; // empty patt
    }

    // mask - byte
    std::vector<std::pair<uint8_t, uint8_t>> replace_bytes{};
    for (;
         replace_patt_offset < replace_patt.size();
         replace_patt_offset = replace_patt.find_first_not_of(" \t", replace_patt_offset)) {
        const auto this_char = replace_patt[replace_patt_offset];
        const auto next_char = (replace_patt_offset + 1) < replace_patt.size()
            ? replace_patt[replace_patt_offset + 1]
            : '\0';
        
        if (this_char == '?') {
            if (next_char == '?' || // "??"
                next_char == ' ' || // "? "
                next_char == 't') { // "?    "
                replace_bytes.push_back({
                    0x00,
                    0,
                });
            } else if (is_hex(next_char)) { // "?c"
                replace_bytes.push_back({
                    0x0F,
                    char_to_byte(next_char),
                });
            } else { // unknown
                return false;
            }

            // skip
            replace_patt_offset += 2;
        } else if (is_hex(this_char)) {
            if (next_char == '?') { // "c?"
                replace_bytes.push_back({
                    0xF0,
                    char_to_byte(this_char) << 4,
                });
            } else if (is_hex(next_char)) { // "34"
                replace_bytes.push_back({
                    0xFF,
                    (char_to_byte(this_char) << 4) | char_to_byte(next_char),
                });
            } else { // unknown
                return false;
            }
            
            // skip
            replace_patt_offset += 2;
        } else { // unknown
            return false;
        }
    }

    // remove trailing "??"
    // while last element == "??"
    while (replace_bytes.size() &&
           replace_bytes.back().first == 0x00)
    {
        replace_bytes.pop_back();
    }
    if (replace_bytes.empty() || replace_bytes.size() > size) return false;

    // change protection
    DWORD current_protection = 0;
    if (!VirtualProtectEx(hProcess, mem, replace_bytes.size(), PAGE_READWRITE, &current_protection)) {
        return false;
    }

    for (auto &rp : replace_bytes) {
        if (rp.first == 0x00) {
            ++mem;
            continue;
        }

        const uint8_t b_mem = (uint8_t)(*mem & (uint8_t)~rp.first);
        const uint8_t b_replace = (uint8_t)(rp.second & rp.first);
        const uint8_t new_b_mem = b_mem | b_replace;
        *mem = b_mem | b_replace;
        ++mem;
    }

    // restore protection
    if (!VirtualProtectEx(hProcess, mem, replace_bytes.size(), current_protection, &current_protection)) {
        return false;
    }

    return true;
}

// https://learn.microsoft.com/en-us/windows/win32/debug/retrieving-the-last-error-code
std::string pe_helpers::get_err_string(DWORD code)
{
    std::string err_str(8192, '\0');

    DWORD msg_chars = FormatMessageA(
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL),
        (LPSTR)&err_str[0],
        err_str.size(),
        NULL);
    
    if (!msg_chars) return std::string();

    err_str = err_str.substr(0, msg_chars);

    return err_str;
}

bool pe_helpers::is_module_64(HMODULE hModule)
{
    auto file_header = get_file_header(hModule);
    return file_header ? (file_header->Machine == IMAGE_FILE_MACHINE_AMD64) : false;
}

bool pe_helpers::is_module_32(HMODULE hModule)
{
    auto file_header = get_file_header(hModule);
    return file_header ? (file_header->Machine == IMAGE_FILE_MACHINE_I386) : false;
}

pe_helpers::SectionHeadersResult pe_helpers::get_section_headers(HMODULE hModule)
{
    PIMAGE_NT_HEADERS ntHeader = get_nt_header(hModule);
    PIMAGE_OPTIONAL_HEADER optionalHeader = &ntHeader->OptionalHeader;

    PIMAGE_FILE_HEADER fileHeader = get_file_header(hModule);
    WORD optionalHeadrSize = fileHeader->SizeOfOptionalHeader;

    struct SectionHeadersResult res {};
    res.count = fileHeader->NumberOfSections;
    res.ptr = fileHeader->NumberOfSections
        ? (PIMAGE_SECTION_HEADER)((char *)optionalHeader + optionalHeadrSize)
        : nullptr;

    return res;
}

PIMAGE_SECTION_HEADER pe_helpers::get_section_header_with_name(HMODULE hModule, const char *name)
{
    if (!name) return nullptr;

    auto res = get_section_headers(hModule);
    if (!res.count) return nullptr;

    for (size_t i = 0; i < res.count; ++i) {
        if (strncmp((const char *)res.ptr[i].Name, name, sizeof(res.ptr[i].Name)) == 0) {
            return &res.ptr[i];
        }
    }

    return nullptr;
}

DWORD pe_helpers::loadlib_remote(HANDLE hProcess, const std::wstring &lib_fullpath, const char** err_reason)
{
    // create a remote page
    const size_t lib_path_str_bytes = lib_fullpath.size() * sizeof(lib_fullpath[0]);
    LPVOID lib_remote_page = VirtualAllocEx(
    hProcess,
    NULL,
    lib_path_str_bytes + sizeof(lib_fullpath[0]) * 2, // *2 just to be safe
    MEM_RESERVE | MEM_COMMIT,
    PAGE_READWRITE
    );

    if (!lib_remote_page) {
        if (err_reason) {
            *err_reason = "Failed to remotely allocate page with VirtualAllocEx()";
        }
        return GetLastError();
    }

    SIZE_T bytes_written = 0;
    BOOL written = WriteProcessMemory(
        hProcess,
        lib_remote_page,
        (LPCVOID)&lib_fullpath[0],
        lib_path_str_bytes,
        &bytes_written
    );

    if (!written || bytes_written < lib_path_str_bytes) {
        // cleanup allcoated page
        VirtualFreeEx(
            hProcess,
            lib_remote_page,
            0,
            MEM_RELEASE);

        if (err_reason) {
            *err_reason = "Failed to remotely write dll path with WriteProcessMemory()";
        }
        return GetLastError();
    }

    // call LoadLibraryW() and pass the dll fullpath
    HANDLE remote_thread = CreateRemoteThread(
        hProcess,
        NULL,
        0,
        (LPTHREAD_START_ROUTINE)LoadLibraryW,
        lib_remote_page,
        0,
        NULL);

    if (!remote_thread) {
        // cleanup allcoated page
        VirtualFreeEx(
            hProcess,
            lib_remote_page,
            0,
            MEM_RELEASE);

        if (err_reason) {
            *err_reason = "Failed to create/run remote thread with CreateRemoteThread()";
        }
        return GetLastError();
    }

    // wait for DllMain
    WaitForSingleObject(remote_thread, INFINITE);
    CloseHandle(remote_thread);

    // cleanup allcoated page
    VirtualFreeEx(
        hProcess,
        lib_remote_page,
        0,
        MEM_RELEASE);

    return ERROR_SUCCESS;
}

size_t pe_helpers::get_pe_size(HMODULE hModule)
{
    // https://stackoverflow.com/a/34695773
    // https://learn.microsoft.com/en-us/windows/win32/debug/pe-format
    // "The PE file header consists of a Microsoft MS-DOS stub, the PE signature, the COFF file header, and an optional header"
    // "The combined size of an MS-DOS stub, PE header, and section headers rounded up to a multiple of FileAlignment."
    size_t size = get_optional_header(hModule)->SizeOfHeaders;
    SectionHeadersResult headers = get_section_headers(hModule);
    for (size_t i = 0; i < headers.count; ++i) {
        size += headers.ptr[i].SizeOfRawData;
    }

    return size;
}

static std::wstring path_w{};
static std::string path_a{};
const std::string pe_helpers::get_current_exe_path()
{
    if (path_a.empty()) {
        get_current_exe_path_w();
    }

    return path_a;
}

const std::wstring pe_helpers::get_current_exe_path_w()
{
    static std::recursive_mutex path_mtx{};
    if (path_w.empty()) {
        std::lock_guard lk(path_mtx);

        if (path_w.empty()) {
            DWORD err = GetLastError();

            path_w.resize(8192);
            DWORD read_chars = GetModuleFileNameW(GetModuleHandleW(nullptr), &path_w[0], (DWORD)path_w.size());
            if (read_chars >= path_w.size()) {
                path_w.resize(read_chars);
                read_chars = GetModuleFileNameW(GetModuleHandleW(nullptr), &path_w[0], (DWORD)path_w.size());
            }

            if ((read_chars < path_w.size()) && path_w[0]) {
                path_w = path_w.substr(0, path_w.find_last_of(L"\\/") + 1);

                auto cvt_state = std::mbstate_t();
                const wchar_t* src = &path_w[0];
                size_t conversion_bytes = std::wcsrtombs(nullptr, &src, 0, &cvt_state);
                path_a.resize(conversion_bytes + 1);
                std::wcsrtombs(&path_a[0], &src, path_a.size(), &cvt_state);
                path_a = path_a.substr(0, conversion_bytes);
            } else {
                path_w.clear();
            }

            SetLastError(err);
        }
    }

    return path_w;
}

bool pe_helpers::ends_with_i(PUNICODE_STRING target, const std::wstring &query)
{
    return common_helpers::ends_with_i(
        std::wstring(target->Buffer, (PWSTR)((char*)target->Buffer + target->Length)),
        query
    );
}

MEMORY_BASIC_INFORMATION pe_helpers::get_mem_page_details(const void* mem)
{
    MEMORY_BASIC_INFORMATION mbi{};
    if (VirtualQuery(mem, &mbi, sizeof(mbi))) {
        return mbi;
    } else {
        return {};
    }
}

size_t pe_helpers::get_current_exe_mem_size()
{
    auto hmod = GetModuleHandleW(NULL);
    size_t size = 0;

    {
        MEMORY_BASIC_INFORMATION mbi{};
        if (!VirtualQuery((LPVOID)hmod, &mbi, sizeof(mbi))) {
            return 0;
        }
        size = mbi.RegionSize; // PE header
    }

    auto sections = get_section_headers(hmod);
    if (!sections.count) {
        return 0;
    }

    for (size_t i = 0; i < sections.count; ++i) {
        auto section = sections.ptr[i];
        MEMORY_BASIC_INFORMATION mbi{};
        if (!VirtualQuery((LPVOID)((uint8_t *)hmod + section.VirtualAddress), &mbi, sizeof(mbi))) {
            return 0;
        }
        size = mbi.RegionSize; // actual section size in mem
    }

    return size;
}
