
#include "crash_printer/linux.hpp"
#include "./test_helper.hpp"


#include <iostream>
#include <fstream>

#include <ucontext.h>
#include <signal.h> // SIGBUS + SA_SIGINFO ...
#include <string.h> // strsignal
#include <execinfo.h> // backtrace + backtrace_symbols

std::string logs_filepath = "./crash_test/asd.txt";

// sa_handler handler for illegal instruction (SIGILL)
void exception_handler_SIGILL(int signal, siginfo_t *info, void *context)
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
    struct sigaction sa_SIGSEGV_prev{};
    sa_SIGSEGV_prev.sa_sigaction = exception_handler_SIGILL;
    sa_SIGSEGV_prev.sa_flags = SA_SIGINFO | SA_RESETHAND;
    sigemptyset(&sa_SIGSEGV_prev.sa_mask); // all signals unblocked
    sigaction(SIGSEGV, &sa_SIGSEGV_prev, nullptr);

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
