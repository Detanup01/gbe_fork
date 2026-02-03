
#ifndef ISTEAMAPPS004_H
#define ISTEAMAPPS004_H
#ifdef STEAM_WIN32
#pragma once
#endif

class ISteamApps004
{
public:
	virtual bool BIsSubscribed() = 0;
	virtual bool BIsLowViolence() = 0;
	virtual bool BIsCybercafe() = 0;
	virtual bool BIsVACBanned() = 0;
	virtual const char *GetCurrentGameLanguage() = 0;
	virtual const char *GetAvailableGameLanguages() = 0;

	// only use this member if you need to check ownership of another game related to yours, a demo for example
	virtual bool BIsSubscribedApp( AppId_t appID ) = 0;

	// Takes AppID of DLC and checks if the user owns the DLC & if the DLC is installed
	virtual bool BIsDlcInstalled( AppId_t appID ) = 0;

	// returns the Unix time of the purchase of the app
	virtual uint32 GetEarliestPurchaseUnixTime( AppId_t nAppID ) = 0;

	// Checks if the user is subscribed to the current app through a free weekend
	// This function will return false for users who have a retail or other type of license
	// Before using, please ask your Valve technical contact how to package and secure your free weekened
	virtual bool BIsSubscribedFromFreeWeekend() = 0;

	// Returns the number of DLC pieces for the running app
	virtual int GetDLCCount() = 0;

	// Returns metadata for DLC by index, of range [0, GetDLCCount()]
	virtual bool BGetDLCDataByIndex( int iDLC, AppId_t *pAppID, bool *pbAvailable, char *pchName, int cchNameBufferSize ) = 0;

	// Install/Uninstall control for optional DLC
	virtual void InstallDLC( AppId_t nAppID ) = 0;
	virtual void UninstallDLC( AppId_t nAppID ) = 0;

#ifdef _PS3
	// Result returned in a RegisterActivationCodeResponse_t callresult
	virtual SteamAPICall_t RegisterActivationCode( const char *pchActivationCode ) = 0;
#endif
};

#endif //ISTEAMAPPS004_H
