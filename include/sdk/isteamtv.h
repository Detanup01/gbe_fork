
#ifndef ISTEAMTV_H
#define ISTEAMTV_H
#ifdef STEAM_WIN32
#pragma once
#endif

#include "steam_api_common.h"
#include "steamhttpenums.h"

struct SteamTVRegion_t {

};

enum ESteamTVRegionBehavior
{

};

class ISteamTV
{
public:
    virtual bool IsBroadcasting(int *pnNumViewers) = 0;
    virtual void AddBroadcastGameData(const char * pchKey, const char * pchValue) = 0;
    virtual void RemoveBroadcastGameData(const char * pchKey) = 0;
    virtual void AddTimelineMarker(const char * pchTemplateName, bool bPersistent, uint8 nColorR, uint8 nColorG, uint8 nColorB) = 0;
    virtual void RemoveTimelineMarker() = 0;
    virtual uint32 AddRegion(const char * pchElementName, const char * pchTimelineDataSection, const SteamTVRegion_t * pSteamTVRegion, ESteamTVRegionBehavior eSteamTVRegionBehavior) = 0;
    virtual void RemoveRegion(uint32 unRegionHandle) = 0;
};

#define STEAMTV_INTERFACE_VERSION "STEAMTV_INTERFACE_V001"

#endif // ISTEAMTV_H