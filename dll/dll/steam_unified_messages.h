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

class Steam_Unified_Messages :
public ISteamUnifiedMessages
{
    class Settings *settings;
    class Networking *network;
    class SteamCallResults *callback_results;
    class SteamCallBacks *callbacks;
    class RunEveryRunCB *run_every_runcb;

public:
static void steam_callback(void *object, Common_Message *msg)
{
    PRINT_DEBUG("steam_steamunifiedmessages_callback\n");

    Steam_Unified_Messages *steam_steamunifiedmessages = (Steam_Unified_Messages *)object;
    steam_steamunifiedmessages->Callback(msg);
}

static void steam_run_every_runcb(void *object)
{
    PRINT_DEBUG("steam_steamunifiedmessages_run_every_runcb\n");

    Steam_Unified_Messages *steam_steamunifiedmessages = (Steam_Unified_Messages *)object;
    steam_steamunifiedmessages->RunCallbacks();
}

Steam_Unified_Messages(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb)
{
    this->settings = settings;
    this->network = network;
    this->run_every_runcb = run_every_runcb;
//    this->network->setCallback(CALLBACK_ID_USER_STATUS, settings->get_local_steam_id(), &Steam_Unified_Messages::steam_callback, this);
//    this->run_every_runcb->add(&Steam_Unified_Messages::steam_run_every_runcb, this);

    this->callback_results = callback_results;
    this->callbacks = callbacks;
}

~Steam_Unified_Messages()
{
    //TODO rm network callbacks
//    this->run_every_runcb->remove(&Steam_Unified_Messages::steam_run_every_runcb, this);
}

// Sends a service method (in binary serialized form) using the Steam Client.
// Returns a unified message handle (k_InvalidUnifiedMessageHandle if could not send the message).
ClientUnifiedMessageHandle SendMethod( const char *pchServiceMethod, const void *pRequestBuffer, uint32 unRequestBufferSize, uint64 unContext )
{
    PRINT_DEBUG("Steam_Unified_Messages::SendMethod\n");
    return ISteamUnifiedMessages::k_InvalidUnifiedMessageHandle;
}


// Gets the size of the response and the EResult. Returns false if the response is not ready yet.
bool GetMethodResponseInfo( ClientUnifiedMessageHandle hHandle, uint32 *punResponseSize, EResult *peResult )
{
    PRINT_DEBUG("Steam_Unified_Messages::GetMethodResponseInfo\n");
    return false;
}


// Gets a response in binary serialized form (and optionally release the corresponding allocated memory).
bool GetMethodResponseData( ClientUnifiedMessageHandle hHandle, void *pResponseBuffer, uint32 unResponseBufferSize, bool bAutoRelease )
{
    PRINT_DEBUG("Steam_Unified_Messages::GetMethodResponseData\n");
    return false;
}


// Releases the message and its corresponding allocated memory.
bool ReleaseMethod( ClientUnifiedMessageHandle hHandle )
{
    PRINT_DEBUG("Steam_Unified_Messages::ReleaseMethod\n");
    return false;
}


// Sends a service notification (in binary serialized form) using the Steam Client.
// Returns true if the notification was sent successfully.
bool SendNotification( const char *pchServiceNotification, const void *pNotificationBuffer, uint32 unNotificationBufferSize )
{
    PRINT_DEBUG("Steam_Unified_Messages::SendNotification\n");
    return false;
}


void RunCallbacks()
{
}

void Callback(Common_Message *msg)
{
    if (msg->has_low_level()) {
        if (msg->low_level().type() == Low_Level::CONNECT) {
            
        }

        if (msg->low_level().type() == Low_Level::DISCONNECT) {

        }
    }
}

};
