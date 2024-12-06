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


const std::chrono::system_clock::time_point& Steam_Timeline::TimelineEvent_t::get_time_added() const
{
    return time_added;
}



const std::chrono::system_clock::time_point& Steam_Timeline::TimelineState_t::get_time_added() const
{
    return time_added;
}



const std::chrono::system_clock::time_point& Steam_Timeline::TimelineGamePhase_t::get_time_added() const
{
    return time_added;
}



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
    PRINT_DEBUG("adding an initial game mode");
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
void Steam_Timeline::SetTimelineTooltip( const char *pchDescription, float flTimeDelta )
{
    PRINT_DEBUG("'%s' %f", pchDescription, flTimeDelta);
    std::lock_guard lock(timeline_mutex);

    const auto target_timepoint = std::chrono::system_clock::now() + std::chrono::milliseconds(static_cast<long long>(flTimeDelta * 1000));

    // reverse iterators to find last/nearest match in recent time
    auto event_it = std::find_if(timeline_states.rbegin(), timeline_states.rend(), [this, &target_timepoint](const TimelineState_t &item) {
        return target_timepoint >= item.get_time_added();
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

void Steam_Timeline::ClearTimelineTooltip( float flTimeDelta )
{
    PRINT_DEBUG("%f", flTimeDelta);
    std::lock_guard lock(timeline_mutex);

    const auto target_timepoint = std::chrono::system_clock::now() + std::chrono::milliseconds(static_cast<long long>(flTimeDelta * 1000));

    // reverse iterators to find last/nearest match in recent time
    auto event_it = std::find_if(timeline_states.rbegin(), timeline_states.rend(), [this, &target_timepoint](const TimelineState_t &item) {
        return target_timepoint >= item.get_time_added();
    });

    if (timeline_states.rend() != event_it) {
        PRINT_DEBUG("clearing timeline state description");
        event_it->description.clear();
    }

}

void Steam_Timeline::SetTimelineStateDescription( const char *pchDescription, float flTimeDelta )
{
    PRINT_DEBUG("old v1");
    SetTimelineTooltip(pchDescription, flTimeDelta);
}

void Steam_Timeline::ClearTimelineStateDescription( float flTimeDelta )
{
    PRINT_DEBUG("old v1");
    ClearTimelineTooltip(flTimeDelta);
}

void Steam_Timeline::SetTimelineGameMode( ETimelineGameMode eMode )
{
    PRINT_DEBUG("%i", (int)eMode);
    std::lock_guard lock(timeline_mutex);

    auto &new_timeline_state = timeline_states.emplace_back(TimelineState_t{});
    new_timeline_state.bar_color = eMode;
}

TimelineEventHandle_t Steam_Timeline::AddInstantaneousTimelineEvent( const char *pchTitle, const char *pchDescription, const char *pchIcon, uint32 unIconPriority, float flStartOffsetSeconds, ETimelineEventClipPriority ePossibleClip )
{
    PRINT_DEBUG_TODO();
    return AddRangeTimelineEvent(pchTitle, pchDescription, pchIcon, unIconPriority, flStartOffsetSeconds, 0, ePossibleClip);
}

TimelineEventHandle_t Steam_Timeline::AddRangeTimelineEvent( const char *pchTitle, const char *pchDescription, const char *pchIcon, uint32 unIconPriority, float flStartOffsetSeconds, float flDuration, ETimelineEventClipPriority ePossibleClip)
{
    PRINT_DEBUG("'%s' ('%s') icon='%s', %u, [%f, %f) %i", pchTitle, pchDescription, pchIcon, unIconPriority, flStartOffsetSeconds, flDuration, (int)ePossibleClip);
    std::lock_guard lock(timeline_mutex);

    auto event_id = StartRangeTimelineEvent(pchTitle, pchDescription, pchIcon, unIconPriority, flStartOffsetSeconds, ePossibleClip);
    if (!event_id || event_id > timeline_events.size()) return 0;

    auto& my_event = timeline_events[static_cast<size_t>(event_id - 1)];
    my_event.ended = true; // ranged and instantaneous events are ended/closed events, they can't be modified later according to docs

    // make events last at least 1 sec
    if (static_cast<long long>(flDuration * 1000) < 1000LL) { // < 1000ms
        flDuration = 1;
    }
    // for events with priority=ETimelineEventClipPriority::k_ETimelineEventClipPriority_Featured steam creates ~30 sec clip
    if (flDuration < PRIORITY_CLIP_MIN_SEC && ePossibleClip == ETimelineEventClipPriority::k_ETimelineEventClipPriority_Featured) {
        flDuration = PRIORITY_CLIP_MIN_SEC;
    }
    if (flDuration > k_flMaxTimelineEventDuration) {
        flDuration = k_flMaxTimelineEventDuration;
    }

    my_event.flDurationSeconds = flDuration;

    return event_id;
}

TimelineEventHandle_t Steam_Timeline::AddTimelineEvent( const char *pchTitle, const char *pchDescription, const char *pchIcon, uint32 unIconPriority, float flStartOffsetSeconds, float flDurationSeconds, ETimelineEventClipPriority ePossibleClip )
{
    PRINT_DEBUG("undocumented v2/v3");

    // this is how actual steamclient64.dll implements it
    if (flDurationSeconds > 0) {
        return AddRangeTimelineEvent(pchTitle, pchDescription, pchIcon, unIconPriority, flStartOffsetSeconds, flDurationSeconds, ePossibleClip);
    } else {
        return AddInstantaneousTimelineEvent(pchTitle, pchDescription, pchIcon, unIconPriority, flStartOffsetSeconds, ePossibleClip);
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
void Steam_Timeline::AddTimelineEvent_old( const char *pchIcon, const char *pchTitle, const char *pchDescription, uint32 unPriority, float flStartOffsetSeconds, float flDurationSeconds, ETimelineEventClipPriority ePossibleClip )
{
    PRINT_DEBUG("old v1");

    // this is how actual steamclient64.dll implements it
    if (flDurationSeconds > 0) {
        AddRangeTimelineEvent(pchTitle, pchDescription, pchIcon, unPriority, flStartOffsetSeconds, flDurationSeconds, ePossibleClip);
    } else {
        AddInstantaneousTimelineEvent(pchTitle, pchDescription, pchIcon, unPriority, flStartOffsetSeconds, ePossibleClip);
    }
}

TimelineEventHandle_t Steam_Timeline::StartRangeTimelineEvent( const char *pchTitle, const char *pchDescription, const char *pchIcon, uint32 unPriority, float flStartOffsetSeconds, ETimelineEventClipPriority ePossibleClip )
{
    PRINT_DEBUG("'%s' ('%s') icon='%s', %u, @[%f]sec %i", pchTitle, pchDescription, pchIcon, unPriority, flStartOffsetSeconds, (int)ePossibleClip);
    std::lock_guard lock(timeline_mutex);
    // this adds a new event, but the duration is set once EndRangeTimelineEvent is called
    // also its "ended" flag is not set

    auto &new_event = timeline_events.emplace_back(TimelineEvent_t{});
    new_event.pchTitle = pchTitle ? pchTitle : "";
    new_event.pchDescription = pchDescription ? pchDescription : "";
    new_event.pchIcon = pchIcon ? pchIcon : "";
    new_event.unPriority = unPriority;
    new_event.flStartOffsetSeconds = flStartOffsetSeconds;
    new_event.ePossibleClip = ePossibleClip;
    
    auto new_event_id = timeline_events.size(); // never return 0, most APIs in other interfaces use it for invalid IDs
    PRINT_DEBUG("  new event ID = [%zu]", new_event_id);
    return static_cast<TimelineEventHandle_t>(new_event_id);
}

void Steam_Timeline::UpdateRangeTimelineEvent( TimelineEventHandle_t ulEvent, const char *pchTitle, const char *pchDescription, const char *pchIcon, uint32 unPriority, ETimelineEventClipPriority ePossibleClip )
{
    PRINT_DEBUG("[%llu] '%s' ('%s') | icon='%s', %u, %i", ulEvent, pchTitle, pchDescription, pchIcon, unPriority, (int)ePossibleClip);
    std::lock_guard lock(timeline_mutex);

    if (!ulEvent || ulEvent > timeline_events.size()) return;

    auto& my_event = timeline_events[static_cast<size_t>(ulEvent - 1)];
    if (my_event.ended) return;
    
    if (pchTitle) {
        my_event.pchTitle = pchTitle;
    } else {
        my_event.pchTitle.clear();
    }

    if (pchDescription) {
        my_event.pchDescription = pchDescription;
    } else {
        my_event.pchDescription.clear();
    }

    if (pchIcon) {
        my_event.pchIcon = pchIcon;
    } else {
        my_event.pchIcon.clear();
    }

    my_event.unPriority = unPriority;
    my_event.ePossibleClip = ePossibleClip;

    PRINT_DEBUG("  updated event");
}

void Steam_Timeline::EndRangeTimelineEvent( TimelineEventHandle_t ulEvent, float flEndOffsetSeconds )
{
    PRINT_DEBUG("[%llu] %f", ulEvent, flEndOffsetSeconds);
    std::lock_guard lock(timeline_mutex);

    if (!ulEvent || ulEvent > timeline_events.size()) return;

    auto& my_event = timeline_events[static_cast<size_t>(ulEvent - 1)];
    if (my_event.ended) return;

    my_event.ended = true;

    auto end_timepoint = std::chrono::system_clock::now();
    auto start_timepoint = my_event.get_time_added() + std::chrono::milliseconds(static_cast<long long>(my_event.flStartOffsetSeconds * 1000));
    auto duration_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end_timepoint - start_timepoint);

    my_event.flDurationSeconds = duration_ms.count() / 1000.0f;

    PRINT_DEBUG("  ended event // TODO show in the UI");
}

void Steam_Timeline::RemoveTimelineEvent( TimelineEventHandle_t ulEvent )
{
    PRINT_DEBUG("[%llu]", ulEvent);
    std::lock_guard lock(timeline_mutex);

    if (!ulEvent || ulEvent > timeline_events.size()) return;

    timeline_events.erase(timeline_events.begin() + static_cast<size_t>(ulEvent - 1));

    PRINT_DEBUG("  removed event // TODO remove from the UI");
}

STEAM_CALL_RESULT( SteamTimelineEventRecordingExists_t )
SteamAPICall_t Steam_Timeline::DoesEventRecordingExist(TimelineEventHandle_t ulEvent)
{
    PRINT_DEBUG("[%llu] // TODO", ulEvent);
    std::lock_guard lock(timeline_mutex);

    if (!ulEvent || ulEvent > timeline_events.size()) {
        SteamTimelineEventRecordingExists_t data_invalid{};
        data_invalid.m_bRecordingExists = false;
        data_invalid.m_ulEventID = ulEvent;
        auto ret = callback_results->addCallResult(data_invalid.k_iCallback, &data_invalid, sizeof(data_invalid));
        callbacks->addCBResult(data_invalid.k_iCallback, &data_invalid, sizeof(data_invalid));
        return ret;
    }

    auto& my_event = timeline_events[static_cast<size_t>(ulEvent - 1)];
    auto recordings_count = my_event.recordings.size();

    SteamTimelineEventRecordingExists_t data{};
    data.m_bRecordingExists = !my_event.recordings.empty();
    data.m_ulEventID = ulEvent;
    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return ret;
}

void Steam_Timeline::StartGamePhase()
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard lock(timeline_mutex);

    timeline_game_phases.emplace_back(TimelineGamePhase_t{});
}

void Steam_Timeline::EndGamePhase()
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard lock(timeline_mutex);

    if (timeline_game_phases.empty()) return;

    auto &last_game_phase = timeline_game_phases.back();
    last_game_phase.ended = true;
}

void Steam_Timeline::SetGamePhaseID( const char *pchPhaseID )
{
    PRINT_DEBUG("['%s']", pchPhaseID);
    std::lock_guard lock(timeline_mutex);

    if (timeline_game_phases.empty()) return;

    auto &last_game_phase = timeline_game_phases.back();
    if (last_game_phase.ended) return;

    last_game_phase.pchPhaseID = pchPhaseID ? pchPhaseID : "";
    PRINT_DEBUG("  changed phase ID");
}

STEAM_CALL_RESULT( SteamTimelineGamePhaseRecordingExists_t )
SteamAPICall_t Steam_Timeline::DoesGamePhaseRecordingExist( const char *pchPhaseID )
{
    PRINT_DEBUG("'%s' // TODO", pchPhaseID);
    std::lock_guard lock(timeline_mutex);

    if (!pchPhaseID) pchPhaseID = "";
    std::string_view game_phase_id_view(pchPhaseID);

    const auto trigger_failure = [game_phase_id_view, this]() {
        SteamTimelineGamePhaseRecordingExists_t data_invalid{};
        auto chars_copied = game_phase_id_view.copy(data_invalid.m_rgchPhaseID, sizeof(data_invalid.m_rgchPhaseID) - 1);
        data_invalid.m_rgchPhaseID[chars_copied] = 0;
        data_invalid.m_ulLongestClipMS = 0;
        data_invalid.m_ulRecordingMS = 0;
        data_invalid.m_unClipCount = 0;
        data_invalid.m_unScreenshotCount = 0;
        
        auto ret = callback_results->addCallResult(data_invalid.k_iCallback, &data_invalid, sizeof(data_invalid));
        callbacks->addCBResult(data_invalid.k_iCallback, &data_invalid, sizeof(data_invalid));
        return ret;
    };

    if (timeline_game_phases.empty()) {
        return trigger_failure();
    }
    
    auto phase_it = std::find_if(timeline_game_phases.begin(), timeline_game_phases.end(), [game_phase_id_view](const TimelineGamePhase_t &item){
        return game_phase_id_view == item.pchPhaseID;
    });
    if (timeline_game_phases.end() == phase_it) {
        return trigger_failure();
    }

    // TODO return actual count ?
    auto recordings_count = phase_it->recordings.size();
    return trigger_failure();
}

void Steam_Timeline::AddGamePhaseTag( const char *pchTagName, const char *pchTagIcon, const char *pchTagGroup, uint32 unPriority )
{
    PRINT_DEBUG("['%s']: '%s' '%s' <%u>", pchTagGroup, pchTagName, pchTagIcon, unPriority);
    std::lock_guard lock(timeline_mutex);

    if (timeline_game_phases.empty()) return;

    auto &last_game_phase = timeline_game_phases.back();
    if (last_game_phase.ended) return;

    if (!pchTagGroup) pchTagGroup = "";

    auto &phase_tag = last_game_phase.tags[pchTagGroup].emplace_back(Steam_Timeline::TimelineGamePhase_t::Tag_t{});
    phase_tag.pchTagName = pchTagName ? pchTagName : "";
    phase_tag.pchTagIcon = pchTagIcon ? pchTagIcon : "";
    phase_tag.unPriority = unPriority;
    PRINT_DEBUG("  added phase tag");
}

void Steam_Timeline::SetGamePhaseAttribute( const char *pchAttributeGroup, const char *pchAttributeValue, uint32 unPriority )
{
    PRINT_DEBUG("['%s']: '%s' <%u>", pchAttributeGroup, pchAttributeValue, unPriority);
    std::lock_guard lock(timeline_mutex);

    if (timeline_game_phases.empty()) return;

    auto &last_game_phase = timeline_game_phases.back();
    if (last_game_phase.ended) return;

    if (!pchAttributeGroup) pchAttributeGroup = "";

    auto &phase_att = last_game_phase.attributes[pchAttributeGroup];
    phase_att.pchAttributeValue = pchAttributeValue ? pchAttributeValue : "";
    phase_att.unPriority = unPriority;
    PRINT_DEBUG("  changed phase attribute");
}

void Steam_Timeline::OpenOverlayToGamePhase( const char *pchPhaseID )
{
    PRINT_DEBUG("['%s'] // TODO", pchPhaseID);
    std::lock_guard lock(timeline_mutex);

}

void Steam_Timeline::OpenOverlayToTimelineEvent( const TimelineEventHandle_t ulEvent )
{
    PRINT_DEBUG("[%llu] // TODO", ulEvent);
    std::lock_guard lock(timeline_mutex);

}


uint32 Steam_Timeline::unknown_ret0_1()
{
    PRINT_DEBUG_TODO();
    return 0;
}

uint32 Steam_Timeline::unknown_ret0_2()
{
    PRINT_DEBUG_TODO();
    return 0;
}

void Steam_Timeline::unknown_nop_3()
{
    PRINT_DEBUG_TODO();
}

void Steam_Timeline::unknown_nop_4()
{
    PRINT_DEBUG_TODO();
}

void Steam_Timeline::unknown_nop_5()
{
    PRINT_DEBUG_TODO();
}

void Steam_Timeline::unknown_nop_6()
{
    PRINT_DEBUG_TODO();
}

void Steam_Timeline::unknown_nop_7()
{
    PRINT_DEBUG_TODO();
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
