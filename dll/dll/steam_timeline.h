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
public ISteamTimeline,
public ISteamTimeline003,
public ISteamTimeline002,
public ISteamTimeline001
{
private:
    // "Steam Client Update - November 12th": * Increased Instant Clip default duration from 10s to 30s.
    constexpr const static float PRIORITY_CLIP_MIN_SEC = 30.0f;

    // TimelineState_t controls the bar of the timeline, independant of anything else even the events
    struct TimelineState_t
    {
    private:
        // emu specific: time when this state was changed via 'Steam_Timeline::SetTimelineGameMode()'
        std::chrono::system_clock::time_point time_added = std::chrono::system_clock::now();
        
    public:
        const std::chrono::system_clock::time_point& get_time_added() const;

        std::string description{}; // A localized string in the language returned by SteamUtils()->GetSteamUILanguage()
        ETimelineGameMode bar_color{}; // the color of the timeline bar
    };
    
    // TimelineEvent_t are little clips/markers added over the bar of the timeline
    struct TimelineEvent_t
    {
    private:
        // emu specific: time when this event was added to the list via 'Steam_Timeline::AddTimelineEvent()'
        std::chrono::system_clock::time_point time_added = std::chrono::system_clock::now();

    public:
        // TODO not documented in public SDK yet
        struct Recording_t
        {

        };

        const std::chrono::system_clock::time_point& get_time_added() const;

        // The name of the icon to show at the timeline at this point. This can be one of the icons uploaded through the Steamworks partner Site for your title, or one of the provided icons that start with steam_. The Steam Timelines overview includes a list of available icons.
        // https://partner.steamgames.com/doc/features/timeline#icons
        std::string pchIcon{};

        // Title-provided localized string in the language returned by SteamUtils()->GetSteamUILanguage().
        std::string pchTitle{};

        // Title-provided localized string in the language returned by SteamUtils()->GetSteamUILanguage().
        std::string pchDescription{};

        // Provide the priority to use when the UI is deciding which icons to display in crowded parts of the timeline. Events with larger priority values will be displayed more prominently than events with smaller priority values. This value must be between 0 and k_unMaxTimelinePriority.
        uint32 unPriority{}; 

        // One use of this parameter is to handle events whose significance is not clear until after the fact. For instance if the player starts a damage over time effect on another player, which kills them 3.5 seconds later, the game could pass -3.5 as the start offset and cause the event to appear in the timeline where the effect started.
        float flStartOffsetSeconds{};

        // The duration of the event, in seconds. Pass 0 for instantaneous events.
        float flDurationSeconds{};

        // Allows the game to describe events that should be suggested to the user as possible video clips.
        ETimelineEventClipPriority ePossibleClip{};

        bool ended = false;

        // TODO not documented in public SDK yet
        // emu specific: available recordings for this event
        std::vector<Recording_t> recordings{};
    };

    struct TimelineGamePhase_t
    {
    private:
        // emu specific: time when this state was changed via 'Steam_Timeline::SetTimelineGameMode()'
        std::chrono::system_clock::time_point time_added = std::chrono::system_clock::now();
        
    public:
        struct Attribute_t
        {
            std::string pchAttributeValue{};
            uint32 unPriority{};
        };

        struct Tag_t
        {
            std::string pchTagName{};
            std::string pchTagIcon{};
            uint32 unPriority{};
        };

        // TODO not documented in public SDK yet
        struct Recording_t
        {

        };

        const std::chrono::system_clock::time_point& get_time_added() const;

        std::string pchTagIcon{}; // The name of a game provided timeline icon or builtin "steam_" icon
        std::string pchPhaseID{}; // A game-provided persistent ID for a game phase. This could be a the match ID in a multiplayer game, etc...
        std::string pchTagName{}; // The localized name of the tag in the language returned by SteamUtils()->GetSteamUILanguage()
        std::string pchTagGroup{}; // The localized name of the tag group

        std::map<std::string, Attribute_t> attributes{};
        std::map<std::string, std::vector<Tag_t>> tags{};

        bool ended = false;
        // emu specific: available recordings for this phase
        std::vector<Recording_t> recordings{};
    };

    std::recursive_mutex timeline_mutex{};
    class Settings *settings{};
    class Networking *network{};
    class SteamCallResults *callback_results{};
    class SteamCallBacks *callbacks{};
    class RunEveryRunCB *run_every_runcb{};

    std::vector<TimelineState_t> timeline_states{};
    std::vector<TimelineEvent_t> timeline_events{};
    std::vector<TimelineGamePhase_t> timeline_game_phases{};
    

    // unconditional periodic callback
    void RunCallbacks();
    // network callback, triggered once we have a network message
    void Callback(Common_Message *msg);

    static void steam_callback(void *object, Common_Message *msg);
    static void steam_run_every_runcb(void *object);

public:
    Steam_Timeline(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb);
    ~Steam_Timeline();

    void SetTimelineTooltip( const char *pchDescription, float flTimeDelta );
    void ClearTimelineTooltip( float flTimeDelta );

    void SetTimelineStateDescription( const char *pchDescription, float flTimeDelta ); // renamed to SetTimelineTooltip() in sdk v1.61
    void ClearTimelineStateDescription( float flTimeDelta ); // renamed to ClearTimelineTooltip() in sdk v1.61

    // Changes the color of the timeline bar. See ETimelineGameMode comments for how to use each value
    void SetTimelineGameMode( ETimelineGameMode eMode );

    TimelineEventHandle_t AddInstantaneousTimelineEvent( const char *pchTitle, const char *pchDescription, const char *pchIcon, uint32 unIconPriority, float flStartOffsetSeconds = 0.f, ETimelineEventClipPriority ePossibleClip = k_ETimelineEventClipPriority_None );
    TimelineEventHandle_t AddRangeTimelineEvent( const char *pchTitle, const char *pchDescription, const char *pchIcon, uint32 unIconPriority, float flStartOffsetSeconds = 0.f, float flDuration = 0.f, ETimelineEventClipPriority ePossibleClip = k_ETimelineEventClipPriority_None );
    
    TimelineEventHandle_t AddTimelineEvent( const char *pchTitle, const char *pchDescription, const char *pchIcon, uint32 unIconPriority, float flStartOffsetSeconds, float flDurationSeconds, ETimelineEventClipPriority ePossibleClip );
    void AddTimelineEvent_old( const char *pchIcon, const char *pchTitle, const char *pchDescription, uint32 unPriority, float flStartOffsetSeconds, float flDurationSeconds, ETimelineEventClipPriority ePossibleClip );

    TimelineEventHandle_t StartRangeTimelineEvent( const char *pchTitle, const char *pchDescription, const char *pchIcon, uint32 unPriority, float flStartOffsetSeconds, ETimelineEventClipPriority ePossibleClip );

    void UpdateRangeTimelineEvent( TimelineEventHandle_t ulEvent, const char *pchTitle, const char *pchDescription, const char *pchIcon, uint32 unPriority, ETimelineEventClipPriority ePossibleClip );

    void EndRangeTimelineEvent( TimelineEventHandle_t ulEvent, float flEndOffsetSeconds );

    void RemoveTimelineEvent( TimelineEventHandle_t ulEvent );

    STEAM_CALL_RESULT( SteamTimelineEventRecordingExists_t )
    SteamAPICall_t DoesEventRecordingExist( TimelineEventHandle_t ulEvent );

    void StartGamePhase();
    
    void EndGamePhase();

    void SetGamePhaseID( const char *pchPhaseID );
    STEAM_CALL_RESULT( SteamTimelineGamePhaseRecordingExists_t )
    SteamAPICall_t DoesGamePhaseRecordingExist( const char *pchPhaseID );

    void AddGamePhaseTag( const char *pchTagName, const char *pchTagIcon, const char *pchTagGroup, uint32 unPriority );

    void SetGamePhaseAttribute( const char *pchAttributeGroup, const char *pchAttributeValue, uint32 unPriority );

    void OpenOverlayToGamePhase( const char *pchPhaseID );

    void OpenOverlayToTimelineEvent( const TimelineEventHandle_t ulEvent );


    uint32 unknown_ret0_1();
    uint32 unknown_ret0_2();
    void unknown_nop_3();
    void unknown_nop_4();
    void unknown_nop_5();
    void unknown_nop_6();
    void unknown_nop_7();

};

#endif // __INCLUDED_STEAM_TIMELINE_H__
