This directory contains additional resources used during build.  

* The folder [win](./win/) contains the resources added to the .dll/.exe binaries,  
  these include version info and an imitation of any extra resources found in the original .dll/.exe.  
  
  These resources are built using Microsoft's resource compiler `rc.exe` during the build process,  
  and the output files are stored in `build\tmp\win\rsrc` as `*.res`.  

  These resources are later passed to the compiler `cl.exe` as any normal `.cpp` or `.c` file:  
  ```bash
  cl.exe myfile.cpp myres.res -o myout.exe
  ```
    * [api](./win/api/): contains an imitation of the resources found in `steam_api(64).dll`
    * [client](./win/client/): contains an imitation of the resources found in `steamclient(64).dll`
    * [launcher](./win/launcher/): contains an imitation of the resources found in `steam.exe`
    * [game_overlay_renderer](./win/game_overlay_renderer/): contains an imitation of the resources found in `GameOverlayRenderer(64).dll`
    * [file_dos_stub](./win/file_dos_stub/): contains an imitation of how the DOS stub is manipulated after build

