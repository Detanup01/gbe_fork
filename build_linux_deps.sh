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
script_dir=$( cd -- "$( dirname -- "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )
deps_dir="$script_dir/build-linux-deps"
third_party_dir="$script_dir/third-party"
third_party_deps_dir="$third_party_dir/deps/linux"
third_party_common_dir="$third_party_dir/deps/common/src"
mycmake="$third_party_deps_dir/cmake-3.27.7-linux-x86_64/bin/cmake"

[[ -f "$mycmake" ]] || {
  echo "[X] Couldn't find cmake" >&2
  exit 1
}

# < 0: deduce, > 1: force
PARALLEL_THREADS_OVERRIDE=-1
VERBOSITY=''

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
  else
    echo "[X] Invalid arg: $var" >&2
    exit 1
  fi
done


# use 70%
build_threads="$(( $(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 0) * 70 / 100 ))"
[[ $PARALLEL_THREADS_OVERRIDE -gt 0 ]] && build_threads="$PARALLEL_THREADS_OVERRIDE"
[[ $build_threads -lt 2 ]] && build_threads=2

echo ===========================
echo Tools will be installed in: "$deps_dir"
echo Building with $build_threads threads
echo ===========================
echo // Recreating dir...
rm -r -d -f "$deps_dir"
sleep 1
mkdir -p "$deps_dir" || {
  echo "Couldn't create dir \"$deps_dir\"" >&2;
  exit 1;
}

echo; echo 

last_code=0


############## required packages ##############
echo // installing required packages
apt update -y
apt install coreutils -y # echo, printf, etc...
apt install build-essential -y
apt install gcc-multilib g++-multilib -y # needed for 32-bit builds
apt install clang -y
apt install binutils -y # (optional) contains the tool 'readelf' mainly, and other usefull binary stuff
apt install tar -y # we need to extract packages
#apt install cmake git wget unzip -y

echo ; echo


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

echo ; echo

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


############## build ssq ##############
echo // building ssq lib
tar -zxf "$third_party_common_dir/v3.0.0.tar.gz" -C "$deps_dir"

for i in {1..10}; do
  mv "$deps_dir/libssq-3.0.0" "$deps_dir/ssq" && { break; } || { sleep 1; }
done

chmod -R 777 "$deps_dir/ssq"
pushd "$deps_dir/ssq"

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
echo ; echo


############## build zlib ##############
echo // building zlib lib
tar -zxf "$third_party_common_dir/zlib-1.3.tar.gz" -C "$deps_dir"

for i in {1..10}; do
  mv "$deps_dir/zlib-1.3" "$deps_dir/zlib" && { break; } || { sleep 1; }
done

chmod -R 777 "$deps_dir/zlib"
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
echo ; echo


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
tar -zxf "$third_party_common_dir/curl-8.4.0.tar.gz" -C "$deps_dir"

for i in {1..10}; do
  mv "$deps_dir/curl-8.4.0" "$deps_dir/curl" && { break; } || { sleep 1; }
done

chmod -R 777 "$deps_dir/curl"
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
echo ; echo


############## build protobuf ##############
echo // building protobuf lib
tar -zxf "$third_party_common_dir/v21.12.tar.gz" -C "$deps_dir"

for i in {1..10}; do
  mv "$deps_dir/protobuf-21.12" "$deps_dir/protobuf" && { break; } || { sleep 1; }
done

chmod -R 777 "$deps_dir/protobuf"
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
echo ; echo

exit $last_code
