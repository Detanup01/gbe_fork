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

    // timeline starts with a default event as seen here: https://www.youtube.com/watch?v=YwBD0E4-EsI
    SetTimelineGameMode(ETimelineGameMode::k_ETimelineGameMode_Invalid);
}

Steam_Timeline::~Steam_Timeline()
{
    // this->network->rmCallback(CALLBACK_ID_USER_STATUS, settings->get_local_steam_id(), &Steam_Timeline::steam_callback, this);
    this->run_every_runcb->remove(&Steam_Timeline::steam_run_every_runcb, this);
}


// Sets a description for the current game state in the timeline. These help the user to find specific
// moments in the timeline when saving clips. Setting a new state description replaces any previous
// description.
// 
// Examples could include:
//  * Where the user is in the world in a single player game
//  * Which round is happening in a multiplayer game
//  * The current score for a sports game
// 	
// Parameters:
// - pchDescription: provide a localized string in the language returned by SteamUtils()->GetSteamUILanguage()
// - flTimeDelta: The time offset in seconds to apply to this event. Negative times indicate an 
//			event that happened in the past.
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

// Use this to mark an event on the Timeline. The event can be instantaneous or take some amount of time
// to complete, depending on the value passed in flDurationSeconds
// 
// Examples could include:
//   * a boss battle
//   * a cut scene
//   * a large team fight
//   * picking up a new weapon or ammunition
//   * scoring a goal
// 	
// Parameters:
// 
// - pchIcon: specify the name of the icon uploaded through the Steamworks Partner Site for your title
//   or one of the provided icons that start with steam_
// - pchTitle & pchDescription: provide a localized string in the language returned by
//	 SteamUtils()->GetSteamUILanguage()
// - unPriority: specify how important this range is compared to other markers provided by the game. 
//   Ranges with larger priority values will be displayed more prominently in the UI. This value
//   may be between 0 and k_unMaxTimelinePriority.
// - flStartOffsetSeconds: The time that this range started relative to now. Negative times 
//   indicate an event that happened in the past.
// - flDurationSeconds: How long the time range should be in seconds. For instantaneous events, this
//   should be 0
// - ePossibleClip: By setting this parameter to Featured or Standard, the game indicates to Steam that it
//   would be appropriate to offer this range as a clip to the user. For instantaneous events, the
//   suggested clip will be for a short time before and after the event itself.
void Steam_Timeline::AddTimelineEvent( const char *pchIcon, const char *pchTitle, const char *pchDescription, uint32 unPriority, float flStartOffsetSeconds, float flDurationSeconds, ETimelineEventClipPriority ePossibleClip )
{
    PRINT_DEBUG("icon='%s' | '%s' - '%s', %u, [%f, %f) %i", pchIcon, pchTitle, pchDescription, unPriority, flStartOffsetSeconds, flDurationSeconds, (int)ePossibleClip);
    std::lock_guard lock(global_mutex);

    auto &new_event = timeline_events.emplace_back(TimelineEvent_t{});
    new_event.pchIcon = pchIcon;
    new_event.pchTitle = pchTitle;
    new_event.pchDescription = pchDescription;
    new_event.unPriority = unPriority;

    new_event.flStartOffsetSeconds = flStartOffsetSeconds;
    
    // make events last at least 1 sec
    if (static_cast<long>(flDurationSeconds * 1000) < 1000) { // < 1000ms
        flDurationSeconds = 1;
    }
    // for events with priority=ETimelineEventClipPriority::k_ETimelineEventClipPriority_Featured steam creates ~8 sec clip
    // seen here: https://www.youtube.com/watch?v=YwBD0E4-EsI
    if (flDurationSeconds < PRIORITY_CLIP_MIN_SEC && ePossibleClip == ETimelineEventClipPriority::k_ETimelineEventClipPriority_Featured) {
        flDurationSeconds = PRIORITY_CLIP_MIN_SEC;
    }
    new_event.flDurationSeconds = flDurationSeconds;

    new_event.ePossibleClip = ePossibleClip;
}

// Changes the color of the timeline bar. See ETimelineGameMode comments for how to use each value
void Steam_Timeline::SetTimelineGameMode( ETimelineGameMode eMode )
{
    PRINT_DEBUG("%i", (int)eMode);
    std::lock_guard lock(global_mutex);

    auto &new_timeline_state = timeline_states.emplace_back(TimelineState_t{});
    new_timeline_state.bar_color = eMode;
}

void Steam_Timeline::SetTimelineTooltip(const char* pchDescription, float flTimeDelta)
{
    SetTimelineStateDescription(pchDescription, flTimeDelta);
}

void Steam_Timeline::ClearTimelineTooltip(float flTimeDelta)
{
    ClearTimelineStateDescription(flTimeDelta);
}

TimelineEventHandle_t Steam_Timeline::AddInstantaneousTimelineEvent(const char* pchTitle, const char* pchDescription, const char* pchIcon, uint32 unIconPriority, float flStartOffsetSeconds, ETimelineEventClipPriority ePossibleClip)
{
    return 0;
}

TimelineEventHandle_t Steam_Timeline::AddRangeTimelineEvent(const char* pchTitle, const char* pchDescription, const char* pchIcon, uint32 unIconPriority, float flStartOffsetSeconds, float flDuration, ETimelineEventClipPriority ePossibleClip)
{
    return 0;
}

TimelineEventHandle_t Steam_Timeline::StartRangeTimelineEvent(const char* pchTitle, const char* pchDescription, const char* pchIcon, uint32 unPriority, float flStartOffsetSeconds, ETimelineEventClipPriority ePossibleClip)
{
    return 0;
}

void Steam_Timeline::UpdateRangeTimelineEvent(TimelineEventHandle_t ulEvent, const char* pchTitle, const char* pchDescription, const char* pchIcon, uint32 unPriority, ETimelineEventClipPriority ePossibleClip)
{

}

void Steam_Timeline::EndRangeTimelineEvent(TimelineEventHandle_t ulEvent, float flEndOffsetSeconds)
{

}

void Steam_Timeline::RemoveTimelineEvent(TimelineEventHandle_t ulEvent)
{

}

SteamAPICall_t Steam_Timeline::DoesEventRecordingExist(TimelineEventHandle_t ulEvent)
{
    return 0;
}

void Steam_Timeline::StartGamePhase()
{

}

void Steam_Timeline::EndGamePhase()
{

}

void Steam_Timeline::SetGamePhaseID(const char* pchPhaseID)
{

}

SteamAPICall_t Steam_Timeline::DoesGamePhaseRecordingExist(const char* pchPhaseID)
{
    return 0;
}

void Steam_Timeline::AddGamePhaseTag(const char* pchTagName, const char* pchTagIcon, const char* pchTagGroup, uint32 unPriority)
{

}

void Steam_Timeline::SetGamePhaseAttribute(const char* pchAttributeGroup, const char* pchAttributeValue, uint32 unPriority)
{

}

void Steam_Timeline::OpenOverlayToGamePhase(const char* pchPhaseID)
{

}

void Steam_Timeline::OpenOverlayToTimelineEvent(const TimelineEventHandle_t ulEvent)
{

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
