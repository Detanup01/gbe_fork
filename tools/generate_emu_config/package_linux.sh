#!/usr/bin/env bash


if [ "$(id -u)" -ne 0 ]; then
  echo "Please run as root" >&2
  exit 1
fi

build_dir="bin/linux"
out_dir="bin/package/linux"
script_dir=$( cd -- "$( dirname -- "${0}" )" &> /dev/null && pwd )

[[ -d "$script_dir/$build_dir" ]] || {
  echo "[X] build folder wasn't found" >&2
  exit 1
}

apt update || exit 1
apt install tar -y || exit 1

mkdir -p "$script_dir/$out_dir"

archive_file="$script_dir/$out_dir/generate_emu_config-linux.tar.bz2"
[[ -f "$archive_file" ]] && rm -f "$archive_file"

pushd "$script_dir/$build_dir"
tar -c -j -vf "$archive_file" $(ls -d */)
popd
