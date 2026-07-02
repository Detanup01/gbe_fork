## :large_orange_diamond: **This is a fork**
Fork of https://gitlab.com/Mr_Goldberg/goldberg_emulator  

### Feel free to make a PR.

---

:red_circle:  

**This fork is not a takeover, not a resurrection of the original project, and not a replacement.**  
**This is just a fork, don't take it seriously.**  
**You are highly encouraged to fork/clone it and do whatever you want with it.**  

:red_circle:

---

## **Compatibility**
This fork is incompatible with the original repo, lots of things has changed and might be even broken.  
If something doesn't work, feel free to create a pull request with the appropriate fix, otherwise ignore this fork and use the original emu.  

---

## **Credits**
Thanks to everyone contributing to this project in any way possible, we try to keep the [CHANGELOG.md](./CHANGELOG.md) updated with all the changes and their authors.  

This project depends on many third-party libraries and tools, credits to them for their amazing work, you can find their listing here in [CREDITS.md](./CREDITS.md).  

---

# How to use the emu
* **Always generate the interfaces file using the `generate_interfaces` tool.**  
* **If things don't work, try the `ColdClientLoader` setup.**  

You can find helper guides, scripts, and tools here:

**(These guides, scripts, and tools are maintained by their authors.)**

* **[gbe_fork_tools](https://github.com/Detanup01/gbe_fork_tools)**
* **[gen.emu.sharp](https://github.com/otavepto/gen.emu.sharp)**
* **[gse_fork_tools](https://github.com/alex47exe/gse_fork_tools)**
* **[Semuexec](https://gitlab.com/detiam/Semuexec)**
* **[Steam Emu Utility](https://github.com/turusudiro/SteamEmuUtility)**
* **[How to use Goldberg Emulator](https://rentry.co/goldberg_emulator)**
* **[GSE-Generator](https://github.com/brunolee-GIT/GSE-Generator)**
* **If you created a generator tool create a Feature PR**

You can also find instructions here in [README.release.md](./post_build/README.release.md)  

---
---

<br/>

# **Compiling**

## Prerequisites

### Linux (Ubuntu/Debian)
```shell
sudo apt update -y
sudo apt install -y clang lld ninja-build cmake git python3
```

### Windows
* Install Visual Studio 2022+ with "Desktop development with C++" workload
* Or use MSVC + CMake from command line

---

## **Building**

This project uses CMake with FetchContent -- all dependencies are downloaded and built automatically.  
No submodules or manual dependency management required.

### On Linux (Clang + Ninja)
```shell
./build_linux.sh [Release|Debug]
```

Or manually:
```shell
cmake -S . -B build/linux \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-linux-native.cmake \
    -G Ninja
cmake --build build/linux -j "$(nproc)"
```

### On Windows (MSVC)
```shell
cmake -S . -B build/win -G "Visual Studio 18 2026"
cmake --build build/win --config Release
```

### Cross-compile Windows from Linux (Clang-cl + msvc-wine)
Requires the [msvc-wine](https://github.com/mstorsjo/msvc-wine) SDK installed at `/opt/msvc` (or set `WINDOWS_SDK_PATH`).

```shell
./build_windows.sh [Release|Debug]
```

Or manually:
```shell
cmake -S . -B build/windows \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE=cmake/toolchain-linux-winsdk.cmake \
    -DGBE_BUILD_TOOLS=ON \
    -DGBE_BUILD_TESTS=OFF \
    -DGBE_BUILD_STEAMCLIENT=ON \
    -G Ninja
cmake --build build/windows -j "$(nproc)"
```

Output: `build/windows/` → `steam_api64.dll`, `steamclient64.dll`, `GameOverlayRenderer64.dll`, etc.

### CMake Options
| Option | Default | Description |
|--------|---------|-------------|
| `GBE_BUILD_EXPERIMENTAL` | OFF | Build with ImGui overlay |
| `GBE_BUILD_TESTS` | OFF | Build tests |
| `GBE_BUILD_TOOLS` | ON | Build tools (lobby_connect, etc.) |
| `GBE_BUILD_STEAMCLIENT` | ON | Build steamclient DLL |

---

## **Output**

Output files go to:
- Linux: `build/linux/` → `libsteam_api.so`, `steamclient.so`, etc.
- Windows (MSVC): `build/win/` → `steam_api64.dll`, `steamclient64.dll`, etc.
- Windows (cross-compile): `build/windows/` → `steam_api64.dll`, `steamclient64.dll`, etc.

---

## ***(Optional)* Packaging**

### On Windows:
```batch
package_win.bat <build_folder>
```

### On Linux:
```shell
package_linux.sh <build_folder>
```
