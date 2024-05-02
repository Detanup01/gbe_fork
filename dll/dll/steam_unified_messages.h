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

#ifndef __INCLUDED_STEAM_UNIFIED_MESSAGES_H__
#define __INCLUDED_STEAM_UNIFIED_MESSAGES_H__

#include "base.h"

class Steam_Unified_Messages:
public ISteamUnifiedMessages
{
    class Settings *settings{};
    class Networking *network{};
    class SteamCallResults *callback_results{};
    class SteamCallBacks *callbacks{};
    class RunEveryRunCB *run_every_runcb{};

    static void network_callback(void *object, Common_Message *msg);
    static void steam_runcb(void *object);

public:
    Steam_Unified_Messages(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb);
    ~Steam_Unified_Messages();

    // Sends a service method (in binary serialized form) using the Steam Client.
    // Returns a unified message handle (k_InvalidUnifiedMessageHandle if could not send the message).
    ClientUnifiedMessageHandle SendMethod( const char *pchServiceMethod, const void *pRequestBuffer, uint32 unRequestBufferSize, uint64 unContext );


    // Gets the size of the response and the EResult. Returns false if the response is not ready yet.
    bool GetMethodResponseInfo( ClientUnifiedMessageHandle hHandle, uint32 *punResponseSize, EResult *peResult );


    // Gets a response in binary serialized form (and optionally release the corresponding allocated memory).
    bool GetMethodResponseData( ClientUnifiedMessageHandle hHandle, void *pResponseBuffer, uint32 unResponseBufferSize, bool bAutoRelease );


    // Releases the message and its corresponding allocated memory.
    bool ReleaseMethod( ClientUnifiedMessageHandle hHandle );


    // Sends a service notification (in binary serialized form) using the Steam Client.
    // Returns true if the notification was sent successfully.
    bool SendNotification( const char *pchServiceNotification, const void *pNotificationBuffer, uint32 unNotificationBufferSize );


    void RunCallbacks();

    void Callback(Common_Message *msg);

};

#endif // __INCLUDED_STEAM_UNIFIED_MESSAGES_H__
