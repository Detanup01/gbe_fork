#include "base.h"

class Steam_Apps :
public ISteamApps001,
public ISteamApps002,
public ISteamApps003,
public ISteamApps004,
public ISteamApps005,
public ISteamApps006,
public ISteamApps007,
public ISteamApps
{
    Settings *settings;
    class SteamCallResults *callback_results;

public:
    Steam_Apps(Settings *settings, class SteamCallResults *callback_results);

	// returns 0 if the key does not exist
	// this may be true on first call, since the app data may not be cached locally yet
	// If you expect it to exists wait for the AppDataChanged_t after the first failure and ask again
	int GetAppData( AppId_t nAppID, const char *pchKey, char *pchValue, int cchValueMax );

	bool BIsSubscribed();
	bool BIsLowViolence();
	bool BIsCybercafe();
	bool BIsVACBanned();
	const char *GetCurrentGameLanguage();
	const char *GetAvailableGameLanguages();

	// only use this member if you need to check ownership of another game related to yours, a demo for example
	bool BIsSubscribedApp( AppId_t appID );

	// Takes AppID of DLC and checks if the user owns the DLC & if the DLC is installed
	bool BIsDlcInstalled( AppId_t appID );

	// returns the Unix time of the purchase of the app
	uint32 GetEarliestPurchaseUnixTime( AppId_t nAppID );

	// Checks if the user is subscribed to the current app through a free weekend
	// This function will return false for users who have a retail or other type of license
	// Before using, please ask your Valve technical contact how to package and secure your free weekened
	bool BIsSubscribedFromFreeWeekend();

	// Returns the number of DLC pieces for the running app
	int GetDLCCount();

	// Returns metadata for DLC by index, of range [0, GetDLCCount()]
	bool BGetDLCDataByIndex( int iDLC, AppId_t *pAppID, bool *pbAvailable, char *pchName, int cchNameBufferSize );

	// Install/Uninstall control for optional DLC
	void InstallDLC( AppId_t nAppID );
	void UninstallDLC( AppId_t nAppID );
	
	// Request legacy cd-key for yourself or owned DLC. If you are interested in this
	// data then make sure you provide us with a list of valid keys to be distributed
	// to users when they purchase the game, before the game ships.
	// You'll receive an AppProofOfPurchaseKeyResponse_t callback when
	// the key is available (which may be immediately).
	void RequestAppProofOfPurchaseKey( AppId_t nAppID );

	bool GetCurrentBetaName( char *pchName, int cchNameBufferSize ); // returns current beta branch name, 'public' is the default branch
	bool MarkContentCorrupt( bool bMissingFilesOnly ); // signal Steam that game files seems corrupt or missing
	uint32 GetInstalledDepots( AppId_t appID, DepotId_t *pvecDepots, uint32 cMaxDepots ); // return installed depots in mount order
	uint32 GetInstalledDepots( DepotId_t *pvecDepots, uint32 cMaxDepots );

	// returns current app install folder for AppID, returns folder name length
	uint32 GetAppInstallDir( AppId_t appID, char *pchFolder, uint32 cchFolderBufferSize );
	bool BIsAppInstalled( AppId_t appID ); // returns true if that app is installed (not necessarily owned)
	
	CSteamID GetAppOwner(); // returns the SteamID of the original owner. If different from current user, it's borrowed

	// Returns the associated launch param if the game is run via steam://run/<appid>//?param1=value1;param2=value2;param3=value3 etc.
	// Parameter names starting with the character '@' are reserved for internal use and will always return and empty string.
	// Parameter names starting with an underscore '_' are reserved for steam features -- they can be queried by the game,
	// but it is advised that you not param names beginning with an underscore for your own features.
	const char *GetLaunchQueryParam( const char *pchKey );

	// get download progress for optional DLC
	bool GetDlcDownloadProgress( AppId_t nAppID, uint64 *punBytesDownloaded, uint64 *punBytesTotal ); 

	// return the buildid of this app, may change at any time based on backend updates to the game
	int GetAppBuildId();

	// Request all proof of purchase keys for the calling appid and asociated DLC.
	// A series of AppProofOfPurchaseKeyResponse_t callbacks will be sent with
	// appropriate appid values, ending with a final callback where the m_nAppId
	// member is k_uAppIdInvalid (zero).
	void RequestAllProofOfPurchaseKeys();

	STEAM_CALL_RESULT( FileDetailsResult_t )
	SteamAPICall_t GetFileDetails( const char* pszFileName );

	// Get command line if game was launched via Steam URL, e.g. steam://run/<appid>//<command line>/.
	// This method of passing a connect string (used when joining via rich presence, accepting an
	// invite, etc) is preferable to passing the connect string on the operating system command
	// line, which is a security risk.  In order for rich presence joins to go through this
	// path and not be placed on the OS command line, you must set a value in your app's
	// configuration on Steam.  Ask Valve for help with this.
	//
	// If game was already running and launched again, the NewUrlLaunchParameters_t will be fired.
	int GetLaunchCommandLine( char *pszCommandLine, int cubCommandLine );

	// Check if user borrowed this game via Family Sharing, If true, call GetAppOwner() to get the lender SteamID
	bool BIsSubscribedFromFamilySharing();

	// check if game is a timed trial with limited playtime
	bool BIsTimedTrial( uint32* punSecondsAllowed, uint32* punSecondsPlayed );

	// set current DLC AppID being played (or 0 if none). Allows Steam to track usage of major DLC extensions
    bool SetDlcContext( AppId_t nAppID );
};
