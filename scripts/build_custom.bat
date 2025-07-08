@echo off
setlocal EnableDelayedExpansion

echo Reading build selection...

:: Read JSON configuration using PowerShell
for /f "delims=" %%i in ('powershell -Command "& {(Get-Content build_selection.json | ConvertFrom-Json).configs -join ' '}"') do set "BUILD_CONFIGS=%%i"
for /f "delims=" %%i in ('powershell -Command "& {(Get-Content build_selection.json | ConvertFrom-Json).platforms -join ' '}"') do set "BUILD_PLATFORMS=%%i"
for /f "delims=" %%i in ('powershell -Command "& {(Get-Content build_selection.json | ConvertFrom-Json).targets -join ' '}"') do set "ALL_TARGETS=%%i"

:: Check if package_docs was selected and remove it from build targets
set "PACKAGE_DOCS=0"
echo %ALL_TARGETS% | findstr /i "package_docs" >nul
if %errorlevel% equ 0 (
    set "PACKAGE_DOCS=1"
)

:: Remove package_docs from the target list for actual building
set "BUILD_TARGETS="
for %%i in (%ALL_TARGETS%) do (
    if /i not "%%i"=="package_docs" (
        if defined BUILD_TARGETS (
            set "BUILD_TARGETS=!BUILD_TARGETS! %%i"
        ) else (
            set "BUILD_TARGETS=%%i"
        )
    )
)

echo.
echo ======================================================
echo  Build Configuration Summary
echo ======================================================
echo  Configurations: %BUILD_CONFIGS%
echo  Platforms:      %BUILD_PLATFORMS%
echo  Targets:        %BUILD_TARGETS%
echo ======================================================
echo.

:: Set up build environment
set "PREMAKE_EXE=third-party\common\win\premake\premake5.exe"
set "VSWHERE_EXE=third-party\common\win\vswhere\vswhere.exe"

for /f "tokens=* delims=" %%A in ('"%VSWHERE_EXE%" -prerelease -latest -nocolor -nologo -property installationPath 2^>nul') do (
    set "MSBUILD_EXE=%%~A\MSBuild\Current\Bin\MSBuild.exe"
)

set /a "MAX_THREADS=2"
if defined NUMBER_OF_PROCESSORS (
    set /a "MAX_THREADS=%NUMBER_OF_PROCESSORS% * 70 / 100"
    if %MAX_THREADS% lss 1 (
        set /a "MAX_THREADS=1"
    )
)

echo Generating Visual Studio solution...
call "%PREMAKE_EXE%" --file="premake5.lua" --genproto --dosstub --winrsrc --winsign --os=windows vs2022 || (
    echo ERROR: Failed to generate solution files!
    pause
    exit /b 1
)

set "SLN_FILE=build\project\vs2022\win\gbe.sln"
if not exist "%SLN_FILE%" (
    echo ERROR: Solution file was not created!
    pause
    exit /b 1
)

echo.
echo ======================================================
echo  Starting Build Process
echo ======================================================
echo.

:: Count total builds for progress tracking
set /a "TOTAL_BUILDS=0"
set /a "CURRENT_BUILD=0"

for %%A in (%BUILD_CONFIGS%) do (
    for %%B in (%BUILD_PLATFORMS%) do (
        for %%C in (%BUILD_TARGETS%) do (
            set /a "TOTAL_BUILDS+=1"
        )
    )
)

echo Total builds to process: %TOTAL_BUILDS%
echo.

:: Execute builds
for %%A in (%BUILD_CONFIGS%) do (
    set "BUILD_CONFIG=%%A"
    for %%B in (%BUILD_PLATFORMS%) do (
        set "BUILD_PLATFORM=%%B"
        for %%C in (%BUILD_TARGETS%) do (
            set "BUILD_TARGET=%%C"
            set /a "CURRENT_BUILD+=1"
            
            echo.
            echo [!CURRENT_BUILD!/!TOTAL_BUILDS!] Building !BUILD_TARGET! !BUILD_CONFIG! !BUILD_PLATFORM!
            echo ------------------------------------------------------
            
            call "%MSBUILD_EXE%" /nologo -m:%MAX_THREADS% -v:n /p:Configuration=!BUILD_CONFIG!,Platform=!BUILD_PLATFORM! /target:!BUILD_TARGET! "%SLN_FILE%" || (
                echo.
                echo ERROR: Build failed for !BUILD_TARGET! !BUILD_CONFIG! !BUILD_PLATFORM!
                echo This error will be logged, but the build process will continue...
                echo.
                pause
            )
        )
    )
)

echo.
echo ======================================================
echo  Build Process Complete!
echo ======================================================
echo  Total builds attempted: %TOTAL_BUILDS%

:: Check if documentation packaging was requested
if %PACKAGE_DOCS% equ 1 (
    echo.
    echo ======================================================
    echo  Copying Documentation Files
    echo ======================================================
    echo.
    
    for %%A in (%BUILD_CONFIGS%) do (
        if /i "%%A"=="debug" (
            echo Copying debug documentation...
            if not exist "build\win\vs2022\debug\" mkdir "build\win\vs2022\debug\"
            copy /y "CHANGELOG.md" "build\win\vs2022\debug\"
            copy /y "CREDITS.md" "build\win\vs2022\debug\"
            copy /y "post_build\README.debug.md" "build\win\vs2022\debug\"
            if not exist "build\win\vs2022\debug\steam_settings.EXAMPLE\" mkdir "build\win\vs2022\debug\steam_settings.EXAMPLE\"
            xcopy /y /s /e "post_build\steam_settings.EXAMPLE\*" "build\win\vs2022\debug\steam_settings.EXAMPLE\"
        )
        if /i "%%A"=="release" (
            echo Copying release documentation...
            if not exist "build\win\vs2022\release\" mkdir "build\win\vs2022\release\"
            copy /y "CHANGELOG.md" "build\win\vs2022\release\"
            copy /y "CREDITS.md" "build\win\vs2022\release\"
            copy /y "post_build\README.release.md" "build\win\vs2022\release\"
            if not exist "build\win\vs2022\release\steam_settings.EXAMPLE\" mkdir "build\win\vs2022\release\steam_settings.EXAMPLE\"
            xcopy /y /s /e "post_build\steam_settings.EXAMPLE\*" "build\win\vs2022\release\steam_settings.EXAMPLE\"
        )
    )
)

:: Always copy steamclient_experimental specific files directly to build output
echo %BUILD_TARGETS% | findstr /i "steamclient_experimental" >nul
if %errorlevel% equ 0 (
    echo.
    echo Copying SteamClient Experimental files...
    
    for %%A in (%BUILD_CONFIGS%) do (
        if exist "build\win\vs2022\%%A\steamclient_experimental\" (
            echo Copying files for %%A build...
            copy /y "tools\steamclient_loader\win\ColdClientLoader.ini" "build\win\vs2022\%%A\steamclient_experimental\" >nul 2>&1
            copy /y "post_build\README.experimental_steamclient.md" "build\win\vs2022\%%A\steamclient_experimental\" >nul 2>&1
            if not exist "build\win\vs2022\%%A\steamclient_experimental\dll_injection.EXAMPLE\" mkdir "build\win\vs2022\%%A\steamclient_experimental\dll_injection.EXAMPLE" >nul 2>&1
            xcopy /y /s /e /q "post_build\win\ColdClientLoader.EXAMPLE\*" "build\win\vs2022\%%A\steamclient_experimental\dll_injection.EXAMPLE\" >nul 2>&1
        )
    )
)

echo.
echo  Build artifacts can be found in:
echo  - build\win\vs2022\debug\   (for debug builds)
echo  - build\win\vs2022\release\ (for release builds)
echo ======================================================

exit /b 0