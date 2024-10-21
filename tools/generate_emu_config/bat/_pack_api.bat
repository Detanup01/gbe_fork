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

echo:// packing latest steam_api.dll for generate_emu_config

set "ACHIVE_DIR=..\_DEFAULT\0\steam_api"

if exist "%ACHIVE_DIR%\" (
  rmdir /s /q "%ACHIVE_DIR%"
)
mkdir "..\_DEFAULT\0\steam_api\release"
mkdir "..\_DEFAULT\0\steam_api\debug"

set "TARGET_DIR=%BUILD_DIR%\release"
xcopy /y "%TARGET_DIR%\experimental\x32\steam_api.dll" "..\_DEFAULT\0\steam_api\release\steam_api.dll"*
xcopy /y "%TARGET_DIR%\experimental\x32\steam_api.dll" "..\_DEFAULT\0\steam_api.dll"*
set "TARGET_DIR=%BUILD_DIR%\debug"
xcopy /y "%TARGET_DIR%\experimental\x32\steam_api.dll" "..\_DEFAULT\0\steam_api\debug\steam_api.dll"*
rem xcopy /y "%TARGET_DIR%\experimental\x32\steam_api.dll" "..\_DEFAULT\0\steam_api.dll"* rem do not overwrite dll with debug version

set "ACHIVE_FILE=..\_DEFAULT\0\steam_api.7z"
if exist "%ACHIVE_FILE%" (
  del /f /q "%ACHIVE_FILE%"
)

call "%PKG_EXE%" a "%ACHIVE_FILE%" "%ACHIVE_DIR%\*" -t7z -xr^^!*.lib -xr^^!*.exp -slp -ssw -mx -myx -mmemuse=p%PKG_EXE_MEM_PERCENT% -ms=on -mqs=off -mf=on -mhc+ -mhe- -m0=LZMA2:d=%PKG_EXE_DICT_SIZE_MB%m -mmt=%MAX_THREADS% -mmtf+ -mtm- -mtc- -mta- -mtr+ || (
  goto :end_script_with_err
)

rmdir /s /q "%ACHIVE_DIR%"

echo:// packing latest steam_api64.dll for generate_emu_config

set "ACHIVE_DIR=..\_DEFAULT\0\steam_api64"

if exist "%ACHIVE_DIR%\" (
  rmdir /s /q "%ACHIVE_DIR%"
)
mkdir "..\_DEFAULT\0\steam_api64\release"
mkdir "..\_DEFAULT\0\steam_api64\debug"

set "TARGET_DIR=%BUILD_DIR%\release"
xcopy /y "%TARGET_DIR%\experimental\x64\steam_api64.dll" "..\_DEFAULT\0\steam_api64\release\steam_api64.dll"*
xcopy /y "%TARGET_DIR%\experimental\x64\steam_api64.dll" "..\_DEFAULT\0\steam_api64.dll"*
set "TARGET_DIR=%BUILD_DIR%\debug"
xcopy /y "%TARGET_DIR%\experimental\x64\steam_api64.dll" "..\_DEFAULT\0\steam_api64\debug\steam_api64.dll"*
rem xcopy /y "%TARGET_DIR%\experimental\x64\steam_api64.dll" "..\_DEFAULT\0\steam_api64.dll"* rem do not overwrite dll with debug version

set "ACHIVE_FILE=..\_DEFAULT\0\steam_api64.7z"
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
