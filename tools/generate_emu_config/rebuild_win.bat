@echo off

setlocal
pushd "%~dp0"

set "venv=.env-win"
set "out_dir=bin\win"
set "build_temp_dir=bin\tmp\win"
set "icon_file=icon\Froyoshark-Enkel-Steam.ico"
set "signer_tool=..\..\third-party\build\win\cert\sign_helper.bat"

set /a last_code=0

if not exist "%signer_tool%" (
    1>&2 echo "[X] signing tool wasn't found"
    set /a last_code=1
    goto :script_end
)

if exist "%out_dir%" (
    rmdir /s /q "%out_dir%"
)
mkdir "%out_dir%"

if exist "%build_temp_dir%" (
    rmdir /s /q "%build_temp_dir%"
)

del /f /q "*.spec"

call "%venv%\Scripts\activate.bat"

echo building generate_emu_config...
pyinstaller "generate_emu_config.py" --distpath "%out_dir%" -y --clean --onedir --name "generate_emu_config" --noupx --console -i "%icon_file%" --workpath "%build_temp_dir%" --collect-submodules "steam" || (
    set /a last_code=1
    goto :script_end
)
call "%signer_tool%" "%out_dir%\generate_emu_config\generate_emu_config.exe"

echo building parse_controller_vdf...
pyinstaller "controller_config_generator\parse_controller_vdf.py" --distpath "%out_dir%" -y --clean --onedir --name "parse_controller_vdf" --noupx --console -i "NONE" --workpath "%build_temp_dir%" || (
    set /a last_code=1
    goto :script_end
)
call "%signer_tool%" "%out_dir%\parse_controller_vdf\parse_controller_vdf.exe"

echo building parse_achievements_schema...
pyinstaller "stats_schema_achievement_gen\achievements_gen.py" --distpath "%out_dir%" -y --clean --onedir --name "parse_achievements_schema" --noupx --console -i "NONE" --workpath "%build_temp_dir%" || (
    set /a last_code=1
    goto :script_end
)
call "%signer_tool%" "%out_dir%\parse_achievements_schema\parse_achievements_schema.exe"

copy /y "steam_default_icon_locked.jpg" "%out_dir%\generate_emu_config\"
copy /y "steam_default_icon_unlocked.jpg" "%out_dir%\generate_emu_config\"
copy /y "README.md" "%out_dir%\generate_emu_config\"
1>"%out_dir%\generate_emu_config\my_login.EXAMPLE.txt" echo Check the README
1>"%out_dir%\generate_emu_config\top_owners_ids.EXAMPLE.txt" echo Check the README
1>>"%out_dir%\generate_emu_config\top_owners_ids.EXAMPLE.txt" echo You can use a website like: https://steamladder.com/games/

echo:
echo =============
echo Built inside: "%out_dir%\"


:script_end
if exist "%build_temp_dir%" (
    rmdir /s /q "%build_temp_dir%"
)
popd
endlocal & (
    exit /b %last_code%
)
