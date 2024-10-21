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

echo building migrate_gse...
pyinstaller "main.py" --distpath "$out_dir" -y --clean --onedir --name "migrate_gse" --noupx --console -i "NONE" --workpath "$build_temp_dir" --specpath "$build_temp_dir" || exit 1

#cp -a "post_build" "$out_dir/generate_emu_config/post_build" # skip this, as it's only for Windows; will have to use a different 'post_build' folder for Linux
cp -a "_DEFAULT" "$out_dir/generate_emu_config/_DEFAULT"

cp -f "README.md" "$out_dir/migrate_gse"

echo;
echo =============
echo Built inside: "$out_dir/"

[[ -d "$build_temp_dir" ]] && rm -r -f "$build_temp_dir"

deactivate
