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

class Steam_Networking_Sockets_Serialized :
public ISteamNetworkingSocketsSerialized002,
public ISteamNetworkingSocketsSerialized003,
public ISteamNetworkingSocketsSerialized004,
public ISteamNetworkingSocketsSerialized005
{
    class Settings *settings;
    class Networking *network;
    class SteamCallResults *callback_results;
    class SteamCallBacks *callbacks;
    class RunEveryRunCB *run_every_runcb;

public:
static void steam_callback(void *object, Common_Message *msg)
{
    PRINT_DEBUG("steam_networkingsockets_callback\n");

    Steam_Networking_Sockets_Serialized *steam_networkingsockets = (Steam_Networking_Sockets_Serialized *)object;
    steam_networkingsockets->Callback(msg);
}

static void steam_run_every_runcb(void *object)
{
    PRINT_DEBUG("steam_networkingsockets_run_every_runcb\n");

    Steam_Networking_Sockets_Serialized *steam_networkingsockets = (Steam_Networking_Sockets_Serialized *)object;
    steam_networkingsockets->RunCallbacks();
}

Steam_Networking_Sockets_Serialized(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb)
{
    this->settings = settings;
    this->network = network;
    this->run_every_runcb = run_every_runcb;
    this->network->setCallback(CALLBACK_ID_USER_STATUS, settings->get_local_steam_id(), &Steam_Networking_Sockets_Serialized::steam_callback, this);
    this->run_every_runcb->add(&Steam_Networking_Sockets_Serialized::steam_run_every_runcb, this);

    this->callback_results = callback_results;
    this->callbacks = callbacks;
}

~Steam_Networking_Sockets_Serialized()
{
    //TODO rm network callbacks
    this->run_every_runcb->remove(&Steam_Networking_Sockets_Serialized::steam_run_every_runcb, this);
}

void SendP2PRendezvous( CSteamID steamIDRemote, uint32 unConnectionIDSrc, const void *pMsgRendezvous, uint32 cbRendezvous )
{
    PRINT_DEBUG("Steam_Networking_Sockets_Serialized::SendP2PRendezvous\n");
}

void SendP2PConnectionFailure( CSteamID steamIDRemote, uint32 unConnectionIDDest, uint32 nReason, const char *pszReason )
{
    PRINT_DEBUG("Steam_Networking_Sockets_Serialized::SendP2PConnectionFailure\n");
}

SteamAPICall_t GetCertAsync()
{
    PRINT_DEBUG("Steam_Networking_Sockets_Serialized::GetCertAsync\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    struct SteamNetworkingSocketsCert_t data = {};
    data.m_eResult = k_EResultOK;

    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}

int GetNetworkConfigJSON( void *buf, uint32 cbBuf, const char *pszLauncherPartner )
{
    PRINT_DEBUG("Steam_Networking_Sockets_Serialized::GetNetworkConfigJSON %s\n", pszLauncherPartner);
    return 0;
}

int GetNetworkConfigJSON( void *buf, uint32 cbBuf )
{
    PRINT_DEBUG("Steam_Networking_Sockets_Serialized::GetNetworkConfigJSON old\n");
    return GetNetworkConfigJSON(buf, cbBuf, "");
}

void CacheRelayTicket( const void *pTicket, uint32 cbTicket )
{
    PRINT_DEBUG("Steam_Networking_Sockets_Serialized::CacheRelayTicket\n");
}

uint32 GetCachedRelayTicketCount()
{
    PRINT_DEBUG("Steam_Networking_Sockets_Serialized::GetCachedRelayTicketCount\n");
    return 0;
}

int GetCachedRelayTicket( uint32 idxTicket, void *buf, uint32 cbBuf )
{
    PRINT_DEBUG("Steam_Networking_Sockets_Serialized::GetCachedRelayTicket\n");
    return 0;
}

void PostConnectionStateMsg( const void *pMsg, uint32 cbMsg )
{
    PRINT_DEBUG("Steam_Networking_Sockets_Serialized::PostConnectionStateMsg\n");
}

bool GetSTUNServer(int dont_know, char *buf, unsigned int len)
{
    PRINT_DEBUG("Steam_Networking_Sockets_Serialized::GetSTUNServer %i %p %u\n", dont_know, buf, len);
    return false;
}

bool BAllowDirectConnectToPeer(SteamNetworkingIdentity const &identity)
{
    PRINT_DEBUG("Steam_Networking_Sockets_Serialized::BAllowDirectConnectToPeer\n");
    return true;
}

int BeginAsyncRequestFakeIP(int a)
{
    PRINT_DEBUG("Steam_Networking_Sockets_Serialized::BeginAsyncRequestFakeIP\n");
    return true;
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
}

};
