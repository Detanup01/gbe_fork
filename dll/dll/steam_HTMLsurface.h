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

#ifndef __INCLUDED_STEAM_HTMLSURFACE_H__
#define __INCLUDED_STEAM_HTMLSURFACE_H__

#include "base.h"

class Steam_HTMLsurface :
public ISteamHTMLSurface001,
public ISteamHTMLSurface002,
public ISteamHTMLSurface003,
public ISteamHTMLSurface004,
public ISteamHTMLSurface
{
    class Settings *settings{};
    class Networking *network{};
    class SteamCallResults *callback_results{};
    class SteamCallBacks *callbacks{};

public:
    Steam_HTMLsurface(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks);

    // Must call init and shutdown when starting/ending use of the interface
    bool Init();

    bool Shutdown();


    // Create a browser object for display of a html page, when creation is complete the call handle
    // will return a HTML_BrowserReady_t callback for the HHTMLBrowser of your new browser.
    //   The user agent string is a substring to be added to the general user agent string so you can
    // identify your client on web servers.
    //   The userCSS string lets you apply a CSS style sheet to every displayed page, leave null if
    // you do not require this functionality.
    //
    // YOU MUST HAVE IMPLEMENTED HANDLERS FOR HTML_BrowserReady_t, HTML_StartRequest_t,
    // HTML_JSAlert_t, HTML_JSConfirm_t, and HTML_FileOpenDialog_t! See the CALLBACKS
    // section of this interface (AllowStartRequest, etc) for more details. If you do
    // not implement these callback handlers, the browser may appear to hang instead of
    // navigating to new pages or triggering javascript popups.
    //
    STEAM_CALL_RESULT( HTML_BrowserReady_t )
    SteamAPICall_t CreateBrowser( const char *pchUserAgent, const char *pchUserCSS );


    // Call this when you are done with a html surface, this lets us free the resources being used by it
    void RemoveBrowser( HHTMLBrowser unBrowserHandle );


    // Navigate to this URL, results in a HTML_StartRequest_t as the request commences 
    void LoadURL( HHTMLBrowser unBrowserHandle, const char *pchURL, const char *pchPostData );


    // Tells the surface the size in pixels to display the surface
    void SetSize( HHTMLBrowser unBrowserHandle, uint32 unWidth, uint32 unHeight );


    // Stop the load of the current html page
    void StopLoad( HHTMLBrowser unBrowserHandle );

    // Reload (most likely from local cache) the current page
    void Reload( HHTMLBrowser unBrowserHandle );

    // navigate back in the page history
    void GoBack( HHTMLBrowser unBrowserHandle );

    // navigate forward in the page history
    void GoForward( HHTMLBrowser unBrowserHandle );


    // add this header to any url requests from this browser
    void AddHeader( HHTMLBrowser unBrowserHandle, const char *pchKey, const char *pchValue );

    // run this javascript script in the currently loaded page
    void ExecuteJavascript( HHTMLBrowser unBrowserHandle, const char *pchScript );

    // Mouse click and mouse movement commands
    void MouseUp( HHTMLBrowser unBrowserHandle, EHTMLMouseButton eMouseButton );

    void MouseDown( HHTMLBrowser unBrowserHandle, EHTMLMouseButton eMouseButton );

    void MouseDoubleClick( HHTMLBrowser unBrowserHandle, EHTMLMouseButton eMouseButton );

    // x and y are relative to the HTML bounds
    void MouseMove( HHTMLBrowser unBrowserHandle, int x, int y );

    // nDelta is pixels of scroll
    void MouseWheel( HHTMLBrowser unBrowserHandle, int32 nDelta );

    // keyboard interactions, native keycode is the key code value from your OS
    void KeyDown( HHTMLBrowser unBrowserHandle, uint32 nNativeKeyCode, EHTMLKeyModifiers eHTMLKeyModifiers, bool bIsSystemKey = false );

    void KeyDown( HHTMLBrowser unBrowserHandle, uint32 nNativeKeyCode, EHTMLKeyModifiers eHTMLKeyModifiers);


    void KeyUp( HHTMLBrowser unBrowserHandle, uint32 nNativeKeyCode, EHTMLKeyModifiers eHTMLKeyModifiers );

    // cUnicodeChar is the unicode character point for this keypress (and potentially multiple chars per press)
    void KeyChar( HHTMLBrowser unBrowserHandle, uint32 cUnicodeChar, EHTMLKeyModifiers eHTMLKeyModifiers );


    // programmatically scroll this many pixels on the page
    void SetHorizontalScroll( HHTMLBrowser unBrowserHandle, uint32 nAbsolutePixelScroll );

    void SetVerticalScroll( HHTMLBrowser unBrowserHandle, uint32 nAbsolutePixelScroll );


    // tell the html control if it has key focus currently, controls showing the I-beam cursor in text controls amongst other things
    void SetKeyFocus( HHTMLBrowser unBrowserHandle, bool bHasKeyFocus );


    // open the current pages html code in the local editor of choice, used for debugging
    void ViewSource( HHTMLBrowser unBrowserHandle );

    // copy the currently selected text on the html page to the local clipboard
    void CopyToClipboard( HHTMLBrowser unBrowserHandle );

    // paste from the local clipboard to the current html page
    void PasteFromClipboard( HHTMLBrowser unBrowserHandle );


    // find this string in the browser, if bCurrentlyInFind is true then instead cycle to the next matching element
    void Find( HHTMLBrowser unBrowserHandle, const char *pchSearchStr, bool bCurrentlyInFind, bool bReverse );

    // cancel a currently running find
    void StopFind( HHTMLBrowser unBrowserHandle );


    // return details about the link at position x,y on the current page
    void GetLinkAtPosition(  HHTMLBrowser unBrowserHandle, int x, int y );


    // set a webcookie for the hostname in question
    void SetCookie( const char *pchHostname, const char *pchKey, const char *pchValue, const char *pchPath, RTime32 nExpires, bool bSecure, bool bHTTPOnly );


    // Zoom the current page by flZoom ( from 0.0 to 2.0, so to zoom to 120% use 1.2 ), zooming around point X,Y in the page (use 0,0 if you don't care)
    void SetPageScaleFactor( HHTMLBrowser unBrowserHandle, float flZoom, int nPointX, int nPointY );


    // Enable/disable low-resource background mode, where javascript and repaint timers are throttled, resources are
    // more aggressively purged from memory, and audio/video elements are paused. When background mode is enabled,
    // all HTML5 video and audio objects will execute ".pause()" and gain the property "._steam_background_paused = 1".
    // When background mode is disabled, any video or audio objects with that property will resume with ".play()".
    void SetBackgroundMode( HHTMLBrowser unBrowserHandle, bool bBackgroundMode );


    // Scale the output display space by this factor, this is useful when displaying content on high dpi devices.
    // Specifies the ratio between physical and logical pixels.
    void SetDPIScalingFactor( HHTMLBrowser unBrowserHandle, float flDPIScaling );

    void OpenDeveloperTools( HHTMLBrowser unBrowserHandle );

    // CALLBACKS
    //
    //  These set of functions are used as responses to callback requests
    //

    // You MUST call this in response to a HTML_StartRequest_t callback
    //  Set bAllowed to true to allow this navigation, false to cancel it and stay 
    // on the current page. You can use this feature to limit the valid pages
    // allowed in your HTML surface.
    void AllowStartRequest( HHTMLBrowser unBrowserHandle, bool bAllowed );


    // You MUST call this in response to a HTML_JSAlert_t or HTML_JSConfirm_t callback
    //  Set bResult to true for the OK option of a confirm, use false otherwise
    void JSDialogResponse( HHTMLBrowser unBrowserHandle, bool bResult );


    // You MUST call this in response to a HTML_FileOpenDialog_t callback
    STEAM_IGNOREATTR()
    void FileLoadDialogResponse( HHTMLBrowser unBrowserHandle, const char **pchSelectedFiles );

};

#endif // __INCLUDED_STEAM_HTMLSURFACE_H__
