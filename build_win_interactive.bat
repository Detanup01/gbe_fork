@echo off
setlocal EnableDelayedExpansion
cd /d "%~dp0"

echo.
echo ======================================================
echo  GBE Fork Interactive Builder
echo  Professional Build System with GUI
echo ======================================================
echo.

:: Check for PowerShell
where powershell >nul 2>nul
if %errorlevel% neq 0 (
    echo ERROR: PowerShell not found! Please install PowerShell.
    pause
    exit /b 1
)

:: Check premake
set "PREMAKE_EXE=third-party\common\win\premake\premake5.exe"
if not exist "%PREMAKE_EXE%" (
    echo ERROR: Premake5 not found at %PREMAKE_EXE%
    echo Please build dependencies first: build_win_premake.bat --deps
    pause
    exit /b 1
)

:: Check vswhere  
set "VSWHERE_EXE=third-party\common\win\vswhere\vswhere.exe"
if not exist "%VSWHERE_EXE%" (
    echo ERROR: vswhere not found at %VSWHERE_EXE%
    pause
    exit /b 1
)

:: Check MSBuild
set "MSBUILD_EXE="
for /f "tokens=* delims=" %%A in ('"%VSWHERE_EXE%" -prerelease -latest -nocolor -nologo -property installationPath 2^>nul') do (
    set "MSBUILD_EXE=%%~A\MSBuild\Current\Bin\MSBuild.exe"
)
if not exist "%MSBUILD_EXE%" (
    echo ERROR: MSBuild not found! Please install Visual Studio 2022.
    pause
    exit /b 1
)

:: Calculate max threads
set /a "MAX_THREADS=2"
if defined NUMBER_OF_PROCESSORS (
    set /a "MAX_THREADS=%NUMBER_OF_PROCESSORS% * 70 / 100"
    if %MAX_THREADS% lss 1 (
        set /a "MAX_THREADS=1"
    )
)

echo Starting GUI Builder...
echo.

:: Launch PowerShell GUI
powershell -ExecutionPolicy Bypass -File "scripts\build_gui.ps1"

:: Check if user made a selection
if exist "build_selection.json" (
    echo.
    echo ======================================================
    echo  Executing Build Process
    echo ======================================================
    echo.
    call scripts\build_custom.bat
    del build_selection.json >nul 2>nul
    echo.
    echo ======================================================
    echo  Build Process Complete!
    echo ======================================================
) else (
    echo Build cancelled by user.
)

echo.