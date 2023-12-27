
#ifndef ISTEAMAPPS002_H
#define ISTEAMAPPS002_H
#ifdef STEAM_WIN32
#pragma once
#endif

class ISteamApps002
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
};

#endif //ISTEAMAPPS002_H
