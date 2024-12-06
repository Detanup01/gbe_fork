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

#ifndef __INCLUDED_STEAM_USER_STATS_H__
#define __INCLUDED_STEAM_USER_STATS_H__

#include <limits>
#include "base.h"
#include "overlay/steam_overlay.h"

struct Steam_Leaderboard_Entry {
    CSteamID steam_id{};
    int32 score{};
    std::vector<int32> score_details{};
};

struct Steam_Leaderboard {
    std::string name{};
    ELeaderboardSortMethod sort_method = k_ELeaderboardSortMethodNone;
    ELeaderboardDisplayType display_type = k_ELeaderboardDisplayTypeNone;
    std::vector<Steam_Leaderboard_Entry> entries{};

    Steam_Leaderboard_Entry* find_recent_entry(const CSteamID &steamid) const;
    void remove_entries(const CSteamID &steamid);

    // remove entries with the same steamid, keeping only most recent one
    void remove_duplicate_entries();

    void sort_entries();

};

struct achievement_trigger {
    std::string name{}; // defined achievement name
    std::string value_operation{};
    std::string min_value{}; // min progress
    std::string max_value{}; // max progress

    bool should_unlock_ach(float stat) const;
    bool should_unlock_ach(int32 stat) const;

    bool should_indicate_progress(float stat) const;
    bool should_indicate_progress(int32 stat) const;
};

class Steam_User_Stats :
public ISteamUserStats001,
public ISteamUserStats002,
public ISteamUserStats003,
public ISteamUserStats004,
public ISteamUserStats005,
public ISteamUserStats006,
public ISteamUserStats007,
public ISteamUserStats008,
public ISteamUserStats009,
public ISteamUserStats010,
public ISteamUserStats011,
public ISteamUserStats012,
public ISteamUserStats
{
public:
    static constexpr const auto achievements_user_file = "achievements.json";

private:
    template<typename T>
    struct InternalSetResult {
        bool success = false;
        bool notify_server = false;
        std::string internal_name{};
        T current_val{};
    };

    class Local_Storage *local_storage{};
    class Settings *settings{};
    class SteamCallResults *callback_results{};
    class SteamCallBacks *callbacks{};
    class Networking *network{};
    class RunEveryRunCB *run_every_runcb{};
    class Steam_Overlay* overlay{};

    std::vector<struct Steam_Leaderboard> cached_leaderboards{};

    nlohmann::json defined_achievements{};
    nlohmann::json user_achievements{};
    std::vector<std::string> sorted_achievement_names{};
    size_t last_loaded_ach_icon{};

    std::map<std::string, int32> stats_cache_int{};
    std::map<std::string, float> stats_cache_float{};

    std::map<std::string, std::vector<achievement_trigger>> achievement_stat_trigger{};
    
    // triggered when an achievement is unlocked
    // https://partner.steamgames.com/doc/api/ISteamUserStats#StoreStats
    std::map<std::string, UserAchievementStored_t> store_stats_trigger{};

    GameServerStats_Messages::AllStats pending_server_updates{};

    void load_achievements_db();
    void load_achievements();
    void save_achievements();

    int load_ach_icon(nlohmann::json &defined_ach, bool achieved);

    nlohmann::detail::iter_impl<nlohmann::json> defined_achievements_find(const std::string &key);
    std::string get_value_for_language(const nlohmann::json &json, std::string_view key, std::string_view language);

    std::vector<Steam_Leaderboard_Entry> load_leaderboard_entries(const std::string &name);
    void save_my_leaderboard_entry(const Steam_Leaderboard &leaderboard);
    Steam_Leaderboard_Entry* update_leaderboard_entry(Steam_Leaderboard &leaderboard, const Steam_Leaderboard_Entry &entry, bool overwrite = true);

    // returns a value 1 -> leaderboards.size(), inclusive
    unsigned int find_cached_leaderboard(const std::string &name);
    unsigned int cache_leaderboard_ifneeded(const std::string &name, ELeaderboardSortMethod eLeaderboardSortMethod, ELeaderboardDisplayType eLeaderboardDisplayType);
    // null steamid means broadcast to all
    void send_my_leaderboard_score(const Steam_Leaderboard &board, const CSteamID *steamid = nullptr, bool want_scores_back = false);
    void request_user_leaderboard_entry(const Steam_Leaderboard &board, const CSteamID &steamid);

    // change stats/achievements without sending back to server
    bool clear_stats_internal();
    InternalSetResult<int32> set_stat_internal( const char *pchName, int32 nData );
    InternalSetResult<std::pair<GameServerStats_Messages::StatInfo::Stat_Type, float>> set_stat_internal( const char *pchName, float fData );
    InternalSetResult<std::pair<GameServerStats_Messages::StatInfo::Stat_Type, float>> update_avg_rate_stat_internal( const char *pchName, float flCountThisSession, double dSessionLength );
    InternalSetResult<bool> set_achievement_internal( const char *pchName );
    InternalSetResult<bool> clear_achievement_internal( const char *pchName );

    void send_updated_stats();
    void load_achievements_icons();
    void steam_run_callback();

    // requests from server
    void network_stats_initial(Common_Message *msg);
    void network_stats_updated(Common_Message *msg);
    void network_callback_stats(Common_Message *msg);

    // requests from other users to share leaderboards
    void network_leaderboard_update_score(Common_Message *msg, Steam_Leaderboard &board, bool send_score_back);
    void network_leaderboard_send_my_score(Common_Message *msg, const Steam_Leaderboard &board);
    void network_callback_leaderboards(Common_Message *msg);
    
    // user connect/disconnect
    void network_callback_low_level(Common_Message *msg);

    static void steam_user_stats_network_low_level(void *object, Common_Message *msg);
    static void steam_user_stats_network_stats(void *object, Common_Message *msg);
    static void steam_user_stats_network_leaderboards(void *object, Common_Message *msg);
    static void steam_user_stats_run_every_runcb(void *object);

public:
    Steam_User_Stats(Settings *settings, class Networking *network, Local_Storage *local_storage, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb, Steam_Overlay* overlay);
    ~Steam_User_Stats();

    // Ask the server to send down this user's data and achievements for this game
    STEAM_CALL_BACK( UserStatsReceived_t )
    bool RequestCurrentStats();


    // Data accessors
    bool GetStat( const char *pchName, int32 *pData );

    bool GetStat( const char *pchName, float *pData );


    // Set / update data
    bool SetStat( const char *pchName, int32 nData );

    bool SetStat( const char *pchName, float fData );

    bool UpdateAvgRateStat( const char *pchName, float flCountThisSession, double dSessionLength );


    // Achievement flag accessors
    bool GetAchievement( const char *pchName, bool *pbAchieved );

    bool SetAchievement( const char *pchName );

    bool ClearAchievement( const char *pchName );


    // Get the achievement status, and the time it was unlocked if unlocked.
    // If the return value is true, but the unlock time is zero, that means it was unlocked before Steam 
    // began tracking achievement unlock times (December 2009). Time is seconds since January 1, 1970.
    bool GetAchievementAndUnlockTime( const char *pchName, bool *pbAchieved, uint32 *punUnlockTime );


    // Store the current data on the server, will get a callback when set
    // And one callback for every new achievement
    //
    // If the callback has a result of k_EResultInvalidParam, one or more stats 
    // uploaded has been rejected, either because they broke constraints
    // or were out of date. In this case the server sends back updated values.
    // The stats should be re-iterated to keep in sync.
    bool StoreStats();


    // Achievement / GroupAchievement metadata

    // Gets the icon of the achievement, which is a handle to be used in ISteamUtils::GetImageRGBA(), or 0 if none set. 
    // A return value of 0 may indicate we are still fetching data, and you can wait for the UserAchievementIconFetched_t callback
    // which will notify you when the bits are ready. If the callback still returns zero, then there is no image set for the
    // specified achievement.
    int GetAchievementIcon( const char *pchName );

    int get_achievement_icon_handle( const std::string &ach_name, bool pbAchieved, bool force_load = false );

    std::string get_achievement_icon_name( const char *pchName, bool achieved );


    // Get general attributes for an achievement. Accepts the following keys:
    // - "name" and "desc" for retrieving the localized achievement name and description (returned in UTF8)
    // - "hidden" for retrieving if an achievement is hidden (returns "0" when not hidden, "1" when hidden)
    const char * GetAchievementDisplayAttribute( const char *pchName, const char *pchKey );


    // Achievement progress - triggers an AchievementProgress callback, that is all.
    // Calling this w/ N out of N progress will NOT set the achievement, the game must still do that.
    bool IndicateAchievementProgress( const char *pchName, uint32 nCurProgress, uint32 nMaxProgress );


    // Used for iterating achievements. In general games should not need these functions because they should have a
    // list of existing achievements compiled into them
    uint32 GetNumAchievements();

    // Get achievement name iAchievement in [0,GetNumAchievements)
    const char * GetAchievementName( uint32 iAchievement );


    // Friends stats & achievements

    // downloads stats for the user
    // returns a UserStatsReceived_t received when completed
    // if the other user has no stats, UserStatsReceived_t.m_eResult will be set to k_EResultFail
    // these stats won't be auto-updated; you'll need to call RequestUserStats() again to refresh any data
    STEAM_CALL_RESULT( UserStatsReceived_t )
    SteamAPICall_t RequestUserStats( CSteamID steamIDUser );


    // requests stat information for a user, usable after a successful call to RequestUserStats()
    bool GetUserStat( CSteamID steamIDUser, const char *pchName, int32 *pData );

    bool GetUserStat( CSteamID steamIDUser, const char *pchName, float *pData );

    bool GetUserAchievement( CSteamID steamIDUser, const char *pchName, bool *pbAchieved );

    // See notes for GetAchievementAndUnlockTime above
    bool GetUserAchievementAndUnlockTime( CSteamID steamIDUser, const char *pchName, bool *pbAchieved, uint32 *punUnlockTime );


    // Reset stats 
    bool ResetAllStats( bool bAchievementsToo );


    // Leaderboard functions

    // asks the Steam back-end for a leaderboard by name, and will create it if it's not yet
    // This call is asynchronous, with the result returned in LeaderboardFindResult_t
    STEAM_CALL_RESULT(LeaderboardFindResult_t)
    SteamAPICall_t FindOrCreateLeaderboard( const char *pchLeaderboardName, ELeaderboardSortMethod eLeaderboardSortMethod, ELeaderboardDisplayType eLeaderboardDisplayType );


    // as above, but won't create the leaderboard if it's not found
    // This call is asynchronous, with the result returned in LeaderboardFindResult_t
    STEAM_CALL_RESULT( LeaderboardFindResult_t )
    SteamAPICall_t FindLeaderboard( const char *pchLeaderboardName );


    // returns the name of a leaderboard
    const char * GetLeaderboardName( SteamLeaderboard_t hSteamLeaderboard );


    // returns the total number of entries in a leaderboard, as of the last request
    int GetLeaderboardEntryCount( SteamLeaderboard_t hSteamLeaderboard );


    // returns the sort method of the leaderboard
    ELeaderboardSortMethod GetLeaderboardSortMethod( SteamLeaderboard_t hSteamLeaderboard );


    // returns the display type of the leaderboard
    ELeaderboardDisplayType GetLeaderboardDisplayType( SteamLeaderboard_t hSteamLeaderboard );


    // Asks the Steam back-end for a set of rows in the leaderboard.
    // This call is asynchronous, with the result returned in LeaderboardScoresDownloaded_t
    // LeaderboardScoresDownloaded_t will contain a handle to pull the results from GetDownloadedLeaderboardEntries() (below)
    // You can ask for more entries than exist, and it will return as many as do exist.
    // k_ELeaderboardDataRequestGlobal requests rows in the leaderboard from the full table, with nRangeStart & nRangeEnd in the range [1, TotalEntries]
    // k_ELeaderboardDataRequestGlobalAroundUser requests rows around the current user, nRangeStart being negate
    //   e.g. DownloadLeaderboardEntries( hLeaderboard, k_ELeaderboardDataRequestGlobalAroundUser, -3, 3 ) will return 7 rows, 3 before the user, 3 after
    // k_ELeaderboardDataRequestFriends requests all the rows for friends of the current user 
    STEAM_CALL_RESULT( LeaderboardScoresDownloaded_t )
    SteamAPICall_t DownloadLeaderboardEntries( SteamLeaderboard_t hSteamLeaderboard, ELeaderboardDataRequest eLeaderboardDataRequest, int nRangeStart, int nRangeEnd );

    // as above, but downloads leaderboard entries for an arbitrary set of users - ELeaderboardDataRequest is k_ELeaderboardDataRequestUsers
    // if a user doesn't have a leaderboard entry, they won't be included in the result
    // a max of 100 users can be downloaded at a time, with only one outstanding call at a time
    STEAM_METHOD_DESC(Downloads leaderboard entries for an arbitrary set of users - ELeaderboardDataRequest is k_ELeaderboardDataRequestUsers)
    STEAM_CALL_RESULT( LeaderboardScoresDownloaded_t )
    SteamAPICall_t DownloadLeaderboardEntriesForUsers( SteamLeaderboard_t hSteamLeaderboard,
                                                                STEAM_ARRAY_COUNT_D(cUsers, Array of users to retrieve) CSteamID *prgUsers, int cUsers );


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
    bool GetDownloadedLeaderboardEntry( SteamLeaderboardEntries_t hSteamLeaderboardEntries, int index, LeaderboardEntry_t *pLeaderboardEntry, int32 *pDetails, int cDetailsMax );


    // Uploads a user score to the Steam back-end.
    // This call is asynchronous, with the result returned in LeaderboardScoreUploaded_t
    // Details are extra game-defined information regarding how the user got that score
    // pScoreDetails points to an array of int32's, cScoreDetailsCount is the number of int32's in the list
    STEAM_CALL_RESULT( LeaderboardScoreUploaded_t )
    SteamAPICall_t UploadLeaderboardScore( SteamLeaderboard_t hSteamLeaderboard, ELeaderboardUploadScoreMethod eLeaderboardUploadScoreMethod, int32 nScore, const int32 *pScoreDetails, int cScoreDetailsCount );

    SteamAPICall_t UploadLeaderboardScore( SteamLeaderboard_t hSteamLeaderboard, int32 nScore, int32 *pScoreDetails, int cScoreDetailsCount );


    // Attaches a piece of user generated content the user's entry on a leaderboard.
    // hContent is a handle to a piece of user generated content that was shared using ISteamUserRemoteStorage::FileShare().
    // This call is asynchronous, with the result returned in LeaderboardUGCSet_t.
    STEAM_CALL_RESULT( LeaderboardUGCSet_t )
    SteamAPICall_t AttachLeaderboardUGC( SteamLeaderboard_t hSteamLeaderboard, UGCHandle_t hUGC );


    // Retrieves the number of players currently playing your game (online + offline)
    // This call is asynchronous, with the result returned in NumberOfCurrentPlayers_t
    STEAM_CALL_RESULT( NumberOfCurrentPlayers_t )
    SteamAPICall_t GetNumberOfCurrentPlayers();


    // Requests that Steam fetch data on the percentage of players who have received each achievement
    // for the game globally.
    // This call is asynchronous, with the result returned in GlobalAchievementPercentagesReady_t.
    STEAM_CALL_RESULT( GlobalAchievementPercentagesReady_t )
    SteamAPICall_t RequestGlobalAchievementPercentages();


    // Get the info on the most achieved achievement for the game, returns an iterator index you can use to fetch
    // the next most achieved afterwards.  Will return -1 if there is no data on achievement 
    // percentages (ie, you haven't called RequestGlobalAchievementPercentages and waited on the callback).
    int GetMostAchievedAchievementInfo( char *pchName, uint32 unNameBufLen, float *pflPercent, bool *pbAchieved );


    // Get the info on the next most achieved achievement for the game. Call this after GetMostAchievedAchievementInfo or another
    // GetNextMostAchievedAchievementInfo call passing the iterator from the previous call. Returns -1 after the last
    // achievement has been iterated.
    int GetNextMostAchievedAchievementInfo( int iIteratorPrevious, char *pchName, uint32 unNameBufLen, float *pflPercent, bool *pbAchieved );


    // Returns the percentage of users who have achieved the specified achievement.
    bool GetAchievementAchievedPercent( const char *pchName, float *pflPercent );


    // Requests global stats data, which is available for stats marked as "aggregated".
    // This call is asynchronous, with the results returned in GlobalStatsReceived_t.
    // nHistoryDays specifies how many days of day-by-day history to retrieve in addition
    // to the overall totals. The limit is 60.
    STEAM_CALL_RESULT( GlobalStatsReceived_t )
    SteamAPICall_t RequestGlobalStats( int nHistoryDays );


    // Gets the lifetime totals for an aggregated stat
    bool GetGlobalStat( const char *pchStatName, int64 *pData );

    bool GetGlobalStat( const char *pchStatName, double *pData );


    // Gets history for an aggregated stat. pData will be filled with daily values, starting with today.
    // So when called, pData[0] will be today, pData[1] will be yesterday, and pData[2] will be two days ago, 
    // etc. cubData is the size in bytes of the pubData buffer. Returns the number of 
    // elements actually set.
    int32 GetGlobalStatHistory( const char *pchStatName, STEAM_ARRAY_COUNT(cubData) int64 *pData, uint32 cubData );

    int32 GetGlobalStatHistory( const char *pchStatName, STEAM_ARRAY_COUNT(cubData) double *pData, uint32 cubData );

    // For achievements that have related Progress stats, use this to query what the bounds of that progress are.
    // You may want this info to selectively call IndicateAchievementProgress when appropriate milestones of progress
    // have been made, to show a progress notification to the user.
    bool GetAchievementProgressLimits( const char *pchName, int32 *pnMinProgress, int32 *pnMaxProgress );

    bool GetAchievementProgressLimits( const char *pchName, float *pfMinProgress, float *pfMaxProgress );

    // old interface version
    uint32 GetNumStats( CGameID nGameID );
    const char *GetStatName( CGameID nGameID, uint32 iStat );
    ESteamUserStatType GetStatType( CGameID nGameID, const char *pchName );
    uint32 GetNumAchievements( CGameID nGameID );
    const char *GetAchievementName( CGameID nGameID, uint32 iAchievement );
    uint32 GetNumGroupAchievements( CGameID nGameID );
    const char *GetGroupAchievementName( CGameID nGameID, uint32 iAchievement );
    bool RequestCurrentStats( CGameID nGameID );
    bool GetStat( CGameID nGameID, const char *pchName, int32 *pData );
    bool GetStat( CGameID nGameID, const char *pchName, float *pData );
    bool SetStat( CGameID nGameID, const char *pchName, int32 nData );
    bool SetStat( CGameID nGameID, const char *pchName, float fData );
    bool UpdateAvgRateStat( CGameID nGameID, const char *pchName, float flCountThisSession, double dSessionLength );
    bool GetAchievement( CGameID nGameID, const char *pchName, bool *pbAchieved );
    bool GetGroupAchievement( CGameID nGameID, const char *pchName, bool *pbAchieved );
    bool SetAchievement( CGameID nGameID, const char *pchName );
    bool SetGroupAchievement( CGameID nGameID, const char *pchName );
    bool StoreStats( CGameID nGameID );
    bool ClearAchievement( CGameID nGameID, const char *pchName );
    bool ClearGroupAchievement( CGameID nGameID, const char *pchName );
    int GetAchievementIcon( CGameID nGameID, const char *pchName );
    const char *GetAchievementDisplayAttribute( CGameID nGameID, const char *pchName, const char *pchKey );
    bool IndicateAchievementProgress( CGameID nGameID, const char *pchName, uint32 nCurProgress, uint32 nMaxProgress );

};

#endif//__INCLUDED_STEAM_USER_STATS_H__
