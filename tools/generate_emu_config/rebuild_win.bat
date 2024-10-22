@echo off
setlocal EnableDelayedExpansion
cd /d "%~dp0"

set "ROOT=%cd%"
set "VENV=%ROOT%\.env-win"
set "OUT_DIR=%ROOT%\bin\win"
set "BUILD_TEMP_DIR=%ROOT%\bin\tmp\win"
set "ICON_FILE=%ROOT%\icon\Froyoshark-Enkel-Steam.ico"

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

echo:building generate_emu_config...
pyinstaller "generate_emu_config.py" --distpath "%OUT_DIR%" -y --clean --onedir --name "generate_emu_config" --noupx --console -i "%ICON_FILE%" --collect-submodules "steam" --workpath "%BUILD_TEMP_DIR%" --specpath "%BUILD_TEMP_DIR%" || (
  set /a "LAST_ERR_CODE=1"
  goto :end_script
)
call "%SIGNER_TOOL%" "%OUT_DIR%\generate_emu_config\generate_emu_config.exe"

echo:building parse_controller_vdf...
pyinstaller "controller_config_generator\parse_controller_vdf.py" --distpath "%OUT_DIR%" -y --clean --onedir --name "parse_controller_vdf" --noupx --console -i "NONE" --workpath "%BUILD_TEMP_DIR%" --specpath "%BUILD_TEMP_DIR%" || (
  set /a "LAST_ERR_CODE=1"
  goto :end_script
)
call "%SIGNER_TOOL%" "%OUT_DIR%\parse_controller_vdf\parse_controller_vdf.exe"

echo:building parse_achievements_schema...
pyinstaller "stats_schema_achievement_gen\achievements_gen.py" --distpath "%OUT_DIR%" -y --clean --onedir --name "parse_achievements_schema" --noupx --console -i "NONE" --workpath "%BUILD_TEMP_DIR%" --specpath "%BUILD_TEMP_DIR%" || (
  set /a "LAST_ERR_CODE=1"
  goto :end_script
)
call "%SIGNER_TOOL%" "%OUT_DIR%\parse_achievements_schema\parse_achievements_schema.exe"

call ".\bat\_pack_api.bat"
call ".\bat\_pack_exe.bat"

xcopy /s /y /e "post_build" "%out_dir%\generate_emu_config\"
xcopy /s /y /e "_DEFAULT" "%out_dir%\generate_emu_config\_DEFAULT\"

copy /y "steam_default_icon_locked.jpg" "%out_dir%\generate_emu_config\"
copy /y "steam_default_icon_unlocked.jpg" "%out_dir%\generate_emu_config\"
copy /y "README.md" "%out_dir%\generate_emu_config\"
if exist "top_owners_ids.txt" (
	copy /y "top_owners_ids.txt" "%out_dir%\generate_emu_config\"
) else (
echo Check the README>> "%OUT_DIR%\generate_emu_config\my_login.EXAMPLE.txt"
echo Check the README>> "%OUT_DIR%\generate_emu_config\top_owners_ids.EXAMPLE.txt"
echo You can use a website like: https://steamladder.com/games/>> "%OUT_DIR%\generate_emu_config\top_owners_ids.EXAMPLE.txt"
)

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