# Cross-compilation toolchain: Linux -> Windows (x86_64) using clang-cl + MSVC ABI.
# Requires a Windows SDK installed via msvc-wine (https://github.com/mstorsjo/msvc-wine).
#
# Usage: cmake -S . -B build/windows -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-linux-winsdk.cmake
#
# The Windows SDK path is resolved in this order:
#   1. -DWINDOWS_SDK_PATH=/path at configure time
#   2. $WINDOWS_SDK_PATH environment variable
#   3. ../../my_msvc/opt/msvc  (relative to project root)
#   4. /opt/msvc               (fallback)

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR AMD64)

# Always cross-compiling with this toolchain
set(WINDOWS_CROSSCOMPILE TRUE CACHE INTERNAL "Cross-compiling for Windows")

set(CMAKE_C_COMPILER clang-cl)
set(CMAKE_CXX_COMPILER clang-cl)
set(CMAKE_LINKER lld-link)

set(CMAKE_C_COMPILER_FRONTEND_VARIANT MSVC)
set(CMAKE_CXX_COMPILER_FRONTEND_VARIANT MSVC)

set(CMAKE_RC_COMPILER llvm-rc)

# Skip linking during compiler detection (avoids llvm-rc manifest issue in try_compile)
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

# --- locate Windows SDK ---
set(WINDOWS_SDK_PATH "" CACHE PATH "Path to Windows SDK root (installed by msvc-wine)")
set(CMAKE_TRY_COMPILE_PLATFORM_VARIABLES WINDOWS_SDK_PATH)

if(NOT WINDOWS_SDK_PATH)
    if(DEFINED ENV{WINDOWS_SDK_PATH})
        set(WINDOWS_SDK_PATH $ENV{WINDOWS_SDK_PATH})
    else()
        # Try relative to project root (my_msvc in home), then /opt/msvc
        get_filename_component(_proj_root "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
        set(_candidate "${_proj_root}/../../my_msvc/opt/msvc")
        get_filename_component(_candidate "${_candidate}" ABSOLUTE)
        if(NOT IS_DIRECTORY "${_candidate}/kits/10/Include")
            set(_candidate /opt/msvc)
        endif()
        message(STATUS "WINDOWS_SDK_PATH fallback: ${_candidate}")
        set(WINDOWS_SDK_PATH ${_candidate})
    endif()
endif()

message(STATUS "WINDOWS_SDK_PATH=${WINDOWS_SDK_PATH}")

if(NOT EXISTS "${WINDOWS_SDK_PATH}/kits/10/Include")
    message(FATAL_ERROR "Invalid WINDOWS_SDK_PATH: ${WINDOWS_SDK_PATH} (missing kits/10/Include)")
endif()

# Pick the newest SDK version
file(GLOB _winsdk_versions LIST_DIRECTORIES true
     "${WINDOWS_SDK_PATH}/kits/10/Include/*")
if(NOT _winsdk_versions)
    message(FATAL_ERROR "No Windows SDK versions found in ${WINDOWS_SDK_PATH}/kits/10/Include")
endif()
list(GET _winsdk_versions 0 _winsdk_dir)
get_filename_component(_winsdk_ver "${_winsdk_dir}" NAME)
message(STATUS "Windows SDK version: ${_winsdk_ver}")

# clang-cl flags
add_compile_options(
    --target=x86_64-pc-windows-msvc
    -fms-compatibility
    -fms-extensions
    -fdelayed-template-parsing
    -Wno-c++98-compat
    -Wno-c++98-compat-pedantic
    -Wno-unsafe-buffer-usage
    -Wno-padded
    -Wno-unused-command-line-argument
    -Wno-nonportable-system-include-path
    -Wno-unsafe-buffer-usage-in-libc-call
    -Wno-zero-as-null-pointer-constant
    -Wno-invalid-offsetof
    -Wno-implicit-int-conversion
    -Wno-reserved-macro-identifier
    -Wno-reserved-identifier
    -Wno-extra-semi-stmt
    -Wno-implicit-void-ptr-cast
    -Wno-sign-conversion
    -Wno-documentation-unknown-command
    -Wno-switch-enum
    -Wno-non-virtual-dtor
    -Wno-signed-enum-bitfield
    -Wno-microsoft-enum-value
    -Wno-undef
    -Wno-documentation
    -Wno-old-style-cast
    -Wno-format-signedness
    -Wno-suggest-override
    -Wno-language-extension-token
    /winsysroot ${WINDOWS_SDK_PATH}
)

add_link_options(/winsysroot:${WINDOWS_SDK_PATH})

# --- locate MSVC toolchain version ---
file(GLOB _msvc_versions LIST_DIRECTORIES true
     "${WINDOWS_SDK_PATH}/VC/Tools/MSVC/*")
if(NOT _msvc_versions)
    message(FATAL_ERROR "No MSVC toolchain found in ${WINDOWS_SDK_PATH}/VC/Tools/MSVC")
endif()
list(GET _msvc_versions 0 _msvc_dir)
get_filename_component(_msvc_ver "${_msvc_dir}" NAME)
message(STATUS "MSVC toolchain version: ${_msvc_ver}")

# llvm-rc flags: must explicitly pass Windows SDK include paths
set(CMAKE_RC_FLAGS "${CMAKE_RC_FLAGS} \
    -I${WINDOWS_SDK_PATH}/kits/10/Include/${_winsdk_ver}/shared \
    -I${WINDOWS_SDK_PATH}/kits/10/Include/${_winsdk_ver}/um \
    -I${WINDOWS_SDK_PATH}/kits/10/Include/${_winsdk_ver}/ucrt \
    -I${WINDOWS_SDK_PATH}/VC/Tools/MSVC/${_msvc_ver}/include \
" CACHE STRING "Resource compiler flags" FORCE)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
