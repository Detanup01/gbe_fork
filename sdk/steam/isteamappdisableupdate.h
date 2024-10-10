
#ifndef ISTEAMAPPDISABLEUPDATE_H
#define ISTEAMAPPDISABLEUPDATE_H

// this interface is not found in public SDK archives, it is based on reversing the returned vftable from steamclient64.dll
// requested by appid 730

class ISteamAppDisableUpdate
{
public:

	// probably means how many seconds to keep the updates disabled
	virtual void SetAppUpdateDisabledSecondsRemaining(int32 nSeconds) = 0;

};

#define STEAMAPPDISABLEUPDATE_INTERFACE_VERSION "SteamAppDisableUpdate001"

#endif // ISTEAMAPPDISABLEUPDATE_H
