import vdf
import sys
import os
import json
import copy


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
                        if x == 'name':
                            x = 'displayName'
                        elif x == 'desc':
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
                default_num = int(float(s['default']))
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

if __name__ == '__main__':
    if len(sys.argv) < 2:
        print("format: {} UserGameStatsSchema_480.bin".format(sys.argv[0]))
        exit(0)


    with open(sys.argv[1], 'rb') as f:
        schema = f.read()

    generate_stats_achievements(schema, os.path.join("{}".format( "{}_output".format(sys.argv[1])), "steam_settings"))
