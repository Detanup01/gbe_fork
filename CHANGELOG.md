# 2424/2/20

* generate_emu_config: allow setting the steam id of apps/games owners from an external file `top_owners_ids.txt` beside the script, suggested by **[M4RCK5]**
* generate_emu_config: support the new format for `supported_languages`
* generate_emu_config: update the code which parses controller inputs
* generate_emu_config: always use the directory of the script for: the data `backup` folder, the `login_temp` folder, the `my_login.txt` file

---

## 2024/2/13

* cold client loader: validate the PE signature before attempting to detect arch

---

## 2024/2/10

* a hacky fix for the overlay on directx12, currently very slow when loading images
* limit the attempts to load the achievements images, to prevent a never ending FPS drop

---

## 2024/2/7

* new persistent modes for cold client loader, mode 2 is a more accurate simulation and allows launching apps from their .exe
* allow setting the IP country via the file `ip_country.txt`

---

* **[Breaking]** changed the ini sections of the cold client loader

---

## 2024/1/26

* **[Detanup01]** added a new command line option for the tool `generate_emu_config` to disable the generation of `disable_xxx.txt` files,  
  suggested by **[Vlxst]**
* added new settings to the overlay which allow specifying the notifications positions, check the example file `overlay_appearance.EXAMPLE.txt`,  
  suggested by **[ugurkahriman]**
* fixed a mistake when discarding the utf8 bom marker

---

## 2024/1/25

* added new options to the overlay to allow copying a friend's ID, plus current player ID, suggested by **[Vlxst]**
* added a new option to the overlay to invite all friends playing the same game, suggested by **[Vlxst]**
* added new `auto_accept_invite.txt` setting to automatically accept game/lobby invites from this list, each SteamID64 on a separate line  
  also you can leave the file empty to accept invitations from anyone, check the updated release readme, suggested by **[Vlxst]**
* added new `disable_overlay_warning_*.txt` settings to disable certain or all warnings in the overlay, suggested by **[Vlxst]**
  * `disable_overlay_warning_forced_setting.txt`:  
    - disable the warning for the usage of any file `force_*.txt` in the overlay
    - unlocks the settings menu, this may result in an undesirable output
  * `disable_overlay_warning_bad_appid.txt`: disable the warning for bad app ID (when app ID = 0) in the overlay
  * `disable_overlay_warning_local_save.txt`: disable the warning for using local save in the overlay
  * `disable_overlay_warning_any.txt`: all the above
* **deprecated** `disable_overlay_warning.txt` in `steam_settings` folder in favor of new the options/files
* added more Stub variants
* fixed the condition of `warn_forced_setting`, previously it may be reset back to `false` accidentally
* fixed a casting mistake when displaying friend ID
* avoid spam loading the achievements forever on failure, only try 3 times
* removed a debug flag in `UGC::GetItemState()` left by mistake

---

## 2024/1/20

* **[Detanup01]** added implementation for `Steam_Remote_Storage::EnumerateUserSubscribedFiles()` +   
  mods files handles in `Steam_Remote_Storage::UGCDownload()` + `Steam_Remote_Storage::UGCDownloadToLocation()`  
  which makes mods now work for many games
* **[Kola124]** enhanced the settings parser to detect primary and preview mod files sizes automatically +  
  use the base Steam URL by default for workshop URL + auto calculate the mod `score` from up/down votes  
  also thanks to **[BTFighter]** for providing logs
* **Breaking change** mod preview image file must exist in `steam_settings\mod_images\<MOD_ID>`
* an enhancement to the settings parser to attempt to auto detect mods when `mods.json` is not present, with the same behavior as when the json file was created.  
  this works for mods with only 1 primary file and only 1 preview file
* fixed the generated path of mod `preview_url`, previously it would contain back slashes `\` on Windows
* use last week epoch as the default time for mods dates (created, added, etc...)
* make sure the mod path is always normalized and absolute, required by some APIs
* `Steam UGC`: implement `SetUserItemVote()`, `GetUserItemVote()`, `AddItemToFavorites()`, `RemoveItemFromFavorites()`,  
  favorite mods list are now saved in `favorites.txt` in the user save data folder
* cold client loader can now inject user dlls, and force inject the `steamclient(64).dll` library,  
  also you can control the injection order via a file `load_order.txt`, check its readme and the provided example
* a new experimental dll (which must be injected first) to patch Stub drm v3.1 in memory, check the injection example of the cold client loader
* cold client loader will now treat relative paths as relative to its own path, previously it used the current active directory
* cold client loader no longer needs an explicit setting for the `ExeRunDir`, by default it would be the folder of the exe
* in cold client loader, the option `ResumeByDebugger` is now available for the release build
* cold client loader is now built for 32-bit and 64-bit separately, and will display a nag about architecture difference if for example the app was 32-bit and the loader was 64-bit, this could be disabled via the setting `IgnoreLoaderArchDifference=1`
* the cold client loader will output useful debug info when the debug build is used
* added a very basic crashes logger/printer, enabled by creating a file called `crash_printer_location.txt` inside the `steam_settings` folder, check README.realease.md for more details  
* fixed a problem in the overlay which would cause a crash for the guest player when an invitation was sent
* `Steam UGC`: make sure returned mod folder from `GetItemInstallInfo()` is null terminated, previously some apps would get a bad malformed string because of this
* `Steam_RemoteStorage`: very basic implementation for `GetQueryUGCNumTags()`, `GetQueryUGCTag()`, `GetQueryUGCTagDisplayName()`
* new function in local storage to get list of folders at root level, given some path
* imitate how the DOS Stub is manipulated during/after the build
* some fixes to the win build script + use the undocumented linker flag `/emittoolversioninfo:no` to prevent adding the MSVC Rich Header
* debug messages are now mostly scoped, ex: `Steam_Ugc::XXX`
* added a bunch of helper functions, `common_helpers::XXX` + `pe_helpers::XXX`

---

## 2024/1/5

* **[Detanup01]** Fixed parsing of old Steam interfaces, reported by **[LuKeStorm]**: https://cs.rin.ru/forum/viewtopic.php?p=2971639#p2971639  
* refactored the tool `find_intrfaces` to search accurately for old interfaces  

---

## 2024/1/3  

* added a new option to the Windows version of the client loader to aid in debugging.  
the option is called `ResumeByDebugger`, and setting it to `1` will prevent the loader from  
auto resuming the main app thread, giving you a chance to attach your debugger.  
* make the script `generate_emu_config` generate an empty `DLC.txt` if the app has no DLCs
* windows build: sign each file after build with a self-signed generated certificate + note in the release readme regarding false-positives
* windows build: note in readme about Windows SDK
* windows build: added vesion resource (.rc file)
* gen emu config: readme + icon attribution
* added anonymous login to gen emu script, these accounts have very limited access
* linux + win build scripts: introduce -verbose flag
* windows build script: ensure /MT when compiling
* output protoc generated in a subfolder in dll/ for easier code reference +  
  don't cleanup protoc generated files, because VScode gets confused and cannot find files/types

---

## 2023/12/21 - 2023/12/27

* **[Detanup01]** added option to send auth token with new Ticket! + an option to include the GC token  
  by default the emu will send the old token format for various APIs, like:  
  * `Steam_GameServer::GetAuthSessionTicket()`
  * `Steam_User::GetAuthSessionTicket()`
  * `Steam_User::GetAuthTicketForWebApi()`  

  this allows the emu to generate new ticket data, and additionally the GC token.  
  check the new config files `new_app_ticket.txt` and `gc_token.txt` in the `steam_settings` folder
* **[Detanup01]** fixed print issues in some places
* **[remelt]** use the `index` argument to grab the preview URL from UGC query result, fixed by: https://cs.rin.ru/forum/viewtopic.php?p=2964432#p2964432
* **[remelt]** allow overriding mod `path` & mod `preview_url` in the `mods.json` file, suggested by: https://cs.rin.ru/forum/viewtopic.php?p=2964432#p2964432
* allow setting the mod `score` in the `mods.json`
* when the mod `preview_url` is not overridden, don't set it automatically if `preview_filename` was empty, otherwise the `preview_url` will be pointing to the entire `mod_images` folder, like: `file://C:/my_game/steam_settings/mod_images/`  
  instead set it to an empty string
* updated `mods.EXAMPLE.json`
* added 2 new config files `is_beta_branch.txt` and `force_branch_name.txt`  
  by default the emu will report a `non-beta` branch with the name `public` when the game calls `Steam_Apps::GetCurrentBetaName()`  
  these new config files allow changing that behavior, check the `steam_settings` folder
* refactored the `steamclient_loader` script for Linux + new options and enhancements to make it similar to the Windows version, check its new README!
* for steamclient loader (Windows + Linux): pass loader arguments to the target exe, allowing it to be used from external callers, example by the `lobby_connect` tool
* deprecated the `find_interface` scripts, now the executable is built for Windows & Linux!
* included the `steam_settings.EXAMPLE` for Linux build
* updated release READMEs!
* added a README for the repo with detailed build steps
>>>>>>>>> ---

* check for invalid data pointer in `GetAuthSessionTicket()`
* additional sanity check in `InitiateGameConnection()` + print input data address in debug build
* moved the example `app id` and `interfaces` files inside `steam_settings` folder, to avoid encouraging putting files outside

>>>>>>>>> ---

* fixed all debug build warnings for Linux & Windows (no more scary messages!)
* updated Linux & Windows build scripts to avoid removing the entire build folder before building + introduced `clean` flag
* added licenses & sources of all extrnal libraries + added a new cryptography library `Mbed TLS`  
  you have to rebuilt the deps
* deprecated the separate/dedicated cleanup script for Windows, it's now inlined in the main build script
* For Windows build script: deprecated `low perf` & `win xp` options
* For Linux build script: deprecated `low perf` option
* restored all original but unused repo files into their separate folder
* lots of refactoring and relocation in the source repo:
  - all build stuff will be inside `build` folder
  - restructured the entire repo
  - generate proto source files in the `build\tmp` folder insead of the actual source folder

>>>>>>>>> ---

* `settings_parser.cpp`:
  - cleanup the settings parser code by split it into functions
  - increase the buffer size for `account_name` to 100 chars
  - increase the buffer size for `language` to 64 chars
* `common_includes.h`:
  - refactor includes order
  - added new helper function to keep yielding the thread for a given amount of time (currently unused)
* build scripts:
  - in Linux build scripts don't use `-d` flag with `rm`
  - added global build stat message
  - use an obnoxious name for the file handle variable used if the PRINT_DEBUG macro to avoid collisions, in the caller has a variable with same name
* don't cache deps build when pushing tag or opening pull requests
* remove hardcoded repo path + remove Git LFS flag since it's no longer needed

---

## 2023/12/20

* fixed the implementation of `BIsAppInstalled()`, it must lock the global mutex since it is thread-safe, otherwise it will cause starvation and the current thread wion't yield, which triggers some games

* more accurate behavior for `BIsAppInstalled()`, reject app ID if it was in the DLC list and isUnlockAllDlc was false

* basic implementation for `RequestAppProofOfPurchaseKey()` and `RequestAllProofOfPurchaseKeys()`

* a simple implementation for `GetEarliestPurchaseUnixTime()`

* more accurate implementation for `BGetSessionClientResolution()`, set both x & y to 0

* return false in `BIsDlcInstalled()` when the given app ID is the base game

* check for invalid app ID `uint32_max` in different places

* more accurate implementation for `BReleaseSteamPipe()`, return true if the pipe was released successfully

* lock the global mutex and the overlay mutex in different places just to be on the safe side, without it, some games suffer from thread starvation, might slow things down

* added missing env var `SteamOverlayGameId` to steam_client and client_loader

* added a startup timer + counter for reference, currently used to print timestamp in debug log

* consistent debug log location, for games that change cwd multiple times while running

* fixed error propagation in Windows build script, apparently set /a var+=another_var works only if another_var is a defined env var but NOT one of the "magic" builtins like errorlevel

---

## 2023/12/17
* More accurate implementation for BIsAppInstalled(), it now rejects uint32_max

* Allow behavior customizization via installed_app_ids.txt config file

* Limit/Lock list of installed apps on an empty file (similar to dlc.txt)

* Changed the behavior of GetCurrentBetaName() to comply with the docs, might break stuff

* Allow customizing the behavior via ne config files: `is_beta_branch.txt` + `force_branch_name.txt` 

* New script to generate native executable for `generate_emu_config` on Linux using pyinstaller

* Deprecate the old `RtlGenRandom()` in favor of the new `BCryptGenRandom()`

* Setup Github Worflows to:
  * Build `generate_emu_config` for `Linux` when you push code to a branch whose name matches the pattern `ci-build-gen-linux*`
  * Build `generate_emu_config` for `Windows` when you push code to a branch whose name matches the pattern `ci-build-gen-win*`
  * Build the emu for `Linux` when you push code to a branch whose name matches the pattern `ci-build-emu-linux*`
  * Build the emu for `Windows` when you push code to a branch whose name matches the pattern `ci-build-emu-win*`
  * Build everything when you push code to a branch whose name is `ci-build-all`
  * Build everything and create a release when you push a tag whose name matches the pattern `release*`

* Packaging scripts for both Windows & Linux, usable locally and via Github Workflows
  * For the emu:
    * First run `build_win_deps.bat` (Windows)  
    or `sudo ./build_linux_deps.sh` (Linux)
    * Run `build_win.bat release` + `build_win.bat debug` (Windows)  
    or `./build_linux.sh release` + `./build_linux.sh debug` (Linux)
    * Finally run `package_win.bat release` + `package_win.bat debug` (Windows)  
    or `sudo ./package_linux.sh release` + `sudo ./package_linux.sh debug` (Linux)
  * The same goes for `generate_emu_config` (scripts folder) but the scripts do not take any arguments, so no `release` or `debug`

* Added all third-party dependencies as local branches in this repo + refer to these branches as submodules, making the repo self contained

---

## 2023/12/14
* based on cvsR4U1 by ce20fdf2 from viewtopic.php?p=2936697#p2936697

* apply the fix for the Linux build (due to newer glibc) from this pull request by Randy Li: https://gitlab.com/Mr_Goldberg/goldberg_emulator/-/merge_requests/42/

* add updated translation of Spanish + Latin American to the overlay by dragonslayer609 from viewtopic.php?p=2936892#p2936892
* add updated translation of Russian to the overlay by GogoVan from viewtopic.php?p=2939565#p2939565

* add more interfaces to look for in the original steam_api by alex47exe from viewtopic.php?p=2935557#p2935557

* add fix for glyphs icons for xbox 360 controller by 0x0315 from viewtopic.php?p=2949498#p2949498

* bare minimum implementation for SDK 1.58a
  + backup the current version of the interface 'steam ugc'
    -  create new file: isteamugc017.h
        + copy the current version of the interface to this file
        + don't copy enums, structs, constants, etc..., just copy the pure virtual (abstract) class of the interface
        + rename the abstract class to include the current version number in its name, i.e. 'class ISteamUGC017'
        + create a file header guard containing the interface version in its name, i.e. 'ISTEAMUGC017_H'
        + if the file has '#pragma once', then guard this line with '#ifdef STEAM_WIN32' ... '#endif', I don't know why
  + isteamugc.h (this always contains the declaration of latest interface version)
    - declare the new API: GetUserContentDescriptorPreferences()
    - update the API: SetItemTags() to use the new argument
    - update the interface version to STEAMUGC_INTERFACE_VERSION018
  + steam_ugc.h (this always contains the implementation of ALL interfaces versions)
    - add the backed-up abstract class to the list of inheritance, i.e. 'public ISteamUGC017'
    - (needs revise) implement the new API: GetUserContentDescriptorPreferences()
    - add a new overload of the API: SetItemTags() which takes the new additional argument
  
  + backup the current version of the interface 'steam remote play'
    -  create new file: isteamremoteplay001.h
        + copy the current version of the interface to this file
        + don't copy enums, structs, constants, etc..., just copy the pure virtual (abstract) class of the interface
        + rename the abstract class to include the current version number in its name, i.e. 'class ISteamRemotePlay001'
        + create a file header guard containing the interface version in its name, i.e. 'ISTEAMREMOTEPLAY001_H'
        + if the file has '#pragma once', then guard this line with '#ifdef STEAM_WIN32' ... '#endif', I don't know why
  + isteamremoteplay.h (this always contains the declaration of latest interface version)
    - declare the new API: BStartRemotePlayTogether()
    - update the interface version to STEAMREMOTEPLAY_INTERFACE_VERSION002
    - fix file header guard from _WIN32 to STEAM_WIN32
  + steam_remoteplay.h (this always contains the implementation of ALL interfaces versions)
    - add the backed-up abstract class to the list of inheritance, i.e. 'public ISteamRemotePlay001'
    - (needs revise) implement the new API: BStartRemotePlayTogether()
  
  + steam_api.h
    - #include the backed-up interface files:
        + #include "isteamugc017.h"
        + #include "isteamremoteplay001.h"
    - declare the new API: SteamInternal_SteamAPI_Init()
    - add a new enum ESteamAPIInitResult
    - fix return type of SteamAPI_InitSafe() from bool to steam_bool (some stupid games read the whole EAX register)
    - add a useless inline implementation for the API: SteamAPI_InitEx(), not exported yet but just in case for the future
  + steam_gameserver.h
    - declare the new API: SteamInternal_GameServer_Init_V2()
    - fix return type of SteamGameServer_Init() from bool to steam_bool (some stupid games read the whole EAX register)
    - add a useless inline implementation for the API: SteamGameServer_InitEx(), not exported yet but just in case for the future
  + steam_api_common.h
    - declare a new type: SteamErrMsg
  + dll.cpp (this has the implementation of whatever inside steam_api.h + steam_gameserver.h)
    - (needs revise) implement the new API: SteamInternal_SteamAPI_Init()
    - (needs revise) implement the new API: SteamInternal_GameServer_Init_V2()
    - read some missing interfaces versions when parsing steam_interfaces.txt
    - initialize all interfaces versions with the latest ones available, instead of hardcoding them
  
  + steam_client.cpp
    - add a new version string for the interface getter GetISteamUGC()
    - add a new version string for the interface getter GetISteamRemotePlay()
  
  + isteamnetworkingsockets.h
    - fix the signatures of the APIs: (ISteamNetworkingConnectionCustomSignaling vs ISteamNetworkingConnectionSignaling)
        + ConnectP2PCustomSignaling()
        + ReceivedP2PCustomSignal()
  + isteamnetworkingsockets009.h
    - fix the signatures of the APIs: (ISteamNetworkingConnectionCustomSignaling vs ISteamNetworkingConnectionSignaling)
        + ConnectP2PCustomSignaling()
        + ReceivedP2PCustomSignal()
  + steam_networking_sockets.h
    - implement the missing overloads of the APIs: (ISteamNetworkingConnectionCustomSignaling vs ISteamNetworkingConnectionSignaling)
        + ConnectP2PCustomSignaling()
        + ReceivedP2PCustomSignal()
  
  + steam_api_flat.h  
    ////////////////////  
    - declare new interfaces getters:
        + SteamAPI_SteamUGC_v018()
        + SteamAPI_SteamGameServerUGC_v018()
    - declare the new API: SteamAPI_ISteamUGC_GetUserContentDescriptorPreferences()
    - (needs revise) update signature of the API: SteamAPI_ISteamUGC_SetItemTags() to add the new argument
      this will potentially break compatibility with older version of the flat API  
    ////////////////////  
    - declare new interface getter: SteamAPI_SteamRemotePlay_v002()
    - declare the new API: SteamAPI_ISteamRemotePlay_BStartRemotePlayTogether()  
    ////////////////////  
    - fix the signatures of the APIs: (ISteamNetworkingConnectionCustomSignaling vs ISteamNetworkingConnectionSignaling)
        + SteamAPI_ISteamNetworkingSockets_ConnectP2PCustomSignaling()
        + SteamAPI_ISteamNetworkingSockets_ReceivedP2PCustomSignal()
  
  + flat.cpp  
    ////////////////////  
    - implement new interfaces getters:
        + SteamAPI_SteamUGC_v018()
        + SteamAPI_SteamGameServerUGC_v018()
    - implement the new API: SteamAPI_ISteamUGC_GetUserContentDescriptorPreferences()
    - (needs revise) update signature of the API: SteamAPI_ISteamUGC_SetItemTags() to use the new argument
      this will potentially break compatibility with older version of the flat API  
    ////////////////////  
    - implement new interface getter SteamAPI_SteamRemotePlay_v002()
    - implement the new API: SteamAPI_ISteamRemotePlay_BStartRemotePlayTogether()  
    ////////////////////  
    - fix the signatures of the APIs: (ISteamNetworkingConnectionCustomSignaling vs ISteamNetworkingConnectionSignaling)
        + SteamAPI_ISteamNetworkingSockets_ConnectP2PCustomSignaling()
        + SteamAPI_ISteamNetworkingSockets_ReceivedP2PCustomSignal()
  
  + isteamfriends.h
    - (needs revise) add a missing (or new?) member m_dwOverlayPID to the struct GameOverlayActivated_t, hopefully this doesn't break stuff
  
  + steamnetworkingtypes.h
    - add new (or missing?) members to the enum ESteamNetworkingConfigValue:
        + k_ESteamNetworkingConfig_RecvBufferSize
        + k_ESteamNetworkingConfig_RecvBufferMessages
        + k_ESteamNetworkingConfig_RecvMaxMessageSize
        + k_ESteamNetworkingConfig_RecvMaxSegmentsPerPacket
  
  + add the file isteamdualsense.h, it isn't used currently but just in case for the future
  
  + update descriptions/comments or refactor/spacing
    - isteamapplist.h
    - isteamgamecoordinator.h
    - isteamps3overlayrenderer.h
    - isteamuserstats.h
    - isteamutils.h
    - isteamvideo.h
    - steamhttpenums.h
    - steamtypes.h

* use Unicode when sanitizing settings, mainly for local_save.txt config file
  + new dir "utfcpp": containg all the source/include files of this library: https://github.com/nemtrif/utfcpp
  + common_includes.h: include the new library "utfcpp"
  + settings.cpp: in Settings::sanitize(): convert to utf-32 first, do the sanitization, then convert back to std::string and return the result

* avoid locking the global mutex every time when getting the global steamclient instance
  + dll.cpp: in get_steam_client(): only lock when the instance is null and double check for null, should speed up things a little bit
* in different places, avoid locking gloal mutex if the relevant functionality was disabled
  + example in steam_user_stats.h: SetAchievement()
  + example in steam_overlay.cpp:
      + Steam_Overlay::AddMessageNotification()
      + Steam_Overlay::AddInviteNotification()

* explicitly use the ASCII version of Windows APIs to avoid conflict when building with define symbols UNICODE + _UNICODE
  - base.cpp: GetModuleHandleA()
  - steam_overlay.cpp: PlaySoundA()

* fix the implementation of RtlGenRandom stub:
  + return a number
  + use extern "C" if building in C++ mode

* add new build scripts for both Windows and Linux for a much easier dev/build experience,  
  both Windows and Linux scripts will run parallel build jobs for a much faster build times,  
  by default, the scripts will use 70% of the max available threads, but if the auto detection didn't work,  
  you can pass for example `-j 10` to the scripts to use 10 parallel jobs  
  
  on Linux, archives (.a files) of third party libraries are bundled wholly, and built statically via:  
  `-Wl,--whole-archive -Wl,-Bstatic -lssq -lcurl ... -Wl,-Bdynamic -Wl,--no-whole-archive`  
  this ensures that the final output binary (for example: libsteam.so) won't require these libraries at runtime

  + to build on Linux (I'm using latest Ubuntu on WSL)
    - run as `sudo ./build_linux_deps.sh`, this will do the following:
      + download and install the required build tools via `apt-install`
      + unpack the third party libraries (protobuf, zlib, etc...) from the folder `third-party` to `build-linux-deps`
      + build the unpacked libraries from `build-linux-deps`
      
      you only need this step once, additionally you can pass these arguments to the script:  
      + `-verbose`: force cmake to display extra info
      + `-j <n>`: force cmake to use `<n>` parallel build jobs
      
    - without sudo, run `./build_linux.sh` and pass the argument `release` or `debug` to build the emu in the corresponding mode, this will build the emu inside the folder `build-linux`  
      some additional arguments you can pass to the script:  
      + `-lib-32`: prevent building 32-bit libsteam_api.so
      + `-lib-64`: prevent building 64-bit libsteam_api.so
        
      + `-client-32`: prevent building 32-bit steamclient.so
      + `-client-64`: prevent building 64-bit steamclient.so
        
      + `-tool-clientldr`: prevent copying the script steamclient_loader.sh
      + `-tool-itf`: prevent copying the script find_interfaces.sh
      + `-tool-lobby-32`: prevent building executable lobby_connect_x32
      + `-tool-lobby-64`: prevent building executable lobby_connect_x64
        
      + `+lowperf`: (UNTESTED) pass some arguments to the compiler to prevent emmiting instructions for: SSE4, popcnt, AVX
        
      + `-j <n>`: force build operations to use `<n>` parallel jobs
    
  + to build on Windows (just install Visual Studio 2019/2022)
    - without admin, run `build_win_deps.bat`, this will do the following:
       + unpack the third party libraries (protobuf, zlib, etc...) from the folder `third-party` to `build-win-deps`
       + build the unpacked libraries from `build-win-deps`
      
      you only need this step once, additionally you can pass these arguments to the script:  
      + `-verbose`: force cmake to display extra info
      + `-j <n>`: force cmake to use `<n>` parallel build jobs
      
    - without admin, run `build_win.bat` and pass the argument `release` or `debug` to build the emu in the corresponding mode,  
      this will build the emu inside the folder `build-win`
      some additional arguments you can pass to the script:
      + `-lib-32`: prevent building 32-bit steam_api.dll
      + `-lib-64`: prevent building 64-bit steam_api64.dll
        
      + `-ex-lib-32`: prevent building `experimental steam_api.dll`
      + `-ex-lib-64`: prevent building `experimental steam_api64.dll`
        
      + `-ex-client-32`: prevent building `experimental steamclient.dll`
      + `-ex-client-64`: prevent building `experimental steamclient64.dll`
        
      + `-exclient-32`: prevent building experimental `client steamclient.dll`
      + `-exclient-64`: prevent building experimental `client steamclient64.dll`
      + `-exclient-ldr`: prevent building experimental `client loader steamclient_loader.exe`
        
      + `-tool-itf`: prevent building executable `find_interfaces.exe`
      + `-tool-lobby`: prevent building executable `lobby_connect.exe`
        
      + `+lowperf`: (UNTESTED) for 32-bit build only, pass the argument `/arch:IA32` to the compiler
        
      + `-j <n>`: force build operations to use `<n>` parallel jobs

* added all required third-party libraries inside the folder `third-party`

* greatly enhanced the functionality of the `generate_emu_config` script + add a build script
  + run `recreate_venv.bat` to
     + create a python virtual environemnt
     + install all required packages inside this env
  + run `rebuild.bat` to produce a bootstrapped .exe built using `pyinstaller`
  + inside the folder of the built executable
     + create a file called `my_login.txt`, then add your username in the first line, and your password in the second line
     + run the .exe file without any args to display all available options
    

* revert the changes to `SetProduct()` and `SetGameDescription()`

* in `steam_overlay.cpp`, in `AddAchievementNotification()`: prefer original paths of achievements icons first, then fallback to `achievement_images/`


## Older changes
* add missing implementation of (de)sanitize_string when `NO_DISK_WRITE` is defined which fixes compilation of `lobby_connect`

* check for empty string in (de)sanitize_file_name() before accessing its items

* implement new API: `GetAuthTicketForWebApi()`
  + `base.h`: declare the new API: `getWebApiTicket()`
  + `base.cpp`: implement the new API: `Auth_Ticket_Manager::getWebApiTicket()`
  + `steam_user.h`: call the new API inside `GetAuthTicketForWebApi()`

* add an updated and safer impl for `Local_Storage::load_image_resized()` by RIPAciD from viewtopic.php?p=2884627#p2884627

* add missing note in ReadMe about `libssq`

* add new release 4 by ce20fdf2 from viewtopic.php?p=2933673#p2933673
* add hotfix 3 by ce20fdf2 from viewtopic.php?p=2921215#p2921215
* add hotfix 2 by ce20fdf2: viewtopic.php?p=2884110#p2884110
* add initial hotfix by ce20fdf2
