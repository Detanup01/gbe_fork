#!/usr/bin/env bash


venv=".env-linux"
out_dir="bin/linux"
build_temp_dir="bin/tmp/linux"

[[ -d "$out_dir" ]] && rm -r -f "$out_dir"
mkdir -p "$out_dir"

[[ -d "$build_temp_dir" ]] && rm -r -f "$build_temp_dir"

rm -f *.spec

chmod 777 "./$venv/bin/activate"
source "./$venv/bin/activate"

echo building generate_emu_config...
pyinstaller "generate_emu_config.py" --distpath "$out_dir" -y --clean --onedir --name "generate_emu_config" --noupx --console -i "NONE" --collect-submodules "steam" --workpath "$build_temp_dir" --specpath "$build_temp_dir" || exit 1

echo building parse_controller_vdf...
pyinstaller "controller_config_generator/parse_controller_vdf.py" --distpath "$out_dir" -y --clean --onedir --name "parse_controller_vdf" --noupx --console -i "NONE" --workpath "$build_temp_dir" --specpath "$build_temp_dir" || exit 1

echo building parse_achievements_schema...
pyinstaller "stats_schema_achievement_gen/achievements_gen.py" --distpath "$out_dir" -y --clean --onedir --name "parse_achievements_schema" --noupx --console -i "NONE" --workpath "$build_temp_dir" --specpath "$build_temp_dir" || exit 1

cp -f "steam_default_icon_locked.jpg" "$out_dir/generate_emu_config"
cp -f "steam_default_icon_unlocked.jpg" "$out_dir/generate_emu_config"
cp -f "README.md" "$out_dir/generate_emu_config"
echo "Check the README" > "$out_dir/generate_emu_config/my_login.EXAMPLE.txt"
echo "Check the README" > "$out_dir/generate_emu_config/top_owners_ids.EXAMPLE.txt"
echo "You can use a website like: https://steamladder.com/games/" >> "$out_dir/generate_emu_config/top_owners_ids.EXAMPLE.txt"

echo;
echo =============
echo Built inside: "$out_dir/"

[[ -d "$build_temp_dir" ]] && rm -r -f "$build_temp_dir"

deactivate
