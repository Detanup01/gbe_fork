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

#include "dll/steam_HTMLsurface.h"

Steam_HTMLsurface::Steam_HTMLsurface(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks)
{
    this->settings = settings;
    this->network = network;
    this->callback_results = callback_results;
    this->callbacks = callbacks;
}


// Must call init and shutdown when starting/ending use of the interface
bool Steam_HTMLsurface::Init()
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return true;
}

bool Steam_HTMLsurface::Shutdown()
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return true;
}


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
SteamAPICall_t Steam_HTMLsurface::CreateBrowser( const char *pchUserAgent, const char *pchUserCSS )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    HTML_BrowserReady_t data;
    data.unBrowserHandle = 1234869;
    
    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return ret;
}


// Call this when you are done with a html surface, this lets us free the resources being used by it
void Steam_HTMLsurface::RemoveBrowser( HHTMLBrowser unBrowserHandle )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}


// Navigate to this URL, results in a HTML_StartRequest_t as the request commences 
void Steam_HTMLsurface::LoadURL( HHTMLBrowser unBrowserHandle, const char *pchURL, const char *pchPostData )
{
    PRINT_DEBUG("TODO %s %s", pchURL, pchPostData);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    static char url[256];
    strncpy(url, pchURL, sizeof(url));
    static char target[] = "_self";
    static char title[] = "title";

    {
        HTML_StartRequest_t data;
        data.unBrowserHandle = unBrowserHandle;
        data.pchURL = url;
        data.pchTarget = target;
        data.pchPostData = "";
        data.bIsRedirect = false;
        callbacks->addCBResult(data.k_iCallback, &data, sizeof(data), 0.1);
    }

    {
        HTML_FinishedRequest_t data;
        data.unBrowserHandle = unBrowserHandle;
        data.pchURL = url;
        data.pchPageTitle = title;
        callbacks->addCBResult(data.k_iCallback, &data, sizeof(data), 0.8);
    }

}


// Tells the surface the size in pixels to display the surface
void Steam_HTMLsurface::SetSize( HHTMLBrowser unBrowserHandle, uint32 unWidth, uint32 unHeight )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}


// Stop the load of the current html page
void Steam_HTMLsurface::StopLoad( HHTMLBrowser unBrowserHandle )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}

// Reload (most likely from local cache) the current page
void Steam_HTMLsurface::Reload( HHTMLBrowser unBrowserHandle )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}

// navigate back in the page history
void Steam_HTMLsurface::GoBack( HHTMLBrowser unBrowserHandle )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}

// navigate forward in the page history
void Steam_HTMLsurface::GoForward( HHTMLBrowser unBrowserHandle )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}


// add this header to any url requests from this browser
void Steam_HTMLsurface::AddHeader( HHTMLBrowser unBrowserHandle, const char *pchKey, const char *pchValue )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}

// run this javascript script in the currently loaded page
void Steam_HTMLsurface::ExecuteJavascript( HHTMLBrowser unBrowserHandle, const char *pchScript )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}

// Mouse click and mouse movement commands
void Steam_HTMLsurface::MouseUp( HHTMLBrowser unBrowserHandle, EHTMLMouseButton eMouseButton )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}

void Steam_HTMLsurface::MouseDown( HHTMLBrowser unBrowserHandle, EHTMLMouseButton eMouseButton )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}

void Steam_HTMLsurface::MouseDoubleClick( HHTMLBrowser unBrowserHandle, EHTMLMouseButton eMouseButton )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}

// x and y are relative to the HTML bounds
void Steam_HTMLsurface::MouseMove( HHTMLBrowser unBrowserHandle, int x, int y )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}

// nDelta is pixels of scroll
void Steam_HTMLsurface::MouseWheel( HHTMLBrowser unBrowserHandle, int32 nDelta )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}

// keyboard interactions, native keycode is the key code value from your OS
void Steam_HTMLsurface::KeyDown( HHTMLBrowser unBrowserHandle, uint32 nNativeKeyCode, EHTMLKeyModifiers eHTMLKeyModifiers, bool bIsSystemKey )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}

void Steam_HTMLsurface::KeyDown( HHTMLBrowser unBrowserHandle, uint32 nNativeKeyCode, EHTMLKeyModifiers eHTMLKeyModifiers)
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    KeyDown(unBrowserHandle, nNativeKeyCode, eHTMLKeyModifiers, false);
}


void Steam_HTMLsurface::KeyUp( HHTMLBrowser unBrowserHandle, uint32 nNativeKeyCode, EHTMLKeyModifiers eHTMLKeyModifiers )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}

// cUnicodeChar is the unicode character point for this keypress (and potentially multiple chars per press)
void Steam_HTMLsurface::KeyChar( HHTMLBrowser unBrowserHandle, uint32 cUnicodeChar, EHTMLKeyModifiers eHTMLKeyModifiers )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}


// programmatically scroll this many pixels on the page
void Steam_HTMLsurface::SetHorizontalScroll( HHTMLBrowser unBrowserHandle, uint32 nAbsolutePixelScroll )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}

void Steam_HTMLsurface::SetVerticalScroll( HHTMLBrowser unBrowserHandle, uint32 nAbsolutePixelScroll )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}


// tell the html control if it has key focus currently, controls showing the I-beam cursor in text controls amongst other things
void Steam_HTMLsurface::SetKeyFocus( HHTMLBrowser unBrowserHandle, bool bHasKeyFocus )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}


// open the current pages html code in the local editor of choice, used for debugging
void Steam_HTMLsurface::ViewSource( HHTMLBrowser unBrowserHandle )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}

// copy the currently selected text on the html page to the local clipboard
void Steam_HTMLsurface::CopyToClipboard( HHTMLBrowser unBrowserHandle )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}

// paste from the local clipboard to the current html page
void Steam_HTMLsurface::PasteFromClipboard( HHTMLBrowser unBrowserHandle )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}


// find this string in the browser, if bCurrentlyInFind is true then instead cycle to the next matching element
void Steam_HTMLsurface::Find( HHTMLBrowser unBrowserHandle, const char *pchSearchStr, bool bCurrentlyInFind, bool bReverse )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}

// cancel a currently running find
void Steam_HTMLsurface::StopFind( HHTMLBrowser unBrowserHandle )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}


// return details about the link at position x,y on the current page
void Steam_HTMLsurface::GetLinkAtPosition(  HHTMLBrowser unBrowserHandle, int x, int y )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}


// set a webcookie for the hostname in question
void Steam_HTMLsurface::SetCookie( const char *pchHostname, const char *pchKey, const char *pchValue, const char *pchPath, RTime32 nExpires, bool bSecure, bool bHTTPOnly )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}


// Zoom the current page by flZoom ( from 0.0 to 2.0, so to zoom to 120% use 1.2 ), zooming around point X,Y in the page (use 0,0 if you don't care)
void Steam_HTMLsurface::SetPageScaleFactor( HHTMLBrowser unBrowserHandle, float flZoom, int nPointX, int nPointY )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}


// Enable/disable low-resource background mode, where javascript and repaint timers are throttled, resources are
// more aggressively purged from memory, and audio/video elements are paused. When background mode is enabled,
// all HTML5 video and audio objects will execute ".pause()" and gain the property "._steam_background_paused = 1".
// When background mode is disabled, any video or audio objects with that property will resume with ".play()".
void Steam_HTMLsurface::SetBackgroundMode( HHTMLBrowser unBrowserHandle, bool bBackgroundMode )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}


// Scale the output display space by this factor, this is useful when displaying content on high dpi devices.
// Specifies the ratio between physical and logical pixels.
void Steam_HTMLsurface::SetDPIScalingFactor( HHTMLBrowser unBrowserHandle, float flDPIScaling )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}

void Steam_HTMLsurface::OpenDeveloperTools( HHTMLBrowser unBrowserHandle )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}

// CALLBACKS
//
//  These set of functions are used as responses to callback requests
//

// You MUST call this in response to a HTML_StartRequest_t callback
//  Set bAllowed to true to allow this navigation, false to cancel it and stay 
// on the current page. You can use this feature to limit the valid pages
// allowed in your HTML surface.
void Steam_HTMLsurface::AllowStartRequest( HHTMLBrowser unBrowserHandle, bool bAllowed )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}


// You MUST call this in response to a HTML_JSAlert_t or HTML_JSConfirm_t callback
//  Set bResult to true for the OK option of a confirm, use false otherwise
void Steam_HTMLsurface::JSDialogResponse( HHTMLBrowser unBrowserHandle, bool bResult )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}


// You MUST call this in response to a HTML_FileOpenDialog_t callback
STEAM_IGNOREATTR()
void Steam_HTMLsurface::FileLoadDialogResponse( HHTMLBrowser unBrowserHandle, const char **pchSelectedFiles )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}
