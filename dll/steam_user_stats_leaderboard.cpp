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

#include "dll/steam_user_stats.h"
#include <random>


// --- Steam_Leaderboard ---

Steam_Leaderboard_Entry* Steam_Leaderboard::find_recent_entry(const CSteamID &steamid) const
{
    auto my_it = std::find_if(entries.begin(), entries.end(), [&steamid](const Steam_Leaderboard_Entry &item) {
        return item.steam_id == steamid;
    });
    if (entries.end() == my_it) return nullptr;
    return const_cast<Steam_Leaderboard_Entry*>(&*my_it);
}

void Steam_Leaderboard::remove_entries(const CSteamID &steamid)
{
    auto rm_it = std::remove_if(entries.begin(), entries.end(), [&](const Steam_Leaderboard_Entry &item){
        return item.steam_id == steamid;
    });
    if (entries.end() != rm_it) entries.erase(rm_it, entries.end());
}

void Steam_Leaderboard::remove_duplicate_entries()
{
    if (entries.size() <= 1) return;

    auto rm = std::remove_if(entries.begin(), entries.end(), [&](const Steam_Leaderboard_Entry& item) {
        auto recent = find_recent_entry(item.steam_id);
        return &item != recent;
    });
    if (entries.end() != rm) entries.erase(rm, entries.end());
}

void Steam_Leaderboard::sort_entries()
{
    if (sort_method == k_ELeaderboardSortMethodNone) return;
    if (entries.size() <= 1) return;

    std::sort(entries.begin(), entries.end(), [this](const Steam_Leaderboard_Entry &item1, const Steam_Leaderboard_Entry &item2) {
        if (sort_method == k_ELeaderboardSortMethodAscending) {
            return item1.score < item2.score;
        } else { // k_ELeaderboardSortMethodDescending
            return item1.score > item2.score;
        }
    });

}

// --- Steam_Leaderboard ---


/*
layout of each item in the leaderboard file
| steamid - lower 32-bits | steamid - higher 32-bits | score (4 bytes) | score details count (4 bytes) | score details array (4 bytes each) ...
  [0]                     | [1]                      | [2]             | [3]                            | [4]
  ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ main header ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
*/

std::vector<Steam_Leaderboard_Entry> Steam_User_Stats::load_leaderboard_entries(const std::string &name)
{
    constexpr const static unsigned int MAIN_HEADER_ELEMENTS_COUNT = 4;
    constexpr const static unsigned int ELEMENT_SIZE = (unsigned int)sizeof(uint32_t);

    std::vector<Steam_Leaderboard_Entry> out{};

    std::string leaderboard_name(common_helpers::to_lower(name));
    unsigned read_bytes = local_storage->file_size(Local_Storage::leaderboard_storage_folder, leaderboard_name);
    if ((read_bytes == 0) ||
        (read_bytes < (ELEMENT_SIZE * MAIN_HEADER_ELEMENTS_COUNT)) ||
        (read_bytes % ELEMENT_SIZE) != 0) {
        return out;
    }

    std::vector<uint32_t> output(read_bytes / ELEMENT_SIZE);
    if (local_storage->get_data(Local_Storage::leaderboard_storage_folder, leaderboard_name, (char* )output.data(), read_bytes) != read_bytes) return out;

    unsigned int i = 0;
    while (true) {
        if ((i + MAIN_HEADER_ELEMENTS_COUNT) > output.size()) break; // invalid main header, or end of buffer

        Steam_Leaderboard_Entry new_entry{};
        new_entry.steam_id = CSteamID((uint64)output[i] + (((uint64)output[i + 1]) << 32));
        new_entry.score = (int32)output[i + 2];
        uint32_t details_count = output[i + 3];
        i += MAIN_HEADER_ELEMENTS_COUNT; // skip main header

        if ((i + details_count) > output.size()) break; // invalid score details count

        for (uint32_t j = 0; j < details_count; ++j) {
            new_entry.score_details.push_back(output[i]);
            ++i; // move past this score detail
        }

        PRINT_DEBUG("'%s': user %llu, score %i, details count = %zu",
            name.c_str(), new_entry.steam_id.ConvertToUint64(), new_entry.score, new_entry.score_details.size()
        );
        out.push_back(new_entry);
    }

    PRINT_DEBUG("'%s' total entries = %zu", name.c_str(), out.size());
    return out;
}

void Steam_User_Stats::save_my_leaderboard_entry(const Steam_Leaderboard &leaderboard)
{
     auto my_entry = leaderboard.find_recent_entry(settings->get_local_steam_id());
     if (!my_entry) return; // we don't have a score entry

    PRINT_DEBUG("saving entries for leaderboard '%s'", leaderboard.name.c_str());

    std::vector<uint32_t> output{};

    uint64_t steam_id = my_entry->steam_id.ConvertToUint64();
    output.push_back((uint32_t)(steam_id & 0xFFFFFFFF)); // lower 4 bytes
    output.push_back((uint32_t)(steam_id >> 32)); // higher 4 bytes

    output.push_back(my_entry->score);
    output.push_back((uint32_t)my_entry->score_details.size());
    for (const auto &detail : my_entry->score_details) {
        output.push_back(detail);
    }

    std::string leaderboard_name(common_helpers::to_lower(leaderboard.name));
    unsigned int buffer_size = static_cast<unsigned int>(output.size() * sizeof(output[0])); // in bytes
    local_storage->store_data(Local_Storage::leaderboard_storage_folder, leaderboard_name, (char* )&output[0], buffer_size);
}

Steam_Leaderboard_Entry* Steam_User_Stats::update_leaderboard_entry(Steam_Leaderboard &leaderboard, const Steam_Leaderboard_Entry &entry, bool overwrite)
{
    bool added = false;
    auto user_entry = leaderboard.find_recent_entry(entry.steam_id);
    if (!user_entry) { // user doesn't have an entry yet, create one
        added = true;
        leaderboard.entries.push_back(entry);
        user_entry = &leaderboard.entries.back();
    } else if (overwrite) {
        added = true;
        *user_entry = entry;
    }

    if (added) { // if we added a new entry then we have to sort and find the target entry again
        leaderboard.sort_entries();
        user_entry = leaderboard.find_recent_entry(entry.steam_id);
        PRINT_DEBUG("added/updated entry for user %llu", entry.steam_id.ConvertToUint64());
    }
    
    return user_entry;
}


unsigned int Steam_User_Stats::find_cached_leaderboard(const std::string &name)
{
    unsigned index = 1;
    for (const auto &leaderboard : cached_leaderboards) {
        if (common_helpers::str_cmp_insensitive(leaderboard.name, name)) return index;

        ++index;
    }

    return 0;
}

unsigned int Steam_User_Stats::cache_leaderboard_ifneeded(const std::string &name, ELeaderboardSortMethod eLeaderboardSortMethod, ELeaderboardDisplayType eLeaderboardDisplayType)
{
    unsigned int board_handle = find_cached_leaderboard(name);
    if (board_handle) return board_handle;
    // PRINT_DEBUG("cache miss '%s'", name.c_str());

    // create a new entry in-memory and try reading the entries from disk
    struct Steam_Leaderboard new_board{};
    // don't make this lower/upper case, appid 1372280 later calls GetLeaderboardName() and hangs if the name wasn't the same as the original
    new_board.name = name;
    new_board.sort_method = eLeaderboardSortMethod;
    new_board.display_type = eLeaderboardDisplayType;
    new_board.entries = load_leaderboard_entries(name);

    new_board.sort_entries();
    new_board.remove_duplicate_entries();

    // save it in memory for later
    cached_leaderboards.push_back(new_board);
    board_handle = static_cast<unsigned int>(cached_leaderboards.size());

    PRINT_DEBUG("cached a new leaderboard '%s' %i %i",
        new_board.name.c_str(), (int)eLeaderboardSortMethod, (int)eLeaderboardDisplayType
    );
    return board_handle;
}

void Steam_User_Stats::send_my_leaderboard_score(const Steam_Leaderboard &board, const CSteamID *steamid, bool want_scores_back)
{
    if (!settings->share_leaderboards_over_network) return;

    const auto my_entry = board.find_recent_entry(settings->get_local_steam_id());
    Leaderboards_Messages::UserScoreEntry *score_entry_msg = nullptr;
    
    if (my_entry) {
        score_entry_msg = new Leaderboards_Messages::UserScoreEntry();
        score_entry_msg->set_score(my_entry->score);
        score_entry_msg->mutable_score_details()->Assign(my_entry->score_details.begin(), my_entry->score_details.end());
    }

    auto board_info_msg = new Leaderboards_Messages::LeaderboardInfo();
    board_info_msg->set_allocated_board_name(new std::string(board.name));
    board_info_msg->set_sort_method(board.sort_method);
    board_info_msg->set_display_type(board.display_type);

    auto board_msg = new Leaderboards_Messages();
    if (want_scores_back) board_msg->set_type(Leaderboards_Messages::UpdateUserScoreMutual);
    else board_msg->set_type(Leaderboards_Messages::UpdateUserScore);
    board_msg->set_appid(settings->get_local_game_id().AppID());
    board_msg->set_allocated_leaderboard_info(board_info_msg);
    // if we have an entry
    if (score_entry_msg) board_msg->set_allocated_user_score_entry(score_entry_msg);

    Common_Message common_msg{};
    common_msg.set_source_id(settings->get_local_steam_id().ConvertToUint64());
    if (steamid) common_msg.set_dest_id(steamid->ConvertToUint64());
    common_msg.set_allocated_leaderboards_messages(board_msg);

    if (steamid) network->sendTo(&common_msg, false);
    else network->sendToAll(&common_msg, false);
}

void Steam_User_Stats::request_user_leaderboard_entry(const Steam_Leaderboard &board, const CSteamID &steamid)
{
    if (!settings->share_leaderboards_over_network) return;

    auto board_info_msg = new Leaderboards_Messages::LeaderboardInfo();
    board_info_msg->set_allocated_board_name(new std::string(board.name));
    board_info_msg->set_sort_method(board.sort_method);
    board_info_msg->set_display_type(board.display_type);

    auto board_msg = new Leaderboards_Messages();
    board_msg->set_type(Leaderboards_Messages::RequestUserScore);
    board_msg->set_appid(settings->get_local_game_id().AppID());
    board_msg->set_allocated_leaderboard_info(board_info_msg);

    Common_Message common_msg{};
    common_msg.set_source_id(settings->get_local_steam_id().ConvertToUint64());
    common_msg.set_dest_id(steamid.ConvertToUint64());
    common_msg.set_allocated_leaderboards_messages(board_msg);

    network->sendTo(&common_msg, false);
}


void Steam_User_Stats::steam_user_stats_network_leaderboards(void *object, Common_Message *msg)
{
    // PRINT_DEBUG_ENTRY();

    auto inst = (Steam_User_Stats *)object;
    inst->network_callback_leaderboards(msg);
}



// Leaderboard functions

// asks the Steam back-end for a leaderboard by name, and will create it if it's not yet
// This call is asynchronous, with the result returned in LeaderboardFindResult_t
STEAM_CALL_RESULT(LeaderboardFindResult_t)
SteamAPICall_t Steam_User_Stats::FindOrCreateLeaderboard( const char *pchLeaderboardName, ELeaderboardSortMethod eLeaderboardSortMethod, ELeaderboardDisplayType eLeaderboardDisplayType )
{
    PRINT_DEBUG("'%s'", pchLeaderboardName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!pchLeaderboardName) {
        LeaderboardFindResult_t data{};
        data.m_hSteamLeaderboard = 0;
        data.m_bLeaderboardFound = 0;
        auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
        callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
        return ret;
    }

    unsigned int board_handle = cache_leaderboard_ifneeded(pchLeaderboardName, eLeaderboardSortMethod, eLeaderboardDisplayType);
    send_my_leaderboard_score(cached_leaderboards[board_handle - 1], nullptr, true);

    LeaderboardFindResult_t data{};
    data.m_hSteamLeaderboard = board_handle;
    data.m_bLeaderboardFound = 1;
    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data), 0.1); // TODO is the timing ok?
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data), 0.1);
    return ret;
}


// as above, but won't create the leaderboard if it's not found
// This call is asynchronous, with the result returned in LeaderboardFindResult_t
STEAM_CALL_RESULT( LeaderboardFindResult_t )
SteamAPICall_t Steam_User_Stats::FindLeaderboard( const char *pchLeaderboardName )
{
    PRINT_DEBUG("'%s'", pchLeaderboardName);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (!pchLeaderboardName) {
        LeaderboardFindResult_t data{};
        data.m_hSteamLeaderboard = 0;
        data.m_bLeaderboardFound = 0;
        auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
        callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
        return ret;
    }

    std::string board_name(pchLeaderboardName);
    const auto &settings_Leaderboards = settings->getLeaderboards();
    auto it = std::find_if(settings_Leaderboards.begin(), settings_Leaderboards.end(), [&board_name](const std::pair<const std::string, Leaderboard_config> &item){
        return common_helpers::str_cmp_insensitive(item.first, board_name);
    });
    if (settings_Leaderboards.end() != it) {
        auto &config = it->second;
        return FindOrCreateLeaderboard(pchLeaderboardName, config.sort_method, config.display_type);
    } else if (!settings->disable_leaderboards_create_unknown) {
        return FindOrCreateLeaderboard(pchLeaderboardName, k_ELeaderboardSortMethodDescending, k_ELeaderboardDisplayTypeNumeric);
    } else {
        LeaderboardFindResult_t data{};
        data.m_hSteamLeaderboard = find_cached_leaderboard(board_name);
        data.m_bLeaderboardFound = !!data.m_hSteamLeaderboard;
        auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
        callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
        return ret;
    }
}


// returns the name of a leaderboard
const char * Steam_User_Stats::GetLeaderboardName( SteamLeaderboard_t hSteamLeaderboard )
{
    PRINT_DEBUG("%llu", hSteamLeaderboard);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (hSteamLeaderboard > cached_leaderboards.size() || hSteamLeaderboard <= 0) return "";

    auto name_ptr = cached_leaderboards[static_cast<unsigned>(hSteamLeaderboard - 1)].name.c_str();
    PRINT_DEBUG("  returned '%s'", name_ptr);
    return name_ptr;
}


// returns the total number of entries in a leaderboard, as of the last request
int Steam_User_Stats::GetLeaderboardEntryCount( SteamLeaderboard_t hSteamLeaderboard )
{
    PRINT_DEBUG("%llu", hSteamLeaderboard);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (hSteamLeaderboard > cached_leaderboards.size() || hSteamLeaderboard <= 0) return 0;

    return (int)cached_leaderboards[static_cast<unsigned>(hSteamLeaderboard - 1)].entries.size();
}


// returns the sort method of the leaderboard
ELeaderboardSortMethod Steam_User_Stats::GetLeaderboardSortMethod( SteamLeaderboard_t hSteamLeaderboard )
{
    PRINT_DEBUG("%llu", hSteamLeaderboard);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (hSteamLeaderboard > cached_leaderboards.size() || hSteamLeaderboard <= 0) return k_ELeaderboardSortMethodNone;

    return cached_leaderboards[static_cast<unsigned>(hSteamLeaderboard - 1)].sort_method;
}


// returns the display type of the leaderboard
ELeaderboardDisplayType Steam_User_Stats::GetLeaderboardDisplayType( SteamLeaderboard_t hSteamLeaderboard )
{
    PRINT_DEBUG("%llu", hSteamLeaderboard);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (hSteamLeaderboard > cached_leaderboards.size() || hSteamLeaderboard <= 0) return k_ELeaderboardDisplayTypeNone;

    return cached_leaderboards[static_cast<unsigned>(hSteamLeaderboard - 1)].display_type;
}


// Asks the Steam back-end for a set of rows in the leaderboard.
// This call is asynchronous, with the result returned in LeaderboardScoresDownloaded_t
// LeaderboardScoresDownloaded_t will contain a handle to pull the results from GetDownloadedLeaderboardEntries() (below)
// You can ask for more entries than exist, and it will return as many as do exist.
// k_ELeaderboardDataRequestGlobal requests rows in the leaderboard from the full table, with nRangeStart & nRangeEnd in the range [1, TotalEntries]
// k_ELeaderboardDataRequestGlobalAroundUser requests rows around the current user, nRangeStart being negate
//   e.g. DownloadLeaderboardEntries( hLeaderboard, k_ELeaderboardDataRequestGlobalAroundUser, -3, 3 ) will return 7 rows, 3 before the user, 3 after
// k_ELeaderboardDataRequestFriends requests all the rows for friends of the current user 
STEAM_CALL_RESULT( LeaderboardScoresDownloaded_t )
SteamAPICall_t Steam_User_Stats::DownloadLeaderboardEntries( SteamLeaderboard_t hSteamLeaderboard, ELeaderboardDataRequest eLeaderboardDataRequest, int nRangeStart, int nRangeEnd )
{
    PRINT_DEBUG("%llu %i [%i, %i]", hSteamLeaderboard, eLeaderboardDataRequest, nRangeStart, nRangeEnd);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (hSteamLeaderboard > cached_leaderboards.size() || hSteamLeaderboard <= 0) return k_uAPICallInvalid; //might return callresult even if hSteamLeaderboard is invalid

    int entries_count = (int)cached_leaderboards[static_cast<unsigned>(hSteamLeaderboard - 1)].entries.size();
    // https://partner.steamgames.com/doc/api/ISteamUserStats#ELeaderboardDataRequest
    if (eLeaderboardDataRequest != k_ELeaderboardDataRequestFriends) {
        int required_count = nRangeEnd - nRangeStart + 1;
        if (required_count < 0) required_count = 0;
        
        if (required_count < entries_count) entries_count = required_count;
    }
    LeaderboardScoresDownloaded_t data{};
    data.m_hSteamLeaderboard = hSteamLeaderboard;
    data.m_hSteamLeaderboardEntries = hSteamLeaderboard;
    data.m_cEntryCount = entries_count;
    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data), 0.1); // TODO is this timing ok?
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data), 0.1);
    return ret;
}

// as above, but downloads leaderboard entries for an arbitrary set of users - ELeaderboardDataRequest is k_ELeaderboardDataRequestUsers
// if a user doesn't have a leaderboard entry, they won't be included in the result
// a max of 100 users can be downloaded at a time, with only one outstanding call at a time
STEAM_METHOD_DESC(Downloads leaderboard entries for an arbitrary set of users - ELeaderboardDataRequest is k_ELeaderboardDataRequestUsers)
STEAM_CALL_RESULT( LeaderboardScoresDownloaded_t )
SteamAPICall_t Steam_User_Stats::DownloadLeaderboardEntriesForUsers( SteamLeaderboard_t hSteamLeaderboard,
                                                            STEAM_ARRAY_COUNT_D(cUsers, Array of users to retrieve) CSteamID *prgUsers, int cUsers )
{
    PRINT_DEBUG("%i %llu", cUsers, cUsers > 0 ? prgUsers[0].ConvertToUint64() : 0);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (hSteamLeaderboard > cached_leaderboards.size() || hSteamLeaderboard <= 0) return k_uAPICallInvalid; //might return callresult even if hSteamLeaderboard is invalid

    auto& board = cached_leaderboards[static_cast<unsigned>(hSteamLeaderboard - 1)];
    bool ok = true;
    int total_count = 0;
    if (prgUsers && cUsers > 0) {
        for (int i = 0; i < cUsers; ++i) {
            const auto &user_steamid = prgUsers[i];
            if (!user_steamid.IsValid()) {
                ok = false;
                PRINT_DEBUG("bad userid %llu", user_steamid.ConvertToUint64());
                break;
            }
            if (board.find_recent_entry(user_steamid)) ++total_count;

            request_user_leaderboard_entry(board, user_steamid);
        }
    }

    PRINT_DEBUG("total count %i", total_count);
    // https://partner.steamgames.com/doc/api/ISteamUserStats#DownloadLeaderboardEntriesForUsers
    if (!ok || total_count > 100) return k_uAPICallInvalid;

    LeaderboardScoresDownloaded_t data{};
    data.m_hSteamLeaderboard = hSteamLeaderboard;
    data.m_hSteamLeaderboardEntries = hSteamLeaderboard;
    data.m_cEntryCount = total_count;
    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data), 0.1); // TODO is this timing ok?
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data), 0.1);
    return ret;
}


// Returns data about a single leaderboard entry
// use a for loop from 0 to LeaderboardScoresDownloaded_t::m_cEntryCount to get all the downloaded entries
// e.g.
//		void OnLeaderboardScoresDownloaded( LeaderboardScoresDownloaded_t *pLeaderboardScoresDownloaded )
//		{
//			for ( int index = 0; index < pLeaderboardScoresDownloaded->m_cEntryCount; index++ )
//			{
//				LeaderboardEntry_t leaderboardEntry;
//				int32 details[3];		// we know this is how many we've stored previously
//				GetDownloadedLeaderboardEntry( pLeaderboardScoresDownloaded->m_hSteamLeaderboardEntries, index, &leaderboardEntry, details, 3 );
//				assert( leaderboardEntry.m_cDetails == 3 );
//				...
//			}
// once you've accessed all the entries, the data will be free'd, and the SteamLeaderboardEntries_t handle will become invalid
bool Steam_User_Stats::GetDownloadedLeaderboardEntry( SteamLeaderboardEntries_t hSteamLeaderboardEntries, int index, LeaderboardEntry_t *pLeaderboardEntry, int32 *pDetails, int cDetailsMax )
{
    PRINT_DEBUG("[%i] (%i) %llu %p %p", index, cDetailsMax, hSteamLeaderboardEntries, pLeaderboardEntry, pDetails);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (hSteamLeaderboardEntries > cached_leaderboards.size() || hSteamLeaderboardEntries <= 0) return false;
    
    const auto &board = cached_leaderboards[static_cast<unsigned>(hSteamLeaderboardEntries - 1)];
    if (index < 0 || static_cast<size_t>(index) >= board.entries.size()) return false;

    const auto &target_entry = board.entries[index];
    
    if (pLeaderboardEntry) {
        LeaderboardEntry_t entry{};
        entry.m_steamIDUser = target_entry.steam_id;
        entry.m_nGlobalRank = 1 + (int)(&target_entry - &board.entries[0]);
        entry.m_nScore = target_entry.score;
        
        *pLeaderboardEntry = entry;
    }
    
    if (pDetails && cDetailsMax > 0) {
        for (unsigned i = 0; i < target_entry.score_details.size() && i < static_cast<unsigned>(cDetailsMax); ++i) {
            pDetails[i] = target_entry.score_details[i];
        }
    }

    return true;
}


// Uploads a user score to the Steam back-end.
// This call is asynchronous, with the result returned in LeaderboardScoreUploaded_t
// Details are extra game-defined information regarding how the user got that score
// pScoreDetails points to an array of int32's, cScoreDetailsCount is the number of int32's in the list
STEAM_CALL_RESULT( LeaderboardScoreUploaded_t )
SteamAPICall_t Steam_User_Stats::UploadLeaderboardScore( SteamLeaderboard_t hSteamLeaderboard, ELeaderboardUploadScoreMethod eLeaderboardUploadScoreMethod, int32 nScore, const int32 *pScoreDetails, int cScoreDetailsCount )
{
    PRINT_DEBUG("%llu %i", hSteamLeaderboard, nScore);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (hSteamLeaderboard > cached_leaderboards.size() || hSteamLeaderboard <= 0) return k_uAPICallInvalid; //TODO: might return callresult even if hSteamLeaderboard is invalid

    auto &board = cached_leaderboards[static_cast<unsigned>(hSteamLeaderboard - 1)];
    auto my_entry = board.find_recent_entry(settings->get_local_steam_id());
    int current_rank = my_entry
        ? 1 + (int)(my_entry - &board.entries[0])
        : 0;
    int new_rank = current_rank;

    bool score_updated = false;
    if (my_entry) {
        switch (eLeaderboardUploadScoreMethod)
        {
        case k_ELeaderboardUploadScoreMethodKeepBest: { // keep user's best score
            if (board.sort_method == k_ELeaderboardSortMethodAscending) { // keep user's lowest score
                score_updated = nScore < my_entry->score;
            } else { // keep user's highest score
                score_updated = nScore > my_entry->score;
            }
        }
        break;

        case k_ELeaderboardUploadScoreMethodForceUpdate: { // always replace score with specified
            score_updated = my_entry->score != nScore;
        }
        break;
        
        default: break;
        }
    } else { // no entry yet for us
        score_updated = true;
    }

    if (score_updated || (eLeaderboardUploadScoreMethod == k_ELeaderboardUploadScoreMethodForceUpdate)) {
        Steam_Leaderboard_Entry new_entry{};
        new_entry.steam_id = settings->get_local_steam_id();
        new_entry.score = nScore;
        if (pScoreDetails && cScoreDetailsCount  > 0) {
            for (int i = 0; i < cScoreDetailsCount; ++i) {
                new_entry.score_details.push_back(pScoreDetails[i]);
            }
        }
        
        update_leaderboard_entry(board, new_entry);
        new_rank = 1 + (int)(board.find_recent_entry(settings->get_local_steam_id()) - &board.entries[0]);

        // check again in case this was a forced update
        // avoid disk write if score is the same
        if (score_updated) save_my_leaderboard_entry(board);
        send_my_leaderboard_score(board);
            
    }

    LeaderboardScoreUploaded_t data{};
    data.m_bSuccess = 1; //needs to be success or DOA6 freezes when uploading score.
    data.m_hSteamLeaderboard = hSteamLeaderboard;
    data.m_nScore = nScore;
    data.m_bScoreChanged = score_updated;
    data.m_nGlobalRankNew = new_rank;
    data.m_nGlobalRankPrevious = current_rank;
    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data), 0.1); // TODO is this timing ok?
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data), 0.1);
    return ret;
}

SteamAPICall_t Steam_User_Stats::UploadLeaderboardScore( SteamLeaderboard_t hSteamLeaderboard, int32 nScore, int32 *pScoreDetails, int cScoreDetailsCount )
{
	PRINT_DEBUG("old");
	return UploadLeaderboardScore(hSteamLeaderboard, k_ELeaderboardUploadScoreMethodKeepBest, nScore, pScoreDetails, cScoreDetailsCount);
}


// Attaches a piece of user generated content the user's entry on a leaderboard.
// hContent is a handle to a piece of user generated content that was shared using ISteamUserRemoteStorage::FileShare().
// This call is asynchronous, with the result returned in LeaderboardUGCSet_t.
STEAM_CALL_RESULT( LeaderboardUGCSet_t )
SteamAPICall_t Steam_User_Stats::AttachLeaderboardUGC( SteamLeaderboard_t hSteamLeaderboard, UGCHandle_t hUGC )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    LeaderboardUGCSet_t data{};
    if (hSteamLeaderboard > cached_leaderboards.size() || hSteamLeaderboard <= 0) {
        data.m_eResult = k_EResultFail;
    } else {
        data.m_eResult = k_EResultOK;
    }

    data.m_hSteamLeaderboard = hSteamLeaderboard;
    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return ret;
}



// --- networking callbacks
// only triggered when we have a message

// someone updated their score
void Steam_User_Stats::network_leaderboard_update_score(Common_Message *msg, Steam_Leaderboard &board, bool send_score_back)
{
    CSteamID sender_steamid((uint64)msg->source_id());
    PRINT_DEBUG("got score for user %llu on leaderboard '%s' (send our score back=%i)",
        (uint64)msg->source_id(), board.name.c_str(), (int)send_score_back
    );
    
    // when players initally load a board, and they don't have an entry in it,
    // they send this msg but without their user score entry
    if (msg->leaderboards_messages().has_user_score_entry()) {
        const auto &user_score_msg = msg->leaderboards_messages().user_score_entry();

        Steam_Leaderboard_Entry updated_entry{};
        updated_entry.steam_id = sender_steamid;
        updated_entry.score = user_score_msg.score();
        updated_entry.score_details.reserve(user_score_msg.score_details().size());
        updated_entry.score_details.assign(user_score_msg.score_details().begin(), user_score_msg.score_details().end());
        update_leaderboard_entry(board, updated_entry);
    }

    // if the sender wants back our score, send it to all, not just them
    // in case we have 3 or more players and none of them have our data
    if (send_score_back) send_my_leaderboard_score(board);
}

// someone is requesting our score on a leaderboard
void Steam_User_Stats::network_leaderboard_send_my_score(Common_Message *msg, const Steam_Leaderboard &board)
{
    CSteamID sender_steamid((uint64)msg->source_id());
    PRINT_DEBUG("user %llu requested our score for leaderboard '%s'", (uint64)msg->source_id(), board.name.c_str());

    send_my_leaderboard_score(board, &sender_steamid);
}

void Steam_User_Stats::network_callback_leaderboards(Common_Message *msg)
{
    // network->sendToAll() sends to current user also
    if (msg->source_id() == settings->get_local_steam_id().ConvertToUint64()) return; 
    if (settings->get_local_game_id().AppID() != msg->leaderboards_messages().appid()) return;

    if (!msg->leaderboards_messages().has_leaderboard_info()) {
        PRINT_DEBUG("error empty leaderboard msg");
        return;
    }

    const auto &board_info_msg = msg->leaderboards_messages().leaderboard_info();
    
    PRINT_DEBUG("attempting to cache leaderboard '%s'", board_info_msg.board_name().c_str());
    unsigned int board_handle = cache_leaderboard_ifneeded(
        board_info_msg.board_name(),
        (ELeaderboardSortMethod)board_info_msg.sort_method(),
        (ELeaderboardDisplayType)board_info_msg.display_type()
    );

    switch (msg->leaderboards_messages().type()) {
        // someone updated their score
        case Leaderboards_Messages::UpdateUserScore:
            network_leaderboard_update_score(msg, cached_leaderboards[board_handle - 1], false);
        break;

        // someone updated their score and wants us to share back ours
        case Leaderboards_Messages::UpdateUserScoreMutual:
            network_leaderboard_update_score(msg, cached_leaderboards[board_handle - 1], true);
        break;

        // someone is requesting our score on a leaderboard
        case Leaderboards_Messages::RequestUserScore:
            network_leaderboard_send_my_score(msg, cached_leaderboards[board_handle - 1]);
        break;
        
        default:
            PRINT_DEBUG("unhandled type %i", (int)msg->leaderboards_messages().type());
        break;
    }

}
