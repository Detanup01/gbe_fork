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

#ifndef __INCLUDED_STEAM_UGC_H__
#define __INCLUDED_STEAM_UGC_H__

#include "base.h"
#include "ugc_remote_storage_bridge.h"

enum EQueryType {
	eUserUGCRequest = 0,
	eAllUGCRequestPage,
	eAllUGCRequestCursor,
	eUGCDetailsRequest
};

struct UGC_query {
    UGCQueryHandle_t handle{};
    std::set<PublishedFileId_t> return_only{};
    bool return_all_subscribed{};
    uint32 page{};
    bool next_cursor = true;
    EQueryType query_type{};

    std::set<PublishedFileId_t> results{};
    
    bool admin_query = false; // added in sdk 1.60 (currently unused)
    std::string min_branch{}; // added in sdk 1.60 (currently unused)
    std::string max_branch{}; // added in sdk 1.60 (currently unused)
};

class Steam_UGC :
public ISteamUGC001,
public ISteamUGC002,
public ISteamUGC003,
public ISteamUGC004,
public ISteamUGC005,
public ISteamUGC006,
public ISteamUGC007,
public ISteamUGC008,
public ISteamUGC009,
public ISteamUGC010,
public ISteamUGC011,
public ISteamUGC012,
public ISteamUGC013,
public ISteamUGC014,
public ISteamUGC015,
public ISteamUGC016,
public ISteamUGC017,
public ISteamUGC018,
public ISteamUGC019,
public ISteamUGC
{
public:
    enum class IUgcItfVersion : unsigned {
        v018, // sdk 1.59
        v020, // sdk 1.60
    };

private:
    constexpr static const char ugc_favorits_file[] = "favorites.txt";

    class Settings *settings{};
    class Ugc_Remote_Storage_Bridge *ugc_bridge{};
    class Local_Storage *local_storage{};
    class SteamCallResults *callback_results{};
    class SteamCallBacks *callbacks{};

    UGCQueryHandle_t handle = 50; // just makes debugging easier, any initial val is fine, even 1
    std::vector<struct UGC_query> ugc_queries{};
    std::set<PublishedFileId_t> favorites{};

    UGCQueryHandle_t new_ugc_query(
        EQueryType query_type,
        bool return_all_subscribed = false,
        uint32 page = 0,
        bool next_cursor = true,
        const std::set<PublishedFileId_t> &return_only = std::set<PublishedFileId_t>());

    std::optional<Mod_entry> get_query_ugc(UGCQueryHandle_t handle, uint32 index);

    std::optional<std::string> get_query_ugc_tag(UGCQueryHandle_t handle, uint32 index, uint32 indexTag);

    std::vector<std::string> get_query_ugc_tags(UGCQueryHandle_t handle, uint32 index);

    void set_details(PublishedFileId_t id, SteamUGCDetails_t *pDetails, IUgcItfVersion ver);

    void read_ugc_favorites();

    bool write_ugc_favorites();

    bool internal_GetQueryUGCResult( UGCQueryHandle_t handle, uint32 index, SteamUGCDetails_t *pDetails, IUgcItfVersion ver );
    SteamAPICall_t internal_RequestUGCDetails( PublishedFileId_t nPublishedFileID, uint32 unMaxAgeSeconds, IUgcItfVersion ver );

public:
    Steam_UGC(class Settings *settings, class Ugc_Remote_Storage_Bridge *ugc_bridge, class Local_Storage *local_storage, class SteamCallResults *callback_results, class SteamCallBacks *callbacks);


    // Query UGC associated with a user. Creator app id or consumer app id must be valid and be set to the current running app. unPage should start at 1.
    UGCQueryHandle_t CreateQueryUserUGCRequest( AccountID_t unAccountID, EUserUGCList eListType, EUGCMatchingUGCType eMatchingUGCType, EUserUGCListSortOrder eSortOrder, AppId_t nCreatorAppID, AppId_t nConsumerAppID, uint32 unPage );


    // Query for all matching UGC. Creator app id or consumer app id must be valid and be set to the current running app. unPage should start at 1.
    UGCQueryHandle_t CreateQueryAllUGCRequest( EUGCQuery eQueryType, EUGCMatchingUGCType eMatchingeMatchingUGCTypeFileType, AppId_t nCreatorAppID, AppId_t nConsumerAppID, uint32 unPage );

    // Query for all matching UGC using the new deep paging interface. Creator app id or consumer app id must be valid and be set to the current running app. pchCursor should be set to NULL or "*" to get the first result set.
    UGCQueryHandle_t CreateQueryAllUGCRequest( EUGCQuery eQueryType, EUGCMatchingUGCType eMatchingeMatchingUGCTypeFileType, AppId_t nCreatorAppID, AppId_t nConsumerAppID, const char *pchCursor = NULL );

    // Query for the details of the given published file ids (the RequestUGCDetails call is deprecated and replaced with this)
    UGCQueryHandle_t CreateQueryUGCDetailsRequest( PublishedFileId_t *pvecPublishedFileID, uint32 unNumPublishedFileIDs );


    // Send the query to Steam
    STEAM_CALL_RESULT( SteamUGCQueryCompleted_t )
    SteamAPICall_t SendQueryUGCRequest( UGCQueryHandle_t handle );


    // Retrieve an individual result after receiving the callback for querying UGC
    bool GetQueryUGCResult( UGCQueryHandle_t handle, uint32 index, SteamUGCDetails_t *pDetails );
    // before SDK v1.60
    bool GetQueryUGCResult_old( UGCQueryHandle_t handle, uint32 index, SteamUGCDetails_t *pDetails );

    uint32 GetQueryUGCNumTags( UGCQueryHandle_t handle, uint32 index );

    bool GetQueryUGCTag( UGCQueryHandle_t handle, uint32 index, uint32 indexTag, STEAM_OUT_STRING_COUNT( cchValueSize ) char* pchValue, uint32 cchValueSize );

    bool GetQueryUGCTagDisplayName( UGCQueryHandle_t handle, uint32 index, uint32 indexTag, STEAM_OUT_STRING_COUNT( cchValueSize ) char* pchValue, uint32 cchValueSize );

    bool GetQueryUGCPreviewURL( UGCQueryHandle_t handle, uint32 index, STEAM_OUT_STRING_COUNT(cchURLSize) char *pchURL, uint32 cchURLSize );


    bool GetQueryUGCMetadata( UGCQueryHandle_t handle, uint32 index, STEAM_OUT_STRING_COUNT(cchMetadatasize) char *pchMetadata, uint32 cchMetadatasize );

    bool GetQueryUGCChildren( UGCQueryHandle_t handle, uint32 index, PublishedFileId_t* pvecPublishedFileID, uint32 cMaxEntries );


    bool GetQueryUGCStatistic( UGCQueryHandle_t handle, uint32 index, EItemStatistic eStatType, uint64 *pStatValue );

    bool GetQueryUGCStatistic( UGCQueryHandle_t handle, uint32 index, EItemStatistic eStatType, uint32 *pStatValue );

    uint32 GetQueryUGCNumAdditionalPreviews( UGCQueryHandle_t handle, uint32 index );


    bool GetQueryUGCAdditionalPreview( UGCQueryHandle_t handle, uint32 index, uint32 previewIndex, STEAM_OUT_STRING_COUNT(cchURLSize) char *pchURLOrVideoID, uint32 cchURLSize, STEAM_OUT_STRING_COUNT(cchURLSize) char *pchOriginalFileName, uint32 cchOriginalFileNameSize, EItemPreviewType *pPreviewType );

    bool GetQueryUGCAdditionalPreview( UGCQueryHandle_t handle, uint32 index, uint32 previewIndex, char *pchURLOrVideoID, uint32 cchURLSize, bool *hz );

    uint32 GetQueryUGCNumKeyValueTags( UGCQueryHandle_t handle, uint32 index );


    bool GetQueryUGCKeyValueTag( UGCQueryHandle_t handle, uint32 index, uint32 keyValueTagIndex, STEAM_OUT_STRING_COUNT(cchKeySize) char *pchKey, uint32 cchKeySize, STEAM_OUT_STRING_COUNT(cchValueSize) char *pchValue, uint32 cchValueSize );

    bool GetQueryUGCKeyValueTag( UGCQueryHandle_t handle, uint32 index, const char *pchKey, STEAM_OUT_STRING_COUNT(cchValueSize) char *pchValue, uint32 cchValueSize );

	// Some items can specify that they have a version that is valid for a range of game versions (Steam branch)
    uint32 GetNumSupportedGameVersions( UGCQueryHandle_t handle, uint32 index );

    bool GetSupportedGameVersionData( UGCQueryHandle_t handle, uint32 index, uint32 versionIndex, STEAM_OUT_STRING_COUNT( cchGameBranchSize ) char *pchGameBranchMin, STEAM_OUT_STRING_COUNT( cchGameBranchSize ) char *pchGameBranchMax, uint32 cchGameBranchSize );

    uint32 GetQueryUGCContentDescriptors( UGCQueryHandle_t handle, uint32 index, EUGCContentDescriptorID *pvecDescriptors, uint32 cMaxEntries );

    // Release the request to free up memory, after retrieving results
    bool ReleaseQueryUGCRequest( UGCQueryHandle_t handle );


    // Options to set for querying UGC
    bool AddRequiredTag( UGCQueryHandle_t handle, const char *pTagName );

    bool AddRequiredTagGroup( UGCQueryHandle_t handle, const SteamParamStringArray_t *pTagGroups );

    bool AddExcludedTag( UGCQueryHandle_t handle, const char *pTagName );

    bool SetReturnOnlyIDs( UGCQueryHandle_t handle, bool bReturnOnlyIDs );

    bool SetReturnKeyValueTags( UGCQueryHandle_t handle, bool bReturnKeyValueTags );

    bool SetReturnLongDescription( UGCQueryHandle_t handle, bool bReturnLongDescription );

    bool SetReturnMetadata( UGCQueryHandle_t handle, bool bReturnMetadata );

    bool SetReturnChildren( UGCQueryHandle_t handle, bool bReturnChildren );

    bool SetReturnAdditionalPreviews( UGCQueryHandle_t handle, bool bReturnAdditionalPreviews );

    bool SetReturnTotalOnly( UGCQueryHandle_t handle, bool bReturnTotalOnly );

    bool SetReturnPlaytimeStats( UGCQueryHandle_t handle, uint32 unDays );

    bool SetLanguage( UGCQueryHandle_t handle, const char *pchLanguage );

    bool SetAllowCachedResponse( UGCQueryHandle_t handle, uint32 unMaxAgeSeconds );

    // allow ISteamUGC to be used in a tools like environment for users who have the appropriate privileges for the calling appid
    bool SetAdminQuery( UGCUpdateHandle_t handle, bool bAdminQuery );

    // Options only for querying user UGC
    bool SetCloudFileNameFilter( UGCQueryHandle_t handle, const char *pMatchCloudFileName );

    // Options only for querying all UGC
    bool SetMatchAnyTag( UGCQueryHandle_t handle, bool bMatchAnyTag );

    bool SetSearchText( UGCQueryHandle_t handle, const char *pSearchText );

    bool SetRankedByTrendDays( UGCQueryHandle_t handle, uint32 unDays );

    bool AddRequiredKeyValueTag( UGCQueryHandle_t handle, const char *pKey, const char *pValue );

    bool SetTimeCreatedDateRange( UGCQueryHandle_t handle, RTime32 rtStart, RTime32 rtEnd );

    bool SetTimeUpdatedDateRange( UGCQueryHandle_t handle, RTime32 rtStart, RTime32 rtEnd );


    // DEPRECATED - Use CreateQueryUGCDetailsRequest call above instead!
    SteamAPICall_t RequestUGCDetails( PublishedFileId_t nPublishedFileID, uint32 unMaxAgeSeconds );
    // before SDK v1.60
    SteamAPICall_t RequestUGCDetails_old( PublishedFileId_t nPublishedFileID, uint32 unMaxAgeSeconds );

    SteamAPICall_t RequestUGCDetails( PublishedFileId_t nPublishedFileID );


    // Steam Workshop Creator API
    STEAM_CALL_RESULT( CreateItemResult_t )
    SteamAPICall_t CreateItem( AppId_t nConsumerAppId, EWorkshopFileType eFileType );
    // create new item for this app with no content attached yet

    UGCUpdateHandle_t StartItemUpdate( AppId_t nConsumerAppId, PublishedFileId_t nPublishedFileID );
    // start an UGC item update. Set changed properties before commiting update with CommitItemUpdate()

    bool SetItemTitle( UGCUpdateHandle_t handle, const char *pchTitle );
    // change the title of an UGC item

    bool SetItemDescription( UGCUpdateHandle_t handle, const char *pchDescription );
    // change the description of an UGC item

    bool SetItemUpdateLanguage( UGCUpdateHandle_t handle, const char *pchLanguage );
    // specify the language of the title or description that will be set

    bool SetItemMetadata( UGCUpdateHandle_t handle, const char *pchMetaData );
    // change the metadata of an UGC item (max = k_cchDeveloperMetadataMax)


    bool SetItemVisibility( UGCUpdateHandle_t handle, ERemoteStoragePublishedFileVisibility eVisibility );
    // change the visibility of an UGC item


    bool SetItemTags( UGCUpdateHandle_t updateHandle, const SteamParamStringArray_t *pTags );

    bool SetItemTags( UGCUpdateHandle_t updateHandle, const SteamParamStringArray_t *pTags, bool bAllowAdminTags );
    // change the tags of an UGC item

    bool SetItemContent( UGCUpdateHandle_t handle, const char *pszContentFolder );
    // update item content from this local folder


    bool SetItemPreview( UGCUpdateHandle_t handle, const char *pszPreviewFile );
    //  change preview image file for this item. pszPreviewFile points to local image file, which must be under 1MB in size

    bool SetAllowLegacyUpload( UGCUpdateHandle_t handle, bool bAllowLegacyUpload );

    bool RemoveAllItemKeyValueTags( UGCUpdateHandle_t handle );
    // remove all existing key-value tags (you can add new ones via the AddItemKeyValueTag function)

    bool RemoveItemKeyValueTags( UGCUpdateHandle_t handle, const char *pchKey );
    // remove any existing key-value tags with the specified key


    bool AddItemKeyValueTag( UGCUpdateHandle_t handle, const char *pchKey, const char *pchValue );
    // add new key-value tags for the item. Note that there can be multiple values for a tag.

    bool AddItemPreviewFile( UGCUpdateHandle_t handle, const char *pszPreviewFile, EItemPreviewType type );
    //  add preview file for this item. pszPreviewFile points to local file, which must be under 1MB in size

    bool AddItemPreviewVideo( UGCUpdateHandle_t handle, const char *pszVideoID );
    //  add preview video for this item

    bool UpdateItemPreviewFile( UGCUpdateHandle_t handle, uint32 index, const char *pszPreviewFile );
    //  updates an existing preview file for this item. pszPreviewFile points to local file, which must be under 1MB in size


    bool UpdateItemPreviewVideo( UGCUpdateHandle_t handle, uint32 index, const char *pszVideoID );
    //  updates an existing preview video for this item


    bool RemoveItemPreview( UGCUpdateHandle_t handle, uint32 index );
    // remove a preview by index starting at 0 (previews are sorted)

    bool AddContentDescriptor( UGCUpdateHandle_t handle, EUGCContentDescriptorID descid );

    bool RemoveContentDescriptor( UGCUpdateHandle_t handle, EUGCContentDescriptorID descid );

    bool SetRequiredGameVersions( UGCUpdateHandle_t handle, const char *pszGameBranchMin, const char *pszGameBranchMax );
    
    STEAM_CALL_RESULT( SubmitItemUpdateResult_t )
    SteamAPICall_t SubmitItemUpdate( UGCUpdateHandle_t handle, const char *pchChangeNote );
    // commit update process started with StartItemUpdate()


    EItemUpdateStatus GetItemUpdateProgress( UGCUpdateHandle_t handle, uint64 *punBytesProcessed, uint64* punBytesTotal );


    // Steam Workshop Consumer API

    STEAM_CALL_RESULT( SetUserItemVoteResult_t )
    SteamAPICall_t SetUserItemVote( PublishedFileId_t nPublishedFileID, bool bVoteUp );


    STEAM_CALL_RESULT( GetUserItemVoteResult_t )
    SteamAPICall_t GetUserItemVote( PublishedFileId_t nPublishedFileID );


    STEAM_CALL_RESULT( UserFavoriteItemsListChanged_t )
    SteamAPICall_t AddItemToFavorites( AppId_t nAppId, PublishedFileId_t nPublishedFileID );


    STEAM_CALL_RESULT( UserFavoriteItemsListChanged_t )
    SteamAPICall_t RemoveItemFromFavorites( AppId_t nAppId, PublishedFileId_t nPublishedFileID );


    STEAM_CALL_RESULT( RemoteStorageSubscribePublishedFileResult_t )
    SteamAPICall_t SubscribeItem( PublishedFileId_t nPublishedFileID );
    // subscribe to this item, will be installed ASAP

    STEAM_CALL_RESULT( RemoteStorageUnsubscribePublishedFileResult_t )
    SteamAPICall_t UnsubscribeItem( PublishedFileId_t nPublishedFileID );
    // unsubscribe from this item, will be uninstalled after game quits

    uint32 GetNumSubscribedItems();
    // number of subscribed items 

    uint32 GetSubscribedItems( PublishedFileId_t* pvecPublishedFileID, uint32 cMaxEntries );
    // all subscribed item PublishFileIDs

    // get EItemState flags about item on this client
    uint32 GetItemState( PublishedFileId_t nPublishedFileID );


    // get info about currently installed content on disc for items that have k_EItemStateInstalled set
    // if k_EItemStateLegacyItem is set, pchFolder contains the path to the legacy file itself (not a folder)
    bool GetItemInstallInfo( PublishedFileId_t nPublishedFileID, uint64 *punSizeOnDisk, STEAM_OUT_STRING_COUNT( cchFolderSize ) char *pchFolder, uint32 cchFolderSize, uint32 *punTimeStamp );


    // get info about pending update for items that have k_EItemStateNeedsUpdate set. punBytesTotal will be valid after download started once
    bool GetItemDownloadInfo( PublishedFileId_t nPublishedFileID, uint64 *punBytesDownloaded, uint64 *punBytesTotal );

    bool GetItemInstallInfo( PublishedFileId_t nPublishedFileID, uint64 *punSizeOnDisk, STEAM_OUT_STRING_COUNT( cchFolderSize ) char *pchFolder, uint32 cchFolderSize, bool *pbLegacyItem ); // returns true if item is installed


    bool GetItemUpdateInfo( PublishedFileId_t nPublishedFileID, bool *pbNeedsUpdate, bool *pbIsDownloading, uint64 *punBytesDownloaded, uint64 *punBytesTotal );

    bool GetItemInstallInfo( PublishedFileId_t nPublishedFileID, uint64 *punSizeOnDisk, char *pchFolder, uint32 cchFolderSize ); // returns true if item is installed


    // download new or update already installed item. If function returns true, wait for DownloadItemResult_t. If the item is already installed,
    // then files on disk should not be used until callback received. If item is not subscribed to, it will be cached for some time.
    // If bHighPriority is set, any other item download will be suspended and this item downloaded ASAP.
    bool DownloadItem( PublishedFileId_t nPublishedFileID, bool bHighPriority );


    // game servers can set a specific workshop folder before issuing any UGC commands.
    // This is helpful if you want to support multiple game servers running out of the same install folder
    bool BInitWorkshopForGameServer( DepotId_t unWorkshopDepotID, const char *pszFolder );


    // SuspendDownloads( true ) will suspend all workshop downloads until SuspendDownloads( false ) is called or the game ends
    void SuspendDownloads( bool bSuspend );


    // usage tracking
    STEAM_CALL_RESULT( StartPlaytimeTrackingResult_t )
    SteamAPICall_t StartPlaytimeTracking( PublishedFileId_t *pvecPublishedFileID, uint32 unNumPublishedFileIDs );

    STEAM_CALL_RESULT( StopPlaytimeTrackingResult_t )
    SteamAPICall_t StopPlaytimeTracking( PublishedFileId_t *pvecPublishedFileID, uint32 unNumPublishedFileIDs );

    STEAM_CALL_RESULT( StopPlaytimeTrackingResult_t )
    SteamAPICall_t StopPlaytimeTrackingForAllItems();


    // parent-child relationship or dependency management
    STEAM_CALL_RESULT( AddUGCDependencyResult_t )
    SteamAPICall_t AddDependency( PublishedFileId_t nParentPublishedFileID, PublishedFileId_t nChildPublishedFileID );

    STEAM_CALL_RESULT( RemoveUGCDependencyResult_t )
    SteamAPICall_t RemoveDependency( PublishedFileId_t nParentPublishedFileID, PublishedFileId_t nChildPublishedFileID );


    // add/remove app dependence/requirements (usually DLC)
    STEAM_CALL_RESULT( AddAppDependencyResult_t )
    SteamAPICall_t AddAppDependency( PublishedFileId_t nPublishedFileID, AppId_t nAppID );

    STEAM_CALL_RESULT( RemoveAppDependencyResult_t )
    SteamAPICall_t RemoveAppDependency( PublishedFileId_t nPublishedFileID, AppId_t nAppID );

    // request app dependencies. note that whatever callback you register for GetAppDependenciesResult_t may be called multiple times
    // until all app dependencies have been returned
    STEAM_CALL_RESULT( GetAppDependenciesResult_t )
    SteamAPICall_t GetAppDependencies( PublishedFileId_t nPublishedFileID );


    // delete the item without prompting the user
    STEAM_CALL_RESULT( DeleteItemResult_t )
    SteamAPICall_t DeleteItem( PublishedFileId_t nPublishedFileID );

    // Show the app's latest Workshop EULA to the user in an overlay window, where they can accept it or not
    bool ShowWorkshopEULA();

    // Retrieve information related to the user's acceptance or not of the app's specific Workshop EULA
    STEAM_CALL_RESULT( WorkshopEULAStatus_t )
    SteamAPICall_t GetWorkshopEULAStatus();

    // Return the user's community content descriptor preferences
    uint32 GetUserContentDescriptorPreferences( EUGCContentDescriptorID *pvecDescriptors, uint32 cMaxEntries );

};

#endif // __INCLUDED_STEAM_UGC_H__
