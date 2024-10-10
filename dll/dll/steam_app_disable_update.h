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

#ifndef __INCLUDED_STEAM_APP_DISABLE_UPDATE_H__
#define __INCLUDED_STEAM_APP_DISABLE_UPDATE_H__

#include "base.h"

class Steam_App_Disable_Update:
public ISteamAppDisableUpdate
{
    class Settings *settings{};
    class Networking *network{};
    class SteamCallResults *callback_results{};
    class SteamCallBacks *callbacks{};
    class RunEveryRunCB *run_every_runcb{};
    
    static void steam_network_callback(void *object, Common_Message *msg);
    static void steam_run_every_runcb(void *object);

	void steam_run_callback();
	void network_callback(Common_Message *msg);

public:
    Steam_App_Disable_Update(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb);
    ~Steam_App_Disable_Update();

	// probably means how many seconds to keep the updates disabled
    void SetAppUpdateDisabledSecondsRemaining(int32 nSeconds);

};

#endif // __INCLUDED_STEAM_APP_DISABLE_UPDATE_H__
