
#ifndef ISTEAMUGC006_H
#define ISTEAMUGC006_H
#ifdef STEAM_WIN32
#pragma once
#endif


class ISteamUGC006
{
public:
	// Query UGC associated with a user. Creator app id or consumer app id must be valid and be set to the current running app. unPage should start at 1.
	virtual UGCQueryHandle_t CreateQueryUserUGCRequest( AccountID_t unAccountID, EUserUGCList eListType, EUGCMatchingUGCType eMatchingUGCType, EUserUGCListSortOrder eSortOrder, AppId_t nCreatorAppID, AppId_t nConsumerAppID, uint32 unPage ) = 0;

	// Query for all matching UGC. Creator app id or consumer app id must be valid and be set to the current running app. unPage should start at 1.
	virtual UGCQueryHandle_t CreateQueryAllUGCRequest( EUGCQuery eQueryType, EUGCMatchingUGCType eMatchingeMatchingUGCTypeFileType, AppId_t nCreatorAppID, AppId_t nConsumerAppID, uint32 unPage ) = 0;

	virtual UGCQueryHandle_t CreateQueryUGCDetailsRequest( PublishedFileId_t *pvecPublishedFileID, uint32 unNumPublishedFileIDs ) = 0;
	
	// Send the query to Steam
	virtual SteamAPICall_t SendQueryUGCRequest( UGCQueryHandle_t handle ) = 0;

	// Retrieve an individual result after receiving the callback for querying UGC
	virtual bool GetQueryUGCResult_old( UGCQueryHandle_t handle, uint32 index, SteamUGCDetails_t *pDetails ) = 0;
	
	virtual bool GetQueryUGCPreviewURL( UGCQueryHandle_t handle, uint32 index, char *pchURL, uint32 cchURLSize ) = 0;
	virtual bool GetQueryUGCMetadata( UGCQueryHandle_t handle, uint32 index, char *pchMetadata, uint32 cchMetadatasize ) = 0;
	virtual bool GetQueryUGCChildren( UGCQueryHandle_t handle, uint32 index, PublishedFileId_t *pvecPublishedFileID, uint32 cMaxEntries )= 0;
	virtual bool GetQueryUGCStatistic( UGCQueryHandle_t handle, uint32 index, EItemStatistic eStatType, uint32 *pStatValue ) = 0;

	virtual uint32 GetQueryUGCNumAdditionalPreviews( UGCQueryHandle_t handle, uint32 index ) = 0;
	virtual bool GetQueryUGCAdditionalPreview( UGCQueryHandle_t handle, uint32 index, uint32 previewIndex, char *pchURLOrVideoID, uint32 cchURLSize, bool *hz ) = 0;
	
	// Release the request to free up memory, after retrieving results
	virtual bool ReleaseQueryUGCRequest( UGCQueryHandle_t handle ) = 0;

	// Options to set for querying UGC
	virtual bool AddRequiredTag( UGCQueryHandle_t handle, const char *pTagName ) = 0;
	virtual bool AddExcludedTag( UGCQueryHandle_t handle, const char *pTagName ) = 0;
	virtual bool SetReturnLongDescription( UGCQueryHandle_t handle, bool bReturnLongDescription ) = 0;
	virtual bool SetReturnMetadata( UGCQueryHandle_t handle, bool bReturnMetadata  ) = 0;
	virtual bool SetReturnChildren( UGCQueryHandle_t handle, bool bReturnChildren  ) = 0;
	virtual bool SetReturnAdditionalPreviews( UGCQueryHandle_t handle, bool bReturnAdditionalPreviews ) = 0;
	virtual bool SetReturnTotalOnly( UGCQueryHandle_t handle, bool bReturnTotalOnly ) = 0;
	virtual bool SetLanguage( UGCQueryHandle_t handle, const char *pchLanguage ) = 0;
	virtual bool SetAllowCachedResponse( UGCQueryHandle_t handle, uint32 unMaxAgeSeconds ) = 0;

	// Options only for querying user UGC
	virtual bool SetCloudFileNameFilter( UGCQueryHandle_t handle, const char *pMatchCloudFileName ) = 0;

	// Options only for querying all UGC
	virtual bool SetMatchAnyTag( UGCQueryHandle_t handle, bool bMatchAnyTag ) = 0;
	virtual bool SetSearchText( UGCQueryHandle_t handle, const char *pSearchText ) = 0;
	virtual bool SetRankedByTrendDays( UGCQueryHandle_t handle, uint32 unDays ) = 0;

	// Request full details for one piece of UGC
	virtual SteamAPICall_t RequestUGCDetails_old( PublishedFileId_t nPublishedFileID, uint32 unMaxAgeSeconds ) = 0;

	// Steam Workshop Creator API
	virtual SteamAPICall_t CreateItem( AppId_t nConsumerAppId, EWorkshopFileType eFileType ) = 0; // create new item for this app with no content attached yet

	virtual UGCUpdateHandle_t StartItemUpdate( AppId_t nConsumerAppId, PublishedFileId_t nPublishedFileID ) = 0; // start an UGC item update. Set changed properties before commiting update with CommitItemUpdate()

	virtual bool SetItemTitle( UGCUpdateHandle_t handle, const char *pchTitle ) = 0; // change the title of an UGC item
	virtual bool SetItemDescription( UGCUpdateHandle_t handle, const char *pchDescription ) = 0; // change the description of an UGC item
	virtual bool SetItemUpdateLanguage( UGCUpdateHandle_t handle, const char *pchUpdateLanguage ) = 0;
	virtual bool SetItemMetadata( UGCUpdateHandle_t handle, const char *pchMetadata ) = 0;
	virtual bool SetItemVisibility( UGCUpdateHandle_t handle, ERemoteStoragePublishedFileVisibility eVisibility ) = 0; // change the visibility of an UGC item
	virtual bool SetItemTags( UGCUpdateHandle_t updateHandle, const SteamParamStringArray_t *pTags ) = 0; // change the tags of an UGC item
	virtual bool SetItemContent( UGCUpdateHandle_t handle, const char *pszContentFolder ) = 0; // update item content from this local folder
	virtual bool SetItemPreview( UGCUpdateHandle_t handle, const char *pszPreviewFile ) = 0; //  change preview image file for this item. pszPreviewFile points to local image file 

	virtual SteamAPICall_t SubmitItemUpdate( UGCUpdateHandle_t handle, const char *pchChangeNote ) = 0; // commit update process started with StartItemUpdate()
	virtual EItemUpdateStatus GetItemUpdateProgress( UGCUpdateHandle_t handle, uint64 *punBytesProcessed, uint64* punBytesTotal ) = 0;

	virtual SteamAPICall_t SetUserItemVote( PublishedFileId_t nPublishedFileID, bool bVoteUp ) = 0;
	virtual SteamAPICall_t GetUserItemVote( PublishedFileId_t nPublishedFileID ) = 0;	
	
	virtual SteamAPICall_t AddItemToFavorites( AppId_t nAppId, PublishedFileId_t nPublishedFileID ) = 0;
	virtual SteamAPICall_t RemoveItemFromFavorites( AppId_t nAppId, PublishedFileId_t nPublishedFileID ) = 0;

	// Steam Workshop Consumer API
	virtual SteamAPICall_t SubscribeItem( PublishedFileId_t nPublishedFileID ) = 0; // subscript to this item, will be installed ASAP
	virtual SteamAPICall_t UnsubscribeItem( PublishedFileId_t nPublishedFileID ) = 0; // unsubscribe from this item, will be uninstalled after game quits
	virtual uint32 GetNumSubscribedItems() = 0; // number of subscribed items 
	virtual uint32 GetSubscribedItems( PublishedFileId_t* pvecPublishedFileID, uint32 cMaxEntries ) = 0; // all subscribed item PublishFileIDs

	// Get info about the item on disk.  If you are supporting items published through the legacy RemoteStorage APIs then *pbLegacyItem will be set to true
	// and pchFolder will contain the full path to the file rather than the containing folder.
	virtual uint32 GetItemState( PublishedFileId_t nPublishedFileID ) = 0;
	virtual bool GetItemInstallInfo( PublishedFileId_t nPublishedFileID, uint64 *punSizeOnDisk, char *pchFolder, uint32 cchFolderSize, uint32 *punTimeStamp ) = 0;
	virtual bool GetItemDownloadInfo( PublishedFileId_t nPublishedFileID, uint64 *punBytesDownloaded, uint64 *punBytesTotal ) = 0;
	virtual bool DownloadItem( PublishedFileId_t nPublishedFileID, bool bHighPriority ) = 0;
};

#endif // ISTEAMUGC006_H
