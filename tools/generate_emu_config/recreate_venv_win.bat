@echo off
cd /d "%~dp0"

set "ROOT=%cd%"
set "VENV=%ROOT%\.env-win"
set "REQS_FILE=%ROOT%\requirements.txt"

set /a "LAST_ERR_CODE=0"

if exist "%VENV%" (
  rmdir /s /q "%VENV%"
)

python -m venv "%VENV%" || (
  set /a "LAST_ERR_CODE=1"
  goto :end_script
)

timeout /t 1 /nobreak

call "%VENV%\Scripts\activate.bat"
pip install -r "%REQS_FILE%"
set /a "LAST_ERR_CODE=%ERRORLEVEL%"
call "%VENV%\Scripts\deactivate.bat"

goto :end_script

:end_script
  exit /b %LAST_ERR_CODE%
