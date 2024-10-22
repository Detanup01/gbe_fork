@echo off
copy .\steam_misc\tools\au3\scripts\debug_switch.a3x .\
copy .\steam_misc\tools\au3\scripts\debug_switch.ini .\
ren .\debug_switch.a3x gse_debug_switch.a3x 
ren .\debug_switch.ini gse_debug_switch.ini
.\steam_misc\tools\au3\au3.exe /AutoIt3ExecuteScript ".\gse_debug_switch.a3x" 
del .\gse_debug_switch.a3x
del .\gse_debug_switch.ini