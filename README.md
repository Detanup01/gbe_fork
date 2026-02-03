# Goldberg Steam Emulator (GBE) Fork

Fork of <https://gitlab.com/Mr_Goldberg/goldberg_emulator>

## About This Fork

This fork features significant modernization and restructuring:
- Migrated build system from premake5 to **xmake**
- Reorganized project structure with clearer separation of concerns
- Updated dependencies and build toolchain
- Enhanced documentation and distribution guides

**This fork is incompatible with the original repository.** Many things have changed and some features may be broken. If something doesn't work, feel free to create a pull request with the appropriate fix.

## Credits

Thanks to everyone contributing to this project. See [CHANGELOG.md](./CHANGELOG.md) for a list of changes and their authors.

This project depends on many third-party libraries and tools. See [CREDITS.md](./CREDITS.md) for the complete listing.

---

# Using the Emulator

**Important:**
* Always generate the interfaces file using the `generate_interfaces` tool
* If things don't work, try the `ColdClientLoader` setup

## User Guides and Tools

* **[GBE Fork Tools](https://github.com/Detanup01/gbe_fork_tools)**
* **[Semuexec](https://gitlab.com/detiam/Semuexec)**
* **[Steam Emu Utility](https://github.com/turusudiro/SteamEmuUtility)**
* **[How to use Goldberg Emulator](https://rentry.co/goldberg_emulator)**
* **[GSE-Generator](https://github.com/brunolee-GIT/GSE-Generator)**

## Documentation

Detailed usage instructions are available in the `docs/distribution/` folder:
* [README.release.md](./docs/distribution/README.release.md) - Basic usage
* [README.experimental.md](./docs/distribution/README.experimental.md) - Experimental features
* [README.debug.md](./docs/distribution/README.debug.md) - Debug builds
* [README.generate_interfaces.md](./docs/distribution/README.generate_interfaces.md) - Interface generation
* [README.lobby_connect.md](./docs/distribution/README.lobby_connect.md) - Lobby connection tool  



---

# Building from Source

## Prerequisites

### Cloning the Repository

Clone the repo and its submodules **recursively**:

```shell
git clone --recurse-submodules -j8 https://github.com/Rustbeard86/gbe_fork.git
```

The `-j8` switch is optional and allows Git to fetch up to 8 submodules in parallel.

Keep submodules up to date:

```shell
git submodule update --init --recursive --remote
```

### Required Tools

**Windows:**
* Windows 10 or later (Windows 11 recommended)
* Visual Studio 2022 Community: <https://visualstudio.microsoft.com/vs/community/>
  * Select the `Desktop development with C++` workload
  * In `Individual components`, select the latest `Windows 11 SDK` (e.g., `10.0.22621.0` or newer)
* **xmake** build tool: <https://xmake.io/#/getting_started>
  * Install via: `winget install xmake` or download from website
* Python 3.10 or above: <https://www.python.org/downloads/windows/>
  * Verify installation: `python --version`

**Linux:**
* Ubuntu 22.04 LTS or newer: <https://ubuntu.com/download/desktop>
* Required packages:
  ```shell
  sudo apt update -y
  sudo apt install -y build-essential gcc-multilib g++-multilib
  sudo apt install -y libglx-dev libgl-dev  # For overlay builds
  sudo apt install -y python3 python3-pip
  ```
* **xmake** build tool:
  ```shell
  curl -fsSL https://xmake.io/shget.text | bash
  ```
* Python 3.10 or above

## Building

### Quick Start (Interactive)

Use the interactive build script:

```powershell
.\build.ps1
```

This interactive script provides a menu to:
1. Install dependencies (xmake packages + custom deps)
2. Generate protobuf files (required before first build)
3. Build all targets or select specific ones
4. Configure architecture (x64/x86) and mode (debug/release)

### Command-Line Build

**Configure the project:**
```bash
xmake f -p windows -a x64 -m release -c -y  # Windows
xmake f -p linux -a x86_64 -m release -c -y  # Linux
```

**Generate protobuf files (required once, or after .proto changes):**
```bash
xmake f --genproto=true
```

**Build all targets:**
```bash
xmake build
```

**Build specific targets:**
```bash
xmake build api_regular          # Regular steam_api.dll
xmake build api_experimental     # Experimental with overlay
xmake build steamclient_experimental
xmake build tool_generate_interfaces
xmake build tool_lobby_connect
```

**Available build targets:**
* `api_regular` - Standard steam_api DLL
* `api_experimental` - Experimental build with overlay support
* `steamclient_experimental` - Full steamclient implementation
* `lib_steamnetworkingsockets` - Standalone networking library
* `lib_game_overlay_renderer` - Overlay renderer
* `tool_generate_interfaces` - Interface scanner utility
* `tool_lobby_connect` - Lobby connection tool

### Build Options

Configure with additional options:
```bash
xmake f --genproto=true          # Generate protobuf files
xmake f --winrsrc=true           # Add Windows resources (icons, version info)
xmake f --winsign=true           # Sign with fake certificate
xmake f --emubuild="custom_tag"  # Set custom build string
```

### Output Locations

Built files are placed in `.build/`:
```
.build/
├── release/
│   ├── windows/
│   │   ├── x64/
│   │   │   ├── steam_api64.dll
│   │   │   └── ...
│   │   └── x86/
│   │       ├── steam_api.dll
│   │       └── ...
│   └── linux/
│       └── ...
└── debug/
    └── ...
```  



---

## Project Structure

```
gbe_fork/
├── src/
│   ├── core/          # Steam API interface implementations
│   ├── overlay/       # Overlay integration
│   ├── networking/    # Network sockets library
│   ├── tools/         # Utilities (generate_interfaces, lobby_connect)
│   ├── common/        # Shared helpers
│   └── libraries/     # Embedded third-party code
├── include/
│   ├── gbe/           # Project headers
│   └── sdk/           # Steam SDK headers
├── proto/             # Protobuf definitions
├── proto_gen/         # Generated protobuf code (auto-generated)
├── docs/
│   └── distribution/  # End-user documentation
├── resources/         # Windows resources (icons, version info)
├── third_party/       # Git submodules and vendored dependencies
├── xmake.lua          # Build configuration
└── build.ps1          # Interactive build script
```

---

## Contributing

Contributions are welcome! Please:
1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Submit a pull request to the `dev` branch

For questions or issues, use the GitHub issue tracker: <https://github.com/Rustbeard86/gbe_fork/issues>

---

## License

See [LICENSE](./LICENSE) for details.

This project is based on the Goldberg Steam Emulator by Mr_Goldberg, licensed under LGPL v3.
