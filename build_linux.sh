#!/usr/bin/env bash

### make sure to cd to the emu src dir
required_files=(
  "dll/dll.cpp"
  "dll/steam_client.cpp"
  "controller/gamepad.c"
  "sdk_includes/isteamclient.h"
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

BUILD_LOW_PERF=0

# < 0: deduce, > 1: force
PARALLEL_THREADS_OVERRIDE=-1

# 0 = release, 1 = debug, otherwise error
BUILD_TYPE=-1

CLEAN_BUILD=0

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
  elif [[ "$var" = "-clean" ]]; then
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

additional_compiler_args=''
if [[ "$BUILD_LOW_PERF" = "1" ]]; then
  echo "[?] Adding low perf flags"
  # https://gcc.gnu.org/onlinedocs/gcc/x86-Options.html#index-mmmx
  instr_set_to_disable=(
    'popcnt'
    
    'sse4'
    'sse4a'
    'sse4.1'
    'sse4.2'

    'avx'
    'avx2'
    'avx512f'
    'avx512pf'
    'avx512er'
    'avx512cd'
    'avx512vl'
    'avx512bw'
    'avx512dq'
    'avx512ifma'
    'avx512vbmi'

    'avx512vbmi2'
    'avx512bf16'
    'avx512fp16'
    'avx512bitalg'
    'avx5124vnniw'
    'avx512vpopcntdq'
    'avx512vp2intersect'
    'avx5124fmaps'
    'avx512vnni'

    'avxifma'
    'avxvnniint8'
    'avxneconvert'
    'avxvnni'
    'avxvnniint16'

    'avx10.1'
    'avx10.1-256'
    'avx10.1-512'
  )
  additional_compiler_args="$additional_compiler_args ${instr_set_to_disable[@]/#/-mno-}"
  
  build_folder="$build_folder-low_perf"
fi


# common stuff
deps_dir="build-linux-deps"
build_root_dir="build-linux/$build_folder"
build_temp_dir="build-linux-temp"
third_party_dir="third-party"
third_party_build_dir="$third_party_dir/build/linux"
build_root_32="$build_root_dir/x32"
build_root_64="$build_root_dir/x64"
build_root_tools="$build_root_dir/tools"

# https://gcc.gnu.org/onlinedocs/gcc/Code-Gen-Options.html
common_compiler_args="$additional_compiler_args -std=c++17 -fvisibility=hidden -fexceptions -fno-jump-tables"

ssq_inc="$deps_dir/ssq/include"
ssq_lib32="$deps_dir/ssq/build32"
ssq_lib64="$deps_dir/ssq/build64"

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

release_incs_both=(
  "$ssq_inc"
  "utfcpp"
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

release_ignore_warn="-Wno-switch"
common_defs="-DGNUC -DUTF_CPP_CPLUSPLUS=201703L -DCURL_STATICLIB"
release_defs="$dbg_defs $common_defs"
release_src=(
  "dll/*.cpp"
  "dll/*.cc"
)
# if it's called libXYZ.a, then only type "XYZ"
# these will be statically linked, make sure to build a PIC static lib
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

protoc_exe_32="$deps_dir/protobuf/install32/bin/protoc"
protoc_exe_64="$deps_dir/protobuf/install64/bin/protoc"

parallel_exe="$third_party_build_dir/rush-v0.5.4-linux/rush"

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

  local tmp_work_dir="${out_filepath##*/}"
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
      echo "[X] file "$src_file" wasn't found" >&2;
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
  printf '%s\n' "${build_cmds[@]}" | "$parallel_exe" -j$build_threads -d '<>' -k "$build_cmd"
  exit_code=$?
  echo "  -- Exit code = $exit_code"
  echo ; echo ;
  sleep 1
  [[ $exit_code = 0 ]] || return $exit_code

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
  # https://access.redhat.com/documentation/en-us/red_hat_enterprise_linux/7/html/developer_guide/gcc-using-libraries#gcc-using-libraries_using-both-static-dynamic-library-gcc
  # https://linux.die.net/man/1/ld
  clang++ $common_compiler_args $cpiler_pic_pie $cpiler_m32 $optimize_level $dbg_level $linker_pic_pie $linker_strip_dbg_symbols -o "$out_filepath" "${obj_files[@]}" "${target_libs_dirs[@]/#/-L}" -Wl,--whole-archive -Wl,-Bstatic "${release_libs[@]/#/-l}" -Wl,-Bdynamic -Wl,--no-whole-archive -Wl,--exclude-libs,ALL
  exit_code=$?
  echo "  -- Exit code = $exit_code"
  echo ; echo ;
  sleep 1
  return $exit_code

}


if [[ "$CLEAN_BUILD" = "1" ]]; then
  echo // cleaning previous build
  [[ -d "$build_root_dir" ]] && rm -f -r "$build_root_dir"
  echo; echo;
fi


## tools
if [[ "$BUILD_TOOL_CLIENT_LDR" = "1" ]]; then
  echo // copying tool steamclient_loader
  [[ -d "$build_root_tools/steamclient_loader/" ]] || mkdir -p "$build_root_tools/steamclient_loader/"
  steamclient_files=(
    "ldr_appid.txt"
    "ldr_cmdline.txt"
    "ldr_cwd.txt"
    "ldr_exe.txt"
    "ldr_steam_rt.txt"
    "README.md"
    "steamclient_loader.sh"
  )
  for f in "${steamclient_files[@]}"; do
    cp -f "tools_linux/steamclient_loader/$f" "$build_root_tools/steamclient_loader/";
  done
  
  echo; echo;
fi


### x32 build
rm -f dll/net.pb.cc
rm -f dll/net.pb.h
echo // invoking protobuf compiler - 32
"$protoc_exe_32" -I./dll/ --cpp_out=./dll/ ./dll/*.proto
echo; echo;

if [[ "$BUILD_LIB32" = "1" ]]; then
  echo // building shared lib libsteam_api.so - 32
  [[ -d "$build_root_32" ]] || mkdir -p "$build_root_32"

  all_src_files=(
    "${release_src[@]}"
    "controller/*.c"
  )
  build_for 1 0 "$build_root_32/libsteam_api.so" '-DCONTROLLER_SUPPORT' all_src_files 
  last_code=$((last_code + $?))
fi

if [[ "$BUILD_CLIENT32" = "1" ]]; then
  echo // building shared lib steamclient.so - 32
  [[ -d "$build_root_32" ]] || mkdir -p "$build_root_32"
  
  all_src_files=(
    "${release_src[@]}"
    "controller/*.c"
  )
  build_for 1 0 "$build_root_32/steamclient.so" '-DCONTROLLER_SUPPORT -DSTEAMCLIENT_DLL' all_src_files 
  last_code=$((last_code + $?))
fi

if [[ "$BUILD_TOOL_LOBBY32" = "1" ]]; then
  echo // building executable lobby_connect_x32 - 32
  [[ -d "$build_root_tools/lobby_connect" ]] || mkdir -p "$build_root_tools/lobby_connect"
  
  all_src_files=(
    "${release_src[@]}"
    "lobby_connect.cpp"
  )
  build_for 1 1 "$build_root_tools/lobby_connect/lobby_connect_x32" '-DNO_DISK_WRITES -DLOBBY_CONNECT' all_src_files 
  last_code=$((last_code + $?))
fi

if [[ "$BUILD_TOOL_FIND_ITFS32" = "1" ]]; then
  echo // building executable generate_interfaces_file_x32 - 32
  [[ -d "$build_root_tools/find_interfaces" ]] || mkdir -p "$build_root_tools/find_interfaces"

  all_src_files=(
    "generate_interfaces_file.cpp"
  )
  build_for 1 1 "$build_root_tools/find_interfaces/generate_interfaces_file_x32" '' all_src_files 
  last_code=$((last_code + $?))
fi


### x64 build
rm -f dll/net.pb.cc
rm -f dll/net.pb.h
rm -f -r "$build_temp_dir"
mkdir -p "$build_temp_dir"
echo // invoking protobuf compiler - 64
"$protoc_exe_64" -I./dll/ --cpp_out=./dll/ ./dll/*.proto
echo; echo;

if [[ "$BUILD_LIB64" = "1" ]]; then
  echo // building shared lib libsteam_api.so - 64
  [[ -d "$build_root_64" ]] || mkdir -p "$build_root_64"
  
  all_src_files=(
    "${release_src[@]}"
    "controller/*.c"
  )
  build_for 0 0 "$build_root_64/libsteam_api.so" '-DCONTROLLER_SUPPORT' all_src_files 
  last_code=$((last_code + $?))
fi

if [[ "$BUILD_CLIENT64" = "1" ]]; then
  echo // building shared lib steamclient.so - 64
  [[ -d "$build_root_64" ]] || mkdir -p "$build_root_64"
  
  all_src_files=(
    "${release_src[@]}"
    "controller/*.c"
  )
  build_for 0 0 "$build_root_64/steamclient.so" '-DCONTROLLER_SUPPORT -DSTEAMCLIENT_DLL' all_src_files 
  last_code=$((last_code + $?))
fi

if [[ "$BUILD_TOOL_LOBBY64" = "1" ]]; then
  echo // building executable lobby_connect_x64 - 64
  [[ -d "$build_root_tools/lobby_connect" ]] || mkdir -p "$build_root_tools/lobby_connect"
  
  all_src_files=(
    "${release_src[@]}"
    "lobby_connect.cpp"
  )
  build_for 0 1 "$build_root_tools/lobby_connect/lobby_connect_x64" '-DNO_DISK_WRITES -DLOBBY_CONNECT' all_src_files 
  last_code=$((last_code + $?))
fi

if [[ "$BUILD_TOOL_FIND_ITFS64" = "1" ]]; then
  echo // building executable generate_interfaces_file_x64 - 64
  [[ -d "$build_root_tools/find_interfaces" ]] || mkdir -p "$build_root_tools/find_interfaces"

  all_src_files=(
    "generate_interfaces_file.cpp"
  )
  build_for 0 1 "$build_root_tools/find_interfaces/generate_interfaces_file_x64" '' all_src_files 
  last_code=$((last_code + $?))
fi


# cleanup
rm -f dll/net.pb.cc
rm -f dll/net.pb.h
rm -f -r "$build_temp_dir"

echo;
if [[ $last_code = 0 ]]; then
  echo "[GG] no failures"
else
  echo "[XX] general failure" >&2
fi

exit $last_code
