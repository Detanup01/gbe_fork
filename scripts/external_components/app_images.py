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

    icons_out_dir = os.path.join(base_out_dir, "images")
    print(f"downloading common app images in: {icons_out_dir}")

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
        r'capsule_184x69.jpg',
        r'capsule_231x87.jpg',
        r'capsule_231x87_alt_assets_0.jpg',
        r'capsule_467x181.jpg',
        r'capsule_616x353.jpg',
        r'capsule_616x353_alt_assets_0.jpg',
        r'library_600x900.jpg',
        r'library_600x900_2x.jpg',
        r'library_hero.jpg',
        r'broadcast_left_panel.jpg',
        r'broadcast_right_panel.jpg',
        r'page.bg.jpg',
        r'page_bg_raw.jpg',
        r'page_bg_generated.jpg',
        r'page_bg_generated_v6b.jpg',
        r'header.jpg',
        r'header_alt_assets_0.jpg',
        r'hero_capsule.jpg',
        r'logo.png',
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

    for t in threads_list:
        t.join()

    print(f"finished downloading common app images")