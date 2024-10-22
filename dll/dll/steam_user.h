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

#ifndef __INCLUDED_STEAM_USER_H__
#define __INCLUDED_STEAM_USER_H__

#include "base.h"
#include "auth.h"

class Steam_User :
public ISteamUser004,
public ISteamUser005,
public ISteamUser006,
public ISteamUser007,
public ISteamUser008,
public ISteamUser009,
public ISteamUser010,
public ISteamUser011,
public ISteamUser012,
public ISteamUser013,
public ISteamUser014,
public ISteamUser015,
public ISteamUser016,
public ISteamUser017,
public ISteamUser018,
public ISteamUser019,
public ISteamUser020,
public ISteamUser021,
public ISteamUser022,
public ISteamUser
{
    Settings *settings{};
	class Networking *network{};
    class SteamCallBacks *callbacks{};
	class SteamCallResults *callback_results{};
    Local_Storage *local_storage{};

	bool recording = false;
	std::chrono::high_resolution_clock::time_point last_get_voice{};
	std::string encrypted_app_ticket{};
	Auth_Manager *auth_manager{};
    std::map<std::string, std::string> registry{};
    std::string registry_nullptr{};

public:
    Steam_User(Settings *settings, Local_Storage *local_storage, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks);
    ~Steam_User();

    // returns the HSteamUser this interface represents
    // this is only used internally by the API, and by a few select interfaces that support multi-user
    HSteamUser GetHSteamUser();

    void LogOn( CSteamID steamID );
    void LogOff();

    // returns true if the Steam client current has a live connection to the Steam servers. 
    // If false, it means there is no active connection due to either a networking issue on the local machine, or the Steam server is down/busy.
    // The Steam client will automatically be trying to recreate the connection as often as possible.
    bool BLoggedOn();

    ELogonState GetLogonState();
	bool BConnected();

    // returns the CSteamID of the account currently logged into the Steam client
    // a CSteamID is a unique identifier for an account, and used to differentiate users in all parts of the Steamworks API
    CSteamID GetSteamID();

	bool IsVACBanned( int nGameID );
	bool RequireShowVACBannedMessage( int nGameID );
	void AcknowledgeVACBanning( int nGameID );

	// These are dead.
	int NClientGameIDAdd( int nGameID );
	void RemoveClientGame( int nClientGameID );
	void SetClientGameServer( int nClientGameID, uint32 unIPServer, uint16 usPortServer );

	void SetSteam2Ticket( uint8 *pubTicket, int cubTicket );
	void AddServerNetAddress( uint32 unIP, uint16 unPort );
	bool SetEmail( const char *pchEmail );

	// logon cookie - this is obsolete and never used
	int GetSteamGameConnectToken( void *pBlob, int cbMaxBlob );
	bool SetRegistryString( EConfigSubTree eRegistrySubTree, const char *pchKey, const char *pchValue );
	bool GetRegistryString( EConfigSubTree eRegistrySubTree, const char *pchKey, char *pchValue, int cbValue );
	bool SetRegistryInt( EConfigSubTree eRegistrySubTree, const char *pchKey, int iValue );
	bool GetRegistryInt( EConfigSubTree eRegistrySubTree, const char *pchKey, int *piValue );

    // Multiplayer Authentication functions

    // InitiateGameConnection() starts the state machine for authenticating the game client with the game server
    // It is the client portion of a three-way handshake between the client, the game server, and the steam servers
    //
    // Parameters:
    // void *pAuthBlob - a pointer to empty memory that will be filled in with the authentication token.
    // int cbMaxAuthBlob - the number of bytes of allocated memory in pBlob. Should be at least 2048 bytes.
    // CSteamID steamIDGameServer - the steamID of the game server, received from the game server by the client
    // CGameID gameID - the ID of the current game. For games without mods, this is just CGameID( <appID> )
    // uint32 unIPServer, uint16 usPortServer - the IP address of the game server
    // bool bSecure - whether or not the client thinks that the game server is reporting itself as secure (i.e. VAC is running)
    //
    // return value - returns the number of bytes written to pBlob. If the return is 0, then the buffer passed in was too small, and the call has failed
    // The contents of pBlob should then be sent to the game server, for it to use to complete the authentication process.

    int InitiateGameConnection( void *pAuthBlob, int cbMaxAuthBlob, CSteamID steamIDGameServer, uint32 unIPServer, uint16 usPortServer, bool bSecure );

    int InitiateGameConnection( void *pAuthBlob, int cbMaxAuthBlob, CSteamID steamIDGameServer, CGameID gameID, uint32 unIPServer, uint16 usPortServer, bool bSecure );

    int InitiateGameConnection( void *pBlob, int cbMaxBlob, CSteamID steamID, CGameID gameID, uint32 unIPServer, uint16 usPortServer, bool bSecure, void *pvSteam2GetEncryptionKey, int cbSteam2GetEncryptionKey );

    int InitiateGameConnection( void *pBlob, int cbMaxBlob, CSteamID steamID, int nGameAppID, uint32 unIPServer, uint16 usPortServer, bool bSecure );

    // notify of disconnect
    // needs to occur when the game client leaves the specified game server, needs to match with the InitiateGameConnection() call
    void TerminateGameConnection( uint32 unIPServer, uint16 usPortServer );

    // Legacy functions

	void SetSelfAsPrimaryChatDestination();
	bool IsPrimaryChatDestination();
	void RequestLegacyCDKey( uint32 nAppID );
	bool SendGuestPassByEmail( const char *pchEmailAccount, GID_t gidGuestPassID, bool bResending );
	bool SendGuestPassByAccountID( uint32 uAccountID, GID_t gidGuestPassID, bool bResending );
	bool AckGuestPass(const char *pchGuestPassCode);
	bool RedeemGuestPass(const char *pchGuestPassCode);
	uint32 GetGuestPassToGiveCount();
	uint32 GetGuestPassToRedeemCount();
	RTime32 GetGuestPassLastUpdateTime();
	bool GetGuestPassToGiveInfo( uint32 nPassIndex, GID_t *pgidGuestPassID, PackageId_t *pnPackageID, RTime32 *pRTime32Created, RTime32 *pRTime32Expiration, RTime32 *pRTime32Sent, RTime32 *pRTime32Redeemed, char *pchRecipientAddress, int cRecipientAddressSize );
	bool GetGuestPassToRedeemInfo( uint32 nPassIndex, GID_t *pgidGuestPassID, PackageId_t *pnPackageID, RTime32 *pRTime32Created, RTime32 *pRTime32Expiration, RTime32 *pRTime32Sent, RTime32 *pRTime32Redeemed);
	bool GetGuestPassToRedeemSenderAddress( uint32 nPassIndex, char* pchSenderAddress, int cSenderAddressSize );
	bool GetGuestPassToRedeemSenderName( uint32 nPassIndex, char* pchSenderName, int cSenderNameSize );
	void AcknowledgeMessageByGID( const char *pchMessageGID );
	bool SetLanguage( const char *pchLanguage );

    // used by only a few games to track usage events
    void TrackAppUsageEvent( CGameID gameID, int eAppUsageEvent, const char *pchExtraInfo);

	void SetAccountName( const char *pchAccountName );
	void SetPassword( const char *pchPassword );
	void SetAccountCreationTime( RTime32 rt );

    void RefreshSteam2Login();

    // get the local storage folder for current Steam account to write application data, e.g. save games, configs etc.
    // this will usually be something like "C:\Progam Files\Steam\userdata\<SteamID>\<AppID>\local"
    bool GetUserDataFolder( char *pchBuffer, int cubBuffer );

    // Starts voice recording. Once started, use GetVoice() to get the data
    void StartVoiceRecording( );

    // Stops voice recording. Because people often release push-to-talk keys early, the system will keep recording for
    // a little bit after this function is called. GetVoice() should continue to be called until it returns
    // k_eVoiceResultNotRecording
    void StopVoiceRecording( );

    // Determine the size of captured audio data that is available from GetVoice.
    // Most applications will only use compressed data and should ignore the other
    // parameters, which exist primarily for backwards compatibility. See comments
    // below for further explanation of "uncompressed" data.
    EVoiceResult GetAvailableVoice( uint32 *pcbCompressed, uint32 *pcbUncompressed_Deprecated, uint32 nUncompressedVoiceDesiredSampleRate_Deprecated  );

    EVoiceResult GetAvailableVoice(uint32 *pcbCompressed, uint32 *pcbUncompressed);

    // ---------------------------------------------------------------------------
    // NOTE: "uncompressed" audio is a deprecated feature and should not be used
    // by most applications. It is raw single-channel 16-bit PCM wave data which
    // may have been run through preprocessing filters and/or had silence removed,
    // so the uncompressed audio could have a shorter duration than you expect.
    // There may be no data at all during long periods of silence. Also, fetching
    // uncompressed audio will cause GetVoice to discard any leftover compressed
    // audio, so you must fetch both types at once. Finally, GetAvailableVoice is
    // not precisely accurate when the uncompressed size is requested. So if you
    // really need to use uncompressed audio, you should call GetVoice frequently
    // with two very large (20kb+) output buffers instead of trying to allocate
    // perfectly-sized buffers. But most applications should ignore all of these
    // details and simply leave the "uncompressed" parameters as NULL/zero.
    // ---------------------------------------------------------------------------

    // Read captured audio data from the microphone buffer. This should be called
    // at least once per frame, and preferably every few milliseconds, to keep the
    // microphone input delay as low as possible. Most applications will only use
    // compressed data and should pass NULL/zero for the "uncompressed" parameters.
    // Compressed data can be transmitted by your application and decoded into raw
    // using the DecompressVoice function below.
    EVoiceResult GetVoice( bool bWantCompressed, void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten, bool bWantUncompressed_Deprecated, void *pUncompressedDestBuffer_Deprecated , uint32 cbUncompressedDestBufferSize_Deprecated , uint32 *nUncompressBytesWritten_Deprecated , uint32 nUncompressedVoiceDesiredSampleRate_Deprecated  );

    EVoiceResult GetVoice( bool bWantCompressed, void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten, bool bWantUncompressed, void *pUncompressedDestBuffer, uint32 cbUncompressedDestBufferSize, uint32 *nUncompressBytesWritten );

    EVoiceResult GetCompressedVoice( void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten );

    // Decodes the compressed voice data returned by GetVoice. The output data is
    // raw single-channel 16-bit PCM audio. The decoder supports any sample rate
    // from 11025 to 48000; see GetVoiceOptimalSampleRate() below for details.
    // If the output buffer is not large enough, then *nBytesWritten will be set
    // to the required buffer size, and k_EVoiceResultBufferTooSmall is returned.
    // It is suggested to start with a 20kb buffer and reallocate as necessary.
    EVoiceResult DecompressVoice( const void *pCompressed, uint32 cbCompressed, void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten, uint32 nDesiredSampleRate );

    EVoiceResult DecompressVoice( const void *pCompressed, uint32 cbCompressed, void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten );

    EVoiceResult DecompressVoice( void *pCompressed, uint32 cbCompressed, void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten );

    // This returns the native sample rate of the Steam voice decompressor
    // this sample rate for DecompressVoice will perform the least CPU processing.
    // However, the final audio quality will depend on how well the audio device
    // (and/or your application's audio output SDK) deals with lower sample rates.
    // You may find that you get the best audio output quality when you ignore
    // this function and use the native sample rate of your audio output device,
    // which is usually 48000 or 44100.
    uint32 GetVoiceOptimalSampleRate();

    // Retrieve ticket to be sent to the entity who wishes to authenticate you. 
    // pcbTicket retrieves the length of the actual ticket.
    HAuthTicket GetAuthSessionTicket( void *pTicket, int cbMaxTicket, uint32 *pcbTicket );
    // SteamNetworkingIdentity is an optional input parameter to hold the public IP address or SteamID of the entity you are connecting to
    // if an IP address is passed Steam will only allow the ticket to be used by an entity with that IP address
    // if a Steam ID is passed Steam will only allow the ticket to be used by that Steam ID
    HAuthTicket GetAuthSessionTicket( void *pTicket, int cbMaxTicket, uint32 *pcbTicket, const SteamNetworkingIdentity *pSteamNetworkingIdentity );

    // Request a ticket which will be used for webapi "ISteamUserAuth\AuthenticateUserTicket"
    // pchIdentity is an optional input parameter to identify the service the ticket will be sent to
    // the ticket will be returned in callback GetTicketForWebApiResponse_t
    HAuthTicket GetAuthTicketForWebApi( const char *pchIdentity );

    // Authenticate ticket from entity steamID to be sure it is valid and isnt reused
    // Registers for callbacks if the entity goes offline or cancels the ticket ( see ValidateAuthTicketResponse_t callback and EAuthSessionResponse )
    EBeginAuthSessionResult BeginAuthSession( const void *pAuthTicket, int cbAuthTicket, CSteamID steamID );

    // Stop tracking started by BeginAuthSession - called when no longer playing game with this entity
    void EndAuthSession( CSteamID steamID );

    // Cancel auth ticket from GetAuthSessionTicket, called when no longer playing game with the entity you gave the ticket to
    void CancelAuthTicket( HAuthTicket hAuthTicket );

    // After receiving a user's authentication data, and passing it to BeginAuthSession, use this function
    // to determine if the user owns downloadable content specified by the provided AppID.
    EUserHasLicenseForAppResult UserHasLicenseForApp( CSteamID steamID, AppId_t appID );

    // returns true if this users looks like they are behind a NAT device. Only valid once the user has connected to steam 
    // (i.e a SteamServersConnected_t has been issued) and may not catch all forms of NAT.
    bool BIsBehindNAT();

    // set data to be replicated to friends so that they can join your game
    // CSteamID steamIDGameServer - the steamID of the game server, received from the game server by the client
    // uint32 unIPServer, uint16 usPortServer - the IP address of the game server
    void AdvertiseGame( CSteamID steamIDGameServer, uint32 unIPServer, uint16 usPortServer );

    // Requests a ticket encrypted with an app specific shared key
    // pDataToInclude, cbDataToInclude will be encrypted into the ticket
    // ( This is asynchronous, you must wait for the ticket to be completed by the server )
    STEAM_CALL_RESULT( EncryptedAppTicketResponse_t )
    SteamAPICall_t RequestEncryptedAppTicket( void *pDataToInclude, int cbDataToInclude );

    // retrieve a finished ticket
    bool GetEncryptedAppTicket( void *pTicket, int cbMaxTicket, uint32 *pcbTicket );

    // Trading Card badges data access
    // if you only have one set of cards, the series will be 1
    // the user has can have two different badges for a series; the regular (max level 5) and the foil (max level 1)
    int GetGameBadgeLevel( int nSeries, bool bFoil );

    // gets the Steam Level of the user, as shown on their profile
    int GetPlayerSteamLevel();

    // Requests a URL which authenticates an in-game browser for store check-out,
    // and then redirects to the specified URL. As long as the in-game browser
    // accepts and handles session cookies, Steam microtransaction checkout pages
    // will automatically recognize the user instead of presenting a login page.
    // The result of this API call will be a StoreAuthURLResponse_t callback.
    // NOTE: The URL has a very short lifetime to prevent history-snooping attacks,
    // so you should only call this API when you are about to launch the browser,
    // or else immediately navigate to the result URL using a hidden browser window.
    // NOTE 2: The resulting authorization cookie has an expiration time of one day,
    // so it would be a good idea to request and visit a new auth URL every 12 hours.
    STEAM_CALL_RESULT( StoreAuthURLResponse_t )
    SteamAPICall_t RequestStoreAuthURL( const char *pchRedirectURL );

    // gets whether the users phone number is verified 
    bool BIsPhoneVerified();

    // gets whether the user has two factor enabled on their account
    bool BIsTwoFactorEnabled();

    // gets whether the users phone number is identifying
    bool BIsPhoneIdentifying();

    // gets whether the users phone number is awaiting (re)verification
    bool BIsPhoneRequiringVerification();

    STEAM_CALL_RESULT( MarketEligibilityResponse_t )
    SteamAPICall_t GetMarketEligibility();

    // Retrieves anti indulgence / duration control for current user
    STEAM_CALL_RESULT( DurationControl_t )
    SteamAPICall_t GetDurationControl();

    // Advise steam china duration control system about the online state of the game.
    // This will prevent offline gameplay time from counting against a user's
    // playtime limits.
    bool BSetDurationControlOnlineState( EDurationControlOnlineState eNewState );

};

#endif // __INCLUDED_STEAM_USER_H__
