#include <regex>
#include <string>
#include <fstream>
#include <streambuf>
#include <iostream>

// these are defined in dll.cpp at the top like this:
// static char old_xxx[128] = ...

const static std::vector<std::string> interface_patterns = {
    R"(SteamClient\d+)",
    
    R"(SteamGameServerStats\d+)",
    R"(SteamGameServer\d+)",

    R"(SteamMatchMakingServers\d+)",
    R"(SteamMatchMaking\d+)",

    R"(SteamUser\d+)",
    R"(SteamFriends\d+)",
    R"(SteamUtils\d+)",
    R"(STEAMUSERSTATS_INTERFACE_VERSION\d+)",
    R"(STEAMAPPS_INTERFACE_VERSION\d+)",
    R"(SteamNetworking\d+)",
    R"(STEAMREMOTESTORAGE_INTERFACE_VERSION\d+)",
    R"(STEAMSCREENSHOTS_INTERFACE_VERSION\d+)",
    R"(STEAMHTTP_INTERFACE_VERSION\d+)",
    R"(STEAMUNIFIEDMESSAGES_INTERFACE_VERSION\d+)",
    
    R"(STEAMCONTROLLER_INTERFACE_VERSION\d+)",
    R"(SteamController\d+)",
    
    R"(STEAMUGC_INTERFACE_VERSION\d+)",
    R"(STEAMAPPLIST_INTERFACE_VERSION\d+)",
    R"(STEAMMUSIC_INTERFACE_VERSION\d+)",
    R"(STEAMMUSICREMOTE_INTERFACE_VERSION\d+)",
    R"(STEAMHTMLSURFACE_INTERFACE_VERSION_\d+)",
    R"(STEAMINVENTORY_INTERFACE_V\d+)",
    R"(STEAMVIDEO_INTERFACE_V\d+)",
    R"(SteamMasterServerUpdater\d+)",
};

unsigned int findinterface(
    std::ofstream &out_file,
    const std::string &file_contents,
    const std::string &interface_patt)
{
    std::regex interface_regex(interface_patt);
    auto begin = std::sregex_iterator(file_contents.begin(), file_contents.end(), interface_regex);
    auto end = std::sregex_iterator();

    unsigned int matches = 0;
    for (std::sregex_iterator i = begin; i != end; ++i) {
        std::smatch match = *i;
        std::string match_str = match.str();
        out_file << match_str << std::endl;
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

    if (steam_api_contents.size() == 0) {
        std::cerr << "Error loading data" << std::endl;
        return 1;
    }

    unsigned int total_matches = 0;
    std::ofstream out_file("steam_interfaces.txt");
    if (!out_file.is_open()) {
        std::cerr << "Error opening output file" << std::endl;
        return 1;
    }

    for (const auto &patt : interface_patterns) {
        total_matches += findinterface(out_file, steam_api_contents, patt);
    }
    out_file.close();

    if (total_matches == 0) {
        std::cerr << "No interfaces were found" << std::endl;
        return 1;
    }

    return 0;
}
