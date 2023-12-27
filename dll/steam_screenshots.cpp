/* Copyright (C) 2019 Mr Goldberg
   This file is part of the Goldberg Emulator

   The Goldberg Emulator is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   The Goldberg Emulator is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the Goldberg Emulator; if not, see
   <http://www.gnu.org/licenses/>.  */

#include "dll/steam_screenshots.h" 

Steam_Screenshots::Steam_Screenshots(class Local_Storage* local_storage, class SteamCallBacks* callbacks) :
    local_storage(local_storage),
    callbacks(callbacks)
{
}

ScreenshotHandle Steam_Screenshots::create_screenshot_handle()
{
    static ScreenshotHandle handle = 100;
    return handle++;
}

// Writes a screenshot to the user's screenshot library given the raw image data, which must be in RGB format.
// The return value is a handle that is valid for the duration of the game process and can be used to apply tags.
ScreenshotHandle Steam_Screenshots::WriteScreenshot( void *pubRGB, uint32 cubRGB, int nWidth, int nHeight )
{
    PRINT_DEBUG("WriteScreenshot\n");
    
    char buff[128];
    auto now = std::chrono::system_clock::now();
    time_t now_time;
    now_time = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    strftime(buff, 128, "%a_%b_%d_%H_%M_%S_%Y", localtime(&now_time));
    std::string screenshot_name = buff;
    screenshot_name += ".png";

    if (!local_storage->save_screenshot( screenshot_name, (uint8_t*)pubRGB, nWidth, nHeight, 3))
        return INVALID_SCREENSHOT_HANDLE;

    auto handle = create_screenshot_handle();
    auto& infos = _screenshots[handle];
    infos.screenshot_name = buff;

    return handle;
}


// Adds a screenshot to the user's screenshot library from disk.  If a thumbnail is provided, it must be 200 pixels wide and the same aspect ratio
// as the screenshot, otherwise a thumbnail will be generated if the user uploads the screenshot.  The screenshots must be in either JPEG or TGA format.
// The return value is a handle that is valid for the duration of the game process and can be used to apply tags.
// JPEG, TGA, and PNG formats are supported.
ScreenshotHandle Steam_Screenshots::AddScreenshotToLibrary( const char *pchFilename, const char *pchThumbnailFilename, int nWidth, int nHeight )
{
    PRINT_DEBUG("AddScreenshotToLibrary\n");
    
    if (pchFilename == nullptr)
        return INVALID_SCREENSHOT_HANDLE;

    std::vector<image_pixel_t> pixels(std::move(local_storage->load_image(pchFilename)));
    if (pixels.size() != size_t(nWidth) * size_t(nHeight))
        return INVALID_SCREENSHOT_HANDLE;

    char buff[128];
    auto now = std::chrono::system_clock::now();
    time_t now_time;
    now_time = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count();
    strftime(buff, 128, "%a_%b_%d_%H_%M_%S_%Y", localtime(&now_time));
    std::string screenshot_name = buff;
    screenshot_name += ".png";

    if (!local_storage->save_screenshot(screenshot_name, (uint8_t*)pixels.data(), nWidth, nHeight, 4))
        return INVALID_SCREENSHOT_HANDLE;

    auto handle = create_screenshot_handle();
    auto& infos = _screenshots[handle];
    infos.screenshot_name = buff;

    return handle;
}


// Causes the Steam overlay to take a screenshot.  If screenshots are being hooked by the game then a ScreenshotRequested_t callback is sent back to the game instead. 
void Steam_Screenshots::TriggerScreenshot()
{
    PRINT_DEBUG("TriggerScreenshot\n");

    if (hooked)
    {
        ScreenshotRequested_t data;
        callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    }
    else
    {
        PRINT_DEBUG("TODO: Make the overlay take a screenshot");
    }
}


// Toggles whether the overlay handles screenshots when the user presses the screenshot hotkey, or the game handles them.  If the game is hooking screenshots,
// then the ScreenshotRequested_t callback will be sent if the user presses the hotkey, and the game is expected to call WriteScreenshot or AddScreenshotToLibrary
// in response.
void Steam_Screenshots::HookScreenshots( bool bHook )
{
    PRINT_DEBUG("HookScreenshots\n");
    hooked = bHook;
}


// Sets metadata about a screenshot's location (for example, the name of the map)
bool Steam_Screenshots::SetLocation( ScreenshotHandle hScreenshot, const char *pchLocation )
{
    PRINT_DEBUG("SetLocation\n");
    
    auto it = _screenshots.find(hScreenshot);
    if (it == _screenshots.end())
        return false;

    it->second.metadatas["locations"].push_back(pchLocation);
    local_storage->write_json_file(Local_Storage::screenshots_folder, it->second.screenshot_name + ".json", it->second.metadatas);

    return true;
}


// Tags a user as being visible in the screenshot
bool Steam_Screenshots::TagUser( ScreenshotHandle hScreenshot, CSteamID steamID )
{
    PRINT_DEBUG("TagUser\n");
    
    auto it = _screenshots.find(hScreenshot);
    if (it == _screenshots.end())
        return false;

    it->second.metadatas["users"].push_back(uint64_t(steamID.ConvertToUint64()));
    local_storage->write_json_file(Local_Storage::screenshots_folder, it->second.screenshot_name + ".json", it->second.metadatas);

    return true;
}


// Tags a published file as being visible in the screenshot
bool Steam_Screenshots::TagPublishedFile( ScreenshotHandle hScreenshot, PublishedFileId_t unPublishedFileID )
{
    PRINT_DEBUG("TagPublishedFile\n");
    
    auto it = _screenshots.find(hScreenshot);
    if (it == _screenshots.end())
        return false;

    it->second.metadatas["published_files"].push_back(uint64_t(unPublishedFileID));
    local_storage->write_json_file(Local_Storage::screenshots_folder, it->second.screenshot_name + ".json", it->second.metadatas);

    return true;
}


// Returns true if the app has hooked the screenshot
bool Steam_Screenshots::IsScreenshotsHooked()
{
    PRINT_DEBUG("IsScreenshotsHooked\n");
    return hooked;
}


// Adds a VR screenshot to the user's screenshot library from disk in the supported type.
// pchFilename should be the normal 2D image used in the library view
// pchVRFilename should contain the image that matches the correct type
// The return value is a handle that is valid for the duration of the game process and can be used to apply tags.
// JPEG, TGA, and PNG formats are supported.
ScreenshotHandle Steam_Screenshots::AddVRScreenshotToLibrary( EVRScreenshotType eType, const char *pchFilename, const char *pchVRFilename )
{
    PRINT_DEBUG("AddVRScreenshotToLibrary\n");
    return INVALID_SCREENSHOT_HANDLE;
}

