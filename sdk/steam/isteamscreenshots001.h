//====== Copyright © 1996-2008, Valve Corporation, All rights reserved. =======
//
// Purpose: public interface to user remote file storage in Steam
//
//=============================================================================

#ifndef ISTEAMSCREENSHOTS001_H
#define ISTEAMSCREENSHOTS001_H
#ifdef STEAM_WIN32
#pragma once
#endif

//-----------------------------------------------------------------------------
// Purpose: Functions for adding screenshots to the user's screenshot library
//-----------------------------------------------------------------------------
class ISteamScreenshots001
{
public:
	// Writes a screenshot to the user's screenshot library given the raw image data, which must be in RGB format.
	// The return value is a handle that is valid for the duration of the game process and can be used to apply tags.
	virtual ScreenshotHandle WriteScreenshot( void *pubRGB, uint32 cubRGB, int nWidth, int nHeight ) = 0;

	// Adds a screenshot to the user's screenshot library from disk.  If a thumbnail is provided, it must be 200 pixels wide and the same aspect ratio
	// as the screenshot, otherwise a thumbnail will be generated if the user uploads the screenshot.  The screenshots must be in either JPEG or TGA format.
	// The return value is a handle that is valid for the duration of the game process and can be used to apply tags.
	virtual ScreenshotHandle AddScreenshotToLibrary( const char *pchJpegOrTGAFilename, const char *pchJpegOrTGAThumbFilename, int nWidth, int nHeight ) = 0;

	// Causes the Steam overlay to take a screenshot.  If screenshots are being hooked by the game then a ScreenshotRequested_t callback is sent back to the game instead. 
	virtual void TriggerScreenshot() = 0;

	// Toggles whether the overlay handles screenshots when the user presses the screenshot hotkey, or the game handles them.  If the game is hooking screenshots,
	// then the ScreenshotRequested_t callback will be sent if the user presses the hotkey, and the game is expected to call WriteScreenshot or AddScreenshotToLibrary
	// in response.
	virtual void HookScreenshots( bool bHook ) = 0;

	// Sets metadata about a screenshot's location (for example, the name of the map)
	virtual bool SetLocation( ScreenshotHandle hScreenshot, const char *pchLocation ) = 0;

	// Tags a user as being visible in the screenshot
	virtual bool TagUser( ScreenshotHandle hScreenshot, CSteamID steamID ) = 0;
};
#endif // ISTEAMSCREENSHOTS001_H