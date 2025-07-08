# Goldberg Steam Emulator Fork

**A professional Steam emulator fork with enhanced features**

## Credits

- **Original Project**: [Mr. Goldberg's Goldberg Emulator](https://gitlab.com/Mr_Goldberg/goldberg_emulator)
- **Best Fork**: [Detanup01's gbe_fork](https://github.com/Detanup01/gbe_fork)
- **Enhanced Fork**: Made with <3 from GittyGittyKit

---

## Key Features

- **EncryptedAppTicket Support**: Professional ticket management system
- **Steam Hybrid**: Advanced Steam client emulation
- **Steam Savegame System**: Dynamic Steam ID switching for multiple save profiles
- **Full Steam API Coverage**: Complete Steam functionality emulation

---

## Quick Start

### 1. Clone Repository

```bash
git clone --recurse-submodules -j8 https://github.com/Detanup01/gbe_fork.git
cd gbe_fork
```

### 2. Build Dependencies (First Time Only)

**Windows:**
```batch
build_win_premake.bat --deps
```

**Linux:**
```bash
./build_linux_premake.sh --deps
```

### 3. Build Emulator

**Windows - Full Build:**
```batch
build_win_premake.bat
```

**Windows - Hybrid Only:**
```batch
build_win_premake.bat --hybrid
```

**Linux - Full Build:**
```bash
./build_linux_premake.sh
```

**Linux - Hybrid Only:**
```bash
./build_linux_premake.sh --hybrid
```

---

## Steam Savegame System

The Steam Savegame System allows games to use different Steam IDs for separate save profiles, solving compatibility issues with games that check Steam ID for savegame management.

### Configuration

Create `configs.user.ini` in your game's steam_settings folder:

```ini
[user::general]
# Main Steam ID
account_steamid=76561198000000000

# Alternative Steam ID for savegame system
alt_steamid=76561198111111111

# Number of GetSteamID() calls before switching to alt_steamid
# Examples:
# - RE4: alt_steamid_count=4
# - Monster Hunter Wilds: alt_steamid_count=3
# - Doom: alt_steamid_count=2
# Set to 0 to disable (default behavior)
alt_steamid_count=3

account_name=YourName
language=english
```

### How It Works

1. First N calls to `GetSteamID()` return your main Steam ID
2. After reaching `alt_steamid_count`, all subsequent calls return the alternative Steam ID
3. This allows games to create separate save profiles based on different Steam IDs
4. Perfect for games that tie save data to Steam ID

---

## EncryptedAppTicket Support

### Basic Configuration

Add to `configs.user.ini`:

```ini
[user::general]
# Your pre-generated Base64 encrypted app ticket
token=CAIxxxxxxxxxxxxxxx...
```

### Generating Tickets

```python
import base64
with open('ticket.bin', 'rb') as f:
    print(base64.b64encode(f.read()).decode('utf-8'))
```

Most valid tickets start with `CAI` after Base64 encoding.

---

## Build Requirements

### Windows
- Visual Studio 2022 Community with C++ workload
- Windows 10/11 SDK
- Python 3.10+

### Linux
- Ubuntu 22.04 LTS or compatible
- GCC with multilib support
- OpenGL development headers
- Python 3.10+

```bash
sudo apt update
sudo apt install -y build-essential gcc-multilib g++-multilib libglx-dev libgl-dev
```

---

## Usage Instructions

1. **Always generate interfaces**: Use the `generate_interfaces` tool first
2. **For compatibility issues**: Try the `ColdClientLoader` setup
3. **Game-specific configs**: Place configuration files in the game's steam_settings folder

### Helper Tools

- [GBE Fork Tools](https://github.com/Detanup01/gbe_fork_tools)
- [Steam Emu Utility](https://github.com/turusudiro/SteamEmuUtility)
- [Goldberg Emulator Guide](https://rentry.co/goldberg_emulator)

---

## Compatibility

This fork includes significant improvements over the original emulator:
- Enhanced stability and compatibility
- Professional-grade ticket management
- Advanced savegame handling
- Improved Steam API coverage

For detailed technical documentation, see:
- [Build Instructions](./post_build/README.release.md)
- [Changelog](./CHANGELOG.md)
- [Credits](./CREDITS.md)

---

## Professional Support

This emulator is designed for legitimate use cases including:
- Game preservation and archival
- Offline gaming environments
- Development and testing scenarios
- Educational purposes

**Note**: Always respect software licenses and terms of service.