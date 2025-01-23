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

#ifndef STEAMCLIENT_DLL
#define STEAM_API_FUNCTIONS_IMPL
#include "dll/dll.h"
#include "steam/steam_api_flat.h"

STEAMAPI_API HSteamPipe SteamAPI_ISteamClient_CreateSteamPipe( ISteamClient* self )
{
    return get_steam_client()->CreateSteamPipe();
}

STEAMAPI_API steam_bool SteamAPI_ISteamClient_BReleaseSteamPipe( ISteamClient* self, HSteamPipe hSteamPipe )
{
    return get_steam_client()->BReleaseSteamPipe(hSteamPipe);
}

STEAMAPI_API HSteamUser SteamAPI_ISteamClient_ConnectToGlobalUser( ISteamClient* self, HSteamPipe hSteamPipe )
{
    return get_steam_client()->ConnectToGlobalUser(hSteamPipe);
}

STEAMAPI_API HSteamUser SteamAPI_ISteamClient_CreateLocalUser( ISteamClient* self, HSteamPipe * phSteamPipe, EAccountType eAccountType )
{
    return get_steam_client()->CreateLocalUser(phSteamPipe, eAccountType);
}

STEAMAPI_API void SteamAPI_ISteamClient_ReleaseUser( ISteamClient* self, HSteamPipe hSteamPipe, HSteamUser hUser )
{
    return get_steam_client()->ReleaseUser(hSteamPipe, hUser);
}

STEAMAPI_API ISteamUser * SteamAPI_ISteamClient_GetISteamUser( ISteamClient* self, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char * pchVersion )
{
    return get_steam_client()->GetISteamUser(hSteamUser, hSteamPipe, pchVersion);
}

STEAMAPI_API ISteamGameServer * SteamAPI_ISteamClient_GetISteamGameServer( ISteamClient* self, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char * pchVersion )
{
    return get_steam_client()->GetISteamGameServer(hSteamUser, hSteamPipe, pchVersion);
}

STEAMAPI_API void SteamAPI_ISteamClient_SetLocalIPBinding( ISteamClient* self, const SteamIPAddress_t & unIP, uint16 usPort )
{
    //Note: this function was updated but currently doesn't do anything so I'm just leaving it like this for now.
    //void SteamAPI_ISteamClient_SetLocalIPBinding(intptr_t instancePtr, const struct SteamIPAddress_t & unIP, uint16 usPort)
    return get_steam_client()->SetLocalIPBinding(unIP, usPort);
}

STEAMAPI_API ISteamFriends * SteamAPI_ISteamClient_GetISteamFriends( ISteamClient* self, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char * pchVersion )
{
    return get_steam_client()->GetISteamFriends(hSteamUser, hSteamPipe, pchVersion);
}

STEAMAPI_API ISteamUtils * SteamAPI_ISteamClient_GetISteamUtils( ISteamClient* self, HSteamPipe hSteamPipe, const char * pchVersion )
{
    return get_steam_client()->GetISteamUtils(hSteamPipe, pchVersion);
}

STEAMAPI_API ISteamMatchmaking * SteamAPI_ISteamClient_GetISteamMatchmaking( ISteamClient* self, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char * pchVersion )
{
    return get_steam_client()->GetISteamMatchmaking(hSteamUser, hSteamPipe, pchVersion);
}

STEAMAPI_API ISteamMatchmakingServers * SteamAPI_ISteamClient_GetISteamMatchmakingServers( ISteamClient* self, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char * pchVersion )
{
    return get_steam_client()->GetISteamMatchmakingServers(hSteamUser, hSteamPipe, pchVersion);
}

STEAMAPI_API void * SteamAPI_ISteamClient_GetISteamGenericInterface( ISteamClient* self, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char * pchVersion )
{
    return get_steam_client()->GetISteamGenericInterface(hSteamUser, hSteamPipe, pchVersion);
}

STEAMAPI_API ISteamUserStats * SteamAPI_ISteamClient_GetISteamUserStats( ISteamClient* self, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char * pchVersion )
{
    return get_steam_client()->GetISteamUserStats(hSteamUser, hSteamPipe, pchVersion);
}

STEAMAPI_API ISteamGameServerStats * SteamAPI_ISteamClient_GetISteamGameServerStats( ISteamClient* self, HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char * pchVersion )
{
    return get_steam_client()->GetISteamGameServerStats(hSteamuser, hSteamPipe, pchVersion);
}

STEAMAPI_API ISteamApps * SteamAPI_ISteamClient_GetISteamApps( ISteamClient* self, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char * pchVersion )
{
    return get_steam_client()->GetISteamApps(hSteamUser, hSteamPipe, pchVersion);
}

STEAMAPI_API ISteamNetworking * SteamAPI_ISteamClient_GetISteamNetworking( ISteamClient* self, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char * pchVersion )
{
    return get_steam_client()->GetISteamNetworking(hSteamUser, hSteamPipe, pchVersion);
}

STEAMAPI_API ISteamRemoteStorage * SteamAPI_ISteamClient_GetISteamRemoteStorage( ISteamClient* self, HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char * pchVersion )
{
    return get_steam_client()->GetISteamRemoteStorage(hSteamuser, hSteamPipe, pchVersion);
}

STEAMAPI_API ISteamScreenshots * SteamAPI_ISteamClient_GetISteamScreenshots( ISteamClient* self, HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char * pchVersion )
{
    return get_steam_client()->GetISteamScreenshots(hSteamuser, hSteamPipe, pchVersion);
}

STEAMAPI_API ISteamGameSearch * SteamAPI_ISteamClient_GetISteamGameSearch( ISteamClient* self, HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char * pchVersion )
{
    return get_steam_client()->GetISteamGameSearch(hSteamuser, hSteamPipe, pchVersion);
}

STEAMAPI_API uint32 SteamAPI_ISteamClient_GetIPCCallCount( ISteamClient* self )
{
    return get_steam_client()->GetIPCCallCount();
}

STEAMAPI_API void SteamAPI_ISteamClient_SetWarningMessageHook( ISteamClient* self, SteamAPIWarningMessageHook_t pFunction )
{
    return get_steam_client()->SetWarningMessageHook(pFunction);
}

STEAMAPI_API steam_bool SteamAPI_ISteamClient_BShutdownIfAllPipesClosed( ISteamClient* self )
{
    return get_steam_client()->BShutdownIfAllPipesClosed();
}

STEAMAPI_API ISteamHTTP * SteamAPI_ISteamClient_GetISteamHTTP( ISteamClient* self, HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char * pchVersion )
{
    return get_steam_client()->GetISteamHTTP(hSteamuser, hSteamPipe, pchVersion);
}

STEAMAPI_API void *SteamAPI_ISteamClient_GetISteamUnifiedMessages(intptr_t instancePtr, HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char * pchVersion)
{
    return get_steam_client()->DEPRECATED_GetISteamUnifiedMessages(hSteamuser,hSteamPipe,pchVersion);
}

STEAMAPI_API ISteamController * SteamAPI_ISteamClient_GetISteamController( ISteamClient* self, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char * pchVersion )
{
    return get_steam_client()->GetISteamController(hSteamUser, hSteamPipe, pchVersion);
}

STEAMAPI_API ISteamUGC * SteamAPI_ISteamClient_GetISteamUGC( ISteamClient* self, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char * pchVersion )
{
    return get_steam_client()->GetISteamUGC(hSteamUser, hSteamPipe, pchVersion);
}

STEAMAPI_API ISteamAppList * SteamAPI_ISteamClient_GetISteamAppList( ISteamClient* self, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char * pchVersion )
{
    return get_steam_client()->GetISteamAppList(hSteamUser, hSteamPipe, pchVersion);
}

STEAMAPI_API ISteamMusic * SteamAPI_ISteamClient_GetISteamMusic( ISteamClient* self, HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char * pchVersion )
{
    return get_steam_client()->GetISteamMusic(hSteamuser, hSteamPipe, pchVersion);
}

STEAMAPI_API ISteamMusicRemote * SteamAPI_ISteamClient_GetISteamMusicRemote( ISteamClient* self, HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char * pchVersion )
{
    return get_steam_client()->GetISteamMusicRemote(hSteamuser, hSteamPipe, pchVersion);
}

STEAMAPI_API ISteamHTMLSurface * SteamAPI_ISteamClient_GetISteamHTMLSurface( ISteamClient* self, HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char * pchVersion )
{
    return get_steam_client()->GetISteamHTMLSurface(hSteamuser, hSteamPipe, pchVersion);
}

STEAMAPI_API ISteamInventory * SteamAPI_ISteamClient_GetISteamInventory( ISteamClient* self, HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char * pchVersion )
{
    return get_steam_client()->GetISteamInventory(hSteamuser, hSteamPipe, pchVersion);
}

STEAMAPI_API ISteamVideo * SteamAPI_ISteamClient_GetISteamVideo( ISteamClient* self, HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char * pchVersion )
{
    return get_steam_client()->GetISteamVideo(hSteamuser, hSteamPipe, pchVersion);
}

STEAMAPI_API ISteamParentalSettings * SteamAPI_ISteamClient_GetISteamParentalSettings( ISteamClient* self, HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char * pchVersion )
{
    return get_steam_client()->GetISteamParentalSettings(hSteamuser, hSteamPipe, pchVersion);
}

STEAMAPI_API ISteamInput * SteamAPI_ISteamClient_GetISteamInput( ISteamClient* self, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char * pchVersion )
{
    return get_steam_client()->GetISteamInput(hSteamUser, hSteamPipe, pchVersion);
}

STEAMAPI_API ISteamParties * SteamAPI_ISteamClient_GetISteamParties( ISteamClient* self, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char * pchVersion )
{
    return get_steam_client()->GetISteamParties(hSteamUser, hSteamPipe, pchVersion);
}

STEAMAPI_API ISteamRemotePlay * SteamAPI_ISteamClient_GetISteamRemotePlay( ISteamClient* self, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char * pchVersion )
{
    return get_steam_client()->GetISteamRemotePlay(hSteamUser, hSteamPipe, pchVersion);
}

STEAMAPI_API ISteamUser *SteamAPI_SteamUser_v023()
{
    return get_steam_client()->GetISteamUser(flat_hsteamuser(), flat_hsteampipe(), "SteamUser023");
}

STEAMAPI_API ISteamUser *SteamAPI_SteamUser_v022()
{
    return get_steam_client()->GetISteamUser(flat_hsteamuser(), flat_hsteampipe(), "SteamUser022");
}

STEAMAPI_API ISteamUser *SteamAPI_SteamUser_v021()
{
    return get_steam_client()->GetISteamUser(flat_hsteamuser(), flat_hsteampipe(), "SteamUser021");
}

STEAMAPI_API ISteamUser *SteamAPI_SteamUser_v020()
{
    return get_steam_client()->GetISteamUser(flat_hsteamuser(), flat_hsteampipe(), "SteamUser020");
}

STEAMAPI_API HSteamUser SteamAPI_ISteamUser_GetHSteamUser( ISteamUser* self )
{
    return (get_steam_client()->steam_user)->GetHSteamUser();
}

STEAMAPI_API steam_bool SteamAPI_ISteamUser_BLoggedOn( ISteamUser* self )
{
    return (get_steam_client()->steam_user)->BLoggedOn();
}

STEAMAPI_API uint64_steamid SteamAPI_ISteamUser_GetSteamID( ISteamUser* self )
{
    return (get_steam_client()->steam_user)->GetSteamID().ConvertToUint64();
}

STEAMAPI_API int SteamAPI_ISteamUser_InitiateGameConnection( ISteamUser* self, void * pAuthBlob, int cbMaxAuthBlob, uint64_steamid steamIDGameServer, uint32 unIPServer, uint16 usPortServer, bool bSecure )
{
    return (get_steam_client()->steam_user)->InitiateGameConnection(pAuthBlob, cbMaxAuthBlob, steamIDGameServer, unIPServer, usPortServer, bSecure);
}

STEAMAPI_API void SteamAPI_ISteamUser_TerminateGameConnection( ISteamUser* self, uint32 unIPServer, uint16 usPortServer )
{
    return (get_steam_client()->steam_user)->TerminateGameConnection(unIPServer, usPortServer);
}

STEAMAPI_API int SteamAPI_ISteamUser_InitiateGameConnection_DEPRECATED( ISteamUser* self, void * pAuthBlob, int cbMaxAuthBlob, uint64_steamid steamIDGameServer, uint32 unIPServer, uint16 usPortServer, bool bSecure )
{
    return (get_steam_client()->steam_user)->InitiateGameConnection(pAuthBlob, cbMaxAuthBlob, steamIDGameServer, unIPServer, usPortServer, bSecure);
}

STEAMAPI_API void SteamAPI_ISteamUser_TerminateGameConnection_DEPRECATED( ISteamUser* self, uint32 unIPServer, uint16 usPortServer )
{
    return (get_steam_client()->steam_user)->TerminateGameConnection(unIPServer, usPortServer);
}

STEAMAPI_API void SteamAPI_ISteamUser_TrackAppUsageEvent( ISteamUser* self, uint64_gameid gameID, int eAppUsageEvent, const char * pchExtraInfo )
{
    return (get_steam_client()->steam_user)->TrackAppUsageEvent(CGameID(gameID), eAppUsageEvent, pchExtraInfo);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUser_GetUserDataFolder( ISteamUser* self, char * pchBuffer, int cubBuffer )
{
    return (get_steam_client()->steam_user)->GetUserDataFolder(pchBuffer, cubBuffer);
}

STEAMAPI_API void SteamAPI_ISteamUser_StartVoiceRecording( ISteamUser* self )
{
    return (get_steam_client()->steam_user)->StartVoiceRecording();
}

STEAMAPI_API void SteamAPI_ISteamUser_StopVoiceRecording( ISteamUser* self )
{
    return (get_steam_client()->steam_user)->StopVoiceRecording();
}

STEAMAPI_API EVoiceResult SteamAPI_ISteamUser_GetAvailableVoice( ISteamUser* self, uint32 * pcbCompressed, uint32 * pcbUncompressed_Deprecated, uint32 nUncompressedVoiceDesiredSampleRate_Deprecated )
{
    return (get_steam_client()->steam_user)->GetAvailableVoice(pcbCompressed, pcbUncompressed_Deprecated, nUncompressedVoiceDesiredSampleRate_Deprecated);
}

STEAMAPI_API EVoiceResult SteamAPI_ISteamUser_GetVoice( ISteamUser* self, bool bWantCompressed, void * pDestBuffer, uint32 cbDestBufferSize, uint32 * nBytesWritten, bool bWantUncompressed_Deprecated, void * pUncompressedDestBuffer_Deprecated, uint32 cbUncompressedDestBufferSize_Deprecated, uint32 * nUncompressBytesWritten_Deprecated, uint32 nUncompressedVoiceDesiredSampleRate_Deprecated )
{
    return (get_steam_client()->steam_user)->GetVoice(bWantCompressed, pDestBuffer, cbDestBufferSize, nBytesWritten, bWantUncompressed_Deprecated, pUncompressedDestBuffer_Deprecated, cbUncompressedDestBufferSize_Deprecated, nUncompressBytesWritten_Deprecated, nUncompressedVoiceDesiredSampleRate_Deprecated);
}

STEAMAPI_API EVoiceResult SteamAPI_ISteamUser_DecompressVoice( ISteamUser* self, const void * pCompressed, uint32 cbCompressed, void * pDestBuffer, uint32 cbDestBufferSize, uint32 * nBytesWritten, uint32 nDesiredSampleRate )
{
    return (get_steam_client()->steam_user)->DecompressVoice(pCompressed, cbCompressed, pDestBuffer, cbDestBufferSize, nBytesWritten, nDesiredSampleRate);
}

STEAMAPI_API uint32 SteamAPI_ISteamUser_GetVoiceOptimalSampleRate( ISteamUser* self )
{
    return (get_steam_client()->steam_user)->GetVoiceOptimalSampleRate();
}

STEAMAPI_API HAuthTicket SteamAPI_ISteamUser_GetAuthSessionTicket( ISteamUser* self, void * pTicket, int cbMaxTicket, uint32 * pcbTicket, const SteamNetworkingIdentity * pSteamNetworkingIdentity)
{
    return (get_steam_client()->steam_user)->GetAuthSessionTicket(pTicket, cbMaxTicket, pcbTicket);
}

STEAMAPI_API HAuthTicket SteamAPI_ISteamUser_GetAuthTicketForWebApi( ISteamUser* self, const char * pchIdentity )
{
    return (get_steam_client()->steam_user)->GetAuthTicketForWebApi(pchIdentity);
}

STEAMAPI_API EBeginAuthSessionResult SteamAPI_ISteamUser_BeginAuthSession( ISteamUser* self, const void * pAuthTicket, int cbAuthTicket, uint64_steamid steamID )
{
    return (get_steam_client()->steam_user)->BeginAuthSession(pAuthTicket, cbAuthTicket, steamID);
}

STEAMAPI_API void SteamAPI_ISteamUser_EndAuthSession( ISteamUser* self, uint64_steamid steamID )
{
    return (get_steam_client()->steam_user)->EndAuthSession(steamID);
}

STEAMAPI_API void SteamAPI_ISteamUser_CancelAuthTicket( ISteamUser* self, HAuthTicket hAuthTicket )
{
    return (get_steam_client()->steam_user)->CancelAuthTicket(hAuthTicket);
}

STEAMAPI_API EUserHasLicenseForAppResult SteamAPI_ISteamUser_UserHasLicenseForApp( ISteamUser* self, uint64_steamid steamID, AppId_t appID )
{
    return (get_steam_client()->steam_user)->UserHasLicenseForApp(steamID, appID);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUser_BIsBehindNAT( ISteamUser* self )
{
    return (get_steam_client()->steam_user)->BIsBehindNAT();
}

STEAMAPI_API void SteamAPI_ISteamUser_AdvertiseGame( ISteamUser* self, uint64_steamid steamIDGameServer, uint32 unIPServer, uint16 usPortServer )
{
    return (get_steam_client()->steam_user)->AdvertiseGame(steamIDGameServer, unIPServer, usPortServer);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUser_RequestEncryptedAppTicket( ISteamUser* self, void * pDataToInclude, int cbDataToInclude )
{
    return (get_steam_client()->steam_user)->RequestEncryptedAppTicket(pDataToInclude, cbDataToInclude);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUser_GetEncryptedAppTicket( ISteamUser* self, void * pTicket, int cbMaxTicket, uint32 * pcbTicket )
{
    return (get_steam_client()->steam_user)->GetEncryptedAppTicket(pTicket, cbMaxTicket, pcbTicket);
}

STEAMAPI_API int SteamAPI_ISteamUser_GetGameBadgeLevel( ISteamUser* self, int nSeries, bool bFoil )
{
    return (get_steam_client()->steam_user)->GetGameBadgeLevel(nSeries, bFoil);
}

STEAMAPI_API int SteamAPI_ISteamUser_GetPlayerSteamLevel( ISteamUser* self )
{
    return (get_steam_client()->steam_user)->GetPlayerSteamLevel();
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUser_RequestStoreAuthURL( ISteamUser* self, const char * pchRedirectURL )
{
    return (get_steam_client()->steam_user)->RequestStoreAuthURL(pchRedirectURL);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUser_BIsPhoneVerified( ISteamUser* self )
{
    return (get_steam_client()->steam_user)->BIsPhoneVerified();
}

STEAMAPI_API steam_bool SteamAPI_ISteamUser_BIsTwoFactorEnabled( ISteamUser* self )
{
    return (get_steam_client()->steam_user)->BIsTwoFactorEnabled();
}

STEAMAPI_API steam_bool SteamAPI_ISteamUser_BIsPhoneIdentifying( ISteamUser* self )
{
    return (get_steam_client()->steam_user)->BIsPhoneIdentifying();
}

STEAMAPI_API steam_bool SteamAPI_ISteamUser_BIsPhoneRequiringVerification( ISteamUser* self )
{
    return (get_steam_client()->steam_user)->BIsPhoneRequiringVerification();
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUser_GetMarketEligibility( ISteamUser* self )
{
    return (get_steam_client()->steam_user)->GetMarketEligibility();
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUser_GetDurationControl( ISteamUser* self )
{
    return (get_steam_client()->steam_user)->GetDurationControl();
}

STEAMAPI_API steam_bool SteamAPI_ISteamUser_BSetDurationControlOnlineState( ISteamUser* self, EDurationControlOnlineState eNewState )
{
    return (get_steam_client()->steam_user)->BSetDurationControlOnlineState(eNewState);
}

STEAMAPI_API ISteamFriends *SteamAPI_SteamFriends_v017()
{
    return get_steam_client()->GetISteamFriends(flat_hsteamuser(), flat_hsteampipe(), "SteamFriends017");
}

STEAMAPI_API const char * SteamAPI_ISteamFriends_GetPersonaName( ISteamFriends* self )
{
    return (get_steam_client()->steam_friends)->GetPersonaName();
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamFriends_SetPersonaName( ISteamFriends* self, const char * pchPersonaName )
{
    return (get_steam_client()->steam_friends)->SetPersonaName(pchPersonaName);
}

STEAMAPI_API EPersonaState SteamAPI_ISteamFriends_GetPersonaState( ISteamFriends* self )
{
    return (get_steam_client()->steam_friends)->GetPersonaState();
}

STEAMAPI_API int SteamAPI_ISteamFriends_GetFriendCount( ISteamFriends* self, int iFriendFlags )
{
    return (get_steam_client()->steam_friends)->GetFriendCount(iFriendFlags);
}

STEAMAPI_API uint64_steamid SteamAPI_ISteamFriends_GetFriendByIndex( ISteamFriends* self, int iFriend, int iFriendFlags )
{
    return (get_steam_client()->steam_friends)->GetFriendByIndex(iFriend, iFriendFlags).ConvertToUint64();
}

STEAMAPI_API EFriendRelationship SteamAPI_ISteamFriends_GetFriendRelationship( ISteamFriends* self, uint64_steamid steamIDFriend )
{
    return (get_steam_client()->steam_friends)->GetFriendRelationship(steamIDFriend);
}

STEAMAPI_API EPersonaState SteamAPI_ISteamFriends_GetFriendPersonaState( ISteamFriends* self, uint64_steamid steamIDFriend )
{
    return (get_steam_client()->steam_friends)->GetFriendPersonaState(steamIDFriend);
}

STEAMAPI_API const char * SteamAPI_ISteamFriends_GetFriendPersonaName( ISteamFriends* self, uint64_steamid steamIDFriend )
{
    return (get_steam_client()->steam_friends)->GetFriendPersonaName(steamIDFriend);
}

STEAMAPI_API steam_bool SteamAPI_ISteamFriends_GetFriendGamePlayed( ISteamFriends* self, uint64_steamid steamIDFriend, FriendGameInfo_t * pFriendGameInfo )
{
    return (get_steam_client()->steam_friends)->GetFriendGamePlayed(steamIDFriend, pFriendGameInfo);
}

STEAMAPI_API const char * SteamAPI_ISteamFriends_GetFriendPersonaNameHistory( ISteamFriends* self, uint64_steamid steamIDFriend, int iPersonaName )
{
    return (get_steam_client()->steam_friends)->GetFriendPersonaNameHistory(steamIDFriend, iPersonaName);
}

STEAMAPI_API int SteamAPI_ISteamFriends_GetFriendSteamLevel( ISteamFriends* self, uint64_steamid steamIDFriend )
{
    return (get_steam_client()->steam_friends)->GetFriendSteamLevel(steamIDFriend);
}

STEAMAPI_API const char * SteamAPI_ISteamFriends_GetPlayerNickname( ISteamFriends* self, uint64_steamid steamIDPlayer )
{
    return (get_steam_client()->steam_friends)->GetPlayerNickname(steamIDPlayer);
}

STEAMAPI_API int SteamAPI_ISteamFriends_GetFriendsGroupCount( ISteamFriends* self )
{
    return (get_steam_client()->steam_friends)->GetFriendsGroupCount();
}

STEAMAPI_API FriendsGroupID_t SteamAPI_ISteamFriends_GetFriendsGroupIDByIndex( ISteamFriends* self, int iFG )
{
    return (get_steam_client()->steam_friends)->GetFriendsGroupIDByIndex(iFG);
}

STEAMAPI_API const char * SteamAPI_ISteamFriends_GetFriendsGroupName( ISteamFriends* self, FriendsGroupID_t friendsGroupID )
{
    return (get_steam_client()->steam_friends)->GetFriendsGroupName(friendsGroupID);
}

STEAMAPI_API int SteamAPI_ISteamFriends_GetFriendsGroupMembersCount( ISteamFriends* self, FriendsGroupID_t friendsGroupID )
{
    return (get_steam_client()->steam_friends)->GetFriendsGroupMembersCount(friendsGroupID);
}

STEAMAPI_API void SteamAPI_ISteamFriends_GetFriendsGroupMembersList( ISteamFriends* self, FriendsGroupID_t friendsGroupID, CSteamID * pOutSteamIDMembers, int nMembersCount )
{
    return (get_steam_client()->steam_friends)->GetFriendsGroupMembersList(friendsGroupID, pOutSteamIDMembers, nMembersCount);
}

STEAMAPI_API steam_bool SteamAPI_ISteamFriends_HasFriend( ISteamFriends* self, uint64_steamid steamIDFriend, int iFriendFlags )
{
    return (get_steam_client()->steam_friends)->HasFriend(steamIDFriend, iFriendFlags);
}

STEAMAPI_API int SteamAPI_ISteamFriends_GetClanCount( ISteamFriends* self )
{
    return (get_steam_client()->steam_friends)->GetClanCount();
}

STEAMAPI_API uint64_steamid SteamAPI_ISteamFriends_GetClanByIndex( ISteamFriends* self, int iClan )
{
    return (get_steam_client()->steam_friends)->GetClanByIndex(iClan).ConvertToUint64();
}

STEAMAPI_API const char * SteamAPI_ISteamFriends_GetClanName( ISteamFriends* self, uint64_steamid steamIDClan )
{
    return (get_steam_client()->steam_friends)->GetClanName(steamIDClan);
}

STEAMAPI_API const char * SteamAPI_ISteamFriends_GetClanTag( ISteamFriends* self, uint64_steamid steamIDClan )
{
    return (get_steam_client()->steam_friends)->GetClanTag(steamIDClan);
}

STEAMAPI_API steam_bool SteamAPI_ISteamFriends_GetClanActivityCounts( ISteamFriends* self, uint64_steamid steamIDClan, int * pnOnline, int * pnInGame, int * pnChatting )
{
    return (get_steam_client()->steam_friends)->GetClanActivityCounts(steamIDClan, pnOnline, pnInGame, pnChatting);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamFriends_DownloadClanActivityCounts( ISteamFriends* self, CSteamID * psteamIDClans, int cClansToRequest )
{
    return (get_steam_client()->steam_friends)->DownloadClanActivityCounts(psteamIDClans, cClansToRequest);
}

STEAMAPI_API int SteamAPI_ISteamFriends_GetFriendCountFromSource( ISteamFriends* self, uint64_steamid steamIDSource )
{
    return (get_steam_client()->steam_friends)->GetFriendCountFromSource(steamIDSource);
}

STEAMAPI_API uint64_steamid SteamAPI_ISteamFriends_GetFriendFromSourceByIndex( ISteamFriends* self, uint64_steamid steamIDSource, int iFriend )
{
    return (get_steam_client()->steam_friends)->GetFriendFromSourceByIndex(steamIDSource, iFriend).ConvertToUint64();
}

STEAMAPI_API steam_bool SteamAPI_ISteamFriends_IsUserInSource( ISteamFriends* self, uint64_steamid steamIDUser, uint64_steamid steamIDSource )
{
    return (get_steam_client()->steam_friends)->IsUserInSource(steamIDUser, steamIDSource);
}

STEAMAPI_API void SteamAPI_ISteamFriends_SetInGameVoiceSpeaking( ISteamFriends* self, uint64_steamid steamIDUser, bool bSpeaking )
{
    return (get_steam_client()->steam_friends)->SetInGameVoiceSpeaking(steamIDUser, bSpeaking);
}

STEAMAPI_API void SteamAPI_ISteamFriends_ActivateGameOverlay( ISteamFriends* self, const char * pchDialog )
{
    return (get_steam_client()->steam_friends)->ActivateGameOverlay(pchDialog);
}

STEAMAPI_API void SteamAPI_ISteamFriends_ActivateGameOverlayToUser( ISteamFriends* self, const char * pchDialog, uint64_steamid steamID )
{
    return (get_steam_client()->steam_friends)->ActivateGameOverlayToUser(pchDialog, steamID);
}

STEAMAPI_API void SteamAPI_ISteamFriends_ActivateGameOverlayToWebPage( ISteamFriends* self, const char * pchURL, EActivateGameOverlayToWebPageMode eMode )
{
    return (get_steam_client()->steam_friends)->ActivateGameOverlayToWebPage(pchURL, eMode);
}

STEAMAPI_API void SteamAPI_ISteamFriends_ActivateGameOverlayToStore( ISteamFriends* self, AppId_t nAppID, EOverlayToStoreFlag eFlag )
{
    return (get_steam_client()->steam_friends)->ActivateGameOverlayToStore(nAppID, eFlag);
}

STEAMAPI_API void SteamAPI_ISteamFriends_SetPlayedWith( ISteamFriends* self, uint64_steamid steamIDUserPlayedWith )
{
    return (get_steam_client()->steam_friends)->SetPlayedWith(steamIDUserPlayedWith);
}

STEAMAPI_API void SteamAPI_ISteamFriends_ActivateGameOverlayInviteDialog( ISteamFriends* self, uint64_steamid steamIDLobby )
{
    return (get_steam_client()->steam_friends)->ActivateGameOverlayInviteDialog(steamIDLobby);
}

STEAMAPI_API int SteamAPI_ISteamFriends_GetSmallFriendAvatar( ISteamFriends* self, uint64_steamid steamIDFriend )
{
    return (get_steam_client()->steam_friends)->GetSmallFriendAvatar(steamIDFriend);
}

STEAMAPI_API int SteamAPI_ISteamFriends_GetMediumFriendAvatar( ISteamFriends* self, uint64_steamid steamIDFriend )
{
    return (get_steam_client()->steam_friends)->GetMediumFriendAvatar(steamIDFriend);
}

STEAMAPI_API int SteamAPI_ISteamFriends_GetLargeFriendAvatar( ISteamFriends* self, uint64_steamid steamIDFriend )
{
    return (get_steam_client()->steam_friends)->GetLargeFriendAvatar(steamIDFriend);
}

STEAMAPI_API steam_bool SteamAPI_ISteamFriends_RequestUserInformation( ISteamFriends* self, uint64_steamid steamIDUser, bool bRequireNameOnly )
{
    return (get_steam_client()->steam_friends)->RequestUserInformation(steamIDUser, bRequireNameOnly);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamFriends_RequestClanOfficerList( ISteamFriends* self, uint64_steamid steamIDClan )
{
    return (get_steam_client()->steam_friends)->RequestClanOfficerList(steamIDClan);
}

STEAMAPI_API uint64_steamid SteamAPI_ISteamFriends_GetClanOwner( ISteamFriends* self, uint64_steamid steamIDClan )
{
    return (get_steam_client()->steam_friends)->GetClanOwner(steamIDClan).ConvertToUint64();
}

STEAMAPI_API int SteamAPI_ISteamFriends_GetClanOfficerCount( ISteamFriends* self, uint64_steamid steamIDClan )
{
    return (get_steam_client()->steam_friends)->GetClanOfficerCount(steamIDClan);
}

STEAMAPI_API uint64_steamid SteamAPI_ISteamFriends_GetClanOfficerByIndex( ISteamFriends* self, uint64_steamid steamIDClan, int iOfficer )
{
    return (get_steam_client()->steam_friends)->GetClanOfficerByIndex(steamIDClan, iOfficer).ConvertToUint64();
}

STEAMAPI_API uint32 SteamAPI_ISteamFriends_GetUserRestrictions( ISteamFriends* self )
{
    return (get_steam_client()->steam_friends)->GetUserRestrictions();
}

STEAMAPI_API steam_bool SteamAPI_ISteamFriends_SetRichPresence( ISteamFriends* self, const char * pchKey, const char * pchValue )
{
    return (get_steam_client()->steam_friends)->SetRichPresence(pchKey, pchValue);
}

STEAMAPI_API void SteamAPI_ISteamFriends_ClearRichPresence( ISteamFriends* self )
{
    return (get_steam_client()->steam_friends)->ClearRichPresence();
}

STEAMAPI_API const char * SteamAPI_ISteamFriends_GetFriendRichPresence( ISteamFriends* self, uint64_steamid steamIDFriend, const char * pchKey )
{
    return (get_steam_client()->steam_friends)->GetFriendRichPresence(steamIDFriend, pchKey);
}

STEAMAPI_API int SteamAPI_ISteamFriends_GetFriendRichPresenceKeyCount( ISteamFriends* self, uint64_steamid steamIDFriend )
{
    return (get_steam_client()->steam_friends)->GetFriendRichPresenceKeyCount(steamIDFriend);
}

STEAMAPI_API const char * SteamAPI_ISteamFriends_GetFriendRichPresenceKeyByIndex( ISteamFriends* self, uint64_steamid steamIDFriend, int iKey )
{
    return (get_steam_client()->steam_friends)->GetFriendRichPresenceKeyByIndex(steamIDFriend, iKey);
}

STEAMAPI_API void SteamAPI_ISteamFriends_RequestFriendRichPresence( ISteamFriends* self, uint64_steamid steamIDFriend )
{
    return (get_steam_client()->steam_friends)->RequestFriendRichPresence(steamIDFriend);
}

STEAMAPI_API steam_bool SteamAPI_ISteamFriends_InviteUserToGame( ISteamFriends* self, uint64_steamid steamIDFriend, const char * pchConnectString )
{
    return (get_steam_client()->steam_friends)->InviteUserToGame(steamIDFriend, pchConnectString);
}

STEAMAPI_API int SteamAPI_ISteamFriends_GetCoplayFriendCount( ISteamFriends* self )
{
    return (get_steam_client()->steam_friends)->GetCoplayFriendCount();
}

STEAMAPI_API uint64_steamid SteamAPI_ISteamFriends_GetCoplayFriend( ISteamFriends* self, int iCoplayFriend )
{
    return (get_steam_client()->steam_friends)->GetCoplayFriend(iCoplayFriend).ConvertToUint64();
}

STEAMAPI_API int SteamAPI_ISteamFriends_GetFriendCoplayTime( ISteamFriends* self, uint64_steamid steamIDFriend )
{
    return (get_steam_client()->steam_friends)->GetFriendCoplayTime(steamIDFriend);
}

STEAMAPI_API AppId_t SteamAPI_ISteamFriends_GetFriendCoplayGame( ISteamFriends* self, uint64_steamid steamIDFriend )
{
    return (get_steam_client()->steam_friends)->GetFriendCoplayGame(steamIDFriend);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamFriends_JoinClanChatRoom( ISteamFriends* self, uint64_steamid steamIDClan )
{
    return (get_steam_client()->steam_friends)->JoinClanChatRoom(steamIDClan);
}

STEAMAPI_API steam_bool SteamAPI_ISteamFriends_LeaveClanChatRoom( ISteamFriends* self, uint64_steamid steamIDClan )
{
    return (get_steam_client()->steam_friends)->LeaveClanChatRoom(steamIDClan);
}

STEAMAPI_API int SteamAPI_ISteamFriends_GetClanChatMemberCount( ISteamFriends* self, uint64_steamid steamIDClan )
{
    return (get_steam_client()->steam_friends)->GetClanChatMemberCount(steamIDClan);
}

STEAMAPI_API uint64_steamid SteamAPI_ISteamFriends_GetChatMemberByIndex( ISteamFriends* self, uint64_steamid steamIDClan, int iUser )
{
    return (get_steam_client()->steam_friends)->GetChatMemberByIndex(steamIDClan, iUser).ConvertToUint64();
}

STEAMAPI_API steam_bool SteamAPI_ISteamFriends_SendClanChatMessage( ISteamFriends* self, uint64_steamid steamIDClanChat, const char * pchText )
{
    return (get_steam_client()->steam_friends)->SendClanChatMessage(steamIDClanChat, pchText);
}

STEAMAPI_API int SteamAPI_ISteamFriends_GetClanChatMessage( ISteamFriends* self, uint64_steamid steamIDClanChat, int iMessage, void * prgchText, int cchTextMax, EChatEntryType * peChatEntryType, CSteamID * psteamidChatter )
{
    return (get_steam_client()->steam_friends)->GetClanChatMessage(steamIDClanChat, iMessage, prgchText, cchTextMax, peChatEntryType, psteamidChatter);
}

STEAMAPI_API steam_bool SteamAPI_ISteamFriends_IsClanChatAdmin( ISteamFriends* self, uint64_steamid steamIDClanChat, uint64_steamid steamIDUser )
{
    return (get_steam_client()->steam_friends)->IsClanChatAdmin(steamIDClanChat, steamIDUser);
}

STEAMAPI_API steam_bool SteamAPI_ISteamFriends_IsClanChatWindowOpenInSteam( ISteamFriends* self, uint64_steamid steamIDClanChat )
{
    return (get_steam_client()->steam_friends)->IsClanChatWindowOpenInSteam(steamIDClanChat);
}

STEAMAPI_API steam_bool SteamAPI_ISteamFriends_OpenClanChatWindowInSteam( ISteamFriends* self, uint64_steamid steamIDClanChat )
{
    return (get_steam_client()->steam_friends)->OpenClanChatWindowInSteam(steamIDClanChat);
}

STEAMAPI_API steam_bool SteamAPI_ISteamFriends_CloseClanChatWindowInSteam( ISteamFriends* self, uint64_steamid steamIDClanChat )
{
    return (get_steam_client()->steam_friends)->CloseClanChatWindowInSteam(steamIDClanChat);
}

STEAMAPI_API steam_bool SteamAPI_ISteamFriends_SetListenForFriendsMessages( ISteamFriends* self, bool bInterceptEnabled )
{
    return (get_steam_client()->steam_friends)->SetListenForFriendsMessages(bInterceptEnabled);
}

STEAMAPI_API steam_bool SteamAPI_ISteamFriends_ReplyToFriendMessage( ISteamFriends* self, uint64_steamid steamIDFriend, const char * pchMsgToSend )
{
    return (get_steam_client()->steam_friends)->ReplyToFriendMessage(steamIDFriend, pchMsgToSend);
}

STEAMAPI_API int SteamAPI_ISteamFriends_GetFriendMessage( ISteamFriends* self, uint64_steamid steamIDFriend, int iMessageID, void * pvData, int cubData, EChatEntryType * peChatEntryType )
{
    return (get_steam_client()->steam_friends)->GetFriendMessage(steamIDFriend, iMessageID, pvData, cubData, peChatEntryType);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamFriends_GetFollowerCount( ISteamFriends* self, uint64_steamid steamID )
{
    return (get_steam_client()->steam_friends)->GetFollowerCount(steamID);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamFriends_IsFollowing( ISteamFriends* self, uint64_steamid steamID )
{
    return (get_steam_client()->steam_friends)->IsFollowing(steamID);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamFriends_EnumerateFollowingList( ISteamFriends* self, uint32 unStartIndex )
{
    return (get_steam_client()->steam_friends)->EnumerateFollowingList(unStartIndex);
}

STEAMAPI_API steam_bool SteamAPI_ISteamFriends_IsClanPublic( ISteamFriends* self, uint64_steamid steamIDClan )
{
    return (get_steam_client()->steam_friends)->IsClanPublic(steamIDClan);
}

STEAMAPI_API steam_bool SteamAPI_ISteamFriends_IsClanOfficialGameGroup( ISteamFriends* self, uint64_steamid steamIDClan )
{
    return (get_steam_client()->steam_friends)->IsClanOfficialGameGroup(steamIDClan);
}

STEAMAPI_API int SteamAPI_ISteamFriends_GetNumChatsWithUnreadPriorityMessages( ISteamFriends* self )
{
    return (get_steam_client()->steam_friends)->GetNumChatsWithUnreadPriorityMessages();
}

STEAMAPI_API void SteamAPI_ISteamFriends_ActivateGameOverlayRemotePlayTogetherInviteDialog( ISteamFriends* self, uint64_steamid steamIDLobby )
{
    return (get_steam_client()->steam_friends)->ActivateGameOverlayRemotePlayTogetherInviteDialog(steamIDLobby);
}

STEAMAPI_API steam_bool SteamAPI_ISteamFriends_RegisterProtocolInOverlayBrowser( ISteamFriends* self, const char * pchProtocol )
{
    return (get_steam_client()->steam_friends)->RegisterProtocolInOverlayBrowser(pchProtocol);
}

STEAMAPI_API void SteamAPI_ISteamFriends_ActivateGameOverlayInviteDialogConnectString( ISteamFriends* self, const char * pchConnectString )
{
    return (get_steam_client()->steam_friends)->ActivateGameOverlayInviteDialogConnectString(pchConnectString);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamFriends_RequestEquippedProfileItems( ISteamFriends* self, uint64_steamid steamID )
{
    return (get_steam_client()->steam_friends)->RequestEquippedProfileItems(steamID);
}

STEAMAPI_API steam_bool SteamAPI_ISteamFriends_BHasEquippedProfileItem( ISteamFriends* self, uint64_steamid steamID, ECommunityProfileItemType itemType )
{
    return (get_steam_client()->steam_friends)->BHasEquippedProfileItem(steamID, itemType);
}

STEAMAPI_API const char * SteamAPI_ISteamFriends_GetProfileItemPropertyString( ISteamFriends* self, uint64_steamid steamID, ECommunityProfileItemType itemType, ECommunityProfileItemProperty prop )
{
    return (get_steam_client()->steam_friends)->GetProfileItemPropertyString(steamID, itemType, prop);
}

STEAMAPI_API uint32 SteamAPI_ISteamFriends_GetProfileItemPropertyUint( ISteamFriends* self, uint64_steamid steamID, ECommunityProfileItemType itemType, ECommunityProfileItemProperty prop )
{
    return (get_steam_client()->steam_friends)->GetProfileItemPropertyUint(steamID, itemType, prop);
}

STEAMAPI_API ISteamUtils *SteamAPI_SteamUtils_v010()
{
    return get_steam_client()->GetISteamUtils(flat_hsteampipe(), "SteamUtils010");
}

STEAMAPI_API ISteamUtils *SteamAPI_SteamGameServerUtils_v010()
{
    return get_steam_client()->GetISteamUtils(flat_gs_hsteampipe(), "SteamUtils010");
}

STEAMAPI_API ISteamUtils *SteamAPI_SteamUtils_v009()
{
    return get_steam_client()->GetISteamUtils(flat_hsteampipe(), "SteamUtils009");
}

STEAMAPI_API ISteamUtils *SteamAPI_SteamGameServerUtils_v009()
{
    return get_steam_client()->GetISteamUtils(flat_gs_hsteampipe(), "SteamUtils009");
}

STEAMAPI_API uint32 SteamAPI_ISteamUtils_GetSecondsSinceAppActive( ISteamUtils* self )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->GetSecondsSinceAppActive();
}

STEAMAPI_API uint32 SteamAPI_ISteamUtils_GetSecondsSinceComputerActive( ISteamUtils* self )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->GetSecondsSinceComputerActive();
}

STEAMAPI_API EUniverse SteamAPI_ISteamUtils_GetConnectedUniverse( ISteamUtils* self )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->GetConnectedUniverse();
}

STEAMAPI_API uint32 SteamAPI_ISteamUtils_GetServerRealTime( ISteamUtils* self )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->GetServerRealTime();
}

STEAMAPI_API const char * SteamAPI_ISteamUtils_GetIPCountry( ISteamUtils* self )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->GetIPCountry();
}

STEAMAPI_API steam_bool SteamAPI_ISteamUtils_GetImageSize( ISteamUtils* self, int iImage, uint32 * pnWidth, uint32 * pnHeight )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->GetImageSize(iImage, pnWidth, pnHeight);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUtils_GetImageRGBA( ISteamUtils* self, int iImage, uint8 * pubDest, int nDestBufferSize )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->GetImageRGBA(iImage, pubDest, nDestBufferSize);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUtils_GetCSERIPPort( ISteamUtils* self, uint32 * unIP, uint16 * usPort )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->GetCSERIPPort(unIP, usPort);
}

STEAMAPI_API uint8 SteamAPI_ISteamUtils_GetCurrentBatteryPower( ISteamUtils* self )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->GetCurrentBatteryPower();
}

STEAMAPI_API uint32 SteamAPI_ISteamUtils_GetAppID( ISteamUtils* self )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->GetAppID();
}

STEAMAPI_API void SteamAPI_ISteamUtils_SetOverlayNotificationPosition( ISteamUtils* self, ENotificationPosition eNotificationPosition )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->SetOverlayNotificationPosition(eNotificationPosition);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUtils_IsAPICallCompleted( ISteamUtils* self, SteamAPICall_t hSteamAPICall, bool * pbFailed )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->IsAPICallCompleted(hSteamAPICall, pbFailed);
}

STEAMAPI_API ESteamAPICallFailure SteamAPI_ISteamUtils_GetAPICallFailureReason( ISteamUtils* self, SteamAPICall_t hSteamAPICall )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->GetAPICallFailureReason(hSteamAPICall);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUtils_GetAPICallResult( ISteamUtils* self, SteamAPICall_t hSteamAPICall, void * pCallback, int cubCallback, int iCallbackExpected, bool * pbFailed )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->GetAPICallResult(hSteamAPICall, pCallback, cubCallback, iCallbackExpected, pbFailed);
}

STEAMAPI_API uint32 SteamAPI_ISteamUtils_GetIPCCallCount( ISteamUtils* self )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->GetIPCCallCount();
}

STEAMAPI_API void SteamAPI_ISteamUtils_SetWarningMessageHook( ISteamUtils* self, SteamAPIWarningMessageHook_t pFunction )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->SetWarningMessageHook(pFunction);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUtils_IsOverlayEnabled( ISteamUtils* self )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->IsOverlayEnabled();
}

STEAMAPI_API steam_bool SteamAPI_ISteamUtils_BOverlayNeedsPresent( ISteamUtils* self )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->BOverlayNeedsPresent();
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUtils_CheckFileSignature( ISteamUtils* self, const char * szFileName )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->CheckFileSignature(szFileName);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUtils_ShowGamepadTextInput( ISteamUtils* self, EGamepadTextInputMode eInputMode, EGamepadTextInputLineMode eLineInputMode, const char * pchDescription, uint32 unCharMax, const char * pchExistingText )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->ShowGamepadTextInput(eInputMode, eLineInputMode, pchDescription, unCharMax, pchExistingText);
}

STEAMAPI_API uint32 SteamAPI_ISteamUtils_GetEnteredGamepadTextLength( ISteamUtils* self )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->GetEnteredGamepadTextLength();
}

STEAMAPI_API steam_bool SteamAPI_ISteamUtils_GetEnteredGamepadTextInput( ISteamUtils* self, char * pchText, uint32 cchText )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->GetEnteredGamepadTextInput(pchText, cchText);
}

STEAMAPI_API const char * SteamAPI_ISteamUtils_GetSteamUILanguage( ISteamUtils* self )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->GetSteamUILanguage();
}

STEAMAPI_API steam_bool SteamAPI_ISteamUtils_IsSteamRunningInVR( ISteamUtils* self )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->IsSteamRunningInVR();
}

STEAMAPI_API void SteamAPI_ISteamUtils_SetOverlayNotificationInset( ISteamUtils* self, int nHorizontalInset, int nVerticalInset )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->SetOverlayNotificationInset(nHorizontalInset, nVerticalInset);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUtils_IsSteamInBigPictureMode( ISteamUtils* self )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->IsSteamInBigPictureMode();
}

STEAMAPI_API void SteamAPI_ISteamUtils_StartVRDashboard( ISteamUtils* self )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->StartVRDashboard();
}

STEAMAPI_API steam_bool SteamAPI_ISteamUtils_IsVRHeadsetStreamingEnabled( ISteamUtils* self )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->IsVRHeadsetStreamingEnabled();
}

STEAMAPI_API void SteamAPI_ISteamUtils_SetVRHeadsetStreamingEnabled( ISteamUtils* self, bool bEnabled )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->SetVRHeadsetStreamingEnabled(bEnabled);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUtils_IsSteamChinaLauncher( ISteamUtils* self )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->IsSteamChinaLauncher();
}

STEAMAPI_API steam_bool SteamAPI_ISteamUtils_InitFilterText( ISteamUtils* self, uint32 unFilterOptions )
{
    //Note: older function only has less arguments
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->InitFilterText(unFilterOptions);
}

STEAMAPI_API int SteamAPI_ISteamUtils_FilterText( ISteamUtils* self, ETextFilteringContext eContext, uint64_steamid sourceSteamID, const char * pchInputMessage, char * pchOutFilteredText, uint32 nByteSizeOutFilteredText )
{
    //Note: older function only has less arguments
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->FilterText(eContext, sourceSteamID, pchInputMessage, pchOutFilteredText, nByteSizeOutFilteredText);
}

STEAMAPI_API ESteamIPv6ConnectivityState SteamAPI_ISteamUtils_GetIPv6ConnectivityState( ISteamUtils* self, ESteamIPv6ConnectivityProtocol eProtocol )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->GetIPv6ConnectivityState(eProtocol);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUtils_IsSteamRunningOnSteamDeck( ISteamUtils* self )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->IsSteamRunningOnSteamDeck();
}

STEAMAPI_API steam_bool SteamAPI_ISteamUtils_ShowModalGamepadTextInput( ISteamUtils* self, EGamepadTextInputLineMode eLineInputMode )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    //flat vs normal api mismatch
    return (ptr)->ShowFloatingGamepadTextInput((EFloatingGamepadTextInputMode)eLineInputMode, 0, 0, 0, 0);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUtils_ShowFloatingGamepadTextInput( ISteamUtils* self, EFloatingGamepadTextInputMode eKeyboardMode, int nTextFieldXPosition, int nTextFieldYPosition, int nTextFieldWidth, int nTextFieldHeight )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    //flat vs normal api mismatch
    return (ptr)->ShowFloatingGamepadTextInput( eKeyboardMode, nTextFieldXPosition, nTextFieldYPosition, nTextFieldWidth, nTextFieldHeight);
}

STEAMAPI_API void SteamAPI_ISteamUtils_SetGameLauncherMode( ISteamUtils* self, bool bLauncherMode )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->SetGameLauncherMode(bLauncherMode);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUtils_DismissFloatingGamepadTextInput( ISteamUtils* self )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->DismissFloatingGamepadTextInput();
}

STEAMAPI_API steam_bool SteamAPI_ISteamUtils_DismissGamepadTextInput( ISteamUtils* self )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_utils);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_utils);
    auto ptr = get_steam_client()->steam_gameserver_utils;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_utils;
    }

    return (ptr)->DismissGamepadTextInput();
}

STEAMAPI_API ISteamMatchmaking *SteamAPI_SteamMatchmaking_v009()
{
    return get_steam_client()->GetISteamMatchmaking(flat_hsteamuser(), flat_hsteampipe(), "SteamMatchMaking009");
}

STEAMAPI_API int SteamAPI_ISteamMatchmaking_GetFavoriteGameCount( ISteamMatchmaking* self )
{
    return (get_steam_client()->steam_matchmaking)->GetFavoriteGameCount();
}

STEAMAPI_API steam_bool SteamAPI_ISteamMatchmaking_GetFavoriteGame( ISteamMatchmaking* self, int iGame, AppId_t * pnAppID, uint32 * pnIP, uint16 * pnConnPort, uint16 * pnQueryPort, uint32 * punFlags, uint32 * pRTime32LastPlayedOnServer )
{
    return (get_steam_client()->steam_matchmaking)->GetFavoriteGame(iGame, pnAppID, pnIP, pnConnPort, pnQueryPort, punFlags, pRTime32LastPlayedOnServer);
}

STEAMAPI_API int SteamAPI_ISteamMatchmaking_AddFavoriteGame( ISteamMatchmaking* self, AppId_t nAppID, uint32 nIP, uint16 nConnPort, uint16 nQueryPort, uint32 unFlags, uint32 rTime32LastPlayedOnServer )
{
    return (get_steam_client()->steam_matchmaking)->AddFavoriteGame(nAppID, nIP, nConnPort, nQueryPort, unFlags, rTime32LastPlayedOnServer);
}

STEAMAPI_API steam_bool SteamAPI_ISteamMatchmaking_RemoveFavoriteGame( ISteamMatchmaking* self, AppId_t nAppID, uint32 nIP, uint16 nConnPort, uint16 nQueryPort, uint32 unFlags )
{
    return (get_steam_client()->steam_matchmaking)->RemoveFavoriteGame(nAppID, nIP, nConnPort, nQueryPort, unFlags);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamMatchmaking_RequestLobbyList( ISteamMatchmaking* self )
{
    return (get_steam_client()->steam_matchmaking)->RequestLobbyList();
}

STEAMAPI_API void SteamAPI_ISteamMatchmaking_AddRequestLobbyListStringFilter( ISteamMatchmaking* self, const char * pchKeyToMatch, const char * pchValueToMatch, ELobbyComparison eComparisonType )
{
    return (get_steam_client()->steam_matchmaking)->AddRequestLobbyListStringFilter(pchKeyToMatch, pchValueToMatch, eComparisonType);
}

STEAMAPI_API void SteamAPI_ISteamMatchmaking_AddRequestLobbyListNumericalFilter( ISteamMatchmaking* self, const char * pchKeyToMatch, int nValueToMatch, ELobbyComparison eComparisonType )
{
    return (get_steam_client()->steam_matchmaking)->AddRequestLobbyListNumericalFilter(pchKeyToMatch, nValueToMatch, eComparisonType);
}

STEAMAPI_API void SteamAPI_ISteamMatchmaking_AddRequestLobbyListNearValueFilter( ISteamMatchmaking* self, const char * pchKeyToMatch, int nValueToBeCloseTo )
{
    return (get_steam_client()->steam_matchmaking)->AddRequestLobbyListNearValueFilter(pchKeyToMatch, nValueToBeCloseTo);
}

STEAMAPI_API void SteamAPI_ISteamMatchmaking_AddRequestLobbyListFilterSlotsAvailable( ISteamMatchmaking* self, int nSlotsAvailable )
{
    return (get_steam_client()->steam_matchmaking)->AddRequestLobbyListFilterSlotsAvailable(nSlotsAvailable);
}

STEAMAPI_API void SteamAPI_ISteamMatchmaking_AddRequestLobbyListDistanceFilter( ISteamMatchmaking* self, ELobbyDistanceFilter eLobbyDistanceFilter )
{
    return (get_steam_client()->steam_matchmaking)->AddRequestLobbyListDistanceFilter(eLobbyDistanceFilter);
}

STEAMAPI_API void SteamAPI_ISteamMatchmaking_AddRequestLobbyListResultCountFilter( ISteamMatchmaking* self, int cMaxResults )
{
    return (get_steam_client()->steam_matchmaking)->AddRequestLobbyListResultCountFilter(cMaxResults);
}

STEAMAPI_API void SteamAPI_ISteamMatchmaking_AddRequestLobbyListCompatibleMembersFilter( ISteamMatchmaking* self, uint64_steamid steamIDLobby )
{
    return (get_steam_client()->steam_matchmaking)->AddRequestLobbyListCompatibleMembersFilter(steamIDLobby);
}

STEAMAPI_API uint64_steamid SteamAPI_ISteamMatchmaking_GetLobbyByIndex( ISteamMatchmaking* self, int iLobby )
{
    return (get_steam_client()->steam_matchmaking)->GetLobbyByIndex(iLobby).ConvertToUint64();
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamMatchmaking_CreateLobby( ISteamMatchmaking* self, ELobbyType eLobbyType, int cMaxMembers )
{
    return (get_steam_client()->steam_matchmaking)->CreateLobby(eLobbyType, cMaxMembers);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamMatchmaking_JoinLobby( ISteamMatchmaking* self, uint64_steamid steamIDLobby )
{
    return (get_steam_client()->steam_matchmaking)->JoinLobby(steamIDLobby);
}

STEAMAPI_API void SteamAPI_ISteamMatchmaking_LeaveLobby( ISteamMatchmaking* self, uint64_steamid steamIDLobby )
{
    return (get_steam_client()->steam_matchmaking)->LeaveLobby(steamIDLobby);
}

STEAMAPI_API steam_bool SteamAPI_ISteamMatchmaking_InviteUserToLobby( ISteamMatchmaking* self, uint64_steamid steamIDLobby, uint64_steamid steamIDInvitee )
{
    return (get_steam_client()->steam_matchmaking)->InviteUserToLobby(steamIDLobby, steamIDInvitee);
}

STEAMAPI_API int SteamAPI_ISteamMatchmaking_GetNumLobbyMembers( ISteamMatchmaking* self, uint64_steamid steamIDLobby )
{
    return (get_steam_client()->steam_matchmaking)->GetNumLobbyMembers(steamIDLobby);
}

STEAMAPI_API uint64_steamid SteamAPI_ISteamMatchmaking_GetLobbyMemberByIndex( ISteamMatchmaking* self, uint64_steamid steamIDLobby, int iMember )
{
    return (get_steam_client()->steam_matchmaking)->GetLobbyMemberByIndex(steamIDLobby, iMember).ConvertToUint64();
}

STEAMAPI_API const char * SteamAPI_ISteamMatchmaking_GetLobbyData( ISteamMatchmaking* self, uint64_steamid steamIDLobby, const char * pchKey )
{
    return (get_steam_client()->steam_matchmaking)->GetLobbyData(steamIDLobby, pchKey);
}

STEAMAPI_API steam_bool SteamAPI_ISteamMatchmaking_SetLobbyData( ISteamMatchmaking* self, uint64_steamid steamIDLobby, const char * pchKey, const char * pchValue )
{
    return (get_steam_client()->steam_matchmaking)->SetLobbyData(steamIDLobby, pchKey, pchValue);
}

STEAMAPI_API int SteamAPI_ISteamMatchmaking_GetLobbyDataCount( ISteamMatchmaking* self, uint64_steamid steamIDLobby )
{
    return (get_steam_client()->steam_matchmaking)->GetLobbyDataCount(steamIDLobby);
}

STEAMAPI_API steam_bool SteamAPI_ISteamMatchmaking_GetLobbyDataByIndex( ISteamMatchmaking* self, uint64_steamid steamIDLobby, int iLobbyData, char * pchKey, int cchKeyBufferSize, char * pchValue, int cchValueBufferSize )
{
    return (get_steam_client()->steam_matchmaking)->GetLobbyDataByIndex(steamIDLobby, iLobbyData, pchKey, cchKeyBufferSize, pchValue, cchValueBufferSize);
}

STEAMAPI_API steam_bool SteamAPI_ISteamMatchmaking_DeleteLobbyData( ISteamMatchmaking* self, uint64_steamid steamIDLobby, const char * pchKey )
{
    return (get_steam_client()->steam_matchmaking)->DeleteLobbyData(steamIDLobby, pchKey);
}

STEAMAPI_API const char * SteamAPI_ISteamMatchmaking_GetLobbyMemberData( ISteamMatchmaking* self, uint64_steamid steamIDLobby, uint64_steamid steamIDUser, const char * pchKey )
{
    return (get_steam_client()->steam_matchmaking)->GetLobbyMemberData(steamIDLobby, steamIDUser, pchKey);
}

STEAMAPI_API void SteamAPI_ISteamMatchmaking_SetLobbyMemberData( ISteamMatchmaking* self, uint64_steamid steamIDLobby, const char * pchKey, const char * pchValue )
{
    return (get_steam_client()->steam_matchmaking)->SetLobbyMemberData(steamIDLobby, pchKey, pchValue);
}

STEAMAPI_API steam_bool SteamAPI_ISteamMatchmaking_SendLobbyChatMsg( ISteamMatchmaking* self, uint64_steamid steamIDLobby, const void * pvMsgBody, int cubMsgBody )
{
    return (get_steam_client()->steam_matchmaking)->SendLobbyChatMsg(steamIDLobby, pvMsgBody, cubMsgBody);
}

STEAMAPI_API int SteamAPI_ISteamMatchmaking_GetLobbyChatEntry( ISteamMatchmaking* self, uint64_steamid steamIDLobby, int iChatID, CSteamID * pSteamIDUser, void * pvData, int cubData, EChatEntryType * peChatEntryType )
{
    return (get_steam_client()->steam_matchmaking)->GetLobbyChatEntry(steamIDLobby, iChatID, pSteamIDUser, pvData, cubData, peChatEntryType);
}

STEAMAPI_API steam_bool SteamAPI_ISteamMatchmaking_RequestLobbyData( ISteamMatchmaking* self, uint64_steamid steamIDLobby )
{
    return (get_steam_client()->steam_matchmaking)->RequestLobbyData(steamIDLobby);
}

STEAMAPI_API void SteamAPI_ISteamMatchmaking_SetLobbyGameServer( ISteamMatchmaking* self, uint64_steamid steamIDLobby, uint32 unGameServerIP, uint16 unGameServerPort, uint64_steamid steamIDGameServer )
{
    return (get_steam_client()->steam_matchmaking)->SetLobbyGameServer(steamIDLobby, unGameServerIP, unGameServerPort, steamIDGameServer);
}

STEAMAPI_API steam_bool SteamAPI_ISteamMatchmaking_GetLobbyGameServer( ISteamMatchmaking* self, uint64_steamid steamIDLobby, uint32 * punGameServerIP, uint16 * punGameServerPort, CSteamID * psteamIDGameServer )
{
    return (get_steam_client()->steam_matchmaking)->GetLobbyGameServer(steamIDLobby, punGameServerIP, punGameServerPort, psteamIDGameServer);
}

STEAMAPI_API steam_bool SteamAPI_ISteamMatchmaking_SetLobbyMemberLimit( ISteamMatchmaking* self, uint64_steamid steamIDLobby, int cMaxMembers )
{
    return (get_steam_client()->steam_matchmaking)->SetLobbyMemberLimit(steamIDLobby, cMaxMembers);
}

STEAMAPI_API int SteamAPI_ISteamMatchmaking_GetLobbyMemberLimit( ISteamMatchmaking* self, uint64_steamid steamIDLobby )
{
    return (get_steam_client()->steam_matchmaking)->GetLobbyMemberLimit(steamIDLobby);
}

STEAMAPI_API steam_bool SteamAPI_ISteamMatchmaking_SetLobbyType( ISteamMatchmaking* self, uint64_steamid steamIDLobby, ELobbyType eLobbyType )
{
    return (get_steam_client()->steam_matchmaking)->SetLobbyType(steamIDLobby, eLobbyType);
}

STEAMAPI_API steam_bool SteamAPI_ISteamMatchmaking_SetLobbyJoinable( ISteamMatchmaking* self, uint64_steamid steamIDLobby, bool bLobbyJoinable )
{
    return (get_steam_client()->steam_matchmaking)->SetLobbyJoinable(steamIDLobby, bLobbyJoinable);
}

STEAMAPI_API uint64_steamid SteamAPI_ISteamMatchmaking_GetLobbyOwner( ISteamMatchmaking* self, uint64_steamid steamIDLobby )
{
    return (get_steam_client()->steam_matchmaking)->GetLobbyOwner(steamIDLobby).ConvertToUint64();
}

STEAMAPI_API steam_bool SteamAPI_ISteamMatchmaking_SetLobbyOwner( ISteamMatchmaking* self, uint64_steamid steamIDLobby, uint64_steamid steamIDNewOwner )
{
    return (get_steam_client()->steam_matchmaking)->SetLobbyOwner(steamIDLobby, steamIDNewOwner);
}

STEAMAPI_API steam_bool SteamAPI_ISteamMatchmaking_SetLinkedLobby( ISteamMatchmaking* self, uint64_steamid steamIDLobby, uint64_steamid steamIDLobbyDependent )
{
    return (get_steam_client()->steam_matchmaking)->SetLinkedLobby(steamIDLobby, steamIDLobbyDependent);
}

STEAMAPI_API void SteamAPI_ISteamMatchmakingServerListResponse_ServerResponded( ISteamMatchmakingServerListResponse* self, HServerListRequest hRequest, int iServer )
{
    return self->ServerResponded(hRequest, iServer);
}

STEAMAPI_API void SteamAPI_ISteamMatchmakingServerListResponse_ServerFailedToRespond( ISteamMatchmakingServerListResponse* self, HServerListRequest hRequest, int iServer )
{
    return self->ServerFailedToRespond(hRequest, iServer);
}

STEAMAPI_API void SteamAPI_ISteamMatchmakingServerListResponse_RefreshComplete( ISteamMatchmakingServerListResponse* self, HServerListRequest hRequest, EMatchMakingServerResponse response )
{
    return self->RefreshComplete(hRequest, response);
}

STEAMAPI_API void SteamAPI_ISteamMatchmakingPingResponse_ServerResponded( ISteamMatchmakingPingResponse* self, gameserveritem_t & server )
{
    return self->ServerResponded(server);
}

STEAMAPI_API void SteamAPI_ISteamMatchmakingPingResponse_ServerFailedToRespond( ISteamMatchmakingPingResponse* self )
{
    return self->ServerFailedToRespond();
}

STEAMAPI_API void SteamAPI_ISteamMatchmakingPlayersResponse_AddPlayerToList( ISteamMatchmakingPlayersResponse* self, const char * pchName, int nScore, float flTimePlayed )
{
    return self->AddPlayerToList(pchName, nScore, flTimePlayed);
}

STEAMAPI_API void SteamAPI_ISteamMatchmakingPlayersResponse_PlayersFailedToRespond( ISteamMatchmakingPlayersResponse* self )
{
    return self->PlayersFailedToRespond();
}

STEAMAPI_API void SteamAPI_ISteamMatchmakingPlayersResponse_PlayersRefreshComplete( ISteamMatchmakingPlayersResponse* self )
{
    return self->PlayersRefreshComplete();
}

STEAMAPI_API void SteamAPI_ISteamMatchmakingRulesResponse_RulesResponded( ISteamMatchmakingRulesResponse* self, const char * pchRule, const char * pchValue )
{
    return self->RulesResponded(pchRule, pchValue);
}

STEAMAPI_API void SteamAPI_ISteamMatchmakingRulesResponse_RulesFailedToRespond( ISteamMatchmakingRulesResponse* self )
{
    return self->RulesFailedToRespond();
}

STEAMAPI_API void SteamAPI_ISteamMatchmakingRulesResponse_RulesRefreshComplete( ISteamMatchmakingRulesResponse* self )
{
    return self->RulesRefreshComplete();
}

STEAMAPI_API ISteamMatchmakingServers *SteamAPI_SteamMatchmakingServers_v002()
{
    return get_steam_client()->GetISteamMatchmakingServers(flat_hsteamuser(), flat_hsteampipe(), "SteamMatchMakingServers002");
}

STEAMAPI_API HServerListRequest SteamAPI_ISteamMatchmakingServers_RequestInternetServerList( ISteamMatchmakingServers* self, AppId_t iApp, MatchMakingKeyValuePair_t ** ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse * pRequestServersResponse )
{
    return (get_steam_client()->steam_matchmaking_servers)->RequestInternetServerList(iApp, ppchFilters, nFilters, pRequestServersResponse);
}

STEAMAPI_API HServerListRequest SteamAPI_ISteamMatchmakingServers_RequestLANServerList( ISteamMatchmakingServers* self, AppId_t iApp, ISteamMatchmakingServerListResponse * pRequestServersResponse )
{
    return (get_steam_client()->steam_matchmaking_servers)->RequestLANServerList(iApp, pRequestServersResponse);
}

STEAMAPI_API HServerListRequest SteamAPI_ISteamMatchmakingServers_RequestFriendsServerList( ISteamMatchmakingServers* self, AppId_t iApp, MatchMakingKeyValuePair_t ** ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse * pRequestServersResponse )
{
    return (get_steam_client()->steam_matchmaking_servers)->RequestFriendsServerList(iApp, ppchFilters, nFilters, pRequestServersResponse);
}

STEAMAPI_API HServerListRequest SteamAPI_ISteamMatchmakingServers_RequestFavoritesServerList( ISteamMatchmakingServers* self, AppId_t iApp, MatchMakingKeyValuePair_t ** ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse * pRequestServersResponse )
{
    return (get_steam_client()->steam_matchmaking_servers)->RequestFavoritesServerList(iApp, ppchFilters, nFilters, pRequestServersResponse);
}

STEAMAPI_API HServerListRequest SteamAPI_ISteamMatchmakingServers_RequestHistoryServerList( ISteamMatchmakingServers* self, AppId_t iApp, MatchMakingKeyValuePair_t ** ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse * pRequestServersResponse )
{
    return (get_steam_client()->steam_matchmaking_servers)->RequestHistoryServerList(iApp, ppchFilters, nFilters, pRequestServersResponse);
}

STEAMAPI_API HServerListRequest SteamAPI_ISteamMatchmakingServers_RequestSpectatorServerList( ISteamMatchmakingServers* self, AppId_t iApp, MatchMakingKeyValuePair_t ** ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse * pRequestServersResponse )
{
    return (get_steam_client()->steam_matchmaking_servers)->RequestSpectatorServerList(iApp, ppchFilters, nFilters, pRequestServersResponse);
}

STEAMAPI_API void SteamAPI_ISteamMatchmakingServers_ReleaseRequest( ISteamMatchmakingServers* self, HServerListRequest hServerListRequest )
{
    return (get_steam_client()->steam_matchmaking_servers)->ReleaseRequest(hServerListRequest);
}

STEAMAPI_API gameserveritem_t * SteamAPI_ISteamMatchmakingServers_GetServerDetails( ISteamMatchmakingServers* self, HServerListRequest hRequest, int iServer )
{
    return (get_steam_client()->steam_matchmaking_servers)->GetServerDetails(hRequest, iServer);
}

STEAMAPI_API void SteamAPI_ISteamMatchmakingServers_CancelQuery( ISteamMatchmakingServers* self, HServerListRequest hRequest )
{
    return (get_steam_client()->steam_matchmaking_servers)->CancelQuery(hRequest);
}

STEAMAPI_API void SteamAPI_ISteamMatchmakingServers_RefreshQuery( ISteamMatchmakingServers* self, HServerListRequest hRequest )
{
    return (get_steam_client()->steam_matchmaking_servers)->RefreshQuery(hRequest);
}

STEAMAPI_API steam_bool SteamAPI_ISteamMatchmakingServers_IsRefreshing( ISteamMatchmakingServers* self, HServerListRequest hRequest )
{
    return (get_steam_client()->steam_matchmaking_servers)->IsRefreshing(hRequest);
}

STEAMAPI_API int SteamAPI_ISteamMatchmakingServers_GetServerCount( ISteamMatchmakingServers* self, HServerListRequest hRequest )
{
    return (get_steam_client()->steam_matchmaking_servers)->GetServerCount(hRequest);
}

STEAMAPI_API void SteamAPI_ISteamMatchmakingServers_RefreshServer( ISteamMatchmakingServers* self, HServerListRequest hRequest, int iServer )
{
    return (get_steam_client()->steam_matchmaking_servers)->RefreshServer(hRequest, iServer);
}

STEAMAPI_API HServerQuery SteamAPI_ISteamMatchmakingServers_PingServer( ISteamMatchmakingServers* self, uint32 unIP, uint16 usPort, ISteamMatchmakingPingResponse * pRequestServersResponse )
{
    return (get_steam_client()->steam_matchmaking_servers)->PingServer(unIP, usPort, pRequestServersResponse);
}

STEAMAPI_API HServerQuery SteamAPI_ISteamMatchmakingServers_PlayerDetails( ISteamMatchmakingServers* self, uint32 unIP, uint16 usPort, ISteamMatchmakingPlayersResponse * pRequestServersResponse )
{
    return (get_steam_client()->steam_matchmaking_servers)->PlayerDetails(unIP, usPort, pRequestServersResponse);
}

STEAMAPI_API HServerQuery SteamAPI_ISteamMatchmakingServers_ServerRules( ISteamMatchmakingServers* self, uint32 unIP, uint16 usPort, ISteamMatchmakingRulesResponse * pRequestServersResponse )
{
    return (get_steam_client()->steam_matchmaking_servers)->ServerRules(unIP, usPort, pRequestServersResponse);
}

STEAMAPI_API void SteamAPI_ISteamMatchmakingServers_CancelServerQuery( ISteamMatchmakingServers* self, HServerQuery hServerQuery )
{
    return (get_steam_client()->steam_matchmaking_servers)->CancelServerQuery(hServerQuery);
}

STEAMAPI_API ISteamGameSearch *SteamAPI_SteamGameSearch_v001()
{
    return get_steam_client()->GetISteamGameSearch(flat_hsteamuser(), flat_hsteampipe(), "SteamMatchGameSearch001");
}

STEAMAPI_API EGameSearchErrorCode_t SteamAPI_ISteamGameSearch_AddGameSearchParams( ISteamGameSearch* self, const char * pchKeyToFind, const char * pchValuesToFind )
{
    return self->AddGameSearchParams(pchKeyToFind, pchValuesToFind);
}

STEAMAPI_API EGameSearchErrorCode_t SteamAPI_ISteamGameSearch_SearchForGameWithLobby( ISteamGameSearch* self, uint64_steamid steamIDLobby, int nPlayerMin, int nPlayerMax )
{
    return self->SearchForGameWithLobby(steamIDLobby, nPlayerMin, nPlayerMax);
}

STEAMAPI_API EGameSearchErrorCode_t SteamAPI_ISteamGameSearch_SearchForGameSolo( ISteamGameSearch* self, int nPlayerMin, int nPlayerMax )
{
    return self->SearchForGameSolo(nPlayerMin, nPlayerMax);
}

STEAMAPI_API EGameSearchErrorCode_t SteamAPI_ISteamGameSearch_AcceptGame( ISteamGameSearch* self )
{
    return self->AcceptGame();
}

STEAMAPI_API EGameSearchErrorCode_t SteamAPI_ISteamGameSearch_DeclineGame( ISteamGameSearch* self )
{
    return self->DeclineGame();
}

STEAMAPI_API EGameSearchErrorCode_t SteamAPI_ISteamGameSearch_RetrieveConnectionDetails( ISteamGameSearch* self, uint64_steamid steamIDHost, char * pchConnectionDetails, int cubConnectionDetails )
{
    return self->RetrieveConnectionDetails(steamIDHost, pchConnectionDetails, cubConnectionDetails);
}

STEAMAPI_API EGameSearchErrorCode_t SteamAPI_ISteamGameSearch_EndGameSearch( ISteamGameSearch* self )
{
    return self->EndGameSearch();
}

STEAMAPI_API EGameSearchErrorCode_t SteamAPI_ISteamGameSearch_SetGameHostParams( ISteamGameSearch* self, const char * pchKey, const char * pchValue )
{
    return self->SetGameHostParams(pchKey, pchValue);
}

STEAMAPI_API EGameSearchErrorCode_t SteamAPI_ISteamGameSearch_SetConnectionDetails( ISteamGameSearch* self, const char * pchConnectionDetails, int cubConnectionDetails )
{
    return self->SetConnectionDetails(pchConnectionDetails, cubConnectionDetails);
}

STEAMAPI_API EGameSearchErrorCode_t SteamAPI_ISteamGameSearch_RequestPlayersForGame( ISteamGameSearch* self, int nPlayerMin, int nPlayerMax, int nMaxTeamSize )
{
    return self->RequestPlayersForGame(nPlayerMin, nPlayerMax, nMaxTeamSize);
}

STEAMAPI_API EGameSearchErrorCode_t SteamAPI_ISteamGameSearch_HostConfirmGameStart( ISteamGameSearch* self, uint64 ullUniqueGameID )
{
    return self->HostConfirmGameStart(ullUniqueGameID);
}

STEAMAPI_API EGameSearchErrorCode_t SteamAPI_ISteamGameSearch_CancelRequestPlayersForGame( ISteamGameSearch* self )
{
    return self->CancelRequestPlayersForGame();
}

STEAMAPI_API EGameSearchErrorCode_t SteamAPI_ISteamGameSearch_SubmitPlayerResult( ISteamGameSearch* self, uint64 ullUniqueGameID, uint64_steamid steamIDPlayer, EPlayerResult_t EPlayerResult )
{
    return self->SubmitPlayerResult(ullUniqueGameID, steamIDPlayer, EPlayerResult);
}

STEAMAPI_API EGameSearchErrorCode_t SteamAPI_ISteamGameSearch_EndGame( ISteamGameSearch* self, uint64 ullUniqueGameID )
{
    return self->EndGame(ullUniqueGameID);
}

STEAMAPI_API ISteamParties *SteamAPI_SteamParties_v002()
{
    return get_steam_client()->GetISteamParties(flat_hsteamuser(), flat_hsteampipe(), "SteamParties002");
}

STEAMAPI_API uint32 SteamAPI_ISteamParties_GetNumActiveBeacons( ISteamParties* self )
{
    return self->GetNumActiveBeacons();
}

STEAMAPI_API PartyBeaconID_t SteamAPI_ISteamParties_GetBeaconByIndex( ISteamParties* self, uint32 unIndex )
{
    return self->GetBeaconByIndex(unIndex);
}

STEAMAPI_API steam_bool SteamAPI_ISteamParties_GetBeaconDetails( ISteamParties* self, PartyBeaconID_t ulBeaconID, CSteamID * pSteamIDBeaconOwner, SteamPartyBeaconLocation_t * pLocation, char * pchMetadata, int cchMetadata )
{
    return self->GetBeaconDetails(ulBeaconID, pSteamIDBeaconOwner, pLocation, pchMetadata, cchMetadata);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamParties_JoinParty( ISteamParties* self, PartyBeaconID_t ulBeaconID )
{
    return self->JoinParty(ulBeaconID);
}

STEAMAPI_API steam_bool SteamAPI_ISteamParties_GetNumAvailableBeaconLocations( ISteamParties* self, uint32 * puNumLocations )
{
    return self->GetNumAvailableBeaconLocations(puNumLocations);
}

STEAMAPI_API steam_bool SteamAPI_ISteamParties_GetAvailableBeaconLocations( ISteamParties* self, SteamPartyBeaconLocation_t * pLocationList, uint32 uMaxNumLocations )
{
    return self->GetAvailableBeaconLocations(pLocationList, uMaxNumLocations);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamParties_CreateBeacon( ISteamParties* self, uint32 unOpenSlots, SteamPartyBeaconLocation_t * pBeaconLocation, const char * pchConnectString, const char * pchMetadata )
{
    return self->CreateBeacon(unOpenSlots, pBeaconLocation, pchConnectString, pchMetadata);
}

STEAMAPI_API void SteamAPI_ISteamParties_OnReservationCompleted( ISteamParties* self, PartyBeaconID_t ulBeacon, uint64_steamid steamIDUser )
{
    return self->OnReservationCompleted(ulBeacon, steamIDUser);
}

STEAMAPI_API void SteamAPI_ISteamParties_CancelReservation( ISteamParties* self, PartyBeaconID_t ulBeacon, uint64_steamid steamIDUser )
{
    return self->CancelReservation(ulBeacon, steamIDUser);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamParties_ChangeNumOpenSlots( ISteamParties* self, PartyBeaconID_t ulBeacon, uint32 unOpenSlots )
{
    return self->ChangeNumOpenSlots(ulBeacon, unOpenSlots);
}

STEAMAPI_API steam_bool SteamAPI_ISteamParties_DestroyBeacon( ISteamParties* self, PartyBeaconID_t ulBeacon )
{
    return self->DestroyBeacon(ulBeacon);
}

STEAMAPI_API steam_bool SteamAPI_ISteamParties_GetBeaconLocationData( ISteamParties* self, SteamPartyBeaconLocation_t BeaconLocation, ESteamPartyBeaconLocationData eData, char * pchDataStringOut, int cchDataStringOut )
{
    return self->GetBeaconLocationData(BeaconLocation, eData, pchDataStringOut, cchDataStringOut);
}

STEAMAPI_API ISteamRemoteStorage *SteamAPI_SteamRemoteStorage_v014()
{
    return get_steam_client()->GetISteamRemoteStorage(flat_hsteamuser(), flat_hsteampipe(), "STEAMREMOTESTORAGE_INTERFACE_VERSION014");
}

STEAMAPI_API ISteamRemoteStorage *SteamAPI_SteamRemoteStorage_v016()
{
    return get_steam_client()->GetISteamRemoteStorage(flat_hsteamuser(), flat_hsteampipe(), "STEAMREMOTESTORAGE_INTERFACE_VERSION016");
}

STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_FileWrite( ISteamRemoteStorage* self, const char * pchFile, const void * pvData, int32 cubData )
{
    return (get_steam_client()->steam_remote_storage)->FileWrite(pchFile, pvData, cubData);
}

STEAMAPI_API int32 SteamAPI_ISteamRemoteStorage_FileRead( ISteamRemoteStorage* self, const char * pchFile, void * pvData, int32 cubDataToRead )
{
    return (get_steam_client()->steam_remote_storage)->FileRead(pchFile, pvData, cubDataToRead);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_FileWriteAsync( ISteamRemoteStorage* self, const char * pchFile, const void * pvData, uint32 cubData )
{
    return (get_steam_client()->steam_remote_storage)->FileWriteAsync(pchFile, pvData, cubData);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_FileReadAsync( ISteamRemoteStorage* self, const char * pchFile, uint32 nOffset, uint32 cubToRead )
{
    return (get_steam_client()->steam_remote_storage)->FileReadAsync(pchFile, nOffset, cubToRead);
}

STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_FileReadAsyncComplete( ISteamRemoteStorage* self, SteamAPICall_t hReadCall, void * pvBuffer, uint32 cubToRead )
{
    return (get_steam_client()->steam_remote_storage)->FileReadAsyncComplete(hReadCall, pvBuffer, cubToRead);
}

STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_FileForget( ISteamRemoteStorage* self, const char * pchFile )
{
    return (get_steam_client()->steam_remote_storage)->FileForget(pchFile);
}

STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_FileDelete( ISteamRemoteStorage* self, const char * pchFile )
{
    return (get_steam_client()->steam_remote_storage)->FileDelete(pchFile);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_FileShare( ISteamRemoteStorage* self, const char * pchFile )
{
    return (get_steam_client()->steam_remote_storage)->FileShare(pchFile);
}

STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_SetSyncPlatforms( ISteamRemoteStorage* self, const char * pchFile, ERemoteStoragePlatform eRemoteStoragePlatform )
{
    return (get_steam_client()->steam_remote_storage)->SetSyncPlatforms(pchFile, eRemoteStoragePlatform);
}

STEAMAPI_API UGCFileWriteStreamHandle_t SteamAPI_ISteamRemoteStorage_FileWriteStreamOpen( ISteamRemoteStorage* self, const char * pchFile )
{
    return (get_steam_client()->steam_remote_storage)->FileWriteStreamOpen(pchFile);
}

STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_FileWriteStreamWriteChunk( ISteamRemoteStorage* self, UGCFileWriteStreamHandle_t writeHandle, const void * pvData, int32 cubData )
{
    return (get_steam_client()->steam_remote_storage)->FileWriteStreamWriteChunk(writeHandle, pvData, cubData);
}

STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_FileWriteStreamClose( ISteamRemoteStorage* self, UGCFileWriteStreamHandle_t writeHandle )
{
    return (get_steam_client()->steam_remote_storage)->FileWriteStreamClose(writeHandle);
}

STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_FileWriteStreamCancel( ISteamRemoteStorage* self, UGCFileWriteStreamHandle_t writeHandle )
{
    return (get_steam_client()->steam_remote_storage)->FileWriteStreamCancel(writeHandle);
}

STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_FileExists( ISteamRemoteStorage* self, const char * pchFile )
{
    return (get_steam_client()->steam_remote_storage)->FileExists(pchFile);
}

STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_FilePersisted( ISteamRemoteStorage* self, const char * pchFile )
{
    return (get_steam_client()->steam_remote_storage)->FilePersisted(pchFile);
}

STEAMAPI_API int32 SteamAPI_ISteamRemoteStorage_GetFileSize( ISteamRemoteStorage* self, const char * pchFile )
{
    return (get_steam_client()->steam_remote_storage)->GetFileSize(pchFile);
}

STEAMAPI_API int64 SteamAPI_ISteamRemoteStorage_GetFileTimestamp( ISteamRemoteStorage* self, const char * pchFile )
{
    return (get_steam_client()->steam_remote_storage)->GetFileTimestamp(pchFile);
}

STEAMAPI_API ERemoteStoragePlatform SteamAPI_ISteamRemoteStorage_GetSyncPlatforms( ISteamRemoteStorage* self, const char * pchFile )
{
    return (get_steam_client()->steam_remote_storage)->GetSyncPlatforms(pchFile);
}

STEAMAPI_API int32 SteamAPI_ISteamRemoteStorage_GetFileCount( ISteamRemoteStorage* self )
{
    return (get_steam_client()->steam_remote_storage)->GetFileCount();
}

STEAMAPI_API const char * SteamAPI_ISteamRemoteStorage_GetFileNameAndSize( ISteamRemoteStorage* self, int iFile, int32 * pnFileSizeInBytes )
{
    return (get_steam_client()->steam_remote_storage)->GetFileNameAndSize(iFile, pnFileSizeInBytes);
}

STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_GetQuota( ISteamRemoteStorage* self, uint64 * pnTotalBytes, uint64 * puAvailableBytes )
{
    return (get_steam_client()->steam_remote_storage)->GetQuota(pnTotalBytes, puAvailableBytes);
}

STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_IsCloudEnabledForAccount( ISteamRemoteStorage* self )
{
    return (get_steam_client()->steam_remote_storage)->IsCloudEnabledForAccount();
}

STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_IsCloudEnabledForApp( ISteamRemoteStorage* self )
{
    return (get_steam_client()->steam_remote_storage)->IsCloudEnabledForApp();
}

STEAMAPI_API void SteamAPI_ISteamRemoteStorage_SetCloudEnabledForApp( ISteamRemoteStorage* self, bool bEnabled )
{
    return (get_steam_client()->steam_remote_storage)->SetCloudEnabledForApp(bEnabled);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_UGCDownload( ISteamRemoteStorage* self, UGCHandle_t hContent, uint32 unPriority )
{
    return (get_steam_client()->steam_remote_storage)->UGCDownload(hContent, unPriority);
}

STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_GetUGCDownloadProgress( ISteamRemoteStorage* self, UGCHandle_t hContent, int32 * pnBytesDownloaded, int32 * pnBytesExpected )
{
    return (get_steam_client()->steam_remote_storage)->GetUGCDownloadProgress(hContent, pnBytesDownloaded, pnBytesExpected);
}

STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_GetUGCDetails( ISteamRemoteStorage* self, UGCHandle_t hContent, AppId_t * pnAppID, char ** ppchName, int32 * pnFileSizeInBytes, CSteamID * pSteamIDOwner )
{
    return (get_steam_client()->steam_remote_storage)->GetUGCDetails(hContent, pnAppID, ppchName, pnFileSizeInBytes, pSteamIDOwner);
}

STEAMAPI_API int32 SteamAPI_ISteamRemoteStorage_UGCRead( ISteamRemoteStorage* self, UGCHandle_t hContent, void * pvData, int32 cubDataToRead, uint32 cOffset, EUGCReadAction eAction )
{
    return (get_steam_client()->steam_remote_storage)->UGCRead(hContent, pvData, cubDataToRead, cOffset, eAction);
}

STEAMAPI_API int32 SteamAPI_ISteamRemoteStorage_GetCachedUGCCount( ISteamRemoteStorage* self )
{
    return (get_steam_client()->steam_remote_storage)->GetCachedUGCCount();
}

STEAMAPI_API UGCHandle_t SteamAPI_ISteamRemoteStorage_GetCachedUGCHandle( ISteamRemoteStorage* self, int32 iCachedContent )
{
    return (get_steam_client()->steam_remote_storage)->GetCachedUGCHandle(iCachedContent);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_PublishWorkshopFile( ISteamRemoteStorage* self, const char * pchFile, const char * pchPreviewFile, AppId_t nConsumerAppId, const char * pchTitle, const char * pchDescription, ERemoteStoragePublishedFileVisibility eVisibility, SteamParamStringArray_t * pTags, EWorkshopFileType eWorkshopFileType )
{
    return (get_steam_client()->steam_remote_storage)->PublishWorkshopFile(pchFile, pchPreviewFile, nConsumerAppId, pchTitle, pchDescription, eVisibility, pTags, eWorkshopFileType);
}

STEAMAPI_API PublishedFileUpdateHandle_t SteamAPI_ISteamRemoteStorage_CreatePublishedFileUpdateRequest( ISteamRemoteStorage* self, PublishedFileId_t unPublishedFileId )
{
    return (get_steam_client()->steam_remote_storage)->CreatePublishedFileUpdateRequest(unPublishedFileId);
}

STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_UpdatePublishedFileFile( ISteamRemoteStorage* self, PublishedFileUpdateHandle_t updateHandle, const char * pchFile )
{
    return (get_steam_client()->steam_remote_storage)->UpdatePublishedFileFile(updateHandle, pchFile);
}

STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_UpdatePublishedFilePreviewFile( ISteamRemoteStorage* self, PublishedFileUpdateHandle_t updateHandle, const char * pchPreviewFile )
{
    return (get_steam_client()->steam_remote_storage)->UpdatePublishedFilePreviewFile(updateHandle, pchPreviewFile);
}

STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_UpdatePublishedFileTitle( ISteamRemoteStorage* self, PublishedFileUpdateHandle_t updateHandle, const char * pchTitle )
{
    return (get_steam_client()->steam_remote_storage)->UpdatePublishedFileTitle(updateHandle, pchTitle);
}

STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_UpdatePublishedFileDescription( ISteamRemoteStorage* self, PublishedFileUpdateHandle_t updateHandle, const char * pchDescription )
{
    return (get_steam_client()->steam_remote_storage)->UpdatePublishedFileDescription(updateHandle, pchDescription);
}

STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_UpdatePublishedFileVisibility( ISteamRemoteStorage* self, PublishedFileUpdateHandle_t updateHandle, ERemoteStoragePublishedFileVisibility eVisibility )
{
    return (get_steam_client()->steam_remote_storage)->UpdatePublishedFileVisibility(updateHandle, eVisibility);
}

STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_UpdatePublishedFileTags( ISteamRemoteStorage* self, PublishedFileUpdateHandle_t updateHandle, SteamParamStringArray_t * pTags )
{
    return (get_steam_client()->steam_remote_storage)->UpdatePublishedFileTags(updateHandle, pTags);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_CommitPublishedFileUpdate( ISteamRemoteStorage* self, PublishedFileUpdateHandle_t updateHandle )
{
    return (get_steam_client()->steam_remote_storage)->CommitPublishedFileUpdate(updateHandle);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_GetPublishedFileDetails( ISteamRemoteStorage* self, PublishedFileId_t unPublishedFileId, uint32 unMaxSecondsOld )
{
    return (get_steam_client()->steam_remote_storage)->GetPublishedFileDetails(unPublishedFileId, unMaxSecondsOld);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_DeletePublishedFile( ISteamRemoteStorage* self, PublishedFileId_t unPublishedFileId )
{
    return (get_steam_client()->steam_remote_storage)->DeletePublishedFile(unPublishedFileId);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_EnumerateUserPublishedFiles( ISteamRemoteStorage* self, uint32 unStartIndex )
{
    return (get_steam_client()->steam_remote_storage)->EnumerateUserPublishedFiles(unStartIndex);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_SubscribePublishedFile( ISteamRemoteStorage* self, PublishedFileId_t unPublishedFileId )
{
    return (get_steam_client()->steam_remote_storage)->SubscribePublishedFile(unPublishedFileId);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_EnumerateUserSubscribedFiles( ISteamRemoteStorage* self, uint32 unStartIndex )
{
    return (get_steam_client()->steam_remote_storage)->EnumerateUserSubscribedFiles(unStartIndex);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_UnsubscribePublishedFile( ISteamRemoteStorage* self, PublishedFileId_t unPublishedFileId )
{
    return (get_steam_client()->steam_remote_storage)->UnsubscribePublishedFile(unPublishedFileId);
}

STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_UpdatePublishedFileSetChangeDescription( ISteamRemoteStorage* self, PublishedFileUpdateHandle_t updateHandle, const char * pchChangeDescription )
{
    return (get_steam_client()->steam_remote_storage)->UpdatePublishedFileSetChangeDescription(updateHandle, pchChangeDescription);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_GetPublishedItemVoteDetails( ISteamRemoteStorage* self, PublishedFileId_t unPublishedFileId )
{
    return (get_steam_client()->steam_remote_storage)->GetPublishedItemVoteDetails(unPublishedFileId);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_UpdateUserPublishedItemVote( ISteamRemoteStorage* self, PublishedFileId_t unPublishedFileId, bool bVoteUp )
{
    return (get_steam_client()->steam_remote_storage)->UpdateUserPublishedItemVote(unPublishedFileId, bVoteUp);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_GetUserPublishedItemVoteDetails( ISteamRemoteStorage* self, PublishedFileId_t unPublishedFileId )
{
    return (get_steam_client()->steam_remote_storage)->GetUserPublishedItemVoteDetails(unPublishedFileId);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_EnumerateUserSharedWorkshopFiles( ISteamRemoteStorage* self, uint64_steamid steamId, uint32 unStartIndex, SteamParamStringArray_t * pRequiredTags, SteamParamStringArray_t * pExcludedTags )
{
    return (get_steam_client()->steam_remote_storage)->EnumerateUserSharedWorkshopFiles(steamId, unStartIndex, pRequiredTags, pExcludedTags);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_PublishVideo( ISteamRemoteStorage* self, EWorkshopVideoProvider eVideoProvider, const char * pchVideoAccount, const char * pchVideoIdentifier, const char * pchPreviewFile, AppId_t nConsumerAppId, const char * pchTitle, const char * pchDescription, ERemoteStoragePublishedFileVisibility eVisibility, SteamParamStringArray_t * pTags )
{
    return (get_steam_client()->steam_remote_storage)->PublishVideo(eVideoProvider, pchVideoAccount, pchVideoIdentifier, pchPreviewFile, nConsumerAppId, pchTitle, pchDescription, eVisibility, pTags);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_SetUserPublishedFileAction( ISteamRemoteStorage* self, PublishedFileId_t unPublishedFileId, EWorkshopFileAction eAction )
{
    return (get_steam_client()->steam_remote_storage)->SetUserPublishedFileAction(unPublishedFileId, eAction);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_EnumeratePublishedFilesByUserAction( ISteamRemoteStorage* self, EWorkshopFileAction eAction, uint32 unStartIndex )
{
    return (get_steam_client()->steam_remote_storage)->EnumeratePublishedFilesByUserAction(eAction, unStartIndex);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_EnumeratePublishedWorkshopFiles( ISteamRemoteStorage* self, EWorkshopEnumerationType eEnumerationType, uint32 unStartIndex, uint32 unCount, uint32 unDays, SteamParamStringArray_t * pTags, SteamParamStringArray_t * pUserTags )
{
    return (get_steam_client()->steam_remote_storage)->EnumeratePublishedWorkshopFiles(eEnumerationType, unStartIndex, unCount, unDays, pTags, pUserTags);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_UGCDownloadToLocation( ISteamRemoteStorage* self, UGCHandle_t hContent, const char * pchLocation, uint32 unPriority )
{
    return (get_steam_client()->steam_remote_storage)->UGCDownloadToLocation(hContent, pchLocation, unPriority);
}

STEAMAPI_API int32 SteamAPI_ISteamRemoteStorage_GetLocalFileChangeCount( ISteamRemoteStorage* self )
{
    return (get_steam_client()->steam_remote_storage)->GetLocalFileChangeCount();
}

STEAMAPI_API const char * SteamAPI_ISteamRemoteStorage_GetLocalFileChange( ISteamRemoteStorage* self, int iFile, ERemoteStorageLocalFileChange * pEChangeType, ERemoteStorageFilePathType * pEFilePathType )
{
    return (get_steam_client()->steam_remote_storage)->GetLocalFileChange(iFile, pEChangeType, pEFilePathType);
}

STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_BeginFileWriteBatch( ISteamRemoteStorage* self )
{
    return (get_steam_client()->steam_remote_storage)->BeginFileWriteBatch();
}

STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_EndFileWriteBatch( ISteamRemoteStorage* self )
{
    return (get_steam_client()->steam_remote_storage)->EndFileWriteBatch();
}

STEAMAPI_API ISteamUserStats *SteamAPI_SteamUserStats_v013()
{
    return get_steam_client()->GetISteamUserStats(flat_hsteamuser(), flat_hsteampipe(), "STEAMUSERSTATS_INTERFACE_VERSION013");
}

STEAMAPI_API ISteamUserStats *SteamAPI_SteamUserStats_v012()
{
    return get_steam_client()->GetISteamUserStats(flat_hsteamuser(), flat_hsteampipe(), "STEAMUSERSTATS_INTERFACE_VERSION012");
}

STEAMAPI_API ISteamUserStats *SteamAPI_SteamUserStats_v011()
{
    return get_steam_client()->GetISteamUserStats(flat_hsteamuser(), flat_hsteampipe(), "STEAMUSERSTATS_INTERFACE_VERSION011");
}

STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_RequestCurrentStats( ISteamUserStats* self )
{
    return (get_steam_client()->steam_user_stats)->RequestCurrentStats();
}

STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_GetStat(intptr_t instancePtr, const char * pchName, int32 * pData)
{
    return (get_steam_client()->steam_user_stats)->GetStat(pchName, pData);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_GetStat0(intptr_t instancePtr, const char * pchName, float * pData)
{
    return (get_steam_client()->steam_user_stats)->GetStat(pchName, pData);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_GetStatInt32( ISteamUserStats* self, const char * pchName, int32 * pData )
{
    return (get_steam_client()->steam_user_stats)->GetStat(pchName, pData);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_GetStatFloat( ISteamUserStats* self, const char * pchName, float * pData )
{
    return (get_steam_client()->steam_user_stats)->GetStat(pchName, pData);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_SetStat(intptr_t instancePtr, const char * pchName, int32 nData)
{
    return (get_steam_client()->steam_user_stats)->SetStat(pchName, nData);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_SetStat0(intptr_t instancePtr, const char * pchName, float fData)
{
    return (get_steam_client()->steam_user_stats)->SetStat(pchName, fData);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_SetStatInt32( ISteamUserStats* self, const char * pchName, int32 nData )
{
    return (get_steam_client()->steam_user_stats)->SetStat(pchName, nData);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_SetStatFloat( ISteamUserStats* self, const char * pchName, float fData )
{
    return (get_steam_client()->steam_user_stats)->SetStat(pchName, fData);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_UpdateAvgRateStat( ISteamUserStats* self, const char * pchName, float flCountThisSession, double dSessionLength )
{
    return (get_steam_client()->steam_user_stats)->UpdateAvgRateStat(pchName, flCountThisSession, dSessionLength);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_GetAchievement( ISteamUserStats* self, const char * pchName, bool * pbAchieved )
{
    return (get_steam_client()->steam_user_stats)->GetAchievement(pchName, pbAchieved);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_SetAchievement( ISteamUserStats* self, const char * pchName )
{
    return (get_steam_client()->steam_user_stats)->SetAchievement(pchName);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_ClearAchievement( ISteamUserStats* self, const char * pchName )
{
    return (get_steam_client()->steam_user_stats)->ClearAchievement(pchName);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_GetAchievementAndUnlockTime( ISteamUserStats* self, const char * pchName, bool * pbAchieved, uint32 * punUnlockTime )
{
    return (get_steam_client()->steam_user_stats)->GetAchievementAndUnlockTime(pchName, pbAchieved, punUnlockTime);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_StoreStats( ISteamUserStats* self )
{
    return (get_steam_client()->steam_user_stats)->StoreStats();
}

STEAMAPI_API int SteamAPI_ISteamUserStats_GetAchievementIcon( ISteamUserStats* self, const char * pchName )
{
    return (get_steam_client()->steam_user_stats)->GetAchievementIcon(pchName);
}

STEAMAPI_API const char * SteamAPI_ISteamUserStats_GetAchievementDisplayAttribute( ISteamUserStats* self, const char * pchName, const char * pchKey )
{
    return (get_steam_client()->steam_user_stats)->GetAchievementDisplayAttribute(pchName, pchKey);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_IndicateAchievementProgress( ISteamUserStats* self, const char * pchName, uint32 nCurProgress, uint32 nMaxProgress )
{
    return (get_steam_client()->steam_user_stats)->IndicateAchievementProgress(pchName, nCurProgress, nMaxProgress);
}

STEAMAPI_API uint32 SteamAPI_ISteamUserStats_GetNumAchievements( ISteamUserStats* self )
{
    return (get_steam_client()->steam_user_stats)->GetNumAchievements();
}

STEAMAPI_API const char * SteamAPI_ISteamUserStats_GetAchievementName( ISteamUserStats* self, uint32 iAchievement )
{
    return (get_steam_client()->steam_user_stats)->GetAchievementName(iAchievement);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUserStats_RequestUserStats( ISteamUserStats* self, uint64_steamid steamIDUser )
{
    return (get_steam_client()->steam_user_stats)->RequestUserStats(steamIDUser);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_GetUserStat(intptr_t instancePtr, class CSteamID steamIDUser, const char * pchName, int32 * pData)
{
    return (get_steam_client()->steam_user_stats)->GetUserStat(steamIDUser, pchName, pData);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_GetUserStat0(intptr_t instancePtr, class CSteamID steamIDUser, const char * pchName, float * pData)
{
    return (get_steam_client()->steam_user_stats)->GetUserStat(steamIDUser, pchName, pData);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_GetUserStatInt32( ISteamUserStats* self, uint64_steamid steamIDUser, const char * pchName, int32 * pData )
{
    return (get_steam_client()->steam_user_stats)->GetUserStat(steamIDUser, pchName, pData);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_GetUserStatFloat( ISteamUserStats* self, uint64_steamid steamIDUser, const char * pchName, float * pData )
{
    return (get_steam_client()->steam_user_stats)->GetUserStat(steamIDUser, pchName, pData);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_GetUserAchievement( ISteamUserStats* self, uint64_steamid steamIDUser, const char * pchName, bool * pbAchieved )
{
    return (get_steam_client()->steam_user_stats)->GetUserAchievement(steamIDUser, pchName, pbAchieved);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_GetUserAchievementAndUnlockTime( ISteamUserStats* self, uint64_steamid steamIDUser, const char * pchName, bool * pbAchieved, uint32 * punUnlockTime )
{
    return (get_steam_client()->steam_user_stats)->GetUserAchievementAndUnlockTime(steamIDUser, pchName, pbAchieved, punUnlockTime);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_ResetAllStats( ISteamUserStats* self, bool bAchievementsToo )
{
    return (get_steam_client()->steam_user_stats)->ResetAllStats(bAchievementsToo);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUserStats_FindOrCreateLeaderboard( ISteamUserStats* self, const char * pchLeaderboardName, ELeaderboardSortMethod eLeaderboardSortMethod, ELeaderboardDisplayType eLeaderboardDisplayType )
{
    return (get_steam_client()->steam_user_stats)->FindOrCreateLeaderboard(pchLeaderboardName, eLeaderboardSortMethod, eLeaderboardDisplayType);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUserStats_FindLeaderboard( ISteamUserStats* self, const char * pchLeaderboardName )
{
    return (get_steam_client()->steam_user_stats)->FindLeaderboard(pchLeaderboardName);
}

STEAMAPI_API const char * SteamAPI_ISteamUserStats_GetLeaderboardName( ISteamUserStats* self, SteamLeaderboard_t hSteamLeaderboard )
{
    return (get_steam_client()->steam_user_stats)->GetLeaderboardName(hSteamLeaderboard);
}

STEAMAPI_API int SteamAPI_ISteamUserStats_GetLeaderboardEntryCount( ISteamUserStats* self, SteamLeaderboard_t hSteamLeaderboard )
{
    return (get_steam_client()->steam_user_stats)->GetLeaderboardEntryCount(hSteamLeaderboard);
}

STEAMAPI_API ELeaderboardSortMethod SteamAPI_ISteamUserStats_GetLeaderboardSortMethod( ISteamUserStats* self, SteamLeaderboard_t hSteamLeaderboard )
{
    return (get_steam_client()->steam_user_stats)->GetLeaderboardSortMethod(hSteamLeaderboard);
}

STEAMAPI_API ELeaderboardDisplayType SteamAPI_ISteamUserStats_GetLeaderboardDisplayType( ISteamUserStats* self, SteamLeaderboard_t hSteamLeaderboard )
{
    return (get_steam_client()->steam_user_stats)->GetLeaderboardDisplayType(hSteamLeaderboard);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUserStats_DownloadLeaderboardEntries( ISteamUserStats* self, SteamLeaderboard_t hSteamLeaderboard, ELeaderboardDataRequest eLeaderboardDataRequest, int nRangeStart, int nRangeEnd )
{
    return (get_steam_client()->steam_user_stats)->DownloadLeaderboardEntries(hSteamLeaderboard, eLeaderboardDataRequest, nRangeStart, nRangeEnd);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUserStats_DownloadLeaderboardEntriesForUsers( ISteamUserStats* self, SteamLeaderboard_t hSteamLeaderboard, CSteamID * prgUsers, int cUsers )
{
    return (get_steam_client()->steam_user_stats)->DownloadLeaderboardEntriesForUsers(hSteamLeaderboard, prgUsers, cUsers);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_GetDownloadedLeaderboardEntry( ISteamUserStats* self, SteamLeaderboardEntries_t hSteamLeaderboardEntries, int index, LeaderboardEntry_t * pLeaderboardEntry, int32 * pDetails, int cDetailsMax )
{
    return (get_steam_client()->steam_user_stats)->GetDownloadedLeaderboardEntry(hSteamLeaderboardEntries, index, pLeaderboardEntry, pDetails, cDetailsMax);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUserStats_UploadLeaderboardScore( ISteamUserStats* self, SteamLeaderboard_t hSteamLeaderboard, ELeaderboardUploadScoreMethod eLeaderboardUploadScoreMethod, int32 nScore, const int32 * pScoreDetails, int cScoreDetailsCount )
{
    return (get_steam_client()->steam_user_stats)->UploadLeaderboardScore(hSteamLeaderboard, eLeaderboardUploadScoreMethod, nScore, pScoreDetails, cScoreDetailsCount);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUserStats_AttachLeaderboardUGC( ISteamUserStats* self, SteamLeaderboard_t hSteamLeaderboard, UGCHandle_t hUGC )
{
    return (get_steam_client()->steam_user_stats)->AttachLeaderboardUGC(hSteamLeaderboard, hUGC);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUserStats_GetNumberOfCurrentPlayers( ISteamUserStats* self )
{
    return (get_steam_client()->steam_user_stats)->GetNumberOfCurrentPlayers();
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUserStats_RequestGlobalAchievementPercentages( ISteamUserStats* self )
{
    return (get_steam_client()->steam_user_stats)->RequestGlobalAchievementPercentages();
}

STEAMAPI_API int SteamAPI_ISteamUserStats_GetMostAchievedAchievementInfo( ISteamUserStats* self, char * pchName, uint32 unNameBufLen, float * pflPercent, bool * pbAchieved )
{
    return (get_steam_client()->steam_user_stats)->GetMostAchievedAchievementInfo(pchName, unNameBufLen, pflPercent, pbAchieved);
}

STEAMAPI_API int SteamAPI_ISteamUserStats_GetNextMostAchievedAchievementInfo( ISteamUserStats* self, int iIteratorPrevious, char * pchName, uint32 unNameBufLen, float * pflPercent, bool * pbAchieved )
{
    return (get_steam_client()->steam_user_stats)->GetNextMostAchievedAchievementInfo(iIteratorPrevious, pchName, unNameBufLen, pflPercent, pbAchieved);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_GetAchievementAchievedPercent( ISteamUserStats* self, const char * pchName, float * pflPercent )
{
    return (get_steam_client()->steam_user_stats)->GetAchievementAchievedPercent(pchName, pflPercent);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUserStats_RequestGlobalStats( ISteamUserStats* self, int nHistoryDays )
{
    return (get_steam_client()->steam_user_stats)->RequestGlobalStats(nHistoryDays);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_GetGlobalStat(intptr_t instancePtr, const char * pchStatName, int64 * pData)
{
    return (get_steam_client()->steam_user_stats)->GetGlobalStat(pchStatName, pData);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_GetGlobalStat0(intptr_t instancePtr, const char * pchStatName, double * pData)
{
    return (get_steam_client()->steam_user_stats)->GetGlobalStat(pchStatName, pData);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_GetGlobalStatInt64( ISteamUserStats* self, const char * pchStatName, int64 * pData )
{
    return (get_steam_client()->steam_user_stats)->GetGlobalStat(pchStatName, pData);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_GetGlobalStatDouble( ISteamUserStats* self, const char * pchStatName, double * pData )
{
    return (get_steam_client()->steam_user_stats)->GetGlobalStat(pchStatName, pData);
}

STEAMAPI_API int32 SteamAPI_ISteamUserStats_GetGlobalStatHistory(intptr_t instancePtr, const char * pchStatName, int64 * pData, uint32 cubData)
{
    return (get_steam_client()->steam_user_stats)->GetGlobalStatHistory(pchStatName, pData, cubData);
}

STEAMAPI_API int32 SteamAPI_ISteamUserStats_GetGlobalStatHistory0(intptr_t instancePtr, const char * pchStatName, double * pData, uint32 cubData)
{
    return (get_steam_client()->steam_user_stats)->GetGlobalStatHistory(pchStatName, pData, cubData);
}

STEAMAPI_API int32 SteamAPI_ISteamUserStats_GetGlobalStatHistoryInt64( ISteamUserStats* self, const char * pchStatName, int64 * pData, uint32 cubData )
{
    return (get_steam_client()->steam_user_stats)->GetGlobalStatHistory(pchStatName, pData, cubData);
}

STEAMAPI_API int32 SteamAPI_ISteamUserStats_GetGlobalStatHistoryDouble( ISteamUserStats* self, const char * pchStatName, double * pData, uint32 cubData )
{
    return (get_steam_client()->steam_user_stats)->GetGlobalStatHistory(pchStatName, pData, cubData);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_GetAchievementProgressLimitsInt32( ISteamUserStats* self, const char * pchName, int32 * pnMinProgress, int32 * pnMaxProgress )
{
    return (get_steam_client()->steam_user_stats)->GetAchievementProgressLimits(pchName, pnMinProgress, pnMaxProgress);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_GetAchievementProgressLimitsFloat( ISteamUserStats* self, const char * pchName, float * pfMinProgress, float * pfMaxProgress )
{
    return (get_steam_client()->steam_user_stats)->GetAchievementProgressLimits(pchName, pfMinProgress, pfMaxProgress);
}

STEAMAPI_API ISteamApps *SteamAPI_SteamApps_v008()
{
    return get_steam_client()->GetISteamApps(flat_hsteamuser(), flat_hsteampipe(), "STEAMAPPS_INTERFACE_VERSION008");
}

STEAMAPI_API ISteamApps *SteamAPI_SteamGameServerApps_v008()
{
    return get_steam_client()->GetISteamApps(flat_gs_hsteamuser(), flat_gs_hsteampipe(), "STEAMAPPS_INTERFACE_VERSION008");
}

STEAMAPI_API steam_bool SteamAPI_ISteamApps_BIsSubscribed( ISteamApps* self )
{
    return self->BIsSubscribed();
}

STEAMAPI_API steam_bool SteamAPI_ISteamApps_BIsLowViolence( ISteamApps* self )
{
    return self->BIsLowViolence();
}

STEAMAPI_API steam_bool SteamAPI_ISteamApps_BIsCybercafe( ISteamApps* self )
{
    return self->BIsCybercafe();
}

STEAMAPI_API steam_bool SteamAPI_ISteamApps_BIsVACBanned( ISteamApps* self )
{
    return self->BIsVACBanned();
}

STEAMAPI_API const char * SteamAPI_ISteamApps_GetCurrentGameLanguage( ISteamApps* self )
{
    return self->GetCurrentGameLanguage();
}

STEAMAPI_API const char * SteamAPI_ISteamApps_GetAvailableGameLanguages( ISteamApps* self )
{
    return self->GetAvailableGameLanguages();
}

STEAMAPI_API steam_bool SteamAPI_ISteamApps_BIsSubscribedApp( ISteamApps* self, AppId_t appID )
{
    return self->BIsSubscribedApp(appID);
}

STEAMAPI_API steam_bool SteamAPI_ISteamApps_BIsDlcInstalled( ISteamApps* self, AppId_t appID )
{
    return self->BIsDlcInstalled(appID);
}

STEAMAPI_API uint32 SteamAPI_ISteamApps_GetEarliestPurchaseUnixTime( ISteamApps* self, AppId_t nAppID )
{
    return self->GetEarliestPurchaseUnixTime(nAppID);
}

STEAMAPI_API steam_bool SteamAPI_ISteamApps_BIsSubscribedFromFreeWeekend( ISteamApps* self )
{
    return self->BIsSubscribedFromFreeWeekend();
}

STEAMAPI_API int SteamAPI_ISteamApps_GetDLCCount( ISteamApps* self )
{
    return self->GetDLCCount();
}

STEAMAPI_API steam_bool SteamAPI_ISteamApps_BGetDLCDataByIndex( ISteamApps* self, int iDLC, AppId_t * pAppID, bool * pbAvailable, char * pchName, int cchNameBufferSize )
{
    return self->BGetDLCDataByIndex(iDLC, pAppID, pbAvailable, pchName, cchNameBufferSize);
}

STEAMAPI_API void SteamAPI_ISteamApps_InstallDLC( ISteamApps* self, AppId_t nAppID )
{
    return self->InstallDLC(nAppID);
}

STEAMAPI_API void SteamAPI_ISteamApps_UninstallDLC( ISteamApps* self, AppId_t nAppID )
{
    return self->UninstallDLC(nAppID);
}

STEAMAPI_API void SteamAPI_ISteamApps_RequestAppProofOfPurchaseKey( ISteamApps* self, AppId_t nAppID )
{
    return self->RequestAppProofOfPurchaseKey(nAppID);
}

STEAMAPI_API steam_bool SteamAPI_ISteamApps_GetCurrentBetaName( ISteamApps* self, char * pchName, int cchNameBufferSize )
{
    return self->GetCurrentBetaName(pchName, cchNameBufferSize);
}

STEAMAPI_API steam_bool SteamAPI_ISteamApps_MarkContentCorrupt( ISteamApps* self, bool bMissingFilesOnly )
{
    return self->MarkContentCorrupt(bMissingFilesOnly);
}

STEAMAPI_API uint32 SteamAPI_ISteamApps_GetInstalledDepots( ISteamApps* self, AppId_t appID, DepotId_t * pvecDepots, uint32 cMaxDepots )
{
    return self->GetInstalledDepots(appID, pvecDepots, cMaxDepots);
}

STEAMAPI_API uint32 SteamAPI_ISteamApps_GetAppInstallDir( ISteamApps* self, AppId_t appID, char * pchFolder, uint32 cchFolderBufferSize )
{
    return self->GetAppInstallDir(appID, pchFolder, cchFolderBufferSize);
}

STEAMAPI_API steam_bool SteamAPI_ISteamApps_BIsAppInstalled( ISteamApps* self, AppId_t appID )
{
    return self->BIsAppInstalled(appID);
}

STEAMAPI_API uint64_steamid SteamAPI_ISteamApps_GetAppOwner( ISteamApps* self )
{
    return self->GetAppOwner().ConvertToUint64();
}

STEAMAPI_API const char * SteamAPI_ISteamApps_GetLaunchQueryParam( ISteamApps* self, const char * pchKey )
{
    return self->GetLaunchQueryParam(pchKey);
}

STEAMAPI_API steam_bool SteamAPI_ISteamApps_GetDlcDownloadProgress( ISteamApps* self, AppId_t nAppID, uint64 * punBytesDownloaded, uint64 * punBytesTotal )
{
    return self->GetDlcDownloadProgress(nAppID, punBytesDownloaded, punBytesTotal);
}

STEAMAPI_API int SteamAPI_ISteamApps_GetAppBuildId( ISteamApps* self )
{
    return self->GetAppBuildId();
}

STEAMAPI_API void SteamAPI_ISteamApps_RequestAllProofOfPurchaseKeys( ISteamApps* self )
{
    return self->RequestAllProofOfPurchaseKeys();
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamApps_GetFileDetails( ISteamApps* self, const char * pszFileName )
{
    return self->GetFileDetails(pszFileName);
}

STEAMAPI_API int SteamAPI_ISteamApps_GetLaunchCommandLine( ISteamApps* self, char * pszCommandLine, int cubCommandLine )
{
    return self->GetLaunchCommandLine(pszCommandLine, cubCommandLine);
}

STEAMAPI_API steam_bool SteamAPI_ISteamApps_BIsSubscribedFromFamilySharing( ISteamApps* self )
{
    return self->BIsSubscribedFromFamilySharing();
}

STEAMAPI_API steam_bool SteamAPI_ISteamApps_BIsTimedTrial( ISteamApps* self, uint32 * punSecondsAllowed, uint32 * punSecondsPlayed )
{
    return self->BIsTimedTrial(punSecondsAllowed, punSecondsPlayed);
}

STEAMAPI_API steam_bool SteamAPI_ISteamApps_SetDlcContext( ISteamApps* self, AppId_t nAppID )
{
    return self->SetDlcContext(nAppID);
}

// SDK 1.61 removed unAppID param
STEAMAPI_API int SteamAPI_ISteamApps_GetNumBetas(ISteamApps* self, int* pnAvailable, int* pnPrivate)
{
    return self->GetNumBetas(pnAvailable, pnPrivate);
}

// SDK 1.61 removed unAppID param
STEAMAPI_API steam_bool SteamAPI_ISteamApps_GetBetaInfo(ISteamApps* self, int iBetaIndex, uint32* punFlags, uint32* punBuildID, char* pchBetaName, int cchBetaName, char* pchDescription, int cchDescription)
{
    return self->GetBetaInfo(iBetaIndex, punFlags, punBuildID, pchBetaName, cchBetaName, pchDescription, cchDescription);
}

// SDK 1.61 removed unAppID param
STEAMAPI_API steam_bool SteamAPI_ISteamApps_SetActiveBeta(ISteamApps* self, const char* pchBetaName)
{
    return self->SetActiveBeta(pchBetaName);
}

/*
//SDK 1.60
STEAMAPI_API int SteamAPI_ISteamApps_GetNumBetas( ISteamApps* self, AppId_t unAppID, int * pnAvailable, int * pnPrivate )
{
    // in SDK 1.60 param unAppID is unused
    if (get_steam_client()->settings_client->get_local_game_id().AppID() != unAppID) return false;

    return self->GetNumBetas(pnAvailable, pnPrivate);
}

STEAMAPI_API steam_bool SteamAPI_ISteamApps_GetBetaInfo( ISteamApps* self, AppId_t unAppID, int iBetaIndex, uint32 * punFlags, uint32 * punBuildID, char * pchBetaName, int cchBetaName, char * pchDescription, int cchDescription )
{
    // in SDK 1.60 param unAppID is unused
    if (get_steam_client()->settings_client->get_local_game_id().AppID() != unAppID) return false;

    return self->GetBetaInfo(iBetaIndex, punFlags, punBuildID, pchBetaName, cchBetaName, pchDescription, cchDescription);
}

STEAMAPI_API steam_bool SteamAPI_ISteamApps_SetActiveBeta( ISteamApps* self, AppId_t unAppID, const char * pchBetaName )
{
    // in SDK 1.60 param unAppID is unused
    if (get_steam_client()->settings_client->get_local_game_id().AppID() != unAppID) return false;

    return self->SetActiveBeta(pchBetaName);
}
*/

STEAMAPI_API ISteamNetworking *SteamAPI_SteamNetworking_v006()
{
    return get_steam_client()->GetISteamNetworking(flat_hsteamuser(), flat_hsteampipe(), "SteamNetworking006");
}

STEAMAPI_API ISteamNetworking *SteamAPI_SteamGameServerNetworking_v006()
{
    return get_steam_client()->GetISteamNetworking(flat_gs_hsteamuser(), flat_gs_hsteampipe(), "SteamNetworking006");
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworking_SendP2PPacket( ISteamNetworking* self, uint64_steamid steamIDRemote, const void * pubData, uint32 cubData, EP2PSend eP2PSendType, int nChannel )
{
    return self->SendP2PPacket(steamIDRemote, pubData, cubData, eP2PSendType, nChannel);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworking_IsP2PPacketAvailable( ISteamNetworking* self, uint32 * pcubMsgSize, int nChannel )
{
    return self->IsP2PPacketAvailable(pcubMsgSize, nChannel);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworking_ReadP2PPacket( ISteamNetworking* self, void * pubDest, uint32 cubDest, uint32 * pcubMsgSize, CSteamID * psteamIDRemote, int nChannel )
{
    return self->ReadP2PPacket(pubDest, cubDest, pcubMsgSize, psteamIDRemote, nChannel);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworking_AcceptP2PSessionWithUser( ISteamNetworking* self, uint64_steamid steamIDRemote )
{
    return self->AcceptP2PSessionWithUser(steamIDRemote);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworking_CloseP2PSessionWithUser( ISteamNetworking* self, uint64_steamid steamIDRemote )
{
    return self->CloseP2PSessionWithUser(steamIDRemote);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworking_CloseP2PChannelWithUser( ISteamNetworking* self, uint64_steamid steamIDRemote, int nChannel )
{
    return self->CloseP2PChannelWithUser(steamIDRemote, nChannel);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworking_GetP2PSessionState( ISteamNetworking* self, uint64_steamid steamIDRemote, P2PSessionState_t * pConnectionState )
{
    return self->GetP2PSessionState(steamIDRemote, pConnectionState);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworking_AllowP2PPacketRelay( ISteamNetworking* self, bool bAllow )
{
    return self->AllowP2PPacketRelay(bAllow);
}

STEAMAPI_API SNetListenSocket_t SteamAPI_ISteamNetworking_CreateListenSocket( ISteamNetworking* self, int nVirtualP2PPort, SteamIPAddress_t nIP, uint16 nPort, bool bAllowUseOfPacketRelay )
{
    return self->CreateListenSocket(nVirtualP2PPort, nIP, nPort, bAllowUseOfPacketRelay);
}

STEAMAPI_API SNetSocket_t SteamAPI_ISteamNetworking_CreateP2PConnectionSocket( ISteamNetworking* self, uint64_steamid steamIDTarget, int nVirtualPort, int nTimeoutSec, bool bAllowUseOfPacketRelay )
{
    return self->CreateP2PConnectionSocket(steamIDTarget, nVirtualPort, nTimeoutSec, bAllowUseOfPacketRelay);
}

STEAMAPI_API SNetSocket_t SteamAPI_ISteamNetworking_CreateConnectionSocket( ISteamNetworking* self, SteamIPAddress_t nIP, uint16 nPort, int nTimeoutSec )
{
    return self->CreateConnectionSocket(nIP, nPort, nTimeoutSec);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworking_DestroySocket( ISteamNetworking* self, SNetSocket_t hSocket, bool bNotifyRemoteEnd )
{
    return self->DestroySocket(hSocket, bNotifyRemoteEnd);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworking_DestroyListenSocket( ISteamNetworking* self, SNetListenSocket_t hSocket, bool bNotifyRemoteEnd )
{
    return self->DestroyListenSocket(hSocket, bNotifyRemoteEnd);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworking_SendDataOnSocket( ISteamNetworking* self, SNetSocket_t hSocket, void * pubData, uint32 cubData, bool bReliable )
{
    return self->SendDataOnSocket(hSocket, pubData, cubData, bReliable);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworking_IsDataAvailableOnSocket( ISteamNetworking* self, SNetSocket_t hSocket, uint32 * pcubMsgSize )
{
    return self->IsDataAvailableOnSocket(hSocket, pcubMsgSize);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworking_RetrieveDataFromSocket( ISteamNetworking* self, SNetSocket_t hSocket, void * pubDest, uint32 cubDest, uint32 * pcubMsgSize )
{
    return self->RetrieveDataFromSocket(hSocket, pubDest, cubDest, pcubMsgSize);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworking_IsDataAvailable( ISteamNetworking* self, SNetListenSocket_t hListenSocket, uint32 * pcubMsgSize, SNetSocket_t * phSocket )
{
    return self->IsDataAvailable(hListenSocket, pcubMsgSize, phSocket);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworking_RetrieveData( ISteamNetworking* self, SNetListenSocket_t hListenSocket, void * pubDest, uint32 cubDest, uint32 * pcubMsgSize, SNetSocket_t * phSocket )
{
    return self->RetrieveData(hListenSocket, pubDest, cubDest, pcubMsgSize, phSocket);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworking_GetSocketInfo( ISteamNetworking* self, SNetSocket_t hSocket, CSteamID * pSteamIDRemote, int * peSocketStatus, SteamIPAddress_t * punIPRemote, uint16 * punPortRemote )
{
    return self->GetSocketInfo(hSocket, pSteamIDRemote, peSocketStatus, punIPRemote, punPortRemote);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworking_GetListenSocketInfo( ISteamNetworking* self, SNetListenSocket_t hListenSocket, SteamIPAddress_t * pnIP, uint16 * pnPort )
{
    return self->GetListenSocketInfo(hListenSocket, pnIP, pnPort);
}

STEAMAPI_API ESNetSocketConnectionType SteamAPI_ISteamNetworking_GetSocketConnectionType( ISteamNetworking* self, SNetSocket_t hSocket )
{
    return self->GetSocketConnectionType(hSocket);
}

STEAMAPI_API int SteamAPI_ISteamNetworking_GetMaxPacketSize( ISteamNetworking* self, SNetSocket_t hSocket )
{
    return self->GetMaxPacketSize(hSocket);
}

STEAMAPI_API ISteamScreenshots *SteamAPI_SteamScreenshots_v003()
{
    return get_steam_client()->GetISteamScreenshots(flat_hsteamuser(), flat_hsteampipe(), "STEAMSCREENSHOTS_INTERFACE_VERSION003");
}

STEAMAPI_API ScreenshotHandle SteamAPI_ISteamScreenshots_WriteScreenshot( ISteamScreenshots* self, void * pubRGB, uint32 cubRGB, int nWidth, int nHeight )
{
    return (get_steam_client()->steam_screenshots)->WriteScreenshot(pubRGB, cubRGB, nWidth, nHeight);
}

STEAMAPI_API ScreenshotHandle SteamAPI_ISteamScreenshots_AddScreenshotToLibrary( ISteamScreenshots* self, const char * pchFilename, const char * pchThumbnailFilename, int nWidth, int nHeight )
{
    return (get_steam_client()->steam_screenshots)->AddScreenshotToLibrary(pchFilename, pchThumbnailFilename, nWidth, nHeight);
}

STEAMAPI_API void SteamAPI_ISteamScreenshots_TriggerScreenshot( ISteamScreenshots* self )
{
    return (get_steam_client()->steam_screenshots)->TriggerScreenshot();
}

STEAMAPI_API void SteamAPI_ISteamScreenshots_HookScreenshots( ISteamScreenshots* self, bool bHook )
{
    return (get_steam_client()->steam_screenshots)->HookScreenshots(bHook);
}

STEAMAPI_API steam_bool SteamAPI_ISteamScreenshots_SetLocation( ISteamScreenshots* self, ScreenshotHandle hScreenshot, const char * pchLocation )
{
    return (get_steam_client()->steam_screenshots)->SetLocation(hScreenshot, pchLocation);
}

STEAMAPI_API steam_bool SteamAPI_ISteamScreenshots_TagUser( ISteamScreenshots* self, ScreenshotHandle hScreenshot, uint64_steamid steamID )
{
    return (get_steam_client()->steam_screenshots)->TagUser(hScreenshot, steamID);
}

STEAMAPI_API steam_bool SteamAPI_ISteamScreenshots_TagPublishedFile( ISteamScreenshots* self, ScreenshotHandle hScreenshot, PublishedFileId_t unPublishedFileID )
{
    return (get_steam_client()->steam_screenshots)->TagPublishedFile(hScreenshot, unPublishedFileID);
}

STEAMAPI_API steam_bool SteamAPI_ISteamScreenshots_IsScreenshotsHooked( ISteamScreenshots* self )
{
    return (get_steam_client()->steam_screenshots)->IsScreenshotsHooked();
}

STEAMAPI_API ScreenshotHandle SteamAPI_ISteamScreenshots_AddVRScreenshotToLibrary( ISteamScreenshots* self, EVRScreenshotType eType, const char * pchFilename, const char * pchVRFilename )
{
    return (get_steam_client()->steam_screenshots)->AddVRScreenshotToLibrary(eType, pchFilename, pchVRFilename);
}

STEAMAPI_API ISteamMusic *SteamAPI_SteamMusic_v001()
{
    return get_steam_client()->GetISteamMusic(flat_hsteamuser(), flat_hsteampipe(), "STEAMMUSIC_INTERFACE_VERSION001");
}

STEAMAPI_API steam_bool SteamAPI_ISteamMusic_BIsEnabled( ISteamMusic* self )
{
    return self->BIsEnabled();
}

STEAMAPI_API steam_bool SteamAPI_ISteamMusic_BIsPlaying( ISteamMusic* self )
{
    return self->BIsPlaying();
}

STEAMAPI_API AudioPlayback_Status SteamAPI_ISteamMusic_GetPlaybackStatus( ISteamMusic* self )
{
    return self->GetPlaybackStatus();
}

STEAMAPI_API void SteamAPI_ISteamMusic_Play( ISteamMusic* self )
{
    return self->Play();
}

STEAMAPI_API void SteamAPI_ISteamMusic_Pause( ISteamMusic* self )
{
    return self->Pause();
}

STEAMAPI_API void SteamAPI_ISteamMusic_PlayPrevious( ISteamMusic* self )
{
    return self->PlayPrevious();
}

STEAMAPI_API void SteamAPI_ISteamMusic_PlayNext( ISteamMusic* self )
{
    return self->PlayNext();
}

STEAMAPI_API void SteamAPI_ISteamMusic_SetVolume( ISteamMusic* self, float flVolume )
{
    return self->SetVolume(flVolume);
}

STEAMAPI_API float SteamAPI_ISteamMusic_GetVolume( ISteamMusic* self )
{
    return self->GetVolume();
}

STEAMAPI_API ISteamMusicRemote *SteamAPI_SteamMusicRemote_v001()
{
    return get_steam_client()->GetISteamMusicRemote(flat_hsteamuser(), flat_hsteampipe(), "STEAMMUSICREMOTE_INTERFACE_VERSION001");
}

STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_RegisterSteamMusicRemote( ISteamMusicRemote* self, const char * pchName )
{
    return self->RegisterSteamMusicRemote(pchName);
}

STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_DeregisterSteamMusicRemote( ISteamMusicRemote* self )
{
    return self->DeregisterSteamMusicRemote();
}

STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_BIsCurrentMusicRemote( ISteamMusicRemote* self )
{
    return self->BIsCurrentMusicRemote();
}

STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_BActivationSuccess( ISteamMusicRemote* self, bool bValue )
{
    return self->BActivationSuccess(bValue);
}

STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_SetDisplayName( ISteamMusicRemote* self, const char * pchDisplayName )
{
    return self->SetDisplayName(pchDisplayName);
}

STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_SetPNGIcon_64x64( ISteamMusicRemote* self, void * pvBuffer, uint32 cbBufferLength )
{
    return self->SetPNGIcon_64x64(pvBuffer, cbBufferLength);
}

STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_EnablePlayPrevious( ISteamMusicRemote* self, bool bValue )
{
    return self->EnablePlayPrevious(bValue);
}

STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_EnablePlayNext( ISteamMusicRemote* self, bool bValue )
{
    return self->EnablePlayNext(bValue);
}

STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_EnableShuffled( ISteamMusicRemote* self, bool bValue )
{
    return self->EnableShuffled(bValue);
}

STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_EnableLooped( ISteamMusicRemote* self, bool bValue )
{
    return self->EnableLooped(bValue);
}

STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_EnableQueue( ISteamMusicRemote* self, bool bValue )
{
    return self->EnableQueue(bValue);
}

STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_EnablePlaylists( ISteamMusicRemote* self, bool bValue )
{
    return self->EnablePlaylists(bValue);
}

STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_UpdatePlaybackStatus( ISteamMusicRemote* self, AudioPlayback_Status nStatus )
{
    return self->UpdatePlaybackStatus(nStatus);
}

STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_UpdateShuffled( ISteamMusicRemote* self, bool bValue )
{
    return self->UpdateShuffled(bValue);
}

STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_UpdateLooped( ISteamMusicRemote* self, bool bValue )
{
    return self->UpdateLooped(bValue);
}

STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_UpdateVolume( ISteamMusicRemote* self, float flValue )
{
    return self->UpdateVolume(flValue);
}

STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_CurrentEntryWillChange( ISteamMusicRemote* self )
{
    return self->CurrentEntryWillChange();
}

STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_CurrentEntryIsAvailable( ISteamMusicRemote* self, bool bAvailable )
{
    return self->CurrentEntryIsAvailable(bAvailable);
}

STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_UpdateCurrentEntryText( ISteamMusicRemote* self, const char * pchText )
{
    return self->UpdateCurrentEntryText(pchText);
}

STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_UpdateCurrentEntryElapsedSeconds( ISteamMusicRemote* self, int nValue )
{
    return self->UpdateCurrentEntryElapsedSeconds(nValue);
}

STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_UpdateCurrentEntryCoverArt( ISteamMusicRemote* self, void * pvBuffer, uint32 cbBufferLength )
{
    return self->UpdateCurrentEntryCoverArt(pvBuffer, cbBufferLength);
}

STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_CurrentEntryDidChange( ISteamMusicRemote* self )
{
    return self->CurrentEntryDidChange();
}

STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_QueueWillChange( ISteamMusicRemote* self )
{
    return self->QueueWillChange();
}

STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_ResetQueueEntries( ISteamMusicRemote* self )
{
    return self->ResetQueueEntries();
}

STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_SetQueueEntry( ISteamMusicRemote* self, int nID, int nPosition, const char * pchEntryText )
{
    return self->SetQueueEntry(nID, nPosition, pchEntryText);
}

STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_SetCurrentQueueEntry( ISteamMusicRemote* self, int nID )
{
    return self->SetCurrentQueueEntry(nID);
}

STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_QueueDidChange( ISteamMusicRemote* self )
{
    return self->QueueDidChange();
}

STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_PlaylistWillChange( ISteamMusicRemote* self )
{
    return self->PlaylistWillChange();
}

STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_ResetPlaylistEntries( ISteamMusicRemote* self )
{
    return self->ResetPlaylistEntries();
}

STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_SetPlaylistEntry( ISteamMusicRemote* self, int nID, int nPosition, const char * pchEntryText )
{
    return self->SetPlaylistEntry(nID, nPosition, pchEntryText);
}

STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_SetCurrentPlaylistEntry( ISteamMusicRemote* self, int nID )
{
    return self->SetCurrentPlaylistEntry(nID);
}

STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_PlaylistDidChange( ISteamMusicRemote* self )
{
    return self->PlaylistDidChange();
}

STEAMAPI_API ISteamHTTP *SteamAPI_SteamHTTP_v003()
{
    return get_steam_client()->GetISteamHTTP(flat_hsteamuser(), flat_hsteampipe(), "STEAMHTTP_INTERFACE_VERSION003");
}

STEAMAPI_API ISteamHTTP *SteamAPI_SteamGameServerHTTP_v003()
{
    return get_steam_client()->GetISteamHTTP(flat_gs_hsteamuser(), flat_gs_hsteampipe(), "STEAMHTTP_INTERFACE_VERSION003");
}

STEAMAPI_API HTTPRequestHandle SteamAPI_ISteamHTTP_CreateHTTPRequest( ISteamHTTP* self, EHTTPMethod eHTTPRequestMethod, const char * pchAbsoluteURL )
{
    return self->CreateHTTPRequest(eHTTPRequestMethod, pchAbsoluteURL);
}

STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_SetHTTPRequestContextValue( ISteamHTTP* self, HTTPRequestHandle hRequest, uint64 ulContextValue )
{
    return self->SetHTTPRequestContextValue(hRequest, ulContextValue);
}

STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_SetHTTPRequestNetworkActivityTimeout( ISteamHTTP* self, HTTPRequestHandle hRequest, uint32 unTimeoutSeconds )
{
    return self->SetHTTPRequestNetworkActivityTimeout(hRequest, unTimeoutSeconds);
}

STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_SetHTTPRequestHeaderValue( ISteamHTTP* self, HTTPRequestHandle hRequest, const char * pchHeaderName, const char * pchHeaderValue )
{
    return self->SetHTTPRequestHeaderValue(hRequest, pchHeaderName, pchHeaderValue);
}

STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_SetHTTPRequestGetOrPostParameter( ISteamHTTP* self, HTTPRequestHandle hRequest, const char * pchParamName, const char * pchParamValue )
{
    return self->SetHTTPRequestGetOrPostParameter(hRequest, pchParamName, pchParamValue);
}

STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_SendHTTPRequest( ISteamHTTP* self, HTTPRequestHandle hRequest, SteamAPICall_t * pCallHandle )
{
    return self->SendHTTPRequest(hRequest, pCallHandle);
}

STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_SendHTTPRequestAndStreamResponse( ISteamHTTP* self, HTTPRequestHandle hRequest, SteamAPICall_t * pCallHandle )
{
    return self->SendHTTPRequestAndStreamResponse(hRequest, pCallHandle);
}

STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_DeferHTTPRequest( ISteamHTTP* self, HTTPRequestHandle hRequest )
{
    return self->DeferHTTPRequest(hRequest);
}

STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_PrioritizeHTTPRequest( ISteamHTTP* self, HTTPRequestHandle hRequest )
{
    return self->PrioritizeHTTPRequest(hRequest);
}

STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_GetHTTPResponseHeaderSize( ISteamHTTP* self, HTTPRequestHandle hRequest, const char * pchHeaderName, uint32 * unResponseHeaderSize )
{
    return self->GetHTTPResponseHeaderSize(hRequest, pchHeaderName, unResponseHeaderSize);
}

STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_GetHTTPResponseHeaderValue( ISteamHTTP* self, HTTPRequestHandle hRequest, const char * pchHeaderName, uint8 * pHeaderValueBuffer, uint32 unBufferSize )
{
    return self->GetHTTPResponseHeaderValue(hRequest, pchHeaderName, pHeaderValueBuffer, unBufferSize);
}

STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_GetHTTPResponseBodySize( ISteamHTTP* self, HTTPRequestHandle hRequest, uint32 * unBodySize )
{
    return self->GetHTTPResponseBodySize(hRequest, unBodySize);
}

STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_GetHTTPResponseBodyData( ISteamHTTP* self, HTTPRequestHandle hRequest, uint8 * pBodyDataBuffer, uint32 unBufferSize )
{
    return self->GetHTTPResponseBodyData(hRequest, pBodyDataBuffer, unBufferSize);
}

STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_GetHTTPStreamingResponseBodyData( ISteamHTTP* self, HTTPRequestHandle hRequest, uint32 cOffset, uint8 * pBodyDataBuffer, uint32 unBufferSize )
{
    return self->GetHTTPStreamingResponseBodyData(hRequest, cOffset, pBodyDataBuffer, unBufferSize);
}

STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_ReleaseHTTPRequest( ISteamHTTP* self, HTTPRequestHandle hRequest )
{
    return self->ReleaseHTTPRequest(hRequest);
}

STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_GetHTTPDownloadProgressPct( ISteamHTTP* self, HTTPRequestHandle hRequest, float * pflPercentOut )
{
    return self->GetHTTPDownloadProgressPct(hRequest, pflPercentOut);
}

STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_SetHTTPRequestRawPostBody( ISteamHTTP* self, HTTPRequestHandle hRequest, const char * pchContentType, uint8 * pubBody, uint32 unBodyLen )
{
    return self->SetHTTPRequestRawPostBody(hRequest, pchContentType, pubBody, unBodyLen);
}

STEAMAPI_API HTTPCookieContainerHandle SteamAPI_ISteamHTTP_CreateCookieContainer( ISteamHTTP* self, bool bAllowResponsesToModify )
{
    return self->CreateCookieContainer(bAllowResponsesToModify);
}

STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_ReleaseCookieContainer( ISteamHTTP* self, HTTPCookieContainerHandle hCookieContainer )
{
    return self->ReleaseCookieContainer(hCookieContainer);
}

STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_SetCookie( ISteamHTTP* self, HTTPCookieContainerHandle hCookieContainer, const char * pchHost, const char * pchUrl, const char * pchCookie )
{
    return self->SetCookie(hCookieContainer, pchHost, pchUrl, pchCookie);
}

STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_SetHTTPRequestCookieContainer( ISteamHTTP* self, HTTPRequestHandle hRequest, HTTPCookieContainerHandle hCookieContainer )
{
    return self->SetHTTPRequestCookieContainer(hRequest, hCookieContainer);
}

STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_SetHTTPRequestUserAgentInfo( ISteamHTTP* self, HTTPRequestHandle hRequest, const char * pchUserAgentInfo )
{
    return self->SetHTTPRequestUserAgentInfo(hRequest, pchUserAgentInfo);
}

STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_SetHTTPRequestRequiresVerifiedCertificate( ISteamHTTP* self, HTTPRequestHandle hRequest, bool bRequireVerifiedCertificate )
{
    return self->SetHTTPRequestRequiresVerifiedCertificate(hRequest, bRequireVerifiedCertificate);
}

STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_SetHTTPRequestAbsoluteTimeoutMS( ISteamHTTP* self, HTTPRequestHandle hRequest, uint32 unMilliseconds )
{
    return self->SetHTTPRequestAbsoluteTimeoutMS(hRequest, unMilliseconds);
}

STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_GetHTTPRequestWasTimedOut( ISteamHTTP* self, HTTPRequestHandle hRequest, bool * pbWasTimedOut )
{
    return self->GetHTTPRequestWasTimedOut(hRequest, pbWasTimedOut);
}

STEAMAPI_API ISteamInput *SteamAPI_SteamInput_v001()
{
    return get_steam_client()->GetISteamInput(flat_hsteamuser(), flat_hsteampipe(), "SteamInput001");
}

STEAMAPI_API ISteamInput *SteamAPI_SteamInput_v002()
{
    return get_steam_client()->GetISteamInput(flat_hsteamuser(), flat_hsteampipe(), "SteamInput002");
}

STEAMAPI_API ISteamInput *SteamAPI_SteamInput_v005()
{
    return get_steam_client()->GetISteamInput(flat_hsteamuser(), flat_hsteampipe(), "SteamInput005");
}

STEAMAPI_API ISteamInput *SteamAPI_SteamInput_v006()
{
    return get_steam_client()->GetISteamInput(flat_hsteamuser(), flat_hsteampipe(), "SteamInput006");
}

STEAMAPI_API steam_bool SteamAPI_ISteamInput_Init( ISteamInput* self, bool bExplicitlyCallRunFrame )
{
    //TODO: do something with bExplicitlyCallRunFrame
    return (get_steam_client()->steam_controller)->Init();
}

STEAMAPI_API steam_bool SteamAPI_ISteamInput_Shutdown( ISteamInput* self )
{
    return (get_steam_client()->steam_controller)->Shutdown();
}

STEAMAPI_API steam_bool SteamAPI_ISteamInput_SetInputActionManifestFilePath( ISteamInput* self, const char * pchInputActionManifestAbsolutePath )
{
    return (get_steam_client()->steam_controller)->SetInputActionManifestFilePath(pchInputActionManifestAbsolutePath);
}

STEAMAPI_API void SteamAPI_ISteamInput_RunFrame( ISteamInput* self, bool bReservedValue )
{
    return (get_steam_client()->steam_controller)->RunFrame(bReservedValue);
}

STEAMAPI_API steam_bool SteamAPI_ISteamInput_BWaitForData( ISteamInput* self, bool bWaitForever, uint32 unTimeout )
{
    return (get_steam_client()->steam_controller)->BWaitForData(bWaitForever, unTimeout);
}

STEAMAPI_API steam_bool SteamAPI_ISteamInput_BNewDataAvailable( ISteamInput* self )
{
    return (get_steam_client()->steam_controller)->BNewDataAvailable();
}

STEAMAPI_API int SteamAPI_ISteamInput_GetConnectedControllers( ISteamInput* self, InputHandle_t * handlesOut )
{
    return (get_steam_client()->steam_controller)->GetConnectedControllers(handlesOut);
}

STEAMAPI_API void SteamAPI_ISteamInput_EnableDeviceCallbacks( ISteamInput* self )
{
    return (get_steam_client()->steam_controller)->EnableDeviceCallbacks();
}

STEAMAPI_API void SteamAPI_ISteamInput_EnableActionEventCallbacks( ISteamInput* self, SteamInputActionEventCallbackPointer pCallback )
{
    return (get_steam_client()->steam_controller)->EnableActionEventCallbacks(pCallback);
}

STEAMAPI_API InputActionSetHandle_t SteamAPI_ISteamInput_GetActionSetHandle( ISteamInput* self, const char * pszActionSetName )
{
    return (get_steam_client()->steam_controller)->GetActionSetHandle(pszActionSetName);
}

STEAMAPI_API void SteamAPI_ISteamInput_ActivateActionSet( ISteamInput* self, InputHandle_t inputHandle, InputActionSetHandle_t actionSetHandle )
{
    return (get_steam_client()->steam_controller)->ActivateActionSet(inputHandle, actionSetHandle);
}

STEAMAPI_API InputActionSetHandle_t SteamAPI_ISteamInput_GetCurrentActionSet( ISteamInput* self, InputHandle_t inputHandle )
{
    return (get_steam_client()->steam_controller)->GetCurrentActionSet(inputHandle);
}

STEAMAPI_API void SteamAPI_ISteamInput_ActivateActionSetLayer( ISteamInput* self, InputHandle_t inputHandle, InputActionSetHandle_t actionSetLayerHandle )
{
    return (get_steam_client()->steam_controller)->ActivateActionSetLayer(inputHandle, actionSetLayerHandle);
}

STEAMAPI_API void SteamAPI_ISteamInput_DeactivateActionSetLayer( ISteamInput* self, InputHandle_t inputHandle, InputActionSetHandle_t actionSetLayerHandle )
{
    return (get_steam_client()->steam_controller)->DeactivateActionSetLayer(inputHandle, actionSetLayerHandle);
}

STEAMAPI_API void SteamAPI_ISteamInput_DeactivateAllActionSetLayers( ISteamInput* self, InputHandle_t inputHandle )
{
    return (get_steam_client()->steam_controller)->DeactivateAllActionSetLayers(inputHandle);
}

STEAMAPI_API int SteamAPI_ISteamInput_GetActiveActionSetLayers( ISteamInput* self, InputHandle_t inputHandle, InputActionSetHandle_t * handlesOut )
{
    return (get_steam_client()->steam_controller)->GetActiveActionSetLayers(inputHandle, handlesOut);
}

STEAMAPI_API InputDigitalActionHandle_t SteamAPI_ISteamInput_GetDigitalActionHandle( ISteamInput* self, const char * pszActionName )
{
    return (get_steam_client()->steam_controller)->GetDigitalActionHandle(pszActionName);
}

STEAMAPI_API InputDigitalActionData_t SteamAPI_ISteamInput_GetDigitalActionData( ISteamInput* self, InputHandle_t inputHandle, InputDigitalActionHandle_t digitalActionHandle )
{
    return (get_steam_client()->steam_controller)->GetDigitalActionData(inputHandle, digitalActionHandle);
}

STEAMAPI_API int SteamAPI_ISteamInput_GetDigitalActionOrigins( ISteamInput* self, InputHandle_t inputHandle, InputActionSetHandle_t actionSetHandle, InputDigitalActionHandle_t digitalActionHandle, EInputActionOrigin * originsOut )
{
    return (get_steam_client()->steam_controller)->GetDigitalActionOrigins(inputHandle, actionSetHandle, digitalActionHandle, originsOut);
}

STEAMAPI_API const char * SteamAPI_ISteamInput_GetStringForDigitalActionName( ISteamInput* self, InputDigitalActionHandle_t eActionHandle )
{
    return (get_steam_client()->steam_controller)->GetStringForDigitalActionName(eActionHandle);
}

STEAMAPI_API InputAnalogActionHandle_t SteamAPI_ISteamInput_GetAnalogActionHandle( ISteamInput* self, const char * pszActionName )
{
    return (get_steam_client()->steam_controller)->GetAnalogActionHandle(pszActionName);
}

STEAMAPI_API InputAnalogActionData_t SteamAPI_ISteamInput_GetAnalogActionData( ISteamInput* self, InputHandle_t inputHandle, InputAnalogActionHandle_t analogActionHandle )
{
    return (get_steam_client()->steam_controller)->GetAnalogActionData(inputHandle, analogActionHandle);
}

STEAMAPI_API int SteamAPI_ISteamInput_GetAnalogActionOrigins( ISteamInput* self, InputHandle_t inputHandle, InputActionSetHandle_t actionSetHandle, InputAnalogActionHandle_t analogActionHandle, EInputActionOrigin * originsOut )
{
    return (get_steam_client()->steam_controller)->GetAnalogActionOrigins(inputHandle, actionSetHandle, analogActionHandle, originsOut);
}

STEAMAPI_API const char * SteamAPI_ISteamInput_GetGlyphForActionOrigin( ISteamInput* self, EInputActionOrigin eOrigin )
{
    return (get_steam_client()->steam_controller)->GetGlyphForActionOrigin(eOrigin);
}

STEAMAPI_API const char * SteamAPI_ISteamInput_GetGlyphPNGForActionOrigin( ISteamInput* self, EInputActionOrigin eOrigin, ESteamInputGlyphSize eSize, uint32 unFlags )
{
    return (get_steam_client()->steam_controller)->GetGlyphPNGForActionOrigin(eOrigin, eSize, unFlags);
}

STEAMAPI_API const char * SteamAPI_ISteamInput_GetGlyphSVGForActionOrigin( ISteamInput* self, EInputActionOrigin eOrigin, uint32 unFlags )
{
    return (get_steam_client()->steam_controller)->GetGlyphSVGForActionOrigin(eOrigin, unFlags);
}

STEAMAPI_API const char * SteamAPI_ISteamInput_GetGlyphForActionOrigin_Legacy( ISteamInput* self, EInputActionOrigin eOrigin )
{
    return (get_steam_client()->steam_controller)->GetGlyphForActionOrigin_Legacy(eOrigin);
}

STEAMAPI_API const char * SteamAPI_ISteamInput_GetStringForActionOrigin( ISteamInput* self, EInputActionOrigin eOrigin )
{
    return (get_steam_client()->steam_controller)->GetStringForActionOrigin(eOrigin);
}

STEAMAPI_API const char * SteamAPI_ISteamInput_GetStringForAnalogActionName( ISteamInput* self, InputAnalogActionHandle_t eActionHandle )
{
    return (get_steam_client()->steam_controller)->GetStringForAnalogActionName(eActionHandle);
}

STEAMAPI_API void SteamAPI_ISteamInput_StopAnalogActionMomentum( ISteamInput* self, InputHandle_t inputHandle, InputAnalogActionHandle_t eAction )
{
    return (get_steam_client()->steam_controller)->StopAnalogActionMomentum(inputHandle, eAction);
}

STEAMAPI_API InputMotionData_t SteamAPI_ISteamInput_GetMotionData( ISteamInput* self, InputHandle_t inputHandle )
{
    return (get_steam_client()->steam_controller)->GetMotionData(inputHandle);
}

STEAMAPI_API void SteamAPI_ISteamInput_TriggerVibration( ISteamInput* self, InputHandle_t inputHandle, unsigned short usLeftSpeed, unsigned short usRightSpeed )
{
    return (get_steam_client()->steam_controller)->TriggerVibration(inputHandle, usLeftSpeed, usRightSpeed);
}

STEAMAPI_API void SteamAPI_ISteamInput_TriggerVibrationExtended( ISteamInput* self, InputHandle_t inputHandle, unsigned short usLeftSpeed, unsigned short usRightSpeed, unsigned short usLeftTriggerSpeed, unsigned short usRightTriggerSpeed )
{
    return (get_steam_client()->steam_controller)->TriggerVibrationExtended(inputHandle, usLeftSpeed, usRightSpeed, usLeftTriggerSpeed, usRightTriggerSpeed);
}

STEAMAPI_API void SteamAPI_ISteamInput_TriggerSimpleHapticEvent( ISteamInput* self, InputHandle_t inputHandle, EControllerHapticLocation eHapticLocation, uint8 nIntensity, char nGainDB, uint8 nOtherIntensity, char nOtherGainDB )
{
    return (get_steam_client()->steam_controller)->TriggerSimpleHapticEvent(inputHandle, eHapticLocation, nIntensity, nGainDB, nOtherIntensity, nOtherGainDB);
}

STEAMAPI_API void SteamAPI_ISteamInput_SetLEDColor( ISteamInput* self, InputHandle_t inputHandle, uint8 nColorR, uint8 nColorG, uint8 nColorB, unsigned int nFlags )
{
    return (get_steam_client()->steam_controller)->SetLEDColor(inputHandle, nColorR, nColorG, nColorB, nFlags);
}

STEAMAPI_API void SteamAPI_ISteamInput_TriggerHapticPulse( ISteamInput* self, InputHandle_t inputHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec )
{
    return (get_steam_client()->steam_controller)->TriggerHapticPulse(inputHandle, eTargetPad, usDurationMicroSec);
}

STEAMAPI_API void SteamAPI_ISteamInput_TriggerRepeatedHapticPulse( ISteamInput* self, InputHandle_t inputHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec, unsigned short usOffMicroSec, unsigned short unRepeat, unsigned int nFlags )
{
    return (get_steam_client()->steam_controller)->TriggerRepeatedHapticPulse(inputHandle, eTargetPad, usDurationMicroSec, usOffMicroSec, unRepeat, nFlags);
}

STEAMAPI_API void SteamAPI_ISteamInput_Legacy_TriggerHapticPulse( ISteamInput* self, InputHandle_t inputHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec )
{
    return (get_steam_client()->steam_controller)->Legacy_TriggerHapticPulse(inputHandle, eTargetPad, usDurationMicroSec);
}

STEAMAPI_API void SteamAPI_ISteamInput_Legacy_TriggerRepeatedHapticPulse( ISteamInput* self, InputHandle_t inputHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec, unsigned short usOffMicroSec, unsigned short unRepeat, unsigned int nFlags )
{
    return (get_steam_client()->steam_controller)->Legacy_TriggerRepeatedHapticPulse(inputHandle, eTargetPad, usDurationMicroSec, usOffMicroSec, unRepeat, nFlags);
}

STEAMAPI_API steam_bool SteamAPI_ISteamInput_ShowBindingPanel( ISteamInput* self, InputHandle_t inputHandle )
{
    return (get_steam_client()->steam_controller)->ShowBindingPanel(inputHandle);
}

STEAMAPI_API ESteamInputType SteamAPI_ISteamInput_GetInputTypeForHandle( ISteamInput* self, InputHandle_t inputHandle )
{
    return (get_steam_client()->steam_controller)->GetInputTypeForHandle(inputHandle);
}

STEAMAPI_API InputHandle_t SteamAPI_ISteamInput_GetControllerForGamepadIndex( ISteamInput* self, int nIndex )
{
    return (get_steam_client()->steam_controller)->GetControllerForGamepadIndex(nIndex);
}

STEAMAPI_API int SteamAPI_ISteamInput_GetGamepadIndexForController( ISteamInput* self, InputHandle_t ulinputHandle )
{
    return (get_steam_client()->steam_controller)->GetGamepadIndexForController(ulinputHandle);
}

STEAMAPI_API const char * SteamAPI_ISteamInput_GetStringForXboxOrigin( ISteamInput* self, EXboxOrigin eOrigin )
{
    return (get_steam_client()->steam_controller)->GetStringForXboxOrigin(eOrigin);
}

STEAMAPI_API const char * SteamAPI_ISteamInput_GetGlyphForXboxOrigin( ISteamInput* self, EXboxOrigin eOrigin )
{
    return (get_steam_client()->steam_controller)->GetGlyphForXboxOrigin(eOrigin);
}

STEAMAPI_API EInputActionOrigin SteamAPI_ISteamInput_GetActionOriginFromXboxOrigin( ISteamInput* self, InputHandle_t inputHandle, EXboxOrigin eOrigin )
{
    return (get_steam_client()->steam_controller)->GetActionOriginFromXboxOrigin(inputHandle, eOrigin);
}

STEAMAPI_API EInputActionOrigin SteamAPI_ISteamInput_TranslateActionOrigin( ISteamInput* self, ESteamInputType eDestinationInputType, EInputActionOrigin eSourceOrigin )
{
    return (get_steam_client()->steam_controller)->TranslateActionOrigin(eDestinationInputType, eSourceOrigin);
}

STEAMAPI_API steam_bool SteamAPI_ISteamInput_GetDeviceBindingRevision( ISteamInput* self, InputHandle_t inputHandle, int * pMajor, int * pMinor )
{
    return (get_steam_client()->steam_controller)->GetDeviceBindingRevision(inputHandle, pMajor, pMinor);
}

STEAMAPI_API uint32 SteamAPI_ISteamInput_GetRemotePlaySessionID( ISteamInput* self, InputHandle_t inputHandle )
{
    return (get_steam_client()->steam_controller)->GetRemotePlaySessionID(inputHandle);
}

STEAMAPI_API uint16 SteamAPI_ISteamInput_GetSessionInputConfigurationSettings( ISteamInput* self )
{
    return (get_steam_client()->steam_controller)->GetSessionInputConfigurationSettings();
}

STEAMAPI_API void SteamAPI_ISteamInput_SetDualSenseTriggerEffect( ISteamInput* self, InputHandle_t inputHandle, const ScePadTriggerEffectParam * pParam )
{
    return (get_steam_client()->steam_controller)->SetDualSenseTriggerEffect(inputHandle, pParam);
}

STEAMAPI_API ISteamController *SteamAPI_SteamController_v007()
{
    return get_steam_client()->GetISteamController(flat_hsteamuser(), flat_hsteampipe(), "SteamController007");
}

STEAMAPI_API ISteamController *SteamAPI_SteamController_v008()
{
    return get_steam_client()->GetISteamController(flat_hsteamuser(), flat_hsteampipe(), "SteamController008");
}

STEAMAPI_API steam_bool SteamAPI_ISteamController_Init( ISteamController* self )
{
    return (get_steam_client()->steam_controller)->Init();
}

STEAMAPI_API steam_bool SteamAPI_ISteamController_Shutdown( ISteamController* self )
{
    return (get_steam_client()->steam_controller)->Shutdown();
}

STEAMAPI_API void SteamAPI_ISteamController_RunFrame( ISteamController* self )
{
    return (get_steam_client()->steam_controller)->RunFrame();
}

STEAMAPI_API int SteamAPI_ISteamController_GetConnectedControllers( ISteamController* self, ControllerHandle_t * handlesOut )
{
    return (get_steam_client()->steam_controller)->GetConnectedControllers(handlesOut);
}

STEAMAPI_API ControllerActionSetHandle_t SteamAPI_ISteamController_GetActionSetHandle( ISteamController* self, const char * pszActionSetName )
{
    return (get_steam_client()->steam_controller)->GetActionSetHandle(pszActionSetName);
}

STEAMAPI_API void SteamAPI_ISteamController_ActivateActionSet( ISteamController* self, ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetHandle )
{
    return (get_steam_client()->steam_controller)->ActivateActionSet(controllerHandle, actionSetHandle);
}

STEAMAPI_API ControllerActionSetHandle_t SteamAPI_ISteamController_GetCurrentActionSet( ISteamController* self, ControllerHandle_t controllerHandle )
{
    return (get_steam_client()->steam_controller)->GetCurrentActionSet(controllerHandle);
}

STEAMAPI_API void SteamAPI_ISteamController_ActivateActionSetLayer( ISteamController* self, ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetLayerHandle )
{
    return (get_steam_client()->steam_controller)->ActivateActionSetLayer(controllerHandle, actionSetLayerHandle);
}

STEAMAPI_API void SteamAPI_ISteamController_DeactivateActionSetLayer( ISteamController* self, ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetLayerHandle )
{
    return (get_steam_client()->steam_controller)->DeactivateActionSetLayer(controllerHandle, actionSetLayerHandle);
}

STEAMAPI_API void SteamAPI_ISteamController_DeactivateAllActionSetLayers( ISteamController* self, ControllerHandle_t controllerHandle )
{
    return (get_steam_client()->steam_controller)->DeactivateAllActionSetLayers(controllerHandle);
}

STEAMAPI_API int SteamAPI_ISteamController_GetActiveActionSetLayers( ISteamController* self, ControllerHandle_t controllerHandle, ControllerActionSetHandle_t * handlesOut )
{
    return (get_steam_client()->steam_controller)->GetActiveActionSetLayers(controllerHandle, handlesOut);
}

STEAMAPI_API ControllerDigitalActionHandle_t SteamAPI_ISteamController_GetDigitalActionHandle( ISteamController* self, const char * pszActionName )
{
    return (get_steam_client()->steam_controller)->GetDigitalActionHandle(pszActionName);
}

STEAMAPI_API InputDigitalActionData_t SteamAPI_ISteamController_GetDigitalActionData( ISteamController* self, ControllerHandle_t controllerHandle, ControllerDigitalActionHandle_t digitalActionHandle )
{
    return (get_steam_client()->steam_controller)->GetDigitalActionData(controllerHandle, digitalActionHandle);
}

STEAMAPI_API int SteamAPI_ISteamController_GetDigitalActionOrigins( ISteamController* self, ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetHandle, ControllerDigitalActionHandle_t digitalActionHandle, EControllerActionOrigin * originsOut )
{
    return (get_steam_client()->steam_controller)->GetDigitalActionOrigins(controllerHandle, actionSetHandle, digitalActionHandle, originsOut);
}

STEAMAPI_API ControllerAnalogActionHandle_t SteamAPI_ISteamController_GetAnalogActionHandle( ISteamController* self, const char * pszActionName )
{
    return (get_steam_client()->steam_controller)->GetAnalogActionHandle(pszActionName);
}

STEAMAPI_API InputAnalogActionData_t SteamAPI_ISteamController_GetAnalogActionData( ISteamController* self, ControllerHandle_t controllerHandle, ControllerAnalogActionHandle_t analogActionHandle )
{
    return (get_steam_client()->steam_controller)->GetAnalogActionData(controllerHandle, analogActionHandle);
}

STEAMAPI_API int SteamAPI_ISteamController_GetAnalogActionOrigins( ISteamController* self, ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetHandle, ControllerAnalogActionHandle_t analogActionHandle, EControllerActionOrigin * originsOut )
{
    return (get_steam_client()->steam_controller)->GetAnalogActionOrigins(controllerHandle, actionSetHandle, analogActionHandle, originsOut);
}

STEAMAPI_API const char * SteamAPI_ISteamController_GetGlyphForActionOrigin( ISteamController* self, EControllerActionOrigin eOrigin )
{
    return (get_steam_client()->steam_controller)->GetGlyphForActionOrigin(eOrigin);
}

STEAMAPI_API const char * SteamAPI_ISteamController_GetStringForActionOrigin( ISteamController* self, EControllerActionOrigin eOrigin )
{
    return (get_steam_client()->steam_controller)->GetStringForActionOrigin(eOrigin);
}

STEAMAPI_API void SteamAPI_ISteamController_StopAnalogActionMomentum( ISteamController* self, ControllerHandle_t controllerHandle, ControllerAnalogActionHandle_t eAction )
{
    return (get_steam_client()->steam_controller)->StopAnalogActionMomentum(controllerHandle, eAction);
}

STEAMAPI_API InputMotionData_t SteamAPI_ISteamController_GetMotionData( ISteamController* self, ControllerHandle_t controllerHandle )
{
    return (get_steam_client()->steam_controller)->GetMotionData(controllerHandle);
}

STEAMAPI_API void SteamAPI_ISteamController_TriggerHapticPulse( ISteamController* self, ControllerHandle_t controllerHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec )
{
    return (get_steam_client()->steam_controller)->TriggerHapticPulse(controllerHandle, eTargetPad, usDurationMicroSec);
}

STEAMAPI_API void SteamAPI_ISteamController_TriggerRepeatedHapticPulse( ISteamController* self, ControllerHandle_t controllerHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec, unsigned short usOffMicroSec, unsigned short unRepeat, unsigned int nFlags )
{
    return (get_steam_client()->steam_controller)->TriggerRepeatedHapticPulse(controllerHandle, eTargetPad, usDurationMicroSec, usOffMicroSec, unRepeat, nFlags);
}

STEAMAPI_API void SteamAPI_ISteamController_TriggerVibration( ISteamController* self, ControllerHandle_t controllerHandle, unsigned short usLeftSpeed, unsigned short usRightSpeed )
{
    return (get_steam_client()->steam_controller)->TriggerVibration(controllerHandle, usLeftSpeed, usRightSpeed);
}

STEAMAPI_API void SteamAPI_ISteamController_SetLEDColor( ISteamController* self, ControllerHandle_t controllerHandle, uint8 nColorR, uint8 nColorG, uint8 nColorB, unsigned int nFlags )
{
    return (get_steam_client()->steam_controller)->SetLEDColor(controllerHandle, nColorR, nColorG, nColorB, nFlags);
}

STEAMAPI_API steam_bool SteamAPI_ISteamController_ShowBindingPanel( ISteamController* self, ControllerHandle_t controllerHandle )
{
    return (get_steam_client()->steam_controller)->ShowBindingPanel(controllerHandle);
}

STEAMAPI_API ESteamInputType SteamAPI_ISteamController_GetInputTypeForHandle( ISteamController* self, ControllerHandle_t controllerHandle )
{
    return (get_steam_client()->steam_controller)->GetInputTypeForHandle(controllerHandle);
}

STEAMAPI_API ControllerHandle_t SteamAPI_ISteamController_GetControllerForGamepadIndex( ISteamController* self, int nIndex )
{
    return (get_steam_client()->steam_controller)->GetControllerForGamepadIndex(nIndex);
}

STEAMAPI_API int SteamAPI_ISteamController_GetGamepadIndexForController( ISteamController* self, ControllerHandle_t ulControllerHandle )
{
    return (get_steam_client()->steam_controller)->GetGamepadIndexForController(ulControllerHandle);
}

STEAMAPI_API const char * SteamAPI_ISteamController_GetStringForXboxOrigin( ISteamController* self, EXboxOrigin eOrigin )
{
    return (get_steam_client()->steam_controller)->GetStringForXboxOrigin(eOrigin);
}

STEAMAPI_API const char * SteamAPI_ISteamController_GetGlyphForXboxOrigin( ISteamController* self, EXboxOrigin eOrigin )
{
    return (get_steam_client()->steam_controller)->GetGlyphForXboxOrigin(eOrigin);
}

STEAMAPI_API EControllerActionOrigin SteamAPI_ISteamController_GetActionOriginFromXboxOrigin( ISteamController* self, ControllerHandle_t controllerHandle, EXboxOrigin eOrigin )
{
    return (get_steam_client()->steam_controller)->GetActionOriginFromXboxOrigin_(controllerHandle, eOrigin);
}

STEAMAPI_API EControllerActionOrigin SteamAPI_ISteamController_TranslateActionOrigin( ISteamController* self, ESteamInputType eDestinationInputType, EControllerActionOrigin eSourceOrigin )
{
    return (get_steam_client()->steam_controller)->TranslateActionOrigin(eDestinationInputType, eSourceOrigin);
}

STEAMAPI_API steam_bool SteamAPI_ISteamController_GetControllerBindingRevision( ISteamController* self, ControllerHandle_t controllerHandle, int * pMajor, int * pMinor )
{
    return (get_steam_client()->steam_controller)->GetControllerBindingRevision(controllerHandle, pMajor, pMinor);
}

STEAMAPI_API ISteamUGC *SteamAPI_SteamUGC_v014()
{
    return get_steam_client()->GetISteamUGC(flat_hsteamuser(), flat_hsteampipe(), "STEAMUGC_INTERFACE_VERSION014");
}

STEAMAPI_API ISteamUGC *SteamAPI_SteamUGC_v015()
{
    return get_steam_client()->GetISteamUGC(flat_hsteamuser(), flat_hsteampipe(), "STEAMUGC_INTERFACE_VERSION015");
}

STEAMAPI_API ISteamUGC *SteamAPI_SteamUGC_v016()
{
    return get_steam_client()->GetISteamUGC(flat_hsteamuser(), flat_hsteampipe(), "STEAMUGC_INTERFACE_VERSION016");
}

STEAMAPI_API ISteamUGC *SteamAPI_SteamUGC_v017()
{
    return get_steam_client()->GetISteamUGC(flat_hsteamuser(), flat_hsteampipe(), "STEAMUGC_INTERFACE_VERSION017");
}

STEAMAPI_API ISteamUGC *SteamAPI_SteamUGC_v018()
{
    return get_steam_client()->GetISteamUGC(flat_hsteamuser(), flat_hsteampipe(), "STEAMUGC_INTERFACE_VERSION018");
}

STEAMAPI_API ISteamUGC *SteamAPI_SteamUGC_v020()
{
    return get_steam_client()->GetISteamUGC(flat_hsteamuser(), flat_hsteampipe(), "STEAMUGC_INTERFACE_VERSION020");
}

STEAMAPI_API ISteamUGC *SteamAPI_SteamGameServerUGC_v014()
{
    return get_steam_client()->GetISteamUGC(flat_gs_hsteamuser(), flat_gs_hsteampipe(), "STEAMUGC_INTERFACE_VERSION014");
}

STEAMAPI_API ISteamUGC *SteamAPI_SteamGameServerUGC_v015()
{
    return get_steam_client()->GetISteamUGC(flat_gs_hsteamuser(), flat_gs_hsteampipe(), "STEAMUGC_INTERFACE_VERSION015");
}

STEAMAPI_API ISteamUGC *SteamAPI_SteamGameServerUGC_v016()
{
    return get_steam_client()->GetISteamUGC(flat_gs_hsteamuser(), flat_gs_hsteampipe(), "STEAMUGC_INTERFACE_VERSION016");
}

STEAMAPI_API ISteamUGC *SteamAPI_SteamGameServerUGC_v017()
{
    return get_steam_client()->GetISteamUGC(flat_gs_hsteamuser(), flat_gs_hsteampipe(), "STEAMUGC_INTERFACE_VERSION017");
}

STEAMAPI_API ISteamUGC *SteamAPI_SteamGameServerUGC_v018()
{
    return get_steam_client()->GetISteamUGC(flat_gs_hsteamuser(), flat_gs_hsteampipe(), "STEAMUGC_INTERFACE_VERSION018");
}

STEAMAPI_API ISteamUGC *SteamAPI_SteamGameServerUGC_v020()
{
    return get_steam_client()->GetISteamUGC(flat_gs_hsteamuser(), flat_gs_hsteampipe(), "STEAMUGC_INTERFACE_VERSION020");
}

STEAMAPI_API UGCQueryHandle_t SteamAPI_ISteamUGC_CreateQueryUserUGCRequest( ISteamUGC* self, AccountID_t unAccountID, EUserUGCList eListType, EUGCMatchingUGCType eMatchingUGCType, EUserUGCListSortOrder eSortOrder, AppId_t nCreatorAppID, AppId_t nConsumerAppID, uint32 unPage )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->CreateQueryUserUGCRequest(unAccountID, eListType, eMatchingUGCType, eSortOrder, nCreatorAppID, nConsumerAppID, unPage);
}

STEAMAPI_API UGCQueryHandle_t SteamAPI_ISteamUGC_CreateQueryAllUGCRequest(intptr_t instancePtr, EUGCQuery eQueryType, EUGCMatchingUGCType eMatchingeMatchingUGCTypeFileType, AppId_t nCreatorAppID, AppId_t nConsumerAppID, uint32 unPage)
{
    long long client_vftable_distance = ((char *)instancePtr - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)instancePtr - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->CreateQueryAllUGCRequest(eQueryType, eMatchingeMatchingUGCTypeFileType, nCreatorAppID, nConsumerAppID, unPage);
}

STEAMAPI_API UGCQueryHandle_t SteamAPI_ISteamUGC_CreateQueryAllUGCRequest0(intptr_t instancePtr, EUGCQuery eQueryType, EUGCMatchingUGCType eMatchingeMatchingUGCTypeFileType, AppId_t nCreatorAppID, AppId_t nConsumerAppID, const char * pchCursor)
{
    long long client_vftable_distance = ((char *)instancePtr - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)instancePtr - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->CreateQueryAllUGCRequest(eQueryType, eMatchingeMatchingUGCTypeFileType, nCreatorAppID, nConsumerAppID, pchCursor);
}

STEAMAPI_API UGCQueryHandle_t SteamAPI_ISteamUGC_CreateQueryAllUGCRequestPage( ISteamUGC* self, EUGCQuery eQueryType, EUGCMatchingUGCType eMatchingeMatchingUGCTypeFileType, AppId_t nCreatorAppID, AppId_t nConsumerAppID, uint32 unPage )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->CreateQueryAllUGCRequest(eQueryType, eMatchingeMatchingUGCTypeFileType, nCreatorAppID, nConsumerAppID, unPage);
}

STEAMAPI_API UGCQueryHandle_t SteamAPI_ISteamUGC_CreateQueryAllUGCRequestCursor( ISteamUGC* self, EUGCQuery eQueryType, EUGCMatchingUGCType eMatchingeMatchingUGCTypeFileType, AppId_t nCreatorAppID, AppId_t nConsumerAppID, const char * pchCursor )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->CreateQueryAllUGCRequest(eQueryType, eMatchingeMatchingUGCTypeFileType, nCreatorAppID, nConsumerAppID, pchCursor);
}

STEAMAPI_API UGCQueryHandle_t SteamAPI_ISteamUGC_CreateQueryUGCDetailsRequest( ISteamUGC* self, PublishedFileId_t * pvecPublishedFileID, uint32 unNumPublishedFileIDs )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->CreateQueryUGCDetailsRequest(pvecPublishedFileID, unNumPublishedFileIDs);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_SendQueryUGCRequest( ISteamUGC* self, UGCQueryHandle_t handle )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->SendQueryUGCRequest(handle);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_GetQueryUGCResult( ISteamUGC* self, UGCQueryHandle_t handle, uint32 index, SteamUGCDetails_t * pDetails )
{
    return self->GetQueryUGCResult(handle, index, pDetails);
}

STEAMAPI_API uint32 SteamAPI_ISteamUGC_GetQueryUGCNumTags( ISteamUGC* self, UGCQueryHandle_t handle, uint32 index )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->GetQueryUGCNumTags(handle, index);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_GetQueryUGCTag( ISteamUGC* self, UGCQueryHandle_t handle, uint32 index, uint32 indexTag, char * pchValue, uint32 cchValueSize )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->GetQueryUGCTag(handle, index, indexTag, pchValue, cchValueSize);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_GetQueryUGCTagDisplayName( ISteamUGC* self, UGCQueryHandle_t handle, uint32 index, uint32 indexTag, char * pchValue, uint32 cchValueSize )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->GetQueryUGCTagDisplayName(handle, index, indexTag, pchValue, cchValueSize);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_GetQueryUGCPreviewURL( ISteamUGC* self, UGCQueryHandle_t handle, uint32 index, char * pchURL, uint32 cchURLSize )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->GetQueryUGCPreviewURL(handle, index, pchURL, cchURLSize);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_GetQueryUGCMetadata( ISteamUGC* self, UGCQueryHandle_t handle, uint32 index, char * pchMetadata, uint32 cchMetadatasize )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->GetQueryUGCMetadata(handle, index, pchMetadata, cchMetadatasize);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_GetQueryUGCChildren( ISteamUGC* self, UGCQueryHandle_t handle, uint32 index, PublishedFileId_t * pvecPublishedFileID, uint32 cMaxEntries )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->GetQueryUGCChildren(handle, index, pvecPublishedFileID, cMaxEntries);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_GetQueryUGCStatistic( ISteamUGC* self, UGCQueryHandle_t handle, uint32 index, EItemStatistic eStatType, uint64 * pStatValue )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->GetQueryUGCStatistic(handle, index, eStatType, pStatValue);
}

STEAMAPI_API uint32 SteamAPI_ISteamUGC_GetQueryUGCNumAdditionalPreviews( ISteamUGC* self, UGCQueryHandle_t handle, uint32 index )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->GetQueryUGCNumAdditionalPreviews(handle, index);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_GetQueryUGCAdditionalPreview( ISteamUGC* self, UGCQueryHandle_t handle, uint32 index, uint32 previewIndex, char * pchURLOrVideoID, uint32 cchURLSize, char * pchOriginalFileName, uint32 cchOriginalFileNameSize, EItemPreviewType * pPreviewType )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->GetQueryUGCAdditionalPreview(handle, index, previewIndex, pchURLOrVideoID, cchURLSize, pchOriginalFileName, cchOriginalFileNameSize, pPreviewType);
}

STEAMAPI_API uint32 SteamAPI_ISteamUGC_GetQueryUGCNumKeyValueTags( ISteamUGC* self, UGCQueryHandle_t handle, uint32 index )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->GetQueryUGCNumKeyValueTags(handle, index);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_GetQueryUGCKeyValueTag( ISteamUGC* self, UGCQueryHandle_t handle, uint32 index, uint32 keyValueTagIndex, char * pchKey, uint32 cchKeySize, char * pchValue, uint32 cchValueSize )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->GetQueryUGCKeyValueTag(handle, index, keyValueTagIndex, pchKey, cchKeySize, pchValue, cchValueSize);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_GetQueryUGCKeyValueTag0(intptr_t instancePtr, UGCQueryHandle_t handle, uint32 index, const char * pchKey, char * pchValue, uint32 cchValueSize)
{
    long long client_vftable_distance = ((char *)instancePtr - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)instancePtr - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->GetQueryUGCKeyValueTag(handle, index, pchKey, pchValue, cchValueSize);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_GetQueryFirstUGCKeyValueTag( ISteamUGC* self, UGCQueryHandle_t handle, uint32 index, const char * pchKey, char * pchValue, uint32 cchValueSize )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->GetQueryUGCKeyValueTag(handle, index, pchKey, pchValue, cchValueSize);
}

STEAMAPI_API uint32 SteamAPI_ISteamUGC_GetNumSupportedGameVersions( ISteamUGC* self, UGCQueryHandle_t handle, uint32 index )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->GetNumSupportedGameVersions(handle, index);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_GetSupportedGameVersionData( ISteamUGC* self, UGCQueryHandle_t handle, uint32 index, uint32 versionIndex, char * pchGameBranchMin, char * pchGameBranchMax, uint32 cchGameBranchSize )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->GetSupportedGameVersionData(handle, index, versionIndex, pchGameBranchMin, pchGameBranchMax, cchGameBranchSize);
}

STEAMAPI_API uint32 SteamAPI_ISteamUGC_GetQueryUGCContentDescriptors( ISteamUGC* self, UGCQueryHandle_t handle, uint32 index, EUGCContentDescriptorID * pvecDescriptors, uint32 cMaxEntries )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->GetQueryUGCContentDescriptors(handle, index, pvecDescriptors, cMaxEntries);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_ReleaseQueryUGCRequest( ISteamUGC* self, UGCQueryHandle_t handle )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->ReleaseQueryUGCRequest(handle);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_AddRequiredTag( ISteamUGC* self, UGCQueryHandle_t handle, const char * pTagName )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->AddRequiredTag(handle, pTagName);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_AddRequiredTagGroup( ISteamUGC* self, UGCQueryHandle_t handle, const SteamParamStringArray_t * pTagGroups )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->AddRequiredTagGroup(handle, pTagGroups);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_AddExcludedTag( ISteamUGC* self, UGCQueryHandle_t handle, const char * pTagName )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->AddExcludedTag(handle, pTagName);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetReturnOnlyIDs( ISteamUGC* self, UGCQueryHandle_t handle, bool bReturnOnlyIDs )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->SetReturnOnlyIDs(handle, bReturnOnlyIDs);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetReturnKeyValueTags( ISteamUGC* self, UGCQueryHandle_t handle, bool bReturnKeyValueTags )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->SetReturnKeyValueTags(handle, bReturnKeyValueTags);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetReturnLongDescription( ISteamUGC* self, UGCQueryHandle_t handle, bool bReturnLongDescription )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->SetReturnLongDescription(handle, bReturnLongDescription);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetReturnMetadata( ISteamUGC* self, UGCQueryHandle_t handle, bool bReturnMetadata )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->SetReturnMetadata(handle, bReturnMetadata);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetReturnChildren( ISteamUGC* self, UGCQueryHandle_t handle, bool bReturnChildren )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->SetReturnChildren(handle, bReturnChildren);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetReturnAdditionalPreviews( ISteamUGC* self, UGCQueryHandle_t handle, bool bReturnAdditionalPreviews )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->SetReturnAdditionalPreviews(handle, bReturnAdditionalPreviews);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetReturnTotalOnly( ISteamUGC* self, UGCQueryHandle_t handle, bool bReturnTotalOnly )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->SetReturnTotalOnly(handle, bReturnTotalOnly);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetReturnPlaytimeStats( ISteamUGC* self, UGCQueryHandle_t handle, uint32 unDays )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->SetReturnPlaytimeStats(handle, unDays);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetLanguage( ISteamUGC* self, UGCQueryHandle_t handle, const char * pchLanguage )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->SetLanguage(handle, pchLanguage);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetAllowCachedResponse( ISteamUGC* self, UGCQueryHandle_t handle, uint32 unMaxAgeSeconds )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->SetAllowCachedResponse(handle, unMaxAgeSeconds);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetAdminQuery( ISteamUGC* self, UGCUpdateHandle_t handle, bool bAdminQuery )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->SetAdminQuery(handle, bAdminQuery);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetCloudFileNameFilter( ISteamUGC* self, UGCQueryHandle_t handle, const char * pMatchCloudFileName )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->SetCloudFileNameFilter(handle, pMatchCloudFileName);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetMatchAnyTag( ISteamUGC* self, UGCQueryHandle_t handle, bool bMatchAnyTag )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->SetMatchAnyTag(handle, bMatchAnyTag);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetSearchText( ISteamUGC* self, UGCQueryHandle_t handle, const char * pSearchText )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->SetSearchText(handle, pSearchText);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetRankedByTrendDays( ISteamUGC* self, UGCQueryHandle_t handle, uint32 unDays )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->SetRankedByTrendDays(handle, unDays);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetTimeCreatedDateRange( ISteamUGC* self, UGCQueryHandle_t handle, RTime32 rtStart, RTime32 rtEnd )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->SetTimeCreatedDateRange(handle, rtStart, rtEnd);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetTimeUpdatedDateRange( ISteamUGC* self, UGCQueryHandle_t handle, RTime32 rtStart, RTime32 rtEnd )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->SetTimeUpdatedDateRange(handle, rtStart, rtEnd);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_AddRequiredKeyValueTag( ISteamUGC* self, UGCQueryHandle_t handle, const char * pKey, const char * pValue )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->AddRequiredKeyValueTag(handle, pKey, pValue);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_RequestUGCDetails( ISteamUGC* self, PublishedFileId_t nPublishedFileID, uint32 unMaxAgeSeconds )
{
    return self->RequestUGCDetails(nPublishedFileID, unMaxAgeSeconds);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_CreateItem( ISteamUGC* self, AppId_t nConsumerAppId, EWorkshopFileType eFileType )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->CreateItem(nConsumerAppId, eFileType);
}

STEAMAPI_API UGCUpdateHandle_t SteamAPI_ISteamUGC_StartItemUpdate( ISteamUGC* self, AppId_t nConsumerAppId, PublishedFileId_t nPublishedFileID )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->StartItemUpdate(nConsumerAppId, nPublishedFileID);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetItemTitle( ISteamUGC* self, UGCUpdateHandle_t handle, const char * pchTitle )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->SetItemTitle(handle, pchTitle);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetItemDescription( ISteamUGC* self, UGCUpdateHandle_t handle, const char * pchDescription )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->SetItemDescription(handle, pchDescription);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetItemUpdateLanguage( ISteamUGC* self, UGCUpdateHandle_t handle, const char * pchLanguage )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->SetItemUpdateLanguage(handle, pchLanguage);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetItemMetadata( ISteamUGC* self, UGCUpdateHandle_t handle, const char * pchMetaData )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->SetItemMetadata(handle, pchMetaData);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetItemVisibility( ISteamUGC* self, UGCUpdateHandle_t handle, ERemoteStoragePublishedFileVisibility eVisibility )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->SetItemVisibility(handle, eVisibility);
}

//STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetItemTags( ISteamUGC* self, UGCUpdateHandle_t updateHandle, const SteamParamStringArray_t * pTags )
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetItemTags( ISteamUGC* self, UGCUpdateHandle_t updateHandle, const SteamParamStringArray_t * pTags, bool bAllowAdminTags )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    //return (ptr)->SetItemTags(updateHandle, pTags);
    return (ptr)->SetItemTags(updateHandle, pTags, bAllowAdminTags);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetItemContent( ISteamUGC* self, UGCUpdateHandle_t handle, const char * pszContentFolder )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->SetItemContent(handle, pszContentFolder);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetItemPreview( ISteamUGC* self, UGCUpdateHandle_t handle, const char * pszPreviewFile )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->SetItemPreview(handle, pszPreviewFile);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetAllowLegacyUpload( ISteamUGC* self, UGCUpdateHandle_t handle, bool bAllowLegacyUpload )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->SetAllowLegacyUpload(handle, bAllowLegacyUpload);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_RemoveAllItemKeyValueTags( ISteamUGC* self, UGCUpdateHandle_t handle )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->RemoveAllItemKeyValueTags(handle);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_RemoveItemKeyValueTags( ISteamUGC* self, UGCUpdateHandle_t handle, const char * pchKey )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->RemoveItemKeyValueTags(handle, pchKey);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_AddItemKeyValueTag( ISteamUGC* self, UGCUpdateHandle_t handle, const char * pchKey, const char * pchValue )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->AddItemKeyValueTag(handle, pchKey, pchValue);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_AddItemPreviewFile( ISteamUGC* self, UGCUpdateHandle_t handle, const char * pszPreviewFile, EItemPreviewType type )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->AddItemPreviewFile(handle, pszPreviewFile, type);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_AddItemPreviewVideo( ISteamUGC* self, UGCUpdateHandle_t handle, const char * pszVideoID )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->AddItemPreviewVideo(handle, pszVideoID);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_UpdateItemPreviewFile( ISteamUGC* self, UGCUpdateHandle_t handle, uint32 index, const char * pszPreviewFile )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->UpdateItemPreviewFile(handle, index, pszPreviewFile);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_UpdateItemPreviewVideo( ISteamUGC* self, UGCUpdateHandle_t handle, uint32 index, const char * pszVideoID )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->UpdateItemPreviewVideo(handle, index, pszVideoID);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_RemoveItemPreview( ISteamUGC* self, UGCUpdateHandle_t handle, uint32 index )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->RemoveItemPreview(handle, index);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_AddContentDescriptor( ISteamUGC* self, UGCUpdateHandle_t handle, EUGCContentDescriptorID descid )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->AddContentDescriptor(handle, descid);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_RemoveContentDescriptor( ISteamUGC* self, UGCUpdateHandle_t handle, EUGCContentDescriptorID descid )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->RemoveContentDescriptor(handle, descid);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetRequiredGameVersions( ISteamUGC* self, UGCUpdateHandle_t handle, const char * pszGameBranchMin, const char * pszGameBranchMax )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->SetRequiredGameVersions(handle, pszGameBranchMin, pszGameBranchMax);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_SubmitItemUpdate( ISteamUGC* self, UGCUpdateHandle_t handle, const char * pchChangeNote )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->SubmitItemUpdate(handle, pchChangeNote);
}

STEAMAPI_API EItemUpdateStatus SteamAPI_ISteamUGC_GetItemUpdateProgress( ISteamUGC* self, UGCUpdateHandle_t handle, uint64 * punBytesProcessed, uint64 * punBytesTotal )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->GetItemUpdateProgress(handle, punBytesProcessed, punBytesTotal);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_SetUserItemVote( ISteamUGC* self, PublishedFileId_t nPublishedFileID, bool bVoteUp )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->SetUserItemVote(nPublishedFileID, bVoteUp);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_GetUserItemVote( ISteamUGC* self, PublishedFileId_t nPublishedFileID )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->GetUserItemVote(nPublishedFileID);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_AddItemToFavorites( ISteamUGC* self, AppId_t nAppId, PublishedFileId_t nPublishedFileID )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->AddItemToFavorites(nAppId, nPublishedFileID);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_RemoveItemFromFavorites( ISteamUGC* self, AppId_t nAppId, PublishedFileId_t nPublishedFileID )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->RemoveItemFromFavorites(nAppId, nPublishedFileID);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_SubscribeItem( ISteamUGC* self, PublishedFileId_t nPublishedFileID )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->SubscribeItem(nPublishedFileID);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_UnsubscribeItem( ISteamUGC* self, PublishedFileId_t nPublishedFileID )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->UnsubscribeItem(nPublishedFileID);
}

STEAMAPI_API uint32 SteamAPI_ISteamUGC_GetNumSubscribedItems( ISteamUGC* self )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->GetNumSubscribedItems();
}

STEAMAPI_API uint32 SteamAPI_ISteamUGC_GetSubscribedItems( ISteamUGC* self, PublishedFileId_t * pvecPublishedFileID, uint32 cMaxEntries )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->GetSubscribedItems(pvecPublishedFileID, cMaxEntries);
}

STEAMAPI_API uint32 SteamAPI_ISteamUGC_GetItemState( ISteamUGC* self, PublishedFileId_t nPublishedFileID )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->GetItemState(nPublishedFileID);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_GetItemInstallInfo( ISteamUGC* self, PublishedFileId_t nPublishedFileID, uint64 * punSizeOnDisk, char * pchFolder, uint32 cchFolderSize, uint32 * punTimeStamp )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->GetItemInstallInfo(nPublishedFileID, punSizeOnDisk, pchFolder, cchFolderSize, punTimeStamp);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_GetItemDownloadInfo( ISteamUGC* self, PublishedFileId_t nPublishedFileID, uint64 * punBytesDownloaded, uint64 * punBytesTotal )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->GetItemDownloadInfo(nPublishedFileID, punBytesDownloaded, punBytesTotal);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_DownloadItem( ISteamUGC* self, PublishedFileId_t nPublishedFileID, bool bHighPriority )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->DownloadItem(nPublishedFileID, bHighPriority);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_BInitWorkshopForGameServer( ISteamUGC* self, DepotId_t unWorkshopDepotID, const char * pszFolder )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->BInitWorkshopForGameServer(unWorkshopDepotID, pszFolder);
}

STEAMAPI_API void SteamAPI_ISteamUGC_SuspendDownloads( ISteamUGC* self, bool bSuspend )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->SuspendDownloads(bSuspend);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_StartPlaytimeTracking( ISteamUGC* self, PublishedFileId_t * pvecPublishedFileID, uint32 unNumPublishedFileIDs )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->StartPlaytimeTracking(pvecPublishedFileID, unNumPublishedFileIDs);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_StopPlaytimeTracking( ISteamUGC* self, PublishedFileId_t * pvecPublishedFileID, uint32 unNumPublishedFileIDs )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->StopPlaytimeTracking(pvecPublishedFileID, unNumPublishedFileIDs);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_StopPlaytimeTrackingForAllItems( ISteamUGC* self )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->StopPlaytimeTrackingForAllItems();
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_AddDependency( ISteamUGC* self, PublishedFileId_t nParentPublishedFileID, PublishedFileId_t nChildPublishedFileID )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->AddDependency(nParentPublishedFileID, nChildPublishedFileID);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_RemoveDependency( ISteamUGC* self, PublishedFileId_t nParentPublishedFileID, PublishedFileId_t nChildPublishedFileID )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->RemoveDependency(nParentPublishedFileID, nChildPublishedFileID);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_AddAppDependency( ISteamUGC* self, PublishedFileId_t nPublishedFileID, AppId_t nAppID )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->AddAppDependency(nPublishedFileID, nAppID);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_RemoveAppDependency( ISteamUGC* self, PublishedFileId_t nPublishedFileID, AppId_t nAppID )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->RemoveAppDependency(nPublishedFileID, nAppID);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_GetAppDependencies( ISteamUGC* self, PublishedFileId_t nPublishedFileID )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->GetAppDependencies(nPublishedFileID);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_DeleteItem( ISteamUGC* self, PublishedFileId_t nPublishedFileID )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->DeleteItem(nPublishedFileID);
}

STEAMAPI_API steam_bool SteamAPI_ISteamUGC_ShowWorkshopEULA( ISteamUGC* self )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->ShowWorkshopEULA();
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_GetWorkshopEULAStatus( ISteamUGC* self )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->GetWorkshopEULAStatus();
}

STEAMAPI_API uint32 SteamAPI_ISteamUGC_GetUserContentDescriptorPreferences( ISteamUGC* self, EUGCContentDescriptorID * pvecDescriptors, uint32 cMaxEntries )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_ugc);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_ugc);
    auto ptr = get_steam_client()->steam_gameserver_ugc;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_ugc;
    }

    return (ptr)->GetUserContentDescriptorPreferences(pvecDescriptors, cMaxEntries);
}


STEAMAPI_API ISteamAppList *SteamAPI_SteamAppList_v001()
{
    return get_steam_client()->GetISteamAppList(flat_hsteamuser(), flat_hsteampipe(), "STEAMAPPLIST_INTERFACE_VERSION001");
}

STEAMAPI_API uint32 SteamAPI_ISteamAppList_GetNumInstalledApps( ISteamAppList* self )
{
    return self->GetNumInstalledApps();
}

STEAMAPI_API uint32 SteamAPI_ISteamAppList_GetInstalledApps( ISteamAppList* self, AppId_t * pvecAppID, uint32 unMaxAppIDs )
{
    return self->GetInstalledApps(pvecAppID, unMaxAppIDs);
}

STEAMAPI_API int SteamAPI_ISteamAppList_GetAppName( ISteamAppList* self, AppId_t nAppID, char * pchName, int cchNameMax )
{
    return self->GetAppName(nAppID, pchName, cchNameMax);
}

STEAMAPI_API int SteamAPI_ISteamAppList_GetAppInstallDir( ISteamAppList* self, AppId_t nAppID, char * pchDirectory, int cchNameMax )
{
    return self->GetAppInstallDir(nAppID, pchDirectory, cchNameMax);
}

STEAMAPI_API int SteamAPI_ISteamAppList_GetAppBuildId( ISteamAppList* self, AppId_t nAppID )
{
    return self->GetAppBuildId(nAppID);
}

STEAMAPI_API ISteamHTMLSurface *SteamAPI_SteamHTMLSurface_v005()
{
    return get_steam_client()->GetISteamHTMLSurface(flat_hsteamuser(), flat_hsteampipe(), "STEAMHTMLSURFACE_INTERFACE_VERSION_005");
}

STEAMAPI_API void SteamAPI_ISteamHTMLSurface_DestructISteamHTMLSurface(intptr_t instancePtr)
{
    //return (get_steam_client()->steam_HTMLsurface)->DestructISteamHTMLSurface();
}

STEAMAPI_API steam_bool SteamAPI_ISteamHTMLSurface_Init( ISteamHTMLSurface* self )
{
    return (get_steam_client()->steam_HTMLsurface)->Init();
}

STEAMAPI_API steam_bool SteamAPI_ISteamHTMLSurface_Shutdown( ISteamHTMLSurface* self )
{
    return (get_steam_client()->steam_HTMLsurface)->Shutdown();
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamHTMLSurface_CreateBrowser( ISteamHTMLSurface* self, const char * pchUserAgent, const char * pchUserCSS )
{
    return (get_steam_client()->steam_HTMLsurface)->CreateBrowser(pchUserAgent, pchUserCSS);
}

STEAMAPI_API void SteamAPI_ISteamHTMLSurface_RemoveBrowser( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle )
{
    return (get_steam_client()->steam_HTMLsurface)->RemoveBrowser(unBrowserHandle);
}

STEAMAPI_API void SteamAPI_ISteamHTMLSurface_LoadURL( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, const char * pchURL, const char * pchPostData )
{
    return (get_steam_client()->steam_HTMLsurface)->LoadURL(unBrowserHandle, pchURL, pchPostData);
}

STEAMAPI_API void SteamAPI_ISteamHTMLSurface_SetSize( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, uint32 unWidth, uint32 unHeight )
{
    return (get_steam_client()->steam_HTMLsurface)->SetSize(unBrowserHandle, unWidth, unHeight);
}

STEAMAPI_API void SteamAPI_ISteamHTMLSurface_StopLoad( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle )
{
    return (get_steam_client()->steam_HTMLsurface)->StopLoad(unBrowserHandle);
}

STEAMAPI_API void SteamAPI_ISteamHTMLSurface_Reload( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle )
{
    return (get_steam_client()->steam_HTMLsurface)->Reload(unBrowserHandle);
}

STEAMAPI_API void SteamAPI_ISteamHTMLSurface_GoBack( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle )
{
    return (get_steam_client()->steam_HTMLsurface)->GoBack(unBrowserHandle);
}

STEAMAPI_API void SteamAPI_ISteamHTMLSurface_GoForward( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle )
{
    return (get_steam_client()->steam_HTMLsurface)->GoForward(unBrowserHandle);
}

STEAMAPI_API void SteamAPI_ISteamHTMLSurface_AddHeader( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, const char * pchKey, const char * pchValue )
{
    return (get_steam_client()->steam_HTMLsurface)->AddHeader(unBrowserHandle, pchKey, pchValue);
}

STEAMAPI_API void SteamAPI_ISteamHTMLSurface_ExecuteJavascript( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, const char * pchScript )
{
    return (get_steam_client()->steam_HTMLsurface)->ExecuteJavascript(unBrowserHandle, pchScript);
}

STEAMAPI_API void SteamAPI_ISteamHTMLSurface_MouseUp( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, EHTMLMouseButton eMouseButton )
{
    return (get_steam_client()->steam_HTMLsurface)->MouseUp(unBrowserHandle, eMouseButton);
}

STEAMAPI_API void SteamAPI_ISteamHTMLSurface_MouseDown( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, EHTMLMouseButton eMouseButton )
{
    return (get_steam_client()->steam_HTMLsurface)->MouseDown(unBrowserHandle, eMouseButton);
}

STEAMAPI_API void SteamAPI_ISteamHTMLSurface_MouseDoubleClick( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, EHTMLMouseButton eMouseButton )
{
    return (get_steam_client()->steam_HTMLsurface)->MouseDoubleClick(unBrowserHandle, eMouseButton);
}

STEAMAPI_API void SteamAPI_ISteamHTMLSurface_MouseMove( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, int x, int y )
{
    return (get_steam_client()->steam_HTMLsurface)->MouseMove(unBrowserHandle, x, y);
}

STEAMAPI_API void SteamAPI_ISteamHTMLSurface_MouseWheel( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, int32 nDelta )
{
    return (get_steam_client()->steam_HTMLsurface)->MouseWheel(unBrowserHandle, nDelta);
}

STEAMAPI_API void SteamAPI_ISteamHTMLSurface_KeyDown( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, uint32 nNativeKeyCode, EHTMLKeyModifiers eHTMLKeyModifiers, bool bIsSystemKey )
{
    return (get_steam_client()->steam_HTMLsurface)->KeyDown(unBrowserHandle, nNativeKeyCode, eHTMLKeyModifiers, bIsSystemKey);
}

STEAMAPI_API void SteamAPI_ISteamHTMLSurface_KeyUp( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, uint32 nNativeKeyCode, EHTMLKeyModifiers eHTMLKeyModifiers )
{
    return (get_steam_client()->steam_HTMLsurface)->KeyUp(unBrowserHandle, nNativeKeyCode, eHTMLKeyModifiers);
}

STEAMAPI_API void SteamAPI_ISteamHTMLSurface_KeyChar( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, uint32 cUnicodeChar, EHTMLKeyModifiers eHTMLKeyModifiers )
{
    return (get_steam_client()->steam_HTMLsurface)->KeyChar(unBrowserHandle, cUnicodeChar, eHTMLKeyModifiers);
}

STEAMAPI_API void SteamAPI_ISteamHTMLSurface_SetHorizontalScroll( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, uint32 nAbsolutePixelScroll )
{
    return (get_steam_client()->steam_HTMLsurface)->SetHorizontalScroll(unBrowserHandle, nAbsolutePixelScroll);
}

STEAMAPI_API void SteamAPI_ISteamHTMLSurface_SetVerticalScroll( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, uint32 nAbsolutePixelScroll )
{
    return (get_steam_client()->steam_HTMLsurface)->SetVerticalScroll(unBrowserHandle, nAbsolutePixelScroll);
}

STEAMAPI_API void SteamAPI_ISteamHTMLSurface_SetKeyFocus( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, bool bHasKeyFocus )
{
    return (get_steam_client()->steam_HTMLsurface)->SetKeyFocus(unBrowserHandle, bHasKeyFocus);
}

STEAMAPI_API void SteamAPI_ISteamHTMLSurface_ViewSource( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle )
{
    return (get_steam_client()->steam_HTMLsurface)->ViewSource(unBrowserHandle);
}

STEAMAPI_API void SteamAPI_ISteamHTMLSurface_CopyToClipboard( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle )
{
    return (get_steam_client()->steam_HTMLsurface)->CopyToClipboard(unBrowserHandle);
}

STEAMAPI_API void SteamAPI_ISteamHTMLSurface_PasteFromClipboard( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle )
{
    return (get_steam_client()->steam_HTMLsurface)->PasteFromClipboard(unBrowserHandle);
}

STEAMAPI_API void SteamAPI_ISteamHTMLSurface_Find( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, const char * pchSearchStr, bool bCurrentlyInFind, bool bReverse )
{
    return (get_steam_client()->steam_HTMLsurface)->Find(unBrowserHandle, pchSearchStr, bCurrentlyInFind, bReverse);
}

STEAMAPI_API void SteamAPI_ISteamHTMLSurface_StopFind( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle )
{
    return (get_steam_client()->steam_HTMLsurface)->StopFind(unBrowserHandle);
}

STEAMAPI_API void SteamAPI_ISteamHTMLSurface_GetLinkAtPosition( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, int x, int y )
{
    return (get_steam_client()->steam_HTMLsurface)->GetLinkAtPosition(unBrowserHandle, x, y);
}

STEAMAPI_API void SteamAPI_ISteamHTMLSurface_SetCookie( ISteamHTMLSurface* self, const char * pchHostname, const char * pchKey, const char * pchValue, const char * pchPath, RTime32 nExpires, bool bSecure, bool bHTTPOnly )
{
    return (get_steam_client()->steam_HTMLsurface)->SetCookie(pchHostname, pchKey, pchValue, pchPath, nExpires, bSecure, bHTTPOnly);
}

STEAMAPI_API void SteamAPI_ISteamHTMLSurface_SetPageScaleFactor( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, float flZoom, int nPointX, int nPointY )
{
    return (get_steam_client()->steam_HTMLsurface)->SetPageScaleFactor(unBrowserHandle, flZoom, nPointX, nPointY);
}

STEAMAPI_API void SteamAPI_ISteamHTMLSurface_SetBackgroundMode( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, bool bBackgroundMode )
{
    return (get_steam_client()->steam_HTMLsurface)->SetBackgroundMode(unBrowserHandle, bBackgroundMode);
}

STEAMAPI_API void SteamAPI_ISteamHTMLSurface_SetDPIScalingFactor( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, float flDPIScaling )
{
    return (get_steam_client()->steam_HTMLsurface)->SetDPIScalingFactor(unBrowserHandle, flDPIScaling);
}

STEAMAPI_API void SteamAPI_ISteamHTMLSurface_OpenDeveloperTools( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle )
{
    return (get_steam_client()->steam_HTMLsurface)->OpenDeveloperTools(unBrowserHandle);
}

STEAMAPI_API void SteamAPI_ISteamHTMLSurface_AllowStartRequest( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, bool bAllowed )
{
    return (get_steam_client()->steam_HTMLsurface)->AllowStartRequest(unBrowserHandle, bAllowed);
}

STEAMAPI_API void SteamAPI_ISteamHTMLSurface_JSDialogResponse( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, bool bResult )
{
    return (get_steam_client()->steam_HTMLsurface)->JSDialogResponse(unBrowserHandle, bResult);
}

STEAMAPI_API void SteamAPI_ISteamHTMLSurface_FileLoadDialogResponse( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, const char ** pchSelectedFiles )
{
    return (get_steam_client()->steam_HTMLsurface)->FileLoadDialogResponse(unBrowserHandle, pchSelectedFiles);
}

STEAMAPI_API ISteamInventory *SteamAPI_SteamInventory_v003()
{
    return get_steam_client()->GetISteamInventory(flat_hsteamuser(), flat_hsteampipe(), "STEAMINVENTORY_INTERFACE_V003");
}

STEAMAPI_API ISteamInventory *SteamAPI_SteamGameServerInventory_v003()
{
    return get_steam_client()->GetISteamInventory(flat_gs_hsteamuser(), flat_gs_hsteampipe(), "STEAMINVENTORY_INTERFACE_V003");
}

STEAMAPI_API EResult SteamAPI_ISteamInventory_GetResultStatus( ISteamInventory* self, SteamInventoryResult_t resultHandle )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->GetResultStatus(resultHandle);
}

STEAMAPI_API steam_bool SteamAPI_ISteamInventory_GetResultItems( ISteamInventory* self, SteamInventoryResult_t resultHandle, SteamItemDetails_t * pOutItemsArray, uint32 * punOutItemsArraySize )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->GetResultItems(resultHandle, pOutItemsArray, punOutItemsArraySize);
}

STEAMAPI_API steam_bool SteamAPI_ISteamInventory_GetResultItemProperty( ISteamInventory* self, SteamInventoryResult_t resultHandle, uint32 unItemIndex, const char * pchPropertyName, char * pchValueBuffer, uint32 * punValueBufferSizeOut )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->GetResultItemProperty(resultHandle, unItemIndex, pchPropertyName, pchValueBuffer, punValueBufferSizeOut);
}

STEAMAPI_API uint32 SteamAPI_ISteamInventory_GetResultTimestamp( ISteamInventory* self, SteamInventoryResult_t resultHandle )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->GetResultTimestamp(resultHandle);
}

STEAMAPI_API steam_bool SteamAPI_ISteamInventory_CheckResultSteamID( ISteamInventory* self, SteamInventoryResult_t resultHandle, uint64_steamid steamIDExpected )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->CheckResultSteamID(resultHandle, steamIDExpected);
}

STEAMAPI_API void SteamAPI_ISteamInventory_DestroyResult( ISteamInventory* self, SteamInventoryResult_t resultHandle )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->DestroyResult(resultHandle);
}

STEAMAPI_API steam_bool SteamAPI_ISteamInventory_GetAllItems( ISteamInventory* self, SteamInventoryResult_t * pResultHandle )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->GetAllItems(pResultHandle);
}

STEAMAPI_API steam_bool SteamAPI_ISteamInventory_GetItemsByID( ISteamInventory* self, SteamInventoryResult_t * pResultHandle, const SteamItemInstanceID_t * pInstanceIDs, uint32 unCountInstanceIDs )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->GetItemsByID(pResultHandle, pInstanceIDs, unCountInstanceIDs);
}

STEAMAPI_API steam_bool SteamAPI_ISteamInventory_SerializeResult( ISteamInventory* self, SteamInventoryResult_t resultHandle, void * pOutBuffer, uint32 * punOutBufferSize )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->SerializeResult(resultHandle, pOutBuffer, punOutBufferSize);
}

STEAMAPI_API steam_bool SteamAPI_ISteamInventory_DeserializeResult( ISteamInventory* self, SteamInventoryResult_t * pOutResultHandle, const void * pBuffer, uint32 unBufferSize, bool bRESERVED_MUST_BE_FALSE )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->DeserializeResult(pOutResultHandle, pBuffer, unBufferSize, bRESERVED_MUST_BE_FALSE);
}

STEAMAPI_API steam_bool SteamAPI_ISteamInventory_GenerateItems( ISteamInventory* self, SteamInventoryResult_t * pResultHandle, const SteamItemDef_t * pArrayItemDefs, const uint32 * punArrayQuantity, uint32 unArrayLength )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->GenerateItems(pResultHandle, pArrayItemDefs, punArrayQuantity, unArrayLength);
}

STEAMAPI_API steam_bool SteamAPI_ISteamInventory_GrantPromoItems( ISteamInventory* self, SteamInventoryResult_t * pResultHandle )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->GrantPromoItems(pResultHandle);
}

STEAMAPI_API steam_bool SteamAPI_ISteamInventory_AddPromoItem( ISteamInventory* self, SteamInventoryResult_t * pResultHandle, SteamItemDef_t itemDef )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->AddPromoItem(pResultHandle, itemDef);
}

STEAMAPI_API steam_bool SteamAPI_ISteamInventory_AddPromoItems( ISteamInventory* self, SteamInventoryResult_t * pResultHandle, const SteamItemDef_t * pArrayItemDefs, uint32 unArrayLength )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->AddPromoItems(pResultHandle, pArrayItemDefs, unArrayLength);
}

STEAMAPI_API steam_bool SteamAPI_ISteamInventory_ConsumeItem( ISteamInventory* self, SteamInventoryResult_t * pResultHandle, SteamItemInstanceID_t itemConsume, uint32 unQuantity )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->ConsumeItem(pResultHandle, itemConsume, unQuantity);
}

STEAMAPI_API steam_bool SteamAPI_ISteamInventory_ExchangeItems( ISteamInventory* self, SteamInventoryResult_t * pResultHandle, const SteamItemDef_t * pArrayGenerate, const uint32 * punArrayGenerateQuantity, uint32 unArrayGenerateLength, const SteamItemInstanceID_t * pArrayDestroy, const uint32 * punArrayDestroyQuantity, uint32 unArrayDestroyLength )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->ExchangeItems(pResultHandle, pArrayGenerate, punArrayGenerateQuantity, unArrayGenerateLength, pArrayDestroy, punArrayDestroyQuantity, unArrayDestroyLength);
}

STEAMAPI_API steam_bool SteamAPI_ISteamInventory_TransferItemQuantity( ISteamInventory* self, SteamInventoryResult_t * pResultHandle, SteamItemInstanceID_t itemIdSource, uint32 unQuantity, SteamItemInstanceID_t itemIdDest )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->TransferItemQuantity(pResultHandle, itemIdSource, unQuantity, itemIdDest);
}

STEAMAPI_API void SteamAPI_ISteamInventory_SendItemDropHeartbeat( ISteamInventory* self )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->SendItemDropHeartbeat();
}

STEAMAPI_API steam_bool SteamAPI_ISteamInventory_TriggerItemDrop( ISteamInventory* self, SteamInventoryResult_t * pResultHandle, SteamItemDef_t dropListDefinition )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->TriggerItemDrop(pResultHandle, dropListDefinition);
}

STEAMAPI_API steam_bool SteamAPI_ISteamInventory_TradeItems( ISteamInventory* self, SteamInventoryResult_t * pResultHandle, uint64_steamid steamIDTradePartner, const SteamItemInstanceID_t * pArrayGive, const uint32 * pArrayGiveQuantity, uint32 nArrayGiveLength, const SteamItemInstanceID_t * pArrayGet, const uint32 * pArrayGetQuantity, uint32 nArrayGetLength )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->TradeItems(pResultHandle, steamIDTradePartner, pArrayGive, pArrayGiveQuantity, nArrayGiveLength, pArrayGet, pArrayGetQuantity, nArrayGetLength);
}

STEAMAPI_API steam_bool SteamAPI_ISteamInventory_LoadItemDefinitions( ISteamInventory* self )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->LoadItemDefinitions();
}

STEAMAPI_API steam_bool SteamAPI_ISteamInventory_GetItemDefinitionIDs( ISteamInventory* self, SteamItemDef_t * pItemDefIDs, uint32 * punItemDefIDsArraySize )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->GetItemDefinitionIDs(pItemDefIDs, punItemDefIDsArraySize);
}

STEAMAPI_API steam_bool SteamAPI_ISteamInventory_GetItemDefinitionProperty( ISteamInventory* self, SteamItemDef_t iDefinition, const char * pchPropertyName, char * pchValueBuffer, uint32 * punValueBufferSizeOut )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->GetItemDefinitionProperty(iDefinition, pchPropertyName, pchValueBuffer, punValueBufferSizeOut);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamInventory_RequestEligiblePromoItemDefinitionsIDs( ISteamInventory* self, uint64_steamid steamID )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->RequestEligiblePromoItemDefinitionsIDs(steamID);
}

STEAMAPI_API steam_bool SteamAPI_ISteamInventory_GetEligiblePromoItemDefinitionIDs( ISteamInventory* self, uint64_steamid steamID, SteamItemDef_t * pItemDefIDs, uint32 * punItemDefIDsArraySize )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->GetEligiblePromoItemDefinitionIDs(steamID, pItemDefIDs, punItemDefIDsArraySize);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamInventory_StartPurchase( ISteamInventory* self, const SteamItemDef_t * pArrayItemDefs, const uint32 * punArrayQuantity, uint32 unArrayLength )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->StartPurchase(pArrayItemDefs, punArrayQuantity, unArrayLength);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamInventory_RequestPrices( ISteamInventory* self )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->RequestPrices();
}

STEAMAPI_API uint32 SteamAPI_ISteamInventory_GetNumItemsWithPrices( ISteamInventory* self )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->GetNumItemsWithPrices();
}

STEAMAPI_API steam_bool SteamAPI_ISteamInventory_GetItemsWithPrices( ISteamInventory* self, SteamItemDef_t * pArrayItemDefs, uint64 * pCurrentPrices, uint64 * pBasePrices, uint32 unArrayLength )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->GetItemsWithPrices(pArrayItemDefs, pCurrentPrices, pBasePrices, unArrayLength);
}

STEAMAPI_API steam_bool SteamAPI_ISteamInventory_GetItemPrice( ISteamInventory* self, SteamItemDef_t iDefinition, uint64 * pCurrentPrice, uint64 * pBasePrice )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->GetItemPrice(iDefinition, pCurrentPrice, pBasePrice);
}

STEAMAPI_API SteamInventoryUpdateHandle_t SteamAPI_ISteamInventory_StartUpdateProperties( ISteamInventory* self )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->StartUpdateProperties();
}

STEAMAPI_API steam_bool SteamAPI_ISteamInventory_RemoveProperty( ISteamInventory* self, SteamInventoryUpdateHandle_t handle, SteamItemInstanceID_t nItemID, const char * pchPropertyName )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->RemoveProperty(handle, nItemID, pchPropertyName);
}

STEAMAPI_API steam_bool SteamAPI_ISteamInventory_SetProperty(intptr_t instancePtr, SteamInventoryUpdateHandle_t handle, SteamItemInstanceID_t nItemID, const char * pchPropertyName, const char * pchPropertyValue)
{
    long long client_vftable_distance = ((char *)instancePtr - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)instancePtr - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->SetProperty(handle, nItemID, pchPropertyName, pchPropertyValue);
}

STEAMAPI_API steam_bool SteamAPI_ISteamInventory_SetProperty0(intptr_t instancePtr, SteamInventoryUpdateHandle_t handle, SteamItemInstanceID_t nItemID, const char * pchPropertyName, bool bValue)
{
    long long client_vftable_distance = ((char *)instancePtr - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)instancePtr - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->SetProperty(handle, nItemID, pchPropertyName, bValue);
}

STEAMAPI_API steam_bool SteamAPI_ISteamInventory_SetProperty1(intptr_t instancePtr, SteamInventoryUpdateHandle_t handle, SteamItemInstanceID_t nItemID, const char * pchPropertyName, int64 nValue)
{
    long long client_vftable_distance = ((char *)instancePtr - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)instancePtr - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->SetProperty(handle, nItemID, pchPropertyName, nValue);
}

STEAMAPI_API steam_bool SteamAPI_ISteamInventory_SetProperty2(intptr_t instancePtr, SteamInventoryUpdateHandle_t handle, SteamItemInstanceID_t nItemID, const char * pchPropertyName, float flValue)
{
    long long client_vftable_distance = ((char *)instancePtr - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)instancePtr - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->SetProperty(handle, nItemID, pchPropertyName, flValue);
}

STEAMAPI_API steam_bool SteamAPI_ISteamInventory_SetPropertyString( ISteamInventory* self, SteamInventoryUpdateHandle_t handle, SteamItemInstanceID_t nItemID, const char * pchPropertyName, const char * pchPropertyValue )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->SetProperty(handle, nItemID, pchPropertyName, pchPropertyValue);
}

STEAMAPI_API steam_bool SteamAPI_ISteamInventory_SetPropertyBool( ISteamInventory* self, SteamInventoryUpdateHandle_t handle, SteamItemInstanceID_t nItemID, const char * pchPropertyName, bool bValue )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->SetProperty(handle, nItemID, pchPropertyName, bValue);
}

STEAMAPI_API steam_bool SteamAPI_ISteamInventory_SetPropertyInt64( ISteamInventory* self, SteamInventoryUpdateHandle_t handle, SteamItemInstanceID_t nItemID, const char * pchPropertyName, int64 nValue )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->SetProperty(handle, nItemID, pchPropertyName, nValue);
}

STEAMAPI_API steam_bool SteamAPI_ISteamInventory_SetPropertyFloat( ISteamInventory* self, SteamInventoryUpdateHandle_t handle, SteamItemInstanceID_t nItemID, const char * pchPropertyName, float flValue )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->SetProperty(handle, nItemID, pchPropertyName, flValue);
}

STEAMAPI_API steam_bool SteamAPI_ISteamInventory_SubmitUpdateProperties( ISteamInventory* self, SteamInventoryUpdateHandle_t handle, SteamInventoryResult_t * pResultHandle )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->SubmitUpdateProperties(handle, pResultHandle);
}

STEAMAPI_API steam_bool SteamAPI_ISteamInventory_InspectItem( ISteamInventory* self, SteamInventoryResult_t * pResultHandle, const char * pchItemToken )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_inventory);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_inventory);
    auto ptr = get_steam_client()->steam_gameserver_inventory;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_inventory;
    }

    return (ptr)->InspectItem(pResultHandle, pchItemToken);
}

STEAMAPI_API ISteamTimeline *SteamAPI_SteamTimeline_v004()
{
    return get_steam_client()->GetISteamTimeline(flat_hsteamuser(), flat_hsteampipe(), "STEAMTIMELINE_INTERFACE_V004");
}

STEAMAPI_API ISteamTimeline *SteamAPI_SteamTimeline_v001()
{
    return get_steam_client()->GetISteamTimeline(flat_hsteamuser(), flat_hsteampipe(), "STEAMTIMELINE_INTERFACE_V001");
}

STEAMAPI_API void SteamAPI_ISteamTimeline_SetTimelineStateDescription( ISteamTimeline* self, const char * pchDescription, float flTimeDelta )
{
    return (get_steam_client()->steam_timeline)->SetTimelineStateDescription(pchDescription, flTimeDelta);
}

STEAMAPI_API void SteamAPI_ISteamTimeline_ClearTimelineStateDescription( ISteamTimeline* self, float flTimeDelta )
{
    return (get_steam_client()->steam_timeline)->ClearTimelineStateDescription(flTimeDelta);
}

STEAMAPI_API void SteamAPI_ISteamTimeline_AddTimelineEvent( ISteamTimeline* self, const char * pchIcon, const char * pchTitle, const char * pchDescription, uint32 unPriority, float flStartOffsetSeconds, float flDurationSeconds, ETimelineEventClipPriority ePossibleClip )
{
    get_steam_client()->steam_timeline->AddTimelineEvent_old(pchIcon, pchTitle, pchDescription, unPriority, flStartOffsetSeconds, flDurationSeconds, ePossibleClip);
}

STEAMAPI_API void SteamAPI_ISteamTimeline_SetTimelineGameMode( ISteamTimeline* self, ETimelineGameMode eMode )
{
    (get_steam_client()->steam_timeline)->SetTimelineGameMode(eMode);
}

STEAMAPI_API void SteamAPI_ISteamTimeline_SetTimelineTooltip(ISteamTimeline* self, const char* pchDescription, float flTimeDelta)
{
    (get_steam_client()->steam_timeline)->SetTimelineTooltip(pchDescription, flTimeDelta);
}

STEAMAPI_API void SteamAPI_ISteamTimeline_ClearTimelineTooltip(ISteamTimeline* self, float flTimeDelta)
{
    return (get_steam_client()->steam_timeline)->ClearTimelineTooltip(flTimeDelta);
}

STEAMAPI_API TimelineEventHandle_t SteamAPI_ISteamTimeline_AddInstantaneousTimelineEvent(ISteamTimeline* self, const char* pchTitle, const char* pchDescription, const char* pchIcon, uint32 unIconPriority, float flStartOffsetSeconds, ETimelineEventClipPriority ePossibleClip)
{
    return (get_steam_client()->steam_timeline)->AddInstantaneousTimelineEvent(pchTitle, pchDescription, pchIcon, unIconPriority, flStartOffsetSeconds, ePossibleClip);
}

STEAMAPI_API TimelineEventHandle_t SteamAPI_ISteamTimeline_AddRangeTimelineEvent(ISteamTimeline* self, const char* pchTitle, const char* pchDescription, const char* pchIcon, uint32 unIconPriority, float flStartOffsetSeconds, float flDuration, ETimelineEventClipPriority ePossibleClip)
{
    return (get_steam_client()->steam_timeline)->AddRangeTimelineEvent(pchTitle, pchDescription, pchIcon, unIconPriority, flStartOffsetSeconds, flDuration, ePossibleClip);
}

STEAMAPI_API TimelineEventHandle_t SteamAPI_ISteamTimeline_StartRangeTimelineEvent(ISteamTimeline* self, const char* pchTitle, const char* pchDescription, const char* pchIcon, uint32 unPriority, float flStartOffsetSeconds, ETimelineEventClipPriority ePossibleClip)
{
    return (get_steam_client()->steam_timeline)->StartRangeTimelineEvent(pchTitle, pchDescription, pchIcon, unPriority, flStartOffsetSeconds, ePossibleClip);
}

STEAMAPI_API void SteamAPI_ISteamTimeline_UpdateRangeTimelineEvent(ISteamTimeline* self, TimelineEventHandle_t ulEvent, const char* pchTitle, const char* pchDescription, const char* pchIcon, uint32 unPriority, ETimelineEventClipPriority ePossibleClip)
{
    return (get_steam_client()->steam_timeline)->UpdateRangeTimelineEvent(ulEvent, pchTitle, pchDescription, pchIcon, unPriority, ePossibleClip);
}

STEAMAPI_API void SteamAPI_ISteamTimeline_EndRangeTimelineEvent(ISteamTimeline* self, TimelineEventHandle_t ulEvent, float flEndOffsetSeconds)
{
    return (get_steam_client()->steam_timeline)->EndRangeTimelineEvent(ulEvent, flEndOffsetSeconds);
}

STEAMAPI_API void SteamAPI_ISteamTimeline_RemoveTimelineEvent(ISteamTimeline* self, TimelineEventHandle_t ulEvent)
{
    return (get_steam_client()->steam_timeline)->RemoveTimelineEvent(ulEvent);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamTimeline_DoesEventRecordingExist(ISteamTimeline* self, TimelineEventHandle_t ulEvent)
{
    return (get_steam_client()->steam_timeline)->DoesEventRecordingExist(ulEvent);
}

STEAMAPI_API void SteamAPI_ISteamTimeline_StartGamePhase(ISteamTimeline* self)
{
    return (get_steam_client()->steam_timeline)->StartGamePhase();
}

STEAMAPI_API void SteamAPI_ISteamTimeline_EndGamePhase(ISteamTimeline* self)
{
    return (get_steam_client()->steam_timeline)->EndGamePhase();
}

STEAMAPI_API void SteamAPI_ISteamTimeline_SetGamePhaseID(ISteamTimeline* self, const char* pchPhaseID)
{
    return (get_steam_client()->steam_timeline)->SetGamePhaseID(pchPhaseID);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamTimeline_DoesGamePhaseRecordingExist(ISteamTimeline* self, const char* pchPhaseID)
{
    return (get_steam_client()->steam_timeline)->DoesGamePhaseRecordingExist(pchPhaseID);
}

STEAMAPI_API void SteamAPI_ISteamTimeline_AddGamePhaseTag(ISteamTimeline* self, const char* pchTagName, const char* pchTagIcon, const char* pchTagGroup, uint32 unPriority)
{
    return (get_steam_client()->steam_timeline)->AddGamePhaseTag(pchTagName, pchTagIcon, pchTagGroup, unPriority);
}

STEAMAPI_API void SteamAPI_ISteamTimeline_SetGamePhaseAttribute(ISteamTimeline* self, const char* pchAttributeGroup, const char* pchAttributeValue, uint32 unPriority)
{
    return (get_steam_client()->steam_timeline)->SetGamePhaseAttribute(pchAttributeGroup, pchAttributeValue, unPriority);
}

STEAMAPI_API void SteamAPI_ISteamTimeline_OpenOverlayToGamePhase(ISteamTimeline* self, const char* pchPhaseID)
{
    return (get_steam_client()->steam_timeline)->OpenOverlayToGamePhase(pchPhaseID);
}

STEAMAPI_API void SteamAPI_ISteamTimeline_OpenOverlayToTimelineEvent(ISteamTimeline* self, const TimelineEventHandle_t ulEvent)
{
    return (get_steam_client()->steam_timeline)->OpenOverlayToTimelineEvent(ulEvent);
}

STEAMAPI_API ISteamVideo *SteamAPI_SteamVideo_v001()
{
    return get_steam_client()->GetISteamVideo(flat_hsteamuser(), flat_hsteampipe(), "STEAMVIDEO_INTERFACE_V001");
}

STEAMAPI_API ISteamVideo *SteamAPI_SteamVideo_v002()
{
    return get_steam_client()->GetISteamVideo(flat_hsteamuser(), flat_hsteampipe(), "STEAMVIDEO_INTERFACE_V002");
}

STEAMAPI_API ISteamVideo *SteamAPI_SteamVideo_v007()
{
    return get_steam_client()->GetISteamVideo(flat_hsteamuser(), flat_hsteampipe(), "STEAMVIDEO_INTERFACE_V007");
}

STEAMAPI_API void SteamAPI_ISteamVideo_GetVideoURL( ISteamVideo* self, AppId_t unVideoAppID )
{
    return self->GetVideoURL(unVideoAppID);
}

STEAMAPI_API steam_bool SteamAPI_ISteamVideo_IsBroadcasting( ISteamVideo* self, int * pnNumViewers )
{
    return self->IsBroadcasting(pnNumViewers);
}

STEAMAPI_API void SteamAPI_ISteamVideo_GetOPFSettings( ISteamVideo* self, AppId_t unVideoAppID )
{
    return self->GetOPFSettings(unVideoAppID);
}

STEAMAPI_API steam_bool SteamAPI_ISteamVideo_GetOPFStringForApp( ISteamVideo* self, AppId_t unVideoAppID, char * pchBuffer, int32 * pnBufferSize )
{
    return self->GetOPFStringForApp(unVideoAppID, pchBuffer, pnBufferSize);
}

ISteamTV *SteamAPI_SteamTV_v001()
{
    return (ISteamTV *)get_steam_client()->GetISteamGenericInterface(flat_hsteamuser(), flat_hsteampipe(), "STEAMTV_INTERFACE_V001");
}

STEAMAPI_API steam_bool SteamAPI_ISteamTV_IsBroadcasting( ISteamTV* self, int * pnNumViewers )
{
    return self->IsBroadcasting(pnNumViewers);
}

STEAMAPI_API void SteamAPI_ISteamTV_AddBroadcastGameData( ISteamTV* self, const char * pchKey, const char * pchValue )
{
    return self->AddBroadcastGameData(pchKey, pchValue);
}

STEAMAPI_API void SteamAPI_ISteamTV_RemoveBroadcastGameData( ISteamTV* self, const char * pchKey )
{
    return self->RemoveBroadcastGameData(pchKey);
}

STEAMAPI_API void SteamAPI_ISteamTV_AddTimelineMarker( ISteamTV* self, const char * pchTemplateName, bool bPersistent, uint8 nColorR, uint8 nColorG, uint8 nColorB )
{
    return self->AddTimelineMarker(pchTemplateName, bPersistent, nColorR, nColorG, nColorB);
}

STEAMAPI_API void SteamAPI_ISteamTV_RemoveTimelineMarker( ISteamTV* self )
{
    return self->RemoveTimelineMarker();
}

STEAMAPI_API uint32 SteamAPI_ISteamTV_AddRegion( ISteamTV* self, const char * pchElementName, const char * pchTimelineDataSection, const SteamTVRegion_t * pSteamTVRegion, ESteamTVRegionBehavior eSteamTVRegionBehavior )
{
    return self->AddRegion(pchElementName, pchTimelineDataSection, pSteamTVRegion, eSteamTVRegionBehavior);
}

STEAMAPI_API void SteamAPI_ISteamTV_RemoveRegion( ISteamTV* self, uint32 unRegionHandle )
{
    return self->RemoveRegion(unRegionHandle);
}

STEAMAPI_API ISteamParentalSettings *SteamAPI_SteamParentalSettings_v001()
{
    return get_steam_client()->GetISteamParentalSettings(flat_hsteamuser(), flat_hsteampipe(), "STEAMPARENTALSETTINGS_INTERFACE_VERSION001");
}

STEAMAPI_API steam_bool SteamAPI_ISteamParentalSettings_BIsParentalLockEnabled( ISteamParentalSettings* self )
{
    return self->BIsParentalLockEnabled();
}

STEAMAPI_API steam_bool SteamAPI_ISteamParentalSettings_BIsParentalLockLocked( ISteamParentalSettings* self )
{
    return self->BIsParentalLockLocked();
}

STEAMAPI_API steam_bool SteamAPI_ISteamParentalSettings_BIsAppBlocked( ISteamParentalSettings* self, AppId_t nAppID )
{
    return self->BIsAppBlocked(nAppID);
}

STEAMAPI_API steam_bool SteamAPI_ISteamParentalSettings_BIsAppInBlockList( ISteamParentalSettings* self, AppId_t nAppID )
{
    return self->BIsAppInBlockList(nAppID);
}

STEAMAPI_API steam_bool SteamAPI_ISteamParentalSettings_BIsFeatureBlocked( ISteamParentalSettings* self, EParentalFeature eFeature )
{
    return self->BIsFeatureBlocked(eFeature);
}

STEAMAPI_API steam_bool SteamAPI_ISteamParentalSettings_BIsFeatureInBlockList( ISteamParentalSettings* self, EParentalFeature eFeature )
{
    return self->BIsFeatureInBlockList(eFeature);
}

STEAMAPI_API ISteamRemotePlay *SteamAPI_SteamRemotePlay_v001()
{
    return get_steam_client()->GetISteamRemotePlay(flat_hsteamuser(), flat_hsteampipe(), "STEAMREMOTEPLAY_INTERFACE_VERSION001");
}

STEAMAPI_API ISteamRemotePlay *SteamAPI_SteamRemotePlay_v002()
{
    return get_steam_client()->GetISteamRemotePlay(flat_hsteamuser(), flat_hsteampipe(), "STEAMREMOTEPLAY_INTERFACE_VERSION002");
}

STEAMAPI_API uint32 SteamAPI_ISteamRemotePlay_GetSessionCount( ISteamRemotePlay* self )
{
    return self->GetSessionCount();
}

STEAMAPI_API RemotePlaySessionID_t SteamAPI_ISteamRemotePlay_GetSessionID( ISteamRemotePlay* self, int iSessionIndex )
{
    return self->GetSessionID(iSessionIndex);
}

STEAMAPI_API uint64_steamid SteamAPI_ISteamRemotePlay_GetSessionSteamID( ISteamRemotePlay* self, RemotePlaySessionID_t unSessionID )
{
    return self->GetSessionSteamID(unSessionID).ConvertToUint64();
}

STEAMAPI_API const char * SteamAPI_ISteamRemotePlay_GetSessionClientName( ISteamRemotePlay* self, RemotePlaySessionID_t unSessionID )
{
    return self->GetSessionClientName(unSessionID);
}

STEAMAPI_API ESteamDeviceFormFactor SteamAPI_ISteamRemotePlay_GetSessionClientFormFactor( ISteamRemotePlay* self, RemotePlaySessionID_t unSessionID )
{
    return self->GetSessionClientFormFactor(unSessionID);
}

STEAMAPI_API steam_bool SteamAPI_ISteamRemotePlay_BGetSessionClientResolution( ISteamRemotePlay* self, RemotePlaySessionID_t unSessionID, int * pnResolutionX, int * pnResolutionY )
{
    return self->BGetSessionClientResolution(unSessionID, pnResolutionX, pnResolutionY);
}

STEAMAPI_API steam_bool SteamAPI_ISteamRemotePlay_BStartRemotePlayTogether( ISteamRemotePlay* self, bool bShowOverlay )
{
    return self->BStartRemotePlayTogether(bShowOverlay);
}

STEAMAPI_API steam_bool SteamAPI_ISteamRemotePlay_BSendRemotePlayTogetherInvite( ISteamRemotePlay* self, uint64_steamid steamIDFriend )
{
    return self->BSendRemotePlayTogetherInvite(steamIDFriend);
}

STEAMAPI_API ISteamNetworkingMessages *SteamAPI_SteamNetworkingMessages_v002()
{
    return (ISteamNetworkingMessages *)get_steam_client()->GetISteamGenericInterface(flat_hsteamuser(), flat_hsteampipe(), "SteamNetworkingMessages002");
}

STEAMAPI_API ISteamNetworkingMessages *SteamAPI_SteamNetworkingMessages_SteamAPI_v002()
{
    return (ISteamNetworkingMessages *)get_steam_client()->GetISteamGenericInterface(flat_hsteamuser(), flat_hsteampipe(), "SteamNetworkingMessages002");
}

STEAMAPI_API ISteamNetworkingMessages *SteamAPI_SteamGameServerNetworkingMessages_v002()
{
    return (ISteamNetworkingMessages *)get_steam_client()->GetISteamGenericInterface(flat_gs_hsteamuser(), flat_gs_hsteampipe(), "SteamNetworkingMessages002");
}

STEAMAPI_API ISteamNetworkingMessages *SteamAPI_SteamGameServerNetworkingMessages_SteamAPI_v002()
{
    return (ISteamNetworkingMessages *)get_steam_client()->GetISteamGenericInterface(flat_gs_hsteamuser(), flat_gs_hsteampipe(), "SteamNetworkingMessages002");
}

STEAMAPI_API EResult SteamAPI_ISteamNetworkingMessages_SendMessageToUser( ISteamNetworkingMessages* self, const SteamNetworkingIdentity & identityRemote, const void * pubData, uint32 cubData, int nSendFlags, int nRemoteChannel )
{
    return self->SendMessageToUser(identityRemote, pubData, cubData, nSendFlags, nRemoteChannel);
}

STEAMAPI_API int SteamAPI_ISteamNetworkingMessages_ReceiveMessagesOnChannel( ISteamNetworkingMessages* self, int nLocalChannel, SteamNetworkingMessage_t ** ppOutMessages, int nMaxMessages )
{
    return self->ReceiveMessagesOnChannel(nLocalChannel, ppOutMessages, nMaxMessages);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingMessages_AcceptSessionWithUser( ISteamNetworkingMessages* self, const SteamNetworkingIdentity & identityRemote )
{
    return self->AcceptSessionWithUser(identityRemote);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingMessages_CloseSessionWithUser( ISteamNetworkingMessages* self, const SteamNetworkingIdentity & identityRemote )
{
    return self->CloseSessionWithUser(identityRemote);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingMessages_CloseChannelWithUser( ISteamNetworkingMessages* self, const SteamNetworkingIdentity & identityRemote, int nLocalChannel )
{
    return self->CloseChannelWithUser(identityRemote, nLocalChannel);
}

STEAMAPI_API ESteamNetworkingConnectionState SteamAPI_ISteamNetworkingMessages_GetSessionConnectionInfo( ISteamNetworkingMessages* self, const SteamNetworkingIdentity & identityRemote, SteamNetConnectionInfo_t * pConnectionInfo, SteamNetworkingQuickConnectionStatus * pQuickStatus )
{
    return self->GetSessionConnectionInfo(identityRemote, pConnectionInfo, pQuickStatus);
}

STEAMAPI_API ISteamNetworkingSockets *SteamAPI_SteamNetworkingSockets_SteamAPI_v012()
{
    return (ISteamNetworkingSockets *)get_steam_client()->GetISteamGenericInterface(flat_hsteamuser(), flat_hsteampipe(), "SteamNetworkingSockets012");
}

STEAMAPI_API ISteamNetworkingSockets *SteamAPI_SteamGameServerNetworkingSockets_SteamAPI_v012()
{
    return (ISteamNetworkingSockets *)get_steam_client()->GetISteamGenericInterface(flat_gs_hsteamuser(), flat_gs_hsteampipe(), "SteamNetworkingSockets012");
}

STEAMAPI_API ISteamNetworkingSockets *SteamAPI_SteamNetworkingSockets_SteamAPI_v011()
{
    return (ISteamNetworkingSockets *)get_steam_client()->GetISteamGenericInterface(flat_hsteamuser(), flat_hsteampipe(), "SteamNetworkingSockets011");
}

STEAMAPI_API ISteamNetworkingSockets *SteamAPI_SteamGameServerNetworkingSockets_SteamAPI_v011()
{
    return (ISteamNetworkingSockets *)get_steam_client()->GetISteamGenericInterface(flat_gs_hsteamuser(), flat_gs_hsteampipe(), "SteamNetworkingSockets011");
}

STEAMAPI_API ISteamNetworkingSockets *SteamAPI_SteamNetworkingSockets_SteamAPI_v009()
{
    return (ISteamNetworkingSockets *)get_steam_client()->GetISteamGenericInterface(flat_hsteamuser(), flat_hsteampipe(), "SteamNetworkingSockets009");
}

STEAMAPI_API ISteamNetworkingSockets *SteamAPI_SteamGameServerNetworkingSockets_SteamAPI_v009()
{
    return (ISteamNetworkingSockets *)get_steam_client()->GetISteamGenericInterface(flat_gs_hsteamuser(), flat_gs_hsteampipe(), "SteamNetworkingSockets009");
}

STEAMAPI_API ISteamNetworkingSockets *SteamAPI_SteamNetworkingSockets_v009()
{
    return (ISteamNetworkingSockets *)get_steam_client()->GetISteamGenericInterface(flat_hsteamuser(), flat_hsteampipe(), "SteamNetworkingSockets009");
}

STEAMAPI_API ISteamNetworkingSockets *SteamAPI_SteamGameServerNetworkingSockets_v009()
{
    return (ISteamNetworkingSockets *)get_steam_client()->GetISteamGenericInterface(flat_gs_hsteamuser(), flat_gs_hsteampipe(), "SteamNetworkingSockets009");
}

STEAMAPI_API ISteamNetworkingSockets *SteamAPI_SteamNetworkingSockets_v008()
{
    return (ISteamNetworkingSockets *)get_steam_client()->GetISteamGenericInterface(flat_hsteamuser(), flat_hsteampipe(), "SteamNetworkingSockets008");
}

STEAMAPI_API ISteamNetworkingSockets *SteamAPI_SteamGameServerNetworkingSockets_v008()
{
    return (ISteamNetworkingSockets *)get_steam_client()->GetISteamGenericInterface(flat_gs_hsteamuser(), flat_gs_hsteampipe(), "SteamNetworkingSockets008");
}

STEAMAPI_API HSteamListenSocket SteamAPI_ISteamNetworkingSockets_CreateListenSocketIP( ISteamNetworkingSockets* self, const SteamNetworkingIPAddr & localAddress, int nOptions, const SteamNetworkingConfigValue_t * pOptions )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->CreateListenSocketIP(localAddress, nOptions, pOptions);
}

STEAMAPI_API HSteamNetConnection SteamAPI_ISteamNetworkingSockets_ConnectByIPAddress( ISteamNetworkingSockets* self, const SteamNetworkingIPAddr & address, int nOptions, const SteamNetworkingConfigValue_t * pOptions )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->ConnectByIPAddress(address, nOptions, pOptions);
}

STEAMAPI_API HSteamListenSocket SteamAPI_ISteamNetworkingSockets_CreateListenSocketP2P( ISteamNetworkingSockets* self, int nLocalVirtualPort, int nOptions, const SteamNetworkingConfigValue_t * pOptions )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->CreateListenSocketP2P(nLocalVirtualPort, nOptions, pOptions);
}

STEAMAPI_API HSteamNetConnection SteamAPI_ISteamNetworkingSockets_ConnectP2P( ISteamNetworkingSockets* self, const SteamNetworkingIdentity & identityRemote, int nRemoteVirtualPort, int nOptions, const SteamNetworkingConfigValue_t * pOptions )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->ConnectP2P(identityRemote, nRemoteVirtualPort, nOptions, pOptions);
}

STEAMAPI_API EResult SteamAPI_ISteamNetworkingSockets_AcceptConnection( ISteamNetworkingSockets* self, HSteamNetConnection hConn )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->AcceptConnection(hConn);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingSockets_CloseConnection( ISteamNetworkingSockets* self, HSteamNetConnection hPeer, int nReason, const char * pszDebug, bool bEnableLinger )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->CloseConnection(hPeer, nReason, pszDebug, bEnableLinger);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingSockets_CloseListenSocket( ISteamNetworkingSockets* self, HSteamListenSocket hSocket )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->CloseListenSocket(hSocket);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingSockets_SetConnectionUserData( ISteamNetworkingSockets* self, HSteamNetConnection hPeer, int64 nUserData )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->SetConnectionUserData(hPeer, nUserData);
}

STEAMAPI_API int64 SteamAPI_ISteamNetworkingSockets_GetConnectionUserData( ISteamNetworkingSockets* self, HSteamNetConnection hPeer )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->GetConnectionUserData(hPeer);
}

STEAMAPI_API void SteamAPI_ISteamNetworkingSockets_SetConnectionName( ISteamNetworkingSockets* self, HSteamNetConnection hPeer, const char * pszName )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->SetConnectionName(hPeer, pszName);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingSockets_GetConnectionName( ISteamNetworkingSockets* self, HSteamNetConnection hPeer, char * pszName, int nMaxLen )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->GetConnectionName(hPeer, pszName, nMaxLen);
}

STEAMAPI_API EResult SteamAPI_ISteamNetworkingSockets_SendMessageToConnection( ISteamNetworkingSockets* self, HSteamNetConnection hConn, const void * pData, uint32 cbData, int nSendFlags, int64 * pOutMessageNumber )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->SendMessageToConnection(hConn, pData, cbData, nSendFlags, pOutMessageNumber);
}

STEAMAPI_API void SteamAPI_ISteamNetworkingSockets_SendMessages( ISteamNetworkingSockets* self, int nMessages, SteamNetworkingMessage_t *const * pMessages, int64 * pOutMessageNumberOrResult )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->SendMessages(nMessages, pMessages, pOutMessageNumberOrResult);
}

STEAMAPI_API EResult SteamAPI_ISteamNetworkingSockets_FlushMessagesOnConnection( ISteamNetworkingSockets* self, HSteamNetConnection hConn )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->FlushMessagesOnConnection(hConn);
}

STEAMAPI_API int SteamAPI_ISteamNetworkingSockets_ReceiveMessagesOnConnection( ISteamNetworkingSockets* self, HSteamNetConnection hConn, SteamNetworkingMessage_t ** ppOutMessages, int nMaxMessages )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->ReceiveMessagesOnConnection(hConn, ppOutMessages, nMaxMessages);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingSockets_GetConnectionInfo( ISteamNetworkingSockets* self, HSteamNetConnection hConn, SteamNetConnectionInfo_t * pInfo )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->GetConnectionInfo(hConn, pInfo);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingSockets_GetQuickConnectionStatus( ISteamNetworkingSockets009* self, HSteamNetConnection hConn, SteamNetworkingQuickConnectionStatus * pStats )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->GetQuickConnectionStatus(hConn, pStats);
}

STEAMAPI_API EResult SteamAPI_ISteamNetworkingSockets_GetConnectionRealTimeStatus( ISteamNetworkingSockets* self, HSteamNetConnection hConn, SteamNetConnectionRealTimeStatus_t * pStatus, int nLanes, SteamNetConnectionRealTimeLaneStatus_t * pLanes )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->GetConnectionRealTimeStatus(hConn, pStatus, nLanes, pLanes);
}

STEAMAPI_API int SteamAPI_ISteamNetworkingSockets_GetDetailedConnectionStatus( ISteamNetworkingSockets* self, HSteamNetConnection hConn, char * pszBuf, int cbBuf )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->GetDetailedConnectionStatus(hConn, pszBuf, cbBuf);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingSockets_GetListenSocketAddress( ISteamNetworkingSockets* self, HSteamListenSocket hSocket, SteamNetworkingIPAddr * address )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->GetListenSocketAddress(hSocket, address);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingSockets_CreateSocketPair( ISteamNetworkingSockets* self, HSteamNetConnection * pOutConnection1, HSteamNetConnection * pOutConnection2, bool bUseNetworkLoopback, const SteamNetworkingIdentity * pIdentity1, const SteamNetworkingIdentity * pIdentity2 )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->CreateSocketPair(pOutConnection1, pOutConnection2, bUseNetworkLoopback, pIdentity1, pIdentity2);
}

STEAMAPI_API EResult SteamAPI_ISteamNetworkingSockets_ConfigureConnectionLanes( ISteamNetworkingSockets* self, HSteamNetConnection hConn, int nNumLanes, const int * pLanePriorities, const uint16 * pLaneWeights )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->ConfigureConnectionLanes(hConn, nNumLanes, pLanePriorities, pLaneWeights);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingSockets_GetIdentity( ISteamNetworkingSockets* self, SteamNetworkingIdentity * pIdentity )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->GetIdentity(pIdentity);
}

STEAMAPI_API ESteamNetworkingAvailability SteamAPI_ISteamNetworkingSockets_InitAuthentication( ISteamNetworkingSockets* self )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->InitAuthentication();
}

STEAMAPI_API ESteamNetworkingAvailability SteamAPI_ISteamNetworkingSockets_GetAuthenticationStatus( ISteamNetworkingSockets* self, SteamNetAuthenticationStatus_t * pDetails )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->GetAuthenticationStatus(pDetails);
}

STEAMAPI_API HSteamNetPollGroup SteamAPI_ISteamNetworkingSockets_CreatePollGroup( ISteamNetworkingSockets* self )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->CreatePollGroup();
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingSockets_DestroyPollGroup( ISteamNetworkingSockets* self, HSteamNetPollGroup hPollGroup )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->DestroyPollGroup(hPollGroup);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingSockets_SetConnectionPollGroup( ISteamNetworkingSockets* self, HSteamNetConnection hConn, HSteamNetPollGroup hPollGroup )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->SetConnectionPollGroup(hConn, hPollGroup);
}

STEAMAPI_API int SteamAPI_ISteamNetworkingSockets_ReceiveMessagesOnPollGroup( ISteamNetworkingSockets* self, HSteamNetPollGroup hPollGroup, SteamNetworkingMessage_t ** ppOutMessages, int nMaxMessages )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->ReceiveMessagesOnPollGroup(hPollGroup, ppOutMessages, nMaxMessages);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingSockets_ReceivedRelayAuthTicket( ISteamNetworkingSockets* self, const void * pvTicket, int cbTicket, SteamDatagramRelayAuthTicket * pOutParsedTicket )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->ReceivedRelayAuthTicket(pvTicket, cbTicket, pOutParsedTicket);
}

STEAMAPI_API int SteamAPI_ISteamNetworkingSockets_FindRelayAuthTicketForServer( ISteamNetworkingSockets* self, const SteamNetworkingIdentity & identityGameServer, int nRemoteVirtualPort, SteamDatagramRelayAuthTicket * pOutParsedTicket )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->FindRelayAuthTicketForServer(identityGameServer, nRemoteVirtualPort, pOutParsedTicket);
}

STEAMAPI_API HSteamNetConnection SteamAPI_ISteamNetworkingSockets_ConnectToHostedDedicatedServer( ISteamNetworkingSockets* self, const SteamNetworkingIdentity & identityTarget, int nRemoteVirtualPort, int nOptions, const SteamNetworkingConfigValue_t * pOptions )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->ConnectToHostedDedicatedServer(identityTarget, nRemoteVirtualPort, nOptions, pOptions);
}

STEAMAPI_API uint16 SteamAPI_ISteamNetworkingSockets_GetHostedDedicatedServerPort( ISteamNetworkingSockets* self )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->GetHostedDedicatedServerPort();
}

STEAMAPI_API SteamNetworkingPOPID SteamAPI_ISteamNetworkingSockets_GetHostedDedicatedServerPOPID( ISteamNetworkingSockets* self )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->GetHostedDedicatedServerPOPID();
}

STEAMAPI_API EResult SteamAPI_ISteamNetworkingSockets_GetHostedDedicatedServerAddress( ISteamNetworkingSockets* self, SteamDatagramHostedAddress * pRouting )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->GetHostedDedicatedServerAddress(pRouting);
}

STEAMAPI_API HSteamListenSocket SteamAPI_ISteamNetworkingSockets_CreateHostedDedicatedServerListenSocket( ISteamNetworkingSockets* self, int nLocalVirtualPort, int nOptions, const SteamNetworkingConfigValue_t * pOptions )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->CreateHostedDedicatedServerListenSocket(nLocalVirtualPort, nOptions, pOptions);
}

STEAMAPI_API EResult SteamAPI_ISteamNetworkingSockets_GetGameCoordinatorServerLogin( ISteamNetworkingSockets* self, SteamDatagramGameCoordinatorServerLogin * pLoginInfo, int * pcbSignedBlob, void * pBlob )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->GetGameCoordinatorServerLogin(pLoginInfo, pcbSignedBlob, pBlob);
}

//TODO: implement old version if needed
//I don't think anything actually uses this function, especially not in flat api form so I'll just leave it like this.
//STEAMAPI_API HSteamNetConnection SteamAPI_ISteamNetworkingSockets_ConnectP2PCustomSignaling( ISteamNetworkingSockets* self, ISteamNetworkingConnectionCustomSignaling * pSignaling, const SteamNetworkingIdentity * pPeerIdentity, int nOptions, const SteamNetworkingConfigValue_t * pOptions );
//STEAMAPI_API HSteamNetConnection SteamAPI_ISteamNetworkingSockets_ConnectP2PCustomSignaling( ISteamNetworkingSockets* self, ISteamNetworkingConnectionCustomSignaling * pSignaling, const SteamNetworkingIdentity * pPeerIdentity, int nRemoteVirtualPort, int nOptions, const SteamNetworkingConfigValue_t * pOptions )
STEAMAPI_API HSteamNetConnection SteamAPI_ISteamNetworkingSockets_ConnectP2PCustomSignaling( ISteamNetworkingSockets* self, ISteamNetworkingConnectionSignaling * pSignaling, const SteamNetworkingIdentity * pPeerIdentity, int nRemoteVirtualPort, int nOptions, const SteamNetworkingConfigValue_t * pOptions )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->ConnectP2PCustomSignaling(pSignaling, pPeerIdentity, nRemoteVirtualPort, nOptions, pOptions);
}

//STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingSockets_ReceivedP2PCustomSignal( ISteamNetworkingSockets* self, const void * pMsg, int cbMsg, ISteamNetworkingCustomSignalingRecvContext * pContext )
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingSockets_ReceivedP2PCustomSignal( ISteamNetworkingSockets* self, const void * pMsg, int cbMsg, ISteamNetworkingSignalingRecvContext * pContext )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->ReceivedP2PCustomSignal(pMsg, cbMsg, pContext);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingSockets_GetCertificateRequest( ISteamNetworkingSockets* self, int * pcbBlob, void * pBlob, SteamNetworkingErrMsg & errMsg )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->GetCertificateRequest(pcbBlob, pBlob, errMsg);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingSockets_SetCertificate( ISteamNetworkingSockets* self, const void * pCertificate, int cbCertificate, SteamNetworkingErrMsg & errMsg )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->SetCertificate(pCertificate, cbCertificate, errMsg);
}

STEAMAPI_API void SteamAPI_ISteamNetworkingSockets_ResetIdentity( ISteamNetworkingSockets* self, const SteamNetworkingIdentity * pIdentity )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->ResetIdentity(pIdentity);
}

STEAMAPI_API void SteamAPI_ISteamNetworkingSockets_RunCallbacks( ISteamNetworkingSockets* self )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->RunCallbacks();
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingSockets_BeginAsyncRequestFakeIP( ISteamNetworkingSockets* self, int nNumPorts )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->BeginAsyncRequestFakeIP(nNumPorts);
}

STEAMAPI_API void SteamAPI_ISteamNetworkingSockets_GetFakeIP( ISteamNetworkingSockets* self, int idxFirstPort, SteamNetworkingFakeIPResult_t * pInfo )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->GetFakeIP(idxFirstPort, pInfo);
}

STEAMAPI_API HSteamListenSocket SteamAPI_ISteamNetworkingSockets_CreateListenSocketP2PFakeIP( ISteamNetworkingSockets* self, int idxFakePort, int nOptions, const SteamNetworkingConfigValue_t * pOptions )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->CreateListenSocketP2PFakeIP(idxFakePort, nOptions, pOptions);
}

STEAMAPI_API EResult SteamAPI_ISteamNetworkingSockets_GetRemoteFakeIPForConnection( ISteamNetworkingSockets* self, HSteamNetConnection hConn, SteamNetworkingIPAddr * pOutAddr )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->GetRemoteFakeIPForConnection(hConn, pOutAddr);
}

STEAMAPI_API ISteamNetworkingFakeUDPPort * SteamAPI_ISteamNetworkingSockets_CreateFakeUDPPort( ISteamNetworkingSockets* self, int idxFakeServerPort )
{
    long long client_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_networking_sockets);
    long long server_vftable_distance = ((char *)self - (char*)get_steam_client()->steam_gameserver_networking_sockets);
    auto ptr = get_steam_client()->steam_gameserver_networking_sockets;
    if (client_vftable_distance >= 0 && (server_vftable_distance < 0 || client_vftable_distance < server_vftable_distance)) {
        ptr = get_steam_client()->steam_networking_sockets;
    }

    return (ptr)->CreateFakeUDPPort(idxFakeServerPort);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingConnectionCustomSignaling_SendSignal( ISteamNetworkingConnectionCustomSignaling* self, HSteamNetConnection hConn, const SteamNetConnectionInfo_t & info, const void * pMsg, int cbMsg )
{
    return self->SendSignal(hConn, info, pMsg, cbMsg);
}

STEAMAPI_API void SteamAPI_ISteamNetworkingConnectionCustomSignaling_Release( ISteamNetworkingConnectionCustomSignaling* self )
{
    return self->Release();
}

STEAMAPI_API ISteamNetworkingConnectionCustomSignaling * SteamAPI_ISteamNetworkingCustomSignalingRecvContext_OnConnectRequest( ISteamNetworkingCustomSignalingRecvContext* self, HSteamNetConnection hConn, const SteamNetworkingIdentity & identityPeer )
{
    return self->OnConnectRequest(hConn, identityPeer);
}

STEAMAPI_API void SteamAPI_ISteamNetworkingCustomSignalingRecvContext_SendRejectionSignal( ISteamNetworkingCustomSignalingRecvContext* self, const SteamNetworkingIdentity & identityPeer, const void * pMsg, int cbMsg )
{
    return self->SendRejectionSignal(identityPeer, pMsg, cbMsg);
}

STEAMAPI_API ISteamNetworkingUtils *SteamAPI_SteamNetworkingUtils_SteamAPI_v003()
{
    return (ISteamNetworkingUtils *)get_steam_client()->GetISteamGenericInterface(flat_hsteamuser(), flat_hsteampipe(), "SteamNetworkingUtils003");
}

STEAMAPI_API ISteamNetworkingUtils *SteamAPI_SteamNetworkingUtils_SteamAPI_v004()
{
    return (ISteamNetworkingUtils *)get_steam_client()->GetISteamGenericInterface(flat_hsteamuser(), flat_hsteampipe(), "SteamNetworkingUtils004");
}

STEAMAPI_API ISteamNetworkingUtils *SteamAPI_SteamNetworkingUtils_v003()
{
    return (ISteamNetworkingUtils *)get_steam_client()->GetISteamGenericInterface(flat_hsteamuser(), flat_hsteampipe(), "SteamNetworkingUtils003");
}

STEAMAPI_API SteamNetworkingMessage_t * SteamAPI_ISteamNetworkingUtils_AllocateMessage( ISteamNetworkingUtils* self, int cbAllocateBuffer )
{
    return (get_steam_client()->steam_networking_utils)->AllocateMessage(cbAllocateBuffer);
}

STEAMAPI_API void SteamAPI_ISteamNetworkingUtils_InitRelayNetworkAccess( ISteamNetworkingUtils* self )
{
    return ((ISteamNetworkingUtils*)get_steam_client()->steam_networking_utils)->InitRelayNetworkAccess();
}

STEAMAPI_API ESteamNetworkingAvailability SteamAPI_ISteamNetworkingUtils_GetRelayNetworkStatus( ISteamNetworkingUtils* self, SteamRelayNetworkStatus_t * pDetails )
{
    return (get_steam_client()->steam_networking_utils)->GetRelayNetworkStatus(pDetails);
}

STEAMAPI_API float SteamAPI_ISteamNetworkingUtils_GetLocalPingLocation( ISteamNetworkingUtils* self, SteamNetworkPingLocation_t & result )
{
    return (get_steam_client()->steam_networking_utils)->GetLocalPingLocation(result);
}

STEAMAPI_API int SteamAPI_ISteamNetworkingUtils_EstimatePingTimeBetweenTwoLocations( ISteamNetworkingUtils* self, const SteamNetworkPingLocation_t & location1, const SteamNetworkPingLocation_t & location2 )
{
    return (get_steam_client()->steam_networking_utils)->EstimatePingTimeBetweenTwoLocations(location1, location2);
}

STEAMAPI_API int SteamAPI_ISteamNetworkingUtils_EstimatePingTimeFromLocalHost( ISteamNetworkingUtils* self, const SteamNetworkPingLocation_t & remoteLocation )
{
    return (get_steam_client()->steam_networking_utils)->EstimatePingTimeFromLocalHost(remoteLocation);
}

STEAMAPI_API void SteamAPI_ISteamNetworkingUtils_ConvertPingLocationToString( ISteamNetworkingUtils* self, const SteamNetworkPingLocation_t & location, char * pszBuf, int cchBufSize )
{
    return (get_steam_client()->steam_networking_utils)->ConvertPingLocationToString(location, pszBuf, cchBufSize);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_ParsePingLocationString( ISteamNetworkingUtils* self, const char * pszString, SteamNetworkPingLocation_t & result )
{
    return (get_steam_client()->steam_networking_utils)->ParsePingLocationString(pszString, result);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_CheckPingDataUpToDate( ISteamNetworkingUtils* self, float flMaxAgeSeconds )
{
    return (get_steam_client()->steam_networking_utils)->CheckPingDataUpToDate(flMaxAgeSeconds);
}

STEAMAPI_API int SteamAPI_ISteamNetworkingUtils_GetPingToDataCenter( ISteamNetworkingUtils* self, SteamNetworkingPOPID popID, SteamNetworkingPOPID * pViaRelayPoP )
{
    return (get_steam_client()->steam_networking_utils)->GetPingToDataCenter(popID, pViaRelayPoP);
}

STEAMAPI_API int SteamAPI_ISteamNetworkingUtils_GetDirectPingToPOP( ISteamNetworkingUtils* self, SteamNetworkingPOPID popID )
{
    return (get_steam_client()->steam_networking_utils)->GetDirectPingToPOP(popID);
}

STEAMAPI_API int SteamAPI_ISteamNetworkingUtils_GetPOPCount( ISteamNetworkingUtils* self )
{
    return (get_steam_client()->steam_networking_utils)->GetPOPCount();
}

STEAMAPI_API int SteamAPI_ISteamNetworkingUtils_GetPOPList( ISteamNetworkingUtils* self, SteamNetworkingPOPID * list, int nListSz )
{
    return (get_steam_client()->steam_networking_utils)->GetPOPList(list, nListSz);
}

STEAMAPI_API SteamNetworkingMicroseconds SteamAPI_ISteamNetworkingUtils_GetLocalTimestamp( ISteamNetworkingUtils* self )
{
    return (get_steam_client()->steam_networking_utils)->GetLocalTimestamp();
}

STEAMAPI_API void SteamAPI_ISteamNetworkingUtils_SetDebugOutputFunction( ISteamNetworkingUtils* self, ESteamNetworkingSocketsDebugOutputType eDetailLevel, FSteamNetworkingSocketsDebugOutput pfnFunc )
{
    return (get_steam_client()->steam_networking_utils)->SetDebugOutputFunction(eDetailLevel, pfnFunc);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_IsFakeIPv4( ISteamNetworkingUtils* self, uint32 nIPv4 )
{
    return (get_steam_client()->steam_networking_utils)->IsFakeIPv4(nIPv4);
}

STEAMAPI_API ESteamNetworkingFakeIPType SteamAPI_ISteamNetworkingUtils_GetIPv4FakeIPType( ISteamNetworkingUtils* self, uint32 nIPv4 )
{
    return (get_steam_client()->steam_networking_utils)->GetIPv4FakeIPType(nIPv4);
}

STEAMAPI_API EResult SteamAPI_ISteamNetworkingUtils_GetRealIdentityForFakeIP( ISteamNetworkingUtils* self, const SteamNetworkingIPAddr & fakeIP, SteamNetworkingIdentity * pOutRealIdentity )
{
    return (get_steam_client()->steam_networking_utils)->GetRealIdentityForFakeIP(fakeIP, pOutRealIdentity);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_SetGlobalConfigValueInt32( ISteamNetworkingUtils* self, ESteamNetworkingConfigValue eValue, int32 val )
{
    return ((ISteamNetworkingUtils*)get_steam_client()->steam_networking_utils)->SetGlobalConfigValueInt32(eValue, val);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_SetGlobalConfigValueFloat( ISteamNetworkingUtils* self, ESteamNetworkingConfigValue eValue, float val )
{
    return ((ISteamNetworkingUtils*)get_steam_client()->steam_networking_utils)->SetGlobalConfigValueFloat(eValue, val);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_SetGlobalConfigValueString( ISteamNetworkingUtils* self, ESteamNetworkingConfigValue eValue, const char * val )
{
    return ((ISteamNetworkingUtils*)get_steam_client()->steam_networking_utils)->SetGlobalConfigValueString(eValue, val);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_SetGlobalConfigValuePtr( ISteamNetworkingUtils* self, ESteamNetworkingConfigValue eValue, void * val )
{
    return ((ISteamNetworkingUtils*)get_steam_client()->steam_networking_utils)->SetGlobalConfigValuePtr(eValue, val);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_SetConnectionConfigValueInt32( ISteamNetworkingUtils* self, HSteamNetConnection hConn, ESteamNetworkingConfigValue eValue, int32 val )
{
    return ((ISteamNetworkingUtils*)get_steam_client()->steam_networking_utils)->SetConnectionConfigValueInt32(hConn, eValue, val);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_SetConnectionConfigValueFloat( ISteamNetworkingUtils* self, HSteamNetConnection hConn, ESteamNetworkingConfigValue eValue, float val )
{
    return ((ISteamNetworkingUtils*)get_steam_client()->steam_networking_utils)->SetConnectionConfigValueFloat(hConn, eValue, val);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_SetConnectionConfigValueString( ISteamNetworkingUtils* self, HSteamNetConnection hConn, ESteamNetworkingConfigValue eValue, const char * val )
{
    return ((ISteamNetworkingUtils*)get_steam_client()->steam_networking_utils)->SetConnectionConfigValueString(hConn, eValue, val);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_SetGlobalCallback_SteamNetConnectionStatusChanged( ISteamNetworkingUtils* self, FnSteamNetConnectionStatusChanged fnCallback )
{
    return ((ISteamNetworkingUtils*)get_steam_client()->steam_networking_utils)->SetGlobalCallback_SteamNetConnectionStatusChanged(fnCallback);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_SetGlobalCallback_SteamNetAuthenticationStatusChanged( ISteamNetworkingUtils* self, FnSteamNetAuthenticationStatusChanged fnCallback )
{
    return ((ISteamNetworkingUtils*)get_steam_client()->steam_networking_utils)->SetGlobalCallback_SteamNetAuthenticationStatusChanged(fnCallback);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_SetGlobalCallback_SteamRelayNetworkStatusChanged( ISteamNetworkingUtils* self, FnSteamRelayNetworkStatusChanged fnCallback )
{
    return ((ISteamNetworkingUtils*)get_steam_client()->steam_networking_utils)->SetGlobalCallback_SteamRelayNetworkStatusChanged(fnCallback);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_SetGlobalCallback_FakeIPResult( ISteamNetworkingUtils* self, FnSteamNetworkingFakeIPResult fnCallback )
{
    return ((ISteamNetworkingUtils*)get_steam_client()->steam_networking_utils)->SetGlobalCallback_FakeIPResult(fnCallback);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_SetGlobalCallback_MessagesSessionRequest( ISteamNetworkingUtils* self, FnSteamNetworkingMessagesSessionRequest fnCallback )
{
    return ((ISteamNetworkingUtils*)get_steam_client()->steam_networking_utils)->SetGlobalCallback_MessagesSessionRequest(fnCallback);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_SetGlobalCallback_MessagesSessionFailed( ISteamNetworkingUtils* self, FnSteamNetworkingMessagesSessionFailed fnCallback )
{
    return ((ISteamNetworkingUtils*)get_steam_client()->steam_networking_utils)->SetGlobalCallback_MessagesSessionFailed(fnCallback);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_SetConfigValue( ISteamNetworkingUtils* self, ESteamNetworkingConfigValue eValue, ESteamNetworkingConfigScope eScopeType, intptr_t scopeObj, ESteamNetworkingConfigDataType eDataType, const void * pArg )
{
    return (get_steam_client()->steam_networking_utils)->SetConfigValue(eValue, eScopeType, scopeObj, eDataType, pArg);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_SetConfigValueStruct( ISteamNetworkingUtils* self, const SteamNetworkingConfigValue_t & opt, ESteamNetworkingConfigScope eScopeType, intptr_t scopeObj )
{
    return ((ISteamNetworkingUtils*)get_steam_client()->steam_networking_utils)->SetConfigValueStruct(opt, eScopeType, scopeObj);
}

STEAMAPI_API ESteamNetworkingGetConfigValueResult SteamAPI_ISteamNetworkingUtils_GetConfigValue( ISteamNetworkingUtils* self, ESteamNetworkingConfigValue eValue, ESteamNetworkingConfigScope eScopeType, intptr_t scopeObj, ESteamNetworkingConfigDataType * pOutDataType, void * pResult, size_t * cbResult )
{
    return (get_steam_client()->steam_networking_utils)->GetConfigValue(eValue, eScopeType, scopeObj, pOutDataType, pResult, cbResult);
}

//TODO: implement this once the GetConfigValueInfo does something other than return 0 or NULL
//STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_GetConfigValueInfo( ISteamNetworkingUtils* self, ESteamNetworkingConfigValue eValue, const char ** pOutName, ESteamNetworkingConfigDataType * pOutDataType, ESteamNetworkingConfigScope * pOutScope, ESteamNetworkingConfigValue * pOutNextValue )
STEAMAPI_API const char * SteamAPI_ISteamNetworkingUtils_GetConfigValueInfo( ISteamNetworkingUtils* self, ESteamNetworkingConfigValue eValue, ESteamNetworkingConfigDataType * pOutDataType, ESteamNetworkingConfigScope * pOutScope )
{
    //return (get_steam_client()->steam_networking_utils)->GetConfigValueInfo(eValue, pOutDataType, pOutScope);
    return NULL;
}

STEAMAPI_API ESteamNetworkingConfigValue SteamAPI_ISteamNetworkingUtils_GetFirstConfigValue( ISteamNetworkingUtils* self )
{
    return (get_steam_client()->steam_networking_utils)->GetFirstConfigValue();
}

STEAMAPI_API ESteamNetworkingConfigValue SteamAPI_ISteamNetworkingUtils_IterateGenericEditableConfigValues( ISteamNetworkingUtils* self, ESteamNetworkingConfigValue eCurrent, bool bEnumerateDevVars )
{
    return (get_steam_client()->steam_networking_utils)->IterateGenericEditableConfigValues(eCurrent, bEnumerateDevVars);
}

STEAMAPI_API void SteamAPI_ISteamNetworkingUtils_SteamNetworkingIPAddr_ToString( ISteamNetworkingUtils* self, const SteamNetworkingIPAddr & addr, char * buf, uint32 cbBuf, bool bWithPort )
{
    return (get_steam_client()->steam_networking_utils)->SteamNetworkingIPAddr_ToString(addr, buf, cbBuf, bWithPort);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_SteamNetworkingIPAddr_ParseString( ISteamNetworkingUtils* self, SteamNetworkingIPAddr * pAddr, const char * pszStr )
{
    return (get_steam_client()->steam_networking_utils)->SteamNetworkingIPAddr_ParseString(pAddr, pszStr);
}

STEAMAPI_API ESteamNetworkingFakeIPType SteamAPI_ISteamNetworkingUtils_SteamNetworkingIPAddr_GetFakeIPType( ISteamNetworkingUtils* self, const SteamNetworkingIPAddr & addr )
{
    return (get_steam_client()->steam_networking_utils)->SteamNetworkingIPAddr_GetFakeIPType(addr);
}

STEAMAPI_API void SteamAPI_ISteamNetworkingUtils_SteamNetworkingIdentity_ToString( ISteamNetworkingUtils* self, const SteamNetworkingIdentity & identity, char * buf, uint32 cbBuf )
{
    return (get_steam_client()->steam_networking_utils)->SteamNetworkingIdentity_ToString(identity, buf, cbBuf);
}

STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_SteamNetworkingIdentity_ParseString( ISteamNetworkingUtils* self, SteamNetworkingIdentity * pIdentity, const char * pszStr )
{
    return (get_steam_client()->steam_networking_utils)->SteamNetworkingIdentity_ParseString(pIdentity, pszStr);
}

STEAMAPI_API ISteamGameServer *SteamAPI_SteamGameServer_v013()
{
    return get_steam_client()->GetISteamGameServer(flat_gs_hsteamuser(), flat_gs_hsteampipe(), "SteamGameServer013");
}

STEAMAPI_API ISteamGameServer *SteamAPI_SteamGameServer_v014()
{
    return get_steam_client()->GetISteamGameServer(flat_gs_hsteamuser(), flat_gs_hsteampipe(), "SteamGameServer014");
}

STEAMAPI_API ISteamGameServer *SteamAPI_SteamGameServer_v015()
{
    return get_steam_client()->GetISteamGameServer(flat_gs_hsteamuser(), flat_gs_hsteampipe(), "SteamGameServer015");
}

STEAMAPI_API steam_bool SteamAPI_ISteamGameServer_InitGameServer(intptr_t instancePtr, uint32 unIP, uint16 usGamePort, uint16 usQueryPort, uint32 unFlags, AppId_t nGameAppId, const char * pchVersionString)
{
    return ((ISteamGameServer *)instancePtr)->InitGameServer(unIP, usGamePort, usQueryPort, unFlags, nGameAppId, pchVersionString);
}

STEAMAPI_API void SteamAPI_ISteamGameServer_SetProduct( ISteamGameServer* self, const char * pszProduct )
{
    return get_steam_client()->steam_gameserver->SetProduct(pszProduct);
}

STEAMAPI_API void SteamAPI_ISteamGameServer_SetGameDescription( ISteamGameServer* self, const char * pszGameDescription )
{
    return get_steam_client()->steam_gameserver->SetGameDescription(pszGameDescription);
}

STEAMAPI_API void SteamAPI_ISteamGameServer_SetModDir( ISteamGameServer* self, const char * pszModDir )
{
    return get_steam_client()->steam_gameserver->SetModDir(pszModDir);
}

STEAMAPI_API void SteamAPI_ISteamGameServer_SetDedicatedServer( ISteamGameServer* self, bool bDedicated )
{
    return get_steam_client()->steam_gameserver->SetDedicatedServer(bDedicated);
}

STEAMAPI_API void SteamAPI_ISteamGameServer_LogOn( ISteamGameServer* self, const char * pszToken )
{
    return get_steam_client()->steam_gameserver->LogOn(pszToken);
}

STEAMAPI_API void SteamAPI_ISteamGameServer_LogOnAnonymous( ISteamGameServer* self )
{
    return get_steam_client()->steam_gameserver->LogOnAnonymous();
}

STEAMAPI_API void SteamAPI_ISteamGameServer_LogOff( ISteamGameServer* self )
{
    return get_steam_client()->steam_gameserver->LogOff();
}

STEAMAPI_API steam_bool SteamAPI_ISteamGameServer_BLoggedOn( ISteamGameServer* self )
{
    return get_steam_client()->steam_gameserver->BLoggedOn();
}

STEAMAPI_API steam_bool SteamAPI_ISteamGameServer_BSecure( ISteamGameServer* self )
{
    return get_steam_client()->steam_gameserver->BSecure();
}

STEAMAPI_API uint64_steamid SteamAPI_ISteamGameServer_GetSteamID( ISteamGameServer* self )
{
    return get_steam_client()->steam_gameserver->GetSteamID().ConvertToUint64();
}

STEAMAPI_API steam_bool SteamAPI_ISteamGameServer_WasRestartRequested( ISteamGameServer* self )
{
    return get_steam_client()->steam_gameserver->WasRestartRequested();
}

STEAMAPI_API void SteamAPI_ISteamGameServer_SetMaxPlayerCount( ISteamGameServer* self, int cPlayersMax )
{
    return get_steam_client()->steam_gameserver->SetMaxPlayerCount(cPlayersMax);
}

STEAMAPI_API void SteamAPI_ISteamGameServer_SetBotPlayerCount( ISteamGameServer* self, int cBotplayers )
{
    return get_steam_client()->steam_gameserver->SetBotPlayerCount(cBotplayers);
}

STEAMAPI_API void SteamAPI_ISteamGameServer_SetServerName( ISteamGameServer* self, const char * pszServerName )
{
    return get_steam_client()->steam_gameserver->SetServerName(pszServerName);
}

STEAMAPI_API void SteamAPI_ISteamGameServer_SetMapName( ISteamGameServer* self, const char * pszMapName )
{
    return get_steam_client()->steam_gameserver->SetMapName(pszMapName);
}

STEAMAPI_API void SteamAPI_ISteamGameServer_SetPasswordProtected( ISteamGameServer* self, bool bPasswordProtected )
{
    return get_steam_client()->steam_gameserver->SetPasswordProtected(bPasswordProtected);
}

STEAMAPI_API void SteamAPI_ISteamGameServer_SetSpectatorPort( ISteamGameServer* self, uint16 unSpectatorPort )
{
    return get_steam_client()->steam_gameserver->SetSpectatorPort(unSpectatorPort);
}

STEAMAPI_API void SteamAPI_ISteamGameServer_SetSpectatorServerName( ISteamGameServer* self, const char * pszSpectatorServerName )
{
    return get_steam_client()->steam_gameserver->SetSpectatorServerName(pszSpectatorServerName);
}

STEAMAPI_API void SteamAPI_ISteamGameServer_ClearAllKeyValues( ISteamGameServer* self )
{
    return get_steam_client()->steam_gameserver->ClearAllKeyValues();
}

STEAMAPI_API void SteamAPI_ISteamGameServer_SetKeyValue( ISteamGameServer* self, const char * pKey, const char * pValue )
{
    return get_steam_client()->steam_gameserver->SetKeyValue(pKey, pValue);
}

STEAMAPI_API void SteamAPI_ISteamGameServer_SetGameTags( ISteamGameServer* self, const char * pchGameTags )
{
    return get_steam_client()->steam_gameserver->SetGameTags(pchGameTags);
}

STEAMAPI_API void SteamAPI_ISteamGameServer_SetGameData( ISteamGameServer* self, const char * pchGameData )
{
    return get_steam_client()->steam_gameserver->SetGameData(pchGameData);
}

STEAMAPI_API void SteamAPI_ISteamGameServer_SetRegion( ISteamGameServer* self, const char * pszRegion )
{
    return get_steam_client()->steam_gameserver->SetRegion(pszRegion);
}

STEAMAPI_API steam_bool SteamAPI_ISteamGameServer_SendUserConnectAndAuthenticate( ISteamGameServer* self, uint32 unIPClient, const void * pvAuthBlob, uint32 cubAuthBlobSize, CSteamID * pSteamIDUser )
{
    return get_steam_client()->steam_gameserver->SendUserConnectAndAuthenticate(unIPClient, pvAuthBlob, cubAuthBlobSize, pSteamIDUser);
}

STEAMAPI_API steam_bool SteamAPI_ISteamGameServer_SendUserConnectAndAuthenticate_DEPRECATED( ISteamGameServer* self, uint32 unIPClient, const void * pvAuthBlob, uint32 cubAuthBlobSize, CSteamID * pSteamIDUser )
{
    return get_steam_client()->steam_gameserver->SendUserConnectAndAuthenticate(unIPClient, pvAuthBlob, cubAuthBlobSize, pSteamIDUser);
}

STEAMAPI_API uint64_steamid SteamAPI_ISteamGameServer_CreateUnauthenticatedUserConnection( ISteamGameServer* self )
{
    return get_steam_client()->steam_gameserver->CreateUnauthenticatedUserConnection().ConvertToUint64();
}

STEAMAPI_API void SteamAPI_ISteamGameServer_SendUserDisconnect( ISteamGameServer* self, uint64_steamid steamIDUser )
{
    return get_steam_client()->steam_gameserver->SendUserDisconnect(steamIDUser);
}

STEAMAPI_API void SteamAPI_ISteamGameServer_SendUserDisconnect_DEPRECATED( ISteamGameServer* self, uint64_steamid steamIDUser )
{
    return get_steam_client()->steam_gameserver->SendUserDisconnect(steamIDUser);
}

STEAMAPI_API steam_bool SteamAPI_ISteamGameServer_BUpdateUserData( ISteamGameServer* self, uint64_steamid steamIDUser, const char * pchPlayerName, uint32 uScore )
{
    return get_steam_client()->steam_gameserver->BUpdateUserData(steamIDUser, pchPlayerName, uScore);
}

STEAMAPI_API void SteamAPI_ISteamGameServer_SetAdvertiseServerActive( ISteamGameServer* self, bool bActive )
{
    return get_steam_client()->steam_gameserver->SetAdvertiseServerActive(bActive);
}

STEAMAPI_API HAuthTicket SteamAPI_ISteamGameServer_GetAuthSessionTicket( ISteamGameServer* self, void * pTicket, int cbMaxTicket, uint32 * pcbTicket, const SteamNetworkingIdentity * pSnid )
{
    return get_steam_client()->steam_gameserver->GetAuthSessionTicket(pTicket, cbMaxTicket, pcbTicket);
}

STEAMAPI_API EBeginAuthSessionResult SteamAPI_ISteamGameServer_BeginAuthSession( ISteamGameServer* self, const void * pAuthTicket, int cbAuthTicket, uint64_steamid steamID )
{
    return get_steam_client()->steam_gameserver->BeginAuthSession(pAuthTicket, cbAuthTicket, steamID);
}

STEAMAPI_API void SteamAPI_ISteamGameServer_EndAuthSession( ISteamGameServer* self, uint64_steamid steamID )
{
    return get_steam_client()->steam_gameserver->EndAuthSession(steamID);
}

STEAMAPI_API void SteamAPI_ISteamGameServer_CancelAuthTicket( ISteamGameServer* self, HAuthTicket hAuthTicket )
{
    return get_steam_client()->steam_gameserver->CancelAuthTicket(hAuthTicket);
}

STEAMAPI_API EUserHasLicenseForAppResult SteamAPI_ISteamGameServer_UserHasLicenseForApp( ISteamGameServer* self, uint64_steamid steamID, AppId_t appID )
{
    return get_steam_client()->steam_gameserver->UserHasLicenseForApp(steamID, appID);
}

STEAMAPI_API steam_bool SteamAPI_ISteamGameServer_RequestUserGroupStatus( ISteamGameServer* self, uint64_steamid steamIDUser, uint64_steamid steamIDGroup )
{
    return get_steam_client()->steam_gameserver->RequestUserGroupStatus(steamIDUser, steamIDGroup);
}

STEAMAPI_API void SteamAPI_ISteamGameServer_GetGameplayStats( ISteamGameServer* self )
{
    return get_steam_client()->steam_gameserver->GetGameplayStats();
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamGameServer_GetServerReputation( ISteamGameServer* self )
{
    return get_steam_client()->steam_gameserver->GetServerReputation();
}

STEAMAPI_API void *SteamAPI_ISteamGameServer_GetPublicIP( intptr_t instancePtr, void *instancePtr_possible )
{
    //abuse call convention rules to get this working.
    if (steamclient_has_ipv6_functions()) {
        get_steam_client()->steam_gameserver->GetPublicIP_fix((SteamIPAddress_t *)instancePtr);
        return (void *)instancePtr;
    } else {
        // (char*)0 will silence the compiler warning
        return (void *)( (char*)0 + ((ISteamGameServer012 *)instancePtr)->GetPublicIP_old() );
    }
}

STEAMAPI_API steam_bool SteamAPI_ISteamGameServer_HandleIncomingPacket( ISteamGameServer* self, const void * pData, int cbData, uint32 srcIP, uint16 srcPort )
{
    return get_steam_client()->steam_gameserver->HandleIncomingPacket(pData, cbData, srcIP, srcPort);
}

STEAMAPI_API int SteamAPI_ISteamGameServer_GetNextOutgoingPacket( ISteamGameServer* self, void * pOut, int cbMaxOut, uint32 * pNetAdr, uint16 * pPort )
{
    return get_steam_client()->steam_gameserver->GetNextOutgoingPacket(pOut, cbMaxOut, pNetAdr, pPort);
}

STEAMAPI_API void SteamAPI_ISteamGameServer_EnableHeartbeats( ISteamGameServer* self, bool bActive )
{
    return get_steam_client()->steam_gameserver->EnableHeartbeats(bActive);
}

STEAMAPI_API void SteamAPI_ISteamGameServer_SetHeartbeatInterval( ISteamGameServer* self, int iHeartbeatInterval )
{
    return get_steam_client()->steam_gameserver->SetHeartbeatInterval(iHeartbeatInterval);
}

STEAMAPI_API void SteamAPI_ISteamGameServer_ForceHeartbeat( ISteamGameServer* self )
{
    return get_steam_client()->steam_gameserver->ForceHeartbeat();
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamGameServer_AssociateWithClan( ISteamGameServer* self, uint64_steamid steamIDClan )
{
    return get_steam_client()->steam_gameserver->AssociateWithClan(steamIDClan);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamGameServer_ComputeNewPlayerCompatibility( ISteamGameServer* self, uint64_steamid steamIDNewPlayer )
{
    return get_steam_client()->steam_gameserver->ComputeNewPlayerCompatibility(steamIDNewPlayer);
}

STEAMAPI_API ISteamGameServerStats *SteamAPI_SteamGameServerStats_v001()
{
    return get_steam_client()->GetISteamGameServerStats(flat_gs_hsteamuser(), flat_gs_hsteampipe(), "SteamGameServerStats001");
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamGameServerStats_RequestUserStats( ISteamGameServerStats* self, uint64_steamid steamIDUser )
{
    return self->RequestUserStats(steamIDUser);
}

STEAMAPI_API steam_bool SteamAPI_ISteamGameServerStats_GetUserStat(intptr_t instancePtr, class CSteamID steamIDUser, const char * pchName, int32 * pData)
{
    return ((ISteamGameServerStats *)instancePtr)->GetUserStat(steamIDUser, pchName, pData);
}

STEAMAPI_API steam_bool SteamAPI_ISteamGameServerStats_GetUserStat0(intptr_t instancePtr, class CSteamID steamIDUser, const char * pchName, float * pData)
{
    return ((ISteamGameServerStats *)instancePtr)->GetUserStat(steamIDUser, pchName, pData);
}

STEAMAPI_API steam_bool SteamAPI_ISteamGameServerStats_GetUserStatInt32( ISteamGameServerStats* self, uint64_steamid steamIDUser, const char * pchName, int32 * pData )
{
    return self->GetUserStat(steamIDUser, pchName, pData);
}

STEAMAPI_API steam_bool SteamAPI_ISteamGameServerStats_GetUserStatFloat( ISteamGameServerStats* self, uint64_steamid steamIDUser, const char * pchName, float * pData )
{
    return self->GetUserStat(steamIDUser, pchName, pData);
}

STEAMAPI_API steam_bool SteamAPI_ISteamGameServerStats_GetUserAchievement( ISteamGameServerStats* self, uint64_steamid steamIDUser, const char * pchName, bool * pbAchieved )
{
    return self->GetUserAchievement(steamIDUser, pchName, pbAchieved);
}

STEAMAPI_API steam_bool SteamAPI_ISteamGameServerStats_SetUserStat(intptr_t instancePtr, class CSteamID steamIDUser, const char * pchName, int32 nData)
{
    return ((ISteamGameServerStats *)instancePtr)->SetUserStat(steamIDUser, pchName, nData);
}

STEAMAPI_API steam_bool SteamAPI_ISteamGameServerStats_SetUserStat0(intptr_t instancePtr, class CSteamID steamIDUser, const char * pchName, float fData)
{
    return ((ISteamGameServerStats *)instancePtr)->SetUserStat(steamIDUser, pchName, fData);
}

STEAMAPI_API steam_bool SteamAPI_ISteamGameServerStats_SetUserStatInt32( ISteamGameServerStats* self, uint64_steamid steamIDUser, const char * pchName, int32 nData )
{
    return self->SetUserStat(steamIDUser, pchName, nData);
}

STEAMAPI_API steam_bool SteamAPI_ISteamGameServerStats_SetUserStatFloat( ISteamGameServerStats* self, uint64_steamid steamIDUser, const char * pchName, float fData )
{
    return self->SetUserStat(steamIDUser, pchName, fData);
}

STEAMAPI_API steam_bool SteamAPI_ISteamGameServerStats_UpdateUserAvgRateStat( ISteamGameServerStats* self, uint64_steamid steamIDUser, const char * pchName, float flCountThisSession, double dSessionLength )
{
    return self->UpdateUserAvgRateStat(steamIDUser, pchName, flCountThisSession, dSessionLength);
}

STEAMAPI_API steam_bool SteamAPI_ISteamGameServerStats_SetUserAchievement( ISteamGameServerStats* self, uint64_steamid steamIDUser, const char * pchName )
{
    return self->SetUserAchievement(steamIDUser, pchName);
}

STEAMAPI_API steam_bool SteamAPI_ISteamGameServerStats_ClearUserAchievement( ISteamGameServerStats* self, uint64_steamid steamIDUser, const char * pchName )
{
    return self->ClearUserAchievement(steamIDUser, pchName);
}

STEAMAPI_API SteamAPICall_t SteamAPI_ISteamGameServerStats_StoreUserStats( ISteamGameServerStats* self, uint64_steamid steamIDUser )
{
    return self->StoreUserStats(steamIDUser);
}

STEAMAPI_API void SteamAPI_ISteamNetworkingFakeUDPPort_DestroyFakeUDPPort( ISteamNetworkingFakeUDPPort* self )
{
    return self->DestroyFakeUDPPort();
}

STEAMAPI_API EResult SteamAPI_ISteamNetworkingFakeUDPPort_SendMessageToFakeIP( ISteamNetworkingFakeUDPPort* self, const SteamNetworkingIPAddr & remoteAddress, const void * pData, uint32 cbData, int nSendFlags )
{
    return self->SendMessageToFakeIP(remoteAddress, pData, cbData, nSendFlags);
}

STEAMAPI_API int SteamAPI_ISteamNetworkingFakeUDPPort_ReceiveMessages( ISteamNetworkingFakeUDPPort* self, SteamNetworkingMessage_t ** ppOutMessages, int nMaxMessages )
{
    return self->ReceiveMessages(ppOutMessages, nMaxMessages);
}

STEAMAPI_API void SteamAPI_ISteamNetworkingFakeUDPPort_ScheduleCleanup( ISteamNetworkingFakeUDPPort* self, const SteamNetworkingIPAddr & remoteAddress )
{
    return self->ScheduleCleanup(remoteAddress);
}

STEAMAPI_API steam_bool SteamAPI_SteamIPAddress_t_IsSet( SteamIPAddress_t* self )
{
    return self->IsSet();
}

STEAMAPI_API void SteamAPI_MatchMakingKeyValuePair_t_Construct( MatchMakingKeyValuePair_t* self )
{
    new(self) MatchMakingKeyValuePair_t();
}

STEAMAPI_API void SteamAPI_servernetadr_t_Construct( servernetadr_t* self )
{
    new(self) servernetadr_t();
}

STEAMAPI_API void SteamAPI_servernetadr_t_Init( servernetadr_t* self, unsigned int ip, uint16 usQueryPort, uint16 usConnectionPort )
{
    return self->Init(ip, usQueryPort, usConnectionPort);
}

STEAMAPI_API uint16 SteamAPI_servernetadr_t_GetQueryPort( servernetadr_t* self )
{
    return self->GetQueryPort();
}

STEAMAPI_API void SteamAPI_servernetadr_t_SetQueryPort( servernetadr_t* self, uint16 usPort )
{
    // some games maybe sending nullptr to self?
    if (!self)
        return;
    return self->SetQueryPort(usPort);
}

STEAMAPI_API uint16 SteamAPI_servernetadr_t_GetConnectionPort( servernetadr_t* self )
{
    return self->GetConnectionPort();
}

STEAMAPI_API void SteamAPI_servernetadr_t_SetConnectionPort( servernetadr_t* self, uint16 usPort )
{
    return self->SetConnectionPort(usPort);
}

STEAMAPI_API uint32 SteamAPI_servernetadr_t_GetIP( servernetadr_t* self )
{
    return self->GetIP();
}

STEAMAPI_API void SteamAPI_servernetadr_t_SetIP( servernetadr_t* self, uint32 unIP )
{
    return self->SetIP(unIP);
}

STEAMAPI_API const char * SteamAPI_servernetadr_t_GetConnectionAddressString( servernetadr_t* self )
{
    return self->GetConnectionAddressString();
}

STEAMAPI_API const char * SteamAPI_servernetadr_t_GetQueryAddressString( servernetadr_t* self )
{
    return self->GetQueryAddressString();
}

STEAMAPI_API steam_bool SteamAPI_servernetadr_t_IsLessThan( servernetadr_t* self, const servernetadr_t & netadr )
{
    return self->operator<(netadr);
}

STEAMAPI_API void SteamAPI_servernetadr_t_Assign( servernetadr_t* self, const servernetadr_t & that )
{
    return self->operator=(that);
}

STEAMAPI_API void SteamAPI_gameserveritem_t_Construct( gameserveritem_t* self )
{
    new(self) gameserveritem_t();
}

STEAMAPI_API const char * SteamAPI_gameserveritem_t_GetName( gameserveritem_t* self )
{
    return self->GetName();
}

STEAMAPI_API void SteamAPI_gameserveritem_t_SetName( gameserveritem_t* self, const char * pName )
{
    return self->SetName(pName);
}

STEAMAPI_API void SteamAPI_SteamNetworkingIPAddr_Clear( SteamNetworkingIPAddr* self )
{
    return self->Clear();
}

STEAMAPI_API steam_bool SteamAPI_SteamNetworkingIPAddr_IsIPv6AllZeros( SteamNetworkingIPAddr* self )
{
    return self->IsIPv6AllZeros();
}

STEAMAPI_API void SteamAPI_SteamNetworkingIPAddr_SetIPv6( SteamNetworkingIPAddr* self, const uint8 * ipv6, uint16 nPort )
{
    return self->SetIPv6(ipv6, nPort);
}

STEAMAPI_API void SteamAPI_SteamNetworkingIPAddr_SetIPv4( SteamNetworkingIPAddr* self, uint32 nIP, uint16 nPort )
{
    return self->SetIPv4(nIP, nPort);
}

STEAMAPI_API steam_bool SteamAPI_SteamNetworkingIPAddr_IsIPv4( SteamNetworkingIPAddr* self )
{
    return self->IsIPv4();
}

STEAMAPI_API uint32 SteamAPI_SteamNetworkingIPAddr_GetIPv4( SteamNetworkingIPAddr* self )
{
    return self->GetIPv4();
}

STEAMAPI_API void SteamAPI_SteamNetworkingIPAddr_SetIPv6LocalHost( SteamNetworkingIPAddr* self, uint16 nPort )
{
    return self->SetIPv6LocalHost(nPort);
}

STEAMAPI_API steam_bool SteamAPI_SteamNetworkingIPAddr_IsLocalHost( SteamNetworkingIPAddr* self )
{
    return self->IsLocalHost();
}

STEAMAPI_API void SteamAPI_SteamNetworkingIPAddr_ToString( SteamNetworkingIPAddr* self, char * buf, uint32 cbBuf, bool bWithPort )
{
    return self->ToString(buf, cbBuf, bWithPort);
}

STEAMAPI_API steam_bool SteamAPI_SteamNetworkingIPAddr_ParseString( SteamNetworkingIPAddr* self, const char * pszStr )
{
    return self->ParseString(pszStr);
}

STEAMAPI_API steam_bool SteamAPI_SteamNetworkingIPAddr_IsEqualTo( SteamNetworkingIPAddr* self, const SteamNetworkingIPAddr & x )
{
    return self->operator==(x);
}

STEAMAPI_API ESteamNetworkingFakeIPType SteamAPI_SteamNetworkingIPAddr_GetFakeIPType( SteamNetworkingIPAddr* self )
{
    return self->GetFakeIPType();
}

STEAMAPI_API steam_bool SteamAPI_SteamNetworkingIPAddr_IsFakeIP( SteamNetworkingIPAddr* self )
{
    return self->IsFakeIP();
}

STEAMAPI_API void SteamAPI_SteamNetworkingIdentity_Clear( SteamNetworkingIdentity* self )
{
    return self->Clear();
}

STEAMAPI_API steam_bool SteamAPI_SteamNetworkingIdentity_IsInvalid( SteamNetworkingIdentity* self )
{
    return self->IsInvalid();
}

STEAMAPI_API void SteamAPI_SteamNetworkingIdentity_SetSteamID( SteamNetworkingIdentity* self, uint64_steamid steamID )
{
    return self->SetSteamID(steamID);
}

STEAMAPI_API uint64_steamid SteamAPI_SteamNetworkingIdentity_GetSteamID( SteamNetworkingIdentity* self )
{
    return self->GetSteamID().ConvertToUint64();
}

STEAMAPI_API void SteamAPI_SteamNetworkingIdentity_SetSteamID64( SteamNetworkingIdentity* self, uint64 steamID )
{
    return self->SetSteamID64(steamID);
}

STEAMAPI_API uint64 SteamAPI_SteamNetworkingIdentity_GetSteamID64( SteamNetworkingIdentity* self )
{
    return self->GetSteamID64();
}

STEAMAPI_API steam_bool SteamAPI_SteamNetworkingIdentity_SetXboxPairwiseID( SteamNetworkingIdentity* self, const char * pszString )
{
    return false;//self->SetXboxPairwiseID(pszString);
}

STEAMAPI_API const char * SteamAPI_SteamNetworkingIdentity_GetXboxPairwiseID( SteamNetworkingIdentity* self )
{
    return "";//self->GetXboxPairwiseID();
}

STEAMAPI_API void SteamAPI_SteamNetworkingIdentity_SetPSNID( SteamNetworkingIdentity* self, uint64 id )
{

}

STEAMAPI_API uint64 SteamAPI_SteamNetworkingIdentity_GetPSNID( SteamNetworkingIdentity* self )
{
    return 0;
}

STEAMAPI_API void SteamAPI_SteamNetworkingIdentity_SetStadiaID( SteamNetworkingIdentity* self, uint64 id )
{

}

STEAMAPI_API uint64 SteamAPI_SteamNetworkingIdentity_GetStadiaID( SteamNetworkingIdentity* self )
{
    return 0;
}

STEAMAPI_API void SteamAPI_SteamNetworkingIdentity_SetIPAddr( SteamNetworkingIdentity* self, const SteamNetworkingIPAddr & addr )
{
    return self->SetIPAddr(addr);
}

STEAMAPI_API const SteamNetworkingIPAddr * SteamAPI_SteamNetworkingIdentity_GetIPAddr( SteamNetworkingIdentity* self )
{
    return self->GetIPAddr();
}

STEAMAPI_API void SteamAPI_SteamNetworkingIdentity_SetIPv4Addr( SteamNetworkingIdentity* self, uint32 nIPv4, uint16 nPort )
{
    SteamNetworkingIPAddr addr = {};
    addr.SetIPv4(nIPv4, nPort);
    self->SetIPAddr(addr);
}

STEAMAPI_API uint32 SteamAPI_SteamNetworkingIdentity_GetIPv4( SteamNetworkingIdentity* self )
{
    return self->GetIPAddr()->GetIPv4();
}

STEAMAPI_API ESteamNetworkingFakeIPType SteamAPI_SteamNetworkingIdentity_GetFakeIPType( SteamNetworkingIdentity* self )
{
    //return self->GetFakeIPType();
    return self->GetIPAddr()->GetFakeIPType();
}

STEAMAPI_API steam_bool SteamAPI_SteamNetworkingIdentity_IsFakeIP( SteamNetworkingIdentity* self )
{
    return self->GetIPAddr()->IsFakeIP();
}

STEAMAPI_API void SteamAPI_SteamNetworkingIdentity_SetLocalHost( SteamNetworkingIdentity* self )
{
    return self->SetLocalHost();
}

STEAMAPI_API steam_bool SteamAPI_SteamNetworkingIdentity_IsLocalHost( SteamNetworkingIdentity* self )
{
    return self->IsLocalHost();
}

STEAMAPI_API steam_bool SteamAPI_SteamNetworkingIdentity_SetGenericString( SteamNetworkingIdentity* self, const char * pszString )
{
    return self->SetGenericString(pszString);
}

STEAMAPI_API const char * SteamAPI_SteamNetworkingIdentity_GetGenericString( SteamNetworkingIdentity* self )
{
    return self->GetGenericString();
}

STEAMAPI_API steam_bool SteamAPI_SteamNetworkingIdentity_SetGenericBytes( SteamNetworkingIdentity* self, const void * data, uint32 cbLen )
{
    return self->SetGenericBytes(data, cbLen);
}

STEAMAPI_API const uint8 * SteamAPI_SteamNetworkingIdentity_GetGenericBytes( SteamNetworkingIdentity* self, int & cbLen )
{
    return self->GetGenericBytes(cbLen);
}

STEAMAPI_API steam_bool SteamAPI_SteamNetworkingIdentity_IsEqualTo( SteamNetworkingIdentity* self, const SteamNetworkingIdentity & x )
{
    return self->operator==(x);
}

STEAMAPI_API void SteamAPI_SteamNetworkingIdentity_ToString( SteamNetworkingIdentity* self, char * buf, uint32 cbBuf )
{
    return self->ToString(buf, cbBuf);
}

STEAMAPI_API steam_bool SteamAPI_SteamNetworkingIdentity_ParseString( SteamNetworkingIdentity* self, const char * pszStr )
{
    return self->ParseString(pszStr);
}

STEAMAPI_API void SteamAPI_SteamNetworkingMessage_t_Release( SteamNetworkingMessage_t* self )
{
    return self->Release();
}

STEAMAPI_API void SteamAPI_SteamNetworkingConfigValue_t_SetInt32( SteamNetworkingConfigValue_t* self, ESteamNetworkingConfigValue eVal, int32_t data )
{
    return self->SetInt32(eVal, data);
}

STEAMAPI_API void SteamAPI_SteamNetworkingConfigValue_t_SetInt64( SteamNetworkingConfigValue_t* self, ESteamNetworkingConfigValue eVal, int64_t data )
{
    return self->SetInt64(eVal, data);
}

STEAMAPI_API void SteamAPI_SteamNetworkingConfigValue_t_SetFloat( SteamNetworkingConfigValue_t* self, ESteamNetworkingConfigValue eVal, float data )
{
    return self->SetFloat(eVal, data);
}

STEAMAPI_API void SteamAPI_SteamNetworkingConfigValue_t_SetPtr( SteamNetworkingConfigValue_t* self, ESteamNetworkingConfigValue eVal, void * data )
{
    return self->SetPtr(eVal, data);
}

STEAMAPI_API void SteamAPI_SteamNetworkingConfigValue_t_SetString( SteamNetworkingConfigValue_t* self, ESteamNetworkingConfigValue eVal, const char * data )
{
    return self->SetString(eVal, data);
}

STEAMAPI_API const char * SteamAPI_SteamNetworkingPOPIDRender_c_str( SteamNetworkingPOPIDRender* self )
{
    return self->c_str();
}

STEAMAPI_API const char * SteamAPI_SteamNetworkingIdentityRender_c_str( SteamNetworkingIdentityRender* self )
{
    return self->c_str();
}

STEAMAPI_API const char * SteamAPI_SteamNetworkingIPAddrRender_c_str( SteamNetworkingIPAddrRender* self )
{
    return self->c_str();
}

STEAMAPI_API void SteamAPI_SteamDatagramHostedAddress_Clear( SteamDatagramHostedAddress* self )
{
    return self->Clear();
}

STEAMAPI_API SteamNetworkingPOPID SteamAPI_SteamDatagramHostedAddress_GetPopID( SteamDatagramHostedAddress* self )
{
    return self->GetPopID();
}

STEAMAPI_API void SteamAPI_SteamDatagramHostedAddress_SetDevAddress( SteamDatagramHostedAddress* self, uint32 nIP, uint16 nPort, SteamNetworkingPOPID popid )
{
    return self->SetDevAddress(nIP, nPort, popid);
}

#endif
