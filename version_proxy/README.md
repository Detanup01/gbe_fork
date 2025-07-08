# Version.dll Proxy for GBE Fork

  A clean `version.dll` proxy that hijacks DLL loading to inject Steam environment variables and registry entries required for GBE Fork emulation.

  ## What it does
  - Forwards `version.dll` calls to system `version.dll`
  - Reads App ID from `steam_settings/steam_appid.txt`
  - Sets Steam environment variables
  - Creates Steam registry entries
  - Auto-cleanup after `7 seconds`

  ## Environment Variables
  - `SteamAppId` / `SteamGameId` - Game's App ID
  - `SteamClientLaunch` - "1"
  - `SteamEnv` - "1"
  - `SteamAppUser` / `SteamUser` - "cold_player"

  ## Registry Entries
  - `HKCU\Software\Valve\Steam\ActiveProcess\pid` - Current process ID
  - `HKCU\Software\Valve\Steam\ActiveProcess\SteamClientDll64` - Path to steamclient64.dll
  - `HKCU\Software\Valve\Steam\RunningAppID` - Game's App ID
  - `HKCU\Software\Valve\Steam\SteamPath` - Current directory

  ## Building
  ```batch
  # Debug build
  build_debug.bat

  # Release build
  build_release.bat
  ```
  ## Requirements
  - `Visual Studio 2022`
  - `Windows 10 SDK`

  Usage

  1. Drop `version.dll` and hybrid `steamclient64.dll` in game directory
  2. Create `steam_settings/steam_appid.txt` with App ID (or use GBE Fork Tools)
  3. Start game executable

## Security Note
This system uses DLL hijacking, which is a legitimate technique for software interoperability. Some antivirus software may flag this as suspicious. Add exceptions as needed.

## License
Based on the Goldberg Steam Emulator (LGPL v3)
version.dll proxy functionality - Custom implementation