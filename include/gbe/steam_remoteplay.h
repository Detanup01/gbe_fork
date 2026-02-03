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

#ifndef __INCLUDED_STEAM_REMOTEPLAY_H__
#define __INCLUDED_STEAM_REMOTEPLAY_H__

#include "base.h"

class Steam_RemotePlay :
public ISteamRemotePlay001,
public ISteamRemotePlay002,
public ISteamRemotePlay
{
    class Settings *settings{};
    class Networking *network{};
    class SteamCallResults *callback_results{};
    class SteamCallBacks *callbacks{};
    class RunEveryRunCB *run_every_runcb{};

    bool direct_input{};
    RemotePlayCursorID_t cursor_handle{};

    static void steam_callback(void *object, Common_Message *msg);
    static void steam_run_every_runcb(void *object);

public:
    Steam_RemotePlay(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb);
    ~Steam_RemotePlay();

    // Get the number of currently connected Steam Remote Play sessions
    uint32 GetSessionCount();

    // Get the currently connected Steam Remote Play session ID at the specified index. Returns zero if index is out of bounds.
    uint32 GetSessionID( int iSessionIndex );

    // Get the SteamID of the connected user
    CSteamID GetSessionSteamID( uint32 unSessionID );

    // Get the name of the session client device
    // This returns NULL if the sessionID is not valid
    const char *GetSessionClientName( uint32 unSessionID );

    // Get the form factor of the session client device
    ESteamDeviceFormFactor GetSessionClientFormFactor( uint32 unSessionID );

    // Get the resolution, in pixels, of the session client device
    // This is set to 0x0 if the resolution is not available
    bool BGetSessionClientResolution( uint32 unSessionID, int *pnResolutionX, int *pnResolutionY );

    bool BStartRemotePlayTogether( bool bShowOverlay );

    bool ShowRemotePlayTogetherUI();

    // Invite a friend to Remote Play Together
    // This returns false if the invite can't be sent
    bool BSendRemotePlayTogetherInvite( CSteamID steamIDFriend );

    // Make mouse and keyboard input for Remote Play Together sessions available via GetInput() instead of being merged with local input
    bool BEnableRemotePlayTogetherDirectInput();

    // Merge Remote Play Together mouse and keyboard input with local input
    void DisableRemotePlayTogetherDirectInput();

    // Get input events from Remote Play Together sessions
    // This is available after calling BEnableRemotePlayTogetherDirectInput()
    //
    // pInput is an array of input events that will be filled in by this function, up to unMaxEvents.
    // This returns the number of events copied to pInput, or the number of events available if pInput is nullptr.
    uint32 GetInput( RemotePlayInput_t *pInput, uint32 unMaxEvents );

    // Set the mouse cursor visibility for a remote player
    // This is available after calling BEnableRemotePlayTogetherDirectInput()
    void SetMouseVisibility( RemotePlaySessionID_t unSessionID, bool bVisible );

    // Set the mouse cursor position for a remote player
    // This is available after calling BEnableRemotePlayTogetherDirectInput()
    //
    // This is used to warp the cursor to a specific location and isn't needed during normal event processing.
    //
    // The position is normalized relative to the window, where 0,0 is the upper left, and 1,1 is the lower right.
    void SetMousePosition( RemotePlaySessionID_t unSessionID, float flNormalizedX, float flNormalizedY );

    // Create a cursor that can be used with SetMouseCursor()
    // This is available after calling BEnableRemotePlayTogetherDirectInput()
    //
    // Parameters:
    // nWidth - The width of the cursor, in pixels
    // nHeight - The height of the cursor, in pixels
    // nHotX - The X coordinate of the cursor hot spot in pixels, offset from the left of the cursor
    // nHotY - The Y coordinate of the cursor hot spot in pixels, offset from the top of the cursor
    // pBGRA - A pointer to the cursor pixels, with the color channels in red, green, blue, alpha order
    // nPitch - The distance between pixel rows in bytes, defaults to nWidth * 4
    RemotePlayCursorID_t CreateMouseCursor( int nWidth, int nHeight, int nHotX, int nHotY, const void *pBGRA, int nPitch );

    // Set the mouse cursor for a remote player
    // This is available after calling BEnableRemotePlayTogetherDirectInput()
    //
    // The cursor ID is a value returned by CreateMouseCursor()
    void SetMouseCursor( RemotePlaySessionID_t unSessionID, RemotePlayCursorID_t unCursorID );

    void RunCallbacks();

    void Callback(Common_Message *msg);

};

#endif // __INCLUDED_STEAM_REMOTEPLAY_H__
