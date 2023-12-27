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

#include "dll/steam_gameserverstats.h"

Steam_GameServerStats::Steam_GameServerStats(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks)
{
    this->settings = settings;
    this->network = network;
    this->callback_results = callback_results;
    this->callbacks = callbacks;
}

// downloads stats for the user
// returns a GSStatsReceived_t callback when completed
// if the user has no stats, GSStatsReceived_t.m_eResult will be set to k_EResultFail
// these stats will only be auto-updated for clients playing on the server. For other
// users you'll need to call RequestUserStats() again to refresh any data
STEAM_CALL_RESULT( GSStatsReceived_t )
SteamAPICall_t Steam_GameServerStats::RequestUserStats( CSteamID steamIDUser )
{
    PRINT_DEBUG("RequestUserStats\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    GSStatsReceived_t data;
    data.m_eResult = k_EResultFail;//k_EResultOK;
    data.m_steamIDUser = steamIDUser;
    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}


// requests stat information for a user, usable after a successful call to RequestUserStats()
bool Steam_GameServerStats::GetUserStat( CSteamID steamIDUser, const char *pchName, int32 *pData )
{
    PRINT_DEBUG("GetUserStat\n");
    return false;
}

bool Steam_GameServerStats::GetUserStat( CSteamID steamIDUser, const char *pchName, float *pData )
{
    PRINT_DEBUG("GetUserStat\n");
    return false;
}

bool Steam_GameServerStats::GetUserAchievement( CSteamID steamIDUser, const char *pchName, bool *pbAchieved )
{
    PRINT_DEBUG("GetUserAchievement\n");
    return false;
}


// Set / update stats and achievements. 
// Note: These updates will work only on stats game servers are allowed to edit and only for 
// game servers that have been declared as officially controlled by the game creators. 
// Set the IP range of your official servers on the Steamworks page
bool Steam_GameServerStats::SetUserStat( CSteamID steamIDUser, const char *pchName, int32 nData )
{
    PRINT_DEBUG("SetUserStat\n");
    return false;
}

bool Steam_GameServerStats::SetUserStat( CSteamID steamIDUser, const char *pchName, float fData )
{
    PRINT_DEBUG("SetUserStat\n");
    return false;
}

bool Steam_GameServerStats::UpdateUserAvgRateStat( CSteamID steamIDUser, const char *pchName, float flCountThisSession, double dSessionLength )
{
    PRINT_DEBUG("UpdateUserAvgRateStat\n");
    return false;
}


bool Steam_GameServerStats::SetUserAchievement( CSteamID steamIDUser, const char *pchName )
{
    PRINT_DEBUG("SetUserAchievement\n");
    return false;
}

bool Steam_GameServerStats::ClearUserAchievement( CSteamID steamIDUser, const char *pchName )
{
    PRINT_DEBUG("ClearUserAchievement\n");
    return false;
}


// Store the current data on the server, will get a GSStatsStored_t callback when set.
//
// If the callback has a result of k_EResultInvalidParam, one or more stats 
// uploaded has been rejected, either because they broke constraints
// or were out of date. In this case the server sends back updated values.
// The stats should be re-iterated to keep in sync.
STEAM_CALL_RESULT( GSStatsStored_t )
SteamAPICall_t Steam_GameServerStats::StoreUserStats( CSteamID steamIDUser )
{
    PRINT_DEBUG("StoreUserStats\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    GSStatsStored_t data;
    data.m_eResult = k_EResultOK;
    data.m_steamIDUser = steamIDUser;
    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}
