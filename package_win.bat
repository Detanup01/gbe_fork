@echo off

setlocal
pushd "%~dp0"

set /a last_code=0

set "build_base_dir=build\win"
set "out_dir=build\package\win"

set /a MEM_PERCENT=90
set /a DICT_SIZE_MB=384
set "packager=third-party\deps\win\7za\7za.exe"

:: use 70%
set /a THREAD_COUNT=2
if defined NUMBER_OF_PROCESSORS (
  set /a THREAD_COUNT=NUMBER_OF_PROCESSORS*70/100
)
if %THREAD_COUNT% lss 2 (
  set /a THREAD_COUNT=2
)

if not exist "%packager%" (
  1>&2 echo [X] packager app wasn't found
  set /a last_code=1
  goto :script_end
)

if "%~1"=="" (
  1>&2 echo [X] missing build folder
  set /a last_code=1
  goto :script_end
)

set "target_src_dir=%build_base_dir%\%~1"
if not exist "%target_src_dir%" (
  1>&2 echo [X] build folder wasn't found
  set /a last_code=1
  goto :script_end
)

::::::::::::::::::::::::::::::::::::::::::
echo // copying readmes + example files
xcopy /y /s /e /r "post_build\steam_settings.EXAMPLE\" "%target_src_dir%\steam_settings.EXAMPLE\"
copy /y "post_build\README.release.md" "%target_src_dir%\"
copy /y "CHANGELOG.md" "%target_src_dir%\"
if "%~2"=="1" (
  copy /y "post_build\README.debug.md" "%target_src_dir%\"
)
if exist "%target_src_dir%\experimental\" (
  copy /y "post_build\README.experimental.md" "%target_src_dir%\experimental\"
)
if exist "%target_src_dir%\steamclient_experimental\" (
  xcopy /y /s /e /r "post_build\win\ColdClientLoader.EXAMPLE\" "%target_src_dir%\steamclient_experimental\dll_injection.EXAMPLE\"
  copy /y "post_build\README.experimental_steamclient.md" "%target_src_dir%\steamclient_experimental\"
  copy /y "tools\steamclient_loader\win\ColdClientLoader.ini" "%target_src_dir%\steamclient_experimental\"
)
if exist "%target_src_dir%\tools\generate_interfaces\" (
  copy /y "post_build\README.generate_interfaces.md" "%target_src_dir%\tools\generate_interfaces\"
)
if exist "%target_src_dir%\tools\lobby_connect\" (
  copy /y "post_build\README.lobby_connect.md" "%target_src_dir%\tools\lobby_connect\"
)
::::::::::::::::::::::::::::::::::::::::::

set "archive_dir=%out_dir%\%~1"
if exist "%archive_dir%\" (
  rmdir /s /q "%archive_dir%"
)
set "archive_file="
for %%A in ("%archive_dir%") do (
  set "archive_file=%archive_dir%\emu-win-%%~nxA.7z"
)

mkdir "%archive_dir%"
"%packager%" a "%archive_file%" ".\%target_src_dir%" -t7z -slp -ssw -mx -myx -mmemuse=p%MEM_PERCENT% -ms=on -mqs=off -mf=on -mhc+ -mhe- -m0=LZMA2:d=%DICT_SIZE_MB%m -mmt=%THREAD_COUNT% -mmtf+ -mtm- -mtc- -mta- -mtr+


:script_end
popd
endlocal & (
    exit /b %last_code%
)
