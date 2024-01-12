#include "common_helpers/common_helpers.hpp"
#include "crash_printer/win.hpp"

#include <sstream>
#include <chrono>
#include <ctime>
#include <vector>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <DbgHelp.h>

static LPTOP_LEVEL_EXCEPTION_FILTER originalExceptionFilter = nullptr;

static std::wstring logs_filepath{};

static void print_stacktrace(std::ofstream &file, CONTEXT* context) {
    auto this_proc = GetCurrentProcess();
    auto this_thread = GetCurrentThread();

    if (!SymInitialize(this_proc, NULL, TRUE)) {
        return;
    }

    STACKFRAME stack_frame{};

#ifdef _WIN64
    constexpr DWORD machine_type = IMAGE_FILE_MACHINE_AMD64;
    DWORD64 symbol_displacement = 0;

    stack_frame.AddrPC.Offset = context->Rip;
    stack_frame.AddrFrame.Offset = context->Rsp;
    stack_frame.AddrStack.Offset = context->Rsp;
#else
    constexpr DWORD machine_type = IMAGE_FILE_MACHINE_I386;
    DWORD symbol_displacement = 0;

    stack_frame.AddrPC.Offset = context->Eip;
    stack_frame.AddrFrame.Offset = context->Esp; // EBP may not be used by module
    stack_frame.AddrStack.Offset = context->Esp;
#endif

    stack_frame.AddrPC.Mode = ADDRESS_MODE::AddrModeFlat;
    stack_frame.AddrFrame.Mode = ADDRESS_MODE::AddrModeFlat;
    stack_frame.AddrStack.Mode = ADDRESS_MODE::AddrModeFlat;
    
    common_helpers::write(file, "*********** Stack trace ***********");
    std::vector<std::string> symbols{};
    while (true) {
        BOOL res = StackWalk(
            machine_type,
            this_proc, this_thread,
            &stack_frame,
            context,
            NULL, SymFunctionTableAccess, SymGetModuleBase, NULL);
        if (!res) {
            break;
        }
        
        bool print_symbol = false;
        IMAGEHLP_SYMBOL *symbol = (IMAGEHLP_SYMBOL *)_malloca(sizeof(IMAGEHLP_SYMBOL) + MAX_PATH);
        if (symbol) {
            symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
            symbol->MaxNameLength = MAX_PATH - 1; // "in characters, not including the null-terminating character"

            symbol_displacement = 0;

            if (SymGetSymFromAddr(this_proc, stack_frame.AddrPC.Offset, &symbol_displacement, symbol)) {
                print_symbol = true;
            }
        }

        std::stringstream ss{};
        ss << "0x" << std::hex << (void *)stack_frame.AddrPC.Offset << " | ";
        if (print_symbol) {
            ss << symbol->Name;
        } else {
            ss << "<unknown function>";
        }
        symbols.push_back(ss.str());

        if (symbol) {
            _freea(symbol);
        }
    }

    SymCleanup(this_proc);

    auto idx = symbols.size();
    for (const auto &s : symbols) {
        std::stringstream ss{};
        ss << "[frame " << std::dec << (idx - 1) << "]: "
           << s;
        common_helpers::write(file, ss.str());
        idx--;
    }
}

static void log_exception(LPEXCEPTION_POINTERS ex_pointers)
{
    if (!common_helpers::create_dir(logs_filepath)) {
        return;
    }
    
    std::ofstream file(logs_filepath, std::ios::app);

    auto now = std::chrono::system_clock::now();
    auto t_now = std::chrono::system_clock::to_time_t(now);
    auto gm_time = std::gmtime(&t_now);
    auto time = std::string(std::asctime(gm_time));
    time.pop_back(); // remove the trailing '\n' added by asctime
    common_helpers::write(file, "[" + time + "]");
    {
        std::stringstream ss{};
        ss << "Unhandled exception:" << std::endl
           << "  code: 0x" << std::hex << ex_pointers->ExceptionRecord->ExceptionCode << std::endl
           << "  @address = 0x" << std::hex << ex_pointers->ExceptionRecord->ExceptionAddress;
        common_helpers::write(file, ss.str());
    }

    print_stacktrace(file, ex_pointers->ContextRecord);
    common_helpers::write(file, "**********************************\n");
    file.close();
}

static LONG WINAPI exception_handler(LPEXCEPTION_POINTERS ex_pointers)
{
    log_exception(ex_pointers);

    if (originalExceptionFilter) {
        return originalExceptionFilter(ex_pointers);
    }

    return EXCEPTION_CONTINUE_SEARCH;
}


bool crash_printer::init(const std::wstring &log_file)
{
    logs_filepath = log_file;
    originalExceptionFilter = SetUnhandledExceptionFilter(exception_handler);
    return true;
}

void crash_printer::deinit()
{
    if (originalExceptionFilter) {
        SetUnhandledExceptionFilter(originalExceptionFilter);
    }
}
