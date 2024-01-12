#pragma once

#include <string>

namespace dbg_log
{

bool init(const wchar_t *path);

bool init(const char *path);

void write(const std::wstring &str);

void write(const std::string &str);

void write(const char* fmt, ...);

void close();
    
}
