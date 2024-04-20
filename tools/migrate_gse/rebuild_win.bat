@echo off

setlocal
pushd "%~dp0"

set "venv=.env-win"
set "out_dir=bin\win"
set "build_temp_dir=bin\tmp\win"
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

echo building migrate_gse...
pyinstaller "main.py" --distpath "%out_dir%" -y --clean --onedir --name "migrate_gse" --noupx --console -i "NONE" --workpath "%build_temp_dir%" || (
    set /a last_code=1
    goto :script_end
)
call "%signer_tool%" "%out_dir%\migrate_gse\migrate_gse.exe"

copy /y README.md "%out_dir%\migrate_gse\"

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
