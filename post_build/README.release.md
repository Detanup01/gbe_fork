
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
5. Copy the the example files `steam_settings.EXAMPLE\configs.xxx.EXAMPLE.ini` and paste them inside your own `steam_settings` folder, then the postfix `.EXAMPLE`.  
6. Make sure to keep the **only** options you need, do **NOT** sepecify everything inside these .ini files


Mr.Goldberg's note:
> If the game has DRM (other than steamworks) you need to remove it first.

---

## Various configurations:
Due to the various changes and additions, it became tedious to document everything,  
so it is recommended to check each example file in the `steam_settings` folder, and the various options inside `configs.xxx.ini`.

---

When you use the emu at least once, some default configurations are saved inside the global settings folder

* `configs.main.ini`: configurations for the emu itself
* `configs.user.ini`: configurations specific to the user
* `configs.app.ini`: configurations specific to the game/app
* `configs.overlay.ini`: configurations of the overlay

---

## Saves/Settings location:
* On Windows:  
   `%appdata%\GSE Saves\`  
   For example, if your user Windows user name is `Lion`, the save location would be:  
   `C:\Users\Lion\AppData\Roaming\GSE Saves\`
* On Linux: 
   * if `XDG_DATA_HOME` is defined:  
      `$XDG_DATA_HOME/GSE Saves/`
   * Otherwise, if `HOME` is defined:  
      `$HOME/.local/share/GSE Saves/`
   * Otherwise:  
      `SAVE/GSE Saves/` (relative to the current directory, which might not be the same as app/game directory)

You can override the name of the base folder `GSE Saves` to whatever you want per game, to do this modify `configs.user.ini` inside your `steam_settings` folder and change the value of `saves_folder_name`.  
Check the example file in `steam_settings.EXAMPLE\configs.EXAMPLE.ini`.  

Any configurations inside your `steam_settings` folder will always override the global configurations.  

---

## Portable saves:

If you want to use local/portable saves, specify the parameter `local_save_path` inside your local `steam_settings/configs.user.ini`, the emu will completely ignore the global settings folder, allowing a full portable behavior

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

See `[app::paths]` inside `steam_settings.EXAMPLE\configs.app.EXAMPLE.ini` for an example.  

Note that paths are treated as relative paths **from where the steam_api dll is located**.
---

---

## Mods:
* Put your mod in the `steam_settings\mods\<MOD NUMBER>\` folder
* (Optional) Modify `mods.json` and specify `primary_filename` and `preview_filename`, other options in this json file are optional.  
* (Optional) Put the mod image/preview inside `steam_settings\mod_images\<MOD NUMBER>`

Mod data folder must be a number corresponding to the file id of the mod.

See the `steam_settings.EXAMPLE` folder for an example.

---

## Steam appid:
Put your `steam_appid.txt` in the `steam_settings` folder because this is where the emulator checks first.  

If there is no `steam_appid.txt` in the `steam_settings` folder it will try opening it from the run path of the game.  
If one isn't there it will try to load it from beside steam api dll.  

The steam appid can also be set using the `SteamAppId` or `SteamGameId` env variables (this is how real steam tells games what their appid is).

But it is highly recommended to always create this file inside `steam_settings` folder
---

---

## Custom Broadcast IPs:
If you want to set custom ips (or domains) which the emulator will send broadcast packets to, make a list of them, one on each line in: `GSE Saves\settings\custom_broadcasts.txt`  

If the custom IPs/domains are specific for one game only you can put the `custom_broadcasts.txt` in the `steam_settings` folder.  

An example is provided in `steam_settings.EXAMPLE\custom_broadcasts.EXAMPLE.txt`

---

## Achievements, Items or Inventory:
Create a file named `items.json` and/or `achievements.json` inside the `steam_settings` folder which will contain every item/achievement you want to have in your game.

An example can be found in `steam_settings.EXAMPLE` that works with Killing Floor 2.

The `items.json` syntax is simple, you **MUST** validate your `.json` file before trying to run your game or you won't have any item in your inventory.  
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
By default this emulator assumes all stats do not exist unless the option `allow_unknown_stats` in `configs.main.ini` is set to 1.  

To properly define the stats of a game, put a `stats.txt` file in the `steam_settings` folder.  
The format is:  
`stat_name=type=default value`  

The type can be
* int
* float
* avgrate

The default value is simply a number that represents the default value for the stat.  

You can use the command line tool `generate_emu_config` to generate a stats config

---

## SteamHTTP:
Create a `steam_settings\http\` folder which should contain the domain name and path to the files that will be returned by steamHTTP like so \(For example this url: `https://en.wikipedia.org/wiki/Main_Page`\):

* Create a folder `steam_settings\http\en.wikipedia.org\wiki\Main_Page`
* The `Main_Page` **file** would contain the data returned by the steamHTTP api when it tries to access: `https://en.wikipedia.org/wiki/Main_Page`  

An example that was made for payday 2 can be found in `steam_settings.EXAMPLE`  

---

## Avatar:
Copy a `png`, or a `jpg`, or a `jpeg` image file to your `GSE Settings/settings` folder and name it `account_avatar`.  
You can also place this file inside the local `steam_settings` folder of the game.  

Players avatars are shared over the local network

You can find an example in `steam_settings.EXAMPLE`

---

## Support for CPY steam_api(64).dll cracks (Windows only):
See the build in the experimental folder.

**Notes:**  
You must all be on the same LAN for it to work.

**IMPORTANT:**  
Do not run more than one steam game with the **same appid** at the same time on the same computer with this emu or there might be network issues (dedicated servers should be fine though).

---

## Overlay:
**Note: at the moment this feature is only enabled in the experimental builds**  
**It's a highly experimental feature, use at your own risk**
---

This is made possible using the amazing third-party library `Ingame Overlay project`: https://github.com/Nemirtingas/ingame_overlay  

The overlay can be enabled by modifying `configs.overlay.ini` and setting `enable_experimental_overlay=1`.  
Use `SHIFT-TAB` to open the overlay.  

This feature is **highly experimental** and might cause crashes or other problems.  

Alternatively, you can use the dedicated tool `lobby_connect` to join a game lobby.  

---

## Overlay notifications sounds:
**Note: at the moment this feature is only enabled in the experimental builds for Windows only**
---

You can place a `.wav` file called `overlay_achievement_notification.wav` inside either the local `steam_settings/sounds` folder of the game, or inside `GSE Settings/settings/sounds` folder, which will be played whenever an achievement is unlocked.  

You can place a `.wav` file called `overlay_friend_notification.wav` inside either the local `steam_settings/sounds` folder of the game, or inside `GSE Settings/settings/sounds` folder, which will be played whenever a friend sends an invitation.  

You can find an example in `steam_settings.EXAMPLE`

---

## Controller:
**Note: at the moment this feature is only enabled in the Windows experimental builds and the linux builds**
---

SteamController/SteamInput support is limited to **XInput** controllers.  
---
If your controller is not XInput, there are many tools (at least for windows) that you can use to make it emulate an XInput one.

Steam uses things called action sets for controller configuration. An action set is a group of action names.  
Action names are bound to buttons, triggers or joysticks.  
The emulator needs to know for each action set, which button is linked to which action name.  

Create a `ACTION_SET_NAME.txt` file in the `steam_settings\controller\` folder for every action set the game uses.  

To see an example for the game Crystar see: `steam_settings.EXAMPLE\controller.EXAMPLE`  

In the action set txt files the format is:  
* For digital actions (buttons, on or off):  
  `ACTION_NAME=BUTTON_NAME`
* For analog actions (joysticks, triggers):
  `ACTION_NAME=ANALOG_NAME=input source mode`

Actions can be bound to more than one button by separating the buttons with, like this:  
`ACTION_NAME=A,B`

You can use the command line tool `generate_emu_config` to generate a config file.  
Or if you want to configure a game yourself, find the `vdf` file for `xbox360` or `xbox one` controller of the game and use the tool `parse_controller_vdf`, you should be able to figure things out.  

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
Modify `configs.main.ini` and set `new_app_ticket=1` and additionally `gc_token=1`

---

## Fake Windows dll/exe certificate and antivirus software:

The Windows build is signed with a fake self-signed certificate, this helps in bypassing some basic checks by apps,  
but it also triggers some antivirus software.  

First of all, **never** install these certificates to your certificate store, they are randomly generated.  
Second, the project is not a malware, if your antivirus software complains, be sure it's a false-positive.  

---

## List of valid steam languages:
Check this link for a complete list: https://partner.steamgames.com/doc/store/localization/languages  
Look for the column `API language code`

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

## Bypass overlay and auto accept game/lobby invites:
**Note: at the moment this feature is only enabled in the experimental builds**
---

When the overlay is enabled and working, you can bypass it and auto-accept invites (lobby or game) from a list of Steam IDs (SteamID64 format).  
The configuration file `auto_accept_invite.txt` allows you to do that, it works like this:  
* Adding an empty file: will accept invites from anyone (same behavior as if the overlay was disabled)
* Adding a file with some friends IDs (each on a separate line):
  * If the friend ID is found in this file, the invitation will be accepted automatically
  * If the friend ID is not found, you'll get the regular overlay invitation  

Check the example file in the `steam_settings` folder  

---

## Enable non-LAN behavior in `steam_matchmaking_servers`:

By default, match making servers (which handles browsing for matches) will always return LAN servers list whenever the game inquires about the available servers with a specific type (Internet, Friends, LAN, etc...).  
You can make the emu return the proper/actual servers list for the given type, by modifying `configs.main.ini` and setting `matchmaking_server_list_actual_type-1`.  
**This is currently broken**.  

Also, match making servers will return the info of the server from the incoming local packets, you can make the emu retrieve the actual server info by performing a source server query, this is enabled by setting `matchmaking_server_details_via_source_query-1` inside `configs.main.ini`.  
**This is currently broken**.  

---
