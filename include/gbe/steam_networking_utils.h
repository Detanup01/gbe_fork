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

#ifndef __INCLUDED_STEAM_NETWORKING_UTILS_H__
#define __INCLUDED_STEAM_NETWORKING_UTILS_H__

#include "base.h"

class Steam_Networking_Utils :
public ISteamNetworkingUtils001,
public ISteamNetworkingUtils002,
public ISteamNetworkingUtils003,
public ISteamNetworkingUtils
{
    class Settings *settings{};
    class Networking *network{};
    class SteamCallResults *callback_results{};
    class SteamCallBacks *callbacks{};
    class RunEveryRunCB *run_every_runcb{};

    std::chrono::time_point<std::chrono::steady_clock> initialized_time = std::chrono::steady_clock::now();
    FSteamNetworkingSocketsDebugOutput debug_function{};
    bool relay_initialized = false;
    bool init_relay = true; //  Initializing relay immediately when we ask for it. Fixing Elden Ring SeamlessCoop
    // NOTE from Detanup01: They should call InitializeRelayAccess BTW. Whoever write that mod cannot read valve docs.
    /*
    Valve docs:
    If you know that you are going to be using the relay network
    (for example, because you anticipate making P2P connections), 
    call this to initialize the relay network. 
    If you do not call this, the initialization will be delayed until
    the first time you use a feature that requires access to the relay network, 
    which will delay that first access.
    */

    static void free_steam_message_data(SteamNetworkingMessage_t *pMsg);
    static void delete_steam_message(SteamNetworkingMessage_t *pMsg);

    static void steam_callback(void *object, Common_Message *msg);
    static void steam_run_every_runcb(void *object);

public:
    Steam_Networking_Utils(class Settings *settings, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks, class RunEveryRunCB *run_every_runcb);
    ~Steam_Networking_Utils();

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
    SteamNetworkingMessage_t *AllocateMessage( int cbAllocateBuffer );

    bool InitializeRelayAccess();

    SteamRelayNetworkStatus_t get_network_status();

    /// Fetch current status of the relay network.
    ///
    /// SteamRelayNetworkStatus_t is also a callback.  It will be triggered on
    /// both the user and gameserver interfaces any time the status changes, or
    /// ping measurement starts or stops.
    ///
    /// SteamRelayNetworkStatus_t::m_eAvail is returned.  If you want
    /// more details, you can pass a non-NULL value.
    ESteamNetworkingAvailability GetRelayNetworkStatus( SteamRelayNetworkStatus_t *pDetails );

    float GetLocalPingLocation( SteamNetworkPingLocation_t &result );

    int EstimatePingTimeBetweenTwoLocations( const SteamNetworkPingLocation_t &location1, const SteamNetworkPingLocation_t &location2 );


    int EstimatePingTimeFromLocalHost( const SteamNetworkPingLocation_t &remoteLocation );


    void ConvertPingLocationToString( const SteamNetworkPingLocation_t &location, char *pszBuf, int cchBufSize );


    bool ParsePingLocationString( const char *pszString, SteamNetworkPingLocation_t &result );


    bool CheckPingDataUpToDate( float flMaxAgeSeconds );


    bool IsPingMeasurementInProgress();


    int GetPingToDataCenter( SteamNetworkingPOPID popID, SteamNetworkingPOPID *pViaRelayPoP );


    int GetDirectPingToPOP( SteamNetworkingPOPID popID );


    int GetPOPCount();


    int GetPOPList( SteamNetworkingPOPID *list, int nListSz );


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
    SteamNetworkingMicroseconds GetLocalTimestamp();


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
    void SetDebugOutputFunction( ESteamNetworkingSocketsDebugOutputType eDetailLevel, FSteamNetworkingSocketsDebugOutput pfnFunc );

    //
    // Fake IP
    //
    // Useful for interfacing with code that assumes peers are identified using an IPv4 address
    //

    /// Return true if an IPv4 address is one that might be used as a "fake" one.
    /// This function is fast; it just does some logical tests on the IP and does
    /// not need to do any lookup operations.
    // inline bool IsFakeIPv4( uint32 nIPv4 ) { return GetIPv4FakeIPType( nIPv4 ) > k_ESteamNetworkingFakeIPType_NotFake; }
    ESteamNetworkingFakeIPType GetIPv4FakeIPType( uint32 nIPv4 );

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
    EResult GetRealIdentityForFakeIP( const SteamNetworkingIPAddr &fakeIP, SteamNetworkingIdentity *pOutRealIdentity );


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
        ESteamNetworkingConfigDataType eDataType, const void *pArg );


    /// Get a configuration value.
    /// - eValue: which value to fetch
    /// - eScopeType: query setting on what type of object
    /// - eScopeArg: the object to query the setting for
    /// - pOutDataType: If non-NULL, the data type of the value is returned.
    /// - pResult: Where to put the result.  Pass NULL to query the required buffer size.  (k_ESteamNetworkingGetConfigValue_BufferTooSmall will be returned.)
    /// - cbResult: IN: the size of your buffer.  OUT: the number of bytes filled in or required.
    ESteamNetworkingGetConfigValueResult GetConfigValue( ESteamNetworkingConfigValue eValue, ESteamNetworkingConfigScope eScopeType, intptr_t scopeObj,
        ESteamNetworkingConfigDataType *pOutDataType, void *pResult, size_t *cbResult );


    /// Returns info about a configuration value.  Returns false if the value does not exist.
    /// pOutNextValue can be used to iterate through all of the known configuration values.
    /// (Use GetFirstConfigValue() to begin the iteration, will be k_ESteamNetworkingConfig_Invalid on the last value)
    /// Any of the output parameters can be NULL if you do not need that information.
    bool GetConfigValueInfo( ESteamNetworkingConfigValue eValue, const char **pOutName, ESteamNetworkingConfigDataType *pOutDataType, ESteamNetworkingConfigScope *pOutScope, ESteamNetworkingConfigValue *pOutNextValue );

    /// Get info about a configuration value.  Returns the name of the value,
    /// or NULL if the value doesn't exist.  Other output parameters can be NULL
    /// if you do not need them.
    const char *GetConfigValueInfo( ESteamNetworkingConfigValue eValue, ESteamNetworkingConfigDataType *pOutDataType, ESteamNetworkingConfigScope *pOutScope );

    /// Return the lowest numbered configuration value available in the current environment.
    ESteamNetworkingConfigValue GetFirstConfigValue();

    /// Iterate the list of all configuration values in the current environment that it might
    /// be possible to display or edit using a generic UI.  To get the first iterable value,
    /// pass k_ESteamNetworkingConfig_Invalid.  Returns k_ESteamNetworkingConfig_Invalid
    /// to signal end of list.
    ///
    /// The bEnumerateDevVars argument can be used to include "dev" vars.  These are vars that
    /// are recommended to only be editable in "debug" or "dev" mode and typically should not be
    /// shown in a retail environment where a malicious local user might use this to cheat.
    ESteamNetworkingConfigValue IterateGenericEditableConfigValues( ESteamNetworkingConfigValue eCurrent, bool bEnumerateDevVars );


    // String conversions.  You'll usually access these using the respective
    // inline methods.
    void SteamNetworkingIPAddr_ToString( const SteamNetworkingIPAddr &addr, char *buf, size_t cbBuf, bool bWithPort );

    bool SteamNetworkingIPAddr_ParseString( SteamNetworkingIPAddr *pAddr, const char *pszStr );

    ESteamNetworkingFakeIPType SteamNetworkingIPAddr_GetFakeIPType( const SteamNetworkingIPAddr &addr );


    void SteamNetworkingIdentity_ToString( const SteamNetworkingIdentity &identity, char *buf, size_t cbBuf );

    bool SteamNetworkingIdentity_ParseString( SteamNetworkingIdentity *pIdentity, const char *pszStr );


    void RunCallbacks();

    void Callback(Common_Message *msg);

};

#endif // __INCLUDED_STEAM_NETWORKING_UTILS_H__
