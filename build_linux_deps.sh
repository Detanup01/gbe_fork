#!/usr/bin/env bash


############## helpful commands ##############
### to read header of .elf file
#readelf -h install32/lib/libcurl.a

### list all options availble in a CMAKE project, everything prefixed with CMAKE_ are specific to CMAKE system
#cmake -LAH

### list all exports in a shared lib
# https://linux.die.net/man/1/nm
#nm -D --defined-only libsteam.so | grep " T "


if [ "$(id -u)" -ne 0 ]; then
  echo "Please run as root" >&2
  exit 1
fi

# common stuff
script_dir=$( cd -- "$( dirname -- "${0}" )" &> /dev/null && pwd )
deps_dir="$script_dir/build/deps/linux"
third_party_dir="$script_dir/third-party"
third_party_deps_dir="$third_party_dir/deps/linux"
third_party_common_dir="$third_party_dir/deps/common"
mycmake="$third_party_deps_dir/cmake/bin/cmake"

deps_archives=(
  "libssq/libssq.tar.gz"
  "zlib/zlib.tar.gz"
  "curl/curl.tar.gz"
  "protobuf/protobuf.tar.gz"
  "mbedtls/mbedtls.tar.gz"
)

# < 0: deduce, > 1: force
PARALLEL_THREADS_OVERRIDE=-1
VERBOSITY=''
INSTALL_PACKAGES_ONLY=0

for (( i=1; i<=$#; i++ )); do
  var="${!i}"
  if [[ "$var" = "-j" ]]; then
    i=$((i+1))
    PARALLEL_THREADS_OVERRIDE="${!i}"
    [[ "$PARALLEL_THREADS_OVERRIDE" =~ ^[0-9]+$ ]] || {
      echo "[X] Invalid arg after -j, expected a number" >&2;
      exit 1;
    }
    #echo "[?] Overriding parralel build jobs count with $PARALLEL_THREADS_OVERRIDE"
  elif [[ "$var" = "-verbose" ]]; then
    VERBOSITY='-v'
  elif [[ "$var" = "-packages_only" ]]; then
    INSTALL_PACKAGES_ONLY=1
  else
    echo "[X] Invalid arg: $var" >&2
    exit 1
  fi
done


last_code=0


############## required packages ##############
echo // installing required packages
apt update -y
last_code=$((last_code + $?))
apt install coreutils -y # echo, printf, etc...
last_code=$((last_code + $?))
apt install build-essential -y
last_code=$((last_code + $?))
apt install gcc-multilib g++-multilib -y # needed for 32-bit builds
last_code=$((last_code + $?))
apt install clang -y
last_code=$((last_code + $?))
apt install binutils -y # (optional) contains the tool 'readelf' mainly, and other usefull binary stuff
last_code=$((last_code + $?))
apt install tar -y # we need to extract packages
last_code=$((last_code + $?))
#apt install cmake git wget unzip -y

# exit early if we should install packages only, used by CI mainly
[[ "$INSTALL_PACKAGES_ONLY" -ne 0 ]] && exit $last_code

echo; echo;


[[ -f "$mycmake" ]] || {
  echo "[X] Couldn't find cmake" >&2;
  exit 1;
}

# use 70%
build_threads="$(( $(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 0) * 70 / 100 ))"
[[ $PARALLEL_THREADS_OVERRIDE -gt 0 ]] && build_threads="$PARALLEL_THREADS_OVERRIDE"
[[ $build_threads -lt 2 ]] && build_threads=2

echo ===========================
echo Tools will be installed in: "$deps_dir"
echo Building with $build_threads threads
echo ===========================
echo // Recreating dir...
rm -r -f "$deps_dir"
sleep 1
mkdir -p "$deps_dir" || {
  echo "Couldn't create dir \"$deps_dir\"" >&2;
  exit 1;
}

echo; echo 


############## copy CMAKE toolchains to the home directory
echo // creating CMAKE toolchains in "$deps_dir"
cat > "$deps_dir/32-bit-toolchain.cmake" << EOL
# cmake -G "xx" ... -DCMAKE_TOOLCHAIN_FILE=/.../64bit.toolchain
# https://github.com/google/boringssl/blob/master/util/32-bit-toolchain.cmake
# https://cmake.org/cmake/help/book/mastering-cmake/chapter/Cross%20Compiling%20With%20CMake.html#toolchain-files
# https://cmake.org/cmake/help/book/mastering-cmake/chapter/Cross%20Compiling%20With%20CMake.html#cross-compiling-for-a-microcontroller
# the name of the target operating system
set(CMAKE_SYSTEM_NAME Linux)

# which compilers to use for C and C++
set(CMAKE_C_COMPILER   clang)
set(CMAKE_CXX_COMPILER clang++)

set(CMAKE_C_FLAGS_INIT   "-m32")
set(CMAKE_CXX_FLAGS_INIT "-m32")

EOL

cat > "$deps_dir/64-bit-toolchain.cmake" << EOL
# cmake -G "xx" ... -DCMAKE_TOOLCHAIN_FILE=/.../64bit.toolchain
# https://github.com/google/boringssl/blob/master/util/32-bit-toolchain.cmake
# https://cmake.org/cmake/help/book/mastering-cmake/chapter/Cross%20Compiling%20With%20CMake.html#toolchain-files
# https://cmake.org/cmake/help/book/mastering-cmake/chapter/Cross%20Compiling%20With%20CMake.html#cross-compiling-for-a-microcontroller
# the name of the target operating system
set(CMAKE_SYSTEM_NAME Linux)

# which compilers to use for C and C++
set(CMAKE_C_COMPILER   clang)
set(CMAKE_CXX_COMPILER clang++)

EOL

echo; echo;

############## common CMAKE args ##############
# https://cmake.org/cmake/help/latest/variable/CMAKE_LANG_FLAGS_CONFIG.html#variable:CMAKE_%3CLANG%3E_FLAGS_%3CCONFIG%3E
cmake_common_args='-G "Unix Makefiles" -S .'
cmake_common_defs="-DCMAKE_BUILD_TYPE=Release -DCMAKE_C_STANDARD_REQUIRED=ON -DCMAKE_CXX_STANDARD_REQUIRED=ON -DCMAKE_C_STANDARD=17 -DCMAKE_CXX_STANDARD=17 -DCMAKE_POSITION_INDEPENDENT_CODE=True -DBUILD_SHARED_LIBS=OFF"
recreate_32="rm -f -r build32/ && rm -f -r install32/ && mkdir build32/ && mkdir install32/"
recreate_64="rm -f -r build64/ && rm -f -r install64/ && mkdir build64/ && mkdir install64/"
cmake_gen32="'$mycmake' $cmake_common_args -B build32 -DCMAKE_TOOLCHAIN_FILE=$deps_dir/32-bit-toolchain.cmake -DCMAKE_INSTALL_PREFIX=install32 $cmake_common_defs"
cmake_gen64="'$mycmake' $cmake_common_args -B build64 -DCMAKE_TOOLCHAIN_FILE=$deps_dir/64-bit-toolchain.cmake -DCMAKE_INSTALL_PREFIX=install64 $cmake_common_defs"
cmake_build32="'$mycmake' --build build32 --config Release --parallel $build_threads $VERBOSITY"
cmake_build64="'$mycmake' --build build64 --config Release --parallel $build_threads $VERBOSITY"
clean_gen32="[[ -d build32 ]] && rm -f -r build32/"
clean_gen64="[[ -d build64 ]] && rm -f -r build64/"

chmod 777 "$mycmake"


# the artificial delays "sleep 3" are here because on Windows WSL the
# explorer or search indexer keeps a handle open and causes an error here
echo // extracting archives
dotglob_state="$( shopt -p dotglob )"
for f in "${deps_archives[@]}"; do
  src_arch="$third_party_common_dir/$f"
  [[ -f "$src_arch" ]] || {
    echo "[X] archive '"$src_arch"' not found";
    exit 1;
  }

  target_dir="$deps_dir/$( dirname "$f" )"
  mkdir -p "$target_dir"

  echo   - extracting archive "'$f'" into "'$target_dir'"
  tar -zxf "$src_arch" -C "$target_dir"
  sleep 2

  echo   - flattening dir "'$target_dir'" by moving everything in a subdir outside
  shopt -s dotglob
  for i in {1..10}; do
    mv "$target_dir"/*/** "$target_dir/" && { break; } || { sleep 1; }
  done
  eval $dotglob_state
  sleep 2

  chmod -R 777 "$target_dir"
  sleep 1

  echo;
done


############## build ssq ##############
echo // building ssq lib
pushd "$deps_dir/libssq"

eval $recreate_32
eval $cmake_gen32
last_code=$((last_code + $?))
eval $cmake_build32
last_code=$((last_code + $?))

eval $recreate_64
eval $cmake_gen64
last_code=$((last_code + $?))
eval $cmake_build64
last_code=$((last_code + $?))

popd
echo; echo;


############## build zlib ##############
echo // building zlib lib
pushd "$deps_dir/zlib"

eval $recreate_32
eval $cmake_gen32
last_code=$((last_code + $?))
eval $cmake_build32 --target install
last_code=$((last_code + $?))
eval $clean_gen32

eval $recreate_64
eval $cmake_gen64
last_code=$((last_code + $?))
eval $cmake_build64 --target install
last_code=$((last_code + $?))
eval $clean_gen64

popd
echo; echo;


############## zlib is painful ##############
# lib curl uses the default search paths, even when ZLIB_INCLUDE_DIR and ZLIB_LIBRARY_RELEASE are defined
# check thir CMakeLists.txt line #573
#     optional_dependency(ZLIB)
#     if(ZLIB_FOUND)
#       set(HAVE_LIBZ ON)
#       set(USE_ZLIB ON)
#     
#       # Depend on ZLIB via imported targets if supported by the running
#       # version of CMake.  This allows our dependents to get our dependencies
#       # transitively.
#       if(NOT CMAKE_VERSION VERSION_LESS 3.4)
#         list(APPEND CURL_LIBS ZLIB::ZLIB)    <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< evil
#       else()
#         list(APPEND CURL_LIBS ${ZLIB_LIBRARIES})
#         include_directories(${ZLIB_INCLUDE_DIRS})
#       endif()
#       list(APPEND CMAKE_REQUIRED_INCLUDES ${ZLIB_INCLUDE_DIRS})
#     endif()
# we have to set the ZLIB_ROOT so that it is prepended to the search list
# we have to set ZLIB_LIBRARY NOT ZLIB_LIBRARY_RELEASE in order to override the FindZlib module
# we also should set ZLIB_USE_STATIC_LIBS since we want to force static builds
# https://github.com/Kitware/CMake/blob/a6853135f569f0b040a34374a15a8361bb73901b/Modules/FindZLIB.cmake#L98C4-L98C13
wild_zlib_32=(
  "-DZLIB_USE_STATIC_LIBS=ON"
  "-DZLIB_ROOT='$deps_dir/zlib/install32'"
  "-DZLIB_INCLUDE_DIR='$deps_dir/zlib/install32/include'"
  "-DZLIB_LIBRARY='$deps_dir/zlib/install32/lib/libz.a'"
)
wild_zlib_64=(
  "-DZLIB_USE_STATIC_LIBS=ON"
  "-DZLIB_ROOT='$deps_dir/zlib/install64'"
  "-DZLIB_INCLUDE_DIR='$deps_dir/zlib/install64/include'"
  "-DZLIB_LIBRARY='$deps_dir/zlib/install64/lib/libz.a'"
)


############## build curl ##############
echo // building curl lib
pushd "$deps_dir/curl"

curl_common_defs="-DBUILD_CURL_EXE=OFF -DBUILD_SHARED_LIBS=OFF -DBUILD_STATIC_CURL=OFF -DBUILD_STATIC_LIBS=ON -DCURL_USE_OPENSSL=OFF -DCURL_ZLIB=ON"

eval $recreate_32
eval $cmake_gen32 $curl_common_defs "${wild_zlib_32[@]}" -DCMAKE_SHARED_LINKER_FLAGS_INIT="'$deps_dir/zlib/install32/lib/libz.a'" -DCMAKE_MODULE_LINKER_FLAGS_INIT="'$deps_dir/zlib/install32/lib/libz.a'" -DCMAKE_EXE_LINKER_FLAGS_INIT="'$deps_dir/zlib/install32/lib/libz.a'"
last_code=$((last_code + $?))
eval $cmake_build32 --target install
last_code=$((last_code + $?))
eval $clean_gen32

eval $recreate_64
eval $cmake_gen64 $curl_common_defs "${wild_zlib_64[@]}" -DCMAKE_SHARED_LINKER_FLAGS_INIT="'$deps_dir/zlib/install64/lib/libz.a'" -DCMAKE_MODULE_LINKER_FLAGS_INIT="'$deps_dir/zlib/install64/lib/libz.a'" -DCMAKE_EXE_LINKER_FLAGS_INIT="'$deps_dir/zlib/install64/lib/libz.a'"
last_code=$((last_code + $?))
eval $cmake_build64 --target install
last_code=$((last_code + $?))
eval $clean_gen64

popd
echo; echo;


############## build protobuf ##############
echo // building protobuf lib
pushd "$deps_dir/protobuf"

proto_common_defs="-Dprotobuf_BUILD_TESTS=OFF -Dprotobuf_BUILD_SHARED_LIBS=OFF -Dprotobuf_WITH_ZLIB=ON"

eval $recreate_32
eval $cmake_gen32 $proto_common_defs "${wild_zlib_32[@]}" -DCMAKE_SHARED_LINKER_FLAGS_INIT="'$deps_dir/zlib/install32/lib/libz.a'" -DCMAKE_MODULE_LINKER_FLAGS_INIT="'$deps_dir/zlib/install32/lib/libz.a'" -DCMAKE_EXE_LINKER_FLAGS_INIT="'$deps_dir/zlib/install32/lib/libz.a'"
last_code=$((last_code + $?))
eval $cmake_build32 --target install
last_code=$((last_code + $?))
eval $clean_gen32

eval $recreate_64
eval $cmake_gen64 $proto_common_defs "${wild_zlib_64[@]}" -DCMAKE_SHARED_LINKER_FLAGS_INIT="'$deps_dir/zlib/install64/lib/libz.a'" -DCMAKE_MODULE_LINKER_FLAGS_INIT="'$deps_dir/zlib/install64/lib/libz.a'" -DCMAKE_EXE_LINKER_FLAGS_INIT="'$deps_dir/zlib/install64/lib/libz.a'"
last_code=$((last_code + $?))
eval $cmake_build64 --target install
last_code=$((last_code + $?))
eval $clean_gen64

popd
echo; echo;


############## build mbedtls ##############
echo // building mbedtls lib
pushd "$deps_dir/mbedtls"

# AES-NI on mbedtls v3.5.x:
# https://github.com/Mbed-TLS/mbedtls/issues/8400
# https://github.com/Mbed-TLS/mbedtls/issues/8334
# clang/gcc compiler flags ref:
# https://gcc.gnu.org/onlinedocs/gcc/x86-Options.html#index-mmmx
# instruction set details
# https://en.wikipedia.org/wiki/CLMUL_instruction_set
# https://en.wikipedia.org/wiki/AES_instruction_set
# https://en.wikipedia.org/wiki/SSE2

mbedtls_common_defs="-DUSE_STATIC_MBEDTLS_LIBRARY=ON -DUSE_SHARED_MBEDTLS_LIBRARY=OFF -DENABLE_TESTING=OFF -DENABLE_PROGRAMS=OFF -DLINK_WITH_PTHREAD=ON"

eval $recreate_32
CFLAGS='-mpclmul -msse2 -maes' eval $cmake_gen32 $mbedtls_common_defs
last_code=$((last_code + $?))
eval $cmake_build32 --target install
last_code=$((last_code + $?))
eval $clean_gen32

eval $recreate_64
eval $cmake_gen64 $mbedtls_common_defs
last_code=$((last_code + $?))
eval $cmake_build64 --target install
last_code=$((last_code + $?))
eval $clean_gen64

popd
echo; echo;


echo;
if [[ $last_code = 0 ]]; then
  echo "[GG] no failures"
else
  echo "[XX] general failure" >&2
fi

exit $last_code
