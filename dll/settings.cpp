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

#include "dll/settings.h"
#include "dll/steam_app_ids.h"


std::string Settings::sanitize(const std::string &name)
{
    // https://github.com/microsoft/referencesource/blob/51cf7850defa8a17d815b4700b67116e3fa283c2/mscorlib/system/io/path.cs#L88C9-L89C1
    // https://github.com/microsoft/referencesource/blob/51cf7850defa8a17d815b4700b67116e3fa283c2/mscorlib/system/io/pathinternal.cs#L32
    constexpr const static char InvalidFileNameChars[] = {
        '\"', '<', '>', '|', '\0',
        (char)1, (char)2, (char)3, (char)4, (char)5, (char)6, (char)7, (char)8, (char)9, (char)10,
        (char)11, (char)12, (char)13, (char)14, (char)15, (char)16, (char)17, (char)18, (char)19, (char)20,
        (char)21, (char)22, (char)23, (char)24, (char)25, (char)26, (char)27, (char)28, (char)29, (char)30,
        (char)31,
        ':', '*', '?', /*'\\', '/',*/
    };

    if (name.empty()) return {};

    // we have to use utf-32 because Windows (and probably Linux) allows some chars that need at least 32 bits,
    // such as this one (U+1F5FA) called "World Map": https://www.compart.com/en/unicode/U+1F5FA
    // utf-16 encoding for these characters require 2 ushort, but we would like to iterate
    // over all chars in a linear fashion
    std::u32string unicode_name{};
    utf8::utf8to32(
        name.begin(),
        utf8::find_invalid(name.begin(), name.end()), // returns an iterator pointing to the first invalid octet
        std::back_inserter(unicode_name)
    );
    
    auto rm_itr = std::remove_if(unicode_name.begin(), unicode_name.end(), [](decltype(unicode_name[0]) ch) {
        return ch == '\n' || ch == '\r';
    });
    if (unicode_name.end() != rm_itr) {
        unicode_name.erase(rm_itr, unicode_name.end());
    }

    auto InvalidFileNameChars_last_it = std::end(InvalidFileNameChars);
    for (auto& uch : unicode_name) {
        auto found_it = std::find(std::begin(InvalidFileNameChars), InvalidFileNameChars_last_it, uch);
        if (found_it != InvalidFileNameChars_last_it) { // if illegal
            uch = ' ';
        }
    }

    std::string res{};
    utf8::utf32to8(unicode_name.begin(), unicode_name.end(), std::back_inserter(res));
    return res;
}

Settings::Settings(CSteamID steam_id, CGameID game_id, const std::string &name, const std::string &language, bool offline)
{
    this->steam_id = steam_id;
    this->game_id = game_id;
    this->name = sanitize(name);
    if (this->name.size() == 0) {
        this->name = "  ";
    }

    if (this->name.size() == 1) {
        this->name = this->name + " ";
    }

    auto lang = sanitize(language);
    std::transform(lang.begin(), lang.end(), lang.begin(), ::tolower);
    lang.erase(std::remove(lang.begin(), lang.end(), ' '), lang.end());
    this->language = lang;

    this->offline = offline;
}

// user id
CSteamID Settings::get_local_steam_id()
{
    return steam_id;
}

// game id
CGameID Settings::get_local_game_id()
{
    return game_id;
}

const char *Settings::get_local_name()
{
    return name.c_str();
}

const char *Settings::get_language()
{
    return language.c_str();
}

void Settings::set_local_name(const char *name)
{
    this->name = name;
}

void Settings::set_language(const char *language)
{
    this->language = language;
}

void Settings::set_supported_languages(const std::set<std::string> &langs)
{
    this->supported_languages_set = langs;
    
    this->supported_languages.clear();
    auto lang_it = langs.cbegin();
    while (langs.cend() != lang_it) {
        if (langs.cbegin() == lang_it) this->supported_languages = *lang_it;
        else this->supported_languages.append(",").append(*lang_it); // this isn't C#, .append() will change the string!
        ++lang_it;
    }
}

const std::set<std::string>& Settings::get_supported_languages_set() const
{
    return this->supported_languages_set;
}

const std::string& Settings::get_supported_languages() const
{
    return this->supported_languages;
}

void Settings::set_game_id(CGameID game_id)
{
    this->game_id = game_id;
}

void Settings::set_lobby(CSteamID lobby_id)
{
    this->lobby_id = lobby_id;
}

CSteamID Settings::get_lobby()
{
    return this->lobby_id;
}

bool Settings::is_offline()
{
    return offline;
}

void Settings::set_offline(bool offline)
{
    this->offline = offline;
}

uint16 Settings::get_port()
{
    return port;
}

void Settings::set_port(uint16 port)
{
    this->port = port;
}

void Settings::addMod(PublishedFileId_t id, const std::string &title, const std::string &path)
{
    auto f = std::find_if(mods.begin(), mods.end(), [&id](Mod_entry const& item) { return item.id == id; });
    if (mods.end() != f) {
        f->title = title;
        f->path = path;
        return;
    }

    Mod_entry new_entry{};
    new_entry.id = id;
    new_entry.title = title;
    new_entry.path = path;
    mods.push_back(new_entry);
}

void Settings::addModDetails(PublishedFileId_t id, const Mod_entry &details)
{
    auto f = std::find_if(mods.begin(), mods.end(), [&id](Mod_entry const& item) { return item.id == id; });
    if (f != mods.end()) {
        // don't copy files handles, they're auto generated
        
        f->fileType = details.fileType;
        f->description = details.description;
        f->steamIDOwner = details.steamIDOwner;
        f->timeCreated = details.timeCreated;
        f->timeUpdated = details.timeUpdated;
        f->timeAddedToUserList = details.timeAddedToUserList;
        f->visibility = details.visibility;
        f->banned = details.banned;
        f->acceptedForUse = details.acceptedForUse;
        f->tagsTruncated = details.tagsTruncated;
        f->tags = details.tags;
        f->primaryFileName = details.primaryFileName;
        f->primaryFileSize = details.primaryFileSize;
        f->previewFileName = details.previewFileName;
        f->previewFileSize = details.previewFileSize;
        f->workshopItemURL = details.workshopItemURL;
        f->votesUp = details.votesUp;
        f->votesDown = details.votesDown;
        f->score = details.score;
        f->numChildren = details.numChildren;
        f->previewURL = details.previewURL;
        f->total_files_sizes = details.total_files_sizes;
        f->min_game_branch = details.min_game_branch;
        f->max_game_branch = details.max_game_branch;
    }
}

Mod_entry Settings::getMod(PublishedFileId_t id)
{
    auto f = std::find_if(mods.begin(), mods.end(), [&id](Mod_entry const& item) { return item.id == id; });
    if (mods.end() != f) {
        return *f;
    }

    return Mod_entry();
}

bool Settings::isModInstalled(PublishedFileId_t id)
{
    auto f = std::find_if(mods.begin(), mods.end(), [&id](Mod_entry const& item) { return item.id == id; });
    if (mods.end() != f) {
        return true;
    }

    return false;
}

std::set<PublishedFileId_t> Settings::modSet()
{
    std::set<PublishedFileId_t> ret_set;

    for (auto & m: mods) {
        ret_set.insert(m.id);
    }

    return ret_set;
}

void Settings::unlockAllDLC(bool value)
{
    this->unlockAllDLCs = value;
}

void Settings::addDLC(AppId_t appID, std::string name, bool available)
{
    auto f = std::find_if(DLCs.begin(), DLCs.end(), [&appID](DLC_entry const& item) { return item.appID == appID; });
    if (DLCs.end() != f) {
        f->name = name;
        f->available = available;
        return;
    }

    DLC_entry new_entry{};
    new_entry.appID = appID;
    new_entry.name = name;
    new_entry.available = available;
    DLCs.push_back(new_entry);
}

unsigned int Settings::DLCCount() const
{
    return static_cast<unsigned int>(this->DLCs.size());
}

bool Settings::hasDLC(AppId_t appID)
{
    if (this->unlockAllDLCs) return true;

    auto f = std::find_if(DLCs.begin(), DLCs.end(), [&appID](DLC_entry const& item) { return item.appID == appID; });
    if (DLCs.end() != f) return f->available;

    if (enable_builtin_preowned_ids && steam_preowned_app_ids.count(appID)) return true;

    return false;
}

bool Settings::getDLC(unsigned int index, AppId_t &appID, bool &available, std::string &name)
{
    if (index >= DLCs.size()) return false;

    appID = DLCs[index].appID;
    available = DLCs[index].available;
    name = DLCs[index].name;
    return true;
}

void Settings::assumeAnyAppInstalled(bool val)
{
    assume_any_app_installed = val;
}

void Settings::addInstalledApp(AppId_t appID)
{
    installed_app_ids.insert(appID);
}

bool Settings::isAppInstalled(AppId_t appID) const
{
    if (assume_any_app_installed) return true;
    if (installed_app_ids.count(appID)) return true;
    if (enable_builtin_preowned_ids && steam_preowned_app_ids.count(appID)) return true;

    return false;
}

void Settings::setAppInstallPath(AppId_t appID, const std::string &path)
{
    app_paths[appID] = path;
}

std::string Settings::getAppInstallPath(AppId_t appID)
{
    return app_paths[appID];
}

void Settings::setLeaderboard(const std::string &leaderboard, enum ELeaderboardSortMethod sort_method, enum ELeaderboardDisplayType display_type)
{
    Leaderboard_config leader{};
    leader.sort_method = sort_method;
    leader.display_type = display_type;

    leaderboards[leaderboard] = leader;
}

const std::map<std::string, Leaderboard_config>& Settings::getLeaderboards() const
{
    return leaderboards;
}

const std::map<std::string, Stat_config>& Settings::getStats() const
{
    return stats;
}

std::map<std::string, Stat_config>::const_iterator Settings::setStatDefiniton(const std::string &name, const struct Stat_config &stat_config)
{
    auto ins_it = stats.insert_or_assign(common_helpers::to_lower(name), stat_config);
    return ins_it.first;
}


int Settings::add_image(const std::string &data, uint32 width, uint32 height)
{
    auto previous_it = std::find_if(images.begin(), images.end(), [&](const std::pair<const size_t, Image_Data> &item) {
        return item.second.data == data
            && item.second.height == height
            && item.second.width == width;
    });
    if (images.end() != previous_it) {
        return static_cast<int>(previous_it->first);
    }

    struct Image_Data dt{};
    dt.width = width;
    dt.height = height;
    dt.data = data;
    
    auto new_handle = images.size() + 1; // never return 0, it is a bad handle for most ISteamUserStats APIs
    images[new_handle] = dt;

    return static_cast<int>(new_handle);
}

Image_Data* Settings::get_image(int handle)
{
    if (INVALID_IMAGE_HANDLE == handle || UNLOADED_IMAGE_HANDLE == handle) {
        return nullptr;
    }
    
    auto image_it = images.find(handle);
    if (images.end() == image_it) {
        return nullptr;
    }
    return &image_it->second;
}


void Settings::acceptAnyOverlayInvites(bool value)
{
    auto_accept_any_overlay_invites = value;
}

void Settings::addFriendToOverlayAutoAccept(uint64_t friend_id)
{
    auto_accept_overlay_invites_friends.insert(friend_id);
}

bool Settings::hasOverlayAutoAcceptInviteFromFriend(uint64_t friend_id) const
{
    if (auto_accept_any_overlay_invites) {
        return true;
    }
    return !!auto_accept_overlay_invites_friends.count(friend_id);
}

size_t Settings::overlayAutoAcceptInvitesCount() const
{
    return auto_accept_overlay_invites_friends.size();
}
