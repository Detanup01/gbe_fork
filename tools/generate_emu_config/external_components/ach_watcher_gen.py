import copy
import os
import time
import json

def __ClosestDictKey(targetKey : str, srcDict : dict[str, object] | set[str]) -> str | None:
    for k in srcDict:
        if k.lower() == f"{targetKey}".lower():
            return k
    
    return None

def __generate_ach_watcher_schema(lang: str, app_id: int, achs: list[dict]) -> list[dict]:
    out_achs_list = []
    for idx in range(len(achs)):
        ach = copy.deepcopy(achs[idx])
        out_ach_data = {}
        
        # adjust the displayName
        displayName = ""
        ach_displayName = ach.get("displayName", None)
        if ach_displayName:
            if type(ach_displayName) == dict: # this is a dictionary
                displayName : str = ach_displayName.get(lang, "")
                if not displayName and ach_displayName: # has some keys but language not found
                    #print(f'[?] Missing language "{lang}" in "displayName" of achievement {ach["name"]}')
                    nearestLang = __ClosestDictKey(lang, ach_displayName)
                    if nearestLang:
                        #print(f'[?] Best matching language "{nearestLang}"')
                        displayName = ach_displayName[nearestLang]
                    else:
                        print(f'[?] Missing language "{lang}", using displayName from the first language for achievement {ach["name"]}')
                        displayName : str = list(ach_displayName.values())[0]
            else: # single string (or anything else)
                displayName = ach_displayName
            
            del ach["displayName"]
        else:
            print(f'[?] Missing "displayName" in achievement {ach["name"]}')
        
        out_ach_data["displayName"] = displayName
        
        desc = ""
        ach_desc = ach.get("description", None)
        if ach_desc:
            if type(ach_desc) == dict: # this is a dictionary
                desc : str = ach_desc.get(lang, "")
                if not desc and ach_desc: # has some keys but language not found
                    #print(f'[?] Missing language "{lang}" in "description" of achievement {ach["name"]}')
                    nearestLang = __ClosestDictKey(lang, ach_desc)
                    if nearestLang:
                        #print(f'[?] Best matching language "{nearestLang}"')
                        desc = ach_desc[nearestLang]
                    else:
                        print(f'[?] Missing language "{lang}", using description from the first language for achievement {ach["name"]}')
                        desc : str = list(ach_desc.values())[0]
            else: # single string (or anything else)
                desc = ach_desc
            
            del ach["description"]
        else:
            print(f'[?] Missing "description" in achievement {ach["name"]}')
        
        # adjust the description
        out_ach_data["description"] = desc

        # copy the rest of the data
        out_ach_data.update(ach)

        # add links to icon, icongray, and icon_gray
        base_icon_url = r'https://cdn.cloudflare.steamstatic.com/steamcommunity/public/images/apps'
        icon_hash = out_ach_data.get("icon", None)
        if icon_hash:
            out_ach_data["icon"] = f'{base_icon_url}/{app_id}/{icon_hash}'
        else:
            out_ach_data["icon"] = ""

        icongray_hash = out_ach_data.get("icongray", None)
        if icongray_hash:
            out_ach_data["icongray"] = f'{base_icon_url}/{app_id}/{icongray_hash}'
        else:
            out_ach_data["icongray"] = ""

        icon_gray_hash = out_ach_data.get("icon_gray", None)
        if icon_gray_hash:
            del out_ach_data["icon_gray"] # use the old key
            out_ach_data["icongray"] = f'{base_icon_url}/{app_id}/{icon_gray_hash}'
        
        if "hidden" in out_ach_data:
            try:
                out_ach_data["hidden"] = int(out_ach_data["hidden"])
            except Exception as e:
                pass
        else:
            out_ach_data["hidden"] = 0

        out_achs_list.append(out_ach_data)
    
    return out_achs_list

def generate_all_ach_watcher_schemas(
    base_out_dir : str,
    appid: int,
    app_name : str,
    app_exe : str,
    achs: list[dict],
    small_icon_hash : str) -> None:
    
    ach_watcher_out_dir = os.path.join(base_out_dir, "Achievement Watcher", "steam_cache", "schema")
    print(f"generating schemas for Achievement Watcher in: {ach_watcher_out_dir}")

    if app_exe:
        print(f"detected app exe: '{app_exe}'")
    else:
        print(f"[X] couldn't detect app exe")
    
    # if not achs:
    #     print("[X] No achievements were found for Achievement Watcher")
    #     return
    
    small_icon_url = ''
    if small_icon_hash:
        small_icon_url = f"https://cdn.cloudflare.steamstatic.com/steamcommunity/public/images/apps/{appid}/{small_icon_hash}.jpg"
    images_base_url = r'https://cdn.cloudflare.steamstatic.com/steam/apps'
    ach_watcher_base_schema = {
        "appid": appid,
        "name": app_name,
        "binary": app_exe,
        "achievement": {
            "total": len(achs),
        },
        "img": {
            "header":     f"{images_base_url}/{appid}/header.jpg",
            "background": f"{images_base_url}/{appid}/page_bg_generated_v6b.jpg",
            "portrait":   f"{images_base_url}/{appid}/library_600x900.jpg",
            "hero":       f"{images_base_url}/{appid}/library_hero.jpg",
            "icon":       small_icon_url,
        },
        "apiVersion": 1,
    }
    
    langs : set[str] = set()
    for ach in achs:
        displayNameLangs = ach.get("displayName", None)
        if displayNameLangs and type(displayNameLangs) == dict:
            langs.update(list(displayNameLangs.keys()))
            
        descriptionLangs = ach.get("description", None)
        if descriptionLangs and type(descriptionLangs) == dict:
            langs.update(list(descriptionLangs.keys()))

    if "token" in langs:
        langs.remove("token")
    
    tokenKey = __ClosestDictKey("token", langs)
    if tokenKey:
        langs.remove(tokenKey)
    
    if not langs:
        print("[X] Couldn't detect supported languages, assuming English is the only supported language for Achievement Watcher")
        langs = ["english"]
    
    for lang in langs:
        out_schema_folder = os.path.join(ach_watcher_out_dir, lang)
        if not os.path.exists(out_schema_folder):
            os.makedirs(out_schema_folder)
            time.sleep(0.050)

        out_schema = copy.copy(ach_watcher_base_schema)
        out_schema["achievement"]["list"] = __generate_ach_watcher_schema(lang, appid, achs)
        out_schema_file = os.path.join(out_schema_folder, f'{appid}.db')
        with open(out_schema_file, "wt", encoding='utf-8') as f:
            json.dump(out_schema, f, ensure_ascii=False, indent=2)
