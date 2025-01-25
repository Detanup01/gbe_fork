import pathlib
import time
from stats_schema_achievement_gen import achievements_gen
from external_components import (
    ach_watcher_gen, cdx_gen, app_images, app_details, safe_name
)
from controller_config_generator import parse_controller_vdf
from steam.client import SteamClient
from steam.webauth import WebAuth
from steam.enums.common import EResult
from steam.enums.emsg import EMsg
from steam.core.msg import MsgProto
import os
import sys
import json
import requests
import threading
import queue
import shutil
import traceback


#steam ids with public profiles that own a lot of games
# https://steamladder.com/ladder/games/
# in browser console:
#const links = $x("/html/body/div[3]/table/tbody/tr/td[2]/a[@href]/@href");
#console.clear();
#for (let index = 0; index < links.length; index++) {
#    const usr_link = links[index].textContent.split('/').filter(s => s);
#    const usr_id = usr_link[usr_link.length - 1]
#    console.log(usr_id)
#}
TOP_OWNER_IDS = list(dict.fromkeys([
    76561198213148949,
    76561198108581917,
    76561198028121353,
    76561197979911851,
    76561198355625888,
    76561198237402290,
    76561197969050296,
    76561198152618007,
    76561198001237877,
    76561198037867621,
    76561198001678750,
    76561198217186687,
    76561198094227663,
    76561197993544755,
    76561197963550511,
    76561198095049646,
    76561197973009892,
    76561197969810632,
    76561198388522904,
    76561198864213876,
    76561198166734878,
    # 76561198017975643,
    # 76561198044596404,
    # 76561197976597747,
    # 76561197962473290,
    # 76561197976968076,
    # 76561198235911884,
    # 76561198313790296,
    # 76561198407953371,
    # 76561198063574735,
    # 76561198122859224,
    # 76561198154462478,
    # 76561197996432822,
    # 76561197979667190,
    # 76561198139084236,
    # 76561198842864763,
    # 76561198096081579,
    # 76561198019712127,
    # 76561198033715344,
    # 76561198121398682,
    # 76561198027233260,
    # 76561198104323854,
    # 76561197995070100,
    # 76561198001221571,
    # 76561198005337430,
    # 76561198085065107,
    # 76561198027214426,
    # 76561198062901118,
    # 76561198008181611,
    # 76561198124872187,
    # 76561198048373585,
    # 76561197974742349,
    # 76561198040421250,
    # 76561198017902347,
    # 76561198010615256,
    # 76561197970825215,
    # 76561198077213101,
    # 76561197971011821,
    # 76561197992133229,
    # 76561197963534359,
    # 76561198077248235,
    # 76561198152760885,
    # 76561198256917957,
    # 76561198326510209,
    # 76561198019009765,
    # 76561198047438206,
    # 76561198128158703,
    # 76561198037809069,
    # 76561198121336040,
    # 76561198102767019,
    # 76561198063728345,
    # 76561198082995144,
    # 76561197981111953,
    # 76561197995008105,
    # 76561198109083829,
    # 76561197968410781,
    # 76561198808371265,
    # 76561198025858988,
    # 76561198252374474,
    # 76561198382166453,
    # 76561198396723427,
    # 76561197992548975,
    # 76561198134044398,
    # 76561198029503957,
    # 76561197990233857,
    # 76561197971026489,
    # 76561197965978376,
    # 76561197976796589,
    # 76561197994616562,
    # 76561197984235967,
    # 76561197992967892,
    # 76561198097945516,
    # 76561198251835488,
    # 76561198281128349,
    # 76561198044387084,
    # 76561198015685843,
    # 76561197993312863,
    # 76561198020125851,
    # 76561198006391846,
    # 76561198158932704,
    # 76561198039492467,
    # 76561198035552258,
    # 76561198031837797,
    # 76561197982718230,
    # 76561198025653291,
    # 76561197972951657,
    # 76561198269242105,
    # 76561198004332929,
    # 76561197972378106,
    # 76561197962630138,
    # 76561198192399786,
    # 76561198119667710,
    # 76561198120120943,
    # 76561198015992850,
    # 76561198096632451,
    # 76561198008797636,
    # 76561198118726910,
    # 76561198018254158,
    # 76561198061393233,
    # 76561198086250077,
    # 76561198025391492,
    # 76561198050474710,
    # 76561197997477460,
    # 76561198105279930,
    # 76561198026221141,
    # 76561198443388781,
    # 76561197981228012,
    # 76561197986240493,
    # 76561198003041763,
    # 76561198056971296,
    # 76561198072936438,
    # 76561198264362271,
    # 76561198101049562,
    # 76561198831075066,
    # 76561197991699268,
    # 76561198042965266,
    # 76561198019555404,
    # 76561198111433283,
    # 76561197984010356,
    # 76561198427572372,
    # 76561198071709714,
    # 76561198034213886,
    # 76561198846208086,
    # 76561197991613008,
    # 76561197978640923,
    # 76561198009596142,
    # 76561199173688191,
    # 76561198294806446,
    # 76561197992105918,
    # 76561198155124847,
    # 76561198032614383,
    # 76561198051740093,
    # 76561198051725954,
    # 76561198048151962,
    # 76561198172367910,
    # 76561198043532513,
    # 76561198029532782,
    # 76561198106145311,
    # 76561198020746864,
    # 76561198122276418,
    # 76561198844130640,
    # 76561198890581618,
    # 76561198021180815,
    # 76561198046642155,
    # 76561197985091630,
    # 76561198119915053,
    # 76561198318547224,
    # 76561198426000196,
    # 76561197988052802,
    # 76561198008549198,
    # 76561198054210948,
    # 76561198028011423,
    # 76561198026306582,
    # 76561198079227501,
    # 76561198070220549,
    # 76561198034503074,
    # 76561198172925593,
    # 76561198286209051,
    # 76561197998058239,
    # 76561198057648189,
    # 76561197982273259,
    # 76561198093579202,
    # 76561198035612474,
    # 76561197970307937,
    # 76561197996825541,
    # 76561197981027062,
    # 76561198019841907,
    # 76561197970727958,
    # 76561197967716198,
    # 76561197970545939,
    # 76561198315929726,
    # 76561198093753361,
    # 76561198413266831,
    # 76561198045540632,
    # 76561198015514779,
    # 76561198004532679,
    # 76561198080773680,
    # 76561198079896896,
    # 76561198005299723,
    # 76561198337784749,
    # 76561198150126284,
    # 76561197988445370,
    # 76561198258304011,
    # 76561198321551799,
    # 76561197973701057,
    # 76561197973230221,
    # 76561198002535276,
    # 76561198100306249,
    # 76561198116086535,
    # 76561197970970678,
    # 76561198085238363,
    # 76561198007200913,
    # 76561198025111129,
    # 76561198068747739,
    # 76561197970539274,
    # 76561198148627568,
    # 76561197970360549,
    # 76561198098314980,
    # 76561197972529138,
    # 76561198007403855,
    # 76561197977403803,
    # 76561198124865933,
    # 76561197981323238,
    # 76561197960330700,
    # 76561198217979953,
    # 76561197960366517,
    # 76561198044067612,
    # 76561197967197052,
    # 76561198027066612,
    # 76561198072833066,
    # 76561198033967307,
    # 76561198104561325,
    # 76561198272374716,
    # 76561197970127197,
    # 76561197970257188,
    # 76561198026921217,
    # 76561198027904347,
    # 76561198062469228,
    # 76561198026278913,
    # 76561197970548935,
    # 76561197966617426,
    # 76561198356842617,
    # 76561198034276722,
    # 76561198355953202,
    # 76561197986603983,
    # 76561197967923946,
    # 76561197961542845,
    # 76561198121938079,
    # 76561197992357639,
    # 76561198002536379,
    # 76561198017054389,
    # 76561198031129658,
    # 76561198020728639,
]))

# extra features/options to disable
EXTRA_FEATURES_DISABLE = {
    'configs.main.ini': {
        'main::connectivity': {
            'disable_networking': (1, 'disable all steam networking interface functionality'),
            'disable_source_query': (1, 'do not send server details to the server browser, only works for game servers'),
            'disable_sharing_stats_with_gameserver': (1, 'prevent sharing stats and achievements with any game server, this also disables the interface ISteamGameServerStats'),
        },
    },
}

# extra convenient features/options to enable
EXTRA_FEATURES_CONVENIENT = {
    'configs.main.ini': {
        'main::general': {
            'new_app_ticket': (1, 'generate new app auth ticket'),
            'gc_token': (1, 'generate/embed GC token inside new App Ticket'),
            'enable_account_avatar': (1, 'enable avatar functionality'),
        },
        'main::connectivity': {
            'disable_lan_only': (1, 'prevent hooking OS networking APIs and allow any external requests'),
            'share_leaderboards_over_network': (1, 'enable sharing leaderboards scores with people playing the same game on the same network'),
            'download_steamhttp_requests': (1, 'attempt to download external HTTP(S) requests made via Steam_HTTP::SendHTTPRequest()'),
        },
    },
    'configs.overlay.ini': {
        'overlay::general': {
            'enable_experimental_overlay': (1, 'XXX USE AT YOUR OWN RISK XXX, enable the experimental overlay, might cause crashes or other problems'),
            'disable_warning_any': (1, 'disable any warning in the overlay'),
        },
    }
}


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
    print(f"downloading achievements images inside '{output_folder }', images count = {len(image_names)}")
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
                    print("HTTPError downloading", url, file=sys.stderr)
                    traceback.print_exception(e, file=sys.stderr)
            if not succeeded:
                print("error could not download", name)
            
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
    print("finished downloading achievements images")

def generate_achievement_stats(client, game_id : int, output_directory, backup_directory) -> list[dict]:
    stats_schema_found = None
    print(f"finding achievements stats...")
    for id in TOP_OWNER_IDS:
        #print(f"finding achievements stats using account ID {id}...")
        out = get_stats_schema(client, game_id, id)
        if out is not None and len(out.body.schema) > 0:
            stats_schema_found = out
            #print(f"found achievement stats using account ID {id}")
            break

    if stats_schema_found is None: # nothing found
        print(f"[X] app id {game_id} has not achievements")
        return []

    achievement_images_dir = os.path.join(output_directory, "img")
    images_to_download : set[str] = set()
    
    with open(os.path.join(backup_directory, f'UserGameStatsSchema_{game_id}.bin'), 'wb') as f:
        f.write(stats_schema_found.body.schema)
    (
        achievements, stats,
        copy_default_unlocked_img, copy_default_locked_img
    ) = achievements_gen.generate_stats_achievements(stats_schema_found.body.schema, output_directory)
    
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
        print("failed getting published file", published_file_id)
        return None

    file_details = ugc_info.body.publishedfiledetails[0]
    if (file_details.result != EResult.OK):
        print("failed getting published file", published_file_id, file_details.result)
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
            print(f"Error downloading from '{file_details.file_url}'", file=sys.stderr)
            traceback.print_exception(e, file=sys.stderr)
            return None
    else:
        print("Could not download file", published_file_id, "no url (you can ignore this if the game doesn't need a controller config)")
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
def get_depots_infos(raw_infos):
    try:
        dlc_list = set()
        depot_app_list = set()
        all_depots = set()
        all_branches = []
        try:
            dlc_list = set(map(lambda a: int(f"{a}".strip()), raw_infos["extended"]["listofdlc"].split(",")))
        except Exception:
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
        
        return (dlc_list, depot_app_list, all_depots, all_branches)
    except Exception:
        print("could not get dlc infos, are there any dlcs ?")
        return (set(), set(), set())


def help():
    exe_name = os.path.basename(sys.argv[0])
    print(f"\nUsage: {exe_name} [Switches] appid appid appid ... ")
    print(f" Example: {exe_name} 421050 420 480")
    print(f" Example: {exe_name} -shots -thumbs -vid -imgs -name -cdx -aw -clean -de 421050 480")
    print(f" Example: {exe_name} -shots -thumbs -vid -imgs -name -cdx -aw -clean -de -cve 421050")
    print(f" Example: {exe_name} -shots -thumbs -vid -imgs -name -cdx -aw -clean -cve 421050")
    print("\nSwitches:")
    print(" -shots:    download screenshots for each app if they're available")
    print(" -thumbs:   download screenshots thumbnails for each app if they're available")
    print(" -vid:      download the first video available for each app: trailer, gameplay, announcement, etc...")
    print(" -imgs:     download common images for each app: Steam generated background, icon, logo, etc...")
    print(" -name:     save the output of each app in a folder with the same name as the app, unsafe characters are discarded")
    print(" -cdx:      generate .ini file for CODEX Steam emu for each app")
    print(" -aw:       generate schemas of all possible languages for Achievement Watcher")
    print(" -clean:    delete any folder/file with the same name as the output before generating any data")
    print(" -anon:     login as an anonymous account, these have very limited access and cannot get all app details")
    print(" -token:    save refresh_token to disk, the logged-on account will be saved")
    print(" -de:       disable some extra features by generating the corresponding config files in steam_settings folder")
    print(" -cve:      enable some convenient extra features by generating the corresponding config files in steam_settings folder")
    print(" -reldir:   generate temp files/folders, and expect input files, relative to the current working directory")
    print(" -skip_ach: skip downloading & generating achievements and their images")
    print(" -skip_con: skip downloading & generating controller configuration files")
    print(" -skip_inv: skip downloading & generating inventory data (items.json & default_items.json)")
    print("\nAll switches are optional except app id, at least 1 app id must be provided")
    print("\nAutomate the login prompt:")
    print(" * You can create a file called 'my_login.txt' beside the script, then add your username on the first line")
    print("   and your password on the second line.")
    print(" * You can set these 2 environment variables (will override 'my_login.txt'):")
    print("   GSE_CFG_USERNAME")
    print("   GSE_CFG_PASSWORD")
    print("")


def merge_dict(dest: dict, src: dict):
    # merge similar keys, but don't overwrite values
    for kv in src.items():
        v_dest = dest.get(kv[0], None)
        if isinstance(kv[1], dict) and isinstance(v_dest, dict):
            merge_dict(v_dest, kv[1])
        elif kv[0] not in dest:
            dest[kv[0]] = kv[1]

def write_ini_file(base_path: str, out_ini: dict):
    for file in out_ini.items():
        with open(os.path.join(base_path, file[0]), 'wt', encoding='utf-8') as f:
            for item in file[1].items():
                f.write('[' + str(item[0]) + ']\n') # section
                for kv in item[1].items():
                    if kv[1][1]: # comment
                        f.write('# ' + str(kv[1][1]) + '\n')
                    f.write(str(kv[0]) + '=' + str(kv[1][0]) + '\n') # key/value pair
                f.write('\n')

def main():
    USERNAME = ""
    PASSWORD = ""

    DISABLE_EXTRA = False
    CONVENIENT_EXTRA = False
    DOWNLOAD_SCREESHOTS = False
    DOWNLOAD_THUMBNAILS = False
    DOWNLOAD_VIDEOS = False
    DOWNLOAD_COMMON_IMAGES = False
    SAVE_APP_NAME = False
    GENERATE_CODEX_INI = False
    GENERATE_ACHIEVEMENT_WATCHER_SCHEMAS = False
    CLEANUP_BEFORE_GENERATING = False
    ANON_LOGIN = False
    SAVE_REFRESH_TOKEN = False
    RELATIVE_DIR = False
    SKIP_ACH = False
    SKIP_CONTROLLER = False
    SKIP_INVENTORY = False
    
    prompt_for_unavailable = True

    if len(sys.argv) < 2:
        help()
        sys.exit(1)

    appids : set[int] = set()
    for appid in sys.argv[1:]:
        if f'{appid}'.isnumeric():
            appids.add(int(appid))
        elif f'{appid}'.lower() == '-shots':
            DOWNLOAD_SCREESHOTS = True
        elif f'{appid}'.lower() == '-thumbs':
            DOWNLOAD_THUMBNAILS = True
        elif f'{appid}'.lower() == '-vid':
            DOWNLOAD_VIDEOS = True
        elif f'{appid}'.lower() == '-imgs':
            DOWNLOAD_COMMON_IMAGES = True
        elif f'{appid}'.lower() == '-name':
            SAVE_APP_NAME = True
        elif f'{appid}'.lower() == '-cdx':
            GENERATE_CODEX_INI = True
        elif f'{appid}'.lower() == '-aw':
            GENERATE_ACHIEVEMENT_WATCHER_SCHEMAS = True
        elif f'{appid}'.lower() == '-clean':
            CLEANUP_BEFORE_GENERATING = True
        elif f'{appid}'.lower() == '-anon':
            ANON_LOGIN = True
        elif f'{appid}'.lower() == '-token':
            SAVE_REFRESH_TOKEN = True
        elif f'{appid}'.lower() == '-de':
            DISABLE_EXTRA = True
        elif f'{appid}'.lower() == '-cve':
            CONVENIENT_EXTRA = True
        elif f'{appid}'.lower() == '-reldir':
            RELATIVE_DIR = True
        elif f'{appid}'.lower() == '-skip_ach':
            SKIP_ACH = True
        elif f'{appid}'.lower() == '-skip_con':
            SKIP_CONTROLLER = True
        elif f'{appid}'.lower() == '-skip_inv':
            SKIP_INVENTORY = True
        else:
            print(f'[X] invalid switch: {appid}')
            help()
            sys.exit(1)
    
    if not appids:
        print(f'[X] no app id was provided')
        help()
        sys.exit(1)

    client = SteamClient()
    if ANON_LOGIN:
        result = client.anonymous_login()
        trials = 5
        while result != EResult.OK and trials > 0:
            time.sleep(1000)
            result = client.anonymous_login()
            trials -= 1
    else:
        # first read the 'my_login.txt' file
        my_login_file = os.path.join(get_exe_dir(RELATIVE_DIR), "my_login.txt")
        if not ANON_LOGIN and os.path.isfile(my_login_file):
            filedata = ['']
            with open(my_login_file, "r", encoding="utf-8") as f:
                filedata = f.readlines()
            filedata = list(map(lambda s: s.replace("\r", "").replace("\n", ""), filedata))
            filedata = [l for l in filedata if l]
            if len(filedata) == 1:
                USERNAME = filedata[0]
            elif len(filedata) == 2:
                USERNAME, PASSWORD = filedata[0], filedata[1]
        
        # then allow the env vars to override the login details
        env_username = os.environ.get('GSE_CFG_USERNAME', None)
        env_password = os.environ.get('GSE_CFG_PASSWORD', None)
        if env_username:
            USERNAME = env_username
        if env_password:
            PASSWORD = env_password

        # the file to save/load credentials
        REFRESH_TOKENS = os.path.join(get_exe_dir(RELATIVE_DIR), "refresh_tokens.json")
        refresh_tokens = {}
        if os.path.isfile(REFRESH_TOKENS):
            with open(REFRESH_TOKENS) as f:
                try:
                    lf = json.load(f)
                    refresh_tokens = lf if isinstance(lf, dict) else {}
                except:
                    pass

        # select username from credentials if not already persent
        if not USERNAME:
            users = {i: user for i, user in enumerate(refresh_tokens, 1)}
            if len(users) != 0:
                for i, user in users.items():
                    print(f"{i}: {user}")
                while True:
                    try:
                        num=int(input("Choose an account to login (0 for add account): "))
                    except ValueError:
                        print('Please type a number'); continue
                    break
                USERNAME = users.get(num)

        # still no username? ask user
        if not USERNAME:
            USERNAME = input("Steam user: ")

        REFRESH_TOKEN = refresh_tokens.get(USERNAME)

        webauth, result = WebAuth(), None
        while result in (
            EResult.TryAnotherCM, EResult.ServiceUnavailable,
            EResult.InvalidPassword, None):

            if result in (EResult.TryAnotherCM, EResult.ServiceUnavailable):
                if prompt_for_unavailable and result == EResult.ServiceUnavailable:
                    while True:
                        answer = input("Steam is down. Keep retrying? [y/n]: ").lower()
                        if answer in 'yn': break

                    prompt_for_unavailable = False
                    if answer == 'n': break

                client.reconnect(maxdelay=15)
            elif result == EResult.InvalidPassword:
                print("invalid password or refresh_token,")
                print(f"correct the password or/and delete '{REFRESH_TOKENS}' and try again.")
                exit(1)

            if not REFRESH_TOKEN:
                try:
                    webauth.cli_login(USERNAME, PASSWORD)
                except Exception as e:
                    print(f'Unknown exception: {e}, maybe some account info is wrong?')
                    exit(1)
                USERNAME, PASSWORD = webauth.username, webauth.password
                REFRESH_TOKEN = webauth.refresh_token

            result = client.login(USERNAME, PASSWORD, REFRESH_TOKEN)

        if SAVE_REFRESH_TOKEN:
            with open(REFRESH_TOKENS, 'w') as f:
                refresh_tokens.update({USERNAME: REFRESH_TOKEN})
                json.dump(refresh_tokens, f, indent=4)

    # read and prepend top_owners_ids.txt
    top_owners_file = os.path.join(get_exe_dir(RELATIVE_DIR), "top_owners_ids.txt")
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

    for appid in appids:
        out_config_app_ini = {}

        print(f"********* generating info for app id {appid} *********")
        raw = client.get_product_info(apps=[appid])
        game_info : dict = raw["apps"][appid]

        game_info_common : dict = game_info.get("common", {})
        app_name = game_info_common.get("name", "")
        app_name_on_disk = f"{appid}"
        if app_name:
            print(f"App name on store: '{app_name}'")
            if SAVE_APP_NAME:
                sanitized_name = safe_name.create_safe_name(app_name)
                if sanitized_name:
                    app_name_on_disk = f'{sanitized_name}-{appid}'
        else:
            app_name = f"Unknown_Steam_app_{appid}" # we need this for later use in the Achievement Watcher
            print(f"[X] Couldn't find app name on store")

        root_backup_dir = os.path.join(get_exe_dir(RELATIVE_DIR), "backup")
        backup_dir = os.path.join(root_backup_dir, f"{appid}")
        if not os.path.exists(backup_dir):
            os.makedirs(backup_dir)

        root_out_dir = "output"
        base_out_dir = os.path.join(root_out_dir, app_name_on_disk)
        emu_settings_dir = os.path.join(base_out_dir, "steam_settings")
        info_out_dir = os.path.join(base_out_dir, "info")

        if CLEANUP_BEFORE_GENERATING:
            print("cleaning output folder before generating any data")
            base_dir_path = pathlib.Path(base_out_dir)
            if base_dir_path.is_file():
                base_dir_path.unlink()
                time.sleep(0.05)
            elif base_dir_path.is_dir():
                shutil.rmtree(base_dir_path)
                time.sleep(0.05)
            
            while base_dir_path.exists():
                time.sleep(0.05)

        if not os.path.exists(emu_settings_dir):
            os.makedirs(emu_settings_dir)

        if not os.path.exists(info_out_dir):
            os.makedirs(info_out_dir)

        print(f"output dir: '{base_out_dir}'")

        with open(os.path.join(info_out_dir, "product_info.json"), "wt", encoding='utf-8') as f:
            json.dump(game_info, f, ensure_ascii=False, indent=2)
        
        app_details.download_app_details(
            base_out_dir, info_out_dir,
            appid,
            DOWNLOAD_SCREESHOTS,
            DOWNLOAD_THUMBNAILS,
            DOWNLOAD_VIDEOS)

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
            
            #print(f"generating achievement stats")
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
        
        with open(os.path.join(emu_settings_dir, "steam_appid.txt"), 'w') as f:
            f.write(str(appid))

        dlc_config_list : list[tuple[int, str]] = []
        dlc_list, depot_app_list, all_depots, all_branches = get_depots_infos(game_info)
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

        # we set unlock_all=0 nonetheless, to make the emu lock DLCs, otherwise everything is allowed
        # some games use that as a detection mechanism
        merge_dict(out_config_app_ini, {
            'configs.app.ini': {
                'app::dlcs': {
                    'unlock_all': (0, 'should the emu report all DLCs as unlocked, default=1'),
                }
            }
        })
        for x in dlc_config_list:
            merge_dict(out_config_app_ini, {
                'configs.app.ini': {
                    'app::dlcs': {
                        x[0]: (x[1], ''),
                    }
                }
            })
        # write the data as soon as possible in case a later step caused an exception
        write_ini_file(emu_settings_dir, out_config_app_ini)

        if all_depots:
            with open(os.path.join(emu_settings_dir, "depots.txt"), 'wt', encoding="utf-8") as f:
                for game_depot in all_depots:
                    f.write(f"{game_depot}\n")
        
        if all_branches:
            with open(os.path.join(emu_settings_dir, "branches.json"), "wt", encoding='utf-8') as f:
                json.dump(all_branches, f, ensure_ascii=False, indent=2)


        config_generated = False
        if "config" in game_info:
            if not SKIP_CONTROLLER and "steamcontrollerconfigdetails" in game_info["config"]:
                controller_details = game_info["config"]["steamcontrollerconfigdetails"]
                print('downloading controller vdf files')
                for id in controller_details:
                    details = controller_details[id]
                    controller_type = ""
                    enabled_branches = ""
                    if "controller_type" in details:
                        controller_type = details["controller_type"]
                    if "enabled_branches" in details:
                        enabled_branches = details["enabled_branches"]
                    print(f'downloading controller data, file id = {id}, controller type = {controller_type}')

                    out_vdf = download_published_file(client, int(id), os.path.join(backup_dir, f'{controller_type}-{str(id)}'))
                    if out_vdf is not None and not config_generated:
                        if (controller_type in ["controller_xbox360", "controller_xboxone", "controller_steamcontroller_gordon"] and (("default" in enabled_branches) or ("public" in enabled_branches))):
                            print(f'controller type is supported')
                            parse_controller_vdf.generate_controller_config(out_vdf.decode('utf-8'), os.path.join(emu_settings_dir, "controller"))
                            config_generated = True
            if not SKIP_CONTROLLER and "steamcontrollertouchconfigdetails" in game_info["config"]:
                controller_details = game_info["config"]["steamcontrollertouchconfigdetails"]
                for id in controller_details:
                    details = controller_details[id]
                    controller_type = ""
                    enabled_branches = ""
                    if "controller_type" in details:
                        controller_type = details["controller_type"]
                    if "enabled_branches" in details:
                        enabled_branches = details["enabled_branches"]
                    print(id, controller_type)
                    out_vdf = download_published_file(client, int(id), os.path.join(backup_dir, controller_type + str(id)))
            if "launch" in game_info["config"]:
                launch_configs = game_info["config"]["launch"]
                with open(os.path.join(info_out_dir, "launch_config.json"), "wt", encoding='utf-8') as f:
                    json.dump(launch_configs, f, ensure_ascii=False, indent=2)
                
                first_app_exe : str = None
                prefered_app_exe : str = None
                unwanted_app_exes = ["launch", "start", "play", "try", "demo", "_vr",]
                for cfg in launch_configs.values():
                    if "executable" in cfg:
                        app_exe = f'{cfg["executable"]}'
                    
                    if app_exe.lower().endswith(".exe"):
                        app_exe = app_exe.replace("\\", "/").split('/')[-1]
                        if first_app_exe is None:
                            first_app_exe = app_exe
                        if all(app_exe.lower().find(unwanted_exe) < 0 for unwanted_exe in unwanted_app_exes):
                            prefered_app_exe = app_exe
                            break
                
                if prefered_app_exe:
                    app_exe = prefered_app_exe
                elif first_app_exe:
                    app_exe = first_app_exe

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
                dlc_config_list,
                achievements)

        if DOWNLOAD_COMMON_IMAGES:
            app_images.download_app_images(
                base_out_dir,
                appid,
                clienticon,
                icon,
                logo,
                logo_small)
        
        if DISABLE_EXTRA:
            merge_dict(out_config_app_ini, EXTRA_FEATURES_DISABLE)
 
        if CONVENIENT_EXTRA:
            merge_dict(out_config_app_ini, EXTRA_FEATURES_CONVENIENT)
        
        if out_config_app_ini:
            write_ini_file(emu_settings_dir, out_config_app_ini)

        inventory_data = None
        if not SKIP_INVENTORY:
            inventory_data = generate_inventory(client, appid)
        if inventory_data is not None:
            out_inventory = {}
            default_items = {}
            inventory = json.loads(inventory_data.rstrip(b"\x00"))
            raw_inventory = json.dumps(inventory, indent=4)
            with open(os.path.join(backup_dir, "inventory.json"), "w") as f:
                f.write(raw_inventory)
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
                default_items[index] = {"quantity": 1}

            with open(os.path.join(emu_settings_dir, "items.json"), "wt", encoding='utf-8') as f:
                json.dump(out_inventory, f, ensure_ascii=False, indent=2)

            with open(os.path.join(emu_settings_dir, "default_items.json"), "wt", encoding='utf-8') as f:
                json.dump(default_items, f, ensure_ascii=False, indent=2)

        with open(os.path.join(backup_dir, "product_info.json"), "wt", encoding='utf-8') as f:
            json.dump(game_info, f, ensure_ascii=False, indent=2)
        
        with open(os.path.join(backup_dir, "dlc_product_info.json"), "wt", encoding='utf-8') as f:
            json.dump(dlc_raw, f, ensure_ascii=False, indent=2)
        
        print(f"######### done for app id {appid} #########\n\n")

if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print("Unexpected error:")
        print(e)
        print("-----------------------")
        for line in traceback.format_exception(e):
            print(line)
        print("-----------------------")
        sys.exit(1)

