import os
import threading
import time
import requests

def download_app_images(
    base_out_dir : str,
    appid : int,
    clienticon : str,
    icon : str,
    logo : str,
    logo_small : str):

    icons_out_dir = os.path.join(base_out_dir, "steam_misc", "app_images")

    def downloader_thread(image_name : str, image_url : str):
        # try 3 times
        for download_trial in range(3):
            try:
                r = requests.get(image_url)
                if r.status_code == requests.codes.ok: # if download was successfull
                    with open(os.path.join(icons_out_dir, image_name), "wb") as f:
                        f.write(r.content)
                    break
            except Exception as ex:
                pass

            time.sleep(0.1)

    app_images_names = [
        r'broadcast_left_panel.jpg',
        r'broadcast_right_panel.jpg',
        r'capsule_231x87.jpg',
        r'capsule_231x87_alt_assets_0.jpg',
        r'capsule_231x87_alt_assets_1.jpg',
        r'capsule_231x87_alt_assets_2.jpg',
        r'capsule_231x87_alt_assets_3.jpg',
        r'capsule_231x87_alt_assets_4.jpg',
        r'capsule_231x87_alt_assets_5.jpg',
        r'capsule_231x87_alt_assets_6.jpg',
        r'capsule_231x87_alt_assets_7.jpg',
        r'capsule_231x87_alt_assets_8.jpg',
        r'capsule_231x87_alt_assets_9.jpg',
        r'capsule_467x181.jpg',
        r'capsule_467x181_alt_assets_0.jpg',
        r'capsule_467x181_alt_assets_1.jpg',
        r'capsule_467x181_alt_assets_2.jpg',
        r'capsule_467x181_alt_assets_3.jpg',
        r'capsule_467x181_alt_assets_4.jpg',
        r'capsule_467x181_alt_assets_5.jpg',
        r'capsule_467x181_alt_assets_6.jpg',
        r'capsule_467x181_alt_assets_7.jpg',
        r'capsule_467x181_alt_assets_8.jpg',
        r'capsule_467x181_alt_assets_9.jpg',
        r'capsule_616x353.jpg',
        r'capsule_616x353_alt_assets_0.jpg',
        r'capsule_616x353_alt_assets_1.jpg',
        r'capsule_616x353_alt_assets_2.jpg',
        r'capsule_616x353_alt_assets_3.jpg',
        r'capsule_616x353_alt_assets_4.jpg',
        r'capsule_616x353_alt_assets_5.jpg',
        r'capsule_616x353_alt_assets_6.jpg',
        r'capsule_616x353_alt_assets_7.jpg',
        r'capsule_616x353_alt_assets_8.jpg',
        r'capsule_616x353_alt_assets_9.jpg',
        r'header.jpg',
        r'header_alt_assets_0.jpg',
        r'header_alt_assets_1.jpg',
        r'header_alt_assets_2.jpg',
        r'header_alt_assets_3.jpg',
        r'header_alt_assets_4.jpg',
        r'header_alt_assets_5.jpg',
        r'header_alt_assets_6.jpg',
        r'header_alt_assets_7.jpg',
        r'header_alt_assets_8.jpg',
        r'header_alt_assets_9.jpg',
        r'hero_capsule.jpg',
        r'hero_capsule_alt_assets_0.jpg',
        r'hero_capsule_alt_assets_1.jpg',
        r'hero_capsule_alt_assets_2.jpg',
        r'hero_capsule_alt_assets_3.jpg',
        r'hero_capsule_alt_assets_4.jpg',
        r'hero_capsule_alt_assets_5.jpg',
        r'hero_capsule_alt_assets_6.jpg',
        r'hero_capsule_alt_assets_7.jpg',
        r'hero_capsule_alt_assets_8.jpg',
        r'hero_capsule_alt_assets_9.jpg',
        r'library_600x900.jpg',
        r'library_600x900_alt_assets_0.jpg',
        r'library_600x900_alt_assets_1.jpg',
        r'library_600x900_alt_assets_2.jpg',
        r'library_600x900_alt_assets_3.jpg',
        r'library_600x900_alt_assets_4.jpg',
        r'library_600x900_alt_assets_5.jpg',
        r'library_600x900_alt_assets_6.jpg',
        r'library_600x900_alt_assets_7.jpg',
        r'library_600x900_alt_assets_8.jpg',
        r'library_600x900_alt_assets_9.jpg',
        r'library_600x900_2x.jpg',
        r'library_600x900_2x_alt_assets_0.jpg',
        r'library_600x900_2x_alt_assets_1.jpg',
        r'library_600x900_2x_alt_assets_2.jpg',
        r'library_600x900_2x_alt_assets_3.jpg',
        r'library_600x900_2x_alt_assets_4.jpg',
        r'library_600x900_2x_alt_assets_5.jpg',
        r'library_600x900_2x_alt_assets_6.jpg',
        r'library_600x900_2x_alt_assets_7.jpg',
        r'library_600x900_2x_alt_assets_8.jpg',
        r'library_600x900_2x_alt_assets_9.jpg',
        r'library_hero.jpg',
        r'library_hero_alt_assets_0.jpg',
        r'library_hero_alt_assets_1.jpg',
        r'library_hero_alt_assets_2.jpg',
        r'library_hero_alt_assets_3.jpg',
        r'library_hero_alt_assets_4.jpg',
        r'library_hero_alt_assets_5.jpg',
        r'library_hero_alt_assets_6.jpg',
        r'library_hero_alt_assets_7.jpg',
        r'library_hero_alt_assets_8.jpg',
        r'library_hero_alt_assets_9.jpg',
        r'page_bg_raw.jpg',
        r'page_bg_raw_alt_assets_0.jpg',
        r'page_bg_raw_alt_assets_1.jpg',
        r'page_bg_raw_alt_assets_2.jpg',
        r'page_bg_raw_alt_assets_3.jpg',
        r'page_bg_raw_alt_assets_4.jpg',
        r'page_bg_raw_alt_assets_5.jpg',
        r'page_bg_raw_alt_assets_6.jpg',
        r'page_bg_raw_alt_assets_7.jpg',
        r'page_bg_raw_alt_assets_8.jpg',
        r'page_bg_raw_alt_assets_9.jpg',
        r'page_bg_generated.jpg',
        r'page_bg_generated_alt_assets_0.jpg',
        r'page_bg_generated_alt_assets_1.jpg',
        r'page_bg_generated_alt_assets_2.jpg',
        r'page_bg_generated_alt_assets_3.jpg',
        r'page_bg_generated_alt_assets_4.jpg',
        r'page_bg_generated_alt_assets_5.jpg',
        r'page_bg_generated_alt_assets_6.jpg',
        r'page_bg_generated_alt_assets_7.jpg',
        r'page_bg_generated_alt_assets_8.jpg',
        r'page_bg_generated_alt_assets_9.jpg',
        r'page_bg_generated_v6b.jpg',
        r'page_bg_generated_v6b_alt_assets_0.jpg',
        r'page_bg_generated_v6b_alt_assets_1.jpg',
        r'page_bg_generated_v6b_alt_assets_2.jpg',
        r'page_bg_generated_v6b_alt_assets_3.jpg',
        r'page_bg_generated_v6b_alt_assets_4.jpg',
        r'page_bg_generated_v6b_alt_assets_5.jpg',
        r'page_bg_generated_v6b_alt_assets_6.jpg',
        r'page_bg_generated_v6b_alt_assets_7.jpg',
        r'page_bg_generated_v6b_alt_assets_8.jpg',
        r'page_bg_generated_v6b_alt_assets_9.jpg',
        r'logo.png'
    ]

    if not os.path.exists(icons_out_dir):
        os.makedirs(icons_out_dir)
        time.sleep(0.050)

    threads_list : list[threading.Thread] = []
    for image_name in app_images_names:
        image_url = f'https://cdn.cloudflare.steamstatic.com/steam/apps/{appid}/{image_name}'
        t = threading.Thread(target=downloader_thread, args=(image_name, image_url), daemon=True)
        threads_list.append(t)
        t.start()
    
    community_images_url = f'https://cdn.cloudflare.steamstatic.com/steamcommunity/public/images/apps/{appid}'
    if clienticon:
        image_url = f'{community_images_url}/{clienticon}.ico'
        t = threading.Thread(target=downloader_thread, args=('clienticon.ico', image_url), daemon=True)
        threads_list.append(t)
        t.start()

    if icon:
        image_url = f'{community_images_url}/{icon}.jpg'
        t = threading.Thread(target=downloader_thread, args=('icon.jpg', image_url), daemon=True)
        threads_list.append(t)
        t.start()

    if logo:
        image_url = f'{community_images_url}/{logo}.jpg'
        t = threading.Thread(target=downloader_thread, args=('logo.jpg', image_url), daemon=True)
        threads_list.append(t)
        t.start()

    if logo_small:
        image_url = f'{community_images_url}/{logo_small}.jpg'
        t = threading.Thread(target=downloader_thread, args=('logo_small.jpg', image_url), daemon=True)
        threads_list.append(t)
        t.start()

    print(f"[ ] Downloading app art images...")

    for t in threads_list:
        t.join()
