import os

__cdx_ini = '''
###        мллллл   м
###      Алллл плл лВ ппплллллллм     пппппллВллм     мВлллп
###     Блллп   Бллп        ппллллА           пллл  Блллп
###    Вллл      п         ллВ  плллБ           АллВллл
###   Вллл        млллллм  ллл   пллл мллллллм  Бллллл
###   лллА     Аллллп  плВ ллл    лллВллВ  Алл  лллВллл
###   Бллл     ллллА    лл ллл   Алллллллллллп лллБ Бллл
###    Алллм мллпВллм млл Влл   лллБлллА     млллА   Алллм
###      плллллп   плллВп  ллп АлллА  плллллллВлп       пВллм
###                        мллллллБ
###          пппллВмммммлВлллВпп    
###
###
### Game data is stored at %SystemDrive%\\Users\\Public\\Documents\\Steam\\CODEX\\{cdx_id}
###

[Settings]
###
### Game identifier (http://store.steampowered.com/app/{cdx_id})
###
AppId={cdx_id}
###
### Steam Account ID, set it to 0 to get a random Account ID
###
#AccountId=0
### 
### Name of the current player
###
UserName=Player2
###
### Language that will be used in the game
###
Language=english
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

[Interfaces]
###
### Steam Client API interface versions
###
SteamAppList=STEAMAPPLIST_INTERFACE_VERSION001
SteamApps=STEAMAPPS_INTERFACE_VERSION008
SteamClient=SteamClient017
SteamController=SteamController008
SteamFriends=SteamFriends017
SteamGameServer=SteamGameServer013
SteamGameServerStats=SteamGameServerStats001
SteamHTMLSurface=STEAMHTMLSURFACE_INTERFACE_VERSION_005
SteamHTTP=STEAMHTTP_INTERFACE_VERSION003
SteamInput=SteamInput002
SteamInventory=STEAMINVENTORY_INTERFACE_V003
SteamMatchGameSearch=SteamMatchGameSearch001
SteamMatchMaking=SteamMatchMaking009
SteamMatchMakingServers=SteamMatchMakingServers002
SteamMusic=STEAMMUSIC_INTERFACE_VERSION001
SteamMusicRemote=STEAMMUSICREMOTE_INTERFACE_VERSION001
SteamNetworking=SteamNetworking006
SteamNetworkingSockets=SteamNetworkingSockets008
SteamNetworkingUtils=SteamNetworkingUtils003
SteamParentalSettings=STEAMPARENTALSETTINGS_INTERFACE_VERSION001
SteamParties=SteamParties002
SteamRemotePlay=STEAMREMOTEPLAY_INTERFACE_VERSION001
SteamRemoteStorage=STEAMREMOTESTORAGE_INTERFACE_VERSION014
SteamScreenshots=STEAMSCREENSHOTS_INTERFACE_VERSION003
SteamTV=STEAMTV_INTERFACE_V001
SteamUGC=STEAMUGC_INTERFACE_VERSION015
SteamUser=SteamUser021
SteamUserStats=STEAMUSERSTATS_INTERFACE_VERSION012
SteamUtils=SteamUtils010
SteamVideo=STEAMVIDEO_INTERFACE_V002
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
00ec7837693245e3=b7d5bc716512b5d6

'''


def generate_cdx_ini(
    base_out_dir : str,
    appid: int,
    dlc: list[tuple[int, str]],
    achs: list[dict]) -> None:

    cdx_ini_path = os.path.join(base_out_dir, "steam_emu.ini")
    print(f"generating steam_emu.ini for CODEX emulator in: {cdx_ini_path}")

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

    formatted_ini = __cdx_ini.format(
        cdx_id = appid,
        cdx_dlc_list = "\n".join(dlc_list),
        cdx_ach_list = "\n".join(achs_list)
    )
    
    with open(cdx_ini_path, "wt", encoding='utf-8') as f:
        f.writelines(formatted_ini)

