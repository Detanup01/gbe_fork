@echo off
cd /d "%~dp0"
rem Put in the base path in which Visual Studio is installed, default would be C:\Program Files (x86)

set VS_Base_Path=C:\Program Files (x86)

if exist "%VS_Base_Path%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat" goto vs2019
if exist "%VS_Base_Path%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat" goto vs2017
if exist "%VS_Base_Path%\Microsoft Visual Studio 14.0\VC\bin\amd64\vcvars64.bat" goto vs14
if exist "%VS_Base_Path%\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat" goto vs2019_bt
if exist "%VS_Base_Path%\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvars64.bat" goto vs2017_bt
if exist "%VS_Base_Path%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" goto vs2022
if exist "%VS_Base_Path%\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat" goto vs2022_bt
if exist ".\sdk_standalone\set_vars64.bat" goto gitlabci

:vs2022
call "%VS_Base_Path%\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
goto batend

:vs2019
call "%VS_Base_Path%\Microsoft Visual Studio\2019\Community\VC\Auxiliary\Build\vcvars64.bat"
goto batend

:vs2017
call "%VS_Base_Path%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars64.bat"
goto batend

:vs14
call "%VS_Base_Path%\Microsoft Visual Studio 14.0\VC\bin\amd64\vcvars64.bat"
goto batend

:vs2022_bt
call "%VS_Base_Path%\Microsoft Visual Studio\2022\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
goto batend

:vs2019_bt
call "%VS_Base_Path%\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
goto batend

:vs2017_bt
call "%VS_Base_Path%\Microsoft Visual Studio\2017\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
goto batend

:gitlabci
call ".\sdk_standalone\set_vars64.bat"
goto batend
:batend
