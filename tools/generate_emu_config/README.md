## What is this ?
This is a command line tool to generate the `steam_settings` folder for the emu,  
you need a Steam account to grab most info, but you can use an anonymous account with limited access to Steam data.  

<br/>

## Usage
```bash
generate_emu_config [options] <app id 1> [app id 2] [app id 3] ...
```  

---

### Login:
You'll be asked each time to enter your username and password.  
You can also place a file called `my_login.txt` beside this tool with the following data:  
* Your **username** on the **first** line
* Your **password** on the **second** line  

**But beware though of accidentally distributing your login data when using this file**.  
---

---

### Account ID:
The script uses public Steam IDs (in Steam64 format) of apps/games owners in order to query the required info, such as achievement data.  
By default, it has a built-in list of public users IDs, and you can extend this list by creating a file called `top_owners_ids.txt` beside the script, then add each new ID in Steam64 format on a separate line.  

When you login with a non-anonymous account, its ID will be added to the top of the list.  

<br/>

## Available **\[options\]**
To get all available options, run the tool without any arguments.  
* `app id 1`: the ID of the first app, this is mandatory
* `app id <n>`: multiple/more app IDs to grab the data for
* `-shots`: download some additional screenshots for the app/game
* `-thumbs`: download some additional thumbnails for the app/game
* `-vid`: attempt to download the trailer/promo video
* `-imgs`: download common Steam imgaes, this includes a poster, Steam generated background, icon(s), and some other assets
* `-name`: output the data in a folder with the name of the app/game, illegal characters are removed
* `-cdx`: generate `.ini` file to be used by CODEX emu, this is quite outdated and needs manual editiong
* `-aw`: generate all schemas for Achievement Watcher by xan105: https://github.com/xan105/Achievement-Watcher  
* `-clean`: attempt to cleanup/remove the output folder from previous runs, if it exists, before downloading the data again
* `-anon`: use anonymous account to login to Steam instead of using your own,  
  note that this account has very limited access to data, and it can't download most info
* `-nd`: don't generate `disable_xxx.txt` files

### Example

```bash
generate_emu_config -shots -thumbs -vid -imgs -name -cdx -aw -clean 421050 480
```  

---

## Attributions and credits

* Windows icon by: [FroyoShark](https://www.iconarchive.com/artist/froyoshark.html)  
  license: [Creative Commons Attribution 4.0 International](https://creativecommons.org/licenses/by/4.0/)  
  Source: [icon archive: Steam Icon](https://www.iconarchive.com/show/enkel-icons-by-froyoshark/Steam-icon.html)  
