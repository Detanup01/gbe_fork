## What is this ?
This is a command line tool to generate the `steam_settings` folder for the emu,  
you need a Steam account to grab most info, but you can use an anonymous account with limited access to Steam data.  

<br/>

## Usage
```bash
generate_emu_config [options] <app id 1> [app id 2] [app id 3] ...
```  

---

### Available **\[options\]**
To get all available options, run the tool without any arguments.  

---

### Login:
You'll be asked each time to enter your username and password, but you can automate this prompt.  

* You can create a file called `my_login.txt` beside this tool with the following data:  
  - Your **username** on the **first** line
  - Your **password** on the **second** line  

  **But beware though of accidentally distributing your login data when using this file**.  
  ---  
* You can define these environment variables, note that these environment variables will override the file `my_login.txt`:  
  - `GSE_CFG_USERNAME`
  - `GSE_CFG_PASSWORD`  

  When defining these environment variables in a script, take care of special characters.  
  
  Example for Windows:
  ```shell
  set GSE_CFG_USERNAME=my_username
  set GSE_CFG_PASSWORD=123 abc
  generate_emu_config.exe 480
  ```

  Example for Linux:
  ```shell
  export GSE_CFG_USERNAME=my_username
  export GSE_CFG_PASSWORD=123 abc
  ./generate_emu_config 480
  ```

---

### Downloading data for new apps/games and defining extra account IDs:  
The script uses public Steam IDs (in Steam64 format) of apps/games owners in order to query the required info, such as achievement data.  
By default, it has a built-in list of public users IDs, and you can extend this list by creating a file called `top_owners_ids.txt` beside the script, then add each new ID in Steam64 format on a separate line.  

When you login with a non-anonymous account, its ID will be added to the top of the list.  

<br/>

---

## Attributions and credits

* Windows icon by: [FroyoShark](https://www.iconarchive.com/artist/froyoshark.html)  
  license: [Creative Commons Attribution 4.0 International](https://creativecommons.org/licenses/by/4.0/)  
  Source: [icon archive: Steam Icon](https://www.iconarchive.com/show/enkel-icons-by-froyoshark/Steam-icon.html)  
