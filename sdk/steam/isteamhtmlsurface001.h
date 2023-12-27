
#ifndef ISTEAMHTMLSURFACE001_H
#define ISTEAMHTMLSURFACE001_H
#ifdef STEAM_WIN32
#pragma once
#endif

class ISteamHTMLSurface001
{
public:										
	virtual ~ISteamHTMLSurface001() {}

	// Must call init and shutdown when starting/ending use of the interface
	virtual bool Init() = 0;
	virtual bool Shutdown() = 0;

	// Create a browser object for display of a html page, when creation is complete the call handle
	// will return a HTML_BrowserReady_t callback for the HHTMLBrowser of your new browser.
	//   The user agent string is a substring to be added to the general user agent string so you can
	// identify your client on web servers.
	//   The userCSS string lets you apply a CSS style sheet to every displayed page, leave null if
	// you do not require this functionality.
	virtual SteamAPICall_t CreateBrowser( const char *pchUserAgent, const char *pchUserCSS ) = 0;

	// Call this when you are done with a html surface, this lets us free the resources being used by it
	virtual void RemoveBrowser( HHTMLBrowser unBrowserHandle ) = 0;

	// Navigate to this URL, results in a HTML_StartRequest_t as the request commences 
	virtual void LoadURL( HHTMLBrowser unBrowserHandle, const char *pchURL, const char *pchPostData ) = 0;

	// Tells the surface the size in pixels to display the surface
	virtual void SetSize( HHTMLBrowser unBrowserHandle, uint32 unWidth, uint32 unHeight ) = 0;

	// Stop the load of the current html page
	virtual void StopLoad( HHTMLBrowser unBrowserHandle ) = 0;
	// Reload (most likely from local cache) the current page
	virtual void Reload( HHTMLBrowser unBrowserHandle ) = 0;
	// navigate back in the page history
	virtual void GoBack( HHTMLBrowser unBrowserHandle ) = 0;
	// navigate forward in the page history
	virtual void GoForward( HHTMLBrowser unBrowserHandle ) = 0;

	// add this header to any url requests from this browser
	virtual void AddHeader( HHTMLBrowser unBrowserHandle, const char *pchKey, const char *pchValue ) = 0;
	// run this javascript script in the currently loaded page
	virtual void ExecuteJavascript( HHTMLBrowser unBrowserHandle, const char *pchScript ) = 0;

	// Mouse click and mouse movement commands
	virtual void MouseUp( HHTMLBrowser unBrowserHandle, EHTMLMouseButton eMouseButton ) = 0;
	virtual void MouseDown( HHTMLBrowser unBrowserHandle, EHTMLMouseButton eMouseButton ) = 0;
	virtual void MouseDoubleClick( HHTMLBrowser unBrowserHandle, EHTMLMouseButton eMouseButton ) = 0;
	// x and y are relative to the HTML bounds
	virtual void MouseMove( HHTMLBrowser unBrowserHandle, int x, int y ) = 0;
	// nDelta is pixels of scroll
	virtual void MouseWheel( HHTMLBrowser unBrowserHandle, int32 nDelta ) = 0;

	// keyboard interactions, native keycode is the virtual key code value from your OS
	virtual void KeyDown( HHTMLBrowser unBrowserHandle, uint32 nNativeKeyCode, EHTMLKeyModifiers eHTMLKeyModifiers ) = 0;
	virtual void KeyUp( HHTMLBrowser unBrowserHandle, uint32 nNativeKeyCode, EHTMLKeyModifiers eHTMLKeyModifiers ) = 0;
	// cUnicodeChar is the unicode character point for this keypress (and potentially multiple chars per press)
	virtual void KeyChar( HHTMLBrowser unBrowserHandle, uint32 cUnicodeChar, EHTMLKeyModifiers eHTMLKeyModifiers ) = 0;

	// programmatically scroll this many pixels on the page
	virtual void SetHorizontalScroll( HHTMLBrowser unBrowserHandle, uint32 nAbsolutePixelScroll ) = 0;
	virtual void SetVerticalScroll( HHTMLBrowser unBrowserHandle, uint32 nAbsolutePixelScroll ) = 0;

	// tell the html control if it has key focus currently, controls showing the I-beam cursor in text controls amongst other things
	virtual void SetKeyFocus( HHTMLBrowser unBrowserHandle, bool bHasKeyFocus ) = 0;

	// open the current pages html code in the local editor of choice, used for debugging
	virtual void ViewSource( HHTMLBrowser unBrowserHandle ) = 0;
	// copy the currently selected text on the html page to the local clipboard
	virtual void CopyToClipboard( HHTMLBrowser unBrowserHandle ) = 0;
	// paste from the local clipboard to the current html page
	virtual void PasteFromClipboard( HHTMLBrowser unBrowserHandle ) = 0;

	// find this string in the browser, if bCurrentlyInFind is true then instead cycle to the next matching element
	virtual void Find( HHTMLBrowser unBrowserHandle, const char *pchSearchStr, bool bCurrentlyInFind, bool bReverse ) = 0;
	// cancel a currently running find
	virtual void StopFind( HHTMLBrowser unBrowserHandle ) = 0;

	// return details about the link at position x,y on the current page
	virtual void GetLinkAtPosition(  HHTMLBrowser unBrowserHandle, int x, int y ) = 0;

	// CALLBACKS
	//
	//  These set of functions are used as responses to callback requests
	//

	// You MUST call this in response to a HTML_StartRequest_t callback
	//  Set bAllowed to true to allow this navigation, false to cancel it and stay 
	// on the current page. You can use this feature to limit the valid pages
	// allowed in your HTML surface.
	virtual void AllowStartRequest( HHTMLBrowser unBrowserHandle, bool bAllowed ) = 0;

	// You MUST call this in response to a HTML_JSAlert_t or HTML_JSConfirm_t callback
	//  Set bResult to true for the OK option of a confirm, use false otherwise
	virtual void JSDialogResponse( HHTMLBrowser unBrowserHandle, bool bResult ) = 0;

	// You MUST call this in response to a HTML_FileOpenDialog_t callback
	virtual void FileLoadDialogResponse( HHTMLBrowser unBrowserHandle, const char **pchSelectedFiles ) = 0;
};

#endif // ISTEAMHTMLSURFACE001_H
