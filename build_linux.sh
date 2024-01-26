#!/usr/bin/env bash

### make sure to cd to the emu src dir
required_files=(
  "dll/dll.cpp"
  "dll/steam_client.cpp"
  "controller/gamepad.c"
  "sdk/steam/isteamclient.h"
)

for emu_file in "${required_files[@]}"; do
  if [ ! -f "$emu_file" ]; then
    echo "[X] Invalid emu directory, change directory to emu's src dir (missing file '$emu_file')" >&2
    exit 1
  fi
done

BUILD_LIB32=1
BUILD_LIB64=1

BUILD_CLIENT32=1
BUILD_CLIENT64=1
BUILD_TOOL_CLIENT_LDR=1

BUILD_TOOL_FIND_ITFS32=1
BUILD_TOOL_FIND_ITFS64=1
BUILD_TOOL_LOBBY32=1
BUILD_TOOL_LOBBY64=1

BUILD_LIB_NET_SOCKETS_32=0
BUILD_LIB_NET_SOCKETS_64=0

# < 0: deduce, > 1: force
PARALLEL_THREADS_OVERRIDE=-1

# 0 = release, 1 = debug, otherwise error
BUILD_TYPE=-1

CLEAN_BUILD=0

VERBOSE=0

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
  elif [[ "$var" = "-lib-32" ]]; then
    BUILD_LIB32=0
  elif [[ "$var" = "-lib-64" ]]; then
    BUILD_LIB64=0
  elif [[ "$var" = "-client-32" ]]; then
    BUILD_CLIENT32=0
  elif [[ "$var" = "-client-64" ]]; then
    BUILD_CLIENT64=0
  elif [[ "$var" = "-tool-clientldr" ]]; then
    BUILD_TOOL_CLIENT_LDR=0
  elif [[ "$var" = "-tool-itf-32" ]]; then
    BUILD_TOOL_FIND_ITFS32=0
  elif [[ "$var" = "-tool-itf-64" ]]; then
    BUILD_TOOL_FIND_ITFS64=0
  elif [[ "$var" = "-tool-lobby-32" ]]; then
    BUILD_TOOL_LOBBY32=0
  elif [[ "$var" = "-tool-lobby-64" ]]; then
    BUILD_TOOL_LOBBY64=0
  elif [[ "$var" = "+lib-netsockets-32" ]]; then
    BUILD_LIB_NET_SOCKETS_32=1
  elif [[ "$var" = "+lib-netsockets-64" ]]; then
    BUILD_LIB_NET_SOCKETS_64=1
  elif [[ "$var" = "-verbose" ]]; then
    VERBOSE=1
  elif [[ "$var" = "clean" ]]; then
    CLEAN_BUILD=1
  elif [[ "$var" = "release" ]]; then
    BUILD_TYPE=0
  elif [[ "$var" = "debug" ]]; then
    BUILD_TYPE=1
  else
    echo "[X] Invalid arg: $var" >&2
    exit 1
  fi
done

# use 70%
build_threads="$(( $(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 0) * 70 / 100 ))"
[[ $PARALLEL_THREADS_OVERRIDE -gt 0 ]] && build_threads="$PARALLEL_THREADS_OVERRIDE"
[[ $build_threads -lt 2 ]] && build_threads=2

# build type
optimize_level=""
dbg_level=""
dbg_defs=""
linker_strip_dbg_symbols=''
build_folder=""
if [[ "$BUILD_TYPE" = "0" ]]; then
  optimize_level="-O2"
  dbg_level="-g0"
  dbg_defs="-DEMU_RELEASE_BUILD -DNDEBUG"
  linker_strip_dbg_symbols='-Wl,-S'
  build_folder="release"
elif [[ "$BUILD_TYPE" = "1" ]]; then
  optimize_level="-Og"
  dbg_level="-g3"
  dbg_defs=""
  linker_strip_dbg_symbols=""
  build_folder="debug"
else
  echo "[X] You must specify any of: [release debug]" >&2
  exit 1
fi

build_root_dir="build/linux/$build_folder"
build_root_32="$build_root_dir/x32"
build_root_64="$build_root_dir/x64"
build_root_tools="$build_root_dir/tools"

# common stuff
deps_dir="build/deps/linux"
libs_dir="libs"
tools_dir='tools'
build_temp_dir="build/tmp/linux"
protoc_out_dir="dll/proto_gen/linux"
third_party_dir="third-party"
third_party_build_dir="$third_party_dir/build/linux"

protoc_exe_32="$deps_dir/protobuf/install32/bin/protoc"
protoc_exe_64="$deps_dir/protobuf/install64/bin/protoc"

parallel_exe="$third_party_build_dir/rush/rush"

# https://gcc.gnu.org/onlinedocs/gcc/Code-Gen-Options.html
common_compiler_args="-std=c++17 -fvisibility=hidden -fexceptions -fno-jump-tables"

# third party dependencies (include folder + folder containing .a file)
ssq_inc="$deps_dir/libssq/include"
ssq_lib32="$deps_dir/libssq/build32"
ssq_lib64="$deps_dir/libssq/build64"

curl_inc32="$deps_dir/curl/install32/include"
curl_inc64="$deps_dir/curl/install64/include"
curl_lib32="$deps_dir/curl/install32/lib"
curl_lib64="$deps_dir/curl/install64/lib"

protob_inc32="$deps_dir/protobuf/install32/include"
protob_inc64="$deps_dir/protobuf/install64/include"
protob_lib32="$deps_dir/protobuf/install32/lib"
protob_lib64="$deps_dir/protobuf/install64/lib"

zlib_inc32="$deps_dir/zlib/install32/include"
zlib_inc64="$deps_dir/zlib/install64/include"
zlib_lib32="$deps_dir/zlib/install32/lib"
zlib_lib64="$deps_dir/zlib/install64/lib"

mbedtls_inc32="$deps_dir/mbedtls/install32/include"
mbedtls_inc64="$deps_dir/mbedtls/install64/include"
mbedtls_lib32="$deps_dir/mbedtls/install32/lib"
mbedtls_lib64="$deps_dir/mbedtls/install64/lib"

# directories to use for #include
release_incs_both=(
  "$ssq_inc"
  "$libs_dir"
  "$protoc_out_dir"
  "$libs_dir/utfcpp"
  "controller"
  "dll"
  "sdk"
  "overlay_experimental"
  "crash_printer"
  "helpers"
)
release_incs32=(
  "${release_incs_both[@]}"
  "$curl_inc32"
  "$protob_inc32"
  "$zlib_inc32"
  "$mbedtls_inc32"
)
release_incs64=(
  "${release_incs_both[@]}"
  "$curl_inc64"
  "$protob_inc64"
  "$zlib_inc64"
  "$mbedtls_inc64"
)

# directories where libraries (.a or .so) will be looked up
release_libs_dir32=(
  "$ssq_lib32"
  "$curl_lib32"
  "$protob_lib32"
  "$zlib_lib32"
  "$mbedtls_lib32"
)
release_libs_dir64=(
  "$ssq_lib64"
  "$curl_lib64"
  "$protob_lib64"
  "$zlib_lib64"
  "$mbedtls_lib64"
)

# libraries to link with, either static ".a" or dynamic ".so" (name only)
# if it's called libXYZ.a, then only write "XYZ"
# for static libs make sure to build a PIC lib
# each will be prefixed with -l, ex: -lpthread
release_libs=(
  "pthread"
  "dl"
  "ssq"
  "z" # libz library
  "curl"
  "protobuf-lite"
  "mbedcrypto"
)

# common source files used everywhere, just for convinience, you still have to provide a complete list later
release_src=(
  "dll/*.cpp"
  "$protoc_out_dir/*.cc"
  "crash_printer/linux.cpp"
  "helpers/common_helpers.cpp"
)

# additional #defines
common_defs="-DGNUC -DUTF_CPP_CPLUSPLUS=201703L -DCURL_STATICLIB"
release_defs="$dbg_defs $common_defs"

# errors to ignore during build
release_ignore_warn="-Wno-switch"

[ ! -d "$deps_dir" ] && {
  echo "[X] Dependencies dir \"$deps_dir\" was not found" >&2;
  exit 1;
}

[ ! -f "$protoc_exe_32" ] && {
  echo "[X] protobuff compiler wasn't found - 32" >&2;
  exit 1;
}

[ ! -f "$protoc_exe_64" ] && {
  echo "[X] protobuff compiler wasn't found - 64" >&2;
  exit 1;
}

[ ! -f "$parallel_exe" ] && {
  echo "[X] tool to run parallel compilation jobs '$parallel_exe' wasn't found" >&2;
  exit 1;
}

chmod 777 "$parallel_exe"

echo "[?] All build operations will use $build_threads parallel jobs"

last_code=0


function build_for () {

  local is_32_build=$( [[ "$1" != 0 ]] && { echo 1; } || { echo 0; }  )
  local is_exe=$( [[ "$2" != 0 ]] && { echo 1; } || { echo 0; }  )
  local out_filepath="${3:?'missing filepath'}"
  local extra_defs="$4"
  local -n all_src=$5

  [[ "${#all_src[@]}" = "0" ]] && {
    echo [X] "No source files specified" >&2;
    return 1;
  }

  # https://gcc.gnu.org/onlinedocs/gcc/x86-Options.html
  local cpiler_m32=''
  [[ $is_32_build = 1 ]] && cpiler_m32='-m32'

  # https://gcc.gnu.org/onlinedocs/gcc/Code-Gen-Options.html
  local cpiler_pic_pie='-fPIC'
  [[ $is_exe = 1 ]] && cpiler_pic_pie='-fPIE'

  # https://gcc.gnu.org/onlinedocs/gcc/Link-Options.html
  # https://www.baeldung.com/linux/library-convert-static-to-shared
  local linker_pic_pie='-shared'
  [[ $is_exe = 1 ]] && linker_pic_pie='-pie'

  local tmp_work_dir="${out_filepath##*/}_$is_32_build"
  tmp_work_dir="$build_temp_dir/${tmp_work_dir%.*}"
  echo "  -- Cleaning compilation directory '$tmp_work_dir/'"
  rm -f -r "$tmp_work_dir"
  mkdir -p "$tmp_work_dir" || {
    echo [X] "Failed to create compilation directory" >&2;
    return 1;
  }
  
  local build_cmds=()
  local -A objs_dirs=()
  for src_file in $( echo "${all_src[@]}" ); do
    [[ -f "$src_file" ]] || {
      echo "[X] source file "$src_file" wasn't found" >&2;
      return 1;
    }
    
    # https://stackoverflow.com/a/9559024
    local obj_dir=$( [[ -d "${src_file%/*}" ]] && echo "${src_file%/*}" || echo '.' )
    obj_dir="$tmp_work_dir/$obj_dir"
    
    local obj_name="${src_file##*/}"
    obj_name="${obj_name%.*}.o"
    
    build_cmds+=("$src_file<>$obj_dir/$obj_name")
    objs_dirs["$obj_dir"]="$obj_dir"
    
    mkdir -p "$obj_dir"
  done

  [[ "${#build_cmds[@]}" = "0" ]] && {
    echo [X] "No valid source files were found" >&2;
    return 1;
  }

  local target_incs=()
  local target_libs_dirs=()
  if [[ $is_32_build = 1 ]]; then
    target_incs=("${release_incs32[@]}")
    target_libs_dirs=("${release_libs_dir32[@]}")
  else
    target_incs=("${release_incs64[@]}")
    target_libs_dirs=("${release_libs_dir64[@]}")
  fi
  # wrap each -ImyIncDir with single quotes -> '-ImyIncDir'
  target_incs=("${target_incs[@]/#/\'-I}")
  target_incs=("${target_incs[@]/%/\'}")
  
  echo "  -- Compiling object files with $build_threads parallel jobs inside directory '$tmp_work_dir/'"
  local build_cmd="clang++ -c -x c++ $common_compiler_args $cpiler_pic_pie $cpiler_m32 $optimize_level $dbg_level $release_ignore_warn $release_defs $extra_defs ${target_incs[@]} '{1}' '-o{2}'"

  if [[ $VERBOSE = 1 ]]; then
    printf '%s\n' "${build_cmds[@]}" | "$parallel_exe" --dry-run -j$build_threads -d '<>' -k "$build_cmd"
    echo;
  fi

  printf '%s\n' "${build_cmds[@]}" | "$parallel_exe" -j$build_threads -d '<>' -k "$build_cmd"
  exit_code=$?
  echo "  -- Exit code = $exit_code"
  echo; echo;
  sleep 1
  [[ $exit_code = 0 ]] || {
    rm -f -r "$tmp_work_dir";
    sleep 1
    return $exit_code;
  }

  echo "  -- Linking all object files from '$tmp_work_dir/' to produce '$out_filepath'"
  local obj_files=()
  for obj_file in $(echo "${objs_dirs[@]/%//*.o}" ); do
    [[ -f "$obj_file" ]] && obj_files+=("$obj_file")
  done
  # if no files were added
  [[ "${#obj_files[@]}" -gt 0 ]] || {
    echo "[X] No files to link" >&2;
    return 1;
  }
  
  local out_dir="${out_filepath%/*}"
  [[ "$out_dir" = "$out_filepath" ]] && out_dir='.'
  mkdir -p "$out_dir"
  
  # https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/7/html/developer_guide/gcc-using-libraries#gcc-using-libraries_using-both-static-dynamic-library-gcc
  # https://linux.die.net/man/1/ld
  if [[ $VERBOSE = 1 ]]; then
    echo "clang++ $common_compiler_args $cpiler_pic_pie $cpiler_m32 $optimize_level $dbg_level $linker_pic_pie $linker_strip_dbg_symbols -o" "$out_filepath" "${obj_files[@]}" "${target_libs_dirs[@]/#/-L}" "-Wl,--whole-archive -Wl,-Bstatic" "${release_libs[@]/#/-l}" "-Wl,-Bdynamic -Wl,--no-whole-archive -Wl,--exclude-libs,ALL"
    echo;
  fi

  clang++ $common_compiler_args $cpiler_pic_pie $cpiler_m32 $optimize_level $dbg_level $linker_pic_pie $linker_strip_dbg_symbols -o "$out_filepath" "${obj_files[@]}" "${target_libs_dirs[@]/#/-L}" -Wl,--whole-archive -Wl,-Bstatic "${release_libs[@]/#/-l}" -Wl,-Bdynamic -Wl,--no-whole-archive -Wl,--exclude-libs,ALL
  exit_code=$?
  echo "  -- Exit code = $exit_code"
  echo; echo;
  rm -f -r "$tmp_work_dir"
  sleep 1
  return $exit_code

}

function cleanup () {
  rm -f -r "$build_temp_dir"
}

if [[ "$CLEAN_BUILD" = "1" ]]; then
  echo // cleaning previous build
  [[ -d "$build_root_dir" ]] && rm -f -r "$build_root_dir"
  echo; echo;
fi


## tools
if [[ "$BUILD_TOOL_CLIENT_LDR" = "1" ]]; then
  [[ -d "$build_root_tools/steamclient_loader/" ]] || mkdir -p "$build_root_tools/steamclient_loader/"
  cp -f "$tools_dir"/steamclient_loader/linux/* "$build_root_tools/steamclient_loader/"
  echo; echo;
fi


### x32 build
cleanup

echo // invoking protobuf compiler - 32
rm -f -r "$protoc_out_dir"
mkdir -p "$protoc_out_dir"
"$protoc_exe_32" ./dll/*.proto -I./dll/ --cpp_out="$protoc_out_dir/"
last_code=$((last_code + $?))
echo; echo;

if [[ "$BUILD_LIB32" = "1" ]]; then
  echo // building shared lib libsteam_api.so - 32
  all_src_files=(
    "${release_src[@]}"
    "controller/*.c"
  )
  build_for 1 0 "$build_root_32/libsteam_api.so" '-DCONTROLLER_SUPPORT' all_src_files 
  last_code=$((last_code + $?))
fi

if [[ "$BUILD_CLIENT32" = "1" ]]; then
  echo // building shared lib steamclient.so - 32
  all_src_files=(
    "${release_src[@]}"
    "controller/*.c"
  )
  build_for 1 0 "$build_root_32/steamclient.so" '-DCONTROLLER_SUPPORT -DSTEAMCLIENT_DLL' all_src_files 
  last_code=$((last_code + $?))
fi

if [[ "$BUILD_TOOL_LOBBY32" = "1" ]]; then
  echo // building executable lobby_connect_x32 - 32
  all_src_files=(
    "${release_src[@]}"
    "$tools_dir/lobby_connect/lobby_connect.cpp"
  )
  build_for 1 1 "$build_root_tools/lobby_connect/lobby_connect_x32" '-DNO_DISK_WRITES -DLOBBY_CONNECT' all_src_files 
  last_code=$((last_code + $?))
fi

if [[ "$BUILD_TOOL_FIND_ITFS32" = "1" ]]; then
  echo // building executable generate_interfaces_file_x32 - 32
  all_src_files=(
    "$tools_dir/generate_interfaces/generate_interfaces.cpp"
  )
  build_for 1 1 "$build_root_tools/find_interfaces/generate_interfaces_file_x32" '' all_src_files 
  last_code=$((last_code + $?))
fi

if [[ "$BUILD_LIB_NET_SOCKETS_32" = "1" ]]; then
  echo // building shared lib steamnetworkingsockets.so - 32
  all_src_files=(
    "networking_sockets_lib/steamnetworkingsockets.cpp"
  )
  build_for 1 0 "$build_root_dir/networking_sockets_lib/steamnetworkingsockets.so" '' all_src_files 
  last_code=$((last_code + $?))
fi


### x64 build
cleanup

echo // invoking protobuf compiler - 64
rm -f -r "$protoc_out_dir"
mkdir -p "$protoc_out_dir"
"$protoc_exe_64" ./dll/*.proto -I./dll/ --cpp_out="$protoc_out_dir/"
last_code=$((last_code + $?))
echo; echo;

if [[ "$BUILD_LIB64" = "1" ]]; then
  echo // building shared lib libsteam_api.so - 64
  all_src_files=(
    "${release_src[@]}"
    "controller/*.c"
  )
  build_for 0 0 "$build_root_64/libsteam_api.so" '-DCONTROLLER_SUPPORT' all_src_files 
  last_code=$((last_code + $?))
fi

if [[ "$BUILD_CLIENT64" = "1" ]]; then
  echo // building shared lib steamclient.so - 64
  all_src_files=(
    "${release_src[@]}"
    "controller/*.c"
  )
  build_for 0 0 "$build_root_64/steamclient.so" '-DCONTROLLER_SUPPORT -DSTEAMCLIENT_DLL' all_src_files 
  last_code=$((last_code + $?))
fi

if [[ "$BUILD_TOOL_LOBBY64" = "1" ]]; then
  echo // building executable lobby_connect_x64 - 64
  all_src_files=(
    "${release_src[@]}"
    "$tools_dir/lobby_connect/lobby_connect.cpp"
  )
  build_for 0 1 "$build_root_tools/lobby_connect/lobby_connect_x64" '-DNO_DISK_WRITES -DLOBBY_CONNECT' all_src_files 
  last_code=$((last_code + $?))
fi

if [[ "$BUILD_TOOL_FIND_ITFS64" = "1" ]]; then
  echo // building executable generate_interfaces_file_x64 - 64
  all_src_files=(
    "$tools_dir/generate_interfaces/generate_interfaces.cpp"
  )
  build_for 0 1 "$build_root_tools/find_interfaces/generate_interfaces_file_x64" '' all_src_files 
  last_code=$((last_code + $?))
fi

if [[ "$BUILD_LIB_NET_SOCKETS_64" = "1" ]]; then
  echo // building shared lib steamnetworkingsockets64.so - 64
  all_src_files=(
    "networking_sockets_lib/steamnetworkingsockets.cpp"
  )
  build_for 0 0 "$build_root_dir/networking_sockets_lib/steamnetworkingsockets64.so" '' all_src_files 
  last_code=$((last_code + $?))
fi


# cleanup
cleanup


# copy configs + examples
if [[ $last_code = 0 ]]; then
  echo "// copying readmes + files examples"
  cp -f -r "post_build/steam_settings.EXAMPLE/" "$build_root_dir/"
  cp -f "post_build/README.release.md" "$build_root_dir/"
  cp -f "CHANGELOG.md" "$build_root_dir/"
  if [[ $BUILD_TYPE = 1 ]]; then
    cp -f "post_build/README.debug.md" "$build_root_dir/"
  fi
  if [[ -d "$build_root_tools/find_interfaces" ]]; then
    cp -f "post_build/README.find_interfaces.md" "$build_root_tools/find_interfaces/"
  fi
  if [[ -d "$build_root_tools/lobby_connect" ]]; then
    cp -f "post_build/README.lobby_connect.md" "$build_root_tools/lobby_connect/"
  fi
else
  echo "[X] Not copying readmes or files examples due to previous errors" >&2
fi
echo; echo;


echo;
if [[ $last_code = 0 ]]; then
  echo "[GG] no failures"
else
  echo "[XX] general failure" >&2
fi

exit $last_code
