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

class Steam_Networking_Utils :
public ISteamNetworkingUtils001,
public ISteamNetworkingUtils002,
public ISteamNetworkingUtils003,
public ISteamNetworkingUtils
{
    class Settings *settings;
    class Networking *network;
    class SteamCallResults *callback_results;
    class SteamCallBacks *callbacks;
    class RunEveryRunCB *run_every_runcb;
    std::chrono::time_point<std::chrono::steady_clock> initialized_time = std::chrono::steady_clock::now();
    FSteamNetworkingSocketsDebugOutput debug_function;
    bool relay_initialized = false;
    bool init_relay = false;

public:
static void steam_callback(void *object, Common_Message *msg)
{
    PRINT_DEBUG("steam_networkingutils_callback\n");

    Steam_Networking_Utils *steam_networkingutils = (Steam_Networking_Utils *)object;
    steam_networkingutils->Callback(msg);
}

static void steam_run_every_runcb(void *object)
{
    PRINT_DEBUG("steam_networkingutils_run_every_runcb\n");

    Steam_Networking_Utils *steam_networkingutils = (Steam_Networking_Utils *)object;
    steam_networkingutils->RunCallbacks();
}

Steam_Networking_Utils(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb)
{
    this->settings = settings;
    this->network = network;
    this->run_every_runcb = run_every_runcb;
    //this->network->setCallback(CALLBACK_ID_USER_STATUS, settings->get_local_steam_id(), &Steam_Networking_Utils::steam_callback, this);
    this->run_every_runcb->add(&Steam_Networking_Utils::steam_run_every_runcb, this);

    this->callback_results = callback_results;
    this->callbacks = callbacks;
}

~Steam_Networking_Utils()
{
    //TODO rm network callbacks
    this->run_every_runcb->remove(&Steam_Networking_Utils::steam_run_every_runcb, this);
}

static void free_steam_message_data(SteamNetworkingMessage_t *pMsg)
{
    free(pMsg->m_pData);
    pMsg->m_pData = NULL;
}

static void delete_steam_message(SteamNetworkingMessage_t *pMsg)
{
    if (pMsg->m_pfnFreeData) pMsg->m_pfnFreeData(pMsg);
    delete pMsg;
}

/// Allocate and initialize a message object.  Usually the reason
/// you call this is to pass it to ISteamNetworkingSockets::SendMessages.
/// The returned object will have all of the relevant fields cleared to zero.
///
/// Optionally you can also request that this system allocate space to
/// hold the payload itself.  If cbAllocateBuffer is nonzero, the system
/// will allocate memory to hold a payload of at least cbAllocateBuffer bytes.
/// m_pData will point to the allocated buffer, m_cbSize will be set to the
/// size, and m_pfnFreeData will be set to the proper function to free up
/// the buffer.
///
/// If cbAllocateBuffer=0, then no buffer is allocated.  m_pData will be NULL,
/// m_cbSize will be zero, and m_pfnFreeData will be NULL.  You will need to
/// set each of these.
///
/// You can use SteamNetworkingMessage_t::Release to free up the message
/// bookkeeping object and any associated buffer.  See
/// ISteamNetworkingSockets::SendMessages for details on reference
/// counting and ownership.
SteamNetworkingMessage_t *AllocateMessage( int cbAllocateBuffer )
{
    PRINT_DEBUG("Steam_Networking_Utils::AllocateMessage\n");
    SteamNetworkingMessage_t *pMsg = new SteamNetworkingMessage_t();
    pMsg->m_pfnFreeData = &free_steam_message_data;
    pMsg->m_pfnRelease = &delete_steam_message;
    if (cbAllocateBuffer < 0)
        cbAllocateBuffer = 0;

    if (cbAllocateBuffer)
        pMsg->m_pData = malloc(cbAllocateBuffer);

    pMsg->m_cbSize = cbAllocateBuffer;
    return pMsg;
}

bool InitializeRelayAccess()
{
    PRINT_DEBUG("Steam_Networking_Utils::InitializeRelayAccess\n");
    init_relay = true;
    return relay_initialized;
}

SteamRelayNetworkStatus_t get_network_status()
{
    SteamRelayNetworkStatus_t data = {};
    data.m_eAvail = k_ESteamNetworkingAvailability_Current;
    data.m_bPingMeasurementInProgress = 0;
    data.m_eAvailAnyRelay = k_ESteamNetworkingAvailability_Current;
    data.m_eAvailNetworkConfig = k_ESteamNetworkingAvailability_Current;
    strcpy(data.m_debugMsg, "OK");
    return data;
}

/// Fetch current status of the relay network.
///
/// SteamRelayNetworkStatus_t is also a callback.  It will be triggered on
/// both the user and gameserver interfaces any time the status changes, or
/// ping measurement starts or stops.
///
/// SteamRelayNetworkStatus_t::m_eAvail is returned.  If you want
/// more details, you can pass a non-NULL value.
ESteamNetworkingAvailability GetRelayNetworkStatus( SteamRelayNetworkStatus_t *pDetails )
{
    PRINT_DEBUG("Steam_Networking_Utils::GetRelayNetworkStatus %p\n", pDetails);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    //TODO: check if this is how real steam returns it
    SteamRelayNetworkStatus_t data = {};
    if (relay_initialized) {
        data = get_network_status();
    }

    if (pDetails) {
        *pDetails = data;
    }

    return k_ESteamNetworkingAvailability_Current;
}

float GetLocalPingLocation( SteamNetworkPingLocation_t &result )
{
    PRINT_DEBUG("Steam_Networking_Utils::GetLocalPingLocation\n");
    if (relay_initialized) {
        result.m_data[2] = 123;
        result.m_data[8] = 67;
        return 2.0;
    }

    return -1;
}

int EstimatePingTimeBetweenTwoLocations( const SteamNetworkPingLocation_t &location1, const SteamNetworkPingLocation_t &location2 )
{
    PRINT_DEBUG("Steam_Networking_Utils::EstimatePingTimeBetweenTwoLocations\n");
    //return k_nSteamNetworkingPing_Unknown;
    return 10;
}


int EstimatePingTimeFromLocalHost( const SteamNetworkPingLocation_t &remoteLocation )
{
    PRINT_DEBUG("Steam_Networking_Utils::EstimatePingTimeFromLocalHost\n");
    return 10;
}


void ConvertPingLocationToString( const SteamNetworkPingLocation_t &location, char *pszBuf, int cchBufSize )
{
    PRINT_DEBUG("Steam_Networking_Utils::ConvertPingLocationToString\n");
    strncpy(pszBuf, "fra=10+2", cchBufSize);
}


bool ParsePingLocationString( const char *pszString, SteamNetworkPingLocation_t &result )
{
    PRINT_DEBUG("Steam_Networking_Utils::ParsePingLocationString\n");
    return true;
}


bool CheckPingDataUpToDate( float flMaxAgeSeconds )
{
    PRINT_DEBUG("Steam_Networking_Utils::CheckPingDataUpToDate %f\n", flMaxAgeSeconds);
    init_relay = true;
    return relay_initialized;
}


bool IsPingMeasurementInProgress()
{
    PRINT_DEBUG("Steam_Networking_Utils::IsPingMeasurementInProgress\n");
    return false;
}


int GetPingToDataCenter( SteamNetworkingPOPID popID, SteamNetworkingPOPID *pViaRelayPoP )
{
    PRINT_DEBUG("Steam_Networking_Utils::GetPingToDataCenter\n");
    return 0;
}


int GetDirectPingToPOP( SteamNetworkingPOPID popID )
{
    PRINT_DEBUG("Steam_Networking_Utils::GetDirectPingToPOP\n");
    return 0;
}


int GetPOPCount()
{
    PRINT_DEBUG("Steam_Networking_Utils::GetPOPCount\n");
    return 0;
}


int GetPOPList( SteamNetworkingPOPID *list, int nListSz )
{
    PRINT_DEBUG("Steam_Networking_Utils::GetPOPList\n");
    return 0;
}


//
// Misc
//

/// Fetch current timestamp.  This timer has the following properties:
///
/// - Monotonicity is guaranteed.
/// - The initial value will be at least 24*3600*30*1e6, i.e. about
///   30 days worth of microseconds.  In this way, the timestamp value of
///   0 will always be at least "30 days ago".  Also, negative numbers
///   will never be returned.
/// - Wraparound / overflow is not a practical concern.
///
/// If you are running under the debugger and stop the process, the clock
/// might not advance the full wall clock time that has elapsed between
/// calls.  If the process is not blocked from normal operation, the
/// timestamp values will track wall clock time, even if you don't call
/// the function frequently.
///
/// The value is only meaningful for this run of the process.  Don't compare
/// it to values obtained on another computer, or other runs of the same process.
SteamNetworkingMicroseconds GetLocalTimestamp()
{
    PRINT_DEBUG("Steam_Networking_Utils::GetLocalTimestamp\n");
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::steady_clock::now() - initialized_time).count() + (SteamNetworkingMicroseconds)24*3600*30*1e6;
}


/// Set a function to receive network-related information that is useful for debugging.
/// This can be very useful during development, but it can also be useful for troubleshooting
/// problems with tech savvy end users.  If you have a console or other log that customers
/// can examine, these log messages can often be helpful to troubleshoot network issues.
/// (Especially any warning/error messages.)
///
/// The detail level indicates what message to invoke your callback on.  Lower numeric
/// value means more important, and the value you pass is the lowest priority (highest
/// numeric value) you wish to receive callbacks for.
///
/// Except when debugging, you should only use k_ESteamNetworkingSocketsDebugOutputType_Msg
/// or k_ESteamNetworkingSocketsDebugOutputType_Warning.  For best performance, do NOT
/// request a high detail level and then filter out messages in your callback.  Instead,
/// call function function to adjust the desired level of detail.
///
/// IMPORTANT: This may be called from a service thread, while we own a mutex, etc.
/// Your output function must be threadsafe and fast!  Do not make any other
/// Steamworks calls from within the handler.
void SetDebugOutputFunction( ESteamNetworkingSocketsDebugOutputType eDetailLevel, FSteamNetworkingSocketsDebugOutput pfnFunc )
{
    PRINT_DEBUG("Steam_Networking_Utils::SetDebugOutputFunction %i\n", eDetailLevel);
    if (eDetailLevel != k_ESteamNetworkingSocketsDebugOutputType_None) {
        debug_function = pfnFunc;
    }
}

//
// Fake IP
//
// Useful for interfacing with code that assumes peers are identified using an IPv4 address
//

/// Return true if an IPv4 address is one that might be used as a "fake" one.
/// This function is fast; it just does some logical tests on the IP and does
/// not need to do any lookup operations.
// inline bool IsFakeIPv4( uint32 nIPv4 ) { return GetIPv4FakeIPType( nIPv4 ) > k_ESteamNetworkingFakeIPType_NotFake; }
ESteamNetworkingFakeIPType GetIPv4FakeIPType( uint32 nIPv4 )
{
    PRINT_DEBUG("TODO: %s\n", __FUNCTION__);
    return k_ESteamNetworkingFakeIPType_NotFake;
}

/// Get the real identity associated with a given FakeIP.
///
/// On failure, returns:
/// - k_EResultInvalidParam: the IP is not a FakeIP.
/// - k_EResultNoMatch: we don't recognize that FakeIP and don't know the corresponding identity.
///
/// FakeIP's used by active connections, or the FakeIPs assigned to local identities,
/// will always work.  FakeIPs for recently destroyed connections will continue to
/// return results for a little while, but not forever.  At some point, we will forget
/// FakeIPs to save space.  It's reasonably safe to assume that you can read back the
/// real identity of a connection very soon after it is destroyed.  But do not wait
/// indefinitely.
EResult GetRealIdentityForFakeIP( const SteamNetworkingIPAddr &fakeIP, SteamNetworkingIdentity *pOutRealIdentity )
{
    PRINT_DEBUG("TODO: %s\n", __FUNCTION__);
    return k_EResultNoMatch;
}


//
// Set and get configuration values, see ESteamNetworkingConfigValue for individual descriptions.
//

// Shortcuts for common cases.  (Implemented as inline functions below)
/*
bool SetGlobalConfigValueInt32( ESteamNetworkingConfigValue eValue, int32 val );
bool SetGlobalConfigValueFloat( ESteamNetworkingConfigValue eValue, float val );
bool SetGlobalConfigValueString( ESteamNetworkingConfigValue eValue, const char *val );
bool SetConnectionConfigValueInt32( HSteamNetConnection hConn, ESteamNetworkingConfigValue eValue, int32 val );
bool SetConnectionConfigValueFloat( HSteamNetConnection hConn, ESteamNetworkingConfigValue eValue, float val );
bool SetConnectionConfigValueString( HSteamNetConnection hConn, ESteamNetworkingConfigValue eValue, const char *val );
*/
/// Set a configuration value.
/// - eValue: which value is being set
/// - eScope: Onto what type of object are you applying the setting?
/// - scopeArg: Which object you want to change?  (Ignored for global scope).  E.g. connection handle, listen socket handle, interface pointer, etc.
/// - eDataType: What type of data is in the buffer at pValue?  This must match the type of the variable exactly!
/// - pArg: Value to set it to.  You can pass NULL to remove a non-global sett at this scope,
///   causing the value for that object to use global defaults.  Or at global scope, passing NULL
///   will reset any custom value and restore it to the system default.
///   NOTE: When setting callback functions, do not pass the function pointer directly.
///   Your argument should be a pointer to a function pointer.
bool SetConfigValue( ESteamNetworkingConfigValue eValue, ESteamNetworkingConfigScope eScopeType, intptr_t scopeObj,
    ESteamNetworkingConfigDataType eDataType, const void *pArg )
{
    PRINT_DEBUG("Steam_Networking_Utils::SetConfigValue %i %i " "%" PRIdPTR " %i %p\n", eValue, eScopeType, scopeObj, eDataType, pArg);
    return true;
}


/// Get a configuration value.
/// - eValue: which value to fetch
/// - eScopeType: query setting on what type of object
/// - eScopeArg: the object to query the setting for
/// - pOutDataType: If non-NULL, the data type of the value is returned.
/// - pResult: Where to put the result.  Pass NULL to query the required buffer size.  (k_ESteamNetworkingGetConfigValue_BufferTooSmall will be returned.)
/// - cbResult: IN: the size of your buffer.  OUT: the number of bytes filled in or required.
ESteamNetworkingGetConfigValueResult GetConfigValue( ESteamNetworkingConfigValue eValue, ESteamNetworkingConfigScope eScopeType, intptr_t scopeObj,
    ESteamNetworkingConfigDataType *pOutDataType, void *pResult, size_t *cbResult )
{
    PRINT_DEBUG("Steam_Networking_Utils::GetConfigValue\n");
    return k_ESteamNetworkingGetConfigValue_BadValue;
}


/// Returns info about a configuration value.  Returns false if the value does not exist.
/// pOutNextValue can be used to iterate through all of the known configuration values.
/// (Use GetFirstConfigValue() to begin the iteration, will be k_ESteamNetworkingConfig_Invalid on the last value)
/// Any of the output parameters can be NULL if you do not need that information.
bool GetConfigValueInfo( ESteamNetworkingConfigValue eValue, const char **pOutName, ESteamNetworkingConfigDataType *pOutDataType, ESteamNetworkingConfigScope *pOutScope, ESteamNetworkingConfigValue *pOutNextValue )
{
    PRINT_DEBUG("TODO: Steam_Networking_Utils::GetConfigValueInfo old\n");
    //TODO flat api
    return false;
}

/// Get info about a configuration value.  Returns the name of the value,
/// or NULL if the value doesn't exist.  Other output parameters can be NULL
/// if you do not need them.
const char *GetConfigValueInfo( ESteamNetworkingConfigValue eValue, ESteamNetworkingConfigDataType *pOutDataType, ESteamNetworkingConfigScope *pOutScope )
{
    PRINT_DEBUG("TODO: %s\n", __FUNCTION__);
    //TODO flat api
    return NULL;
}

/// Return the lowest numbered configuration value available in the current environment.
ESteamNetworkingConfigValue GetFirstConfigValue()
{
    PRINT_DEBUG("Steam_Networking_Utils::GetFirstConfigValue\n");
    return k_ESteamNetworkingConfig_Invalid;
}

/// Iterate the list of all configuration values in the current environment that it might
/// be possible to display or edit using a generic UI.  To get the first iterable value,
/// pass k_ESteamNetworkingConfig_Invalid.  Returns k_ESteamNetworkingConfig_Invalid
/// to signal end of list.
///
/// The bEnumerateDevVars argument can be used to include "dev" vars.  These are vars that
/// are recommended to only be editable in "debug" or "dev" mode and typically should not be
/// shown in a retail environment where a malicious local user might use this to cheat.
ESteamNetworkingConfigValue IterateGenericEditableConfigValues( ESteamNetworkingConfigValue eCurrent, bool bEnumerateDevVars )
{
    PRINT_DEBUG("TODO: %s\n", __FUNCTION__);
    return k_ESteamNetworkingConfig_Invalid;
}


// String conversions.  You'll usually access these using the respective
// inline methods.
void SteamNetworkingIPAddr_ToString( const SteamNetworkingIPAddr &addr, char *buf, size_t cbBuf, bool bWithPort )
{
    PRINT_DEBUG("Steam_Networking_Utils::SteamNetworkingIPAddr_ToString\n");
    if (buf == nullptr || cbBuf == 0)
        return;

    char buffer[64]; // Its enought for ipv4 & ipv6 + port
    std::string str_addr;
    if (addr.IsIPv4())
    {
        in_addr ipv4_addr;
        ipv4_addr.s_addr = htonl(addr.GetIPv4());

        if (inet_ntop(AF_INET, &ipv4_addr, buffer, sizeof(buffer) / sizeof(*buffer)) != nullptr)
        {
            if (bWithPort)
            {
                str_addr = buffer;
                str_addr += ':';
                str_addr += std::to_string(addr.m_port);
            }
            else
            {
                str_addr = buffer;
            }
        }
    }
    else
    {
        in6_addr ipv6_addr;
        memcpy(ipv6_addr.s6_addr, addr.m_ipv6, sizeof(addr.m_ipv6));

        if (inet_ntop(AF_INET6, &ipv6_addr, buffer, sizeof(buffer) / sizeof(*buffer)) != nullptr)
        {
            if (bWithPort)
            {
                str_addr = '[';
                str_addr += buffer;
                str_addr += "]:";
                str_addr += std::to_string(addr.m_port);
            }
            else
            {
                str_addr = buffer;
            }
        }
    }

    cbBuf = std::min<size_t>(cbBuf, str_addr.length() + 1);
    strncpy(buf, str_addr.c_str(), cbBuf);
    buf[cbBuf - 1] = '\0';
}

bool SteamNetworkingIPAddr_ParseString( SteamNetworkingIPAddr *pAddr, const char *pszStr )
{
    PRINT_DEBUG("Steam_Networking_Utils::SteamNetworkingIPAddr_ParseString\n");
    
    bool valid = false;

    if (pAddr == nullptr || pszStr == nullptr)
        return valid;

    std::string str(pszStr);
    size_t pos = str.find(':');

    if (pos != std::string::npos)
    {// Try ipv4 with port
        in_addr ipv4_addr;
        std::string tmp(str);
        tmp[pos] = 0;
        const char* ip = tmp.c_str();
        const char* port = &tmp[pos + 1];

        if (inet_pton(AF_INET, ip, &ipv4_addr) == 1)
        {
            valid = true;
            pAddr->SetIPv4(ntohl(ipv4_addr.s_addr), strtoul(port, nullptr, 10));
        }
    }
    else
    {// Try ipv4 without port
        in_addr ipv4_addr;
        if (inet_pton(AF_INET, str.c_str(), &ipv4_addr) == 1)
        {
            valid = true;
            pAddr->SetIPv4(ntohl(ipv4_addr.s_addr), 0);
        }
    }

    if (!valid)
    {// Try ipv6
        addrinfo* info = nullptr;
        addrinfo hints = {};
        hints.ai_family = AF_INET6;
        hints.ai_socktype = SOCK_STREAM;
        hints.ai_flags = AI_NUMERICHOST | AI_NUMERICSERV;

        size_t sep_pos = 0;
        std::string ip;
        int sep_count = 0;
        for (int i = 0; i < str.length(); ++i)
        {
            if (str[i] == ':')
            {
                sep_pos = i;
                ++sep_count;
            }
        }
        if (sep_count == 8)
        {
            ip = std::move(std::string(str.begin(), str.begin() + sep_pos));
        }
        else
        {
            ip = str;
        }

        if (getaddrinfo(ip.c_str(), nullptr, &hints, &info) == 0)
        {
            sockaddr_in6* maddr = (sockaddr_in6*)info->ai_addr;

            size_t pos = str.find(']');
            std::string str_port("0");
            if (pos != std::string::npos)
            {
                str_port = std::move(std::string(str.begin() + pos + 2, str.end()));
            }
            else if (sep_count == 8)
            {
                str_port = std::move(std::string(str.begin() + sep_pos + 1, str.end()));
            }
            try
            {
                int port = std::stoi(str_port);
                if (port >= 0 && port <= 65535)
                {
                    pAddr->SetIPv6(maddr->sin6_addr.s6_addr, port);
                    valid = true;
                }
            }
            catch(...)
            { }
        }

        if (info)
        {
            freeaddrinfo(info);
        }
    }

    if (!valid)
    {
        pAddr->Clear();
    }

    return valid;
}

ESteamNetworkingFakeIPType SteamNetworkingIPAddr_GetFakeIPType( const SteamNetworkingIPAddr &addr )
{
    PRINT_DEBUG("TODO: %s\n", __FUNCTION__);
    return k_ESteamNetworkingFakeIPType_NotFake;
}


void SteamNetworkingIdentity_ToString( const SteamNetworkingIdentity &identity, char *buf, size_t cbBuf )
{
    PRINT_DEBUG("Steam_Networking_Utils::SteamNetworkingIdentity_ToString\n");
    if (buf == nullptr)
        return;

    std::string str;
    str.reserve(SteamNetworkingIdentity::k_cchMaxString);
    switch (identity.m_eType)
    {
        case k_ESteamNetworkingIdentityType_SteamID:
        {
            str = "steamid:";
            str += std::move(std::to_string(identity.GetSteamID64()));
        }
        break;

        case k_ESteamNetworkingIdentityType_IPAddress:
        {
            str = "ip:";
            char buff[SteamNetworkingIPAddr::k_cchMaxString];
            auto& addr = *identity.GetIPAddr();
            SteamNetworkingIPAddr_ToString(addr, buff, sizeof(buff), true);
            str += buff;
        }
        break;

        case k_ESteamNetworkingIdentityType_GenericBytes:
        {
            int generic_len;
            const uint8* pBuf = identity.GetGenericBytes(generic_len);

            str = "gen:";
            str.resize(4 + (generic_len * 2));

            char* pDest = &str[4];
            while(generic_len--)
            {
                // I don't care for the last char, I've reserved the max string size
                snprintf(pDest, 3, "%02x", *pBuf); 
                ++pBuf;
                pDest += 2;
            }
        }
        break;

        case k_ESteamNetworkingIdentityType_GenericString:
        {
            str = "str:";
            str += identity.GetGenericString();
        }
        break;

        case k_ESteamNetworkingIdentityType_UnknownType:
        {
            str = identity.m_szUnknownRawString;
        }
        break;
    }
    cbBuf = std::min<size_t>(cbBuf, str.length() + 1);
    strncpy(buf, str.c_str(), cbBuf);
    buf[cbBuf - 1] = '\0';
}

bool SteamNetworkingIdentity_ParseString( SteamNetworkingIdentity *pIdentity, const char *pszStr )
{
    PRINT_DEBUG("Steam_Networking_Utils::SteamNetworkingIdentity_ParseString\n");
    bool valid = false;
    if (pIdentity == nullptr)
    {
        return valid;
    }

    if (pszStr != nullptr)
    {
        const char* end = strchr(pszStr, ':');
        if (end != nullptr)
        {
            ++end;
            if (strncmp(pszStr, "gen:", end - pszStr) == 0)
            {
                size_t length = strlen(end);
                if (!(length % 2) && length <= (sizeof(pIdentity->m_genericBytes) * 2))
                {// Must be even
                    valid = true;
                    length /= 2;
                    pIdentity->m_eType = k_ESteamNetworkingIdentityType_GenericBytes;
                    pIdentity->m_cbSize = length;
                    uint8* pBytes = pIdentity->m_genericBytes;

                    char hex[3] = { 0,0,0 };
                    while (length)
                    {
                        hex[0] = end[0];
                        hex[1] = end[1];
                        // Steam doesn't check if wasn't a hex char
                        *pBytes = strtol(hex, nullptr, 16);

                        ++pBytes;
                        end += 2;
                        --length;
                    }
                }
            }
            else if (strncmp(pszStr, "steamid:", end - pszStr) == 0)
            {
                CSteamID steam_id(uint64(strtoull(end, nullptr, 10)));
                if (steam_id.IsValid())
                {
                    valid = true;
                    pIdentity->SetSteamID(steam_id);
                }
            }
            else if (strncmp(pszStr, "str:", end - pszStr) == 0)
            {
                valid = pIdentity->SetGenericString(end);
            }
            else if (strncmp(pszStr, "ip:", end - pszStr) == 0)
            {
                SteamNetworkingIPAddr steam_addr;
                if (SteamNetworkingIPAddr_ParseString(&steam_addr, end))
                {
                    valid = true;
                    pIdentity->SetIPAddr(steam_addr);
                }
            }
        }
    }

    return valid;
}


void RunCallbacks()
{
    if (init_relay && !relay_initialized) {
        relay_initialized = true;
        SteamRelayNetworkStatus_t data = get_network_status();
        callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    }
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
