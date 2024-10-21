import os, re, sys, requests, shutil, traceback
import urllib.request
from configobj import ConfigObj
from external_components import (
    safe_name
)

# https://stackoverflow.com/a/48336994 # NOTE alternatively we could use 're.findall(str(re.escape(string1))+"(.*)"+str(re.escape(string2)),stringSubject)[0]'    
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

# https://stackoverflow.com/a/13641746 # NOTE using this fix a strange issue where first name value of some ini files had starting and trailing double quotes ( " )
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

def ParseNumber(number):
    if int(number) <= 9:
        number = str(0) + str(number)
    else:
        number = str(number)
    return number

def download_scx(base_out_dir : str, appid : int):

    market_link = f"https://www.steamcardexchange.net/index.php?gamepage-appid-{appid}"

    if not os.path.exists(os.path.join(base_out_dir, 'steam_misc\\app_scx')):
        os.makedirs(os.path.join(base_out_dir, 'steam_misc\\app_scx'))

    with urllib.request.urlopen(market_link) as f:
        html = f.read().decode('utf-8')

    file = os.path.join(base_out_dir, "steam_misc\\app_scx\\app_scx.txt")
    with open(file, 'w', encoding='utf-8') as f:
        f.write(html)

    if os.path.isfile(os.path.join(base_out_dir, 'steam_misc\\app_scx\\app_scx.txt')):
        with open(os.path.join(base_out_dir, 'steam_misc\\app_scx\\app_scx.txt'), 'r', encoding='utf-8') as app_scx:
            app_scx_line = app_scx.readlines()
        
        #line_number_prev = 0 # previous line number, unused
        line_section = ""
        line_series_hash = ""
        line_series_name = ""
        line_series_count = 0

        _trading_cards = ""
        _foil_trading_cards = ""
        _booster_pack = ""
        _badges = ""
        _foil_badges = ""
        _emoticons = ""
        _backgrounds = ""
        _animated_stickers = ""
        _animated_backgrounds = ""
        _animated_mini_backgrounds = ""
        _avatar_frames = ""
        _animated_avatars = ""
        _profiles = ""

        _game_found=True

        print(f"[ ] Searching SCX content...")

        for line in app_scx_line:
            if 'Game not found' in line:
                shutil.rmtree(os.path.join(base_out_dir, 'steam_misc\\app_scx'))
                _game_found=False
                print(f"[?] __ no SCX content found --- nothing downloaded to <OUT_DIR>\\steam_misc\\app_scx folder")
                break

            if ('class="tracking-wider font-league-gothic"' in line) and ('<a href="' in line):
                series_hash = GetListOfSubstrings(line, '<a href="', '">')[0]
                series_name = GetListOfSubstrings(line, f'<a href="{series_hash}">', '</a>')[0]

                not_series = ['>Note<', '>Trading Cards<', '>Foil Trading Cards<', '>Booster Pack<', '>Badges<', '>Foil Badges<', '>Emoticons<', '>Backgrounds<', '>Animated Stickers<', '>Animated Backgrounds<', '>Animated Mini Backgrounds<', '>Avatar Frames<', '>Animated Avatars<', '>Startup Movie<', '>Profiles<']
                
                if all(sub not in line for sub in not_series):
                    line_series_hash = str(series_hash)
                    line_series_name = str(series_name)
                    line_series_name = line_series_name.replace('&amp;', '&').replace("&apos;", "'").replace('&quot;', '"').replace('&nbsp;', ' ')
                    line_series_name_safe = safe_name.create_safe_name(line_series_name)

                    if not os.path.exists(os.path.join(base_out_dir, f'steam_misc\\app_scx\\{line_series_name_safe}')):
                        os.makedirs(os.path.join(base_out_dir, f'steam_misc\\app_scx\\{line_series_name_safe}'))
                        if not os.path.exists(os.path.join(base_out_dir, f'steam_misc\\app_scx\\{appid}_s.txt')):
                            with open(os.path.join(base_out_dir, f'steam_misc\\app_scx\\{appid}_s.txt'), 'w') as f_txt:
                                f_txt.close()
                        with open(os.path.join(base_out_dir, f'steam_misc\\app_scx\\{appid}_s.txt'), 'a') as f_txt:
                            f_txt.write(f'{line_series_name_safe}\n')
                            f_txt.close()

                    line_series_count = line_series_count + 1

            if 'Last update:' in line:
                last_update = GetListOfSubstrings(line, '>Last update: ', ' - ')[0]
                if not os.path.exists(os.path.join(base_out_dir, f'steam_misc\\app_scx\\{appid}_u.txt')):
                    with open(os.path.join(base_out_dir, f'steam_misc\\app_scx\\{appid}_u.txt'), 'w') as f_txt:
                        f_txt.close()
                with open(os.path.join(base_out_dir, f'steam_misc\\app_scx\\{appid}_u.txt'), 'a') as f_txt:
                    f_txt.write(f'{last_update}\n')
                    f_txt.close()

            if f'<a href="{line_series_hash}-cards">' in line:
                line_section = "Trading Cards"
                _trading_cards_series = line_series_count
                if not os.path.exists(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_trading_cards.ini")): 
                    with open(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_trading_cards.ini"), 'w') as file: 
                        file.write("[trading_cards]") 
                #ini_cards_blue = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name}\\app_trading_cards.ini"), encoding='utf-8', create_empty=True)
                trading_cards_blue_count = 99
                trading_card_blue_bg_count = 0
            elif f'<a href="{line_series_hash}-foilcards">' in line:
                line_section = "Foil Trading Cards"
                _foil_trading_cards_series = line_series_count
                if not os.path.exists(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_trading_cards_foil.ini")): 
                    with open(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_trading_cards_foil.ini"), 'w') as file: 
                        file.write("[trading_cards_foil]")
                #ini_cards_foil = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name}\\app_trading_cards_foil.ini"), encoding='utf-8', create_empty=True)
                trading_cards_foil_count = 99
                trading_card_foil_bg_count = 0
            elif f'<a href="{line_series_hash}-booster">' in line:
                line_section = "Booster Pack"
                _booster_pack_series = line_series_count
            elif f'<a href="{line_series_hash}-badges">' in line:
                line_section = "Badges"
                _badges_series = line_series_count
                if not os.path.exists(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_badges.ini")): 
                    with open(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_badges.ini"), 'w') as file: 
                        file.write("[badges]") 
                #ini_badges = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name}\\app_badges.ini"), encoding='utf-8', create_empty=True)
                badge_number = 0
            elif f'<a href="{line_series_hash}-foilbadges">' in line:
                line_section = "Foil Badges"
                _foil_badges_series = line_series_count
                if not os.path.exists(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_badges_foil.ini")): 
                    with open(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_badges_foil.ini"), 'w') as file: 
                        file.write("[badges_foil]")
                #ini_badges_foil = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name}\\app_badges_foil.ini"), encoding='utf-8', create_empty=True)
                badge_foil_number = 0
            elif f'<a href="{line_series_hash}-emoticons">' in line:
                line_section = "Emoticons"
                _emoticons_series = line_series_count
                if not os.path.exists(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_emoticons.ini")): 
                    with open(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_emoticons.ini"), 'w') as file: 
                        file.write("[emoticons]")
                #ini_emoticons = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name}\\app_emoticons.ini"), encoding='utf-8', create_empty=True)
                emoticon_number_small = 0
                emoticon_number_large = 0
            elif f'<a href="{line_series_hash}-backgrounds">' in line:
                line_section = "Backgrounds"
                _backgrounds_series = line_series_count
                if not os.path.exists(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_backgrounds.ini")): 
                    with open(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_backgrounds.ini"), 'w') as file: 
                        file.write("[backgrounds]")
                #ini_backgrounds = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name}\\app_backgrounds.ini"), encoding='utf-8', create_empty=True)
            elif f'<a href="{line_series_hash}-animatedstickers">' in line:
                line_section = "Animated Stickers"
                _animated_stickers_series = line_series_count
                if not os.path.exists(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_animated_stickers.ini")): 
                    with open(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_animated_stickers.ini"), 'w') as file: 
                        file.write("[animated_stickers]") 
                #ini_stickers = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name}\\app_animated_stickers.ini"), encoding='utf-8', create_empty=True)
                sticker_animated_number = 0
                sticker_static_number = 0
            elif f'<a href="{line_series_hash}-animatedbackgrounds">' in line:
                line_section = "Animated Backgrounds"
                _animated_backgrounds_series = line_series_count
                if not os.path.exists(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_animated_backgrounds.ini")): 
                    with open(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_animated_backgrounds.ini"), 'w') as file: 
                        file.write("[animated_backgrounds]") 
                #ini_animated_bg = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name}\\app_animated_backgrounds.ini"), encoding='utf-8', create_empty=True)
                animated_bg_number = 0
            elif f'<a href="{line_series_hash}-animatedminibackgrounds">' in line:
                line_section = "Animated Mini Backgrounds"
                _animated_mini_backgrounds_series = line_series_count
                if not os.path.exists(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_animated_mini_backgrounds.ini")): 
                    with open(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_animated_mini_backgrounds.ini"), 'w') as file: 
                        file.write("[animated_mini_backgrounds]") 
                #ini_animated_minibg = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name}\\app_animated_mini_backgrounds.ini"), encoding='utf-8', create_empty=True)
                animated_minibg_number = 0
            elif f'<a href="{line_series_hash}-avatarframes">' in line:
                line_section = "Avatar Frames"
                _avatar_frames_series = line_series_count
                if not os.path.exists(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_avatar_frames.ini")): 
                    with open(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_avatar_frames.ini"), 'w') as file: 
                        file.write("[avatar_frames]") 
                #ini_avatar_frames = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name}\\app_avatar_frames.ini"), encoding='utf-8', create_empty=True)
                avatar_frame_count = 0
                avatar_frame_static_count =0
            elif f'<a href="{line_series_hash}-avataranimated">' in line:
                line_section = "Animated Avatars"
                _animated_avatars_series = line_series_count
                if not os.path.exists(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_animated_avatars.ini")): 
                    with open(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_animated_avatars.ini"), 'w') as file: 
                        file.write("[animated_avatars]") 
                #ini_animated_avatars = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name}\\app_animated_avatars.ini"), encoding='utf-8', create_empty=True)
                animated_avatar_count = 0
                animated_avatar_static_count = 0
            elif f'<a href="{line_series_hash}-profiles">' in line:
                line_section = "Profiles"
                _profiles_series = line_series_count
                if not os.path.exists(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_profiles.ini")): 
                    with open(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_profiles.ini"), 'w') as file: 
                        file.write("[profiles]") 
                #ini_profiles = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name}\\app_profiles.ini"), encoding='utf-8', create_empty=True)
                profile_count = 0

            if line_section == "Trading Cards":

                if _trading_cards != line_series_name_safe:
                    if _trading_cards_series == line_series_count: # this fixes duplicating message for last found 'section' in html source, after finding a new 'series'
                        print(f"[ ] __ found '{line_series_name_safe}' trading cards...")
                    _trading_cards = line_series_name_safe

                if 'data-gallery-type="cards"' in line:
                    trading_card_blue_link = GetListOfSubstrings(line, '<img loading="lazy" src="', '" ')[0]
                    trading_card_blue_number = GetListOfSubstrings(line, ' - Card ', ' of ')[0]
                    trading_card_blue_count = GetListOfSubstrings(line, f' - Card {trading_card_blue_number} of ', ' - ')[0]
                    trading_card_blue_name = GetListOfSubstrings(line, f' - Card {trading_card_blue_number} of {trading_card_blue_count} - ', '" data-gallery-desc=')[0]
                    trading_card_blue_name = trading_card_blue_name.replace('&amp;', '&').replace("&apos;", "'").replace('&quot;', '"').replace('&nbsp;', ' ')
                    trading_card_blue_name_safe = safe_name.create_safe_name(trading_card_blue_name)

                    ini_cards_blue = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_trading_cards.ini"), encoding='utf-8', create_empty=True)
                    ini_cards_blue['trading_cards'][f'card{ParseNumber(trading_card_blue_number)}_name'] = trading_card_blue_name.strip('"')
                    ini_cards_blue['trading_cards'][f'card{ParseNumber(trading_card_blue_number)}_small_png'] = trading_card_blue_link
                    ini_cards_blue.write()

                    ReplaceStringInFile(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_trading_cards.ini"), ' = "', '"', '')

                    trading_cards_blue_count = int(trading_card_blue_count)

                    if not os.path.exists(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Trading Cards")):
                        os.makedirs(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Trading Cards"))

                    try:
                        response_png = requests.get(trading_card_blue_link)
                        with open(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Trading Cards\\{ParseNumber(trading_card_blue_number)}. {trading_card_blue_name_safe} _.png"), "wb") as f:
                            f.write(response_png.content)
                    except Exception as e:
                        print(f"[X] __ Error downloading from '{trading_card_blue_link}'", file=sys.stderr)
                        traceback.print_exception(e, file=sys.stderr)

                    try:
                        response_jpg = requests.get(trading_card_blue_bg_link)
                        with open(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Trading Cards\\{ParseNumber(trading_card_blue_number)}. {trading_card_blue_name_safe} _wall.jpg"), "wb") as f:
                            f.write(response_jpg.content)
                    except Exception as e:
                        print(f"[X] __ Error downloading from '{trading_card_blue_bg_link}'", file=sys.stderr)
                        traceback.print_exception(e, file=sys.stderr)

                elif '>Wallpaper<' in line:
                    trading_card_blue_bg_count = trading_card_blue_bg_count + 1
                    if trading_card_blue_bg_count <= trading_cards_blue_count:
                        trading_card_blue_bg_link = GetListOfSubstrings(line, '<a href="', '" ')[0]

                        ini_cards_blue = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_trading_cards.ini"), encoding='utf-8', create_empty=True)

                        ini_cards_blue['trading_cards'][f'card{ParseNumber(trading_card_blue_bg_count)}_large_jpg'] = trading_card_blue_bg_link

                        ini_cards_blue.write()

            elif line_section == "Foil Trading Cards":

                #if _foil_trading_cards != line_series_name_safe:
                    #if _foil_trading_cards_series == line_series_count: # this fixes duplicating message for last found 'section' in html source, after finding a new 'series'
                        #print(f"[ ] __ found '{line_series_name_safe}' foil trading cards...")
                    #_foil_trading_cards = line_series_name_safe 

                if 'data-gallery-type="foil-cards"' in line:
                    trading_card_foil_link = GetListOfSubstrings(line, '<img loading="lazy" src="', '" ')[0]
                    trading_card_foil_number = GetListOfSubstrings(line, ' - Card ', ' of ')[0]
                    trading_card_foil_count = GetListOfSubstrings(line, f' - Card {trading_card_foil_number} of ', ' - ')[0]
                    trading_card_foil_name = GetListOfSubstrings(line, f' - Card {trading_card_foil_number} of {trading_card_foil_count} - ', '" data-gallery-desc=')[0]
                    trading_card_foil_name = trading_card_foil_name.replace('&amp;', '&').replace("&apos;", "'").replace('&quot;', '"').replace('&nbsp;', ' ')
                    trading_card_foil_name_safe = safe_name.create_safe_name(trading_card_foil_name)

                    ini_cards_foil = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_trading_cards_foil.ini"), encoding='utf-8', create_empty=True)
                    ini_cards_foil['trading_cards_foil'][f'card{ParseNumber(trading_card_foil_number)}_foil_name'] = trading_card_foil_name.strip('"')
                    ini_cards_foil['trading_cards_foil'][f'card{ParseNumber(trading_card_foil_number)}_foil_small_png'] = trading_card_foil_link
                    ini_cards_foil.write()

                    ReplaceStringInFile(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_trading_cards_foil.ini"), ' = "', '"', '')

                    trading_cards_foil_count = int(trading_card_foil_count)

                    if not os.path.exists(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Trading Cards\\foil")):
                        os.makedirs(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Trading Cards\\foil"))

                    try:
                        response_png = requests.get(trading_card_foil_link)
                        with open(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Trading Cards\\foil\\{ParseNumber(trading_card_foil_number)}. {trading_card_foil_name_safe} _foil.png"), "wb") as f:
                            f.write(response_png.content)
                    except Exception as e:
                        print(f"[X] __ Error downloading from '{trading_card_foil_link}'", file=sys.stderr)
                        traceback.print_exception(e, file=sys.stderr)

                elif '>Wallpaper<' in line:
                    trading_card_foil_bg_count = trading_card_foil_bg_count + 1
                    if trading_card_foil_bg_count <= trading_cards_foil_count:
                        trading_card_foil_bg_link = GetListOfSubstrings(line, '<a href="', '" ')[0]

                        ini_cards_foil = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_trading_cards_foil.ini"), encoding='utf-8', create_empty=True)
                        ini_cards_foil['trading_cards_foil'][f'card{ParseNumber(trading_card_foil_bg_count)}_foil_large_jpg'] = trading_card_foil_bg_link
                        ini_cards_foil.write()
            
            elif line_section == "Backgrounds":

                if _backgrounds != line_series_name_safe:
                    if _backgrounds_series == line_series_count: # this fixes duplicating message for last found 'section' in html source, after finding a new 'series'
                        print(f"[ ] __ found '{line_series_name_safe}' backgrounds...")
                    _backgrounds = line_series_name_safe 

                if 'data-gallery-type="backgrounds"' in line:
                    wallpaper_link = GetListOfSubstrings(line, '<img loading="lazy" src="', '?size=')[0]
                    wallpaper_link_small = GetListOfSubstrings(line, '<img loading="lazy" src="', '" ')[0]
                    wallpaper_number = GetListOfSubstrings(line, ' - Background ', ' of ')[0]
                    wallpaper_count = GetListOfSubstrings(line, f' - Background {wallpaper_number} of ', ' - ')[0]
                    wallpaper_name = GetListOfSubstrings(line, f' - Background {wallpaper_number} of {wallpaper_count} - ', '" data-gallery-type=')[0]
                    wallpaper_name = wallpaper_name.replace('&amp;', '&').replace("&apos;", "'").replace('&quot;', '"').replace('&nbsp;', ' ')
                    wallpaper_name_safe = safe_name.create_safe_name(wallpaper_name)

                    ini_backgrounds = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_backgrounds.ini"), encoding='utf-8', create_empty=True)
                    ini_backgrounds['backgrounds'][f'background{ParseNumber(wallpaper_number)}_name'] = wallpaper_name.strip('"')
                    ini_backgrounds['backgrounds'][f'background{ParseNumber(wallpaper_number)}_large_jpg'] = wallpaper_link
                    ini_backgrounds['backgrounds'][f'background{ParseNumber(wallpaper_number)}_small_jpg'] = wallpaper_link_small
                    ini_backgrounds.write()

                    ReplaceStringInFile(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_backgrounds.ini"), ' = "', '"', '')

                    if not os.path.exists(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Backgrounds")):
                        os.makedirs(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Backgrounds"))

                    try:
                        response_jpg = requests.get(wallpaper_link)
                        with open(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Backgrounds\\{ParseNumber(wallpaper_number)}. {wallpaper_name_safe} _.jpg"), "wb") as f:
                            f.write(response_jpg.content)
                    except Exception as e:
                        print(f"[X] __ Error downloading from '{wallpaper_link}'", file=sys.stderr)
                        traceback.print_exception(e, file=sys.stderr)

                    if not os.path.exists(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Backgrounds\\thumbs")):
                        os.makedirs(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Backgrounds\\thumbs"))

                    try:
                        response_jpg = requests.get(wallpaper_link_small)
                        with open(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Backgrounds\\thumbs\\{ParseNumber(wallpaper_number)}. {wallpaper_name_safe} _small.jpg"), "wb") as f:
                            f.write(response_jpg.content)
                    except Exception as e:
                        print(f"[X] __ Error downloading from '{wallpaper_link_small}'", file=sys.stderr)
                        traceback.print_exception(e, file=sys.stderr)

            elif line_section == "Badges":

                if _badges != line_series_name_safe:
                    if _badges_series == line_series_count: # this fixes duplicating message for last found 'section' in html source, after finding a new 'series'
                        print(f"[ ] __ found '{line_series_name_safe}' badges...")
                    _badges = line_series_name_safe

                if 'class="sm:h-[80px]"' in line:
                    badge_link = GetListOfSubstrings(line, '<img loading="lazy" src="', '"')[0]
                    badge_number = badge_number + 1
                    badge_name = GetListOfSubstrings(line, f'alt="{line_series_name} - ', '">')[0]
                    badge_name = badge_name.replace('&amp;', '&').replace("&apos;", "'").replace('&quot;', '"').replace('&nbsp;', ' ')
                    badge_name_safe = safe_name.create_safe_name(badge_name)

                    ini_badges = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_badges.ini"), encoding='utf-8', create_empty=True)
                    ini_badges['badges'][f'badge{ParseNumber(badge_number)}_name'] = badge_name.strip('"')
                    ini_badges['badges'][f'badge{ParseNumber(badge_number)}_png'] = badge_link
                    ini_badges.write()

                    ReplaceStringInFile(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_badges.ini"), ' = "', '"', '')

                    if not os.path.exists(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Badges")):
                        os.makedirs(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Badges"))

                    try:
                        response_png = requests.get(badge_link)
                        with open(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Badges\\{ParseNumber(badge_number)}. {badge_name_safe} _.png"), "wb") as f:
                            f.write(response_png.content)
                    except Exception as e:
                        print(f"[X] __ Error downloading from '{badge_link}'", file=sys.stderr)
                        traceback.print_exception(e, file=sys.stderr)

            elif line_section == "Foil Badges":

                #if _foil_badges != line_series_name_safe:
                    #if _foil_badges_series == line_series_count: # this fixes duplicating message for last found 'section' in html source, after finding a new 'series'
                        #print(f"[ ] __ found '{line_series_name_safe}' foil badges...")
                    #_foil_badges = line_series_name_safe

                if 'class="sm:h-[80px]"' in line:
                    badge_foil_link = GetListOfSubstrings(line, '<img loading="lazy" src="', '"')[0]
                    badge_foil_number = badge_foil_number + 1
                    badge_foil_name = GetListOfSubstrings(line, f'alt="{line_series_name} - ', '">')[0]
                    badge_foil_name = badge_foil_name.replace('&amp;', '&').replace("&apos;", "'").replace('&quot;', '"').replace('&nbsp;', ' ')
                    badge_foil_name_safe = safe_name.create_safe_name(badge_foil_name)

                    ini_badges_foil = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_badges_foil.ini"), encoding='utf-8', create_empty=True)
                    ini_badges_foil['badges_foil'][f'badge{ParseNumber(badge_foil_number)}_foil_name'] = badge_foil_name.strip('"')
                    ini_badges_foil['badges_foil'][f'badge{ParseNumber(badge_foil_number)}_foil_png'] = badge_foil_link
                    ini_badges_foil.write()

                    ReplaceStringInFile(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_badges_foil.ini"), ' = "', '"', '')

                    if not os.path.exists(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Badges\\foil")):
                        os.makedirs(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Badges\\foil"))

                    try:
                        response_png = requests.get(badge_foil_link)
                        with open(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Badges\\foil\\{ParseNumber(badge_foil_number)}. {badge_foil_name_safe} _foil.png"), "wb") as f:
                            f.write(response_png.content)
                    except Exception as e:
                        print(f"[X] __ Error downloading from '{badge_foil_link}'", file=sys.stderr)
                        traceback.print_exception(e, file=sys.stderr)

            elif line_section == "Emoticons":

                if _emoticons != line_series_name_safe:
                    if _emoticons_series == line_series_count: # this fixes duplicating message for last found 'section' in html source, after finding a new 'series'
                        print(f"[ ] __ found '{line_series_name_safe}' emoticons...")
                    _emoticons = line_series_name_safe

                if 'class="sm:h-[54px]"' in line:
                    emoticon_link = GetListOfSubstrings(line, '<img loading="lazy" src="', '"')[0]
                    emoticon_number_large = emoticon_number_large + 1
                    emoticon_name = GetListOfSubstrings(line, 'alt=":', ':"')[0]
                    emoticon_name = emoticon_name.replace('&amp;', '&').replace("&apos;", "'").replace('&quot;', '"').replace('&nbsp;', ' ')
                    emoticon_name_safe = safe_name.create_safe_name(emoticon_name)

                    ini_emoticons = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_emoticons.ini"), encoding='utf-8', create_empty=True)
                    ini_emoticons['emoticons'][f'emoticon{ParseNumber(emoticon_number_large)}_name'] = emoticon_name.strip('"')
                    ini_emoticons['emoticons'][f'emoticon{ParseNumber(emoticon_number_large)}_large_png'] = emoticon_link
                    ini_emoticons.write()

                    ReplaceStringInFile(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_emoticons.ini"), ' = "', '"', '')

                    if not os.path.exists(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Emoticons")):
                        os.makedirs(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Emoticons"))

                    try:
                        response_png = requests.get(emoticon_link)
                        with open(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Emoticons\\{ParseNumber(emoticon_number_large)}. {emoticon_name_safe} _.png"), "wb") as f:
                            f.write(response_png.content)
                    except Exception as e:
                        print(f"[X] __ Error downloading from '{emoticon_link}'", file=sys.stderr)
                        traceback.print_exception(e, file=sys.stderr)

                if 'class="h-[18px]"' in line:
                    emoticon_link_small = GetListOfSubstrings(line, '<img loading="lazy" src="', '"')[0]
                    emoticon_number_small = emoticon_number_small + 1
                    emoticon_name = GetListOfSubstrings(line, 'alt=":', ': Chat Preview"')[0]
                    emoticon_name = emoticon_name.replace('&amp;', '&').replace("&apos;", "'").replace('&quot;', '"').replace('&nbsp;', ' ')
                    emoticon_name_safe = safe_name.create_safe_name(emoticon_name)

                    ini_emoticons = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_emoticons.ini"), encoding='utf-8', create_empty=True)
                    ini_emoticons['emoticons'][f'emoticon{ParseNumber(emoticon_number_small)}_small_png'] = emoticon_link_small
                    ini_emoticons.write()

                    if not os.path.exists(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Emoticons\\thumbs")):
                        os.makedirs(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Emoticons\\thumbs"))

                    try:
                        response_png = requests.get(emoticon_link_small)
                        with open(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Emoticons\\thumbs\\{ParseNumber(emoticon_number_small)}. {emoticon_name_safe} _small.png"), "wb") as f:
                            f.write(response_png.content)
                    except Exception as e:
                        print(f"[X] __ Error downloading from '{emoticon_link_small}'", file=sys.stderr)
                        traceback.print_exception(e, file=sys.stderr)

            elif line_section == "Animated Stickers":

                if _animated_stickers != line_series_name_safe:
                    if _animated_stickers_series == line_series_count: # this fixes duplicating message for last found 'section' in html source, after finding a new 'series'
                        print(f"[ ] __ found '{line_series_name_safe}' animated stickers...")
                    _animated_stickers = line_series_name_safe

                if 'Animated" class=' in line:
                    sticker_animated_link = GetListOfSubstrings(line, '<img loading="lazy" src="', '"')[0]
                    sticker_animated_number = sticker_animated_number + 1
                    sticker_animated_name = GetListOfSubstrings(line, 'alt="', ' Animated"')[0]
                    sticker_animated_name = sticker_animated_name.replace('&amp;', '&').replace("&apos;", "'").replace('&quot;', '"').replace('&nbsp;', ' ')
                    sticker_animated_name_safe = safe_name.create_safe_name(sticker_animated_name)

                    ini_stickers = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_animated_stickers.ini"), encoding='utf-8', create_empty=True)
                    ini_stickers['animated_stickers'][f'sticker{ParseNumber(sticker_animated_number)}_name'] = sticker_animated_name.strip('"')
                    ini_stickers['animated_stickers'][f'sticker{ParseNumber(sticker_animated_number)}_anim_png'] = sticker_animated_link
                    ini_stickers.write()

                    ReplaceStringInFile(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_animated_stickers.ini"), ' = "', '"', '')

                    if not os.path.exists(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Animated Stickers")):
                        os.makedirs(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Animated Stickers"))

                    try:
                        response_png = requests.get(sticker_animated_link)
                        with open(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Animated Stickers\\{ParseNumber(sticker_animated_number)}. {sticker_animated_name_safe} _.png"), "wb") as f:
                            f.write(response_png.content)
                    except Exception as e:
                        print(f"[X] __ Error downloading from '{sticker_animated_link}'", file=sys.stderr)
                        traceback.print_exception(e, file=sys.stderr)

                elif 'Static" class=' in line:

                    sticker_static_link = GetListOfSubstrings(line, '<img loading="lazy" src="', '"')[0]
                    sticker_static_number = sticker_static_number + 1
                    sticker_static_name = GetListOfSubstrings(line, 'alt="', ' Static"')[0]
                    sticker_static_name = sticker_static_name.replace('&amp;', '&').replace("&apos;", "'").replace('&quot;', '"').replace('&nbsp;', ' ')
                    sticker_static_name_safe = safe_name.create_safe_name(sticker_static_name)

                    ini_stickers = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_animated_stickers.ini"), encoding='utf-8', create_empty=True)
                    ini_stickers['animated_stickers'][f'sticker{ParseNumber(sticker_static_number)}_static_png'] = sticker_static_link
                    ini_stickers.write()

                    if not os.path.exists(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Animated Stickers\\static")):
                        os.makedirs(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Animated Stickers\\static"))

                    try:
                        response_png = requests.get(sticker_static_link)
                        with open(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Animated Stickers\\static\\{ParseNumber(sticker_static_number)}. {sticker_static_name_safe} _static.png"), "wb") as f:
                            f.write(response_png.content)
                    except Exception as e:
                        print(f"[X] __ Error downloading from '{sticker_static_link}'", file=sys.stderr)
                        traceback.print_exception(e, file=sys.stderr)

            elif line_section == "Animated Backgrounds":

                if _animated_backgrounds != line_series_name_safe:
                    if _animated_backgrounds_series == line_series_count: # this fixes duplicating message for last found 'section' in html source, after finding a new 'series'
                        print(f"[ ] __ found '{line_series_name_safe}' animated backgrounds...")
                    _animated_backgrounds = line_series_name_safe

                if 'class="sm:h-[99px] md:h-[87px] lg:h-[75px] xl:h-[83px] 2xl:h-[88px]"' in line:
                    animated_bg_number = animated_bg_number + 1
                    animated_bg_name = GetListOfSubstrings(line, 'alt="', '"')[0]
                    animated_bg_name = animated_bg_name.replace('&amp;', '&').replace("&apos;", "'").replace('&quot;', '"').replace('&nbsp;', ' ')
                    animated_bg_name_safe = safe_name.create_safe_name(animated_bg_name)
                    animated_bg_static_large = GetListOfSubstrings(line, '<img loading="lazy" src="', '?size=300x180f"')[0]
                    animated_bg_static_small = GetListOfSubstrings(line, '<img loading="lazy" src="', '"')[0]

                    ini_animated_bg = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_animated_backgrounds.ini"), encoding='utf-8', create_empty=True)
                    ini_animated_bg['animated_backgrounds'][f'background{ParseNumber(animated_bg_number)}_name'] = animated_bg_name.strip('"')
                    ini_animated_bg['animated_backgrounds'][f'background{ParseNumber(animated_bg_number)}_static_jpg'] = animated_bg_static_large
                    ini_animated_bg.write()

                    ReplaceStringInFile(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_animated_backgrounds.ini"), ' = "', '"', '')

                    if not os.path.exists(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Animated Backgrounds\\static")):
                        os.makedirs(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Animated Backgrounds\\static"))

                    try:
                        response_jpg = requests.get(animated_bg_static_large)
                        with open(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Animated Backgrounds\\static\\{ParseNumber(animated_bg_number)}. {animated_bg_name_safe} _static.jpg"), "wb") as f:
                            f.write(response_jpg.content)
                    except Exception as e:
                        print(f"[X] __ Error downloading from '{animated_minibg_static_large}'", file=sys.stderr)
                        traceback.print_exception(e, file=sys.stderr)

                    if not os.path.exists(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Animated Backgrounds\\thumbs")):
                        os.makedirs(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Animated Backgrounds\\thumbs"))

                    try:
                        response_jpg = requests.get(animated_bg_static_small)
                        with open(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Animated Backgrounds\\thumbs\\{ParseNumber(animated_bg_number)}. {animated_bg_name_safe} _small.jpg"), "wb") as f:
                            f.write(response_jpg.content)
                    except Exception as e:
                        print(f"[X] __ Error downloading from '{animated_bg_static_small}'", file=sys.stderr)
                        traceback.print_exception(e, file=sys.stderr)

                elif 'type="video/webm"' in line:
                    animated_bg_link_webm = GetListOfSubstrings(line, '<source src="', '"')[0]

                    ini_animated_bg = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_animated_backgrounds.ini"), encoding='utf-8', create_empty=True)
                    ini_animated_bg['animated_backgrounds'][f'background{ParseNumber(animated_bg_number)}_anim_webm'] = animated_bg_link_webm
                    ini_animated_bg.write()

                elif 'type="video/mp4"' in line:
                    animated_bg_link_mp4 = GetListOfSubstrings(line, '<source src="', '"')[0]

                    ini_animated_bg = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_animated_backgrounds.ini"), encoding='utf-8', create_empty=True)
                    ini_animated_bg['animated_backgrounds'][f'background{ParseNumber(animated_bg_number)}_anim_mp4'] = animated_bg_link_mp4
                    ini_animated_bg.write()

                    if not os.path.exists(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Animated Backgrounds")):
                        os.makedirs(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Animated Backgrounds"))

                    try:
                        response_mp4 = requests.get(animated_bg_link_mp4)
                        with open(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Animated Backgrounds\\{ParseNumber(animated_bg_number)}. {animated_bg_name_safe} _.mp4"), "wb") as f:
                            f.write(response_mp4.content)
                    except Exception as e:
                        print(f"[X] __ Error downloading from '{animated_bg_link_mp4}'", file=sys.stderr)
                        traceback.print_exception(e, file=sys.stderr)

            elif line_section == "Animated Mini Backgrounds":

                if _animated_mini_backgrounds != line_series_name_safe:
                    if _animated_mini_backgrounds_series == line_series_count: # this fixes duplicating message for last found 'section' in html source, after finding a new 'series'
                        print(f"[ ] __ found '{line_series_name_safe}' animated mini backgrounds...")
                    _animated_mini_backgrounds = line_series_name_safe

                if 'class="sm:h-[148px] md:h-[130px] lg:h-[112px] xl:h-[123px] 2xl:h-[132px]"' in line:
                    animated_minibg_number = animated_minibg_number + 1
                    animated_minibg_name = GetListOfSubstrings(line, 'alt="', '"')[0]
                    animated_minibg_name = animated_minibg_name.replace('&amp;', '&').replace("&apos;", "'").replace('&quot;', '"').replace('&nbsp;', ' ')
                    animated_minibg_name_safe = safe_name.create_safe_name(animated_minibg_name)
                    animated_minibg_static_large = GetListOfSubstrings(line, '<img loading="lazy" src="', '?size=280x250f"')[0]
                    animated_minibg_static_small = GetListOfSubstrings(line, '<img loading="lazy" src="', '"')[0]

                    ini_animated_minibg = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_animated_mini_backgrounds.ini"), encoding='utf-8', create_empty=True)
                    ini_animated_minibg['animated_mini_backgrounds'][f'background{ParseNumber(animated_minibg_number)}_name'] = animated_minibg_name.strip('"')
                    ini_animated_minibg['animated_mini_backgrounds'][f'background{ParseNumber(animated_minibg_number)}_static_jpg'] = animated_minibg_static_large
                    ini_animated_minibg.write()

                    ReplaceStringInFile(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_animated_mini_backgrounds.ini"), ' = "', '"', '')

                    if not os.path.exists(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Animated Mini Backgrounds\\static")):
                        os.makedirs(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Animated Mini Backgrounds\\static"))

                    try:
                        response_jpg = requests.get(animated_minibg_static_large)
                        with open(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Animated Mini Backgrounds\\static\\{ParseNumber(animated_minibg_number)}. {animated_minibg_name_safe} _static.jpg"), "wb") as f:
                            f.write(response_jpg.content)
                    except Exception as e:
                        print(f"[X] __ Error downloading from '{animated_minibg_static_large}'", file=sys.stderr)
                        traceback.print_exception(e, file=sys.stderr)

                    if not os.path.exists(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Animated Mini Backgrounds\\thumbs")):
                        os.makedirs(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Animated Mini Backgrounds\\thumbs"))

                    try:
                        response_jpg = requests.get(animated_minibg_static_small)
                        with open(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Animated Mini Backgrounds\\thumbs\\{ParseNumber(animated_minibg_number)}. {animated_minibg_name_safe} _small.jpg"), "wb") as f:
                            f.write(response_jpg.content)
                    except Exception as e:
                        print(f"[X] __ Error downloading from '{animated_minibg_static_small}'", file=sys.stderr)
                        traceback.print_exception(e, file=sys.stderr)
                
                elif 'type="video/webm"' in line:
                    animated_minibg_link_webm = GetListOfSubstrings(line, '<source src="', '"')[0]

                    ini_animated_minibg = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_animated_mini_backgrounds.ini"), encoding='utf-8', create_empty=True)
                    ini_animated_minibg['animated_mini_backgrounds'][f'background{ParseNumber(animated_minibg_number)}_anim_webm'] = animated_minibg_link_webm
                    ini_animated_minibg.write()
                
                elif 'type="video/mp4"' in line:
                    animated_minibg_link_mp4 = GetListOfSubstrings(line, '<source src="', '"')[0]

                    ini_animated_minibg = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_animated_mini_backgrounds.ini"), encoding='utf-8', create_empty=True)
                    ini_animated_minibg['animated_mini_backgrounds'][f'background{ParseNumber(animated_minibg_number)}_anim_mp4'] = animated_minibg_link_mp4
                    ini_animated_minibg.write()

                    if not os.path.exists(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Animated Mini Backgrounds")):
                        os.makedirs(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Animated Mini Backgrounds"))

                    try:
                        response_mp4 = requests.get(animated_minibg_link_mp4)
                        with open(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Animated Mini Backgrounds\\{ParseNumber(animated_minibg_number)}. {animated_minibg_name_safe} _.mp4"), "wb") as f:
                            f.write(response_mp4.content)
                    except Exception as e:
                        print(f"[X] __ Error downloading from '{animated_minibg_link_mp4}'", file=sys.stderr)
                        traceback.print_exception(e, file=sys.stderr)
            
            elif line_section == "Avatar Frames":

                if _avatar_frames != line_series_name_safe:
                    if _avatar_frames_series == line_series_count: # this fixes duplicating message for last found 'section' in html source, after finding a new 'series'
                        print(f"[ ] __ found '{line_series_name_safe}' avatar frames...")
                    _avatar_frames = line_series_name_safe

                if '>Animation<' in line:
                    avatar_frame_count = avatar_frame_count + 1
                    avatar_frame_png = GetListOfSubstrings(line, '<a href="', '"')[0]

                    ini_avatar_frames = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_avatar_frames.ini"), encoding='utf-8', create_empty=True)
                    ini_avatar_frames['avatar_frames'][f'frame{ParseNumber(avatar_frame_count)}_anim_png'] = avatar_frame_png
                    ini_avatar_frames.write()

                elif '>Static<' in line:
                    avatar_frame_static_count = avatar_frame_static_count + 1
                    avatar_frame_static_png = GetListOfSubstrings(line, '<a href="', '"')[0]

                    ini_avatar_frames = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_avatar_frames.ini"), encoding='utf-8', create_empty=True)
                    ini_avatar_frames['avatar_frames'][f'frame{ParseNumber(avatar_frame_static_count)}_static_png'] = avatar_frame_static_png
                    ini_avatar_frames.write()

                elif 'class="text-sm text-center break-words"' in line:
                    avatar_frame_name = GetListOfSubstrings(line, '>', '<')[0]
                    avatar_frame_name = avatar_frame_name.replace('&amp;', '&').replace("&apos;", "'").replace('&quot;', '"').replace('&nbsp;', ' ')
                    avatar_frame_name_safe = safe_name.create_safe_name(avatar_frame_name)

                    ini_avatar_frames = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_avatar_frames.ini"), encoding='utf-8', create_empty=True)
                    ini_avatar_frames['avatar_frames'][f'frame{ParseNumber(avatar_frame_count)}_name'] = avatar_frame_name.strip('"')
                    ini_avatar_frames.write()

                    ReplaceStringInFile(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_avatar_frames.ini"), ' = "', '"', '')

                    if not os.path.exists(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Avatar Frames")):
                        os.makedirs(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Avatar Frames"))

                    try:
                        response_png = requests.get(avatar_frame_png)
                        with open(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Avatar Frames\\{ParseNumber(avatar_frame_count)}. {avatar_frame_name_safe} _.png"), "wb") as f:
                            f.write(response_png.content)
                    except Exception as e:
                        print(f"[X] __ Error downloading from '{avatar_frame_png}'", file=sys.stderr)
                        traceback.print_exception(e, file=sys.stderr)

                    if not os.path.exists(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Avatar Frames\\static")):
                        os.makedirs(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Avatar Frames\\static"))

                    try:
                        response_png = requests.get(avatar_frame_static_png)
                        with open(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Avatar Frames\\static\\{ParseNumber(avatar_frame_count)}. {avatar_frame_name_safe} _static.png"), "wb") as f:
                            f.write(response_png.content)
                    except Exception as e:
                        print(f"[X] __ Error downloading from '{avatar_frame_static_png}'", file=sys.stderr)
                        traceback.print_exception(e, file=sys.stderr)

            elif line_section == "Animated Avatars":

                if _animated_avatars != line_series_name_safe:
                    if _animated_avatars_series == line_series_count: # this fixes duplicating message for last found 'section' in html source, after finding a new 'series'
                        print(f"[ ] __ found '{line_series_name_safe}' animated avatars...")
                    _animated_avatars = line_series_name_safe

                if '>Animation<' in line:
                    animated_avatar_count = animated_avatar_count + 1
                    animated_avatar_gif = GetListOfSubstrings(line, '<a href="', '"')[0]

                    ini_animated_avatars = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_animated_avatars.ini"), encoding='utf-8', create_empty=True)
                    ini_animated_avatars['animated_avatars'][f'avatar{ParseNumber(animated_avatar_count)}_anim_gif'] = animated_avatar_gif
                    ini_animated_avatars.write()

                elif '>Static<' in line:
                    animated_avatar_static_count = animated_avatar_static_count + 1
                    animated_avatar_jpg = GetListOfSubstrings(line, '<a href="', '"')[0]

                    ini_animated_avatars = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_animated_avatars.ini"), encoding='utf-8', create_empty=True)
                    ini_animated_avatars['animated_avatars'][f'avatar{ParseNumber(animated_avatar_static_count)}_static_jpg'] = animated_avatar_jpg
                    ini_animated_avatars.write()

                elif 'class="text-sm text-center break-words"' in line:
                    animated_avatar_name = GetListOfSubstrings(line, '>', '<')[0]
                    animated_avatar_name = animated_avatar_name.replace('&amp;', '&').replace("&apos;", "'").replace('&quot;', '"').replace('&nbsp;', ' ')
                    animated_avatar_name_safe = safe_name.create_safe_name(animated_avatar_name)

                    ini_animated_avatars = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_animated_avatars.ini"), encoding='utf-8', create_empty=True)
                    ini_animated_avatars['animated_avatars'][f'avatar{ParseNumber(animated_avatar_count)}_name'] = animated_avatar_name.strip('"')
                    ini_animated_avatars.write()

                    ReplaceStringInFile(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_animated_avatars.ini"), ' = "', '"', '')

                    if not os.path.exists(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Animated Avatars")):
                        os.makedirs(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Animated Avatars"))

                    try:
                        response_gif = requests.get(animated_avatar_gif)
                        with open(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Animated Avatars\\{ParseNumber(animated_avatar_count)}. {animated_avatar_name_safe} _.gif"), "wb") as f:
                            f.write(response_gif.content)
                    except Exception as e:
                        print(f"[X] __ Error downloading from '{animated_avatar_gif}'", file=sys.stderr)
                        traceback.print_exception(e, file=sys.stderr)

                    if not os.path.exists(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Animated Avatars\\static")):
                        os.makedirs(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Animated Avatars\\static"))

                    try:
                        response_jpg = requests.get(animated_avatar_jpg)
                        with open(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Animated Avatars\\static\\{ParseNumber(animated_avatar_count)}. {animated_avatar_name_safe} _static.jpg"), "wb") as f:
                            f.write(response_jpg.content)
                    except Exception as e:
                        print(f"[X] __ Error downloading from '{animated_avatar_jpg}'", file=sys.stderr)
                        traceback.print_exception(e, file=sys.stderr)

            elif line_section == "Profiles":

                if _profiles != line_series_name_safe:
                    if _profiles_series == line_series_count: # this fixes duplicating message for last found 'section' in html source, after finding a new 'series'
                        print(f"[ ] __ found '{line_series_name_safe}' profiles...")
                    _profiles = line_series_name_safe

                if 'class="sm:h-[166px] md:h-[146px] lg:h-[126px] xl:h-[138px] 2xl:h-[148px]"' in line:
                    profile_count = profile_count + 1
                    profile_jpg = GetListOfSubstrings(line, 'src="', '"')[0]
                    profile_name = GetListOfSubstrings(line, 'alt="', '"')[0]
                    profile_name = profile_name.replace('&amp;', '&').replace("&apos;", "'").replace('&quot;', '"').replace('&nbsp;', ' ')
                    profile_name_safe = safe_name.create_safe_name(profile_name)

                    ini_profiles = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_profiles.ini"), encoding='utf-8', create_empty=True)
                    ini_profiles['profiles'][f'profile{ParseNumber(profile_count)}_name'] = profile_name.strip('"')
                    ini_profiles['profiles'][f'profile{ParseNumber(profile_count)}_jpg'] = profile_jpg
                    ini_profiles.write()

                    ReplaceStringInFile(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_profiles.ini"), ' = "', '"', '')

                    if not os.path.exists(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Profiles")):
                        os.makedirs(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Profiles"))

                    try:
                        response_jpg = requests.get(profile_jpg)
                        with open(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe} _Download\\Profiles\\{ParseNumber(profile_count)}. {profile_name_safe} _.jpg"), "wb") as f:
                            f.write(response_jpg.content)
                    except Exception as e:
                        print(f"[X] __ Error downloading from '{profile_jpg}'", file=sys.stderr)
                        traceback.print_exception(e, file=sys.stderr)

                elif '>Preview<' in line:
                    profile_preview = GetListOfSubstrings(line, '<a href="', '"')[0]

                    ini_profiles = ConfigObj(os.path.join(base_out_dir, f"steam_misc\\app_scx\\{line_series_name_safe}\\app_profiles.ini"), encoding='utf-8', create_empty=True)
                    ini_profiles['profiles'][f'profile{ParseNumber(profile_count)}_preview'] = profile_preview
                    ini_profiles.write()

            #line_number = line_number + 1 # previous line number, unused

        if _game_found:
            os.remove(os.path.join(base_out_dir, 'steam_misc\\app_scx\\app_scx.txt'))
            with open(os.path.join(base_out_dir, f'steam_misc\\app_scx\\{appid}_s.txt'), 'r') as file:
                lines = []
                for line in file:
                    lines.append(line)
                    if os.path.isdir(os.path.join(base_out_dir, f'steam_misc\\app_scx\\{line.rstrip()}')):
                        if os.listdir(os.path.join(base_out_dir, f'steam_misc\\app_scx\\{line.rstrip()}')): # zip folder only if not empty
                            shutil.make_archive(os.path.join(base_out_dir, f'steam_misc\\app_scx\\{line.rstrip()}'), 'zip', os.path.join(base_out_dir, f'steam_misc\\app_scx\\{line.rstrip()}')) # first argument is the name of the zip file
                            shutil.rmtree(os.path.join(base_out_dir, f'steam_misc\\app_scx\\{line.rstrip()}'))
                            shutil.move(os.path.join(base_out_dir, f'steam_misc\\app_scx\\{line.rstrip()} _Download'), os.path.join(base_out_dir, f'steam_misc\\app_scx\\{line.rstrip()}'))
                            shutil.move(os.path.join(base_out_dir, f'steam_misc\\app_scx\\{line.rstrip()}.zip'), os.path.join(base_out_dir, f'steam_misc\\app_scx\\{line.rstrip()}\\{line.rstrip()}.zip'))
