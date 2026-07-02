#!/usr/bin/env bash
# Cross-compile gbe_fork for Windows (x86_64) using clang-cl + MSVC ABI.
# Requires msvc-wine Windows SDK (see cmake/toolchain-linux-winsdk.cmake).
#
# Usage: ./build_windows.sh [Release|Debug] [additional cmake args...]

set -euo pipefail

BUILD_TYPE="${1:-Release}"
shift 2>/dev/null || true
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build/windows"
JOBS="$(nproc 2>/dev/null || echo 4)"

# Ensure WINDOWS_SDK_PATH is discoverable
export WINDOWS_SDK_PATH="${WINDOWS_SDK_PATH:-/home/twig/my_msvc/opt/msvc}"

if [ ! -d "${WINDOWS_SDK_PATH}/kits/10/Include" ]; then
    echo "Error: Windows SDK not found at ${WINDOWS_SDK_PATH}"
    echo "Set WINDOWS_SDK_PATH or install msvc-wine (https://github.com/mstorsjo/msvc-wine)"
    exit 1
fi

echo "==> Configuring (${BUILD_TYPE}) for Windows x86_64..."
echo "    WINDOWS_SDK_PATH=${WINDOWS_SDK_PATH}"

# Create case-insensitive symlinks in the Windows SDK for .lib files
# so that lld-link (case-sensitive on Linux) can find e.g. XINPUT9_1_0.lib
# when the original file is Xinput9_1_0.lib.
# msvc-wine's install.sh creates lowercase symlinks but not uppercase ones,
# and #pragma comment(lib, "...") often uses ALL CAPS.
SDK_LIB_DIRS=$(find "${WINDOWS_SDK_PATH}/kits/10/Lib" -type d -name "x64" 2>/dev/null || true)
for libdir in ${SDK_LIB_DIRS}; do
    for f in "${libdir}"/*.lib; do
        [ -f "$f" ] || continue
        base=$(basename "$f")
        stem=${base%.*}
        ext=${base##*.}
        # Create uppercase-stem symlink (e.g. XINPUT9_1_0.lib -> Xinput9_1_0.lib)
        upper_stem=$(echo "$stem" | tr '[:lower:]' '[:upper:]')
        upper="${upper_stem}.${ext}"
        if [ "$base" != "$upper" ] && [ ! -e "${libdir}/${upper}" ]; then
            ln -sf "$base" "${libdir}/${upper}" 2>/dev/null || true
        fi
        # Also create fully-uppercase variant (e.g. XINPUT9_1_0.LIB)
        upper_all=$(echo "$base" | tr '[:lower:]' '[:upper:]')
        if [ "$base" != "$upper_all" ] && [ ! -e "${libdir}/${upper_all}" ]; then
            ln -sf "$base" "${libdir}/${upper_all}" 2>/dev/null || true
        fi
    done
done

cmake -S "${SCRIPT_DIR}" -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DCMAKE_TOOLCHAIN_FILE="${SCRIPT_DIR}/cmake/toolchain-linux-winsdk.cmake" \
    -DWINDOWS_SDK_PATH="${WINDOWS_SDK_PATH}" \
    -DGBE_BUILD_TOOLS=ON \
    -DGBE_BUILD_TESTS=OFF \
    -DGBE_BUILD_STEAMCLIENT=ON \
    -DGBE_BUILD_EXPERIMENTAL=OFF \
    -G Ninja \
    "$@"

echo "==> Building..."
cmake --build "${BUILD_DIR}" -j "${JOBS}"

echo "==> Done! Output in ${BUILD_DIR}/"
