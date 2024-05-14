@echo off
if exist "premake/premake5.exe" (
    goto :premakerun
)
curl.exe -L  "https://github.com/premake/premake-core/releases/download/v5.0.0-beta2/premake-5.0.0-beta2-windows.zip" --ssl-no-revoke --output premake.zip
call "third-party/deps/win/7za/7za.exe" x -y premake.zip -opremake -aoa
:premakerun
call "premake/premake5.exe" --os=windows --file="premake5.lua" generateproto
call "premake/premake5.exe" --os=windows --file="premake5.lua" vs2022

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
:: Set configuration and platform here :)

:: -v:n make it so we can actually see what commands it runs
call %my_vs_path% GBE_Build\GBE.sln /p:Configuration=Release /p:Platform=x64 -v:n
set /a _exit=%errorlevel%
if %_exit% equ 0 (
    echo Please do change_dos_stub and sign it. (or you can move into premake to do it for you)
    ::call :change_dos_stub 
    ::call "%signer_tool%"
) else (
    goto :end_script_with_err
)
exit /b

:: exit with error
:end_script_with_err
popd
endlocal & (
  exit /b 1
)