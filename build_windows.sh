#!/usr/bin/env bash
# Cross-compile gbe_fork for Windows (x86_64) using clang-cl + MSVC ABI.
# Requires msvc-wine Windows SDK (see cmake/toolchain-linux-winsdk.cmake).
#
# Usage: ./build_windows.sh [Release|Debug] [additional cmake args...]

set -euo pipefail

BUILD_TYPE="${1:-Release}"
shift 2>/dev/null || true
BUILD_DIR="build/windows"
JOBS="$(nproc 2>/dev/null || echo 4)"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# Ensure WINDOWS_SDK_PATH is discoverable
export WINDOWS_SDK_PATH="${WINDOWS_SDK_PATH:-/home/twig/my_msvc/opt/msvc}"

if [ ! -d "${WINDOWS_SDK_PATH}/kits/10/Include" ]; then
    echo "Error: Windows SDK not found at ${WINDOWS_SDK_PATH}"
    echo "Set WINDOWS_SDK_PATH or install msvc-wine (https://github.com/mstorsjo/msvc-wine)"
    exit 1
fi

echo "==> Configuring (${BUILD_TYPE}) for Windows x86_64..."
echo "    WINDOWS_SDK_PATH=${WINDOWS_SDK_PATH}"

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
