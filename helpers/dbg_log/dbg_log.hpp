#pragma once

namespace dbg_log
{

bool init(const wchar_t *path);

bool init(const char *path);

void write(const char* fmt, ...);

void close();
    
}
