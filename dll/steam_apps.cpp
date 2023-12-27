/* Copyright (C) 2019 Mr Goldberg
   This file is part of the Goldberg Emulator

   The Goldberg Emulator is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 3 of the License, or (at your option) any later version.

   The Goldberg Emulator is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the Goldberg Emulator; if not, see
   <http://www.gnu.org/licenses/>.  */

#include "dll/steam_apps.h"
#include "sha/sha1.hpp"

Steam_Apps::Steam_Apps(Settings *settings, class SteamCallResults *callback_results)
{
    this->settings = settings;
    this->callback_results = callback_results;
}

// returns 0 if the key does not exist
// this may be true on first call, since the app data may not be cached locally yet
// If you expect it to exists wait for the AppDataChanged_t after the first failure and ask again
int Steam_Apps::GetAppData( AppId_t nAppID, const char *pchKey, char *pchValue, int cchValueMax )
{
    //TODO
    PRINT_DEBUG("GetAppData %u %s\n", nAppID, pchKey);
    return 0;
}

bool Steam_Apps::BIsSubscribed()
{
    PRINT_DEBUG("BIsSubscribed\n");
    return true;
}

bool Steam_Apps::BIsLowViolence()
{
    PRINT_DEBUG("BIsLowViolence\n");
    return false;
}

bool Steam_Apps::BIsCybercafe()
{
    PRINT_DEBUG("BIsCybercafe\n");
    return false;
}

bool Steam_Apps::BIsVACBanned()
{
    PRINT_DEBUG("BIsVACBanned\n");
    return false;
}

const char *Steam_Apps::GetCurrentGameLanguage()
{
    PRINT_DEBUG("GetCurrentGameLanguage\n");
    return settings->get_language();
}

const char *Steam_Apps::GetAvailableGameLanguages()
{
    PRINT_DEBUG("GetAvailableGameLanguages\n");
    //TODO?
    return settings->get_language();
}


// only use this member if you need to check ownership of another game related to yours, a demo for example
bool Steam_Apps::BIsSubscribedApp( AppId_t appID )
{
    PRINT_DEBUG("BIsSubscribedApp %u\n", appID);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (appID == 0) return true; //I think appid 0 is always owned
    if (appID == UINT32_MAX) return false; // check Steam_Apps::BIsAppInstalled()
    if (appID == settings->get_local_game_id().AppID()) return true;
    return settings->hasDLC(appID);
}


// Takes AppID of DLC and checks if the user owns the DLC & if the DLC is installed
bool Steam_Apps::BIsDlcInstalled( AppId_t appID )
{
    PRINT_DEBUG("BIsDlcInstalled %u\n", appID);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (appID == 0) return true;
    if (appID == UINT32_MAX) return false; // check Steam_Apps::BIsAppInstalled()
    if (appID == settings->get_local_game_id().AppID()) return false; //TODO is this correct?
    return settings->hasDLC(appID);
}


// returns the Unix time of the purchase of the app
uint32 Steam_Apps::GetEarliestPurchaseUnixTime( AppId_t nAppID )
{
    PRINT_DEBUG("GetEarliestPurchaseUnixTime\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (nAppID == 0) return 0; //TODO is this correct?
    if (nAppID == UINT32_MAX) return 0; // check Steam_Apps::BIsAppInstalled() TODO is this correct?
    if (nAppID == settings->get_local_game_id().AppID() || settings->hasDLC(nAppID)) {
        auto t =
            std::chrono::system_clock::now()
            - std::chrono::hours(24 * 4); // 4 days ago
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(t.time_since_epoch());
        return (uint32)duration.count();
    }

    //TODO ?
    return 0;
}


// Checks if the user is subscribed to the current app through a free weekend
// This function will return false for users who have a retail or other type of license
// Before using, please ask your Valve technical contact how to package and secure your free weekened
bool Steam_Apps::BIsSubscribedFromFreeWeekend()
{
    PRINT_DEBUG("BIsSubscribedFromFreeWeekend\n");
    return false;
}


// Returns the number of DLC pieces for the running app
int Steam_Apps::GetDLCCount()
{
    PRINT_DEBUG("GetDLCCount\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return settings->DLCCount();
}


// Returns metadata for DLC by index, of range [0, GetDLCCount()]
bool Steam_Apps::BGetDLCDataByIndex( int iDLC, AppId_t *pAppID, bool *pbAvailable, char *pchName, int cchNameBufferSize )
{
    PRINT_DEBUG("BGetDLCDataByIndex\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    AppId_t appid;
    bool available;
    std::string name;
    if (!settings->getDLC(iDLC, appid, available, name)) return false;

    if (pAppID) *pAppID = appid;
    if (pbAvailable) *pbAvailable = available;
    if (pchName && cchNameBufferSize > 0) {
        if (cchNameBufferSize > name.size()) {
            cchNameBufferSize = name.size();
            pchName[cchNameBufferSize] = 0;
        }

        //TODO: should pchName be null terminated if the size is smaller or equal to the name size?
        memcpy(pchName, name.data(), cchNameBufferSize);
    }

    return true;
}


// Install/Uninstall control for optional DLC
void Steam_Apps::InstallDLC( AppId_t nAppID )
{
    PRINT_DEBUG("InstallDLC\n");
    // we lock here because the API is supposed to modify the DLC list
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}

void Steam_Apps::UninstallDLC( AppId_t nAppID )
{
    PRINT_DEBUG("UninstallDLC\n");
    // we lock here because the API is supposed to modify the DLC list
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
}


static void FillProofOfPurchaseKey( AppProofOfPurchaseKeyResponse_t& data, AppId_t nAppID, bool ok_result, std::string key = "cdcdcdcdcdcdcdcdcdcdcdcdcdcdcdcd" )
{
    data.m_nAppID = nAppID;
    if (ok_result) {
        // TODO maybe read this from a config file "purchased_keys.txt":
        // 480=AAAAA-BBBBB-CCCCC-DDDDD
        // 218620=XYZFJ-13370-98765
        size_t min_len = key.size() < k_cubAppProofOfPurchaseKeyMax
            ? key.size() < k_cubAppProofOfPurchaseKeyMax
            : k_cubAppProofOfPurchaseKeyMax - 1; // -1 because we need space for null
        data.m_eResult = EResult::k_EResultOK;
        data.m_cchKeyLength = min_len;
        memcpy(data.m_rgchKey, key.c_str(), min_len);
        data.m_rgchKey[min_len] = 0;
    } else {
        data.m_eResult = EResult::k_EResultFail;
        data.m_cchKeyLength = 0;
        data.m_rgchKey[0] = 0;
        data.m_rgchKey[1] = 0;
    }
}

// Request legacy cd-key for yourself or owned DLC. If you are interested in this
// data then make sure you provide us with a list of valid keys to be distributed
// to users when they purchase the game, before the game ships.
// You'll receive an AppProofOfPurchaseKeyResponse_t callback when
// the key is available (which may be immediately).
void Steam_Apps::RequestAppProofOfPurchaseKey( AppId_t nAppID )
{
    PRINT_DEBUG("TODO RequestAppProofOfPurchaseKey\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    AppProofOfPurchaseKeyResponse_t data{};
    data.m_nAppID = nAppID;
    
    // check Steam_Apps::BIsAppInstalled()
    if (nAppID == 0 || nAppID == UINT32_MAX) {
        FillProofOfPurchaseKey(data, nAppID, false);
    } else if (nAppID == settings->get_local_game_id().AppID() || settings->hasDLC(nAppID)) {
        FillProofOfPurchaseKey(data, nAppID, true);
    } else {
        //TODO what to do here?
        FillProofOfPurchaseKey(data, nAppID, false);
    }

    callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}

// returns current beta branch name, 'public' is the default branch
// "true if the user is on a beta branch; otherwise, false"
// https://partner.steamgames.com/doc/api/ISteamApps
bool Steam_Apps::GetCurrentBetaName( char *pchName, int cchNameBufferSize )
{
    PRINT_DEBUG("GetCurrentBetaName %i\n", cchNameBufferSize);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (pchName && cchNameBufferSize > settings->current_branch_name.size()) {
        memcpy(pchName, settings->current_branch_name.c_str(), settings->current_branch_name.size());
    }

    return settings->is_beta_branch;
}

// signal Steam that game files seems corrupt or missing
bool Steam_Apps::MarkContentCorrupt( bool bMissingFilesOnly )
{
    PRINT_DEBUG("MarkContentCorrupt\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    //TODO: warn user
    return true;
}

// return installed depots in mount order
uint32 Steam_Apps::GetInstalledDepots( AppId_t appID, DepotId_t *pvecDepots, uint32 cMaxDepots )
{
    PRINT_DEBUG("GetInstalledDepots %u, %u\n", appID, cMaxDepots);
    //TODO not sure about the behavior of this function, I didn't actually test this.
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!pvecDepots) return 0;
    unsigned int count = settings->depots.size();
    if (cMaxDepots < count) count = cMaxDepots;
    std::copy(settings->depots.begin(), settings->depots.begin() + count, pvecDepots);
    return count;
}

uint32 Steam_Apps::GetInstalledDepots( DepotId_t *pvecDepots, uint32 cMaxDepots )
{
    PRINT_DEBUG("GetInstalledDepots old\n");
    return GetInstalledDepots( settings->get_local_game_id().AppID(), pvecDepots, cMaxDepots );
}

// returns current app install folder for AppID, returns folder name length
uint32 Steam_Apps::GetAppInstallDir( AppId_t appID, char *pchFolder, uint32 cchFolderBufferSize )
{
    PRINT_DEBUG("GetAppInstallDir %u %p %u\n", appID, pchFolder, cchFolderBufferSize);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    //TODO return real path instead of dll path
    std::string installed_path = settings->getAppInstallPath(appID);

    if (installed_path.size() == 0) {
        std::string dll_path = get_full_program_path();
        std::string current_path = get_current_path();
        PRINT_DEBUG("paths %s %s\n", dll_path.c_str(), current_path.c_str());

        //Just pick the smallest path, it has the most chances of being the good one
        if (dll_path.size() > current_path.size() && current_path.size()) {
            installed_path = current_path;
        } else {
            installed_path = dll_path;
        }
    }

    PRINT_DEBUG("path %s\n", installed_path.c_str());
    if (cchFolderBufferSize && pchFolder) {
        snprintf(pchFolder, cchFolderBufferSize, "%s", installed_path.c_str());
    }

    return installed_path.length(); //Real steam always returns the actual path length, not the copied one.
}

// returns true if that app is installed (not necessarily owned)
// "This only works for base applications, not Downloadable Content (DLC). Use BIsDlcInstalled for DLC instead"
// https://partner.steamgames.com/doc/api/ISteamApps
bool Steam_Apps::BIsAppInstalled( AppId_t appID )
{
    PRINT_DEBUG("BIsAppInstalled %u\n", appID);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    // "0 Base Goldsource Shared Binaries"
    // https://developer.valvesoftware.com/wiki/Steam_Application_IDs
    if (appID == 0) return true;
    // game LEGO® 2K Drive (app id 1451810) checks for a proper steam behavior by sending uint32_max and expects false in return
    if (appID == UINT32_MAX) return false;
    if (appID == settings->get_local_game_id().AppID()) return true;

    // only check for DLC if the the list is limited/bounded,
    // calling hasDLC() wehn unlockAllDLCs is true will always satisfy the below condition
    if (!settings->allDLCUnlocked() && settings->hasDLC(appID)) {
        return false;
    }

    return settings->appIsInstalled(appID);
}

// returns the SteamID of the original owner. If different from current user, it's borrowed
CSteamID Steam_Apps::GetAppOwner()
{
    PRINT_DEBUG("GetAppOwner\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return settings->get_local_steam_id();
}

// Returns the associated launch param if the game is run via steam://run/<appid>//?param1=value1;param2=value2;param3=value3 etc.
// Parameter names starting with the character '@' are reserved for internal use and will always return and empty string.
// Parameter names starting with an underscore '_' are reserved for steam features -- they can be queried by the game,
// but it is advised that you not param names beginning with an underscore for your own features.
const char *Steam_Apps::GetLaunchQueryParam( const char *pchKey )
{
    PRINT_DEBUG("GetLaunchQueryParam\n");
    return "";
}


// get download progress for optional DLC
bool Steam_Apps::GetDlcDownloadProgress( AppId_t nAppID, uint64 *punBytesDownloaded, uint64 *punBytesTotal )
{
    PRINT_DEBUG("GetDlcDownloadProgress\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return false;
}
 

// return the buildid of this app, may change at any time based on backend updates to the game
int Steam_Apps::GetAppBuildId()
{
    PRINT_DEBUG("GetAppBuildId\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return this->settings->build_id;
}


// Request all proof of purchase keys for the calling appid and asociated DLC.
// A series of AppProofOfPurchaseKeyResponse_t callbacks will be sent with
// appropriate appid values, ending with a final callback where the m_nAppId
// member is k_uAppIdInvalid (zero).
void Steam_Apps::RequestAllProofOfPurchaseKeys()
{
    PRINT_DEBUG("TODO RequestAllProofOfPurchaseKeys\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    // current app
    {
        AppProofOfPurchaseKeyResponse_t data{};
        FillProofOfPurchaseKey(data, settings->get_local_game_id().AppID(), true);
        callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    }

    // DLCs
    const auto count = settings->DLCCount();
    for (size_t i = 0; i < settings->DLCCount(); i++) {
        AppId_t app_id;
        bool available;
        std::string name;
        settings->getDLC(i, app_id, available, name);

        AppProofOfPurchaseKeyResponse_t data{};
        FillProofOfPurchaseKey(data, app_id, true);
        callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    }
    
    // termination entry
    {
        AppProofOfPurchaseKeyResponse_t data{};
        FillProofOfPurchaseKey(data, k_uAppIdInvalid, true, "");
        callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    }

}


STEAM_CALL_RESULT( FileDetailsResult_t )
SteamAPICall_t Steam_Apps::GetFileDetails( const char* pszFileName )
{
    PRINT_DEBUG("GetFileDetails %s\n", pszFileName);
    FileDetailsResult_t data = {};
    //TODO? this function should only return found if file is actually part of the steam depots
    if (file_exists_(pszFileName)) {
        data.m_eResult = k_EResultOK; //
        std::ifstream stream(utf8_decode(pszFileName), std::ios::binary);
        SHA1 checksum;
        checksum.update(stream);
        checksum.final().copy((char *)data.m_FileSHA, sizeof(data.m_FileSHA));
        data.m_ulFileSize = file_size_(pszFileName);
        //TODO data.m_unFlags; 0 is file //TODO: check if 64 is folder
    } else {
        data.m_eResult = k_EResultFileNotFound;
    }

    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}

// Get command line if game was launched via Steam URL, e.g. steam://run/<appid>//<command line>/.
// This method of passing a connect string (used when joining via rich presence, accepting an
// invite, etc) is preferable to passing the connect string on the operating system command
// line, which is a security risk.  In order for rich presence joins to go through this
// path and not be placed on the OS command line, you must set a value in your app's
// configuration on Steam.  Ask Valve for help with this.
//
// If game was already running and launched again, the NewUrlLaunchParameters_t will be fired.
int Steam_Apps::GetLaunchCommandLine( char *pszCommandLine, int cubCommandLine )
{
    PRINT_DEBUG("TODO GetLaunchCommandLine\n");
    return 0;
}

// Check if user borrowed this game via Family Sharing, If true, call GetAppOwner() to get the lender SteamID
bool Steam_Apps::BIsSubscribedFromFamilySharing()
{
    PRINT_DEBUG("BIsSubscribedFromFamilySharing\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return false;
}

// check if game is a timed trial with limited playtime
bool Steam_Apps::BIsTimedTrial( uint32* punSecondsAllowed, uint32* punSecondsPlayed )
{
    PRINT_DEBUG("BIsTimedTrial\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return false;
}

// set current DLC AppID being played (or 0 if none). Allows Steam to track usage of major DLC extensions
bool Steam_Apps::SetDlcContext( AppId_t nAppID )
{
    PRINT_DEBUG("SetDlcContext %u\n", nAppID);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return true;
}
