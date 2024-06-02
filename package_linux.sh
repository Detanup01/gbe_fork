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

archive_file="$script_dir/$out_dir/$1/emu-linux-$1.tar.bz2"
[[ -f "$archive_file" ]] && rm -f "$archive_file"
mkdir -p "$(dirname "$archive_file")"
tar -C "$(dirname "$target_src_dir")" -c -j -vf "$archive_file" "$(basename "$target_src_dir")"
