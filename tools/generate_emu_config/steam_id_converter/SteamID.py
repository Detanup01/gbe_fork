"""
User SteamID
This Class helps you convert the different user steam id types from one to another
Important this class only works with steam ids of users.
Other Types of steam accounts (https://developer.valvesoftware.com/wiki/SteamID#Types_of_Steam_Accounts)
are not supported!

But this should work for the most usecases.

Base information about the different kinds of Steam IDs:
https://developer.valvesoftware.com/wiki/SteamID

This scripts support those kind of steam IDs:

|  ID type       |  example                         |
|----------------|----------------------------------|
|   SteamID      | STEAM_0:1:40370747               |
|   Steam ID3    | U:1:80741495 or [U:1:80741495]   |
|   Steam32 ID   | 80741495                         |
|   Steam64 ID   | 76561198041007223                |
"""

class InvalidSteamID(Exception):
    def __init__(self,*args,**kwargs):
        Exception.__init__(self, "The given steam id is not valid.")


class SteamID:
    x:int = None
    y:int = None
    z:int = None
 
    def __init__(self, any_steamid:str):
        """
        Insert any kind of steam id as a string in the init method
        :param any_steamid: One of the differnt kinds of steamids as string
        """
        self.convert(any_steamid)
    
    def get_steam64_id(self):
        return int(self.get_steam32_id() + 0x0110000100000000)

    def get_steam32_id(self):
        return int(2 * int(self.z) + int(self.y))

    def get_steam_id3(self):
        return f"U:{self.y}:{self.get_steam32_id()}"
    
    def get_steam_id(self):
        return f"STEAM_{self.x}:{self.y}:{self.z}"

    def convert(self, any_steamid:str):
        """
        This method will convert the class variables to the different kinds
        of steam ids, based on the paramerter.
        :param any_steamid: One of the differnt kinds of steamids as string
        """
        # preprocessing, lower the text and remove spaces
        any_steamid = any_steamid.lower()
        any_steamid = any_steamid.replace(" ", "")

        # check the kind of steam id from the input parameter
        try:
            # check if its a steam64 id
            steamid = int(any_steamid)
            if len(bin(steamid)) >= 32:
                steam32_id = steamid - 0x0110000100000000
                self.x = 0
                self.y = 0 if steam32_id % 2 == 0 else 1
                self.z = int( (steam32_id - self.y) / 2 )

            if len(bin(steamid)) < 32:
                steam32_id = steamid
                self.x = 0
                self.y = 0 if steam32_id % 2 == 0 else 1
                self.z = int( (steam32_id - self.y) / 2 )

        except ValueError:
            # there is a character inside the id
            # now it can only be two types of steam ids
            # either steamid3 or steamid
            if any_steamid.startswith("steam_"):
                # this is a steam id
                any_steamid.replace("steam_", "")
                steamid_parts = any_steamid.split(":")

                self.x = steamid_parts[0]
                self.y = steamid_parts[1]
                self.z = steamid_parts[2]

                self.steam32_id = 2 * self.z + self.y

            if any_steamid.startswith("["):
                # this is a steam id3
                any_steamid.replace("[", "")
                any_steamid.replace("]", "")
                any_steamid.replace("u:", "")
                steamid_parts = any_steamid.split(":")
                self.x = 0
                self.y = steamid_parts[0]
                self.z = int( (steamid_parts[1] - self.y) / 2 )
            
            if any_steamid.startswith("u"):
                any_steamid.replace("u:", "")
                steamid_parts = any_steamid.split(":")
                self.x = 0
                self.y = steamid_parts[0]
                self.z = int( (steamid_parts[1] - self.y) / 2 )

        except Exception:
            raise InvalidSteamID

        # check if the information is there
        if self.x is None or self.y is None or self.z is None:
            # if not throw an error
            raise InvalidSteamID
