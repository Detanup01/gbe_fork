
#ifndef ISTEAMCONTROLLER002_H
#define ISTEAMCONTROLLER002_H
#ifdef STEAM_WIN32
#pragma once
#endif

class ISteamController002
{
public:

	//
	// Native controller support API
	//

	// Must call init and shutdown when starting/ending use of the interface
	virtual bool Init() = 0;
	virtual bool Shutdown() = 0;

	// Pump callback/callresult events, SteamAPI_RunCallbacks will do this for you, 
	// normally never need to call directly.
	virtual void RunFrame() = 0;

	virtual int GetConnectedControllers( uint64 * ) = 0;
 
	// Trigger a haptic pulse on the controller
	virtual void TriggerHapticPulse( uint32 unControllerIndex, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec ) = 0;

	virtual unknown_ret ActivateMode( uint64, int32 ) = 0;
	virtual int32 GetJoystickForHandle( uint64 ) = 0;
	virtual uint64 GetHandleForJoystick( int32 ) = 0;
	virtual unknown_ret GetModeAnalogOutputData( uint64, int32 ) = 0;

};

#endif //ISTEAMCONTROLLER002_H
