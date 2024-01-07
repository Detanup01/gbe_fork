#include "crash_printer/common.hpp"
#include <fstream>
#include <filesystem>

static bool create_dir_impl(std::filesystem::path &dirpath)
{
    if (std::filesystem::is_directory(dirpath))
    {
        return true;
    }
    else if (std::filesystem::exists(dirpath)) // a file, we can't do anything
    {
        return false;
    }
    
    return std::filesystem::create_directories(dirpath);
}

bool crash_printer::create_dir(const std::string &filepath)
{
    std::filesystem::path dirpath = std::filesystem::path(filepath).parent_path();
    return create_dir_impl(dirpath);
}

bool crash_printer::create_dir(const std::wstring &filepath)
{
    std::filesystem::path dirpath = std::filesystem::path(filepath).parent_path();
    return create_dir_impl(dirpath);
}

void crash_printer::write(std::ofstream &file, const std::string &data)
{
    if (!file.is_open()) {
        return;
    }

    file << data << std::endl;
}
