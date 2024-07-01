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

#include "dll/steam_timeline.h"

// https://partner.steamgames.com/doc/api/ISteamTimeline


void Steam_Timeline::steam_callback(void *object, Common_Message *msg)
{
    // PRINT_DEBUG_ENTRY();

    auto instance = (Steam_Timeline *)object;
    instance->Callback(msg);
}

void Steam_Timeline::steam_run_every_runcb(void *object)
{
    // PRINT_DEBUG_ENTRY();

    auto instance = (Steam_Timeline *)object;
    instance->RunCallbacks();
}


Steam_Timeline::Steam_Timeline(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb)
{
    this->settings = settings;
    this->network = network;
    this->callback_results = callback_results;
    this->callbacks = callbacks;
    this->run_every_runcb = run_every_runcb;
    
    // this->network->setCallback(CALLBACK_ID_USER_STATUS, settings->get_local_steam_id(), &Steam_Timeline::steam_callback, this);
    this->run_every_runcb->add(&Steam_Timeline::steam_run_every_runcb, this);
}

Steam_Timeline::~Steam_Timeline()
{
    // this->network->rmCallback(CALLBACK_ID_USER_STATUS, settings->get_local_steam_id(), &Steam_Timeline::steam_callback, this);
    this->run_every_runcb->remove(&Steam_Timeline::steam_run_every_runcb, this);
}

void Steam_Timeline::SetTimelineStateDescription( const char *pchDescription, float flTimeDelta )
{
    PRINT_DEBUG("'%s' %f", pchDescription, flTimeDelta);
    std::lock_guard lock(global_mutex);

    const auto target_timepoint = std::chrono::system_clock::now() + std::chrono::milliseconds(static_cast<long>(flTimeDelta * 1000));

    // reverse iterators to search from end
    auto event_it = std::find_if(timeline_states.rbegin(), timeline_states.rend(), [this, &target_timepoint](const TimelineState_t &item) {
        return target_timepoint >= item.time_added;
    });

    if (timeline_states.rend() != event_it) {
        PRINT_DEBUG("setting timeline state description");
        if (pchDescription) {
            event_it->description = pchDescription;
        } else {
            event_it->description.clear();
        }
    }

}


void Steam_Timeline::ClearTimelineStateDescription( float flTimeDelta )
{
    PRINT_DEBUG("%f", flTimeDelta);
    std::lock_guard lock(global_mutex);

    const auto target_timepoint = std::chrono::system_clock::now() + std::chrono::milliseconds(static_cast<long>(flTimeDelta * 1000));

    // reverse iterators to search from end
    auto event_it = std::find_if(timeline_states.rbegin(), timeline_states.rend(), [this, &target_timepoint](const TimelineState_t &item) {
        return target_timepoint >= item.time_added;
    });

    if (timeline_states.rend() != event_it) {
        PRINT_DEBUG("clearing timeline state description");
        event_it->description.clear();
    }

}


void Steam_Timeline::AddTimelineEvent( const char *pchIcon, const char *pchTitle, const char *pchDescription, uint32 unPriority, float flStartOffsetSeconds, float flDurationSeconds, ETimelineEventClipPriority ePossibleClip )
{
    PRINT_DEBUG("'%s' | '%s' - '%s', %u, [%f, %f) %i", pchIcon, pchTitle, pchDescription, unPriority, flStartOffsetSeconds, flDurationSeconds, (int)ePossibleClip);
    std::lock_guard lock(global_mutex);

    auto &new_event = timeline_events.emplace_back(TimelineEvent_t{});
    new_event.pchIcon = pchIcon;
    new_event.pchTitle = pchTitle;
    new_event.pchDescription = pchDescription;
    new_event.unPriority = unPriority;

    new_event.flStartOffsetSeconds = flStartOffsetSeconds;
    
    // for instantanious event with flDurationSeconds=0 steam creates 8 sec clip
    if (static_cast<long>(flDurationSeconds * 1000) <= 100) { // <= 100ms
        flDurationSeconds = 8;
    }
    new_event.flDurationSeconds = flDurationSeconds;

    new_event.ePossibleClip = ePossibleClip;
}


void Steam_Timeline::SetTimelineGameMode( ETimelineGameMode eMode )
{
    PRINT_DEBUG("%i", (int)eMode);
    std::lock_guard lock(global_mutex);

    if (timeline_states.empty()) return;

    timeline_states.back().bar_color = eMode;
}



void Steam_Timeline::RunCallbacks()
{
    
}



void Steam_Timeline::Callback(Common_Message *msg)
{
    if (msg->has_low_level()) {
        if (msg->low_level().type() == Low_Level::CONNECT) {
            
        }

        if (msg->low_level().type() == Low_Level::DISCONNECT) {

        }
    }
}
