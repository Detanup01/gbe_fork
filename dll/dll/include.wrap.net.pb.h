
#if defined(__linux__) || defined(linux) || defined(GNUC) || defined(__MINGW32__) || defined(__MINGW64__)
    // https://gcc.gnu.org/onlinedocs/gcc/Diagnostic-Pragmas.html
    // https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html#index-Wall
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wall"
    #include "net.pb.h"
    #pragma GCC diagnostic pop
#else
    // https://learn.microsoft.com/en-us/cpp/preprocessor/warning?view=msvc-170#push-and-pop
    #pragma warning(push, 0)
    #include "net.pb.h"
    #pragma warning(pop)
#endif
