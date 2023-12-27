
#ifndef ISTEAMREMOTESTORAGE005_H
#define ISTEAMREMOTESTORAGE005_H
#ifdef STEAM_WIN32
#pragma once
#endif

struct RemoteStorageUpdatePublishedFileRequest_t
{
public:
	RemoteStorageUpdatePublishedFileRequest_t()
	{
		Initialize( k_GIDNil );
	}

	RemoteStorageUpdatePublishedFileRequest_t( PublishedFileId_t unPublishedFileId )
	{
		Initialize( unPublishedFileId );
	}

	PublishedFileId_t GetPublishedFileId() { return m_unPublishedFileId; }

	void SetFile( const char *pchFile )
	{
		m_pchFile = pchFile;
		m_bUpdateFile = true;
	}

	const char *GetFile() { return m_pchFile; }
	bool BUpdateFile() { return m_bUpdateFile; }

	void SetPreviewFile( const char *pchPreviewFile )
	{
		m_pchPreviewFile = pchPreviewFile;
		m_bUpdatePreviewFile = true;
	}

	const char *GetPreviewFile() { return m_pchPreviewFile; }
	bool BUpdatePreviewFile() { return m_bUpdatePreviewFile; }

	void SetTitle( const char *pchTitle )
	{
		m_pchTitle = pchTitle;
		m_bUpdateTitle = true;
	}

	const char *GetTitle() { return m_pchTitle; }
	bool BUpdateTitle() { return m_bUpdateTitle; }

	void SetDescription( const char *pchDescription )
	{
		m_pchDescription = pchDescription;
		m_bUpdateDescription = true;
	}

	const char *GetDescription() { return m_pchDescription; }
	bool BUpdateDescription() { return m_bUpdateDescription; }

	void SetVisibility( ERemoteStoragePublishedFileVisibility eVisibility )
	{
		m_eVisibility = eVisibility;
		m_bUpdateVisibility = true;
	}

	const ERemoteStoragePublishedFileVisibility GetVisibility() { return m_eVisibility; }
	bool BUpdateVisibility() { return m_bUpdateVisibility; }

	void SetTags( SteamParamStringArray_t *pTags )
	{
		m_pTags = pTags;
		m_bUpdateTags = true;
	}

	SteamParamStringArray_t *GetTags() { return m_pTags; }
	bool BUpdateTags() { return m_bUpdateTags; }
	SteamParamStringArray_t **GetTagsPointer() { return &m_pTags; }

	void Initialize( PublishedFileId_t unPublishedFileId )
	{
		m_unPublishedFileId = unPublishedFileId;
		m_pchFile = 0;
		m_pchPreviewFile = 0;
		m_pchTitle = 0;
		m_pchDescription = 0;
		m_pTags = 0;

		m_bUpdateFile = false;
		m_bUpdatePreviewFile = false;
		m_bUpdateTitle = false;
		m_bUpdateDescription = false;
		m_bUpdateTags = false;
		m_bUpdateVisibility = false;
	}

private:
	PublishedFileId_t m_unPublishedFileId;
	const char *m_pchFile;
	const char *m_pchPreviewFile;
	const char *m_pchTitle;
	const char *m_pchDescription;
	ERemoteStoragePublishedFileVisibility m_eVisibility;
	SteamParamStringArray_t *m_pTags;

	bool m_bUpdateFile;
	bool m_bUpdatePreviewFile;
	bool m_bUpdateTitle;
	bool m_bUpdateDescription;
	bool m_bUpdateVisibility;
	bool m_bUpdateTags;
};


//-----------------------------------------------------------------------------
// Purpose: Functions for accessing, reading and writing files stored remotely 
//			and cached locally
//-----------------------------------------------------------------------------
class ISteamRemoteStorage005
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
	virtual SteamAPICall_t UGCDownload( UGCHandle_t hContent ) = 0;
	virtual bool	GetUGCDetails( UGCHandle_t hContent, AppId_t *pnAppID, char **ppchName, int32 *pnFileSizeInBytes, CSteamID *pSteamIDOwner ) = 0;
	virtual int32	UGCRead( UGCHandle_t hContent, void *pvData, int32 cubDataToRead ) = 0;

	// user generated content iteration
	virtual int32	GetCachedUGCCount() = 0;
	virtual	UGCHandle_t GetCachedUGCHandle( int32 iCachedContent ) = 0;

	// The following functions are only necessary on the Playstation 3. On PC & Mac, the Steam client will handle these operations for you
	// On Playstation 3, the game controls which files are stored in the cloud, via FilePersist, FileFetch, and FileForget.
		
#if defined(_PS3) || defined(_SERVER)
	// Connect to Steam and get a list of files in the Cloud - results in a RemoteStorageAppSyncStatusCheck_t callback
	virtual void GetFileListFromServer() = 0;
	// Indicate this file should be downloaded in the next sync
	virtual bool FileFetch( const char *pchFile ) = 0;
	// Indicate this file should be persisted in the next sync
	virtual bool FilePersist( const char *pchFile ) = 0;
	// Pull any requested files down from the Cloud - results in a RemoteStorageAppSyncedClient_t callback
	virtual bool SynchronizeToClient() = 0;
	// Upload any requested files to the Cloud - results in a RemoteStorageAppSyncedServer_t callback
	virtual bool SynchronizeToServer() = 0;
	// Reset any fetch/persist/etc requests
	virtual bool ResetFileRequestState() = 0;
#endif

	// publishing UGC
	virtual SteamAPICall_t	PublishFile( const char *pchFile, const char *pchPreviewFile, AppId_t nConsumerAppId, const char *pchTitle, const char *pchDescription, ERemoteStoragePublishedFileVisibility eVisibility, SteamParamStringArray_t *pTags ) = 0;
	virtual SteamAPICall_t	PublishWorkshopFile( const char *pchFile, const char *pchPreviewFile, AppId_t nConsumerAppId, const char *pchTitle, const char *pchDescription, SteamParamStringArray_t *pTags ) = 0;
	virtual SteamAPICall_t	UpdatePublishedFile( RemoteStorageUpdatePublishedFileRequest_t updatePublishedFileRequest ) = 0;
	virtual SteamAPICall_t	GetPublishedFileDetails( PublishedFileId_t unPublishedFileId ) = 0;
	virtual SteamAPICall_t	DeletePublishedFile( PublishedFileId_t unPublishedFileId ) = 0;
	virtual SteamAPICall_t	EnumerateUserPublishedFiles( uint32 unStartIndex ) = 0;
	virtual SteamAPICall_t	SubscribePublishedFile( PublishedFileId_t unPublishedFileId ) = 0;
	virtual SteamAPICall_t	EnumerateUserSubscribedFiles( uint32 unStartIndex ) = 0;
	virtual SteamAPICall_t	UnsubscribePublishedFile( PublishedFileId_t unPublishedFileId ) = 0;
};

#endif // ISTEAMREMOTESTORAGE005_H
