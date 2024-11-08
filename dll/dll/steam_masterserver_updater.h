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

#ifndef __INCLUDED_STEAM_MASTERSERVER_UPDATER_H__
#define __INCLUDED_STEAM_MASTERSERVER_UPDATER_H__


#include "base.h"
#include "dll/steam_gameserver.h"


class Steam_Masterserver_Updater :
public ISteamMasterServerUpdater
{
    class Settings *settings{};
    class Networking *network{};
    class SteamCallResults *callback_results{};
    class SteamCallBacks *callbacks{};
    class RunEveryRunCB *run_every_runcb{};
    class Steam_GameServer *gameserver{};

    static void steam_callback(void *object, Common_Message *msg);
    static void steam_run_every_runcb(void *object);

    void RunCallbacks();
    void Callback(Common_Message *msg);

public:
    Steam_Masterserver_Updater(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb, class Steam_GameServer *gameserver);
    ~Steam_Masterserver_Updater();

    // Call this as often as you like to tell the master server updater whether or not
    // you want it to be active (default: off).
    void SetActive( bool bActive );


    // You usually don't need to modify this.
    // Pass -1 to use the default value for iHeartbeatInterval.
    // Some mods change this.
    void SetHeartbeatInterval( int iHeartbeatInterval );



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
    bool HandleIncomingPacket( const void *pData, int cbData, uint32 srcIP, uint16 srcPort );


    // AFTER calling HandleIncomingPacket for any packets that came in that frame, call this.
    // This gets a packet that the master server updater needs to send out on UDP.
    // It returns the length of the packet it wants to send, or 0 if there are no more packets to send.
    // Call this each frame until it returns 0.
    int GetNextOutgoingPacket( void *pOut, int cbMaxOut, uint32 *pNetAdr, uint16 *pPort );



    // Functions to set various fields that are used to respond to queries.

    // Call this to set basic data that is passed to the server browser.
    void SetBasicServerData(
        unsigned short nProtocolVersion,
        bool bDedicatedServer,
        const char *pRegionName,
        const char *pProductName,
        unsigned short nMaxReportedClients,
        bool bPasswordProtected,
        const char *pGameDescription );


    // Call this to clear the whole list of key/values that are sent in rules queries.
    void ClearAllKeyValues();


    // Call this to add/update a key/value pair.
    void SetKeyValue( const char *pKey, const char *pValue );



    // You can call this upon shutdown to clear out data stored for this game server and
    // to tell the master servers that this server is going away.
    void NotifyShutdown();


    // Returns true if the master server has requested a restart.
    // Only returns true once per request.
    bool WasRestartRequested();


    // Force it to request a heartbeat from the master servers.
    void ForceHeartbeat();


    // Manually edit and query the master server list.
    // It will provide name resolution and use the default master server port if none is provided.
    bool AddMasterServer( const char *pServerAddress );

    bool RemoveMasterServer( const char *pServerAddress );


    int GetNumMasterServers();


    // Returns the # of bytes written to pOut.
    int GetMasterServerAddress( int iServer, char *pOut, int outBufferSize );

};

#endif // __INCLUDED_STEAM_MASTERSERVER_UPDATER_H__
