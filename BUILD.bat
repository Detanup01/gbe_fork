@echo off
:: GBE Fork Quick Build Launcher
:: Choose your build method

title GBE Fork Builder

echo.
echo  ===============================================
echo   GBE Fork Professional Build System
echo  ===============================================
echo.
echo  Choose your build method:
echo.
echo  [1] Interactive GUI Builder    (Recommended)
echo  [2] Quick Hybrid Build         (Fastest)
echo  [3] Dependencies Only          (Setup)
echo  [4] Classic Command Line       (Advanced)
echo  [5] Exit
echo.
set /p choice="Enter your choice (1-5): "

if "%choice%"=="1" goto gui_build
if "%choice%"=="2" goto quick_build  
if "%choice%"=="3" goto deps_build
if "%choice%"=="4" goto classic_build
if "%choice%"=="5" goto exit
goto invalid

:gui_build
echo.
echo Starting Interactive GUI Builder...
call build_win_interactive.bat
goto end

:quick_build
echo.
echo Starting Quick Hybrid Build...
call build_win_premake.bat --hybrid
goto end

:deps_build
echo.
echo Building Dependencies...
call build_win_premake.bat --deps
goto end

:classic_build
echo.
echo Starting Classic Build...
call build_win_premake.bat
goto end

:invalid
echo.
echo Invalid choice. Please try again.
pause
goto start

:exit
echo Goodbye!
exit /b 0

:end
echo.
echo Build process completed!
pause