
#ifndef ISTEAMREMOTESTORAGE009_H
#define ISTEAMREMOTESTORAGE009_H
#ifdef STEAM_WIN32
#pragma once
#endif

//-----------------------------------------------------------------------------
// Purpose: Functions for accessing, reading and writing files stored remotely 
//			and cached locally
//-----------------------------------------------------------------------------
class ISteamRemoteStorage009
{
public:
	// NOTE
	//
	// Filenames are case-insensitive, and will be converted to lowercase automatically.
	// So "foo.bar" and "Foo.bar" are the same file, and if you write "Foo.bar" then
	// iterate the files, the filename returned will be "foo.bar".
	//

	// file operations
	virtual bool	FileWrite( const char *pchFile, const void *pvData, int32 cubData ) = 0;
	virtual int32	FileRead( const char *pchFile, void *pvData, int32 cubDataToRead ) = 0;
	virtual bool	FileForget( const char *pchFile ) = 0;
	virtual bool	FileDelete( const char *pchFile ) = 0;
	virtual SteamAPICall_t FileShare( const char *pchFile ) = 0;
	virtual bool	SetSyncPlatforms( const char *pchFile, ERemoteStoragePlatform eRemoteStoragePlatform ) = 0;

	// file operations that cause network IO
	virtual UGCFileWriteStreamHandle_t FileWriteStreamOpen( const char *pchFile ) = 0;
	virtual bool FileWriteStreamWriteChunk( UGCFileWriteStreamHandle_t writeHandle, const void *pvData, int32 cubData ) = 0;
	virtual bool FileWriteStreamClose( UGCFileWriteStreamHandle_t writeHandle ) = 0;
	virtual bool FileWriteStreamCancel( UGCFileWriteStreamHandle_t writeHandle ) = 0;

	// file information
	virtual bool	FileExists( const char *pchFile ) = 0;
	virtual bool	FilePersisted( const char *pchFile ) = 0;
	virtual int32	GetFileSize( const char *pchFile ) = 0;
	virtual int64	GetFileTimestamp( const char *pchFile ) = 0;
	virtual ERemoteStoragePlatform GetSyncPlatforms( const char *pchFile ) = 0;

	// iteration
	virtual int32 GetFileCount() = 0;
	virtual const char *GetFileNameAndSize( int iFile, int32 *pnFileSizeInBytes ) = 0;

	// configuration management
	virtual bool GetQuota( int32 *pnTotalBytes, int32 *puAvailableBytes ) = 0;
	virtual bool IsCloudEnabledForAccount() = 0;
	virtual bool IsCloudEnabledForApp() = 0;
	virtual void SetCloudEnabledForApp( bool bEnabled ) = 0;

	// user generated content

	// Downloads a UGC file.  A priority value of 0 will download the file immediately,
	// otherwise it will wait to download the file until all downloads with a lower priority
	// value are completed.  Downloads with equal priority will occur simultaneously.
	virtual SteamAPICall_t UGCDownload( UGCHandle_t hContent ) = 0;

	// Gets the amount of data downloaded so far for a piece of content. pnBytesExpected can be 0 if function returns false
	// or if the transfer hasn't started yet, so be careful to check for that before dividing to get a percentage
	virtual bool	GetUGCDownloadProgress( UGCHandle_t hContent, int32 *pnBytesDownloaded, int32 *pnBytesExpected ) = 0;

	// Gets metadata for a file after it has been downloaded. This is the same metadata given in the RemoteStorageDownloadUGCResult_t call result
	virtual bool	GetUGCDetails( UGCHandle_t hContent, AppId_t *pnAppID, char **ppchName, int32 *pnFileSizeInBytes, CSteamID *pSteamIDOwner ) = 0;

	// After download, gets the content of the file
	virtual int32	UGCRead( UGCHandle_t hContent, void *pvData, int32 cubDataToRead, uint32 cOffset ) = 0;

	// Functions to iterate through UGC that has finished downloading but has not yet been read via UGCRead()
	virtual int32	GetCachedUGCCount() = 0;
	virtual	UGCHandle_t GetCachedUGCHandle( int32 iCachedContent ) = 0;

	// publishing UGC
	virtual SteamAPICall_t	PublishWorkshopFile( const char *pchFile, const char *pchPreviewFile, AppId_t nConsumerAppId, const char *pchTitle, const char *pchDescription, ERemoteStoragePublishedFileVisibility eVisibility, SteamParamStringArray_t *pTags, EWorkshopFileType eWorkshopFileType ) = 0;
	virtual PublishedFileUpdateHandle_t CreatePublishedFileUpdateRequest( PublishedFileId_t unPublishedFileId ) = 0;
	virtual bool UpdatePublishedFileFile( PublishedFileUpdateHandle_t updateHandle, const char *pchFile ) = 0;
	virtual bool UpdatePublishedFilePreviewFile( PublishedFileUpdateHandle_t updateHandle, const char *pchPreviewFile ) = 0;
	virtual bool UpdatePublishedFileTitle( PublishedFileUpdateHandle_t updateHandle, const char *pchTitle ) = 0;
	virtual bool UpdatePublishedFileDescription( PublishedFileUpdateHandle_t updateHandle, const char *pchDescription ) = 0;
	virtual bool UpdatePublishedFileVisibility( PublishedFileUpdateHandle_t updateHandle, ERemoteStoragePublishedFileVisibility eVisibility ) = 0;
	virtual bool UpdatePublishedFileTags( PublishedFileUpdateHandle_t updateHandle, SteamParamStringArray_t *pTags ) = 0;
	virtual SteamAPICall_t	CommitPublishedFileUpdate( PublishedFileUpdateHandle_t updateHandle ) = 0;
	virtual SteamAPICall_t	GetPublishedFileDetails( PublishedFileId_t unPublishedFileId ) = 0;
	virtual SteamAPICall_t	DeletePublishedFile( PublishedFileId_t unPublishedFileId ) = 0;
	// enumerate the files that the current user published with this app
	virtual SteamAPICall_t	EnumerateUserPublishedFiles( uint32 unStartIndex ) = 0;
	virtual SteamAPICall_t	SubscribePublishedFile( PublishedFileId_t unPublishedFileId ) = 0;
	virtual SteamAPICall_t	EnumerateUserSubscribedFiles( uint32 unStartIndex ) = 0;
	virtual SteamAPICall_t	UnsubscribePublishedFile( PublishedFileId_t unPublishedFileId ) = 0;
	virtual bool UpdatePublishedFileSetChangeDescription( PublishedFileUpdateHandle_t updateHandle, const char *pchChangeDescription ) = 0;
	virtual SteamAPICall_t	GetPublishedItemVoteDetails( PublishedFileId_t unPublishedFileId ) = 0;
	virtual SteamAPICall_t	UpdateUserPublishedItemVote( PublishedFileId_t unPublishedFileId, bool bVoteUp ) = 0;
	virtual SteamAPICall_t	GetUserPublishedItemVoteDetails( PublishedFileId_t unPublishedFileId ) = 0;
	virtual SteamAPICall_t	EnumerateUserSharedWorkshopFiles( CSteamID steamId, uint32 unStartIndex, SteamParamStringArray_t *pRequiredTags, SteamParamStringArray_t *pExcludedTags ) = 0;
	virtual SteamAPICall_t	PublishVideo( EWorkshopVideoProvider eVideoProvider, const char *pchVideoAccount, const char *pchVideoIdentifier, const char *pchPreviewFile, AppId_t nConsumerAppId, const char *pchTitle, const char *pchDescription, ERemoteStoragePublishedFileVisibility eVisibility, SteamParamStringArray_t *pTags ) = 0;
	virtual SteamAPICall_t	SetUserPublishedFileAction( PublishedFileId_t unPublishedFileId, EWorkshopFileAction eAction ) = 0;
	virtual SteamAPICall_t	EnumeratePublishedFilesByUserAction( EWorkshopFileAction eAction, uint32 unStartIndex ) = 0;
	// this method enumerates the public view of workshop files
	virtual SteamAPICall_t	EnumeratePublishedWorkshopFiles( EWorkshopEnumerationType eEnumerationType, uint32 unStartIndex, uint32 unCount, uint32 unDays, SteamParamStringArray_t *pTags, SteamParamStringArray_t *pUserTags ) = 0;
};

#endif // ISTEAMREMOTESTORAGE009_H
