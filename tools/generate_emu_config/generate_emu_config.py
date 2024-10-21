import pathlib
import time
from stats_schema_achievement_gen import achievements_gen
from external_components import (
    ach_watcher_gen, cdx_gen, rne_gen, app_images, app_details, safe_name, scx_gen, pcgw_page, top_own
)
from controller_config_generator import parse_controller_vdf
from steam.client import SteamClient
from steam.webauth import WebAuth
from steam.enums.common import EResult
from steam.enums.emsg import EMsg
from steam.core.msg import MsgProto
import os
import re
import sys
import platform
import json
import requests
import threading
import queue
import shutil
import traceback
from configobj import ConfigObj

# Steam ids with public profiles that own a lot of games --- https://steamladder.com/ladder/games/
# How to generate/update top_owners_ids.txt upon running generate_emu_config:
#  - copy and paste the above address in your web browser
#  - right click and save web page, html only with the name top_owners_ids.html
#  - copy and paste top_owners_ids.html next to generate_emu_config exe or py
TOP_OWNER_IDS = list(dict.fromkeys([
    76561198028121353,
    76561197979911851,
    76561198017975643,
    76561197993544755,
    76561198355953202,
    76561198001237877,
    76561198237402290,
    76561198152618007,
    76561198355625888,
    76561198213148949,
    76561197969050296,
    76561198217186687,
    76561198037867621,
    76561198094227663,
    76561198019712127,
    76561197963550511,
    76561198134044398,
    76561198001678750,
    76561197973009892,
    76561198044596404,
    76561197976597747,
    76561197969810632,
    76561198095049646,
    76561198085065107,
    76561198864213876,
    76561197962473290,
    76561198388522904,
    76561198033715344,
    76561197995070100,
    76561198313790296,
    76561198063574735,
    76561197996432822,
    76561197976968076,
    76561198281128349,
    76561198154462478,
    76561198027233260,
    76561198842864763,
    76561198010615256,
    76561198035900006,
    76561198122859224,
    76561198235911884,
    76561198027214426,
    76561197970825215,
    76561197968410781,
    76561198104323854,
    76561198001221571,
    76561198256917957,
    76561198008181611,
    76561198407953371,
    76561198062901118,
    #76561197979667190,
    #76561197974742349,
    #76561198077213101,
    #76561198121398682,
    #76561198019009765,
    #76561198119667710,
    #76561197990233857,
    #76561199130977924,
    #76561198096081579,
    #76561198139084236,
    #76561197971011821,
    #76561198063728345,
    #76561198082995144,
    #76561197963534359,
    #76561198118726910,
    #76561198097945516,
    #76561198124872187,
    #76561198077248235,
    #76561198326510209,
    #76561198109083829,
    #76561198808371265,
    #76561198048373585,
    #76561198005337430,
    #76561198045455280,
    #76561197981111953,
    #76561197992133229,
    #76561198152760885,
    #76561198037809069,
    #76561198382166453,
    #76561198093753361,
    #76561198396723427,
    #76561199168919006,
    #76561198006391846,
    #76561198040421250,
    #76561197994616562,
    #76561198017902347,
    #76561198044387084,
    #76561198172367910,
    #76561199353305847,
    #76561198121336040,
    #76561197972951657,
    #76561198251835488,
    #76561198102767019,
    #76561198021180815,
    #76561197976796589,
    #76561197992548975,
    #76561198367471798,
    #76561197965978376,
    #76561197993312863,
    #76561198128158703,
    #76561198015685843,
    #76561198047438206,
    #76561197971026489,
    #76561198252374474,
    #76561198061393233,
    #76561199173688191,
    #76561198008797636,
    #76561197995008105,
    #76561197984235967,
    #76561198417144062,
    #76561197978640923,
    #76561198219343843,
    #76561197982718230,
    #76561198031837797,
    #76561198039492467,
    #76561198020125851,
    #76561198192399786,
    #76561198028011423,
    #76561198318111105,
    #76561198155124847,
    #76561198168877244,
    #76561198105279930,
    #76561197988664525,
    #76561198996604130,
    #76561197969148931,
    #76561198035552258,
    #76561198015992850,
    #76561198050474710,
    #76561198029503957,
    #76561198026221141,
    #76561198025653291,
    #76561198034213886,
    #76561198096632451,
    #76561197972378106,
    #76561197997477460,
    #76561198054210948,
    #76561198111433283,
    #76561198004332929,
    #76561198045540632,
    #76561198043532513,
    #76561199080934614,
    #76561197970246998,
    #76561197986240493,
    #76561198029532782,
    #76561198018254158,
    #76561197973230221,
    #76561198020746864,
    #76561198158932704,
    #76561198086250077,
    #76561198269242105,
    #76561198294806446,
    #76561198031164839,
    #76561198019555404,
    #76561198048151962,
    #76561198003041763,
    #76561198025391492,
    #76561197962630138,
    #76561198072936438,
    #76561198120120943,
    #76561197984010356,
    #76561198042965266,
    #76561198046642155,
    #76561198015856631,
    #76561198124865933,
    #76561198042781427,
    #76561198443388781,
    #76561198426000196,
    #76561198051725954,
    #76561197992105918,
    #76561198172925593,
    #76561198071709714,
    #76561197981228012,
    #76561197981027062,
    #76561198122276418,
    #76561198019841907,
    #76561197985091630,
    #76561199492215670,
    #76561198106206019,
    #76561198090111762,
    #76561198104561325,
    #76561197991699268,
    #76561198072361453,
    #76561198027066612,
    #76561198032614383,
    #76561198844130640,
    #76561198106145311,
    #76561198079227501,
    #76561198093579202,
    #76561198315929726,
    #76561198171791210,
    #76561198264362271,
    #76561198846208086,
    #76561197991613008,
    #76561198026306582,
    #76561197973701057,
    #76561198028428529,
    #76561198427572372,
    #76561197983517848,
    #76561198085238363,
    #76561198070220549,
    #76561198101049562,
    #76561197969365800,
    #76561198413266831,
    #76561198015514779,
    #76561198811114019,
    #76561198165450871,
    #76561197994575642,
    #76561198034906703,
    #76561198119915053,
    #76561198079896896,
    #76561198008549198,
    #76561197988052802,
    #76561198004532679,
    #76561198002535276,
    #76561197970545939,
    #76561197977920776,
    #76561198007200913,
    #76561197984605215,
    #76561198831075066,
    #76561197970970678,
    #76561197982273259,
    #76561197970307937,
    #76561198413088851,
    #76561197970360549,
    #76561198051740093,
    #76561197966617426,
    #76561198356842617,
    #76561198025111129,
    #76561197996825541,
    #76561197967716198,
    #76561197975329196,
    #76561197998058239,
    #76561198027668357,
    #76561197962850521,
    #76561198258304011,
    #76561198098314980,
    #76561198127957838,
    #76561198060520130,
    #76561198035612474,
    #76561198318547224,
    #76561198020810038,
    #76561198080773680,
    #76561198033967307,
    #76561198034503074,
    #76561198150467988,
    #76561197994153029,
    #76561198026278913,
    #76561198217979953,
    #76561197988445370,
    #76561198083977059
]))

def get_exe_dir(relative = False):
    # https://pyinstaller.org/en/stable/runtime-information.html
    if relative:
        return os.path.curdir

    if getattr(sys, 'frozen', False) and hasattr(sys, '_MEIPASS'):
        return os.path.dirname(sys.executable)
    else:
        return os.path.dirname(os.path.abspath(__file__))

def get_stats_schema(client, game_id, owner_id):
    message = MsgProto(EMsg.ClientGetUserStats)
    message.body.game_id = game_id
    message.body.schema_local_version = -1
    message.body.crc_stats = 0
    message.body.steam_id_for_user = owner_id

    client.send(message)
    return client.wait_msg(EMsg.ClientGetUserStatsResponse, timeout=5)

def download_achievement_images(game_id : int, image_names : set[str], output_folder : str):
    print(f"[ ] Found {len(image_names)} achievements images --- downloading to <OUT_DIR>\\steam_settings\\img folder")

    q : queue.Queue[str] = queue.Queue()

    def downloader_thread():
        while True:
            name = q.get()
            if name is None:
                q.task_done()
                return
            
            succeeded = False
            for u in ["https://cdn.akamai.steamstatic.com/steamcommunity/public/images/apps/", "https://cdn.cloudflare.steamstatic.com/steamcommunity/public/images/apps/"]:
                url = "{}{}/{}".format(u, game_id, name)
                try:
                    response = requests.get(url, allow_redirects=True)
                    response.raise_for_status()
                    image_data = response.content
                    with open(os.path.join(output_folder, name), "wb") as f:
                        f.write(image_data)
                    succeeded = True
                    break
                except Exception as e:
                    print("____ HTTPError downloading", url, file=sys.stderr)
                    traceback.print_exception(e, file=sys.stderr)
            if not succeeded:
                print("____ Error, could not download", name)

            q.task_done()

    num_threads = 50

    for i in range(num_threads):
        threading.Thread(target=downloader_thread, daemon=True).start()

    for name in image_names:
        q.put(name)
    q.join()

    for i in range(num_threads):
        q.put(None)
    q.join()

def generate_achievement_stats(client, game_id : int, output_directory, backup_directory) -> list[dict]:
    stats_schema_found = None
    #print(f"[ ] Finding achievements stats...")
    for id in TOP_OWNER_IDS:
        #print(f"[ ] Finding achievements stats using account ID {id}...")
        out = get_stats_schema(client, game_id, id)
        if out is not None and len(out.body.schema) > 0:
            stats_schema_found = out
            #print(f"[ ] Found achievement stats using account ID {id}")
            break

    if stats_schema_found is None: # no achievement found
        print(f"[?] No achievements found - skip creating <OUT_DIR>\\steam_settings\\achievements.json")
        return []

    achievement_images_dir = os.path.join(output_directory, "img")
    images_to_download : set[str] = set()
    
    with open(os.path.join(backup_directory, f'UserGameStatsSchema_{game_id}.bin'), 'wb') as f:
        f.write(stats_schema_found.body.schema)
        
    (
        achievements, stats,
        copy_default_unlocked_img, copy_default_locked_img
    ) = achievements_gen.generate_stats_achievements(stats_schema_found.body.schema, output_directory)

    if len(achievements) != 1:
        print(f"[ ] Found {len(achievements)} achievements --- writing to <OUT_DIR>\\steam_settings\\achievements.json")
    else:
        print(f"[ ] Found {len(achievements)} achievement --- writing to <OUT_DIR>\\steam_settings\\achievements.json")

    #print(f"[ ] Writing 'UserGameStatsSchema_{game_id}.bin'")
    
    for ach in achievements:
        icon = f"{ach.get('icon', '')}".strip()
        if icon:
            images_to_download.add(icon)
        icon_gray = f"{ach.get('icon_gray', '')}".strip()
        if icon_gray:
            images_to_download.add(icon_gray)

    if images_to_download:
        if not os.path.exists(achievement_images_dir):
            os.makedirs(achievement_images_dir)
        if copy_default_unlocked_img:
            shutil.copy(os.path.join(get_exe_dir(), "steam_default_icon_unlocked.jpg"), achievement_images_dir)
        if copy_default_locked_img:
            shutil.copy(os.path.join(get_exe_dir(), "steam_default_icon_locked.jpg"), achievement_images_dir)
        download_achievement_images(game_id, images_to_download, achievement_images_dir)

    return achievements

def get_ugc_info(client, published_file_id):
    return client.send_um_and_wait('PublishedFile.GetDetails#1', {
            'publishedfileids': [published_file_id],
            'includetags': False,
            'includeadditionalpreviews': False,
            'includechildren': False,
            'includekvtags': False,
            'includevotes': False,
            'short_description': True,
            'includeforsaledata': False,
            'includemetadata': False,
            'language': 0
        })

def download_published_file(client, published_file_id, backup_directory):
    ugc_info = get_ugc_info(client, published_file_id)

    if (ugc_info is None):
        print("____ Failed getting published file", published_file_id)
        return None

    file_details = ugc_info.body.publishedfiledetails[0]
    if (file_details.result != EResult.OK):
        print("____ Failed getting published file", published_file_id, file_details.result)
        return None

    if not os.path.exists(backup_directory):
        os.makedirs(backup_directory)

    with open(os.path.join(backup_directory, "info.txt"), "w") as f:
        f.write(str(ugc_info.body))

    if len(file_details.file_url) > 0:
        try:
            response = requests.get(file_details.file_url, allow_redirects=True)
            response.raise_for_status()
            data = response.content
            with open(os.path.join(backup_directory, file_details.filename.replace("/", "_").replace("\\", "_")), "wb") as f:
                f.write(data)
            return data
        except Exception as e:
            print(f"____ Error downloading from '{file_details.file_url}'", file=sys.stderr)
            traceback.print_exception(e, file=sys.stderr)
            return None
    else:
        print("____ Could not download file", published_file_id, "no url")
        print("____ You can ignore this if the game doesn't need a controller config")
        return None

def get_inventory_info(client, game_id):
    return client.send_um_and_wait('Inventory.GetItemDefMeta#1', {
            'appid': game_id
        })

def generate_inventory(client, game_id):
    inventory = get_inventory_info(client, game_id)
    if inventory.header.eresult != EResult.OK:
        return None

    url = f"https://api.steampowered.com/IGameInventory/GetItemDefArchive/v0001?appid={game_id}&digest={inventory.body.digest}"
    try:
        response = requests.get(url, allow_redirects=True)
        response.raise_for_status()
        return response.content
    except Exception as e:
        print(f"Error downloading from '{url}'", file=sys.stderr)
        traceback.print_exception(e, file=sys.stderr)
    return None

def parse_branches(branches: dict) -> list[dict]:
    ret = []
    for branch_name in  branches:
        branch_data: dict = branches[branch_name]
        branch_info = {
            'name': branch_name,
            'description': f'{branch_data.get("description", "")}',
            'protected': False,
            'build_id': 0, # dummy
            'time_updated': int(time.time()), # dummy
        }
        # password protected
        if 'pwdrequired' in branch_data:
            try:
                protected = f'{branch_data["pwdrequired"]}'.lower()
                branch_info["protected"] = protected == "true" or protected == "1"
            except Exception as e:
                pass
        # build id
        try:
            buildid = int( f'{branch_data.get("buildid", 0)}' )
            branch_info["build_id"] = buildid
        except Exception as e:
            pass
        # time updated
        if 'timeupdated' in branch_data:
            try:
                timeupdated = int( f'{branch_data["timeupdated"]}' )
                branch_info["time_updated"] = timeupdated
            except Exception as e:
                pass

        ret.append(branch_info)

    return ret

# DLC, Depots, Branches
def get_depots_infos(raw_infos, appid):
    #print(f"[ ] Finding DLC infos...")
    try:
        dlc_list = set()
        depot_app_list = set()
        all_depots = set()
        all_branches = []
        try:
            dlc_list = set(map(lambda a: int(f"{a}".strip()), raw_infos["extended"]["listofdlc"].split(",")))
        except Exception:
            #print(f"[?] Could not get DLCs info. Is there any depot for {appid}?")
            pass
        
        if "depots" in raw_infos:
            depots : dict[str, object] = raw_infos["depots"]
            for dep in depots:
                depot_info = depots[dep]
                if "dlcappid" in depot_info:
                    dlc_list.add(int(depot_info["dlcappid"]))
                if "depotfromapp" in depot_info:
                    depot_app_list.add(int(depot_info["depotfromapp"]))
                if dep.isnumeric():
                    all_depots.add(int(dep))
                elif f'{dep}'.lower() == 'branches':
                    all_branches.extend(parse_branches(depot_info))
        #else:
            #print(f"[?] Could not get depots info. Is there any DLC for {appid}?")
        
        return (dlc_list, depot_app_list, all_depots, all_branches)
    except Exception:
        return (set(), set(), set())

# https://stackoverflow.com/a/48336994    
def GetListOfSubstrings(stringSubject,string1,string2):
    MyList = []
    intstart=0
    strlength=len(stringSubject)
    continueloop = 1

    while(intstart < strlength and continueloop == 1):
        intindex1=stringSubject.find(string1,intstart)
        if(intindex1 != -1): #The substring was found, lets proceed
            intindex1 = intindex1+len(string1)
            intindex2 = stringSubject.find(string2,intindex1)
            if(intindex2 != -1):
                subsequence=stringSubject[intindex1:intindex2]
                MyList.append(subsequence)
                intstart=intindex2+len(string2)
            else:
                continueloop=0
        else:
            continueloop=0
    return MyList

# https://stackoverflow.com/a/13641746 # NOTE using this fix a strange issue where some DLC names had starting and trailing double quotes ( " )
def ReplaceStringInFile(f_file, search_string, old_string, new_string):
    with open(f_file, 'r') as file:
        lines = file.readlines()
        #matching_lines = [line.strip() for line in lines if ' = "' in line]
        #return matching_lines
        for line in lines:
            if search_string in line:
                # Read contents from file as a single string
                f_handle = open(f_file, 'r')
                f_string = f_handle.read()
                f_handle.close()

                # Use RE package to allow for replacement, also allowing for multi-line REGEX
                f_string = (re.sub(old_string, new_string, f_string))

                # Write contents to file - using 'w' truncates the file
                f_handle = open(f_file, 'w')
                f_handle.write(f_string)
                f_handle.close()

def help():
    exe_name = os.path.basename(sys.argv[0])
    print(f"\nUsage: {exe_name} [Switches] appid appid appid ... ")
    print(f" Example: {exe_name} 421050 420 480")
    print(f" Example: {exe_name} -img -scr -vids_max -scx -cdx -rne -acw -clr 421050 480")
    print("\nSwitches:")
    print(" -img:      download art images for each app: Steam generated background, icon, logo, etc...")
    print(" -scr:      download screenshots for each app if they're available")
    print(" -vids_low: download low quality videos for each app if they're available")
    print(" -vids_max: download max quality videos for each app if they're available")
    print(" -scx:      download market images for each app: Steam trading cards, badges, backgrounds, etc...")
    print(" -cdx:      generate .ini file for CODEX Steam emu for each app")
    print(" -rne:      generate .ini file for RUNE Steam emu for each app")
    print(" -acw:      generate schemas of all possible languages for Achievement Watcher")
    print(" -skip_ach: skip downloading & generating achievements and their images")
    print(" -skip_con: skip downloading & generating controller configuration files (action sets txt files)")
    print(" -skip_inv: skip downloading & generating inventory data ('items.json' & 'default_items.json')")
    print(" -anon:     login as an anonymous account, these have very limited access and cannot get all app details")
    print(" -name:     save the complete game config in a folder with the same name as the app (unsafe characters are discarded)")
    print(" -rel_out:  generate complete game config in _OUTPUT/appid folder, relative to the bat, sh or app calling generate_emu_config app")
    print(" -rel_raw:  generate complete game config in the same folder that contains the bat, sh or app calling generate_emu_config app")
    print(" -clr:      clear output folder before generating the complete game config")
    print("            do note that it will not work when '-rel_raw' argument is used too")
    print("\nAll switches are optional except appid, at least 1 appid must be provided")
    print("\nAutomate the login prompt:")
    print(" * You can create a file called 'my_login.txt' beside the script, then add your:")
    print("   USERNAME on the first line")
    print("   PASSWORD on the second line")
    print(" * You can set these 2 environment variables (will override 'my_login.txt'):")
    print("   GSE_CFG_USERNAME")
    print("   GSE_CFG_PASSWORD")
    print("")


def main():
    USERNAME = ""
    PASSWORD = ""

    DOWNLOAD_SCREENSHOTS = False
    DOWNLOAD_VIDEOS = False
    DOWNLOAD_LOW = False
    DOWNLOAD_MAX = False
    DOWNLOAD_COMMON_IMAGES = False
    DOWNLOAD_SCX = False
    SAVE_APP_NAME = False
    GENERATE_CODEX_INI = False
    GENERATE_RUNE_INI = False
    GENERATE_ACHIEVEMENT_WATCHER_SCHEMAS = False
    CLEANUP_BEFORE_GENERATING = False
    ANON_LOGIN = False
    RELATIVE_DIR = False
    SKIP_ACH = False
    SKIP_CONTROLLER = False
    SKIP_INVENTORY = False
    DEFAULT_PRESET = True
    DEFAULT_PRESET_NO = 1


    if len(sys.argv) < 2:
        help()
        sys.exit(1)

    appids : set[int] = set()
    for appid in sys.argv[1:]:
        if f'{appid}'.isnumeric():
            appids.add(int(appid))
        elif f'{appid}'.lower() == '-scr':
            DOWNLOAD_SCREENSHOTS = True
        elif f'{appid}'.lower() == '-vids_low':
            DOWNLOAD_VIDEOS = True
            DOWNLOAD_LOW = True
        elif f'{appid}'.lower() == '-vids_max':
            DOWNLOAD_VIDEOS = True
            DOWNLOAD_MAX = True
        elif f'{appid}'.lower() == '-img':
            DOWNLOAD_COMMON_IMAGES = True
        elif f'{appid}'.lower() == '-name':
            SAVE_APP_NAME = True
        elif f'{appid}'.lower() == '-scx':
            DOWNLOAD_SCX = True
        elif f'{appid}'.lower() == '-cdx':
            GENERATE_CODEX_INI = True
        elif f'{appid}'.lower() == '-rne':
            GENERATE_RUNE_INI = True
        elif f'{appid}'.lower() == '-acw':
            GENERATE_ACHIEVEMENT_WATCHER_SCHEMAS = True
        elif f'{appid}'.lower() == '-clr':
            CLEANUP_BEFORE_GENERATING = True
        elif f'{appid}'.lower() == '-anon':
            ANON_LOGIN = True
        elif f'{appid}'.lower() == '-rel_out':
            RELATIVE_DIR = True
            RELATIVE_set = 'out'
        elif f'{appid}'.lower() == '-rel_raw':
            RELATIVE_DIR = True
            RELATIVE_set = 'raw'
        elif f'{appid}'.lower() == '-skip_ach':
            SKIP_ACH = True
        elif f'{appid}'.lower() == '-skip_con':
            SKIP_CONTROLLER = True
        elif f'{appid}'.lower() == '-skip_inv':
            SKIP_INVENTORY = True
        elif f'{appid}'.lower() == '-def1':
            DEFAULT_PRESET = True
            DEFAULT_PRESET_NO = 1
        elif f'{appid}'.lower() == '-def2':
            DEFAULT_PRESET = True
            DEFAULT_PRESET_NO = 2
        elif f'{appid}'.lower() == '-def3':
            DEFAULT_PRESET = True
            DEFAULT_PRESET_NO = 3
        elif f'{appid}'.lower() == '-def4':
            DEFAULT_PRESET = True
            DEFAULT_PRESET_NO = 4
        elif f'{appid}'.lower() == '-def5':
            DEFAULT_PRESET = True
            DEFAULT_PRESET_NO = 5
        else:
            print(f'___ Invalid switch: {appid}')
            help()
            sys.exit(1)
    
    if not appids:
        print(f'___ No app id was provided')
        help()
        sys.exit(1)

    client = SteamClient()
    # login_tmp_folder = os.path.join(get_exe_dir(False), "login_temp") # replaced 'RELATIVE_DIR with 'False' to always look for or create login_temp in generate_emu_config folder
    # if not os.path.exists(login_tmp_folder):
    #     os.makedirs(login_tmp_folder)
    # client.set_credential_location(login_tmp_folder)

    # first read the 'my_login.txt' file
    my_login_file = os.path.join(get_exe_dir(False), "my_login.txt") # replaced 'RELATIVE_DIR with 'False' to always look for or create my_login.txt in generate_emu_config folder
    if not ANON_LOGIN and os.path.isfile(my_login_file):
        filedata = ['']
        with open(my_login_file, "r", encoding="utf-8") as f:
            filedata = f.readlines()
        filedata = list(map(lambda s: s.replace("\r", "").replace("\n", ""), filedata))
        filedata = [l for l in filedata if l]
        if len(filedata) == 2:
            USERNAME = filedata[0]
            PASSWORD = filedata[1]
    
    # then allow the env vars to override the login details
    env_username = os.environ.get('GSE_CFG_USERNAME', None)
    env_password = os.environ.get('GSE_CFG_PASSWORD', None)
    if env_username:
        USERNAME = env_username
    if env_password:
        PASSWORD = env_password

    if ANON_LOGIN:
        result = client.anonymous_login()
        trials = 5
        while result != EResult.OK and trials > 0:
            time.sleep(1000)
            result = client.anonymous_login()
            trials -= 1
    else:
        webauth = WebAuth(USERNAME, PASSWORD)
        if (len(USERNAME) > 0 and len(PASSWORD) > 0):
            webauth.cli_login(USERNAME, PASSWORD)
        else:
            webauth_prompt_username = input("Enter Steam username: ")
            webauth.cli_login(webauth_prompt_username)
        client.login(webauth.username, access_token=webauth.refresh_token)

    # generate 'top_owners_ids.txt' if 'top_owners_ids.html' exists
    top_own.top_owners()

    # read and prepend top_owners_ids.txt
    top_owners_file = os.path.join(get_exe_dir(False), "top_owners_ids.txt") # replaced 'RELATIVE_DIR with 'False' to always look for or create top_owners_ids.txt in generate_emu_config folder
    if os.path.isfile(top_owners_file):
        filedata = ['']
        with open(top_owners_file, "r", encoding="utf-8") as f:
            filedata = f.readlines()
        filedata = list(map(lambda s: s.replace("\r", "").replace("\n", "").strip(), filedata))
        filedata = [l for l in filedata if len(l) > 1 and l.isdecimal()]
        all_ids = list(map(lambda s: int(s), filedata))
        TOP_OWNER_IDS[:0] = all_ids
            
    # prepend user account ID as a top owner
    if not ANON_LOGIN:
        TOP_OWNER_IDS.insert(0, client.steam_id.as_64)

    user_name = ''
    user_repl = ''
    if platform.system() == "Windows": # Windows
        user_name = os.getenv("USERNAME")
        user_repl = r'%username%'
    elif platform.system() == "Linux": # Linux
        user_name = os.getenv("USER")
        user_repl = r'$user'

    username = os.getenv("USERNAME") or os.getenv("USER")

    for appid in appids:

        print(" ")
        print(f"*** STARTED config for app id {appid} ***")
        print(" ")

        raw = client.get_product_info(apps=[appid])
        if raw["apps"]:
            print(f"[ ] Found app id on Steam store")
        else:
            print(f"[X] Cannot find app id on Steam store")
            print(" ")
            print(f"*** ABORTED config for app id {appid} ***")
            print(" ")
            break

        game_info : dict = raw["apps"][appid]
        game_info_common : dict = game_info.get("common", {})
        app_name = game_info_common.get("name", "")
        app_name_on_disk = f"{appid}"
        if app_name:
            print(f"[ ] Found app name on Steam store")
            print(f"[ ] __ orig name: '{app_name}'")
            sanitized_name = safe_name.create_safe_name(app_name)
            if sanitized_name:
                print(f"[ ] __ safe name: '{sanitized_name}'")
                if SAVE_APP_NAME:
                    app_name_on_disk = f'{sanitized_name} _ {appid}'
        else:
            app_name = f"Unknown_Steam_app_{appid}" # we need this for later use in the Achievement Watcher
            print(f"[X] Cannot find app name on Steam store")

        #root_backup_dir = os.path.join(get_exe_dir(False), "_BACKUP") # replaced 'RELATIVE_DIR with 'False' to always look for or create _BACKUP in generate_emu_config folder
        #backup_dir = os.path.join(root_backup_dir, f"{appid}")
        #if not os.path.exists(backup_dir):
        #    os.makedirs(backup_dir)

        root_def_dir_RELATIVE = os.path.join(get_exe_dir(True), "_DEFAULT") # _DEFAULT folder relative to external bat, sh or app calling generate_emu_config exe; with get_exe_dir(False) is only relative to generate_emu_config exe
        root_out_dir_RELATIVE = os.path.join(get_exe_dir(True), "_OUTPUT") # _OUTPUT folder relative to external bat, sh or app calling generate_emu_config exe; with get_exe_dir(False) is only relative to generate_emu_config exe

        if RELATIVE_DIR:
            if RELATIVE_set == 'out':
                root_out_dir = os.path.join(get_exe_dir(True), "_OUTPUT")
                base_out_dir = os.path.join(root_out_dir, app_name_on_disk)
                CLEANUP_override = 0
            elif RELATIVE_set == 'raw':
                root_out_dir = os.path.join(get_exe_dir(True))
                base_out_dir = os.path.join(get_exe_dir(True))
                CLEANUP_override = 1
                
            if os.path.exists(root_def_dir_RELATIVE) and os.path.isdir(root_def_dir_RELATIVE):
                if os.listdir(root_def_dir_RELATIVE):
                    root_def_dir = os.path.join(get_exe_dir(True), "_DEFAULT")
                else:
                    root_def_dir = os.path.join(get_exe_dir(False), "_DEFAULT")
            else:
                root_def_dir = os.path.join(get_exe_dir(False), "_DEFAULT")
        else:
            root_out_dir = os.path.join(get_exe_dir(False), "_OUTPUT")
            base_out_dir = os.path.join(root_out_dir, app_name_on_disk)
            CLEANUP_override = 0

            root_def_dir = os.path.join(get_exe_dir(False), "_DEFAULT")

        emu_settings_dir = os.path.join(base_out_dir, "steam_settings")
        info_out_dir = os.path.join(base_out_dir, "steam_misc\\app_info")

        print(f"[ ] DEF_DIR = {root_def_dir.replace(user_name, user_repl, 1)}")
        if RELATIVE_DIR and (RELATIVE_set == 'raw'):
            print(f"[ ] OUT_DIR = {os.getcwd().replace(user_name, user_repl, 1)}")
        else:
            print(f"[ ] OUT_DIR = {base_out_dir.replace(user_name, user_repl, 1)}")

        if CLEANUP_BEFORE_GENERATING:
            if CLEANUP_override == 0:
                print(f"[ ] Cleaning <OUT_DIR> folder...")
                base_dir_path = pathlib.Path(base_out_dir)
                if base_dir_path.is_file():
                    base_dir_path.unlink()
                    time.sleep(0.05)
                elif base_dir_path.is_dir():
                    shutil.rmtree(base_dir_path)
                    time.sleep(0.05)
                while base_dir_path.exists():
                    time.sleep(0.05)

        root_backup_dir = os.path.join(base_out_dir, "steam_misc\\app_backup")
        #backup_dir = os.path.join(root_backup_dir, f"{appid}")
        backup_dir = root_backup_dir #use different structure for 'backup' dir

        if not os.path.exists(backup_dir):
            os.makedirs(backup_dir)

        if not os.path.exists(emu_settings_dir):
            os.makedirs(emu_settings_dir)

        if not os.path.exists(info_out_dir):
            os.makedirs(info_out_dir)

        #with open(os.path.join(info_out_dir, "app_widget.url"), mode='w', newline='\r\n') as f:
            #f.write(f"[InternetShortcut]\nURL=https://store.steampowered.com/widget/{appid}/")

        print(f"[ ] Copying preset emu configs...")
        shutil.copytree(os.path.join(root_def_dir, str(0)), base_out_dir, dirs_exist_ok=True) # copy from default emu dir
        print(f"[ ] __ default emu config from <DEF_DIR>\\{str(0)} folder")
        shutil.copytree(os.path.join(root_def_dir, str(DEFAULT_PRESET_NO)), base_out_dir, dirs_exist_ok=True) # copy from preset emu dir
        print(f"[ ] __ preset emu config from <DEF_DIR>\\{str(DEFAULT_PRESET_NO)} folder")
        if os.path.exists(os.path.join(root_def_dir, str(appid))):
            shutil.copytree(os.path.join(root_def_dir, str(appid)), base_out_dir, dirs_exist_ok=True) # copy from preset app dir
            print(f"[ ] __ app emu config from <DEF_DIR>\\{str(appid)} folder")

        with open(os.path.join(emu_settings_dir, "steam_appid.txt"), 'w') as f:
            f.write(str(appid))
            #print(f"[ ] Writing 'steam_appid.txt'")

        print(f"[ ] Found product info --- writing to <OUT_DIR>\\steam_misc\\app_info\\app_product_info.json")

        with open(os.path.join(info_out_dir, "app_product_info.json"), "wt", encoding='utf-8') as f:
            json.dump(game_info, f, ensure_ascii=False, indent=2)
            #print(f"[ ] Writing 'app_product_info.json'")

        with open(os.path.join(backup_dir, "product_info.json"), "wt", encoding='utf-8') as f:
            json.dump(game_info, f, ensure_ascii=False, indent=2)
            #print(f"[ ] Writing 'product_info.json'")
        
        app_details.download_app_details(
            base_out_dir, info_out_dir,
            appid,
            DOWNLOAD_SCREENSHOTS,
            DOWNLOAD_VIDEOS,
            DOWNLOAD_LOW,
            DOWNLOAD_MAX)

        clienticon : str = None
        icon : str = None
        logo : str = None
        logo_small : str = None
        achievements : list[dict] = []
        languages : list[str] = []
        app_exe = ''
        if game_info_common:
            if "clienticon" in game_info_common:
                clienticon = f"{game_info_common['clienticon']}"
            
            if "icon" in game_info_common:
                icon = f"{game_info_common['icon']}"
            
            if "logo" in game_info_common:
                logo = f"{game_info_common['logo']}"
            
            if "logo_small" in game_info_common:
                logo_small = f"{game_info_common['logo_small']}"
            
            #if "community_visible_stats" in game_info_common: #NOTE: checking this seems to skip stats on a few games so it's commented out
            if not SKIP_ACH:
                achievements = generate_achievement_stats(client, appid, emu_settings_dir, backup_dir)

            if "supported_languages" in game_info_common:
                langs: dict[str, dict] = game_info_common["supported_languages"]
                for lang in langs:
                    support: str = langs[lang].get("supported", "").lower()
                    if support == "true" or support == "1":
                        languages.append(f'{lang}'.lower())

        if languages:
            with open(os.path.join(emu_settings_dir, "supported_languages.txt"), 'wt', encoding='utf-8') as f:
                for lang in languages:
                    f.write(f'{lang}\n')
                #print(f"[ ] Writing 'supported_languages.txt'")
                if len(languages) == 1:
                    print(f"[ ] Found {len(languages)} supported language --- writing to <OUT_DIR>\\steam_settings\\supported_languages.txt")
                else:
                    print(f"[ ] Found {len(languages)} supported languages --- writing to <OUT_DIR>\\steam_settings\\supported_languages.txt")
        else:
            print(f"[?] No supported languages found - skip creating <OUT_DIR>\\steam_settings\\supported_languages.txt")

        ReplaceStringInFile(os.path.join(emu_settings_dir, "configs.app.ini"), 'This is another example DLC name', '#   56789=', '56789=') # make sure we write DLCs after '#   56789=This is another example DLC name'

        # use ConfigObj to correctly update existing 'configs.app.ini' copied from ./DEFAULT configuration --- START, read ini
        configs_app = ConfigObj(os.path.join(emu_settings_dir, "configs.app.ini"), encoding='utf-8')

        ''' # NOTE no need to write build_id to ini anymore - it will be read from 'branches.json'
        if "depots" in game_info:
            if "branches" in game_info["depots"]:
                if "public" in game_info["depots"]["branches"]:
                    if "buildid" in game_info["depots"]["branches"]["public"]:
                        buildid = game_info["depots"]["branches"]["public"]["buildid"]
                        configs_app['app::general']['build_id'] = str(buildid) #updated ini through ConfigObj # NOTE deprecated, build id is read from 'branches.json'
        '''
                        
        dlc_config_list : list[tuple[int, str]] = []
        dlc_list, depot_app_list, all_depots, all_branches = get_depots_infos(game_info, appid)
        dlc_raw = {}
        if dlc_list:
            dlc_raw = client.get_product_info(apps=dlc_list)["apps"]
            for dlc in dlc_raw:
                dlc_name = ''
                try:
                    dlc_name = f'{dlc_raw[dlc]["common"]["name"]}'
                except Exception:
                    pass
                
                if not dlc_name:
                    dlc_name = f"Unknown Steam app {dlc}"
                
                dlc_config_list.append((dlc, dlc_name))

            if len(dlc_list) == 1:
                print(f"[ ] Found {len(dlc_config_list)} DLC --- writing to <OUT_DIR>\\steam_settings\\configs.app.ini")
            else:
                print(f"[ ] Found {len(dlc_config_list)} DLCs --- writing to <OUT_DIR>\\steam_settings\\configs.app.ini")
        else:
            print(f"[?] No DLCs found - skip writing to <OUT_DIR>\\steam_settings\\configs.app.ini")

        if not dlc_raw == {}:
            with open(os.path.join(info_out_dir, "dlc_product_info.json"), "wt", encoding='utf-8') as f:
                json.dump(dlc_raw, f, ensure_ascii=False, indent=2)
                #print(f"[ ] Writing 'dlc_product_info.json'")

        # we set unlock_all=0 nonetheless, to make the emu lock DLCs, otherwise everything is allowed
        # some games use that as a detection mechanism 
        #configs["app::dlcs"]["unlock_all"] = str(0) #updated ini through ConfigObj - disabled, keep the existing value from default 'configs.app.ini'

        for x in dlc_config_list:
            configs_app["app::dlcs"][str(x[0])] = str(x[1]) #updated ini through ConfigObj
            # used x[1].encode('utf-8') instead of str(x[1]) to properly deal with DLC names containing special characters like (TM) sign, (C) sign, etc

        # use ConfigObj to correctly update existing 'configs.app.ini' copied from ./DEFAULT configuration --- END, write ini
        configs_app.write()
        #print(f"[ ] Writing 'configs.app.ini'")

        ReplaceStringInFile(os.path.join(emu_settings_dir, "configs.app.ini"), ' = "', '"', '')

        # ConfigObj overrides the default ini format, adding spaces before and after '=' and '""' for empty keys, so we'll use this to undo the changes
        with open(os.path.join(emu_settings_dir, "configs.app.ini"), 'r') as file:
            filedata = file.read()
        filedata = filedata.replace(' = ""', '=')
        filedata = filedata.replace(' = ', '=')
        with open(os.path.join(emu_settings_dir, "configs.app.ini"), 'w') as file:
            file.write(filedata)

        ReplaceStringInFile(os.path.join(emu_settings_dir, "configs.app.ini"), 'This is another example DLC name', '56789=', '#   56789=') # make sure we write DLCs after '#   56789=This is another example DLC name'

        # use ConfigObj to correctly update existing 'configs.main.ini' copied from ./DEFAULT configuration --- START, read ini
        configs_main = ConfigObj(os.path.join(emu_settings_dir, "configs.main.ini"), encoding='utf-8')

        # use CongigObj to correctly update existing 'configs.main.ini' copied from ./DEFAULT configuration --- END, write ini
        configs_main.write()
        #print(f"[ ] Writing 'configs.main.ini'")

        # ConfigObj overrides the default ini format, adding spaces before and after '=' and '""' for empty keys, so we'll use this to undo the changes
        with open(os.path.join(emu_settings_dir, "configs.main.ini"), 'r') as file:
            filedata = file.read()
        filedata = filedata.replace(' = ""', '=')
        filedata = filedata.replace(' = ', '=')
        with open(os.path.join(emu_settings_dir, "configs.main.ini"), 'w') as file:
            file.write(filedata)

        # use ConfigObj to correctly update existing 'configs.overlay.ini' copied from ./DEFAULT configuration --- START, read ini
        configs_overlay = ConfigObj(os.path.join(emu_settings_dir, "configs.overlay.ini"), encoding='utf-8')

        # use CongigObj to correctly update existing 'configs.overlay.ini' copied from ./DEFAULT configuration --- END, write ini
        configs_overlay.write()
        #print(f"[ ] Writing 'configs.overlay.ini'")

        # ConfigObj overrides the default ini format, adding spaces before and after '=' and '""' for empty keys, so we'll use this to undo the changes
        with open(os.path.join(emu_settings_dir, "configs.overlay.ini"), 'r') as file:
            filedata = file.read()
        filedata = filedata.replace(' = ""', '=')
        filedata = filedata.replace(' = ', '=')
        with open(os.path.join(emu_settings_dir, "configs.overlay.ini"), 'w') as file:
            file.write(filedata)

        # use ConfigObj to correctly update existing 'configs.user.ini' copied from ./DEFAULT configuration --- START, read ini
        configs_user = ConfigObj(os.path.join(emu_settings_dir, "configs.user.ini"), encoding='utf-8')

        # use ConfigObj to correctly update existing 'configs.user.ini' copied from ./DEFAULT configuration --- END, write ini
        configs_user.write()
        #print(f"[ ] Writing 'configs.user.ini'")

        # ConfigObj overrides the default ini format, adding spaces before and after '=' and '""' for empty keys, so we'll use this to undo the changes
        with open(os.path.join(emu_settings_dir, "configs.user.ini"), 'r') as file:
            filedata = file.read()
        filedata = filedata.replace(' = ""', '=')
        filedata = filedata.replace(' = ', '=')
        with open(os.path.join(emu_settings_dir, "configs.user.ini"), 'w') as file:
            file.write(filedata)

        if all_depots:
            with open(os.path.join(emu_settings_dir, "depots.txt"), 'wt', encoding="utf-8") as f:
                for game_depot in all_depots:
                    f.write(f"{game_depot}\n")
                #print(f"[ ] Writing 'depots.txt'")
                if len(all_depots) == 1:
                    print(f"[ ] Found {len(all_depots)} depot --- writing to <OUT_DIR>\\steam_settings\\depots.txt")
                else:
                    print(f"[ ] Found {len(all_depots)} depots --- writing to <OUT_DIR>\\steam_settings\\depots.txt")
        else:
            print(f"[?] No depots found - skip creating <OUT_DIR>\\steam_settings\\depots.txt")

        if len(all_branches) >= 1:
            with open(os.path.join(emu_settings_dir, "branches.json"), "wt", encoding='utf-8') as f:
                json.dump(all_branches, f, ensure_ascii=False, indent=2)
                if len(all_branches) == 1:
                    print(f"[ ] Found {len(all_branches)} branch --- writing to <OUT_DIR>\\steam_settings\\branches.json")
                else:
                    print(f"[ ] Found {len(all_branches)} branches --- writing to <OUT_DIR>\\steam_settings\\branches.json")
                if "public" in game_info["depots"]["branches"]:
                    if "buildid" in game_info["depots"]["branches"]["public"]:
                        buildid = game_info["depots"]["branches"]["public"]["buildid"]
                        print(f"[ ] __ default branch name: public, latest build id: {buildid}")
        else:
            print(f"[?] No branches found - skip creating <OUT_DIR>\\steam_settings\\branches.json")

        # read some keys from 'configs.user.ini'
        cfg_user = ConfigObj(os.path.join(emu_settings_dir, "configs.user.ini"), encoding='utf-8')
        cfg_user_account_name = cfg_user["user::general"]["account_name"]
        cfg_user_account_steamid = cfg_user["user::general"]["account_steamid"]
        cfg_user_language = cfg_user["user::general"]["language"]

        config_found = 0 # needed to show 'No controller configs found...' if value remains 0
        config_generated = 0 # used to avoid overwriting supported config by unsupported one
        downloading_ctrl_vdf = 0 # needed to remove possible duplicate 'Found controller configs...'
        valid_id = 0 # needed to skip showing "Found controller configs..." if no valid is found in either "steamcontrollerconfigdetails" or "steamcontrollertouchconfigdetails"
        if "config" in game_info:
            if not SKIP_CONTROLLER and "steamcontrollerconfigdetails" in game_info["config"]:
                controller_details = game_info["config"]["steamcontrollerconfigdetails"]
                for id in controller_details:
                    # make sure the controller config id exists and is a numeric string
                    # fixes "TypeError: string indices must be integers, not 'str'" when generating for "Unknown 9: Awakening" (appid 1477940)
                    if id.isdigit():
                        if (downloading_ctrl_vdf == 0) and (valid_id == 0):
                            print(f"[ ] Found controller configs --- generating action sets...")
                            downloading_ctrl_vdf = 1
                            valid_id = 1
                        details = controller_details[id]
                        controller_type = ""
                        enabled_branches = ""
                        if "controller_type" in details:
                            controller_type = details["controller_type"]
                        if "enabled_branches" in details:
                            enabled_branches = details["enabled_branches"]

                        out_vdf = None # initialize out_vdf, fixes "UnboundLocalError: cannot access local variable 'out_vdf' where it is not associated with a value" when generating for "Factorio" (appid 427520)
                            
                        if (("default" in enabled_branches) or ("public" in enabled_branches)): # download only 'default' and 'public' branches to avoid multiple configs for same controller type
                            print(f'[ ] __ downloading config, file id = {id}, controller type = {controller_type}') # first noticed for Elden Ring, two 'controller_ps4' vdf configs are downloaded, but only one of them is converted to action sets
                            out_vdf = download_published_file(client, int(id), os.path.join(backup_dir, 'controller\\' + f'{controller_type}' + '_' + f'{id}')) 

                        if out_vdf is not None:
                            if (controller_type in ["controller_xbox360", "controller_xboxone"] and (("default" in enabled_branches) or ("public" in enabled_branches))):
                                config_found = 1
                                #print(f"[ ] __ controller type '{controller_type}' is supported ... converting .vdf to action sets")
                                if config_generated == 0:
                                    print(f"[ ] __ parsing '{controller_type}' vdf - supported, can be used with emu")
                                    parse_controller_vdf.generate_controller_config(out_vdf.decode('utf-8'), os.path.join(os.path.join(backup_dir, 'controller\\' + f'{controller_type}' + '_' + f'{id}'), "action_set"))

                                    # delete txt files in .\steam_settings\controller folder
                                    for txt_file in os.listdir(os.path.join(emu_settings_dir, "controller")):
                                        if not txt_file.endswith(".txt"):
                                            continue
                                        os.remove(os.path.join(os.path.join(emu_settings_dir, "controller"), txt_file))
                                    shutil.copytree(os.path.join(os.path.join(backup_dir, 'controller\\' + f'{controller_type}' + '_' + f'{id}'), "action_set"), os.path.join(emu_settings_dir, "controller"), dirs_exist_ok=True)
                                    config_generated = 1
                                else:
                                    print(f"[ ] __ parsing '{controller_type}' vdf - supported, can be used with emu")
                                    parse_controller_vdf.generate_controller_config(out_vdf.decode('utf-8'), os.path.join(os.path.join(backup_dir, 'controller\\' + f'{controller_type}' + '_' + f'{id}'), "action_set"))

                                    if controller_type in ["controller_xboxone"]: # always use xboxone config if present
                                        # delete txt files in .\steam_settings\controller folder
                                        for txt_file in os.listdir(os.path.join(emu_settings_dir, "controller")):
                                            if not txt_file.endswith(".txt"):
                                                continue
                                            os.remove(os.path.join(os.path.join(emu_settings_dir, "controller"), txt_file))
                                        shutil.copytree(os.path.join(os.path.join(backup_dir, 'controller\\' + f'{controller_type}' + '_' + f'{id}'), "action_set"), os.path.join(emu_settings_dir, "controller"), dirs_exist_ok=True)
                                        #config_generated = 1

                            elif (controller_type in ["controller_ps4", "controller_ps5", "controller_steamcontroller_gordon", "controller_neptune", "controller_switch_pro"] and (("default" in enabled_branches) or ("public" in enabled_branches))):
                                config_found=1
                                #print(f"[ ] __ controller type '{controller_type}' is not supported ... converting .vdf to action sets")
                                print(f"[ ] __ parsing '{controller_type}' vdf - not supported, backup purposes only")
                                parse_controller_vdf.generate_controller_config(out_vdf.decode('utf-8'), os.path.join(os.path.join(backup_dir, 'controller\\' + f'{controller_type}' + '_' + f'{id}'), "action_set"))
            
            if not SKIP_CONTROLLER and "steamcontrollertouchconfigdetails" in game_info["config"]:
                controller_details = game_info["config"]["steamcontrollertouchconfigdetails"]
                for id in controller_details:
                    # make sure the controller config id exists and is a numeric string
                    # fixes "TypeError: string indices must be integers, not 'str'" when generating for "Unknown 9: Awakening" (appid 1477940)
                    if id.isdigit():
                        if (downloading_ctrl_vdf == 0) and (valid_id == 0):
                            print(f"[ ] Found controller configs --- generating action sets...")
                            downloading_ctrl_vdf = 1
                            valid_id = 1
                        details = controller_details[id]
                        controller_type = ""
                        enabled_branches = ""
                        if "controller_type" in details:
                            controller_type = details["controller_type"]
                        if "enabled_branches" in details:
                            enabled_branches = details["enabled_branches"]

                        out_vdf = None # initialize out_vdf, fixes "UnboundLocalError: cannot access local variable 'out_vdf' where it is not associated with a value" when generating for "Factorio" (appid 427520)

                        if (("default" in enabled_branches) or ("public" in enabled_branches)): # download only 'default' and 'public' branches to avoid multiple configs for same controller type
                            print(f'[ ] __ downloading config, file id = {id}, controller type = {controller_type}') # first noticed for Elden Ring, two 'controller_ps4' vdf configs are downloaded, but only one of them is converted to action sets
                            out_vdf = download_published_file(client, int(id), os.path.join(backup_dir, 'controller\\' + f'{controller_type}' + '_' + f'{id}')) 
                            
                        if out_vdf is not None:
                            if (controller_type in ["controller_mobile_touch"] and (("default" in enabled_branches) or ("public" in enabled_branches))):
                                config_found = 1
                                #print(f"[ ] __ controller type '{controller_type}' is not supported ... converting .vdf to action sets")
                                print(f"[ ] __ parsing '{controller_type}' vdf - not supported, backup purposes only")
                                parse_controller_vdf.generate_controller_config(out_vdf.decode('utf-8'), os.path.join(os.path.join(backup_dir, 'controller\\' + f'{controller_type}' + '_' + f'{id}'), "action_set"))

            ''' # NOTE zip the parent 'app_backup' folder instead of only the child 'controller' folder
            if config_found:
                shutil.make_archive(os.path.join(backup_dir, 'controller'), 'zip', os.path.join(backup_dir, 'controller')) # first argument is the name of the zip file
                shutil.rmtree(os.path.join(backup_dir, 'controller'))
                os.makedirs(os.path.join(backup_dir, 'controller'))
                shutil.move(os.path.join(backup_dir, 'controller.zip'), os.path.join(backup_dir, 'controller\\controller.zip'))
            '''

            if config_found == 0:
                print(f"[?] No controller configs found - skip generating action sets")

            if "supported_languages" in game_info["common"]:
                languages_config = game_info["common"]["supported_languages"]
                with open(os.path.join(info_out_dir, "common_supported_languages.json"), "wt", encoding='utf-8') as f:
                    json.dump(languages_config, f, ensure_ascii=False, indent=2)
                    #print(f"[ ] Writing 'common_supported_languages.json'")

            if "launch" in game_info["config"]:
                launch_configs = game_info["config"]["launch"]
                with open(os.path.join(info_out_dir, "config_launch.json"), "wt", encoding='utf-8') as f:
                    json.dump(launch_configs, f, ensure_ascii=False, indent=2)
                    #print(f"[ ] Writing 'config_launch.json'")
                
                app_type : str = ""
                app_mode_tmp : str = ""
                app_mode_new : str = ""
                first_app_exe : str = None
                default_app_exe : str = None
                prefered_app_exe : str = None
                unwanted_app_exe_launcher = ["launch", "start", "play", "settings"]
                unwanted_app_exe_demo = ["demo"]
                unwanted_app_exe_vr = ["_vr", "-vr", "vr_", "vr-"]
                unwanted_app_exe_benchmark = ["benchmark"]
                for cfg in launch_configs.values():
                    if "executable" in cfg:
                        app_exe = f'{cfg["executable"]}'
                        app_exe = app_exe.replace("\\", "/").split('/')[-1]

                        first_app_exe = app_exe
                        if "type" in cfg:
                            app_type = f'{cfg["type"]}'
                            if app_type == "default":
                                default_app_exe = app_exe    
                            else:
                                prefered_app_exe = app_exe 
                        else:
                            prefered_app_exe = app_exe
                    
                    if all(app_exe.lower().find(unwanted_exe_l) < 0 for unwanted_exe_l in unwanted_app_exe_launcher):
                        app_mode_tmp = app_mode_tmp + " _no_launcher_ "
                    if all(app_exe.lower().find(unwanted_exe_d) < 0 for unwanted_exe_d in unwanted_app_exe_demo):
                        app_mode_tmp = app_mode_tmp + " _no_demo_ "
                    if all(app_exe.lower().find(unwanted_exe_v) < 0 for unwanted_exe_v in unwanted_app_exe_vr):
                        app_mode_tmp = app_mode_tmp + " _no_vr_ "
                    if all(app_exe.lower().find(unwanted_exe_b) < 0 for unwanted_exe_b in unwanted_app_exe_benchmark):
                        app_mode_tmp = app_mode_tmp + " _no_benchmark_ "

                    if platform.system() == "Windows": # Windows
                        if app_exe.lower().endswith(".exe"):
                            break
                    elif platform.system() == "Linux": # Linux
                        if app_exe.lower().endswith(".sh"):
                            break
                    elif platform.system() == "Darwin": # OSX
                        if app_exe.lower().endswith(".app"):
                            break

                if default_app_exe:
                    app_exe = default_app_exe
                elif prefered_app_exe:
                    app_exe = prefered_app_exe
                elif first_app_exe:
                    app_exe = first_app_exe

                if not "_no_launcher_" in app_mode_tmp:
                    app_mode_new = app_mode_new + " launcher "
                if not "_no_demo_" in app_mode_tmp:
                    app_mode_new = app_mode_new + " demo "
                if not "_no_vr_" in app_mode_tmp:
                    app_mode_new = app_mode_new + " vr "
                if not "_no_benchmark_" in app_mode_tmp:
                    app_mode_new = app_mode_new + " benchmark "

                app_mode_new = app_mode_new.lstrip(" ")
                app_mode_new = app_mode_new.replace("  ", ", ")
                app_mode_new = app_mode_new.rstrip(" ")

            if "ufs" in game_info:
                savegame_configs = game_info["ufs"]
                with open(os.path.join(info_out_dir, "config_ufs.json"), "wt", encoding='utf-8') as f:
                    json.dump(savegame_configs, f, ensure_ascii=False, indent=2)
                    #print(f"[ ] Writing 'config_ufs.json'")
                
        inventory_data = None
        if not SKIP_INVENTORY:
            inventory_data = generate_inventory(client, appid)
        if inventory_data is not None:
            out_inventory = {}
            default_items = {}
            inventory = json.loads(inventory_data.rstrip(b"\x00"))
            raw_inventory = json.dumps(inventory, indent=4)

            if len(inventory) != 1:
                print(f"[ ] Found {len(inventory)} inventory items --- writing to <OUT_DIR>\\steam_settings\\items.json & default_items.json")
            else:
                print(f"[ ] Found {len(inventory)} inventory item --- writing to <OUT_DIR>\\steam_settings\\items.json & default_items.json")

            with open(os.path.join(backup_dir, f"InventoryItems_{appid}.json"), "w") as f:
                f.write(raw_inventory)
                #print(f"[ ] Writing 'inventory.json'")

            for i in inventory:
                index = str(i["itemdefid"])
                x = {}
                for t in i:
                    if i[t] is True:
                        x[t] = "true"
                    elif i[t] is False:
                        x[t] = "false"
                    else:
                        x[t] = str(i[t])
                out_inventory[index] = x
                default_items[index] = 1

            with open(os.path.join(emu_settings_dir, "items.json"), "wt", encoding='utf-8') as f:
                json.dump(out_inventory, f, ensure_ascii=False, indent=2)
                #print(f"[ ] __ writing 'items.json'")

            with open(os.path.join(emu_settings_dir, "default_items.json"), "wt", encoding='utf-8') as f:
                json.dump(default_items, f, ensure_ascii=False, indent=2)
                #print(f"[ ] __ writing 'default_items.json'")

        else:
            print(f"[?] No inventory items found - skip creating <OUT_DIR>\\steam_settings\\items.json & default_items.json")

        if app_exe:
            if app_mode_new != "":
                #print(f"[ ] Detected app exe: '{app_exe}', tags: {app_mode_new}") # use it to get some idea of what the exe might be
                print(f"[ ] Detected app exe: '{app_exe}'")
            else:
                #print(f"[ ] Detected app exe: '{app_exe}', tags: app") # use it to get some idea of what the exe might be
                print(f"[ ] Detected app exe: '{app_exe}'")
        else:
            print(f"[X] Cannot detect app exe")

        ''' # NOTE proof of concept code to get a string between two substrings in a file, e.g. get metacritic link for app from app_details.json
        app_metacritic = []
        if os.path.isfile(os.path.join(base_out_dir, 'steam_misc\\app_info\\app_details.json')):
            with open(os.path.join(base_out_dir, 'steam_misc\\app_info\\app_details.json'), 'r', encoding='utf-8') as app_det:
                app_det_line = app_det.readlines()
            for line in app_det_line:
                if '"url": "https://www.metacritic' in line:
                    app_metacritic = GetListOfSubstrings(line,'"url": "https://www.metacritic', '"')
                    break
        '''

        if os.path.isdir(os.path.join(base_out_dir, 'steam_misc\\app_backup')):
            if os.listdir(os.path.join(base_out_dir, 'steam_misc\\app_backup')): # zip 'app_backup' folder only if not empty
                shutil.make_archive(os.path.join(base_out_dir, 'steam_misc\\app_backup'), 'zip', os.path.join(base_out_dir, 'steam_misc\\app_backup')) # first argument is the name of the zip file
                shutil.rmtree(os.path.join(base_out_dir, 'steam_misc\\app_backup'))
                os.makedirs(os.path.join(base_out_dir, 'steam_misc\\app_backup'))
                shutil.move(os.path.join(base_out_dir, 'steam_misc\\app_backup.zip'), os.path.join(base_out_dir, 'steam_misc\\app_backup\\app_backup.zip'))

        if DOWNLOAD_COMMON_IMAGES:
            app_images.download_app_images(
                base_out_dir,
                appid,
                clienticon,
                icon,
                logo,
                logo_small)

        if GENERATE_ACHIEVEMENT_WATCHER_SCHEMAS:
            ach_watcher_gen.generate_all_ach_watcher_schemas(
                base_out_dir,
                appid,
                app_name,
                app_exe,
                achievements,
                icon)
        
        if GENERATE_CODEX_INI:
            cdx_gen.generate_cdx_ini(
                base_out_dir,
                appid,
                cfg_user_account_steamid,
                cfg_user_account_name,
                cfg_user_language,
                dlc_config_list,
                achievements)
            
        if GENERATE_RUNE_INI:
            rne_gen.generate_rne_ini(
                base_out_dir,
                appid,
                cfg_user_account_steamid,
                cfg_user_account_name,
                cfg_user_language,
                dlc_config_list,
                achievements)
            
        if DOWNLOAD_SCX: 
            scx_gen.download_scx(base_out_dir, appid)
        
        print(" ")
        print(f"*** FINISHED config for app id {appid} ***")
        print(" ")

def _tracebackPrint(_errorValue):
    print("Unexpected error:")
    print(_errorValue)
    print("-----------------------")
    for line in traceback.format_exception(_errorValue):
        print(line)
    print("-----------------------")

if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        if 'client_id' in e.args:
            print("Wrong Steam username and / or password. Please try again!")
            try:
                main()
            except Exception as e:
                if 'client_id' in e.args:
                    print("Wrong Steam username and / or password. Please try again!")
                    try:
                        main()
                    except Exception as e:
                        if 'client_id' in e.args:
                            print("Wrong Steam username and / or password. Please try again!")
                            sys.exit(1)
                        else:
                            _tracebackPrint(e)
                            sys.exit(1)
                else:
                    _tracebackPrint(e)
                    sys.exit(1)
        else:
            _tracebackPrint(e)
            sys.exit(1)

