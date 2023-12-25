#include <regex>
#include <string>
#include <fstream>
#include <streambuf>
#include <iostream>


unsigned int findinterface(std::ofstream &out_file, std::string &file_contents, std::string interface)
{
    std::regex interface_regex(interface);
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
        std::cout << "usage: " << argv[0] << " <path to steam_api .dll or .so>" << std::endl;
        return 1;
    }

    std::ifstream steam_api_file(argv[1], std::ios::binary);
    std::string steam_api_contents((std::istreambuf_iterator<char>(steam_api_file)),
                    std::istreambuf_iterator<char>());
    steam_api_file.close();

    if (steam_api_contents.size() == 0) {
        std::cout << "Error opening file" << std::endl;
        return 1;
    }

    std::ofstream out_file("steam_interfaces.txt");
    std::vector<std::string> interface_names = {
        "SteamClient",
        "SteamGameServer",
        "SteamGameServerStats",
        "SteamUser",
        "SteamFriends",
        "SteamUtils",
        "SteamMatchMaking",
        "SteamMatchMakingServers",
        "STEAMUSERSTATS_INTERFACE_VERSION",
        "STEAMAPPS_INTERFACE_VERSION",
        "SteamNetworking",
        "STEAMREMOTESTORAGE_INTERFACE_VERSION",
        "STEAMSCREENSHOTS_INTERFACE_VERSION",
        "STEAMHTTP_INTERFACE_VERSION",
        "STEAMUNIFIEDMESSAGES_INTERFACE_VERSION",
        "SteamController",
        "STEAMUGC_INTERFACE_VERSION",
        "STEAMAPPLIST_INTERFACE_VERSION",
        "STEAMMUSIC_INTERFACE_VERSION",
        "STEAMMUSICREMOTE_INTERFACE_VERSION",
        "STEAMHTMLSURFACE_INTERFACE_VERSION_",
        "STEAMINVENTORY_INTERFACE_V",
        "STEAMVIDEO_INTERFACE_V",
        "SteamMasterServerUpdater",
        "STEAMPARENTALSETTINGS_INTERFACE_VERSION",
        "STEAMREMOTEPLAY_INTERFACE_VERSION",
        "STEAMTV_INTERFACE_V",
        "SteamInput",
        "SteamMatchGameSearch",
        "SteamNetworkingSockets",
        "SteamNetworkingUtils",
        "SteamParties",
    };

    for (auto name : interface_names) {
        findinterface(out_file, steam_api_contents, name + "\\d{3}");
    }

    if (!findinterface(out_file, steam_api_contents, "STEAMCONTROLLER_INTERFACE_VERSION\\d{3}")) {
        findinterface(out_file, steam_api_contents, "STEAMCONTROLLER_INTERFACE_VERSION");
    }

    out_file.close();
}
