@echo off

setlocal
pushd "%~dp0"

if /i "%~1"=="clean" (
  rmdir /s /q "build-win" >nul 2>&1
)

del /f /q *.exp >nul 2>&1
del /f /q *.lib >nul 2>&1
del /f /q *.a >nul 2>&1
del /f /q *.obj >nul 2>&1
del /f /q *.pdb >nul 2>&1
del /f /q *.ilk >nul 2>&1
del /f /q dll\net.pb.cc >nul 2>&1
del /f /q dll\net.pb.h >nul 2>&1
rmdir /s /q "build-win-temp" >nul 2>&1

endlocal
popd
