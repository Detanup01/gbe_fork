cmake_minimum_required(VERSION 3.24)
project(gbe VERSION 1.0.0 LANGUAGES C CXX)

# ============================================================
# Policies
# ============================================================
cmake_policy(SET CMP0135 NEW)

# ============================================================
# Options
# ============================================================
option(GBE_BUILD_EXPERIMENTAL "Build experimental overlay variant" OFF)
option(GBE_BUILD_TESTS "Build tests" OFF)
option(GBE_BUILD_TOOLS "Build tools" ON)
option(GBE_BUILD_STEAMCLIENT "Build steamclient DLL" ON)

# ============================================================
# Git info
# ============================================================
execute_process(
    COMMAND git rev-parse --short HEAD
    OUTPUT_VARIABLE GIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)
if(NOT GIT_HASH)
    set(GIT_HASH "unknown")
endif()

string(TIMESTAMP EMU_BUILD_STRING "%Y_%m_%d-%H_%M_%S")

# ============================================================
# Compiler & standard
# ============================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

# ============================================================
# Dependencies (all fetched via FetchContent)
# ============================================================
include(FetchContent)

# ---- nlohmann_json (header-only) ----
FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.12.0
)
set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(nlohmann_json)

# ---- zlib ----
set(ZLIB_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
    zlib
    GIT_REPOSITORY https://github.com/madler/zlib.git
    GIT_TAG v1.3.1
)
FetchContent_MakeAvailable(zlib)

# ---- mbedtls ----
set(MBEDTLS_VERSION "3.6.0")
FetchContent_Declare(
    mbedtls
    GIT_REPOSITORY https://github.com/Mbed-TLS/mbedtls.git
    GIT_TAG v${MBEDTLS_VERSION}
)
set(UNSAFE_BUILD ON CACHE BOOL "" FORCE)
set(ENABLE_TESTING OFF CACHE BOOL "" FORCE)
set(ENABLE_PROGRAMS OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(mbedtls)

foreach(_mbed_target IN ITEMS mbedtls mbedcrypto mbedx509 everest p256m)
    if(TARGET ${_mbed_target})
        target_compile_options(${_mbed_target} PRIVATE
            -Wno-unterminated-string-initialization
            -Wno-error
        )
    endif()
endforeach()
unset(_mbed_target)

# ---- curl (with mbedtls backend) ----
set(CURL_USE_MBEDTLS ON)
set(CURL_USE_OPENSSL OFF)
set(CURL_USE_SCHANNEL OFF)
set(CURL_USE_SECURETRANSPORT OFF)
set(CURL_USE_WOLFSSL OFF)
set(HTTP_ONLY ON)
set(BUILD_CURL_EXE OFF)
set(BUILD_TESTING OFF)
if(NOT WIN32)
    set(CURL_CA_PATH "/etc/ssl/certs")
endif()
foreach(_opt LDAP LDAPS TELNET DICT FILE FTP GOPHER IMAP MQTT POP3 RTSP SMB SMTP TFTP)
    set(CURL_DISABLE_${_opt} ON)
endforeach()

set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(MBEDTLS_INCLUDE_DIRS "${mbedtls_SOURCE_DIR}/include" CACHE STRING "" FORCE)
if(WIN32)
    set(MBEDTLS_LIBRARY "${mbedtls_BINARY_DIR}/library/mbedtls.lib" CACHE STRING "" FORCE)
    set(MBEDX509_LIBRARY "${mbedtls_BINARY_DIR}/library/mbedx509.lib" CACHE STRING "" FORCE)
    set(MBEDCRYPTO_LIBRARY "${mbedtls_BINARY_DIR}/library/mbedcrypto.lib" CACHE STRING "" FORCE)
else()
    set(MBEDTLS_LIBRARY "${mbedtls_BINARY_DIR}/library/libmbedtls.a" CACHE STRING "" FORCE)
    set(MBEDX509_LIBRARY "${mbedtls_BINARY_DIR}/library/libmbedx509.a" CACHE STRING "" FORCE)
    set(MBEDCRYPTO_LIBRARY "${mbedtls_BINARY_DIR}/library/libmbedcrypto.a" CACHE STRING "" FORCE)
endif()

# Cross-compilation workarounds for FetchContent dependencies.
if(CMAKE_CROSSCOMPILING)
    # --- curl: prevent host pkg-config / find_path from leaking
    # Linux /usr/include into clang-cl compile flags and defeating
    # /winsysroot isolation.
    foreach(_opt
        CURL_ZLIB CURL_BROTLI CURL_ZSTD
        CURL_USE_LIBPSL CURL_USE_LIBSSH2 USE_LIBIDN2
        USE_NGHTTP2 USE_NGTCP2
    )
        set(${_opt} OFF CACHE BOOL "" FORCE)
    endforeach()
    unset(_opt)
    foreach(_var
        ZLIB_INCLUDE_DIR LIBPSL_INCLUDE_DIR LIBSSH2_INCLUDE_DIR
        LIBIDN2_INCLUDE_DIR LIBIDN2_CFLAGS LIBIDN2_INCLUDEDIR
        LIBIDN2_INCLUDE_DIRS LIBIDN2_STATIC_CFLAGS
        LIBIDN2_STATIC_INCLUDE_DIRS
    )
        set(${_var} "" CACHE PATH "" FORCE)
    endforeach()
    unset(_var)

    # --- opus: disable all SIMD intrinsics for clang-cl cross-compile.
    # clang-cl reports as MSVC but needs -msse4.1 / -mavx2 per-file flags
    # that opus' CMake skips for MSVC.  Rather than patching every flag,
    # disable intrinsics entirely.
    set(OPUS_DISABLE_INTRINSICS ON CACHE BOOL "" FORCE)
endif()

FetchContent_Declare(
    curl
    GIT_REPOSITORY https://github.com/curl/curl.git
    GIT_TAG curl-8_9_1
)
FetchContent_MakeAvailable(curl)

# ---- protobuf ----
set(protobuf_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(protobuf_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(protobuf_WITH_ZLIB OFF CACHE BOOL "" FORCE)
set(protobuf_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(absl_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    protobuf
    GIT_REPOSITORY https://github.com/protocolbuffers/protobuf.git
    GIT_TAG v27.3
)

# When cross-compiling (e.g., Linux→Windows), build the protobuf libraries for the
# target but use a host-architecture protoc binary for code generation.
if(CMAKE_CROSSCOMPILING)
    set(protobuf_BUILD_PROTOC_BINARIES OFF CACHE BOOL "" FORCE)

    # Locate a host protoc from an existing native build
    find_program(_host_protoc protoc
        PATHS
            "${CMAKE_CURRENT_SOURCE_DIR}/build/linux/_deps/protobuf-build"
            "${CMAKE_CURRENT_SOURCE_DIR}/build/linux/_deps/protobuf-build/bin"
        NO_DEFAULT_PATH
    )
    if(NOT _host_protoc)
        find_program(_host_protoc protoc)
    endif()
    if(NOT _host_protoc)
        message(FATAL_ERROR
            "Cross-compiling for Windows but no host-architecture protoc found.\n"
            "Either: (1) build for Linux first, or (2) install protoc on your system."
        )
    endif()
    message(STATUS "Using host protoc: ${_host_protoc}")
    set(Protobuf_PROTOC_EXECUTABLE "${_host_protoc}" CACHE FILEPATH "Host protoc binary" FORCE)
else()
    set(protobuf_BUILD_PROTOC_BINARIES ON CACHE BOOL "" FORCE)
endif()

FetchContent_MakeAvailable(protobuf)

# When cross-compiling, create the protobuf::protoc target pointing to the host protoc.
# This is needed because protobuf_generate() uses this target as the command.
if(CMAKE_CROSSCOMPILING AND NOT TARGET protobuf::protoc)
    add_executable(protobuf::protoc IMPORTED)
    set_target_properties(protobuf::protoc PROPERTIES
        IMPORTED_LOCATION "${Protobuf_PROTOC_EXECUTABLE}"
    )
endif()

include("${protobuf_SOURCE_DIR}/cmake/protobuf-generate.cmake")

# ---- opus ----
FetchContent_Declare(
    opus
    GIT_REPOSITORY https://github.com/xiph/opus.git
    GIT_TAG v1.5.2
)
FetchContent_MakeAvailable(opus)

# Create include/opus/ → include/ so <opus/opus.h> resolves; on Linux
# this is provided by the system-installed opus at /usr/include/opus/.
if(CMAKE_CROSSCOMPILING AND NOT EXISTS "${opus_SOURCE_DIR}/include/opus")
    file(CREATE_LINK "${opus_SOURCE_DIR}/include"
                     "${opus_SOURCE_DIR}/include/opus" SYMBOLIC)
endif()

# ---- portaudio ----
set(CMAKE_POLICY_VERSION_MINIMUM "3.5")
FetchContent_Declare(
    portaudio
    GIT_REPOSITORY https://github.com/PortAudio/portaudio.git
    GIT_TAG v19.7.0
)
set(PA_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(PA_BUILD_STATIC ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(portaudio)

# ---- libssq ----
FetchContent_Declare(
    libssq
    GIT_REPOSITORY https://github.com/BinaryAlien/libssq.git
    GIT_TAG v3.0.1
)
FetchContent_MakeAvailable(libssq)

# Patch libssq for clang-cl cross-compile: trailing comma in FormatMessage()
# call (error.c:36) is invalid C before C23; clang-cl rejects it.
if(CMAKE_CROSSCOMPILING)
    set(_ssq_error "${libssq_SOURCE_DIR}/src/error.c")
    file(READ "${_ssq_error}" _ssq_content)
    string(REPLACE "        NULL,\n    );" "        NULL\n    );" _ssq_content "${_ssq_content}")
    file(WRITE "${_ssq_error}" "${_ssq_content}")
    unset(_ssq_error)
    unset(_ssq_content)
endif()

# ---- ingame_overlay (GPLv3, only for experimental) ----
if(GBE_BUILD_EXPERIMENTAL)
    FetchContent_Declare(
        ingame_overlay
        GIT_REPOSITORY https://github.com/Nemirtingas/ingame_overlay.git
        GIT_TAG 4ec39d4fd246288c98ee9ae9771e18a4b42ad681
    )
    FetchContent_MakeAvailable(ingame_overlay)
endif()

# ============================================================
# Protobuf generation
# ============================================================
set(PROTO_GEN_DIR "${CMAKE_CURRENT_BINARY_DIR}/proto_gen")
set(PROTO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/dll")

set(PROTO_SOURCES)
set(PROTO_TF2_SOURCES)

protobuf_generate(
    LANGUAGE cpp
    PROTOS "${PROTO_DIR}/net.proto"
    PROTOC_OUT_DIR "${PROTO_GEN_DIR}"
    IMPORT_DIRS "${PROTO_DIR}" "${protobuf_SOURCE_DIR}/src"
    OUT_VAR PROTO_NET_SOURCES
)

protobuf_generate(
    LANGUAGE cpp
    PROTOS "${PROTO_DIR}/gc_steam/steammessages.proto"
    PROTOC_OUT_DIR "${PROTO_GEN_DIR}"
    IMPORT_DIRS "${PROTO_DIR}/gc_steam" "${protobuf_SOURCE_DIR}/src"
    OUT_VAR PROTO_STEAM_SOURCES
)

set(PROTO_SOURCES ${PROTO_NET_SOURCES} ${PROTO_STEAM_SOURCES})

protobuf_generate(
    LANGUAGE cpp
    PROTOS
        "${PROTO_DIR}/gc_tf2/base_gcmessages.proto"
        "${PROTO_DIR}/gc_tf2/econ_gcmessages.proto"
        "${PROTO_DIR}/gc_tf2/gcsdk_gcmessages.proto"
        "${PROTO_DIR}/gc_tf2/gcsystemmsgs.proto"
        "${PROTO_DIR}/gc_tf2/tf_gcmessages.proto"
    PROTOC_OUT_DIR "${PROTO_GEN_DIR}/tf2"
    IMPORT_DIRS "${PROTO_DIR}/gc_steam" "${PROTO_DIR}/gc_tf2" "${protobuf_SOURCE_DIR}/src"
    OUT_VAR PROTO_TF2_SOURCES
)

add_library(gbe_proto OBJECT
    ${PROTO_SOURCES}
    ${PROTO_TF2_SOURCES}
)
target_include_directories(gbe_proto PUBLIC
    "${PROTO_GEN_DIR}"
    "${PROTO_GEN_DIR}/tf2"
)
target_link_libraries(gbe_proto PUBLIC protobuf::libprotobuf)

# ============================================================
# Common include paths
# ============================================================
set(GBE_INCLUDES
    "${CMAKE_CURRENT_SOURCE_DIR}/dll"
    "${PROTO_GEN_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs"
    "${CMAKE_CURRENT_SOURCE_DIR}/libs/utfcpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/helpers"
    "${CMAKE_CURRENT_SOURCE_DIR}/crash_printer"
    "${CMAKE_CURRENT_SOURCE_DIR}/sdk"
    "${CMAKE_CURRENT_SOURCE_DIR}/overlay_experimental"
)

# ============================================================
# Compile definitions
# ============================================================
set(GBE_DEFS
    UTF_CPP_CPLUSPLUS=201703L
    CURL_STATICLIB
    CONTROLLER_SUPPORT
    EMU_BUILD_STRING="${EMU_BUILD_STRING}"
    $<$<CONFIG:Debug>:DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
    $<$<CONFIG:Release>:EMU_RELEASE_BUILD>
    $<IF:$<PLATFORM_ID:Windows>,_CRT_SECURE_NO_WARNINGS,GNUC>
)

# ============================================================
# Platform detection helpers
# ============================================================
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(GBE_IS_64BIT TRUE)
else()
    set(GBE_IS_64BIT FALSE)
endif()

# ============================================================
# Common compile options
# ============================================================
set(GBE_CXX_OPTS
    $<$<CXX_COMPILER_ID:Clang>:-Wall -Wextra -Wpedantic -Wno-switch -fno-jump-tables -fno-char8_t>
    $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic -Wno-switch -fno-jump-tables -fno-char8_t>
    $<$<CXX_COMPILER_ID:MSVC>:/W4 /utf-8 /permissive- /Zc:char8_t- /EHsc /bigobj>
)

set(GBE_LINK_OPTS)
if(NOT WIN32)
    list(APPEND GBE_LINK_OPTS "-Wl,--exclude-libs,ALL")
endif()

# ============================================================
# Internal helper libraries
# ============================================================
add_library(gbe_helpers STATIC
    helpers/common_helpers.cpp
    helpers/dbg_log.cpp
)
target_include_directories(gbe_helpers PUBLIC
    helpers
    ${CMAKE_CURRENT_SOURCE_DIR}/libs
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utfcpp
    ${CMAKE_CURRENT_SOURCE_DIR}/helpers
)
target_compile_features(gbe_helpers PUBLIC cxx_std_17)

add_library(gbe_gamepad STATIC libs/gamepad/gamepad.c)
target_include_directories(gbe_gamepad PUBLIC libs)
target_compile_features(gbe_gamepad PUBLIC cxx_std_17)

# ============================================================
# Common DLL sources (wrap.cpp is platform-specific, added per-target)
# ============================================================
file(GLOB GBE_DLL_SOURCES CONFIGURE_DEPENDS
    dll/*.cpp
)
list(REMOVE_ITEM GBE_DLL_SOURCES
    "${CMAKE_CURRENT_SOURCE_DIR}/dll/wrap.cpp"
)

list(APPEND GBE_DLL_SOURCES
    $<$<NOT:$<PLATFORM_ID:Windows>>:crash_printer/linux.cpp>
    $<$<PLATFORM_ID:Windows>:crash_printer/win.cpp>
)

# ============================================================
# Common link libraries
# ============================================================
set(GBE_DEPS_LIBS
    gbe_helpers
    gbe_gamepad
    nlohmann_json::nlohmann_json
    zlibstatic
    mbedcrypto
    mbedtls
    mbedx509
    CURL::libcurl
    protobuf::libprotobuf
    opus
    portaudio_static
    ssq
    $<$<PLATFORM_ID:Windows>:ws2_32>
    $<$<PLATFORM_ID:Windows>:iphlpapi>
    $<$<PLATFORM_ID:Windows>:bcrypt>
    $<$<PLATFORM_ID:Windows>:wldap32>
    $<$<PLATFORM_ID:Windows>:winmm>
    $<$<PLATFORM_ID:Windows>:dbghelp>
    $<$<NOT:$<PLATFORM_ID:Windows>>:pthread>
    $<$<NOT:$<PLATFORM_ID:Windows>>:dl>
)

# ============================================================
# Target: steam_api (libsteam_api / steam_api / steam_api64)
# ============================================================
add_library(steam_api SHARED)

target_sources(steam_api PRIVATE
    ${GBE_DLL_SOURCES}
    $<$<NOT:$<PLATFORM_ID:Windows>>:dll/wrap.cpp>
)

target_include_directories(steam_api PRIVATE ${GBE_INCLUDES})
target_compile_definitions(steam_api PRIVATE ${GBE_DEFS})
target_compile_options(steam_api PRIVATE ${GBE_CXX_OPTS})
target_link_options(steam_api PRIVATE ${GBE_LINK_OPTS})
target_link_libraries(steam_api PRIVATE ${GBE_DEPS_LIBS} gbe_proto)

set_target_properties(steam_api PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    PREFIX ""
)

if(WIN32)
    set_target_properties(steam_api PROPERTIES
        OUTPUT_NAME "$<IF:$<BOOL:${GBE_IS_64BIT}>,steam_api64,steam_api>"
    )
else()
    set_target_properties(steam_api PROPERTIES OUTPUT_NAME "libsteam_api")
endif()

# ============================================================
# Target: steam_api_experimental (with overlay) [optional]
# ============================================================
if(GBE_BUILD_EXPERIMENTAL)
    add_library(steam_api_experimental SHARED)

    target_sources(steam_api_experimental PRIVATE
        ${GBE_DLL_SOURCES}
        $<$<NOT:$<PLATFORM_ID:Windows>>:dll/wrap.cpp>
        overlay_experimental/steam_overlay.cpp
        overlay_experimental/steam_overlay_stats.cpp
    )

    target_include_directories(steam_api_experimental PRIVATE
        ${GBE_INCLUDES}
        overlay_experimental
    )

    target_compile_definitions(steam_api_experimental PRIVATE
        ${GBE_DEFS}
        EMU_OVERLAY
        ImTextureID=ImU64
        EMU_EXPERIMENTAL_BUILD
    )

    target_compile_options(steam_api_experimental PRIVATE ${GBE_CXX_OPTS})
    target_link_options(steam_api_experimental PRIVATE ${GBE_LINK_OPTS})

    target_link_libraries(steam_api_experimental PRIVATE
        ${GBE_DEPS_LIBS}
        gbe_proto
        ingame_overlay
        $<$<NOT:$<PLATFORM_ID:Windows>>:X11>
    )

    set_target_properties(steam_api_experimental PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        PREFIX ""
    )

    if(WIN32)
        set_target_properties(steam_api_experimental PROPERTIES
            OUTPUT_NAME "$<IF:$<BOOL:${GBE_IS_64BIT}>,steam_api64,steam_api>"
        )
    else()
        set_target_properties(steam_api_experimental PROPERTIES OUTPUT_NAME "libsteam_api")
    endif()
endif()

# ============================================================
# Target: steamclient (steamclient / steamclient64)
# ============================================================
if(GBE_BUILD_STEAMCLIENT)
    add_library(steamclient SHARED)

    target_sources(steamclient PRIVATE
        ${GBE_DLL_SOURCES}
    )

    if(WIN32 AND NOT WINDOWS_CROSSCOMPILE)
        target_sources(steamclient PRIVATE steamclient/steamclient.cpp)
    endif()

    target_include_directories(steamclient PRIVATE ${GBE_INCLUDES})
    target_compile_definitions(steamclient PRIVATE
        ${GBE_DEFS}
        STEAMCLIENT_DLL
    )
    target_compile_options(steamclient PRIVATE ${GBE_CXX_OPTS})
    target_link_options(steamclient PRIVATE ${GBE_LINK_OPTS})
    target_link_libraries(steamclient PRIVATE ${GBE_DEPS_LIBS} gbe_proto)

    set_target_properties(steamclient PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        PREFIX ""
    )

    if(WIN32)
        set_target_properties(steamclient PROPERTIES
            OUTPUT_NAME "$<IF:$<BOOL:${GBE_IS_64BIT}>,steamclient64,steamclient>"
        )
    else()
        set_target_properties(steamclient PROPERTIES OUTPUT_NAME "steamclient")
    endif()
endif()

# ============================================================
# Target: steamnetworkingsockets
# ============================================================
add_library(steamnetworkingsockets SHARED
    networking_sockets_lib/steamnetworkingsockets.cpp
    helpers/dbg_log.cpp
    helpers/common_helpers.cpp
)
target_include_directories(steamnetworkingsockets PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/helpers
    ${CMAKE_CURRENT_SOURCE_DIR}/libs
    ${CMAKE_CURRENT_SOURCE_DIR}/libs/utfcpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dll
    ${CMAKE_CURRENT_SOURCE_DIR}/sdk
)
target_compile_features(steamnetworkingsockets PUBLIC cxx_std_17)
target_compile_options(steamnetworkingsockets PRIVATE ${GBE_CXX_OPTS})
set_target_properties(steamnetworkingsockets PROPERTIES
    OUTPUT_NAME "libsteamnetworkingsockets"
    PREFIX ""
)

# ============================================================
# Target: gameoverlayrenderer (stub)
# ============================================================
add_library(gameoverlayrenderer SHARED
    game_overlay_renderer_lib/game_overlay_renderer_lib.cpp
    game_overlay_renderer_lib/game_overlay_renderer_lib.h
    dll/dll/common_includes.h
)
target_include_directories(gameoverlayrenderer PRIVATE
    ${GBE_INCLUDES}
)
target_link_libraries(gameoverlayrenderer PRIVATE gbe_proto CURL::libcurl)
set_target_properties(gameoverlayrenderer PROPERTIES PREFIX "")
if(WIN32)
    set_target_properties(gameoverlayrenderer PROPERTIES
        OUTPUT_NAME "$<IF:$<BOOL:${GBE_IS_64BIT}>,GameOverlayRenderer64,GameOverlayRenderer>"
    )
endif()

# ============================================================
# Tools
# ============================================================
if(GBE_BUILD_TOOLS)
    add_executable(lobby_connect
        tools/lobby_connect/lobby_connect.cpp
    )
    target_include_directories(lobby_connect PRIVATE ${GBE_INCLUDES})
    target_compile_definitions(lobby_connect PRIVATE
        ${GBE_DEFS}
        NO_DISK_WRITES
        LOBBY_CONNECT
    )
    target_compile_options(lobby_connect PRIVATE -UCONTROLLER_SUPPORT)
    target_compile_options(lobby_connect PRIVATE ${GBE_CXX_OPTS})
    target_link_libraries(lobby_connect PRIVATE
        steam_api
        CURL::libcurl
        $<$<PLATFORM_ID:Windows>:comdlg32>
    )

    add_executable(generate_interfaces
        tools/generate_interfaces/generate_interfaces.cpp
        helpers/common_helpers.cpp
    )
    target_include_directories(generate_interfaces PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/helpers
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/utfcpp
        ${CMAKE_CURRENT_SOURCE_DIR}/libs
    )
    target_compile_features(generate_interfaces PRIVATE cxx_std_17)
endif()

# ============================================================
# Tests
# ============================================================
if(GBE_BUILD_TESTS AND NOT WIN32)
    add_executable(test_crash_printer_sa_handler
        crash_printer/linux.cpp
        helpers/common_helpers.cpp
        crash_printer/tests/test_linux_sa_handler.cpp
    )
    target_include_directories(test_crash_printer_sa_handler PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/crash_printer
        ${CMAKE_CURRENT_SOURCE_DIR}/helpers
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/utfcpp
    )
    target_compile_features(test_crash_printer_sa_handler PRIVATE cxx_std_17)
    target_link_libraries(test_crash_printer_sa_handler PRIVATE dl)

    add_executable(test_crash_printer_sa_sigaction
        crash_printer/linux.cpp
        helpers/common_helpers.cpp
        crash_printer/tests/test_linux_sa_sigaction.cpp
    )
    target_include_directories(test_crash_printer_sa_sigaction PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/crash_printer
        ${CMAKE_CURRENT_SOURCE_DIR}/helpers
        ${CMAKE_CURRENT_SOURCE_DIR}/libs/utfcpp
    )
    target_compile_features(test_crash_printer_sa_sigaction PRIVATE cxx_std_17)
    target_link_libraries(test_crash_printer_sa_sigaction PRIVATE dl)

    add_executable(test_gamepad_linux
        libs/gamepad/gamepad.c
        tests/gamepad/test_gamepad_linux.cpp
    )
    target_include_directories(test_gamepad_linux PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/libs)
    target_compile_definitions(test_gamepad_linux PRIVATE GAMEPAD_TESTING)
    target_compile_features(test_gamepad_linux PRIVATE cxx_std_17)
endif()
