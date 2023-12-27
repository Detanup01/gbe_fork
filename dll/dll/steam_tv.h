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

class Steam_TV :
public ISteamTV
{
    class Settings *settings;
    class Networking *network;
    class SteamCallResults *callback_results;
    class SteamCallBacks *callbacks;
    class RunEveryRunCB *run_every_runcb;
    std::chrono::time_point<std::chrono::steady_clock> initialized_time = std::chrono::steady_clock::now();
    FSteamNetworkingSocketsDebugOutput debug_function;

public:
static void steam_callback(void *object, Common_Message *msg)
{
    PRINT_DEBUG("steam_tv_callback\n");

    Steam_TV *steam_parties = (Steam_TV *)object;
    steam_parties->Callback(msg);
}

static void steam_run_every_runcb(void *object)
{
    PRINT_DEBUG("steam_tv_run_every_runcb\n");

    Steam_TV *steam_parties = (Steam_TV *)object;
    steam_parties->RunCallbacks();
}

Steam_TV(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb)
{
    this->settings = settings;
    this->network = network;
    this->run_every_runcb = run_every_runcb;
    //this->network->setCallback(CALLBACK_ID_USER_STATUS, settings->get_local_steam_id(), &Steam_TV::steam_callback, this);
    // this->run_every_runcb->add(&Steam_TV::steam_run_every_runcb, this);

    this->callback_results = callback_results;
    this->callbacks = callbacks;
}

~Steam_TV()
{
    //TODO rm network callbacks
    //this->run_every_runcb->remove(&Steam_TV::steam_run_every_runcb, this);
}

bool IsBroadcasting(int *pnNumViewers)
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
    return false;
}

void AddBroadcastGameData(const char * pchKey, const char * pchValue)
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
}

void RemoveBroadcastGameData(const char * pchKey)
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
}

void AddTimelineMarker(const char * pchTemplateName, bool bPersistent, uint8 nColorR, uint8 nColorG, uint8 nColorB)
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
}

void RemoveTimelineMarker()
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
}

uint32 AddRegion(const char * pchElementName, const char * pchTimelineDataSection, const SteamTVRegion_t * pSteamTVRegion, ESteamTVRegionBehavior eSteamTVRegionBehavior)
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
    return 0;
}

void RemoveRegion(uint32 unRegionHandle)
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
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
