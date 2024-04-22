## What is this ?
This tool will generate the ini section `app::steam_interfaces` which is **always** recommended to be created.

## How to use it ?
1. * On Windows
     * Drag `steam_api.dll` or `steam_api64.dll` on this exe
     * Or run the command line (example):
       ```batch
       generate_interfaces_file.exe steam_api64.dll
       ```
    * On Linux:
      * Run the command line (example):
        ```bash
        chmod 777 generate_interfaces_file_x64 # make sure we can execute the binary
        generate_interfaces_file_x64 libsteam_api.so
        ```
2. Copy the section `app::steam_interfaces` to your `configs.app.ini` 

---

In both cases, make sure the .dll/.so is **the original** one
---
