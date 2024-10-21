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

set "ROOT=%cd%"
set "BUILD_DIR=..\..\..\build\win\vs2022"
set "OUT_DIR=..\build\package\win"

if not exist "%BUILD_DIR%\release\" (
  1>&2 echo:release build target folder wasn't found
  goto :end_script_with_err
)
if not exist "%BUILD_DIR%\debug\" (
  1>&2 echo:debug build target folder wasn't found
  goto :end_script_with_err
)

set /a "PKG_EXE_MEM_PERCENT=90"
set /a "PKG_EXE_DICT_SIZE_MB=384"
set "PKG_EXE=..\..\..\third-party\deps\win\7za\7za.exe"
if not exist "%PKG_EXE%" (
  1>&2 echo:packager wasn't found
  goto :end_script_with_err
)

::::::::::::::::::::::::::::::::::::::::::

echo:// packing latest generate_interfaces.exe for generate_emu_config

set "ACHIVE_DIR=..\_DEFAULT\0\steam_misc\tools\generate_interfaces\generate_interfaces"

if exist "%ACHIVE_DIR%\" (
  rmdir /s /q "%ACHIVE_DIR%"
)
mkdir "..\_DEFAULT\0\steam_misc\tools\generate_interfaces\generate_interfaces"

set "TARGET_DIR=%BUILD_DIR%\release"
xcopy /y "%TARGET_DIR%\tools\generate_interfaces\generate_interfaces_x32.exe" "..\_DEFAULT\0\steam_misc\tools\generate_interfaces\generate_interfaces\generate_interfaces.exe"*
xcopy /y "%TARGET_DIR%\tools\generate_interfaces\generate_interfaces_x64.exe" "..\_DEFAULT\0\steam_misc\tools\generate_interfaces\generate_interfaces\generate_interfaces64.exe"*

set "ACHIVE_FILE=..\_DEFAULT\0\steam_misc\tools\generate_interfaces\generate_interfaces.7z"
if exist "%ACHIVE_FILE%" (
  del /f /q "%ACHIVE_FILE%"
)

call "%PKG_EXE%" a "%ACHIVE_FILE%" "%ACHIVE_DIR%\*" -t7z -xr^^!*.lib -xr^^!*.exp -slp -ssw -mx -myx -mmemuse=p%PKG_EXE_MEM_PERCENT% -ms=on -mqs=off -mf=on -mhc+ -mhe- -m0=LZMA2:d=%PKG_EXE_DICT_SIZE_MB%m -mmt=%MAX_THREADS% -mmtf+ -mtm- -mtc- -mta- -mtr+ || (
  goto :end_script_with_err
)

rmdir /s /q "%ACHIVE_DIR%"

echo:// packing latest lobby_connect.exe for generate_emu_config

set "ACHIVE_DIR=..\_DEFAULT\0\steam_misc\tools\lobby_connect\lobby_connect"

if exist "%ACHIVE_DIR%\" (
  rmdir /s /q "%ACHIVE_DIR%"
)
mkdir "..\_DEFAULT\0\steam_misc\tools\lobby_connect\lobby_connect"

set "TARGET_DIR=%BUILD_DIR%\release"
xcopy /y "%TARGET_DIR%\tools\lobby_connect\lobby_connect_x32.exe" "..\_DEFAULT\0\steam_misc\tools\lobby_connect\lobby_connect\lobby_connect.exe"*
xcopy /y "%TARGET_DIR%\tools\lobby_connect\lobby_connect_x64.exe" "..\_DEFAULT\0\steam_misc\tools\lobby_connect\lobby_connect\lobby_connect64.exe"*

set "ACHIVE_FILE=..\_DEFAULT\0\steam_misc\tools\lobby_connect\lobby_connect.7z"
if exist "%ACHIVE_FILE%" (
  del /f /q "%ACHIVE_FILE%"
)

call "%PKG_EXE%" a "%ACHIVE_FILE%" "%ACHIVE_DIR%\*" -t7z -xr^^!*.lib -xr^^!*.exp -slp -ssw -mx -myx -mmemuse=p%PKG_EXE_MEM_PERCENT% -ms=on -mqs=off -mf=on -mhc+ -mhe- -m0=LZMA2:d=%PKG_EXE_DICT_SIZE_MB%m -mmt=%MAX_THREADS% -mmtf+ -mtm- -mtc- -mta- -mtr+ || (
  goto :end_script_with_err
)

rmdir /s /q "%ACHIVE_DIR%"

::::::::::::::::::::::::::::::::::::::::::

goto :end_script

:end_script
  endlocal
  exit /b 0

:end_script_with_err
  endlocal
  exit /b 1
