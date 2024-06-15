import os
import sys
import traceback
import json
import queue
import threading
import time
import requests
import urllib.parse
from external_components import (
    safe_name
)

def __downloader_thread(q : queue.Queue[tuple[str, str]]):
    while True:
        url, path = q.get()
        if not url:
            q.task_done()
            return

        # try 3 times
        for download_trial in range(3):
            try:
                r = requests.get(url)
                r.raise_for_status()
                if r.status_code == requests.codes.ok: # if download was successfull
                    with open(path, "wb") as f:
                        f.write(r.content)

                    break
            except Exception as e:
                print(f"Error downloading from '{url}'", file=sys.stderr)
                traceback.print_exception(e, file=sys.stderr)

            time.sleep(0.1)
        
        q.task_done()

def __remove_url_query(url : str) -> str:
    url_parts = urllib.parse.urlsplit(url)
    url_parts_list = list(url_parts)
    url_parts_list[3] = '' # remove query
    return str(urllib.parse.urlunsplit(url_parts_list))

def __download_screenshots(
        base_out_dir : str,
        appid : int,
        app_details : dict,
        download_screenshots : bool,
        download_thumbnails : bool):
    if not download_screenshots and not download_thumbnails:
        return

    screenshots : list[dict[str, object]] = app_details.get(f'{appid}', {}).get('data', {}).get('screenshots', [])
    if not screenshots:
        print(f'[?] no screenshots or thumbnails are available')
        return
    
    screenshots_out_dir = os.path.join(base_out_dir, "screenshots")
    if download_screenshots:
        print(f"downloading screenshots in: {screenshots_out_dir}")
        if not os.path.exists(screenshots_out_dir):
            os.makedirs(screenshots_out_dir)
            time.sleep(0.025)

    thumbnails_out_dir = os.path.join(screenshots_out_dir, "thumbnails")
    if download_thumbnails:
        print(f"downloading screenshots thumbnails in: {thumbnails_out_dir}")
        if not os.path.exists(thumbnails_out_dir):
            os.makedirs(thumbnails_out_dir)
            time.sleep(0.025)

    q : queue.Queue[tuple[str, str]] = queue.Queue()

    max_threads = 20
    for i in range(max_threads):
        threading.Thread(target=__downloader_thread, args=(q,), daemon=True).start()

    for scrn in screenshots:
        if download_screenshots:
            full_image_url = scrn.get('path_full', None)
            if full_image_url:
                full_image_url_sanitized = __remove_url_query(full_image_url)
                image_hash_name = f'{full_image_url_sanitized.rsplit("/", 1)[-1]}'.rstrip()
                if image_hash_name:
                    q.put((full_image_url_sanitized, os.path.join(screenshots_out_dir, image_hash_name)))
                else:
                    print(f'[X] cannot download screenshot from url: "{full_image_url}", failed to get image name')
    
        if download_thumbnails:
            thumbnail_url = scrn.get('path_thumbnail', None)
            if thumbnail_url:
                thumbnail_url_sanitized = __remove_url_query(thumbnail_url)
                image_hash_name = f'{thumbnail_url_sanitized.rsplit("/", 1)[-1]}'.rstrip()
                if image_hash_name:
                    q.put((thumbnail_url_sanitized, os.path.join(thumbnails_out_dir, image_hash_name)))
                else:
                    print(f'[X] cannot download screenshot thumbnail from url: "{thumbnail_url}", failed to get image name')
        
    q.join()

    for i in range(max_threads):
        q.put((None, None))
    
    q.join()
        
    print(f"finished downloading app screenshots")

PREFERED_VIDS = [
    'trailer', 'gameplay', 'announcement'
]

def __download_videos(base_out_dir : str, appid : int, app_details : dict):
    videos : list[dict[str, object]] = app_details.get(f'{appid}', {}).get('data', {}).get('movies', [])
    if not videos:
        print(f'[?] no videos were found')
        return
    
    videos_out_dir = os.path.join(base_out_dir, "videos")
    print(f"downloading app videos in: {videos_out_dir}")

    first_vid : tuple[str, str] = None
    prefered_vid : tuple[str, str] = None
    for vid in videos:
        vid_name = f"{vid.get('name', '')}"
        webm_url = vid.get('webm', {}).get("480", None)
        mp4_url = vid.get('mp4', {}).get("480", None)

        ext : str = None
        prefered_url : str = None
        if mp4_url:
            prefered_url = mp4_url
            ext = 'mp4'
        elif webm_url:
            prefered_url = webm_url
            ext = 'webm'
        else: # no url is found
            print(f'[X] no url is found for video "{vid_name}"')
            continue
    
        vid_url_sanitized = __remove_url_query(prefered_url)
        vid_name_in_url = f'{vid_url_sanitized.rsplit("/", 1)[-1]}'.rstrip()
        vid_name = safe_name.create_safe_name(vid_name)
        if vid_name:
            vid_name = f'{vid_name}.{ext}'
        else:
            vid_name = vid_name_in_url

        if vid_name:
            if not first_vid:
                first_vid = (vid_url_sanitized, vid_name)

            if any(vid_name.lower().find(candidate) > -1 for candidate in PREFERED_VIDS):
                prefered_vid = (vid_url_sanitized, vid_name)

            if prefered_vid:
                break
        else:
            print(f'[X] cannot download video from url: "{prefered_url}", failed to get vido name')
    
    if not first_vid and not prefered_vid:
        print(f'[X] no video url could be found')
        return
    elif not prefered_vid:
        prefered_vid = first_vid

    if not os.path.exists(videos_out_dir):
        os.makedirs(videos_out_dir)
        time.sleep(0.05)

    q : queue.Queue[tuple[str, str]] = queue.Queue()

    max_threads = 1
    for i in range(max_threads):
        threading.Thread(target=__downloader_thread, args=(q,), daemon=True).start()

    # TODO download all videos
    print(f'donwloading video: "{prefered_vid[1]}"')
    q.put((prefered_vid[0], os.path.join(videos_out_dir, prefered_vid[1])))
    q.join()

    for i in range(max_threads):
        q.put((None, None))
    
    q.join()
        
    print(f"finished downloading app videos")


def download_app_details(
    base_out_dir : str,
    info_out_dir : str,
    appid : int,
    download_screenshots : bool,
    download_thumbnails : bool,
    download_vids : bool):

    details_out_file = os.path.join(info_out_dir, "app_details.json")
    print(f"downloading app details in: {details_out_file}")

    app_details : dict = None
    last_exception : Exception | str = None
    # try 3 times
    for download_trial in range(3):
        try:
            r = requests.get(f'http://store.steampowered.com/api/appdetails?appids={appid}&format=json')
            if r.status_code == requests.codes.ok: # if download was successfull
                result : dict = r.json()
                json_ok = result.get(f'{appid}', {}).get('success', False)
                if json_ok:
                    app_details = result
                    break
                else:
                    last_exception = "JSON success was False"
        except Exception as e:
            last_exception = e

        time.sleep(0.1)

    if not app_details:
        err = "[X] failed to download app details"
        if last_exception:
            err += f', last error: "{last_exception}"'
        
        print(err)
        return

    with open(details_out_file, "wt", encoding='utf-8') as f:
        json.dump(app_details, f, ensure_ascii=False, indent=2)
    
    __download_screenshots(base_out_dir, appid, app_details, download_screenshots, download_thumbnails)
        
    if download_vids:
        __download_videos(base_out_dir, appid, app_details)
