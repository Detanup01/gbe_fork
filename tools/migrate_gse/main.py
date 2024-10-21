import json
import platform
import os
import sys
import glob
import configparser
import time
import traceback
import shutil
from configobj import ConfigObj

def help():
    exe_name = os.path.basename(sys.argv[0])
    print(f"\nUsage: {exe_name} [Switches] [settings folder path]")
    print(f"\nSwitches:")
    print(f" -revert: convert all .ini files back to .txt files")
    print(f" /?, -?, --?, /h, -h, --h, /help, -help, --help")
    print(f"    show this help page")
    print(f"\nExamples:")
    print(f" Example: {exe_name}")
    print(f" Example: {exe_name} -revert")
    print(f' Example: {exe_name} "D:\\game\\steam_settings"')
    print(f' Example: {exe_name} -revert "D:\\game\\steam_settings"')
    print("\nNote:")
    print(" Running the tool without any switches will make it attempt to read the global settings folder")
    print("")

NEW_STEAM_SETTINGS_FOLDER = os.path.join('_OUTPUT', 'steam_settings')

def create_new_steam_settings_folder():
    if not os.path.exists(NEW_STEAM_SETTINGS_FOLDER):
        os.makedirs(NEW_STEAM_SETTINGS_FOLDER)

# use CongigObj to correctly update existing 'configs.app.ini' copied from ./_DEFAULT configuration --- START, read ini
configs_app = ConfigObj(os.path.join(NEW_STEAM_SETTINGS_FOLDER, "configs.app.ini"), encoding='utf-8')
configs_app_initial = configs_app

# use CongigObj to correctly update existing 'configs.main.ini' copied from ./_DEFAULT configuration --- START, read ini
configs_main = ConfigObj(os.path.join(NEW_STEAM_SETTINGS_FOLDER, "configs.main.ini"), encoding='utf-8')
configs_main_initial = configs_main

# use CongigObj to correctly update existing 'configs.overlay.ini' copied from ./_DEFAULT configuration --- START, read ini
configs_overlay = ConfigObj(os.path.join(NEW_STEAM_SETTINGS_FOLDER, "configs.overlay.ini"), encoding='utf-8')
configs_overlay_initial = configs_overlay

# use CongigObj to correctly update existing 'configs.user.ini' copied from ./_DEFAULT configuration --- START, read ini
configs_user = ConfigObj(os.path.join(NEW_STEAM_SETTINGS_FOLDER, "configs.user.ini"), encoding='utf-8')
configs_user_initial = configs_user

def convert_to_ini(global_settings: str):
    # oh no, they're too many! --- they are indeed... it is way simpler to use ConfigObj to properly update the default ini files
    for file in glob.glob('*.*', root_dir=global_settings):
        file = file.lower()
        if file == 'steam_appid.txt':
            steam_appid = fr.readline().strip('\n').strip('\r')
        elif file == 'force_account_name.txt' or file == 'account_name.txt':
            with open(os.path.join(global_settings, file), "r", encoding='utf-8') as fr:
                configs_user["user::general"]["account_name"] = fr.readline().strip('\n').strip('\r') #updated ini through ConfigObj
        elif file == 'force_branch_name.txt':
            with open(os.path.join(global_settings, file), "r", encoding='utf-8') as fr:
                configs_app["app::general"]["branch_name"] = fr.readline().strip('\n').strip('\r') #updated ini through ConfigObj
        elif file == 'force_language.txt' or file == 'language.txt':
            with open(os.path.join(global_settings, file), "r", encoding='utf-8') as fr:
                configs_user["user::general"]["language"] = fr.readline().strip('\n').strip('\r') #updated ini through ConfigObj
        elif file == 'force_listen_port.txt' or file == 'listen_port.txt':
            with open(os.path.join(global_settings, file), "r", encoding='utf-8') as fr:
                configs_main["main::connectivity"]["listen_port"] = fr.readline().strip('\n').strip('\r') #updated ini through ConfigObj
        elif file == 'force_steamid.txt' or file == 'user_steam_id.txt':
            with open(os.path.join(global_settings, file), "r", encoding='utf-8') as fr:
                configs_user["user::general"]["account_steamid"] = fr.readline().strip('\n').strip('\r') #updated ini through ConfigObj
        elif file == 'ip_country.txt':
            with open(os.path.join(global_settings, file), "r", encoding='utf-8') as fr:
                configs_user["user::general"]["ip_country"] = fr.readline().strip('\n').strip('\r') #updated ini through ConfigObj
        elif file == 'overlay_appearance.txt':
            with open(os.path.join(global_settings, file), "r", encoding='utf-8') as fr:
                ov_lines = [lll.strip() for lll in fr.readlines() if lll.strip() and lll.strip()[0] != ';' and lll.strip()[0] != '#']
                for ovl in ov_lines:
                    [ov_name, ov_val] = ovl.split(' ', 1)
                    configs_overlay["overlay::appearance"][ov_name.strip()] = ov_val.strip() #updated ini through ConfigObj
        elif file == 'build_id.txt':
            with open(os.path.join(global_settings, file), "r", encoding='utf-8') as fr:
                create_new_steam_settings_folder()
                with open(os.path.join(NEW_STEAM_SETTINGS_FOLDER, 'branches.json'), 'wt', encoding='utf-8') as fout:
                    json.dump([{
                        "name": "public",
                        "description": "",
                        "protected": False,
                        "build_id": int(fr.readline().strip()),
                        "time_updated": int(time.time())
                    }], fout, indent=2)
        elif file == 'disable_account_avatar.txt':
            configs_main["main::general"]["enable_account_avatar"] = 0 #updated ini through ConfigObj
        elif file == 'disable_networking.txt':
            configs_main["main::connectivity"]["disable_networking"] = 1 #updated ini through ConfigObj
        elif file == 'disable_sharing_stats_with_gameserver.txt':
            configs_main["main::connectivity"]["disable_sharing_stats_with_gameserver"] = 1 #updated ini through ConfigObj
        elif file == 'disable_source_query.txt':
            configs_main["main::connectivity"]["disable_source_query"] = 1 #updated ini through ConfigObj
        elif file == 'overlay_hook_delay_sec.txt':
            with open(os.path.join(global_settings, file), "r", encoding='utf-8') as fr:
                configs_overlay["overlay::general"]["hook_delay_sec"] = fr.readline().strip('\n').strip('\r') #updated ini through ConfigObj
        elif file == 'overlay_renderer_detector_timeout_sec.txt':
            with open(os.path.join(global_settings, file), "r", encoding='utf-8') as fr:
                configs_overlay["overlay::general"]["renderer_detector_timeout_sec"] = fr.readline().strip('\n').strip('\r') #updated ini through ConfigObj
        elif file == 'enable_experimental_overlay.txt' or file == 'disable_overlay.txt':
            enable_ovl = 0
            if file == 'enable_experimental_overlay.txt':
                enable_ovl = 1
            configs_overlay["overlay::general"]["enable_experimental_overlay"] = enable_ovl #updated ini through ConfigObj
        elif file == 'app_paths.txt':
            with open(os.path.join(global_settings, file), "r", encoding='utf-8') as fr:
                app_lines = [lll.strip('\n').strip('\r') for lll in fr.readlines() if lll.strip() and lll.strip()[0] != '#']
                for app_lll in app_lines:
                    [appid, apppath] = app_lll.split('=', 1)
                    configs_app["app::paths"][appid.strip()] = apppath #updated ini through ConfigObj
        elif file == 'dlc.txt':
            with open(os.path.join(global_settings, file), "r", encoding='utf-8') as fr:
                dlc_lines = [lll.strip('\n').strip('\r') for lll in fr.readlines() if lll.strip() and lll.strip()[0] != '#']
                configs_app["app::dlcs"]["unlock_all"] = 0 #updated ini through ConfigObj
                for dlc_lll in dlc_lines:
                    [dlcid, dlcname] = dlc_lll.split('=', 1)
                    configs_app["app::dlcs"][dlcid.strip()] = dlcname #updated ini through ConfigObj
        elif file == 'achievements_bypass.txt':
            configs_main["main::misc"]["achievements_bypass"] = 1 #updated ini through ConfigObj
        elif file == 'crash_printer_location.txt':
            with open(os.path.join(global_settings, file), "r", encoding='utf-8') as fr:
                configs_main["main::general"]["crash_printer_location"] = fr.readline().strip('\n').strip('\r') #updated ini through ConfigObj
        elif file == 'disable_lan_only.txt':
            configs_main["main::connectivity"]["disable_lan_only"] = 1 #updated ini through ConfigObj
        elif file == 'disable_leaderboards_create_unknown.txt':
            configs_main["main::general"]["disable_leaderboards_create_unknown"] = 1 #updated ini through ConfigObj
        elif file == 'disable_lobby_creation.txt':
            configs_main["main::connectivity"]["disable_lobby_creation"] = 1 #updated ini through ConfigObj
        elif file == 'disable_overlay_achievement_notification.txt':
            configs_overlay["overlay::general"]["disable_achievement_notification"] = 1 #updated ini through ConfigObj
        elif file == 'disable_overlay_friend_notification.txt':
            configs_overlay["overlay::general"]["disable_friend_notification"] = 1 #updated ini through ConfigObj
        elif file == 'disable_overlay_warning_any.txt':
            configs_overlay["overlay::general"]["disable_warning_any"] = 1 #updated ini through ConfigObj
        elif file == 'disable_overlay_warning_bad_appid.txt':
            configs_overlay["overlay::general"]["disable_warning_bad_appid"] = 1 #updated ini through ConfigObj
        elif file == 'disable_overlay_warning_local_save.txt':
            configs_overlay["overlay::general"]["disable_warning_local_save"] = 1 #updated ini through ConfigObj
        elif file == 'download_steamhttp_requests.txt':
            configs_main["main::connectivity"]["download_steamhttp_requests"] = 1 #updated ini through ConfigObj
        elif file == 'force_steamhttp_success.txt':
            configs_main["main::misc"]["force_steamhttp_success"] = 1 #updated ini through ConfigObj
        elif file == 'new_app_ticket.txt':
            configs_main["main::general"]["new_app_ticket"] = 1 #updated ini through ConfigObj
        elif file == 'gc_token.txt':
            configs_main["main::general"]["gc_token"] = 1 #updated ini through ConfigObj
        elif file == 'immediate_gameserver_stats.txt':
            configs_main["main::general"]["immediate_gameserver_stats"] = 1 #updated ini through ConfigObj
        elif file == 'is_beta_branch.txt':
            configs_app["app::general"]["is_beta_branch"] = 1 #updated ini through ConfigObj
        elif file == 'matchmaking_server_details_via_source_query.txt':
            configs_main["main::general"]["matchmaking_server_details_via_source_query"] = 1 #updated ini through ConfigObj
        elif file == 'matchmaking_server_list_actual_type.txt':
            configs_main["main::general"]["matchmaking_server_list_actual_type"] = 1 #updated ini through ConfigObj
        elif file == 'offline.txt':
            configs_main["main::connectivity"]["offline"] = 1 #updated ini through ConfigObj
        elif file == 'share_leaderboards_over_network.txt':
            configs_main["main::connectivity"]["share_leaderboards_over_network"] = 1 #updated ini through ConfigObj
        elif file == 'steam_deck.txt':
            configs_main["main::general"]["steam_deck"] = 1 #updated ini through ConfigObj
         
def write_txt_file(filename: str, dict_ini: dict, section: str, key: str):
    val = dict_ini.get(section, {}).get(key, None)
    if val is None:
        return False
    
    create_new_steam_settings_folder()
    
    with open(os.path.join(NEW_STEAM_SETTINGS_FOLDER, filename), "wt", encoding='utf-8') as fw:
        fw.write(str(val))

    return True

def write_txt_file_bool(filename: str, dict_ini: dict, section: str, key: str, write_if: bool):
    val = dict_ini.get(section, {}).get(key, None)
    if val is None:
        return False
    val = str(val).lower()[0]
    bool_val = val == '1' or val == "t" or val == "y"
    if bool_val != write_if:
        return False
    
    create_new_steam_settings_folder()
    
    with open(os.path.join(NEW_STEAM_SETTINGS_FOLDER, filename), "wt", encoding='utf-8') as fw:
        fw.write(f'{key}={val}')
    
    return True

def write_txt_file_multi(filename: str, dict_ini: dict, section: str):
    val = dict_ini.get(section, {})
    if len(val) <= 0:
        return False
    
    create_new_steam_settings_folder()
    
    with open(os.path.join(NEW_STEAM_SETTINGS_FOLDER, filename), "wt", encoding='utf-8') as fw:
        for kv in val.items():
            fw.write(f'{kv[0]}={kv[1]}\n')
    
    return True

def convert_public_branch_to_txt(global_settings: str):
    src_file = os.path.join(global_settings, 'branches.json')
    if not os.path.isfile(src_file):
        return False
    build_id = -1
    with open(src_file, "r", encoding='utf-8') as fr:
        branches: list[dict] = json.load(fr)
        for branch in branches:
            if f'{branch.get("name", "")}'.lower() == "public":
                build_id = branch.get('build_id', -1)
                break
        if build_id == -1:
            return False
    create_new_steam_settings_folder()
    with open(os.path.join(NEW_STEAM_SETTINGS_FOLDER, 'build_id.txt'), "wt", encoding='utf-8') as fw:
        fw.write(f"{build_id}")
    return True

def convert_to_txt(global_settings: str):
    # oh no, they're too many! --- they are indeed... it seems ConfigParser does the job here
    config = configparser.ConfigParser(strict=False, empty_lines_in_values=False)
    for file in glob.glob('*.ini*', root_dir=global_settings):
        config.read(os.path.join(global_settings, file), encoding='utf-8')

    dict_ini = dict(config)
    if 'DEFAULT' in dict_ini: # remove the "magic" default section
        del dict_ini['DEFAULT']

    done = 0
    done += write_txt_file_bool('achievements_bypass.txt', dict_ini, 'main::misc', 'achievements_bypass', True)
    done += write_txt_file_multi('app_paths.txt', dict_ini, 'app::paths')
    done += convert_public_branch_to_txt(global_settings)
    done += write_txt_file('crash_printer_location.txt', dict_ini, 'main::general', 'crash_printer_location')
    done += write_txt_file_bool('disable_account_avatar.txt', dict_ini, 'main::general', 'enable_account_avatar', False)
    done += write_txt_file_bool('disable_lan_only.txt', dict_ini, 'main::connectivity', 'disable_lan_only',True)
    done += write_txt_file_bool('disable_leaderboards_create_unknown.txt', dict_ini, 'main::general', 'disable_leaderboards_create_unknown', True)
    done += write_txt_file_bool('disable_lobby_creation.txt', dict_ini, 'main::connectivity', 'disable_lobby_creation', True)
    done += write_txt_file_bool('disable_networking.txt', dict_ini, 'main::connectivity', 'disable_networking', True)
    done += write_txt_file_bool('disable_overlay_achievement_notification.txt', dict_ini, 'overlay::general', 'disable_achievement_notification', True)
    done += write_txt_file_bool('disable_overlay_friend_notification.txt', dict_ini, 'overlay::general', 'disable_friend_notification', True)
    done += write_txt_file_bool('disable_overlay_warning_any.txt', dict_ini, 'overlay::general', 'disable_warning_any', True)
    done += write_txt_file_bool('disable_overlay_warning_bad_appid.txt', dict_ini, 'overlay::general', 'disable_warning_bad_appid', True)
    done += write_txt_file_bool('disable_overlay_warning_local_save.txt', dict_ini, 'overlay::general', 'disable_warning_local_save', True)
    done += write_txt_file_bool('disable_sharing_stats_with_gameserver.txt', dict_ini, 'main::connectivity', 'disable_sharing_stats_with_gameserver', True)
    done += write_txt_file_bool('disable_source_query.txt', dict_ini, 'main::connectivity', 'disable_source_query', True)
    done += write_txt_file_multi('dlc.txt', dict_ini, 'app::dlcs')
    done += write_txt_file_bool('download_steamhttp_requests.txt', dict_ini, 'main::connectivity', 'download_steamhttp_requests', True)
    done += write_txt_file_bool('disable_overlay.txt', dict_ini, 'overlay::general', 'enable_experimental_overlay', False)
    done += write_txt_file_bool('enable_experimental_overlay.txt', dict_ini, 'overlay::general', 'enable_experimental_overlay', True)
    done += write_txt_file('force_account_name.txt', dict_ini, 'user::general', 'account_name')
    done += write_txt_file('force_branch_name.txt', dict_ini, 'app::general', 'branch_name')
    done += write_txt_file('force_language.txt', dict_ini, 'user::general', 'language')
    done += write_txt_file('force_listen_port.txt', dict_ini, 'main::connectivity', 'listen_port')
    done += write_txt_file_bool('force_steamhttp_success.txt', dict_ini, 'main::misc', 'force_steamhttp_success', True)
    done += write_txt_file('force_steamid.txt', dict_ini, 'user::general', 'account_steamid')
    done += write_txt_file_bool('gc_token.txt', dict_ini, 'main::general', 'gc_token', True)
    done += write_txt_file_bool('immediate_gameserver_stats.txt', dict_ini, 'main::general', 'immediate_gameserver_stats', True)
    done += write_txt_file('ip_country.txt', dict_ini, 'user::general', 'ip_country')
    done += write_txt_file_bool('is_beta_branch.txt', dict_ini, 'app::general', 'is_beta_branch', True)
    done += write_txt_file_bool('matchmaking_server_details_via_source_query.txt', dict_ini, 'main::general', 'matchmaking_server_details_via_source_query', True)
    done += write_txt_file_bool('matchmaking_server_list_actual_type.txt', dict_ini, 'main::general', 'matchmaking_server_list_actual_type', True)
    done += write_txt_file_bool('new_app_ticket.txt', dict_ini, 'main::general', 'new_app_ticket', True)
    done += write_txt_file_bool('offline.txt', dict_ini, 'main::connectivity', 'offline', True)
    done += write_txt_file_multi('overlay_appearance.txt', dict_ini, 'overlay::appearance')
    done += write_txt_file('overlay_hook_delay_sec.txt', dict_ini, 'overlay::general', 'hook_delay_sec')
    done += write_txt_file('overlay_renderer_detector_timeout_sec.txt', dict_ini, 'overlay::general', 'renderer_detector_timeout_sec')
    done += write_txt_file_bool('share_leaderboards_over_network.txt', dict_ini, 'main::connectivity', 'share_leaderboards_over_network', True)
    done += write_txt_file_bool('steam_deck.txt', dict_ini, 'main::general', 'steam_deck', True)
    
    return done


def main():
    is_windows = platform.system().lower() == "windows"
    global_settings = ''

    CONVERT_TO_INI = True
    SHOW_HELP = False

    argc = len(sys.argv)
    for idx in range(1, argc):
        arg = sys.argv[idx]
        if arg.lower() == "-revert":
            CONVERT_TO_INI = False
        elif arg == "/?" or arg == "-?" or arg == "--?" or arg.lower() == "/h" or arg.lower() == "-h" or arg.lower() == "--h" or arg.lower() == "/help" or arg.lower() == "-help" or arg.lower() == "--help":
            SHOW_HELP = True
        elif os.path.isdir(arg):
            global_settings = arg
        else:
            print(f'invalid arg #{idx} "{arg}"', file=sys.stderr)
            help()
            sys.exit(1)
    
    if SHOW_HELP:
        help()
        sys.exit(0)
        
    if not global_settings:
        if is_windows:
            appdata = os.getenv('APPDATA')
            if appdata:
                global_settings = os.path.join(appdata, 'GSE Saves', 'settings')
        else:
            xdg = os.getenv('XDG_DATA_HOME')
            if xdg:
                global_settings = os.path.join(xdg, 'GSE Saves', 'settings')
            
            if not global_settings:
                home_env = os.getenv('HOME')
                if home_env:
                    global_settings = os.path.join(home_env, 'GSE Saves', 'settings')

    if not global_settings or not os.path.isdir(global_settings):
        print('failed to detect folder', file=sys.stderr)
        help()
        sys.exit(1)

    print(f'searching inside the folder: "{global_settings}"')

    if CONVERT_TO_INI:
        shutil.copytree(os.path.join("_DEFAULT", str(0)), "_OUTPUT", dirs_exist_ok=True) # copy from ./_DEFAULT/0 dir
        shutil.copytree(os.path.join("_DEFAULT", str(1)), "_OUTPUT", dirs_exist_ok=True) # copy from ./_DEFAULT/1 dir
        convert_to_ini(global_settings)

        if convert_to_ini(global_settings):
            create_new_steam_settings_folder()
            configs_app.write() # use CongigObj to correctly update existing 'configs.app.ini' copied from ./_DEFAULT configuration --- END, write ini
            # ConfigObj overrides the default ini format, adding spaces before and after '=' and '""' for empty keys, so we'll use this to undo the changes
            with open(os.path.join(NEW_STEAM_SETTINGS_FOLDER, "configs.app.ini"), 'r') as file:
                filedata = file.read()
                filedata = filedata.replace(' = ""', '=')
                filedata = filedata.replace(' = ', '=')
            with open(os.path.join(NEW_STEAM_SETTINGS_FOLDER, "configs.app.ini"), 'w') as file:
                file.write(filedata)
            configs_main.write() # use CongigObj to correctly update existing 'configs.main.ini' copied from ./_DEFAULT configuration --- END, write ini
            # ConfigObj overrides the default ini format, adding spaces before and after '=' and '""' for empty keys, so we'll use this to undo the changes
            with open(os.path.join(NEW_STEAM_SETTINGS_FOLDER, "configs.main.ini"), 'r') as file:
                filedata = file.read()
                filedata = filedata.replace(' = ""', '=')
                filedata = filedata.replace(' = ', '=')
            with open(os.path.join(NEW_STEAM_SETTINGS_FOLDER, "configs.main.ini"), 'w') as file:
                file.write(filedata)
            configs_overlay.write() # use CongigObj to correctly update existing 'configs.overlay.ini' copied from ./_DEFAULT configuration --- END, write ini
            # ConfigObj overrides the default ini format, adding spaces before and after '=' and '""' for empty keys, so we'll use this to undo the changes
            with open(os.path.join(NEW_STEAM_SETTINGS_FOLDER, "configs.overlay.ini"), 'r') as file:
                filedata = file.read()
                filedata = filedata.replace(' = ""', '=')
                filedata = filedata.replace(' = ', '=')
            with open(os.path.join(NEW_STEAM_SETTINGS_FOLDER, "configs.overlay.ini"), 'w') as file:
                file.write(filedata)
            configs_user.write() # use CongigObj to correctly update existing 'configs.user.ini' copied from ./_DEFAULT configuration --- END, write ini
            # ConfigObj overrides the default ini format, adding spaces before and after '=' and '""' for empty keys, so we'll use this to undo the changes
            with open(os.path.join(NEW_STEAM_SETTINGS_FOLDER, "configs.user.ini"), 'r') as file:
                filedata = file.read()
                filedata = filedata.replace(' = ""', '=')
                filedata = filedata.replace(' = ', '=')
            with open(os.path.join(NEW_STEAM_SETTINGS_FOLDER, "configs.user.ini"), 'w') as file:
                file.write(filedata)
            print(f'new settings written inside: "{os.path.join(os.path.curdir, NEW_STEAM_SETTINGS_FOLDER)}"')
        else:
            print('nothing found!', file=sys.stderr)
            sys.exit(1)
    else:
        if convert_to_txt(global_settings):
            print(f'new settings written inside: "{os.path.join(os.path.curdir, NEW_STEAM_SETTINGS_FOLDER)}"')
        else:
            print('nothing found!', file=sys.stderr)
            sys.exit(1)


if __name__ == "__main__":
    try:
        main()
        sys.exit(0)
    except Exception as e:
        print("Unexpected error:")
        print(e)
        print("-----------------------")
        for line in traceback.format_exception(e):
            print(line)
        print("-----------------------")
        sys.exit(1)
