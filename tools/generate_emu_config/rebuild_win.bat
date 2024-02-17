@echo off

setlocal
pushd "%~dp0"

set "venv=.env-win"
set "out_dir=bin\win"
set "build_temp_dir=bin\tmp\win"
set "tool_name=generate_emu_config"
set "icon_file=icon\Froyoshark-Enkel-Steam.ico"
set "main_file=generate_emu_config.py"
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

pyinstaller "%main_file%" --distpath "%out_dir%" -y --clean --onedir --name "%tool_name%" --noupx --console -i "%icon_file%" --workpath "%build_temp_dir%" --collect-submodules "steam" || (
    set /a last_code=1
    goto :script_end
)
for /f "usebackq tokens=* delims=" %%A in ('"%main_file%"') do (
    call "%signer_tool%" "%out_dir%\%tool_name%\%%~nA.exe"
)

copy /y "steam_default_icon_locked.jpg" "%out_dir%\%tool_name%\"
copy /y "steam_default_icon_unlocked.jpg" "%out_dir%\%tool_name%\"
copy /y "README.md" "%out_dir%\%tool_name%\"
1>"%out_dir%\%tool_name%\my_login.EXAMPLE.txt" echo Check the README
1>"%out_dir%\%tool_name%\top_owners_ids.EXAMPLE.txt" echo Check the README
1>>"%out_dir%\%tool_name%\top_owners_ids.EXAMPLE.txt" echo You can use a website like: https://steamladder.com/ladder/games/

echo:
echo =============
echo Built inside: "%out_dir%\"

if exist "%build_temp_dir%" (
    rmdir /s /q "%build_temp_dir%"
)

:script_end
popd
endlocal & (
    exit /b %last_code%
)
