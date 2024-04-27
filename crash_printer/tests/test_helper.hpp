#ifndef _TEST_CRASH_PRINTER_HELPER_H
#define _TEST_CRASH_PRINTER_HELPER_H


#include <filesystem>
#include <string>

static inline bool remove_file(const std::string &file)
{
    const std::filesystem::u8path p_file(std::filesystem::u8path(file));
    if (!std::filesystem::exists(p_file)) {
        return true;
    }

    if (std::filesystem::is_directory(p_file)) {
        return false;
    }

    return std::filesystem::remove(p_file);
}

static inline bool remove_file(const std::wstring &file)
{
    if (!std::filesystem::exists(file)) {
        return true;
    }

    if (std::filesystem::is_directory(file)) {
        return false;
    }

    return std::filesystem::remove(file);
}


#endif // _TEST_CRASH_PRINTER_HELPER_H
