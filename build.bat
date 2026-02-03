@echo off
where pwsh >nul 2>nul
if %ERRORLEVEL% EQU 0 (
    pwsh -ExecutionPolicy Bypass -File build.ps1
) else (
    echo PowerShell Core (pwsh) not found. Trying Windows PowerShell...
    powershell -ExecutionPolicy Bypass -File build.ps1
)
