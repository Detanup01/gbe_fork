#ifndef ISTEAMVIDEO002_H
#define ISTEAMVIDEO002_H
#ifdef STEAM_WIN32
#pragma once
#endif

class ISteamVideo002
{
public:
	// Get a URL suitable for streaming the given Video app ID's video
	virtual void GetVideoURL( AppId_t unVideoAppID ) = 0;

	// returns true if user is uploading a live broadcast
	virtual bool IsBroadcasting( int *pnNumViewers ) = 0;

	// Get the OPF Details for 360 Video Playback
	STEAM_CALL_BACK( GetOPFSettingsResult_t )
	virtual void GetOPFSettings( AppId_t unVideoAppID ) = 0;
	virtual bool GetOPFStringForApp( AppId_t unVideoAppID, char *pchBuffer, int32 *pnBufferSize ) = 0;
};

#endif // ISTEAMVIDEO002_H