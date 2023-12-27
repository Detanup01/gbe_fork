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

#include "base.h"

struct Async_Read {
	SteamAPICall_t api_call;
	uint32 offset;
	uint32 to_read;
	uint32 size;
	std::string file_name;
};

struct Stream_Write {
    std::string file_name;
    UGCFileWriteStreamHandle_t write_stream_handle;
    std::vector<char> file_data;
};

struct Downloaded_File {
    std::string file;
    uint64 total_size;
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
public ISteamRemoteStorage
{
private:
    class Settings *settings;
    Local_Storage *local_storage;
    class SteamCallResults *callback_results;
    bool steam_cloud_enabled;
    std::vector<struct Async_Read> async_reads;
    std::vector<struct Stream_Write> stream_writes;
    std::map<UGCHandle_t, std::string> shared_files;
    std::map<UGCHandle_t, struct Downloaded_File> downloaded_files;

    public:
Steam_Remote_Storage(class Settings *settings, Local_Storage *local_storage, class SteamCallResults *callback_results)
{
    this->settings = settings;
    this->local_storage = local_storage;
    this->callback_results = callback_results;
    steam_cloud_enabled = true;
    local_storage->update_save_filenames(Local_Storage::remote_storage_folder);
}

// NOTE
//
// Filenames are case-insensitive, and will be converted to lowercase automatically.
// So "foo.bar" and "Foo.bar" are the same file, and if you write "Foo.bar" then
// iterate the files, the filename returned will be "foo.bar".
//

// file operations
bool	FileWrite( const char *pchFile, const void *pvData, int32 cubData )
{
    PRINT_DEBUG("Steam_Remote_Storage::FileWrite %s %u\n", pchFile, cubData);
    if (!pchFile || cubData <= 0 || cubData > k_unMaxCloudFileChunkSize || !pvData) {
        return false;
    }

    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    int data_stored = local_storage->store_data(Local_Storage::remote_storage_folder, pchFile, (char* )pvData, cubData);
    PRINT_DEBUG("Steam_Remote_Storage::Stored %i, %u\n", data_stored, data_stored == cubData);
    return data_stored == cubData;
}

int32	FileRead( const char *pchFile, void *pvData, int32 cubDataToRead )
{
    PRINT_DEBUG("Steam_Remote_Storage::FileRead %s %i\n", pchFile, cubDataToRead);
    if (!pchFile || !pvData || !cubDataToRead) return 0;
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    int read_data = local_storage->get_data(Local_Storage::remote_storage_folder, pchFile, (char* )pvData, cubDataToRead);
    if (read_data < 0) read_data = 0;
    PRINT_DEBUG("Read %i\n", read_data);
    return read_data;
}

STEAM_CALL_RESULT( RemoteStorageFileWriteAsyncComplete_t )
SteamAPICall_t FileWriteAsync( const char *pchFile, const void *pvData, uint32 cubData )
{
    PRINT_DEBUG("Steam_Remote_Storage::FileWriteAsync\n");
    if (!pchFile || cubData > k_unMaxCloudFileChunkSize || cubData == 0 || !pvData) {
        return k_uAPICallInvalid;
    }

    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    bool success = local_storage->store_data(Local_Storage::remote_storage_folder, pchFile, (char* )pvData, cubData) == cubData;
    RemoteStorageFileWriteAsyncComplete_t data;
    data.m_eResult = success ? k_EResultOK : k_EResultFail;

    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data), 0.01);
}


STEAM_CALL_RESULT( RemoteStorageFileReadAsyncComplete_t )
SteamAPICall_t FileReadAsync( const char *pchFile, uint32 nOffset, uint32 cubToRead )
{
    PRINT_DEBUG("Steam_Remote_Storage::FileReadAsync\n");
    if (!pchFile) return k_uAPICallInvalid;
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    unsigned int size = local_storage->file_size(Local_Storage::remote_storage_folder, pchFile);

    RemoteStorageFileReadAsyncComplete_t data;
    if (size <= nOffset) {
	    return k_uAPICallInvalid;
    }

    if ((size - nOffset) < cubToRead) cubToRead = size - nOffset;

    struct Async_Read a_read;
    data.m_eResult = k_EResultOK;
    a_read.offset = data.m_nOffset = nOffset;
    a_read.api_call = data.m_hFileReadAsync = callback_results->reserveCallResult();
	a_read.to_read = data.m_cubRead = cubToRead;
    a_read.file_name = std::string(pchFile);
    a_read.size = size;

    async_reads.push_back(a_read);
    callback_results->addCallResult(data.m_hFileReadAsync, data.k_iCallback, &data, sizeof(data), 0.0);
    return data.m_hFileReadAsync;
}

bool	FileReadAsyncComplete( SteamAPICall_t hReadCall, void *pvBuffer, uint32 cubToRead )
{
    PRINT_DEBUG("Steam_Remote_Storage::FileReadAsyncComplete\n");
    if (!pvBuffer) return false;
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    auto a_read = std::find_if(async_reads.begin(), async_reads.end(), [&hReadCall](Async_Read const& item) { return item.api_call == hReadCall; });
    if (async_reads.end() == a_read)
        return false;

    if (cubToRead < a_read->to_read)
        return false;

    char *temp = new char[a_read->size];
    int read_data = local_storage->get_data(Local_Storage::remote_storage_folder, a_read->file_name, (char* )temp, a_read->size);
    if (read_data < a_read->to_read + a_read->offset) {
        delete[] temp;
        return false;
    }

    memcpy(pvBuffer, temp + a_read->offset, a_read->to_read);
    delete[] temp;
    async_reads.erase(a_read);
    return true;
}


bool	FileForget( const char *pchFile )
{
    PRINT_DEBUG("Steam_Remote_Storage::FileForget\n");
    return true;
}

bool	FileDelete( const char *pchFile )
{
    PRINT_DEBUG("Steam_Remote_Storage::FileDelete\n");
    return local_storage->file_delete(Local_Storage::remote_storage_folder, pchFile);
}

STEAM_CALL_RESULT( RemoteStorageFileShareResult_t )
SteamAPICall_t FileShare( const char *pchFile )
{
    PRINT_DEBUG("Steam_Remote_Storage::FileShare\n");
    if (!pchFile) return k_uAPICallInvalid;
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    RemoteStorageFileShareResult_t data = {};
    if (local_storage->file_exists(Local_Storage::remote_storage_folder, pchFile)) {
        data.m_eResult = k_EResultOK;
        data.m_hFile = generate_steam_api_call_id();
        strncpy(data.m_rgchFilename, pchFile, sizeof(data.m_rgchFilename) - 1);
        shared_files[data.m_hFile] = pchFile;
    } else {
        data.m_eResult = k_EResultFileNotFound;
    }

    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}

bool	SetSyncPlatforms( const char *pchFile, ERemoteStoragePlatform eRemoteStoragePlatform )
{
    PRINT_DEBUG("Steam_Remote_Storage::SetSyncPlatforms\n");
    return true;
}


// file operations that cause network IO
UGCFileWriteStreamHandle_t FileWriteStreamOpen( const char *pchFile )
{
    PRINT_DEBUG("Steam_Remote_Storage::FileWriteStreamOpen\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    static UGCFileWriteStreamHandle_t handle;
    ++handle;
    struct Stream_Write stream_write;
    stream_write.file_name = std::string(pchFile);
    stream_write.write_stream_handle = handle;
    stream_writes.push_back(stream_write);
    return stream_write.write_stream_handle;
}

bool FileWriteStreamWriteChunk( UGCFileWriteStreamHandle_t writeHandle, const void *pvData, int32 cubData )
{
    PRINT_DEBUG("Steam_Remote_Storage::FileWriteStreamWriteChunk\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    auto request = std::find_if(stream_writes.begin(), stream_writes.end(), [&writeHandle](struct Stream_Write const& item) { return item.write_stream_handle == writeHandle; });
    if (stream_writes.end() == request)
        return false;

    std::copy((char *)pvData, (char *)pvData + cubData, std::back_inserter(request->file_data));
    return true;
}

bool FileWriteStreamClose( UGCFileWriteStreamHandle_t writeHandle )
{
    PRINT_DEBUG("Steam_Remote_Storage::FileWriteStreamClose\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    auto request = std::find_if(stream_writes.begin(), stream_writes.end(), [&writeHandle](struct Stream_Write const& item) { return item.write_stream_handle == writeHandle; });
    if (stream_writes.end() == request)
        return false;

    local_storage->store_data(Local_Storage::remote_storage_folder, request->file_name, request->file_data.data(), request->file_data.size());
    stream_writes.erase(request);
    return true;
}

bool FileWriteStreamCancel( UGCFileWriteStreamHandle_t writeHandle )
{
    PRINT_DEBUG("Steam_Remote_Storage::FileWriteStreamCancel\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    auto request = std::find_if(stream_writes.begin(), stream_writes.end(), [&writeHandle](struct Stream_Write const& item) { return item.write_stream_handle == writeHandle; });
    if (stream_writes.end() == request)
        return false;

    stream_writes.erase(request);
    return true;
}

// file information
bool	FileExists( const char *pchFile )
{
    PRINT_DEBUG("Steam_Remote_Storage::FileExists %s\n", pchFile);
    return local_storage->file_exists(Local_Storage::remote_storage_folder, pchFile);
}

bool	FilePersisted( const char *pchFile )
{
    PRINT_DEBUG("Steam_Remote_Storage::FilePersisted\n");
    return local_storage->file_exists(Local_Storage::remote_storage_folder, pchFile);
}

int32	GetFileSize( const char *pchFile )
{
    PRINT_DEBUG("Steam_Remote_Storage::GetFileSize %s\n", pchFile);
    return local_storage->file_size(Local_Storage::remote_storage_folder, pchFile);
}

int64	GetFileTimestamp( const char *pchFile )
{
    PRINT_DEBUG("Steam_Remote_Storage::GetFileTimestamp\n");
    return local_storage->file_timestamp(Local_Storage::remote_storage_folder, pchFile);
}

ERemoteStoragePlatform GetSyncPlatforms( const char *pchFile )
{
    PRINT_DEBUG("Steam_Remote_Storage::GetSyncPlatforms\n");
    return k_ERemoteStoragePlatformAll;
}


// iteration
int32 GetFileCount()
{
    PRINT_DEBUG("Steam_Remote_Storage::GetFileCount\n");
    int32 num = local_storage->count_files(Local_Storage::remote_storage_folder);
    PRINT_DEBUG("Steam_Remote_Storage::File count: %i\n", num);
    return num;
}

const char *GetFileNameAndSize( int iFile, int32 *pnFileSizeInBytes )
{
    PRINT_DEBUG("Steam_Remote_Storage::GetFileNameAndSize %i\n", iFile);
    static char output_filename[MAX_FILENAME_LENGTH];
    if (local_storage->iterate_file(Local_Storage::remote_storage_folder, iFile, output_filename, pnFileSizeInBytes)) {
        PRINT_DEBUG("Steam_Remote_Storage::Name: |%s|, size: %i\n", output_filename, pnFileSizeInBytes ? *pnFileSizeInBytes : 0);
        return output_filename;
    } else {
        return "";
    }
}


// configuration management
bool GetQuota( uint64 *pnTotalBytes, uint64 *puAvailableBytes )
{
    PRINT_DEBUG("Steam_Remote_Storage::GetQuota\n");
    uint64 quota = 2 << 26;
    *pnTotalBytes = quota;
    *puAvailableBytes = (quota);
    return true;
}

bool GetQuota( int32 *pnTotalBytes, int32 *puAvailableBytes )
{
    PRINT_DEBUG("Steam_Remote_Storage::GetQuota\n");
    uint64 quota = 2 << 26;
    *pnTotalBytes = quota;
    *puAvailableBytes = (quota);
    return true;
}

bool IsCloudEnabledForAccount()
{
    PRINT_DEBUG("Steam_Remote_Storage::IsCloudEnabledForAccount\n");
    return true;
}

bool IsCloudEnabledForApp()
{
    PRINT_DEBUG("Steam_Remote_Storage::IsCloudEnabledForApp\n");
    return steam_cloud_enabled;
}

bool IsCloudEnabledThisApp()
{
    PRINT_DEBUG("Steam_Remote_Storage::IsCloudEnabledThisApp\n");
    return steam_cloud_enabled;
}

void SetCloudEnabledForApp( bool bEnabled )
{
    PRINT_DEBUG("Steam_Remote_Storage::SetCloudEnabledForApp\n");
    steam_cloud_enabled = bEnabled;
}

bool SetCloudEnabledThisApp( bool bEnabled )
{
    PRINT_DEBUG("Steam_Remote_Storage::SetCloudEnabledThisApp\n");
    steam_cloud_enabled = bEnabled;
    return true;
}

// user generated content

// Downloads a UGC file.  A priority value of 0 will download the file immediately,
// otherwise it will wait to download the file until all downloads with a lower priority
// value are completed.  Downloads with equal priority will occur simultaneously.
STEAM_CALL_RESULT( RemoteStorageDownloadUGCResult_t )
SteamAPICall_t UGCDownload( UGCHandle_t hContent, uint32 unPriority )
{
    PRINT_DEBUG("Steam_Remote_Storage::UGCDownload\n");
    RemoteStorageDownloadUGCResult_t data = {};
    if (shared_files.count(hContent)) {
        data.m_eResult = k_EResultOK;
        data.m_hFile = hContent;
        data.m_nAppID = settings->get_local_game_id().AppID();
        data.m_nSizeInBytes = local_storage->file_size(Local_Storage::remote_storage_folder, shared_files[hContent]);
        shared_files[hContent].copy(data.m_pchFileName, sizeof(data.m_pchFileName) - 1);
        data.m_ulSteamIDOwner = settings->get_local_steam_id().ConvertToUint64();
        downloaded_files[hContent].file = shared_files[hContent];
        downloaded_files[hContent].total_size = data.m_nSizeInBytes;
    } else {
        data.m_eResult = k_EResultFileNotFound; //TODO: not sure if this is the right result
    }

    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}

STEAM_CALL_RESULT( RemoteStorageDownloadUGCResult_t )
SteamAPICall_t UGCDownload( UGCHandle_t hContent )
{
    PRINT_DEBUG("Steam_Remote_Storage::UGCDownload old\n");
    return UGCDownload(hContent, 1);
}


// Gets the amount of data downloaded so far for a piece of content. pnBytesExpected can be 0 if function returns false
// or if the transfer hasn't started yet, so be careful to check for that before dividing to get a percentage
bool	GetUGCDownloadProgress( UGCHandle_t hContent, int32 *pnBytesDownloaded, int32 *pnBytesExpected )
{
    PRINT_DEBUG("Steam_Remote_Storage::GetUGCDownloadProgress\n");
    return false;
}

bool	GetUGCDownloadProgress( UGCHandle_t hContent, uint32 *pnBytesDownloaded, uint32 *pnBytesExpected )
{
    PRINT_DEBUG("Steam_Remote_Storage::GetUGCDownloadProgress old\n");
    return false;
}


// Gets metadata for a file after it has been downloaded. This is the same metadata given in the RemoteStorageDownloadUGCResult_t call result
bool	GetUGCDetails( UGCHandle_t hContent, AppId_t *pnAppID, STEAM_OUT_STRING() char **ppchName, int32 *pnFileSizeInBytes, STEAM_OUT_STRUCT() CSteamID *pSteamIDOwner )
{
    PRINT_DEBUG("Steam_Remote_Storage::GetUGCDetails\n");
    return false;
}


// After download, gets the content of the file.  
// Small files can be read all at once by calling this function with an offset of 0 and cubDataToRead equal to the size of the file.
// Larger files can be read in chunks to reduce memory usage (since both sides of the IPC client and the game itself must allocate
// enough memory for each chunk).  Once the last byte is read, the file is implicitly closed and further calls to UGCRead will fail
// unless UGCDownload is called again.
// For especially large files (anything over 100MB) it is a requirement that the file is read in chunks.
int32	UGCRead( UGCHandle_t hContent, void *pvData, int32 cubDataToRead, uint32 cOffset, EUGCReadAction eAction )
{
    PRINT_DEBUG("Steam_Remote_Storage::UGCRead\n");
    if (!downloaded_files.count(hContent) || cubDataToRead < 0) {
        return -1; //TODO: is this the right return value?
    }

    Downloaded_File f = downloaded_files[hContent];
    int read_data = local_storage->get_data(Local_Storage::remote_storage_folder, f.file, (char* )pvData, cubDataToRead, cOffset);

    if (eAction == k_EUGCRead_Close || (eAction == k_EUGCRead_ContinueReadingUntilFinished && (read_data < cubDataToRead || (cOffset + cubDataToRead) >= f.total_size))) {
        downloaded_files.erase(hContent);
    }

    PRINT_DEBUG("Read %i\n", read_data);
    return read_data;
}

int32	UGCRead( UGCHandle_t hContent, void *pvData, int32 cubDataToRead )
{
    PRINT_DEBUG("Steam_Remote_Storage::UGCRead old\n");
    return UGCRead( hContent, pvData, cubDataToRead, 0);
}

int32	UGCRead( UGCHandle_t hContent, void *pvData, int32 cubDataToRead, uint32 cOffset)
{
    PRINT_DEBUG("Steam_Remote_Storage::UGCRead old2\n");
    return UGCRead(hContent, pvData, cubDataToRead, cOffset, k_EUGCRead_ContinueReadingUntilFinished);
}

// Functions to iterate through UGC that has finished downloading but has not yet been read via UGCRead()
int32	GetCachedUGCCount()
{
    PRINT_DEBUG("Steam_Remote_Storage::GetCachedUGCCount\n");
    return 0;
}

UGCHandle_t GetCachedUGCHandle( int32 iCachedContent )
{
    PRINT_DEBUG("Steam_Remote_Storage::GetCachedUGCHandle\n");
    return k_UGCHandleInvalid;
}


// The following functions are only necessary on the Playstation 3. On PC & Mac, the Steam client will handle these operations for you
// On Playstation 3, the game controls which files are stored in the cloud, via FilePersist, FileFetch, and FileForget.
    
#if defined(_PS3) || defined(_SERVER)
// Connect to Steam and get a list of files in the Cloud - results in a RemoteStorageAppSyncStatusCheck_t callback
void GetFileListFromServer()
{
    PRINT_DEBUG("Steam_Remote_Storage::GetFileListFromServer\n");
}

// Indicate this file should be downloaded in the next sync
bool FileFetch( const char *pchFile )
{
    PRINT_DEBUG("Steam_Remote_Storage::FileFetch\n");
    return true;
}

// Indicate this file should be persisted in the next sync
bool FilePersist( const char *pchFile )
{
    PRINT_DEBUG("Steam_Remote_Storage::FilePersist\n");
    return true;
}

// Pull any requested files down from the Cloud - results in a RemoteStorageAppSyncedClient_t callback
bool SynchronizeToClient()
{
    PRINT_DEBUG("Steam_Remote_Storage::SynchronizeToClient\n");
}

// Upload any requested files to the Cloud - results in a RemoteStorageAppSyncedServer_t callback
bool SynchronizeToServer()
{
    PRINT_DEBUG("Steam_Remote_Storage::SynchronizeToServer\n");
}

// Reset any fetch/persist/etc requests
bool ResetFileRequestState()
{
    PRINT_DEBUG("Steam_Remote_Storage::ResetFileRequestState\n");
}

#endif

// publishing UGC
STEAM_CALL_RESULT( RemoteStoragePublishFileProgress_t )
SteamAPICall_t	PublishWorkshopFile( const char *pchFile, const char *pchPreviewFile, AppId_t nConsumerAppId, const char *pchTitle, const char *pchDescription, ERemoteStoragePublishedFileVisibility eVisibility, SteamParamStringArray_t *pTags, EWorkshopFileType eWorkshopFileType )
{
    PRINT_DEBUG("Steam_Remote_Storage::PublishWorkshopFile\n");
    return 0;
}

PublishedFileUpdateHandle_t CreatePublishedFileUpdateRequest( PublishedFileId_t unPublishedFileId )
{
    PRINT_DEBUG("Steam_Remote_Storage::CreatePublishedFileUpdateRequest\n");
    return 0;
}

bool UpdatePublishedFileFile( PublishedFileUpdateHandle_t updateHandle, const char *pchFile )
{
    PRINT_DEBUG("Steam_Remote_Storage::UpdatePublishedFileFile\n");
    return false;
}

SteamAPICall_t	PublishFile( const char *pchFile, const char *pchPreviewFile, AppId_t nConsumerAppId, const char *pchTitle, const char *pchDescription, ERemoteStoragePublishedFileVisibility eVisibility, SteamParamStringArray_t *pTags )
{
    PRINT_DEBUG("Steam_Remote_Storage::PublishFile\n");
    return 0;
}

SteamAPICall_t	PublishWorkshopFile( const char *pchFile, const char *pchPreviewFile, AppId_t nConsumerAppId, const char *pchTitle, const char *pchDescription, SteamParamStringArray_t *pTags )
{
    PRINT_DEBUG("Steam_Remote_Storage::PublishWorkshopFile old\n");
    return 0;
}

SteamAPICall_t	UpdatePublishedFile( RemoteStorageUpdatePublishedFileRequest_t updatePublishedFileRequest )
{
    PRINT_DEBUG("Steam_Remote_Storage::UpdatePublishedFile\n");
    return 0;
}

bool UpdatePublishedFilePreviewFile( PublishedFileUpdateHandle_t updateHandle, const char *pchPreviewFile )
{
    PRINT_DEBUG("Steam_Remote_Storage::UpdatePublishedFilePreviewFile\n");
    return false;
}

bool UpdatePublishedFileTitle( PublishedFileUpdateHandle_t updateHandle, const char *pchTitle )
{
    PRINT_DEBUG("Steam_Remote_Storage::UpdatePublishedFileTitle\n");
    return false;
}

bool UpdatePublishedFileDescription( PublishedFileUpdateHandle_t updateHandle, const char *pchDescription )
{
    PRINT_DEBUG("Steam_Remote_Storage::UpdatePublishedFileDescription\n");
    return false;
}

bool UpdatePublishedFileVisibility( PublishedFileUpdateHandle_t updateHandle, ERemoteStoragePublishedFileVisibility eVisibility )
{
    PRINT_DEBUG("Steam_Remote_Storage::UpdatePublishedFileVisibility\n");
    return false;
}

bool UpdatePublishedFileTags( PublishedFileUpdateHandle_t updateHandle, SteamParamStringArray_t *pTags )
{
    PRINT_DEBUG("Steam_Remote_Storage::UpdatePublishedFileTags\n");
    return false;
}

STEAM_CALL_RESULT( RemoteStorageUpdatePublishedFileResult_t )
SteamAPICall_t	CommitPublishedFileUpdate( PublishedFileUpdateHandle_t updateHandle )
{
    PRINT_DEBUG("Steam_Remote_Storage::CommitPublishedFileUpdate\n");
    return 0;
}

// Gets published file details for the given publishedfileid.  If unMaxSecondsOld is greater than 0,
// cached data may be returned, depending on how long ago it was cached.  A value of 0 will force a refresh.
// A value of k_WorkshopForceLoadPublishedFileDetailsFromCache will use cached data if it exists, no matter how old it is.
STEAM_CALL_RESULT( RemoteStorageGetPublishedFileDetailsResult_t )
SteamAPICall_t	GetPublishedFileDetails( PublishedFileId_t unPublishedFileId, uint32 unMaxSecondsOld )
{
    PRINT_DEBUG("Steam_Remote_Storage::GetPublishedFileDetails %llu\n", unPublishedFileId);
    //TODO: check what this function really returns
    return 0;
/*
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    RemoteStorageGetPublishedFileDetailsResult_t data = {};
    data.m_eResult = k_EResultFail;
    data.m_nPublishedFileId = unPublishedFileId;
    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
*/
}

STEAM_CALL_RESULT( RemoteStorageGetPublishedFileDetailsResult_t )
SteamAPICall_t	GetPublishedFileDetails( PublishedFileId_t unPublishedFileId )
{
    PRINT_DEBUG("Steam_Remote_Storage::GetPublishedFileDetails old\n");
    return GetPublishedFileDetails(unPublishedFileId, 0);
}

STEAM_CALL_RESULT( RemoteStorageDeletePublishedFileResult_t )
SteamAPICall_t	DeletePublishedFile( PublishedFileId_t unPublishedFileId )
{
    PRINT_DEBUG("Steam_Remote_Storage::DeletePublishedFile\n");
    return 0;
}

// enumerate the files that the current user published with this app
STEAM_CALL_RESULT( RemoteStorageEnumerateUserPublishedFilesResult_t )
SteamAPICall_t	EnumerateUserPublishedFiles( uint32 unStartIndex )
{
    PRINT_DEBUG("Steam_Remote_Storage::EnumerateUserPublishedFiles\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    RemoteStorageEnumerateUserPublishedFilesResult_t data;
    data.m_eResult = k_EResultOK;
    data.m_nResultsReturned = 0;
    data.m_nTotalResultCount = 0;
    //data.m_rgPublishedFileId;
    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}

STEAM_CALL_RESULT( RemoteStorageSubscribePublishedFileResult_t )
SteamAPICall_t	SubscribePublishedFile( PublishedFileId_t unPublishedFileId )
{
    PRINT_DEBUG("Steam_Remote_Storage::SubscribePublishedFile\n");
    return 0;
}

STEAM_CALL_RESULT( RemoteStorageEnumerateUserSubscribedFilesResult_t )
SteamAPICall_t	EnumerateUserSubscribedFiles( uint32 unStartIndex )
{
    PRINT_DEBUG("Steam_Remote_Storage::EnumerateUserSubscribedFiles\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    RemoteStorageEnumerateUserSubscribedFilesResult_t data;
    data.m_eResult = k_EResultOK;
    data.m_nResultsReturned = 0;
    data.m_nTotalResultCount = 0;
    //data.m_rgPublishedFileId;
    //data.m_rgRTimeSubscribed;
    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}

STEAM_CALL_RESULT( RemoteStorageUnsubscribePublishedFileResult_t )
SteamAPICall_t	UnsubscribePublishedFile( PublishedFileId_t unPublishedFileId )
{
    PRINT_DEBUG("Steam_Remote_Storage::UnsubscribePublishedFile\n");
    return 0;
}

bool UpdatePublishedFileSetChangeDescription( PublishedFileUpdateHandle_t updateHandle, const char *pchChangeDescription )
{
    PRINT_DEBUG("Steam_Remote_Storage::UpdatePublishedFileSetChangeDescription\n");
    return false;
}

STEAM_CALL_RESULT( RemoteStorageGetPublishedItemVoteDetailsResult_t )
SteamAPICall_t	GetPublishedItemVoteDetails( PublishedFileId_t unPublishedFileId )
{
    PRINT_DEBUG("Steam_Remote_Storage::GetPublishedItemVoteDetails\n");
    return 0;
}

STEAM_CALL_RESULT( RemoteStorageUpdateUserPublishedItemVoteResult_t )
SteamAPICall_t	UpdateUserPublishedItemVote( PublishedFileId_t unPublishedFileId, bool bVoteUp )
{
    PRINT_DEBUG("Steam_Remote_Storage::UpdateUserPublishedItemVote\n");
    return 0;
}

STEAM_CALL_RESULT( RemoteStorageGetPublishedItemVoteDetailsResult_t )
SteamAPICall_t	GetUserPublishedItemVoteDetails( PublishedFileId_t unPublishedFileId )
{
    PRINT_DEBUG("Steam_Remote_Storage::GetUserPublishedItemVoteDetails\n");
    return 0;
}

STEAM_CALL_RESULT( RemoteStorageEnumerateUserPublishedFilesResult_t )
SteamAPICall_t	EnumerateUserSharedWorkshopFiles( CSteamID steamId, uint32 unStartIndex, SteamParamStringArray_t *pRequiredTags, SteamParamStringArray_t *pExcludedTags )
{
    PRINT_DEBUG("Steam_Remote_Storage::EnumerateUserSharedWorkshopFiles\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    RemoteStorageEnumerateUserPublishedFilesResult_t data;
    data.m_eResult = k_EResultOK;
    data.m_nResultsReturned = 0;
    data.m_nTotalResultCount = 0;
    //data.m_rgPublishedFileId;
    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}

STEAM_CALL_RESULT( RemoteStorageEnumerateUserPublishedFilesResult_t )
SteamAPICall_t	EnumerateUserSharedWorkshopFiles(AppId_t nAppId, CSteamID steamId, uint32 unStartIndex, SteamParamStringArray_t *pRequiredTags, SteamParamStringArray_t *pExcludedTags )
{
    PRINT_DEBUG("Steam_Remote_Storage::EnumerateUserSharedWorkshopFiles old\n");
    return EnumerateUserSharedWorkshopFiles(steamId, unStartIndex, pRequiredTags, pExcludedTags);
}

STEAM_CALL_RESULT( RemoteStoragePublishFileProgress_t )
SteamAPICall_t	PublishVideo( EWorkshopVideoProvider eVideoProvider, const char *pchVideoAccount, const char *pchVideoIdentifier, const char *pchPreviewFile, AppId_t nConsumerAppId, const char *pchTitle, const char *pchDescription, ERemoteStoragePublishedFileVisibility eVisibility, SteamParamStringArray_t *pTags )
{
    PRINT_DEBUG("Steam_Remote_Storage::PublishVideo\n");
    return 0;
}

STEAM_CALL_RESULT( RemoteStoragePublishFileProgress_t )
SteamAPICall_t	PublishVideo(const char *pchFileName, const char *pchPreviewFile, AppId_t nConsumerAppId, const char *pchTitle, const char *pchDescription, ERemoteStoragePublishedFileVisibility eVisibility, SteamParamStringArray_t *pTags )
{
    PRINT_DEBUG("Steam_Remote_Storage::PublishVideo old\n");
    return 0;
}

STEAM_CALL_RESULT( RemoteStorageSetUserPublishedFileActionResult_t )
SteamAPICall_t	SetUserPublishedFileAction( PublishedFileId_t unPublishedFileId, EWorkshopFileAction eAction )
{
    PRINT_DEBUG("Steam_Remote_Storage::SetUserPublishedFileAction\n");
    return 0;
}

STEAM_CALL_RESULT( RemoteStorageEnumeratePublishedFilesByUserActionResult_t )
SteamAPICall_t	EnumeratePublishedFilesByUserAction( EWorkshopFileAction eAction, uint32 unStartIndex )
{
    PRINT_DEBUG("Steam_Remote_Storage::EnumeratePublishedFilesByUserAction\n");
    return 0;
}

// this method enumerates the public view of workshop files
STEAM_CALL_RESULT( RemoteStorageEnumerateWorkshopFilesResult_t )
SteamAPICall_t	EnumeratePublishedWorkshopFiles( EWorkshopEnumerationType eEnumerationType, uint32 unStartIndex, uint32 unCount, uint32 unDays, SteamParamStringArray_t *pTags, SteamParamStringArray_t *pUserTags )
{
    PRINT_DEBUG("Steam_Remote_Storage::EnumeratePublishedWorkshopFiles\n");
    return 0;
}


STEAM_CALL_RESULT( RemoteStorageDownloadUGCResult_t )
SteamAPICall_t UGCDownloadToLocation( UGCHandle_t hContent, const char *pchLocation, uint32 unPriority )
{
    PRINT_DEBUG("Steam_Remote_Storage::UGCDownloadToLocation\n");
    return 0;
}

// Cloud dynamic state change notification
int32 GetLocalFileChangeCount()
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
    return 0;
}

const char *GetLocalFileChange( int iFile, ERemoteStorageLocalFileChange *pEChangeType, ERemoteStorageFilePathType *pEFilePathType )
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
    return "";
}

// Indicate to Steam the beginning / end of a set of local file
// operations - for example, writing a game save that requires updating two files.
bool BeginFileWriteBatch()
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
    return true;
}

bool EndFileWriteBatch()
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
    return true;
}


};
