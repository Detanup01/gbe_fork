#!/usr/bin/env bash


build_base_dir="build/linux"
out_dir="build/package/linux"
script_dir=$( cd -- "$( dirname -- "${0}" )" &> /dev/null && pwd )

[[ "$1" = '' ]] && {
  echo "[X] missing build folder";
  exit 1;
}

if [ "$(id -u)" -ne 0 ]; then
  echo "Please run as root" >&2
  exit 1
fi

[[ -d "$script_dir/$build_base_dir/$1" ]] || {
  echo "[X] build folder wasn't found";
  exit 1;
}

apt update || exit 1
apt install tar -y || exit 1

mkdir -p "$script_dir/$out_dir/$1"

archive_file="$script_dir/$out_dir/$1/emu-linux-$1.tar.gz"
[[ -f "$archive_file" ]] && rm -f "$archive_file"
tar -C "$script_dir/$build_base_dir" -c -j -vf "$archive_file" "$1"
