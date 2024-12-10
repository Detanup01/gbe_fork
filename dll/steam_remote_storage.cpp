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

#include "dll/steam_remote_storage.h"


Downloaded_File::Downloaded_File(DownloadSource src)
    :source(src)
{ }

Downloaded_File::DownloadSource Downloaded_File::get_source() const
{
    return source;
}

static void copy_file(const std::string &src_filepath, const std::string &dst_filepath)
{
    try
    {
        PRINT_DEBUG("copying file '%s' to '%s'", src_filepath.c_str(), dst_filepath.c_str());
        const auto src_p(std::filesystem::u8path(src_filepath));
        
        if (!common_helpers::file_exist(src_p)) return;
        
        const auto dst_p(std::filesystem::u8path(dst_filepath));
        std::filesystem::create_directories(dst_p.parent_path()); // make the folder tree if needed
        std::filesystem::copy_file(src_p, dst_p, std::filesystem::copy_options::overwrite_existing | std::filesystem::copy_options::copy_symlinks);
    } catch(...) {}
}


void Steam_Remote_Storage::steam_run_every_runcb(void *object)
{
    // PRINT_DEBUG_ENTRY();

    auto inst = reinterpret_cast<Steam_Remote_Storage *>(object);
    inst->RunCallbacks();
}

Steam_Remote_Storage::Steam_Remote_Storage(class Settings *settings, class Ugc_Remote_Storage_Bridge *ugc_bridge, class Local_Storage *local_storage, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb)
{
    this->settings = settings;
    this->ugc_bridge = ugc_bridge;
    this->local_storage = local_storage;
    this->callback_results = callback_results;
    this->callbacks = callbacks;
    this->run_every_runcb = run_every_runcb;

    steam_cloud_enabled = true;
    this->run_every_runcb->add(&Steam_Remote_Storage::steam_run_every_runcb, this);
}

Steam_Remote_Storage::~Steam_Remote_Storage()
{
	this->run_every_runcb->remove(&Steam_Remote_Storage::steam_run_every_runcb, this);
}

// NOTE
//
// Filenames are case-insensitive, and will be converted to lowercase automatically.
// So "foo.bar" and "Foo.bar" are the same file, and if you write "Foo.bar" then
// iterate the files, the filename returned will be "foo.bar".
//

// file operations
bool Steam_Remote_Storage::FileWrite( const char *pchFile, const void *pvData, int32 cubData )
{
    PRINT_DEBUG("'%s' %p %u", pchFile, pvData, cubData);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pchFile || !pchFile[0] || cubData <= 0 || cubData > k_unMaxCloudFileChunkSize || !pvData) {
        return false;
    }

    int data_stored = local_storage->store_data(Local_Storage::remote_storage_folder, pchFile, (char* )pvData, cubData);
    PRINT_DEBUG("%i, %u", data_stored, data_stored == cubData);
    return data_stored == cubData;
}

int32 Steam_Remote_Storage::FileRead( const char *pchFile, void *pvData, int32 cubDataToRead )
{
    PRINT_DEBUG("'%s' %p %i", pchFile, pvData, cubDataToRead);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pchFile || !pchFile[0] || !pvData || !cubDataToRead) return 0;
    int read_data = local_storage->get_data(Local_Storage::remote_storage_folder, pchFile, (char* )pvData, cubDataToRead);
    if (read_data < 0) read_data = 0;
    PRINT_DEBUG("  Read %i", read_data);
    return read_data;
}

STEAM_CALL_RESULT( RemoteStorageFileWriteAsyncComplete_t )
SteamAPICall_t Steam_Remote_Storage::FileWriteAsync( const char *pchFile, const void *pvData, uint32 cubData )
{
    PRINT_DEBUG("'%s' %p %u", pchFile, pvData, cubData);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pchFile || !pchFile[0] || cubData > k_unMaxCloudFileChunkSize || cubData == 0 || !pvData) {
        return k_uAPICallInvalid;
    }

    bool success = local_storage->store_data(Local_Storage::remote_storage_folder, pchFile, (char* )pvData, cubData) == cubData;
    RemoteStorageFileWriteAsyncComplete_t data;
    data.m_eResult = success ? k_EResultOK : k_EResultFail;

    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data), 0.01);
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data), 0.01);
    return ret;
}


STEAM_CALL_RESULT( RemoteStorageFileReadAsyncComplete_t )
SteamAPICall_t Steam_Remote_Storage::FileReadAsync( const char *pchFile, uint32 nOffset, uint32 cubToRead )
{
    PRINT_DEBUG("'%s' %u %u", pchFile, nOffset, cubToRead);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pchFile || !pchFile[0]) return k_uAPICallInvalid;
    unsigned int size = local_storage->file_size(Local_Storage::remote_storage_folder, pchFile);

    RemoteStorageFileReadAsyncComplete_t data;
    if (size <= nOffset) {
     return k_uAPICallInvalid;
    }

    if ((size - nOffset) < cubToRead) cubToRead = size - nOffset;

    struct Async_Read a_read{};
    data.m_eResult = k_EResultOK;
    a_read.offset = data.m_nOffset = nOffset;
    a_read.api_call = data.m_hFileReadAsync = callback_results->reserveCallResult();
    a_read.to_read = data.m_cubRead = cubToRead;
    a_read.file_name = std::string(pchFile);
    a_read.size = size;

    async_reads.push_back(a_read);

    callback_results->addCallResult(data.m_hFileReadAsync, data.k_iCallback, &data, sizeof(data), 0.0);
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data), 0.0);
    return data.m_hFileReadAsync;
}

bool Steam_Remote_Storage::FileReadAsyncComplete( SteamAPICall_t hReadCall, void *pvBuffer, uint32 cubToRead )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!pvBuffer) return false;

    auto a_read = std::find_if(async_reads.begin(), async_reads.end(), [&hReadCall](Async_Read const& item) { return item.api_call == hReadCall; });
    if (async_reads.end() == a_read)
        return false;

    if (cubToRead < a_read->to_read)
        return false;

    std::vector<char> temp(a_read->size);
    int read_data = local_storage->get_data(Local_Storage::remote_storage_folder, a_read->file_name, (char* )&temp[0], a_read->size);
    if (read_data < 0 || (static_cast<uint32>(read_data) < (a_read->to_read + a_read->offset))) {
        return false;
    }

    memcpy(pvBuffer, &temp[0] + a_read->offset, a_read->to_read);
    async_reads.erase(a_read);
    return true;
}


bool Steam_Remote_Storage::FileForget( const char *pchFile )
{
    PRINT_DEBUG("'%s'", pchFile);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!pchFile || !pchFile[0]) return false;

    return true;
}

bool Steam_Remote_Storage::FileDelete( const char *pchFile )
{
    PRINT_DEBUG("'%s'", pchFile);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!pchFile || !pchFile[0]) return false;
    
    return local_storage->file_delete(Local_Storage::remote_storage_folder, pchFile);
}

STEAM_CALL_RESULT( RemoteStorageFileShareResult_t )
SteamAPICall_t Steam_Remote_Storage::FileShare( const char *pchFile )
{
    PRINT_DEBUG("'%s'", pchFile);
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

    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return ret;
}

bool Steam_Remote_Storage::SetSyncPlatforms( const char *pchFile, ERemoteStoragePlatform eRemoteStoragePlatform )
{
    PRINT_DEBUG("'%s' %i", pchFile, (int)eRemoteStoragePlatform);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!pchFile || !pchFile[0]) return false;
    
    return true;
}


// file operations that cause network IO
UGCFileWriteStreamHandle_t Steam_Remote_Storage::FileWriteStreamOpen( const char *pchFile )
{
    PRINT_DEBUG("'%s'", pchFile);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!pchFile || !pchFile[0]) return k_UGCFileStreamHandleInvalid;
    
    static UGCFileWriteStreamHandle_t handle = 0;
    ++handle;
    if (!handle) handle = 1;

    struct Stream_Write stream_write{};
    stream_write.file_name = std::string(pchFile);
    stream_write.write_stream_handle = handle;
    stream_writes.push_back(stream_write);
    return stream_write.write_stream_handle;
}

bool Steam_Remote_Storage::FileWriteStreamWriteChunk( UGCFileWriteStreamHandle_t writeHandle, const void *pvData, int32 cubData )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!pvData || cubData < 0) return false;

    auto request = std::find_if(stream_writes.begin(), stream_writes.end(), [&writeHandle](struct Stream_Write const& item) { return item.write_stream_handle == writeHandle; });
    if (stream_writes.end() == request)
        return false;

    std::copy((char *)pvData, (char *)pvData + cubData, std::back_inserter(request->file_data));
    return true;
}

bool Steam_Remote_Storage::FileWriteStreamClose( UGCFileWriteStreamHandle_t writeHandle )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    auto request = std::find_if(stream_writes.begin(), stream_writes.end(), [&writeHandle](struct Stream_Write const& item) { return item.write_stream_handle == writeHandle; });
    if (stream_writes.end() == request)
        return false;

    local_storage->store_data(Local_Storage::remote_storage_folder, request->file_name, request->file_data.data(), static_cast<unsigned int>(request->file_data.size()));
    stream_writes.erase(request);
    return true;
}

bool Steam_Remote_Storage::FileWriteStreamCancel( UGCFileWriteStreamHandle_t writeHandle )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    auto request = std::find_if(stream_writes.begin(), stream_writes.end(), [&writeHandle](struct Stream_Write const& item) { return item.write_stream_handle == writeHandle; });
    if (stream_writes.end() == request)
        return false;

    stream_writes.erase(request);
    return true;
}

// file information
bool Steam_Remote_Storage::FileExists( const char *pchFile )
{
    PRINT_DEBUG("'%s'", pchFile);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!pchFile || !pchFile[0]) return false;
    
    return local_storage->file_exists(Local_Storage::remote_storage_folder, pchFile);
}

bool Steam_Remote_Storage::FilePersisted( const char *pchFile )
{
    PRINT_DEBUG("'%s'", pchFile);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!pchFile || !pchFile[0]) return false;
    
    return local_storage->file_exists(Local_Storage::remote_storage_folder, pchFile);
}

int32 Steam_Remote_Storage::GetFileSize( const char *pchFile )
{
    PRINT_DEBUG("'%s'", pchFile);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!pchFile || !pchFile[0]) return 0;
    
    return local_storage->file_size(Local_Storage::remote_storage_folder, pchFile);
}

int64 Steam_Remote_Storage::GetFileTimestamp( const char *pchFile )
{
    PRINT_DEBUG("'%s'", pchFile);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!pchFile || !pchFile[0]) return 0;
    
    return local_storage->file_timestamp(Local_Storage::remote_storage_folder, pchFile);
}

ERemoteStoragePlatform Steam_Remote_Storage::GetSyncPlatforms( const char *pchFile )
{
    PRINT_DEBUG("'%s'", pchFile);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return k_ERemoteStoragePlatformAll;
}


// iteration
int32 Steam_Remote_Storage::GetFileCount()
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    int32 num = local_storage->count_files(Local_Storage::remote_storage_folder);
    PRINT_DEBUG("count: %i", num);
    return num;
}

const char* Steam_Remote_Storage::GetFileNameAndSize( int iFile, int32 *pnFileSizeInBytes )
{
    PRINT_DEBUG("%i %p", iFile, pnFileSizeInBytes);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    std::string output_filename{};
    if (local_storage->iterate_file(Local_Storage::remote_storage_folder, iFile, output_filename, pnFileSizeInBytes)) {
        auto &request = requests_GetFileNameAndSize.create(std::chrono::minutes(15), std::move(output_filename));
        PRINT_DEBUG("file '%s', size: %i", request.c_str(), pnFileSizeInBytes ? *pnFileSizeInBytes : 0);
        return request.c_str();
    }
    
    return "";
}


// configuration management
bool Steam_Remote_Storage::GetQuota( uint64 *pnTotalBytes, uint64 *puAvailableBytes )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    uint64 quota = 2 << 26;
    if (pnTotalBytes) *pnTotalBytes = quota;
    if (puAvailableBytes) *puAvailableBytes = (quota);
    return true;
}

bool Steam_Remote_Storage::GetQuota( int32 *pnTotalBytes, int32 *puAvailableBytes )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    int32 quota = 2 << 26;
    if (pnTotalBytes) *pnTotalBytes = quota;
    if (puAvailableBytes) *puAvailableBytes = (quota);
    return true;
}

bool Steam_Remote_Storage::IsCloudEnabledForAccount()
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return true;
}

bool Steam_Remote_Storage::IsCloudEnabledForApp()
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return steam_cloud_enabled;
}

bool Steam_Remote_Storage::IsCloudEnabledThisApp()
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return steam_cloud_enabled;
}

void Steam_Remote_Storage::SetCloudEnabledForApp( bool bEnabled )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    steam_cloud_enabled = bEnabled;
}

bool Steam_Remote_Storage::SetCloudEnabledThisApp( bool bEnabled )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    steam_cloud_enabled = bEnabled;
    return true;
}

// user generated content

// Downloads a UGC file.  A priority value of 0 will download the file immediately,
// otherwise it will wait to download the file until all downloads with a lower priority
// value are completed.  Downloads with equal priority will occur simultaneously.
STEAM_CALL_RESULT( RemoteStorageDownloadUGCResult_t )
SteamAPICall_t Steam_Remote_Storage::UGCDownload( UGCHandle_t hContent, uint32 unPriority )
{
    PRINT_DEBUG("%llu", hContent);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (hContent == k_UGCHandleInvalid) return k_uAPICallInvalid;

    RemoteStorageDownloadUGCResult_t data{};
    data.m_hFile = hContent;
    data.m_nAppID = settings->get_local_game_id().AppID();

    if (shared_files.count(hContent)) {
        data.m_eResult = k_EResultOK;
        data.m_ulSteamIDOwner = settings->get_local_steam_id().ConvertToUint64();
        data.m_nSizeInBytes = local_storage->file_size(Local_Storage::remote_storage_folder, shared_files[hContent]);

        shared_files[hContent].copy(data.m_pchFileName, sizeof(data.m_pchFileName) - 1);
        PRINT_DEBUG("  FileShare data.m_pchFileName = '%s'", data.m_pchFileName);

        auto [ele_itr, _] = downloaded_files.insert_or_assign(hContent, Downloaded_File::DownloadSource::AfterFileShare);
        auto &ele = ele_itr->second;
        ele.file = shared_files[hContent];
        ele.total_size = data.m_nSizeInBytes;
    } else if (auto query_res = ugc_bridge->get_ugc_query_result(hContent)) {
        auto mod = settings->getMod(query_res.value().mod_id);
        auto &mod_name = query_res.value().is_primary_file
            ? mod.primaryFileName
            : mod.previewFileName;
        int32 mod_size = query_res.value().is_primary_file
            ? mod.primaryFileSize
            : mod.previewFileSize;

        data.m_eResult = k_EResultOK;
        data.m_ulSteamIDOwner = mod.steamIDOwner;
        data.m_nSizeInBytes = mod_size;

        mod_name.copy(data.m_pchFileName, sizeof(data.m_pchFileName) - 1);
        PRINT_DEBUG("  QueryUGCRequest data.m_pchFileName = '%s'", data.m_pchFileName);
        
        auto [ele_itr, _] = downloaded_files.insert_or_assign(hContent, Downloaded_File::DownloadSource::AfterSendQueryUGCRequest);
        auto &ele = ele_itr->second;
        ele.file = mod_name;
        ele.total_size = mod_size;
        ele.mod_query_info = query_res.value();
    } else {
        data.m_eResult = k_EResultFileNotFound; //TODO: not sure if this is the right result
    }

    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return ret;
}

STEAM_CALL_RESULT( RemoteStorageDownloadUGCResult_t )
SteamAPICall_t Steam_Remote_Storage::UGCDownload( UGCHandle_t hContent )
{
    PRINT_DEBUG("old");
    return UGCDownload(hContent, 1);
}


// Gets the amount of data downloaded so far for a piece of content. pnBytesExpected can be 0 if function returns false
// or if the transfer hasn't started yet, so be careful to check for that before dividing to get a percentage
bool Steam_Remote_Storage::GetUGCDownloadProgress( UGCHandle_t hContent, int32 *pnBytesDownloaded, int32 *pnBytesExpected )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}

bool Steam_Remote_Storage::GetUGCDownloadProgress( UGCHandle_t hContent, uint32 *pnBytesDownloaded, uint32 *pnBytesExpected )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}


// Gets metadata for a file after it has been downloaded. This is the same metadata given in the RemoteStorageDownloadUGCResult_t call result
bool Steam_Remote_Storage::GetUGCDetails( UGCHandle_t hContent, AppId_t *pnAppID, STEAM_OUT_STRING() char **ppchName, int32 *pnFileSizeInBytes, STEAM_OUT_STRUCT() CSteamID *pSteamIDOwner )
{
    PRINT_DEBUG("%llu", hContent);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (hContent == k_UGCHandleInvalid) return false;

    if (pnAppID) *pnAppID = settings->get_local_game_id().AppID();
    if (pSteamIDOwner) *pSteamIDOwner = k_steamIDNil;
    if (pnFileSizeInBytes) *pnFileSizeInBytes = 0;
    if (ppchName) *ppchName = nullptr;

    if (auto query_res = ugc_bridge->get_ugc_query_result(hContent)) {
        const auto mod = settings->getMod(query_res.value().mod_id);
        const auto &mod_name = query_res.value().is_primary_file
            ? mod.primaryFileName
            : mod.previewFileName;
        int32 mod_size = query_res.value().is_primary_file
            ? mod.primaryFileSize
            : mod.previewFileSize;

        if (ppchName) {
            auto &new_str = requests_GetUGCDetails.create(std::chrono::minutes(30), mod_name); // this will make a copy of mod_name
            *ppchName = const_cast<char *>(new_str.c_str());
        }
        if (pnFileSizeInBytes) *pnFileSizeInBytes = mod_size;
        if (pSteamIDOwner) *pSteamIDOwner = mod.steamIDOwner;

        return true;
    }
    
    return false;
}


// After download, gets the content of the file.  
// Small files can be read all at once by calling this function with an offset of 0 and cubDataToRead equal to the size of the file.
// Larger files can be read in chunks to reduce memory usage (since both sides of the IPC client and the game itself must allocate
// enough memory for each chunk).  Once the last byte is read, the file is implicitly closed and further calls to UGCRead will fail
// unless UGCDownload is called again.
// For especially large files (anything over 100MB) it is a requirement that the file is read in chunks.
int32 Steam_Remote_Storage::UGCRead( UGCHandle_t hContent, void *pvData, int32 cubDataToRead, uint32 cOffset, EUGCReadAction eAction )
{
    PRINT_DEBUG("%llu, %p, %i, %u, %i", hContent, pvData, cubDataToRead, cOffset, eAction);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    auto f_itr = downloaded_files.find(hContent);
    if (hContent == k_UGCHandleInvalid || (downloaded_files.end() == f_itr) || cubDataToRead < 0) {
        return -1; //TODO: is this the right return value?
    }

    int read_data = -1;
    uint64 total_size = 0;
    Downloaded_File &dwf = f_itr->second;

    // depending on the download source, we have to decide where to grab the content/data
    switch (dwf.get_source())
    {
    case Downloaded_File::DownloadSource::AfterFileShare: {
        PRINT_DEBUG("  source = AfterFileShare '%s'", dwf.file.c_str());
        read_data = local_storage->get_data(Local_Storage::remote_storage_folder, dwf.file, (char *)pvData, cubDataToRead, cOffset);
        total_size = dwf.total_size;
    }
    break;
    
    case Downloaded_File::DownloadSource::AfterSendQueryUGCRequest:
    case Downloaded_File::DownloadSource::FromUGCDownloadToLocation: {
        PRINT_DEBUG("  source = AfterSendQueryUGCRequest || FromUGCDownloadToLocation [%i]", (int)dwf.get_source());
        auto mod = settings->getMod(dwf.mod_query_info.mod_id);
        auto &mod_name = dwf.mod_query_info.is_primary_file
            ? mod.primaryFileName
            : mod.previewFileName;

        std::string mod_fullpath{};
        if (dwf.get_source() == Downloaded_File::DownloadSource::AfterSendQueryUGCRequest) {
            std::string mod_base_path = dwf.mod_query_info.is_primary_file
                ? mod.path
                : Local_Storage::get_game_settings_path() + "mod_images" + PATH_SEPARATOR + std::to_string(mod.id);

            mod_fullpath = common_helpers::to_absolute(mod_name, mod_base_path);
        } else { // Downloaded_File::DownloadSource::FromUGCDownloadToLocation
            mod_fullpath = dwf.download_to_location_fullpath;
        }
        
        read_data = Local_Storage::get_file_data(mod_fullpath, (char *)pvData, cubDataToRead, cOffset);
        PRINT_DEBUG("  mod file '%s' [%i]", mod_fullpath.c_str(), read_data);
        total_size = dwf.total_size;
    }
    break;
    
    default:
        PRINT_DEBUG("  unhandled download source %i", (int)dwf.get_source());
        return -1; //TODO: is this the right return value?
    break;
    }
    
    PRINT_DEBUG("  read bytes = %i", read_data);
    if (read_data < 0) return -1; //TODO: is this the right return value?

    if (eAction == k_EUGCRead_Close ||
        (eAction == k_EUGCRead_ContinueReadingUntilFinished && (read_data < cubDataToRead || (cOffset + cubDataToRead) >= total_size))) {
        downloaded_files.erase(hContent);
    }

    return read_data;
}

int32 Steam_Remote_Storage::UGCRead( UGCHandle_t hContent, void *pvData, int32 cubDataToRead )
{
    PRINT_DEBUG("old");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return UGCRead( hContent, pvData, cubDataToRead, 0);
}

int32 Steam_Remote_Storage::UGCRead( UGCHandle_t hContent, void *pvData, int32 cubDataToRead, uint32 cOffset)
{
    PRINT_DEBUG("old2");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return UGCRead(hContent, pvData, cubDataToRead, cOffset, k_EUGCRead_ContinueReadingUntilFinished);
}

// Functions to iterate through UGC that has finished downloading but has not yet been read via UGCRead()
int32 Steam_Remote_Storage::GetCachedUGCCount()
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return 0;
}

UGCHandle_t Steam_Remote_Storage::GetCachedUGCHandle( int32 iCachedContent )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return k_UGCHandleInvalid;
}


// The following functions are only necessary on the Playstation 3. On PC & Mac, the Steam client will handle these operations for you
// On Playstation 3, the game controls which files are stored in the cloud, via FilePersist, FileFetch, and FileForget.
    
#if defined(_PS3) || defined(_SERVER)
// Connect to Steam and get a list of files in the Cloud - results in a RemoteStorageAppSyncStatusCheck_t callback
void Steam_Remote_Storage::GetFileListFromServer()
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
}

// Indicate this file should be downloaded in the next sync
bool Steam_Remote_Storage::FileFetch( const char *pchFile )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return true;
}

// Indicate this file should be persisted in the next sync
bool Steam_Remote_Storage::FilePersist( const char *pchFile )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return true;
}

// Pull any requested files down from the Cloud - results in a RemoteStorageAppSyncedClient_t callback
bool Steam_Remote_Storage::SynchronizeToClient()
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
}

// Upload any requested files to the Cloud - results in a RemoteStorageAppSyncedServer_t callback
bool Steam_Remote_Storage::SynchronizeToServer()
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
}

// Reset any fetch/persist/etc requests
bool Steam_Remote_Storage::ResetFileRequestState()
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
}

#endif

// publishing UGC
STEAM_CALL_RESULT( RemoteStoragePublishFileProgress_t )
SteamAPICall_t Steam_Remote_Storage::PublishWorkshopFile( const char *pchFile, const char *pchPreviewFile, AppId_t nConsumerAppId, const char *pchTitle, const char *pchDescription, ERemoteStoragePublishedFileVisibility eVisibility, SteamParamStringArray_t *pTags, EWorkshopFileType eWorkshopFileType )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    return k_uAPICallInvalid;
}

PublishedFileUpdateHandle_t Steam_Remote_Storage::CreatePublishedFileUpdateRequest( PublishedFileId_t unPublishedFileId )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return k_PublishedFileUpdateHandleInvalid;
}

bool Steam_Remote_Storage::UpdatePublishedFileFile( PublishedFileUpdateHandle_t updateHandle, const char *pchFile )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}

SteamAPICall_t Steam_Remote_Storage::PublishFile( const char *pchFile, const char *pchPreviewFile, AppId_t nConsumerAppId, const char *pchTitle, const char *pchDescription, ERemoteStoragePublishedFileVisibility eVisibility, SteamParamStringArray_t *pTags )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return k_uAPICallInvalid;
}

SteamAPICall_t Steam_Remote_Storage::PublishWorkshopFile( const char *pchFile, const char *pchPreviewFile, AppId_t nConsumerAppId, const char *pchTitle, const char *pchDescription, SteamParamStringArray_t *pTags )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return k_uAPICallInvalid;
}

SteamAPICall_t Steam_Remote_Storage::UpdatePublishedFile( RemoteStorageUpdatePublishedFileRequest_t updatePublishedFileRequest )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return k_uAPICallInvalid;
}

bool Steam_Remote_Storage::UpdatePublishedFilePreviewFile( PublishedFileUpdateHandle_t updateHandle, const char *pchPreviewFile )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}

bool Steam_Remote_Storage::UpdatePublishedFileTitle( PublishedFileUpdateHandle_t updateHandle, const char *pchTitle )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}

bool Steam_Remote_Storage::UpdatePublishedFileDescription( PublishedFileUpdateHandle_t updateHandle, const char *pchDescription )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}

bool Steam_Remote_Storage::UpdatePublishedFileVisibility( PublishedFileUpdateHandle_t updateHandle, ERemoteStoragePublishedFileVisibility eVisibility )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}

bool Steam_Remote_Storage::UpdatePublishedFileTags( PublishedFileUpdateHandle_t updateHandle, SteamParamStringArray_t *pTags )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}

STEAM_CALL_RESULT( RemoteStorageUpdatePublishedFileResult_t )
SteamAPICall_t Steam_Remote_Storage::CommitPublishedFileUpdate( PublishedFileUpdateHandle_t updateHandle )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return k_uAPICallInvalid;
}

// Gets published file details for the given publishedfileid.  If unMaxSecondsOld is greater than 0,
// cached data may be returned, depending on how long ago it was cached.  A value of 0 will force a refresh.
// A value of k_WorkshopForceLoadPublishedFileDetailsFromCache will use cached data if it exists, no matter how old it is.
STEAM_CALL_RESULT( RemoteStorageGetPublishedFileDetailsResult_t )
SteamAPICall_t Steam_Remote_Storage::GetPublishedFileDetails( PublishedFileId_t unPublishedFileId, uint32 unMaxSecondsOld )
{
    PRINT_DEBUG("TODO %llu %u", unPublishedFileId, unMaxSecondsOld);
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

    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return ret;

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
SteamAPICall_t Steam_Remote_Storage::GetPublishedFileDetails( PublishedFileId_t unPublishedFileId )
{
    PRINT_DEBUG_TODO();
    return GetPublishedFileDetails(unPublishedFileId, 0);
}

STEAM_CALL_RESULT( RemoteStorageDeletePublishedFileResult_t )
SteamAPICall_t Steam_Remote_Storage::DeletePublishedFile( PublishedFileId_t unPublishedFileId )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return k_uAPICallInvalid;
}

// enumerate the files that the current user published with this app
STEAM_CALL_RESULT( RemoteStorageEnumerateUserPublishedFilesResult_t )
SteamAPICall_t Steam_Remote_Storage::EnumerateUserPublishedFiles( uint32 unStartIndex )
{
    PRINT_DEBUG("TODO %u", unStartIndex);
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
            PRINT_DEBUG("  EnumerateUserPublishedFiles file %llu", modId);
        }
        data.m_nResultsReturned = iterated;
    }

    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return ret;
}

STEAM_CALL_RESULT( RemoteStorageSubscribePublishedFileResult_t )
SteamAPICall_t Steam_Remote_Storage::SubscribePublishedFile( PublishedFileId_t unPublishedFileId )
{
    PRINT_DEBUG("TODO %llu", unPublishedFileId);
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

    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return ret;
}

STEAM_CALL_RESULT( RemoteStorageEnumerateUserSubscribedFilesResult_t )
SteamAPICall_t Steam_Remote_Storage::EnumerateUserSubscribedFiles( uint32 unStartIndex )
{
    // https://partner.steamgames.com/doc/api/ISteamRemoteStorage
    PRINT_DEBUG("%u", unStartIndex);
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
            PRINT_DEBUG("  EnumerateUserSubscribedFiles file %llu", modId);
        }
        data.m_nResultsReturned = iterated;
    }

    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return ret;
}

STEAM_CALL_RESULT( RemoteStorageUnsubscribePublishedFileResult_t )
SteamAPICall_t Steam_Remote_Storage::UnsubscribePublishedFile( PublishedFileId_t unPublishedFileId )
{
    PRINT_DEBUG("TODO %llu", unPublishedFileId);
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

    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return ret;
}

bool Steam_Remote_Storage::UpdatePublishedFileSetChangeDescription( PublishedFileUpdateHandle_t updateHandle, const char *pchChangeDescription )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    return false;
}

STEAM_CALL_RESULT( RemoteStorageGetPublishedItemVoteDetailsResult_t )
SteamAPICall_t Steam_Remote_Storage::GetPublishedItemVoteDetails( PublishedFileId_t unPublishedFileId )
{
    PRINT_DEBUG_TODO();
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

    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return ret;
}

STEAM_CALL_RESULT( RemoteStorageUpdateUserPublishedItemVoteResult_t )
SteamAPICall_t Steam_Remote_Storage::UpdateUserPublishedItemVote( PublishedFileId_t unPublishedFileId, bool bVoteUp )
{
    // I assume this function is supposed to increase the upvotes of the mod,
    // given that the mod owner is the current user
    PRINT_DEBUG_TODO();
    // TODO is this implementation correct?
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (unPublishedFileId == k_PublishedFileIdInvalid) return k_uAPICallInvalid;

    RemoteStorageUpdateUserPublishedItemVoteResult_t data{};
    data.m_nPublishedFileId = unPublishedFileId;
    if (settings->isModInstalled(unPublishedFileId)) {
        data.m_eResult = EResult::k_EResultOK;
        auto mod = settings->getMod(unPublishedFileId);
        if (bVoteUp) {
            ++mod.votesUp;
        } else {
            ++mod.votesDown;
        }
        settings->addModDetails(unPublishedFileId, mod);
    } else { // mod not installed
        data.m_eResult = EResult::k_EResultFail; // TODO is this correct?
    }

    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return ret;
}

STEAM_CALL_RESULT( RemoteStorageGetPublishedItemVoteDetailsResult_t )
SteamAPICall_t Steam_Remote_Storage::GetUserPublishedItemVoteDetails( PublishedFileId_t unPublishedFileId )
{
    PRINT_DEBUG_ENTRY();
    
    // TODO is this implementation correct?
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (unPublishedFileId == k_PublishedFileIdInvalid) return k_uAPICallInvalid;

    RemoteStorageGetPublishedItemVoteDetailsResult_t data{};
    data.m_unPublishedFileId = unPublishedFileId;
    if (settings->isModInstalled(unPublishedFileId)) {
        auto mod = settings->getMod(unPublishedFileId);
        data.m_eResult = EResult::k_EResultOK;
        data.m_fScore = mod.score;
        data.m_nReports = 0; // TODO is this ok?
        data.m_nVotesAgainst = mod.votesDown;
        data.m_nVotesFor = mod.votesUp;
    } else { // mod not installed
        data.m_eResult = EResult::k_EResultFail; // TODO is this correct?
    }

    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return ret;
}

STEAM_CALL_RESULT( RemoteStorageEnumerateUserPublishedFilesResult_t )
SteamAPICall_t Steam_Remote_Storage::EnumerateUserSharedWorkshopFiles( CSteamID steamId, uint32 unStartIndex, SteamParamStringArray_t *pRequiredTags, SteamParamStringArray_t *pExcludedTags )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    RemoteStorageEnumerateUserPublishedFilesResult_t data{};
    data.m_eResult = k_EResultOK;
    data.m_nResultsReturned = 0;
    data.m_nTotalResultCount = 0;
    //data.m_rgPublishedFileId;
    
    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return ret;
}

STEAM_CALL_RESULT( RemoteStorageEnumerateUserPublishedFilesResult_t )
SteamAPICall_t Steam_Remote_Storage::EnumerateUserSharedWorkshopFiles(AppId_t nAppId, CSteamID steamId, uint32 unStartIndex, SteamParamStringArray_t *pRequiredTags, SteamParamStringArray_t *pExcludedTags )
{
    PRINT_DEBUG("old");
    return EnumerateUserSharedWorkshopFiles(steamId, unStartIndex, pRequiredTags, pExcludedTags);
}

STEAM_CALL_RESULT( RemoteStoragePublishFileProgress_t )
SteamAPICall_t Steam_Remote_Storage::PublishVideo( EWorkshopVideoProvider eVideoProvider, const char *pchVideoAccount, const char *pchVideoIdentifier, const char *pchPreviewFile, AppId_t nConsumerAppId, const char *pchTitle, const char *pchDescription, ERemoteStoragePublishedFileVisibility eVisibility, SteamParamStringArray_t *pTags )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return k_uAPICallInvalid;
}

STEAM_CALL_RESULT( RemoteStoragePublishFileProgress_t )
SteamAPICall_t Steam_Remote_Storage::PublishVideo(const char *pchFileName, const char *pchPreviewFile, AppId_t nConsumerAppId, const char *pchTitle, const char *pchDescription, ERemoteStoragePublishedFileVisibility eVisibility, SteamParamStringArray_t *pTags )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return k_uAPICallInvalid;
}

STEAM_CALL_RESULT( RemoteStorageSetUserPublishedFileActionResult_t )
SteamAPICall_t Steam_Remote_Storage::SetUserPublishedFileAction( PublishedFileId_t unPublishedFileId, EWorkshopFileAction eAction )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return k_uAPICallInvalid;
}

STEAM_CALL_RESULT( RemoteStorageEnumeratePublishedFilesByUserActionResult_t )
SteamAPICall_t Steam_Remote_Storage::EnumeratePublishedFilesByUserAction( EWorkshopFileAction eAction, uint32 unStartIndex )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return k_uAPICallInvalid;
}

// this method enumerates the public view of workshop files
STEAM_CALL_RESULT( RemoteStorageEnumerateWorkshopFilesResult_t )
SteamAPICall_t Steam_Remote_Storage::EnumeratePublishedWorkshopFiles( EWorkshopEnumerationType eEnumerationType, uint32 unStartIndex, uint32 unCount, uint32 unDays, SteamParamStringArray_t *pTags, SteamParamStringArray_t *pUserTags )
{
    PRINT_DEBUG_TODO();
    // TODO is this implementation correct?
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    RemoteStorageEnumerateWorkshopFilesResult_t data{};
    data.m_eResult = EResult::k_EResultOK;
    data.m_nResultsReturned = 0;
    data.m_nTotalResultCount = 0;
    
    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return ret;
}


STEAM_CALL_RESULT( RemoteStorageDownloadUGCResult_t )
SteamAPICall_t Steam_Remote_Storage::UGCDownloadToLocation( UGCHandle_t hContent, const char *pchLocation, uint32 unPriority )
{
    PRINT_DEBUG("TODO %llu %s", hContent, pchLocation);
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

        mod_name.copy(data.m_pchFileName, sizeof(data.m_pchFileName) - 1);
        
        // copy the file
        const auto mod_fullpath = common_helpers::to_absolute(mod_name, mod_base_path);
        copy_file(mod_fullpath, pchLocation);

        // TODO not sure about this though
        auto [ele_itr, _] = downloaded_files.insert_or_assign(hContent, Downloaded_File::DownloadSource::FromUGCDownloadToLocation);
        auto &ele = ele_itr->second;
        ele.file = mod_name;
        ele.total_size = mod_size;
        ele.mod_query_info = query_res.value();
        ele.download_to_location_fullpath = pchLocation;
        
    } else {
        data.m_eResult = k_EResultFileNotFound; //TODO: not sure if this is the right result
    }

    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return ret;
}

// Cloud dynamic state change notification
int32 Steam_Remote_Storage::GetLocalFileChangeCount()
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return 0;
}

const char* Steam_Remote_Storage::GetLocalFileChange( int iFile, ERemoteStorageLocalFileChange *pEChangeType, ERemoteStorageFilePathType *pEFilePathType )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return "";
}

// Indicate to Steam the beginning / end of a set of local file
// operations - for example, writing a game save that requires updating two files.
bool Steam_Remote_Storage::BeginFileWriteBatch()
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return true;
}

bool Steam_Remote_Storage::EndFileWriteBatch()
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return true;
}



void Steam_Remote_Storage::RunCallbacks()
{
    requests_GetFileNameAndSize.cleanup();
    requests_GetUGCDetails.cleanup();
}
