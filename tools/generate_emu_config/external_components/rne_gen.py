import os
from steam_id_converter.SteamID import (
    SteamID
)

__rune_ini = r'''###
###                                                                \    /  
###                       _  _                 _            _      \\__//  
###      ____ ._/______:_//\//_/____       _  //___  ./_ __//_____:_\\//   
###     :\  //_/    _  . /_/  /    /_/__:_//_/    /\ /\__/_    _  . /\\\   
###      \\///      ____/___./    / /     / /    /  /  \X_/   //___/ /_\_  
###     . \///   _______   _/_   /_/    _/_/     \\/   //        /_\_\  /. 
###       z_/   _/\  _/   // /         //      /  \   ///     __//   :\//  
###     | / _   / /\//   /__//      __//_   _ /\     /X/     /__/   |/\/2  
###   --+-_=\__/ / /    / \_____:__/ //\____// /\   /\/__:_______=_-+--\4  
###     |-\__\- / /________\____.__\/- -\--/_\/_______\--.\________\|___\  
###      = dS!\/- -\_______\ =-RUNE- -== \/ ==-\______\-= ======== --\__\ 
###
###
### Game data is stored at %SystemDrive%\Users\Public\Documents\Steam\RUNE\{rne_id}
###

[Settings]
###
### Game identifier (http://store.steampowered.com/app/{rne_id})
###
AppId={rne_id}
###
### Steam Account ID, set it to 0 to get a random Account ID
###
AccountId={rne_accountid}
### 
### Name of the current player
###
UserName={rne_username}
###
### Language that will be used in the game
###
Language={rne_language}
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
BlockConnection=0
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
{rne_dlc_list}
###

[Crack]
7f508eddb6d6c0b1=3681a9beddbae875

'''

def generate_rne_ini(
    base_out_dir : str,
    appid: int,
    accountid: int,
    username: str,
    language: str,
    dlc: list[tuple[int, str]],
    achs: list[dict]) -> None:

    if not os.path.exists(os.path.join(base_out_dir, "steam_misc\\extra_crk\\RUNE")):
        os.makedirs(os.path.join(base_out_dir, "steam_misc\\extra_crk\\RUNE"))

    rune_ini_path = os.path.join(base_out_dir, "steam_misc\\extra_crk\\RUNE\\steam_emu.ini")
    print(f"[ ] Generating RUNE config --- writing <OUT_DIR>\\steam_misc\\extra_crk\\RUNE\\steam_emu.ini")
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

    formatted_ini = __rune_ini.format(
        rne_id = appid,
        rne_username = username,
        rne_accountid = steam_id.get_steam32_id(),
        rne_language = language,
        rne_dlc_list = "\n".join(dlc_list),
        rne_ach_list = "\n".join(achs_list)
    )
    
    with open(rune_ini_path, "wt", encoding='utf-8') as f:
        f.writelines(formatted_ini)
