@echo off

setlocal
pushd "%~dp0"

set "venv=.env"
set "out_dir=bin"
set "build_temp_dir=build_tmp"
set "tool_name=generate_emu_config"
set "icon_file=icon\Froyoshark-Enkel-Steam.ico"
set "main_file=generate_emu_config.py"

if exist "%out_dir%" (
    rmdir /s /q "%out_dir%"
)

if exist "%build_temp_dir%" (
    rmdir /s /q "%build_temp_dir%"
)

del /f /q "*.spec"

call "%venv%\Scripts\activate.bat"

pyinstaller "%main_file%" --distpath "%out_dir%" -y --clean --onedir --name "%tool_name%" --noupx --console -i "%icon_file%" --workpath "%build_temp_dir%" --collect-submodules "steam"

copy /y "steam_default_icon_locked.jpg" "%out_dir%\%tool_name%\"
copy /y "steam_default_icon_unlocked.jpg" "%out_dir%\%tool_name%\"

echo:
echo =============
echo Built inside : "%out_dir%\"

:script_end
popd
endlocal

