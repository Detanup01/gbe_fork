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

#ifndef __INCLUDED_STEAM_GAMESERVER_H__
#define __INCLUDED_STEAM_GAMESERVER_H__

#include "base.h"
#include "auth.h"

//-----------------------------------------------------------------------------
// Purpose: Functions for authenticating users via Steam to play on a game server
//-----------------------------------------------------------------------------

struct Gameserver_Outgoing_Packet {
    std::vector<uint8_t> data{};

    uint32 ip{};
    uint16 port{};
};

struct Gameserver_Player_Info_t {
    std::chrono::steady_clock::time_point join_time{};
    std::string name{};
    uint32 score{};
};

class Steam_GameServer : 
public ISteamGameServer002,
public ISteamGameServer003,
public ISteamGameServer004,
public ISteamGameServer005,
public ISteamGameServer008,
public ISteamGameServer009,
public ISteamGameServer010,
public ISteamGameServer011,
public ISteamGameServer012,
public ISteamGameServer013,
public ISteamGameServer014,
public ISteamGameServer
{
    class Settings *settings{};
    class Networking *network{};
    class SteamCallBacks *callbacks{};

    CSteamID steam_id{};

    bool call_servers_connected = false;
    bool logged_in = false;
    bool call_servers_disconnected = false;
    Gameserver server_data{};
    std::vector<std::pair<CSteamID, Gameserver_Player_Info_t>> players{};

    uint32 flags{};
    bool policy_response_called{};

    std::chrono::high_resolution_clock::time_point last_sent_server_info{};
    Auth_Manager *auth_manager{};

    std::vector<struct Gameserver_Outgoing_Packet> outgoing_packets{};

    void set_version(const char *pchVersionString);


public:
    Steam_GameServer(class Settings *settings, class Networking *network, class SteamCallBacks *callbacks);
    ~Steam_GameServer();

    std::vector<std::pair<CSteamID, Gameserver_Player_Info_t>>* get_players();

//
// Basic server data.  These properties, if set, must be set before before calling LogOn.  They
// may not be changed after logged in.
//

    /// This is called by SteamGameServer_Init, and you will usually not need to call it directly
    bool InitGameServer( uint32 unIP, uint16 usGamePort, uint16 usQueryPort, uint32 unFlags, AppId_t nGameAppId, const char *pchVersionString );

    /// Game product identifier.  This is currently used by the master server for version checking purposes.
    /// It's a required field, but will eventually will go away, and the AppID will be used for this purpose.
    void SetProduct( const char *pszProduct );

    /// Description of the game.  This is a required field and is displayed in the steam server browser....for now.
    /// This is a required field, but it will go away eventually, as the data should be determined from the AppID.
    void SetGameDescription( const char *pszGameDescription );

    /// If your game is a "mod," pass the string that identifies it.  The default is an empty string, meaning
    /// this application is the original game, not a mod.
    ///
    /// @see k_cbMaxGameServerGameDir
    void SetModDir( const char *pszModDir );

    /// Is this is a dedicated server?  The default value is false.
    void SetDedicatedServer( bool bDedicated );

//
// Login
//

    /// Begin process to login to a persistent game server account
    ///
    /// You need to register for callbacks to determine the result of this operation.
    /// @see SteamServersConnected_t
    /// @see SteamServerConnectFailure_t
    /// @see SteamServersDisconnected_t
    void LogOn( const char *pszToken );
    void LogOn(
        const char *pszAccountName,
        const char *pszPassword
    );
    void LogOn();

    /// Login to a generic, anonymous account.
    ///
    /// Note: in previous versions of the SDK, this was automatically called within SteamGameServer_Init,
    /// but this is no longer the case.
    void LogOnAnonymous();

    /// Begin process of logging game server out of steam
    void LogOff();
    
    // status functions
    bool BLoggedOn();
    bool BSecure(); 
    CSteamID GetSteamID();

    /// Returns true if the master server has requested a restart.
    /// Only returns true once per request.
    bool WasRestartRequested();

//
// Server state.  These properties may be changed at any time.
//

    /// Max player count that will be reported to server browser and client queries
    void SetMaxPlayerCount( int cPlayersMax );

    /// Number of bots.  Default value is zero
    void SetBotPlayerCount( int cBotplayers );

    /// Set the name of server as it will appear in the server browser
    ///
    /// @see k_cbMaxGameServerName
    void SetServerName( const char *pszServerName );

    /// Set name of map to report in the server browser
    ///
    /// @see k_cbMaxGameServerName
    void SetMapName( const char *pszMapName );

    /// Let people know if your server will require a password
    void SetPasswordProtected( bool bPasswordProtected );

    /// Spectator server.  The default value is zero, meaning the service
    /// is not used.
    void SetSpectatorPort( uint16 unSpectatorPort );

    /// Name of the spectator server.  (Only used if spectator port is nonzero.)
    ///
    /// @see k_cbMaxGameServerMapName
    void SetSpectatorServerName( const char *pszSpectatorServerName );

    /// Call this to clear the whole list of key/values that are sent in rules queries.
    void ClearAllKeyValues();
    
    /// Call this to add/update a key/value pair.
    void SetKeyValue( const char *pKey, const char *pValue );

    /// Sets a string defining the "gametags" for this server, this is optional, but if it is set
    /// it allows users to filter in the matchmaking/server-browser interfaces based on the value
    ///
    /// @see k_cbMaxGameServerTags
    void SetGameTags( const char *pchGameTags );

    /// Sets a string defining the "gamedata" for this server, this is optional, but if it is set
    /// it allows users to filter in the matchmaking/server-browser interfaces based on the value
    /// don't set this unless it actually changes, its only uploaded to the master once (when
    /// acknowledged)
    ///
    /// @see k_cbMaxGameServerGameData
    void SetGameData( const char *pchGameData );

    /// Region identifier.  This is an optional field, the default value is empty, meaning the "world" region
    void SetRegion( const char *pszRegion );

//
// Player list management / authentication
//

    // Handles receiving a new connection from a Steam user.  This call will ask the Steam
    // servers to validate the users identity, app ownership, and VAC status.  If the Steam servers 
    // are off-line, then it will validate the cached ticket itself which will validate app ownership 
    // and identity.  The AuthBlob here should be acquired on the game client using SteamUser()->InitiateGameConnection()
    // and must then be sent up to the game server for authentication.
    //
    // Return Value: returns true if the users ticket passes basic checks. pSteamIDUser will contain the Steam ID of this user. pSteamIDUser must NOT be NULL
    // If the call succeeds then you should expect a GSClientApprove_t or GSClientDeny_t callback which will tell you whether authentication
    // for the user has succeeded or failed (the steamid in the callback will match the one returned by this call)
    bool SendUserConnectAndAuthenticate( uint32 unIPClient, const void *pvAuthBlob, uint32 cubAuthBlobSize, CSteamID *pSteamIDUser );
    void SendUserConnectAndAuthenticate( CSteamID steamIDUser, uint32 unIPClient, void *pvAuthBlob, uint32 cubAuthBlobSize );

    // Creates a fake user (ie, a bot) which will be listed as playing on the server, but skips validation.  
    // 
    // Return Value: Returns a SteamID for the user to be tracked with, you should call HandleUserDisconnect()
    // when this user leaves the server just like you would for a real user.
    CSteamID CreateUnauthenticatedUserConnection();

    // Should be called whenever a user leaves our game server, this lets Steam internally
    // track which users are currently on which servers for the purposes of preventing a single
    // account being logged into multiple servers, showing who is currently on a server, etc.
    void SendUserDisconnect( CSteamID steamIDUser );

    // Update the data to be displayed in the server browser and matchmaking interfaces for a user
    // currently connected to the server.  For regular users you must call this after you receive a
    // GSUserValidationSuccess callback.
    // 
    // Return Value: true if successful, false if failure (ie, steamIDUser wasn't for an active player)
    bool BUpdateUserData( CSteamID steamIDUser, const char *pchPlayerName, uint32 uScore );

    // You shouldn't need to call this as it is called internally by SteamGameServer_Init() and can only be called once.
    //
    // To update the data in this call which may change during the servers lifetime see UpdateServerStatus() below.
    //
    // Input:    nGameAppID - The Steam assigned AppID for the game
    //            unServerFlags - Any applicable combination of flags (see k_unServerFlag____ constants below)
    //            unGameIP - The IP Address the server is listening for client connections on (might be INADDR_ANY)
    //            unGamePort - The port which the server is listening for client connections on
    //            unSpectatorPort - the port on which spectators can join to observe the server, 0 if spectating is not supported
    //            usQueryPort - The port which the ISteamMasterServerUpdater API should use in order to listen for matchmaking requests
    //            pchGameDir - A unique string identifier for your game
    //            pchVersion - The current version of the server as a string like 1.0.0.0
    //            bLanMode - Is this a LAN only server?
    //            
    // bugbug jmccaskey - figure out how to remove this from the API and only expose via SteamGameServer_Init... or make this actually used,
    // and stop calling it in SteamGameServer_Init()?
    bool BSetServerType( uint32 unServerFlags, uint32 unGameIP, uint16 unGamePort, 
                                uint16 unSpectatorPort, uint16 usQueryPort, const char *pchGameDir, const char *pchVersion, bool bLANMode );

    bool BSetServerType( int32 nGameAppId, uint32 unServerFlags, uint32 unGameIP, uint16 unGamePort, 
                                    uint16 unSpectatorPort, uint16 usQueryPort, const char *pchGameDir, const char *pchVersion, bool bLANMode );

    // Updates server status values which shows up in the server browser and matchmaking APIs
    void UpdateServerStatus( int cPlayers, int cPlayersMax, int cBotPlayers, 
                                     const char *pchServerName, const char *pSpectatorServerName, 
                                     const char *pchMapName );

    // This can be called if spectator goes away or comes back (passing 0 means there is no spectator server now).
    void UpdateSpectatorPort( uint16 unSpectatorPort );

    // Sets a string defining the "gametype" for this server, this is optional, but if it is set
    // it allows users to filter in the matchmaking/server-browser interfaces based on the value
    void SetGameType( const char *pchGameType ); 

    // Ask if a user has a specific achievement for this game, will get a callback on reply
    bool BGetUserAchievementStatus( CSteamID steamID, const char *pchAchievementName );

    // New auth system APIs - do not mix with the old auth system APIs.
    // ----------------------------------------------------------------

    // Retrieve ticket to be sent to the entity who wishes to authenticate you ( using BeginAuthSession API ). 
    // pcbTicket retrieves the length of the actual ticket.
    HAuthTicket GetAuthSessionTicket( void *pTicket, int cbMaxTicket, uint32 *pcbTicket );
    // SteamNetworkingIdentity is an optional parameter to hold the public IP address of the entity you are connecting to
    // if an IP address is passed Steam will only allow the ticket to be used by an entity with that IP address
    HAuthTicket GetAuthSessionTicket( void *pTicket, int cbMaxTicket, uint32 *pcbTicket, const SteamNetworkingIdentity *pSnid );

    // Authenticate ticket ( from GetAuthSessionTicket ) from entity steamID to be sure it is valid and isnt reused
    // Registers for callbacks if the entity goes offline or cancels the ticket ( see ValidateAuthTicketResponse_t callback and EAuthSessionResponse )
    EBeginAuthSessionResult BeginAuthSession( const void *pAuthTicket, int cbAuthTicket, CSteamID steamID );

    // Stop tracking started by BeginAuthSession - called when no longer playing game with this entity
    void EndAuthSession( CSteamID steamID );

    // Cancel auth ticket from GetAuthSessionTicket, called when no longer playing game with the entity you gave the ticket to
    void CancelAuthTicket( HAuthTicket hAuthTicket );

    // After receiving a user's authentication data, and passing it to SendUserConnectAndAuthenticate, use this function
    // to determine if the user owns downloadable content specified by the provided AppID.
    EUserHasLicenseForAppResult UserHasLicenseForApp( CSteamID steamID, AppId_t appID );

    // Ask if a user in in the specified group, results returns async by GSUserGroupStatus_t
    // returns false if we're not connected to the steam servers and thus cannot ask
    bool RequestUserGroupStatus( CSteamID steamIDUser, CSteamID steamIDGroup );


    // these two functions s are deprecated, and will not return results
    // they will be removed in a future version of the SDK
    void GetGameplayStats( );
    STEAM_CALL_RESULT( GSReputation_t )
    SteamAPICall_t GetServerReputation();

    // Returns the public IP of the server according to Steam, useful when the server is 
    // behind NAT and you want to advertise its IP in a lobby for other clients to directly
    // connect to
    uint32 GetPublicIP_old();
    SteamIPAddress_t GetPublicIP();
    void GetPublicIP_fix(SteamIPAddress_t *out);

// These are in GameSocketShare mode, where instead of ISteamGameServer creating its own
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

//
// Control heartbeats / advertisement with master server
//

    // Call this as often as you like to tell the master server updater whether or not
    // you want it to be active (default: off).
    void EnableHeartbeats( bool bActive );

    /// Indicate whether you wish to be listed on the master server list
    /// and/or respond to server browser / LAN discovery packets.
    /// The server starts with this value set to false.  You should set all
    /// relevant server parameters before enabling advertisement on the server.
    ///
    /// (This function used to be named EnableHeartbeats, so if you are wondering
    /// where that function went, it's right here.  It does the same thing as before,
    /// the old name was just confusing.)
    void SetAdvertiseServerActive( bool bActive );

    // You usually don't need to modify this.
    // Pass -1 to use the default value for iHeartbeatInterval.
    // Some mods change this.
    void SetHeartbeatInterval( int iHeartbeatInterval );

    // Force a heartbeat to steam at the next opportunity
    void ForceHeartbeat();

    void SetMasterServerHeartbeatInterval_DEPRECATED( int iHeartbeatInterval );
    void ForceMasterServerHeartbeat_DEPRECATED();

    // associate this game server with this clan for the purposes of computing player compat
    STEAM_CALL_RESULT( AssociateWithClanResult_t )
    SteamAPICall_t AssociateWithClan( CSteamID steamIDClan );
    
    // ask if any of the current players dont want to play with this new player - or vice versa
    STEAM_CALL_RESULT( ComputeNewPlayerCompatibilityResult_t )
    SteamAPICall_t ComputeNewPlayerCompatibility( CSteamID steamIDNewPlayer );


    // called by steam_client::runcallbacks
    void RunCallbacks();


    // older sdk -----------------------------------------------
    void GSSetSpawnCount( uint32 ucSpawn );
    bool GSGetSteam2GetEncryptionKeyToSendToNewClient( void *pvEncryptionKey, uint32 *pcbEncryptionKey, uint32 cbMaxEncryptionKey );
    bool GSSendSteam2UserConnect(  uint32 unUserID, const void *pvRawKey, uint32 unKeyLen, uint32 unIPPublic, uint16 usPort, const void *pvCookie, uint32 cubCookie );
    bool GSSendSteam3UserConnect( CSteamID steamID, uint32 unIPPublic, const void *pvCookie, uint32 cubCookie );
    bool GSRemoveUserConnect( uint32 unUserID );
    bool GSSendUserDisconnect( CSteamID steamID, uint32 unUserID );
    bool GSSendUserStatusResponse( CSteamID steamID, int nSecondsConnected, int nSecondsSinceLast );
    bool Obsolete_GSSetStatus( int32 nAppIdServed, uint32 unServerFlags, int cPlayers, int cPlayersMax, int cBotPlayers, int unGamePort, const char *pchServerName, const char *pchGameDir, const char *pchMapName, const char *pchVersion );
    bool GSUpdateStatus( int cPlayers, int cPlayersMax, int cBotPlayers, const char *pchServerName, const char *pchMapName );
    bool GSSetServerType( int32 nGameAppId, uint32 unServerFlags, uint32 unGameIP, uint32 unGamePort, const char *pchGameDir, const char *pchVersion );
    bool GSSetServerType2( int32 nGameAppId, uint32 unServerFlags, uint32 unGameIP, uint16 unGamePort, uint16 unSpectatorPort, uint16 usQueryPort, const char *pchGameDir, const char *pchVersion, bool bLANMode );
    bool GSUpdateStatus2( int cPlayers, int cPlayersMax, int cBotPlayers, const char *pchServerName, const char *pSpectatorServerName, const char *pchMapName );
    bool GSCreateUnauthenticatedUser( CSteamID *pSteamID );
    bool GSSetUserData( CSteamID steamID, const char *pPlayerName, uint32 nFrags );
    void GSUpdateSpectatorPort( uint16 unSpectatorPort );
    void GSSetGameType( const char *pchType );

    bool GSSendUserConnect( uint32 unUserID, uint32 unIPPublic, uint16 usPort, const void *pvCookie, uint32 cubCookie );
    bool GSSetServerType( int32 nGameAppId, uint32 unServerFlags, uint32 unGameIP, uint16 unGamePort, uint16 unSpectatorPort, uint16 usQueryPort, const char *pchGameDir, const char *pchVersion, bool bLANMode );
    bool GSUpdateStatus( int cPlayers, int cPlayersMax, int cBotPlayers, const char *pchServerName, const char *pSpectatorServerName, const char *pchMapName );
    bool GSGetUserAchievementStatus( CSteamID steamID, const char *pchAchievementName );
    // older sdk -----------------------------------------------

};

#endif // __INCLUDED_STEAM_GAMESERVER_H__
