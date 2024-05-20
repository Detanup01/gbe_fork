@echo off
call "third-party\common\win\premake\premake5.exe" --os=windows --file="premake5.lua" genproto
call "third-party\common\win\premake\premake5.exe" --os=windows --file="premake5.lua" vs2022

:: VS WHERE to get MSBUILD
set "vswhere_exe=third-party\common\win\vswhere\vswhere.exe"
if not exist "%vswhere_exe%" (
  echo "vswhere.exe wasn't found"
  goto :end_script_with_err
)
set "my_vs_path=%vs_static_path%"
if "%my_vs_path%"=="" (
  for /f "tokens=* delims=" %%A in ('"%vswhere_exe%" -prerelease -latest -nocolor -nologo -property installationPath 2^>nul') do (
    set "my_vs_path=%%~A\MSBuild\Current\Bin\MSBuild.exe"
  )
)
goto :builder

:: exit with error
:end_script_with_err
popd
endlocal & (
  exit /b 1
)


:builder

:: -v:n make it so we can actually see what commands it runs
call %my_vs_path% build\project\vs2022\win\GBE.sln /p:Configuration=debug /p:Platform=x64 -v:n
set /a _exit=%errorlevel%
if %_exit% equ 1 (
    goto :end_script_with_err
)

call %my_vs_path% build\project\vs2022\win\GBE.sln /p:Configuration=debug /p:Platform=Win32 -v:n
set /a _exit=%errorlevel%
if %_exit% equ 1 (
    goto :end_script_with_err
)

call %my_vs_path% build\project\vs2022\win\GBE.sln /p:Configuration=release /p:Platform=x64 -v:n
set /a _exit=%errorlevel%
if %_exit% equ 1 (
    goto :end_script_with_err
)

call %my_vs_path% build\project\vs2022\win\GBE.sln /p:Configuration=release /p:Platform=Win32 -v:n
set /a _exit=%errorlevel%
if %_exit% equ 1 (
    goto :end_script_with_err
)