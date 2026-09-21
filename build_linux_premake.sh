#!/usr/bin/env bash

function help_page () {
  echo "./$(basename "$0") [switches]"
  echo "switches:"
  echo "  --deps: rebuild third-party dependencies"
  echo "  --nogen: don't regenerate build files"
  echo "  --j: parallel build jobs"
  echo "  --help: show this page"
}

BUILD_DEPS=0
GEN_PROJECT=1
BUILD_JOBS=-1
for (( i=1; i<=$#; ++i )); do
  arg="${!i}"
  if [[ "$arg" = "--deps" ]]; then
    BUILD_DEPS=1
  elif [[ "$arg" = "--nogen" ]]; then
    GEN_PROJECT=0
  elif [[ "$arg" = "--j" ]]; then
    BUILD_JOBS="$2"
    shift 1
  elif [[ "$arg" = "--help" ]]; then
    help_page
    exit 0
  else
    echo "invalid arg $arg" 1>&2
    exit 1
  fi
done

# use 70%
build_threads="$(( $(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 0) * 70 / 100 ))"
[[ $build_threads -lt 2 ]] && build_threads=2
[[ "$BUILD_JOBS" -ge 1 ]] && build_threads="$BUILD_JOBS"

premake_exe=./"third-party/common/linux/premake/premake5"
if [[ ! -f "$premake_exe" ]]; then
  echo "preamke wasn't found" 1>&2
  exit 1
fi
chmod 777 "$premake_exe"

# build deps
if [[ $BUILD_DEPS = 1 ]]; then
  export CMAKE_GENERATOR="Unix Makefiles"
  "$premake_exe" --file="premake5-deps.lua" --all-ext --all-build --64-build --32-build --arm-build --verbose --clean --j=$build_threads --os=linux gmake2 || {
    exit 1;
  }
fi

if [[ $GEN_PROJECT = 1 ]]; then
  "$premake_exe" --genproto --os=linux gmake2 || {
    exit 1;
  }
fi

pushd ./"build/project/gmake2/linux"

# you can select individual or all

echo; echo building debug x64
make -j $build_threads config=debug_x64 || {
  exit 1;
}

echo; echo building debug x32
make -j $build_threads config=debug_x32 || {
  exit 1;
}

echo; echo building debug arm64
make -j $build_threads config=debug_arm64 || {
  exit 1;
}

echo; echo building release x64
make -j $build_threads config=release_x64 || {
  exit 1;
}

echo; echo building release x32
make -j $build_threads config=release_x32 || {
  exit 1;
}

echo; echo building release arm64
make -j $build_threads config=release_arm64 || {
  exit 1;
}

popd
