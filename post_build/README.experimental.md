## What is this ?
This experimental build of the emulator:
* Blocks all outgoing connections from the game to non-LAN IPs

* Lets you use CPY style cracks that use the `steam_api` dll to patch the exe in memory when the `SteamAPI_Init()` method is called

* Can load extra dlls in memory via `LoadLibraryW()`

Mr_Goldberg's note:
> In this folder is an experimental build of the emulator with code that hooks a few windows socket functions. It should block all connections from the game to non-LAN IPs. This means the game should work without any problems for LAN play (even with VPN LAN as long as you use standard LAN IPs 10.x.x.x, 192.168.x.x, etc...)  

> It likely doesn't work for some games but it should work for most of them  

> Since this blocks all non LAN connections doing things like hosting a cracked server for people on the internet will not work or connecting to a cracked server that's hosted on an internet IP will not work  

## Why ?
Mr_Goldberg's note:
> Lots of games seem to connect to analytics services and other crap  
> Blocking the game from communicating with online IPs without affecting the LAN functionality of this emu is a pain if you try to use a firewall

## Which IPs are allowed ?
With `disable_lan_only=0` inside `.\steam_settings\configs.main.ini`, only **LAN-only IPs** are allowed:
* 10.0.0.0 - 10.255.255.255
* 127.0.0.0 - 127.255.255.255
* 169.254.0.0 - 169.254.255.255
* 172.16.0.0 - 172.31.255.255
* 192.168.0.0 - 192.168.255.255
* 224.0.0.0 - 255.255.255.255

To allow all IPs, including **non-LAN IPs**, modify your `.\steam_settings\configs.main.ini` and set `disable_lan_only=1`

## How to use a CPY style crack
1. Rename `steam_api.dll` crack to `cracksteam_api.dll`, or `steam_api64.dll` to `cracksteam_api64.dll`
2. Replace the `steamclient(64).dll` crack with the one in this folder

## How to load extra dlls in memory
Put the dll file inside the folder `.\steam_settings\load_dlls`, and it will be loaded automatically using the `LoadLibraryW()` function

