@echo off
set /p arg="Generate Emu Config for Steam AppId: "
generate_emu_config.exe -img -scr -vids_low -vids_max -scx -cdx -rne -acw -clr %arg%