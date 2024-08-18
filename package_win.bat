@echo off
setlocal EnableDelayedExpansion
cd /d "%~dp0"

set /a "MAX_THREADS=2"
if defined NUMBER_OF_PROCESSORS (
  :: use 70%
  set /a "MAX_THREADS=%NUMBER_OF_PROCESSORS% * 70 / 100"
  if %MAX_THREADS% lss 1 (
    set /a "MAX_THREADS=1"
  )
)

set "BUILD_DIR=build\win"
set "OUT_DIR=build\package\win"

if "%~1" equ "" (
  1>&2 echo: missing build target folder arg
  goto :end_script_with_err
)

set "TARGET_DIR=%BUILD_DIR%\%~1"
if not exist "%TARGET_DIR%\" (
  1>&2 echo: build target folder wasn't found
  goto :end_script_with_err
)

set /a "BUILD_DEBUG=0"
if "%~2" equ "1" (
  set /a "BUILD_DEBUG=1"
)

set /a "PKG_EXE_MEM_PERCENT=90"
set /a "PKG_EXE_DICT_SIZE_MB=384"
set "PKG_EXE=third-party\deps\win\7za\7za.exe"
if not exist "%PKG_EXE%" (
  1>&2 echo: packager wasn't found
  goto :end_script_with_err
)

::::::::::::::::::::::::::::::::::::::::::
echo: // copying readmes + example files

xcopy /y /s /e /r "post_build\steam_settings.EXAMPLE\" "%TARGET_DIR%\steam_settings.EXAMPLE\"

copy /y "post_build\README.release.md" "%TARGET_DIR%\"
copy /y "CHANGELOG.md" "%TARGET_DIR%\"
copy /y "CREDITS.md" "%TARGET_DIR%\"

if %BUILD_DEBUG% equ 1 (
  copy /y "post_build\README.debug.md" "%TARGET_DIR%\"
)

if exist "%TARGET_DIR%\experimental\" (
  copy /y "post_build\README.experimental.md" "%TARGET_DIR%\experimental\"
)

if exist "%TARGET_DIR%\steamclient_experimental\" (
  xcopy /y /s /e /r "post_build\win\ColdClientLoader.EXAMPLE\" "%TARGET_DIR%\steamclient_experimental\dll_injection.EXAMPLE\"
  copy /y "post_build\README.experimental_steamclient.md" "%TARGET_DIR%\steamclient_experimental\"
  copy /y "tools\steamclient_loader\win\ColdClientLoader.ini" "%TARGET_DIR%\steamclient_experimental\"
)

if exist "%TARGET_DIR%\tools\generate_interfaces\" (
  copy /y "post_build\README.generate_interfaces.md" "%TARGET_DIR%\tools\generate_interfaces\"
)

if exist "%TARGET_DIR%\tools\lobby_connect\" (
  copy /y "post_build\README.lobby_connect.md" "%TARGET_DIR%\tools\lobby_connect\"
)
::::::::::::::::::::::::::::::::::::::::::

set "ACHIVE_DIR=%OUT_DIR%\%~1"
if exist "%ACHIVE_DIR%\" (
  rmdir /s /q "%ACHIVE_DIR%"
)

for %%A in ("%ACHIVE_DIR%") do (
  md "%%~dpA"
)

set "ACHIVE_FILE="
for %%A in ("%ACHIVE_DIR%") do (
  set "ACHIVE_FILE=%%~dpAemu-win-%%~nxA.7z"
)

if exist "%ACHIVE_FILE%" (
  del /f /s /q "%ACHIVE_FILE%"
)

call "%PKG_EXE%" a "%ACHIVE_FILE%" ".\%TARGET_DIR%" -t7z -xr^^!*.lib -xr^^!*.exp -slp -ssw -mx -myx -mmemuse=p%PKG_EXE_MEM_PERCENT% -ms=on -mqs=off -mf=on -mhc+ -mhe- -m0=LZMA2:d=%PKG_EXE_DICT_SIZE_MB%m -mmt=%MAX_THREADS% -mmtf+ -mtm- -mtc- -mta- -mtr+
if %errorlevel% neq 0 (
  goto :end_script_with_err
)

goto :end_script

:: exit without error
:end_script
  endlocal
  exit /b 0

:: exit with error
:end_script_with_err
  endlocal
  exit /b 1
