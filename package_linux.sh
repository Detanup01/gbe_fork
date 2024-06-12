#!/usr/bin/env bash


build_base_dir="build/linux"
out_dir="build/package/linux"
script_dir=$( cd -- "$( dirname -- "${0}" )" &> /dev/null && pwd )

[[ "$1" = '' ]] && {
  echo "[X] missing build folder";
  exit 1;
}

target_src_dir="$script_dir/$build_base_dir/$1"
[[ -d "$target_src_dir" ]] || {
  echo "[X] build folder wasn't found";
  exit 1;
}

# ::::::::::::::::::::::::::::::::::::::::::
echo "// copying readmes + example files"
cp -f -r "post_build/steam_settings.EXAMPLE/" "$target_src_dir/"
cp -f "post_build/README.release.md" "$target_src_dir/"
cp -f "CHANGELOG.md" "$target_src_dir/"
if [[ "$2" = "1" ]]; then
  cp -f "post_build/README.debug.md" "$target_src_dir/"
fi
if [[ -d "$target_src_dir/experimental" ]]; then
  cp -f "post_build/README.experimental_linux.md" "$target_src_dir/experimental/"
fi
if [[ -d "$target_src_dir/tools" ]]; then
  mkdir -p "$target_src_dir/tools/steamclient_loader/"
  cp -f -r tools/steamclient_loader/linux/* "$target_src_dir/tools/steamclient_loader/"
fi
if [[ -d "$target_src_dir/tools/generate_interfaces" ]]; then
  cp -f "post_build/README.generate_interfaces.md" "$target_src_dir/tools/generate_interfaces/"
fi
if [[ -d "$target_src_dir/tools/lobby_connect" ]]; then
  cp -f "post_build/README.lobby_connect.md" "$target_src_dir/tools/lobby_connect/"
fi
# ::::::::::::::::::::::::::::::::::::::::::

archive_dir="$script_dir/$out_dir/$1"
[[ -d "$archive_dir" ]] && rm -r -f "$archive_dir"
archive_file="$(dirname "$archive_dir")/emu-linux-$(basename "$archive_dir").tar.bz2"

mkdir -p "$(dirname "$archive_dir")"
tar -C "$(dirname "$target_src_dir")" -c -j -vf "$archive_file" "$(basename "$target_src_dir")"
