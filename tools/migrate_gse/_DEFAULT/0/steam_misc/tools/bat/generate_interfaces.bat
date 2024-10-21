@echo off
copy .\steam_misc\tools\au3\scripts\generate_interfaces.a3x .\
copy .\steam_misc\tools\au3\scripts\generate_interfaces.ini .\
ren .\generate_interfaces.a3x gse_generate_interfaces.a3x
ren .\generate_interfaces.ini gse_generate_interfaces.ini 
.\steam_misc\tools\au3\au3.exe /AutoIt3ExecuteScript .\gse_generate_interfaces.a3x 
del .\gse_generate_interfaces.a3x
del .\gse_generate_interfaces.ini