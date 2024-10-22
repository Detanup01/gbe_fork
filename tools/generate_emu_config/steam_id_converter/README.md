# SteamID-Converter-python
A python class that helps you convert the different user steam id types from one to another.

This script **doesn't** need an internet connection. It will **calculate** the steam id's.

Important: This class only works with steam ids of users.
Other Types of [steam accounts](https://developer.valvesoftware.com/wiki/SteamID#Types_of_Steam_Accounts)
are not supported!

But this should work for the most usecases.

Base information about the different kinds of Steam IDs:
https://developer.valvesoftware.com/wiki/SteamID

This script supports the following steam IDs:

|  ID type       |  example                         |
|----------------|----------------------------------|
|   SteamID      | STEAM_0:1:40370747               |
|   Steam ID3    | U:1:80741495 or [U:1:80741495]   |
|   Steam32 ID   | 80741495                         |
|   Steam64 ID   | 76561198041007223                |



## example code
```python
from SteamID import SteamID

steam_id = SteamID("76561198041007223")
print(steam_id.get_steam_id()) # returns STEAM_0:1:40370747  
```
