@echo off
setlocal

:: use 70%
set /a build_threads=2
if defined NUMBER_OF_PROCESSORS (
  set /a build_threads=NUMBER_OF_PROCESSORS*70/100
)
if %build_threads% lss 1 (
  set /a build_threads=1
)

set /a BUILD_DEPS=0
:args_loop
  if "%~1"=="" (
    goto :args_loop_end
  ) else if "%~1"=="--deps" (
    set /a BUILD_DEPS=1
  ) else if "%~1"=="--help" (
    call :help_page
    goto :end_script
  ) else (
    1>&2 echo "invalid arg %~1"
    goto :end_script_with_err
  )
shift /1
goto :args_loop
:args_loop_end

set "premake_exe=third-party\common\win\premake\premake5.exe"
if not exist "%premake_exe%" (
  1>&2 echo "preamke wasn't found"
  goto :end_script_with_err
)

:: build deps
if %BUILD_DEPS%==0 (
  goto :build_deps_end
)
set "CMAKE_GENERATOR=Visual Studio 17 2022"
call "%premake_exe%" --file="premake5-deps.lua" --all-ext --all-build --64-build --32-build --verbose --clean --os=windows vs2022
if %errorlevel% neq 0 (
  goto :end_script_with_err
)
:build_deps_end

:: VS WHERE to get MSBUILD
set "vswhere_exe=third-party\common\win\vswhere\vswhere.exe"
if not exist "%vswhere_exe%" (
  1>&2 echo "vswhere wasn't found"
  goto :end_script_with_err
)

:: .sln file
set "sln_file=build\project\vs2022\win\gbe.sln"

:: get msbuild path
set "my_vs_path="
for /f "tokens=* delims=" %%A in ('"%vswhere_exe%" -prerelease -latest -nocolor -nologo -property installationPath 2^>nul') do (
  set "my_vs_path=%%~A\MSBuild\Current\Bin\MSBuild.exe"
)
if not exist "%my_vs_path%" (
  1>&2 echo "MSBuild wasn't found"
  goto :end_script_with_err
)

call "%premake_exe%" --file="premake5.lua" --dosstub --winrsrc --winsign --genproto --os=windows vs2022
if %errorlevel% neq 0 (
  goto :end_script_with_err
)
if not exist "%sln_file%" (
  1>&2 echo "project solution file wasn't found"
  goto :end_script_with_err
)

:: -v:n make it so we can actually see what commands it runs
echo: & echo building debug x64
call "%my_vs_path%" /nologo "%sln_file%" /p:Configuration=debug /p:Platform=x64 -v:n -m:%build_threads%
if %errorlevel% neq 0 (
  goto :end_script_with_err
)

echo: & echo building debug x32
call "%my_vs_path%" /nologo "%sln_file%" /p:Configuration=debug /p:Platform=Win32 -v:n -m:%build_threads%
if %errorlevel% neq 0 (
  goto :end_script_with_err
)

echo: & echo building release x64
call "%my_vs_path%" /nologo "%sln_file%" /p:Configuration=release /p:Platform=x64 -v:n -m:%build_threads%
if %errorlevel% neq 0 (
  goto :end_script_with_err
)

echo: & echo building release x32
call "%my_vs_path%" /nologo "%sln_file%" /p:Configuration=release /p:Platform=Win32 -v:n -m:%build_threads%
if %errorlevel% neq 0 (
  goto :end_script_with_err
)


:: if all ok
:end_script
endlocal
exit /b 0


:: exit with error
:end_script_with_err
endlocal
exit /b 1


:help_page
echo:
echo "%~nx0" [switches]
echo switches:
echo   --deps: rebuild third-party dependencies
echo   --help: show this page
exit /b 0
