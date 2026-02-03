
#ifndef ISTEAMUSERSTATS003_H
#define ISTEAMUSERSTATS003_H
#ifdef STEAM_WIN32
#pragma once
#endif


class ISteamUserStats003
{
public:

	// Ask the server to send down this user's data and achievements for nGameID
	virtual bool RequestCurrentStats( ) = 0;

	// Data accessors
	virtual bool GetStat( const char *pchName, int32 *pData ) = 0;
	virtual bool GetStat( const char *pchName, float *pData ) = 0;

	// Set / update data
	virtual bool SetStat( const char *pchName, int32 nData ) = 0;
	virtual bool SetStat( const char *pchName, float fData ) = 0;
	virtual bool UpdateAvgRateStat( const char *pchName, float flCountThisSession, double dSessionLength ) = 0;

	// Achievement flag accessors
	virtual bool GetAchievement( const char *pchName, bool *pbAchieved ) = 0;
	virtual bool SetAchievement( const char *pchName ) = 0;
	virtual bool ClearAchievement( const char *pchName ) = 0;

	// Store the current data on the server, will get a callback when set
	// And one callback for every new achievement
	virtual bool StoreStats( ) = 0;

	// Achievement / GroupAchievement metadata

	// Gets the icon of the achievement, which is a handle to be used in IClientUtils::GetImageRGBA(), or 0 if none set
	virtual int GetAchievementIcon( const char *pchName ) = 0;
	// Get general attributes (display name / text, etc) for an Achievement
	virtual const char *GetAchievementDisplayAttribute( const char *pchName, const char *pchKey ) = 0;

	// Achievement progress - triggers an AchievementProgress callback, that is all.
	// Calling this w/ N out of N progress will NOT set the achievement, the game must still do that.
virtual bool IndicateAchievementProgress( const char *pchName, uint32 nCurProgress, uint32 nMaxProgress ) = 0;
};

#endif // ISTEAMUSERSTATS003_H
