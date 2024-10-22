## What is this ?
This is a command line tool to generate complete custom configs, including the `steam_settings` folder for the emu.  
You need a Steam account to grab most info, but you can use an anonymous account with limited access to Steam data.  

## Usage
```bash
generate_emu_config [options] <app id 1> [app id 2] [app id 3] ...
```

To get all available options, run the tool without any arguments.  

---

## Using *my_login.txt*
You'll be asked each time to enter your username and password, but you can automate this prompt.  

* You can create a file called `my_login.txt` beside this tool with the following data:  
  - Your **username** on the **first** line
  - Your **password** on the **second** line  

  Beware of accidentally distributing your login data when using this file !  
  ---  
* You can define these environment variables, note that these environment variables will override the file `my_login.txt`:  
  - `GSE_CFG_USERNAME`
  - `GSE_CFG_PASSWORD`  

  When defining these environment variables in a script, take care of special characters.  
  
  ​	Example for Windows:
  ```shell
  set GSE_CFG_USERNAME=my_username
  set GSE_CFG_PASSWORD=123abc
  generate_emu_config.exe 480
  ```

  ​	Example for Linux:
  ```shell
  export GSE_CFG_USERNAME=my_username
  export GSE_CFG_PASSWORD=123abc
  ./generate_emu_config 480
  ```

---

## Using *top_owners_ids.txt*  
The script uses public Steam IDs (in Steam64 format) of apps/games owners in order to query the required info, such as achievement data.  
By default, it has a built-in list of public users IDs, which can be extended by creating a file called `top_owners_ids.txt` beside the script, then adding each new ID in Steam64 format on a separate line.  When you login with a non-anonymous account, its ID will be added to the top of the list.

Steam IDs with public profiles that own a lot of games --- https://steamladder.com/ladder/games/
How to generate/update `top_owners_ids.txt` upon running generate_emu_config:

- copy and paste the above address in your web browser
- right click and save web page, html only with the name top_owners_ids.html
- copy and paste `top_owners_ids.html` next to generate_emu_config exe or py  

---

## Attributions and credits

* Windows icon by: [FroyoShark](https://www.iconarchive.com/artist/froyoshark.html)  
  License: [Creative Commons Attribution 4.0 International](https://creativecommons.org/licenses/by/4.0/)  
  Source: [icon archive: Steam Icon](https://www.iconarchive.com/show/enkel-icons-by-froyoshark/Steam-icon.html)
