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

#include "dll/steam_game_coordinator.h"

void Steam_Game_Coordinator::push_incoming(std::string message)
{
    outgoing_messages.push(message);

    struct GCMessageAvailable_t data;
    data.m_nMessageSize = message.size();
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
}

void Steam_Game_Coordinator::steam_callback(void *object, Common_Message *msg)
{
    // PRINT_DEBUG_ENTRY();

    Steam_Game_Coordinator *steam_gamecoordinator = (Steam_Game_Coordinator *)object;
    steam_gamecoordinator->Callback(msg);
}

void Steam_Game_Coordinator::steam_run_every_runcb(void *object)
{
    // PRINT_DEBUG_ENTRY();

    Steam_Game_Coordinator *steam_gamecoordinator = (Steam_Game_Coordinator *)object;
    steam_gamecoordinator->RunCallbacks();
}


Steam_Game_Coordinator::Steam_Game_Coordinator(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb)
{
    this->settings = settings;
    this->network = network;
    this->callback_results = callback_results;
    this->callbacks = callbacks;
    this->run_every_runcb = run_every_runcb;
    
    this->network->setCallback(CALLBACK_ID_USER_STATUS, settings->get_local_steam_id(), &Steam_Game_Coordinator::steam_callback, this);
    this->run_every_runcb->add(&Steam_Game_Coordinator::steam_run_every_runcb, this);
}

Steam_Game_Coordinator::~Steam_Game_Coordinator()
{
    this->network->rmCallback(CALLBACK_ID_USER_STATUS, settings->get_local_steam_id(), &Steam_Game_Coordinator::steam_callback, this);
    this->run_every_runcb->remove(&Steam_Game_Coordinator::steam_run_every_runcb, this);
}


// sends a message to the Game Coordinator
EGCResults Steam_Game_Coordinator::SendMessage_( uint32 unMsgType, const void *pubData, uint32 cubData )
{
    PRINT_DEBUG("%X %u len %u", unMsgType, (~protobuf_mask) & unMsgType, cubData);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (protobuf_mask & unMsgType) {
        uint32 message_type = (~protobuf_mask) & unMsgType;
        if (message_type == 4006) { //client hello
            std::string message("\xA4\x0F\x00\x80\x00\x00\x00\x00\x08\x00", 10);
            push_incoming(message);
        } else
        if (message_type == 4007) { //server hello
            std::string message("\xA5\x0F\x00\x80\x00\x00\x00\x00\x08\x00", 10);
            push_incoming(message);
        }
    }

    return k_EGCResultOK;
}

// returns true if there is a message waiting from the game coordinator
bool Steam_Game_Coordinator::IsMessageAvailable( uint32 *pcubMsgSize )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (outgoing_messages.size()) {
        if (pcubMsgSize) *pcubMsgSize = outgoing_messages.front().size();
        return true;
    } else {
        return false;
    }
}

// fills the provided buffer with the first message in the queue and returns k_EGCResultOK or 
// returns k_EGCResultNoMessage if there is no message waiting. pcubMsgSize is filled with the message size.
// If the provided buffer is not large enough to fit the entire message, k_EGCResultBufferTooSmall is returned
// and the message remains at the head of the queue.
EGCResults Steam_Game_Coordinator::RetrieveMessage( uint32 *punMsgType, void *pubDest, uint32 cubDest, uint32 *pcubMsgSize )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (outgoing_messages.size()) {
        if (outgoing_messages.front().size() > cubDest) {
            return k_EGCResultBufferTooSmall;
        }

        outgoing_messages.front().copy((char *)pubDest, cubDest);
        if (pcubMsgSize) *pcubMsgSize = outgoing_messages.front().size();
        if (punMsgType && outgoing_messages.front().size() >= sizeof(uint32)) {
            outgoing_messages.front().copy((char *)punMsgType, sizeof(uint32));
            *punMsgType = ntohl(*punMsgType);
        }

        outgoing_messages.pop();
        return k_EGCResultOK;
    } else {
        return k_EGCResultNoMessage;
    }
}



void Steam_Game_Coordinator::RunCallbacks()
{
}

void Steam_Game_Coordinator::Callback(Common_Message *msg)
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
