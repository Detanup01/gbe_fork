This directory contains additional resources used during build.  

* The folder [win](./win/) contains the resources added to the .dll/.exe binaries,  
  these include version info and an immitation of any extra resources found in the original .dll/.exe.  
  
  These resources are built using Microsoft's resourec compiler `rc.exe` during the build process,  
  and the output files are stored in `build\tmp\win\rsrc` as `*.res`.  

  These resources are later passed to the compiler `cl.exe` as any normal `.cpp` or `.c` file:  
  ```bash
  cl.exe myfile.cpp myres.res -o myout.exe
  ```
    * [api](./win/api/): contains an immitation of the resources found in `steam_api(64).dll`
    * [client](./win/client/): contains an immitation of the resources found in `steamclient(64).dll`
    * [launcher](./win/launcher/): contains an immitation of the resources found in `steam.exe`
    * [file_dos_stub](./win/file_dos_stub/): contains an immitation of how the DOS stub is manipulated after build

