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
public ISteamUGC
{
    class Settings *settings;
    class SteamCallResults *callback_results;
    class SteamCallBacks *callbacks;

    std::set<PublishedFileId_t> subscribed;
    UGCQueryHandle_t handle = 0;
    std::vector<struct UGC_query> ugc_queries;

UGCQueryHandle_t new_ugc_query(bool return_all_subscribed = false, std::set<PublishedFileId_t> return_only = std::set<PublishedFileId_t>())
{
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    struct UGC_query query;
    ++handle;
    query.handle = handle;
    query.return_all_subscribed = return_all_subscribed;
    query.return_only = return_only;
    ugc_queries.push_back(query);
    return query.handle;
}

void set_details(PublishedFileId_t id, SteamUGCDetails_t *pDetails)
{
    if (pDetails) {
        if (settings->isModInstalled(id)) {
            pDetails->m_eResult = k_EResultOK;
            pDetails->m_nPublishedFileId = id;
            pDetails->m_nCreatorAppID = settings->get_local_game_id().AppID();
            pDetails->m_nConsumerAppID = settings->get_local_game_id().AppID();
            snprintf(pDetails->m_rgchTitle, k_cchPublishedDocumentTitleMax, "%s", settings->getMod(id).title.c_str());
            pDetails->m_eFileType = settings->getMod(id).fileType;
            snprintf(pDetails->m_rgchDescription, k_cchPublishedDocumentDescriptionMax, "%s", settings->getMod(id).description.c_str());
            pDetails->m_ulSteamIDOwner = settings->get_local_steam_id().ConvertToUint64();
            pDetails->m_rtimeCreated = settings->getMod(id).timeCreated;
            pDetails->m_rtimeUpdated = settings->getMod(id).timeUpdated;
            pDetails->m_rtimeAddedToUserList = settings->getMod(id).timeAddedToUserList;
            pDetails->m_eVisibility = settings->getMod(id).visibility;
            pDetails->m_bBanned = settings->getMod(id).banned;
            pDetails->m_bAcceptedForUse = settings->getMod(id).acceptedForUse;
            pDetails->m_bTagsTruncated = settings->getMod(id).tagsTruncated;
            snprintf(pDetails->m_rgchTags, k_cchTagListMax, "%s", settings->getMod(id).tags.c_str());
            snprintf(pDetails->m_pchFileName, k_cchFilenameMax, "%s", settings->getMod(id).primaryFileName.c_str());
            pDetails->m_nFileSize = settings->getMod(id).primaryFileSize;
            pDetails->m_nPreviewFileSize = settings->getMod(id).previewFileSize;
            snprintf(pDetails->m_rgchURL, k_cchPublishedFileURLMax, "%s", settings->getMod(id).workshopItemURL.c_str());
            pDetails->m_unVotesUp = settings->getMod(id).votesUp;
            pDetails->m_unVotesDown = settings->getMod(id).votesDown;
            pDetails->m_flScore = settings->getMod(id).score;
            //pDetails->m_unNumChildren = settings->getMod(id).numChildren;
        } else {
            pDetails->m_nPublishedFileId = id;
            pDetails->m_eResult = k_EResultFail;
        }
    }
}

public:
Steam_UGC(class Settings *settings, class SteamCallResults *callback_results, class SteamCallBacks *callbacks)
{
    this->settings = settings;
    this->callbacks = callbacks;
    this->callback_results = callback_results;

    subscribed = settings->modSet();
}


// Query UGC associated with a user. Creator app id or consumer app id must be valid and be set to the current running app. unPage should start at 1.
UGCQueryHandle_t CreateQueryUserUGCRequest( AccountID_t unAccountID, EUserUGCList eListType, EUGCMatchingUGCType eMatchingUGCType, EUserUGCListSortOrder eSortOrder, AppId_t nCreatorAppID, AppId_t nConsumerAppID, uint32 unPage )
{
    PRINT_DEBUG("Steam_UGC::CreateQueryUserUGCRequest %u %i %i %i %u %u %u\n", unAccountID, eListType, eMatchingUGCType, eSortOrder, nCreatorAppID, nConsumerAppID, unPage);
    //TODO
    return new_ugc_query(eListType == k_EUserUGCList_Subscribed || eListType == k_EUserUGCList_Published);
}


// Query for all matching UGC. Creator app id or consumer app id must be valid and be set to the current running app. unPage should start at 1.
UGCQueryHandle_t CreateQueryAllUGCRequest( EUGCQuery eQueryType, EUGCMatchingUGCType eMatchingeMatchingUGCTypeFileType, AppId_t nCreatorAppID, AppId_t nConsumerAppID, uint32 unPage )
{
    PRINT_DEBUG("Steam_UGC::CreateQueryAllUGCRequest\n");
    //TODO
    return new_ugc_query();
}

// Query for all matching UGC using the new deep paging interface. Creator app id or consumer app id must be valid and be set to the current running app. pchCursor should be set to NULL or "*" to get the first result set.
UGCQueryHandle_t CreateQueryAllUGCRequest( EUGCQuery eQueryType, EUGCMatchingUGCType eMatchingeMatchingUGCTypeFileType, AppId_t nCreatorAppID, AppId_t nConsumerAppID, const char *pchCursor = NULL )
{
    PRINT_DEBUG("Steam_UGC::CreateQueryAllUGCRequest other\n");
    //TODO
    return new_ugc_query();
}

// Query for the details of the given published file ids (the RequestUGCDetails call is deprecated and replaced with this)
UGCQueryHandle_t CreateQueryUGCDetailsRequest( PublishedFileId_t *pvecPublishedFileID, uint32 unNumPublishedFileIDs )
{
    PRINT_DEBUG("Steam_UGC::CreateQueryUGCDetailsRequest\n");
    std::set<PublishedFileId_t> only(pvecPublishedFileID, pvecPublishedFileID + unNumPublishedFileIDs);
    return new_ugc_query(false, only);
}


// Send the query to Steam
STEAM_CALL_RESULT( SteamUGCQueryCompleted_t )
SteamAPICall_t SendQueryUGCRequest( UGCQueryHandle_t handle )
{
    PRINT_DEBUG("Steam_UGC::SendQueryUGCRequest\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request)
        return 0;

    if (request->return_all_subscribed) {
        request->results = subscribed;
    }

    if (request->return_only.size()) {
        for (auto & s : request->return_only) {
            if (subscribed.count(s)) {
                request->results.insert(s);
            }
        }
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
    PRINT_DEBUG("Steam_UGC::GetQueryUGCResult %u\n", index);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (pDetails) {
        memset(pDetails, 0, sizeof(SteamUGCDetails_t));
        pDetails->m_eResult = k_EResultFail;
    }

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request) {
        return false;
    }

    if (index >= request->results.size()) {
        return false;
    }

    auto it = request->results.begin();
    std::advance(it, index);
    set_details(*it, pDetails);

    return true;
}

uint32 GetQueryUGCNumTags( UGCQueryHandle_t handle, uint32 index )
{
    PRINT_DEBUG("Steam_UGC::GetQueryUGCNumTags\n");
    return 0;
}

bool GetQueryUGCTag( UGCQueryHandle_t handle, uint32 index, uint32 indexTag, STEAM_OUT_STRING_COUNT( cchValueSize ) char* pchValue, uint32 cchValueSize )
{
    PRINT_DEBUG("Steam_UGC::GetQueryUGCTag\n");
    return false;
}

bool GetQueryUGCTagDisplayName( UGCQueryHandle_t handle, uint32 index, uint32 indexTag, STEAM_OUT_STRING_COUNT( cchValueSize ) char* pchValue, uint32 cchValueSize )
{
    PRINT_DEBUG("Steam_UGC::GetQueryUGCTagDisplayName\n");
    return false;
}

bool GetQueryUGCPreviewURL( UGCQueryHandle_t handle, uint32 index, STEAM_OUT_STRING_COUNT(cchURLSize) char *pchURL, uint32 cchURLSize )
{
    PRINT_DEBUG("Steam_UGC::GetQueryUGCPreviewURL\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    //TODO: escape simulator tries downloading this url and unsubscribes if it fails

    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if ((ugc_queries.end() != request) && !(index >= request->results.size())) {
        auto it = request->results.begin();
        uint32 it2 = (uint32)*it;
        PRINT_DEBUG("Steam_UGC:GetQueryUGCPreviewURL: %u %s\n", it2, settings->getMod(it2).previewURL.c_str());
        snprintf(pchURL, cchURLSize, "%s", settings->getMod(it2).previewURL.c_str());
        return true;
    }

    return false;
}


bool GetQueryUGCMetadata( UGCQueryHandle_t handle, uint32 index, STEAM_OUT_STRING_COUNT(cchMetadatasize) char *pchMetadata, uint32 cchMetadatasize )
{
    PRINT_DEBUG("Steam_UGC::GetQueryUGCMetadata\n");
    return false;
}


bool GetQueryUGCChildren( UGCQueryHandle_t handle, uint32 index, PublishedFileId_t* pvecPublishedFileID, uint32 cMaxEntries )
{
    PRINT_DEBUG("Steam_UGC::GetQueryUGCChildren\n");
    return false;
}


bool GetQueryUGCStatistic( UGCQueryHandle_t handle, uint32 index, EItemStatistic eStatType, uint64 *pStatValue )
{
    PRINT_DEBUG("Steam_UGC::GetQueryUGCStatistic\n");
    return false;
}

bool GetQueryUGCStatistic( UGCQueryHandle_t handle, uint32 index, EItemStatistic eStatType, uint32 *pStatValue )
{
    PRINT_DEBUG("Steam_UGC::GetQueryUGCStatistic old\n");
    return false;
}

uint32 GetQueryUGCNumAdditionalPreviews( UGCQueryHandle_t handle, uint32 index )
{
    PRINT_DEBUG("Steam_UGC::GetQueryUGCNumAdditionalPreviews\n");
    return 0;
}


bool GetQueryUGCAdditionalPreview( UGCQueryHandle_t handle, uint32 index, uint32 previewIndex, STEAM_OUT_STRING_COUNT(cchURLSize) char *pchURLOrVideoID, uint32 cchURLSize, STEAM_OUT_STRING_COUNT(cchURLSize) char *pchOriginalFileName, uint32 cchOriginalFileNameSize, EItemPreviewType *pPreviewType )
{
    PRINT_DEBUG("Steam_UGC::GetQueryUGCAdditionalPreview\n");
    return false;
}

bool GetQueryUGCAdditionalPreview( UGCQueryHandle_t handle, uint32 index, uint32 previewIndex, char *pchURLOrVideoID, uint32 cchURLSize, bool *hz )
{
    PRINT_DEBUG("Steam_UGC::GetQueryUGCAdditionalPreview old\n");
    return false;
}

uint32 GetQueryUGCNumKeyValueTags( UGCQueryHandle_t handle, uint32 index )
{
    PRINT_DEBUG("Steam_UGC::GetQueryUGCNumKeyValueTags\n");
    return 0;
}


bool GetQueryUGCKeyValueTag( UGCQueryHandle_t handle, uint32 index, uint32 keyValueTagIndex, STEAM_OUT_STRING_COUNT(cchKeySize) char *pchKey, uint32 cchKeySize, STEAM_OUT_STRING_COUNT(cchValueSize) char *pchValue, uint32 cchValueSize )
{
    PRINT_DEBUG("Steam_UGC::GetQueryUGCKeyValueTag\n");
    return false;
}

bool GetQueryUGCKeyValueTag( UGCQueryHandle_t handle, uint32 index, const char *pchKey, STEAM_OUT_STRING_COUNT(cchValueSize) char *pchValue, uint32 cchValueSize )
{
    PRINT_DEBUG("Steam_UGC::GetQueryUGCKeyValueTag2\n");
    return false;
}

uint32 GetQueryUGCContentDescriptors( UGCQueryHandle_t handle, uint32 index, EUGCContentDescriptorID *pvecDescriptors, uint32 cMaxEntries )
{
    PRINT_DEBUG("Steam_UGC::GetQueryUGCContentDescriptors\n");
    return 0;
}

// Release the request to free up memory, after retrieving results
bool ReleaseQueryUGCRequest( UGCQueryHandle_t handle )
{
    PRINT_DEBUG("Steam_UGC::ReleaseQueryUGCRequest\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    auto request = std::find_if(ugc_queries.begin(), ugc_queries.end(), [&handle](struct UGC_query const& item) { return item.handle == handle; });
    if (ugc_queries.end() == request)
        return false;

    ugc_queries.erase(request);
    return true;
}


// Options to set for querying UGC
bool AddRequiredTag( UGCQueryHandle_t handle, const char *pTagName )
{
    PRINT_DEBUG("Steam_UGC::AddRequiredTag\n");
    return true;
}

bool AddRequiredTagGroup( UGCQueryHandle_t handle, const SteamParamStringArray_t *pTagGroups )
{
    PRINT_DEBUG("Steam_UGC::AddRequiredTagGroup\n");
    return true;
}

bool AddExcludedTag( UGCQueryHandle_t handle, const char *pTagName )
{
    PRINT_DEBUG("Steam_UGC::AddExcludedTag\n");
    return true;
}


bool SetReturnOnlyIDs( UGCQueryHandle_t handle, bool bReturnOnlyIDs )
{
    PRINT_DEBUG("Steam_UGC::SetReturnOnlyIDs\n");
    return true;
}


bool SetReturnKeyValueTags( UGCQueryHandle_t handle, bool bReturnKeyValueTags )
{
    PRINT_DEBUG("Steam_UGC::SetReturnKeyValueTags\n");
    return true;
}


bool SetReturnLongDescription( UGCQueryHandle_t handle, bool bReturnLongDescription )
{
    PRINT_DEBUG("Steam_UGC::SetReturnLongDescription\n");
    return true;
}


bool SetReturnMetadata( UGCQueryHandle_t handle, bool bReturnMetadata )
{
    PRINT_DEBUG("Steam_UGC::SetReturnMetadata\n");
    return true;
}


bool SetReturnChildren( UGCQueryHandle_t handle, bool bReturnChildren )
{
    PRINT_DEBUG("Steam_UGC::SetReturnChildren\n");
    return true;
}


bool SetReturnAdditionalPreviews( UGCQueryHandle_t handle, bool bReturnAdditionalPreviews )
{
    PRINT_DEBUG("Steam_UGC::SetReturnAdditionalPreviews\n");
    return true;
}


bool SetReturnTotalOnly( UGCQueryHandle_t handle, bool bReturnTotalOnly )
{
    PRINT_DEBUG("Steam_UGC::SetReturnTotalOnly\n");
    return true;
}


bool SetReturnPlaytimeStats( UGCQueryHandle_t handle, uint32 unDays )
{
    PRINT_DEBUG("Steam_UGC::SetReturnPlaytimeStats\n");
    return true;
}


bool SetLanguage( UGCQueryHandle_t handle, const char *pchLanguage )
{
    PRINT_DEBUG("Steam_UGC::SetLanguage\n");
    return true;
}


bool SetAllowCachedResponse( UGCQueryHandle_t handle, uint32 unMaxAgeSeconds )
{
    PRINT_DEBUG("Steam_UGC::SetAllowCachedResponse\n");
    return true;
}


// Options only for querying user UGC
bool SetCloudFileNameFilter( UGCQueryHandle_t handle, const char *pMatchCloudFileName )
{
    PRINT_DEBUG("Steam_UGC::SetCloudFileNameFilter\n");
    return true;
}


// Options only for querying all UGC
bool SetMatchAnyTag( UGCQueryHandle_t handle, bool bMatchAnyTag )
{
    PRINT_DEBUG("Steam_UGC::SetMatchAnyTag\n");
    return true;
}


bool SetSearchText( UGCQueryHandle_t handle, const char *pSearchText )
{
    PRINT_DEBUG("Steam_UGC::SetSearchText\n");
    return true;
}


bool SetRankedByTrendDays( UGCQueryHandle_t handle, uint32 unDays )
{
    PRINT_DEBUG("Steam_UGC::SetRankedByTrendDays\n");
    return true;
}


bool AddRequiredKeyValueTag( UGCQueryHandle_t handle, const char *pKey, const char *pValue )
{
    PRINT_DEBUG("Steam_UGC::AddRequiredKeyValueTag\n");
    return true;
}

bool SetTimeCreatedDateRange( UGCQueryHandle_t handle, RTime32 rtStart, RTime32 rtEnd )
{
    PRINT_DEBUG("Steam_UGC::SetTimeCreatedDateRange\n");
    return true;
}

bool SetTimeUpdatedDateRange( UGCQueryHandle_t handle, RTime32 rtStart, RTime32 rtEnd )
{
    PRINT_DEBUG("Steam_UGC::SetTimeUpdatedDateRange\n");
    return true;
}

// DEPRECATED - Use CreateQueryUGCDetailsRequest call above instead!
SteamAPICall_t RequestUGCDetails( PublishedFileId_t nPublishedFileID, uint32 unMaxAgeSeconds )
{
    PRINT_DEBUG("Steam_UGC::RequestUGCDetails\n");
    SteamUGCRequestUGCDetailsResult_t data = {};
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
    return 0;
}
 // create new item for this app with no content attached yet


UGCUpdateHandle_t StartItemUpdate( AppId_t nConsumerAppId, PublishedFileId_t nPublishedFileID )
{
    PRINT_DEBUG("Steam_UGC::StartItemUpdate\n");
    return 0;
}
 // start an UGC item update. Set changed properties before commiting update with CommitItemUpdate()


bool SetItemTitle( UGCUpdateHandle_t handle, const char *pchTitle )
{
    PRINT_DEBUG("Steam_UGC::SetItemTitle\n");
    return false;
}
 // change the title of an UGC item


bool SetItemDescription( UGCUpdateHandle_t handle, const char *pchDescription )
{
    PRINT_DEBUG("Steam_UGC::SetItemDescription\n");
    return false;
}
 // change the description of an UGC item


bool SetItemUpdateLanguage( UGCUpdateHandle_t handle, const char *pchLanguage )
{
    PRINT_DEBUG("Steam_UGC::SetItemUpdateLanguage\n");
    return false;
}
 // specify the language of the title or description that will be set


bool SetItemMetadata( UGCUpdateHandle_t handle, const char *pchMetaData )
{
    PRINT_DEBUG("Steam_UGC::SetItemMetadata\n");
    return false;
}
 // change the metadata of an UGC item (max = k_cchDeveloperMetadataMax)


bool SetItemVisibility( UGCUpdateHandle_t handle, ERemoteStoragePublishedFileVisibility eVisibility )
{
    PRINT_DEBUG("Steam_UGC::SetItemVisibility\n");
    return false;
}
 // change the visibility of an UGC item


bool SetItemTags( UGCUpdateHandle_t updateHandle, const SteamParamStringArray_t *pTags )
{
    PRINT_DEBUG("Steam_UGC::SetItemTags\n");
    return false;
}
 // change the tags of an UGC item


bool SetItemContent( UGCUpdateHandle_t handle, const char *pszContentFolder )
{
    PRINT_DEBUG("Steam_UGC::SetItemContent\n");
    return false;
}
 // update item content from this local folder


bool SetItemPreview( UGCUpdateHandle_t handle, const char *pszPreviewFile )
{
    PRINT_DEBUG("Steam_UGC::SetItemPreview\n");
    return false;
}
 //  change preview image file for this item. pszPreviewFile points to local image file, which must be under 1MB in size

bool SetAllowLegacyUpload( UGCUpdateHandle_t handle, bool bAllowLegacyUpload )
{
    PRINT_DEBUG("Steam_UGC::SetAllowLegacyUpload\n");
    return false;
}

bool RemoveAllItemKeyValueTags( UGCUpdateHandle_t handle )
{
    PRINT_DEBUG("Steam_UGC::RemoveAllItemKeyValueTags\n");
    return false;
}
 // remove all existing key-value tags (you can add new ones via the AddItemKeyValueTag function)

bool RemoveItemKeyValueTags( UGCUpdateHandle_t handle, const char *pchKey )
{
    PRINT_DEBUG("Steam_UGC::RemoveItemKeyValueTags\n");
    return false;
}
 // remove any existing key-value tags with the specified key


bool AddItemKeyValueTag( UGCUpdateHandle_t handle, const char *pchKey, const char *pchValue )
{
    PRINT_DEBUG("Steam_UGC::AddItemKeyValueTag\n");
    return false;
}
 // add new key-value tags for the item. Note that there can be multiple values for a tag.


bool AddItemPreviewFile( UGCUpdateHandle_t handle, const char *pszPreviewFile, EItemPreviewType type )
{
    PRINT_DEBUG("Steam_UGC::AddItemPreviewFile\n");
    return false;
}
 //  add preview file for this item. pszPreviewFile points to local file, which must be under 1MB in size


bool AddItemPreviewVideo( UGCUpdateHandle_t handle, const char *pszVideoID )
{
    PRINT_DEBUG("Steam_UGC::AddItemPreviewVideo\n");
    return false;
}
 //  add preview video for this item


bool UpdateItemPreviewFile( UGCUpdateHandle_t handle, uint32 index, const char *pszPreviewFile )
{
    PRINT_DEBUG("Steam_UGC::UpdateItemPreviewFile\n");
    return false;
}
 //  updates an existing preview file for this item. pszPreviewFile points to local file, which must be under 1MB in size


bool UpdateItemPreviewVideo( UGCUpdateHandle_t handle, uint32 index, const char *pszVideoID )
{
    PRINT_DEBUG("Steam_UGC::UpdateItemPreviewVideo\n");
    return false;
}
 //  updates an existing preview video for this item


bool RemoveItemPreview( UGCUpdateHandle_t handle, uint32 index )
{
    PRINT_DEBUG("Steam_UGC::RemoveItemPreview %llu %u\n", handle, index);
    return false;
}
 // remove a preview by index starting at 0 (previews are sorted)

bool AddContentDescriptor( UGCUpdateHandle_t handle, EUGCContentDescriptorID descid )
{
    PRINT_DEBUG("Steam_UGC::AddContentDescriptor %llu %u\n", handle, descid);
    return false;
}

bool RemoveContentDescriptor( UGCUpdateHandle_t handle, EUGCContentDescriptorID descid )
{
    PRINT_DEBUG("Steam_UGC::RemoveContentDescriptor %llu %u\n", handle, descid);
    return false;
}

STEAM_CALL_RESULT( SubmitItemUpdateResult_t )
SteamAPICall_t SubmitItemUpdate( UGCUpdateHandle_t handle, const char *pchChangeNote )
{
    PRINT_DEBUG("Steam_UGC::SubmitItemUpdate\n");
    return 0;
}
 // commit update process started with StartItemUpdate()


EItemUpdateStatus GetItemUpdateProgress( UGCUpdateHandle_t handle, uint64 *punBytesProcessed, uint64* punBytesTotal )
{
    PRINT_DEBUG("Steam_UGC::GetItemUpdateProgress\n");
    return k_EItemUpdateStatusInvalid;
}


// Steam Workshop Consumer API

STEAM_CALL_RESULT( SetUserItemVoteResult_t )
SteamAPICall_t SetUserItemVote( PublishedFileId_t nPublishedFileID, bool bVoteUp )
{
    PRINT_DEBUG("Steam_UGC::SetUserItemVote\n");
    return 0;
}


STEAM_CALL_RESULT( GetUserItemVoteResult_t )
SteamAPICall_t GetUserItemVote( PublishedFileId_t nPublishedFileID )
{
    PRINT_DEBUG("Steam_UGC::GetUserItemVote\n");
    return 0;
}


STEAM_CALL_RESULT( UserFavoriteItemsListChanged_t )
SteamAPICall_t AddItemToFavorites( AppId_t nAppId, PublishedFileId_t nPublishedFileID )
{
    PRINT_DEBUG("Steam_UGC::AddItemToFavorites\n");
    return 0;
}


STEAM_CALL_RESULT( UserFavoriteItemsListChanged_t )
SteamAPICall_t RemoveItemFromFavorites( AppId_t nAppId, PublishedFileId_t nPublishedFileID )
{
    PRINT_DEBUG("Steam_UGC::RemoveItemFromFavorites\n");
    return 0;
}


STEAM_CALL_RESULT( RemoteStorageSubscribePublishedFileResult_t )
SteamAPICall_t SubscribeItem( PublishedFileId_t nPublishedFileID )
{
    PRINT_DEBUG("Steam_UGC::SubscribeItem %llu\n", nPublishedFileID);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    subscribed.insert(nPublishedFileID);

    RemoteStorageSubscribePublishedFileResult_t data;
    if (settings->isModInstalled(nPublishedFileID)) {
        data.m_eResult = k_EResultOK;
    } else {
        data.m_eResult = k_EResultFail;
    }
    data.m_nPublishedFileId = nPublishedFileID;
    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}
 // subscribe to this item, will be installed ASAP

STEAM_CALL_RESULT( RemoteStorageUnsubscribePublishedFileResult_t )
SteamAPICall_t UnsubscribeItem( PublishedFileId_t nPublishedFileID )
{
    PRINT_DEBUG("Steam_UGC::UnsubscribeItem %llu\n", nPublishedFileID);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    RemoteStorageUnsubscribePublishedFileResult_t data;
    data.m_eResult = k_EResultOK;
    if (subscribed.count(nPublishedFileID) == 0) {
        data.m_eResult = k_EResultFail; //TODO: check if this is accurate
    }

    subscribed.erase(nPublishedFileID);
    data.m_nPublishedFileId = nPublishedFileID;
    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}
 // unsubscribe from this item, will be uninstalled after game quits

uint32 GetNumSubscribedItems()
{
    PRINT_DEBUG("Steam_UGC::GetNumSubscribedItems\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return subscribed.size();
}
 // number of subscribed items 

uint32 GetSubscribedItems( PublishedFileId_t* pvecPublishedFileID, uint32 cMaxEntries )
{
    PRINT_DEBUG("Steam_UGC::GetSubscribedItems\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (cMaxEntries > subscribed.size()) {
        cMaxEntries = subscribed.size();
    }

    std::copy_n(subscribed.begin(), cMaxEntries, pvecPublishedFileID);
    return cMaxEntries;
}
 // all subscribed item PublishFileIDs

// get EItemState flags about item on this client
uint32 GetItemState( PublishedFileId_t nPublishedFileID )
{
    PRINT_DEBUG("Steam_UGC::GetItemState %llu\n", nPublishedFileID);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (subscribed.count(nPublishedFileID)) {
        if (settings->isModInstalled(nPublishedFileID)) {
            return k_EItemStateInstalled | k_EItemStateSubscribed;
        }

        return k_EItemStateSubscribed;
    }

    return k_EItemStateNone;
}


// get info about currently installed content on disc for items that have k_EItemStateInstalled set
// if k_EItemStateLegacyItem is set, pchFolder contains the path to the legacy file itself (not a folder)
bool GetItemInstallInfo( PublishedFileId_t nPublishedFileID, uint64 *punSizeOnDisk, STEAM_OUT_STRING_COUNT( cchFolderSize ) char *pchFolder, uint32 cchFolderSize, uint32 *punTimeStamp )
{
    PRINT_DEBUG("Steam_UGC::GetItemInstallInfo\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!settings->isModInstalled(nPublishedFileID)) {
        return false;
    }

    if (punSizeOnDisk) *punSizeOnDisk = settings->getMod(nPublishedFileID).primaryFileSize;
    if (punTimeStamp) *punTimeStamp = settings->getMod(nPublishedFileID).timeCreated;
    if (pchFolder && cchFolderSize) {
        snprintf(pchFolder, cchFolderSize, "%s", settings->getMod(nPublishedFileID).path.c_str());
    }

    return true;
}


// get info about pending update for items that have k_EItemStateNeedsUpdate set. punBytesTotal will be valid after download started once
bool GetItemDownloadInfo( PublishedFileId_t nPublishedFileID, uint64 *punBytesDownloaded, uint64 *punBytesTotal )
{
    PRINT_DEBUG("Steam_UGC::GetItemDownloadInfo\n");
    return false;
}

bool GetItemInstallInfo( PublishedFileId_t nPublishedFileID, uint64 *punSizeOnDisk, STEAM_OUT_STRING_COUNT( cchFolderSize ) char *pchFolder, uint32 cchFolderSize, bool *pbLegacyItem ) // returns true if item is installed
{
    PRINT_DEBUG("Steam_UGC::GetItemInstallInfo old\n");
    return GetItemInstallInfo(nPublishedFileID, punSizeOnDisk, pchFolder, cchFolderSize, (uint32*) nullptr);
}

bool GetItemUpdateInfo( PublishedFileId_t nPublishedFileID, bool *pbNeedsUpdate, bool *pbIsDownloading, uint64 *punBytesDownloaded, uint64 *punBytesTotal )
{
    PRINT_DEBUG("Steam_UGC::GetItemDownloadInfo old\n");
    return false;
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
    return false;
}


// game servers can set a specific workshop folder before issuing any UGC commands.
// This is helpful if you want to support multiple game servers running out of the same install folder
bool BInitWorkshopForGameServer( DepotId_t unWorkshopDepotID, const char *pszFolder )
{
    PRINT_DEBUG("Steam_UGC::BInitWorkshopForGameServer\n");
    return false;
}


// SuspendDownloads( true ) will suspend all workshop downloads until SuspendDownloads( false ) is called or the game ends
void SuspendDownloads( bool bSuspend )
{
    PRINT_DEBUG("Steam_UGC::SuspendDownloads\n");
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
    return 0;
}

STEAM_CALL_RESULT( RemoveUGCDependencyResult_t )
SteamAPICall_t RemoveDependency( PublishedFileId_t nParentPublishedFileID, PublishedFileId_t nChildPublishedFileID )
{
    PRINT_DEBUG("Steam_UGC::RemoveDependency\n");
    return 0;
}


// add/remove app dependence/requirements (usually DLC)
STEAM_CALL_RESULT( AddAppDependencyResult_t )
SteamAPICall_t AddAppDependency( PublishedFileId_t nPublishedFileID, AppId_t nAppID )
{
    PRINT_DEBUG("Steam_UGC::AddAppDependency\n");
    return 0;
}

STEAM_CALL_RESULT( RemoveAppDependencyResult_t )
SteamAPICall_t RemoveAppDependency( PublishedFileId_t nPublishedFileID, AppId_t nAppID )
{
    PRINT_DEBUG("Steam_UGC::RemoveAppDependency\n");
    return 0;
}

// request app dependencies. note that whatever callback you register for GetAppDependenciesResult_t may be called multiple times
// until all app dependencies have been returned
STEAM_CALL_RESULT( GetAppDependenciesResult_t )
SteamAPICall_t GetAppDependencies( PublishedFileId_t nPublishedFileID )
{
    PRINT_DEBUG("Steam_UGC::GetAppDependencies\n");
    return 0;
}


// delete the item without prompting the user
STEAM_CALL_RESULT( DeleteItemResult_t )
SteamAPICall_t DeleteItem( PublishedFileId_t nPublishedFileID )
{
    PRINT_DEBUG("Steam_UGC::DeleteItem\n");
    return 0;
}

// Show the app's latest Workshop EULA to the user in an overlay window, where they can accept it or not
bool ShowWorkshopEULA()
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
    return false;
}

// Retrieve information related to the user's acceptance or not of the app's specific Workshop EULA
STEAM_CALL_RESULT( WorkshopEULAStatus_t )
SteamAPICall_t GetWorkshopEULAStatus()
{
    PRINT_DEBUG("%s\n", __FUNCTION__);
    return 0;
}


};
