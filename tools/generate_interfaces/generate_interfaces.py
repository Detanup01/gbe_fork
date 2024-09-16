import re
import sys

interface_patterns = [
    r"SteamClient\d+",

    r"SteamGameServerStats\d+",
    r"SteamGameServer\d+",

    r"SteamMatchMakingServers\d+",
    r"SteamMatchMaking\d+",

    r"SteamUser\d+",
    r"SteamFriends\d+",
    r"SteamUtils\d+",
    r"STEAMUSERSTATS_INTERFACE_VERSION\d+",
    r"STEAMAPPS_INTERFACE_VERSION\d+",
    r"SteamNetworking\d+",
    r"STEAMREMOTESTORAGE_INTERFACE_VERSION\d+",
    r"STEAMSCREENSHOTS_INTERFACE_VERSION\d+",
    r"STEAMHTTP_INTERFACE_VERSION\d+",
    r"STEAMUNIFIEDMESSAGES_INTERFACE_VERSION\d+",

    r"STEAMCONTROLLER_INTERFACE_VERSION",
    r"SteamController\d+",

    r"STEAMUGC_INTERFACE_VERSION\d+",
    r"STEAMAPPLIST_INTERFACE_VERSION\d+",
    r"STEAMMUSIC_INTERFACE_VERSION\d+",
    r"STEAMMUSICREMOTE_INTERFACE_VERSION\d+",
    r"STEAMHTMLSURFACE_INTERFACE_VERSION_\d+",
    r"STEAMINVENTORY_INTERFACE_V\d+",
    r"STEAMVIDEO_INTERFACE_V\d+",
    r"SteamMasterServerUpdater\d+",
]

def findinterface(file_contents, interface_patt):
    interface_regex = re.compile(interface_patt)
    matches = interface_regex.findall(file_contents)
    return matches

def main():
    if len(sys.argv) < 2:
        print(f"usage: {sys.argv[0]} <path to steam_api .dll or .so>")
        return 1

    try:
        with open(sys.argv[1], 'rb') as steam_api_file:
            steam_api_contents = steam_api_file.read().decode('utf-8', errors='ignore')
    except IOError:
        print(f"Error opening file: {sys.argv[1]}")
        return 1

    if not steam_api_contents:
        print("Error loading data")
        return 1

    interfaces = []
    for pattern in interface_patterns:
        interfaces += findinterface(steam_api_contents, pattern)
    interfaces = sorted(set(interfaces))

    if not interfaces:
        print("No interfaces were found")
        return 1

    try:
        with open("steam_interfaces.txt", "w") as out_file:
            for interface in interfaces:
                out_file.write(f"{interface}\n")
    except IOError:
        print("Error opening output file")
        return 1

if __name__ == "__main__":
    sys.exit(main())
