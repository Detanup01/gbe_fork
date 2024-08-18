import vdf
import sys
import os
import json
import copy
import traceback


STAT_TYPE_INT = '1'
STAT_TYPE_FLOAT = '2'
STAT_TYPE_AVGRATE = '3'
STAT_TYPE_BITS = '4'

def generate_stats_achievements(
        schema, config_directory
    ) -> tuple[list[dict], list[dict], bool, bool]:
    schema = vdf.binary_loads(schema)
    # print(schema)
    achievements_out : list[dict] = []
    stats_out : list[dict] = []

    for appid in schema:
        sch = schema[appid]
        stat_info = sch['stats']
        for s in stat_info:
            stat = stat_info[s]
            if stat['type'] == STAT_TYPE_BITS:
                achs = stat['bits']
                for ach_num in achs:
                    out = {}
                    ach = achs[ach_num]
                    out['hidden'] = 0
                    for x in ach['display']:
                        value = ach['display'][x]
                        if f'{x}'.lower() == 'name':
                            x = 'displayName'
                        elif f'{x}'.lower() == 'desc':
                            x = 'description'
                        elif x == 'Hidden' or f'{x}'.lower() == 'hidden':
                            x = 'hidden'
                            try:
                                value = int(value)
                            except Exception as e:
                                pass
                        out[x] = value
                    out['name'] = ach['name']
                    if 'progress' in ach:
                        out['progress'] = ach['progress']
                    achievements_out += [out]
            else:
                out = {}
                out['default'] = 0
                out['name'] = stat['name']
                if 'min' in stat:
                    out['min'] = stat['min']
                if stat['type'] == STAT_TYPE_INT:
                    out['type'] = 'int'
                elif stat['type'] == STAT_TYPE_FLOAT:
                    out['type'] = 'float'
                elif stat['type'] == STAT_TYPE_AVGRATE:
                    out['type'] = 'avgrate'
                if 'Default' in stat:
                    out['default'] = stat['Default']
                elif 'default' in stat:
                    out['default'] = stat['default']

                stats_out += [out]
            #print(stat_info[s])

    copy_default_unlocked_img = False
    copy_default_locked_img = False
    output_ach = copy.deepcopy(achievements_out)
    for out_ach in output_ach:
        icon = out_ach.get("icon", None)
        if icon:
            out_ach["icon"] = f"img/{icon}"
        else:
            out_ach["icon"] = r'img/steam_default_icon_unlocked.jpg'
            copy_default_unlocked_img = True

        icon_gray = out_ach.get("icon_gray", None)
        if icon_gray:
            out_ach["icon_gray"] = f"img/{icon_gray}"
        else:
            out_ach["icon_gray"] = r'img/steam_default_icon_locked.jpg'
            copy_default_locked_img = True

        icongray = out_ach.get("icongray", None)
        if icongray:
            out_ach["icongray"] = f"{icongray}"

    output_stats : list[str] = []
    for s in stats_out:
        default_num = 0
        if f"{s['type']}".lower() == 'int':
            try:
                default_num = int(s['default'])
            except ValueError:
                try:
                    default_num = int(float(s['default']))
                except ValueError:
                    # we set this to min if someone is failed to set to a fucking int value. and after this and still throwing error I gonna throw the dev out of the windows whoever misstyped that!!!
                    # fixes 282800 | STAT_OJ46_C12 (<---THIS ONE)
                    if 'min' in s:
                        default_num = int(s['min'])
                    else:
                        raise ValueError('min not exist in (s) and no way to get the data. please report with the appid')
        else:
            default_num = float(s['default'])
        output_stats.append(f"{s['name']}={s['type']}={default_num}\n")

    # print(output_ach)
    # print(output_stats)

    if not os.path.exists(config_directory):
        os.makedirs(config_directory)

    if output_ach:
        with open(os.path.join(config_directory, "achievements.json"), 'wt', encoding='utf-8') as f:
            json.dump(output_ach, f, indent=2)

    if output_stats:
        with open(os.path.join(config_directory, "stats.txt"), 'wt', encoding='utf-8') as f:
            f.writelines(output_stats)

    return (achievements_out, stats_out,
            copy_default_unlocked_img, copy_default_locked_img)

def help():
    exe_name = os.path.basename(sys.argv[0])
    print(f"\nUsage: {exe_name} UserGameStatsSchema_480.bin [UserGameStatsSchema_2370.bin] ... ")
    print(f" Example: {exe_name} UserGameStatsSchema_480.bin")
    print(f" Example: {exe_name} UserGameStatsSchema_480.bin UserGameStatsSchema_2370.bin")
    print("\nAt least 1 .bin file must be provided\n")

if __name__ == '__main__':
    if len(sys.argv) < 2:
        help()
        sys.exit(1)

    for bin_file in sys.argv[1:]:
        try:
            print(f"parsing schema file '{bin_file}'")
            schema: bytes = b''
            with open(bin_file, 'rb') as f:
                schema = f.read()
            if schema:
                filename = os.path.basename(bin_file)
                outdir = os.path.join(f"{filename}_output", "steam_settings")
                print(f"output dir: '{outdir}'")
                generate_stats_achievements(schema, outdir)
            else:
                print("[X] couldn't load file", file=sys.stderr)
            
            print('**********************************\n')
        except Exception as e:
            print("Unexpected error:")
            print(e)
            print("-----------------------")
            for line in traceback.format_exception(e):
                print(line)
            print('xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n')
        
    sys.exit(0)
