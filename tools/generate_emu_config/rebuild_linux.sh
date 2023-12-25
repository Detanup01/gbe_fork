#!/usr/bin/env bash


venv=".env-linux"
out_dir="bin/linux"
build_temp_dir="build_tmp-linux"
tool_name="generate_emu_config"
main_file="generate_emu_config.py"

[[ -d "$out_dir" ]] && rm -r -f "$out_dir"
mkdir -p "$out_dir"

[[ -d "$build_temp_dir" ]] && rm -r -f "$build_temp_dir"

rm -f *.spec

chmod 777 "./$venv/bin/activate"
source "./$venv/bin/activate"

pyinstaller "$main_file" --distpath "$out_dir" -y --clean --onedir --name "$tool_name" --noupx --console -i "NONE" --workpath "$build_temp_dir" --collect-submodules "steam" || exit 1

cp -f "steam_default_icon_locked.jpg" "$out_dir/$tool_name"
cp -f "steam_default_icon_unlocked.jpg" "$out_dir/$tool_name"

echo;
echo =============
echo Built inside: "$out_dir/"

deactivate
