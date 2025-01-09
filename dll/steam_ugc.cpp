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

#include "dll/steam_ugc.h"

UGCQueryHandle_t Steam_UGC::new_ugc_query(EQueryType query_type, bool return_all_subscribed, uint32 page, bool next_cursor, const std::set<PublishedFileId_t> &return_only)
{
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    ++handle;
    if ((handle == 0) || (handle == k_UGCQueryHandleInvalid)) handle = 50;

    struct UGC_query query{};
    query.handle = handle;
    query.return_all_subscribed = return_all_subscribed;
    query.page = page;
    query.next_cursor = next_cursor;
    query.query_type = query_type;
    query.return_only = return_only;
    ugc_queries.push_back(query);
    PRINT_DEBUG("new request handle = %llu", query.handle);
    return query.handle;
}

std::optional<Mod_entry> Steam_UGC::get_query_ugc(UGCQueryHandle_t handle, uint32 index)
{
    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return std::nullopt;
    if (index >= request->results.size()) return std::nullopt;

    auto it = request->results.begin();
    std::advance(it, index);
    
    PublishedFileId_t file_id = *it;
    if (!settings->isModInstalled(file_id)) return std::nullopt;

    return settings->getMod(file_id);
}

std::vector<std::string> Steam_UGC::get_query_ugc_tags(UGCQueryHandle_t handle, uint32 index)
{
    auto res = get_query_ugc(handle, index);
    if (!res.has_value()) return {};

    std::string tmp = res.value().tags;

    auto tags_tokens = std::vector<std::string>{};
    size_t start = 0;
    while (true) {
        auto end = tmp.find(',', start);
        if (end == std::string::npos) break;

        tags_tokens.push_back(tmp.substr(start, end - start));
        start = end + 1;
    }

    tags_tokens.push_back(tmp.substr(start));

    return tags_tokens;

}

void Steam_UGC::set_details(PublishedFileId_t id, SteamUGCDetails_t *pDetails, IUgcItfVersion ver)
{
    if (pDetails) {
        pDetails->m_nPublishedFileId = id;

        if (settings->isModInstalled(id)) {
            PRINT_DEBUG("  mod is installed, setting details");
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
            pDetails->m_ulSteamIDOwner = mod.steamIDOwner;

            pDetails->m_rtimeAddedToUserList = mod.timeAddedToUserList;
            pDetails->m_unVotesUp = mod.votesUp;
            pDetails->m_unVotesDown = mod.votesDown;
            pDetails->m_flScore = mod.score;

            // real steamclient64.dll may set this to null! (ex: item id 3366485326)
            auto copied_chars = mod.primaryFileName.copy(pDetails->m_pchFileName, sizeof(pDetails->m_pchFileName) - 1);
            pDetails->m_pchFileName[copied_chars] = 0;

            copied_chars = mod.description.copy(pDetails->m_rgchDescription, sizeof(pDetails->m_rgchDescription) - 1);
            pDetails->m_rgchDescription[copied_chars] = 0;

            copied_chars = mod.tags.copy(pDetails->m_rgchTags, sizeof(pDetails->m_rgchTags) - 1);
            pDetails->m_rgchTags[copied_chars] = 0;

            copied_chars = mod.title.copy(pDetails->m_rgchTitle, sizeof(pDetails->m_rgchTitle) - 1);
            pDetails->m_rgchTitle[copied_chars] = 0;

            // real steamclient64.dll may set this to null! (ex: item id 3366485326)
            copied_chars = mod.workshopItemURL.copy(pDetails->m_rgchURL, sizeof(pDetails->m_rgchURL) - 1);
            pDetails->m_rgchURL[copied_chars] = 0;

            // TODO should we enable this?
            // pDetails->m_unNumChildren = mod.numChildren;

            if (ver >= IUgcItfVersion::v020) {
                // TODO make sure the filesize is good
                pDetails->m_ulTotalFilesSize = mod.total_files_sizes;
            }
        } else {
            PRINT_DEBUG("  mod isn't installed, returning failure");
            pDetails->m_eResult = k_EResultFail;
        }
    }
}

void Steam_UGC::read_ugc_favorites()
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
            PRINT_DEBUG("added item to favorites %llu", fav_id);
        } catch(...) { }
    }
    
}

bool Steam_UGC::write_ugc_favorites()
{
    std::stringstream ss{};
    for (auto id : favorites) {
        ss << id << "\n";
        ss.flush();
    }
    auto file_data = ss.str();
    int stored = local_storage->store_data("", ugc_favorits_file, &file_data[0], static_cast<unsigned int>(file_data.size()));
    return (size_t)stored == file_data.size();
}

bool Steam_UGC::internal_GetQueryUGCResult( UGCQueryHandle_t handle, uint32 index, SteamUGCDetails_t *pDetails, IUgcItfVersion ver )
{
    PRINT_DEBUG("%llu [%u] %p <%u>", handle, index, pDetails, (unsigned)ver);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    // some apps (like appid 588650) ignore the return of this function, especially for builtin mods
    if (pDetails) {
        pDetails->m_nPublishedFileId = k_PublishedFileIdInvalid;
        pDetails->m_eResult = k_EResultFail;
        pDetails->m_bAcceptedForUse = false;
        pDetails->m_hFile = k_UGCHandleInvalid;
        pDetails->m_hPreviewFile = k_UGCHandleInvalid;
        pDetails->m_unNumChildren = 0;
    }

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
    set_details(file_id, pDetails, ver);
    return true;
}

SteamAPICall_t Steam_UGC::internal_RequestUGCDetails( PublishedFileId_t nPublishedFileID, uint32 unMaxAgeSeconds, IUgcItfVersion ver )
{
    PRINT_DEBUG("%llu %u <%u>", nPublishedFileID, unMaxAgeSeconds, (unsigned)ver);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    if (ver <= IUgcItfVersion::v018) { // <= SDK 1.59
        SteamUGCRequestUGCDetailsResult018_t data{};
        data.m_bCachedData = false;
        set_details(nPublishedFileID, reinterpret_cast<SteamUGCDetails_t *>(&data.m_details), ver);
        
        auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
        callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
        return ret;
    } else { // >= SDK 1.60
        SteamUGCRequestUGCDetailsResult_t data{};
        data.m_bCachedData = false;
        set_details(nPublishedFileID, &data.m_details, ver);
        
        auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
        callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
        return ret;
    }
}


Steam_UGC::Steam_UGC(class Settings *settings, class Ugc_Remote_Storage_Bridge *ugc_bridge, class Local_Storage *local_storage, class SteamCallResults *callback_results, class SteamCallBacks *callbacks)
{
    this->settings = settings;
    this->ugc_bridge = ugc_bridge;
    this->local_storage = local_storage;
    this->callbacks = callbacks;
    this->callback_results = callback_results;

    read_ugc_favorites();
}


// Query UGC associated with a user. Creator app id or consumer app id must be valid and be set to the current running app. unPage should start at 1.
UGCQueryHandle_t Steam_UGC::CreateQueryUserUGCRequest( AccountID_t unAccountID, EUserUGCList eListType, EUGCMatchingUGCType eMatchingUGCType, EUserUGCListSortOrder eSortOrder, AppId_t nCreatorAppID, AppId_t nConsumerAppID, uint32 unPage )
{
    PRINT_DEBUG("%u %i %i %i %u %u %u", unAccountID, eListType, eMatchingUGCType, eSortOrder, nCreatorAppID, nConsumerAppID, unPage);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (nCreatorAppID != settings->get_local_game_id().AppID() || nConsumerAppID != settings->get_local_game_id().AppID()) return k_UGCQueryHandleInvalid;
    if (unPage < 1) return k_UGCQueryHandleInvalid;
    if (eListType < 0) return k_UGCQueryHandleInvalid;
    if (unAccountID != settings->get_local_steam_id().GetAccountID()) return k_UGCQueryHandleInvalid;
    
    // TODO
    return new_ugc_query(eUserUGCRequest, eListType == k_EUserUGCList_Subscribed || eListType == k_EUserUGCList_Published, unPage);
}


// Query for all matching UGC. Creator app id or consumer app id must be valid and be set to the current running app. unPage should start at 1.
UGCQueryHandle_t Steam_UGC::CreateQueryAllUGCRequest( EUGCQuery eQueryType, EUGCMatchingUGCType eMatchingeMatchingUGCTypeFileType, AppId_t nCreatorAppID, AppId_t nConsumerAppID, uint32 unPage )
{
    PRINT_DEBUG("page");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    if (nCreatorAppID != settings->get_local_game_id().AppID() || nConsumerAppID != settings->get_local_game_id().AppID()) return k_UGCQueryHandleInvalid;
    if (unPage < 1) return k_UGCQueryHandleInvalid;
    if (eQueryType < 0) return k_UGCQueryHandleInvalid;
    
    // TODO
    return new_ugc_query(eAllUGCRequestPage, true, unPage);
}

// Query for all matching UGC using the new deep paging interface. Creator app id or consumer app id must be valid and be set to the current running app. pchCursor should be set to NULL or "*" to get the first result set.
UGCQueryHandle_t Steam_UGC::CreateQueryAllUGCRequest( EUGCQuery eQueryType, EUGCMatchingUGCType eMatchingeMatchingUGCTypeFileType, AppId_t nCreatorAppID, AppId_t nConsumerAppID, const char *pchCursor )
{
    PRINT_DEBUG("cursor");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    if (nCreatorAppID != settings->get_local_game_id().AppID() || nConsumerAppID != settings->get_local_game_id().AppID()) return k_UGCQueryHandleInvalid;
    if (eQueryType < 0) return k_UGCQueryHandleInvalid;
    
    // TODO: totally don't know what does pchCursor mean, we currently emu it to be a string of page number instead
    uint32 page = 0;
    bool next_cursor = true;
    std::string cursor = pchCursor != NULL ? std::string(pchCursor) : std::string("*");

    try {
        if (cursor == std::string("*")) {
            page = 1;
        }
        else if (cursor == std::string("")) {
            page = 1;            // Tested on real steam, "" is a valid cursor, which seems to be always page 1.
            next_cursor = false; // However, under this condition, next cursor will still be "", so we flag it here
        }
        else {
            page = std::stoul(cursor);
        }
    }
    catch (const std::exception &e) {
        PRINT_DEBUG("Conversion error, reason: %s. Is this a valid cursor?", e.what());
        page = 0;
    }

    // TODO
    return new_ugc_query(eAllUGCRequestCursor, true, page, next_cursor);
}

// Query for the details of the given published file ids (the RequestUGCDetails call is deprecated and replaced with this)
UGCQueryHandle_t Steam_UGC::CreateQueryUGCDetailsRequest( PublishedFileId_t *pvecPublishedFileID, uint32 unNumPublishedFileIDs )
{
    PRINT_DEBUG("%p, max file IDs = [%u]", pvecPublishedFileID, unNumPublishedFileIDs);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    if (!pvecPublishedFileID) return k_UGCQueryHandleInvalid;
    if (unNumPublishedFileIDs < 1) return k_UGCQueryHandleInvalid;

    // TODO
    std::set<PublishedFileId_t> only(pvecPublishedFileID, pvecPublishedFileID + unNumPublishedFileIDs);
    
#ifndef EMU_RELEASE_BUILD
    for (const auto &id : only) {
        PRINT_DEBUG("  requesting details for file ID = %llu", id);
    }
#endif

    return new_ugc_query(eUGCDetailsRequest, false, 0, true, only);
}


// Send the query to Steam
STEAM_CALL_RESULT( SteamUGCQueryCompleted_t )
SteamAPICall_t Steam_UGC::SendQueryUGCRequest( UGCQueryHandle_t handle )
{
    PRINT_DEBUG("%llu", handle);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    const auto trigger_failure = [handle, this](){
        SteamUGCQueryCompleted_t data{};
        data.m_handle = handle;
        data.m_eResult = k_EResultFail;
        data.m_unNumResultsReturned = 0;
        data.m_unTotalMatchingResults = 0;
        data.m_bCachedData = false;
        
        auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
        callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
        return ret;
    };
    
    if (handle == k_UGCQueryHandleInvalid) return trigger_failure();

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return trigger_failure();

    SteamUGCQueryCompleted_t data{};
    data.m_handle = handle;
    data.m_eResult = k_EResultOK;
    data.m_bCachedData = false;

    std::set<PublishedFileId_t> all_subscribed = std::set<PublishedFileId_t>(ugc_bridge->subbed_mods_itr_begin(), ugc_bridge->subbed_mods_itr_end());

    if (request->query_type == eUserUGCRequest) {
        if (request->return_all_subscribed) {
            if (request->page > 0) {
                uint32 beg_item = (request->page - 1) * kNumUGCResultsPerPage;
                if (beg_item < all_subscribed.size()) {
                    auto sub = all_subscribed.begin();
                    std::advance(sub, beg_item);
                    for (uint32 i = 0; sub != all_subscribed.end() && i < kNumUGCResultsPerPage; ++sub, ++i) {
                        request->results.insert(*sub);
                    }
                }
            }

            data.m_unNumResultsReturned = static_cast<uint32>(request->results.size());
            data.m_unTotalMatchingResults = static_cast<uint32>(all_subscribed.size());
        }
        else {
            data.m_unNumResultsReturned = 0;
            data.m_unTotalMatchingResults = 0;
        }
    }
    else if (request->query_type == eAllUGCRequestPage || request->query_type == eAllUGCRequestCursor) {
        if (request->page > 0) {
            uint32 beg_item = (request->page - 1) * kNumUGCResultsPerPage;
            if (beg_item < all_subscribed.size()) {
                auto sub = all_subscribed.begin();
                std::advance(sub, beg_item);
                for (uint32 i = 0; sub != all_subscribed.end() && i < kNumUGCResultsPerPage; ++sub, ++i) {
                    request->results.insert(*sub);
                }

                data.m_unNumResultsReturned = static_cast<uint32>(request->results.size());
                data.m_unTotalMatchingResults = static_cast<uint32>(all_subscribed.size());
                if (request->query_type == eAllUGCRequestCursor) {
                    std::string next_page_cursor = request->next_cursor ? std::to_string(request->page + 1) : std::string("");
                    next_page_cursor.copy(data.m_rgchNextCursor, sizeof(data.m_rgchNextCursor) - 1);
                }
            }
            else {
                data.m_eResult = k_EResultInvalidParam;
                data.m_unNumResultsReturned = 0;
                data.m_unTotalMatchingResults = 0;
                if (request->query_type == eAllUGCRequestCursor)
                    data.m_rgchNextCursor[0] = '\0';
            }
        }
        else { // impossible to meet this condition when query_type is eAllUGCRequestPage though
            data.m_eResult = request->query_type == eAllUGCRequestCursor ? k_EResultFail : k_EResultInvalidParam;
            data.m_unNumResultsReturned = 0;
            data.m_unTotalMatchingResults = 0;
            if (request->query_type == eAllUGCRequestCursor)
                data.m_rgchNextCursor[0] = '\0';
        }
    }
    else if (request->query_type == eUGCDetailsRequest) {
        if (request->return_only.size()) {
            for (auto & s : request->return_only) {
                if (ugc_bridge->has_subbed_mod(s)) {
                    request->results.insert(s);
                }
            }
        }

        data.m_unNumResultsReturned = static_cast<uint32>(request->results.size());
        data.m_unTotalMatchingResults = static_cast<uint32>(request->results.size());
    }

    // send these handles to steam_remote_storage since the game will later
    // call Steam_Remote_Storage::UGCDownload() with these files handles (primary + preview)
    for (auto fileid : request->results) {
        auto mod = settings->getMod(fileid);
        ugc_bridge->add_ugc_query_result(mod.handleFile, fileid, true);
        ugc_bridge->add_ugc_query_result(mod.handlePreviewFile, fileid, false);
    }
    
    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return ret;
}


// Retrieve an individual result after receiving the callback for querying UGC
bool Steam_UGC::GetQueryUGCResult( UGCQueryHandle_t handle, uint32 index, SteamUGCDetails_t *pDetails )
{
    PRINT_DEBUG("%llu %u %p", handle, index, pDetails);
    return internal_GetQueryUGCResult(handle, index, pDetails, IUgcItfVersion::v020);
}

bool Steam_UGC::GetQueryUGCResult_old( UGCQueryHandle_t handle, uint32 index, SteamUGCDetails_t *pDetails )
{
    PRINT_DEBUG("%llu %u %p", handle, index, pDetails);
    return internal_GetQueryUGCResult(handle, index, pDetails, IUgcItfVersion::v018);
}

std::optional<std::string> Steam_UGC::get_query_ugc_tag(UGCQueryHandle_t handle, uint32 index, uint32 indexTag)
{
    auto res = get_query_ugc_tags(handle, index);
    if (res.empty()) return std::nullopt;
    if (indexTag >= res.size()) return std::nullopt;

    std::string tmp = res[indexTag];
    if (!tmp.empty() && tmp.back() == ',') {
        tmp = tmp.substr(0, tmp.size() - 1);
    }
    return tmp;
}

uint32 Steam_UGC::GetQueryUGCNumTags( UGCQueryHandle_t handle, uint32 index )
{
    PRINT_DEBUG_TODO();
    // TODO is this correct?
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return 0;
    
    auto res = get_query_ugc_tags(handle, index);
    return static_cast<uint32>(res.size());
}

bool Steam_UGC::GetQueryUGCTag( UGCQueryHandle_t handle, uint32 index, uint32 indexTag, STEAM_OUT_STRING_COUNT( cchValueSize ) char* pchValue, uint32 cchValueSize )
{
    PRINT_DEBUG_TODO();
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

bool Steam_UGC::GetQueryUGCTagDisplayName( UGCQueryHandle_t handle, uint32 index, uint32 indexTag, STEAM_OUT_STRING_COUNT( cchValueSize ) char* pchValue, uint32 cchValueSize )
{
    PRINT_DEBUG_TODO();
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

bool Steam_UGC::GetQueryUGCPreviewURL( UGCQueryHandle_t handle, uint32 index, STEAM_OUT_STRING_COUNT(cchURLSize) char *pchURL, uint32 cchURLSize )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    //TODO: escape simulator tries downloading this url and unsubscribes if it fails
    if (handle == k_UGCQueryHandleInvalid) return false;
    if (!pchURL || !cchURLSize) return false;

    auto res = get_query_ugc(handle, index);
    if (!res.has_value()) return false;

    auto &mod = res.value();
    PRINT_DEBUG("Steam_UGC:GetQueryUGCPreviewURL: '%s'", mod.previewURL.c_str());
    memset(pchURL, 0, cchURLSize);
    mod.previewURL.copy(pchURL, cchURLSize - 1);
    return true;
}


bool Steam_UGC::GetQueryUGCMetadata( UGCQueryHandle_t handle, uint32 index, STEAM_OUT_STRING_COUNT(cchMetadatasize) char *pchMetadata, uint32 cchMetadatasize )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;

    return false;
}


bool Steam_UGC::GetQueryUGCChildren( UGCQueryHandle_t handle, uint32 index, PublishedFileId_t* pvecPublishedFileID, uint32 cMaxEntries )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return false;
}


bool Steam_UGC::GetQueryUGCStatistic( UGCQueryHandle_t handle, uint32 index, EItemStatistic eStatType, uint64 *pStatValue )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return false;
}

bool Steam_UGC::GetQueryUGCStatistic( UGCQueryHandle_t handle, uint32 index, EItemStatistic eStatType, uint32 *pStatValue )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return false;
}

uint32 Steam_UGC::GetQueryUGCNumAdditionalPreviews( UGCQueryHandle_t handle, uint32 index )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return 0;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return 0;
    
    return 0;
}


bool Steam_UGC::GetQueryUGCAdditionalPreview( UGCQueryHandle_t handle, uint32 index, uint32 previewIndex, STEAM_OUT_STRING_COUNT(cchURLSize) char *pchURLOrVideoID, uint32 cchURLSize, STEAM_OUT_STRING_COUNT(cchURLSize) char *pchOriginalFileName, uint32 cchOriginalFileNameSize, EItemPreviewType *pPreviewType )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return false;
}

bool Steam_UGC::GetQueryUGCAdditionalPreview( UGCQueryHandle_t handle, uint32 index, uint32 previewIndex, char *pchURLOrVideoID, uint32 cchURLSize, bool *hz )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return false;
}

uint32 Steam_UGC::GetQueryUGCNumKeyValueTags( UGCQueryHandle_t handle, uint32 index )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return 0;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return 0;
    
    return 0;
}


bool Steam_UGC::GetQueryUGCKeyValueTag( UGCQueryHandle_t handle, uint32 index, uint32 keyValueTagIndex, STEAM_OUT_STRING_COUNT(cchKeySize) char *pchKey, uint32 cchKeySize, STEAM_OUT_STRING_COUNT(cchValueSize) char *pchValue, uint32 cchValueSize )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return false;
}

bool Steam_UGC::GetQueryUGCKeyValueTag( UGCQueryHandle_t handle, uint32 index, const char *pchKey, STEAM_OUT_STRING_COUNT(cchValueSize) char *pchValue, uint32 cchValueSize )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return false;
}

// TODO no public docs
// Some items can specify that they have a version that is valid for a range of game versions (Steam branch)
uint32 Steam_UGC::GetNumSupportedGameVersions( UGCQueryHandle_t handle, uint32 index )
{
    PRINT_DEBUG("%llu %u // TODO", handle, index);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return 0;

    auto res = get_query_ugc(handle, index);
    if (!res.has_value()) return 0;
    
    return 1;
}

// TODO no public docs
bool Steam_UGC::GetSupportedGameVersionData( UGCQueryHandle_t handle, uint32 index, uint32 versionIndex, STEAM_OUT_STRING_COUNT( cchGameBranchSize ) char *pchGameBranchMin, STEAM_OUT_STRING_COUNT( cchGameBranchSize ) char *pchGameBranchMax, uint32 cchGameBranchSize )
{
    PRINT_DEBUG("%llu %u %u // TODO", handle, index, versionIndex);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    if (versionIndex != 0) { // TODO I assume this is supposed to be an index in the range [ 0, GetNumSupportedGameVersions() )
        return false;
    }
    
    auto res = get_query_ugc(handle, index);
    if (!res.has_value()) return false;

    auto &mod = res.value();

    // TODO I assume each mod/workshop item has a min version and max version for the game
    if (pchGameBranchMin && static_cast<size_t>(cchGameBranchSize) > mod.min_game_branch.size()) {
        memset(pchGameBranchMin, 0, cchGameBranchSize);
        memcpy(pchGameBranchMin, mod.min_game_branch.c_str(), mod.min_game_branch.size());
    }
    if (pchGameBranchMax && static_cast<size_t>(cchGameBranchSize) > mod.max_game_branch.size()) {
        memset(pchGameBranchMax, 0, cchGameBranchSize);
        memcpy(pchGameBranchMax, mod.max_game_branch.c_str(), mod.max_game_branch.size());
    }

    return true;
}

uint32 Steam_UGC::GetQueryUGCContentDescriptors( UGCQueryHandle_t handle, uint32 index, EUGCContentDescriptorID *pvecDescriptors, uint32 cMaxEntries )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return 0;

    auto res = get_query_ugc(handle, index);
    if (!res.has_value()) return 0;

    return 0;
}

// Release the request to free up memory, after retrieving results
bool Steam_UGC::ReleaseQueryUGCRequest( UGCQueryHandle_t handle )
{
    PRINT_DEBUG("%llu", handle);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;

    ugc_queries.erase(request);
    return true;
}


// Options to set for querying UGC
bool Steam_UGC::AddRequiredTag( UGCQueryHandle_t handle, const char *pTagName )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}

bool Steam_UGC::AddRequiredTagGroup( UGCQueryHandle_t handle, const SteamParamStringArray_t *pTagGroups )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}

bool Steam_UGC::AddExcludedTag( UGCQueryHandle_t handle, const char *pTagName )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}


bool Steam_UGC::SetReturnOnlyIDs( UGCQueryHandle_t handle, bool bReturnOnlyIDs )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}


bool Steam_UGC::SetReturnKeyValueTags( UGCQueryHandle_t handle, bool bReturnKeyValueTags )
{
    PRINT_DEBUG_TODO();
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}


bool Steam_UGC::SetReturnLongDescription( UGCQueryHandle_t handle, bool bReturnLongDescription )
{
    PRINT_DEBUG_TODO();
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}


bool Steam_UGC::SetReturnMetadata( UGCQueryHandle_t handle, bool bReturnMetadata )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}


bool Steam_UGC::SetReturnChildren( UGCQueryHandle_t handle, bool bReturnChildren )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}


bool Steam_UGC::SetReturnAdditionalPreviews( UGCQueryHandle_t handle, bool bReturnAdditionalPreviews )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}


bool Steam_UGC::SetReturnTotalOnly( UGCQueryHandle_t handle, bool bReturnTotalOnly )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}


bool Steam_UGC::SetReturnPlaytimeStats( UGCQueryHandle_t handle, uint32 unDays )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}


bool Steam_UGC::SetLanguage( UGCQueryHandle_t handle, const char *pchLanguage )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}


bool Steam_UGC::SetAllowCachedResponse( UGCQueryHandle_t handle, uint32 unMaxAgeSeconds )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}

// TODO no public docs
// allow ISteamUGC to be used in a tools like environment for users who have the appropriate privileges for the calling appid
bool Steam_UGC::SetAdminQuery( UGCUpdateHandle_t handle, bool bAdminQuery )
{
    PRINT_DEBUG("%llu %i // TODO", handle, (int)bAdminQuery);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    request->admin_query = bAdminQuery;
    return true;
}


// Options only for querying user UGC
bool Steam_UGC::SetCloudFileNameFilter( UGCQueryHandle_t handle, const char *pMatchCloudFileName )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}


// Options only for querying all UGC
bool Steam_UGC::SetMatchAnyTag( UGCQueryHandle_t handle, bool bMatchAnyTag )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}


bool Steam_UGC::SetSearchText( UGCQueryHandle_t handle, const char *pSearchText )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}


bool Steam_UGC::SetRankedByTrendDays( UGCQueryHandle_t handle, uint32 unDays )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}


bool Steam_UGC::AddRequiredKeyValueTag( UGCQueryHandle_t handle, const char *pKey, const char *pValue )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}

bool Steam_UGC::SetTimeCreatedDateRange( UGCQueryHandle_t handle, RTime32 rtStart, RTime32 rtEnd )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}

bool Steam_UGC::SetTimeUpdatedDateRange( UGCQueryHandle_t handle, RTime32 rtStart, RTime32 rtEnd )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    return true;
}

// DEPRECATED - Use CreateQueryUGCDetailsRequest call above instead!
SteamAPICall_t Steam_UGC::RequestUGCDetails( PublishedFileId_t nPublishedFileID, uint32 unMaxAgeSeconds )
{
    PRINT_DEBUG("%llu", nPublishedFileID);
    return internal_RequestUGCDetails(nPublishedFileID, unMaxAgeSeconds, IUgcItfVersion::v020);
}
 
SteamAPICall_t Steam_UGC::RequestUGCDetails_old( PublishedFileId_t nPublishedFileID, uint32 unMaxAgeSeconds )
{
    PRINT_DEBUG("%llu", nPublishedFileID);
    return internal_RequestUGCDetails(nPublishedFileID, unMaxAgeSeconds, IUgcItfVersion::v018);
}

SteamAPICall_t Steam_UGC::RequestUGCDetails( PublishedFileId_t nPublishedFileID )
{
    PRINT_DEBUG("old");
    return RequestUGCDetails_old(nPublishedFileID, 0);
}


// Steam Workshop Creator API
STEAM_CALL_RESULT( CreateItemResult_t )
SteamAPICall_t Steam_UGC::CreateItem( AppId_t nConsumerAppId, EWorkshopFileType eFileType )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return k_uAPICallInvalid;
}
 // create new item for this app with no content attached yet


UGCUpdateHandle_t Steam_UGC::StartItemUpdate( AppId_t nConsumerAppId, PublishedFileId_t nPublishedFileID )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return k_UGCUpdateHandleInvalid;
}
 // start an UGC item update. Set changed properties before commiting update with CommitItemUpdate()


bool Steam_UGC::SetItemTitle( UGCUpdateHandle_t handle, const char *pchTitle )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}
 // change the title of an UGC item


bool Steam_UGC::SetItemDescription( UGCUpdateHandle_t handle, const char *pchDescription )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}
 // change the description of an UGC item


bool Steam_UGC::SetItemUpdateLanguage( UGCUpdateHandle_t handle, const char *pchLanguage )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}
 // specify the language of the title or description that will be set


bool Steam_UGC::SetItemMetadata( UGCUpdateHandle_t handle, const char *pchMetaData )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}
 // change the metadata of an UGC item (max = k_cchDeveloperMetadataMax)


bool Steam_UGC::SetItemVisibility( UGCUpdateHandle_t handle, ERemoteStoragePublishedFileVisibility eVisibility )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}
 // change the visibility of an UGC item


bool Steam_UGC::SetItemTags( UGCUpdateHandle_t updateHandle, const SteamParamStringArray_t *pTags )
{
    PRINT_DEBUG("old");
    return SetItemTags(updateHandle, pTags, false);
}

bool Steam_UGC::SetItemTags( UGCUpdateHandle_t updateHandle, const SteamParamStringArray_t *pTags, bool bAllowAdminTags )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}
 // change the tags of an UGC item

bool Steam_UGC::SetItemContent( UGCUpdateHandle_t handle, const char *pszContentFolder )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}
 // update item content from this local folder


bool Steam_UGC::SetItemPreview( UGCUpdateHandle_t handle, const char *pszPreviewFile )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}
 //  change preview image file for this item. pszPreviewFile points to local image file, which must be under 1MB in size

bool Steam_UGC::SetAllowLegacyUpload( UGCUpdateHandle_t handle, bool bAllowLegacyUpload )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}

bool Steam_UGC::RemoveAllItemKeyValueTags( UGCUpdateHandle_t handle )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}
 // remove all existing key-value tags (you can add new ones via the AddItemKeyValueTag function)

bool Steam_UGC::RemoveItemKeyValueTags( UGCUpdateHandle_t handle, const char *pchKey )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}
 // remove any existing key-value tags with the specified key


bool Steam_UGC::AddItemKeyValueTag( UGCUpdateHandle_t handle, const char *pchKey, const char *pchValue )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}
 // add new key-value tags for the item. Note that there can be multiple values for a tag.


bool Steam_UGC::AddItemPreviewFile( UGCUpdateHandle_t handle, const char *pszPreviewFile, EItemPreviewType type )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}
 //  add preview file for this item. pszPreviewFile points to local file, which must be under 1MB in size


bool Steam_UGC::AddItemPreviewVideo( UGCUpdateHandle_t handle, const char *pszVideoID )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}
 //  add preview video for this item


bool Steam_UGC::UpdateItemPreviewFile( UGCUpdateHandle_t handle, uint32 index, const char *pszPreviewFile )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}
 //  updates an existing preview file for this item. pszPreviewFile points to local file, which must be under 1MB in size


bool Steam_UGC::UpdateItemPreviewVideo( UGCUpdateHandle_t handle, uint32 index, const char *pszVideoID )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}
 //  updates an existing preview video for this item


bool Steam_UGC::RemoveItemPreview( UGCUpdateHandle_t handle, uint32 index )
{
    PRINT_DEBUG("%llu %u", handle, index);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}
 // remove a preview by index starting at 0 (previews are sorted)

bool Steam_UGC::AddContentDescriptor( UGCUpdateHandle_t handle, EUGCContentDescriptorID descid )
{
    PRINT_DEBUG("%llu %u", handle, descid);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}

bool Steam_UGC::RemoveContentDescriptor( UGCUpdateHandle_t handle, EUGCContentDescriptorID descid )
{
    PRINT_DEBUG("%llu %u", handle, descid);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}

// TODO no public docs
bool Steam_UGC::SetRequiredGameVersions( UGCUpdateHandle_t handle, const char *pszGameBranchMin, const char *pszGameBranchMax )
{
    PRINT_DEBUG("%llu '%s' '%s' // TODO", handle, pszGameBranchMin, pszGameBranchMax);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    if (handle == k_UGCQueryHandleInvalid) return false;

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) return false;
    
    if (pszGameBranchMin) request->min_branch = pszGameBranchMin;
    if (pszGameBranchMax) request->max_branch = pszGameBranchMax;
    return true;
}

STEAM_CALL_RESULT( SubmitItemUpdateResult_t )
SteamAPICall_t Steam_UGC::SubmitItemUpdate( UGCUpdateHandle_t handle, const char *pchChangeNote )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return k_uAPICallInvalid;
}
 // commit update process started with StartItemUpdate()


EItemUpdateStatus Steam_UGC::GetItemUpdateProgress( UGCUpdateHandle_t handle, uint64 *punBytesProcessed, uint64* punBytesTotal )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return k_EItemUpdateStatusInvalid;
}


// Steam Workshop Consumer API

STEAM_CALL_RESULT( SetUserItemVoteResult_t )
SteamAPICall_t Steam_UGC::SetUserItemVote( PublishedFileId_t nPublishedFileID, bool bVoteUp )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!settings->isModInstalled(nPublishedFileID)) return k_uAPICallInvalid; // TODO is this correct
    
    auto mod  = settings->getMod(nPublishedFileID);
    if (bVoteUp) {
        ++mod.votesUp;
    } else {
        ++mod.votesDown;
    }
    settings->addModDetails(nPublishedFileID, mod);
    
    SetUserItemVoteResult_t data{};
    data.m_eResult = EResult::k_EResultOK;
    data.m_nPublishedFileId = nPublishedFileID;
    data.m_bVoteUp = bVoteUp;

    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return ret;
}


STEAM_CALL_RESULT( GetUserItemVoteResult_t )
SteamAPICall_t Steam_UGC::GetUserItemVote( PublishedFileId_t nPublishedFileID )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (nPublishedFileID == k_PublishedFileIdInvalid || !settings->isModInstalled(nPublishedFileID)) return k_uAPICallInvalid; // TODO is this correct

    auto mod  = settings->getMod(nPublishedFileID);
    GetUserItemVoteResult_t data{};
    data.m_eResult = EResult::k_EResultOK;
    data.m_nPublishedFileId = nPublishedFileID;
    data.m_bVotedDown = mod.votesDown;
    data.m_bVotedUp = mod.votesUp;
    data.m_bVoteSkipped = true;
    
    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return ret;
}


STEAM_CALL_RESULT( UserFavoriteItemsListChanged_t )
SteamAPICall_t Steam_UGC::AddItemToFavorites( AppId_t nAppId, PublishedFileId_t nPublishedFileID )
{
    PRINT_DEBUG("%u %llu", nAppId, nPublishedFileID);
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
    
    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return ret;
}


STEAM_CALL_RESULT( UserFavoriteItemsListChanged_t )
SteamAPICall_t Steam_UGC::RemoveItemFromFavorites( AppId_t nAppId, PublishedFileId_t nPublishedFileID )
{
    PRINT_DEBUG_ENTRY();
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
    
    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return ret;
}


STEAM_CALL_RESULT( RemoteStorageSubscribePublishedFileResult_t )
SteamAPICall_t Steam_UGC::SubscribeItem( PublishedFileId_t nPublishedFileID )
{
    PRINT_DEBUG("%llu", nPublishedFileID);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    RemoteStorageSubscribePublishedFileResult_t data{};
    data.m_nPublishedFileId = nPublishedFileID;
    if (settings->isModInstalled(nPublishedFileID)) {
        data.m_eResult = k_EResultOK;
        ugc_bridge->add_subbed_mod(nPublishedFileID);
    } else {
        data.m_eResult = k_EResultFail;
    }
    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return ret;
}
 // subscribe to this item, will be installed ASAP

STEAM_CALL_RESULT( RemoteStorageUnsubscribePublishedFileResult_t )
SteamAPICall_t Steam_UGC::UnsubscribeItem( PublishedFileId_t nPublishedFileID )
{
    PRINT_DEBUG("%llu", nPublishedFileID);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    RemoteStorageUnsubscribePublishedFileResult_t data{};
    data.m_nPublishedFileId = nPublishedFileID;
    if (!ugc_bridge->has_subbed_mod(nPublishedFileID)) {
        data.m_eResult = k_EResultFail; //TODO: check if this is accurate
    } else {
        data.m_eResult = k_EResultOK;
        ugc_bridge->remove_subbed_mod(nPublishedFileID);
    }

    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return ret;
}
 // unsubscribe from this item, will be uninstalled after game quits

uint32 Steam_UGC::GetNumSubscribedItems()
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    PRINT_DEBUG("  Steam_UGC::GetNumSubscribedItems = %zu", ugc_bridge->subbed_mods_count());
    return (uint32)ugc_bridge->subbed_mods_count();
}
 // number of subscribed items 

uint32 Steam_UGC::GetSubscribedItems( PublishedFileId_t* pvecPublishedFileID, uint32 cMaxEntries )
{
    PRINT_DEBUG("%p %u", pvecPublishedFileID, cMaxEntries);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if ((size_t)cMaxEntries > ugc_bridge->subbed_mods_count()) {
        cMaxEntries = (uint32)ugc_bridge->subbed_mods_count();
    }

    std::copy_n(ugc_bridge->subbed_mods_itr_begin(), cMaxEntries, pvecPublishedFileID);
    return cMaxEntries;
}
 // all subscribed item PublishFileIDs

// get EItemState flags about item on this client
uint32 Steam_UGC::GetItemState( PublishedFileId_t nPublishedFileID )
{
    PRINT_DEBUG("%llu", nPublishedFileID);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    if (!settings->isModInstalled(nPublishedFileID)) {
        PRINT_DEBUG("  mod isn't found");
        return k_EItemStateNone;
    }

    if (ugc_bridge->has_subbed_mod(nPublishedFileID)) {
        PRINT_DEBUG("  mod is subscribed and installed");
        return k_EItemStateInstalled | k_EItemStateSubscribed;
    }


    PRINT_DEBUG("  mod is not subscribed");
    return k_EItemStateDisabledLocally;
}


// get info about currently installed content on disc for items that have k_EItemStateInstalled set
// if k_EItemStateLegacyItem is set, pchFolder contains the path to the legacy file itself (not a folder)
bool Steam_UGC::GetItemInstallInfo( PublishedFileId_t nPublishedFileID, uint64 *punSizeOnDisk, STEAM_OUT_STRING_COUNT( cchFolderSize ) char *pchFolder, uint32 cchFolderSize, uint32 *punTimeStamp )
{
    PRINT_DEBUG("%llu %p %p [%u] %p", nPublishedFileID, punSizeOnDisk, pchFolder, cchFolderSize, punTimeStamp);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!cchFolderSize) return false;
    if (!settings->isModInstalled(nPublishedFileID)) return false;

    auto mod = settings->getMod(nPublishedFileID);
    
    // I don't know if this is accurate behavior, but to avoid returning true with invalid data
    if ((cchFolderSize - 1) < mod.path.size()) { // -1 because the last char is reserved for null terminator
        PRINT_DEBUG("  ERROR mod path: '%s' [%zu bytes] cannot fit into the given buffer", mod.path.c_str(), mod.path.size());
        return false;
    }

    if (punSizeOnDisk) *punSizeOnDisk = mod.primaryFileSize;
    if (punTimeStamp) *punTimeStamp = mod.timeAddedToUserList;
    if (pchFolder && cchFolderSize) {
        // human fall flat doesn't send a nulled buffer, and won't recognize the proper mod path because of that
        memset(pchFolder, 0, cchFolderSize);
        mod.path.copy(pchFolder, cchFolderSize - 1);
        PRINT_DEBUG("  final mod path: '%s'", pchFolder);
    }

    return true;
}


// get info about pending update for items that have k_EItemStateNeedsUpdate set. punBytesTotal will be valid after download started once
bool Steam_UGC::GetItemDownloadInfo( PublishedFileId_t nPublishedFileID, uint64 *punBytesDownloaded, uint64 *punBytesTotal )
{
    PRINT_DEBUG("%llu", nPublishedFileID);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!settings->isModInstalled(nPublishedFileID)) return false;

    auto mod = settings->getMod(nPublishedFileID);
    if (punBytesDownloaded) *punBytesDownloaded = mod.primaryFileSize;
    if (punBytesTotal) *punBytesTotal = mod.primaryFileSize;
    return true;
}

bool Steam_UGC::GetItemInstallInfo( PublishedFileId_t nPublishedFileID, uint64 *punSizeOnDisk, STEAM_OUT_STRING_COUNT( cchFolderSize ) char *pchFolder, uint32 cchFolderSize, bool *pbLegacyItem ) // returns true if item is installed
{
    PRINT_DEBUG("old");
    return GetItemInstallInfo(nPublishedFileID, punSizeOnDisk, pchFolder, cchFolderSize, (uint32*) nullptr);
}

bool Steam_UGC::GetItemUpdateInfo( PublishedFileId_t nPublishedFileID, bool *pbNeedsUpdate, bool *pbIsDownloading, uint64 *punBytesDownloaded, uint64 *punBytesTotal )
{
    PRINT_DEBUG("old");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    bool res = GetItemDownloadInfo(nPublishedFileID, punBytesDownloaded, punBytesTotal);
    if (res) {
        if (pbNeedsUpdate) *pbNeedsUpdate = false;
        if (pbIsDownloading) *pbIsDownloading = false;
    }
    return res;
}

bool Steam_UGC::GetItemInstallInfo( PublishedFileId_t nPublishedFileID, uint64 *punSizeOnDisk, char *pchFolder, uint32 cchFolderSize ) // returns true if item is installed
{
    PRINT_DEBUG("older");
    return GetItemInstallInfo(nPublishedFileID, punSizeOnDisk, pchFolder, cchFolderSize, (uint32*) nullptr);
}


// download new or update already installed item. If function returns true, wait for DownloadItemResult_t. If the item is already installed,
// then files on disk should not be used until callback received. If item is not subscribed to, it will be cached for some time.
// If bHighPriority is set, any other item download will be suspended and this item downloaded ASAP.
bool Steam_UGC::DownloadItem( PublishedFileId_t nPublishedFileID, bool bHighPriority )
{
    PRINT_DEBUG("%llu %i // TODO", nPublishedFileID, (int)bHighPriority);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    if (!settings->isModInstalled(nPublishedFileID)) {
        DownloadItemResult_t data_fail{};
        data_fail.m_eResult = EResult::k_EResultFail;
        data_fail.m_nPublishedFileId = nPublishedFileID;
        data_fail.m_unAppID = settings->get_local_game_id().AppID();
        callbacks->addCBResult(data_fail.k_iCallback, &data_fail, sizeof(data_fail), 0.050);
        return false;
    }

    {
        DownloadItemResult_t data{};
        data.m_eResult = EResult::k_EResultOK;
        data.m_nPublishedFileId = nPublishedFileID;
        data.m_unAppID = settings->get_local_game_id().AppID();
        callbacks->addCBResult(data.k_iCallback, &data, sizeof(data), 0.1);
    }

    {
        ItemInstalled_t data{};
        data.m_hLegacyContent = nPublishedFileID;
        data.m_nPublishedFileId = nPublishedFileID;
        data.m_unAppID = settings->get_local_game_id().AppID();
        data.m_unManifestID = 123; // TODO
        callbacks->addCBResult(data.k_iCallback, &data, sizeof(data), 0.15);
    }

    PRINT_DEBUG("downloaded!");
    return true;
}


// game servers can set a specific workshop folder before issuing any UGC commands.
// This is helpful if you want to support multiple game servers running out of the same install folder
bool Steam_UGC::BInitWorkshopForGameServer( DepotId_t unWorkshopDepotID, const char *pszFolder )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}


// SuspendDownloads( true ) will suspend all workshop downloads until SuspendDownloads( false ) is called or the game ends
void Steam_UGC::SuspendDownloads( bool bSuspend )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
}


// usage tracking
STEAM_CALL_RESULT( StartPlaytimeTrackingResult_t )
SteamAPICall_t Steam_UGC::StartPlaytimeTracking( PublishedFileId_t *pvecPublishedFileID, uint32 unNumPublishedFileIDs )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    StopPlaytimeTrackingResult_t data;
    data.m_eResult = k_EResultOK;
    
    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return ret;
}

STEAM_CALL_RESULT( StopPlaytimeTrackingResult_t )
SteamAPICall_t Steam_UGC::StopPlaytimeTracking( PublishedFileId_t *pvecPublishedFileID, uint32 unNumPublishedFileIDs )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    StopPlaytimeTrackingResult_t data;
    data.m_eResult = k_EResultOK;
    
    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return ret;
}

STEAM_CALL_RESULT( StopPlaytimeTrackingResult_t )
SteamAPICall_t Steam_UGC::StopPlaytimeTrackingForAllItems()
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    StopPlaytimeTrackingResult_t data;
    data.m_eResult = k_EResultOK;
    
    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return ret;
}


// parent-child relationship or dependency management
STEAM_CALL_RESULT( AddUGCDependencyResult_t )
SteamAPICall_t Steam_UGC::AddDependency( PublishedFileId_t nParentPublishedFileID, PublishedFileId_t nChildPublishedFileID )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    if (nParentPublishedFileID == k_PublishedFileIdInvalid) return k_uAPICallInvalid;
    
    return k_uAPICallInvalid;
}

STEAM_CALL_RESULT( RemoveUGCDependencyResult_t )
SteamAPICall_t Steam_UGC::RemoveDependency( PublishedFileId_t nParentPublishedFileID, PublishedFileId_t nChildPublishedFileID )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    if (nParentPublishedFileID == k_PublishedFileIdInvalid) return k_uAPICallInvalid;
    
    return k_uAPICallInvalid;
}


// add/remove app dependence/requirements (usually DLC)
STEAM_CALL_RESULT( AddAppDependencyResult_t )
SteamAPICall_t Steam_UGC::AddAppDependency( PublishedFileId_t nPublishedFileID, AppId_t nAppID )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    if (nPublishedFileID == k_PublishedFileIdInvalid) return k_uAPICallInvalid;
    
    return k_uAPICallInvalid;
}

STEAM_CALL_RESULT( RemoveAppDependencyResult_t )
SteamAPICall_t Steam_UGC::RemoveAppDependency( PublishedFileId_t nPublishedFileID, AppId_t nAppID )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    if (nPublishedFileID == k_PublishedFileIdInvalid) return k_uAPICallInvalid;
    
    return k_uAPICallInvalid;
}

// request app dependencies. note that whatever callback you register for GetAppDependenciesResult_t may be called multiple times
// until all app dependencies have been returned
STEAM_CALL_RESULT( GetAppDependenciesResult_t )
SteamAPICall_t Steam_UGC::GetAppDependencies( PublishedFileId_t nPublishedFileID )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    if (nPublishedFileID == k_PublishedFileIdInvalid) return k_uAPICallInvalid;
    
    return k_uAPICallInvalid;
}


// delete the item without prompting the user
STEAM_CALL_RESULT( DeleteItemResult_t )
SteamAPICall_t Steam_UGC::DeleteItem( PublishedFileId_t nPublishedFileID )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    if (nPublishedFileID == k_PublishedFileIdInvalid) return k_uAPICallInvalid;
    
    return k_uAPICallInvalid;
}

// Show the app's latest Workshop EULA to the user in an overlay window, where they can accept it or not
bool Steam_UGC::ShowWorkshopEULA()
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return false;
}

// Retrieve information related to the user's acceptance or not of the app's specific Workshop EULA
STEAM_CALL_RESULT( WorkshopEULAStatus_t )
SteamAPICall_t Steam_UGC::GetWorkshopEULAStatus()
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    WorkshopEULAStatus_t data{};
    data.m_eResult = k_EResultOK;
    data.m_nAppID = settings->get_local_game_id().AppID();
    data.m_unVersion = 0; // TODO
    data.m_rtAction = (RTime32)std::chrono::duration_cast<std::chrono::seconds>(startup_time.time_since_epoch()).count();
    data.m_bAccepted = true;
    data.m_bNeedsAction = false;
    
    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return ret;
}

// Return the user's community content descriptor preferences
uint32 Steam_UGC::GetUserContentDescriptorPreferences( EUGCContentDescriptorID *pvecDescriptors, uint32 cMaxEntries )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return 0;
}
