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
2. Edit `ColdClientLoader.ini` and specify:
   * `AppId`: the app ID
   * `Exe`: the path to the game's executable/launcher, either full path or relative to this `.ini` file
   * `ExeRunDir`: geenrally this must be set to the folder containing the game's exe
   * `ExeCommandLine` additional args to pass to the exe, example: `-dx11 -windowed`  
   Optionally you can specify a different location for `steamclient(64).dll`:  
   * `SteamClientDll`: path to `steamclient.dll`, either full path or relative to this `.ini` file
   * `SteamClientDll`: path to `steamclient(64).dll`, either full path or relative to this `.ini` file  
   * For debug **build** only:
     * `ResumeByDebugger`: setting this to `1` or 'y' or `true` will prevent the loader from calling `ResumeThread` on the main thread after spawning the .exe, and it will display a mesage with the process ID (PID) so you attach your debugger on it.  
     Note that you have to resume the main thread from the debugger after attaching, also the entry breakpoint may not be set automatically, but you can do that manually.  


**Note** that any arguments passed to `steamclient_loader.exe` via command line will be passed to the target `.exe`.  
Example: `steamclient_loader.exe` `-dx11`  
If the additional exe arguments were both: passed via command line and set in the `.ini` file, then both will be cocatenated and passed to the exe.  
This allows the loader to be used/called from other external apps which set additional args.  
