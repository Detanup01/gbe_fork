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

#ifndef __INCLUDED_STEAM_GAME_COORDINATOR_H__
#define __INCLUDED_STEAM_GAME_COORDINATOR_H__

#include "base.h"

class Steam_Game_Coordinator :
public ISteamGameCoordinator
{
    constexpr const static uint32 protobuf_mask = 0x80000000;

    class Settings *settings{};
    class Networking *network{};
    class SteamCallResults *callback_results{};
    class SteamCallBacks *callbacks{};
    class RunEveryRunCB *run_every_runcb{};

    std::queue<std::string> outgoing_messages{};

    void push_incoming(std::string message);

    static void steam_callback(void *object, Common_Message *msg);
    static void steam_run_every_runcb(void *object);

    void RunCallbacks();
    void Callback(Common_Message *msg);

public:
    Steam_Game_Coordinator(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb);
    ~Steam_Game_Coordinator();

    // sends a message to the Game Coordinator
    EGCResults SendMessage_( uint32 unMsgType, const void *pubData, uint32 cubData );

    // returns true if there is a message waiting from the game coordinator
    bool IsMessageAvailable( uint32 *pcubMsgSize );

    // fills the provided buffer with the first message in the queue and returns k_EGCResultOK or 
    // returns k_EGCResultNoMessage if there is no message waiting. pcubMsgSize is filled with the message size.
    // If the provided buffer is not large enough to fit the entire message, k_EGCResultBufferTooSmall is returned
    // and the message remains at the head of the queue.
    EGCResults RetrieveMessage( uint32 *punMsgType, void *pubDest, uint32 cubDest, uint32 *pcubMsgSize );

};

#endif // __INCLUDED_STEAM_GAME_COORDINATOR_H__
