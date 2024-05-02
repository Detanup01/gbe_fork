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

#ifndef __INCLUDED_STEAM_NETWORKING_SOCKETSERIALIZED_H__
#define __INCLUDED_STEAM_NETWORKING_SOCKETSERIALIZED_H__

#include "base.h"

class Steam_Networking_Sockets_Serialized :
public ISteamNetworkingSocketsSerialized002,
public ISteamNetworkingSocketsSerialized003,
public ISteamNetworkingSocketsSerialized004,
public ISteamNetworkingSocketsSerialized005
{
    class Settings *settings{};
    class Networking *network{};
    class SteamCallResults *callback_results{};
    class SteamCallBacks *callbacks{};
    class RunEveryRunCB *run_every_runcb{};

    static void steam_callback(void *object, Common_Message *msg);
    static void steam_run_every_runcb(void *object);

public:
    Steam_Networking_Sockets_Serialized(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb);
    ~Steam_Networking_Sockets_Serialized();

    void SendP2PRendezvous( CSteamID steamIDRemote, uint32 unConnectionIDSrc, const void *pMsgRendezvous, uint32 cbRendezvous );

    void SendP2PConnectionFailure( CSteamID steamIDRemote, uint32 unConnectionIDDest, uint32 nReason, const char *pszReason );

    SteamAPICall_t GetCertAsync();

    int GetNetworkConfigJSON( void *buf, uint32 cbBuf, const char *pszLauncherPartner );

    int GetNetworkConfigJSON( void *buf, uint32 cbBuf );

    void CacheRelayTicket( const void *pTicket, uint32 cbTicket );

    uint32 GetCachedRelayTicketCount();

    int GetCachedRelayTicket( uint32 idxTicket, void *buf, uint32 cbBuf );

    void PostConnectionStateMsg( const void *pMsg, uint32 cbMsg );

    bool GetSTUNServer(int dont_know, char *buf, unsigned int len);

    bool BAllowDirectConnectToPeer(SteamNetworkingIdentity const &identity);

    int BeginAsyncRequestFakeIP(int a);

    void RunCallbacks();

    void Callback(Common_Message *msg);

};

#endif // __INCLUDED_STEAM_NETWORKING_SOCKETSERIALIZED_H__
