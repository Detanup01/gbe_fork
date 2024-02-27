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
#include "auth.h"
#include "appticket.h"

class Steam_User :
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
public ISteamUser
{
    Settings *settings;
	class Networking *network;
    class SteamCallBacks *callbacks;
	class SteamCallResults *callback_results;
    Local_Storage *local_storage;

	bool recording = false;
	std::chrono::high_resolution_clock::time_point last_get_voice;
	std::string encrypted_app_ticket;
	Auth_Manager *auth_manager;

public:

Steam_User(Settings *settings, Local_Storage *local_storage, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks)
{
    this->settings = settings;
    this->local_storage = local_storage;
    this->network = network;
    this->callbacks = callbacks;
    this->callback_results = callback_results;
    recording = false;
    auth_manager = new Auth_Manager(settings, network, callbacks);
}

~Steam_User()
{
    delete auth_manager;
}

// returns the HSteamUser this interface represents
// this is only used internally by the API, and by a few select interfaces that support multi-user
HSteamUser GetHSteamUser()
{
    PRINT_DEBUG("Steam_User::GetHSteamUser\n");
    return CLIENT_HSTEAMUSER;
}

// returns true if the Steam client current has a live connection to the Steam servers. 
// If false, it means there is no active connection due to either a networking issue on the local machine, or the Steam server is down/busy.
// The Steam client will automatically be trying to recreate the connection as often as possible.
bool BLoggedOn()
{
    PRINT_DEBUG("Steam_User::BLoggedOn\n");
    return !settings->is_offline();
}

// returns the CSteamID of the account currently logged into the Steam client
// a CSteamID is a unique identifier for an account, and used to differentiate users in all parts of the Steamworks API
CSteamID GetSteamID()
{
    PRINT_DEBUG("Steam_User::GetSteamID\n");
    CSteamID id = settings->get_local_steam_id();
    
    return id;
}

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

//steam returns 206 bytes
#define INITIATE_GAME_CONNECTION_TICKET_SIZE 206

int InitiateGameConnection( void *pAuthBlob, int cbMaxAuthBlob, CSteamID steamIDGameServer, uint32 unIPServer, uint16 usPortServer, bool bSecure )
{
    PRINT_DEBUG("Steam_User::InitiateGameConnection %i %llu %u %u %u %p\n", cbMaxAuthBlob, steamIDGameServer.ConvertToUint64(), unIPServer, usPortServer, bSecure, pAuthBlob);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (cbMaxAuthBlob < INITIATE_GAME_CONNECTION_TICKET_SIZE) return 0;
    if (!pAuthBlob) return 0;
    uint32 out_size = INITIATE_GAME_CONNECTION_TICKET_SIZE;
    auth_manager->getTicketData(pAuthBlob, INITIATE_GAME_CONNECTION_TICKET_SIZE, &out_size);
    return out_size;
}

int InitiateGameConnection( void *pAuthBlob, int cbMaxAuthBlob, CSteamID steamIDGameServer, CGameID gameID, uint32 unIPServer, uint16 usPortServer, bool bSecure )
{
	PRINT_DEBUG("InitiateGameConnection old\n");
	return InitiateGameConnection(pAuthBlob, cbMaxAuthBlob, steamIDGameServer, unIPServer, usPortServer, bSecure);
}

// notify of disconnect
// needs to occur when the game client leaves the specified game server, needs to match with the InitiateGameConnection() call
void TerminateGameConnection( uint32 unIPServer, uint16 usPortServer )
{
    PRINT_DEBUG("Steam_User::TerminateGameConnection\n");
}

// Legacy functions

// used by only a few games to track usage events
void TrackAppUsageEvent( CGameID gameID, int eAppUsageEvent, const char *pchExtraInfo)
{
    PRINT_DEBUG("Steam_User::TrackAppUsageEvent\n");
}

void RefreshSteam2Login()
{
	PRINT_DEBUG("RefreshSteam2Login\n");
}

// get the local storage folder for current Steam account to write application data, e.g. save games, configs etc.
// this will usually be something like "C:\Progam Files\Steam\userdata\<SteamID>\<AppID>\local"
bool GetUserDataFolder( char *pchBuffer, int cubBuffer )
{
    PRINT_DEBUG("Steam_User::GetUserDataFolder\n");
    if (!cubBuffer) return false;

    std::string user_data = local_storage->get_path(Local_Storage::user_data_storage);
    strncpy(pchBuffer, user_data.c_str(), cubBuffer - 1);
    pchBuffer[cubBuffer - 1] = 0;
    return true;
}

// Starts voice recording. Once started, use GetVoice() to get the data
void StartVoiceRecording( )
{
    PRINT_DEBUG("Steam_User::StartVoiceRecording\n");
    last_get_voice = std::chrono::high_resolution_clock::now();
    recording = true;
    //TODO:fix
    recording = false;
}

// Stops voice recording. Because people often release push-to-talk keys early, the system will keep recording for
// a little bit after this function is called. GetVoice() should continue to be called until it returns
// k_eVoiceResultNotRecording
void StopVoiceRecording( )
{
    PRINT_DEBUG("Steam_User::StopVoiceRecording\n");
    recording = false;
}

// Determine the size of captured audio data that is available from GetVoice.
// Most applications will only use compressed data and should ignore the other
// parameters, which exist primarily for backwards compatibility. See comments
// below for further explanation of "uncompressed" data.
EVoiceResult GetAvailableVoice( uint32 *pcbCompressed, uint32 *pcbUncompressed_Deprecated, uint32 nUncompressedVoiceDesiredSampleRate_Deprecated  )
{
    PRINT_DEBUG("Steam_User::GetAvailableVoice\n");
    if (pcbCompressed) *pcbCompressed = 0;
    if (pcbUncompressed_Deprecated) *pcbUncompressed_Deprecated = 0;
    if (!recording) return k_EVoiceResultNotRecording;
    double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - last_get_voice).count();
    if (pcbCompressed) *pcbCompressed = seconds * 1024.0 * 64.0 / 8.0;
    if (pcbUncompressed_Deprecated) *pcbUncompressed_Deprecated = seconds * (double)nUncompressedVoiceDesiredSampleRate_Deprecated * 2.0;

    return k_EVoiceResultOK;
}

EVoiceResult GetAvailableVoice(uint32 *pcbCompressed, uint32 *pcbUncompressed)
{
	PRINT_DEBUG("GetAvailableVoice old\n");
	return GetAvailableVoice(pcbCompressed, pcbUncompressed, 11025);
}

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
EVoiceResult GetVoice( bool bWantCompressed, void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten, bool bWantUncompressed_Deprecated, void *pUncompressedDestBuffer_Deprecated , uint32 cbUncompressedDestBufferSize_Deprecated , uint32 *nUncompressBytesWritten_Deprecated , uint32 nUncompressedVoiceDesiredSampleRate_Deprecated  )
{
    PRINT_DEBUG("Steam_User::GetVoice\n");
    if (!recording) return k_EVoiceResultNotRecording;
    double seconds = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::high_resolution_clock::now() - last_get_voice).count();
    if (bWantCompressed) {
        uint32 towrite = seconds * 1024.0 * 64.0 / 8.0;
        if (cbDestBufferSize < towrite) towrite = cbDestBufferSize;
        if (pDestBuffer) memset(pDestBuffer, 0, towrite);
        if (nBytesWritten) *nBytesWritten = towrite;
    }

    if (bWantUncompressed_Deprecated) {
        PRINT_DEBUG("Steam_User::GetVoice Wanted Uncompressed\n");
    }

    last_get_voice = std::chrono::high_resolution_clock::now();
    return k_EVoiceResultOK;
}

EVoiceResult GetVoice( bool bWantCompressed, void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten, bool bWantUncompressed, void *pUncompressedDestBuffer, uint32 cbUncompressedDestBufferSize, uint32 *nUncompressBytesWritten )
{
	PRINT_DEBUG("GetVoice old\n");
	return GetVoice(bWantCompressed, pDestBuffer, cbDestBufferSize, nBytesWritten, bWantUncompressed, pUncompressedDestBuffer, cbUncompressedDestBufferSize, nUncompressBytesWritten, 11025);
}

EVoiceResult GetCompressedVoice( void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten )
{
	PRINT_DEBUG("GetCompressedVoice\n");
	return GetVoice(true, pDestBuffer, cbDestBufferSize, nBytesWritten, false, NULL, 0, NULL);
}

// Decodes the compressed voice data returned by GetVoice. The output data is
// raw single-channel 16-bit PCM audio. The decoder supports any sample rate
// from 11025 to 48000; see GetVoiceOptimalSampleRate() below for details.
// If the output buffer is not large enough, then *nBytesWritten will be set
// to the required buffer size, and k_EVoiceResultBufferTooSmall is returned.
// It is suggested to start with a 20kb buffer and reallocate as necessary.
EVoiceResult DecompressVoice( const void *pCompressed, uint32 cbCompressed, void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten, uint32 nDesiredSampleRate )
{
    PRINT_DEBUG("Steam_User::DecompressVoice\n");
    if (!recording) return k_EVoiceResultNotRecording;
    uint32 uncompressed = (double)cbCompressed * ((double)nDesiredSampleRate / 8192.0);
    if(nBytesWritten) *nBytesWritten = uncompressed;
    if (uncompressed > cbDestBufferSize) uncompressed = cbDestBufferSize;
    if (pDestBuffer) memset(pDestBuffer, 0, uncompressed);

    return k_EVoiceResultOK;
}

EVoiceResult DecompressVoice( const void *pCompressed, uint32 cbCompressed, void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten )
{
	PRINT_DEBUG("DecompressVoice old\n");
	return DecompressVoice(pCompressed, cbCompressed, pDestBuffer, cbDestBufferSize, nBytesWritten, 11025);
}

EVoiceResult DecompressVoice( void *pCompressed, uint32 cbCompressed, void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten )
{
	PRINT_DEBUG("DecompressVoice older\n");
	return DecompressVoice(pCompressed, cbCompressed, pDestBuffer, cbDestBufferSize, nBytesWritten, 11025);
}

// This returns the native sample rate of the Steam voice decompressor
// this sample rate for DecompressVoice will perform the least CPU processing.
// However, the final audio quality will depend on how well the audio device
// (and/or your application's audio output SDK) deals with lower sample rates.
// You may find that you get the best audio output quality when you ignore
// this function and use the native sample rate of your audio output device,
// which is usually 48000 or 44100.
uint32 GetVoiceOptimalSampleRate()
{
    PRINT_DEBUG("Steam_User::GetVoiceOptimalSampleRate\n");
    return 48000;
}

// Retrieve ticket to be sent to the entity who wishes to authenticate you. 
// pcbTicket retrieves the length of the actual ticket.
HAuthTicket GetAuthSessionTicket( void *pTicket, int cbMaxTicket, uint32 *pcbTicket )
{
    return GetAuthSessionTicket(pTicket, cbMaxTicket, pcbTicket, NULL);
}
// SteamNetworkingIdentity is an optional input parameter to hold the public IP address or SteamID of the entity you are connecting to
// if an IP address is passed Steam will only allow the ticket to be used by an entity with that IP address
// if a Steam ID is passed Steam will only allow the ticket to be used by that Steam ID
HAuthTicket GetAuthSessionTicket( void *pTicket, int cbMaxTicket, uint32 *pcbTicket, const SteamNetworkingIdentity *pSteamNetworkingIdentity )
{
    PRINT_DEBUG("Steam_User::GetAuthSessionTicket %i\n", cbMaxTicket);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pTicket) return k_HAuthTicketInvalid;
    
    return auth_manager->getTicket(pTicket, cbMaxTicket, pcbTicket);
}

// Request a ticket which will be used for webapi "ISteamUserAuth\AuthenticateUserTicket"
// pchIdentity is an optional input parameter to identify the service the ticket will be sent to
// the ticket will be returned in callback GetTicketForWebApiResponse_t
HAuthTicket GetAuthTicketForWebApi( const char *pchIdentity )
{
    PRINT_DEBUG("Steam_User::GetAuthTicketForWebApi %s\n", pchIdentity);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    return auth_manager->getWebApiTicket(pchIdentity);
}

// Authenticate ticket from entity steamID to be sure it is valid and isnt reused
// Registers for callbacks if the entity goes offline or cancels the ticket ( see ValidateAuthTicketResponse_t callback and EAuthSessionResponse )
EBeginAuthSessionResult BeginAuthSession( const void *pAuthTicket, int cbAuthTicket, CSteamID steamID )
{
    PRINT_DEBUG("Steam_User::BeginAuthSession %i %llu\n", cbAuthTicket, steamID.ConvertToUint64());
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    return auth_manager->beginAuth(pAuthTicket, cbAuthTicket, steamID);
}

// Stop tracking started by BeginAuthSession - called when no longer playing game with this entity
void EndAuthSession( CSteamID steamID )
{
    PRINT_DEBUG("Steam_User::EndAuthSession\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    auth_manager->endAuth(steamID);
}

// Cancel auth ticket from GetAuthSessionTicket, called when no longer playing game with the entity you gave the ticket to
void CancelAuthTicket( HAuthTicket hAuthTicket )
{
    PRINT_DEBUG("Steam_User::CancelAuthTicket\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    auth_manager->cancelTicket(hAuthTicket);
}

// After receiving a user's authentication data, and passing it to BeginAuthSession, use this function
// to determine if the user owns downloadable content specified by the provided AppID.
EUserHasLicenseForAppResult UserHasLicenseForApp( CSteamID steamID, AppId_t appID )
{
    PRINT_DEBUG("Steam_User::UserHasLicenseForApp\n");
    return k_EUserHasLicenseResultHasLicense;
}

// returns true if this users looks like they are behind a NAT device. Only valid once the user has connected to steam 
// (i.e a SteamServersConnected_t has been issued) and may not catch all forms of NAT.
bool BIsBehindNAT()
{
    PRINT_DEBUG("Steam_User::BIsBehindNAT\n");
    return false;
}

// set data to be replicated to friends so that they can join your game
// CSteamID steamIDGameServer - the steamID of the game server, received from the game server by the client
// uint32 unIPServer, uint16 usPortServer - the IP address of the game server
void AdvertiseGame( CSteamID steamIDGameServer, uint32 unIPServer, uint16 usPortServer )
{
    PRINT_DEBUG("Steam_User::AdvertiseGame\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    Gameserver *server = new Gameserver();
    server->set_id(steamIDGameServer.ConvertToUint64());
    server->set_ip(unIPServer);
    server->set_port(usPortServer);
    server->set_query_port(usPortServer);
    server->set_appid(settings->get_local_game_id().ToUint64());
    server->set_type(eFriendsServer);
    Common_Message msg;
    msg.set_allocated_gameserver(server);
    msg.set_source_id(settings->get_local_steam_id().ConvertToUint64());
    network->sendToAllIndividuals(&msg, true);
}

// Requests a ticket encrypted with an app specific shared key
// pDataToInclude, cbDataToInclude will be encrypted into the ticket
// ( This is asynchronous, you must wait for the ticket to be completed by the server )
STEAM_CALL_RESULT( EncryptedAppTicketResponse_t )
SteamAPICall_t RequestEncryptedAppTicket( void *pDataToInclude, int cbDataToInclude )
{
    PRINT_DEBUG("Steam_User::RequestEncryptedAppTicket %i\n", cbDataToInclude);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    EncryptedAppTicketResponse_t data;
	data.m_eResult = k_EResultOK;

    DecryptedAppTicket ticket;
    ticket.TicketV1.Reset();
    ticket.TicketV2.Reset();
    ticket.TicketV4.Reset();

    ticket.TicketV1.TicketVersion = 1;
    if (pDataToInclude) {
        ticket.TicketV1.UserData.assign((uint8_t*)pDataToInclude, (uint8_t*)pDataToInclude + cbDataToInclude);
    }

    ticket.TicketV2.TicketVersion = 4;
    ticket.TicketV2.SteamID = settings->get_local_steam_id().ConvertToUint64();
    ticket.TicketV2.TicketIssueTime = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    ticket.TicketV2.TicketValidityEnd = ticket.TicketV2.TicketIssueTime + (21 * 24 * 60 * 60);

    for (int i = 0; i < 140; ++i)
    {
        AppId_t appid;
        bool available;
        std::string name;
        if (!settings->getDLC(appid, appid, available, name)) break;
        ticket.TicketV4.AppIDs.emplace_back(appid);
    }

    ticket.TicketV4.HasVACStatus = true;
    ticket.TicketV4.VACStatus = 0;

    auto serialized = ticket.SerializeTicket();

    SteamAppTicket_pb pb;
    pb.set_ticket_version_no(1);
    pb.set_crc_encryptedticket(0); // TODO: Find out how to compute the CRC
    pb.set_cb_encrypteduserdata(cbDataToInclude);
    pb.set_cb_encrypted_appownershipticket(serialized.size() - 16);
    pb.mutable_encrypted_ticket()->assign(serialized.begin(), serialized.end()); // TODO: Find how to encrypt datas

    encrypted_app_ticket = pb.SerializeAsString();

    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}

// retrieve a finished ticket
bool GetEncryptedAppTicket( void *pTicket, int cbMaxTicket, uint32 *pcbTicket )
{
    PRINT_DEBUG("Steam_User::GetEncryptedAppTicket %i\n", cbMaxTicket);
    unsigned int ticket_size = encrypted_app_ticket.size();
    if (!cbMaxTicket) {
        if (!pcbTicket) return false;
        *pcbTicket = ticket_size;
        return true;
    }

    if (!pTicket) return false;
    if (ticket_size > cbMaxTicket) return false;
    encrypted_app_ticket.copy((char *)pTicket, cbMaxTicket);
    if (pcbTicket) *pcbTicket = ticket_size;

    return true;
}

// Trading Card badges data access
// if you only have one set of cards, the series will be 1
// the user has can have two different badges for a series; the regular (max level 5) and the foil (max level 1)
int GetGameBadgeLevel( int nSeries, bool bFoil )
{
    PRINT_DEBUG("Steam_User::GetGameBadgeLevel\n");
    return 0;
}

// gets the Steam Level of the user, as shown on their profile
int GetPlayerSteamLevel()
{
    PRINT_DEBUG("Steam_User::GetPlayerSteamLevel\n");
    return 100;
}

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
SteamAPICall_t RequestStoreAuthURL( const char *pchRedirectURL )
{
    PRINT_DEBUG("Steam_User::RequestStoreAuthURL\n");
    return 0;
}

// gets whether the users phone number is verified 
bool BIsPhoneVerified()
{
    PRINT_DEBUG("Steam_User::BIsPhoneVerified\n");
    return true;
}

// gets whether the user has two factor enabled on their account
bool BIsTwoFactorEnabled()
{
    PRINT_DEBUG("Steam_User::BIsTwoFactorEnabled\n");
    return true;
}

// gets whether the users phone number is identifying
bool BIsPhoneIdentifying()
{
    PRINT_DEBUG("Steam_User::BIsPhoneIdentifying\n");
    return false;
}

// gets whether the users phone number is awaiting (re)verification
bool BIsPhoneRequiringVerification()
{
    PRINT_DEBUG("Steam_User::BIsPhoneRequiringVerification\n");
    return false;
}

STEAM_CALL_RESULT( MarketEligibilityResponse_t )
SteamAPICall_t GetMarketEligibility()
{
    PRINT_DEBUG("Steam_User::GetMarketEligibility\n");
    return 0;
}

// Retrieves anti indulgence / duration control for current user
STEAM_CALL_RESULT( DurationControl_t )
SteamAPICall_t GetDurationControl()
{
    PRINT_DEBUG("Steam_User::GetDurationControl\n");
    return 0;
}

// Advise steam china duration control system about the online state of the game.
// This will prevent offline gameplay time from counting against a user's
// playtime limits.
bool BSetDurationControlOnlineState( EDurationControlOnlineState eNewState )
{
    PRINT_DEBUG("Steam_User::BSetDurationControlOnlineState\n");
    return false;
}

};
