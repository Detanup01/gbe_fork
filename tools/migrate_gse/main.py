import platform
import os
import sys
import glob
import re
import traceback


def help():
    exe_name = os.path.basename(sys.argv[0])
    print(f"\nUsage: {exe_name} [settings path]")
    print(f" Example: {exe_name}")
    print(f" Example: {exe_name} D:\\game\\steam_settings")
    print("\nNote:")
    print(" Running the tool without any switches will make it attempt to read the global settings folder")
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
                f.write('[' + item[0] + ']\n') # section
                for kv in item[1].items():
                    if kv[1][1]:
                        f.write('# ' + kv[1][1] + '\n') # comment
                    f.write(kv[0] + '=' + str(kv[1][0]) + '\n') # key/value pair
                f.write('\n') # key/value pair

itf_patts = [
    ( r'SteamClient\d+', "client" ),
    ( r'SteamGameServerStats\d+', "gameserver_stats" ),
    ( r'SteamGameServer\d+', "gameserver" ),
    ( r'SteamMatchMakingServers\d+', "matchmaking_servers" ),
    ( r'SteamMatchMaking\d+', "matchmaking" ),
    ( r'SteamUser\d+', "user" ),
    ( r'SteamFriends\d+', "friends" ),
    ( r'SteamUtils\d+', "utils" ),
    ( r'STEAMUSERSTATS_INTERFACE_VERSION\d+', "user_stats" ),
    ( r'STEAMAPPS_INTERFACE_VERSION\d+', "apps" ),
    ( r'SteamNetworking\d+', "networking" ),
    ( r'STEAMREMOTESTORAGE_INTERFACE_VERSION\d+', "remote_storage" ),
    ( r'STEAMSCREENSHOTS_INTERFACE_VERSION\d+', "screenshots" ),
    ( r'STEAMHTTP_INTERFACE_VERSION\d+', "http" ),
    ( r'STEAMUNIFIEDMESSAGES_INTERFACE_VERSION\d+', "unified_messages" ),
    ( r'STEAMCONTROLLER_INTERFACE_VERSION\d+', "controller" ),
    ( r'SteamController\d+', "controller" ),
    ( r'STEAMUGC_INTERFACE_VERSION\d+', "ugc" ),
    ( r'STEAMAPPLIST_INTERFACE_VERSION\d+', "applist" ),
    ( r'STEAMMUSIC_INTERFACE_VERSION\d+', "music" ),
    ( r'STEAMMUSICREMOTE_INTERFACE_VERSION\d+', "music_remote" ),
    ( r'STEAMHTMLSURFACE_INTERFACE_VERSION_\d+', "html_surface" ),
    ( r'STEAMINVENTORY_INTERFACE_V\d+', "inventory" ),
    ( r'STEAMVIDEO_INTERFACE_V\d+', "video" ),
    ( r'SteamMasterServerUpdater\d+', "masterserver_updater" ),
]

def add_itf_line(itf: str, out_dict_ini: dict):
    for itf_patt in itf_patts:
        if re.match(itf_patt[0], itf):
            merge_dict(out_dict_ini, {
                'configs.app.ini': {
                    'app::steam_interfaces': {
                        itf_patt[1]: (itf, ''),
                    },
                }
            })
            return


def main():
    is_windows = platform.system().lower() == "windows"
    global_settings = ''

    if len(sys.argv) > 1:
        global_settings = sys.argv[1]
    else:
        if is_windows:
            appdata = os.getenv('APPDATA')
            if appdata:
                global_settings = os.path.join(appdata, 'Goldberg SteamEmu Saves', 'settings')
        else:
            xdg = os.getenv('XDG_DATA_HOME')
            if xdg:
                global_settings = os.path.join(xdg, 'Goldberg SteamEmu Saves', 'settings')
            
            if not global_settings:
                home_env = os.getenv('HOME')
                if home_env:
                    global_settings = os.path.join(home_env, 'Goldberg SteamEmu Saves', 'settings')

    if not global_settings or not os.path.isdir(global_settings):
        print('failed to detect settings folder', file=sys.stderr)
        help()
        sys.exit(1)

    out_dict_ini = {}
    for file in glob.glob('*.*', root_dir=global_settings):
        file = file.lower()
        if file == 'force_account_name.txt':
            with open(os.path.join(global_settings, file), "r", encoding='utf-8') as fr:
                merge_dict(out_dict_ini, {
                    'configs.user.ini': {
                        'user::general': {
                            'account_name': (fr.readline(), 'user account name'),
                        },
                    }
                })
        elif file == 'account_name.txt':
            with open(os.path.join(global_settings, file), "r", encoding='utf-8') as fr:
                merge_dict(out_dict_ini, {
                    'configs.user.ini': {
                        'user::general': {
                            'account_name': (fr.readline(), 'user account name'),
                        },
                    }
                })
        elif file == 'force_branch_name.txt':
            with open(os.path.join(global_settings, file), "r", encoding='utf-8') as fr:
                merge_dict(out_dict_ini, {
                    'configs.app.ini': {
                        'app::general': {
                            'branch_name': (fr.readline(), 'the name of the beta branch'),
                        },
                    }
                })
        elif file == 'force_language.txt':
            with open(os.path.join(global_settings, file), "r", encoding='utf-8') as fr:
                merge_dict(out_dict_ini, {
                    'configs.user.ini': {
                        'user::general': {
                            'language': (fr.readline().strip(), 'the language reported to the app/game, https://partner.steamgames.com/doc/store/localization/languages'),
                        },
                    }
                })
        elif file == 'language.txt':
            with open(os.path.join(global_settings, file), "r", encoding='utf-8') as fr:
                merge_dict(out_dict_ini, {
                    'configs.user.ini': {
                        'user::general': {
                            'language': (fr.readline().strip(), 'the language reported to the app/game, https://partner.steamgames.com/doc/store/localization/languages'),
                        },
                    }
                })
        elif file == 'force_listen_port.txt':
            with open(os.path.join(global_settings, file), "r", encoding='utf-8') as fr:
                merge_dict(out_dict_ini, {
                    'configs.main.ini': {
                        'main::connectivity': {
                            'listen_port': (fr.readline().strip(), 'change the UDP/TCP port the emulator listens on'),
                        },
                    }
                })
        elif file == 'listen_port.txt':
            with open(os.path.join(global_settings, file), "r", encoding='utf-8') as fr:
                merge_dict(out_dict_ini, {
                    'configs.main.ini': {
                        'main::connectivity': {
                            'listen_port': (fr.readline().strip(), 'change the UDP/TCP port the emulator listens on'),
                        },
                    }
                })
        elif file == 'force_steamid.txt':
            with open(os.path.join(global_settings, file), "r", encoding='utf-8') as fr:
                merge_dict(out_dict_ini, {
                    'configs.user.ini': {
                        'user::general': {
                            'account_steamid': (fr.readline().strip(), 'Steam64 format'),
                        },
                    }
                })
        elif file == 'user_steam_id.txt':
            with open(os.path.join(global_settings, file), "r", encoding='utf-8') as fr:
                merge_dict(out_dict_ini, {
                    'configs.user.ini': {
                        'user::general': {
                            'account_steamid': (fr.readline().strip(), 'Steam64 format'),
                        },
                    }
                })
        elif file == 'ip_country.txt':
            with open(os.path.join(global_settings, file), "r", encoding='utf-8') as fr:
                merge_dict(out_dict_ini, {
                    'configs.user.ini': {
                        'user::general': {
                            'ip_country': (fr.readline().strip(), 'report a country IP if the game queries it, https://www.iban.com/country-codes'),
                        },
                    }
                })
        elif file == 'overlay_appearance.txt':
            with open(os.path.join(global_settings, file), "r", encoding='utf-8') as fr:
                ov_lines = [lll.strip() for lll in fr.readlines() if lll.strip() and lll.strip()[0] != ';' and lll.strip()[0] != '#']
                for ovl in ov_lines:
                    [ov_name, ov_val] = ovl.split(' ', 1)
                    merge_dict(out_dict_ini, {
                        'configs.overlay.ini': {
                            'overlay::appearance': {
                                ov_name.strip(): (ov_val.strip(), ''),
                            },
                        }
                    })
        elif file == 'build_id.txt':
            with open(os.path.join(global_settings, file), "r", encoding='utf-8') as fr:
                merge_dict(out_dict_ini, {
                    'configs.app.ini': {
                        'app::general': {
                            'build_id': (fr.readline().strip(), 'allow the app/game to show the correct build id'),
                        },
                    }
                })
        elif file == 'disable_account_avatar.txt':
            merge_dict(out_dict_ini, {
                'configs.main.ini': {
                    'main::general': {
                        'disable_account_avatar': (1, 'disable avatar functionality'),
                    },
                }
            })
        elif file == 'disable_networking.txt':
            merge_dict(out_dict_ini, {
                'configs.main.ini': {
                    'main::connectivity': {
                        'disable_networking': (1, 'disable all steam networking interface functionality'),
                    },
                }
            })
        elif file == 'disable_sharing_stats_with_gameserver.txt':
            merge_dict(out_dict_ini, {
                'configs.main.ini': {
                    'main::connectivity': {
                        'disable_sharing_stats_with_gameserver': (1, 'prevent sharing stats and achievements with any game server, this also disables the interface ISteamGameServerStats'),
                    },
                }
            })
        elif file == 'disable_source_query.txt':
            merge_dict(out_dict_ini, {
                'configs.main.ini': {
                    'main::connectivity': {
                        'disable_source_query': (1, 'do not send server details to the server browser, only works for game servers'),
                    },
                }
            })
        elif file == 'steam_interfaces.txt':
            with open(os.path.join(global_settings, file), "r", encoding='utf-8') as fr:
                itf_lines = [lll.strip() for lll in fr.readlines() if lll.strip()]
                for itf in itf_lines:
                    add_itf_line(itf, out_dict_ini)
        elif file == 'overlay_hook_delay_sec.txt':
            with open(os.path.join(global_settings, file), "r", encoding='utf-8') as fr:
                merge_dict(out_dict_ini, {
                    'configs.overlay.ini': {
                        'overlay::general': {
                            'hook_delay_sec': (fr.readline().strip(), 'amount of time to wait before attempting to detect and hook the renderer'),
                        },
                    }
                })
        elif file == 'overlay_renderer_detector_timeout_sec.txt':
            with open(os.path.join(global_settings, file), "r", encoding='utf-8') as fr:
                merge_dict(out_dict_ini, {
                    'configs.overlay.ini': {
                        'overlay::general': {
                            'renderer_detector_timeout_sec': (fr.readline().strip(), 'timeout for the renderer detector'),
                        },
                    }
                })
        elif file == 'enable_experimental_overlay.txt':
            merge_dict(out_dict_ini, {
                'configs.overlay.ini': {
                    'overlay::general': {
                        'enable_experimental_overlay': (1, 'XXX USE AT YOUR OWN RISK XXX, enable the experimental overlay, might cause crashes'),
                    },
                }
            })
        elif file == 'app_paths.txt':
            with open(os.path.join(global_settings, file), "r", encoding='utf-8') as fr:
                app_lines = [lll for lll in fr.readlines() if lll.strip()]
                for app_lll in app_lines:
                    [apppid, appppath] = app_lll.split('=', 1)
                    merge_dict(out_dict_ini, {
                        'configs.app.ini': {
                            'app::paths': {
                                apppid.strip(): (appppath, ''),
                            },
                        }
                    })
        elif file == 'achievements_bypass.txt':
            merge_dict(out_dict_ini, {
                'configs.main.ini': {
                    'main::misc': {
                        'achievements_bypass': (1, 'force SetAchievement() to always return true'),
                    },
                }
            })
        elif file == 'crash_printer_location.txt':
            with open(os.path.join(global_settings, file), "r", encoding='utf-8') as fr:
                merge_dict(out_dict_ini, {
                    'configs.main.ini': {
                        'main::general': {
                            'crash_printer_location': (fr.readline(), 'this is intended to debug some annoying scenarios, and best used with the debug build'),
                        },
                    }
                })
        elif file == 'disable_lan_only.txt':
            merge_dict(out_dict_ini, {
                'configs.main.ini': {
                    'main::connectivity': {
                        'disable_lan_only': (1, 'prevent hooking OS networking APIs and allow any external requests'),
                    },
                }
            })
        elif file == 'disable_leaderboards_create_unknown.txt':
            merge_dict(out_dict_ini, {
                'configs.main.ini': {
                    'main::general': {
                        'disable_leaderboards_create_unknown': (1, 'prevent Steam_User_Stats::FindLeaderboard() from always succeeding and creating the unknown leaderboard'),
                    },
                }
            })
        elif file == 'disable_lobby_creation.txt':
            merge_dict(out_dict_ini, {
                'configs.main.ini': {
                    'main::connectivity': {
                        'disable_lobby_creation': (1, 'prevent lobby creation in steam matchmaking interface'),
                    },
                }
            })
        elif file == 'disable_overlay_achievement_notification.txt':
            merge_dict(out_dict_ini, {
                'configs.overlay.ini': {
                    'overlay::general': {
                        'disable_achievement_notification': (1, 'disable the achievements notifications'),
                    },
                }
            })
        elif file == 'disable_overlay_friend_notification.txt':
            merge_dict(out_dict_ini, {
                'configs.overlay.ini': {
                    'overlay::general': {
                        'disable_friend_notification': (1, 'disable friends invitations and messages notifications'),
                    },
                }
            })
        elif file == 'disable_overlay_warning_any.txt':
            merge_dict(out_dict_ini, {
                'configs.overlay.ini': {
                    'overlay::general': {
                        'disable_warning_any': (1, 'disable any warning in the overlay'),
                    },
                }
            })
        elif file == 'disable_overlay_warning_bad_appid.txt':
            merge_dict(out_dict_ini, {
                'configs.overlay.ini': {
                    'overlay::general': {
                        'disable_warning_bad_appid': (1, 'disable the bad app ID warning in the overlay'),
                    },
                }
            })
        elif file == 'disable_overlay_warning_local_save.txt':
            merge_dict(out_dict_ini, {
                'configs.overlay.ini': {
                    'overlay::general': {
                        'disable_warning_local_save': (1, 'disable the local_save warning in the overlay'),
                    },
                }
            })
        elif file == 'download_steamhttp_requests.txt':
            merge_dict(out_dict_ini, {
                'configs.main.ini': {
                    'main::connectivity': {
                        'download_steamhttp_requests': (1, 'attempt to download external HTTP(S) requests made via Steam_HTTP::SendHTTPRequest()'),
                    },
                }
            })
        elif file == 'force_steamhttp_success.txt':
            merge_dict(out_dict_ini, {
                'configs.main.ini': {
                    'main::misc': {
                        'force_steamhttp_success': (1, 'force the function Steam_HTTP::SendHTTPRequest() to always succeed'),
                    },
                }
            })
        elif file == 'new_app_ticket.txt':
            merge_dict(out_dict_ini, {
                'configs.main.ini': {
                    'main::general': {
                        'new_app_ticket': (1, 'generate new app auth ticket'),
                    },
                }
            })
        elif file == 'gc_token.txt':
            merge_dict(out_dict_ini, {
                'configs.main.ini': {
                    'main::general': {
                        'gc_token': (1, 'generate/embed GC token inside new App Ticket'),
                    },
                }
            })
        elif file == 'immediate_gameserver_stats.txt':
            merge_dict(out_dict_ini, {
                'configs.main.ini': {
                    'main::general': {
                        'immediate_gameserver_stats': (1, 'synchronize user stats/achievements with game servers as soon as possible instead of caching them'),
                    },
                }
            })
        elif file == 'is_beta_branch.txt':
            merge_dict(out_dict_ini, {
                'configs.app.ini': {
                    'app::general': {
                        'is_beta_branch': (1, 'make the game/app think we are playing on a beta branch'),
                    },
                }
            })
        elif file == 'matchmaking_server_details_via_source_query.txt':
            merge_dict(out_dict_ini, {
                'configs.main.ini': {
                    'main::general': {
                        'matchmaking_server_details_via_source_query': (1, 'grab the server details for match making using an actual server query'),
                    },
                }
            })
        elif file == 'matchmaking_server_list_actual_type.txt':
            merge_dict(out_dict_ini, {
                'configs.main.ini': {
                    'main::general': {
                        'matchmaking_server_list_actual_type': (1, 'use the proper type of the server list (internet, friends, etc...) when requested by the game'),
                    },
                }
            })
        elif file == 'offline.txt':
            merge_dict(out_dict_ini, {
                'configs.main.ini': {
                    'main::connectivity': {
                        'offline': (1, 'pretend steam is running in offline mode'),
                    },
                }
            })
        elif file == 'share_leaderboards_over_network.txt':
            merge_dict(out_dict_ini, {
                'configs.main.ini': {
                    'main::connectivity': {
                        'share_leaderboards_over_network': (1, 'enable sharing leaderboards scores with people playing the same game on the same network'),
                    },
                }
            })
        elif file == 'steam_deck.txt':
            merge_dict(out_dict_ini, {
                'configs.main.ini': {
                    'main::general': {
                        'steam_deck': (1, 'pretend the app is running on a steam deck'),
                    },
                }
            })

    if out_dict_ini:
        if not os.path.exists('steam_settings'):
            os.makedirs('steam_settings')

        write_ini_file('steam_settings', out_dict_ini)


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

