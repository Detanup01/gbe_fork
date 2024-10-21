# What is this ?
This tool allows you to migrate your `steam_settings` folder, or your global `settings` folder from the old `.txt` format, to the new `.ini` oriented format.  

## How to migrate the global settings folder
Open the terminal/cmd in the folder of the tool and run it **without** any arguments.  
* On Windows:
  ```shell
  migrate_gse.exe
  ```
* On Linux:
  ```shell
  chmod 777 migrate_gse
  ./migrate_gse
  ```
  The tool will generate a new folder in the current directory called `steam_settings`, copy the **content inside** this folder (a bunch of `.ini` files) and paste them here:
  * On Windows: `%appdata%\GSE Saves\settings\` 
  * On Linux:
    * if env var `XDG_DATA_HOME` is defined:  
      `$XDG_DATA_HOME/GSE Saves/settings/`
    * Otherwise, if env var `HOME` is defined:  
      `$HOME/.local/share/GSE Saves/settings/`

Notice the new name of the global saves folder `GSE Saves`, not to be confused with the old one `Goldberg SteamEmu Saves`  
Finally, copy everything from `Goldberg SteamEmu Saves` to `GSE Saves`  


## How to migrate a local steam settings folder
Open the terminal/cmd in the folder of the tool and run it with only one argument, which is the path to the target `steam_settings` folder.  
* On Windows:
  ```shell
  migrate_gse.exe "D:\my games\some game\steam_settings"
  ```
* On Linux (notice how the tilde character is outside the quotes to allow it to expand):
  ```shell
  chmod 777 migrate_gse
  ./migrate_gse ~/"some game/steam_settings"
  ```
  The tool will generate a new folder in the current directory called `steam_settings`, copy the **content inside** this folder (a bunch of `.ini` files) and paste them inside the target/old `steam_settings` folder  


## General notes
* In all cases, the tool will not copy the achievements images, overlay fonts/sounds, and all the extra stuff, it will just generate the corresponding `.ini` files  

* Some configuration files are still using the same old format, that includes all the `.json` files, depots.txt, subscribed_groups_xxx.txt, etc ...  
  So don't just remove everything from the old `steam_settings` folder  
