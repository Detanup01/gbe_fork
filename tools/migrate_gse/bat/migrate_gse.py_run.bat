@echo off	
set /p arg="Migrate GSE for folder: "	
python -W ignore::DeprecationWarning main.py %arg%
pause