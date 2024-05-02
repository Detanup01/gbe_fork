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

#ifndef __INCLUDED_STEAM_GAMESEARCH_H__
#define __INCLUDED_STEAM_GAMESEARCH_H__

#include "base.h"

class Steam_Game_Search :
public ISteamGameSearch
{
    class Settings *settings{};
    class Networking *network{};
    class SteamCallResults *callback_results{};
    class SteamCallBacks *callbacks{};
    class RunEveryRunCB *run_every_runcb{};
    std::chrono::time_point<std::chrono::steady_clock> initialized_time = std::chrono::steady_clock::now();
    FSteamNetworkingSocketsDebugOutput debug_function{};
    
    static void steam_callback(void *object, Common_Message *msg);
    static void steam_run_every_runcb(void *object);

    void RunCallbacks();
    void Callback(Common_Message *msg);

public:
    Steam_Game_Search(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb);
    ~Steam_Game_Search();

    // =============================================================================================
    // Game Player APIs

    // a keyname and a list of comma separated values: one of which is must be found in order for the match to qualify
    // fails if a search is currently in progress
    EGameSearchErrorCode_t AddGameSearchParams( const char *pchKeyToFind, const char *pchValuesToFind );


    // all players in lobby enter the queue and await a SearchForGameNotificationCallback_t callback. fails if another search is currently in progress
    // if not the owner of the lobby or search already in progress this call fails
    // periodic callbacks will be sent as queue time estimates change
    EGameSearchErrorCode_t SearchForGameWithLobby( CSteamID steamIDLobby, int nPlayerMin, int nPlayerMax );


    // user enter the queue and await a SearchForGameNotificationCallback_t callback. fails if another search is currently in progress
    // periodic callbacks will be sent as queue time estimates change
    EGameSearchErrorCode_t SearchForGameSolo( int nPlayerMin, int nPlayerMax );


    // after receiving SearchForGameResultCallback_t, accept or decline the game
    // multiple SearchForGameResultCallback_t will follow as players accept game until the host starts or cancels the game
    EGameSearchErrorCode_t AcceptGame();

    EGameSearchErrorCode_t DeclineGame();


    // after receiving GameStartedByHostCallback_t get connection details to server
    EGameSearchErrorCode_t RetrieveConnectionDetails( CSteamID steamIDHost, char *pchConnectionDetails, int cubConnectionDetails );


    // leaves queue if still waiting
    EGameSearchErrorCode_t EndGameSearch();


    // =============================================================================================
    // Game Host APIs

    // a keyname and a list of comma separated values: all the values you allow
    EGameSearchErrorCode_t SetGameHostParams( const char *pchKey, const char *pchValue );


    // set connection details for players once game is found so they can connect to this server
    EGameSearchErrorCode_t SetConnectionDetails( const char *pchConnectionDetails, int cubConnectionDetails );


    // mark server as available for more players with nPlayerMin,nPlayerMax desired
    // accept no lobbies with playercount greater than nMaxTeamSize
    // the set of lobbies returned must be partitionable into teams of no more than nMaxTeamSize
    // RequestPlayersForGameNotificationCallback_t callback will be sent when the search has started
    // multple RequestPlayersForGameResultCallback_t callbacks will follow when players are found
    EGameSearchErrorCode_t RequestPlayersForGame( int nPlayerMin, int nPlayerMax, int nMaxTeamSize );


    // accept the player list and release connection details to players
    // players will only be given connection details and host steamid when this is called
    // ( allows host to accept after all players confirm, some confirm, or none confirm. decision is entirely up to the host )
    EGameSearchErrorCode_t HostConfirmGameStart( uint64 ullUniqueGameID );


    // cancel request and leave the pool of game hosts looking for players
    // if a set of players has already been sent to host, all players will receive SearchForGameHostFailedToConfirm_t
    EGameSearchErrorCode_t CancelRequestPlayersForGame();


    // submit a result for one player. does not end the game. ullUniqueGameID continues to describe this game
    EGameSearchErrorCode_t SubmitPlayerResult( uint64 ullUniqueGameID, CSteamID steamIDPlayer, EPlayerResult_t EPlayerResult );


    // ends the game. no further SubmitPlayerResults for ullUniqueGameID will be accepted
    // any future requests will provide a new ullUniqueGameID
    EGameSearchErrorCode_t EndGame( uint64 ullUniqueGameID );

};

#endif // __INCLUDED_STEAM_GAMESEARCH_H__
