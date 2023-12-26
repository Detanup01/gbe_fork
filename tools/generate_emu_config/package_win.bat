@echo off

setlocal
pushd "%~dp0"

set /a last_code=0

set "build_dir=bin\win"
set "tool_name=generate_emu_config"
set "out_dir=bin\package\win"

set /a MEM_PERCENT=90
set /a DICT_SIZE_MB=384
set "packager=..\..\third-party\deps\win\7za\7za.exe"

:: use 70%
if defined NUMBER_OF_PROCESSORS (
  set /a THREAD_COUNT=NUMBER_OF_PROCESSORS*70/100
) else (
  set /a THREAD_COUNT=2
)

if not exist "%packager%" (
  1>&2 echo "[X] packager app wasn't found"
  set /a last_code=1
  goto :script_end
)

if not exist "%build_dir%" (
  1>&2 echo "[X] build folder wasn't found"
  set /a last_code=1
  goto :script_end
)

mkdir "%out_dir%"

set "archive_file=%out_dir%\%tool_name%-win.7z"
if exist "%archive_file%" (
    del /f /q "%archive_file%"
)

"%packager%" a "%archive_file%" ".\%build_dir%\%tool_name%" -t7z -slp -ssw -mx -myx -mmemuse=p%MEM_PERCENT% -ms=on -mqs=off -mf=on -mhc+ -mhe- -m0=LZMA2:d=%DICT_SIZE_MB%m -mmt=%THREAD_COUNT% -mmtf+ -mtm- -mtc- -mta- -mtr+


:script_end
popd
endlocal & (
    exit /b %last_code%
)
