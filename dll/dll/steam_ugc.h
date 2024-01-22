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

struct UGC_query {
    UGCQueryHandle_t handle;
    std::set<PublishedFileId_t> return_only;
    bool return_all_subscribed;

    std::set<PublishedFileId_t> results;
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
public ISteamUGC012,
public ISteamUGC013,
public ISteamUGC014,
public ISteamUGC015,
public ISteamUGC016,
public ISteamUGC017,
public ISteamUGC
{
    constexpr static const char ugc_favorits_file[] = "favorites.txt";

    class Settings *settings;
    class Ugc_Remote_Storage_Bridge *ugc_bridge;
    class Local_Storage *local_storage;
    class SteamCallResults *callback_results;
    class SteamCallBacks *callbacks;

    UGCQueryHandle_t handle = 50; // just makes debugging easier, any initial val is fine, even 1
    std::vector<struct UGC_query> ugc_queries{};
    std::set<PublishedFileId_t> favorites{};

UGCQueryHandle_t new_ugc_query(
    bool return_all_subscribed = false,
    std::set<PublishedFileId_t> return_only = std::set<PublishedFileId_t>())
{
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    ++handle;
    if ((handle == 0) || (handle == k_UGCQueryHandleInvalid)) handle = 50;

    struct UGC_query query{};
    query.handle = handle;
    query.return_all_subscribed = return_all_subscribed;
    query.return_only = return_only;
    ugc_queries.push_back(query);
    PRINT_DEBUG("Steam_UGC::new_ugc_query handle = %llu\n", query.handle);
    return query.handle;
}

void set_details(PublishedFileId_t id, SteamUGCDetails_t *pDetails)
{
    if (pDetails) {
        memset(pDetails, 0, sizeof(SteamUGCDetails_t));

        pDetails->m_nPublishedFileId = id;

        if (settings->isModInstalled(id)) {
            PRINT_DEBUG("  mod is installed, setting details\n");
            pDetails->m_eResult = k_EResultOK;

            auto mod = settings->getMod(id);
            pDetails->m_bAcceptedForUse = mod.acceptedForUse;
            pDetails->m_bBanned = mod.banned;
            pDetails->m_bTagsTruncated = mod.tagsTruncated;
            pDetails->m_eFileType = mod.fileType;
            pDetails->m_eVisibility = mod.visibility;
            pDetails->m_hFile = mod.handleFile;
            pDetails->m_hPreviewFile = mod.handlePreviewFile;
            pDetails->m_nConsumerAppID = settings->get_local_game_id().AppID();
            pDetails->m_nCreatorAppID = settings->get_local_game_id().AppID();
            pDetails->m_nFileSize = mod.primaryFileSize;
            pDetails->m_nPreviewFileSize = mod.previewFileSize;
            pDetails->m_rtimeCreated = mod.timeCreated;
            pDetails->m_rtimeUpdated = mod.timeUpdated;
            pDetails->m_ulSteamIDOwner = settings->get_local_steam_id().ConvertToUint64();

            pDetails->m_rtimeAddedToUserList = mod.timeAddedToUserList;
            pDetails->m_unVotesUp = mod.votesUp;
            pDetails->m_unVotesDown = mod.votesDown;
            pDetails->m_flScore = mod.score;

            mod.primaryFileName.copy(pDetails->m_pchFileName, sizeof(pDetails->m_pchFileName) - 1);
            mod.description.copy(pDetails->m_rgchDescription, sizeof(pDetails->m_rgchDescription) - 1);
            mod.tags.copy(pDetails->m_rgchTags, sizeof(pDetails->m_rgchTags) - 1);
            mod.title.copy(pDetails->m_rgchTitle, sizeof(pDetails->m_rgchTitle) - 1);
            mod.workshopItemURL.copy(pDetails->m_rgchURL, sizeof(pDetails->m_rgchURL) - 1);

            // TODO should we enable this?
            // pDetails->m_unNumChildren = mod.numChildren;
        } else {
            PRINT_DEBUG("  mod isn't installed, returning failure\n");
            pDetails->m_eResult = k_EResultFail;
        }
    }
}

void read_ugc_favorites()
{
    if (!local_storage->file_exists("", ugc_favorits_file)) return;

    unsigned int size = local_storage->file_size("", ugc_favorits_file);
    if (!size) return;

    std::string data(size, '\0');
    int read = local_storage->get_data("", ugc_favorits_file, &data[0], (unsigned int)data.size());
    if ((size_t)read != data.size()) return;

    std::stringstream ss(data);
    std::string line{};
    while (std::getline(ss, line)) {
        try
        {
            unsigned long long fav_id = std::stoull(line);
            favorites.insert(fav_id);
            PRINT_DEBUG("Steam_UGC added item to favorites %llu\n", fav_id);
        } catch(...) { }
    }
    
}

bool write_ugc_favorites()
{
    std::stringstream ss{};
    for (auto id : favorites) {
        ss << id << "\n";
        ss.flush();
    }
    auto file_data = ss.str();
    int stored = local_storage->store_data("", ugc_favorits_file, &file_data[0], file_data.size());
    return (size_t)stored == file_data.size();
}

public:
Steam_UGC(class Settings *settings, class Ugc_Remote_Storage_Bridge *ugc_bridge, class Local_Storage *local_storage, class SteamCallResults *callback_results, class SteamCallBacks *callbacks)
{
    this->settings = settings;
    this->ugc_bridge = ugc_bridge;
    this->local_storage = local_storage;
    this->callbacks = callbacks;
    this->callback_results = callback_results;

    read_ugc_favorites();
}


// Query UGC associated with a user. Creator app id or consumer app id must be valid and be set to the current running app. unPage should start at 1.
UGCQueryHandle_t CreateQueryUserUGCRequest( AccountID_t unAccountID, EUserUGCList eListType, EUGCMatchingUGCType eMatchingUGCType, EUserUGCListSortOrder eSortOrder, AppId_t nCreatorAppID, AppId_t nConsumerAppID, uint32 unPage )
{
    PRINT_DEBUG("Steam_UGC::CreateQueryUserUGCRequest %u %i %i %i %u %u %u\n", unAccountID, eListType, eMatchingUGCType, eSortOrder, nCreatorAppID, nConsumerAppID, unPage);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (nCreatorAppID != settings->get_local_game_id().AppID() || nConsumerAppID != settings->get_local_game_id().AppID()) return k_UGCQueryHandleInvalid;
    if (unPage < 1) return k_UGCQueryHandleInvalid;
    if (eListType < 0) return k_UGCQueryHandleInvalid;
    if (unAccountID != settings->get_local_steam_id().GetAccountID()) return k_UGCQueryHandleInvalid;
    
    // TODO
    return new_ugc_query(eListType == k_EUserUGCList_Subscribed || eListType == k_EUserUGCList_Published);
}


// Query for all matching UGC. Creator app id or consumer app id must be valid and be set to the current running app. unPage should start at 1.
UGCQueryHandle_t CreateQueryAllUGCRequest( EUGCQuery eQueryType, EUGCMatchingUGCType eMatchingeMatchingUGCTypeFileType, AppId_t nCreatorAppID, AppId_t nConsumerAppID, uint32 unPage )
{
    PRINT_DEBUG("Steam_UGC::CreateQueryAllUGCRequest\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    if (nCreatorAppID != settings->get_local_game_id().AppID() || nConsumerAppID != settings->get_local_game_id().AppID()) return k_UGCQueryHandleInvalid;
    if (unPage < 1) return k_UGCQueryHandleInvalid;
    if (eQueryType < 0) return k_UGCQueryHandleInvalid;
    
    // TODO
    return new_ugc_query();
}

// Query for all matching UGC using the new deep paging interface. Creator app id or consumer app id must be valid and be set to the current running app. pchCursor should be set to NULL or "*" to get the first result set.
UGCQueryHandle_t CreateQueryAllUGCRequest( EUGCQuery eQueryType, EUGCMatchingUGCType eMatchingeMatchingUGCTypeFileType, AppId_t nCreatorAppID, AppId_t nConsumerAppID, const char *pchCursor = NULL )
{
    PRINT_DEBUG("Steam_UGC::CreateQueryAllUGCRequest other\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    if (nCreatorAppID != settings->get_local_game_id().AppID() || nConsumerAppID != settings->get_local_game_id().AppID()) return k_UGCQueryHandleInvalid;
    if (eQueryType < 0) return k_UGCQueryHandleInvalid;
    
    // TODO
    return new_ugc_query();
}

// Query for the details of the given published file ids (the RequestUGCDetails call is deprecated and replaced with this)
UGCQueryHandle_t CreateQueryUGCDetailsRequest( PublishedFileId_t *pvecPublishedFileID, uint32 unNumPublishedFileIDs )
{
    PRINT_DEBUG("Steam_UGC::CreateQueryUGCDetailsRequest %p, max file IDs = [%u]\n", pvecPublishedFileID, unNumPublishedFileIDs);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    if (!pvecPublishedFileID) return k_UGCQueryHandleInvalid;
    if (unNumPublishedFileIDs < 1) return k_UGCQueryHandleInvalid;

    // TODO
    std::set<PublishedFileId_t> only(pvecPublishedFileID, pvecPublishedFileID + unNumPublishedFileIDs);
    
#ifndef EMU_RELEASE_BUILD
    for (const auto &id : only) {
        PRINT_DEBUG("  Steam_UGC::CreateQueryUGCDetailsRequest file ID = %llu\n", id);
    }
#endif

    return new_ugc_query(false, only);
}


// Send the query to Steam
STEAM_CALL_RESULT( SteamUGCQueryCompleted_t )
SteamAPICall_t SendQueryUGCRequest( UGCQueryHandle_t handle )
{
    PRINT_DEBUG("Steam_UGC::SendQueryUGCRequest %llu\n", handle);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return k_uAPICallInvalid;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request)
        return k_uAPICallInvalid;

    if (request->return_all_subscribed) {
        request->results = std::set<PublishedFileId_t>(ugc_bridge->subbed_mods_itr_begin(), ugc_bridge->subbed_mods_itr_end());
    }

    if (request->return_only.size()) {
        for (auto & s : request->return_only) {
            if (ugc_bridge->has_subbed_mod(s)) {
                request->results.insert(s);
            }
        }
    }

    // send these handles to steam_remote_storage since the game will later
    // call Steam_Remote_Storage::UGCDownload() with these files handles (primary + preview)
    for (auto fileid : request->results) {
        auto mod = settings->getMod(fileid);
        ugc_bridge->add_ugc_query_result(mod.handleFile, fileid, true);
        ugc_bridge->add_ugc_query_result(mod.handlePreviewFile, fileid, false);
    }

    SteamUGCQueryCompleted_t data = {};
    data.m_handle = handle;
    data.m_eResult = k_EResultOK;
    data.m_unNumResultsReturned = request->results.size();
    data.m_unTotalMatchingResults = request->results.size();
    data.m_bCachedData = false;
    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}


// Retrieve an individual result after receiving the callback for querying UGC
bool GetQueryUGCResult( UGCQueryHandle_t handle, uint32 index, SteamUGCDetails_t *pDetails )
{
    PRINT_DEBUG("Steam_UGC::GetQueryUGCResult %llu %u %p\n", handle, index, pDetails);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) {
        return false;
    }

    if (index >= request->results.size()) {
        return false;
    }

    auto it = request->results.begin();
    std::advance(it, index);
    PublishedFileId_t file_id = *it;
    set_details(file_id, pDetails);
    return true;
}

std::optional<Mod_entry> get_query_ugc(UGCQueryHandle_t handle, uint32 index) {
    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return std::nullopt;
    if (index >= request->results.size()) return std::nullopt;

    auto it = request->results.begin();
    std::advance(it, index);
    
    PublishedFileId_t file_id = *it;
    if (!settings->isModInstalled(file_id)) return std::nullopt;

    return settings->getMod(file_id);
}

std::optional<std::vector<std::string>> get_query_ugc_tags(UGCQueryHandle_t handle, uint32 index) {
    auto res = get_query_ugc(handle, index);
    if (!res.has_value()) return std::nullopt;

    auto tags_tokens = std::vector<std::string>{};
    std::stringstream ss(res.value().tags);
    std::string tmp{};
    while(ss >> tmp) {
        if (tmp.back() == ',') tmp = tmp.substr(0, tmp.size() - 1);
        tags_tokens.push_back(tmp);
    }

    return tags_tokens;

}

std::optional<std::string> get_query_ugc_tag(UGCQueryHandle_t handle, uint32 index, uint32 indexTag) {
    auto res = get_query_ugc_tags(handle, index);
    if (!res.has_value()) return std::nullopt;
    if (indexTag >= res.value().size()) return std::nullopt;

    std::string tmp = res.value()[indexTag];
    if (tmp.back() == ',') {
        tmp = tmp.substr(0, tmp.size() - 1);
    }
    return tmp;
}

uint32 GetQueryUGCNumTags( UGCQueryHandle_t handle, uint32 index )
{
    PRINT_DEBUG("TODO Steam_UGC::GetQueryUGCNumTags\n");
    // TODO is this correct?
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return 0;
    
    auto res = get_query_ugc_tags(handle, index);
    return res.has_value() ? res.value().size() : 0;
}

bool GetQueryUGCTag( UGCQueryHandle_t handle, uint32 index, uint32 indexTag, STEAM_OUT_STRING_COUNT( cchValueSize ) char* pchValue, uint32 cchValueSize )
{
    PRINT_DEBUG("TODO Steam_UGC::GetQueryUGCTag\n");
    // TODO is this correct?
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;
    if (!pchValue || !cchValueSize) return false;

    auto res = get_query_ugc_tag(handle, index, indexTag);
    if (!res.has_value()) return false;

    memset(pchValue, 0, cchValueSize);
    res.value().copy(pchValue, cchValueSize - 1);
    return true;
}

bool GetQueryUGCTagDisplayName( UGCQueryHandle_t handle, uint32 index, uint32 indexTag, STEAM_OUT_STRING_COUNT( cchValueSize ) char* pchValue, uint32 cchValueSize )
{
    PRINT_DEBUG("TODO Steam_UGC::GetQueryUGCTagDisplayName\n");
    // TODO is this correct?
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;
    if (!pchValue || !cchValueSize) return false;

    auto res = get_query_ugc_tag(handle, index, indexTag);
    if (!res.has_value()) return false;

    memset(pchValue, 0, cchValueSize);
    res.value().copy(pchValue, cchValueSize - 1);
    return true;
}

bool GetQueryUGCPreviewURL( UGCQueryHandle_t handle, uint32 index, STEAM_OUT_STRING_COUNT(cchURLSize) char *pchURL, uint32 cchURLSize )
{
    PRINT_DEBUG("Steam_UGC::GetQueryUGCPreviewURL\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    //TODO: escape simulator tries downloading this url and unsubscribes if it fails
    if (handle == k_UGCQueryHandleInvalid) return false;
    if (!pchURL || !cchURLSize) return false;

    auto res = get_query_ugc(handle, index);
    if (!res.has_value()) return false;

    auto mod = res.value();
    PRINT_DEBUG("Steam_UGC:GetQueryUGCPreviewURL: '%s'\n", mod.previewURL.c_str());
    mod.previewURL.copy(pchURL, cchURLSize - 1);
    return true;
}


bool GetQueryUGCMetadata( UGCQueryHandle_t handle, uint32 index, STEAM_OUT_STRING_COUNT(cchMetadatasize) char *pchMetadata, uint32 cchMetadatasize )
{
    PRINT_DEBUG("TODO Steam_UGC::GetQueryUGCMetadata\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;

    return false;
}


bool GetQueryUGCChildren( UGCQueryHandle_t handle, uint32 index, PublishedFileId_t* pvecPublishedFileID, uint32 cMaxEntries )
{
    PRINT_DEBUG("TODO Steam_UGC::GetQueryUGCChildren\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return false;
}


bool GetQueryUGCStatistic( UGCQueryHandle_t handle, uint32 index, EItemStatistic eStatType, uint64 *pStatValue )
{
    PRINT_DEBUG("TODO Steam_UGC::GetQueryUGCStatistic\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return false;
}

bool GetQueryUGCStatistic( UGCQueryHandle_t handle, uint32 index, EItemStatistic eStatType, uint32 *pStatValue )
{
    PRINT_DEBUG("TODO Steam_UGC::GetQueryUGCStatistic old\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return false;
}

uint32 GetQueryUGCNumAdditionalPreviews( UGCQueryHandle_t handle, uint32 index )
{
    PRINT_DEBUG("TODO Steam_UGC::GetQueryUGCNumAdditionalPreviews\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return 0;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return 0;
    
    return 0;
}


bool GetQueryUGCAdditionalPreview( UGCQueryHandle_t handle, uint32 index, uint32 previewIndex, STEAM_OUT_STRING_COUNT(cchURLSize) char *pchURLOrVideoID, uint32 cchURLSize, STEAM_OUT_STRING_COUNT(cchURLSize) char *pchOriginalFileName, uint32 cchOriginalFileNameSize, EItemPreviewType *pPreviewType )
{
    PRINT_DEBUG("TODO Steam_UGC::GetQueryUGCAdditionalPreview\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return false;
}

bool GetQueryUGCAdditionalPreview( UGCQueryHandle_t handle, uint32 index, uint32 previewIndex, char *pchURLOrVideoID, uint32 cchURLSize, bool *hz )
{
    PRINT_DEBUG("TODO Steam_UGC::GetQueryUGCAdditionalPreview old\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return false;
}

uint32 GetQueryUGCNumKeyValueTags( UGCQueryHandle_t handle, uint32 index )
{
    PRINT_DEBUG("TODO Steam_UGC::GetQueryUGCNumKeyValueTags\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return 0;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return 0;
    
    return 0;
}


bool GetQueryUGCKeyValueTag( UGCQueryHandle_t handle, uint32 index, uint32 keyValueTagIndex, STEAM_OUT_STRING_COUNT(cchKeySize) char *pchKey, uint32 cchKeySize, STEAM_OUT_STRING_COUNT(cchValueSize) char *pchValue, uint32 cchValueSize )
{
    PRINT_DEBUG("TODO Steam_UGC::GetQueryUGCKeyValueTag\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return false;
}

bool GetQueryUGCKeyValueTag( UGCQueryHandle_t handle, uint32 index, const char *pchKey, STEAM_OUT_STRING_COUNT(cchValueSize) char *pchValue, uint32 cchValueSize )
{
    PRINT_DEBUG("TODO Steam_UGC::GetQueryUGCKeyValueTag2\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return false;
}

uint32 GetQueryUGCContentDescriptors( UGCQueryHandle_t handle, uint32 index, EUGCContentDescriptorID *pvecDescriptors, uint32 cMaxEntries )
{
    PRINT_DEBUG("TODO Steam_UGC::GetQueryUGCContentDescriptors\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return 0;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return 0;
    
    return 0;
}

// Release the request to free up memory, after retrieving results
bool ReleaseQueryUGCRequest( UGCQueryHandle_t handle )
{
    PRINT_DEBUG("Steam_UGC::ReleaseQueryUGCRequest %llu\n", handle);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;

    ugc_queries.erase(request);
    return true;
}


// Options to set for querying UGC
bool AddRequiredTag( UGCQueryHandle_t handle, const char *pTagName )
{
    PRINT_DEBUG("TODO Steam_UGC::AddRequiredTag\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}

bool AddRequiredTagGroup( UGCQueryHandle_t handle, const SteamParamStringArray_t *pTagGroups )
{
    PRINT_DEBUG("TODO Steam_UGC::AddRequiredTagGroup\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}

bool AddExcludedTag( UGCQueryHandle_t handle, const char *pTagName )
{
    PRINT_DEBUG("TODO Steam_UGC::AddExcludedTag\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}


bool SetReturnOnlyIDs( UGCQueryHandle_t handle, bool bReturnOnlyIDs )
{
    PRINT_DEBUG("TODO Steam_UGC::SetReturnOnlyIDs\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}


bool SetReturnKeyValueTags( UGCQueryHandle_t handle, bool bReturnKeyValueTags )
{
    PRINT_DEBUG("TODO Steam_UGC::SetReturnKeyValueTags\n");
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}


bool SetReturnLongDescription( UGCQueryHandle_t handle, bool bReturnLongDescription )
{
    PRINT_DEBUG("TODO Steam_UGC::SetReturnLongDescription\n");
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}


bool SetReturnMetadata( UGCQueryHandle_t handle, bool bReturnMetadata )
{
    PRINT_DEBUG("TODO Steam_UGC::SetReturnMetadata %i\n", (int)bReturnMetadata);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}


bool SetReturnChildren( UGCQueryHandle_t handle, bool bReturnChildren )
{
    PRINT_DEBUG("TODO Steam_UGC::SetReturnChildren\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}


bool SetReturnAdditionalPreviews( UGCQueryHandle_t handle, bool bReturnAdditionalPreviews )
{
    PRINT_DEBUG("TODO Steam_UGC::SetReturnAdditionalPreviews\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}


bool SetReturnTotalOnly( UGCQueryHandle_t handle, bool bReturnTotalOnly )
{
    PRINT_DEBUG("TODO Steam_UGC::SetReturnTotalOnly\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}


bool SetReturnPlaytimeStats( UGCQueryHandle_t handle, uint32 unDays )
{
    PRINT_DEBUG("TODO Steam_UGC::SetReturnPlaytimeStats\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}


bool SetLanguage( UGCQueryHandle_t handle, const char *pchLanguage )
{
    PRINT_DEBUG("TODO Steam_UGC::SetLanguage\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}


bool SetAllowCachedResponse( UGCQueryHandle_t handle, uint32 unMaxAgeSeconds )
{
    PRINT_DEBUG("TODO Steam_UGC::SetAllowCachedResponse\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}


// Options only for querying user UGC
bool SetCloudFileNameFilter( UGCQueryHandle_t handle, const char *pMatchCloudFileName )
{
    PRINT_DEBUG("TODO Steam_UGC::SetCloudFileNameFilter\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}


// Options only for querying all UGC
bool SetMatchAnyTag( UGCQueryHandle_t handle, bool bMatchAnyTag )
{
    PRINT_DEBUG("TODO Steam_UGC::SetMatchAnyTag\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}


bool SetSearchText( UGCQueryHandle_t handle, const char *pSearchText )
{
    PRINT_DEBUG("TODO Steam_UGC::SetSearchText\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}


bool SetRankedByTrendDays( UGCQueryHandle_t handle, uint32 unDays )
{
    PRINT_DEBUG("TODO Steam_UGC::SetRankedByTrendDays\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}


bool AddRequiredKeyValueTag( UGCQueryHandle_t handle, const char *pKey, const char *pValue )
{
    PRINT_DEBUG("TODO Steam_UGC::AddRequiredKeyValueTag\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}

bool SetTimeCreatedDateRange( UGCQueryHandle_t handle, RTime32 rtStart, RTime32 rtEnd )
{
    PRINT_DEBUG("TODO Steam_UGC::SetTimeCreatedDateRange\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}

bool SetTimeUpdatedDateRange( UGCQueryHandle_t handle, RTime32 rtStart, RTime32 rtEnd )
{
    PRINT_DEBUG("TODO Steam_UGC::SetTimeUpdatedDateRange\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}

// DEPRECATED - Use CreateQueryUGCDetailsRequest call above instead!
SteamAPICall_t RequestUGCDetails( PublishedFileId_t nPublishedFileID, uint32 unMaxAgeSeconds )
{
    PRINT_DEBUG("Steam_UGC::RequestUGCDetails %llu\n", nPublishedFileID);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    SteamUGCRequestUGCDetailsResult_t data{};
    data.m_bCachedData = false;
    set_details(nPublishedFileID, &(data.m_details));
    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}

SteamAPICall_t RequestUGCDetails( PublishedFileId_t nPublishedFileID )
{
    PRINT_DEBUG("Steam_UGC::RequestUGCDetails old\n");
    return RequestUGCDetails(nPublishedFileID, 0);
}


// Steam Workshop Creator API
STEAM_CALL_RESULT( CreateItemResult_t )
SteamAPICall_t CreateItem( AppId_t nConsumerAppId, EWorkshopFileType eFileType )
{
    PRINT_DEBUG("Steam_UGC::CreateItem\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return k_uAPICallInvalid;
}
 // create new item for this app with no content attached yet


UGCUpdateHandle_t StartItemUpdate( AppId_t nConsumerAppId, PublishedFileId_t nPublishedFileID )
{
    PRINT_DEBUG("Steam_UGC::StartItemUpdate\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return k_UGCUpdateHandleInvalid;
}
 // start an UGC item update. Set changed properties before commiting update with CommitItemUpdate()


bool SetItemTitle( UGCUpdateHandle_t handle, const char *pchTitle )
{
    PRINT_DEBUG("Steam_UGC::SetItemTitle\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}
 // change the title of an UGC item


bool SetItemDescription( UGCUpdateHandle_t handle, const char *pchDescription )
{
    PRINT_DEBUG("Steam_UGC::SetItemDescription\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}
 // change the description of an UGC item


bool SetItemUpdateLanguage( UGCUpdateHandle_t handle, const char *pchLanguage )
{
    PRINT_DEBUG("Steam_UGC::SetItemUpdateLanguage\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}
 // specify the language of the title or description that will be set


bool SetItemMetadata( UGCUpdateHandle_t handle, const char *pchMetaData )
{
    PRINT_DEBUG("Steam_UGC::SetItemMetadata\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}
 // change the metadata of an UGC item (max = k_cchDeveloperMetadataMax)


bool SetItemVisibility( UGCUpdateHandle_t handle, ERemoteStoragePublishedFileVisibility eVisibility )
{
    PRINT_DEBUG("Steam_UGC::SetItemVisibility\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}
 // change the visibility of an UGC item


bool SetItemTags( UGCUpdateHandle_t updateHandle, const SteamParamStringArray_t *pTags )
{
    PRINT_DEBUG("Steam_UGC::SetItemTags old\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}

bool SetItemTags( UGCUpdateHandle_t updateHandle, const SteamParamStringArray_t *pTags, bool bAllowAdminTags )
{
    PRINT_DEBUG("Steam_UGC::SetItemTags\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}
 // change the tags of an UGC item

bool SetItemContent( UGCUpdateHandle_t handle, const char *pszContentFolder )
{
    PRINT_DEBUG("Steam_UGC::SetItemContent\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}
 // update item content from this local folder


bool SetItemPreview( UGCUpdateHandle_t handle, const char *pszPreviewFile )
{
    PRINT_DEBUG("Steam_UGC::SetItemPreview\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}
 //  change preview image file for this item. pszPreviewFile points to local image file, which must be under 1MB in size

bool SetAllowLegacyUpload( UGCUpdateHandle_t handle, bool bAllowLegacyUpload )
{
    PRINT_DEBUG("Steam_UGC::SetAllowLegacyUpload\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}

bool RemoveAllItemKeyValueTags( UGCUpdateHandle_t handle )
{
    PRINT_DEBUG("Steam_UGC::RemoveAllItemKeyValueTags\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}
 // remove all existing key-value tags (you can add new ones via the AddItemKeyValueTag function)

bool RemoveItemKeyValueTags( UGCUpdateHandle_t handle, const char *pchKey )
{
    PRINT_DEBUG("Steam_UGC::RemoveItemKeyValueTags\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}
 // remove any existing key-value tags with the specified key


bool AddItemKeyValueTag( UGCUpdateHandle_t handle, const char *pchKey, const char *pchValue )
{
    PRINT_DEBUG("Steam_UGC::AddItemKeyValueTag\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}
 // add new key-value tags for the item. Note that there can be multiple values for a tag.


bool AddItemPreviewFile( UGCUpdateHandle_t handle, const char *pszPreviewFile, EItemPreviewType type )
{
    PRINT_DEBUG("Steam_UGC::AddItemPreviewFile\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}
 //  add preview file for this item. pszPreviewFile points to local file, which must be under 1MB in size


bool AddItemPreviewVideo( UGCUpdateHandle_t handle, const char *pszVideoID )
{
    PRINT_DEBUG("Steam_UGC::AddItemPreviewVideo\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}
 //  add preview video for this item


bool UpdateItemPreviewFile( UGCUpdateHandle_t handle, uint32 index, const char *pszPreviewFile )
{
    PRINT_DEBUG("Steam_UGC::UpdateItemPreviewFile\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}
 //  updates an existing preview file for this item. pszPreviewFile points to local file, which must be under 1MB in size


bool UpdateItemPreviewVideo( UGCUpdateHandle_t handle, uint32 index, const char *pszVideoID )
{
    PRINT_DEBUG("Steam_UGC::UpdateItemPreviewVideo\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}
 //  updates an existing preview video for this item


bool RemoveItemPreview( UGCUpdateHandle_t handle, uint32 index )
{
    PRINT_DEBUG("Steam_UGC::RemoveItemPreview %llu %u\n", handle, index);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}
 // remove a preview by index starting at 0 (previews are sorted)

bool AddContentDescriptor( UGCUpdateHandle_t handle, EUGCContentDescriptorID descid )
{
    PRINT_DEBUG("Steam_UGC::AddContentDescriptor %llu %u\n", handle, descid);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}

bool RemoveContentDescriptor( UGCUpdateHandle_t handle, EUGCContentDescriptorID descid )
{
    PRINT_DEBUG("Steam_UGC::RemoveContentDescriptor %llu %u\n", handle, descid);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}

STEAM_CALL_RESULT( SubmitItemUpdateResult_t )
SteamAPICall_t SubmitItemUpdate( UGCUpdateHandle_t handle, const char *pchChangeNote )
{
    PRINT_DEBUG("Steam_UGC::SubmitItemUpdate\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return k_uAPICallInvalid;
}
 // commit update process started with StartItemUpdate()


EItemUpdateStatus GetItemUpdateProgress( UGCUpdateHandle_t handle, uint64 *punBytesProcessed, uint64* punBytesTotal )
{
    PRINT_DEBUG("Steam_UGC::GetItemUpdateProgress\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return k_EItemUpdateStatusInvalid;
}


// Steam Workshop Consumer API

STEAM_CALL_RESULT( SetUserItemVoteResult_t )
SteamAPICall_t SetUserItemVote( PublishedFileId_t nPublishedFileID, bool bVoteUp )
{
    PRINT_DEBUG("Steam_UGC::SetUserItemVote\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!settings->isModInstalled(nPublishedFileID)) return k_uAPICallInvalid; // TODO is this correct
    
    auto mod  = settings->getMod(nPublishedFileID);
    SetUserItemVoteResult_t data{};
    data.m_eResult = EResult::k_EResultOK;
    data.m_nPublishedFileId = nPublishedFileID;
    if (bVoteUp) {
        ++mod.votesUp;
    } else {
        ++mod.votesDown;
    }
    settings->addModDetails(nPublishedFileID, mod);
    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}


STEAM_CALL_RESULT( GetUserItemVoteResult_t )
SteamAPICall_t GetUserItemVote( PublishedFileId_t nPublishedFileID )
{
    PRINT_DEBUG("Steam_UGC::GetUserItemVote\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (nPublishedFileID == k_PublishedFileIdInvalid || !settings->isModInstalled(nPublishedFileID)) return k_uAPICallInvalid; // TODO is this correct

    auto mod  = settings->getMod(nPublishedFileID);
    GetUserItemVoteResult_t data{};
    data.m_eResult = EResult::k_EResultOK;
    data.m_nPublishedFileId = nPublishedFileID;
    data.m_bVotedDown = mod.votesDown;
    data.m_bVotedUp = mod.votesUp;
    data.m_bVoteSkipped = true;
    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}


STEAM_CALL_RESULT( UserFavoriteItemsListChanged_t )
SteamAPICall_t AddItemToFavorites( AppId_t nAppId, PublishedFileId_t nPublishedFileID )
{
    PRINT_DEBUG("Steam_UGC::AddItemToFavorites %u %llu\n", nAppId, nPublishedFileID);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (nAppId == k_uAppIdInvalid || nAppId != settings->get_local_game_id().AppID()) return k_uAPICallInvalid; // TODO is this correct
    if (nPublishedFileID == k_PublishedFileIdInvalid || !settings->isModInstalled(nPublishedFileID)) return k_uAPICallInvalid; // TODO is this correct

    UserFavoriteItemsListChanged_t data{};
    data.m_nPublishedFileId = nPublishedFileID;
    data.m_bWasAddRequest = true;

    auto add = favorites.insert(nPublishedFileID);
    if (add.second) { // if new insertion
        PRINT_DEBUG(" adding new item to favorites");
        bool ok = write_ugc_favorites();
        data.m_eResult = ok ? EResult::k_EResultOK : EResult::k_EResultFail;
    } else { // nPublishedFileID already exists
        data.m_eResult = EResult::k_EResultOK;
    }
    
    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}


STEAM_CALL_RESULT( UserFavoriteItemsListChanged_t )
SteamAPICall_t RemoveItemFromFavorites( AppId_t nAppId, PublishedFileId_t nPublishedFileID )
{
    PRINT_DEBUG("Steam_UGC::RemoveItemFromFavorites\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (nAppId == k_uAppIdInvalid || nAppId != settings->get_local_game_id().AppID()) return k_uAPICallInvalid; // TODO is this correct
    if (nPublishedFileID == k_PublishedFileIdInvalid || !settings->isModInstalled(nPublishedFileID)) return k_uAPICallInvalid; // TODO is this correct

    UserFavoriteItemsListChanged_t data{};
    data.m_nPublishedFileId = nPublishedFileID;
    data.m_bWasAddRequest = false;

    auto removed = favorites.erase(nPublishedFileID);
    if (removed) {
        PRINT_DEBUG(" removing item from favorites");
        bool ok = write_ugc_favorites();
        data.m_eResult = ok ? EResult::k_EResultOK : EResult::k_EResultFail;
    } else { // nPublishedFileID didn't exist
        data.m_eResult = EResult::k_EResultOK;
    }
    
    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}


STEAM_CALL_RESULT( RemoteStorageSubscribePublishedFileResult_t )
SteamAPICall_t SubscribeItem( PublishedFileId_t nPublishedFileID )
{
    PRINT_DEBUG("Steam_UGC::SubscribeItem %llu\n", nPublishedFileID);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    RemoteStorageSubscribePublishedFileResult_t data;
    data.m_nPublishedFileId = nPublishedFileID;
    if (settings->isModInstalled(nPublishedFileID)) {
        data.m_eResult = k_EResultOK;
        ugc_bridge->add_subbed_mod(nPublishedFileID);
    } else {
        data.m_eResult = k_EResultFail;
    }
    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}
 // subscribe to this item, will be installed ASAP

STEAM_CALL_RESULT( RemoteStorageUnsubscribePublishedFileResult_t )
SteamAPICall_t UnsubscribeItem( PublishedFileId_t nPublishedFileID )
{
    PRINT_DEBUG("Steam_UGC::UnsubscribeItem %llu\n", nPublishedFileID);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    RemoteStorageUnsubscribePublishedFileResult_t data;
    data.m_nPublishedFileId = nPublishedFileID;
    if (!ugc_bridge->has_subbed_mod(nPublishedFileID)) {
        data.m_eResult = k_EResultFail; //TODO: check if this is accurate
    } else {
        data.m_eResult = k_EResultOK;
        ugc_bridge->remove_subbed_mod(nPublishedFileID);
    }

    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}
 // unsubscribe from this item, will be uninstalled after game quits

uint32 GetNumSubscribedItems()
{
    PRINT_DEBUG("Steam_UGC::GetNumSubscribedItems\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    PRINT_DEBUG("  Steam_UGC::GetNumSubscribedItems = %zu\n", ugc_bridge->subbed_mods_count());
    return (uint32)ugc_bridge->subbed_mods_count();
}
 // number of subscribed items 

uint32 GetSubscribedItems( PublishedFileId_t* pvecPublishedFileID, uint32 cMaxEntries )
{
    PRINT_DEBUG("Steam_UGC::GetSubscribedItems %p %u\n", pvecPublishedFileID, cMaxEntries);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if ((size_t)cMaxEntries > ugc_bridge->subbed_mods_count()) {
        cMaxEntries = (uint32)ugc_bridge->subbed_mods_count();
    }

    std::copy_n(ugc_bridge->subbed_mods_itr_begin(), cMaxEntries, pvecPublishedFileID);
    return cMaxEntries;
}
 // all subscribed item PublishFileIDs

// get EItemState flags about item on this client
uint32 GetItemState( PublishedFileId_t nPublishedFileID )
{
    PRINT_DEBUG("Steam_UGC::GetItemState %llu\n", nPublishedFileID);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (ugc_bridge->has_subbed_mod(nPublishedFileID)) {
        if (settings->isModInstalled(nPublishedFileID)) {
            PRINT_DEBUG("  mod is subscribed and installed\n");
            return k_EItemStateInstalled | k_EItemStateSubscribed;
        }

        PRINT_DEBUG("  mod is subscribed\n");
        return k_EItemStateSubscribed;
    }

    PRINT_DEBUG("  mod isn't found\n");
    return k_EItemStateNone;
}


// get info about currently installed content on disc for items that have k_EItemStateInstalled set
// if k_EItemStateLegacyItem is set, pchFolder contains the path to the legacy file itself (not a folder)
bool GetItemInstallInfo( PublishedFileId_t nPublishedFileID, uint64 *punSizeOnDisk, STEAM_OUT_STRING_COUNT( cchFolderSize ) char *pchFolder, uint32 cchFolderSize, uint32 *punTimeStamp )
{
    PRINT_DEBUG("Steam_UGC::GetItemInstallInfo %llu %p %p [%u] %p\n", nPublishedFileID, punSizeOnDisk, pchFolder, cchFolderSize, punTimeStamp);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!cchFolderSize) return false;
    if (!settings->isModInstalled(nPublishedFileID)) return false;

    auto mod = settings->getMod(nPublishedFileID);
    
    // I don't know if this is accurate behavior, but to avoid returning true with invalid data
    if ((cchFolderSize - 1) < mod.path.size()) { // -1 because the last char is reserved for null terminator
        PRINT_DEBUG("  ERROR mod path: '%s' [%zu bytes] cannot fit into the given buffer\n", mod.path.c_str(), mod.path.size());
        return false;
    }

    if (punSizeOnDisk) *punSizeOnDisk = mod.primaryFileSize;
    if (punTimeStamp) *punTimeStamp = mod.timeUpdated;
    if (pchFolder && cchFolderSize) {
        // human fall flat doesn't send a nulled buffer, and won't recognize the proper mod path because of that
        memset(pchFolder, 0, cchFolderSize);
        mod.path.copy(pchFolder, cchFolderSize - 1);
        PRINT_DEBUG("  copied mod path: '%s'\n", pchFolder);
    }

    return true;
}


// get info about pending update for items that have k_EItemStateNeedsUpdate set. punBytesTotal will be valid after download started once
bool GetItemDownloadInfo( PublishedFileId_t nPublishedFileID, uint64 *punBytesDownloaded, uint64 *punBytesTotal )
{
    PRINT_DEBUG("Steam_UGC::GetItemDownloadInfo %llu\n", nPublishedFileID);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!settings->isModInstalled(nPublishedFileID)) return false;

    auto mod = settings->getMod(nPublishedFileID);
    if (punBytesDownloaded) *punBytesDownloaded = mod.primaryFileSize;
    if (punBytesTotal) *punBytesTotal = mod.primaryFileSize;
    return true;
}

bool GetItemInstallInfo( PublishedFileId_t nPublishedFileID, uint64 *punSizeOnDisk, STEAM_OUT_STRING_COUNT( cchFolderSize ) char *pchFolder, uint32 cchFolderSize, bool *pbLegacyItem ) // returns true if item is installed
{
    PRINT_DEBUG("Steam_UGC::GetItemInstallInfo old\n");
    return GetItemInstallInfo(nPublishedFileID, punSizeOnDisk, pchFolder, cchFolderSize, (uint32*) nullptr);
}

bool GetItemUpdateInfo( PublishedFileId_t nPublishedFileID, bool *pbNeedsUpdate, bool *pbIsDownloading, uint64 *punBytesDownloaded, uint64 *punBytesTotal )
{
    PRINT_DEBUG("Steam_UGC::GetItemDownloadInfo old\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    bool res = GetItemDownloadInfo(nPublishedFileID, punBytesDownloaded, punBytesTotal);
    if (res) {
        if (pbNeedsUpdate) *pbNeedsUpdate = false;
        if (pbIsDownloading) *pbIsDownloading = false;
    }
    return res;
}

bool GetItemInstallInfo( PublishedFileId_t nPublishedFileID, uint64 *punSizeOnDisk, char *pchFolder, uint32 cchFolderSize ) // returns true if item is installed
{
    PRINT_DEBUG("Steam_UGC::GetItemInstallInfo older\n");
    return GetItemInstallInfo(nPublishedFileID, punSizeOnDisk, pchFolder, cchFolderSize, (uint32*) nullptr);
}


// download new or update already installed item. If function returns true, wait for DownloadItemResult_t. If the item is already installed,
// then files on disk should not be used until callback received. If item is not subscribed to, it will be cached for some time.
// If bHighPriority is set, any other item download will be suspended and this item downloaded ASAP.
bool DownloadItem( PublishedFileId_t nPublishedFileID, bool bHighPriority )
{
    PRINT_DEBUG("Steam_UGC::DownloadItem\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    return false;
}


// game servers can set a specific workshop folder before issuing any UGC commands.
// This is helpful if you want to support multiple game servers running out of the same install folder
bool BInitWorkshopForGameServer( DepotId_t unWorkshopDepotID, const char *pszFolder )
{
    PRINT_DEBUG("Steam_UGC::BInitWorkshopForGameServer\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}


// SuspendDownloads( true ) will suspend all workshop downloads until SuspendDownloads( false ) is called or the game ends
void SuspendDownloads( bool bSuspend )
{
    PRINT_DEBUG("Steam_UGC::SuspendDownloads\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
}


// usage tracking
STEAM_CALL_RESULT( StartPlaytimeTrackingResult_t )
SteamAPICall_t StartPlaytimeTracking( PublishedFileId_t *pvecPublishedFileID, uint32 unNumPublishedFileIDs )
{
    PRINT_DEBUG("Steam_UGC::StartPlaytimeTracking\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    StopPlaytimeTrackingResult_t data;
    data.m_eResult = k_EResultOK;
    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}

STEAM_CALL_RESULT( StopPlaytimeTrackingResult_t )
SteamAPICall_t StopPlaytimeTracking( PublishedFileId_t *pvecPublishedFileID, uint32 unNumPublishedFileIDs )
{
    PRINT_DEBUG("Steam_UGC::StopPlaytimeTracking\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    StopPlaytimeTrackingResult_t data;
    data.m_eResult = k_EResultOK;
    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}

STEAM_CALL_RESULT( StopPlaytimeTrackingResult_t )
SteamAPICall_t StopPlaytimeTrackingForAllItems()
{
    PRINT_DEBUG("Steam_UGC::StopPlaytimeTrackingForAllItems\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    StopPlaytimeTrackingResult_t data;
    data.m_eResult = k_EResultOK;
    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}


// parent-child relationship or dependency management
STEAM_CALL_RESULT( AddUGCDependencyResult_t )
SteamAPICall_t AddDependency( PublishedFileId_t nParentPublishedFileID, PublishedFileId_t nChildPublishedFileID )
{
    PRINT_DEBUG("Steam_UGC::AddDependency\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    if (nParentPublishedFileID == k_PublishedFileIdInvalid) return k_uAPICallInvalid;
    
    return k_uAPICallInvalid;
}

STEAM_CALL_RESULT( RemoveUGCDependencyResult_t )
SteamAPICall_t RemoveDependency( PublishedFileId_t nParentPublishedFileID, PublishedFileId_t nChildPublishedFileID )
{
    PRINT_DEBUG("Steam_UGC::RemoveDependency\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    if (nParentPublishedFileID == k_PublishedFileIdInvalid) return k_uAPICallInvalid;
    
    return k_uAPICallInvalid;
}


// add/remove app dependence/requirements (usually DLC)
STEAM_CALL_RESULT( AddAppDependencyResult_t )
SteamAPICall_t AddAppDependency( PublishedFileId_t nPublishedFileID, AppId_t nAppID )
{
    PRINT_DEBUG("Steam_UGC::AddAppDependency\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    if (nPublishedFileID == k_PublishedFileIdInvalid) return k_uAPICallInvalid;
    
    return k_uAPICallInvalid;
}

STEAM_CALL_RESULT( RemoveAppDependencyResult_t )
SteamAPICall_t RemoveAppDependency( PublishedFileId_t nPublishedFileID, AppId_t nAppID )
{
    PRINT_DEBUG("Steam_UGC::RemoveAppDependency\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    if (nPublishedFileID == k_PublishedFileIdInvalid) return k_uAPICallInvalid;
    
    return k_uAPICallInvalid;
}

// request app dependencies. note that whatever callback you register for GetAppDependenciesResult_t may be called multiple times
// until all app dependencies have been returned
STEAM_CALL_RESULT( GetAppDependenciesResult_t )
SteamAPICall_t GetAppDependencies( PublishedFileId_t nPublishedFileID )
{
    PRINT_DEBUG("Steam_UGC::GetAppDependencies\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    if (nPublishedFileID == k_PublishedFileIdInvalid) return k_uAPICallInvalid;
    
    return k_uAPICallInvalid;
}


// delete the item without prompting the user
STEAM_CALL_RESULT( DeleteItemResult_t )
SteamAPICall_t DeleteItem( PublishedFileId_t nPublishedFileID )
{
    PRINT_DEBUG("Steam_UGC::DeleteItem\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    if (nPublishedFileID == k_PublishedFileIdInvalid) return k_uAPICallInvalid;
    
    return k_uAPICallInvalid;
}

// Show the app's latest Workshop EULA to the user in an overlay window, where they can accept it or not
bool ShowWorkshopEULA()
{
    PRINT_DEBUG("ShowWorkshopEULA\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}

// Retrieve information related to the user's acceptance or not of the app's specific Workshop EULA
STEAM_CALL_RESULT( WorkshopEULAStatus_t )
SteamAPICall_t GetWorkshopEULAStatus()
{
    PRINT_DEBUG("GetWorkshopEULAStatus\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return k_uAPICallInvalid;
}

// Return the user's community content descriptor preferences
uint32 GetUserContentDescriptorPreferences( EUGCContentDescriptorID *pvecDescriptors, uint32 cMaxEntries )
{
    PRINT_DEBUG("GetWorkshopEULAStatus\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return 0;
}

};
