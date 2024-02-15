
====Goldberg Steam Emulator====
---

An emulator that supports LAN multiplayer without steam.  
https://gitlab.com/Mr_Goldberg/goldberg_emulator

---

## How to use:
1. Replace the `steam_api(64).dll` (Windows) or `libsteam_api.so` (Linux) from the game with the emu.
2. Use the command line tool `generate_emu_config` to generate the `steam_settings` folder
3. Use the command line tool `generate_interfaces` to generate the file `steam_interfaces.txt`,  
   then move it inside the folder `steam_settings`
4. Move the entire folder `steam_settings` beside the emu .dll/.so.


Mr.Goldberg's note:
> If the game has DRM (other than steamworks) you need to remove/crack it first.

---

## Saves location:
* On Windows:  
   `C:\Users\<Your windows user name>\AppData\Roaming\Goldberg SteamEmu Saves\`
* On Linux: 
   * if `XDG_DATA_HOME` is defined:  
      `$XDG_DATA_HOME/Goldberg SteamEmu Saves/`
   * Otherwise:  
      `$HOME/.local/share/Goldberg SteamEmu Saves/`

---

In the settings folder in that save location you will find 3 files (if you have used the emulator at least once):
account_name.txt (Edit this file to change your name)
listen_port.txt (Edit this file if you want to change the UDP/TCP port the emulator listens on (You should probably not change this because everyone needs to use the same port or you won't find yourselves on the network))
user_steam_id.txt (this is where your steam id is saved, you can change it (if your saves for a game are locked to a specific steam id see below for a way to change it on a per game basis) but it has to be valid)
language.txt (Edit this to change the language the emulator will report to the game, default is english, it must be a valid steam language name or the game might have weird behaviour (list provided at the end of this readme))

Note that these are global so you won't have to change them for each game. For game unique stuff (stats and remote storage) a folder is created with the appid of the game.
If you want to change your steam_id on a per game basis, simply create a settings folder in the game unique directory (Full path: C:\Users\<Your windows user name>\AppData\Roaming\Goldberg SteamEmu Saves\<appid>\settings)
In that settings folder create a user_steam_id.txt file that contains the valid steam id that you want to use for that game only.

You can also make the emu ignore certain global settings by using a force_account_name.txt, force_language.txt, force_listen_port.txt or force_steamid.txt that you put in the <path where my emu lib is>\steam_settings\ folder.
See the steam_settings.EXAMPLE folder for an example.

If for some reason you want it to save in the game directory you can create a file named local_save.txt right beside steam_api(64).dll (libsteam_api.so on linux)
The only thing that file should contain is the name of the save directory. This can be useful if you want to use different global settings like a different account name or steam id for a particular game.
Note that this save directory will be beside where the emu dll (or .so) is which may not be the same as the game path.

---

## DLC:
By default the emulator will try to unlock all DLCs (by returning true when the game calls the `BIsDlcInstalled()` function).  
If the game uses the other function you will need to provide a list of DLCs to my emulator.  

To do this first create a `steam_settings` folder right beside where you put my emulator.  
In this folder, put a `DLC.txt` file. (path will be `<path where my emu lib is>\steam_settings\DLC.txt`)

If the DLC file is present, the emulator will only unlock the DLCs in that file. If the file is empty all DLCs will be locked.

The contents of this file are:  
`appid=DLC name`  

See the steam_settings.EXAMPLE folder for an example.

---

## Languages:
You can include a `steam_settings\supported_languages.txt` file with a list of languages that the game supports.  

If the global emu language setting is not in this list of languages, the emu will default to the first language in the list.  

See the `steam_settings.EXAMPLE` folder for an example.

---

## Depots:
This is pretty rare but some games might use depot ids to see if dlcs are installed.  
You can provide a list of installed depots to the game with a `steam_settings\depots.txt` file.  

See the `steam_settings.EXAMPLE` folder for an example.

---

## Subscribed Groups:
Some games like payday 2 check which groups you are subscribed in and unlock things based on that.  
You can provide a list of subscribed groups to the game with a `steam_settings\subscribed_groups.txt` file.  

See `steam_settings.EXAMPLE\subscribed_groups.EXAMPLE.txt` for an example for payday 2.

---

## Subscribed Groups (Clans):
Some games like counter-strike check which groups you are subscribed in and allow you to choose a group clan.  
You can provide a list of subscribed group ids, names, and tags to the game with a `steam_settings\subscribed_groups_clans.txt` file.  

Group ids must be valid and can be obtained by pasting '`/memberslistxml/?xml=1`' at the end of a Steam group page.  

See `steam_settings.EXAMPLE\subscribed_groups_clans.EXAMPLE.txt` for an example.

---

## App paths:
Some rare games might need to be provided one or more paths to app ids.  
For example the path to where a dlc is installed.  
This sets the paths returned by the `Steam_Apps::GetAppInstallDir` function.  

See `steam_settings.EXAMPLE\app_paths.EXAMPLE.txt` for an example.  

This file should be put here: `<path where my emu lib is>\steam_settings\app_paths.txt`  

Note that paths are treated as relative paths from where the steam_api dll is located.
---

---

## Mods:
Put your mods in the `steam_settings\mods\` folder.  

Mod data folder must be a number corresponding to the file id of the mod.

See the `steam_settings.EXAMPLE` folder for an example.

---

## Steam appid:
Put your `steam_appid.txt` in the `steam_settings` folder because this is where the emulator checks first.  

If there is no `steam_appid.txt` in the `steam_settings` folder it will try opening it from the run path of the game.  
If one isn't there it will try to load it from beside my steam api dll.  

The steam appid can also be set using the `SteamAppId` or `SteamGameId` env variables (this is how real steam tells games what their appid is).

But it is highly recommended to always create this file inside `steam_settings` folder
---

---

## Offline mode:
Some games that connect to online servers might only work if the steam emu behaves like steam is in offline mode.  
If you need this, create `offline.txt` file in the `steam_settings` folder.

---

## Disable networking:
If you want to disable all the networking functionality of the emu, you can create a `disable_networking.txt` file in the `steam_settings` folder.  
**This will of course break all the networking functionality** so games that use networking related functionality like lobbies or those that launch a server in the background will not work.

---

## Custom Broadcast IPs:
If you want to set custom ips (or domains) which the emulator will send broadcast packets to, make a list of them, one on each line in: `Goldberg SteamEmu Saves\settings\custom_broadcasts.txt`  

If the custom IPs/domains are specific for one game only you can put the `custom_broadcasts.txt` in the `steam_settings` folder.  

An example is provided in `steam_settings.EXAMPLE\custom_broadcasts.EXAMPLE.txt`

---

## Achievements, Items or Inventory:
Create a file named `items.json` and/or `achievements.json` inside the `steam_settings` folder which will contain every item/achievement you want to have in your game.

An example can be found in `steam_settings.EXAMPLE` that works with Killing Floor 2.

The `items.json` syntax is simple, you **SHOULD** validate your `.json` file before trying to run your game or you won't have any item in your inventory.  
Just look for "online json validator" on your web brower to valide your file.

You can use https://steamdb.info/ to list items and attributes they have and put them into your .json, you can also use the command line tool `generate_emu_config`.

Keep in mind that some item are not valid to have in your inventory.  
---
For example, in PayDay2 all items below `item_id` `50000` will make your game crash.  
* `items.json` should contain all the item definitions for the game,  
* `default_items.json` is the quantity of each item that you want a user to have initially in their inventory. By default the user will have no items.

It is recommended to use the command line tool `generate_emu_config` for that matter

---

## Leaderboards:
By default the emulator assumes all leaderboards queried by the game `FindLeaderboard()` exist and creates them with the most common options (sort method descending, display type numeric).  
In some games this default behavior doesn't work and so you may need to tweak which leaderboards the game sees.

To do that, you can put a `leaderboards.txt` file in the `steam_settings` folder.  
An empty `leaderboards.txt` makes the emu behave as if any leaderboard queried by the game using `FindLeaderboard()` does not exist.  

The format is:  
`LEADERBOARD_NAME=sort method=display type`  

For the sort methods:
* 0 = none
* 1 = ascending
* 2 = descending

For the display type
* 0 = none
* 1 = numeric
* 2 = time seconds
* 3 = milliseconds

An example can be found in `steam_settings.EXAMPLE`

---

## Stats:
By default this emulators assumes all stats do not exist unless they have been written once by the game.  
This works for the majority of games but some games might read a stat for the first time
and expect a default value to be read when doing so.  

To set the type for each stat along with the default value, put a `stats.txt` file in the `steam_settings` folder.  
The format is:  
`STAT_NAME=type=default value`  

The type can be
* int
* float
* avgrate

The default value is simply a number that represents the default value for the stat.  

You can use the command line tool `generate_emu_config` to generate a stats config

---

## Build id:
Add a `steam_settings\build_id.txt` with the build id if the game doesn't show the correct build id and you want the emu to give it the correct one.  

An example can be found in `steam_settings.EXAMPLE`

---

## SteamHTTP:
Add a `steam_settings\http` folder.  
The folder should contain the domain name and path to the files that will be returned by steamHTTP like so \(For example this url: `https://en.wikipedia.org/wiki/Main_Page`\):

* Create a folder `steam_settings\http\en.wikipedia.org\wiki\Main_Page`
* The `Main_Page` **file** would contain the data returned by the steamHTTP api when it tries to access: `https://en.wikipedia.org/wiki/Main_Page`  

An example that was made for payday 2 can be found in `steam_settings.EXAMPLE`  

To allow external downloads which will be stored in this `steam_settings\http` folder copy the `disable_lan_only.txt` file to the `steam_settings` folder.  

---

## Avatar:
Copy a `PNG` or `JPG` image to your `Goldberg SteamEmu Settings\settings` folder and name it `account_avatar`

You can also set a default profile picture for users who are missing one by copying a similar file called `account_avatar_default`  

You can find example in `steam_settings.EXAMPLE`

---

## Support for CPY steam_api(64).dll cracks:
See the build in the experimental folder.

**Notes:**  
You must all be on the same LAN for it to work.

**IMPORTANT:**  
Do not run more than one steam game with the **same appid** at the same time on the same computer with my emu or there might be network issues (dedicated servers should be fine though).

---

## Overlay:
**Note: at the moment this feature is only enabled in the windows experimental builds**
---

The overlay can be disabled by putting a file named `disable_overlay.txt` in the `steam_settings` folder.  

This is for games that depend on the steam overlay to let people join multiplayer games.  
Use `SHIFT-TAB` to open the overlay.

---

## Controller:
**Note: at the moment this feature is only enabled in the windows experimental builds and the linux builds**
---

SteamController/SteamInput support is limited to **XInput** controllers.  
If your controller is not XInput, there are many tools (at least for windows) that you can use to make it emulate an XInput one.

Steam uses things called action sets for controller configuration. An action set is a group of action names.  
Action names are bound to buttons, triggers or joysticks.  
The emulator needs to know for each action set, which button is linked to which action name.  

Create a `ACTION_SET_NAME.txt` file in the `steam_settings\controller` folder for every action set the game uses.  

To see an example for the game Crystar see: `steam_settings.EXAMPLE\controller.EXAMPLE`  

In the action set txt files the format is:  
* For digital actions (buttons, on or off):  
  `ACTION_NAME=BUTTON_NAME`
* For analog actions (joysticks, triggers):
  `ACTION_NAME=ANALOG_NAME=input source mode`

Actions can be bound to more than one button by separating the buttons with, like this:  
`ACTION_NAME=A,B`

You can use the command line tool `generate_emu_config` to generate a config file.  
Or if you want to configure a game yourself, find the xbox360 or xbox one `vdf` file for the game and you should be able to figure things out.  

For example to get the vdf file for the game Crystar: https://steamdb.info/app/981750/config/  
If you look at: `steamcontrollerconfigdetails`, you will see something like: `1779660455/controller_type: controller_xbox360`  
`1779660455` refers to a file id that you can dl using your favorite steam workshop downloader site.  
The url would be: https://steamcommunity.com/sharedfiles/filedetails/?id=1779660455

The glyphs directory contains some glyphs for the controller buttons for the games that use the `GetGlyphForActionOrigin()` function.
If you want to use the real steam glyphs instead of the free ones in the example directory copy them from: `<Steam Directory>\tenfoot\resource\images\library\controller\api` folder.

### Valid digital button names:
* DUP
* DDOWN
* DLEFT
* DRIGHT
* START
* BACK
* LSTICK
* RSTICK
* LBUMPER
* RBUMPER
* A
* B
* X
* Y
* DLTRIGGER  (emulated buttons, the joy ones are used by games in menus for example. When the game wants to know if the trigger is pressed without the intensity)
* DRTRIGGER
* DLJOYUP
* DLJOYDOWN
* DLJOYLEFT
* DLJOYRIGHT
* DRJOYUP
* DRJOYDOWN
* DRJOYLEFT
* DRJOYRIGHT

### Valid analog names:
* LTRIGGER
* RTRIGGER
* LJOY
* RJOY
* DPAD

---

## Auth token (app ticket):

By default the emu will send the old token format for various APIs, like:  
* `Steam_GameServer::GetAuthSessionTicket()`
* `Steam_User::GetAuthSessionTicket()`
* `Steam_User::GetAuthTicketForWebApi()`

You can make the emu generate new ticket data, and additionally the GC token.  
Check the relevant files `new_app_ticket.txt` and `gc_token.txt` in the `steam_settings` folder

---

## Branch name and type:

By default the emu will report a `non-beta` branch with the name `public` when the game calls `Steam_Apps::GetCurrentBetaName()`  

Check the relevant files `is_beta_branch.txt` and `force_branch_name.txt` in the `steam_settings` folder

---

## Crash log/printer:

The emu can setup a very basic crash logger/printer.  
This is intended to debug some annoying scenarios, and best used with the debug build of the emu.  

To enable this feature create a file called `crash_printer_location.txt` inside your `steam_settings` folder,  
and set the path to the crash log file on a single line.  

Note that forward slashes `/` are encouraged for both Windows & Linux.  

Check the example file `crash_printer_location.EXAMPLE.txt`  

---

## Fake Windows dll/exe certificate and antivirus software:

The Windows build is signed with a fake self-signed certificate, this helps in bypassing some basic checks by apps,  
but it also triggers some antivirus software.  

First of all, **never** install these certificates to your certificate store, they are randomly generated.  
Second, the project is not a malware, if your antivirus software complains, be sure it's a false-positive.  

---

## List of valid steam languages:
* arabic
* bulgarian
* schinese
* tchinese
* czech
* danish
* dutch
* english
* finnish
* french
* german
* greek
* hungarian
* italian
* japanese
* koreana
* norwegian
* polish
* portuguese
* brazilian
* romanian
* russian
* spanish
* latam
* swedish
* thai
* turkish
* ukrainian
* vietnamese

---

## Overlay warnings:
**Note: at the moment this feature is only enabled in the windows experimental builds**

These configuration files allow disabling various overlay warnings:  
* `disable_overlay_warning_forced_setting.txt`:  
   - disable the warning for the usage of any file `force_*.txt` in the overlay
   - unlocks the settigs menu, this may result in an undesirable output
* `disable_overlay_warning_bad_appid.txt`: disable the warning for bad app ID (when app ID = 0) in the overlay
* `disable_overlay_warning_local_save.txt`: disable the warning for using local save in the overlay
* `disable_overlay_warning_any.txt`: all the above  

Check the exaxmple files in the `steam_settings` folder

---

## Overlay appearance:
**Note: at the moment this feature is only enabled in the windows experimental builds**

These configuration file `overlay_appearance.txt` has various options to set for the overlay appearance.  
The notifications positions could be set to one of these values:  
* `top_left`
* `top_center`
* `top_right`
* `bot_left`
* `bot_center`
* `bot_right`

Check the exaxmple files in the `steam_settings` folder

---

## Auto accept game/lobby invites:

When the overlay is enabled and working, you can bypass it and auto-accept invites (lobby or game) from a list of Steam IDs (SteamID64 format).  
The configuration file `auto_accept_invite.txt` allows you to do that, it works like this:  
* Adding an empty file: will accept invites from anyone (same behavior as if the overlay was disabled)
* Adding a file with some friends IDs (each on a separate line):
  * If the friend ID is found in this file, the invitation will be accepted automatically
  * If the friend ID is not found, you'll get the regular overlay invitation  

Check the exaxmple file in the `steam_settings` folder  

---

## IP country:

You can report a country IP if the game queries it, by setting the 2 characters code in the file `ip_country.txt`.  
Use this link to get the `Alpha-2` country code: https://www.iban.com/country-codes  

Check the exaxmple file in the `steam_settings` folder

---

## More configurations:
Due to the various changes and additions, it became tedious to document everything,  
so it is recommended to check each example file in the `steam_settings` folder.
