>## How to use it:
Copy both files  
- `steamclient.so`  
- `steamclient64.so`  

beside the script and specify the requird input, either from commdnline or via the configuration files: `ldr_*.txt`  

It is recommended to create a separate setup for each game via the config files  
to avoid specifying the commandline each time.  

Command line arguments will override the values in the configuration files,  
with an exception for the arguments passed to the executable, both specified  
in the cofiguration file and via command line will be passed to the executable.  

---

>## Command line arguments:
* `-exe`:   path to the executable
* `-appid`: numeric app ID
* `-cwd`:   *`(optional)`* working directory to switch to when running the executable
* `-rt`:    *`(optional)`* path to Steam runtime script
  - ex1: `~/.steam/debian-installation/ubuntu12_64/steam-runtime-heavy.sh`
  - ex2: `~/.steam/debian-installation/ubuntu12_64/steam-runtime-heavy/run.sh`
  - ex3: `~/.steam/debian-installation/ubuntu12_32/steam-runtime/run.sh`
  - ex4: `~/.steam/steam_runtime/run.sh`
* `--`:     *`(optional)`* everything after these 2 dashes will be passed directly to the executable"

**Note** that any unknown args will be passed to the executable  

### Examples:
* `./steamclient_loader.sh -exe ~/games/mygame/executable -appid 480`
* `./steamclient_loader.sh -exe ~/games/mygame/executable -appid 480 -- -opengl`
* `./steamclient_loader.sh -exe ~/games/mygame/executable -appid 480 -cwd ~/games/mygame/subdir`
* `./steamclient_loader.sh -exe ~/games/mygame/executable -appid 480 -rt ~/.steam/debian-installation/ubuntu12_64/steam-runtime-heavy/run.sh`

---

>## Configuraion files:
Additionally, you can set these values in the equivalent configuration files,  
everything on a single line for each file, except `ldr_cmd.txt`:  
* `ldr_exe.txt`: specify the executable path in this file
* `ldr_appid.txt`: specify the app ID in this file
* `ldr_cwd.txt`: *`(optional)`* specify the working directory of the executable path in this file
* `ldr_steam_rt.txt`: *`(optional)`* specify the path of the Steam runtime script in this file
* `ldr_cmd.txt`: *`(optional)`* specify aditional arguments that will be passed to the executable in this file,  
    this file has the exception that the arguments must be specified on separate lines

