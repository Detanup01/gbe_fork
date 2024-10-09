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

#include "dll/steam_app_disable_update.h"


void Steam_App_Disable_Update::steam_run_every_runcb(void *object)
{
    // PRINT_DEBUG_ENTRY();

    auto inst = (Steam_App_Disable_Update *)object;
    inst->steam_run_callback();
}

void Steam_App_Disable_Update::steam_network_callback(void *object, Common_Message *msg)
{
    // PRINT_DEBUG_ENTRY();

    auto inst = (Steam_App_Disable_Update *)object;
    inst->network_callback(msg);
}



Steam_App_Disable_Update::Steam_App_Disable_Update(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb)
{
    this->settings = settings;
    this->network = network;
    this->callback_results = callback_results;
    this->callbacks = callbacks;
    this->run_every_runcb = run_every_runcb;
    
    // this->network->setCallback(CALLBACK_ID_USER_STATUS, settings->get_local_steam_id(), &Steam_App_Disable_Update::steam_network_callback, this);
    // this->run_every_runcb->add(&Steam_App_Disable_Update::steam_run_every_runcb, this);
}

Steam_App_Disable_Update::~Steam_App_Disable_Update()
{
    // this->network->rmCallback(CALLBACK_ID_USER_STATUS, settings->get_local_steam_id(), &Steam_App_Disable_Update::steam_network_callback, this);
    // this->run_every_runcb->remove(&Steam_App_Disable_Update::steam_run_every_runcb, this);
}


void Steam_App_Disable_Update::SetAppUpdateDisabledSecondsRemaining(int32 nSeconds)
{
    PRINT_DEBUG_TODO();
    std::lock_guard lock(global_mutex);

}



void Steam_App_Disable_Update::steam_run_callback()
{
}



void Steam_App_Disable_Update::network_callback(Common_Message *msg)
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
