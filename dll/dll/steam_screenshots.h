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

#include "base.h"

struct screenshot_infos_t
{
	std::string screenshot_name;
	nlohmann::json metadatas;
};

class Steam_Screenshots : public ISteamScreenshots
{
    bool hooked = false;
	std::map<ScreenshotHandle, screenshot_infos_t> _screenshots;

	class Local_Storage* local_storage;
	class SteamCallBacks* callbacks;

	ScreenshotHandle create_screenshot_handle();

public:
	Steam_Screenshots(class Local_Storage* local_storage, class SteamCallBacks* callbacks);

	// Writes a screenshot to the user's screenshot library given the raw image data, which must be in RGB format.
	// The return value is a handle that is valid for the duration of the game process and can be used to apply tags.
	ScreenshotHandle WriteScreenshot( void *pubRGB, uint32 cubRGB, int nWidth, int nHeight );

	// Adds a screenshot to the user's screenshot library from disk.  If a thumbnail is provided, it must be 200 pixels wide and the same aspect ratio
	// as the screenshot, otherwise a thumbnail will be generated if the user uploads the screenshot.  The screenshots must be in either JPEG or TGA format.
	// The return value is a handle that is valid for the duration of the game process and can be used to apply tags.
	// JPEG, TGA, and PNG formats are supported.
	ScreenshotHandle AddScreenshotToLibrary( const char *pchFilename, const char *pchThumbnailFilename, int nWidth, int nHeight );

	// Causes the Steam overlay to take a screenshot.  If screenshots are being hooked by the game then a ScreenshotRequested_t callback is sent back to the game instead. 
	void TriggerScreenshot();

	// Toggles whether the overlay handles screenshots when the user presses the screenshot hotkey, or the game handles them.  If the game is hooking screenshots,
	// then the ScreenshotRequested_t callback will be sent if the user presses the hotkey, and the game is expected to call WriteScreenshot or AddScreenshotToLibrary
	// in response.
	void HookScreenshots( bool bHook );

	// Sets metadata about a screenshot's location (for example, the name of the map)
	bool SetLocation( ScreenshotHandle hScreenshot, const char *pchLocation );
	
	// Tags a user as being visible in the screenshot
	bool TagUser( ScreenshotHandle hScreenshot, CSteamID steamID );

	// Tags a published file as being visible in the screenshot
	bool TagPublishedFile( ScreenshotHandle hScreenshot, PublishedFileId_t unPublishedFileID );

	// Returns true if the app has hooked the screenshot
	bool IsScreenshotsHooked();

	// Adds a VR screenshot to the user's screenshot library from disk in the supported type.
	// pchFilename should be the normal 2D image used in the library view
	// pchVRFilename should contain the image that matches the correct type
	// The return value is a handle that is valid for the duration of the game process and can be used to apply tags.
	// JPEG, TGA, and PNG formats are supported.
	ScreenshotHandle AddVRScreenshotToLibrary( EVRScreenshotType eType, const char *pchFilename, const char *pchVRFilename );
};
