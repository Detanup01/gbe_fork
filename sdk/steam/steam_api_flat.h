//====== Copyright Valve Corporation, All rights reserved. ====================
//
// Purpose: Header for "flat" SteamAPI. Use this for binding to other languages.
// This file is auto-generated, do not edit it.
//
//=============================================================================

#ifndef STEAMAPIFLAT_H
#define STEAMAPIFLAT_H

#include "steam_api.h"
#include "isteamgameserver.h"
#include "isteamgameserverstats.h"

typedef uint64 uint64_steamid; // Used when passing or returning CSteamID
typedef uint64 uint64_gameid; // Used when passing or return CGameID



// ISteamClient
STEAMAPI_API HSteamPipe SteamAPI_ISteamClient_CreateSteamPipe( ISteamClient* self );
STEAMAPI_API steam_bool SteamAPI_ISteamClient_BReleaseSteamPipe( ISteamClient* self, HSteamPipe hSteamPipe );
STEAMAPI_API HSteamUser SteamAPI_ISteamClient_ConnectToGlobalUser( ISteamClient* self, HSteamPipe hSteamPipe );
STEAMAPI_API HSteamUser SteamAPI_ISteamClient_CreateLocalUser( ISteamClient* self, HSteamPipe * phSteamPipe, EAccountType eAccountType );
STEAMAPI_API void SteamAPI_ISteamClient_ReleaseUser( ISteamClient* self, HSteamPipe hSteamPipe, HSteamUser hUser );
STEAMAPI_API ISteamUser * SteamAPI_ISteamClient_GetISteamUser( ISteamClient* self, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char * pchVersion );
STEAMAPI_API ISteamGameServer * SteamAPI_ISteamClient_GetISteamGameServer( ISteamClient* self, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char * pchVersion );
STEAMAPI_API void SteamAPI_ISteamClient_SetLocalIPBinding( ISteamClient* self, const SteamIPAddress_t & unIP, uint16 usPort );
STEAMAPI_API ISteamFriends * SteamAPI_ISteamClient_GetISteamFriends( ISteamClient* self, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char * pchVersion );
STEAMAPI_API ISteamUtils * SteamAPI_ISteamClient_GetISteamUtils( ISteamClient* self, HSteamPipe hSteamPipe, const char * pchVersion );
STEAMAPI_API ISteamMatchmaking * SteamAPI_ISteamClient_GetISteamMatchmaking( ISteamClient* self, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char * pchVersion );
STEAMAPI_API ISteamMatchmakingServers * SteamAPI_ISteamClient_GetISteamMatchmakingServers( ISteamClient* self, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char * pchVersion );
STEAMAPI_API void * SteamAPI_ISteamClient_GetISteamGenericInterface( ISteamClient* self, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char * pchVersion );
STEAMAPI_API ISteamUserStats * SteamAPI_ISteamClient_GetISteamUserStats( ISteamClient* self, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char * pchVersion );
STEAMAPI_API ISteamGameServerStats * SteamAPI_ISteamClient_GetISteamGameServerStats( ISteamClient* self, HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char * pchVersion );
STEAMAPI_API ISteamApps * SteamAPI_ISteamClient_GetISteamApps( ISteamClient* self, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char * pchVersion );
STEAMAPI_API ISteamNetworking * SteamAPI_ISteamClient_GetISteamNetworking( ISteamClient* self, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char * pchVersion );
STEAMAPI_API ISteamRemoteStorage * SteamAPI_ISteamClient_GetISteamRemoteStorage( ISteamClient* self, HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char * pchVersion );
STEAMAPI_API ISteamScreenshots * SteamAPI_ISteamClient_GetISteamScreenshots( ISteamClient* self, HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char * pchVersion );
STEAMAPI_API ISteamGameSearch * SteamAPI_ISteamClient_GetISteamGameSearch( ISteamClient* self, HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char * pchVersion );
STEAMAPI_API uint32 SteamAPI_ISteamClient_GetIPCCallCount( ISteamClient* self );
STEAMAPI_API void SteamAPI_ISteamClient_SetWarningMessageHook( ISteamClient* self, SteamAPIWarningMessageHook_t pFunction );
STEAMAPI_API steam_bool SteamAPI_ISteamClient_BShutdownIfAllPipesClosed( ISteamClient* self );
STEAMAPI_API ISteamHTTP * SteamAPI_ISteamClient_GetISteamHTTP( ISteamClient* self, HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char * pchVersion );
STEAMAPI_API ISteamController * SteamAPI_ISteamClient_GetISteamController( ISteamClient* self, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char * pchVersion );
STEAMAPI_API ISteamUGC * SteamAPI_ISteamClient_GetISteamUGC( ISteamClient* self, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char * pchVersion );
STEAMAPI_API ISteamAppList * SteamAPI_ISteamClient_GetISteamAppList( ISteamClient* self, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char * pchVersion );
STEAMAPI_API ISteamMusic * SteamAPI_ISteamClient_GetISteamMusic( ISteamClient* self, HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char * pchVersion );
STEAMAPI_API ISteamMusicRemote * SteamAPI_ISteamClient_GetISteamMusicRemote( ISteamClient* self, HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char * pchVersion );
STEAMAPI_API ISteamHTMLSurface * SteamAPI_ISteamClient_GetISteamHTMLSurface( ISteamClient* self, HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char * pchVersion );
STEAMAPI_API ISteamInventory * SteamAPI_ISteamClient_GetISteamInventory( ISteamClient* self, HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char * pchVersion );
STEAMAPI_API ISteamVideo * SteamAPI_ISteamClient_GetISteamVideo( ISteamClient* self, HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char * pchVersion );
STEAMAPI_API ISteamParentalSettings * SteamAPI_ISteamClient_GetISteamParentalSettings( ISteamClient* self, HSteamUser hSteamuser, HSteamPipe hSteamPipe, const char * pchVersion );
STEAMAPI_API ISteamInput * SteamAPI_ISteamClient_GetISteamInput( ISteamClient* self, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char * pchVersion );
STEAMAPI_API ISteamParties * SteamAPI_ISteamClient_GetISteamParties( ISteamClient* self, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char * pchVersion );
STEAMAPI_API ISteamRemotePlay * SteamAPI_ISteamClient_GetISteamRemotePlay( ISteamClient* self, HSteamUser hSteamUser, HSteamPipe hSteamPipe, const char * pchVersion );

// ISteamUser
STEAMAPI_API ISteamUser *SteamAPI_SteamUser_v023();
STEAMAPI_API ISteamUser *SteamAPI_SteamUser_v022();
STEAMAPI_API ISteamUser *SteamAPI_SteamUser_v021();
STEAMAPI_API ISteamUser *SteamAPI_SteamUser_v020();
STEAMAPI_API HSteamUser SteamAPI_ISteamUser_GetHSteamUser( ISteamUser* self );
STEAMAPI_API steam_bool SteamAPI_ISteamUser_BLoggedOn( ISteamUser* self );
STEAMAPI_API uint64_steamid SteamAPI_ISteamUser_GetSteamID( ISteamUser* self );
STEAMAPI_API int SteamAPI_ISteamUser_InitiateGameConnection( ISteamUser* self, void * pAuthBlob, int cbMaxAuthBlob, uint64_steamid steamIDGameServer, uint32 unIPServer, uint16 usPortServer, bool bSecure );
STEAMAPI_API void SteamAPI_ISteamUser_TerminateGameConnection( ISteamUser* self, uint32 unIPServer, uint16 usPortServer );
STEAMAPI_API int SteamAPI_ISteamUser_InitiateGameConnection_DEPRECATED( ISteamUser* self, void * pAuthBlob, int cbMaxAuthBlob, uint64_steamid steamIDGameServer, uint32 unIPServer, uint16 usPortServer, bool bSecure );
STEAMAPI_API void SteamAPI_ISteamUser_TerminateGameConnection_DEPRECATED( ISteamUser* self, uint32 unIPServer, uint16 usPortServer );
STEAMAPI_API void SteamAPI_ISteamUser_TrackAppUsageEvent( ISteamUser* self, uint64_gameid gameID, int eAppUsageEvent, const char * pchExtraInfo );
STEAMAPI_API steam_bool SteamAPI_ISteamUser_GetUserDataFolder( ISteamUser* self, char * pchBuffer, int cubBuffer );
STEAMAPI_API void SteamAPI_ISteamUser_StartVoiceRecording( ISteamUser* self );
STEAMAPI_API void SteamAPI_ISteamUser_StopVoiceRecording( ISteamUser* self );
STEAMAPI_API EVoiceResult SteamAPI_ISteamUser_GetAvailableVoice( ISteamUser* self, uint32 * pcbCompressed, uint32 * pcbUncompressed_Deprecated, uint32 nUncompressedVoiceDesiredSampleRate_Deprecated );
STEAMAPI_API EVoiceResult SteamAPI_ISteamUser_GetVoice( ISteamUser* self, bool bWantCompressed, void * pDestBuffer, uint32 cbDestBufferSize, uint32 * nBytesWritten, bool bWantUncompressed_Deprecated, void * pUncompressedDestBuffer_Deprecated, uint32 cbUncompressedDestBufferSize_Deprecated, uint32 * nUncompressBytesWritten_Deprecated, uint32 nUncompressedVoiceDesiredSampleRate_Deprecated );
STEAMAPI_API EVoiceResult SteamAPI_ISteamUser_DecompressVoice( ISteamUser* self, const void * pCompressed, uint32 cbCompressed, void * pDestBuffer, uint32 cbDestBufferSize, uint32 * nBytesWritten, uint32 nDesiredSampleRate );
STEAMAPI_API uint32 SteamAPI_ISteamUser_GetVoiceOptimalSampleRate( ISteamUser* self );
STEAMAPI_API HAuthTicket SteamAPI_ISteamUser_GetAuthSessionTicket( ISteamUser* self, void * pTicket, int cbMaxTicket, uint32 * pcbTicket, const SteamNetworkingIdentity * pSteamNetworkingIdentity );
STEAMAPI_API HAuthTicket SteamAPI_ISteamUser_GetAuthTicketForWebApi( ISteamUser* self, const char * pchIdentity );
STEAMAPI_API EBeginAuthSessionResult SteamAPI_ISteamUser_BeginAuthSession( ISteamUser* self, const void * pAuthTicket, int cbAuthTicket, uint64_steamid steamID );
STEAMAPI_API void SteamAPI_ISteamUser_EndAuthSession( ISteamUser* self, uint64_steamid steamID );
STEAMAPI_API void SteamAPI_ISteamUser_CancelAuthTicket( ISteamUser* self, HAuthTicket hAuthTicket );
STEAMAPI_API EUserHasLicenseForAppResult SteamAPI_ISteamUser_UserHasLicenseForApp( ISteamUser* self, uint64_steamid steamID, AppId_t appID );
STEAMAPI_API steam_bool SteamAPI_ISteamUser_BIsBehindNAT( ISteamUser* self );
STEAMAPI_API void SteamAPI_ISteamUser_AdvertiseGame( ISteamUser* self, uint64_steamid steamIDGameServer, uint32 unIPServer, uint16 usPortServer );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUser_RequestEncryptedAppTicket( ISteamUser* self, void * pDataToInclude, int cbDataToInclude );
STEAMAPI_API steam_bool SteamAPI_ISteamUser_GetEncryptedAppTicket( ISteamUser* self, void * pTicket, int cbMaxTicket, uint32 * pcbTicket );
STEAMAPI_API int SteamAPI_ISteamUser_GetGameBadgeLevel( ISteamUser* self, int nSeries, bool bFoil );
STEAMAPI_API int SteamAPI_ISteamUser_GetPlayerSteamLevel( ISteamUser* self );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUser_RequestStoreAuthURL( ISteamUser* self, const char * pchRedirectURL );
STEAMAPI_API steam_bool SteamAPI_ISteamUser_BIsPhoneVerified( ISteamUser* self );
STEAMAPI_API steam_bool SteamAPI_ISteamUser_BIsTwoFactorEnabled( ISteamUser* self );
STEAMAPI_API steam_bool SteamAPI_ISteamUser_BIsPhoneIdentifying( ISteamUser* self );
STEAMAPI_API steam_bool SteamAPI_ISteamUser_BIsPhoneRequiringVerification( ISteamUser* self );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUser_GetMarketEligibility( ISteamUser* self );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUser_GetDurationControl( ISteamUser* self );
STEAMAPI_API steam_bool SteamAPI_ISteamUser_BSetDurationControlOnlineState( ISteamUser* self, EDurationControlOnlineState eNewState );

// ISteamFriends
STEAMAPI_API ISteamFriends *SteamAPI_SteamFriends_v017();
STEAMAPI_API const char * SteamAPI_ISteamFriends_GetPersonaName( ISteamFriends* self );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamFriends_SetPersonaName( ISteamFriends* self, const char * pchPersonaName );
STEAMAPI_API EPersonaState SteamAPI_ISteamFriends_GetPersonaState( ISteamFriends* self );
STEAMAPI_API int SteamAPI_ISteamFriends_GetFriendCount( ISteamFriends* self, int iFriendFlags );
STEAMAPI_API uint64_steamid SteamAPI_ISteamFriends_GetFriendByIndex( ISteamFriends* self, int iFriend, int iFriendFlags );
STEAMAPI_API EFriendRelationship SteamAPI_ISteamFriends_GetFriendRelationship( ISteamFriends* self, uint64_steamid steamIDFriend );
STEAMAPI_API EPersonaState SteamAPI_ISteamFriends_GetFriendPersonaState( ISteamFriends* self, uint64_steamid steamIDFriend );
STEAMAPI_API const char * SteamAPI_ISteamFriends_GetFriendPersonaName( ISteamFriends* self, uint64_steamid steamIDFriend );
STEAMAPI_API steam_bool SteamAPI_ISteamFriends_GetFriendGamePlayed( ISteamFriends* self, uint64_steamid steamIDFriend, FriendGameInfo_t * pFriendGameInfo );
STEAMAPI_API const char * SteamAPI_ISteamFriends_GetFriendPersonaNameHistory( ISteamFriends* self, uint64_steamid steamIDFriend, int iPersonaName );
STEAMAPI_API int SteamAPI_ISteamFriends_GetFriendSteamLevel( ISteamFriends* self, uint64_steamid steamIDFriend );
STEAMAPI_API const char * SteamAPI_ISteamFriends_GetPlayerNickname( ISteamFriends* self, uint64_steamid steamIDPlayer );
STEAMAPI_API int SteamAPI_ISteamFriends_GetFriendsGroupCount( ISteamFriends* self );
STEAMAPI_API FriendsGroupID_t SteamAPI_ISteamFriends_GetFriendsGroupIDByIndex( ISteamFriends* self, int iFG );
STEAMAPI_API const char * SteamAPI_ISteamFriends_GetFriendsGroupName( ISteamFriends* self, FriendsGroupID_t friendsGroupID );
STEAMAPI_API int SteamAPI_ISteamFriends_GetFriendsGroupMembersCount( ISteamFriends* self, FriendsGroupID_t friendsGroupID );
STEAMAPI_API void SteamAPI_ISteamFriends_GetFriendsGroupMembersList( ISteamFriends* self, FriendsGroupID_t friendsGroupID, CSteamID * pOutSteamIDMembers, int nMembersCount );
STEAMAPI_API steam_bool SteamAPI_ISteamFriends_HasFriend( ISteamFriends* self, uint64_steamid steamIDFriend, int iFriendFlags );
STEAMAPI_API int SteamAPI_ISteamFriends_GetClanCount( ISteamFriends* self );
STEAMAPI_API uint64_steamid SteamAPI_ISteamFriends_GetClanByIndex( ISteamFriends* self, int iClan );
STEAMAPI_API const char * SteamAPI_ISteamFriends_GetClanName( ISteamFriends* self, uint64_steamid steamIDClan );
STEAMAPI_API const char * SteamAPI_ISteamFriends_GetClanTag( ISteamFriends* self, uint64_steamid steamIDClan );
STEAMAPI_API steam_bool SteamAPI_ISteamFriends_GetClanActivityCounts( ISteamFriends* self, uint64_steamid steamIDClan, int * pnOnline, int * pnInGame, int * pnChatting );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamFriends_DownloadClanActivityCounts( ISteamFriends* self, CSteamID * psteamIDClans, int cClansToRequest );
STEAMAPI_API int SteamAPI_ISteamFriends_GetFriendCountFromSource( ISteamFriends* self, uint64_steamid steamIDSource );
STEAMAPI_API uint64_steamid SteamAPI_ISteamFriends_GetFriendFromSourceByIndex( ISteamFriends* self, uint64_steamid steamIDSource, int iFriend );
STEAMAPI_API steam_bool SteamAPI_ISteamFriends_IsUserInSource( ISteamFriends* self, uint64_steamid steamIDUser, uint64_steamid steamIDSource );
STEAMAPI_API void SteamAPI_ISteamFriends_SetInGameVoiceSpeaking( ISteamFriends* self, uint64_steamid steamIDUser, bool bSpeaking );
STEAMAPI_API void SteamAPI_ISteamFriends_ActivateGameOverlay( ISteamFriends* self, const char * pchDialog );
STEAMAPI_API void SteamAPI_ISteamFriends_ActivateGameOverlayToUser( ISteamFriends* self, const char * pchDialog, uint64_steamid steamID );
STEAMAPI_API void SteamAPI_ISteamFriends_ActivateGameOverlayToWebPage( ISteamFriends* self, const char * pchURL, EActivateGameOverlayToWebPageMode eMode );
STEAMAPI_API void SteamAPI_ISteamFriends_ActivateGameOverlayToStore( ISteamFriends* self, AppId_t nAppID, EOverlayToStoreFlag eFlag );
STEAMAPI_API void SteamAPI_ISteamFriends_SetPlayedWith( ISteamFriends* self, uint64_steamid steamIDUserPlayedWith );
STEAMAPI_API void SteamAPI_ISteamFriends_ActivateGameOverlayInviteDialog( ISteamFriends* self, uint64_steamid steamIDLobby );
STEAMAPI_API int SteamAPI_ISteamFriends_GetSmallFriendAvatar( ISteamFriends* self, uint64_steamid steamIDFriend );
STEAMAPI_API int SteamAPI_ISteamFriends_GetMediumFriendAvatar( ISteamFriends* self, uint64_steamid steamIDFriend );
STEAMAPI_API int SteamAPI_ISteamFriends_GetLargeFriendAvatar( ISteamFriends* self, uint64_steamid steamIDFriend );
STEAMAPI_API steam_bool SteamAPI_ISteamFriends_RequestUserInformation( ISteamFriends* self, uint64_steamid steamIDUser, bool bRequireNameOnly );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamFriends_RequestClanOfficerList( ISteamFriends* self, uint64_steamid steamIDClan );
STEAMAPI_API uint64_steamid SteamAPI_ISteamFriends_GetClanOwner( ISteamFriends* self, uint64_steamid steamIDClan );
STEAMAPI_API int SteamAPI_ISteamFriends_GetClanOfficerCount( ISteamFriends* self, uint64_steamid steamIDClan );
STEAMAPI_API uint64_steamid SteamAPI_ISteamFriends_GetClanOfficerByIndex( ISteamFriends* self, uint64_steamid steamIDClan, int iOfficer );
STEAMAPI_API uint32 SteamAPI_ISteamFriends_GetUserRestrictions( ISteamFriends* self );
STEAMAPI_API steam_bool SteamAPI_ISteamFriends_SetRichPresence( ISteamFriends* self, const char * pchKey, const char * pchValue );
STEAMAPI_API void SteamAPI_ISteamFriends_ClearRichPresence( ISteamFriends* self );
STEAMAPI_API const char * SteamAPI_ISteamFriends_GetFriendRichPresence( ISteamFriends* self, uint64_steamid steamIDFriend, const char * pchKey );
STEAMAPI_API int SteamAPI_ISteamFriends_GetFriendRichPresenceKeyCount( ISteamFriends* self, uint64_steamid steamIDFriend );
STEAMAPI_API const char * SteamAPI_ISteamFriends_GetFriendRichPresenceKeyByIndex( ISteamFriends* self, uint64_steamid steamIDFriend, int iKey );
STEAMAPI_API void SteamAPI_ISteamFriends_RequestFriendRichPresence( ISteamFriends* self, uint64_steamid steamIDFriend );
STEAMAPI_API steam_bool SteamAPI_ISteamFriends_InviteUserToGame( ISteamFriends* self, uint64_steamid steamIDFriend, const char * pchConnectString );
STEAMAPI_API int SteamAPI_ISteamFriends_GetCoplayFriendCount( ISteamFriends* self );
STEAMAPI_API uint64_steamid SteamAPI_ISteamFriends_GetCoplayFriend( ISteamFriends* self, int iCoplayFriend );
STEAMAPI_API int SteamAPI_ISteamFriends_GetFriendCoplayTime( ISteamFriends* self, uint64_steamid steamIDFriend );
STEAMAPI_API AppId_t SteamAPI_ISteamFriends_GetFriendCoplayGame( ISteamFriends* self, uint64_steamid steamIDFriend );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamFriends_JoinClanChatRoom( ISteamFriends* self, uint64_steamid steamIDClan );
STEAMAPI_API steam_bool SteamAPI_ISteamFriends_LeaveClanChatRoom( ISteamFriends* self, uint64_steamid steamIDClan );
STEAMAPI_API int SteamAPI_ISteamFriends_GetClanChatMemberCount( ISteamFriends* self, uint64_steamid steamIDClan );
STEAMAPI_API uint64_steamid SteamAPI_ISteamFriends_GetChatMemberByIndex( ISteamFriends* self, uint64_steamid steamIDClan, int iUser );
STEAMAPI_API steam_bool SteamAPI_ISteamFriends_SendClanChatMessage( ISteamFriends* self, uint64_steamid steamIDClanChat, const char * pchText );
STEAMAPI_API int SteamAPI_ISteamFriends_GetClanChatMessage( ISteamFriends* self, uint64_steamid steamIDClanChat, int iMessage, void * prgchText, int cchTextMax, EChatEntryType * peChatEntryType, CSteamID * psteamidChatter );
STEAMAPI_API steam_bool SteamAPI_ISteamFriends_IsClanChatAdmin( ISteamFriends* self, uint64_steamid steamIDClanChat, uint64_steamid steamIDUser );
STEAMAPI_API steam_bool SteamAPI_ISteamFriends_IsClanChatWindowOpenInSteam( ISteamFriends* self, uint64_steamid steamIDClanChat );
STEAMAPI_API steam_bool SteamAPI_ISteamFriends_OpenClanChatWindowInSteam( ISteamFriends* self, uint64_steamid steamIDClanChat );
STEAMAPI_API steam_bool SteamAPI_ISteamFriends_CloseClanChatWindowInSteam( ISteamFriends* self, uint64_steamid steamIDClanChat );
STEAMAPI_API steam_bool SteamAPI_ISteamFriends_SetListenForFriendsMessages( ISteamFriends* self, bool bInterceptEnabled );
STEAMAPI_API steam_bool SteamAPI_ISteamFriends_ReplyToFriendMessage( ISteamFriends* self, uint64_steamid steamIDFriend, const char * pchMsgToSend );
STEAMAPI_API int SteamAPI_ISteamFriends_GetFriendMessage( ISteamFriends* self, uint64_steamid steamIDFriend, int iMessageID, void * pvData, int cubData, EChatEntryType * peChatEntryType );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamFriends_GetFollowerCount( ISteamFriends* self, uint64_steamid steamID );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamFriends_IsFollowing( ISteamFriends* self, uint64_steamid steamID );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamFriends_EnumerateFollowingList( ISteamFriends* self, uint32 unStartIndex );
STEAMAPI_API steam_bool SteamAPI_ISteamFriends_IsClanPublic( ISteamFriends* self, uint64_steamid steamIDClan );
STEAMAPI_API steam_bool SteamAPI_ISteamFriends_IsClanOfficialGameGroup( ISteamFriends* self, uint64_steamid steamIDClan );
STEAMAPI_API int SteamAPI_ISteamFriends_GetNumChatsWithUnreadPriorityMessages( ISteamFriends* self );
STEAMAPI_API void SteamAPI_ISteamFriends_ActivateGameOverlayRemotePlayTogetherInviteDialog( ISteamFriends* self, uint64_steamid steamIDLobby );
STEAMAPI_API steam_bool SteamAPI_ISteamFriends_RegisterProtocolInOverlayBrowser( ISteamFriends* self, const char * pchProtocol );
STEAMAPI_API void SteamAPI_ISteamFriends_ActivateGameOverlayInviteDialogConnectString( ISteamFriends* self, const char * pchConnectString );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamFriends_RequestEquippedProfileItems( ISteamFriends* self, uint64_steamid steamID );
STEAMAPI_API steam_bool SteamAPI_ISteamFriends_BHasEquippedProfileItem( ISteamFriends* self, uint64_steamid steamID, ECommunityProfileItemType itemType );
STEAMAPI_API const char * SteamAPI_ISteamFriends_GetProfileItemPropertyString( ISteamFriends* self, uint64_steamid steamID, ECommunityProfileItemType itemType, ECommunityProfileItemProperty prop );
STEAMAPI_API uint32 SteamAPI_ISteamFriends_GetProfileItemPropertyUint( ISteamFriends* self, uint64_steamid steamID, ECommunityProfileItemType itemType, ECommunityProfileItemProperty prop );

// ISteamUtils
STEAMAPI_API ISteamUtils *SteamAPI_SteamUtils_v010();
STEAMAPI_API ISteamUtils *SteamAPI_SteamGameServerUtils_v010();
STEAMAPI_API ISteamUtils *SteamAPI_SteamUtils_v009();
STEAMAPI_API ISteamUtils *SteamAPI_SteamGameServerUtils_v009();
STEAMAPI_API uint32 SteamAPI_ISteamUtils_GetSecondsSinceAppActive( ISteamUtils* self );
STEAMAPI_API uint32 SteamAPI_ISteamUtils_GetSecondsSinceComputerActive( ISteamUtils* self );
STEAMAPI_API EUniverse SteamAPI_ISteamUtils_GetConnectedUniverse( ISteamUtils* self );
STEAMAPI_API uint32 SteamAPI_ISteamUtils_GetServerRealTime( ISteamUtils* self );
STEAMAPI_API const char * SteamAPI_ISteamUtils_GetIPCountry( ISteamUtils* self );
STEAMAPI_API steam_bool SteamAPI_ISteamUtils_GetImageSize( ISteamUtils* self, int iImage, uint32 * pnWidth, uint32 * pnHeight );
STEAMAPI_API steam_bool SteamAPI_ISteamUtils_GetImageRGBA( ISteamUtils* self, int iImage, uint8 * pubDest, int nDestBufferSize );
STEAMAPI_API steam_bool SteamAPI_ISteamUtils_GetCSERIPPort( ISteamUtils* self, uint32 * unIP, uint16 * usPort );
STEAMAPI_API uint8 SteamAPI_ISteamUtils_GetCurrentBatteryPower( ISteamUtils* self );
STEAMAPI_API uint32 SteamAPI_ISteamUtils_GetAppID( ISteamUtils* self );
STEAMAPI_API void SteamAPI_ISteamUtils_SetOverlayNotificationPosition( ISteamUtils* self, ENotificationPosition eNotificationPosition );
STEAMAPI_API steam_bool SteamAPI_ISteamUtils_IsAPICallCompleted( ISteamUtils* self, SteamAPICall_t hSteamAPICall, bool * pbFailed );
STEAMAPI_API ESteamAPICallFailure SteamAPI_ISteamUtils_GetAPICallFailureReason( ISteamUtils* self, SteamAPICall_t hSteamAPICall );
STEAMAPI_API steam_bool SteamAPI_ISteamUtils_GetAPICallResult( ISteamUtils* self, SteamAPICall_t hSteamAPICall, void * pCallback, int cubCallback, int iCallbackExpected, bool * pbFailed );
STEAMAPI_API uint32 SteamAPI_ISteamUtils_GetIPCCallCount( ISteamUtils* self );
STEAMAPI_API void SteamAPI_ISteamUtils_SetWarningMessageHook( ISteamUtils* self, SteamAPIWarningMessageHook_t pFunction );
STEAMAPI_API steam_bool SteamAPI_ISteamUtils_IsOverlayEnabled( ISteamUtils* self );
STEAMAPI_API steam_bool SteamAPI_ISteamUtils_BOverlayNeedsPresent( ISteamUtils* self );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUtils_CheckFileSignature( ISteamUtils* self, const char * szFileName );
STEAMAPI_API steam_bool SteamAPI_ISteamUtils_ShowGamepadTextInput( ISteamUtils* self, EGamepadTextInputMode eInputMode, EGamepadTextInputLineMode eLineInputMode, const char * pchDescription, uint32 unCharMax, const char * pchExistingText );
STEAMAPI_API uint32 SteamAPI_ISteamUtils_GetEnteredGamepadTextLength( ISteamUtils* self );
STEAMAPI_API steam_bool SteamAPI_ISteamUtils_GetEnteredGamepadTextInput( ISteamUtils* self, char * pchText, uint32 cchText );
STEAMAPI_API const char * SteamAPI_ISteamUtils_GetSteamUILanguage( ISteamUtils* self );
STEAMAPI_API steam_bool SteamAPI_ISteamUtils_IsSteamRunningInVR( ISteamUtils* self );
STEAMAPI_API void SteamAPI_ISteamUtils_SetOverlayNotificationInset( ISteamUtils* self, int nHorizontalInset, int nVerticalInset );
STEAMAPI_API steam_bool SteamAPI_ISteamUtils_IsSteamInBigPictureMode( ISteamUtils* self );
STEAMAPI_API void SteamAPI_ISteamUtils_StartVRDashboard( ISteamUtils* self );
STEAMAPI_API steam_bool SteamAPI_ISteamUtils_IsVRHeadsetStreamingEnabled( ISteamUtils* self );
STEAMAPI_API void SteamAPI_ISteamUtils_SetVRHeadsetStreamingEnabled( ISteamUtils* self, bool bEnabled );
STEAMAPI_API steam_bool SteamAPI_ISteamUtils_IsSteamChinaLauncher( ISteamUtils* self );
STEAMAPI_API steam_bool SteamAPI_ISteamUtils_InitFilterText( ISteamUtils* self, uint32 unFilterOptions );
STEAMAPI_API int SteamAPI_ISteamUtils_FilterText( ISteamUtils* self, ETextFilteringContext eContext, uint64_steamid sourceSteamID, const char * pchInputMessage, char * pchOutFilteredText, uint32 nByteSizeOutFilteredText );
STEAMAPI_API ESteamIPv6ConnectivityState SteamAPI_ISteamUtils_GetIPv6ConnectivityState( ISteamUtils* self, ESteamIPv6ConnectivityProtocol eProtocol );
STEAMAPI_API steam_bool SteamAPI_ISteamUtils_IsSteamRunningOnSteamDeck( ISteamUtils* self );
STEAMAPI_API steam_bool SteamAPI_ISteamUtils_ShowModalGamepadTextInput( ISteamUtils* self, EGamepadTextInputLineMode eLineInputMode );
STEAMAPI_API steam_bool SteamAPI_ISteamUtils_ShowFloatingGamepadTextInput( ISteamUtils* self, EFloatingGamepadTextInputMode eKeyboardMode, int nTextFieldXPosition, int nTextFieldYPosition, int nTextFieldWidth, int nTextFieldHeight );
STEAMAPI_API void SteamAPI_ISteamUtils_SetGameLauncherMode( ISteamUtils* self, bool bLauncherMode );
STEAMAPI_API steam_bool SteamAPI_ISteamUtils_DismissFloatingGamepadTextInput( ISteamUtils* self );
STEAMAPI_API steam_bool SteamAPI_ISteamUtils_DismissGamepadTextInput( ISteamUtils* self );

// ISteamMatchmaking
STEAMAPI_API ISteamMatchmaking *SteamAPI_SteamMatchmaking_v009();
STEAMAPI_API int SteamAPI_ISteamMatchmaking_GetFavoriteGameCount( ISteamMatchmaking* self );
STEAMAPI_API steam_bool SteamAPI_ISteamMatchmaking_GetFavoriteGame( ISteamMatchmaking* self, int iGame, AppId_t * pnAppID, uint32 * pnIP, uint16 * pnConnPort, uint16 * pnQueryPort, uint32 * punFlags, uint32 * pRTime32LastPlayedOnServer );
STEAMAPI_API int SteamAPI_ISteamMatchmaking_AddFavoriteGame( ISteamMatchmaking* self, AppId_t nAppID, uint32 nIP, uint16 nConnPort, uint16 nQueryPort, uint32 unFlags, uint32 rTime32LastPlayedOnServer );
STEAMAPI_API steam_bool SteamAPI_ISteamMatchmaking_RemoveFavoriteGame( ISteamMatchmaking* self, AppId_t nAppID, uint32 nIP, uint16 nConnPort, uint16 nQueryPort, uint32 unFlags );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamMatchmaking_RequestLobbyList( ISteamMatchmaking* self );
STEAMAPI_API void SteamAPI_ISteamMatchmaking_AddRequestLobbyListStringFilter( ISteamMatchmaking* self, const char * pchKeyToMatch, const char * pchValueToMatch, ELobbyComparison eComparisonType );
STEAMAPI_API void SteamAPI_ISteamMatchmaking_AddRequestLobbyListNumericalFilter( ISteamMatchmaking* self, const char * pchKeyToMatch, int nValueToMatch, ELobbyComparison eComparisonType );
STEAMAPI_API void SteamAPI_ISteamMatchmaking_AddRequestLobbyListNearValueFilter( ISteamMatchmaking* self, const char * pchKeyToMatch, int nValueToBeCloseTo );
STEAMAPI_API void SteamAPI_ISteamMatchmaking_AddRequestLobbyListFilterSlotsAvailable( ISteamMatchmaking* self, int nSlotsAvailable );
STEAMAPI_API void SteamAPI_ISteamMatchmaking_AddRequestLobbyListDistanceFilter( ISteamMatchmaking* self, ELobbyDistanceFilter eLobbyDistanceFilter );
STEAMAPI_API void SteamAPI_ISteamMatchmaking_AddRequestLobbyListResultCountFilter( ISteamMatchmaking* self, int cMaxResults );
STEAMAPI_API void SteamAPI_ISteamMatchmaking_AddRequestLobbyListCompatibleMembersFilter( ISteamMatchmaking* self, uint64_steamid steamIDLobby );
STEAMAPI_API uint64_steamid SteamAPI_ISteamMatchmaking_GetLobbyByIndex( ISteamMatchmaking* self, int iLobby );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamMatchmaking_CreateLobby( ISteamMatchmaking* self, ELobbyType eLobbyType, int cMaxMembers );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamMatchmaking_JoinLobby( ISteamMatchmaking* self, uint64_steamid steamIDLobby );
STEAMAPI_API void SteamAPI_ISteamMatchmaking_LeaveLobby( ISteamMatchmaking* self, uint64_steamid steamIDLobby );
STEAMAPI_API steam_bool SteamAPI_ISteamMatchmaking_InviteUserToLobby( ISteamMatchmaking* self, uint64_steamid steamIDLobby, uint64_steamid steamIDInvitee );
STEAMAPI_API int SteamAPI_ISteamMatchmaking_GetNumLobbyMembers( ISteamMatchmaking* self, uint64_steamid steamIDLobby );
STEAMAPI_API uint64_steamid SteamAPI_ISteamMatchmaking_GetLobbyMemberByIndex( ISteamMatchmaking* self, uint64_steamid steamIDLobby, int iMember );
STEAMAPI_API const char * SteamAPI_ISteamMatchmaking_GetLobbyData( ISteamMatchmaking* self, uint64_steamid steamIDLobby, const char * pchKey );
STEAMAPI_API steam_bool SteamAPI_ISteamMatchmaking_SetLobbyData( ISteamMatchmaking* self, uint64_steamid steamIDLobby, const char * pchKey, const char * pchValue );
STEAMAPI_API int SteamAPI_ISteamMatchmaking_GetLobbyDataCount( ISteamMatchmaking* self, uint64_steamid steamIDLobby );
STEAMAPI_API steam_bool SteamAPI_ISteamMatchmaking_GetLobbyDataByIndex( ISteamMatchmaking* self, uint64_steamid steamIDLobby, int iLobbyData, char * pchKey, int cchKeyBufferSize, char * pchValue, int cchValueBufferSize );
STEAMAPI_API steam_bool SteamAPI_ISteamMatchmaking_DeleteLobbyData( ISteamMatchmaking* self, uint64_steamid steamIDLobby, const char * pchKey );
STEAMAPI_API const char * SteamAPI_ISteamMatchmaking_GetLobbyMemberData( ISteamMatchmaking* self, uint64_steamid steamIDLobby, uint64_steamid steamIDUser, const char * pchKey );
STEAMAPI_API void SteamAPI_ISteamMatchmaking_SetLobbyMemberData( ISteamMatchmaking* self, uint64_steamid steamIDLobby, const char * pchKey, const char * pchValue );
STEAMAPI_API steam_bool SteamAPI_ISteamMatchmaking_SendLobbyChatMsg( ISteamMatchmaking* self, uint64_steamid steamIDLobby, const void * pvMsgBody, int cubMsgBody );
STEAMAPI_API int SteamAPI_ISteamMatchmaking_GetLobbyChatEntry( ISteamMatchmaking* self, uint64_steamid steamIDLobby, int iChatID, CSteamID * pSteamIDUser, void * pvData, int cubData, EChatEntryType * peChatEntryType );
STEAMAPI_API steam_bool SteamAPI_ISteamMatchmaking_RequestLobbyData( ISteamMatchmaking* self, uint64_steamid steamIDLobby );
STEAMAPI_API void SteamAPI_ISteamMatchmaking_SetLobbyGameServer( ISteamMatchmaking* self, uint64_steamid steamIDLobby, uint32 unGameServerIP, uint16 unGameServerPort, uint64_steamid steamIDGameServer );
STEAMAPI_API steam_bool SteamAPI_ISteamMatchmaking_GetLobbyGameServer( ISteamMatchmaking* self, uint64_steamid steamIDLobby, uint32 * punGameServerIP, uint16 * punGameServerPort, CSteamID * psteamIDGameServer );
STEAMAPI_API steam_bool SteamAPI_ISteamMatchmaking_SetLobbyMemberLimit( ISteamMatchmaking* self, uint64_steamid steamIDLobby, int cMaxMembers );
STEAMAPI_API int SteamAPI_ISteamMatchmaking_GetLobbyMemberLimit( ISteamMatchmaking* self, uint64_steamid steamIDLobby );
STEAMAPI_API steam_bool SteamAPI_ISteamMatchmaking_SetLobbyType( ISteamMatchmaking* self, uint64_steamid steamIDLobby, ELobbyType eLobbyType );
STEAMAPI_API steam_bool SteamAPI_ISteamMatchmaking_SetLobbyJoinable( ISteamMatchmaking* self, uint64_steamid steamIDLobby, bool bLobbyJoinable );
STEAMAPI_API uint64_steamid SteamAPI_ISteamMatchmaking_GetLobbyOwner( ISteamMatchmaking* self, uint64_steamid steamIDLobby );
STEAMAPI_API steam_bool SteamAPI_ISteamMatchmaking_SetLobbyOwner( ISteamMatchmaking* self, uint64_steamid steamIDLobby, uint64_steamid steamIDNewOwner );
STEAMAPI_API steam_bool SteamAPI_ISteamMatchmaking_SetLinkedLobby( ISteamMatchmaking* self, uint64_steamid steamIDLobby, uint64_steamid steamIDLobbyDependent );

// ISteamMatchmakingServerListResponse
STEAMAPI_API void SteamAPI_ISteamMatchmakingServerListResponse_ServerResponded( ISteamMatchmakingServerListResponse* self, HServerListRequest hRequest, int iServer );
STEAMAPI_API void SteamAPI_ISteamMatchmakingServerListResponse_ServerFailedToRespond( ISteamMatchmakingServerListResponse* self, HServerListRequest hRequest, int iServer );
STEAMAPI_API void SteamAPI_ISteamMatchmakingServerListResponse_RefreshComplete( ISteamMatchmakingServerListResponse* self, HServerListRequest hRequest, EMatchMakingServerResponse response );

// ISteamMatchmakingPingResponse
STEAMAPI_API void SteamAPI_ISteamMatchmakingPingResponse_ServerResponded( ISteamMatchmakingPingResponse* self, gameserveritem_t & server );
STEAMAPI_API void SteamAPI_ISteamMatchmakingPingResponse_ServerFailedToRespond( ISteamMatchmakingPingResponse* self );

// ISteamMatchmakingPlayersResponse
STEAMAPI_API void SteamAPI_ISteamMatchmakingPlayersResponse_AddPlayerToList( ISteamMatchmakingPlayersResponse* self, const char * pchName, int nScore, float flTimePlayed );
STEAMAPI_API void SteamAPI_ISteamMatchmakingPlayersResponse_PlayersFailedToRespond( ISteamMatchmakingPlayersResponse* self );
STEAMAPI_API void SteamAPI_ISteamMatchmakingPlayersResponse_PlayersRefreshComplete( ISteamMatchmakingPlayersResponse* self );

// ISteamMatchmakingRulesResponse
STEAMAPI_API void SteamAPI_ISteamMatchmakingRulesResponse_RulesResponded( ISteamMatchmakingRulesResponse* self, const char * pchRule, const char * pchValue );
STEAMAPI_API void SteamAPI_ISteamMatchmakingRulesResponse_RulesFailedToRespond( ISteamMatchmakingRulesResponse* self );
STEAMAPI_API void SteamAPI_ISteamMatchmakingRulesResponse_RulesRefreshComplete( ISteamMatchmakingRulesResponse* self );

// ISteamMatchmakingServers
STEAMAPI_API ISteamMatchmakingServers *SteamAPI_SteamMatchmakingServers_v002();
STEAMAPI_API HServerListRequest SteamAPI_ISteamMatchmakingServers_RequestInternetServerList( ISteamMatchmakingServers* self, AppId_t iApp, MatchMakingKeyValuePair_t ** ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse * pRequestServersResponse );
STEAMAPI_API HServerListRequest SteamAPI_ISteamMatchmakingServers_RequestLANServerList( ISteamMatchmakingServers* self, AppId_t iApp, ISteamMatchmakingServerListResponse * pRequestServersResponse );
STEAMAPI_API HServerListRequest SteamAPI_ISteamMatchmakingServers_RequestFriendsServerList( ISteamMatchmakingServers* self, AppId_t iApp, MatchMakingKeyValuePair_t ** ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse * pRequestServersResponse );
STEAMAPI_API HServerListRequest SteamAPI_ISteamMatchmakingServers_RequestFavoritesServerList( ISteamMatchmakingServers* self, AppId_t iApp, MatchMakingKeyValuePair_t ** ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse * pRequestServersResponse );
STEAMAPI_API HServerListRequest SteamAPI_ISteamMatchmakingServers_RequestHistoryServerList( ISteamMatchmakingServers* self, AppId_t iApp, MatchMakingKeyValuePair_t ** ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse * pRequestServersResponse );
STEAMAPI_API HServerListRequest SteamAPI_ISteamMatchmakingServers_RequestSpectatorServerList( ISteamMatchmakingServers* self, AppId_t iApp, MatchMakingKeyValuePair_t ** ppchFilters, uint32 nFilters, ISteamMatchmakingServerListResponse * pRequestServersResponse );
STEAMAPI_API void SteamAPI_ISteamMatchmakingServers_ReleaseRequest( ISteamMatchmakingServers* self, HServerListRequest hServerListRequest );
STEAMAPI_API gameserveritem_t * SteamAPI_ISteamMatchmakingServers_GetServerDetails( ISteamMatchmakingServers* self, HServerListRequest hRequest, int iServer );
STEAMAPI_API void SteamAPI_ISteamMatchmakingServers_CancelQuery( ISteamMatchmakingServers* self, HServerListRequest hRequest );
STEAMAPI_API void SteamAPI_ISteamMatchmakingServers_RefreshQuery( ISteamMatchmakingServers* self, HServerListRequest hRequest );
STEAMAPI_API steam_bool SteamAPI_ISteamMatchmakingServers_IsRefreshing( ISteamMatchmakingServers* self, HServerListRequest hRequest );
STEAMAPI_API int SteamAPI_ISteamMatchmakingServers_GetServerCount( ISteamMatchmakingServers* self, HServerListRequest hRequest );
STEAMAPI_API void SteamAPI_ISteamMatchmakingServers_RefreshServer( ISteamMatchmakingServers* self, HServerListRequest hRequest, int iServer );
STEAMAPI_API HServerQuery SteamAPI_ISteamMatchmakingServers_PingServer( ISteamMatchmakingServers* self, uint32 unIP, uint16 usPort, ISteamMatchmakingPingResponse * pRequestServersResponse );
STEAMAPI_API HServerQuery SteamAPI_ISteamMatchmakingServers_PlayerDetails( ISteamMatchmakingServers* self, uint32 unIP, uint16 usPort, ISteamMatchmakingPlayersResponse * pRequestServersResponse );
STEAMAPI_API HServerQuery SteamAPI_ISteamMatchmakingServers_ServerRules( ISteamMatchmakingServers* self, uint32 unIP, uint16 usPort, ISteamMatchmakingRulesResponse * pRequestServersResponse );
STEAMAPI_API void SteamAPI_ISteamMatchmakingServers_CancelServerQuery( ISteamMatchmakingServers* self, HServerQuery hServerQuery );

// ISteamGameSearch
STEAMAPI_API ISteamGameSearch *SteamAPI_SteamGameSearch_v001();
STEAMAPI_API EGameSearchErrorCode_t SteamAPI_ISteamGameSearch_AddGameSearchParams( ISteamGameSearch* self, const char * pchKeyToFind, const char * pchValuesToFind );
STEAMAPI_API EGameSearchErrorCode_t SteamAPI_ISteamGameSearch_SearchForGameWithLobby( ISteamGameSearch* self, uint64_steamid steamIDLobby, int nPlayerMin, int nPlayerMax );
STEAMAPI_API EGameSearchErrorCode_t SteamAPI_ISteamGameSearch_SearchForGameSolo( ISteamGameSearch* self, int nPlayerMin, int nPlayerMax );
STEAMAPI_API EGameSearchErrorCode_t SteamAPI_ISteamGameSearch_AcceptGame( ISteamGameSearch* self );
STEAMAPI_API EGameSearchErrorCode_t SteamAPI_ISteamGameSearch_DeclineGame( ISteamGameSearch* self );
STEAMAPI_API EGameSearchErrorCode_t SteamAPI_ISteamGameSearch_RetrieveConnectionDetails( ISteamGameSearch* self, uint64_steamid steamIDHost, char * pchConnectionDetails, int cubConnectionDetails );
STEAMAPI_API EGameSearchErrorCode_t SteamAPI_ISteamGameSearch_EndGameSearch( ISteamGameSearch* self );
STEAMAPI_API EGameSearchErrorCode_t SteamAPI_ISteamGameSearch_SetGameHostParams( ISteamGameSearch* self, const char * pchKey, const char * pchValue );
STEAMAPI_API EGameSearchErrorCode_t SteamAPI_ISteamGameSearch_SetConnectionDetails( ISteamGameSearch* self, const char * pchConnectionDetails, int cubConnectionDetails );
STEAMAPI_API EGameSearchErrorCode_t SteamAPI_ISteamGameSearch_RequestPlayersForGame( ISteamGameSearch* self, int nPlayerMin, int nPlayerMax, int nMaxTeamSize );
STEAMAPI_API EGameSearchErrorCode_t SteamAPI_ISteamGameSearch_HostConfirmGameStart( ISteamGameSearch* self, uint64 ullUniqueGameID );
STEAMAPI_API EGameSearchErrorCode_t SteamAPI_ISteamGameSearch_CancelRequestPlayersForGame( ISteamGameSearch* self );
STEAMAPI_API EGameSearchErrorCode_t SteamAPI_ISteamGameSearch_SubmitPlayerResult( ISteamGameSearch* self, uint64 ullUniqueGameID, uint64_steamid steamIDPlayer, EPlayerResult_t EPlayerResult );
STEAMAPI_API EGameSearchErrorCode_t SteamAPI_ISteamGameSearch_EndGame( ISteamGameSearch* self, uint64 ullUniqueGameID );

// ISteamParties
STEAMAPI_API ISteamParties *SteamAPI_SteamParties_v002();
STEAMAPI_API uint32 SteamAPI_ISteamParties_GetNumActiveBeacons( ISteamParties* self );
STEAMAPI_API PartyBeaconID_t SteamAPI_ISteamParties_GetBeaconByIndex( ISteamParties* self, uint32 unIndex );
STEAMAPI_API steam_bool SteamAPI_ISteamParties_GetBeaconDetails( ISteamParties* self, PartyBeaconID_t ulBeaconID, CSteamID * pSteamIDBeaconOwner, SteamPartyBeaconLocation_t * pLocation, char * pchMetadata, int cchMetadata );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamParties_JoinParty( ISteamParties* self, PartyBeaconID_t ulBeaconID );
STEAMAPI_API steam_bool SteamAPI_ISteamParties_GetNumAvailableBeaconLocations( ISteamParties* self, uint32 * puNumLocations );
STEAMAPI_API steam_bool SteamAPI_ISteamParties_GetAvailableBeaconLocations( ISteamParties* self, SteamPartyBeaconLocation_t * pLocationList, uint32 uMaxNumLocations );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamParties_CreateBeacon( ISteamParties* self, uint32 unOpenSlots, SteamPartyBeaconLocation_t * pBeaconLocation, const char * pchConnectString, const char * pchMetadata );
STEAMAPI_API void SteamAPI_ISteamParties_OnReservationCompleted( ISteamParties* self, PartyBeaconID_t ulBeacon, uint64_steamid steamIDUser );
STEAMAPI_API void SteamAPI_ISteamParties_CancelReservation( ISteamParties* self, PartyBeaconID_t ulBeacon, uint64_steamid steamIDUser );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamParties_ChangeNumOpenSlots( ISteamParties* self, PartyBeaconID_t ulBeacon, uint32 unOpenSlots );
STEAMAPI_API steam_bool SteamAPI_ISteamParties_DestroyBeacon( ISteamParties* self, PartyBeaconID_t ulBeacon );
STEAMAPI_API steam_bool SteamAPI_ISteamParties_GetBeaconLocationData( ISteamParties* self, SteamPartyBeaconLocation_t BeaconLocation, ESteamPartyBeaconLocationData eData, char * pchDataStringOut, int cchDataStringOut );

// ISteamRemoteStorage
STEAMAPI_API ISteamRemoteStorage *SteamAPI_SteamRemoteStorage_v014();
STEAMAPI_API ISteamRemoteStorage *SteamAPI_SteamRemoteStorage_v016();
STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_FileWrite( ISteamRemoteStorage* self, const char * pchFile, const void * pvData, int32 cubData );
STEAMAPI_API int32 SteamAPI_ISteamRemoteStorage_FileRead( ISteamRemoteStorage* self, const char * pchFile, void * pvData, int32 cubDataToRead );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_FileWriteAsync( ISteamRemoteStorage* self, const char * pchFile, const void * pvData, uint32 cubData );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_FileReadAsync( ISteamRemoteStorage* self, const char * pchFile, uint32 nOffset, uint32 cubToRead );
STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_FileReadAsyncComplete( ISteamRemoteStorage* self, SteamAPICall_t hReadCall, void * pvBuffer, uint32 cubToRead );
STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_FileForget( ISteamRemoteStorage* self, const char * pchFile );
STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_FileDelete( ISteamRemoteStorage* self, const char * pchFile );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_FileShare( ISteamRemoteStorage* self, const char * pchFile );
STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_SetSyncPlatforms( ISteamRemoteStorage* self, const char * pchFile, ERemoteStoragePlatform eRemoteStoragePlatform );
STEAMAPI_API UGCFileWriteStreamHandle_t SteamAPI_ISteamRemoteStorage_FileWriteStreamOpen( ISteamRemoteStorage* self, const char * pchFile );
STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_FileWriteStreamWriteChunk( ISteamRemoteStorage* self, UGCFileWriteStreamHandle_t writeHandle, const void * pvData, int32 cubData );
STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_FileWriteStreamClose( ISteamRemoteStorage* self, UGCFileWriteStreamHandle_t writeHandle );
STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_FileWriteStreamCancel( ISteamRemoteStorage* self, UGCFileWriteStreamHandle_t writeHandle );
STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_FileExists( ISteamRemoteStorage* self, const char * pchFile );
STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_FilePersisted( ISteamRemoteStorage* self, const char * pchFile );
STEAMAPI_API int32 SteamAPI_ISteamRemoteStorage_GetFileSize( ISteamRemoteStorage* self, const char * pchFile );
STEAMAPI_API int64 SteamAPI_ISteamRemoteStorage_GetFileTimestamp( ISteamRemoteStorage* self, const char * pchFile );
STEAMAPI_API ERemoteStoragePlatform SteamAPI_ISteamRemoteStorage_GetSyncPlatforms( ISteamRemoteStorage* self, const char * pchFile );
STEAMAPI_API int32 SteamAPI_ISteamRemoteStorage_GetFileCount( ISteamRemoteStorage* self );
STEAMAPI_API const char * SteamAPI_ISteamRemoteStorage_GetFileNameAndSize( ISteamRemoteStorage* self, int iFile, int32 * pnFileSizeInBytes );
STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_GetQuota( ISteamRemoteStorage* self, uint64 * pnTotalBytes, uint64 * puAvailableBytes );
STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_IsCloudEnabledForAccount( ISteamRemoteStorage* self );
STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_IsCloudEnabledForApp( ISteamRemoteStorage* self );
STEAMAPI_API void SteamAPI_ISteamRemoteStorage_SetCloudEnabledForApp( ISteamRemoteStorage* self, bool bEnabled );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_UGCDownload( ISteamRemoteStorage* self, UGCHandle_t hContent, uint32 unPriority );
STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_GetUGCDownloadProgress( ISteamRemoteStorage* self, UGCHandle_t hContent, int32 * pnBytesDownloaded, int32 * pnBytesExpected );
STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_GetUGCDetails( ISteamRemoteStorage* self, UGCHandle_t hContent, AppId_t * pnAppID, char ** ppchName, int32 * pnFileSizeInBytes, CSteamID * pSteamIDOwner );
STEAMAPI_API int32 SteamAPI_ISteamRemoteStorage_UGCRead( ISteamRemoteStorage* self, UGCHandle_t hContent, void * pvData, int32 cubDataToRead, uint32 cOffset, EUGCReadAction eAction );
STEAMAPI_API int32 SteamAPI_ISteamRemoteStorage_GetCachedUGCCount( ISteamRemoteStorage* self );
STEAMAPI_API UGCHandle_t SteamAPI_ISteamRemoteStorage_GetCachedUGCHandle( ISteamRemoteStorage* self, int32 iCachedContent );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_PublishWorkshopFile( ISteamRemoteStorage* self, const char * pchFile, const char * pchPreviewFile, AppId_t nConsumerAppId, const char * pchTitle, const char * pchDescription, ERemoteStoragePublishedFileVisibility eVisibility, SteamParamStringArray_t * pTags, EWorkshopFileType eWorkshopFileType );
STEAMAPI_API PublishedFileUpdateHandle_t SteamAPI_ISteamRemoteStorage_CreatePublishedFileUpdateRequest( ISteamRemoteStorage* self, PublishedFileId_t unPublishedFileId );
STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_UpdatePublishedFileFile( ISteamRemoteStorage* self, PublishedFileUpdateHandle_t updateHandle, const char * pchFile );
STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_UpdatePublishedFilePreviewFile( ISteamRemoteStorage* self, PublishedFileUpdateHandle_t updateHandle, const char * pchPreviewFile );
STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_UpdatePublishedFileTitle( ISteamRemoteStorage* self, PublishedFileUpdateHandle_t updateHandle, const char * pchTitle );
STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_UpdatePublishedFileDescription( ISteamRemoteStorage* self, PublishedFileUpdateHandle_t updateHandle, const char * pchDescription );
STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_UpdatePublishedFileVisibility( ISteamRemoteStorage* self, PublishedFileUpdateHandle_t updateHandle, ERemoteStoragePublishedFileVisibility eVisibility );
STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_UpdatePublishedFileTags( ISteamRemoteStorage* self, PublishedFileUpdateHandle_t updateHandle, SteamParamStringArray_t * pTags );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_CommitPublishedFileUpdate( ISteamRemoteStorage* self, PublishedFileUpdateHandle_t updateHandle );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_GetPublishedFileDetails( ISteamRemoteStorage* self, PublishedFileId_t unPublishedFileId, uint32 unMaxSecondsOld );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_DeletePublishedFile( ISteamRemoteStorage* self, PublishedFileId_t unPublishedFileId );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_EnumerateUserPublishedFiles( ISteamRemoteStorage* self, uint32 unStartIndex );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_SubscribePublishedFile( ISteamRemoteStorage* self, PublishedFileId_t unPublishedFileId );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_EnumerateUserSubscribedFiles( ISteamRemoteStorage* self, uint32 unStartIndex );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_UnsubscribePublishedFile( ISteamRemoteStorage* self, PublishedFileId_t unPublishedFileId );
STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_UpdatePublishedFileSetChangeDescription( ISteamRemoteStorage* self, PublishedFileUpdateHandle_t updateHandle, const char * pchChangeDescription );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_GetPublishedItemVoteDetails( ISteamRemoteStorage* self, PublishedFileId_t unPublishedFileId );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_UpdateUserPublishedItemVote( ISteamRemoteStorage* self, PublishedFileId_t unPublishedFileId, bool bVoteUp );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_GetUserPublishedItemVoteDetails( ISteamRemoteStorage* self, PublishedFileId_t unPublishedFileId );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_EnumerateUserSharedWorkshopFiles( ISteamRemoteStorage* self, uint64_steamid steamId, uint32 unStartIndex, SteamParamStringArray_t * pRequiredTags, SteamParamStringArray_t * pExcludedTags );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_PublishVideo( ISteamRemoteStorage* self, EWorkshopVideoProvider eVideoProvider, const char * pchVideoAccount, const char * pchVideoIdentifier, const char * pchPreviewFile, AppId_t nConsumerAppId, const char * pchTitle, const char * pchDescription, ERemoteStoragePublishedFileVisibility eVisibility, SteamParamStringArray_t * pTags );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_SetUserPublishedFileAction( ISteamRemoteStorage* self, PublishedFileId_t unPublishedFileId, EWorkshopFileAction eAction );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_EnumeratePublishedFilesByUserAction( ISteamRemoteStorage* self, EWorkshopFileAction eAction, uint32 unStartIndex );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_EnumeratePublishedWorkshopFiles( ISteamRemoteStorage* self, EWorkshopEnumerationType eEnumerationType, uint32 unStartIndex, uint32 unCount, uint32 unDays, SteamParamStringArray_t * pTags, SteamParamStringArray_t * pUserTags );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamRemoteStorage_UGCDownloadToLocation( ISteamRemoteStorage* self, UGCHandle_t hContent, const char * pchLocation, uint32 unPriority );
STEAMAPI_API int32 SteamAPI_ISteamRemoteStorage_GetLocalFileChangeCount( ISteamRemoteStorage* self );
STEAMAPI_API const char * SteamAPI_ISteamRemoteStorage_GetLocalFileChange( ISteamRemoteStorage* self, int iFile, ERemoteStorageLocalFileChange * pEChangeType, ERemoteStorageFilePathType * pEFilePathType );
STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_BeginFileWriteBatch( ISteamRemoteStorage* self );
STEAMAPI_API steam_bool SteamAPI_ISteamRemoteStorage_EndFileWriteBatch( ISteamRemoteStorage* self );


// ISteamUserStats

STEAMAPI_API ISteamUserStats *SteamAPI_SteamUserStats_v013();
STEAMAPI_API ISteamUserStats *SteamAPI_SteamUserStats_v012();
STEAMAPI_API ISteamUserStats *SteamAPI_SteamUserStats_v011();
STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_RequestCurrentStats( ISteamUserStats* self );
STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_GetStatInt32( ISteamUserStats* self, const char * pchName, int32 * pData );
STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_GetStatFloat( ISteamUserStats* self, const char * pchName, float * pData );
STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_SetStatInt32( ISteamUserStats* self, const char * pchName, int32 nData );
STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_SetStatFloat( ISteamUserStats* self, const char * pchName, float fData );
STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_UpdateAvgRateStat( ISteamUserStats* self, const char * pchName, float flCountThisSession, double dSessionLength );
STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_GetAchievement( ISteamUserStats* self, const char * pchName, bool * pbAchieved );
STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_SetAchievement( ISteamUserStats* self, const char * pchName );
STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_ClearAchievement( ISteamUserStats* self, const char * pchName );
STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_GetAchievementAndUnlockTime( ISteamUserStats* self, const char * pchName, bool * pbAchieved, uint32 * punUnlockTime );
STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_StoreStats( ISteamUserStats* self );
STEAMAPI_API int SteamAPI_ISteamUserStats_GetAchievementIcon( ISteamUserStats* self, const char * pchName );
STEAMAPI_API const char * SteamAPI_ISteamUserStats_GetAchievementDisplayAttribute( ISteamUserStats* self, const char * pchName, const char * pchKey );
STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_IndicateAchievementProgress( ISteamUserStats* self, const char * pchName, uint32 nCurProgress, uint32 nMaxProgress );
STEAMAPI_API uint32 SteamAPI_ISteamUserStats_GetNumAchievements( ISteamUserStats* self );
STEAMAPI_API const char * SteamAPI_ISteamUserStats_GetAchievementName( ISteamUserStats* self, uint32 iAchievement );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUserStats_RequestUserStats( ISteamUserStats* self, uint64_steamid steamIDUser );
STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_GetUserStatInt32( ISteamUserStats* self, uint64_steamid steamIDUser, const char * pchName, int32 * pData );
STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_GetUserStatFloat( ISteamUserStats* self, uint64_steamid steamIDUser, const char * pchName, float * pData );
STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_GetUserAchievement( ISteamUserStats* self, uint64_steamid steamIDUser, const char * pchName, bool * pbAchieved );
STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_GetUserAchievementAndUnlockTime( ISteamUserStats* self, uint64_steamid steamIDUser, const char * pchName, bool * pbAchieved, uint32 * punUnlockTime );
STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_ResetAllStats( ISteamUserStats* self, bool bAchievementsToo );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUserStats_FindOrCreateLeaderboard( ISteamUserStats* self, const char * pchLeaderboardName, ELeaderboardSortMethod eLeaderboardSortMethod, ELeaderboardDisplayType eLeaderboardDisplayType );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUserStats_FindLeaderboard( ISteamUserStats* self, const char * pchLeaderboardName );
STEAMAPI_API const char * SteamAPI_ISteamUserStats_GetLeaderboardName( ISteamUserStats* self, SteamLeaderboard_t hSteamLeaderboard );
STEAMAPI_API int SteamAPI_ISteamUserStats_GetLeaderboardEntryCount( ISteamUserStats* self, SteamLeaderboard_t hSteamLeaderboard );
STEAMAPI_API ELeaderboardSortMethod SteamAPI_ISteamUserStats_GetLeaderboardSortMethod( ISteamUserStats* self, SteamLeaderboard_t hSteamLeaderboard );
STEAMAPI_API ELeaderboardDisplayType SteamAPI_ISteamUserStats_GetLeaderboardDisplayType( ISteamUserStats* self, SteamLeaderboard_t hSteamLeaderboard );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUserStats_DownloadLeaderboardEntries( ISteamUserStats* self, SteamLeaderboard_t hSteamLeaderboard, ELeaderboardDataRequest eLeaderboardDataRequest, int nRangeStart, int nRangeEnd );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUserStats_DownloadLeaderboardEntriesForUsers( ISteamUserStats* self, SteamLeaderboard_t hSteamLeaderboard, CSteamID * prgUsers, int cUsers );
STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_GetDownloadedLeaderboardEntry( ISteamUserStats* self, SteamLeaderboardEntries_t hSteamLeaderboardEntries, int index, LeaderboardEntry_t * pLeaderboardEntry, int32 * pDetails, int cDetailsMax );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUserStats_UploadLeaderboardScore( ISteamUserStats* self, SteamLeaderboard_t hSteamLeaderboard, ELeaderboardUploadScoreMethod eLeaderboardUploadScoreMethod, int32 nScore, const int32 * pScoreDetails, int cScoreDetailsCount );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUserStats_AttachLeaderboardUGC( ISteamUserStats* self, SteamLeaderboard_t hSteamLeaderboard, UGCHandle_t hUGC );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUserStats_GetNumberOfCurrentPlayers( ISteamUserStats* self );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUserStats_RequestGlobalAchievementPercentages( ISteamUserStats* self );
STEAMAPI_API int SteamAPI_ISteamUserStats_GetMostAchievedAchievementInfo( ISteamUserStats* self, char * pchName, uint32 unNameBufLen, float * pflPercent, bool * pbAchieved );
STEAMAPI_API int SteamAPI_ISteamUserStats_GetNextMostAchievedAchievementInfo( ISteamUserStats* self, int iIteratorPrevious, char * pchName, uint32 unNameBufLen, float * pflPercent, bool * pbAchieved );
STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_GetAchievementAchievedPercent( ISteamUserStats* self, const char * pchName, float * pflPercent );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUserStats_RequestGlobalStats( ISteamUserStats* self, int nHistoryDays );
STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_GetGlobalStatInt64( ISteamUserStats* self, const char * pchStatName, int64 * pData );
STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_GetGlobalStatDouble( ISteamUserStats* self, const char * pchStatName, double * pData );
STEAMAPI_API int32 SteamAPI_ISteamUserStats_GetGlobalStatHistoryInt64( ISteamUserStats* self, const char * pchStatName, int64 * pData, uint32 cubData );
STEAMAPI_API int32 SteamAPI_ISteamUserStats_GetGlobalStatHistoryDouble( ISteamUserStats* self, const char * pchStatName, double * pData, uint32 cubData );
STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_GetAchievementProgressLimitsInt32( ISteamUserStats* self, const char * pchName, int32 * pnMinProgress, int32 * pnMaxProgress );
STEAMAPI_API steam_bool SteamAPI_ISteamUserStats_GetAchievementProgressLimitsFloat( ISteamUserStats* self, const char * pchName, float * pfMinProgress, float * pfMaxProgress );

// ISteamApps
STEAMAPI_API ISteamApps *SteamAPI_SteamApps_v008();
STEAMAPI_API ISteamApps *SteamAPI_SteamGameServerApps_v008();
STEAMAPI_API steam_bool SteamAPI_ISteamApps_BIsSubscribed( ISteamApps* self );
STEAMAPI_API steam_bool SteamAPI_ISteamApps_BIsLowViolence( ISteamApps* self );
STEAMAPI_API steam_bool SteamAPI_ISteamApps_BIsCybercafe( ISteamApps* self );
STEAMAPI_API steam_bool SteamAPI_ISteamApps_BIsVACBanned( ISteamApps* self );
STEAMAPI_API const char * SteamAPI_ISteamApps_GetCurrentGameLanguage( ISteamApps* self );
STEAMAPI_API const char * SteamAPI_ISteamApps_GetAvailableGameLanguages( ISteamApps* self );
STEAMAPI_API steam_bool SteamAPI_ISteamApps_BIsSubscribedApp( ISteamApps* self, AppId_t appID );
STEAMAPI_API steam_bool SteamAPI_ISteamApps_BIsDlcInstalled( ISteamApps* self, AppId_t appID );
STEAMAPI_API uint32 SteamAPI_ISteamApps_GetEarliestPurchaseUnixTime( ISteamApps* self, AppId_t nAppID );
STEAMAPI_API steam_bool SteamAPI_ISteamApps_BIsSubscribedFromFreeWeekend( ISteamApps* self );
STEAMAPI_API int SteamAPI_ISteamApps_GetDLCCount( ISteamApps* self );
STEAMAPI_API steam_bool SteamAPI_ISteamApps_BGetDLCDataByIndex( ISteamApps* self, int iDLC, AppId_t * pAppID, bool * pbAvailable, char * pchName, int cchNameBufferSize );
STEAMAPI_API void SteamAPI_ISteamApps_InstallDLC( ISteamApps* self, AppId_t nAppID );
STEAMAPI_API void SteamAPI_ISteamApps_UninstallDLC( ISteamApps* self, AppId_t nAppID );
STEAMAPI_API void SteamAPI_ISteamApps_RequestAppProofOfPurchaseKey( ISteamApps* self, AppId_t nAppID );
STEAMAPI_API steam_bool SteamAPI_ISteamApps_GetCurrentBetaName( ISteamApps* self, char * pchName, int cchNameBufferSize );
STEAMAPI_API steam_bool SteamAPI_ISteamApps_MarkContentCorrupt( ISteamApps* self, bool bMissingFilesOnly );
STEAMAPI_API uint32 SteamAPI_ISteamApps_GetInstalledDepots( ISteamApps* self, AppId_t appID, DepotId_t * pvecDepots, uint32 cMaxDepots );
STEAMAPI_API uint32 SteamAPI_ISteamApps_GetAppInstallDir( ISteamApps* self, AppId_t appID, char * pchFolder, uint32 cchFolderBufferSize );
STEAMAPI_API steam_bool SteamAPI_ISteamApps_BIsAppInstalled( ISteamApps* self, AppId_t appID );
STEAMAPI_API uint64_steamid SteamAPI_ISteamApps_GetAppOwner( ISteamApps* self );
STEAMAPI_API const char * SteamAPI_ISteamApps_GetLaunchQueryParam( ISteamApps* self, const char * pchKey );
STEAMAPI_API steam_bool SteamAPI_ISteamApps_GetDlcDownloadProgress( ISteamApps* self, AppId_t nAppID, uint64 * punBytesDownloaded, uint64 * punBytesTotal );
STEAMAPI_API int SteamAPI_ISteamApps_GetAppBuildId( ISteamApps* self );
STEAMAPI_API void SteamAPI_ISteamApps_RequestAllProofOfPurchaseKeys( ISteamApps* self );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamApps_GetFileDetails( ISteamApps* self, const char * pszFileName );
STEAMAPI_API int SteamAPI_ISteamApps_GetLaunchCommandLine( ISteamApps* self, char * pszCommandLine, int cubCommandLine );
STEAMAPI_API steam_bool SteamAPI_ISteamApps_BIsSubscribedFromFamilySharing( ISteamApps* self );
STEAMAPI_API steam_bool SteamAPI_ISteamApps_BIsTimedTrial( ISteamApps* self, uint32 * punSecondsAllowed, uint32 * punSecondsPlayed );
STEAMAPI_API steam_bool SteamAPI_ISteamApps_SetDlcContext( ISteamApps* self, AppId_t nAppID );
STEAMAPI_API int SteamAPI_ISteamApps_GetNumBetas( ISteamApps* self, int * pnAvailable, int * pnPrivate );
STEAMAPI_API steam_bool SteamAPI_ISteamApps_GetBetaInfo( ISteamApps* self, int iBetaIndex, uint32 * punFlags, uint32 * punBuildID, char * pchBetaName, int cchBetaName, char * pchDescription, int cchDescription );
STEAMAPI_API steam_bool SteamAPI_ISteamApps_SetActiveBeta( ISteamApps* self, const char * pchBetaName );


// ISteamNetworking
STEAMAPI_API ISteamNetworking *SteamAPI_SteamNetworking_v006();
STEAMAPI_API ISteamNetworking *SteamAPI_SteamGameServerNetworking_v006();
STEAMAPI_API steam_bool SteamAPI_ISteamNetworking_SendP2PPacket( ISteamNetworking* self, uint64_steamid steamIDRemote, const void * pubData, uint32 cubData, EP2PSend eP2PSendType, int nChannel );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworking_IsP2PPacketAvailable( ISteamNetworking* self, uint32 * pcubMsgSize, int nChannel );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworking_ReadP2PPacket( ISteamNetworking* self, void * pubDest, uint32 cubDest, uint32 * pcubMsgSize, CSteamID * psteamIDRemote, int nChannel );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworking_AcceptP2PSessionWithUser( ISteamNetworking* self, uint64_steamid steamIDRemote );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworking_CloseP2PSessionWithUser( ISteamNetworking* self, uint64_steamid steamIDRemote );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworking_CloseP2PChannelWithUser( ISteamNetworking* self, uint64_steamid steamIDRemote, int nChannel );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworking_GetP2PSessionState( ISteamNetworking* self, uint64_steamid steamIDRemote, P2PSessionState_t * pConnectionState );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworking_AllowP2PPacketRelay( ISteamNetworking* self, bool bAllow );
STEAMAPI_API SNetListenSocket_t SteamAPI_ISteamNetworking_CreateListenSocket( ISteamNetworking* self, int nVirtualP2PPort, SteamIPAddress_t nIP, uint16 nPort, bool bAllowUseOfPacketRelay );
STEAMAPI_API SNetSocket_t SteamAPI_ISteamNetworking_CreateP2PConnectionSocket( ISteamNetworking* self, uint64_steamid steamIDTarget, int nVirtualPort, int nTimeoutSec, bool bAllowUseOfPacketRelay );
STEAMAPI_API SNetSocket_t SteamAPI_ISteamNetworking_CreateConnectionSocket( ISteamNetworking* self, SteamIPAddress_t nIP, uint16 nPort, int nTimeoutSec );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworking_DestroySocket( ISteamNetworking* self, SNetSocket_t hSocket, bool bNotifyRemoteEnd );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworking_DestroyListenSocket( ISteamNetworking* self, SNetListenSocket_t hSocket, bool bNotifyRemoteEnd );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworking_SendDataOnSocket( ISteamNetworking* self, SNetSocket_t hSocket, void * pubData, uint32 cubData, bool bReliable );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworking_IsDataAvailableOnSocket( ISteamNetworking* self, SNetSocket_t hSocket, uint32 * pcubMsgSize );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworking_RetrieveDataFromSocket( ISteamNetworking* self, SNetSocket_t hSocket, void * pubDest, uint32 cubDest, uint32 * pcubMsgSize );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworking_IsDataAvailable( ISteamNetworking* self, SNetListenSocket_t hListenSocket, uint32 * pcubMsgSize, SNetSocket_t * phSocket );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworking_RetrieveData( ISteamNetworking* self, SNetListenSocket_t hListenSocket, void * pubDest, uint32 cubDest, uint32 * pcubMsgSize, SNetSocket_t * phSocket );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworking_GetSocketInfo( ISteamNetworking* self, SNetSocket_t hSocket, CSteamID * pSteamIDRemote, int * peSocketStatus, SteamIPAddress_t * punIPRemote, uint16 * punPortRemote );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworking_GetListenSocketInfo( ISteamNetworking* self, SNetListenSocket_t hListenSocket, SteamIPAddress_t * pnIP, uint16 * pnPort );
STEAMAPI_API ESNetSocketConnectionType SteamAPI_ISteamNetworking_GetSocketConnectionType( ISteamNetworking* self, SNetSocket_t hSocket );
STEAMAPI_API int SteamAPI_ISteamNetworking_GetMaxPacketSize( ISteamNetworking* self, SNetSocket_t hSocket );

// ISteamScreenshots
STEAMAPI_API ISteamScreenshots *SteamAPI_SteamScreenshots_v003();
STEAMAPI_API ScreenshotHandle SteamAPI_ISteamScreenshots_WriteScreenshot( ISteamScreenshots* self, void * pubRGB, uint32 cubRGB, int nWidth, int nHeight );
STEAMAPI_API ScreenshotHandle SteamAPI_ISteamScreenshots_AddScreenshotToLibrary( ISteamScreenshots* self, const char * pchFilename, const char * pchThumbnailFilename, int nWidth, int nHeight );
STEAMAPI_API void SteamAPI_ISteamScreenshots_TriggerScreenshot( ISteamScreenshots* self );
STEAMAPI_API void SteamAPI_ISteamScreenshots_HookScreenshots( ISteamScreenshots* self, bool bHook );
STEAMAPI_API steam_bool SteamAPI_ISteamScreenshots_SetLocation( ISteamScreenshots* self, ScreenshotHandle hScreenshot, const char * pchLocation );
STEAMAPI_API steam_bool SteamAPI_ISteamScreenshots_TagUser( ISteamScreenshots* self, ScreenshotHandle hScreenshot, uint64_steamid steamID );
STEAMAPI_API steam_bool SteamAPI_ISteamScreenshots_TagPublishedFile( ISteamScreenshots* self, ScreenshotHandle hScreenshot, PublishedFileId_t unPublishedFileID );
STEAMAPI_API steam_bool SteamAPI_ISteamScreenshots_IsScreenshotsHooked( ISteamScreenshots* self );
STEAMAPI_API ScreenshotHandle SteamAPI_ISteamScreenshots_AddVRScreenshotToLibrary( ISteamScreenshots* self, EVRScreenshotType eType, const char * pchFilename, const char * pchVRFilename );

// ISteamMusic
STEAMAPI_API ISteamMusic *SteamAPI_SteamMusic_v001();
STEAMAPI_API steam_bool SteamAPI_ISteamMusic_BIsEnabled( ISteamMusic* self );
STEAMAPI_API steam_bool SteamAPI_ISteamMusic_BIsPlaying( ISteamMusic* self );
STEAMAPI_API AudioPlayback_Status SteamAPI_ISteamMusic_GetPlaybackStatus( ISteamMusic* self );
STEAMAPI_API void SteamAPI_ISteamMusic_Play( ISteamMusic* self );
STEAMAPI_API void SteamAPI_ISteamMusic_Pause( ISteamMusic* self );
STEAMAPI_API void SteamAPI_ISteamMusic_PlayPrevious( ISteamMusic* self );
STEAMAPI_API void SteamAPI_ISteamMusic_PlayNext( ISteamMusic* self );
STEAMAPI_API void SteamAPI_ISteamMusic_SetVolume( ISteamMusic* self, float flVolume );
STEAMAPI_API float SteamAPI_ISteamMusic_GetVolume( ISteamMusic* self );

// ISteamMusicRemote
STEAMAPI_API ISteamMusicRemote *SteamAPI_SteamMusicRemote_v001();
STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_RegisterSteamMusicRemote( ISteamMusicRemote* self, const char * pchName );
STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_DeregisterSteamMusicRemote( ISteamMusicRemote* self );
STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_BIsCurrentMusicRemote( ISteamMusicRemote* self );
STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_BActivationSuccess( ISteamMusicRemote* self, bool bValue );
STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_SetDisplayName( ISteamMusicRemote* self, const char * pchDisplayName );
STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_SetPNGIcon_64x64( ISteamMusicRemote* self, void * pvBuffer, uint32 cbBufferLength );
STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_EnablePlayPrevious( ISteamMusicRemote* self, bool bValue );
STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_EnablePlayNext( ISteamMusicRemote* self, bool bValue );
STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_EnableShuffled( ISteamMusicRemote* self, bool bValue );
STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_EnableLooped( ISteamMusicRemote* self, bool bValue );
STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_EnableQueue( ISteamMusicRemote* self, bool bValue );
STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_EnablePlaylists( ISteamMusicRemote* self, bool bValue );
STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_UpdatePlaybackStatus( ISteamMusicRemote* self, AudioPlayback_Status nStatus );
STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_UpdateShuffled( ISteamMusicRemote* self, bool bValue );
STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_UpdateLooped( ISteamMusicRemote* self, bool bValue );
STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_UpdateVolume( ISteamMusicRemote* self, float flValue );
STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_CurrentEntryWillChange( ISteamMusicRemote* self );
STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_CurrentEntryIsAvailable( ISteamMusicRemote* self, bool bAvailable );
STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_UpdateCurrentEntryText( ISteamMusicRemote* self, const char * pchText );
STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_UpdateCurrentEntryElapsedSeconds( ISteamMusicRemote* self, int nValue );
STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_UpdateCurrentEntryCoverArt( ISteamMusicRemote* self, void * pvBuffer, uint32 cbBufferLength );
STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_CurrentEntryDidChange( ISteamMusicRemote* self );
STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_QueueWillChange( ISteamMusicRemote* self );
STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_ResetQueueEntries( ISteamMusicRemote* self );
STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_SetQueueEntry( ISteamMusicRemote* self, int nID, int nPosition, const char * pchEntryText );
STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_SetCurrentQueueEntry( ISteamMusicRemote* self, int nID );
STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_QueueDidChange( ISteamMusicRemote* self );
STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_PlaylistWillChange( ISteamMusicRemote* self );
STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_ResetPlaylistEntries( ISteamMusicRemote* self );
STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_SetPlaylistEntry( ISteamMusicRemote* self, int nID, int nPosition, const char * pchEntryText );
STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_SetCurrentPlaylistEntry( ISteamMusicRemote* self, int nID );
STEAMAPI_API steam_bool SteamAPI_ISteamMusicRemote_PlaylistDidChange( ISteamMusicRemote* self );

// ISteamHTTP
STEAMAPI_API ISteamHTTP *SteamAPI_SteamHTTP_v003();
STEAMAPI_API ISteamHTTP *SteamAPI_SteamGameServerHTTP_v003();
STEAMAPI_API HTTPRequestHandle SteamAPI_ISteamHTTP_CreateHTTPRequest( ISteamHTTP* self, EHTTPMethod eHTTPRequestMethod, const char * pchAbsoluteURL );
STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_SetHTTPRequestContextValue( ISteamHTTP* self, HTTPRequestHandle hRequest, uint64 ulContextValue );
STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_SetHTTPRequestNetworkActivityTimeout( ISteamHTTP* self, HTTPRequestHandle hRequest, uint32 unTimeoutSeconds );
STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_SetHTTPRequestHeaderValue( ISteamHTTP* self, HTTPRequestHandle hRequest, const char * pchHeaderName, const char * pchHeaderValue );
STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_SetHTTPRequestGetOrPostParameter( ISteamHTTP* self, HTTPRequestHandle hRequest, const char * pchParamName, const char * pchParamValue );
STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_SendHTTPRequest( ISteamHTTP* self, HTTPRequestHandle hRequest, SteamAPICall_t * pCallHandle );
STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_SendHTTPRequestAndStreamResponse( ISteamHTTP* self, HTTPRequestHandle hRequest, SteamAPICall_t * pCallHandle );
STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_DeferHTTPRequest( ISteamHTTP* self, HTTPRequestHandle hRequest );
STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_PrioritizeHTTPRequest( ISteamHTTP* self, HTTPRequestHandle hRequest );
STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_GetHTTPResponseHeaderSize( ISteamHTTP* self, HTTPRequestHandle hRequest, const char * pchHeaderName, uint32 * unResponseHeaderSize );
STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_GetHTTPResponseHeaderValue( ISteamHTTP* self, HTTPRequestHandle hRequest, const char * pchHeaderName, uint8 * pHeaderValueBuffer, uint32 unBufferSize );
STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_GetHTTPResponseBodySize( ISteamHTTP* self, HTTPRequestHandle hRequest, uint32 * unBodySize );
STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_GetHTTPResponseBodyData( ISteamHTTP* self, HTTPRequestHandle hRequest, uint8 * pBodyDataBuffer, uint32 unBufferSize );
STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_GetHTTPStreamingResponseBodyData( ISteamHTTP* self, HTTPRequestHandle hRequest, uint32 cOffset, uint8 * pBodyDataBuffer, uint32 unBufferSize );
STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_ReleaseHTTPRequest( ISteamHTTP* self, HTTPRequestHandle hRequest );
STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_GetHTTPDownloadProgressPct( ISteamHTTP* self, HTTPRequestHandle hRequest, float * pflPercentOut );
STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_SetHTTPRequestRawPostBody( ISteamHTTP* self, HTTPRequestHandle hRequest, const char * pchContentType, uint8 * pubBody, uint32 unBodyLen );
STEAMAPI_API HTTPCookieContainerHandle SteamAPI_ISteamHTTP_CreateCookieContainer( ISteamHTTP* self, bool bAllowResponsesToModify );
STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_ReleaseCookieContainer( ISteamHTTP* self, HTTPCookieContainerHandle hCookieContainer );
STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_SetCookie( ISteamHTTP* self, HTTPCookieContainerHandle hCookieContainer, const char * pchHost, const char * pchUrl, const char * pchCookie );
STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_SetHTTPRequestCookieContainer( ISteamHTTP* self, HTTPRequestHandle hRequest, HTTPCookieContainerHandle hCookieContainer );
STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_SetHTTPRequestUserAgentInfo( ISteamHTTP* self, HTTPRequestHandle hRequest, const char * pchUserAgentInfo );
STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_SetHTTPRequestRequiresVerifiedCertificate( ISteamHTTP* self, HTTPRequestHandle hRequest, bool bRequireVerifiedCertificate );
STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_SetHTTPRequestAbsoluteTimeoutMS( ISteamHTTP* self, HTTPRequestHandle hRequest, uint32 unMilliseconds );
STEAMAPI_API steam_bool SteamAPI_ISteamHTTP_GetHTTPRequestWasTimedOut( ISteamHTTP* self, HTTPRequestHandle hRequest, bool * pbWasTimedOut );

// ISteamInput
STEAMAPI_API ISteamInput *SteamAPI_SteamInput_v001();
STEAMAPI_API ISteamInput *SteamAPI_SteamInput_v002();
STEAMAPI_API ISteamInput *SteamAPI_SteamInput_v005();
STEAMAPI_API ISteamInput *SteamAPI_SteamInput_v006();
STEAMAPI_API steam_bool SteamAPI_ISteamInput_Init( ISteamInput* self, bool bExplicitlyCallRunFrame );
STEAMAPI_API steam_bool SteamAPI_ISteamInput_Shutdown( ISteamInput* self );
STEAMAPI_API steam_bool SteamAPI_ISteamInput_SetInputActionManifestFilePath( ISteamInput* self, const char * pchInputActionManifestAbsolutePath );
STEAMAPI_API void SteamAPI_ISteamInput_RunFrame( ISteamInput* self, bool bReservedValue );
STEAMAPI_API steam_bool SteamAPI_ISteamInput_BWaitForData( ISteamInput* self, bool bWaitForever, uint32 unTimeout );
STEAMAPI_API steam_bool SteamAPI_ISteamInput_BNewDataAvailable( ISteamInput* self );
STEAMAPI_API int SteamAPI_ISteamInput_GetConnectedControllers( ISteamInput* self, InputHandle_t * handlesOut );
STEAMAPI_API void SteamAPI_ISteamInput_EnableDeviceCallbacks( ISteamInput* self );
STEAMAPI_API void SteamAPI_ISteamInput_EnableActionEventCallbacks( ISteamInput* self, SteamInputActionEventCallbackPointer pCallback );
STEAMAPI_API InputActionSetHandle_t SteamAPI_ISteamInput_GetActionSetHandle( ISteamInput* self, const char * pszActionSetName );
STEAMAPI_API void SteamAPI_ISteamInput_ActivateActionSet( ISteamInput* self, InputHandle_t inputHandle, InputActionSetHandle_t actionSetHandle );
STEAMAPI_API InputActionSetHandle_t SteamAPI_ISteamInput_GetCurrentActionSet( ISteamInput* self, InputHandle_t inputHandle );
STEAMAPI_API void SteamAPI_ISteamInput_ActivateActionSetLayer( ISteamInput* self, InputHandle_t inputHandle, InputActionSetHandle_t actionSetLayerHandle );
STEAMAPI_API void SteamAPI_ISteamInput_DeactivateActionSetLayer( ISteamInput* self, InputHandle_t inputHandle, InputActionSetHandle_t actionSetLayerHandle );
STEAMAPI_API void SteamAPI_ISteamInput_DeactivateAllActionSetLayers( ISteamInput* self, InputHandle_t inputHandle );
STEAMAPI_API int SteamAPI_ISteamInput_GetActiveActionSetLayers( ISteamInput* self, InputHandle_t inputHandle, InputActionSetHandle_t * handlesOut );
STEAMAPI_API InputDigitalActionHandle_t SteamAPI_ISteamInput_GetDigitalActionHandle( ISteamInput* self, const char * pszActionName );
STEAMAPI_API InputDigitalActionData_t SteamAPI_ISteamInput_GetDigitalActionData( ISteamInput* self, InputHandle_t inputHandle, InputDigitalActionHandle_t digitalActionHandle );
STEAMAPI_API int SteamAPI_ISteamInput_GetDigitalActionOrigins( ISteamInput* self, InputHandle_t inputHandle, InputActionSetHandle_t actionSetHandle, InputDigitalActionHandle_t digitalActionHandle, EInputActionOrigin * originsOut );
STEAMAPI_API const char * SteamAPI_ISteamInput_GetStringForDigitalActionName( ISteamInput* self, InputDigitalActionHandle_t eActionHandle );
STEAMAPI_API InputAnalogActionHandle_t SteamAPI_ISteamInput_GetAnalogActionHandle( ISteamInput* self, const char * pszActionName );
STEAMAPI_API InputAnalogActionData_t SteamAPI_ISteamInput_GetAnalogActionData( ISteamInput* self, InputHandle_t inputHandle, InputAnalogActionHandle_t analogActionHandle );
STEAMAPI_API int SteamAPI_ISteamInput_GetAnalogActionOrigins( ISteamInput* self, InputHandle_t inputHandle, InputActionSetHandle_t actionSetHandle, InputAnalogActionHandle_t analogActionHandle, EInputActionOrigin * originsOut );
STEAMAPI_API const char * SteamAPI_ISteamInput_GetGlyphPNGForActionOrigin( ISteamInput* self, EInputActionOrigin eOrigin, ESteamInputGlyphSize eSize, uint32 unFlags );
STEAMAPI_API const char * SteamAPI_ISteamInput_GetGlyphForActionOrigin( ISteamInput* self, EInputActionOrigin eOrigin );
STEAMAPI_API const char * SteamAPI_ISteamInput_GetGlyphSVGForActionOrigin( ISteamInput* self, EInputActionOrigin eOrigin, uint32 unFlags );
STEAMAPI_API const char * SteamAPI_ISteamInput_GetGlyphForActionOrigin_Legacy( ISteamInput* self, EInputActionOrigin eOrigin );
STEAMAPI_API const char * SteamAPI_ISteamInput_GetStringForActionOrigin( ISteamInput* self, EInputActionOrigin eOrigin );
STEAMAPI_API const char * SteamAPI_ISteamInput_GetStringForAnalogActionName( ISteamInput* self, InputAnalogActionHandle_t eActionHandle );
STEAMAPI_API void SteamAPI_ISteamInput_StopAnalogActionMomentum( ISteamInput* self, InputHandle_t inputHandle, InputAnalogActionHandle_t eAction );
STEAMAPI_API InputMotionData_t SteamAPI_ISteamInput_GetMotionData( ISteamInput* self, InputHandle_t inputHandle );
STEAMAPI_API void SteamAPI_ISteamInput_TriggerVibration( ISteamInput* self, InputHandle_t inputHandle, unsigned short usLeftSpeed, unsigned short usRightSpeed );
STEAMAPI_API void SteamAPI_ISteamInput_TriggerVibrationExtended( ISteamInput* self, InputHandle_t inputHandle, unsigned short usLeftSpeed, unsigned short usRightSpeed, unsigned short usLeftTriggerSpeed, unsigned short usRightTriggerSpeed );
STEAMAPI_API void SteamAPI_ISteamInput_TriggerSimpleHapticEvent( ISteamInput* self, InputHandle_t inputHandle, EControllerHapticLocation eHapticLocation, uint8 nIntensity, char nGainDB, uint8 nOtherIntensity, char nOtherGainDB );
STEAMAPI_API void SteamAPI_ISteamInput_SetLEDColor( ISteamInput* self, InputHandle_t inputHandle, uint8 nColorR, uint8 nColorG, uint8 nColorB, unsigned int nFlags );
STEAMAPI_API void SteamAPI_ISteamInput_TriggerHapticPulse( ISteamInput* self, InputHandle_t inputHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec );
STEAMAPI_API void SteamAPI_ISteamInput_TriggerRepeatedHapticPulse( ISteamInput* self, InputHandle_t inputHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec, unsigned short usOffMicroSec, unsigned short unRepeat, unsigned int nFlags );
STEAMAPI_API void SteamAPI_ISteamInput_Legacy_TriggerHapticPulse( ISteamInput* self, InputHandle_t inputHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec );
STEAMAPI_API void SteamAPI_ISteamInput_Legacy_TriggerRepeatedHapticPulse( ISteamInput* self, InputHandle_t inputHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec, unsigned short usOffMicroSec, unsigned short unRepeat, unsigned int nFlags );
STEAMAPI_API steam_bool SteamAPI_ISteamInput_ShowBindingPanel( ISteamInput* self, InputHandle_t inputHandle );
STEAMAPI_API ESteamInputType SteamAPI_ISteamInput_GetInputTypeForHandle( ISteamInput* self, InputHandle_t inputHandle );
STEAMAPI_API InputHandle_t SteamAPI_ISteamInput_GetControllerForGamepadIndex( ISteamInput* self, int nIndex );
STEAMAPI_API int SteamAPI_ISteamInput_GetGamepadIndexForController( ISteamInput* self, InputHandle_t ulinputHandle );
STEAMAPI_API const char * SteamAPI_ISteamInput_GetStringForXboxOrigin( ISteamInput* self, EXboxOrigin eOrigin );
STEAMAPI_API const char * SteamAPI_ISteamInput_GetGlyphForXboxOrigin( ISteamInput* self, EXboxOrigin eOrigin );
STEAMAPI_API EInputActionOrigin SteamAPI_ISteamInput_GetActionOriginFromXboxOrigin( ISteamInput* self, InputHandle_t inputHandle, EXboxOrigin eOrigin );
STEAMAPI_API EInputActionOrigin SteamAPI_ISteamInput_TranslateActionOrigin( ISteamInput* self, ESteamInputType eDestinationInputType, EInputActionOrigin eSourceOrigin );
STEAMAPI_API steam_bool SteamAPI_ISteamInput_GetDeviceBindingRevision( ISteamInput* self, InputHandle_t inputHandle, int * pMajor, int * pMinor );
STEAMAPI_API uint32 SteamAPI_ISteamInput_GetRemotePlaySessionID( ISteamInput* self, InputHandle_t inputHandle );
STEAMAPI_API uint16 SteamAPI_ISteamInput_GetSessionInputConfigurationSettings( ISteamInput* self );
STEAMAPI_API void SteamAPI_ISteamInput_SetDualSenseTriggerEffect( ISteamInput* self, InputHandle_t inputHandle, const ScePadTriggerEffectParam * pParam );

// ISteamController
STEAMAPI_API ISteamController *SteamAPI_SteamController_v007();
STEAMAPI_API ISteamController *SteamAPI_SteamController_v008();
STEAMAPI_API steam_bool SteamAPI_ISteamController_Init( ISteamController* self );
STEAMAPI_API steam_bool SteamAPI_ISteamController_Shutdown( ISteamController* self );
STEAMAPI_API void SteamAPI_ISteamController_RunFrame( ISteamController* self );
STEAMAPI_API int SteamAPI_ISteamController_GetConnectedControllers( ISteamController* self, ControllerHandle_t * handlesOut );
STEAMAPI_API ControllerActionSetHandle_t SteamAPI_ISteamController_GetActionSetHandle( ISteamController* self, const char * pszActionSetName );
STEAMAPI_API void SteamAPI_ISteamController_ActivateActionSet( ISteamController* self, ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetHandle );
STEAMAPI_API ControllerActionSetHandle_t SteamAPI_ISteamController_GetCurrentActionSet( ISteamController* self, ControllerHandle_t controllerHandle );
STEAMAPI_API void SteamAPI_ISteamController_ActivateActionSetLayer( ISteamController* self, ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetLayerHandle );
STEAMAPI_API void SteamAPI_ISteamController_DeactivateActionSetLayer( ISteamController* self, ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetLayerHandle );
STEAMAPI_API void SteamAPI_ISteamController_DeactivateAllActionSetLayers( ISteamController* self, ControllerHandle_t controllerHandle );
STEAMAPI_API int SteamAPI_ISteamController_GetActiveActionSetLayers( ISteamController* self, ControllerHandle_t controllerHandle, ControllerActionSetHandle_t * handlesOut );
STEAMAPI_API ControllerDigitalActionHandle_t SteamAPI_ISteamController_GetDigitalActionHandle( ISteamController* self, const char * pszActionName );
STEAMAPI_API InputDigitalActionData_t SteamAPI_ISteamController_GetDigitalActionData( ISteamController* self, ControllerHandle_t controllerHandle, ControllerDigitalActionHandle_t digitalActionHandle );
STEAMAPI_API int SteamAPI_ISteamController_GetDigitalActionOrigins( ISteamController* self, ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetHandle, ControllerDigitalActionHandle_t digitalActionHandle, EControllerActionOrigin * originsOut );
STEAMAPI_API ControllerAnalogActionHandle_t SteamAPI_ISteamController_GetAnalogActionHandle( ISteamController* self, const char * pszActionName );
STEAMAPI_API InputAnalogActionData_t SteamAPI_ISteamController_GetAnalogActionData( ISteamController* self, ControllerHandle_t controllerHandle, ControllerAnalogActionHandle_t analogActionHandle );
STEAMAPI_API int SteamAPI_ISteamController_GetAnalogActionOrigins( ISteamController* self, ControllerHandle_t controllerHandle, ControllerActionSetHandle_t actionSetHandle, ControllerAnalogActionHandle_t analogActionHandle, EControllerActionOrigin * originsOut );
STEAMAPI_API const char * SteamAPI_ISteamController_GetGlyphForActionOrigin( ISteamController* self, EControllerActionOrigin eOrigin );
STEAMAPI_API const char * SteamAPI_ISteamController_GetStringForActionOrigin( ISteamController* self, EControllerActionOrigin eOrigin );
STEAMAPI_API void SteamAPI_ISteamController_StopAnalogActionMomentum( ISteamController* self, ControllerHandle_t controllerHandle, ControllerAnalogActionHandle_t eAction );
STEAMAPI_API InputMotionData_t SteamAPI_ISteamController_GetMotionData( ISteamController* self, ControllerHandle_t controllerHandle );
STEAMAPI_API void SteamAPI_ISteamController_TriggerHapticPulse( ISteamController* self, ControllerHandle_t controllerHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec );
STEAMAPI_API void SteamAPI_ISteamController_TriggerRepeatedHapticPulse( ISteamController* self, ControllerHandle_t controllerHandle, ESteamControllerPad eTargetPad, unsigned short usDurationMicroSec, unsigned short usOffMicroSec, unsigned short unRepeat, unsigned int nFlags );
STEAMAPI_API void SteamAPI_ISteamController_TriggerVibration( ISteamController* self, ControllerHandle_t controllerHandle, unsigned short usLeftSpeed, unsigned short usRightSpeed );
STEAMAPI_API void SteamAPI_ISteamController_SetLEDColor( ISteamController* self, ControllerHandle_t controllerHandle, uint8 nColorR, uint8 nColorG, uint8 nColorB, unsigned int nFlags );
STEAMAPI_API steam_bool SteamAPI_ISteamController_ShowBindingPanel( ISteamController* self, ControllerHandle_t controllerHandle );
STEAMAPI_API ESteamInputType SteamAPI_ISteamController_GetInputTypeForHandle( ISteamController* self, ControllerHandle_t controllerHandle );
STEAMAPI_API ControllerHandle_t SteamAPI_ISteamController_GetControllerForGamepadIndex( ISteamController* self, int nIndex );
STEAMAPI_API int SteamAPI_ISteamController_GetGamepadIndexForController( ISteamController* self, ControllerHandle_t ulControllerHandle );
STEAMAPI_API const char * SteamAPI_ISteamController_GetStringForXboxOrigin( ISteamController* self, EXboxOrigin eOrigin );
STEAMAPI_API const char * SteamAPI_ISteamController_GetGlyphForXboxOrigin( ISteamController* self, EXboxOrigin eOrigin );
STEAMAPI_API EControllerActionOrigin SteamAPI_ISteamController_GetActionOriginFromXboxOrigin( ISteamController* self, ControllerHandle_t controllerHandle, EXboxOrigin eOrigin );
STEAMAPI_API EControllerActionOrigin SteamAPI_ISteamController_TranslateActionOrigin( ISteamController* self, ESteamInputType eDestinationInputType, EControllerActionOrigin eSourceOrigin );
STEAMAPI_API steam_bool SteamAPI_ISteamController_GetControllerBindingRevision( ISteamController* self, ControllerHandle_t controllerHandle, int * pMajor, int * pMinor );

// ISteamUGC
STEAMAPI_API ISteamUGC *SteamAPI_SteamUGC_v014();
STEAMAPI_API ISteamUGC *SteamAPI_SteamUGC_v015();
STEAMAPI_API ISteamUGC *SteamAPI_SteamUGC_v016();
STEAMAPI_API ISteamUGC *SteamAPI_SteamUGC_v017();
STEAMAPI_API ISteamUGC *SteamAPI_SteamUGC_v018();
STEAMAPI_API ISteamUGC *SteamAPI_SteamUGC_v020();
STEAMAPI_API ISteamUGC *SteamAPI_SteamGameServerUGC_v014();
STEAMAPI_API ISteamUGC *SteamAPI_SteamGameServerUGC_v015();
STEAMAPI_API ISteamUGC *SteamAPI_SteamGameServerUGC_v016();
STEAMAPI_API ISteamUGC *SteamAPI_SteamGameServerUGC_v017();
STEAMAPI_API ISteamUGC *SteamAPI_SteamGameServerUGC_v018();
STEAMAPI_API ISteamUGC *SteamAPI_SteamGameServerUGC_v020();
STEAMAPI_API UGCQueryHandle_t SteamAPI_ISteamUGC_CreateQueryUserUGCRequest( ISteamUGC* self, AccountID_t unAccountID, EUserUGCList eListType, EUGCMatchingUGCType eMatchingUGCType, EUserUGCListSortOrder eSortOrder, AppId_t nCreatorAppID, AppId_t nConsumerAppID, uint32 unPage );
STEAMAPI_API UGCQueryHandle_t SteamAPI_ISteamUGC_CreateQueryAllUGCRequestPage( ISteamUGC* self, EUGCQuery eQueryType, EUGCMatchingUGCType eMatchingeMatchingUGCTypeFileType, AppId_t nCreatorAppID, AppId_t nConsumerAppID, uint32 unPage );
STEAMAPI_API UGCQueryHandle_t SteamAPI_ISteamUGC_CreateQueryAllUGCRequestCursor( ISteamUGC* self, EUGCQuery eQueryType, EUGCMatchingUGCType eMatchingeMatchingUGCTypeFileType, AppId_t nCreatorAppID, AppId_t nConsumerAppID, const char * pchCursor );
STEAMAPI_API UGCQueryHandle_t SteamAPI_ISteamUGC_CreateQueryUGCDetailsRequest( ISteamUGC* self, PublishedFileId_t * pvecPublishedFileID, uint32 unNumPublishedFileIDs );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_SendQueryUGCRequest( ISteamUGC* self, UGCQueryHandle_t handle );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_GetQueryUGCResult( ISteamUGC* self, UGCQueryHandle_t handle, uint32 index, SteamUGCDetails_t * pDetails );
STEAMAPI_API uint32 SteamAPI_ISteamUGC_GetQueryUGCNumTags( ISteamUGC* self, UGCQueryHandle_t handle, uint32 index );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_GetQueryUGCTag( ISteamUGC* self, UGCQueryHandle_t handle, uint32 index, uint32 indexTag, char * pchValue, uint32 cchValueSize );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_GetQueryUGCTagDisplayName( ISteamUGC* self, UGCQueryHandle_t handle, uint32 index, uint32 indexTag, char * pchValue, uint32 cchValueSize );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_GetQueryUGCPreviewURL( ISteamUGC* self, UGCQueryHandle_t handle, uint32 index, char * pchURL, uint32 cchURLSize );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_GetQueryUGCMetadata( ISteamUGC* self, UGCQueryHandle_t handle, uint32 index, char * pchMetadata, uint32 cchMetadatasize );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_GetQueryUGCChildren( ISteamUGC* self, UGCQueryHandle_t handle, uint32 index, PublishedFileId_t * pvecPublishedFileID, uint32 cMaxEntries );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_GetQueryUGCStatistic( ISteamUGC* self, UGCQueryHandle_t handle, uint32 index, EItemStatistic eStatType, uint64 * pStatValue );
STEAMAPI_API uint32 SteamAPI_ISteamUGC_GetQueryUGCNumAdditionalPreviews( ISteamUGC* self, UGCQueryHandle_t handle, uint32 index );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_GetQueryUGCAdditionalPreview( ISteamUGC* self, UGCQueryHandle_t handle, uint32 index, uint32 previewIndex, char * pchURLOrVideoID, uint32 cchURLSize, char * pchOriginalFileName, uint32 cchOriginalFileNameSize, EItemPreviewType * pPreviewType );
STEAMAPI_API uint32 SteamAPI_ISteamUGC_GetQueryUGCNumKeyValueTags( ISteamUGC* self, UGCQueryHandle_t handle, uint32 index );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_GetQueryUGCKeyValueTag( ISteamUGC* self, UGCQueryHandle_t handle, uint32 index, uint32 keyValueTagIndex, char * pchKey, uint32 cchKeySize, char * pchValue, uint32 cchValueSize );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_GetQueryFirstUGCKeyValueTag( ISteamUGC* self, UGCQueryHandle_t handle, uint32 index, const char * pchKey, char * pchValue, uint32 cchValueSize );
STEAMAPI_API uint32 SteamAPI_ISteamUGC_GetNumSupportedGameVersions( ISteamUGC* self, UGCQueryHandle_t handle, uint32 index );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_GetSupportedGameVersionData( ISteamUGC* self, UGCQueryHandle_t handle, uint32 index, uint32 versionIndex, char * pchGameBranchMin, char * pchGameBranchMax, uint32 cchGameBranchSize );
STEAMAPI_API uint32 SteamAPI_ISteamUGC_GetQueryUGCContentDescriptors( ISteamUGC* self, UGCQueryHandle_t handle, uint32 index, EUGCContentDescriptorID * pvecDescriptors, uint32 cMaxEntries );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_ReleaseQueryUGCRequest( ISteamUGC* self, UGCQueryHandle_t handle );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_AddRequiredTag( ISteamUGC* self, UGCQueryHandle_t handle, const char * pTagName );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_AddRequiredTagGroup( ISteamUGC* self, UGCQueryHandle_t handle, const SteamParamStringArray_t * pTagGroups );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_AddExcludedTag( ISteamUGC* self, UGCQueryHandle_t handle, const char * pTagName );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetReturnOnlyIDs( ISteamUGC* self, UGCQueryHandle_t handle, bool bReturnOnlyIDs );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetReturnKeyValueTags( ISteamUGC* self, UGCQueryHandle_t handle, bool bReturnKeyValueTags );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetReturnLongDescription( ISteamUGC* self, UGCQueryHandle_t handle, bool bReturnLongDescription );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetReturnMetadata( ISteamUGC* self, UGCQueryHandle_t handle, bool bReturnMetadata );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetReturnChildren( ISteamUGC* self, UGCQueryHandle_t handle, bool bReturnChildren );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetReturnAdditionalPreviews( ISteamUGC* self, UGCQueryHandle_t handle, bool bReturnAdditionalPreviews );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetReturnTotalOnly( ISteamUGC* self, UGCQueryHandle_t handle, bool bReturnTotalOnly );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetReturnPlaytimeStats( ISteamUGC* self, UGCQueryHandle_t handle, uint32 unDays );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetLanguage( ISteamUGC* self, UGCQueryHandle_t handle, const char * pchLanguage );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetAllowCachedResponse( ISteamUGC* self, UGCQueryHandle_t handle, uint32 unMaxAgeSeconds );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetAdminQuery( ISteamUGC* self, UGCUpdateHandle_t handle, bool bAdminQuery );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetCloudFileNameFilter( ISteamUGC* self, UGCQueryHandle_t handle, const char * pMatchCloudFileName );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetMatchAnyTag( ISteamUGC* self, UGCQueryHandle_t handle, bool bMatchAnyTag );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetSearchText( ISteamUGC* self, UGCQueryHandle_t handle, const char * pSearchText );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetRankedByTrendDays( ISteamUGC* self, UGCQueryHandle_t handle, uint32 unDays );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetTimeCreatedDateRange( ISteamUGC* self, UGCQueryHandle_t handle, RTime32 rtStart, RTime32 rtEnd );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetTimeUpdatedDateRange( ISteamUGC* self, UGCQueryHandle_t handle, RTime32 rtStart, RTime32 rtEnd );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_AddRequiredKeyValueTag( ISteamUGC* self, UGCQueryHandle_t handle, const char * pKey, const char * pValue );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_RequestUGCDetails( ISteamUGC* self, PublishedFileId_t nPublishedFileID, uint32 unMaxAgeSeconds );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_CreateItem( ISteamUGC* self, AppId_t nConsumerAppId, EWorkshopFileType eFileType );
STEAMAPI_API UGCUpdateHandle_t SteamAPI_ISteamUGC_StartItemUpdate( ISteamUGC* self, AppId_t nConsumerAppId, PublishedFileId_t nPublishedFileID );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetItemTitle( ISteamUGC* self, UGCUpdateHandle_t handle, const char * pchTitle );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetItemDescription( ISteamUGC* self, UGCUpdateHandle_t handle, const char * pchDescription );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetItemUpdateLanguage( ISteamUGC* self, UGCUpdateHandle_t handle, const char * pchLanguage );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetItemMetadata( ISteamUGC* self, UGCUpdateHandle_t handle, const char * pchMetaData );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetItemVisibility( ISteamUGC* self, UGCUpdateHandle_t handle, ERemoteStoragePublishedFileVisibility eVisibility );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetItemTags( ISteamUGC* self, UGCUpdateHandle_t updateHandle, const SteamParamStringArray_t * pTags, bool bAllowAdminTags ); // Note: param bool bAllowAdminTags was introduced in SDK v158a, but didn't exist before
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetItemContent( ISteamUGC* self, UGCUpdateHandle_t handle, const char * pszContentFolder );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetItemPreview( ISteamUGC* self, UGCUpdateHandle_t handle, const char * pszPreviewFile );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetAllowLegacyUpload( ISteamUGC* self, UGCUpdateHandle_t handle, bool bAllowLegacyUpload );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_RemoveAllItemKeyValueTags( ISteamUGC* self, UGCUpdateHandle_t handle );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_RemoveItemKeyValueTags( ISteamUGC* self, UGCUpdateHandle_t handle, const char * pchKey );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_AddItemKeyValueTag( ISteamUGC* self, UGCUpdateHandle_t handle, const char * pchKey, const char * pchValue );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_AddItemPreviewFile( ISteamUGC* self, UGCUpdateHandle_t handle, const char * pszPreviewFile, EItemPreviewType type );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_AddItemPreviewVideo( ISteamUGC* self, UGCUpdateHandle_t handle, const char * pszVideoID );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_UpdateItemPreviewFile( ISteamUGC* self, UGCUpdateHandle_t handle, uint32 index, const char * pszPreviewFile );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_UpdateItemPreviewVideo( ISteamUGC* self, UGCUpdateHandle_t handle, uint32 index, const char * pszVideoID );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_RemoveItemPreview( ISteamUGC* self, UGCUpdateHandle_t handle, uint32 index );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_AddContentDescriptor( ISteamUGC* self, UGCUpdateHandle_t handle, EUGCContentDescriptorID descid );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_RemoveContentDescriptor( ISteamUGC* self, UGCUpdateHandle_t handle, EUGCContentDescriptorID descid );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_SetRequiredGameVersions( ISteamUGC* self, UGCUpdateHandle_t handle, const char * pszGameBranchMin, const char * pszGameBranchMax );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_SubmitItemUpdate( ISteamUGC* self, UGCUpdateHandle_t handle, const char * pchChangeNote );
STEAMAPI_API EItemUpdateStatus SteamAPI_ISteamUGC_GetItemUpdateProgress( ISteamUGC* self, UGCUpdateHandle_t handle, uint64 * punBytesProcessed, uint64 * punBytesTotal );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_SetUserItemVote( ISteamUGC* self, PublishedFileId_t nPublishedFileID, bool bVoteUp );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_GetUserItemVote( ISteamUGC* self, PublishedFileId_t nPublishedFileID );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_AddItemToFavorites( ISteamUGC* self, AppId_t nAppId, PublishedFileId_t nPublishedFileID );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_RemoveItemFromFavorites( ISteamUGC* self, AppId_t nAppId, PublishedFileId_t nPublishedFileID );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_SubscribeItem( ISteamUGC* self, PublishedFileId_t nPublishedFileID );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_UnsubscribeItem( ISteamUGC* self, PublishedFileId_t nPublishedFileID );
STEAMAPI_API uint32 SteamAPI_ISteamUGC_GetNumSubscribedItems( ISteamUGC* self );
STEAMAPI_API uint32 SteamAPI_ISteamUGC_GetSubscribedItems( ISteamUGC* self, PublishedFileId_t * pvecPublishedFileID, uint32 cMaxEntries );
STEAMAPI_API uint32 SteamAPI_ISteamUGC_GetItemState( ISteamUGC* self, PublishedFileId_t nPublishedFileID );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_GetItemInstallInfo( ISteamUGC* self, PublishedFileId_t nPublishedFileID, uint64 * punSizeOnDisk, char * pchFolder, uint32 cchFolderSize, uint32 * punTimeStamp );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_GetItemDownloadInfo( ISteamUGC* self, PublishedFileId_t nPublishedFileID, uint64 * punBytesDownloaded, uint64 * punBytesTotal );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_DownloadItem( ISteamUGC* self, PublishedFileId_t nPublishedFileID, bool bHighPriority );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_BInitWorkshopForGameServer( ISteamUGC* self, DepotId_t unWorkshopDepotID, const char * pszFolder );
STEAMAPI_API void SteamAPI_ISteamUGC_SuspendDownloads( ISteamUGC* self, bool bSuspend );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_StartPlaytimeTracking( ISteamUGC* self, PublishedFileId_t * pvecPublishedFileID, uint32 unNumPublishedFileIDs );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_StopPlaytimeTracking( ISteamUGC* self, PublishedFileId_t * pvecPublishedFileID, uint32 unNumPublishedFileIDs );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_StopPlaytimeTrackingForAllItems( ISteamUGC* self );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_AddDependency( ISteamUGC* self, PublishedFileId_t nParentPublishedFileID, PublishedFileId_t nChildPublishedFileID );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_RemoveDependency( ISteamUGC* self, PublishedFileId_t nParentPublishedFileID, PublishedFileId_t nChildPublishedFileID );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_AddAppDependency( ISteamUGC* self, PublishedFileId_t nPublishedFileID, AppId_t nAppID );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_RemoveAppDependency( ISteamUGC* self, PublishedFileId_t nPublishedFileID, AppId_t nAppID );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_GetAppDependencies( ISteamUGC* self, PublishedFileId_t nPublishedFileID );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_DeleteItem( ISteamUGC* self, PublishedFileId_t nPublishedFileID );
STEAMAPI_API steam_bool SteamAPI_ISteamUGC_ShowWorkshopEULA( ISteamUGC* self );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamUGC_GetWorkshopEULAStatus( ISteamUGC* self );
STEAMAPI_API uint32 SteamAPI_ISteamUGC_GetUserContentDescriptorPreferences( ISteamUGC* self, EUGCContentDescriptorID * pvecDescriptors, uint32 cMaxEntries );

// ISteamAppList
STEAMAPI_API ISteamAppList *SteamAPI_SteamAppList_v001();
STEAMAPI_API uint32 SteamAPI_ISteamAppList_GetNumInstalledApps( ISteamAppList* self );
STEAMAPI_API uint32 SteamAPI_ISteamAppList_GetInstalledApps( ISteamAppList* self, AppId_t * pvecAppID, uint32 unMaxAppIDs );
STEAMAPI_API int SteamAPI_ISteamAppList_GetAppName( ISteamAppList* self, AppId_t nAppID, char * pchName, int cchNameMax );
STEAMAPI_API int SteamAPI_ISteamAppList_GetAppInstallDir( ISteamAppList* self, AppId_t nAppID, char * pchDirectory, int cchNameMax );
STEAMAPI_API int SteamAPI_ISteamAppList_GetAppBuildId( ISteamAppList* self, AppId_t nAppID );

// ISteamHTMLSurface
STEAMAPI_API ISteamHTMLSurface *SteamAPI_SteamHTMLSurface_v005();
STEAMAPI_API steam_bool SteamAPI_ISteamHTMLSurface_Init( ISteamHTMLSurface* self );
STEAMAPI_API steam_bool SteamAPI_ISteamHTMLSurface_Shutdown( ISteamHTMLSurface* self );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamHTMLSurface_CreateBrowser( ISteamHTMLSurface* self, const char * pchUserAgent, const char * pchUserCSS );
STEAMAPI_API void SteamAPI_ISteamHTMLSurface_RemoveBrowser( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle );
STEAMAPI_API void SteamAPI_ISteamHTMLSurface_LoadURL( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, const char * pchURL, const char * pchPostData );
STEAMAPI_API void SteamAPI_ISteamHTMLSurface_SetSize( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, uint32 unWidth, uint32 unHeight );
STEAMAPI_API void SteamAPI_ISteamHTMLSurface_StopLoad( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle );
STEAMAPI_API void SteamAPI_ISteamHTMLSurface_Reload( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle );
STEAMAPI_API void SteamAPI_ISteamHTMLSurface_GoBack( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle );
STEAMAPI_API void SteamAPI_ISteamHTMLSurface_GoForward( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle );
STEAMAPI_API void SteamAPI_ISteamHTMLSurface_AddHeader( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, const char * pchKey, const char * pchValue );
STEAMAPI_API void SteamAPI_ISteamHTMLSurface_ExecuteJavascript( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, const char * pchScript );
STEAMAPI_API void SteamAPI_ISteamHTMLSurface_MouseUp( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, EHTMLMouseButton eMouseButton );
STEAMAPI_API void SteamAPI_ISteamHTMLSurface_MouseDown( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, EHTMLMouseButton eMouseButton );
STEAMAPI_API void SteamAPI_ISteamHTMLSurface_MouseDoubleClick( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, EHTMLMouseButton eMouseButton );
STEAMAPI_API void SteamAPI_ISteamHTMLSurface_MouseMove( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, int x, int y );
STEAMAPI_API void SteamAPI_ISteamHTMLSurface_MouseWheel( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, int32 nDelta );
STEAMAPI_API void SteamAPI_ISteamHTMLSurface_KeyDown( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, uint32 nNativeKeyCode, EHTMLKeyModifiers eHTMLKeyModifiers, bool bIsSystemKey );
STEAMAPI_API void SteamAPI_ISteamHTMLSurface_KeyUp( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, uint32 nNativeKeyCode, EHTMLKeyModifiers eHTMLKeyModifiers );
STEAMAPI_API void SteamAPI_ISteamHTMLSurface_KeyChar( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, uint32 cUnicodeChar, EHTMLKeyModifiers eHTMLKeyModifiers );
STEAMAPI_API void SteamAPI_ISteamHTMLSurface_SetHorizontalScroll( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, uint32 nAbsolutePixelScroll );
STEAMAPI_API void SteamAPI_ISteamHTMLSurface_SetVerticalScroll( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, uint32 nAbsolutePixelScroll );
STEAMAPI_API void SteamAPI_ISteamHTMLSurface_SetKeyFocus( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, bool bHasKeyFocus );
STEAMAPI_API void SteamAPI_ISteamHTMLSurface_ViewSource( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle );
STEAMAPI_API void SteamAPI_ISteamHTMLSurface_CopyToClipboard( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle );
STEAMAPI_API void SteamAPI_ISteamHTMLSurface_PasteFromClipboard( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle );
STEAMAPI_API void SteamAPI_ISteamHTMLSurface_Find( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, const char * pchSearchStr, bool bCurrentlyInFind, bool bReverse );
STEAMAPI_API void SteamAPI_ISteamHTMLSurface_StopFind( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle );
STEAMAPI_API void SteamAPI_ISteamHTMLSurface_GetLinkAtPosition( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, int x, int y );
STEAMAPI_API void SteamAPI_ISteamHTMLSurface_SetCookie( ISteamHTMLSurface* self, const char * pchHostname, const char * pchKey, const char * pchValue, const char * pchPath, RTime32 nExpires, bool bSecure, bool bHTTPOnly );
STEAMAPI_API void SteamAPI_ISteamHTMLSurface_SetPageScaleFactor( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, float flZoom, int nPointX, int nPointY );
STEAMAPI_API void SteamAPI_ISteamHTMLSurface_SetBackgroundMode( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, bool bBackgroundMode );
STEAMAPI_API void SteamAPI_ISteamHTMLSurface_SetDPIScalingFactor( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, float flDPIScaling );
STEAMAPI_API void SteamAPI_ISteamHTMLSurface_OpenDeveloperTools( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle );
STEAMAPI_API void SteamAPI_ISteamHTMLSurface_AllowStartRequest( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, bool bAllowed );
STEAMAPI_API void SteamAPI_ISteamHTMLSurface_JSDialogResponse( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, bool bResult );
STEAMAPI_API void SteamAPI_ISteamHTMLSurface_FileLoadDialogResponse( ISteamHTMLSurface* self, HHTMLBrowser unBrowserHandle, const char ** pchSelectedFiles );

// ISteamInventory
STEAMAPI_API ISteamInventory *SteamAPI_SteamInventory_v003();
STEAMAPI_API ISteamInventory *SteamAPI_SteamGameServerInventory_v003();
STEAMAPI_API EResult SteamAPI_ISteamInventory_GetResultStatus( ISteamInventory* self, SteamInventoryResult_t resultHandle );
STEAMAPI_API steam_bool SteamAPI_ISteamInventory_GetResultItems( ISteamInventory* self, SteamInventoryResult_t resultHandle, SteamItemDetails_t * pOutItemsArray, uint32 * punOutItemsArraySize );
STEAMAPI_API steam_bool SteamAPI_ISteamInventory_GetResultItemProperty( ISteamInventory* self, SteamInventoryResult_t resultHandle, uint32 unItemIndex, const char * pchPropertyName, char * pchValueBuffer, uint32 * punValueBufferSizeOut );
STEAMAPI_API uint32 SteamAPI_ISteamInventory_GetResultTimestamp( ISteamInventory* self, SteamInventoryResult_t resultHandle );
STEAMAPI_API steam_bool SteamAPI_ISteamInventory_CheckResultSteamID( ISteamInventory* self, SteamInventoryResult_t resultHandle, uint64_steamid steamIDExpected );
STEAMAPI_API void SteamAPI_ISteamInventory_DestroyResult( ISteamInventory* self, SteamInventoryResult_t resultHandle );
STEAMAPI_API steam_bool SteamAPI_ISteamInventory_GetAllItems( ISteamInventory* self, SteamInventoryResult_t * pResultHandle );
STEAMAPI_API steam_bool SteamAPI_ISteamInventory_GetItemsByID( ISteamInventory* self, SteamInventoryResult_t * pResultHandle, const SteamItemInstanceID_t * pInstanceIDs, uint32 unCountInstanceIDs );
STEAMAPI_API steam_bool SteamAPI_ISteamInventory_SerializeResult( ISteamInventory* self, SteamInventoryResult_t resultHandle, void * pOutBuffer, uint32 * punOutBufferSize );
STEAMAPI_API steam_bool SteamAPI_ISteamInventory_DeserializeResult( ISteamInventory* self, SteamInventoryResult_t * pOutResultHandle, const void * pBuffer, uint32 unBufferSize, bool bRESERVED_MUST_BE_FALSE );
STEAMAPI_API steam_bool SteamAPI_ISteamInventory_GenerateItems( ISteamInventory* self, SteamInventoryResult_t * pResultHandle, const SteamItemDef_t * pArrayItemDefs, const uint32 * punArrayQuantity, uint32 unArrayLength );
STEAMAPI_API steam_bool SteamAPI_ISteamInventory_GrantPromoItems( ISteamInventory* self, SteamInventoryResult_t * pResultHandle );
STEAMAPI_API steam_bool SteamAPI_ISteamInventory_AddPromoItem( ISteamInventory* self, SteamInventoryResult_t * pResultHandle, SteamItemDef_t itemDef );
STEAMAPI_API steam_bool SteamAPI_ISteamInventory_AddPromoItems( ISteamInventory* self, SteamInventoryResult_t * pResultHandle, const SteamItemDef_t * pArrayItemDefs, uint32 unArrayLength );
STEAMAPI_API steam_bool SteamAPI_ISteamInventory_ConsumeItem( ISteamInventory* self, SteamInventoryResult_t * pResultHandle, SteamItemInstanceID_t itemConsume, uint32 unQuantity );
STEAMAPI_API steam_bool SteamAPI_ISteamInventory_ExchangeItems( ISteamInventory* self, SteamInventoryResult_t * pResultHandle, const SteamItemDef_t * pArrayGenerate, const uint32 * punArrayGenerateQuantity, uint32 unArrayGenerateLength, const SteamItemInstanceID_t * pArrayDestroy, const uint32 * punArrayDestroyQuantity, uint32 unArrayDestroyLength );
STEAMAPI_API steam_bool SteamAPI_ISteamInventory_TransferItemQuantity( ISteamInventory* self, SteamInventoryResult_t * pResultHandle, SteamItemInstanceID_t itemIdSource, uint32 unQuantity, SteamItemInstanceID_t itemIdDest );
STEAMAPI_API void SteamAPI_ISteamInventory_SendItemDropHeartbeat( ISteamInventory* self );
STEAMAPI_API steam_bool SteamAPI_ISteamInventory_TriggerItemDrop( ISteamInventory* self, SteamInventoryResult_t * pResultHandle, SteamItemDef_t dropListDefinition );
STEAMAPI_API steam_bool SteamAPI_ISteamInventory_TradeItems( ISteamInventory* self, SteamInventoryResult_t * pResultHandle, uint64_steamid steamIDTradePartner, const SteamItemInstanceID_t * pArrayGive, const uint32 * pArrayGiveQuantity, uint32 nArrayGiveLength, const SteamItemInstanceID_t * pArrayGet, const uint32 * pArrayGetQuantity, uint32 nArrayGetLength );
STEAMAPI_API steam_bool SteamAPI_ISteamInventory_LoadItemDefinitions( ISteamInventory* self );
STEAMAPI_API steam_bool SteamAPI_ISteamInventory_GetItemDefinitionIDs( ISteamInventory* self, SteamItemDef_t * pItemDefIDs, uint32 * punItemDefIDsArraySize );
STEAMAPI_API steam_bool SteamAPI_ISteamInventory_GetItemDefinitionProperty( ISteamInventory* self, SteamItemDef_t iDefinition, const char * pchPropertyName, char * pchValueBuffer, uint32 * punValueBufferSizeOut );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamInventory_RequestEligiblePromoItemDefinitionsIDs( ISteamInventory* self, uint64_steamid steamID );
STEAMAPI_API steam_bool SteamAPI_ISteamInventory_GetEligiblePromoItemDefinitionIDs( ISteamInventory* self, uint64_steamid steamID, SteamItemDef_t * pItemDefIDs, uint32 * punItemDefIDsArraySize );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamInventory_StartPurchase( ISteamInventory* self, const SteamItemDef_t * pArrayItemDefs, const uint32 * punArrayQuantity, uint32 unArrayLength );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamInventory_RequestPrices( ISteamInventory* self );
STEAMAPI_API uint32 SteamAPI_ISteamInventory_GetNumItemsWithPrices( ISteamInventory* self );
STEAMAPI_API steam_bool SteamAPI_ISteamInventory_GetItemsWithPrices( ISteamInventory* self, SteamItemDef_t * pArrayItemDefs, uint64 * pCurrentPrices, uint64 * pBasePrices, uint32 unArrayLength );
STEAMAPI_API steam_bool SteamAPI_ISteamInventory_GetItemPrice( ISteamInventory* self, SteamItemDef_t iDefinition, uint64 * pCurrentPrice, uint64 * pBasePrice );
STEAMAPI_API SteamInventoryUpdateHandle_t SteamAPI_ISteamInventory_StartUpdateProperties( ISteamInventory* self );
STEAMAPI_API steam_bool SteamAPI_ISteamInventory_RemoveProperty( ISteamInventory* self, SteamInventoryUpdateHandle_t handle, SteamItemInstanceID_t nItemID, const char * pchPropertyName );
STEAMAPI_API steam_bool SteamAPI_ISteamInventory_SetPropertyString( ISteamInventory* self, SteamInventoryUpdateHandle_t handle, SteamItemInstanceID_t nItemID, const char * pchPropertyName, const char * pchPropertyValue );
STEAMAPI_API steam_bool SteamAPI_ISteamInventory_SetPropertyBool( ISteamInventory* self, SteamInventoryUpdateHandle_t handle, SteamItemInstanceID_t nItemID, const char * pchPropertyName, bool bValue );
STEAMAPI_API steam_bool SteamAPI_ISteamInventory_SetPropertyInt64( ISteamInventory* self, SteamInventoryUpdateHandle_t handle, SteamItemInstanceID_t nItemID, const char * pchPropertyName, int64 nValue );
STEAMAPI_API steam_bool SteamAPI_ISteamInventory_SetPropertyFloat( ISteamInventory* self, SteamInventoryUpdateHandle_t handle, SteamItemInstanceID_t nItemID, const char * pchPropertyName, float flValue );
STEAMAPI_API steam_bool SteamAPI_ISteamInventory_SubmitUpdateProperties( ISteamInventory* self, SteamInventoryUpdateHandle_t handle, SteamInventoryResult_t * pResultHandle );
STEAMAPI_API steam_bool SteamAPI_ISteamInventory_InspectItem( ISteamInventory* self, SteamInventoryResult_t * pResultHandle, const char * pchItemToken );


// ISteamTimeline

// A versioned accessor is exported by the library
STEAMAPI_API ISteamTimeline *SteamAPI_SteamTimeline_v004();
STEAMAPI_API ISteamTimeline *SteamAPI_SteamTimeline_v001();
STEAMAPI_API void SteamAPI_ISteamTimeline_SetTimelineStateDescription( ISteamTimeline* self, const char * pchDescription, float flTimeDelta );
STEAMAPI_API void SteamAPI_ISteamTimeline_ClearTimelineStateDescription( ISteamTimeline* self, float flTimeDelta );
STEAMAPI_API void SteamAPI_ISteamTimeline_AddTimelineEvent( ISteamTimeline* self, const char * pchIcon, const char * pchTitle, const char * pchDescription, uint32 unPriority, float flStartOffsetSeconds, float flDurationSeconds, ETimelineEventClipPriority ePossibleClip );
STEAMAPI_API void SteamAPI_ISteamTimeline_SetTimelineGameMode( ISteamTimeline* self, ETimelineGameMode eMode );
STEAMAPI_API void SteamAPI_ISteamTimeline_SetTimelineTooltip( ISteamTimeline* self, const char * pchDescription, float flTimeDelta );
STEAMAPI_API void SteamAPI_ISteamTimeline_ClearTimelineTooltip( ISteamTimeline* self, float flTimeDelta );
STEAMAPI_API TimelineEventHandle_t SteamAPI_ISteamTimeline_AddInstantaneousTimelineEvent( ISteamTimeline* self, const char * pchTitle, const char * pchDescription, const char * pchIcon, uint32 unIconPriority, float flStartOffsetSeconds, ETimelineEventClipPriority ePossibleClip );
STEAMAPI_API TimelineEventHandle_t SteamAPI_ISteamTimeline_AddRangeTimelineEvent( ISteamTimeline* self, const char * pchTitle, const char * pchDescription, const char * pchIcon, uint32 unIconPriority, float flStartOffsetSeconds, float flDuration, ETimelineEventClipPriority ePossibleClip );
STEAMAPI_API TimelineEventHandle_t SteamAPI_ISteamTimeline_StartRangeTimelineEvent( ISteamTimeline* self, const char * pchTitle, const char * pchDescription, const char * pchIcon, uint32 unPriority, float flStartOffsetSeconds, ETimelineEventClipPriority ePossibleClip );
STEAMAPI_API void SteamAPI_ISteamTimeline_UpdateRangeTimelineEvent( ISteamTimeline* self, TimelineEventHandle_t ulEvent, const char * pchTitle, const char * pchDescription, const char * pchIcon, uint32 unPriority, ETimelineEventClipPriority ePossibleClip );
STEAMAPI_API void SteamAPI_ISteamTimeline_EndRangeTimelineEvent( ISteamTimeline* self, TimelineEventHandle_t ulEvent, float flEndOffsetSeconds );
STEAMAPI_API void SteamAPI_ISteamTimeline_RemoveTimelineEvent( ISteamTimeline* self, TimelineEventHandle_t ulEvent );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamTimeline_DoesEventRecordingExist( ISteamTimeline* self, TimelineEventHandle_t ulEvent );
STEAMAPI_API void SteamAPI_ISteamTimeline_StartGamePhase( ISteamTimeline* self );
STEAMAPI_API void SteamAPI_ISteamTimeline_EndGamePhase( ISteamTimeline* self );
STEAMAPI_API void SteamAPI_ISteamTimeline_SetGamePhaseID( ISteamTimeline* self, const char * pchPhaseID );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamTimeline_DoesGamePhaseRecordingExist( ISteamTimeline* self, const char * pchPhaseID );
STEAMAPI_API void SteamAPI_ISteamTimeline_AddGamePhaseTag( ISteamTimeline* self, const char * pchTagName, const char * pchTagIcon, const char * pchTagGroup, uint32 unPriority );
STEAMAPI_API void SteamAPI_ISteamTimeline_SetGamePhaseAttribute( ISteamTimeline* self, const char * pchAttributeGroup, const char * pchAttributeValue, uint32 unPriority );
STEAMAPI_API void SteamAPI_ISteamTimeline_OpenOverlayToGamePhase( ISteamTimeline* self, const char * pchPhaseID );
STEAMAPI_API void SteamAPI_ISteamTimeline_OpenOverlayToTimelineEvent( ISteamTimeline* self, const TimelineEventHandle_t ulEvent );

// ISteamVideo
STEAMAPI_API ISteamVideo *SteamAPI_SteamVideo_v001();
STEAMAPI_API ISteamVideo *SteamAPI_SteamVideo_v002();
STEAMAPI_API ISteamVideo *SteamAPI_SteamVideo_v007();
STEAMAPI_API void SteamAPI_ISteamVideo_GetVideoURL( ISteamVideo* self, AppId_t unVideoAppID );
STEAMAPI_API steam_bool SteamAPI_ISteamVideo_IsBroadcasting( ISteamVideo* self, int * pnNumViewers );
STEAMAPI_API void SteamAPI_ISteamVideo_GetOPFSettings( ISteamVideo* self, AppId_t unVideoAppID );
STEAMAPI_API steam_bool SteamAPI_ISteamVideo_GetOPFStringForApp( ISteamVideo* self, AppId_t unVideoAppID, char * pchBuffer, int32 * pnBufferSize );

// ISteamTV
STEAMAPI_API ISteamTV *SteamAPI_SteamTV_v001();
STEAMAPI_API steam_bool SteamAPI_ISteamTV_IsBroadcasting( ISteamTV* self, int * pnNumViewers );
STEAMAPI_API void SteamAPI_ISteamTV_AddBroadcastGameData( ISteamTV* self, const char * pchKey, const char * pchValue );
STEAMAPI_API void SteamAPI_ISteamTV_RemoveBroadcastGameData( ISteamTV* self, const char * pchKey );
STEAMAPI_API void SteamAPI_ISteamTV_AddTimelineMarker( ISteamTV* self, const char * pchTemplateName, bool bPersistent, uint8 nColorR, uint8 nColorG, uint8 nColorB );
STEAMAPI_API void SteamAPI_ISteamTV_RemoveTimelineMarker( ISteamTV* self );
STEAMAPI_API uint32 SteamAPI_ISteamTV_AddRegion( ISteamTV* self, const char * pchElementName, const char * pchTimelineDataSection, const SteamTVRegion_t * pSteamTVRegion, ESteamTVRegionBehavior eSteamTVRegionBehavior );
STEAMAPI_API void SteamAPI_ISteamTV_RemoveRegion( ISteamTV* self, uint32 unRegionHandle );

// ISteamParentalSettings
STEAMAPI_API ISteamParentalSettings *SteamAPI_SteamParentalSettings_v001();
STEAMAPI_API steam_bool SteamAPI_ISteamParentalSettings_BIsParentalLockEnabled( ISteamParentalSettings* self );
STEAMAPI_API steam_bool SteamAPI_ISteamParentalSettings_BIsParentalLockLocked( ISteamParentalSettings* self );
STEAMAPI_API steam_bool SteamAPI_ISteamParentalSettings_BIsAppBlocked( ISteamParentalSettings* self, AppId_t nAppID );
STEAMAPI_API steam_bool SteamAPI_ISteamParentalSettings_BIsAppInBlockList( ISteamParentalSettings* self, AppId_t nAppID );
STEAMAPI_API steam_bool SteamAPI_ISteamParentalSettings_BIsFeatureBlocked( ISteamParentalSettings* self, EParentalFeature eFeature );
STEAMAPI_API steam_bool SteamAPI_ISteamParentalSettings_BIsFeatureInBlockList( ISteamParentalSettings* self, EParentalFeature eFeature );

// ISteamRemotePlay
STEAMAPI_API ISteamRemotePlay *SteamAPI_SteamRemotePlay_v001();
STEAMAPI_API ISteamRemotePlay *SteamAPI_SteamRemotePlay_v002();
STEAMAPI_API uint32 SteamAPI_ISteamRemotePlay_GetSessionCount( ISteamRemotePlay* self );
STEAMAPI_API RemotePlaySessionID_t SteamAPI_ISteamRemotePlay_GetSessionID( ISteamRemotePlay* self, int iSessionIndex );
STEAMAPI_API uint64_steamid SteamAPI_ISteamRemotePlay_GetSessionSteamID( ISteamRemotePlay* self, RemotePlaySessionID_t unSessionID );
STEAMAPI_API const char * SteamAPI_ISteamRemotePlay_GetSessionClientName( ISteamRemotePlay* self, RemotePlaySessionID_t unSessionID );
STEAMAPI_API ESteamDeviceFormFactor SteamAPI_ISteamRemotePlay_GetSessionClientFormFactor( ISteamRemotePlay* self, RemotePlaySessionID_t unSessionID );
STEAMAPI_API steam_bool SteamAPI_ISteamRemotePlay_BGetSessionClientResolution( ISteamRemotePlay* self, RemotePlaySessionID_t unSessionID, int * pnResolutionX, int * pnResolutionY );
STEAMAPI_API steam_bool SteamAPI_ISteamRemotePlay_BStartRemotePlayTogether( ISteamRemotePlay* self, bool bShowOverlay );
STEAMAPI_API steam_bool SteamAPI_ISteamRemotePlay_BSendRemotePlayTogetherInvite( ISteamRemotePlay* self, uint64_steamid steamIDFriend );

// ISteamNetworkingMessages
STEAMAPI_API ISteamNetworkingMessages *SteamAPI_SteamNetworkingMessages_v002();
STEAMAPI_API ISteamNetworkingMessages *SteamAPI_SteamNetworkingMessages_SteamAPI_v002();
STEAMAPI_API ISteamNetworkingMessages *SteamAPI_SteamGameServerNetworkingMessages_v002();
STEAMAPI_API ISteamNetworkingMessages *SteamAPI_SteamGameServerNetworkingMessages_SteamAPI_v002();
STEAMAPI_API EResult SteamAPI_ISteamNetworkingMessages_SendMessageToUser( ISteamNetworkingMessages* self, const SteamNetworkingIdentity & identityRemote, const void * pubData, uint32 cubData, int nSendFlags, int nRemoteChannel );
STEAMAPI_API int SteamAPI_ISteamNetworkingMessages_ReceiveMessagesOnChannel( ISteamNetworkingMessages* self, int nLocalChannel, SteamNetworkingMessage_t ** ppOutMessages, int nMaxMessages );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingMessages_AcceptSessionWithUser( ISteamNetworkingMessages* self, const SteamNetworkingIdentity & identityRemote );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingMessages_CloseSessionWithUser( ISteamNetworkingMessages* self, const SteamNetworkingIdentity & identityRemote );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingMessages_CloseChannelWithUser( ISteamNetworkingMessages* self, const SteamNetworkingIdentity & identityRemote, int nLocalChannel );
STEAMAPI_API ESteamNetworkingConnectionState SteamAPI_ISteamNetworkingMessages_GetSessionConnectionInfo( ISteamNetworkingMessages* self, const SteamNetworkingIdentity & identityRemote, SteamNetConnectionInfo_t * pConnectionInfo, SteamNetworkingQuickConnectionStatus * pQuickStatus );

// ISteamNetworkingSockets
STEAMAPI_API ISteamNetworkingSockets *SteamAPI_SteamNetworkingSockets_SteamAPI_v012();
STEAMAPI_API ISteamNetworkingSockets *SteamAPI_SteamGameServerNetworkingSockets_SteamAPI_v012();
STEAMAPI_API ISteamNetworkingSockets *SteamAPI_SteamNetworkingSockets_SteamAPI_v011();
STEAMAPI_API ISteamNetworkingSockets *SteamAPI_SteamGameServerNetworkingSockets_SteamAPI_v011();
STEAMAPI_API ISteamNetworkingSockets *SteamAPI_SteamNetworkingSockets_SteamAPI_v009();
STEAMAPI_API ISteamNetworkingSockets *SteamAPI_SteamGameServerNetworkingSockets_SteamAPI_v009();
STEAMAPI_API ISteamNetworkingSockets *SteamAPI_SteamNetworkingSockets_v009();
STEAMAPI_API ISteamNetworkingSockets *SteamAPI_SteamGameServerNetworkingSockets_v009();
STEAMAPI_API ISteamNetworkingSockets *SteamAPI_SteamNetworkingSockets_v008();
STEAMAPI_API ISteamNetworkingSockets *SteamAPI_SteamGameServerNetworkingSockets_v008();
STEAMAPI_API HSteamListenSocket SteamAPI_ISteamNetworkingSockets_CreateListenSocketIP( ISteamNetworkingSockets* self, const SteamNetworkingIPAddr & localAddress, int nOptions, const SteamNetworkingConfigValue_t * pOptions );
STEAMAPI_API HSteamNetConnection SteamAPI_ISteamNetworkingSockets_ConnectByIPAddress( ISteamNetworkingSockets* self, const SteamNetworkingIPAddr & address, int nOptions, const SteamNetworkingConfigValue_t * pOptions );
STEAMAPI_API HSteamListenSocket SteamAPI_ISteamNetworkingSockets_CreateListenSocketP2P( ISteamNetworkingSockets* self, int nLocalVirtualPort, int nOptions, const SteamNetworkingConfigValue_t * pOptions );
STEAMAPI_API HSteamNetConnection SteamAPI_ISteamNetworkingSockets_ConnectP2P( ISteamNetworkingSockets* self, const SteamNetworkingIdentity & identityRemote, int nRemoteVirtualPort, int nOptions, const SteamNetworkingConfigValue_t * pOptions );
STEAMAPI_API EResult SteamAPI_ISteamNetworkingSockets_AcceptConnection( ISteamNetworkingSockets* self, HSteamNetConnection hConn );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingSockets_CloseConnection( ISteamNetworkingSockets* self, HSteamNetConnection hPeer, int nReason, const char * pszDebug, bool bEnableLinger );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingSockets_CloseListenSocket( ISteamNetworkingSockets* self, HSteamListenSocket hSocket );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingSockets_SetConnectionUserData( ISteamNetworkingSockets* self, HSteamNetConnection hPeer, int64 nUserData );
STEAMAPI_API int64 SteamAPI_ISteamNetworkingSockets_GetConnectionUserData( ISteamNetworkingSockets* self, HSteamNetConnection hPeer );
STEAMAPI_API void SteamAPI_ISteamNetworkingSockets_SetConnectionName( ISteamNetworkingSockets* self, HSteamNetConnection hPeer, const char * pszName );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingSockets_GetConnectionName( ISteamNetworkingSockets* self, HSteamNetConnection hPeer, char * pszName, int nMaxLen );
STEAMAPI_API EResult SteamAPI_ISteamNetworkingSockets_SendMessageToConnection( ISteamNetworkingSockets* self, HSteamNetConnection hConn, const void * pData, uint32 cbData, int nSendFlags, int64 * pOutMessageNumber );
STEAMAPI_API void SteamAPI_ISteamNetworkingSockets_SendMessages( ISteamNetworkingSockets* self, int nMessages, SteamNetworkingMessage_t *const * pMessages, int64 * pOutMessageNumberOrResult );
STEAMAPI_API EResult SteamAPI_ISteamNetworkingSockets_FlushMessagesOnConnection( ISteamNetworkingSockets* self, HSteamNetConnection hConn );
STEAMAPI_API int SteamAPI_ISteamNetworkingSockets_ReceiveMessagesOnConnection( ISteamNetworkingSockets* self, HSteamNetConnection hConn, SteamNetworkingMessage_t ** ppOutMessages, int nMaxMessages );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingSockets_GetConnectionInfo( ISteamNetworkingSockets* self, HSteamNetConnection hConn, SteamNetConnectionInfo_t * pInfo );
STEAMAPI_API EResult SteamAPI_ISteamNetworkingSockets_GetConnectionRealTimeStatus( ISteamNetworkingSockets* self, HSteamNetConnection hConn, SteamNetConnectionRealTimeStatus_t * pStatus, int nLanes, SteamNetConnectionRealTimeLaneStatus_t * pLanes );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingSockets_GetQuickConnectionStatus( ISteamNetworkingSockets009* self, HSteamNetConnection hConn, SteamNetworkingQuickConnectionStatus * pStats );
STEAMAPI_API int SteamAPI_ISteamNetworkingSockets_GetDetailedConnectionStatus( ISteamNetworkingSockets* self, HSteamNetConnection hConn, char * pszBuf, int cbBuf );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingSockets_GetListenSocketAddress( ISteamNetworkingSockets* self, HSteamListenSocket hSocket, SteamNetworkingIPAddr * address );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingSockets_CreateSocketPair( ISteamNetworkingSockets* self, HSteamNetConnection * pOutConnection1, HSteamNetConnection * pOutConnection2, bool bUseNetworkLoopback, const SteamNetworkingIdentity * pIdentity1, const SteamNetworkingIdentity * pIdentity2 );
STEAMAPI_API EResult SteamAPI_ISteamNetworkingSockets_ConfigureConnectionLanes( ISteamNetworkingSockets* self, HSteamNetConnection hConn, int nNumLanes, const int * pLanePriorities, const uint16 * pLaneWeights );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingSockets_GetIdentity( ISteamNetworkingSockets* self, SteamNetworkingIdentity * pIdentity );
STEAMAPI_API ESteamNetworkingAvailability SteamAPI_ISteamNetworkingSockets_InitAuthentication( ISteamNetworkingSockets* self );
STEAMAPI_API ESteamNetworkingAvailability SteamAPI_ISteamNetworkingSockets_GetAuthenticationStatus( ISteamNetworkingSockets* self, SteamNetAuthenticationStatus_t * pDetails );
STEAMAPI_API HSteamNetPollGroup SteamAPI_ISteamNetworkingSockets_CreatePollGroup( ISteamNetworkingSockets* self );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingSockets_DestroyPollGroup( ISteamNetworkingSockets* self, HSteamNetPollGroup hPollGroup );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingSockets_SetConnectionPollGroup( ISteamNetworkingSockets* self, HSteamNetConnection hConn, HSteamNetPollGroup hPollGroup );
STEAMAPI_API int SteamAPI_ISteamNetworkingSockets_ReceiveMessagesOnPollGroup( ISteamNetworkingSockets* self, HSteamNetPollGroup hPollGroup, SteamNetworkingMessage_t ** ppOutMessages, int nMaxMessages );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingSockets_ReceivedRelayAuthTicket( ISteamNetworkingSockets* self, const void * pvTicket, int cbTicket, SteamDatagramRelayAuthTicket * pOutParsedTicket );
STEAMAPI_API int SteamAPI_ISteamNetworkingSockets_FindRelayAuthTicketForServer( ISteamNetworkingSockets* self, const SteamNetworkingIdentity & identityGameServer, int nRemoteVirtualPort, SteamDatagramRelayAuthTicket * pOutParsedTicket );
STEAMAPI_API HSteamNetConnection SteamAPI_ISteamNetworkingSockets_ConnectToHostedDedicatedServer( ISteamNetworkingSockets* self, const SteamNetworkingIdentity & identityTarget, int nRemoteVirtualPort, int nOptions, const SteamNetworkingConfigValue_t * pOptions );
STEAMAPI_API uint16 SteamAPI_ISteamNetworkingSockets_GetHostedDedicatedServerPort( ISteamNetworkingSockets* self );
STEAMAPI_API SteamNetworkingPOPID SteamAPI_ISteamNetworkingSockets_GetHostedDedicatedServerPOPID( ISteamNetworkingSockets* self );
STEAMAPI_API EResult SteamAPI_ISteamNetworkingSockets_GetHostedDedicatedServerAddress( ISteamNetworkingSockets* self, SteamDatagramHostedAddress * pRouting );
STEAMAPI_API HSteamListenSocket SteamAPI_ISteamNetworkingSockets_CreateHostedDedicatedServerListenSocket( ISteamNetworkingSockets* self, int nLocalVirtualPort, int nOptions, const SteamNetworkingConfigValue_t * pOptions );
STEAMAPI_API EResult SteamAPI_ISteamNetworkingSockets_GetGameCoordinatorServerLogin( ISteamNetworkingSockets* self, SteamDatagramGameCoordinatorServerLogin * pLoginInfo, int * pcbSignedBlob, void * pBlob );
STEAMAPI_API HSteamNetConnection SteamAPI_ISteamNetworkingSockets_ConnectP2PCustomSignaling( ISteamNetworkingSockets* self, ISteamNetworkingConnectionSignaling * pSignaling, const SteamNetworkingIdentity * pPeerIdentity, int nRemoteVirtualPort, int nOptions, const SteamNetworkingConfigValue_t * pOptions );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingSockets_ReceivedP2PCustomSignal( ISteamNetworkingSockets* self, const void * pMsg, int cbMsg, ISteamNetworkingSignalingRecvContext * pContext );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingSockets_GetCertificateRequest( ISteamNetworkingSockets* self, int * pcbBlob, void * pBlob, SteamNetworkingErrMsg & errMsg );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingSockets_SetCertificate( ISteamNetworkingSockets* self, const void * pCertificate, int cbCertificate, SteamNetworkingErrMsg & errMsg );
STEAMAPI_API void SteamAPI_ISteamNetworkingSockets_ResetIdentity( ISteamNetworkingSockets* self, const SteamNetworkingIdentity * pIdentity );
STEAMAPI_API void SteamAPI_ISteamNetworkingSockets_RunCallbacks( ISteamNetworkingSockets* self );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingSockets_BeginAsyncRequestFakeIP( ISteamNetworkingSockets* self, int nNumPorts );
STEAMAPI_API void SteamAPI_ISteamNetworkingSockets_GetFakeIP( ISteamNetworkingSockets* self, int idxFirstPort, SteamNetworkingFakeIPResult_t * pInfo );
STEAMAPI_API HSteamListenSocket SteamAPI_ISteamNetworkingSockets_CreateListenSocketP2PFakeIP( ISteamNetworkingSockets* self, int idxFakePort, int nOptions, const SteamNetworkingConfigValue_t * pOptions );
STEAMAPI_API EResult SteamAPI_ISteamNetworkingSockets_GetRemoteFakeIPForConnection( ISteamNetworkingSockets* self, HSteamNetConnection hConn, SteamNetworkingIPAddr * pOutAddr );
STEAMAPI_API ISteamNetworkingFakeUDPPort * SteamAPI_ISteamNetworkingSockets_CreateFakeUDPPort( ISteamNetworkingSockets* self, int idxFakeServerPort );

// ISteamNetworkingConnectionCustomSignaling
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingConnectionCustomSignaling_SendSignal( ISteamNetworkingConnectionCustomSignaling* self, HSteamNetConnection hConn, const SteamNetConnectionInfo_t & info, const void * pMsg, int cbMsg );
STEAMAPI_API void SteamAPI_ISteamNetworkingConnectionCustomSignaling_Release( ISteamNetworkingConnectionCustomSignaling* self );

// ISteamNetworkingCustomSignalingRecvContext
STEAMAPI_API ISteamNetworkingConnectionCustomSignaling * SteamAPI_ISteamNetworkingCustomSignalingRecvContext_OnConnectRequest( ISteamNetworkingCustomSignalingRecvContext* self, HSteamNetConnection hConn, const SteamNetworkingIdentity & identityPeer );
STEAMAPI_API void SteamAPI_ISteamNetworkingCustomSignalingRecvContext_SendRejectionSignal( ISteamNetworkingCustomSignalingRecvContext* self, const SteamNetworkingIdentity & identityPeer, const void * pMsg, int cbMsg );

// ISteamNetworkingUtils
STEAMAPI_API ISteamNetworkingUtils *SteamAPI_SteamNetworkingUtils_SteamAPI_v003();
STEAMAPI_API ISteamNetworkingUtils *SteamAPI_SteamNetworkingUtils_SteamAPI_v004();
STEAMAPI_API ISteamNetworkingUtils *SteamAPI_SteamNetworkingUtils_v003();
STEAMAPI_API SteamNetworkingMessage_t * SteamAPI_ISteamNetworkingUtils_AllocateMessage( ISteamNetworkingUtils* self, int cbAllocateBuffer );
STEAMAPI_API void SteamAPI_ISteamNetworkingUtils_InitRelayNetworkAccess( ISteamNetworkingUtils* self );
STEAMAPI_API ESteamNetworkingAvailability SteamAPI_ISteamNetworkingUtils_GetRelayNetworkStatus( ISteamNetworkingUtils* self, SteamRelayNetworkStatus_t * pDetails );
STEAMAPI_API float SteamAPI_ISteamNetworkingUtils_GetLocalPingLocation( ISteamNetworkingUtils* self, SteamNetworkPingLocation_t & result );
STEAMAPI_API int SteamAPI_ISteamNetworkingUtils_EstimatePingTimeBetweenTwoLocations( ISteamNetworkingUtils* self, const SteamNetworkPingLocation_t & location1, const SteamNetworkPingLocation_t & location2 );
STEAMAPI_API int SteamAPI_ISteamNetworkingUtils_EstimatePingTimeFromLocalHost( ISteamNetworkingUtils* self, const SteamNetworkPingLocation_t & remoteLocation );
STEAMAPI_API void SteamAPI_ISteamNetworkingUtils_ConvertPingLocationToString( ISteamNetworkingUtils* self, const SteamNetworkPingLocation_t & location, char * pszBuf, int cchBufSize );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_ParsePingLocationString( ISteamNetworkingUtils* self, const char * pszString, SteamNetworkPingLocation_t & result );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_CheckPingDataUpToDate( ISteamNetworkingUtils* self, float flMaxAgeSeconds );
STEAMAPI_API int SteamAPI_ISteamNetworkingUtils_GetPingToDataCenter( ISteamNetworkingUtils* self, SteamNetworkingPOPID popID, SteamNetworkingPOPID * pViaRelayPoP );
STEAMAPI_API int SteamAPI_ISteamNetworkingUtils_GetDirectPingToPOP( ISteamNetworkingUtils* self, SteamNetworkingPOPID popID );
STEAMAPI_API int SteamAPI_ISteamNetworkingUtils_GetPOPCount( ISteamNetworkingUtils* self );
STEAMAPI_API int SteamAPI_ISteamNetworkingUtils_GetPOPList( ISteamNetworkingUtils* self, SteamNetworkingPOPID * list, int nListSz );
STEAMAPI_API SteamNetworkingMicroseconds SteamAPI_ISteamNetworkingUtils_GetLocalTimestamp( ISteamNetworkingUtils* self );
STEAMAPI_API void SteamAPI_ISteamNetworkingUtils_SetDebugOutputFunction( ISteamNetworkingUtils* self, ESteamNetworkingSocketsDebugOutputType eDetailLevel, FSteamNetworkingSocketsDebugOutput pfnFunc );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_IsFakeIPv4( ISteamNetworkingUtils* self, uint32 nIPv4 );
STEAMAPI_API ESteamNetworkingFakeIPType SteamAPI_ISteamNetworkingUtils_GetIPv4FakeIPType( ISteamNetworkingUtils* self, uint32 nIPv4 );
STEAMAPI_API EResult SteamAPI_ISteamNetworkingUtils_GetRealIdentityForFakeIP( ISteamNetworkingUtils* self, const SteamNetworkingIPAddr & fakeIP, SteamNetworkingIdentity * pOutRealIdentity );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_SetGlobalConfigValueInt32( ISteamNetworkingUtils* self, ESteamNetworkingConfigValue eValue, int32 val );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_SetGlobalConfigValueFloat( ISteamNetworkingUtils* self, ESteamNetworkingConfigValue eValue, float val );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_SetGlobalConfigValueString( ISteamNetworkingUtils* self, ESteamNetworkingConfigValue eValue, const char * val );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_SetGlobalConfigValuePtr( ISteamNetworkingUtils* self, ESteamNetworkingConfigValue eValue, void * val );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_SetConnectionConfigValueInt32( ISteamNetworkingUtils* self, HSteamNetConnection hConn, ESteamNetworkingConfigValue eValue, int32 val );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_SetConnectionConfigValueFloat( ISteamNetworkingUtils* self, HSteamNetConnection hConn, ESteamNetworkingConfigValue eValue, float val );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_SetConnectionConfigValueString( ISteamNetworkingUtils* self, HSteamNetConnection hConn, ESteamNetworkingConfigValue eValue, const char * val );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_SetGlobalCallback_SteamNetConnectionStatusChanged( ISteamNetworkingUtils* self, FnSteamNetConnectionStatusChanged fnCallback );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_SetGlobalCallback_SteamNetAuthenticationStatusChanged( ISteamNetworkingUtils* self, FnSteamNetAuthenticationStatusChanged fnCallback );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_SetGlobalCallback_SteamRelayNetworkStatusChanged( ISteamNetworkingUtils* self, FnSteamRelayNetworkStatusChanged fnCallback );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_SetGlobalCallback_FakeIPResult( ISteamNetworkingUtils* self, FnSteamNetworkingFakeIPResult fnCallback );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_SetGlobalCallback_MessagesSessionRequest( ISteamNetworkingUtils* self, FnSteamNetworkingMessagesSessionRequest fnCallback );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_SetGlobalCallback_MessagesSessionFailed( ISteamNetworkingUtils* self, FnSteamNetworkingMessagesSessionFailed fnCallback );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_SetConfigValue( ISteamNetworkingUtils* self, ESteamNetworkingConfigValue eValue, ESteamNetworkingConfigScope eScopeType, intptr_t scopeObj, ESteamNetworkingConfigDataType eDataType, const void * pArg );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_SetConfigValueStruct( ISteamNetworkingUtils* self, const SteamNetworkingConfigValue_t & opt, ESteamNetworkingConfigScope eScopeType, intptr_t scopeObj );
STEAMAPI_API ESteamNetworkingGetConfigValueResult SteamAPI_ISteamNetworkingUtils_GetConfigValue( ISteamNetworkingUtils* self, ESteamNetworkingConfigValue eValue, ESteamNetworkingConfigScope eScopeType, intptr_t scopeObj, ESteamNetworkingConfigDataType * pOutDataType, void * pResult, size_t * cbResult );
STEAMAPI_API const char * SteamAPI_ISteamNetworkingUtils_GetConfigValueInfo( ISteamNetworkingUtils* self, ESteamNetworkingConfigValue eValue, ESteamNetworkingConfigDataType * pOutDataType, ESteamNetworkingConfigScope * pOutScope );
STEAMAPI_API ESteamNetworkingConfigValue SteamAPI_ISteamNetworkingUtils_GetFirstConfigValue( ISteamNetworkingUtils* self );
STEAMAPI_API ESteamNetworkingConfigValue SteamAPI_ISteamNetworkingUtils_IterateGenericEditableConfigValues( ISteamNetworkingUtils* self, ESteamNetworkingConfigValue eCurrent, bool bEnumerateDevVars );
STEAMAPI_API void SteamAPI_ISteamNetworkingUtils_SteamNetworkingIPAddr_ToString( ISteamNetworkingUtils* self, const SteamNetworkingIPAddr & addr, char * buf, uint32 cbBuf, bool bWithPort );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_SteamNetworkingIPAddr_ParseString( ISteamNetworkingUtils* self, SteamNetworkingIPAddr * pAddr, const char * pszStr );
STEAMAPI_API ESteamNetworkingFakeIPType SteamAPI_ISteamNetworkingUtils_SteamNetworkingIPAddr_GetFakeIPType( ISteamNetworkingUtils* self, const SteamNetworkingIPAddr & addr );
STEAMAPI_API void SteamAPI_ISteamNetworkingUtils_SteamNetworkingIdentity_ToString( ISteamNetworkingUtils* self, const SteamNetworkingIdentity & identity, char * buf, uint32 cbBuf );
STEAMAPI_API steam_bool SteamAPI_ISteamNetworkingUtils_SteamNetworkingIdentity_ParseString( ISteamNetworkingUtils* self, SteamNetworkingIdentity * pIdentity, const char * pszStr );

// ISteamGameServer
STEAMAPI_API ISteamGameServer *SteamAPI_SteamGameServer_v013();
STEAMAPI_API ISteamGameServer *SteamAPI_SteamGameServer_v014();
STEAMAPI_API ISteamGameServer *SteamAPI_SteamGameServer_v015();
STEAMAPI_API void SteamAPI_ISteamGameServer_SetProduct( ISteamGameServer* self, const char * pszProduct );
STEAMAPI_API void SteamAPI_ISteamGameServer_SetGameDescription( ISteamGameServer* self, const char * pszGameDescription );
STEAMAPI_API void SteamAPI_ISteamGameServer_SetModDir( ISteamGameServer* self, const char * pszModDir );
STEAMAPI_API void SteamAPI_ISteamGameServer_SetDedicatedServer( ISteamGameServer* self, bool bDedicated );
STEAMAPI_API void SteamAPI_ISteamGameServer_LogOn( ISteamGameServer* self, const char * pszToken );
STEAMAPI_API void SteamAPI_ISteamGameServer_LogOnAnonymous( ISteamGameServer* self );
STEAMAPI_API void SteamAPI_ISteamGameServer_LogOff( ISteamGameServer* self );
STEAMAPI_API steam_bool SteamAPI_ISteamGameServer_BLoggedOn( ISteamGameServer* self );
STEAMAPI_API steam_bool SteamAPI_ISteamGameServer_BSecure( ISteamGameServer* self );
STEAMAPI_API uint64_steamid SteamAPI_ISteamGameServer_GetSteamID( ISteamGameServer* self );
STEAMAPI_API steam_bool SteamAPI_ISteamGameServer_WasRestartRequested( ISteamGameServer* self );
STEAMAPI_API void SteamAPI_ISteamGameServer_SetMaxPlayerCount( ISteamGameServer* self, int cPlayersMax );
STEAMAPI_API void SteamAPI_ISteamGameServer_SetBotPlayerCount( ISteamGameServer* self, int cBotplayers );
STEAMAPI_API void SteamAPI_ISteamGameServer_SetServerName( ISteamGameServer* self, const char * pszServerName );
STEAMAPI_API void SteamAPI_ISteamGameServer_SetMapName( ISteamGameServer* self, const char * pszMapName );
STEAMAPI_API void SteamAPI_ISteamGameServer_SetPasswordProtected( ISteamGameServer* self, bool bPasswordProtected );
STEAMAPI_API void SteamAPI_ISteamGameServer_SetSpectatorPort( ISteamGameServer* self, uint16 unSpectatorPort );
STEAMAPI_API void SteamAPI_ISteamGameServer_SetSpectatorServerName( ISteamGameServer* self, const char * pszSpectatorServerName );
STEAMAPI_API void SteamAPI_ISteamGameServer_ClearAllKeyValues( ISteamGameServer* self );
STEAMAPI_API void SteamAPI_ISteamGameServer_SetKeyValue( ISteamGameServer* self, const char * pKey, const char * pValue );
STEAMAPI_API void SteamAPI_ISteamGameServer_SetGameTags( ISteamGameServer* self, const char * pchGameTags );
STEAMAPI_API void SteamAPI_ISteamGameServer_SetGameData( ISteamGameServer* self, const char * pchGameData );
STEAMAPI_API void SteamAPI_ISteamGameServer_SetRegion( ISteamGameServer* self, const char * pszRegion );
STEAMAPI_API steam_bool SteamAPI_ISteamGameServer_SendUserConnectAndAuthenticate( ISteamGameServer* self, uint32 unIPClient, const void * pvAuthBlob, uint32 cubAuthBlobSize, CSteamID * pSteamIDUser );
STEAMAPI_API void SteamAPI_ISteamGameServer_SendUserDisconnect( ISteamGameServer* self, uint64_steamid steamIDUser );
STEAMAPI_API void SteamAPI_ISteamGameServer_SetAdvertiseServerActive( ISteamGameServer* self, bool bActive );
STEAMAPI_API HAuthTicket SteamAPI_ISteamGameServer_GetAuthSessionTicket( ISteamGameServer* self, void * pTicket, int cbMaxTicket, uint32 * pcbTicket, const SteamNetworkingIdentity * pSnid );
STEAMAPI_API EBeginAuthSessionResult SteamAPI_ISteamGameServer_BeginAuthSession( ISteamGameServer* self, const void * pAuthTicket, int cbAuthTicket, uint64_steamid steamID );
STEAMAPI_API void SteamAPI_ISteamGameServer_EndAuthSession( ISteamGameServer* self, uint64_steamid steamID );
STEAMAPI_API void SteamAPI_ISteamGameServer_CancelAuthTicket( ISteamGameServer* self, HAuthTicket hAuthTicket );
STEAMAPI_API EUserHasLicenseForAppResult SteamAPI_ISteamGameServer_UserHasLicenseForApp( ISteamGameServer* self, uint64_steamid steamID, AppId_t appID );
STEAMAPI_API steam_bool SteamAPI_ISteamGameServer_RequestUserGroupStatus( ISteamGameServer* self, uint64_steamid steamIDUser, uint64_steamid steamIDGroup );
STEAMAPI_API void SteamAPI_ISteamGameServer_GetGameplayStats( ISteamGameServer* self );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamGameServer_GetServerReputation( ISteamGameServer* self );
//STEAMAPI_API SteamIPAddress_t SteamAPI_ISteamGameServer_GetPublicIP( ISteamGameServer* self );
STEAMAPI_API steam_bool SteamAPI_ISteamGameServer_HandleIncomingPacket( ISteamGameServer* self, const void * pData, int cbData, uint32 srcIP, uint16 srcPort );
STEAMAPI_API int SteamAPI_ISteamGameServer_GetNextOutgoingPacket( ISteamGameServer* self, void * pOut, int cbMaxOut, uint32 * pNetAdr, uint16 * pPort );
STEAMAPI_API void SteamAPI_ISteamGameServer_EnableHeartbeats( ISteamGameServer* self, bool bActive );
STEAMAPI_API void SteamAPI_ISteamGameServer_SetHeartbeatInterval( ISteamGameServer* self, int iHeartbeatInterval );
STEAMAPI_API void SteamAPI_ISteamGameServer_ForceHeartbeat( ISteamGameServer* self );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamGameServer_AssociateWithClan( ISteamGameServer* self, uint64_steamid steamIDClan );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamGameServer_ComputeNewPlayerCompatibility( ISteamGameServer* self, uint64_steamid steamIDNewPlayer );
STEAMAPI_API steam_bool SteamAPI_ISteamGameServer_SendUserConnectAndAuthenticate_DEPRECATED( ISteamGameServer* self, uint32 unIPClient, const void * pvAuthBlob, uint32 cubAuthBlobSize, CSteamID * pSteamIDUser );
STEAMAPI_API uint64_steamid SteamAPI_ISteamGameServer_CreateUnauthenticatedUserConnection( ISteamGameServer* self );
STEAMAPI_API void SteamAPI_ISteamGameServer_SendUserDisconnect_DEPRECATED( ISteamGameServer* self, uint64_steamid steamIDUser );
STEAMAPI_API steam_bool SteamAPI_ISteamGameServer_BUpdateUserData( ISteamGameServer* self, uint64_steamid steamIDUser, const char * pchPlayerName, uint32 uScore );


// ISteamGameServerStats
STEAMAPI_API ISteamGameServerStats *SteamAPI_SteamGameServerStats_v001();
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamGameServerStats_RequestUserStats( ISteamGameServerStats* self, uint64_steamid steamIDUser );
STEAMAPI_API steam_bool SteamAPI_ISteamGameServerStats_GetUserStatInt32( ISteamGameServerStats* self, uint64_steamid steamIDUser, const char * pchName, int32 * pData );
STEAMAPI_API steam_bool SteamAPI_ISteamGameServerStats_GetUserStatFloat( ISteamGameServerStats* self, uint64_steamid steamIDUser, const char * pchName, float * pData );
STEAMAPI_API steam_bool SteamAPI_ISteamGameServerStats_GetUserAchievement( ISteamGameServerStats* self, uint64_steamid steamIDUser, const char * pchName, bool * pbAchieved );
STEAMAPI_API steam_bool SteamAPI_ISteamGameServerStats_SetUserStatInt32( ISteamGameServerStats* self, uint64_steamid steamIDUser, const char * pchName, int32 nData );
STEAMAPI_API steam_bool SteamAPI_ISteamGameServerStats_SetUserStatFloat( ISteamGameServerStats* self, uint64_steamid steamIDUser, const char * pchName, float fData );
STEAMAPI_API steam_bool SteamAPI_ISteamGameServerStats_UpdateUserAvgRateStat( ISteamGameServerStats* self, uint64_steamid steamIDUser, const char * pchName, float flCountThisSession, double dSessionLength );
STEAMAPI_API steam_bool SteamAPI_ISteamGameServerStats_SetUserAchievement( ISteamGameServerStats* self, uint64_steamid steamIDUser, const char * pchName );
STEAMAPI_API steam_bool SteamAPI_ISteamGameServerStats_ClearUserAchievement( ISteamGameServerStats* self, uint64_steamid steamIDUser, const char * pchName );
STEAMAPI_API SteamAPICall_t SteamAPI_ISteamGameServerStats_StoreUserStats( ISteamGameServerStats* self, uint64_steamid steamIDUser );

// ISteamNetworkingFakeUDPPort
STEAMAPI_API void SteamAPI_ISteamNetworkingFakeUDPPort_DestroyFakeUDPPort( ISteamNetworkingFakeUDPPort* self );
STEAMAPI_API EResult SteamAPI_ISteamNetworkingFakeUDPPort_SendMessageToFakeIP( ISteamNetworkingFakeUDPPort* self, const SteamNetworkingIPAddr & remoteAddress, const void * pData, uint32 cbData, int nSendFlags );
STEAMAPI_API int SteamAPI_ISteamNetworkingFakeUDPPort_ReceiveMessages( ISteamNetworkingFakeUDPPort* self, SteamNetworkingMessage_t ** ppOutMessages, int nMaxMessages );
STEAMAPI_API void SteamAPI_ISteamNetworkingFakeUDPPort_ScheduleCleanup( ISteamNetworkingFakeUDPPort* self, const SteamNetworkingIPAddr & remoteAddress );

// SteamIPAddress_t
STEAMAPI_API steam_bool SteamAPI_SteamIPAddress_t_IsSet( SteamIPAddress_t* self );

// MatchMakingKeyValuePair_t
STEAMAPI_API void SteamAPI_MatchMakingKeyValuePair_t_Construct( MatchMakingKeyValuePair_t* self );

// servernetadr_t
STEAMAPI_API void SteamAPI_servernetadr_t_Construct( servernetadr_t* self );
STEAMAPI_API void SteamAPI_servernetadr_t_Init( servernetadr_t* self, unsigned int ip, uint16 usQueryPort, uint16 usConnectionPort );
STEAMAPI_API uint16 SteamAPI_servernetadr_t_GetQueryPort( servernetadr_t* self );
STEAMAPI_API void SteamAPI_servernetadr_t_SetQueryPort( servernetadr_t* self, uint16 usPort );
STEAMAPI_API uint16 SteamAPI_servernetadr_t_GetConnectionPort( servernetadr_t* self );
STEAMAPI_API void SteamAPI_servernetadr_t_SetConnectionPort( servernetadr_t* self, uint16 usPort );
STEAMAPI_API uint32 SteamAPI_servernetadr_t_GetIP( servernetadr_t* self );
STEAMAPI_API void SteamAPI_servernetadr_t_SetIP( servernetadr_t* self, uint32 unIP );
STEAMAPI_API const char * SteamAPI_servernetadr_t_GetConnectionAddressString( servernetadr_t* self );
STEAMAPI_API const char * SteamAPI_servernetadr_t_GetQueryAddressString( servernetadr_t* self );
STEAMAPI_API steam_bool SteamAPI_servernetadr_t_IsLessThan( servernetadr_t* self, const servernetadr_t & netadr );
STEAMAPI_API void SteamAPI_servernetadr_t_Assign( servernetadr_t* self, const servernetadr_t & that );

// gameserveritem_t
STEAMAPI_API void SteamAPI_gameserveritem_t_Construct( gameserveritem_t* self );
STEAMAPI_API const char * SteamAPI_gameserveritem_t_GetName( gameserveritem_t* self );
STEAMAPI_API void SteamAPI_gameserveritem_t_SetName( gameserveritem_t* self, const char * pName );

// SteamNetworkingIPAddr
STEAMAPI_API void SteamAPI_SteamNetworkingIPAddr_Clear( SteamNetworkingIPAddr* self );
STEAMAPI_API steam_bool SteamAPI_SteamNetworkingIPAddr_IsIPv6AllZeros( SteamNetworkingIPAddr* self );
STEAMAPI_API void SteamAPI_SteamNetworkingIPAddr_SetIPv6( SteamNetworkingIPAddr* self, const uint8 * ipv6, uint16 nPort );
STEAMAPI_API void SteamAPI_SteamNetworkingIPAddr_SetIPv4( SteamNetworkingIPAddr* self, uint32 nIP, uint16 nPort );
STEAMAPI_API steam_bool SteamAPI_SteamNetworkingIPAddr_IsIPv4( SteamNetworkingIPAddr* self );
STEAMAPI_API uint32 SteamAPI_SteamNetworkingIPAddr_GetIPv4( SteamNetworkingIPAddr* self );
STEAMAPI_API void SteamAPI_SteamNetworkingIPAddr_SetIPv6LocalHost( SteamNetworkingIPAddr* self, uint16 nPort );
STEAMAPI_API steam_bool SteamAPI_SteamNetworkingIPAddr_IsLocalHost( SteamNetworkingIPAddr* self );
STEAMAPI_API void SteamAPI_SteamNetworkingIPAddr_ToString( SteamNetworkingIPAddr* self, char * buf, uint32 cbBuf, bool bWithPort );
STEAMAPI_API steam_bool SteamAPI_SteamNetworkingIPAddr_ParseString( SteamNetworkingIPAddr* self, const char * pszStr );
STEAMAPI_API steam_bool SteamAPI_SteamNetworkingIPAddr_IsEqualTo( SteamNetworkingIPAddr* self, const SteamNetworkingIPAddr & x );
STEAMAPI_API ESteamNetworkingFakeIPType SteamAPI_SteamNetworkingIPAddr_GetFakeIPType( SteamNetworkingIPAddr* self );
STEAMAPI_API steam_bool SteamAPI_SteamNetworkingIPAddr_IsFakeIP( SteamNetworkingIPAddr* self );

// SteamNetworkingIdentity
STEAMAPI_API void SteamAPI_SteamNetworkingIdentity_Clear( SteamNetworkingIdentity* self );
STEAMAPI_API steam_bool SteamAPI_SteamNetworkingIdentity_IsInvalid( SteamNetworkingIdentity* self );
STEAMAPI_API void SteamAPI_SteamNetworkingIdentity_SetSteamID( SteamNetworkingIdentity* self, uint64_steamid steamID );
STEAMAPI_API uint64_steamid SteamAPI_SteamNetworkingIdentity_GetSteamID( SteamNetworkingIdentity* self );
STEAMAPI_API void SteamAPI_SteamNetworkingIdentity_SetSteamID64( SteamNetworkingIdentity* self, uint64 steamID );
STEAMAPI_API uint64 SteamAPI_SteamNetworkingIdentity_GetSteamID64( SteamNetworkingIdentity* self );
STEAMAPI_API steam_bool SteamAPI_SteamNetworkingIdentity_SetXboxPairwiseID( SteamNetworkingIdentity* self, const char * pszString );
STEAMAPI_API const char * SteamAPI_SteamNetworkingIdentity_GetXboxPairwiseID( SteamNetworkingIdentity* self );
STEAMAPI_API void SteamAPI_SteamNetworkingIdentity_SetPSNID( SteamNetworkingIdentity* self, uint64 id );
STEAMAPI_API uint64 SteamAPI_SteamNetworkingIdentity_GetPSNID( SteamNetworkingIdentity* self );
// removed since SDK v1.61
STEAMAPI_API void SteamAPI_SteamNetworkingIdentity_SetStadiaID( SteamNetworkingIdentity* self, uint64 id );
// removed since SDK v1.61
STEAMAPI_API uint64 SteamAPI_SteamNetworkingIdentity_GetStadiaID( SteamNetworkingIdentity* self );
STEAMAPI_API void SteamAPI_SteamNetworkingIdentity_SetIPAddr( SteamNetworkingIdentity* self, const SteamNetworkingIPAddr & addr );
STEAMAPI_API const SteamNetworkingIPAddr * SteamAPI_SteamNetworkingIdentity_GetIPAddr( SteamNetworkingIdentity* self );
STEAMAPI_API void SteamAPI_SteamNetworkingIdentity_SetIPv4Addr( SteamNetworkingIdentity* self, uint32 nIPv4, uint16 nPort );
STEAMAPI_API uint32 SteamAPI_SteamNetworkingIdentity_GetIPv4( SteamNetworkingIdentity* self );
STEAMAPI_API ESteamNetworkingFakeIPType SteamAPI_SteamNetworkingIdentity_GetFakeIPType( SteamNetworkingIdentity* self );
STEAMAPI_API steam_bool SteamAPI_SteamNetworkingIdentity_IsFakeIP( SteamNetworkingIdentity* self );
STEAMAPI_API void SteamAPI_SteamNetworkingIdentity_SetLocalHost( SteamNetworkingIdentity* self );
STEAMAPI_API steam_bool SteamAPI_SteamNetworkingIdentity_IsLocalHost( SteamNetworkingIdentity* self );
STEAMAPI_API steam_bool SteamAPI_SteamNetworkingIdentity_SetGenericString( SteamNetworkingIdentity* self, const char * pszString );
STEAMAPI_API const char * SteamAPI_SteamNetworkingIdentity_GetGenericString( SteamNetworkingIdentity* self );
STEAMAPI_API steam_bool SteamAPI_SteamNetworkingIdentity_SetGenericBytes( SteamNetworkingIdentity* self, const void * data, uint32 cbLen );
STEAMAPI_API const uint8 * SteamAPI_SteamNetworkingIdentity_GetGenericBytes( SteamNetworkingIdentity* self, int & cbLen );
STEAMAPI_API steam_bool SteamAPI_SteamNetworkingIdentity_IsEqualTo( SteamNetworkingIdentity* self, const SteamNetworkingIdentity & x );
STEAMAPI_API void SteamAPI_SteamNetworkingIdentity_ToString( SteamNetworkingIdentity* self, char * buf, uint32 cbBuf );
STEAMAPI_API steam_bool SteamAPI_SteamNetworkingIdentity_ParseString( SteamNetworkingIdentity* self, const char * pszStr );

// SteamNetworkingMessage_t
STEAMAPI_API void SteamAPI_SteamNetworkingMessage_t_Release( SteamNetworkingMessage_t* self );

// SteamNetworkingConfigValue_t
STEAMAPI_API void SteamAPI_SteamNetworkingConfigValue_t_SetInt32( SteamNetworkingConfigValue_t* self, ESteamNetworkingConfigValue eVal, int32_t data );
STEAMAPI_API void SteamAPI_SteamNetworkingConfigValue_t_SetInt64( SteamNetworkingConfigValue_t* self, ESteamNetworkingConfigValue eVal, int64_t data );
STEAMAPI_API void SteamAPI_SteamNetworkingConfigValue_t_SetFloat( SteamNetworkingConfigValue_t* self, ESteamNetworkingConfigValue eVal, float data );
STEAMAPI_API void SteamAPI_SteamNetworkingConfigValue_t_SetPtr( SteamNetworkingConfigValue_t* self, ESteamNetworkingConfigValue eVal, void * data );
STEAMAPI_API void SteamAPI_SteamNetworkingConfigValue_t_SetString( SteamNetworkingConfigValue_t* self, ESteamNetworkingConfigValue eVal, const char * data );

// SteamNetworkingPOPIDRender
STEAMAPI_API const char * SteamAPI_SteamNetworkingPOPIDRender_c_str( SteamNetworkingPOPIDRender* self );

// SteamNetworkingIdentityRender
STEAMAPI_API const char * SteamAPI_SteamNetworkingIdentityRender_c_str( SteamNetworkingIdentityRender* self );

// SteamNetworkingIPAddrRender
STEAMAPI_API const char * SteamAPI_SteamNetworkingIPAddrRender_c_str( SteamNetworkingIPAddrRender* self );

// SteamDatagramHostedAddress
STEAMAPI_API void SteamAPI_SteamDatagramHostedAddress_Clear( SteamDatagramHostedAddress* self );
STEAMAPI_API SteamNetworkingPOPID SteamAPI_SteamDatagramHostedAddress_GetPopID( SteamDatagramHostedAddress* self );
STEAMAPI_API void SteamAPI_SteamDatagramHostedAddress_SetDevAddress( SteamDatagramHostedAddress* self, uint32 nIP, uint16 nPort, SteamNetworkingPOPID popid );
#endif // STEAMAPIFLAT_H
