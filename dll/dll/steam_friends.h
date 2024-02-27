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

#ifndef __INCLUDED_STEAM_FRIENDS_H__
#define __INCLUDED_STEAM_FRIENDS_H__

#include "base.h"
#include "overlay/steam_overlay.h"

#define SEND_FRIEND_RATE 4.0

struct Avatar_Numbers {
    int smallest;
    int medium;
    int large;
};

class Steam_Friends : 
public ISteamFriends003,
public ISteamFriends004,
public ISteamFriends005,
public ISteamFriends006,
public ISteamFriends007,
public ISteamFriends008,
public ISteamFriends009,
public ISteamFriends010,
public ISteamFriends011,
public ISteamFriends012,
public ISteamFriends013,
public ISteamFriends014,
public ISteamFriends015,
public ISteamFriends016,
public ISteamFriends
{
    class Settings *settings;
    class Networking *network;
    class SteamCallBacks *callbacks;
    class SteamCallResults *callback_results;
    class RunEveryRunCB *run_every_runcb;
    class Steam_Overlay* overlay;

    Friend us;
    bool modified;
    std::vector<Friend> friends;

    std::map<uint64, struct Avatar_Numbers> avatars;
    CSteamID lobby_id;

    std::chrono::high_resolution_clock::time_point last_sent_friends;

Friend *find_friend(CSteamID id)
{
    auto f = std::find_if(friends.begin(), friends.end(), [&id](Friend const& item) { return item.id() == id.ConvertToUint64(); });
    if (friends.end() == f)
        return NULL;

    return &(*f);
}

void persona_change(CSteamID id, EPersonaChange flags)
{
    PersonaStateChange_t data;
    data.m_ulSteamID = id.ConvertToUint64();
    data.m_nChangeFlags = flags;
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
}

void rich_presence_updated(CSteamID id, AppId_t appid)
{
    FriendRichPresenceUpdate_t data;
    data.m_steamIDFriend = id;
    data.m_nAppID = appid;
    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
}

bool isAppIdCompatible(Friend *f)
{
    if (settings->is_lobby_connect) return true;
    if (f == &us) return true;
    return settings->get_local_game_id().AppID() == f->appid();
}

struct Avatar_Numbers add_friend_avatars(CSteamID id)
{
    uint64 steam_id = id.ConvertToUint64();
    auto avatar_ids = avatars.find(steam_id);
    if (avatar_ids != avatars.end()) {
        return avatar_ids->second;
    }

    struct Avatar_Numbers avatar_numbers;
    std::string small_avatar(32 * 32 * 4, 0);
    std::string medium_avatar(64 * 64 * 4, 0);
    std::string large_avatar(184 * 184 * 4, 0);

    if (!(settings->disable_account_avatar) && (id == settings->get_local_steam_id())) {
        std::string file_path;
        unsigned long long file_size;

        for (int i = 0; i < 3; i++) {
            std::string file_name;
            if (i == 0) file_name = "account_avatar.png";
            if (i == 1) file_name = "account_avatar.jpg";
            if (i == 2) file_name = "account_avatar.jpeg";
            file_path = Local_Storage::get_game_settings_path() + file_name;
            file_size = file_size_(file_path);
            if (file_size) break;
        }

        if (!file_size) {
            for (int i = 0; i < 3; i++) {
                std::string file_name;
                if (i == 0) file_name = "account_avatar.png";
                if (i == 1) file_name = "account_avatar.jpg";
                if (i == 2) file_name = "account_avatar.jpeg";

                if (settings->local_save.length() > 0) {
                    file_path = settings->local_save + "/settings/" + file_name;
                } else {
                    file_path = Local_Storage::get_user_appdata_path() + "/settings/" + file_name;
                }

                file_size = file_size_(file_path);
                if (file_size) break;
            }
        }

        // no else statement here for default or else this breaks default images for friends
        if (file_size) {
            small_avatar = Local_Storage::load_image_resized(file_path, "", 32);
            medium_avatar = Local_Storage::load_image_resized(file_path, "", 64);
            large_avatar = Local_Storage::load_image_resized(file_path, "", 184);
        }
    } else if (!(settings->disable_account_avatar)) {
        Friend *f = find_friend(id);
        if (f && (large_avatar.compare(f->avatar()) != 0)) {
            large_avatar = f->avatar();
            medium_avatar = Local_Storage::load_image_resized("", f->avatar(), 64);
            small_avatar = Local_Storage::load_image_resized("", f->avatar(), 32);
        } else {
            std::string file_path;
            unsigned long long file_size;

            for (int i = 0; i < 3; i++) {
                std::string file_name;
                if (i == 0) file_name = "account_avatar_default.png";
                if (i == 1) file_name = "account_avatar_default.jpg";
                if (i == 2) file_name = "account_avatar_default.jpeg";
                file_path = Local_Storage::get_game_settings_path() + file_name;
                file_size = file_size_(file_path);
                if (file_size) break;
            }

            if (!file_size) {
                for (int i = 0; i < 3; i++) {
                    std::string file_name;
                    if (i == 0) file_name = "account_avatar_default.png";
                    if (i == 1) file_name = "account_avatar_default.jpg";
                    if (i == 2) file_name = "account_avatar_default.jpeg";

                    if (settings->local_save.length() > 0) {
                        file_path = settings->local_save + "/settings/" + file_name;
                    } else {
                        file_path = Local_Storage::get_user_appdata_path() + "/settings/" + file_name;
                    }

                    file_size = file_size_(file_path);
                    if (file_size) break;
                }
            }

            if (file_size) {
                small_avatar = Local_Storage::load_image_resized(file_path, "", 32);
                medium_avatar = Local_Storage::load_image_resized(file_path, "", 64);
                large_avatar = Local_Storage::load_image_resized(file_path, "", 184);
            }
        }
    }

    avatar_numbers.smallest = settings->add_image(small_avatar, 32, 32);
    avatar_numbers.medium = settings->add_image(medium_avatar, 64, 64);
    avatar_numbers.large = settings->add_image(large_avatar, 184, 184);

    avatars[steam_id] = avatar_numbers;
    return avatar_numbers;
}

public:
static void steam_friends_callback(void *object, Common_Message *msg)
{
    PRINT_DEBUG("Steam_Friends::steam_friends_callback\n");

    Steam_Friends *steam_friends = (Steam_Friends *)object;
    steam_friends->Callback(msg);
}

static void steam_friends_run_every_runcb(void *object)
{
    PRINT_DEBUG("Steam_Friends::steam_friends_run_every_runcb\n");

    Steam_Friends *steam_friends = (Steam_Friends *)object;
    steam_friends->RunCallbacks();
}

void resend_friend_data()
{
    modified = true;
}

Steam_Friends(Settings* settings, Networking* network, SteamCallResults* callback_results, SteamCallBacks* callbacks, RunEveryRunCB* run_every_runcb, Steam_Overlay* overlay):
    settings(settings),
    network(network),
    callbacks(callbacks),
    callback_results(callback_results),
    run_every_runcb(run_every_runcb),
    overlay(overlay)
{
    this->network->setCallback(CALLBACK_ID_FRIEND, settings->get_local_steam_id(), &Steam_Friends::steam_friends_callback, this);
    this->network->setCallback(CALLBACK_ID_FRIEND_MESSAGES, settings->get_local_steam_id(), &Steam_Friends::steam_friends_callback, this);
    this->network->setCallback(CALLBACK_ID_USER_STATUS, settings->get_local_steam_id(), &Steam_Friends::steam_friends_callback, this);
    this->run_every_runcb->add(&Steam_Friends::steam_friends_run_every_runcb, this);
    modified = false;
}

~Steam_Friends()
{
	//TODO rm network callbacks
	this->run_every_runcb->remove(&Steam_Friends::steam_friends_run_every_runcb, this);
}

static bool ok_friend_flags(int iFriendFlags)
{
    if (iFriendFlags & k_EFriendFlagImmediate) return true;

    return false;
}

// returns the local players name - guaranteed to not be NULL.
// this is the same name as on the users community profile page
// this is stored in UTF-8 format
// like all the other interface functions that return a char *, it's important that this pointer is not saved
// off; it will eventually be free'd or re-allocated
const char *GetPersonaName()
{
    PRINT_DEBUG("Steam_Friends::GetPersonaName\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    const char *local_name = settings->get_local_name();
    
    return local_name;
}

// Sets the player name, stores it on the server and publishes the changes to all friends who are online.
// Changes take place locally immediately, and a PersonaStateChange_t is posted, presuming success.
//
// The final results are available through the return value SteamAPICall_t, using SetPersonaNameResponse_t.
//
// If the name change fails to happen on the server, then an additional global PersonaStateChange_t will be posted
// to change the name back, in addition to the SetPersonaNameResponse_t callback.
STEAM_CALL_RESULT( SetPersonaNameResponse_t )
SteamAPICall_t SetPersonaName( const char *pchPersonaName )
{
    PRINT_DEBUG("Steam_Friends::SetPersonaName\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    SetPersonaNameResponse_t data;
    data.m_bSuccess = true;
    data.m_bLocalSuccess = false;
    data.m_result = k_EResultOK;
    persona_change(settings->get_local_steam_id(), k_EPersonaChangeName);

    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}

void SetPersonaName_old( const char *pchPersonaName )
{
	PRINT_DEBUG("Steam_Friends::SetPersonaName old\n");
	SetPersonaName(pchPersonaName);
}

// gets the status of the current user
EPersonaState GetPersonaState()
{
    PRINT_DEBUG("Steam_Friends::GetPersonaState\n");
    return k_EPersonaStateOnline;
}


// friend iteration
// takes a set of k_EFriendFlags, and returns the number of users the client knows about who meet that criteria
// then GetFriendByIndex() can then be used to return the id's of each of those users
int GetFriendCount( int iFriendFlags )
{
    PRINT_DEBUG("Steam_Friends::GetFriendCount %i\n", iFriendFlags);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    int count = 0;
    if (ok_friend_flags(iFriendFlags)) count = friends.size();
    PRINT_DEBUG("count %i\n", count);
    return count;
}

int GetFriendCount( EFriendFlags eFriendFlags )
{
    PRINT_DEBUG("Steam_Friends::GetFriendCount old\n");
	return GetFriendCount((int)eFriendFlags);
}

// returns the steamID of a user
// iFriend is a index of range [0, GetFriendCount())
// iFriendsFlags must be the same value as used in GetFriendCount()
// the returned CSteamID can then be used by all the functions below to access details about the user
CSteamID GetFriendByIndex( int iFriend, int iFriendFlags )
{
    PRINT_DEBUG("Steam_Friends::GetFriendByIndex\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    CSteamID id = k_steamIDNil;
    if (ok_friend_flags(iFriendFlags)) if (iFriend < friends.size()) id = CSteamID((uint64)friends[iFriend].id());
    
    return id;
}

CSteamID GetFriendByIndex( int iFriend, EFriendFlags eFriendFlags )
{
	PRINT_DEBUG("Steam_Friends::GetFriendByIndex old\n");
	return GetFriendByIndex(iFriend, (int)eFriendFlags );
}

// returns a relationship to a user
EFriendRelationship GetFriendRelationship( CSteamID steamIDFriend )
{
    PRINT_DEBUG("Steam_Friends::GetFriendRelationship %llu\n", steamIDFriend.ConvertToUint64());
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (steamIDFriend == settings->get_local_steam_id()) return k_EFriendRelationshipNone; //Real steam behavior
    if (find_friend(steamIDFriend)) return k_EFriendRelationshipFriend;

    return k_EFriendRelationshipNone;
}


// returns the current status of the specified user
// this will only be known by the local user if steamIDFriend is in their friends list; on the same game server; in a chat room or lobby; or in a small group with the local user
EPersonaState GetFriendPersonaState( CSteamID steamIDFriend )
{
    PRINT_DEBUG("Steam_Friends::GetFriendPersonaState %llu\n", steamIDFriend.ConvertToUint64());
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    EPersonaState state = k_EPersonaStateOffline;
    if (steamIDFriend == settings->get_local_steam_id() || find_friend(steamIDFriend)) {
        state = k_EPersonaStateOnline;
    }

    //works because all of those who could be in a lobby are our friends
    return state;
}


// returns the name another user - guaranteed to not be NULL.
// same rules as GetFriendPersonaState() apply as to whether or not the user knowns the name of the other user
// note that on first joining a lobby, chat room or game server the local user will not known the name of the other users automatically; that information will arrive asyncronously
// 
const char *GetFriendPersonaName( CSteamID steamIDFriend )
{
    PRINT_DEBUG("Steam_Friends::GetFriendPersonaName %llu\n", steamIDFriend.ConvertToUint64());
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    const char *name = "Unknown User";
    if (steamIDFriend == settings->get_local_steam_id()) {
        name = settings->get_local_name();
    } else {
        Friend *f = find_friend(steamIDFriend);
        if (f) name = f->name().c_str();
    }

    return name;
}


// returns true if the friend is actually in a game, and fills in pFriendGameInfo with an extra details 
bool GetFriendGamePlayed( CSteamID steamIDFriend, STEAM_OUT_STRUCT() FriendGameInfo_t *pFriendGameInfo )
{
    PRINT_DEBUG("Steam_Friends::GetFriendGamePlayed %llu\n", steamIDFriend.ConvertToUint64());
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    bool ret = false;

    if (steamIDFriend == settings->get_local_steam_id()) {
        if (pFriendGameInfo) {
            pFriendGameInfo->m_gameID = settings->get_local_game_id();
            pFriendGameInfo->m_unGameIP = 0;
            pFriendGameInfo->m_usGamePort = 0;
            pFriendGameInfo->m_usQueryPort = 0;
            pFriendGameInfo->m_steamIDLobby = settings->get_lobby();
            PRINT_DEBUG("self %llu %llu\n", settings->get_local_game_id().ToUint64(), settings->get_lobby().ConvertToUint64());
        }

        ret = true;
    } else {
        Friend *f = find_friend(steamIDFriend);
        if (f) {
            if (pFriendGameInfo) {
                pFriendGameInfo->m_gameID = CGameID(f->appid());
                pFriendGameInfo->m_unGameIP = 0;
                pFriendGameInfo->m_usGamePort = 0;
                pFriendGameInfo->m_usQueryPort = 0;
                pFriendGameInfo->m_steamIDLobby = CSteamID((uint64)f->lobby_id());
                PRINT_DEBUG("%u " "%" PRIu64 "\n", f->appid(), f->lobby_id());
            }

            ret = true;
        }
    }

    return ret;
}

bool GetFriendGamePlayed( CSteamID steamIDFriend, uint64 *pulGameID, uint32 *punGameIP, uint16 *pusGamePort, uint16 *pusQueryPort )
{
	PRINT_DEBUG("Steam_Friends::GetFriendGamePlayed old\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
	FriendGameInfo_t info;
	bool ret = GetFriendGamePlayed(steamIDFriend, &info);
	if (ret) {
		if (pulGameID) *pulGameID = info.m_gameID.ToUint64();
		if (punGameIP) *punGameIP = info.m_unGameIP;
		if (pusGamePort) *pusGamePort = info.m_usGamePort;
		if (pusQueryPort) *pusQueryPort = info.m_usQueryPort;
	}

	return ret;
}

// accesses old friends names - returns an empty string when their are no more items in the history
const char *GetFriendPersonaNameHistory( CSteamID steamIDFriend, int iPersonaName )
{
    PRINT_DEBUG("Steam_Friends::GetFriendPersonaNameHistory\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    const char *ret = "";
    if (iPersonaName == 0) ret = GetFriendPersonaName(steamIDFriend);
    else if (iPersonaName == 1) ret = "Some Old Name";
    
    return ret;
}

// friends steam level
int GetFriendSteamLevel( CSteamID steamIDFriend )
{
    PRINT_DEBUG("Steam_Friends::GetFriendSteamLevel\n");
    return 100;
}


// Returns nickname the current user has set for the specified player. Returns NULL if the no nickname has been set for that player.
const char *GetPlayerNickname( CSteamID steamIDPlayer )
{
    PRINT_DEBUG("Steam_Friends::GetPlayerNickname\n");
    return NULL;
}


// friend grouping (tag) apis
// returns the number of friends groups
int GetFriendsGroupCount()
{
    PRINT_DEBUG("Steam_Friends::GetFriendsGroupCount\n");
    return 0;
}

// returns the friends group ID for the given index (invalid indices return k_FriendsGroupID_Invalid)
FriendsGroupID_t GetFriendsGroupIDByIndex( int iFG )
{
    PRINT_DEBUG("Steam_Friends::GetFriendsGroupIDByIndex\n");
    return k_FriendsGroupID_Invalid;
}

// returns the name for the given friends group (NULL in the case of invalid friends group IDs)
const char *GetFriendsGroupName( FriendsGroupID_t friendsGroupID )
{
    PRINT_DEBUG("Steam_Friends::GetFriendsGroupName\n");
    return NULL;
}

// returns the number of members in a given friends group
int GetFriendsGroupMembersCount( FriendsGroupID_t friendsGroupID )
{
    PRINT_DEBUG("Steam_Friends::GetFriendsGroupMembersCount\n");
    return 0;
}

// gets up to nMembersCount members of the given friends group, if fewer exist than requested those positions' SteamIDs will be invalid
void GetFriendsGroupMembersList( FriendsGroupID_t friendsGroupID, STEAM_OUT_ARRAY_CALL(nMembersCount, GetFriendsGroupMembersCount, friendsGroupID ) CSteamID *pOutSteamIDMembers, int nMembersCount )
{
    PRINT_DEBUG("Steam_Friends::GetFriendsGroupMembersList\n");
}


// returns true if the specified user meets any of the criteria specified in iFriendFlags
// iFriendFlags can be the union (binary or, |) of one or more k_EFriendFlags values
bool HasFriend( CSteamID steamIDFriend, int iFriendFlags )
{
    PRINT_DEBUG("Steam_Friends::HasFriend\n");
    bool ret = false;
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (ok_friend_flags(iFriendFlags)) if (find_friend(steamIDFriend)) ret = true;
    
    return ret;
}

bool HasFriend( CSteamID steamIDFriend, EFriendFlags eFriendFlags ) 
{
    PRINT_DEBUG("Steam_Friends::HasFriend old\n");
	return HasFriend(steamIDFriend, (int)eFriendFlags );
}

// clan (group) iteration and access functions
int GetClanCount()
{
    PRINT_DEBUG("Steam_Friends::GetClanCount\n");
    int counter = 0;
    for (auto &c : settings->subscribed_groups_clans) counter++;
    return counter;
}

CSteamID GetClanByIndex( int iClan )
{
    PRINT_DEBUG("Steam_Friends::GetClanByIndex\n");
    int counter = 0;
    for (auto &c : settings->subscribed_groups_clans) {
        if (counter == iClan) return c.id;
        counter++;
    }
    return k_steamIDNil;
}

const char *GetClanName( CSteamID steamIDClan )
{
    PRINT_DEBUG("Steam_Friends::GetClanName\n");
    for (auto &c : settings->subscribed_groups_clans) {
        if (c.id.ConvertToUint64() == steamIDClan.ConvertToUint64()) return c.name.c_str();
    }
    return "";
}

const char *GetClanTag( CSteamID steamIDClan )
{
    PRINT_DEBUG("Steam_Friends::GetClanTag\n");
    for (auto &c : settings->subscribed_groups_clans) {
        if (c.id.ConvertToUint64() == steamIDClan.ConvertToUint64()) return c.tag.c_str();
    }
    return "";
}

// returns the most recent information we have about what's happening in a clan
bool GetClanActivityCounts( CSteamID steamIDClan, int *pnOnline, int *pnInGame, int *pnChatting )
{
    PRINT_DEBUG("Steam_Friends::GetClanActivityCounts %llu\n", steamIDClan.ConvertToUint64());
    return false;
}

// for clans a user is a member of, they will have reasonably up-to-date information, but for others you'll have to download the info to have the latest
SteamAPICall_t DownloadClanActivityCounts( STEAM_ARRAY_COUNT(cClansToRequest) CSteamID *psteamIDClans, int cClansToRequest )
{
    PRINT_DEBUG("Steam_Friends::DownloadClanActivityCounts\n");
    return 0;
}


// iterators for getting users in a chat room, lobby, game server or clan
// note that large clans that cannot be iterated by the local user
// note that the current user must be in a lobby to retrieve CSteamIDs of other users in that lobby
// steamIDSource can be the steamID of a group, game server, lobby or chat room
int GetFriendCountFromSource( CSteamID steamIDSource )
{
    PRINT_DEBUG("Steam_Friends::GetFriendCountFromSource %llu\n", steamIDSource.ConvertToUint64());
    //TODO
    return 0;
}

CSteamID GetFriendFromSourceByIndex( CSteamID steamIDSource, int iFriend )
{
    PRINT_DEBUG("Steam_Friends::GetFriendFromSourceByIndex\n");
    return k_steamIDNil;
}


// returns true if the local user can see that steamIDUser is a member or in steamIDSource
bool IsUserInSource( CSteamID steamIDUser, CSteamID steamIDSource )
{
    PRINT_DEBUG("Steam_Friends::IsUserInSource %llu %llu\n", steamIDUser.ConvertToUint64(), steamIDSource.ConvertToUint64());
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (steamIDUser == settings->get_local_steam_id()) {
        if (settings->get_lobby() == steamIDSource) {
            return true;
        }

        if (settings->subscribed_groups.find(steamIDSource.ConvertToUint64()) != settings->subscribed_groups.end()) {
            return true;
        }
    } else {
        Friend *f = find_friend(steamIDUser);
        if (!f) return false;
        if (f->lobby_id() == steamIDSource.ConvertToUint64()) return true;
    }
    //TODO
    return false;
}


// User is in a game pressing the talk button (will suppress the microphone for all voice comms from the Steam friends UI)
void SetInGameVoiceSpeaking( CSteamID steamIDUser, bool bSpeaking )
{
    PRINT_DEBUG("Steam_Friends::SetInGameVoiceSpeaking\n");
}


// activates the game overlay, with an optional dialog to open 
// valid options are "Friends", "Community", "Players", "Settings", "OfficialGameGroup", "Stats", "Achievements"
void ActivateGameOverlay( const char *pchDialog )
{
    PRINT_DEBUG("Steam_Friends::ActivateGameOverlay %s\n", pchDialog);
    overlay->OpenOverlay(pchDialog);
}


// activates game overlay to a specific place
// valid options are
//		"steamid" - opens the overlay web browser to the specified user or groups profile
//		"chat" - opens a chat window to the specified user, or joins the group chat 
//		"jointrade" - opens a window to a Steam Trading session that was started with the ISteamEconomy/StartTrade Web API
//		"stats" - opens the overlay web browser to the specified user's stats
//		"achievements" - opens the overlay web browser to the specified user's achievements
//		"friendadd" - opens the overlay in minimal mode prompting the user to add the target user as a friend
//		"friendremove" - opens the overlay in minimal mode prompting the user to remove the target friend
//		"friendrequestaccept" - opens the overlay in minimal mode prompting the user to accept an incoming friend invite
//		"friendrequestignore" - opens the overlay in minimal mode prompting the user to ignore an incoming friend invite
void ActivateGameOverlayToUser( const char *pchDialog, CSteamID steamID )
{
    PRINT_DEBUG("Steam_Friends::ActivateGameOverlayToUser %s %llu\n", pchDialog, steamID.ConvertToUint64());
}


// activates game overlay web browser directly to the specified URL
// full address with protocol type is required, e.g. http://www.steamgames.com/
void ActivateGameOverlayToWebPage( const char *pchURL, EActivateGameOverlayToWebPageMode eMode = k_EActivateGameOverlayToWebPageMode_Default )
{
    PRINT_DEBUG("Steam_Friends::ActivateGameOverlayToWebPage %s %u\n", pchURL, eMode);
    overlay->OpenOverlayWebpage(pchURL);
}

void ActivateGameOverlayToWebPage( const char *pchURL )
{
    PRINT_DEBUG("Steam_Friends::ActivateGameOverlayToWebPage old\n");
    ActivateGameOverlayToWebPage( pchURL, k_EActivateGameOverlayToWebPageMode_Default );
}

// activates game overlay to store page for app
void ActivateGameOverlayToStore( AppId_t nAppID, EOverlayToStoreFlag eFlag )
{
    PRINT_DEBUG("Steam_Friends::ActivateGameOverlayToStore\n");
}

void ActivateGameOverlayToStore( AppId_t nAppID)
{
    PRINT_DEBUG("Steam_Friends::ActivateGameOverlayToStore old\n");
}

// Mark a target user as 'played with'. This is a client-side only feature that requires that the calling user is 
// in game 
void SetPlayedWith( CSteamID steamIDUserPlayedWith )
{
    PRINT_DEBUG("Steam_Friends::SetPlayedWith\n");
}


// activates game overlay to open the invite dialog. Invitations will be sent for the provided lobby.
void ActivateGameOverlayInviteDialog( CSteamID steamIDLobby )
{
    PRINT_DEBUG("Steam_Friends::ActivateGameOverlayInviteDialog\n");
    overlay->OpenOverlayInvite(steamIDLobby);
}

// gets the small (32x32) avatar of the current user, which is a handle to be used in IClientUtils::GetImageRGBA(), or 0 if none set
int GetSmallFriendAvatar( CSteamID steamIDFriend )
{
    PRINT_DEBUG("Steam_Friends::GetSmallFriendAvatar\n");
    //IMPORTANT NOTE: don't change friend avatar numbers for the same friend or else some games endlessly allocate stuff.
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    struct Avatar_Numbers numbers = add_friend_avatars(steamIDFriend);
    return numbers.smallest;
}


// gets the medium (64x64) avatar of the current user, which is a handle to be used in IClientUtils::GetImageRGBA(), or 0 if none set
int GetMediumFriendAvatar( CSteamID steamIDFriend )
{
    PRINT_DEBUG("Steam_Friends::GetMediumFriendAvatar\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    struct Avatar_Numbers numbers = add_friend_avatars(steamIDFriend);
    return numbers.medium;
}


// gets the large (184x184) avatar of the current user, which is a handle to be used in IClientUtils::GetImageRGBA(), or 0 if none set
// returns -1 if this image has yet to be loaded, in this case wait for a AvatarImageLoaded_t callback and then call this again
int GetLargeFriendAvatar( CSteamID steamIDFriend )
{
    PRINT_DEBUG("Steam_Friends::GetLargeFriendAvatar\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    struct Avatar_Numbers numbers = add_friend_avatars(steamIDFriend);
    return numbers.large;
}

int GetFriendAvatar( CSteamID steamIDFriend, int eAvatarSize )
{
	PRINT_DEBUG("Steam_Friends::GetFriendAvatar\n");
	if (eAvatarSize == k_EAvatarSize32x32) {
		return GetSmallFriendAvatar(steamIDFriend);
	} else if (eAvatarSize == k_EAvatarSize64x64) {
		return GetMediumFriendAvatar(steamIDFriend);
	} else if (eAvatarSize == k_EAvatarSize184x184) {
		return GetLargeFriendAvatar(steamIDFriend);
	} else {
		return 0;
	}
}

int GetFriendAvatar(CSteamID steamIDFriend)
{
    PRINT_DEBUG("Steam_Friends::GetFriendAvatar old\n");
    return GetFriendAvatar(steamIDFriend, k_EAvatarSize32x32);
}

// requests information about a user - persona name & avatar
// if bRequireNameOnly is set, then the avatar of a user isn't downloaded 
// - it's a lot slower to download avatars and churns the local cache, so if you don't need avatars, don't request them
// if returns true, it means that data is being requested, and a PersonaStateChanged_t callback will be posted when it's retrieved
// if returns false, it means that we already have all the details about that user, and functions can be called immediately
bool RequestUserInformation( CSteamID steamIDUser, bool bRequireNameOnly )
{
    PRINT_DEBUG("Steam_Friends::RequestUserInformation\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    //persona_change(steamIDUser, k_EPersonaChangeName);
    //We already know everything
    return false;
}


// requests information about a clan officer list
// when complete, data is returned in ClanOfficerListResponse_t call result
// this makes available the calls below
// you can only ask about clans that a user is a member of
// note that this won't download avatars automatically; if you get an officer,
// and no avatar image is available, call RequestUserInformation( steamID, false ) to download the avatar
STEAM_CALL_RESULT( ClanOfficerListResponse_t )
SteamAPICall_t RequestClanOfficerList( CSteamID steamIDClan )
{
    PRINT_DEBUG("Steam_Friends::RequestClanOfficerList\n");
    return 0;
}


// iteration of clan officers - can only be done when a RequestClanOfficerList() call has completed

// returns the steamID of the clan owner
CSteamID GetClanOwner( CSteamID steamIDClan )
{
    PRINT_DEBUG("Steam_Friends::GetClanOwner\n");
    return k_steamIDNil;
}

// returns the number of officers in a clan (including the owner)
int GetClanOfficerCount( CSteamID steamIDClan )
{
    PRINT_DEBUG("Steam_Friends::GetClanOfficerCount\n");
    return 0;
}

// returns the steamID of a clan officer, by index, of range [0,GetClanOfficerCount)
CSteamID GetClanOfficerByIndex( CSteamID steamIDClan, int iOfficer )
{
    PRINT_DEBUG("Steam_Friends::GetClanOfficerByIndex\n");
    return k_steamIDNil;
}

// if current user is chat restricted, he can't send or receive any text/voice chat messages.
// the user can't see custom avatars. But the user can be online and send/recv game invites.
// a chat restricted user can't add friends or join any groups.
uint32 GetUserRestrictions()
{
    PRINT_DEBUG("Steam_Friends::GetUserRestrictions\n");
    return k_nUserRestrictionNone;
}

EUserRestriction GetUserRestrictions_old()
{
    PRINT_DEBUG("Steam_Friends::GetUserRestrictions old\n");
    return k_nUserRestrictionNone;
}

// Rich Presence data is automatically shared between friends who are in the same game
// Each user has a set of Key/Value pairs
// Note the following limits: k_cchMaxRichPresenceKeys, k_cchMaxRichPresenceKeyLength, k_cchMaxRichPresenceValueLength
// There are two magic keys:
//		"status"  - a UTF-8 string that will show up in the 'view game info' dialog in the Steam friends list
//		"connect" - a UTF-8 string that contains the command-line for how a friend can connect to a game
// GetFriendRichPresence() returns an empty string "" if no value is set
// SetRichPresence() to a NULL or an empty string deletes the key
// You can iterate the current set of keys for a friend with GetFriendRichPresenceKeyCount()
// and GetFriendRichPresenceKeyByIndex() (typically only used for debugging)
bool SetRichPresence( const char *pchKey, const char *pchValue )
{
    PRINT_DEBUG("Steam_Friends::SetRichPresence %s %s\n", pchKey, pchValue ? pchValue : "NULL");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    if (pchValue) {
        auto prev_value = (*us.mutable_rich_presence()).find(pchKey);
        if (prev_value == (*us.mutable_rich_presence()).end() || prev_value->second != pchValue) {
            (*us.mutable_rich_presence())[pchKey] = pchValue;
            resend_friend_data();
        }
    } else {
        auto to_remove = us.mutable_rich_presence()->find(pchKey);
        if (to_remove != us.mutable_rich_presence()->end()) {
            us.mutable_rich_presence()->erase(to_remove);
            resend_friend_data();
        }
    }

    return true;
}

void ClearRichPresence()
{
    PRINT_DEBUG("Steam_Friends::ClearRichPresence\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    us.mutable_rich_presence()->clear();
    resend_friend_data();
    
}

const char *GetFriendRichPresence( CSteamID steamIDFriend, const char *pchKey )
{
    PRINT_DEBUG("Steam_Friends::GetFriendRichPresence %llu %s\n", steamIDFriend.ConvertToUint64(), pchKey);
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    const char *value = "";

    Friend *f = NULL;
    if (settings->get_local_steam_id() == steamIDFriend) {
        f = &us;
    } else {
        f = find_friend(steamIDFriend);
    }

    if (f && isAppIdCompatible(f)) {
        auto result = f->rich_presence().find(pchKey);
        if (result != f->rich_presence().end()) value = result->second.c_str();
    }

    return value;
}

int GetFriendRichPresenceKeyCount( CSteamID steamIDFriend )
{
    PRINT_DEBUG("Steam_Friends::GetFriendRichPresenceKeyCount\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    int num = 0;

    Friend *f = NULL;
    if (settings->get_local_steam_id() == steamIDFriend) {
        f = &us;
    } else {
        f = find_friend(steamIDFriend);
    }

    if (f && isAppIdCompatible(f)) num = f->rich_presence().size();
    
    return num;
}

const char *GetFriendRichPresenceKeyByIndex( CSteamID steamIDFriend, int iKey )
{
    PRINT_DEBUG("Steam_Friends::GetFriendRichPresenceKeyByIndex\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    const char *key = "";

    Friend *f = NULL;
    if (settings->get_local_steam_id() == steamIDFriend) {
        f = &us;
    } else {
        f = find_friend(steamIDFriend);
    }

    if (f && isAppIdCompatible(f) && f->rich_presence().size() > iKey && iKey >= 0) {
        auto friend_data = f->rich_presence().begin();
        for (int i = 0; i < iKey; ++i) ++friend_data;
        key = friend_data->first.c_str();
    }

    
    return key;
}

// Requests rich presence for a specific user.
void RequestFriendRichPresence( CSteamID steamIDFriend )
{
    PRINT_DEBUG("Steam_Friends::RequestFriendRichPresence\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    Friend *f = find_friend(steamIDFriend);
    if (f) rich_presence_updated(steamIDFriend, settings->get_local_game_id().AppID());
    
}


// rich invite support
// if the target accepts the invite, the pchConnectString gets added to the command-line for launching the game
// if the game is already running, a GameRichPresenceJoinRequested_t callback is posted containing the connect string
// invites can only be sent to friends
bool InviteUserToGame( CSteamID steamIDFriend, const char *pchConnectString )
{
    PRINT_DEBUG("Steam_Friends::InviteUserToGame\n");
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    Friend *f = find_friend(steamIDFriend);
    if (!f) return false;

    Common_Message msg;
    Friend_Messages *friend_messages = new Friend_Messages();
    friend_messages->set_type(Friend_Messages::GAME_INVITE);
    friend_messages->set_connect_str(pchConnectString);
    msg.set_allocated_friend_messages(friend_messages);
    msg.set_source_id(settings->get_local_steam_id().ConvertToUint64());
    msg.set_dest_id(steamIDFriend.ConvertToUint64());
    return network->sendTo(&msg, true);
}


// recently-played-with friends iteration
// this iterates the entire list of users recently played with, across games
// GetFriendCoplayTime() returns as a unix time
int GetCoplayFriendCount()
{
    PRINT_DEBUG("Steam_Friends::GetCoplayFriendCount\n");
    return 0;
}

CSteamID GetCoplayFriend( int iCoplayFriend )
{
    PRINT_DEBUG("Steam_Friends::GetCoplayFriend\n");
    return k_steamIDNil;
}

int GetFriendCoplayTime( CSteamID steamIDFriend )
{
    PRINT_DEBUG("Steam_Friends::GetFriendCoplayTime\n");
    return 0;
}

AppId_t GetFriendCoplayGame( CSteamID steamIDFriend )
{
    PRINT_DEBUG("Steam_Friends::GetFriendCoplayGame\n");
    return 0;
}


// chat interface for games
// this allows in-game access to group (clan) chats from in the game
// the behavior is somewhat sophisticated, because the user may or may not be already in the group chat from outside the game or in the overlay
// use ActivateGameOverlayToUser( "chat", steamIDClan ) to open the in-game overlay version of the chat
STEAM_CALL_RESULT( JoinClanChatRoomCompletionResult_t )
SteamAPICall_t JoinClanChatRoom( CSteamID steamIDClan )
{
    PRINT_DEBUG("Steam_Friends::JoinClanChatRoom %llu\n", steamIDClan.ConvertToUint64());
    //TODO actually join a room
    std::lock_guard<std::recursive_mutex> lock(global_mutex);
    JoinClanChatRoomCompletionResult_t data;
    data.m_steamIDClanChat = steamIDClan;
    data.m_eChatRoomEnterResponse = k_EChatRoomEnterResponseSuccess;
    return callback_results->addCallResult(data.k_iCallback, &data, sizeof(data));
}

bool LeaveClanChatRoom( CSteamID steamIDClan )
{
    PRINT_DEBUG("Steam_Friends::LeaveClanChatRoom\n");
    return false;
}

int GetClanChatMemberCount( CSteamID steamIDClan )
{
    PRINT_DEBUG("Steam_Friends::GetClanChatMemberCount\n");
    return 0;
}

CSteamID GetChatMemberByIndex( CSteamID steamIDClan, int iUser )
{
    PRINT_DEBUG("Steam_Friends::GetChatMemberByIndex\n");
    return k_steamIDNil;
}

bool SendClanChatMessage( CSteamID steamIDClanChat, const char *pchText )
{
    PRINT_DEBUG("Steam_Friends::SendClanChatMessage\n");
    return false;
}

int GetClanChatMessage( CSteamID steamIDClanChat, int iMessage, void *prgchText, int cchTextMax, EChatEntryType *peChatEntryType, STEAM_OUT_STRUCT() CSteamID *psteamidChatter )
{
    PRINT_DEBUG("Steam_Friends::GetClanChatMessage\n");
    return 0;
}

bool IsClanChatAdmin( CSteamID steamIDClanChat, CSteamID steamIDUser )
{
    PRINT_DEBUG("Steam_Friends::IsClanChatAdmin\n");
    return false;
}


// interact with the Steam (game overlay / desktop)
bool IsClanChatWindowOpenInSteam( CSteamID steamIDClanChat )
{
    PRINT_DEBUG("Steam_Friends::IsClanChatWindowOpenInSteam\n");
    return false;
}

bool OpenClanChatWindowInSteam( CSteamID steamIDClanChat )
{
    PRINT_DEBUG("Steam_Friends::OpenClanChatWindowInSteam\n");
    return true;
}

bool CloseClanChatWindowInSteam( CSteamID steamIDClanChat )
{
    PRINT_DEBUG("Steam_Friends::CloseClanChatWindowInSteam\n");
    return true;
}


// peer-to-peer chat interception
// this is so you can show P2P chats inline in the game
bool SetListenForFriendsMessages( bool bInterceptEnabled )
{
    PRINT_DEBUG("Steam_Friends::SetListenForFriendsMessages\n");
    return true;
}

bool ReplyToFriendMessage( CSteamID steamIDFriend, const char *pchMsgToSend )
{
    PRINT_DEBUG("Steam_Friends::ReplyToFriendMessage\n");
    return false;
}

int GetFriendMessage( CSteamID steamIDFriend, int iMessageID, void *pvData, int cubData, EChatEntryType *peChatEntryType )
{
    PRINT_DEBUG("Steam_Friends::GetFriendMessage\n");
    return 0;
}


// following apis
STEAM_CALL_RESULT( FriendsGetFollowerCount_t )
SteamAPICall_t GetFollowerCount( CSteamID steamID )
{
    PRINT_DEBUG("Steam_Friends::GetFollowerCount\n");
    return 0;
}

STEAM_CALL_RESULT( FriendsIsFollowing_t )
SteamAPICall_t IsFollowing( CSteamID steamID )
{
    PRINT_DEBUG("Steam_Friends::IsFollowing\n");
    return 0;
}

STEAM_CALL_RESULT( FriendsEnumerateFollowingList_t )
SteamAPICall_t EnumerateFollowingList( uint32 unStartIndex )
{
    PRINT_DEBUG("Steam_Friends::EnumerateFollowingList\n");
    return 0;
}


bool IsClanPublic( CSteamID steamIDClan )
{
    PRINT_DEBUG("Steam_Friends::IsClanPublic\n");
    return false;
}

bool IsClanOfficialGameGroup( CSteamID steamIDClan )
{
    PRINT_DEBUG("Steam_Friends::IsClanOfficialGameGroup\n");
    return false;
}

int GetNumChatsWithUnreadPriorityMessages()
{
    PRINT_DEBUG("Steam_Friends::GetNumChatsWithUnreadPriorityMessages\n");
    return 0;
}

void ActivateGameOverlayRemotePlayTogetherInviteDialog( CSteamID steamIDLobby )
{
    PRINT_DEBUG("Steam_Friends::ActivateGameOverlayRemotePlayTogetherInviteDialog\n");
}

// Call this before calling ActivateGameOverlayToWebPage() to have the Steam Overlay Browser block navigations
// to your specified protocol (scheme) uris and instead dispatch a OverlayBrowserProtocolNavigation_t callback to your game.
// ActivateGameOverlayToWebPage() must have been called with k_EActivateGameOverlayToWebPageMode_Modal
bool RegisterProtocolInOverlayBrowser( const char *pchProtocol )
{
    PRINT_DEBUG("Steam_Friends::RegisterProtocolInOverlayBrowser\n");
    return false;
}

// Activates the game overlay to open an invite dialog that will send the provided Rich Presence connect string to selected friends
void ActivateGameOverlayInviteDialogConnectString( const char *pchConnectString )
{
    PRINT_DEBUG("Steam_Friends::ActivateGameOverlayInviteDialogConnectString\n");
}

// Steam Community items equipped by a user on their profile
// You can register for EquippedProfileItemsChanged_t to know when a friend has changed their equipped profile items
STEAM_CALL_RESULT( EquippedProfileItems_t )
SteamAPICall_t RequestEquippedProfileItems( CSteamID steamID )
{
    PRINT_DEBUG("Steam_Friends::RequestEquippedProfileItems\n");
    return 0;
}

bool BHasEquippedProfileItem( CSteamID steamID, ECommunityProfileItemType itemType )
{
    PRINT_DEBUG("Steam_Friends::BHasEquippedProfileItem\n");
    return false;
}

const char *GetProfileItemPropertyString( CSteamID steamID, ECommunityProfileItemType itemType, ECommunityProfileItemProperty prop )
{
    PRINT_DEBUG("Steam_Friends::GetProfileItemPropertyString\n");
    return "";
}

uint32 GetProfileItemPropertyUint( CSteamID steamID, ECommunityProfileItemType itemType, ECommunityProfileItemProperty prop )
{
    PRINT_DEBUG("Steam_Friends::GetProfileItemPropertyUint\n");
    return 0;
}

void RunCallbacks()
{
	PRINT_DEBUG("Steam_Friends::RunCallbacks\n");
    if (settings->get_lobby() != lobby_id) {
        lobby_id = settings->get_lobby();
        resend_friend_data();
    }

    if (modified) {
        Common_Message msg;
        msg.set_source_id(settings->get_local_steam_id().ConvertToUint64());
        Friend *f = new Friend(us);
        f->set_id(settings->get_local_steam_id().ConvertToUint64());
        f->set_name(settings->get_local_name());
        f->set_appid(settings->get_local_game_id().AppID());
        f->set_lobby_id(settings->get_lobby().ConvertToUint64());
        msg.set_allocated_friend_(f);
        network->sendToAllIndividuals(&msg, true);
        modified = false;
        last_sent_friends = std::chrono::high_resolution_clock::now();
    }
}

void Callback(Common_Message *msg)
{
    if (msg->has_low_level()) {
        if (msg->low_level().type() == Low_Level::DISCONNECT) {
            PRINT_DEBUG("Steam_Friends Disconnect\n");
            uint64 id = msg->source_id();
            auto f = std::find_if(friends.begin(), friends.end(), [&id](Friend const& item) { return item.id() == id; });
            if (friends.end() != f) {
                persona_change((uint64)f->id(), k_EPersonaChangeStatus);
                overlay->FriendDisconnect(*f);
                friends.erase(f);
            }
        }

        if (msg->low_level().type() == Low_Level::CONNECT) {
            PRINT_DEBUG("Steam_Friends Connect\n");
            Common_Message msg_;
            msg_.set_source_id(settings->get_local_steam_id().ConvertToUint64());
            msg_.set_dest_id(msg->source_id());
            Friend *f = new Friend(us);
            f->set_id(settings->get_local_steam_id().ConvertToUint64());
            f->set_name(settings->get_local_name());
            f->set_appid(settings->get_local_game_id().AppID());
            f->set_lobby_id(settings->get_lobby().ConvertToUint64());
            int avatar_number = GetLargeFriendAvatar(settings->get_local_steam_id());
            if (settings->images[avatar_number].data.length() > 0) f->set_avatar(settings->images[avatar_number].data);
            else f->set_avatar("");
            msg_.set_allocated_friend_(f);
            network->sendTo(&msg_, true);
        }
    }

    if (msg->has_friend_()) {
        PRINT_DEBUG("Steam_Friends Friend " "%" PRIu64 " " "%" PRIu64 "\n", msg->friend_().id(), msg->friend_().lobby_id());
        Friend *f = find_friend((uint64)msg->friend_().id());
        if (!f) {
            if (msg->friend_().id() != settings->get_local_steam_id().ConvertToUint64()) {
                friends.push_back(msg->friend_());
                overlay->FriendConnect(msg->friend_());
                persona_change((uint64)msg->friend_().id(), k_EPersonaChangeName);
                GetLargeFriendAvatar((uint64)msg->friend_().id());
            }
        } else {
            std::map<std::string, std::string> map1(f->rich_presence().begin(), f->rich_presence().end());
            std::map<std::string, std::string> map2(msg->friend_().rich_presence().begin(), msg->friend_().rich_presence().end());

            if (map1 != map2) {
                //The App ID of the game. This should always be the current game.
                if (isAppIdCompatible(f)) {
                    rich_presence_updated((uint64)msg->friend_().id(), (uint64)msg->friend_().appid());
                }
            }
            //TODO: callbacks?
            *f = msg->friend_();
        }
    }

    if (msg->has_friend_messages()) {
        if (msg->friend_messages().type() == Friend_Messages::LOBBY_INVITE) {
            PRINT_DEBUG("Steam_Friends Got Lobby Invite\n");
            Friend *f = find_friend((uint64)msg->source_id());
            if (f) {
                LobbyInvite_t data;
                data.m_ulSteamIDUser = msg->source_id();
                data.m_ulSteamIDLobby = msg->friend_messages().lobby_id();
                data.m_ulGameID = f->appid();
                callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));

                if (overlay->Ready() && !settings->hasOverlayAutoAcceptInviteFromFriend(msg->source_id()))
                {
                    //TODO: the user should accept the invite first but we auto accept it because there's no gui yet
                    // Then we will handle it !
                    overlay->SetLobbyInvite(*find_friend(static_cast<uint64>(msg->source_id())), msg->friend_messages().lobby_id());
                }
                else
                {
                    GameLobbyJoinRequested_t data;
                    data.m_steamIDLobby = CSteamID((uint64)msg->friend_messages().lobby_id());
                    data.m_steamIDFriend = CSteamID((uint64)msg->source_id());
                    callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
                }
            }
        }

        if (msg->friend_messages().type() == Friend_Messages::GAME_INVITE) {
            PRINT_DEBUG("Steam_Friends Got Game Invite\n");
            //TODO: I'm pretty sure that the user should accept the invite before this is posted but we do like above
            if (overlay->Ready() && !settings->hasOverlayAutoAcceptInviteFromFriend(msg->source_id()))
            {
                // Then we will handle it !
                overlay->SetRichInvite(*find_friend(static_cast<uint64>(msg->source_id())), msg->friend_messages().connect_str().c_str());
            }
            else
            {
                std::string const& connect_str = msg->friend_messages().connect_str();
                GameRichPresenceJoinRequested_t data = {};
                data.m_steamIDFriend = CSteamID((uint64)msg->source_id());
                strncpy(data.m_rgchConnect, connect_str.c_str(), k_cchMaxRichPresenceValueLength - 1);
                callbacks->addCBResult(data.k_iCallback, &data, sizeof(data));
            }
        }
    }
}

};

#endif//__INCLUDED_STEAM_FRIENDS_H__
