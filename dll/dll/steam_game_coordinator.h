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

class Steam_Game_Coordinator :
public ISteamGameCoordinator
{
    class Settings *settings;
    class Networking *network;
    class SteamCallResults *callback_results;
    class SteamCallBacks *callbacks;
    class RunEveryRunCB *run_every_runcb;

    static const uint32 protobuf_mask = 0x80000000;
    std::queue<std::string> outgoing_messages;

void push_incoming(std::string message)
{
    outgoing_messages.push(message);

    struct GCMessageAvailable_t data;
    data.m_nMessageSize = message.size();
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
}

public:
static void steam_callback(void *object, Common_Message *msg)
{
    PRINT_DEBUG("steam_gamecoordinator_callback\n");

    Steam_Game_Coordinator *steam_gamecoordinator = (Steam_Game_Coordinator *)object;
    steam_gamecoordinator->Callback(msg);
}

static void steam_run_every_runcb(void *object)
{
    PRINT_DEBUG("steam_gamecoordinator_run_every_runcb\n");

    Steam_Game_Coordinator *steam_gamecoordinator = (Steam_Game_Coordinator *)object;
    steam_gamecoordinator->RunCallbacks();
}

Steam_Game_Coordinator(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb)
{
    this->settings = settings;
    this->network = network;
    this->run_every_runcb = run_every_runcb;
    //this->network->setCallback(CALLBACK_ID_USER_STATUS, settings->get_local_steam_id(), &Steam_Game_Coordinator::steam_callback, this);
    this->run_every_runcb->add(&Steam_Game_Coordinator::steam_run_every_runcb, this);

    this->callback_results = callback_results;
    this->callbacks = callbacks;
}

~Steam_Game_Coordinator()
{
    //TODO rm network callbacks
    this->run_every_runcb->remove(&Steam_Game_Coordinator::steam_run_every_runcb, this);
}

// sends a message to the Game Coordinator
EGCResults SendMessage_( uint32 unMsgType, const void *pubData, uint32 cubData )
{
    PRINT_DEBUG("Steam_Game_Coordinator::SendMessage %X %u len %u\n", unMsgType, (~protobuf_mask) & unMsgType, cubData);
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
bool IsMessageAvailable( uint32 *pcubMsgSize )
{
    PRINT_DEBUG("Steam_Game_Coordinator::IsMessageAvailable\n");
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
EGCResults RetrieveMessage( uint32 *punMsgType, void *pubDest, uint32 cubDest, uint32 *pcubMsgSize )
{
    PRINT_DEBUG("Steam_Game_Coordinator::RetrieveMessage\n");
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

    if (msg->has_networking_sockets()) {

    }
}

};
