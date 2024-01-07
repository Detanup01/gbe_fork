
#include "crash_printer/win.hpp"
#include "./test_helper.hpp"

#include <iostream>
#include <fstream>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

std::wstring logs_filepath = L"./crash_test/zxc.txt";


static LONG WINAPI exception_handler(LPEXCEPTION_POINTERS ex_pointers)
{
    std::ifstream file(logs_filepath);
    if (file.is_open()) {
        std::string line{};
        std::getline(file, line);
        file.close();

        if (line.size()) {
            std::cout << "Success!" << std::endl;
            exit(0);
        }
    }
    
    std::cerr << "Failed!" << std::endl;
    exit(1);
}


int main()
{
    // simululate the existence of previous handler
    SetUnhandledExceptionFilter(exception_handler);

    if (!remove_file(logs_filepath)) {
        std::cerr << "failed to remove log" << std::endl;
        return 1;
    }

    if (!crash_printer::init(logs_filepath)) {
        std::cerr << "failed to init" << std::endl;
        return 1;
    }

    // simulate a crash
    volatile int * volatile ptr = nullptr;
    *ptr = 42;

    return 0;
}
