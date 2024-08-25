@echo off
setlocal EnableDelayedExpansion
cd /d "%~dp0"

set "ROOT=%cd%"
set "VENV=%ROOT%\.env-win"
set "OUT_DIR=%ROOT%\bin\win"
set "BUILD_TEMP_DIR=%ROOT%\bin\tmp\win"

set /a "LAST_ERR_CODE=0"

set "SIGNER_TOOL=..\..\third-party\build\win\cert\sign_helper.bat"
if not exist "%SIGNER_TOOL%" (
  1>&2 echo:signing tool wasn't found
  set /a "LAST_ERR_CODE=1"
  goto :end_script
)

if exist "%OUT_DIR%" (
  rmdir /s /q "%OUT_DIR%"
)
mkdir "%OUT_DIR%"

if exist "%BUILD_TEMP_DIR%" (
  rmdir /s /q "%BUILD_TEMP_DIR%"
)

call "%VENV%\Scripts\activate.bat"

echo:building migrate_gse...
pyinstaller "main.py" --distpath "%OUT_DIR%" -y --clean --onedir --name "migrate_gse" --noupx --console -i "NONE" --workpath "%BUILD_TEMP_DIR%" --specpath "%BUILD_TEMP_DIR%" || (
  set /a "LAST_ERR_CODE=1"
  goto :end_script
)
call "%SIGNER_TOOL%" "%OUT_DIR%\migrate_gse\migrate_gse.exe"

copy /y "README.md" "%out_dir%\migrate_gse\"

echo:
echo:=============
echo:Built inside: "%OUT_DIR%\"

goto :end_script

:end_script
  if exist "%BUILD_TEMP_DIR%" (
    rmdir /s /q "%BUILD_TEMP_DIR%"
  )

  endlocal
  exit /b %LAST_ERR_CODE%
