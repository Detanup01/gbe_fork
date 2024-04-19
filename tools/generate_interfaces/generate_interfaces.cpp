#include <regex>
#include <string>
#include <fstream>
#include <streambuf>
#include <iostream>

// these are defined in dll.cpp at the top like this:
// static char old_xxx[128] = ...

const static std::vector<std::pair<std::string, std::string>> interface_patterns = {
    { R"(SteamClient\d+)", "client" },

    { R"(SteamGameServerStats\d+)", "gameserver_stats" },
    { R"(SteamGameServer\d+)", "gameserver" },

    { R"(SteamMatchMakingServers\d+)", "matchmaking_servers" },
    { R"(SteamMatchMaking\d+)", "matchmaking" },

    { R"(SteamUser\d+)", "user" },
    { R"(SteamFriends\d+)", "friends" },
    { R"(SteamUtils\d+)", "utils" },
    { R"(STEAMUSERSTATS_INTERFACE_VERSION\d+)", "user_stats" },
    { R"(STEAMAPPS_INTERFACE_VERSION\d+)", "apps" },
    { R"(SteamNetworking\d+)", "networking" },
    { R"(STEAMREMOTESTORAGE_INTERFACE_VERSION\d+)", "remote_storage" },
    { R"(STEAMSCREENSHOTS_INTERFACE_VERSION\d+)", "screenshots" },
    { R"(STEAMHTTP_INTERFACE_VERSION\d+)", "http" },
    { R"(STEAMUNIFIEDMESSAGES_INTERFACE_VERSION\d+)", "unified_messages" },

    { R"(STEAMCONTROLLER_INTERFACE_VERSION\d+)", "controller" },
    { R"(SteamController\d+)", "controller" },

    { R"(STEAMUGC_INTERFACE_VERSION\d+)", "ugc" },
    { R"(STEAMAPPLIST_INTERFACE_VERSION\d+)", "applist" },
    { R"(STEAMMUSIC_INTERFACE_VERSION\d+)", "music" },
    { R"(STEAMMUSICREMOTE_INTERFACE_VERSION\d+)", "music_remote" },
    { R"(STEAMHTMLSURFACE_INTERFACE_VERSION_\d+)", "html_surface" },
    { R"(STEAMINVENTORY_INTERFACE_V\d+)", "inventory" },
    { R"(STEAMVIDEO_INTERFACE_V\d+)", "video" },
    { R"(SteamMasterServerUpdater\d+)", "masterserver_updater" },
};

unsigned int findinterface(std::ofstream &out_file, const std::string &file_contents, const std::pair<std::string, std::string> &interface_patt)
{
    std::regex interface_regex(interface_patt.first);
    auto begin = std::sregex_iterator(file_contents.begin(), file_contents.end(), interface_regex);
    auto end = std::sregex_iterator();

    unsigned int matches = 0;
    for (std::sregex_iterator itr = begin; itr != end; ++itr) {
        out_file << interface_patt.second << "=" << itr->str() << std::endl;
        ++matches;
    }

    return matches;
}

int main (int argc, char *argv[])
{
    if (argc < 2) {
        std::cerr << "usage: " << argv[0] << " <path to steam_api .dll or .so>" << std::endl;
        return 1;
    }

    std::ifstream steam_api_file(argv[1], std::ios::binary);
    if (!steam_api_file.is_open()) {
        std::cerr << "Error opening file" << std::endl;
        return 1;
    }

    std::string steam_api_contents(
        (std::istreambuf_iterator<char>(steam_api_file)),
        std::istreambuf_iterator<char>());
    steam_api_file.close();

    if (steam_api_contents.empty()) {
        std::cerr << "Error loading data" << std::endl;
        return 1;
    }

    unsigned int total_matches = 0;
    std::ofstream out_file("configs.app.ini");
    if (!out_file.is_open()) {
        std::cerr << "Error opening output file" << std::endl;
        return 1;
    }

    out_file << "[app::steam_interfaces]" << std::endl;
    for (const auto &patt : interface_patterns) {
        total_matches += findinterface(out_file, steam_api_contents, patt);
    }
    out_file << std::endl;
    out_file.close();

    if (total_matches == 0) {
        std::cerr << "No interfaces were found" << std::endl;
        return 1;
    }

    return 0;
}
