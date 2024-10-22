@echo off
copy .\steam_misc\tools\au3\scripts\lobby_connect.a3x .\
copy .\steam_misc\tools\au3\scripts\lobby_connect.ini .\
ren .\lobby_connect.a3x gse_lobby_connect.a3x
ren .\lobby_connect.ini gse_lobby_connect.ini
.\steam_misc\tools\au3\au3.exe /AutoIt3ExecuteScript .\gse_lobby_connect.a3x 
del .\gse_lobby_connect.a3x
del .\gse_lobby_connect.ini