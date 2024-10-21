#include <regex>
#include <string>
#include <fstream>
#include <iostream>
#include <filesystem>

// these are defined in dll.cpp at the top like this:
// static char old_xxx[128] = ...
const static std::vector<std::string> interface_patterns = {
    R"(SteamAppDisableUpdate\d+)",
    R"(STEAMAPPLIST_INTERFACE_VERSION\d+)",
    R"(STEAMAPPS_INTERFACE_VERSION\d+)",
    R"(STEAMAPPTICKET_INTERFACE_VERSION\d+)",
    R"(SteamClient\d+)",
    R"(STEAMCONTROLLER_INTERFACE_VERSION)",
    R"(SteamController\d+)",
    R"(SteamFriends\d+)",
    R"(SteamGameCoordinator\d+)",
    R"(SteamGameServerStats\d+)",
    R"(SteamGameServer\d+)",
    R"(SteamGameStats\d+)",
    R"(STEAMHTMLSURFACE_INTERFACE_VERSION_\d+)",
    R"(STEAMHTTP_INTERFACE_VERSION\d+)",
    R"(SteamInput\d+)",
    R"(STEAMINVENTORY_INTERFACE_V\d+)",
    R"(SteamMasterServerUpdater\d+)",
    R"(SteamMatchGameSearch\d+)",
    R"(SteamMatchMakingServers\d+)",
    R"(SteamMatchMaking\d+)",
    R"(STEAMMUSIC_INTERFACE_VERSION\d+)",
    R"(STEAMMUSICREMOTE_INTERFACE_VERSION\d+)",
    R"(SteamNetworkingMessages\d+)",
    R"(SteamNetworkingSocketsSerialized\d+)",
    R"(SteamNetworkingSockets\d+)",
    R"(SteamNetworkingUtils\d+)",
    R"(SteamNetworking\d+)",
    R"(STEAMPARENTALSETTINGS_INTERFACE_VERSION\d+)",
    R"(SteamParties\d+)",
    R"(STEAMREMOTEPLAY_INTERFACE_VERSION\d+)",
    R"(STEAMREMOTESTORAGE_INTERFACE_VERSION\d+)",
    R"(STEAMSCREENSHOTS_INTERFACE_VERSION\d+)",
    R"(STEAMTIMELINE_INTERFACE_V\d+)",
    R"(STEAMTV_INTERFACE_V\d+)",
    R"(STEAMUGC_INTERFACE_VERSION\d+)",
    R"(STEAMUNIFIEDMESSAGES_INTERFACE_VERSION\d+)",
    R"(STEAMUSERSTATS_INTERFACE_VERSION\d+)",
    R"(SteamUser\d+)",
    R"(SteamUtils\d+)",
    R"(STEAMVIDEO_INTERFACE_V\d+)"
};

unsigned int findinterface(
    std::ofstream &out_file,
    const std::string &file_contents,
    const std::string &interface_patt)
{
    std::regex interface_regex(interface_patt);

    auto begin = std::sregex_iterator(file_contents.cbegin(), file_contents.cend(), interface_regex);
    auto end = std::sregex_iterator();

    unsigned int matches = 0;

    for (std::sregex_iterator i = begin; i != end; ++i)
    {
        out_file << i->str() << std::endl;
        ++matches;
    }

    return matches;
}

int main(int argc, char *argv[])
{
    std::cout << "----Generate Interfaces----" << std::endl << std::endl;

    if (argc < 2) 
    {
        std::cerr << "Usage: " << argv[0] << " <path to steam_api .dll or .so>" << std::endl;

        return 1;
    }

    std::ifstream steam_api_file(std::filesystem::u8path(argv[1]), std::ios::binary);
    if (!steam_api_file)
    {
        std::cerr << "Error opening file: " << argv[1] << std::endl;
        return 1;
    }

    std::string steam_api_contents((std::istreambuf_iterator<char>(steam_api_file)), std::istreambuf_iterator<char>());

    steam_api_file.close();

    if (steam_api_contents.empty())
    {
        std::cerr << "Error loading data from file: " << argv[1] << std::endl;
        return 1;
    }

    std::ofstream out_file("steam_interfaces.txt");
    if (!out_file)
    {

        std::cerr << "Error opening output file: steam_interfaces.txt" << std::endl;
        return 1;
    }

    unsigned int total_matches = 0;

    for (const auto &patt : interface_patterns)
    {
        total_matches += findinterface(out_file, steam_api_contents, patt);
        std::cout << "Searching for '" + patt + "'..." << std::endl;
    }

    out_file.close();

    if (total_matches == 0)
    {
        std::cerr << "No interfaces were found" << std::endl;
        return 1;
    }

    return 0;
}
