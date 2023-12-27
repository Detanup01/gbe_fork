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

class Steam_Parties :
public ISteamParties
{
    class Settings *settings;
    class Networking *network;
    class SteamCallResults *callback_results;
    class SteamCallBacks *callbacks;
    class RunEveryRunCB *run_every_runcb;
    std::chrono::time_point<std::chrono::steady_clock> initialized_time = std::chrono::steady_clock::now();
    FSteamNetworkingSocketsDebugOutput debug_function;

public:
static void steam_callback(void *object, Common_Message *msg)
{
    PRINT_DEBUG("steam_parties_callback\n");

    Steam_Parties *steam_parties = (Steam_Parties *)object;
    steam_parties->Callback(msg);
}

static void steam_run_every_runcb(void *object)
{
    PRINT_DEBUG("steam_parties_run_every_runcb\n");

    Steam_Parties *steam_parties = (Steam_Parties *)object;
    steam_parties->RunCallbacks();
}

Steam_Parties(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb)
{
    this->settings = settings;
    this->network = network;
    this->run_every_runcb = run_every_runcb;
    //this->network->setCallback(CALLBACK_ID_USER_STATUS, settings->get_local_steam_id(), &Steam_Parties::steam_callback, this);
    this->run_every_runcb->add(&Steam_Parties::steam_run_every_runcb, this);

    this->callback_results = callback_results;
    this->callbacks = callbacks;
}

~Steam_Parties()
{
    //TODO rm network callbacks
    this->run_every_runcb->remove(&Steam_Parties::steam_run_every_runcb, this);
}


// =============================================================================================
// Party Client APIs

// Enumerate any active beacons for parties you may wish to join
uint32 GetNumActiveBeacons()
{
    PRINT_DEBUG("Steam_Parties::GetNumActiveBeacons\n");
    return 0;
}

PartyBeaconID_t GetBeaconByIndex( uint32 unIndex )
{
    PRINT_DEBUG("Steam_Parties::GetBeaconByIndex\n");
    return k_ulPartyBeaconIdInvalid;
}

bool GetBeaconDetails( PartyBeaconID_t ulBeaconID, CSteamID *pSteamIDBeaconOwner, STEAM_OUT_STRUCT() SteamPartyBeaconLocation_t *pLocation, STEAM_OUT_STRING_COUNT(cchMetadata) char *pchMetadata, int cchMetadata )
{
    PRINT_DEBUG("Steam_Parties::GetBeaconDetails\n");
    return false;
}


// Join an open party. Steam will reserve one beacon slot for your SteamID,
// and return the necessary JoinGame string for you to use to connect
STEAM_CALL_RESULT( JoinPartyCallback_t )
SteamAPICall_t JoinParty( PartyBeaconID_t ulBeaconID )
{
    PRINT_DEBUG("Steam_Parties::JoinParty\n");
    return 0;
}


// =============================================================================================
// Party Host APIs

// Get a list of possible beacon locations
bool GetNumAvailableBeaconLocations( uint32 *puNumLocations )
{
    PRINT_DEBUG("Steam_Parties::GetNumAvailableBeaconLocations\n");
    return false;
}

bool GetAvailableBeaconLocations( SteamPartyBeaconLocation_t *pLocationList, uint32 uMaxNumLocations )
{
    PRINT_DEBUG("Steam_Parties::GetAvailableBeaconLocations\n");
    return false;
}


// Create a new party beacon and activate it in the selected location.
// unOpenSlots is the maximum number of users that Steam will send to you.
// When people begin responding to your beacon, Steam will send you
// PartyReservationCallback_t callbacks to let you know who is on the way.
STEAM_CALL_RESULT( CreateBeaconCallback_t )
SteamAPICall_t CreateBeacon( uint32 unOpenSlots, SteamPartyBeaconLocation_t *pBeaconLocation, const char *pchConnectString, const char *pchMetadata )
{
    PRINT_DEBUG("Steam_Parties::CreateBeacon\n");
    return 0;
}


// Call this function when a user that had a reservation (see callback below) 
// has successfully joined your party.
// Steam will manage the remaining open slots automatically.
void OnReservationCompleted( PartyBeaconID_t ulBeacon, CSteamID steamIDUser )
{
    PRINT_DEBUG("Steam_Parties::OnReservationCompleted\n");
}


// To cancel a reservation (due to timeout or user input), call this.
// Steam will open a new reservation slot.
// Note: The user may already be in-flight to your game, so it's possible they will still connect and try to join your party.
void CancelReservation( PartyBeaconID_t ulBeacon, CSteamID steamIDUser )
{
    PRINT_DEBUG("Steam_Parties::CancelReservation\n");
}


// Change the number of open beacon reservation slots.
// Call this if, for example, someone without a reservation joins your party (eg a friend, or via your own matchmaking system).
STEAM_CALL_RESULT( ChangeNumOpenSlotsCallback_t )
SteamAPICall_t ChangeNumOpenSlots( PartyBeaconID_t ulBeacon, uint32 unOpenSlots )
{
    PRINT_DEBUG("Steam_Parties::ChangeNumOpenSlots\n");
    return 0;
}


// Turn off the beacon. 
bool DestroyBeacon( PartyBeaconID_t ulBeacon )
{
    PRINT_DEBUG("Steam_Parties::DestroyBeacon\n");
    return false;
}


// Utils
bool GetBeaconLocationData( SteamPartyBeaconLocation_t BeaconLocation, ESteamPartyBeaconLocationData eData, STEAM_OUT_STRING_COUNT(cchDataStringOut) char *pchDataStringOut, int cchDataStringOut )
{
    PRINT_DEBUG("Steam_Parties::GetBeaconLocationData\n");
    return false;
}


void RunCallbacks()
{
}

void Callback(Common_Message *msg)
{
    if (msg->has_low_level()) {
        if (msg->low_level().type() == Low_Level::CONNECT) {
            
        }

        if (msg->low_level().type() == Low_Level::DISCONNECT) {

        }
    }

    if (msg->has_networking_sockets()) {

    }
}

};
