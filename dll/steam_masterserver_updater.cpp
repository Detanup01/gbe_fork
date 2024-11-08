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

#include "dll/steam_masterserver_updater.h"


// old games like appid 50300 use this interface instead of ISteamGameServer
// we use it as a proxy for ISteamGameServer since it has an equivalent API
// and handles the actual game server logic


void Steam_Masterserver_Updater::steam_callback(void *object, Common_Message *msg)
{
    // PRINT_DEBUG_ENTRY();

    Steam_Masterserver_Updater *steam_masterserverupdater = (Steam_Masterserver_Updater *)object;
    steam_masterserverupdater->Callback(msg);
}

void Steam_Masterserver_Updater::steam_run_every_runcb(void *object)
{
    // PRINT_DEBUG_ENTRY();

    Steam_Masterserver_Updater *steam_masterserverupdater = (Steam_Masterserver_Updater *)object;
    steam_masterserverupdater->RunCallbacks();
}


Steam_Masterserver_Updater::Steam_Masterserver_Updater(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb, class Steam_GameServer *gameserver)
{
    this->settings = settings;
    this->network = network;
    this->callback_results = callback_results;
    this->callbacks = callbacks;
    this->run_every_runcb = run_every_runcb;
    this->gameserver = gameserver;
    
    this->network->setCallback(CALLBACK_ID_USER_STATUS, settings->get_local_steam_id(), &Steam_Masterserver_Updater::steam_callback, this);
    this->run_every_runcb->add(&Steam_Masterserver_Updater::steam_run_every_runcb, this);
}

Steam_Masterserver_Updater::~Steam_Masterserver_Updater()
{
    this->network->rmCallback(CALLBACK_ID_USER_STATUS, settings->get_local_steam_id(), &Steam_Masterserver_Updater::steam_callback, this);
    this->run_every_runcb->remove(&Steam_Masterserver_Updater::steam_run_every_runcb, this);
}


// Call this as often as you like to tell the master server updater whether or not
// you want it to be active (default: off).
void Steam_Masterserver_Updater::SetActive( bool bActive )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    gameserver->EnableHeartbeats(bActive);
}


// You usually don't need to modify this.
// Pass -1 to use the default value for iHeartbeatInterval.
// Some mods change this.
void Steam_Masterserver_Updater::SetHeartbeatInterval( int iHeartbeatInterval )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    gameserver->SetHeartbeatInterval(iHeartbeatInterval);
}



// These are in GameSocketShare mode, where instead of ISteamMasterServerUpdater creating its own
// socket to talk to the master server on, it lets the game use its socket to forward messages
// back and forth. This prevents us from requiring server ops to open up yet another port
// in their firewalls.
//
// the IP address and port should be in host order, i.e 127.0.0.1 == 0x7f000001

// These are used when you've elected to multiplex the game server's UDP socket
// rather than having the master server updater use its own sockets.
// 
// Source games use this to simplify the job of the server admins, so they 
// don't have to open up more ports on their firewalls.

// Call this when a packet that starts with 0xFFFFFFFF comes in. That means
// it's for us.
bool Steam_Masterserver_Updater::HandleIncomingPacket( const void *pData, int cbData, uint32 srcIP, uint16 srcPort )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return gameserver->HandleIncomingPacket(pData, cbData, srcIP, srcPort);
}


// AFTER calling HandleIncomingPacket for any packets that came in that frame, call this.
// This gets a packet that the master server updater needs to send out on UDP.
// It returns the length of the packet it wants to send, or 0 if there are no more packets to send.
// Call this each frame until it returns 0.
int Steam_Masterserver_Updater::GetNextOutgoingPacket( void *pOut, int cbMaxOut, uint32 *pNetAdr, uint16 *pPort )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return gameserver->GetNextOutgoingPacket(pOut, cbMaxOut, pNetAdr, pPort);
}



// Functions to set various fields that are used to respond to queries.

// Call this to set basic data that is passed to the server browser.
void Steam_Masterserver_Updater::SetBasicServerData(
    unsigned short nProtocolVersion,
    bool bDedicatedServer,
    const char *pRegionName,
    const char *pProductName,
    unsigned short nMaxReportedClients,
    bool bPasswordProtected,
    const char *pGameDescription )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    gameserver->SetDedicatedServer(bDedicatedServer);
    gameserver->SetRegion(pRegionName);
    gameserver->SetProduct(pProductName);
    gameserver->SetMaxPlayerCount(nMaxReportedClients);
    gameserver->SetPasswordProtected(bPasswordProtected);
    gameserver->SetGameDescription(pGameDescription);
}


// Call this to clear the whole list of key/values that are sent in rules queries.
void Steam_Masterserver_Updater::ClearAllKeyValues()
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    gameserver->ClearAllKeyValues();
}


// Call this to add/update a key/value pair.
void Steam_Masterserver_Updater::SetKeyValue( const char *pKey, const char *pValue )
{
    PRINT_DEBUG("TODO '%s'='%s'", pKey, pValue);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    gameserver->SetKeyValue(pKey, pValue);
}



// You can call this upon shutdown to clear out data stored for this game server and
// to tell the master servers that this server is going away.
void Steam_Masterserver_Updater::NotifyShutdown()
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    gameserver->LogOff();
}


// Returns true if the master server has requested a restart.
// Only returns true once per request.
bool Steam_Masterserver_Updater::WasRestartRequested()
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    return gameserver->WasRestartRequested();
}


// Force it to request a heartbeat from the master servers.
void Steam_Masterserver_Updater::ForceHeartbeat()
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    
    gameserver->ForceHeartbeat();
}


// Manually edit and query the master server list.
// It will provide name resolution and use the default master server port if none is provided.
bool Steam_Masterserver_Updater::AddMasterServer( const char *pServerAddress )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return true;
}

bool Steam_Masterserver_Updater::RemoveMasterServer( const char *pServerAddress )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return true;
}


int Steam_Masterserver_Updater::GetNumMasterServers()
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return 0;
}


// Returns the # of bytes written to pOut.
int Steam_Masterserver_Updater::GetMasterServerAddress( int iServer, char *pOut, int outBufferSize )
{
    PRINT_DEBUG_TODO();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    return 0;
}



void Steam_Masterserver_Updater::RunCallbacks()
{
}

void Steam_Masterserver_Updater::Callback(Common_Message *msg)
{
    if (msg->has_low_level()) {
        if (msg->low_level().type() == Low_Level::CONNECT) {
            
        }

        if (msg->low_level().type() == Low_Level::DISCONNECT) {

        }
    }
}
