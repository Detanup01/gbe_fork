import os
import re
import sys
import traceback
import json
import queue
import threading
import time
import shutil
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
                print(f"[X] __ Error downloading from '{url}'", file=sys.stderr)
                traceback.print_exception(e, file=sys.stderr)

            time.sleep(0.1)

        q.task_done()

def __remove_url_query(url : str) -> str:
    url_parts = urllib.parse.urlsplit(url)
    url_parts_list = list(url_parts)
    url_parts_list[3] = '' # remove query
    return str(urllib.parse.urlunsplit(url_parts_list))

def __download_screenshots(base_out_dir : str, appid : int, app_details : dict ):
    screenshots : list[dict[str, object]] = app_details.get(f'{appid}', {}).get('data', {}).get('screenshots', [])
    if not screenshots:
        print(f'[?] No screenshots found - nothing downloaded')   
        return
    
    screenshot_number = len(screenshots)
    if screenshot_number > 1:
        print(f"[ ] Found {screenshot_number} app screenshots --- downloading...")
    else:
        print(f"[ ] Found {screenshot_number} app screenshot --- downloading...")
    
    screenshots_out_dir = os.path.join(base_out_dir, "steam_misc\\app_screens")
    thumbnails_out_dir = os.path.join(screenshots_out_dir, "thumbs")

    if not os.path.exists(screenshots_out_dir):
        os.makedirs(screenshots_out_dir)
        time.sleep(0.025)
    if not os.path.exists(thumbnails_out_dir):
        os.makedirs(thumbnails_out_dir)
        time.sleep(0.025)

    q : queue.Queue[tuple[str, str]] = queue.Queue()

    max_threads = 20
    for i in range(max_threads):
        threading.Thread(target=__downloader_thread, args=(q,), daemon=True).start()

    for scrn in screenshots:
        if screenshot_number <= 9:
            screenshot_number_str = str(0) + str(screenshot_number)
        else:
            screenshot_number_str = str(screenshot_number)

        full_image_url = scrn.get('path_full', None)
        if full_image_url:
            full_image_url_sanitized = __remove_url_query(full_image_url)
            image_hash_name = f'{full_image_url_sanitized.rsplit("/", 1)[-1]}'.rstrip()
            if image_hash_name:
                q.put((full_image_url_sanitized, os.path.join(screenshots_out_dir, "screenshot " + screenshot_number_str + ".jpg")))
            else:
                print(f'[X] __ url: "{full_image_url}"')
                print(f'[X] ____ Cannot download screenshot from url - failed to get image name')
        thumbnail_url = scrn.get('path_thumbnail', None)
        if thumbnail_url:
            thumbnail_url_sanitized = __remove_url_query(thumbnail_url)
            image_hash_name = f'{thumbnail_url_sanitized.rsplit("/", 1)[-1]}'.rstrip()
            if image_hash_name:
                q.put((thumbnail_url_sanitized, os.path.join(thumbnails_out_dir, "screenshot " + screenshot_number_str + " _ small.jpg")))
            else:
                print(f'[X] __ url: "{thumbnail_url}"')
                print(f'[X] ____ Cannot download screenshot thumbnail from url - failed to get image name')

        screenshot_number = screenshot_number - 1
     
    q.join()

    for i in range(max_threads):
        q.put((None, None))
    
    q.join()
        
    #print(f"[ ] Finished downloading app screenshots")

PREFERED_VIDS = ['trailer', 'gameplay', 'announcement']
PREFERED_VIDS_active = 0

def __download_videos(base_out_dir : str, appid : int, app_details : dict, download_low : bool, download_max : bool):
    videos : list[dict[str, object]] = app_details.get(f'{appid}', {}).get('data', {}).get('movies', [])
    if not videos:
        print(f'[?] No app videos found - nothing downloaded')
        return
    
    videos_out_dir = os.path.join(base_out_dir, "steam_misc\\app_videos")

    video_number = len(videos)
    if video_number > 1:
        print(f"[ ] Found {video_number} app videos --- downloading...")
    else:
        print(f"[ ] Found {video_number} app video --- downloading...")

    video_number_low = video_number
    video_number_max = video_number

    first_vid : tuple[str, str] = None
    prefered_vid : tuple[str, str] = None
    
    for vid in videos:
        vid_name = f"{vid.get('name', '')}"
        thumb_url = f"{vid.get('thumbnail', '')}"

        vid_name_low = vid_name
        vid_name_max = vid_name

        if download_low == True:
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
            else: # no url found
                print(f'[X] __ No url found for video "{vid_name}"')
                continue

            if video_number_low <= 9:
                video_number_low_str = str(0) + str(video_number_low)
            else:
                video_number_low_str = str(video_number_low)
        
            vid_url_sanitized = __remove_url_query(prefered_url)
            vid_name_in_url = f'{vid_url_sanitized.rsplit("/", 1)[-1]}'.rstrip()
            vid_name_low = safe_name.create_safe_name(vid_name_low)
            if vid_name_low:
                vid_name_orig = f'{vid_name}.{ext}'
                vid_name_low = video_number_low_str + ". " + f'{vid_name_low} _low_res.{ext}'
            else:
                vid_name_orig = vid_name_in_url
                vid_name_low = video_number_low_str + ". " + vid_name_in_url

            video_download = 0

            if vid_name_low:
                if not first_vid:
                    first_vid = (vid_url_sanitized, vid_name_low)

                if PREFERED_VIDS_active == 1:
                    if any(vid_name_low.lower().find(candidate) > -1 for candidate in PREFERED_VIDS):
                        prefered_vid = (vid_url_sanitized, vid_name_low)
                        video_number_low = video_number_low - 1
                        video_download = 1
                else:
                    prefered_vid = (vid_url_sanitized, vid_name_low)
                    video_number_low = video_number_low - 1
                    video_download = 1

            if video_download == 1:

                if not os.path.exists(videos_out_dir):
                    os.makedirs(videos_out_dir)
                    time.sleep(0.05)

                q : queue.Queue[tuple[str, str]] = queue.Queue()

                max_threads = 1
                for i in range(max_threads):
                    threading.Thread(target=__downloader_thread, args=(q,), daemon=True).start()

                # download all videos
                #print(f'[ ] __ downloading video: "{vid_name_orig}"')
                print(f'[ ] __ downloading low_res video: "{vid_name_orig}"')
                q.put((prefered_vid[0], os.path.join(videos_out_dir, prefered_vid[1])))
                q.join()

                for i in range(max_threads):
                    q.put((None, None))
                        
                q.join()

            else:
                print(f'[X] __ url: "{prefered_url}"')
                print(f'[X] ____ Cannot download video from url - failed to get video name')

        if download_max == True:
            webm_url = vid.get('webm', {}).get("max", None)
            mp4_url = vid.get('mp4', {}).get("max", None)

            ext : str = None
            prefered_url : str = None
            if mp4_url:
                prefered_url = mp4_url
                ext = 'mp4'
            elif webm_url:
                prefered_url = webm_url
                ext = 'webm'
            else: # no url found
                print(f'[X] __ No url found for video "{vid_name}"')
                continue

            if video_number_max <= 9:
                video_number_max_str = str(0) + str(video_number_max)
            else:
                video_number_max_str = str(video_number_max)
        
            vid_url_sanitized = __remove_url_query(prefered_url)
            vid_name_in_url = f'{vid_url_sanitized.rsplit("/", 1)[-1]}'.rstrip()
            vid_name_max = safe_name.create_safe_name(vid_name_max)
            if vid_name_max:
                vid_name_orig = f'{vid_name}.{ext}'
                vid_name_max = video_number_max_str + ". " + f'{vid_name_max} _max_res.{ext}'
            else:
                vid_name_orig = vid_name_in_url
                vid_name_max = video_number_max_str + ". " + vid_name_in_url

            video_download = 0

            if vid_name_max:
                if not first_vid:
                    first_vid = (vid_url_sanitized, vid_name_max)

                if PREFERED_VIDS_active == 1:
                    if any(vid_name_max.lower().find(candidate) > -1 for candidate in PREFERED_VIDS):
                        prefered_vid = (vid_url_sanitized, vid_name_max)
                        video_number_max = video_number_max - 1
                        video_download = 1
                else:
                    prefered_vid = (vid_url_sanitized, vid_name_max)
                    video_number_max = video_number_max - 1
                    video_download = 1

            if video_download == 1:

                if not os.path.exists(videos_out_dir):
                    os.makedirs(videos_out_dir)
                    time.sleep(0.05)

                q : queue.Queue[tuple[str, str]] = queue.Queue()

                max_threads = 1
                for i in range(max_threads):
                    threading.Thread(target=__downloader_thread, args=(q,), daemon=True).start()

                # download all videos
                #print(f'[ ] __ downloading video: "{vid_name_orig}"')
                print(f'[ ] __ downloading max_res video: "{vid_name_orig}"')
                q.put((prefered_vid[0], os.path.join(videos_out_dir, prefered_vid[1])))
                q.join()

                for i in range(max_threads):
                    q.put((None, None))
                        
                q.join()

            else:
                print(f'[X] __ url: "{prefered_url}"')
                print(f'[X] ____ Cannot download video from url - failed to get video name')

        # NOTE some video thumbnails don't get numbered properly - no idea why that happens, as they should have the same number and name as the video... better to disable it now
        # LOL, if I'm gonna duplicate code, at least make sure it's the same in both instances next time (I was using 'if video_number <= 9:' instead of 'if video_number_max <= 9:')
        if not os.path.exists(os.path.join(videos_out_dir, "thumbs")):
            os.makedirs(os.path.join(videos_out_dir, "thumbs"))
            time.sleep(0.05)

        if thumb_url and prefered_vid:
            response = requests.get(thumb_url, stream=True)
            with open(os.path.join(videos_out_dir, "thumbs\\", prefered_vid[1].replace(' _low_res', '').replace(' _max_res', '').strip("." + ext) + " _ small.jpg"),'wb') as thumb_file:
                thumb_file.write(response.content)
        
        ''' # NOTE if enabled, only first found video is downloaded
        if prefered_vid:
            break
        '''
    
    if not first_vid and not prefered_vid:
        print(f'[X] __ No video url found')
        return
        
    #print(f"[ ] Finished downloading app videos")

def download_app_details(
    base_out_dir : str,
    info_out_dir : str,
    appid : int,
    download_screenshots : bool,
    download_vids : bool,
    downl_low : bool,
    downl_max : bool):

    details_out_file = os.path.join(info_out_dir, "app_details.json")

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
        print(f"[?] No app details found - skip creating <OUT_DIR>\\steam_misc\\app_info\\app_details.json")
        #if last_exception: # skip showing last_exception
        #    print(f"[X] __ last error: {last_exception}")
        return

    with open(details_out_file, "wt", encoding='utf-8') as f:
        json.dump(app_details, f, ensure_ascii=False, indent=2)
        print(f"[ ] Found app details --- writing to <OUT_DIR>\\steam_misc\\app_info\\app_details.json") # move it here to avoid showing both 'downloading' and 'cannot download'
    
    if download_screenshots:
        __download_screenshots(base_out_dir, appid, app_details)

        if os.path.isdir(os.path.join(base_out_dir, 'steam_misc\\app_screens\\thumbs')):
            if os.listdir(os.path.join(base_out_dir, 'steam_misc\\app_screens\\thumbs')): # zip 'thumbs' folder only if not empty
                shutil.make_archive(os.path.join(base_out_dir, 'steam_misc\\app_screens\\thumbs'), 'zip', os.path.join(base_out_dir, 'steam_misc\\app_screens\\thumbs')) # first argument is the name of the zip file
                shutil.rmtree(os.path.join(base_out_dir, 'steam_misc\\app_screens\\thumbs'))
                os.makedirs(os.path.join(base_out_dir, 'steam_misc\\app_screens\\thumbs'))
                shutil.move(os.path.join(base_out_dir, 'steam_misc\\app_screens\\thumbs.zip'), os.path.join(base_out_dir, 'steam_misc\\app_screens\\thumbs\\thumbs.zip'))

    if download_vids:
        __download_videos(base_out_dir, appid, app_details, downl_low, downl_max)

        # NOTE some video thumbnails don't get numbered properly - no idea why that happens, as they should have the same number and name as the video... better to disable it now
        if os.path.isdir(os.path.join(base_out_dir, 'steam_misc\\app_videos\\thumbs')):
            if os.listdir(os.path.join(base_out_dir, 'steam_misc\\app_videos\\thumbs')): # zip 'thumbs' folder only if not empty
                shutil.make_archive(os.path.join(base_out_dir, 'steam_misc\\app_videos\\thumbs'), 'zip', os.path.join(base_out_dir, 'steam_misc\\app_videos\\thumbs')) # first argument is the name of the zip file
                shutil.rmtree(os.path.join(base_out_dir, 'steam_misc\\app_videos\\thumbs'))
                os.makedirs(os.path.join(base_out_dir, 'steam_misc\\app_videos\\thumbs'))
                shutil.move(os.path.join(base_out_dir, 'steam_misc\\app_videos\\thumbs.zip'), os.path.join(base_out_dir, 'steam_misc\\app_videos\\thumbs\\thumbs.zip'))
        
