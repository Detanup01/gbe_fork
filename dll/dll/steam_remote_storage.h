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

#ifndef __INCLUDED_STEAM_REMOTE_STORAGE_H__
#define __INCLUDED_STEAM_REMOTE_STORAGE_H__

#include "base.h"
#include "ugc_remote_storage_bridge.h"
#include "common_helpers/forgettable_memory.hpp"

struct Async_Read {
 SteamAPICall_t api_call{};
 uint32 offset{};
 uint32 to_read{};
 uint32 size{};
 std::string file_name{};
};

struct Stream_Write {
    std::string file_name{};
    UGCFileWriteStreamHandle_t write_stream_handle{};
    std::vector<char> file_data{};
};

struct Downloaded_File {
    enum class DownloadSource {
        AfterFileShare, // attempted download after a call to Steam_Remote_Storage::FileShare()
        AfterSendQueryUGCRequest, // attempted download after a call to Steam_UGC::SendQueryUGCRequest()
        FromUGCDownloadToLocation, // attempted download via Steam_Remote_Storage::UGCDownloadToLocation()
    };

private:
    DownloadSource source;

public:
    Downloaded_File(DownloadSource src);

    DownloadSource get_source() const;

    // *** used in any case
    std::string file{};
    uint64 total_size{};

    // *** used when source = AfterSendQueryUGCRequest and FromUGCDownloadToLocation
    Ugc_Remote_Storage_Bridge::QueryInfo mod_query_info{};

    // *** used when source = FromUGCDownloadToLocation only
    std::string download_to_location_fullpath{};
    
};

class Steam_Remote_Storage :
public ISteamRemoteStorage001,
public ISteamRemoteStorage002,
public ISteamRemoteStorage003,
public ISteamRemoteStorage004,
public ISteamRemoteStorage005,
public ISteamRemoteStorage006,
public ISteamRemoteStorage007,
public ISteamRemoteStorage008,
public ISteamRemoteStorage009,
public ISteamRemoteStorage010,
public ISteamRemoteStorage011,
public ISteamRemoteStorage012,
public ISteamRemoteStorage013,
public ISteamRemoteStorage014,
public ISteamRemoteStorage015,
public ISteamRemoteStorage
{
private:
    class Settings *settings{};
    class Ugc_Remote_Storage_Bridge *ugc_bridge{};
    class Local_Storage *local_storage{};
    class SteamCallResults *callback_results{};
    class SteamCallBacks *callbacks{};
    class RunEveryRunCB *run_every_runcb{};

    std::vector<struct Async_Read> async_reads{};
    std::vector<struct Stream_Write> stream_writes{};
    std::map<UGCHandle_t, std::string> shared_files{};
    std::map<UGCHandle_t, struct Downloaded_File> downloaded_files{};
    
    bool steam_cloud_enabled = true;

    common_helpers::ForgettableMemory<std::string> requests_GetFileNameAndSize{};
    common_helpers::ForgettableMemory<std::string> requests_GetUGCDetails{};

    static void steam_run_every_runcb(void *object);
    void RunCallbacks();

public:

    Steam_Remote_Storage(class Settings *settings, class Ugc_Remote_Storage_Bridge *ugc_bridge, class Local_Storage *local_storage, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb);
    ~Steam_Remote_Storage();

    // NOTE
    //
    // Filenames are case-insensitive, and will be converted to lowercase automatically.
    // So "foo.bar" and "Foo.bar" are the same file, and if you write "Foo.bar" then
    // iterate the files, the filename returned will be "foo.bar".
    //

    // file operations
    bool FileWrite( const char *pchFile, const void *pvData, int32 cubData );

    int32 FileRead( const char *pchFile, void *pvData, int32 cubDataToRead );

    STEAM_CALL_RESULT( RemoteStorageFileWriteAsyncComplete_t )
    SteamAPICall_t FileWriteAsync( const char *pchFile, const void *pvData, uint32 cubData );


    STEAM_CALL_RESULT( RemoteStorageFileReadAsyncComplete_t )
    SteamAPICall_t FileReadAsync( const char *pchFile, uint32 nOffset, uint32 cubToRead );

    bool FileReadAsyncComplete( SteamAPICall_t hReadCall, void *pvBuffer, uint32 cubToRead );


    bool FileForget( const char *pchFile );

    bool FileDelete( const char *pchFile );

    STEAM_CALL_RESULT( RemoteStorageFileShareResult_t )
    SteamAPICall_t FileShare( const char *pchFile );

    bool SetSyncPlatforms( const char *pchFile, ERemoteStoragePlatform eRemoteStoragePlatform );


    // file operations that cause network IO
    UGCFileWriteStreamHandle_t FileWriteStreamOpen( const char *pchFile );

    bool FileWriteStreamWriteChunk( UGCFileWriteStreamHandle_t writeHandle, const void *pvData, int32 cubData );

    bool FileWriteStreamClose( UGCFileWriteStreamHandle_t writeHandle );

    bool FileWriteStreamCancel( UGCFileWriteStreamHandle_t writeHandle );

    // file information
    bool FileExists( const char *pchFile );

    bool FilePersisted( const char *pchFile );

    int32 GetFileSize( const char *pchFile );

    int64 GetFileTimestamp( const char *pchFile );

    ERemoteStoragePlatform GetSyncPlatforms( const char *pchFile );


    // iteration
    int32 GetFileCount();

    const char *GetFileNameAndSize( int iFile, int32 *pnFileSizeInBytes );


    // configuration management
    bool GetQuota( uint64 *pnTotalBytes, uint64 *puAvailableBytes );

    bool GetQuota( int32 *pnTotalBytes, int32 *puAvailableBytes );

    bool IsCloudEnabledForAccount();

    bool IsCloudEnabledForApp();

    bool IsCloudEnabledThisApp();

    void SetCloudEnabledForApp( bool bEnabled );

    bool SetCloudEnabledThisApp( bool bEnabled );

    // user generated content

    // Downloads a UGC file.  A priority value of 0 will download the file immediately,
    // otherwise it will wait to download the file until all downloads with a lower priority
    // value are completed.  Downloads with equal priority will occur simultaneously.
    STEAM_CALL_RESULT( RemoteStorageDownloadUGCResult_t )
    SteamAPICall_t UGCDownload( UGCHandle_t hContent, uint32 unPriority );

    STEAM_CALL_RESULT( RemoteStorageDownloadUGCResult_t )
    SteamAPICall_t UGCDownload( UGCHandle_t hContent );


    // Gets the amount of data downloaded so far for a piece of content. pnBytesExpected can be 0 if function returns false
    // or if the transfer hasn't started yet, so be careful to check for that before dividing to get a percentage
    bool GetUGCDownloadProgress( UGCHandle_t hContent, int32 *pnBytesDownloaded, int32 *pnBytesExpected );

    bool GetUGCDownloadProgress( UGCHandle_t hContent, uint32 *pnBytesDownloaded, uint32 *pnBytesExpected );


    // Gets metadata for a file after it has been downloaded. This is the same metadata given in the RemoteStorageDownloadUGCResult_t call result
    bool GetUGCDetails( UGCHandle_t hContent, AppId_t *pnAppID, STEAM_OUT_STRING() char **ppchName, int32 *pnFileSizeInBytes, STEAM_OUT_STRUCT() CSteamID *pSteamIDOwner );


    // After download, gets the content of the file.  
    // Small files can be read all at once by calling this function with an offset of 0 and cubDataToRead equal to the size of the file.
    // Larger files can be read in chunks to reduce memory usage (since both sides of the IPC client and the game itself must allocate
    // enough memory for each chunk).  Once the last byte is read, the file is implicitly closed and further calls to UGCRead will fail
    // unless UGCDownload is called again.
    // For especially large files (anything over 100MB) it is a requirement that the file is read in chunks.
    int32 UGCRead( UGCHandle_t hContent, void *pvData, int32 cubDataToRead, uint32 cOffset, EUGCReadAction eAction );

    int32 UGCRead( UGCHandle_t hContent, void *pvData, int32 cubDataToRead );

    int32 UGCRead( UGCHandle_t hContent, void *pvData, int32 cubDataToRead, uint32 cOffset);

    // Functions to iterate through UGC that has finished downloading but has not yet been read via UGCRead()
    int32 GetCachedUGCCount();

    UGCHandle_t GetCachedUGCHandle( int32 iCachedContent );


    // The following functions are only necessary on the Playstation 3. On PC & Mac, the Steam client will handle these operations for you
    // On Playstation 3, the game controls which files are stored in the cloud, via FilePersist, FileFetch, and FileForget.
        
    #if defined(_PS3) || defined(_SERVER)
    // Connect to Steam and get a list of files in the Cloud - results in a RemoteStorageAppSyncStatusCheck_t callback
    void GetFileListFromServer();

    // Indicate this file should be downloaded in the next sync
    bool FileFetch( const char *pchFile );

    // Indicate this file should be persisted in the next sync
    bool FilePersist( const char *pchFile );

    // Pull any requested files down from the Cloud - results in a RemoteStorageAppSyncedClient_t callback
    bool SynchronizeToClient();

    // Upload any requested files to the Cloud - results in a RemoteStorageAppSyncedServer_t callback
    bool SynchronizeToServer();

    // Reset any fetch/persist/etc requests
    bool ResetFileRequestState();

    #endif

    // publishing UGC
    STEAM_CALL_RESULT( RemoteStoragePublishFileProgress_t )
    SteamAPICall_t PublishWorkshopFile( const char *pchFile, const char *pchPreviewFile, AppId_t nConsumerAppId, const char *pchTitle, const char *pchDescription, ERemoteStoragePublishedFileVisibility eVisibility, SteamParamStringArray_t *pTags, EWorkshopFileType eWorkshopFileType );

    PublishedFileUpdateHandle_t CreatePublishedFileUpdateRequest( PublishedFileId_t unPublishedFileId );

    bool UpdatePublishedFileFile( PublishedFileUpdateHandle_t updateHandle, const char *pchFile );

    SteamAPICall_t PublishFile( const char *pchFile, const char *pchPreviewFile, AppId_t nConsumerAppId, const char *pchTitle, const char *pchDescription, ERemoteStoragePublishedFileVisibility eVisibility, SteamParamStringArray_t *pTags );

    SteamAPICall_t PublishWorkshopFile( const char *pchFile, const char *pchPreviewFile, AppId_t nConsumerAppId, const char *pchTitle, const char *pchDescription, SteamParamStringArray_t *pTags );

    SteamAPICall_t UpdatePublishedFile( RemoteStorageUpdatePublishedFileRequest_t updatePublishedFileRequest );

    bool UpdatePublishedFilePreviewFile( PublishedFileUpdateHandle_t updateHandle, const char *pchPreviewFile );

    bool UpdatePublishedFileTitle( PublishedFileUpdateHandle_t updateHandle, const char *pchTitle );

    bool UpdatePublishedFileDescription( PublishedFileUpdateHandle_t updateHandle, const char *pchDescription );

    bool UpdatePublishedFileVisibility( PublishedFileUpdateHandle_t updateHandle, ERemoteStoragePublishedFileVisibility eVisibility );

    bool UpdatePublishedFileTags( PublishedFileUpdateHandle_t updateHandle, SteamParamStringArray_t *pTags );

    STEAM_CALL_RESULT( RemoteStorageUpdatePublishedFileResult_t )
    SteamAPICall_t CommitPublishedFileUpdate( PublishedFileUpdateHandle_t updateHandle );

    // Gets published file details for the given publishedfileid.  If unMaxSecondsOld is greater than 0,
    // cached data may be returned, depending on how long ago it was cached.  A value of 0 will force a refresh.
    // A value of k_WorkshopForceLoadPublishedFileDetailsFromCache will use cached data if it exists, no matter how old it is.
    STEAM_CALL_RESULT( RemoteStorageGetPublishedFileDetailsResult_t )
    SteamAPICall_t GetPublishedFileDetails( PublishedFileId_t unPublishedFileId, uint32 unMaxSecondsOld );

    STEAM_CALL_RESULT( RemoteStorageGetPublishedFileDetailsResult_t )
    SteamAPICall_t GetPublishedFileDetails( PublishedFileId_t unPublishedFileId );

    STEAM_CALL_RESULT( RemoteStorageDeletePublishedFileResult_t )
    SteamAPICall_t DeletePublishedFile( PublishedFileId_t unPublishedFileId );

    // enumerate the files that the current user published with this app
    STEAM_CALL_RESULT( RemoteStorageEnumerateUserPublishedFilesResult_t )
    SteamAPICall_t EnumerateUserPublishedFiles( uint32 unStartIndex );

    STEAM_CALL_RESULT( RemoteStorageSubscribePublishedFileResult_t )
    SteamAPICall_t SubscribePublishedFile( PublishedFileId_t unPublishedFileId );

    STEAM_CALL_RESULT( RemoteStorageEnumerateUserSubscribedFilesResult_t )
    SteamAPICall_t EnumerateUserSubscribedFiles( uint32 unStartIndex );

    STEAM_CALL_RESULT( RemoteStorageUnsubscribePublishedFileResult_t )
    SteamAPICall_t UnsubscribePublishedFile( PublishedFileId_t unPublishedFileId );

    bool UpdatePublishedFileSetChangeDescription( PublishedFileUpdateHandle_t updateHandle, const char *pchChangeDescription );

    STEAM_CALL_RESULT( RemoteStorageGetPublishedItemVoteDetailsResult_t )
    SteamAPICall_t GetPublishedItemVoteDetails( PublishedFileId_t unPublishedFileId );

    STEAM_CALL_RESULT( RemoteStorageUpdateUserPublishedItemVoteResult_t )
    SteamAPICall_t UpdateUserPublishedItemVote( PublishedFileId_t unPublishedFileId, bool bVoteUp );

    STEAM_CALL_RESULT( RemoteStorageGetPublishedItemVoteDetailsResult_t )
    SteamAPICall_t GetUserPublishedItemVoteDetails( PublishedFileId_t unPublishedFileId );

    STEAM_CALL_RESULT( RemoteStorageEnumerateUserPublishedFilesResult_t )
    SteamAPICall_t EnumerateUserSharedWorkshopFiles( CSteamID steamId, uint32 unStartIndex, SteamParamStringArray_t *pRequiredTags, SteamParamStringArray_t *pExcludedTags );

    STEAM_CALL_RESULT( RemoteStorageEnumerateUserPublishedFilesResult_t )
    SteamAPICall_t EnumerateUserSharedWorkshopFiles(AppId_t nAppId, CSteamID steamId, uint32 unStartIndex, SteamParamStringArray_t *pRequiredTags, SteamParamStringArray_t *pExcludedTags );

    STEAM_CALL_RESULT( RemoteStoragePublishFileProgress_t )
    SteamAPICall_t PublishVideo( EWorkshopVideoProvider eVideoProvider, const char *pchVideoAccount, const char *pchVideoIdentifier, const char *pchPreviewFile, AppId_t nConsumerAppId, const char *pchTitle, const char *pchDescription, ERemoteStoragePublishedFileVisibility eVisibility, SteamParamStringArray_t *pTags );

    STEAM_CALL_RESULT( RemoteStoragePublishFileProgress_t )
    SteamAPICall_t PublishVideo(const char *pchFileName, const char *pchPreviewFile, AppId_t nConsumerAppId, const char *pchTitle, const char *pchDescription, ERemoteStoragePublishedFileVisibility eVisibility, SteamParamStringArray_t *pTags );

    STEAM_CALL_RESULT( RemoteStorageSetUserPublishedFileActionResult_t )
    SteamAPICall_t SetUserPublishedFileAction( PublishedFileId_t unPublishedFileId, EWorkshopFileAction eAction );

    STEAM_CALL_RESULT( RemoteStorageEnumeratePublishedFilesByUserActionResult_t )
    SteamAPICall_t EnumeratePublishedFilesByUserAction( EWorkshopFileAction eAction, uint32 unStartIndex );

    // this method enumerates the public view of workshop files
    STEAM_CALL_RESULT( RemoteStorageEnumerateWorkshopFilesResult_t )
    SteamAPICall_t EnumeratePublishedWorkshopFiles( EWorkshopEnumerationType eEnumerationType, uint32 unStartIndex, uint32 unCount, uint32 unDays, SteamParamStringArray_t *pTags, SteamParamStringArray_t *pUserTags );


    STEAM_CALL_RESULT( RemoteStorageDownloadUGCResult_t )
    SteamAPICall_t UGCDownloadToLocation( UGCHandle_t hContent, const char *pchLocation, uint32 unPriority );

    // Cloud dynamic state change notification
    int32 GetLocalFileChangeCount();

    const char *GetLocalFileChange( int iFile, ERemoteStorageLocalFileChange *pEChangeType, ERemoteStorageFilePathType *pEFilePathType );

    // Indicate to Steam the beginning / end of a set of local file
    // operations - for example, writing a game save that requires updating two files.
    bool BeginFileWriteBatch();

    bool EndFileWriteBatch();


};

#endif // __INCLUDED_STEAM_REMOTE_STORAGE_H__
