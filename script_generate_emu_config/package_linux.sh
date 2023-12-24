#!/usr/bin/env bash


if [ "$(id -u)" -ne 0 ]; then
  echo "Please run as root" >&2
  exit 1
fi

build_dir="bin/linux"
tool_name="generate_emu_config"
out_dir="bin/package/linux"
script_dir=$( cd -- "$( dirname -- "${0}" )" &> /dev/null && pwd )

[[ -d "$script_dir/$build_dir" ]] || {
  echo "[X] build folder wasn't found" >&2
  exit 1
}

apt update || exit 1
apt install tar -y || exit 1

mkdir -p "$script_dir/$out_dir"

archive_file="$script_dir/$out_dir/$tool_name-linux.tar.gz"
[[ -f "$archive_file" ]] && rm -f "$archive_file"
tar -C "$script_dir/$build_dir" -c -j -vf "$archive_file" "$tool_name"
