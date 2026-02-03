#if defined(__linux__) || defined(linux) || defined(GNUC) || defined(__MINGW32__) || defined(__MINGW64__)
    // https://gcc.gnu.org/onlinedocs/gcc/Diagnostic-Pragmas.html
    // https://gcc.gnu.org/onlinedocs/gcc/Warning-Options.html#index-Wall
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wall"
    
    #include "mbedtls/pk.h"
    #include "mbedtls/x509.h"
    #include "mbedtls/error.h"
    #include "mbedtls/sha1.h"
    #include "mbedtls/entropy.h"
    #include "mbedtls/ctr_drbg.h"

    #pragma GCC diagnostic pop
#else
    // https://learn.microsoft.com/en-us/cpp/preprocessor/warning?view=msvc-170#push-and-pop
    #pragma warning(push, 0)

    #include "mbedtls/pk.h"
    #include "mbedtls/x509.h"
    #include "mbedtls/error.h"
    #include "mbedtls/sha1.h"
    #include "mbedtls/entropy.h"
    #include "mbedtls/ctr_drbg.h"

    #pragma warning(pop)
#endif
