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
#include "dll/ugc_remote_storage_bridge.h"

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
    enum DownloadSource {
        AfterFileShare, // attempted download after a call to Steam_Remote_Storage::FileShare()
        AfterSendQueryUGCRequest, // attempted download after a call to Steam_UGC::SendQueryUGCRequest()
        FromUGCDownloadToLocation, // attempted download via Steam_Remote_Storage::UGCDownloadToLocation()
    } source;

    // *** used in any case
    std::string file;
    uint64 total_size;

    // put any additional data needed by other sources here
    
    // *** used when source = SendQueryUGCRequest only
    Ugc_Remote_Storage_Bridge::QueryInfo mod_query_info;

    // *** used when source = FromUGCDownloadToLocation only
    std::string download_to_location_fullpath;
};

static void copy_file(const std::string &src_filepath, const std::string &dst_filepath)
{
    try
    {
        PRINT_DEBUG("Steam_Remote_Storage::copy_file copying file '%s' to '%s'", src_filepath.c_str(), dst_filepath.c_str());
        const auto src_p = std::filesystem::path(src_filepath);
        if (!std::filesystem::exists(src_p) || std::filesystem::is_directory(src_p)) return;
        
        const auto dst_p = std::filesystem::path(dst_filepath);
        std::filesystem::create_directories(dst_p.parent_path()); // make the folder tree if needed
        std::filesystem::copy_file(src_p, dst_p, std::filesystem::copy_options::overwrite_existing);
    } catch(...) {}
}

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
    class Ugc_Remote_Storage_Bridge *ugc_bridge;
    class Local_Storage *local_storage;
    class SteamCallResults *callback_results;
    bool steam_cloud_enabled;
    std::vector<struct Async_Read> async_reads;
    std::vector<struct Stream_Write> stream_writes;
    std::map<UGCHandle_t, std::string> shared_files;
    std::map<UGCHandle_t, struct Downloaded_File> downloaded_files;

    public:
Steam_Remote_Storage(class Settings *settings, class Ugc_Remote_Storage_Bridge *ugc_bridge, class Local_Storage *local_storage, class SteamCallResults *callback_results)
{
    this->settings = settings;
    this->ugc_bridge = ugc_bridge;
    this->local_storage = local_storage;
    this->callback_results = callback_results;
    steam_cloud_enabled = true;
}

// NOTE
//
// Filenames are case-insensitive, and will be converted to lowercase automatically.
// So "foo.bar" and "Foo.bar" are the same file, and if you write "Foo.bar" then
// iterate the files, the filename returned will be "foo.bar".
//

// file operations
bool FileWrite( const char *pchFile, const void *pvData, int32 cubData )
{
    PRINT_DEBUG("Steam_Remote_Storage::FileWrite '%s' %p %u\n", pchFile, pvData, cubData);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pchFile || !pchFile[0] || cubData <= 0 || cubData > k_unMaxCloudFileChunkSize || !pvData) {
        return false;
    }

    int data_stored = local_storage->store_data(Local_Storage::remote_storage_folder, pchFile, (char* )pvData, cubData);
    PRINT_DEBUG("Steam_Remote_Storage::Stored %i, %u\n", data_stored, data_stored == cubData);
    return data_stored == cubData;
}

int32 FileRead( const char *pchFile, void *pvData, int32 cubDataToRead )
{
    PRINT_DEBUG("Steam_Remote_Storage::FileRead '%s' %p %i\n", pchFile, pvData, cubDataToRead);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pchFile || !pchFile[0] || !pvData || !cubDataToRead) return 0;
    int read_data = local_storage->get_data(Local_Storage::remote_storage_folder, pchFile, (char* )pvData, cubDataToRead);
    if (read_data < 0) read_data = 0;
    PRINT_DEBUG("  Read %i\n", read_data);
    return read_data;
}

STEAM_CALL_RESULT( RemoteStorageFileWriteAsyncComplete_t )
SteamAPICall_t FileWriteAsync( const char *pchFile, const void *pvData, uint32 cubData )
{
    PRINT_DEBUG("Steam_Remote_Storage::FileWriteAsync '%s' %p %u\n", pchFile, pvData, cubData);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pchFile || !pchFile[0] || cubData > k_unMaxCloudFileChunkSize || cubData == 0 || !pvData) {
        return k_uAPICallInvalid;
    }

    bool success = local_storage->store_data(Local_Storage::remote_storage_folder, pchFile, (char* )pvData, cubData) == cubData;
    RemoteStorageFileWriteAsyncComplete_t data;
    data.m_eResult = success ? k_EResultOK : k_EResultFail;

    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data), 0.01);
}


STEAM_CALL_RESULT( RemoteStorageFileReadAsyncComplete_t )
SteamAPICall_t FileReadAsync( const char *pchFile, uint32 nOffset, uint32 cubToRead )
{
    PRINT_DEBUG("Steam_Remote_Storage::FileReadAsync '%s' %u %u\n", pchFile, nOffset, cubToRead);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pchFile || !pchFile[0]) return k_uAPICallInvalid;
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

bool FileReadAsyncComplete( SteamAPICall_t hReadCall, void *pvBuffer, uint32 cubToRead )
{
    PRINT_DEBUG("Steam_Remote_Storage::FileReadAsyncComplete\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!pvBuffer) return false;

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


bool FileForget( const char *pchFile )
{
    PRINT_DEBUG("Steam_Remote_Storage::FileForget\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!pchFile || !pchFile[0]) return false;

    return true;
}

bool FileDelete( const char *pchFile )
{
    PRINT_DEBUG("Steam_Remote_Storage::FileDelete\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!pchFile || !pchFile[0]) return false;
    
    return local_storage->file_delete(Local_Storage::remote_storage_folder, pchFile);
}

STEAM_CALL_RESULT( RemoteStorageFileShareResult_t )
SteamAPICall_t FileShare( const char *pchFile )
{
    PRINT_DEBUG("Steam_Remote_Storage::FileShare\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!pchFile || !pchFile[0]) return k_uAPICallInvalid;

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

bool SetSyncPlatforms( const char *pchFile, ERemoteStoragePlatform eRemoteStoragePlatform )
{
    PRINT_DEBUG("Steam_Remote_Storage::SetSyncPlatforms\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!pchFile || !pchFile[0]) return false;
    
    return true;
}


// file operations that cause network IO
UGCFileWriteStreamHandle_t FileWriteStreamOpen( const char *pchFile )
{
    PRINT_DEBUG("Steam_Remote_Storage::FileWriteStreamOpen\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!pchFile || !pchFile[0]) return k_UGCFileStreamHandleInvalid;
    
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
    if (!pvData || cubData < 0) return false;

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
bool FileExists( const char *pchFile )
{
    PRINT_DEBUG("Steam_Remote_Storage::FileExists %s\n", pchFile);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!pchFile || !pchFile[0]) return false;
    
    return local_storage->file_exists(Local_Storage::remote_storage_folder, pchFile);
}

bool FilePersisted( const char *pchFile )
{
    PRINT_DEBUG("Steam_Remote_Storage::FilePersisted\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!pchFile || !pchFile[0]) return false;
    
    return local_storage->file_exists(Local_Storage::remote_storage_folder, pchFile);
}

int32 GetFileSize( const char *pchFile )
{
    PRINT_DEBUG("Steam_Remote_Storage::GetFileSize %s\n", pchFile);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!pchFile || !pchFile[0]) return 0;
    
    return local_storage->file_size(Local_Storage::remote_storage_folder, pchFile);
}

int64 GetFileTimestamp( const char *pchFile )
{
    PRINT_DEBUG("Steam_Remote_Storage::GetFileTimestamp\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!pchFile || !pchFile[0]) return 0;
    
    return local_storage->file_timestamp(Local_Storage::remote_storage_folder, pchFile);
}

ERemoteStoragePlatform GetSyncPlatforms( const char *pchFile )
{
    PRINT_DEBUG("Steam_Remote_Storage::GetSyncPlatforms\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return k_ERemoteStoragePlatformAll;
}


// iteration
int32 GetFileCount()
{
    PRINT_DEBUG("Steam_Remote_Storage::GetFileCount\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    int32 num = local_storage->count_files(Local_Storage::remote_storage_folder);
    PRINT_DEBUG("Steam_Remote_Storage::File count: %i\n", num);
    return num;
}

const char *GetFileNameAndSize( int iFile, int32 *pnFileSizeInBytes )
{
    PRINT_DEBUG("Steam_Remote_Storage::GetFileNameAndSize %i\n", iFile);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
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
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    uint64 quota = 2 << 26;
    if (pnTotalBytes) *pnTotalBytes = quota;
    if (puAvailableBytes) *puAvailableBytes = (quota);
    return true;
}

bool GetQuota( int32 *pnTotalBytes, int32 *puAvailableBytes )
{
    PRINT_DEBUG("Steam_Remote_Storage::GetQuota\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    uint64 quota = 2 << 26;
    if (pnTotalBytes) *pnTotalBytes = quota;
    if (puAvailableBytes) *puAvailableBytes = (quota);
    return true;
}

bool IsCloudEnabledForAccount()
{
    PRINT_DEBUG("Steam_Remote_Storage::IsCloudEnabledForAccount\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return true;
}

bool IsCloudEnabledForApp()
{
    PRINT_DEBUG("Steam_Remote_Storage::IsCloudEnabledForApp\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return steam_cloud_enabled;
}

bool IsCloudEnabledThisApp()
{
    PRINT_DEBUG("Steam_Remote_Storage::IsCloudEnabledThisApp\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return steam_cloud_enabled;
}

void SetCloudEnabledForApp( bool bEnabled )
{
    PRINT_DEBUG("Steam_Remote_Storage::SetCloudEnabledForApp\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    steam_cloud_enabled = bEnabled;
}

bool SetCloudEnabledThisApp( bool bEnabled )
{
    PRINT_DEBUG("Steam_Remote_Storage::SetCloudEnabledThisApp\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
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
    PRINT_DEBUG("Steam_Remote_Storage::UGCDownload %llu\n", hContent);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (hContent == k_UGCHandleInvalid) return k_uAPICallInvalid;

    RemoteStorageDownloadUGCResult_t data{};
    data.m_hFile = hContent;

    if (shared_files.count(hContent)) {
        data.m_eResult = k_EResultOK;
        data.m_nAppID = settings->get_local_game_id().AppID();
        data.m_ulSteamIDOwner = settings->get_local_steam_id().ConvertToUint64();
        data.m_nSizeInBytes = local_storage->file_size(Local_Storage::remote_storage_folder, shared_files[hContent]);

        shared_files[hContent].copy(data.m_pchFileName, sizeof(data.m_pchFileName) - 1);

        downloaded_files[hContent].source = Downloaded_File::DownloadSource::AfterFileShare;
        downloaded_files[hContent].file = shared_files[hContent];
        downloaded_files[hContent].total_size = data.m_nSizeInBytes;
    } else if (auto query_res = ugc_bridge->get_ugc_query_result(hContent)) {
        auto mod = settings->getMod(query_res.value().mod_id);
        auto &mod_name = query_res.value().is_primary_file
            ? mod.primaryFileName
            : mod.previewFileName;
        int32 mod_size = query_res.value().is_primary_file
            ? mod.primaryFileSize
            : mod.previewFileSize;

        data.m_eResult = k_EResultOK;
        data.m_nAppID = settings->get_local_game_id().AppID();
        data.m_ulSteamIDOwner = mod.steamIDOwner;
        data.m_nSizeInBytes = mod_size;
        data.m_ulSteamIDOwner = mod.steamIDOwner;

        mod_name.copy(data.m_pchFileName, sizeof(data.m_pchFileName) - 1);
        
        downloaded_files[hContent].source = Downloaded_File::DownloadSource::AfterSendQueryUGCRequest;
        downloaded_files[hContent].file = mod_name;
        downloaded_files[hContent].total_size = mod_size;
        
        downloaded_files[hContent].mod_query_info = query_res.value();
        
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
bool GetUGCDownloadProgress( UGCHandle_t hContent, int32 *pnBytesDownloaded, int32 *pnBytesExpected )
{
    PRINT_DEBUG("Steam_Remote_Storage::GetUGCDownloadProgress\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}

bool GetUGCDownloadProgress( UGCHandle_t hContent, uint32 *pnBytesDownloaded, uint32 *pnBytesExpected )
{
    PRINT_DEBUG("Steam_Remote_Storage::GetUGCDownloadProgress old\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}


// Gets metadata for a file after it has been downloaded. This is the same metadata given in the RemoteStorageDownloadUGCResult_t call result
bool GetUGCDetails( UGCHandle_t hContent, AppId_t *pnAppID, STEAM_OUT_STRING() char **ppchName, int32 *pnFileSizeInBytes, STEAM_OUT_STRUCT() CSteamID *pSteamIDOwner )
{
    PRINT_DEBUG("Steam_Remote_Storage::GetUGCDetails\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}


// After download, gets the content of the file.  
// Small files can be read all at once by calling this function with an offset of 0 and cubDataToRead equal to the size of the file.
// Larger files can be read in chunks to reduce memory usage (since both sides of the IPC client and the game itself must allocate
// enough memory for each chunk).  Once the last byte is read, the file is implicitly closed and further calls to UGCRead will fail
// unless UGCDownload is called again.
// For especially large files (anything over 100MB) it is a requirement that the file is read in chunks.
int32 UGCRead( UGCHandle_t hContent, void *pvData, int32 cubDataToRead, uint32 cOffset, EUGCReadAction eAction )
{
    PRINT_DEBUG("Steam_Remote_Storage::UGCRead %llu, %p, %i, %u, %i\n", hContent, pvData, cubDataToRead, cOffset, eAction);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    if (hContent == k_UGCHandleInvalid || !downloaded_files.count(hContent) || cubDataToRead < 0) {
        return -1; //TODO: is this the right return value?
    }

    int read_data = -1;
    uint64 total_size = 0;
    Downloaded_File f = downloaded_files[hContent];

    // depending on the download source, we have to decide where to grab the content/data
    switch (f.source)
    {
    case Downloaded_File::DownloadSource::AfterFileShare:
    {
        PRINT_DEBUG("  Steam_Remote_Storage::UGCRead source = AfterFileShare '%s'\n", f.file.c_str());
        read_data = local_storage->get_data(Local_Storage::remote_storage_folder, f.file, (char *)pvData, cubDataToRead, cOffset);
        total_size = f.total_size;
    }
    break;
    
    case Downloaded_File::DownloadSource::AfterSendQueryUGCRequest:
    case Downloaded_File::DownloadSource::FromUGCDownloadToLocation:
    {
        PRINT_DEBUG("  Steam_Remote_Storage::UGCRead source = AfterSendQueryUGCRequest || FromUGCDownloadToLocation [%i]\n", (int)f.source);
        auto mod = settings->getMod(f.mod_query_info.mod_id);
        auto &mod_name = f.mod_query_info.is_primary_file
            ? mod.primaryFileName
            : mod.previewFileName;

        std::string mod_fullpath{};
        if (f.source == Downloaded_File::DownloadSource::AfterSendQueryUGCRequest) {
            std::string mod_base_path = f.mod_query_info.is_primary_file
                ? mod.path
                : Local_Storage::get_game_settings_path() + "mod_images" + PATH_SEPARATOR + std::to_string(mod.id);

            mod_fullpath = common_helpers::to_absolute(mod_name, mod_base_path);
        } else { // Downloaded_File::DownloadSource::FromUGCDownloadToLocation
            mod_fullpath = f.download_to_location_fullpath;
        }
        
        read_data = Local_Storage::get_file_data(mod_fullpath, (char *)pvData, cubDataToRead, cOffset);
        PRINT_DEBUG("  Steam_Remote_Storage::UGCRead mod file '%s' [%i]\n", mod_fullpath.c_str(), read_data);
        total_size = f.total_size;
    }
    break;
    
    default:
        PRINT_DEBUG("  Steam_Remote_Storage::UGCRead unhandled download source %i\n", (int)f.source);
        return -1; //TODO: is this the right return value?
    break;
    }
    
    PRINT_DEBUG("  Steam_Remote_Storage::UGCRead read bytes = %i\n", read_data);
    if (read_data < 0) return -1; //TODO: is this the right return value?

    if (eAction == k_EUGCRead_Close ||
        (eAction == k_EUGCRead_ContinueReadingUntilFinished && (read_data < cubDataToRead || (cOffset + cubDataToRead) >= total_size))) {
        downloaded_files.erase(hContent);
    }

    return read_data;
}

int32 UGCRead( UGCHandle_t hContent, void *pvData, int32 cubDataToRead )
{
    PRINT_DEBUG("Steam_Remote_Storage::UGCRead old\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return UGCRead( hContent, pvData, cubDataToRead, 0);
}

int32 UGCRead( UGCHandle_t hContent, void *pvData, int32 cubDataToRead, uint32 cOffset)
{
    PRINT_DEBUG("Steam_Remote_Storage::UGCRead old2\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return UGCRead(hContent, pvData, cubDataToRead, cOffset, k_EUGCRead_ContinueReadingUntilFinished);
}

// Functions to iterate through UGC that has finished downloading but has not yet been read via UGCRead()
int32 GetCachedUGCCount()
{
    PRINT_DEBUG("Steam_Remote_Storage::GetCachedUGCCount\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return 0;
}

UGCHandle_t GetCachedUGCHandle( int32 iCachedContent )
{
    PRINT_DEBUG("Steam_Remote_Storage::GetCachedUGCHandle\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return k_UGCHandleInvalid;
}


// The following functions are only necessary on the Playstation 3. On PC & Mac, the Steam client will handle these operations for you
// On Playstation 3, the game controls which files are stored in the cloud, via FilePersist, FileFetch, and FileForget.
    
#if defined(_PS3) || defined(_SERVER)
// Connect to Steam and get a list of files in the Cloud - results in a RemoteStorageAppSyncStatusCheck_t callback
void GetFileListFromServer()
{
    PRINT_DEBUG("Steam_Remote_Storage::GetFileListFromServer\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
}

// Indicate this file should be downloaded in the next sync
bool FileFetch( const char *pchFile )
{
    PRINT_DEBUG("Steam_Remote_Storage::FileFetch\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return true;
}

// Indicate this file should be persisted in the next sync
bool FilePersist( const char *pchFile )
{
    PRINT_DEBUG("Steam_Remote_Storage::FilePersist\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return true;
}

// Pull any requested files down from the Cloud - results in a RemoteStorageAppSyncedClient_t callback
bool SynchronizeToClient()
{
    PRINT_DEBUG("Steam_Remote_Storage::SynchronizeToClient\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
}

// Upload any requested files to the Cloud - results in a RemoteStorageAppSyncedServer_t callback
bool SynchronizeToServer()
{
    PRINT_DEBUG("Steam_Remote_Storage::SynchronizeToServer\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
}

// Reset any fetch/persist/etc requests
bool ResetFileRequestState()
{
    PRINT_DEBUG("Steam_Remote_Storage::ResetFileRequestState\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
}

#endif

// publishing UGC
STEAM_CALL_RESULT( RemoteStoragePublishFileProgress_t )
SteamAPICall_t PublishWorkshopFile( const char *pchFile, const char *pchPreviewFile, AppId_t nConsumerAppId, const char *pchTitle, const char *pchDescription, ERemoteStoragePublishedFileVisibility eVisibility, SteamParamStringArray_t *pTags, EWorkshopFileType eWorkshopFileType )
{
    PRINT_DEBUG("TODO Steam_Remote_Storage::PublishWorkshopFile\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    return k_uAPICallInvalid;
}

PublishedFileUpdateHandle_t CreatePublishedFileUpdateRequest( PublishedFileId_t unPublishedFileId )
{
    PRINT_DEBUG("TODO Steam_Remote_Storage::CreatePublishedFileUpdateRequest\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return k_PublishedFileUpdateHandleInvalid;
}

bool UpdatePublishedFileFile( PublishedFileUpdateHandle_t updateHandle, const char *pchFile )
{
    PRINT_DEBUG("TODO Steam_Remote_Storage::UpdatePublishedFileFile\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}

SteamAPICall_t PublishFile( const char *pchFile, const char *pchPreviewFile, AppId_t nConsumerAppId, const char *pchTitle, const char *pchDescription, ERemoteStoragePublishedFileVisibility eVisibility, SteamParamStringArray_t *pTags )
{
    PRINT_DEBUG("TODO Steam_Remote_Storage::PublishFile\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return k_uAPICallInvalid;
}

SteamAPICall_t PublishWorkshopFile( const char *pchFile, const char *pchPreviewFile, AppId_t nConsumerAppId, const char *pchTitle, const char *pchDescription, SteamParamStringArray_t *pTags )
{
    PRINT_DEBUG("TODO Steam_Remote_Storage::PublishWorkshopFile old\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return k_uAPICallInvalid;
}

SteamAPICall_t UpdatePublishedFile( RemoteStorageUpdatePublishedFileRequest_t updatePublishedFileRequest )
{
    PRINT_DEBUG("TODO Steam_Remote_Storage::UpdatePublishedFile\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return k_uAPICallInvalid;
}

bool UpdatePublishedFilePreviewFile( PublishedFileUpdateHandle_t updateHandle, const char *pchPreviewFile )
{
    PRINT_DEBUG("TODO Steam_Remote_Storage::UpdatePublishedFilePreviewFile\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}

bool UpdatePublishedFileTitle( PublishedFileUpdateHandle_t updateHandle, const char *pchTitle )
{
    PRINT_DEBUG("TODO Steam_Remote_Storage::UpdatePublishedFileTitle\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}

bool UpdatePublishedFileDescription( PublishedFileUpdateHandle_t updateHandle, const char *pchDescription )
{
    PRINT_DEBUG("TODO Steam_Remote_Storage::UpdatePublishedFileDescription\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}

bool UpdatePublishedFileVisibility( PublishedFileUpdateHandle_t updateHandle, ERemoteStoragePublishedFileVisibility eVisibility )
{
    PRINT_DEBUG("TODO Steam_Remote_Storage::UpdatePublishedFileVisibility\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}

bool UpdatePublishedFileTags( PublishedFileUpdateHandle_t updateHandle, SteamParamStringArray_t *pTags )
{
    PRINT_DEBUG("TODO Steam_Remote_Storage::UpdatePublishedFileTags\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}

STEAM_CALL_RESULT( RemoteStorageUpdatePublishedFileResult_t )
SteamAPICall_t CommitPublishedFileUpdate( PublishedFileUpdateHandle_t updateHandle )
{
    PRINT_DEBUG("TODO Steam_Remote_Storage::CommitPublishedFileUpdate %llu\n", updateHandle);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return k_uAPICallInvalid;
}

// Gets published file details for the given publishedfileid.  If unMaxSecondsOld is greater than 0,
// cached data may be returned, depending on how long ago it was cached.  A value of 0 will force a refresh.
// A value of k_WorkshopForceLoadPublishedFileDetailsFromCache will use cached data if it exists, no matter how old it is.
STEAM_CALL_RESULT( RemoteStorageGetPublishedFileDetailsResult_t )
SteamAPICall_t GetPublishedFileDetails( PublishedFileId_t unPublishedFileId, uint32 unMaxSecondsOld )
{
    PRINT_DEBUG("TODO Steam_Remote_Storage::GetPublishedFileDetails %llu %u\n", unPublishedFileId, unMaxSecondsOld);
    //TODO: check what this function really returns
    // TODO is this implementation correct?
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (unPublishedFileId == k_PublishedFileIdInvalid) return k_uAPICallInvalid;
    
    RemoteStorageGetPublishedFileDetailsResult_t data{};
    data.m_nPublishedFileId = unPublishedFileId;

    if (settings->isModInstalled(unPublishedFileId)) {
        auto mod = settings->getMod(unPublishedFileId);
        data.m_eResult = EResult::k_EResultOK;
        data.m_bAcceptedForUse = mod.acceptedForUse;
        data.m_bBanned = mod.banned;
        data.m_bTagsTruncated = mod.tagsTruncated;
        data.m_eFileType = mod.fileType;
        data.m_eVisibility = mod.visibility;
        data.m_hFile = mod.handleFile;
        data.m_hPreviewFile = mod.handlePreviewFile;
        data.m_nConsumerAppID = settings->get_local_game_id().AppID(); // TODO is this correct?
        data.m_nCreatorAppID = settings->get_local_game_id().AppID(); // TODO is this correct?
        data.m_nFileSize = mod.primaryFileSize;
        data.m_nPreviewFileSize = mod.previewFileSize;
        data.m_rtimeCreated = mod.timeCreated;
        data.m_rtimeUpdated = mod.timeUpdated;
        data.m_ulSteamIDOwner = mod.steamIDOwner;
        
        mod.primaryFileName.copy(data.m_pchFileName, sizeof(data.m_pchFileName) - 1);
        mod.description.copy(data.m_rgchDescription, sizeof(data.m_rgchDescription) - 1);
        mod.tags.copy(data.m_rgchTags, sizeof(data.m_rgchTags) - 1);
        mod.title.copy(data.m_rgchTitle, sizeof(data.m_rgchTitle) - 1);
        mod.workshopItemURL.copy(data.m_rgchURL, sizeof(data.m_rgchURL) - 1);

    } else {
        data.m_eResult = EResult::k_EResultFail; // TODO is this correct?
    }

    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));

    // return 0;
/*
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    RemoteStorageGetPublishedFileDetailsResult_t data = {};
    data.m_eResult = k_EResultFail;
    data.m_nPublishedFileId = unPublishedFileId;
    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
*/
}

STEAM_CALL_RESULT( RemoteStorageGetPublishedFileDetailsResult_t )
SteamAPICall_t GetPublishedFileDetails( PublishedFileId_t unPublishedFileId )
{
    PRINT_DEBUG("TODO Steam_Remote_Storage::GetPublishedFileDetails old\n");
    return GetPublishedFileDetails(unPublishedFileId, 0);
}

STEAM_CALL_RESULT( RemoteStorageDeletePublishedFileResult_t )
SteamAPICall_t DeletePublishedFile( PublishedFileId_t unPublishedFileId )
{
    PRINT_DEBUG("TODO Steam_Remote_Storage::DeletePublishedFile %llu\n", unPublishedFileId);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return k_uAPICallInvalid;
}

// enumerate the files that the current user published with this app
STEAM_CALL_RESULT( RemoteStorageEnumerateUserPublishedFilesResult_t )
SteamAPICall_t EnumerateUserPublishedFiles( uint32 unStartIndex )
{
    PRINT_DEBUG("TODO Steam_Remote_Storage::EnumerateUserPublishedFiles %u\n", unStartIndex);
    // TODO is this implementation correct?
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    RemoteStorageEnumerateUserPublishedFilesResult_t data{};

    // collect all published mods by this user
    auto mods = settings->modSet();
    std::vector<PublishedFileId_t> user_pubed{};
    for (auto& id : mods) {
        auto mod = settings->getMod(id);
        if (mod.steamIDOwner == settings->get_local_steam_id().ConvertToUint64()) {
            user_pubed.push_back(id);
        }
    }
    uint32_t modCount = (uint32_t)user_pubed.size();

    if (unStartIndex >= modCount) {
        data.m_eResult = EResult::k_EResultInvalidParam; // TODO is this correct?
    } else {
        data.m_eResult = EResult::k_EResultOK;
        data.m_nTotalResultCount = modCount - unStartIndex; // total count starting from this index
        std::vector<PublishedFileId_t>::iterator i = user_pubed.begin();
        std::advance(i, unStartIndex);
        uint32_t iterated = 0;
        for (; i != user_pubed.end() && iterated < k_unEnumeratePublishedFilesMaxResults; i++) {
            PublishedFileId_t modId = *i;
            auto mod = settings->getMod(modId);
            data.m_rgPublishedFileId[iterated] = modId;
            iterated++;
            PRINT_DEBUG("  EnumerateUserPublishedFiles file %llu\n", modId);
        }
        data.m_nResultsReturned = iterated;
    }

    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}

STEAM_CALL_RESULT( RemoteStorageSubscribePublishedFileResult_t )
SteamAPICall_t SubscribePublishedFile( PublishedFileId_t unPublishedFileId )
{
    PRINT_DEBUG("TODO Steam_Remote_Storage::SubscribePublishedFile %llu\n", unPublishedFileId);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (unPublishedFileId == k_PublishedFileIdInvalid) return k_uAPICallInvalid;

    // TODO is this implementation correct?
    RemoteStorageSubscribePublishedFileResult_t data{};
    data.m_nPublishedFileId = unPublishedFileId;

    if (settings->isModInstalled(unPublishedFileId)) {
        data.m_eResult = EResult::k_EResultOK;
        ugc_bridge->add_subbed_mod(unPublishedFileId);
    } else {
        data.m_eResult = EResult::k_EResultFail; // TODO is this correct?
    }

    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}

STEAM_CALL_RESULT( RemoteStorageEnumerateUserSubscribedFilesResult_t )
SteamAPICall_t EnumerateUserSubscribedFiles( uint32 unStartIndex )
{
    // https://partner.steamgames.com/doc/api/ISteamRemoteStorage
    PRINT_DEBUG("Steam_Remote_Storage::EnumerateUserSubscribedFiles %u\n", unStartIndex);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    // Get ready for a working but bad implementation - Detanup01
    RemoteStorageEnumerateUserSubscribedFilesResult_t data{};
    uint32_t modCount = (uint32_t)ugc_bridge->subbed_mods_count();
    if (unStartIndex >= modCount) {
        data.m_eResult = EResult::k_EResultInvalidParam; // is this correct?
    } else {
        data.m_eResult = k_EResultOK;
        data.m_nTotalResultCount = modCount - unStartIndex; // total amount starting from given index
        std::set<PublishedFileId_t>::iterator i = ugc_bridge->subbed_mods_itr_begin();
        std::advance(i, unStartIndex);
        uint32_t iterated = 0;
        for (; i != ugc_bridge->subbed_mods_itr_end() && iterated < k_unEnumeratePublishedFilesMaxResults; i++) {
            PublishedFileId_t modId = *i;
            auto mod = settings->getMod(modId);
            uint32 time = mod.timeAddedToUserList; //this can be changed, default is 1554997000
            data.m_rgPublishedFileId[iterated] = modId;
            data.m_rgRTimeSubscribed[iterated] = time;
            iterated++;
            PRINT_DEBUG("  EnumerateUserSubscribedFiles file %llu\n", modId);
        }
        data.m_nResultsReturned = iterated;
    }

    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}

STEAM_CALL_RESULT( RemoteStorageUnsubscribePublishedFileResult_t )
SteamAPICall_t UnsubscribePublishedFile( PublishedFileId_t unPublishedFileId )
{
    PRINT_DEBUG("TODO Steam_Remote_Storage::UnsubscribePublishedFile %llu\n", unPublishedFileId);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (unPublishedFileId == k_PublishedFileIdInvalid) return k_uAPICallInvalid;

    // TODO is this implementation correct?
    RemoteStorageUnsubscribePublishedFileResult_t data{};
    data.m_nPublishedFileId = unPublishedFileId;
    // TODO is this correct?
    if (ugc_bridge->has_subbed_mod(unPublishedFileId)) {
        data.m_eResult = k_EResultOK;
        ugc_bridge->remove_subbed_mod(unPublishedFileId);
    } else {
        data.m_eResult = k_EResultFail;
    }

    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}

bool UpdatePublishedFileSetChangeDescription( PublishedFileUpdateHandle_t updateHandle, const char *pchChangeDescription )
{
    PRINT_DEBUG("Steam_Remote_Storage::UpdatePublishedFileSetChangeDescription\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    return false;
}

STEAM_CALL_RESULT( RemoteStorageGetPublishedItemVoteDetailsResult_t )
SteamAPICall_t GetPublishedItemVoteDetails( PublishedFileId_t unPublishedFileId )
{
    PRINT_DEBUG("TODO Steam_Remote_Storage::GetPublishedItemVoteDetails\n");
    // TODO s this implementation correct?
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (unPublishedFileId == k_PublishedFileIdInvalid) return k_uAPICallInvalid;

    RemoteStorageGetPublishedItemVoteDetailsResult_t data{};
    data.m_unPublishedFileId = unPublishedFileId;
    if (settings->isModInstalled(unPublishedFileId)) {
        data.m_eResult = EResult::k_EResultOK;
        auto mod = settings->getMod(unPublishedFileId);
        data.m_fScore = mod.score;
        data.m_nReports = 0; // TODO is this ok?
        data.m_nVotesAgainst = mod.votesDown;
        data.m_nVotesFor = mod.votesUp;
    } else {
        data.m_eResult = EResult::k_EResultFail; // TODO is this correct?
    }

    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}

STEAM_CALL_RESULT( RemoteStorageUpdateUserPublishedItemVoteResult_t )
SteamAPICall_t UpdateUserPublishedItemVote( PublishedFileId_t unPublishedFileId, bool bVoteUp )
{
    // I assume this function is supposed to increase the upvotes of the mod,
    // given that the mod owner is the current user
    PRINT_DEBUG("TODO Steam_Remote_Storage::UpdateUserPublishedItemVote\n");
    // TODO is this implementation correct?
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (unPublishedFileId == k_PublishedFileIdInvalid) return k_uAPICallInvalid;

    RemoteStorageUpdateUserPublishedItemVoteResult_t data{};
    data.m_nPublishedFileId = unPublishedFileId;
    if (settings->isModInstalled(unPublishedFileId)) {
        auto mod = settings->getMod(unPublishedFileId);
        if (mod.steamIDOwner == settings->get_local_steam_id().ConvertToUint64()) {
            data.m_eResult = EResult::k_EResultOK;
        } else { // not published by this user
            data.m_eResult = EResult::k_EResultFail; // TODO is this correct?
        }
    } else { // mod not installed
        data.m_eResult = EResult::k_EResultFail; // TODO is this correct?
    }

    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}

STEAM_CALL_RESULT( RemoteStorageGetPublishedItemVoteDetailsResult_t )
SteamAPICall_t GetUserPublishedItemVoteDetails( PublishedFileId_t unPublishedFileId )
{
    PRINT_DEBUG("Steam_Remote_Storage::GetUserPublishedItemVoteDetails\n");
    
    // TODO is this implementation correct?
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (unPublishedFileId == k_PublishedFileIdInvalid) return k_uAPICallInvalid;

    RemoteStorageGetPublishedItemVoteDetailsResult_t data{};
    data.m_unPublishedFileId = unPublishedFileId;
    if (settings->isModInstalled(unPublishedFileId)) {
        auto mod = settings->getMod(unPublishedFileId);
        if (mod.steamIDOwner == settings->get_local_steam_id().ConvertToUint64()) {
            data.m_eResult = EResult::k_EResultOK;
            data.m_fScore = mod.score;
            data.m_nReports = 0; // TODO is this ok?
            data.m_nVotesAgainst = mod.votesDown;
            data.m_nVotesFor = mod.votesUp;
        } else { // not published by this user
            data.m_eResult = EResult::k_EResultFail; // TODO is this correct?
        }
    } else { // mod not installed
        data.m_eResult = EResult::k_EResultFail; // TODO is this correct?
    }

    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));

    return 0;
}

STEAM_CALL_RESULT( RemoteStorageEnumerateUserPublishedFilesResult_t )
SteamAPICall_t EnumerateUserSharedWorkshopFiles( CSteamID steamId, uint32 unStartIndex, SteamParamStringArray_t *pRequiredTags, SteamParamStringArray_t *pExcludedTags )
{
    PRINT_DEBUG("Steam_Remote_Storage::EnumerateUserSharedWorkshopFiles\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    RemoteStorageEnumerateUserPublishedFilesResult_t data{};
    data.m_eResult = k_EResultOK;
    data.m_nResultsReturned = 0;
    data.m_nTotalResultCount = 0;
    //data.m_rgPublishedFileId;
    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}

STEAM_CALL_RESULT( RemoteStorageEnumerateUserPublishedFilesResult_t )
SteamAPICall_t EnumerateUserSharedWorkshopFiles(AppId_t nAppId, CSteamID steamId, uint32 unStartIndex, SteamParamStringArray_t *pRequiredTags, SteamParamStringArray_t *pExcludedTags )
{
    PRINT_DEBUG("Steam_Remote_Storage::EnumerateUserSharedWorkshopFiles old\n");
    return EnumerateUserSharedWorkshopFiles(steamId, unStartIndex, pRequiredTags, pExcludedTags);
}

STEAM_CALL_RESULT( RemoteStoragePublishFileProgress_t )
SteamAPICall_t PublishVideo( EWorkshopVideoProvider eVideoProvider, const char *pchVideoAccount, const char *pchVideoIdentifier, const char *pchPreviewFile, AppId_t nConsumerAppId, const char *pchTitle, const char *pchDescription, ERemoteStoragePublishedFileVisibility eVisibility, SteamParamStringArray_t *pTags )
{
    PRINT_DEBUG("TODO Steam_Remote_Storage::PublishVideo\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return k_uAPICallInvalid;
}

STEAM_CALL_RESULT( RemoteStoragePublishFileProgress_t )
SteamAPICall_t PublishVideo(const char *pchFileName, const char *pchPreviewFile, AppId_t nConsumerAppId, const char *pchTitle, const char *pchDescription, ERemoteStoragePublishedFileVisibility eVisibility, SteamParamStringArray_t *pTags )
{
    PRINT_DEBUG("TODO Steam_Remote_Storage::PublishVideo old\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return k_uAPICallInvalid;
}

STEAM_CALL_RESULT( RemoteStorageSetUserPublishedFileActionResult_t )
SteamAPICall_t SetUserPublishedFileAction( PublishedFileId_t unPublishedFileId, EWorkshopFileAction eAction )
{
    PRINT_DEBUG("TODO Steam_Remote_Storage::SetUserPublishedFileAction\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return k_uAPICallInvalid;
}

STEAM_CALL_RESULT( RemoteStorageEnumeratePublishedFilesByUserActionResult_t )
SteamAPICall_t EnumeratePublishedFilesByUserAction( EWorkshopFileAction eAction, uint32 unStartIndex )
{
    PRINT_DEBUG("TODO Steam_Remote_Storage::EnumeratePublishedFilesByUserAction\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return k_uAPICallInvalid;
}

// this method enumerates the public view of workshop files
STEAM_CALL_RESULT( RemoteStorageEnumerateWorkshopFilesResult_t )
SteamAPICall_t EnumeratePublishedWorkshopFiles( EWorkshopEnumerationType eEnumerationType, uint32 unStartIndex, uint32 unCount, uint32 unDays, SteamParamStringArray_t *pTags, SteamParamStringArray_t *pUserTags )
{
    PRINT_DEBUG("TODO Steam_Remote_Storage::EnumeratePublishedWorkshopFiles\n");
    // TODO is this implementation correct?
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    RemoteStorageEnumerateWorkshopFilesResult_t data{};
    data.m_eResult = EResult::k_EResultOK;
    data.m_nResultsReturned = 0;
    data.m_nTotalResultCount = 0;
    
    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}


STEAM_CALL_RESULT( RemoteStorageDownloadUGCResult_t )
SteamAPICall_t UGCDownloadToLocation( UGCHandle_t hContent, const char *pchLocation, uint32 unPriority )
{
    PRINT_DEBUG("TODO Steam_Remote_Storage::UGCDownloadToLocation %llu %s\n", hContent, pchLocation);
    // TODO is this implementation correct?
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    //TODO: not sure if this is the right result
    if (hContent == k_UGCHandleInvalid || !pchLocation || !pchLocation[0]) return k_uAPICallInvalid;

    RemoteStorageDownloadUGCResult_t data{};
    data.m_hFile = hContent;
    data.m_nAppID = settings->get_local_game_id().AppID();

    auto query_res = ugc_bridge->get_ugc_query_result(hContent);
    if (query_res) {
        auto mod = settings->getMod(query_res.value().mod_id);
        auto &mod_name = query_res.value().is_primary_file
            ? mod.primaryFileName
            : mod.previewFileName;
        std::string mod_base_path = query_res.value().is_primary_file
            ? mod.path
            : Local_Storage::get_game_settings_path() + "mod_images" + PATH_SEPARATOR + std::to_string(mod.id);
        int32 mod_size = query_res.value().is_primary_file
            ? mod.primaryFileSize
            : mod.previewFileSize;

        data.m_eResult = k_EResultOK;
        data.m_nAppID = settings->get_local_game_id().AppID();
        data.m_ulSteamIDOwner = mod.steamIDOwner;
        data.m_nSizeInBytes = mod_size;
        data.m_ulSteamIDOwner = mod.steamIDOwner;

        mod_name.copy(data.m_pchFileName, sizeof(data.m_pchFileName) - 1);
        
        // copy the file
        const auto mod_fullpath = common_helpers::to_absolute(mod_name, mod_base_path);
        copy_file(mod_fullpath, pchLocation);

        // TODO not sure about this though
        downloaded_files[hContent].source = Downloaded_File::DownloadSource::FromUGCDownloadToLocation;
        downloaded_files[hContent].file = mod_name;
        downloaded_files[hContent].total_size = mod_size;
        
        downloaded_files[hContent].mod_query_info = query_res.value();
        downloaded_files[hContent].download_to_location_fullpath = pchLocation;
        
    } else {
        data.m_eResult = k_EResultFileNotFound; //TODO: not sure if this is the right result
    }

    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}

// Cloud dynamic state change notification
int32 GetLocalFileChangeCount()
{
    PRINT_DEBUG("GetLocalFileChangeCount\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return 0;
}

const char *GetLocalFileChange( int iFile, ERemoteStorageLocalFileChange *pEChangeType, ERemoteStorageFilePathType *pEFilePathType )
{
    PRINT_DEBUG("GetLocalFileChange\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return "";
}

// Indicate to Steam the beginning / end of a set of local file
// operations - for example, writing a game save that requires updating two files.
bool BeginFileWriteBatch()
{
    PRINT_DEBUG("BeginFileWriteBatch\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return true;
}

bool EndFileWriteBatch()
{
    PRINT_DEBUG("EndFileWriteBatch\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return true;
}


};
