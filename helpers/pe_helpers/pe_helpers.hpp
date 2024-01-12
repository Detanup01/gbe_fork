#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winternl.h>

#include <string>

namespace pe_helpers
{

typedef struct SectionHeadersResult
{
	WORD count;
	PIMAGE_SECTION_HEADER ptr;
} SectionHeadersResult_t;


uint8_t* search_memory(uint8_t *mem, size_t size, const std::string &search_patt);

bool replace_memory(uint8_t *mem, size_t size, const std::string &replace_patt, HANDLE hProcess);

std::string get_err_string(DWORD code);

bool is_module_64(HMODULE hModule);

bool is_module_32(HMODULE hModule);

SectionHeadersResult get_section_headers(HMODULE hModule);

PIMAGE_SECTION_HEADER get_section_header_with_name(HMODULE hModule, const char* name);

DWORD loadlib_remote(HANDLE hProcess, const std::wstring &lib_fullpath, const char** err_reason = nullptr);

size_t get_pe_size(HMODULE hModule);

const std::string get_current_exe_path();

const std::wstring get_current_exe_path_w();

bool ends_with_i(PUNICODE_STRING target, const std::wstring &query);

}
