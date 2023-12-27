@echo off
cd /d "%~dp0"
mkdir release\tools
del /Q release\tools\*
call build_env_x86.bat
cl generate_interfaces_file.cpp /EHsc /MP12 /Ox /link /debug:none /OUT:release\tools\generate_interfaces_file.exe
del /Q /S release\tools\*.lib
del /Q /S release\tools\*.exp
copy Readme_generate_interfaces.txt release\tools\Readme_generate_interfaces.txt
