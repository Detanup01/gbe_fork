#ifndef __INCLUDED_OS_DETECTOR__
#define __INCLUDED_OS_DETECTOR__


#if defined(WIN64) || defined(_WIN64) || defined(__MINGW64__)
    #define __WINDOWS_64__
#elif defined(WIN32) || defined(_WIN32) || defined(__MINGW32__)
    #define __WINDOWS_32__
#endif

#if defined(__WINDOWS_32__) || defined(__WINDOWS_64__)
    #define __WINDOWS__
#endif

#if defined(__linux__) || defined(linux)
    #if defined(__x86_64__)
        #define __LINUX_64__
    #else
        #define __LINUX_32__
    #endif
#endif

#if defined(__LINUX_32__) || defined(__LINUX_64__)
    #define __LINUX__
#endif

// https://sourceforge.net/p/predef/wiki/OperatingSystems/
#if defined(__APPLE__) || defined(macintosh) || defined(Macintosh) || defined(__MACH__)
    #define __MACOS__
#endif


#endif
