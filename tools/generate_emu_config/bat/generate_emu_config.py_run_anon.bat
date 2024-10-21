@echo off
set /p arg="Generate Emu Config for Steam AppId: "
python -W ignore::DeprecationWarning generate_emu_config.py -img -scr -vids_low -vids_max -scx -cdx -rne -acw -clr -anon %arg%