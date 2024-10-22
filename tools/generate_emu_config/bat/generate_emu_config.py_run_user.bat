@echo off
set /p arg="Generate Emu Config for Steam AppId: "
python -W ignore::DeprecationWarning generate_emu_config.py -cdx -rne -acw -clr %arg%