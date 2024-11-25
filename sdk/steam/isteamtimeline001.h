//====== Copyright © Valve Corporation, All rights reserved. =======
//
// Purpose: interface to Steam Timeline
//
//=============================================================================

#ifndef ISTEAMTIMELINE001_H
#define ISTEAMTIMELINE001_H
#ifdef STEAM_WIN32
#pragma once
#endif

//-----------------------------------------------------------------------------
// Purpose: Steam Timeline API
//-----------------------------------------------------------------------------
class ISteamTimeline001
{
public:

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
	virtual void SetTimelineStateDescription( const char *pchDescription, float flTimeDelta ) = 0;
	virtual void ClearTimelineStateDescription( float flTimeDelta ) = 0;

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
	virtual void AddTimelineEvent_old( const char *pchIcon, const char *pchTitle, const char *pchDescription, uint32 unPriority, float flStartOffsetSeconds, float flDurationSeconds, ETimelineEventClipPriority ePossibleClip ) = 0;

	// Changes the color of the timeline bar. See ETimelineGameMode comments for how to use each value
	virtual void SetTimelineGameMode( ETimelineGameMode eMode ) = 0;
};


#endif // ISTEAMTIMELINE001_H