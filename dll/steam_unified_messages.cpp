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

#include "dll/steam_unified_messages.h"


void Steam_Unified_Messages::network_callback(void *object, Common_Message *msg)
{
    // PRINT_DEBUG_ENTRY();

    Steam_Unified_Messages *steam_steamunifiedmessages = (Steam_Unified_Messages *)object;
    steam_steamunifiedmessages->Callback(msg);
}

void Steam_Unified_Messages::steam_runcb(void *object)
{
    // PRINT_DEBUG_ENTRY();

    Steam_Unified_Messages *steam_steamunifiedmessages = (Steam_Unified_Messages *)object;
    steam_steamunifiedmessages->RunCallbacks();
}


Steam_Unified_Messages::Steam_Unified_Messages(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb)
{
    this->settings = settings;
    this->network = network;
    this->callback_results = callback_results;
    this->callbacks = callbacks;
    this->run_every_runcb = run_every_runcb;

    this->network->setCallback(CALLBACK_ID_USER_STATUS, settings->get_local_steam_id(), &Steam_Unified_Messages::network_callback, this);
    this->run_every_runcb->add(&Steam_Unified_Messages::steam_runcb, this);

}

Steam_Unified_Messages::~Steam_Unified_Messages()
{
    this->network->rmCallback(CALLBACK_ID_USER_STATUS, settings->get_local_steam_id(), &Steam_Unified_Messages::network_callback, this);
    this->run_every_runcb->remove(&Steam_Unified_Messages::steam_runcb, this);
}

// Sends a service method (in binary serialized form) using the Steam Client.
// Returns a unified message handle (k_InvalidUnifiedMessageHandle if could not send the message).
ClientUnifiedMessageHandle Steam_Unified_Messages::SendMethod( const char *pchServiceMethod, const void *pRequestBuffer, uint32 unRequestBufferSize, uint64 unContext )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return ISteamUnifiedMessages::k_InvalidUnifiedMessageHandle;
}


// Gets the size of the response and the EResult. Returns false if the response is not ready yet.
bool Steam_Unified_Messages::GetMethodResponseInfo( ClientUnifiedMessageHandle hHandle, uint32 *punResponseSize, EResult *peResult )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return false;
}


// Gets a response in binary serialized form (and optionally release the corresponding allocated memory).
bool Steam_Unified_Messages::GetMethodResponseData( ClientUnifiedMessageHandle hHandle, void *pResponseBuffer, uint32 unResponseBufferSize, bool bAutoRelease )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return false;
}


// Releases the message and its corresponding allocated memory.
bool Steam_Unified_Messages::ReleaseMethod( ClientUnifiedMessageHandle hHandle )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return false;
}


// Sends a service notification (in binary serialized form) using the Steam Client.
// Returns true if the notification was sent successfully.
bool Steam_Unified_Messages::SendNotification( const char *pchServiceNotification, const void *pNotificationBuffer, uint32 unNotificationBufferSize )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return false;
}


void Steam_Unified_Messages::RunCallbacks()
{
}

void Steam_Unified_Messages::Callback(Common_Message *msg)
{
    if (msg->has_low_level()) {
        if (msg->low_level().type() == Low_Level::CONNECT) {
            
        }

        if (msg->low_level().type() == Low_Level::DISCONNECT) {

        }
    }
}
