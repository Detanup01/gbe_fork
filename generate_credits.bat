@echo off
setlocal EnableDelayedExpansion
cd /d "%~dp0"

set "ROOT=%cd%"
set "CREDITS_FILE=%ROOT%\CREDITS.md"

if exist "%CREDITS_FILE%" (
  del /f /q "%CREDITS_FILE%"
)

set "GLOB=third-party libs tools\steamclient_loader sdk"
set "FILTER=SOURCE.txt"

echo:# Many thanks for these sources>> "%CREDITS_FILE%"

for %%A in (%GLOB%) do (
  powershell -Command "Get-ChildItem -LiteralPath \"%%~A\" -Filter \"%FILTER%\" -File -Recurse | foreach { $parent = Split-Path -Path $_.FullName -Parent; $relative = (Resolve-Path -Path $parent -Relative).replace(\".\\\",\"\"); $relative_tag = $relative.replace(\"\\\", \"\"); Write-Output \"- ^[$^($relative^)^]^(#$^($relative_tag^)^)\"; }">> "%CREDITS_FILE%"
)

echo.>> "%CREDITS_FILE%"

for %%B in (%GLOB%) do (
  powershell -Command "Get-ChildItem -LiteralPath \"%%~B\" -Filter \"%FILTER%\" -File -Recurse | foreach { $parent = Split-Path -Path $_.FullName -Parent; $relative = (Resolve-Path -Path $parent -Relative).replace(\".\\\",\"\"); Write-Output \"### $^($relative^)\"; Write-Output \"\"; Get-Content -LiteralPath $_.FullName -Raw -Encoding utf8; }">> "%CREDITS_FILE%"
)

endlocal
