#include <fstream>
#include <iostream>
#include <vector>

#include "pe_helpers/pe_helpers.hpp"

static size_t get_file_size(std::fstream &file)
{
    try
    {
        auto org_pos = file.tellg();
        file.seekg(0, std::ios::end);
        auto size = file.tellg();
        file.seekg(org_pos, std::ios::beg);
        return (size_t)size;
    }
    catch(...)
    {
        return 0;
    }
    
}

static std::vector<uint8_t> load_file_partial(std::fstream &file)
{
    try
    {
        auto org_pos = file.tellg();
        file.seekg(0, std::ios::beg);

        // 1MB is enough
        std::vector<uint8_t> data(1 * 1024 * 1024, 0);
        file.read((char *)&data[0], data.size());

        file.seekg(org_pos, std::ios::beg);
        return data;
    }
    catch(const std::exception& e)
    {
        return std::vector<uint8_t>();
    }
    
}


constexpr static size_t DOS_STUB_OOFSET = offsetof(IMAGE_DOS_HEADER, e_lfanew) + sizeof(IMAGE_DOS_HEADER::e_lfanew);
constexpr static const char DOS_STUB_SIGNATURE[] = "VLV";
constexpr static uint32_t DOS_STUB_ONE = 1; // what is this?


int main(int argc, char* *argv)
{
    if (argc < 2) {
        std::cerr << "Expected one or more binary files" << std::endl;
        return 1;
    }

    for (size_t i = 1; i < argc; ++i) {
        auto arg = argv[i];
        std::fstream file(arg, std::ios::in | std::ios::out | std::ios::binary);
        if (!file.is_open()) {
            std::cerr << "Failed to open file: '" << arg << "'" << std::endl;
            return 1;
        }

        auto file_size = get_file_size(file);
        if (!file_size) {
            std::cerr << "Failed get file size for file: '" << arg << "'" << std::endl;
            return 1;
        }
        auto data = load_file_partial(file);
        if (data.empty()) {
            std::cerr << "Failed get file data for file: '" << arg << "'" << std::endl;
            return 1;
        }
        uint32_t pe_size = (uint32_t)pe_helpers::get_pe_size((HMODULE)&data[0]);
        if (!pe_size) {
            std::cerr << "Failed get PE size for file: '" << arg << "'" << std::endl;
            return 1;
        }

        // std::cout << "File size = " << file_size << ", PE size = " << pe_size << std::endl;

        file.seekp(DOS_STUB_OOFSET, std::ios::beg);

        // 4 bytes: 'V' 'L' 'V' '\0'
        file.write(DOS_STUB_SIGNATURE, sizeof(DOS_STUB_SIGNATURE));
        // 4 bytes: (uint32_t)1
        file.write((const char *)&DOS_STUB_ONE, sizeof(DOS_STUB_ONE));
        // 4 bytes: PE image size
        file.write((const char *)&pe_size, sizeof(pe_size));

        file.close();
    }
    
    return 0;
}
