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

#ifndef __INCLUDED_STEAM_PARTIES_H__
#define __INCLUDED_STEAM_PARTIES_H__

#include "base.h"

class Steam_Parties :
public ISteamParties
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

public:
    Steam_Parties(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb);
    ~Steam_Parties();


    // =============================================================================================
    // Party Client APIs

    // Enumerate any active beacons for parties you may wish to join
    uint32 GetNumActiveBeacons();

    PartyBeaconID_t GetBeaconByIndex( uint32 unIndex );

    bool GetBeaconDetails( PartyBeaconID_t ulBeaconID, CSteamID *pSteamIDBeaconOwner, STEAM_OUT_STRUCT() SteamPartyBeaconLocation_t *pLocation, STEAM_OUT_STRING_COUNT(cchMetadata) char *pchMetadata, int cchMetadata );


    // Join an open party. Steam will reserve one beacon slot for your SteamID,
    // and return the necessary JoinGame string for you to use to connect
    STEAM_CALL_RESULT( JoinPartyCallback_t )
    SteamAPICall_t JoinParty( PartyBeaconID_t ulBeaconID );


    // =============================================================================================
    // Party Host APIs

    // Get a list of possible beacon locations
    bool GetNumAvailableBeaconLocations( uint32 *puNumLocations );

    bool GetAvailableBeaconLocations( SteamPartyBeaconLocation_t *pLocationList, uint32 uMaxNumLocations );


    // Create a new party beacon and activate it in the selected location.
    // unOpenSlots is the maximum number of users that Steam will send to you.
    // When people begin responding to your beacon, Steam will send you
    // PartyReservationCallback_t callbacks to let you know who is on the way.
    STEAM_CALL_RESULT( CreateBeaconCallback_t )
    SteamAPICall_t CreateBeacon( uint32 unOpenSlots, SteamPartyBeaconLocation_t *pBeaconLocation, const char *pchConnectString, const char *pchMetadata );


    // Call this function when a user that had a reservation (see callback below) 
    // has successfully joined your party.
    // Steam will manage the remaining open slots automatically.
    void OnReservationCompleted( PartyBeaconID_t ulBeacon, CSteamID steamIDUser );


    // To cancel a reservation (due to timeout or user input), call this.
    // Steam will open a new reservation slot.
    // Note: The user may already be in-flight to your game, so it's possible they will still connect and try to join your party.
    void CancelReservation( PartyBeaconID_t ulBeacon, CSteamID steamIDUser );


    // Change the number of open beacon reservation slots.
    // Call this if, for example, someone without a reservation joins your party (eg a friend, or via your own matchmaking system).
    STEAM_CALL_RESULT( ChangeNumOpenSlotsCallback_t )
    SteamAPICall_t ChangeNumOpenSlots( PartyBeaconID_t ulBeacon, uint32 unOpenSlots );


    // Turn off the beacon. 
    bool DestroyBeacon( PartyBeaconID_t ulBeacon );


    // Utils
    bool GetBeaconLocationData( SteamPartyBeaconLocation_t BeaconLocation, ESteamPartyBeaconLocationData eData, STEAM_OUT_STRING_COUNT(cchDataStringOut) char *pchDataStringOut, int cchDataStringOut );


    void RunCallbacks();

    void Callback(Common_Message *msg);

};

#endif // __INCLUDED_STEAM_PARTIES_H__
