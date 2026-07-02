#!/usr/bin/env bash
# Build gbe_fork for Linux using CMake, Clang, and Ninja.
# Dependencies are fetched automatically via FetchContent.
#
# Usage: ./build_linux.sh [Release|Debug] [additional cmake args...]

set -euo pipefail

BUILD_TYPE="${1:-Release}"
shift 2>/dev/null || true
BUILD_DIR="build/linux"
JOBS="$(nproc 2>/dev/null || echo 4)"

echo "==> Configuring (${BUILD_TYPE})..."
cmake -S . -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE="${BUILD_TYPE}" \
    -DCMAKE_C_COMPILER=clang \
    -DCMAKE_CXX_COMPILER=clang++ \
    -DGBE_BUILD_TOOLS=ON \
    -DGBE_BUILD_TESTS=OFF \
    -DGBE_BUILD_STEAMCLIENT=ON \
    -DGBE_BUILD_EXPERIMENTAL=OFF \
    -G Ninja \
    "$@"

echo "==> Building..."
cmake --build "${BUILD_DIR}" -j "${JOBS}"

echo "==> Done! Output in ${BUILD_DIR}/"
