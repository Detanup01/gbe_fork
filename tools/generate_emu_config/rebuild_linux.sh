#!/usr/bin/env bash


venv=".env-linux"
out_dir="bin/linux"
build_temp_dir="bin/tmp/linux"
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
cp -f "README.md" "$out_dir/$tool_name"
echo "Check the README" > "$out_dir/$tool_name/my_login.EXAMPLE.txt"
echo "Check the README" > "$out_dir/$tool_name/top_owners_ids.EXAMPLE.txt"
echo "You can use a website like: https://steamladder.com/ladder/games/"  >> "$out_dir/$tool_name/top_owners_ids.EXAMPLE.txt"

echo;
echo =============
echo Built inside: "$out_dir/"

[[ -d "$build_temp_dir" ]] && rm -r -f "$build_temp_dir"

deactivate
