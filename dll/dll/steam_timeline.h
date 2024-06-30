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

#ifndef __INCLUDED_STEAM_TIMELINE_H__
#define __INCLUDED_STEAM_TIMELINE_H__

#include "base.h"

class Steam_Timeline :
public ISteamTimeline
{
    class Settings *settings{};
    class Networking *network{};
    class SteamCallResults *callback_results{};
    class SteamCallBacks *callbacks{};
    class RunEveryRunCB *run_every_runcb{};
    std::chrono::time_point<std::chrono::steady_clock> initialized_time = std::chrono::steady_clock::now();
    FSteamNetworkingSocketsDebugOutput debug_function{};

    static void steam_callback(void *object, Common_Message *msg);
    static void steam_run_every_runcb(void *object);

public:
    Steam_Timeline(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb);
    ~Steam_Timeline();

    void SetTimelineStateDescription( const char *pchDescription, float flTimeDelta );

    void ClearTimelineStateDescription( float flTimeDelta );

    void AddTimelineEvent( const char *pchIcon, const char *pchTitle, const char *pchDescription, uint32 unPriority, float flStartOffsetSeconds, float flDurationSeconds, ETimelineEventClipPriority ePossibleClip );

    void SetTimelineGameMode( ETimelineGameMode eMode );

    void RunCallbacks();

    void Callback(Common_Message *msg);

};

#endif // __INCLUDED_STEAM_TIMELINE_H__