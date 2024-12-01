## What is this ?
This is a build of the `experimental` version of the emu in `steamclient` mode, with an included loader which was originally [written by Rat431](https://github.com/Rat431/ColdAPI_Steam/tree/master/src/ColdClientLoader) and later modified to suite the needs of this emu.  

The backend .dll/.so of Steam is a library called `steamclient`, this build will act as a `steamclient` allowing you to retain the original `steam_api(64).dll`.  
See both the regular and experimental readmes for how to configure it.

---

**Note** that all emu config files should be put beside the `steamclient(64).dll`.  

You do not need to create a `steam_interfaces.txt` file for the `steamclient` version of the emu.

---

## How to ue it ?
1. Copy the following files to any folder:  
   * `steamclient.dll`
   * `steamclient64.dll`
   * `ColdClientLoader.ini`
   * `steamclient_loader.exe`

2. While it is not mandatory, it is highly recommended to copy the relevant `GameOverlayRenderer` dll(s)  
   This is recommended because some apps check for the existence of this dll, either on disk, on inside their memory space, otherwise they'll trigger custom protection.  
   When in doubt, just copy both dlls.  
   * `GameOverlayRenderer.dll`: for 32-bit apps
   * `GameOverlayRenderer64.dll`: for 64-bit apps

3. Edit `ColdClientLoader.ini` and specify:  
   * `AppId`: the app ID
   * `Exe`: the path to the game's executable/launcher, either full path or relative to this loader  
   * `ExeRunDir` *(optional)*: generally this must be set to the folder containing the game's exe, if left empty then it will be automatically set to the folder containing the game's exe.  
   * `ExeCommandLine` *(optional)*: additional args to pass to the exe, example: `-dx11 -windowed`  
     Optionally you can specify a different location for `steamclient(64).dll`:  
   * `SteamClientDll`: path to `steamclient.dll`, either full path or relative to this loader  
   * `SteamClient64Dll`: path to `steamclient64.dll`, either full path or relative to this loader  
   * `ForceInjectSteamClient`: force inject `steamclient(64).dll` instead of letting the app load it automatically  
   * `ForceInjectGameOverlayRenderer`: force inject `GameOverlayRenderer(64).dll` instead of letting the app load it automatically.  
     These dlls are expected to be in the same folder of `steamclient(64).dll`
   * `ResumeByDebugger`: setting this to `1` or `y` or `true` will prevent the loader from calling `ResumeThread()` on the main thread after spawning the .exe, and it will display a mesage with the process ID (PID) so you attach your debugger on it.  
     Note that you have to resume the main thread from the debugger after attaching, also the entry breakpoint may not be set automatically, but you can do that manually.  
   * `DllsToInjectFolder` *(optional)*: path to a folder containing dlls to force inject into the app upon start,  
     the loader will attempt to detect the dll architecture (32 or 64 bit), if it didn't match the architecture of the exe then it will be ignored.  
     Path is either full or relative to this loader  
   * `IgnoreInjectionError`: setting this to `1` or `y` or `true` will prevent the loader from displaying an error message when a dll injection fails  
   * `IgnoreLoaderArchDifference`: don't display an error message if the architecture of the loader is different from the app.  
   this will result in a silent failure if a dll injection didn't succeed.  
   both the loader and the app must have the same arch for the injection to work  
   * `Mode` (in `[Persistence]` section):
     - 0 = turned off
     - 1 = loader will spawn the exe and keep hanging in the background until you press "OK"
     - 2 = loader will NOT spawn exe, it will just setup the required environemnt and keep hanging in the background  
       you have to run the Exe manually, and finally press "OK" when you've finished playing  
       you have to rename the loader to "steam.exe"  
       it is advised to run the loader as admin in this mode  


**Note** that any arguments passed to `steamclient_loader.exe` via command line will be passed to the target `.exe`.  
Example: `steamclient_loader.exe` `-dx11`  
If the additional exe arguments were both: passed via command line and set in the `.ini` file, then both will be cocatenated and passed to the exe.  
This allows the loader to be used/called from other external apps which set additional args.  

### `DllsToInjectFolder`  
The folder specified by this identifier should contain the .dll files you'd like to inject in the app earlier during its creation.  
All the subfolders inside this folder will be traversed recursively, and the .dll files inside these subfolders will be loaded/injected.  

Additionaly, you can create a file called `load_order.txt` inside your folder (root level, not inside any subdir), mentioning on each line the .dll files to inject.  
The order of the lines will instruct the loader which .dll to inject first, the .dll mentioned on the first line will be injected first and so on.  
Each line inside this file has to be the relative path of your target .dll, and it should be relative to your folder. Check the example.  

If this file is created then the loader will only inject the .dll files mentioned inside it, otherwise it will attempt to find all valid .dll files and inject them.  

---

## `extra_dlls`  
This folder contains an experimental dll which, when injected, will attempt to patch the Stub drm in memory, mainly for newer variants but it also works on some of the older ones.  

This isn't a complete solution, just a different method.  
This dll is meant to be injected during **startup** only, it must **not** be placed inside `steam_settings/load_dlls`, otherwise it would cause a huge FPS drop.  

---

## `GameOverlayRenderer`
Some apps verify the existence of this dll, either on disk, or inside their memory space, that's why this dll exists.  
It is **NOT** recommended to ignore this dll.  

## Mods paths (source-engine games on Windows)
On Windows, the registry key `SourceModInstallPath` is changed to the folder containing the loader.  
```
Registry path:  HKEY_CURRENT_USER\SOFTWARE\Valve\Steam
Registry key:   SourceModInstallPath
Original value: C:\Program Files (x86)\Steam\steamapps\sourcemods
New value:      <FOLDER CONTAINING THE LOADER>
```

This affects source-engine mods  
