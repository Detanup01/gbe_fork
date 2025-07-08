@echo off
echo Building version_release.cpp...

REM Create output directories
if not exist "bin\release" mkdir "bin\release"
if not exist "tmp\release" mkdir "tmp\release"

REM Build Release configuration
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" version.vcxproj /p:Configuration=Release /p:Platform=x64

if %ERRORLEVEL% EQU 0 (
    echo Build successful! Output: bin\release\version.dll
    dir "bin\release\version.dll" | findstr version.dll
) else (
    echo Build failed!
    exit /b %ERRORLEVEL%
)

pause