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

#include "steam_user.h"
#include "auth.h"
#include "appticket.h"
#include "base64.h"
#include "dll.h"

Steam_User::Steam_User(Settings *settings, Local_Storage *local_storage, class Networking *network, class SteamCallResults *callback_results, class SteamCallBacks *callbacks)
{
    this->settings = settings;
    this->local_storage = local_storage;
    this->network = network;
    this->callbacks = callbacks;
    this->callback_results = callback_results;
    
    auth_manager = new Auth_Manager(settings, network, callbacks);
    voicechat = new VoiceChat();
}

Steam_User::~Steam_User()
{
    delete auth_manager;
    delete voicechat;
}

// returns the HSteamUser this interface represents
// this is only used internally by the API, and by a few select interfaces that support multi-user
HSteamUser Steam_User::GetHSteamUser()
{
    PRINT_DEBUG_ENTRY();
    return (settings == get_steam_client()->settings_server) ? SERVER_HSTEAMUSER : CLIENT_HSTEAMUSER;
}

void Steam_User::LogOn( CSteamID steamID )
{
    PRINT_DEBUG_ENTRY();
    settings->set_offline(false);
    logon_time = std::chrono::high_resolution_clock::now();
    call_logged_on = true;
    call_logged_off = false;

    if (settings == get_steam_client()->settings_server) {
        get_steam_client()->steam_gameserver->LogOnAnonymous();
    }
}

void Steam_User::LogOff()
{
    PRINT_DEBUG_ENTRY();
    settings->set_offline(true);
    logoff_time = std::chrono::high_resolution_clock::now();
    call_logged_on = false;
    call_logged_off = true;
    player_auths.clear();

    if (settings == get_steam_client()->settings_server) {
        get_steam_client()->steam_gameserver->LogOff();
    }
}

// returns true if the Steam client current has a live connection to the Steam servers. 
// If false, it means there is no active connection due to either a networking issue on the local machine, or the Steam server is down/busy.
// The Steam client will automatically be trying to recreate the connection as often as possible.
bool Steam_User::BLoggedOn()
{
    PRINT_DEBUG_ENTRY();
    return !settings->is_offline();
}

ELogonState Steam_User::GetLogonState()
{
    PRINT_DEBUG_ENTRY();
    if(settings->is_offline())
        return (ELogonState)0;
    else
        return (ELogonState)4; // tested on real steam, undocumented return value
}

bool Steam_User::BConnected()
{
    PRINT_DEBUG_ENTRY();
    return !settings->is_offline();
}

// returns the CSteamID of the account currently logged into the Steam client
// a CSteamID is a unique identifier for an account, and used to differentiate users in all parts of the Steamworks API
CSteamID Steam_User::GetSteamID()
{
    PRINT_DEBUG_ENTRY();
    CSteamID id = settings->get_current_steam_id();

    PRINT_DEBUG("GetSteamID() call #%u, returning %llu", settings->global_steamid_call_count, id.ConvertToUint64());

    return id;
}

bool Steam_User::IsVACBanned( int nGameID )
{
    PRINT_DEBUG_ENTRY();
    return false;
}

bool Steam_User::RequireShowVACBannedMessage( int nGameID )
{
    PRINT_DEBUG_ENTRY();
    return false;
}

void Steam_User::AcknowledgeVACBanning( int nGameID )
{
    PRINT_DEBUG_ENTRY();
}

// according to comments in sdk, "these are dead."
int Steam_User::NClientGameIDAdd( int nGameID )
{
    PRINT_DEBUG_ENTRY();
    return 0;
}
// according to comments in sdk, "these are dead."
void Steam_User::RemoveClientGame( int nClientGameID )
{
    PRINT_DEBUG_ENTRY();
}
// according to comments in sdk, "these are dead."
void Steam_User::SetClientGameServer( int nClientGameID, uint32 unIPServer, uint16 usPortServer )
{
    PRINT_DEBUG_ENTRY();
}

void Steam_User::SetSteam2Ticket( uint8 *pubTicket, int cubTicket )
{
    PRINT_DEBUG_ENTRY();
}

void Steam_User::AddServerNetAddress( uint32 unIP, uint16 unPort )
{
    PRINT_DEBUG_ENTRY();
}

bool Steam_User::SetEmail( const char *pchEmail )
{
    PRINT_DEBUG_ENTRY();
    return false;
}

// according to comments in sdk, "logon cookie - this is obsolete and never used"
int Steam_User::GetSteamGameConnectToken( void *pBlob, int cbMaxBlob )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (cbMaxBlob < STEAM_TICKET_MIN_SIZE) return 0;
    if (!pBlob) return 0;

    uint32 out_size = STEAM_AUTH_TICKET_SIZE;
    auth_manager->getTicketData(pBlob, cbMaxBlob, &out_size);

    if (out_size > STEAM_AUTH_TICKET_SIZE)
        return 0;
    return out_size;
}

bool Steam_User::SetRegistryString( EConfigSubTree eRegistrySubTree, const char *pchKey, const char *pchValue )
{
    PRINT_DEBUG_TODO();
    if (!pchValue)
        return false; // real steam crashes, so return value is assumed

    if (!pchKey) // tested on real steam
    {
        registry.clear();
        registry_nullptr = std::string(pchValue);
    }
    else
    {
        registry[std::string(pchKey)] = std::string(pchValue);
        // TODO: save it to disk, because real steam can get the string when app restarts
    }

    return true;
}

bool Steam_User::GetRegistryString( EConfigSubTree eRegistrySubTree, const char *pchKey, char *pchValue, int cbValue )
{
    PRINT_DEBUG_TODO();
    // TODO: read data on disk, because real steam can get the string when app restarts
    if (pchValue && cbValue > 0)
        memset(pchValue, 0, cbValue);

    std::string value{};
    if(!pchKey)
    {
        value = registry_nullptr;
    }
    else
    {
        auto it = registry.find(std::string(pchKey));
        if (it == registry.end())
            return false;
        
        value = it->second;
    }

    if (pchValue && cbValue > 0)
        value.copy(pchValue, cbValue - 1);
    return true;
}

bool Steam_User::SetRegistryInt( EConfigSubTree eRegistrySubTree, const char *pchKey, int iValue )
{
    PRINT_DEBUG_TODO();
    if (!pchKey) // tested on real steam
    {
        registry.clear();
        registry_nullptr = std::to_string(iValue);
    }
    else
    {
        registry[std::string(pchKey)] = std::to_string(iValue);
        // TODO: save it to disk, because real steam can get the string when app restarts
    }

    return true;
}

bool Steam_User::GetRegistryInt( EConfigSubTree eRegistrySubTree, const char *pchKey, int *piValue )
{
    PRINT_DEBUG_TODO();
    // TODO: read data on disk, because real steam can get the string when app restarts
    if (piValue)
        *piValue = 0;

    std::string value{};
    if(!pchKey)
    {
        value = registry_nullptr;
    }
    else
    {
        auto it = registry.find(std::string(pchKey));
        if (it == registry.end())
            return false;
        
        value = it->second;
    }

    try
    {    
        if (piValue)
            *piValue = std::stoi(value);
    }
    catch(...)
    {
        PRINT_DEBUG("not a number"); // TODO: real steam returns a value other than 0 under this condition
    }

    return true;
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
int Steam_User::InitiateGameConnection( void *pAuthBlob, int cbMaxAuthBlob, CSteamID steamIDGameServer, uint32 unIPServer, uint16 usPortServer, bool bSecure )
{
    PRINT_DEBUG("%i %llu %u %u %u %p", cbMaxAuthBlob, steamIDGameServer.ConvertToUint64(), unIPServer, usPortServer, bSecure, pAuthBlob);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (cbMaxAuthBlob < STEAM_TICKET_MIN_SIZE) return 0;
    if (!pAuthBlob) return 0;

    uint32 out_size = STEAM_AUTH_TICKET_SIZE;
    auth_manager->getTicketData(pAuthBlob, cbMaxAuthBlob, &out_size);

    if (out_size > STEAM_AUTH_TICKET_SIZE)
        return 0;
    return out_size;
}

int Steam_User::InitiateGameConnection( void *pAuthBlob, int cbMaxAuthBlob, CSteamID steamIDGameServer, CGameID gameID, uint32 unIPServer, uint16 usPortServer, bool bSecure )
{
	PRINT_DEBUG_ENTRY();
	return InitiateGameConnection(pAuthBlob, cbMaxAuthBlob, steamIDGameServer, unIPServer, usPortServer, bSecure);
}

int Steam_User::InitiateGameConnection( void *pBlob, int cbMaxBlob, CSteamID steamID, CGameID gameID, uint32 unIPServer, uint16 usPortServer, bool bSecure, void *pvSteam2GetEncryptionKey, int cbSteam2GetEncryptionKey )
{
    PRINT_DEBUG("sdk 0.99x, 0.99y");
    return InitiateGameConnection(pBlob, cbMaxBlob, steamID, unIPServer, usPortServer, bSecure);
}

int Steam_User::InitiateGameConnection( void *pBlob, int cbMaxBlob, CSteamID steamID, int nGameAppID, uint32 unIPServer, uint16 usPortServer, bool bSecure )
{
	PRINT_DEBUG("sdk 0.99u");
	return InitiateGameConnection(pBlob, cbMaxBlob, steamID, unIPServer, usPortServer, bSecure);
}

// notify of disconnect
// needs to occur when the game client leaves the specified game server, needs to match with the InitiateGameConnection() call
void Steam_User::TerminateGameConnection( uint32 unIPServer, uint16 usPortServer )
{
    PRINT_DEBUG_TODO();
}

// Legacy functions

void Steam_User::SetSelfAsPrimaryChatDestination()
{
    PRINT_DEBUG_TODO();
}

bool Steam_User::IsPrimaryChatDestination()
{
    PRINT_DEBUG_ENTRY();
    return false;
}

void Steam_User::RequestLegacyCDKey( uint32 iAppID )
{
    PRINT_DEBUG_TODO();
}

bool Steam_User::SendGuestPassByEmail( const char *pchEmailAccount, GID_t gidGuestPassID, bool bResending )
{
    PRINT_DEBUG_TODO();
    return false;
}

bool Steam_User::SendGuestPassByAccountID( uint32 uAccountID, GID_t gidGuestPassID, bool bResending )
{
    PRINT_DEBUG_TODO();
    return false;
}

bool Steam_User::AckGuestPass(const char *pchGuestPassCode)
{
    PRINT_DEBUG_TODO();
    return false;
}

bool Steam_User::RedeemGuestPass(const char *pchGuestPassCode)
{
    PRINT_DEBUG_TODO();
    return false;
}

uint32 Steam_User::GetGuestPassToGiveCount()
{
    PRINT_DEBUG_TODO();
    return 0;
}

uint32 Steam_User::GetGuestPassToRedeemCount()
{
    PRINT_DEBUG_TODO();
    return 0;
}

RTime32 Steam_User::GetGuestPassLastUpdateTime()
{
    PRINT_DEBUG_TODO();
    return 0;
}

bool Steam_User::GetGuestPassToGiveInfo( uint32 nPassIndex, GID_t *pgidGuestPassID, PackageId_t *pnPackageID, RTime32 *pRTime32Created, RTime32 *pRTime32Expiration, RTime32 *pRTime32Sent, RTime32 *pRTime32Redeemed, char *pchRecipientAddress, int cRecipientAddressSize )
{
    PRINT_DEBUG_TODO();
    // TODO: pgidGuestPassID
    if (pnPackageID)
        *pnPackageID = 0;
    if (pRTime32Created)
        *pRTime32Created = 0;
    if (pRTime32Expiration)
        *pRTime32Expiration = 0;
    if (pRTime32Sent)
        *pRTime32Sent = 0;
    if (pRTime32Redeemed)
        *pRTime32Redeemed = 0;
    if (pchRecipientAddress && cRecipientAddressSize > 0)
        memset(pchRecipientAddress, 0, cRecipientAddressSize);
    return false;
}

bool Steam_User::GetGuestPassToRedeemInfo( uint32 nPassIndex, GID_t *pgidGuestPassID, PackageId_t *pnPackageID, RTime32 *pRTime32Created, RTime32 *pRTime32Expiration, RTime32 *pRTime32Sent, RTime32 *pRTime32Redeemed)
{
    PRINT_DEBUG_TODO();
    // TODO: pgidGuestPassID
    if (pnPackageID)
        *pnPackageID = 0;
    if (pRTime32Created)
        *pRTime32Created = 0;
    if (pRTime32Expiration)
        *pRTime32Expiration = 0;
    if (pRTime32Sent)
        *pRTime32Sent = 0;
    if (pRTime32Redeemed)
        *pRTime32Redeemed = 0;
    return false;
}

bool Steam_User::GetGuestPassToRedeemSenderAddress( uint32 nPassIndex, char* pchSenderAddress, int cSenderAddressSize )
{
    PRINT_DEBUG_TODO();
    return false;
}

bool Steam_User::GetGuestPassToRedeemSenderName( uint32 nPassIndex, char* pchSenderName, int cSenderNameSize )
{
    PRINT_DEBUG_TODO();
    if (pchSenderName && cSenderNameSize > 0)
        memset(pchSenderName, 0, cSenderNameSize);
    return false;
}

void Steam_User::AcknowledgeMessageByGID( const char *pchMessageGID )
{
    PRINT_DEBUG_TODO();
}

bool Steam_User::SetLanguage( const char *pchLanguage )
{
    PRINT_DEBUG_TODO();
    // TODO: don't know what this api actually does other than returning true
    return true;
}

// used by only a few games to track usage events
void Steam_User::TrackAppUsageEvent( CGameID gameID, int eAppUsageEvent, const char *pchExtraInfo)
{
    PRINT_DEBUG_TODO();
}

void Steam_User::SetAccountName( const char *pchAccountName )
{
    PRINT_DEBUG_TODO();
}

void Steam_User::SetPassword( const char *pchPassword )
{
    PRINT_DEBUG_TODO();
}

void Steam_User::SetAccountCreationTime( RTime32 rt )
{
    PRINT_DEBUG_TODO();
}

void Steam_User::RefreshSteam2Login()
{
    PRINT_DEBUG_TODO();
}

// get the local storage folder for current Steam account to write application data, e.g. save games, configs etc.
// this will usually be something like "C:\Progam Files\Steam\userdata\<SteamID>\<AppID>\local"
bool Steam_User::GetUserDataFolder( char *pchBuffer, int cubBuffer )
{
    PRINT_DEBUG_ENTRY();
    if (!cubBuffer || cubBuffer <= 0) return false;

    std::string user_data = local_storage->get_path(Local_Storage::user_data_storage);
    if (static_cast<size_t>(cubBuffer) <= user_data.size()) return false;

    strncpy(pchBuffer, user_data.c_str(), cubBuffer - 1);
    pchBuffer[cubBuffer - 1] = 0;
    return true;
}

// Starts voice recording. Once started, use GetVoice() to get the data
void Steam_User::StartVoiceRecording( )
{
    if (!settings->enable_voice_chat) return;

    if (!voicechat->IsRecordingActive()) {
        PRINT_DEBUG_ENTRY();

        if (voicechat->InitVoiceSystem()) {
            voicechat->StartVoiceRecording();
        }
    }
}

// Stops voice recording. Because people often release push-to-talk keys early, the system will keep recording for
// a little bit after this function is called. GetVoice() should continue to be called until it returns
// k_eVoiceResultNotRecording
void Steam_User::StopVoiceRecording( )
{
    PRINT_DEBUG_ENTRY();
    if (!settings->enable_voice_chat) return;

    voicechat->StopVoiceRecording();
}

// Determine the size of captured audio data that is available from GetVoice.
// Most applications will only use compressed data and should ignore the other
// parameters, which exist primarily for backwards compatibility. See comments
// below for further explanation of "uncompressed" data.
EVoiceResult Steam_User::GetAvailableVoice( uint32 *pcbCompressed, uint32 *pcbUncompressed_Deprecated, uint32 nUncompressedVoiceDesiredSampleRate_Deprecated  )
{
    PRINT_DEBUG_ENTRY();

    if (pcbCompressed) *pcbCompressed = 0;
    if (pcbUncompressed_Deprecated) *pcbUncompressed_Deprecated = 0;
    if (!settings->enable_voice_chat) return k_EVoiceResultNoData;

    // some games like appid 34330 don't call this
    StartVoiceRecording();
    return voicechat->GetAvailableVoice(pcbCompressed);
}

EVoiceResult Steam_User::GetAvailableVoice(uint32 *pcbCompressed, uint32 *pcbUncompressed)
{
	PRINT_DEBUG("old");
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
EVoiceResult Steam_User::GetVoice( bool bWantCompressed, void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten, bool bWantUncompressed_Deprecated, void *pUncompressedDestBuffer_Deprecated , uint32 cbUncompressedDestBufferSize_Deprecated , uint32 *nUncompressBytesWritten_Deprecated , uint32 nUncompressedVoiceDesiredSampleRate_Deprecated  )
{
    PRINT_DEBUG_ENTRY();
    if (nBytesWritten) *nBytesWritten = 0;
    if (nUncompressBytesWritten_Deprecated) *nUncompressBytesWritten_Deprecated = 0;
    if (!settings->enable_voice_chat) return k_EVoiceResultNoData;

    // should we have this here ? -detanup
    // some games might not initialize this.
    // example appid 34330
    StartVoiceRecording();
    return voicechat->GetVoice(bWantCompressed, pDestBuffer, cbDestBufferSize, nBytesWritten);
}

EVoiceResult Steam_User::GetVoice( bool bWantCompressed, void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten, bool bWantUncompressed, void *pUncompressedDestBuffer, uint32 cbUncompressedDestBufferSize, uint32 *nUncompressBytesWritten )
{
	PRINT_DEBUG("old");
	return GetVoice(bWantCompressed, pDestBuffer, cbDestBufferSize, nBytesWritten, bWantUncompressed, pUncompressedDestBuffer, cbUncompressedDestBufferSize, nUncompressBytesWritten, 11025);
}

EVoiceResult Steam_User::GetCompressedVoice( void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten )
{
	PRINT_DEBUG_ENTRY();
	return GetVoice(true, pDestBuffer, cbDestBufferSize, nBytesWritten, false, NULL, 0, NULL);
}

// Decodes the compressed voice data returned by GetVoice. The output data is
// raw single-channel 16-bit PCM audio. The decoder supports any sample rate
// from 11025 to 48000; see GetVoiceOptimalSampleRate() below for details.
// If the output buffer is not large enough, then *nBytesWritten will be set
// to the required buffer size, and k_EVoiceResultBufferTooSmall is returned.
// It is suggested to start with a 20kb buffer and reallocate as necessary.
EVoiceResult Steam_User::DecompressVoice( const void *pCompressed, uint32 cbCompressed, void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten, uint32 nDesiredSampleRate )
{
    PRINT_DEBUG_ENTRY();
    return voicechat->DecompressVoice(pCompressed, cbCompressed, pDestBuffer, cbDestBufferSize, nBytesWritten, nDesiredSampleRate);
}

EVoiceResult Steam_User::DecompressVoice( const void *pCompressed, uint32 cbCompressed, void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten )
{
	PRINT_DEBUG("old");
	return DecompressVoice(pCompressed, cbCompressed, pDestBuffer, cbDestBufferSize, nBytesWritten, 11025);
}

EVoiceResult Steam_User::DecompressVoice( void *pCompressed, uint32 cbCompressed, void *pDestBuffer, uint32 cbDestBufferSize, uint32 *nBytesWritten )
{
	PRINT_DEBUG("older");
	return DecompressVoice(pCompressed, cbCompressed, pDestBuffer, cbDestBufferSize, nBytesWritten, 11025);
}

// This returns the native sample rate of the Steam voice decompressor
// this sample rate for DecompressVoice will perform the least CPU processing.
// However, the final audio quality will depend on how well the audio device
// (and/or your application's audio output SDK) deals with lower sample rates.
// You may find that you get the best audio output quality when you ignore
// this function and use the native sample rate of your audio output device,
// which is usually 48000 or 44100.
uint32 Steam_User::GetVoiceOptimalSampleRate()
{
    PRINT_DEBUG_ENTRY();
    return SAMPLE_RATE;
}

// Retrieve ticket to be sent to the entity who wishes to authenticate you. 
// pcbTicket retrieves the length of the actual ticket.
HAuthTicket Steam_User::GetAuthSessionTicket( void *pTicket, int cbMaxTicket, uint32 *pcbTicket )
{
    return GetAuthSessionTicket(pTicket, cbMaxTicket, pcbTicket, NULL);
}
// SteamNetworkingIdentity is an optional input parameter to hold the public IP address or SteamID of the entity you are connecting to
// if an IP address is passed Steam will only allow the ticket to be used by an entity with that IP address
// if a Steam ID is passed Steam will only allow the ticket to be used by that Steam ID
HAuthTicket Steam_User::GetAuthSessionTicket( void *pTicket, int cbMaxTicket, uint32 *pcbTicket, const SteamNetworkingIdentity *pSteamNetworkingIdentity )
{
    PRINT_DEBUG("%p [%i] %p", pTicket, cbMaxTicket, pcbTicket);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (!pTicket) return k_HAuthTicketInvalid;
    
    return auth_manager->getTicket(pTicket, cbMaxTicket, pcbTicket);
}

// Request a ticket which will be used for webapi "ISteamUserAuth\AuthenticateUserTicket"
// pchIdentity is an optional input parameter to identify the service the ticket will be sent to
// the ticket will be returned in callback GetTicketForWebApiResponse_t
HAuthTicket Steam_User::GetAuthTicketForWebApi( const char *pchIdentity )
{
    PRINT_DEBUG("'%s'", pchIdentity);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    return auth_manager->getWebApiTicket(pchIdentity);
}

// Authenticate ticket from entity steamID to be sure it is valid and isnt reused
// Registers for callbacks if the entity goes offline or cancels the ticket ( see ValidateAuthTicketResponse_t callback and EAuthSessionResponse )
EBeginAuthSessionResult Steam_User::BeginAuthSession( const void *pAuthTicket, int cbAuthTicket, CSteamID steamID )
{
    PRINT_DEBUG("%i %llu", cbAuthTicket, steamID.ConvertToUint64());
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    return auth_manager->beginAuth(pAuthTicket, cbAuthTicket, steamID);
}

// Stop tracking started by BeginAuthSession - called when no longer playing game with this entity
void Steam_User::EndAuthSession( CSteamID steamID )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    auth_manager->endAuth(steamID);
}

// Cancel auth ticket from GetAuthSessionTicket, called when no longer playing game with the entity you gave the ticket to
void Steam_User::CancelAuthTicket( HAuthTicket hAuthTicket )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    auth_manager->cancelTicket(hAuthTicket);
}

// After receiving a user's authentication data, and passing it to BeginAuthSession, use this function
// to determine if the user owns downloadable content specified by the provided AppID.
EUserHasLicenseForAppResult Steam_User::UserHasLicenseForApp( CSteamID steamID, AppId_t appID )
{
    PRINT_DEBUG_ENTRY();
    return k_EUserHasLicenseResultHasLicense;
}

// returns true if this users looks like they are behind a NAT device. Only valid once the user has connected to steam 
// (i.e a SteamServersConnected_t has been issued) and may not catch all forms of NAT.
bool Steam_User::BIsBehindNAT()
{
    PRINT_DEBUG_ENTRY();
    return false;
}

// set data to be replicated to friends so that they can join your game
// CSteamID steamIDGameServer - the steamID of the game server, received from the game server by the client
// uint32 unIPServer, uint16 usPortServer - the IP address of the game server
void Steam_User::AdvertiseGame( CSteamID steamIDGameServer, uint32 unIPServer, uint16 usPortServer )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    Gameserver *server = new Gameserver();
    server->set_id(steamIDGameServer.ConvertToUint64());
    server->set_ip(unIPServer);
    server->set_port(usPortServer);
    server->set_query_port(usPortServer);
    server->set_appid(settings->get_local_game_id().AppID());
    
    if (settings->matchmaking_server_list_always_lan_type)
        server->set_type(eLANServer);
    else
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
SteamAPICall_t Steam_User::RequestEncryptedAppTicket( void *pDataToInclude, int cbDataToInclude )
{
    PRINT_DEBUG("%i %p", cbDataToInclude, pDataToInclude);
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
    ticket.TicketV2.TicketIssueTime = static_cast<uint32>(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    ticket.TicketV2.TicketValidityEnd = ticket.TicketV2.TicketIssueTime + (21 * 24 * 60 * 60);

    for (unsigned int i = 0; i < 140; ++i)
    {
        AppId_t appid{};
        bool available{};
        std::string name{};
        if (!settings->getDLC(i, appid, available, name)) break;
        ticket.TicketV4.AppIDs.emplace_back(appid);
    }

    ticket.TicketV4.HasVACStatus = true;
    ticket.TicketV4.VACStatus = 0;

    auto serialized = ticket.SerializeTicket();

    SteamAppTicket_pb pb;
    pb.set_ticket_version_no(1);
    pb.set_crc_encryptedticket(0); // TODO: Find out how to compute the CRC
    pb.set_cb_encrypteduserdata(cbDataToInclude);
    pb.set_cb_encrypted_appownershipticket(static_cast<uint32>(serialized.size()) - 16);
    pb.mutable_encrypted_ticket()->assign(serialized.begin(), serialized.end()); // TODO: Find how to encrypt datas

    encrypted_app_ticket = pb.SerializeAsString();

    auto ret = callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
    return ret;
}

// retrieve a finished ticket
bool Steam_User::GetEncryptedAppTicket( void *pTicket, int cbMaxTicket, uint32 *pcbTicket )
{
    PRINT_DEBUG("%i %p %p", cbMaxTicket, pTicket, pcbTicket);
    
    // Try to load a token from configs.user.ini first
    if (!settings->customEncryptedAppTicket.empty()) {
        PRINT_DEBUG("Using token from configs.user.ini\n");
        
        uint32 ticket_size = static_cast<uint32>(settings->customEncryptedAppTicket.size());
        if (pcbTicket) *pcbTicket = ticket_size;

        if (cbMaxTicket <= 0) {
            if (!pcbTicket) return false;
            return true;
        }

        if (!pTicket) return false;
        if (ticket_size > static_cast<uint32>(cbMaxTicket)) return false;

        memcpy(pTicket, settings->customEncryptedAppTicket.data(), ticket_size);
        PRINT_DEBUG("Successfully used token from configs.user.ini (%u bytes)\n", ticket_size);
        return true;
    }
    
    // Fallback to the standard ticket generation if no token was found or decoded
    uint32 ticket_size = static_cast<uint32>(encrypted_app_ticket.size());
    if (pcbTicket) *pcbTicket = ticket_size;

    if (cbMaxTicket <= 0) {
        if (!pcbTicket) return false;
        return true;
    }

    if (!pTicket) return false;
    if (ticket_size > static_cast<uint32>(cbMaxTicket)) return false;
    encrypted_app_ticket.copy((char *)pTicket, cbMaxTicket);

    PRINT_DEBUG("copied successfully");
    return true;
}

// Trading Card badges data access
// if you only have one set of cards, the series will be 1
// the user has can have two different badges for a series; the regular (max level 5) and the foil (max level 1)
int Steam_User::GetGameBadgeLevel( int nSeries, bool bFoil )
{
    PRINT_DEBUG_ENTRY();
    return 0;
}

// gets the Steam Level of the user, as shown on their profile
int Steam_User::GetPlayerSteamLevel()
{
    PRINT_DEBUG_ENTRY();
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
SteamAPICall_t Steam_User::RequestStoreAuthURL( const char *pchRedirectURL )
{
    PRINT_DEBUG_TODO();
    return 0;
}

// gets whether the users phone number is verified 
bool Steam_User::BIsPhoneVerified()
{
    PRINT_DEBUG_ENTRY();
    return true;
}

// gets whether the user has two factor enabled on their account
bool Steam_User::BIsTwoFactorEnabled()
{
    PRINT_DEBUG_ENTRY();
    return true;
}

// gets whether the users phone number is identifying
bool Steam_User::BIsPhoneIdentifying()
{
    PRINT_DEBUG_ENTRY();
    return false;
}

// gets whether the users phone number is awaiting (re)verification
bool Steam_User::BIsPhoneRequiringVerification()
{
    PRINT_DEBUG_ENTRY();
    return false;
}

STEAM_CALL_RESULT( MarketEligibilityResponse_t )
SteamAPICall_t Steam_User::GetMarketEligibility()
{
    PRINT_DEBUG_TODO();
    return 0;
}

// Retrieves anti indulgence / duration control for current user
STEAM_CALL_RESULT( DurationControl_t )
SteamAPICall_t Steam_User::GetDurationControl()
{
    PRINT_DEBUG_TODO();
    return 0;
}

// Advise steam china duration control system about the online state of the game.
// This will prevent offline gameplay time from counting against a user's
// playtime limits.
bool Steam_User::BSetDurationControlOnlineState( EDurationControlOnlineState eNewState )
{
    PRINT_DEBUG_ENTRY();
    return false;
}

// older sdk -----------------------------------------------
void Steam_User::Init( ICMCallback001 *cmcallback, ISteam2Auth *steam2auth )
{
    PRINT_DEBUG_ENTRY();
    callbacks_old1 = cmcallback;
}

void Steam_User::Init( ICMCallback *cmcallback, ISteam2Auth *steam2auth )
{
    PRINT_DEBUG_ENTRY();
    callbacks_old2 = cmcallback;
}

int Steam_User::ProcessCall( int unk )
{
    PRINT_DEBUG_TODO();
    return 0;
}

void Steam_User::LogOn( CSteamID *steamID )
{
    PRINT_DEBUG_ENTRY();
    LogOn(*steamID);
}

int Steam_User::CreateAccount( const char *unk1, void *unk2, void *unk3, const char *unk4, int unk5, void *unk6 )
{
    PRINT_DEBUG_TODO();
    return 0;
}

bool Steam_User::GSSendLogonRequest( CSteamID *steamID )
{
    PRINT_DEBUG("%llu", (*steamID).ConvertToUint64());
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    // Note that SteamID passed into this comes from Steam.dll so it won't match the client's Goldberg SteamID.
    std::pair<CSteamID, std::chrono::high_resolution_clock::time_point> entry(*steamID, std::chrono::high_resolution_clock::now());
    player_auths.push_back(entry);
    get_steam_client()->steam_gameserver->add_player(*steamID);
    return true;
}

bool Steam_User::GSSendDisconnect( CSteamID *steamID )
{
    PRINT_DEBUG("%llu", (*steamID).ConvertToUint64());
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    get_steam_client()->steam_gameserver->remove_player(*steamID);
    return true;
}

bool Steam_User::GSSendStatusResponse( CSteamID *steamID, int nSecondsConnected, int nSecondsSinceLast )
{
    PRINT_DEBUG_TODO();
    return false;
}

bool Steam_User::GSSetStatus( int32 nAppIdServed, uint32 unServerFlags, int cPlayers, int cPlayersMax )
{
    PRINT_DEBUG_TODO();
    return true;
}

bool Steam_User::GSSetStatus( int32 nAppIdServed, uint32 unServerFlags, int cPlayers, int cPlayersMax, int cBotPlayers, int unGamePort, const char *pchServerName, const char *pchGameDir, const char *pchMapName, const char *pchVersion )
{
    PRINT_DEBUG_ENTRY();
    return get_steam_client()->steam_gameserver->Obsolete_GSSetStatus(nAppIdServed, unServerFlags, cPlayers, cPlayersMax, cBotPlayers, unGamePort, pchServerName, pchGameDir, pchMapName, pchVersion);
}

bool Steam_User::BGetCallback( int *piCallback, uint8 **ppubParam, int *unk )
{
    PRINT_DEBUG_ENTRY();
    HSteamUser user = (settings == get_steam_client()->settings_server) ? SERVER_HSTEAMUSER : CLIENT_HSTEAMUSER;
    HSteamPipe pipe = get_steam_client()->get_pipe_for_user(user);
    if (!pipe)
        return false;

    CallbackMsg_t msg;
    if (!steamclient_get_callback(pipe, &msg))
        return false;

    *piCallback = msg.m_iCallback;
    *ppubParam = msg.m_pubParam;
    return true;
}

void Steam_User::FreeLastCallback()
{
    PRINT_DEBUG_ENTRY();
    HSteamUser user = (settings == get_steam_client()->settings_server) ? SERVER_HSTEAMUSER : CLIENT_HSTEAMUSER;
    HSteamPipe pipe = get_steam_client()->get_pipe_for_user(user);
    if (!pipe)
        return;

    steamclient_free_callback(pipe);
}

int Steam_User::GetSteamTicket( void *pBlob, int cbMaxBlob )
{
    PRINT_DEBUG_ENTRY();
    std::lock_guard<std::recursive_mutex> lock(global_mutex);

    if (cbMaxBlob < STEAM_TICKET_MIN_SIZE) return 0;
    if (!pBlob) return 0;

    uint32 out_size = STEAM_AUTH_TICKET_SIZE;
    auth_manager->getTicketData(pBlob, cbMaxBlob, &out_size);

    if (out_size > STEAM_AUTH_TICKET_SIZE)
        return 0;
    return out_size;
}

const char *Steam_User::GetPlayerName()
{
    PRINT_DEBUG_ENTRY();
    return get_steam_client()->steam_friends->GetPersonaName();
}

void Steam_User::SetPlayerName( const char *pchPersonaName )
{
    PRINT_DEBUG_ENTRY();
    get_steam_client()->steam_friends->SetPersonaName_old(pchPersonaName);
}

EPersonaState Steam_User::GetFriendStatus()
{
    PRINT_DEBUG_ENTRY();
    return get_steam_client()->steam_friends->GetPersonaState();
}

void Steam_User::SetFriendStatus( EPersonaState ePersonaState )
{
    PRINT_DEBUG_ENTRY();
    return get_steam_client()->steam_friends->SetPersonaState(ePersonaState);
}

bool Steam_User::AddFriend( CSteamID steamIDFriend )
{
    PRINT_DEBUG_ENTRY();
    return get_steam_client()->steam_friends->AddFriend(steamIDFriend);
}

bool Steam_User::RemoveFriend( CSteamID steamIDFriend )
{
    PRINT_DEBUG_ENTRY();
    return get_steam_client()->steam_friends->RemoveFriend(steamIDFriend);
}

bool Steam_User::HasFriend( CSteamID steamIDFriend )
{
    PRINT_DEBUG_ENTRY();
    return get_steam_client()->steam_friends->HasFriend(steamIDFriend);
}

EFriendRelationship Steam_User::GetFriendRelationship( CSteamID steamIDFriend )
{
    PRINT_DEBUG_ENTRY();
    return get_steam_client()->steam_friends->GetFriendRelationship(steamIDFriend);
}

EPersonaState Steam_User::GetFriendStatus( CSteamID steamIDFriend )
{
    PRINT_DEBUG_ENTRY();
    return get_steam_client()->steam_friends->GetFriendPersonaState(steamIDFriend);
}

bool Steam_User::GetFriendGamePlayed( CSteamID steamIDFriend, int32 *pnGameID, uint32 *punGameIP, uint16 *pusGamePort )
{
    PRINT_DEBUG_ENTRY();
    return get_steam_client()->steam_friends->Deprecated_GetFriendGamePlayed(steamIDFriend, pnGameID, punGameIP, pusGamePort);
}

const char *Steam_User::GetPlayerName( CSteamID steamIDFriend )
{
    PRINT_DEBUG_ENTRY();
    return get_steam_client()->steam_friends->GetFriendPersonaName(steamIDFriend);
}

int32 Steam_User::AddFriendByName( const char *pchEmailOrAccountName )
{
    PRINT_DEBUG_ENTRY();
    return get_steam_client()->steam_friends->AddFriendByName(pchEmailOrAccountName);
}
// older sdk -----------------------------------------------

void Steam_User::RunCallbacks()
{
    if (callbacks_old1) {
        if (call_logged_on && check_timedout(logon_time, 0.1)) {
            PRINT_DEBUG("ICMCallback -> OnLogonSuccess");
            callbacks_old1->OnLogonSuccess();
            call_logged_on = false;
        }

        if (call_logged_off && check_timedout(logoff_time, 0.1)) {
            PRINT_DEBUG("ICMCallback -> OnLoggedOff");
            callbacks_old1->OnLoggedOff();
            call_logged_off = false;
        }

        for (auto it = player_auths.begin(); it != player_auths.end();) {
            if (check_timedout(it->second, 0.1)) {
                PRINT_DEBUG("ICMCallback -> GSHandleClientApprove %llu", it->first.ConvertToUint64());
                callbacks_old1->GSHandleClientApprove(&(it->first));
                it = player_auths.erase(it);
            } else {
                it++;
            }
        }
    } else if (callbacks_old2) {
        if (call_logged_on && check_timedout(logon_time, 0.1)) {
            PRINT_DEBUG("ICMCallback -> OnLogonSuccess");
            callbacks_old2->OnLogonSuccess();
            call_logged_on = false;
        }

        if (call_logged_off && check_timedout(logoff_time, 0.1)) {
            PRINT_DEBUG("ICMCallback -> OnLoggedOff");
            callbacks_old2->OnLoggedOff();
            call_logged_off = false;
        }

        for (auto it = player_auths.begin(); it != player_auths.end();) {
            if (check_timedout(it->second, 0.1)) {
                PRINT_DEBUG("ICMCallback -> GSHandleClientApprove %llu", it->first.ConvertToUint64());
                callbacks_old2->GSHandleClientApprove(&(it->first));
                it = player_auths.erase(it);
            } else {
                it++;
            }
        }
    }
}
