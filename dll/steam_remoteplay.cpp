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

#include "dll/steam_remoteplay.h"

void Steam_RemotePlay::steam_callback(void *object, Common_Message *msg)
{
    // PRINT_DEBUG_ENTRY();

    Steam_RemotePlay *steam_remoteplay = (Steam_RemotePlay *)object;
    steam_remoteplay->Callback(msg);
}

void Steam_RemotePlay::steam_run_every_runcb(void *object)
{
    // PRINT_DEBUG_ENTRY();

    Steam_RemotePlay *steam_remoteplay = (Steam_RemotePlay *)object;
    steam_remoteplay->RunCallbacks();
}

Steam_RemotePlay::Steam_RemotePlay(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb)
{
    this->settings = settings;
    this->network = network;
    this->callback_results = callback_results;
    this->callbacks = callbacks;
    this->run_every_runcb = run_every_runcb;
    
    this->network->setCallback(CALLBACK_ID_USER_STATUS, settings->get_local_steam_id(), &Steam_RemotePlay::steam_callback, this);
    this->run_every_runcb->add(&Steam_RemotePlay::steam_run_every_runcb, this);
}

Steam_RemotePlay::~Steam_RemotePlay()
{
    this->network->rmCallback(CALLBACK_ID_USER_STATUS, settings->get_local_steam_id(), &Steam_RemotePlay::steam_callback, this);
    this->run_every_runcb->remove(&Steam_RemotePlay::steam_run_every_runcb, this);
}

// Get the number of currently connected Steam Remote Play sessions
uint32 Steam_RemotePlay::GetSessionCount()
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return 0;
}

// Get the currently connected Steam Remote Play session ID at the specified index. Returns zero if index is out of bounds.
uint32 Steam_RemotePlay::GetSessionID( int iSessionIndex )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return 0;
}

// Get the SteamID of the connected user
CSteamID Steam_RemotePlay::GetSessionSteamID( uint32 unSessionID )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return k_steamIDNil;
}

// Get the name of the session client device
// This returns NULL if the sessionID is not valid
const char* Steam_RemotePlay::GetSessionClientName( uint32 unSessionID )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return NULL;
}

// Get the form factor of the session client device
ESteamDeviceFormFactor Steam_RemotePlay::GetSessionClientFormFactor( uint32 unSessionID )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return k_ESteamDeviceFormFactorUnknown;
}

// Get the resolution, in pixels, of the session client device
// This is set to 0x0 if the resolution is not available
bool Steam_RemotePlay::BGetSessionClientResolution( uint32 unSessionID, int *pnResolutionX, int *pnResolutionY )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (pnResolutionX) *pnResolutionX = 0;
    if (pnResolutionY) *pnResolutionY = 0;
    return false;
}

bool Steam_RemotePlay::BStartRemotePlayTogether( bool bShowOverlay )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return false;
}

// Invite a friend to Remote Play Together
// This returns false if the invite can't be sent
bool Steam_RemotePlay::BSendRemotePlayTogetherInvite( CSteamID steamIDFriend )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return false;
}

void Steam_RemotePlay::RunCallbacks()
{
    
}

void Steam_RemotePlay::Callback(Common_Message *msg)
{
    if (msg->has_low_level()) {
        if (msg->low_level().type() == Low_Level::CONNECT) {
            
        }

        if (msg->low_level().type() == Low_Level::DISCONNECT) {

        }
    }

    if (msg->has_networking_sockets()) {

    }
}
