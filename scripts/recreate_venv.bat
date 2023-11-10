@echo off

cd /d "%~dp0"

set "venv=.env"
set "reqs_file=requirements.txt"

if exist "%venv%" (
    rmdir /s /q "%venv%"
)

python -m venv "%venv%"
timeout /t 1 /nobreak
call "%venv%\Scripts\activate.bat"
pip install -r "%reqs_file%"
