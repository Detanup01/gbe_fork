#!/usr/bin/env bash


if [ "$(id -u)" -ne 0 ]; then
  echo "Please run as root" >&2
  exit 1
fi

python_package="python3.12"
venv=".env-linux"
reqs_file="requirements.txt"
script_dir=$( cd -- "$( dirname -- "${0}" )" &> /dev/null && pwd )

apt update -y || exit 1
apt install software-properties-common -y
add-apt-repository ppa:deadsnakes/ppa -y
apt update -y || exit 1
apt install "$python_package" -y || exit 1
apt install "$python_package-dev" -y || exit 1
apt install "$python_package-venv" -y || exit 1
apt install python3-dev -y || exit 1

[[ -d "$script_dir/$venv" ]] && rm -r -f "$script_dir/$venv"

$python_package -m venv "$script_dir/$venv" || exit 1
sleep 1

chmod 777 "$script_dir/$venv/bin/activate"
source "$script_dir/$venv/bin/activate"

pip install -r "$script_dir/$reqs_file"
exit_code=$?

deactivate
exit $exit_code
