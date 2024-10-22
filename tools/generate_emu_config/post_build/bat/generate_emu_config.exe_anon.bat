@echo off
set /p arg="Generate Emu Config for Steam AppId: "
generate_emu_config.exe -cdx -rne -acw -clr -anon %arg%