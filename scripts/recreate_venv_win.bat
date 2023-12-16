@echo off

cd /d "%~dp0"

set "venv=.env-win"
set "reqs_file=requirements.txt"

if exist "%venv%" (
    rmdir /s /q "%venv%"
)

python -m venv "%venv%" || exit /b 1
timeout /t 1 /nobreak
call "%venv%\Scripts\activate.bat"
pip install -r "%reqs_file%"
set /a exit_code=errorlevel

call "%venv%\Scripts\deactivate.bat"
exit /b %exit_code%
