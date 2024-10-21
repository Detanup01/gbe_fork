import os
import urllib.request

# download PCGamingWiki page source code only
# to be parsed for additional game info, e.g. config and saved games files locations, extended developer and publisher info, etc
# not yet implemented in main script
def download_pcgw(base_out_dir : str, appid : int):

    pcgw_link = f"https://www.pcgamingwiki.com/api/appid.php?appid={appid}"

    if not os.path.exists(os.path.join(base_out_dir, 'steam_misc\\app_info')):
        os.makedirs(os.path.join(base_out_dir, 'steam_misc\\app_info'))

    with urllib.request.urlopen(pcgw_link) as f:
        html = f.read().decode('utf-8')

    file = os.path.join(base_out_dir, f"steam_misc\\app_info\\pcgw.tmp")
    with open(file, 'w', encoding='utf-8') as f:
        f.write(html)

    if os.path.isfile(os.path.join(base_out_dir, 'steam_misc\\app_info\\pcgw.tmp')):
        with open(os.path.join(base_out_dir, 'steam_misc\\app_info\\pcgw.tmp'), 'r', encoding='utf-8') as app_pcgw:
            app_pcgw_line = app_pcgw.readlines()
