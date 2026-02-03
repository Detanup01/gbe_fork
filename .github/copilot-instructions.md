# Goldberg Steam Emulator (GBE) Fork - Copilot Instructions

## Project Overview

This is a fork of the Goldberg Steam Emulator, which emulates Steam API functionality to enable LAN multiplayer without Steam. The emulator implements all major Steam API interfaces and supports both Windows and Linux platforms.

**Key capabilities:**
- Full Steam API emulation (steam_api.dll/libsteam_api.so)
- LAN multiplayer support without Steam client
- Experimental overlay with ImGui (based on ingame_overlay)
- Controller support (XInput emulation for SteamInput/SteamController)
- Configuration-driven features via steam_settings folder

## Build System

### Dependencies Setup (One-time)

Before building the emulator, build third-party dependencies:

**Windows (Visual Studio 2022):**
```batch
set "CMAKE_GENERATOR=Visual Studio 17 2022"
third-party\common\win\premake\premake5.exe --file=premake5-deps.lua --64-build --32-build --all-ext --all-build --verbose --os=windows vs2022
```

**Linux:**
```bash
export CMAKE_GENERATOR="Unix Makefiles"
./third-party/common/linux/premake/premake5 --file=premake5-deps.lua --64-build --32-build --all-ext --all-build --verbose --os=linux gmake2
```

### Building with xmake

The project uses **xmake** as its build system.

**Interactive build script (recommended):**
```powershell
.\build.ps1
```

This provides a TUI menu to:
1. Install dependencies (runs setup_deps.ps1 + xmake packages)
2. Generate protobuf files
3. Build specific targets
4. Configure architecture (x64/x86) and mode (debug/release)

**Command-line build:**
```bash
# Configure
xmake f -p windows -a x64 -m release -c -y

# Generate protobuf files (required before first build)
xmake f --genproto=true

# Build all targets
xmake build

# Build specific target
xmake build api_regular
```

### Running Tests

No automated test suite exists. Testing is manual via game integration.

## Architecture

### Core Components

The emulator is structured around implementing Steam API interfaces:

1. **src/core/** - Main implementation of Steam interfaces
   - Each `steam_*.cpp` file implements one or more Steam API interfaces (ISteamUser, ISteamFriends, etc.)
   - `base.cpp` - Core initialization, global mutex, random number generation
   - `settings.cpp` - Configuration loading from steam_settings folder
   - `network.cpp` - LAN networking for multiplayer
   - `local_storage.cpp` - Persistent data (saves, achievements, stats)
   - `dll.cpp` - DLL entry point and API exports

2. **src/overlay/** - Experimental overlay implementation
   - `steam_overlay.cpp` - Integration with ingame_overlay library
   - Only enabled in experimental builds with `EMU_OVERLAY` define

3. **src/networking/** - Steam networking sockets library
   - Standalone library implementation (libsteamnetworkingsockets)

4. **src/tools/** - Utility programs
   - `generate_interfaces` - Scans steam_api.dll to extract interface versions
   - `lobby_connect` - Join game lobbies without overlay
   - `steamclient_loader` - Loader for experimental steamclient builds

5. **proto/** - Protobuf message definitions
   - Network protocol messages
   - Must be compiled to `proto_gen/win/` or `proto_gen/linux/` before building

### Build Variants

The project builds multiple variants:

**api_regular:** Standard steam_api(64).dll/libsteam_api.so
- No overlay, basic Steam API emulation

**api_experimental:** Enhanced build with overlay
- Defines: `EMU_OVERLAY`, `EMU_EXPERIMENTAL_BUILD`
- Links: ingame_overlay, minhook (Windows)
- Blocks non-LAN connections

**steamclient_experimental:** Full client implementation
- Defines: `STEAMCLIENT_DLL`, `EMU_OVERLAY`, `EMU_EXPERIMENTAL_BUILD`
- Used with ColdClientLoader or CPY cracks

**lib_steamnetworkingsockets:** Standalone networking library

### Key Data Flow

1. **Initialization:** Game calls `SteamAPI_Init()` → loads steam_appid.txt → reads steam_settings/ configs
2. **Settings:** Parsed from .ini files (configs.main.ini, configs.user.ini, configs.app.ini, configs.overlay.ini)
3. **Networking:** UDP broadcasts on LAN for peer discovery, protobuf for messages
4. **Storage:** Saves to `%appdata%\GSE Saves\` (Windows) or `$XDG_DATA_HOME/GSE Saves/` (Linux)

### Interface Dispatch

Steam interfaces use version strings (e.g., "STEAMUSER_INTERFACE_VERSION021"). The emulator:
1. Exports versioned getter functions (e.g., `SteamUser()` returns latest interface)
2. `steam_client.cpp` handles `ISteamClient::Get*()` methods
3. Uses `#include` pattern to include interface version headers from SDK

## Code Conventions

### Platform Defines
- `__WINDOWS__` - Windows platform
- `GNUC` - GCC/Clang on Linux
- `EMU_RELEASE_BUILD` - Release mode (disables logging)
- `EMU_EXPERIMENTAL_BUILD` - Experimental features
- `EMU_OVERLAY` - Overlay enabled
- `STEAMCLIENT_DLL` - Building steamclient variant
- `NO_DISK_WRITES` - Disable persistent storage (for tools)

### Logging
```cpp
#ifndef EMU_RELEASE_BUILD
#include "dbg_log.hpp"
#endif

PRINT_DEBUG("Message: %s", value);  // Only in debug builds
```

Debug builds write to `STEAM_LOG_<random>.log` in the program directory or `GseAppPath` env var location.

### String Encoding
Always use UTF-8 internally. Use helpers from `common_helpers.cpp`:
- `utf8_decode()` - UTF-8 string to wide string (Windows)
- `utf8_encode()` - Wide string to UTF-8 (Windows)

### Thread Safety
Use `global_mutex` (recursive_mutex) for thread-safe operations across the emulator.

### Steam API Patterns
When implementing a Steam interface:
1. Match the SDK header signature exactly
2. Return sensible defaults for unimplemented features
3. Check `settings` for configuration overrides
4. Use `network->sendTo()` for LAN communication
5. Store persistent data via `local_storage`

### Configuration Files
All configs use INI format parsed by simpleini library:
- Section headers: `[section]`
- Key-value pairs: `key=value`
- Lists: multiple lines with same key, or comma-separated

### Proto Files
After modifying .proto files in `proto/`, regenerate C++ code:
```powershell
.\build.ps1  # Select option 2 (Generate Protobuf Files)
```
Or with xmake:
```bash
xmake f --genproto=true
xmake build
```

## Important Notes

### Always Generate Interfaces File
When setting up the emu for a game, always use `generate_interfaces` tool to create `steam_interfaces.txt` from the game's original steam_api.dll. This ensures correct interface versions.

### Third-Party Libraries
The project depends on precompiled libraries in `third_party/`:
- **libssq** - Source server query library (custom, not in xmake packages)
- **ingame_overlay** - Overlay rendering (experimental builds only, custom)
- **mbedtls** - TLS/crypto
- **protobuf** - Message serialization
- **curl** - HTTP client
- **zlib** - Compression
- **portaudio + opus** - Voice chat (limited implementation)
- **utfcpp** - UTF-8 handling
- **detours** - API hooking (Windows, experimental)

**libssq** and **ingame_overlay** are managed by `setup_deps.ps1`, not xmake packages.

### Windows Resources
Resource files (.rc) in `resources/win/` are conditionally compiled when `--winrsrc` option is enabled. They embed icons and version info.

### ColdClientLoader
For games with aggressive anti-tamper or DRM, use the ColdClientLoader setup instead of direct DLL replacement. See `post_build/win/ColdClientLoader.EXAMPLE/`.

### Experimental Features Status
- **Overlay:** Highly experimental, Windows x64 only reliable, can cause crashes
- **Non-LAN matchmaking:** Currently broken (matchmaking_server_list_actual_type)
- **Source query for server details:** Currently broken (matchmaking_server_details_via_source_query)

## Common Tasks

### Adding a New Steam Interface Method
1. Find interface definition in `include/sdk/steam/`
2. Add method to corresponding `src/core/steam_*.h`
3. Implement in `src/core/steam_*.cpp`
4. For callbacks, define in `capicmcallback.h` if needed

### Modifying Networking Protocol
1. Edit `.proto` files in `proto/`
2. Regenerate with protoc (via build.ps1 or xmake)
3. Update `network.cpp` message handlers

### Adding Configuration Option
1. Add to example file in `post_build/steam_settings.EXAMPLE/configs.*.EXAMPLE.ini`
2. Parse in `settings_parser.cpp` → store in `Settings_Info` struct (`settings.h`)
3. Use in relevant Steam interface implementation

### Debug Build Logging
To enable verbose logging:
1. Build in debug mode (`-m debug`)
2. Run game, logs appear as `STEAM_LOG_<number>.log` beside DLL
3. Search for `PRINT_DEBUG` calls in source to understand logging

### Adding Controller Mapping
Create action set files in `steam_settings/controller/<ACTION_SET>.txt`:
```
DIGITAL_ACTION=BUTTON_NAME
ANALOG_ACTION=ANALOG_NAME=input_source_mode
```
Use `generate_emu_config` tool or parse VDF files with `parse_controller_vdf`.
