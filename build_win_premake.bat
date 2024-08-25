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

set /a "BUILD_DEPS=0"

:args_loop
  if "%~1" equ "" (
    goto :args_loop_end
  ) else if "%~1" equ "--deps" (
    set /a "BUILD_DEPS=1"
  ) else if "%~1" equ "--help" (
    goto :help_page
  ) else (
    1>&2 echo:invalid arg %~1
    goto :end_script_with_err
  )

  shift /1
  goto :args_loop

:args_loop_end
  :: check premake
  set "PREMAKE_EXE=third-party\common\win\premake\premake5.exe"
  if not exist "%PREMAKE_EXE%" (
    1>&2 echo:premake wasn't found
    goto :end_script_with_err
  )

  :: build deps
  if %BUILD_DEPS% equ 1 (
    set "CMAKE_GENERATOR=Visual Studio 17 2022"
    call "%PREMAKE_EXE%" --file="premake5-deps.lua" --64-build --32-build --all-ext --all-build --j=2 --verbose --clean --os=windows vs2022 || (
      goto :end_script_with_err
    )
    goto :end_script
  )

  :: check vswhere
  set "VSWHERE_EXE=third-party\common\win\vswhere\vswhere.exe"
  if not exist "%VSWHERE_EXE%" (
    1>&2 echo:vswhere wasn't found
    goto :end_script_with_err
  )

  :: check msbuild
  set "MSBUILD_EXE="
  for /f "tokens=* delims=" %%A in ('"%VSWHERE_EXE%" -prerelease -latest -nocolor -nologo -property installationPath 2^>nul') do (
    set "MSBUILD_EXE=%%~A\MSBuild\Current\Bin\MSBuild.exe"
  )
  if not exist "%MSBUILD_EXE%" (
    1>&2 echo:MSBuild wasn't found
    goto :end_script_with_err
  )

  :: create .sln
  call "%PREMAKE_EXE%" --file="premake5.lua" --genproto --dosstub --winrsrc --winsign --os=windows vs2022 || (
    goto :end_script_with_err
  )

  :: check .sln
  set "SLN_FILE=build\project\vs2022\win\gbe.sln"
  if not exist "%SLN_FILE%" (
    1>&2 echo:.sln file wasn't found
    goto :end_script_with_err
  )

  :: build .sln
  set "BUILD_TYPES=release debug"
  set "BUILD_PLATFORMS=x64 Win32"
  set "BUILD_TARGETS=api_regular api_experimental steamclient_experimental_stub steamclient_experimental steamclient_experimental_loader steamclient_experimental_extra lib_game_overlay_renderer tool_lobby_connect tool_generate_interfaces"

  for %%A in (%BUILD_TYPES%) do (
    set "BUILD_TYPE=%%A"
    for %%B in (%BUILD_PLATFORMS%) do (
      set "BUILD_PLATFORM=%%B"
      for %%C in (%BUILD_TARGETS%) do (
        set "BUILD_TARGET=%%C"
        echo. & echo:building !BUILD_TARGET! !BUILD_TYPE! !BUILD_PLATFORM!
        call "%MSBUILD_EXE%" /nologo -m:%MAX_THREADS% -v:n /p:Configuration=!BUILD_TYPE!,Platform=!BUILD_PLATFORM! /target:!BUILD_TARGET! "%SLN_FILE%" || (
          goto :end_script_with_err
        )
      )
    )
  )

  goto :end_script

:end_script
  endlocal
  exit /b 0

:end_script_with_err
  endlocal
  exit /b 1

:: show help page
:help_page
  echo:"%~nx0" [switches]
  echo:switches:
  echo:  --deps: rebuild third-party dependencies
  echo:  --help: show this page
  goto :end_script
