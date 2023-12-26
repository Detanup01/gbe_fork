@echo off

setlocal
pushd "%~dp0"

:: Put in the base path in which Visual Studio is installed, examples:
::set "vs_static_path=C:\Program Files\Microsoft Visual Studio\2022\BuildTools"
::set "vs_static_path=C:\Program Files\Microsoft Visual Studio\2022\Community"
::set "vs_static_path=C:\Program Files\Microsoft Visual Studio\2022\Enterprise"
set "vs_static_path="

set "vswhere_exe=third-party\common\win\vswhere\vswhere.exe"

if not exist "%vswhere_exe%" (
  call :err_msg "vswhere.exe wasn't found"
  goto :end_script_with_err
)

set "arch="
if "%~1"=="32" (
  set "arch=32"
) else if "%~1"=="64" (
  set "arch=64"
) else (
  call :err_msg "First arg may be any of [32 64]"
  goto :end_script_with_err
)


set "my_vs_path=%vs_static_path%"
if "%my_vs_path%"=="" (
  for /f "tokens=* delims=" %%A in ('"%vswhere_exe%" -prerelease -latest -nocolor -nologo -property installationPath 2^>nul') do (
    set "my_vs_path=%%~A\VC\Auxiliary\Build"
  )
)

if not exist "%my_vs_path%\vcvars%arch%.bat" (
  set "my_vs_path="
)

if "%my_vs_path%"=="" (
  call :err_msg "Visual Studio couldn't be found, set its path in the script %~nx0"
  goto :end_script_with_err
)

echo:
echo Using Visual Studio found in: "%my_vs_path%"
popd
endlocal & (
  call "%my_vs_path%\vcvars%arch%.bat" && (
    echo: & echo:
    exit /b 0
  ) || (
    1>&2 echo [X] Visual Studio script "vcvars%arch%.bat" failed
    echo: & echo:
    exit /b 1
  )
)


:err_msg
    1>&2 echo [X] %~1
exit /b

:end_script_with_err
popd
endlocal & (
  exit /b 1
)
