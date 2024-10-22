import os
from steam_id_converter.SteamID import (
    SteamID
)

__codex_ini = r'''###
###        ÜÛÛÛÛÛ   Ü
###      °ÛÛÛÛ ßÛÛ Û² ßßßÛÛÛÛÛÛÛÜ     ßßßßßÛÛ²ÛÛÜ     Ü²ÛÛÛß
###     ±ÛÛÛß   ±ÛÛß        ßßÛÛÛÛ°           ßÛÛÛ  ±ÛÛÛß
###    ²ÛÛÛ      ß         ÛÛ²  ßÛÛÛ±           °ÛÛ²ÛÛÛ
###   ²ÛÛÛ        ÜÛÛÛÛÛÜ  ÛÛÛ   ßÛÛÛ ÜÛÛÛÛÛÛÜ  ±ÛÛÛÛÛ
###   ÛÛÛ°     °ÛÛÛÛß  ßÛ² ÛÛÛ    ÛÛÛ²ÛÛ²  °ÛÛ  ÛÛÛ²ÛÛÛ
###   ±ÛÛÛ     ÛÛÛÛ°    ÛÛ ÛÛÛ   °ÛÛÛÛÛÛÛÛÛÛÛß ÛÛÛ± ±ÛÛÛ
###    °ÛÛÛÜ ÜÛÛß²ÛÛÜ ÜÛÛ ²ÛÛ   ÛÛÛ±ÛÛÛ°     ÜÛÛÛ°   °ÛÛÛÜ
###      ßÛÛÛÛÛß   ßÛÛÛ²ß  ÛÛß °ÛÛÛ°  ßÛÛÛÛÛÛÛ²Ûß       ß²ÛÛÜ
###                        ÜÛÛÛÛÛÛ±
###          ßßßÛÛ²ÜÜÜÜÜÛ²ÛÛÛ²ßß    
###
###
### Game data is stored at %SystemDrive%\Users\Public\Documents\Steam\CODEX\{cdx_id}
###

[Settings]
###
### Game identifier (http://store.steampowered.com/app/{cdx_id})
###
AppId={cdx_id}
###
### Steam Account ID, set it to 0 to get a random Account ID
###
AccountId={cdx_accountid}
### 
### Name of the current player
###
UserName={cdx_username}
###
### Language that will be used in the game
###
Language={cdx_language}
###
### Enable lobby mode
###
LobbyEnabled=1
###
### Lobby port to listen on
###
#LobbyPort=31183
###
### Enable/Disable Steam overlay
###
Overlays=1
###
### Set Steam connection to offline mode
###
Offline=0
###
SelfProtect=0
###

[Interfaces]
###
### Steam Client API interface versions
###
SteamAppDisableUpdate=
SteamAppList=
SteamApps=
SteamAppTicket=
SteamClient=
SteamController=
SteamFriends=
SteamGameCoordinator=
SteamGameServerStats=
SteamGameServer=
SteamGameStats=
SteamHTMLSurface=
SteamHTTP=
SteamInput=
SteamInventory=
SteamMasterServerUpdater=
SteamMatchGameSearch=
SteamMatchMakingServers=
SteamMatchMaking=
SteamMusic=
SteamMusicRemote=
SteamNetworkingMessages=
SteamNetworkingSocketsSerialized=
SteamNetworkingSockets=
SteamNetworkingUtils=
SteamNetworking=
SteamParentalSettings=
SteamParties=
SteamRemotePlay=
SteamRemoteStorage=
SteamScreenshots=
SteamTimeline=
SteamTV=
SteamUGC=
SteamUnifiedMessages=
SteamUserStats=
SteamUser=
SteamUtils=
SteamVideo=
###

[DLC]
###
### Automatically unlock all DLCs
###
DLCUnlockall=0
###
### Identifiers for DLCs
###
#ID=Name
{cdx_dlc_list}
###

[AchievementIcons]
###
### Bitmap Icons for Achievements
###
#halloween_8 Achieved=steam_settings\\img\\halloween_8.jpg
#halloween_8 Unachieved=steam_settings\\img\\unachieved\\halloween_8.jpg
{cdx_ach_list}
###

[Crack]
c7a90c3ffa71981e=b7d5bc716512b5d6

'''

def generate_cdx_ini(
    base_out_dir : str,
    appid: int,
    accountid: int,
    username: str,
    language: str,
    dlc: list[tuple[int, str]],
    achs: list[dict]) -> None:

    if not os.path.exists(os.path.join(base_out_dir, "steam_misc\\extra_crk\\CODEX")):
        os.makedirs(os.path.join(base_out_dir, "steam_misc\\extra_crk\\CODEX"))

    codex_ini_path = os.path.join(base_out_dir, "steam_misc\\extra_crk\\CODEX\\steam_emu.ini")
    print(f"[ ] Generating CODEX config --- writing <OUT_DIR>\\steam_misc\\extra_crk\\CODEX\\steam_emu.ini")
    print(f"[ ] __ if to be used, make sure it has the correct interface versions")

    dlc_list = [f"{d[0]}={d[1]}" for d in dlc]
    achs_list = []
    for ach in achs:
        icon = ach.get("icon", None)
        if icon:
            icon = f"steam_settings\\img\\{icon}"
        else:
            icon = 'steam_settings\\img\\steam_default_icon_unlocked.jpg'

        icon_gray = ach.get("icon_gray", None)
        if icon_gray:
            icon_gray = f"steam_settings\\img\\{icon_gray}"
        else:
            icon_gray = 'steam_settings\\img\\steam_default_icon_locked.jpg'

        icongray = ach.get("icongray", None)
        if icongray:
            icon_gray = f"steam_settings\\img\\{icongray}"

        achs_list.append(f'{ach["name"]} Achieved={icon}') # unlocked
        achs_list.append(f'{ach["name"]} Unachieved={icon_gray}') # locked

    steam_id = SteamID(accountid)

    formatted_ini = __codex_ini.format(
        cdx_id = appid,
        cdx_username = username,
        cdx_accountid = steam_id.get_steam32_id(),
        cdx_language = language,
        cdx_dlc_list = "\n".join(dlc_list),
        cdx_ach_list = "\n".join(achs_list)
    )
    
    with open(codex_ini_path, "wt", encoding='utf-8') as f:
        f.writelines(formatted_ini)
