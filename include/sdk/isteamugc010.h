
#ifndef ISTEAMUGC010_H
#define ISTEAMUGC010_H
#ifdef STEAM_WIN32
#pragma once
#endif

class ISteamUGC010
{
public:

	// Query UGC associated with a user. Creator app id or consumer app id must be valid and be set to the current running app. unPage should start at 1.
	virtual UGCQueryHandle_t CreateQueryUserUGCRequest( AccountID_t unAccountID, EUserUGCList eListType, EUGCMatchingUGCType eMatchingUGCType, EUserUGCListSortOrder eSortOrder, AppId_t nCreatorAppID, AppId_t nConsumerAppID, uint32 unPage ) = 0;

	// Query for all matching UGC. Creator app id or consumer app id must be valid and be set to the current running app. unPage should start at 1.
	virtual UGCQueryHandle_t CreateQueryAllUGCRequest( EUGCQuery eQueryType, EUGCMatchingUGCType eMatchingeMatchingUGCTypeFileType, AppId_t nCreatorAppID, AppId_t nConsumerAppID, uint32 unPage ) = 0;

	// Query for the details of the given published file ids (the RequestUGCDetails call is deprecated and replaced with this)
	virtual UGCQueryHandle_t CreateQueryUGCDetailsRequest( PublishedFileId_t *pvecPublishedFileID, uint32 unNumPublishedFileIDs ) = 0;

	// Send the query to Steam
	STEAM_CALL_RESULT( SteamUGCQueryCompleted_t )
	virtual SteamAPICall_t SendQueryUGCRequest( UGCQueryHandle_t handle ) = 0;

	// Retrieve an individual result after receiving the callback for querying UGC
	virtual bool GetQueryUGCResult_old( UGCQueryHandle_t handle, uint32 index, SteamUGCDetails_t *pDetails ) = 0;
	virtual bool GetQueryUGCPreviewURL( UGCQueryHandle_t handle, uint32 index, STEAM_OUT_STRING_COUNT(cchURLSize) char *pchURL, uint32 cchURLSize ) = 0;
	virtual bool GetQueryUGCMetadata( UGCQueryHandle_t handle, uint32 index, STEAM_OUT_STRING_COUNT(cchMetadatasize) char *pchMetadata, uint32 cchMetadatasize ) = 0;
	virtual bool GetQueryUGCChildren( UGCQueryHandle_t handle, uint32 index, PublishedFileId_t* pvecPublishedFileID, uint32 cMaxEntries ) = 0;
	virtual bool GetQueryUGCStatistic( UGCQueryHandle_t handle, uint32 index, EItemStatistic eStatType, uint64 *pStatValue ) = 0;
	virtual uint32 GetQueryUGCNumAdditionalPreviews( UGCQueryHandle_t handle, uint32 index ) = 0;
	virtual bool GetQueryUGCAdditionalPreview( UGCQueryHandle_t handle, uint32 index, uint32 previewIndex, STEAM_OUT_STRING_COUNT(cchURLSize) char *pchURLOrVideoID, uint32 cchURLSize, STEAM_OUT_STRING_COUNT(cchURLSize) char *pchOriginalFileName, uint32 cchOriginalFileNameSize, EItemPreviewType *pPreviewType ) = 0;
	virtual uint32 GetQueryUGCNumKeyValueTags( UGCQueryHandle_t handle, uint32 index ) = 0;
	virtual bool GetQueryUGCKeyValueTag( UGCQueryHandle_t handle, uint32 index, uint32 keyValueTagIndex, STEAM_OUT_STRING_COUNT(cchKeySize) char *pchKey, uint32 cchKeySize, STEAM_OUT_STRING_COUNT(cchValueSize) char *pchValue, uint32 cchValueSize ) = 0;

	// Release the request to free up memory, after retrieving results
	virtual bool ReleaseQueryUGCRequest( UGCQueryHandle_t handle ) = 0;

	// Options to set for querying UGC
	virtual bool AddRequiredTag( UGCQueryHandle_t handle, const char *pTagName ) = 0;
	virtual bool AddExcludedTag( UGCQueryHandle_t handle, const char *pTagName ) = 0;
	virtual bool SetReturnOnlyIDs( UGCQueryHandle_t handle, bool bReturnOnlyIDs ) = 0;
	virtual bool SetReturnKeyValueTags( UGCQueryHandle_t handle, bool bReturnKeyValueTags ) = 0;
	virtual bool SetReturnLongDescription( UGCQueryHandle_t handle, bool bReturnLongDescription ) = 0;
	virtual bool SetReturnMetadata( UGCQueryHandle_t handle, bool bReturnMetadata ) = 0;
	virtual bool SetReturnChildren( UGCQueryHandle_t handle, bool bReturnChildren ) = 0;
	virtual bool SetReturnAdditionalPreviews( UGCQueryHandle_t handle, bool bReturnAdditionalPreviews ) = 0;
	virtual bool SetReturnTotalOnly( UGCQueryHandle_t handle, bool bReturnTotalOnly ) = 0;
	virtual bool SetReturnPlaytimeStats( UGCQueryHandle_t handle, uint32 unDays ) = 0;
	virtual bool SetLanguage( UGCQueryHandle_t handle, const char *pchLanguage ) = 0;
	virtual bool SetAllowCachedResponse( UGCQueryHandle_t handle, uint32 unMaxAgeSeconds ) = 0;

	// Options only for querying user UGC
	virtual bool SetCloudFileNameFilter( UGCQueryHandle_t handle, const char *pMatchCloudFileName ) = 0;

	// Options only for querying all UGC
	virtual bool SetMatchAnyTag( UGCQueryHandle_t handle, bool bMatchAnyTag ) = 0;
	virtual bool SetSearchText( UGCQueryHandle_t handle, const char *pSearchText ) = 0;
	virtual bool SetRankedByTrendDays( UGCQueryHandle_t handle, uint32 unDays ) = 0;
	virtual bool AddRequiredKeyValueTag( UGCQueryHandle_t handle, const char *pKey, const char *pValue ) = 0;

	// DEPRECATED - Use CreateQueryUGCDetailsRequest call above instead!
	virtual SteamAPICall_t RequestUGCDetails_old( PublishedFileId_t nPublishedFileID, uint32 unMaxAgeSeconds ) = 0;

	// Steam Workshop Creator API
	STEAM_CALL_RESULT( CreateItemResult_t )
	virtual SteamAPICall_t CreateItem( AppId_t nConsumerAppId, EWorkshopFileType eFileType ) = 0; // create new item for this app with no content attached yet

	virtual UGCUpdateHandle_t StartItemUpdate( AppId_t nConsumerAppId, PublishedFileId_t nPublishedFileID ) = 0; // start an UGC item update. Set changed properties before commiting update with CommitItemUpdate()

	virtual bool SetItemTitle( UGCUpdateHandle_t handle, const char *pchTitle ) = 0; // change the title of an UGC item
	virtual bool SetItemDescription( UGCUpdateHandle_t handle, const char *pchDescription ) = 0; // change the description of an UGC item
	virtual bool SetItemUpdateLanguage( UGCUpdateHandle_t handle, const char *pchLanguage ) = 0; // specify the language of the title or description that will be set
	virtual bool SetItemMetadata( UGCUpdateHandle_t handle, const char *pchMetaData ) = 0; // change the metadata of an UGC item (max = k_cchDeveloperMetadataMax)
	virtual bool SetItemVisibility( UGCUpdateHandle_t handle, ERemoteStoragePublishedFileVisibility eVisibility ) = 0; // change the visibility of an UGC item
	virtual bool SetItemTags( UGCUpdateHandle_t updateHandle, const SteamParamStringArray_t *pTags ) = 0; // change the tags of an UGC item
	virtual bool SetItemContent( UGCUpdateHandle_t handle, const char *pszContentFolder ) = 0; // update item content from this local folder
	virtual bool SetItemPreview( UGCUpdateHandle_t handle, const char *pszPreviewFile ) = 0; //  change preview image file for this item. pszPreviewFile points to local image file, which must be under 1MB in size
	virtual bool RemoveItemKeyValueTags( UGCUpdateHandle_t handle, const char *pchKey ) = 0; // remove any existing key-value tags with the specified key
	virtual bool AddItemKeyValueTag( UGCUpdateHandle_t handle, const char *pchKey, const char *pchValue ) = 0; // add new key-value tags for the item. Note that there can be multiple values for a tag.
	virtual bool AddItemPreviewFile( UGCUpdateHandle_t handle, const char *pszPreviewFile, EItemPreviewType type ) = 0; //  add preview file for this item. pszPreviewFile points to local file, which must be under 1MB in size
	virtual bool AddItemPreviewVideo( UGCUpdateHandle_t handle, const char *pszVideoID ) = 0; //  add preview video for this item
	virtual bool UpdateItemPreviewFile( UGCUpdateHandle_t handle, uint32 index, const char *pszPreviewFile ) = 0; //  updates an existing preview file for this item. pszPreviewFile points to local file, which must be under 1MB in size
	virtual bool UpdateItemPreviewVideo( UGCUpdateHandle_t handle, uint32 index, const char *pszVideoID ) = 0; //  updates an existing preview video for this item
	virtual bool RemoveItemPreview( UGCUpdateHandle_t handle, uint32 index ) = 0; // remove a preview by index starting at 0 (previews are sorted)

	STEAM_CALL_RESULT( SubmitItemUpdateResult_t )
	virtual SteamAPICall_t SubmitItemUpdate( UGCUpdateHandle_t handle, const char *pchChangeNote ) = 0; // commit update process started with StartItemUpdate()
	virtual EItemUpdateStatus GetItemUpdateProgress( UGCUpdateHandle_t handle, uint64 *punBytesProcessed, uint64* punBytesTotal ) = 0;

	// Steam Workshop Consumer API
	STEAM_CALL_RESULT( SetUserItemVoteResult_t )
	virtual SteamAPICall_t SetUserItemVote( PublishedFileId_t nPublishedFileID, bool bVoteUp ) = 0;
	STEAM_CALL_RESULT( GetUserItemVoteResult_t )
	virtual SteamAPICall_t GetUserItemVote( PublishedFileId_t nPublishedFileID ) = 0;
	STEAM_CALL_RESULT( UserFavoriteItemsListChanged_t )
	virtual SteamAPICall_t AddItemToFavorites( AppId_t nAppId, PublishedFileId_t nPublishedFileID ) = 0;
	STEAM_CALL_RESULT( UserFavoriteItemsListChanged_t )
	virtual SteamAPICall_t RemoveItemFromFavorites( AppId_t nAppId, PublishedFileId_t nPublishedFileID ) = 0;
	STEAM_CALL_RESULT( RemoteStorageSubscribePublishedFileResult_t )
	virtual SteamAPICall_t SubscribeItem( PublishedFileId_t nPublishedFileID ) = 0; // subscribe to this item, will be installed ASAP
	STEAM_CALL_RESULT( RemoteStorageUnsubscribePublishedFileResult_t )
	virtual SteamAPICall_t UnsubscribeItem( PublishedFileId_t nPublishedFileID ) = 0; // unsubscribe from this item, will be uninstalled after game quits
	virtual uint32 GetNumSubscribedItems() = 0; // number of subscribed items 
	virtual uint32 GetSubscribedItems( PublishedFileId_t* pvecPublishedFileID, uint32 cMaxEntries ) = 0; // all subscribed item PublishFileIDs

	// get EItemState flags about item on this client
	virtual uint32 GetItemState( PublishedFileId_t nPublishedFileID ) = 0;

	// get info about currently installed content on disc for items that have k_EItemStateInstalled set
	// if k_EItemStateLegacyItem is set, pchFolder contains the path to the legacy file itself (not a folder)
	virtual bool GetItemInstallInfo( PublishedFileId_t nPublishedFileID, uint64 *punSizeOnDisk, STEAM_OUT_STRING_COUNT( cchFolderSize ) char *pchFolder, uint32 cchFolderSize, uint32 *punTimeStamp ) = 0;

	// get info about pending update for items that have k_EItemStateNeedsUpdate set. punBytesTotal will be valid after download started once
	virtual bool GetItemDownloadInfo( PublishedFileId_t nPublishedFileID, uint64 *punBytesDownloaded, uint64 *punBytesTotal ) = 0;
		
	// download new or update already installed item. If function returns true, wait for DownloadItemResult_t. If the item is already installed,
	// then files on disk should not be used until callback received. If item is not subscribed to, it will be cached for some time.
	// If bHighPriority is set, any other item download will be suspended and this item downloaded ASAP.
	virtual bool DownloadItem( PublishedFileId_t nPublishedFileID, bool bHighPriority ) = 0;

	// game servers can set a specific workshop folder before issuing any UGC commands.
	// This is helpful if you want to support multiple game servers running out of the same install folder
	virtual bool BInitWorkshopForGameServer( DepotId_t unWorkshopDepotID, const char *pszFolder ) = 0;

	// SuspendDownloads( true ) will suspend all workshop downloads until SuspendDownloads( false ) is called or the game ends
	virtual void SuspendDownloads( bool bSuspend ) = 0;

	// usage tracking
	STEAM_CALL_RESULT( StartPlaytimeTrackingResult_t )
	virtual SteamAPICall_t StartPlaytimeTracking( PublishedFileId_t *pvecPublishedFileID, uint32 unNumPublishedFileIDs ) = 0;
	STEAM_CALL_RESULT( StopPlaytimeTrackingResult_t )
	virtual SteamAPICall_t StopPlaytimeTracking( PublishedFileId_t *pvecPublishedFileID, uint32 unNumPublishedFileIDs ) = 0;
	STEAM_CALL_RESULT( StopPlaytimeTrackingResult_t )
	virtual SteamAPICall_t StopPlaytimeTrackingForAllItems() = 0;

	// parent-child relationship or dependency management
	STEAM_CALL_RESULT( AddUGCDependencyResult_t )
	virtual SteamAPICall_t AddDependency( PublishedFileId_t nParentPublishedFileID, PublishedFileId_t nChildPublishedFileID ) = 0;
	STEAM_CALL_RESULT( RemoveUGCDependencyResult_t )
	virtual SteamAPICall_t RemoveDependency( PublishedFileId_t nParentPublishedFileID, PublishedFileId_t nChildPublishedFileID ) = 0;

	// add/remove app dependence/requirements (usually DLC)
	STEAM_CALL_RESULT( AddAppDependencyResult_t )
	virtual SteamAPICall_t AddAppDependency( PublishedFileId_t nPublishedFileID, AppId_t nAppID ) = 0;
	STEAM_CALL_RESULT( RemoveAppDependencyResult_t )
	virtual SteamAPICall_t RemoveAppDependency( PublishedFileId_t nPublishedFileID, AppId_t nAppID ) = 0;
	// request app dependencies. note that whatever callback you register for GetAppDependenciesResult_t may be called multiple times
	// until all app dependencies have been returned
	STEAM_CALL_RESULT( GetAppDependenciesResult_t )
	virtual SteamAPICall_t GetAppDependencies( PublishedFileId_t nPublishedFileID ) = 0;
	
	// delete the item without prompting the user
	STEAM_CALL_RESULT( DeleteItemResult_t )
	virtual SteamAPICall_t DeleteItem( PublishedFileId_t nPublishedFileID ) = 0;
};

#endif // ISTEAMUGC010_H
