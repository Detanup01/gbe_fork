// https://stackoverflow.com/a/1925461

#include "common_helpers/common_helpers.hpp"
#include "crash_printer/linux.hpp"

#include <sstream>
#include <chrono>
#include <ctime>

#include <ucontext.h>
#include <signal.h> // SIGBUS + SA_SIGINFO ...
#include <string.h> // strsignal
#include <execinfo.h> // backtrace + backtrace_symbols

constexpr static const int max_stack_frames = 50;

static bool old_SIGILL = false;
static struct sigaction oldact_SIGILL{};

static bool old_SIGSEGV = false;
static struct sigaction oldact_SIGSEGV{};

static bool old_SIGBUS = false;
static struct sigaction oldact_SIGBUS{};

static std::string logs_filepath{};

static void restore_handlers()
{
    if (old_SIGILL) {
        old_SIGILL = false;
        sigaction(SIGILL, &oldact_SIGILL, nullptr);
    }

    if (old_SIGSEGV) {
        old_SIGSEGV = false;
        sigaction(SIGSEGV, &oldact_SIGSEGV, nullptr);
    }

    if (old_SIGBUS) {
        old_SIGBUS = false;
        sigaction(SIGBUS, &oldact_SIGBUS, nullptr);
    }

}

static void exception_handler(int signal, siginfo_t *info, void *context, struct sigaction *oldact)
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
           << "  code: " << std::dec << signal << " (" << strsignal(signal) << ")" << std::endl;
        common_helpers::write(file, ss.str());
    }
    void* stack_frames[max_stack_frames];
    int stack_size = backtrace(stack_frames, max_stack_frames);
    char** stack_symbols = backtrace_symbols(stack_frames, stack_size);

    if (stack_symbols != nullptr) {
        // fprintf(stderr, "Stack trace:\n");
        common_helpers::write(file, "*********** Stack trace ***********");
        for (int i = 1; i < stack_size; ++i) {
            char *symbol = stack_symbols[i];
            std::stringstream ss{};
            ss << "[frame " << std::dec << (stack_size - i - 1) << "]: "
                << std::hex << stack_frames[i] << " | "
                << symbol;
            common_helpers::write(file, ss.str());
        }

        free(stack_symbols);
    }

    common_helpers::write(file, "**********************************\n");

    file.close();
}

// Register the signal handler for illegal instruction (SIGILL)
static void exception_handler_SIGILL(int signal, siginfo_t *info, void *context) {
    exception_handler(signal, info, context, &oldact_SIGILL);
    sigaction(SIGILL, &oldact_SIGILL, nullptr);
}

static void exception_handler_SIGSEGV(int signal, siginfo_t *info, void *context) {
    exception_handler(signal, info, context, &oldact_SIGSEGV);
    sigaction(SIGSEGV, &oldact_SIGSEGV, nullptr);
}

static void exception_handler_SIGBUS(int signal, siginfo_t *info, void *context) {
    exception_handler(signal, info, context, &oldact_SIGBUS);
    sigaction(SIGBUS, &oldact_SIGBUS, nullptr);
}


bool crash_printer::init(const std::string &log_file)
{
    logs_filepath = log_file;

    // save old handlers
    // https://linux.die.net/man/2/sigaction
    if (
        sigaction(SIGILL, nullptr, &oldact_SIGILL) != 0 ||
        sigaction(SIGSEGV, nullptr, &oldact_SIGSEGV) != 0 ||
        sigaction(SIGBUS, nullptr, &oldact_SIGBUS) != 0) {
        return false;
    }

    constexpr int sa_flags = SA_RESTART | SA_SIGINFO | SA_ONSTACK | SA_RESETHAND | SA_NOCLDSTOP;

    // https://linux.die.net/man/2/sigaction
    // register handler for illegal instruction (SIGILL)
    struct sigaction sa_SIGILL{};
    sa_SIGILL.sa_sigaction = exception_handler_SIGILL;
    sa_SIGILL.sa_flags = sa_flags;
    sigemptyset(&sa_SIGILL.sa_mask); // all signals unblocked
    if (sigaction(SIGILL, &sa_SIGILL, nullptr) != 0) {
        restore_handlers();
        return false;
    }
    old_SIGILL = true;

    // register handler for segmentation fault (SIGSEGV)
    struct sigaction sa_SIGSEGV{};
    sa_SIGSEGV.sa_sigaction = exception_handler_SIGSEGV;
    sa_SIGSEGV.sa_flags = sa_flags;
    sigemptyset(&sa_SIGSEGV.sa_mask); // all signals unblocked
    if (sigaction(SIGSEGV, &sa_SIGSEGV, nullptr) != 0) {
        restore_handlers();
        return false;
    }
    old_SIGSEGV = true;

    // register handler for bus error (SIGBUS)
    struct sigaction sa_SIGBUS{};
    sa_SIGBUS.sa_sigaction = exception_handler_SIGBUS;
    sa_SIGBUS.sa_flags = sa_flags;
    sigemptyset(&sa_SIGBUS.sa_mask); // all signals unblocked
    if (sigaction(SIGBUS, &sa_SIGBUS, nullptr) != 0) {
        restore_handlers();
        return false;
    }
    old_SIGBUS = true;

    // // register handler for floating-point exception (SIGFPE)
    // if (sigaction(SIGFPE, &sa, nullptr) != 0) {
    //     perror("Error setting up signal handler");
    //     return EXIT_FAILURE;
    // }

    return true;
}

void crash_printer::deinit()
{
    restore_handlers();
}
