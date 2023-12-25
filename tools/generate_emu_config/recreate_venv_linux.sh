#!/usr/bin/env bash


if [ "$(id -u)" -ne 0 ]; then
  echo "Please run as root" >&2
  exit 1
fi

venv=".env-linux"
reqs_file="requirements.txt"
script_dir=$( cd -- "$( dirname -- "${0}" )" &> /dev/null && pwd )

apt update || exit 1
apt install "python3.10" -y || exit 1
apt install "python3.10-venv" -y || exit 1

[[ -d "$script_dir/$venv" ]] && rm -r -f "$script_dir/$venv"

python3.10 -m venv "$script_dir/$venv" || exit 1
sleep 1

chmod 777 "$script_dir/$venv/bin/activate"
source "$script_dir/$venv/bin/activate"

pip install -r "$script_dir/$reqs_file"
exit_code=$?

deactivate
exit $exit_code
