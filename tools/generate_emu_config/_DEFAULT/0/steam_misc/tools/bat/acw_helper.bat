@echo off
copy .\steam_misc\tools\au3\scripts\acw_helper.a3x .\
copy .\steam_misc\tools\au3\scripts\acw_helper.ini .\
ren .\acw_helper.a3x gse_acw_helper.a3x 
ren .\acw_helper.ini gse_acw_helper.ini
.\steam_misc\tools\au3\au3.exe /AutoIt3ExecuteScript .\gse_acw_helper.a3x 
del .\gse_acw_helper.a3x
del .\gse_acw_helper.ini