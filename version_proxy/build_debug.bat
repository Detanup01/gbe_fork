@echo off
echo Building version_debug.cpp...

REM Create output directories
if not exist "bin\debug" mkdir "bin\debug"
if not exist "tmp\debug" mkdir "tmp\debug"

REM Build Release configuration with version_debug.cpp
"C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" version.vcxproj /p:Configuration=Release /p:Platform=x64 /p:OutDir=bin\debug\ /p:IntDir=tmp\debug\ /p:DebugBuild=true

if %ERRORLEVEL% EQU 0 (
    echo Build successful! Output: bin\debug\version.dll
    dir "bin\debug\version.dll" | findstr version.dll
) else (
    echo Build failed!
    exit /b %ERRORLEVEL%
)

pause