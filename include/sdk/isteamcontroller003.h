
#ifndef ISTEAMCONTROLLER003_H
#define ISTEAMCONTROLLER003_H
#ifdef STEAM_WIN32
#pragma once
#endif

class ISteamController003
{
public:

	
	// Init and Shutdown must be called when starting/ending use of this interface
	virtual bool Init() = 0;
	virtual bool Shutdown() = 0;
	
	// Synchronize API state with the latest Steam Controller inputs available. This
	// is performed automatically by SteamAPI_RunCallbacks, but for the absolute lowest
	// possible latency, you call this directly before reading controller state.
	virtual void RunFrame() = 0;

	// Enumerate currently connected controllers
	// handlesOut should point to a STEAM_CONTROLLER_MAX_COUNT sized array of ControllerHandle_t handles
	// Returns the number of handles written to handlesOut
	virtual int GetConnectedControllers( ControllerHandle_t *handlesOut ) = 0;
	
	// Invokes the Steam overlay and brings up the binding screen
	// Returns false is overlay is disabled / unavailable, or the user is not in Big Picture mode
	virtual bool ShowBindingPanel( ControllerHandle_t controllerHandle ) = 0;
	
	// ACTION SETS
	// Lookup the handle for an Action Set. Best to do this once on startup, and store the handles for all future API calls.
	virtual ControllerActionSetHandle_t GetActionSetHandle( const char *pszActionSetName ) = 0;
	
	// Reconfigure the controller to use the specified action set (ie 'Menu', 'Walk' or 'Drive')
	// This is cheap, and can be safely called repeatedly. It's often easier to repeatedly call it in
	// your state loops, instead of trying to place it in all of your state transitions.
	virtual void ActivateActionSet( ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetHandle ) = 0;
	virtual ControllerActionSetHandle_t GetCurrentActionSet( ControllerHandle_t controllerHandle ) = 0;
	
	// ACTIONS
	// Lookup the handle for a digital action. Best to do this once on startup, and store the handles for all future API calls.
	virtual ControllerDigitalActionHandle_t GetDigitalActionHandle( const char *pszActionName ) = 0;
	
	// Returns the current state of the supplied digital game action
	virtual ControllerDigitalActionData_t GetDigitalActionData( ControllerHandle_t controllerHandle, ControllerDigitalActionHandle_t digitalActionHandle ) = 0;
	
	// Get the origin(s) for a digital action within an action set. Returns the number of origins supplied in originsOut. Use this to display the appropriate on-screen prompt for the action.
	// originsOut should point to a STEAM_CONTROLLER_MAX_ORIGINS sized array of EControllerActionOrigin handles
	virtual int GetDigitalActionOrigins( ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetHandle, ControllerDigitalActionHandle_t digitalActionHandle, EControllerActionOrigin *originsOut ) = 0;
	
	// Lookup the handle for an analog action. Best to do this once on startup, and store the handles for all future API calls.
	virtual ControllerAnalogActionHandle_t GetAnalogActionHandle( const char *pszActionName ) = 0;
	
	// Returns the current state of these supplied analog game action
	virtual ControllerAnalogActionData_t GetAnalogActionData( ControllerHandle_t controllerHandle, ControllerAnalogActionHandle_t analogActionHandle ) = 0;

	// Get the origin(s) for an analog action within an action set. Returns the number of origins supplied in originsOut. Use this to display the appropriate on-screen prompt for the action.
	// originsOut should point to a STEAM_CONTROLLER_MAX_ORIGINS sized array of EControllerActionOrigin handles
	virtual int GetAnalogActionOrigins( ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetHandle, ControllerAnalogActionHandle_t analogActionHandle, EControllerActionOrigin *originsOut ) = 0;
		
	virtual void StopAnalogActionMomentum( ControllerHandle_t controllerHandle, ControllerAnalogActionHandle_t eAction ) = 0;
	
	// Trigger a haptic pulse on a controller
	virtual void TriggerHapticPulse( ControllerHandle_t controllerHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec ) = 0;

	virtual void TriggerRepeatedHapticPulse( ControllerHandle_t controllerHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec, unsigned short usOffMicroSec, unsigned short unRepeat, unsigned int nFlags ) = 0;
};

#endif //ISTEAMCONTROLLER003_H
