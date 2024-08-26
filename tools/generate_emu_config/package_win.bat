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
set "BUILD_DIR=%ROOT%\bin\win"
set "OUT_DIR=%ROOT%\bin\package\win"

set /a "PKG_EXE_MEM_PERCENT=90"
set /a "PKG_EXE_DICT_SIZE_MB=384"
set "PKG_EXE=..\..\third-party\deps\win\7za\7za.exe"
if not exist "%PKG_EXE%" (
  1>&2 echo:packager wasn't found
  goto :end_script_with_err
)

if not exist "%BUILD_DIR%" (
  1>&2 echo:build folder wasn't found
  goto :end_script_with_err
)

if not exist "%OUT_DIR%" (
  mkdir "%OUT_DIR%"
)

set "ACHIVE_FILE=%OUT_DIR%\generate_emu_config-win.7z"
if exist "%ACHIVE_FILE%" (
  del /f /q "%ACHIVE_FILE%"
)

call "%PKG_EXE%" a "%ACHIVE_FILE%" "%BUILD_DIR%\*" -t7z -slp -ssw -mx -myx -mmemuse=p%PKG_EXE_MEM_PERCENT% -ms=on -mqs=off -mf=on -mhc+ -mhe- -m0=LZMA2:d=%PKG_EXE_DICT_SIZE_MB%m -mmt=%MAX_THREADS% -mmtf+ -mtm- -mtc- -mta- -mtr+ || (
  goto :end_script_with_err
)

goto :end_script

:end_script
  endlocal
  exit /b 0

:end_script_with_err
  endlocal
  exit /b 1
