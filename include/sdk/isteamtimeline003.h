#ifndef ISTEAMTIMELINE003_H
#define ISTEAMTIMELINE003_H
#ifdef STEAM_WIN32
#pragma once
#endif

// this interface version is not found in public SDK archives, it is based on reversing the returned vftable from steamclient64.dll

class ISteamTimeline003
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
	virtual void SetTimelineTooltip( const char *pchDescription, float flTimeDelta ) = 0;
	virtual void ClearTimelineTooltip( float flTimeDelta ) = 0;

	// Changes the color of the timeline bar. See ETimelineGameMode comments for how to use each value
	virtual void SetTimelineGameMode( ETimelineGameMode eMode ) = 0;


///////////////////////////////////// the following functions are not found in public SDK
	virtual TimelineEventHandle_t AddTimelineEvent( const char *pchTitle, const char *pchDescription, const char *pchIcon, uint32 unIconPriority, float flStartOffsetSeconds, float flDurationSeconds, ETimelineEventClipPriority ePossibleClip ) = 0;

	virtual uint32 unknown_ret0_1() = 0; // xor eax, eax; ret;
	virtual uint32 unknown_ret0_2() = 0; // xor eax, eax; ret;

	virtual void unknown_nop_3() = 0; // ret;
/////////////////////////////////////


	// delete the event from the timeline. This can be called on a timeline event from AddInstantaneousTimelineEvent,
	// AddRangeTimelineEvent, or StartRangeTimelineEvent/EndRangeTimelineEvent. The timeline event handle must be from the
	// current game process.
	virtual void RemoveTimelineEvent( TimelineEventHandle_t ulEvent ) = 0;


///////////////////////////////////// the following functions are not found in public SDK
	virtual void unknown_nop_4() = 0; // ret;
	virtual void unknown_nop_5() = 0; // ret;
	virtual void unknown_nop_6() = 0; // ret;
/////////////////////////////////////


	// add a tag to whatever time range is represented by the event
	STEAM_CALL_RESULT( SteamTimelineEventRecordingExists_t )
	virtual SteamAPICall_t DoesEventRecordingExist( TimelineEventHandle_t ulEvent ) = 0;

	/*******************    Game Phases    *******************/

	// Game phases allow the user to navigate their background recordings and clips. Exactly what a game phase means will vary game to game, but
	// the game phase should be a section of gameplay that is usually between 10 minutes and a few hours in length, and should be the
	// main way a user would think to divide up the game. These are presented to the user in a UI that shows the date the game was played,
	// with one row per game slice. Game phases should be used to mark sections of gameplay that the user might be interested in watching.
	//
	//	Examples could include:
	//		* A single match in a multiplayer PvP game
	//		* A chapter of a story-based singleplayer game
	//		* A single run in a roguelike
	//
	// Game phases are started with StartGamePhase, and while a phase is still happening, they can have tags and attributes added to them.
	//
	// Phase attributes represent generic text fields that can be updated throughout the duration of the phase. They are meant
	// to be used for phase metadata that is not part of a well defined set of options. For example, a KDA attribute that starts
	// with the value "0/0/0" and updates as the phase progresses, or something like a played-entered character name. Attributes
	// can be set as many times as the game likes with SetGamePhaseAttribute, and only the last value will be shown to the user.
	//
	// Phase tags represent data with a well defined set of options, which could be data such as match resolution, hero played,
	// game mode, etc. Tags can have an icon in addition to a text name. Multiple tags within the same group may be added per phase
	// and all will be remembered. For example, AddGamePhaseTag may be called multiple times for a "Bosses Defeated" group, with
	// different names and icons for each boss defeated during the phase, all of which will be shown to the user.
	//
	// The phase will continue until the game exits, until the game calls EndGamePhase, or until the game calls
	// StartGamePhase to start a new phase.
	//
	// The game phase functions take these parameters:
	// - pchTagIcon: The name of a game provided timeline icon or builtin "steam_" icon.
	// - pchPhaseID: A game-provided persistent ID for a game phase. This could be a the match ID in a multiplayer game, a chapter name in a
	//   single player game, the ID of a character, etc.
	// - pchTagName: The localized name of the tag in the language returned by SteamUtils()->GetSteamUILanguage().
	// - pchTagGroup: The localized name of the tag group.
	// - pchAttributeValue: The localized name of the attribute.
	// - pchAttributeGroup: The localized name of the attribute group.
	// - unPriority: Used to order tags and attributes in the UI displayed to the user, with higher priority values leading
	//   to more prominent positioning. In contexts where there is limited space, lower priority items may be hidden.
	virtual void StartGamePhase() = 0;
	virtual void EndGamePhase() = 0;

	// Games can set a phase ID so they can refer back to a phase in OpenOverlayToPhase
	virtual void SetGamePhaseID( const char *pchPhaseID ) = 0;
	STEAM_CALL_RESULT( SteamTimelineGamePhaseRecordingExists_t )
	virtual SteamAPICall_t DoesGamePhaseRecordingExist( const char *pchPhaseID ) = 0;

	// Add a tag that applies to the entire phase
	virtual void AddGamePhaseTag( const char *pchTagName, const char *pchTagIcon, const char *pchTagGroup, uint32 unPriority ) = 0;

	// Add a text attribute that applies to the entire phase
	virtual void SetGamePhaseAttribute( const char *pchAttributeGroup, const char *pchAttributeValue, uint32 unPriority ) = 0;

	/*******************    Opening the overlay    *******************/

	// Opens the Steam overlay to a game phase.
	//
	// Parameters:
	// - pchPhaseID: The ID of a phase that was previously provided by the game in SetGamePhaseID.
	virtual void OpenOverlayToGamePhase( const char *pchPhaseID ) = 0;

	// Opens the Steam overlay to a timeline event.
	//
	// Parameters:
	// - ulEventID: The ID of a timeline event returned by StartEvent or AddSimpleTimelineEvent
	virtual void OpenOverlayToTimelineEvent( const TimelineEventHandle_t ulEvent ) = 0;

};

#endif // ISTEAMTIMELINE003_H
