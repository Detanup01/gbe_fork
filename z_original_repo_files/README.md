# Goldberg Steam Emulator

This is a steam emulator that emulates steam online features on a LAN. It works on both Linux and Windows. For a readme on how to use it see: [The Release Readme](Readme_release.txt)

You replace the steam api .dll or .so with mine (for complete steps see the [Release Readme](Readme_release.txt)) and then you can put steam in the trash and play your games either in single player on on LAN without steam (Assuming the games have no DRM and use Steam for online).

If you are a game developper and made the mistake of depending too much on the steam api and want to release of version of your game without it and don't want to rewrite your game, this is for you. It is licenced LGPLv3+ so the only source code you need to publish is the source code of this emulator (and only if you make modification to it).

## How to use

Replace the steam_api(64).dll (libsteam_api.so on linux) from the game with mine. For linux make sure that if the original api is 32 bit you use a 32 bit build and if it's 64 bit you use a 64 bit build.

Put a steam_appid.txt file that contains the appid of the game right beside it if there isn't one already.

If your game has an original steam_api(64).dll or libsteam_api.so older than may 2016 (On windows: Properties->Digital Signatures->Timestamp) you might have to add a steam_interfaces.txt beside my emulator library if the game isn't working. There is a linux script to generate it in the scripts folder of this repo.

For more information see: [The Release Readme](Readme_release.txt)

## Download Binaries

You can download the latest git builds for Linux and Windows on [the Gitlab pages website](https://mr_goldberg.gitlab.io/goldberg_emulator/) and the stable releases in the [release section](https://gitlab.com/Mr_Goldberg/goldberg_emulator/releases) of this repo.

## Contributions

One of the reasons I made this code open source is because I want contributions. Unless your code is related to the experimental stuff it needs to work on both Linux and Windows. Having accurate behavior is more important than making games work. Having inaccurate behavior might fix one game but it will break others.

## Matrix Channel

[#goldberg:matrix.org](https://matrix.to/#/#goldberg:matrix.org)

## Building

Dependencies: protobuf-lite

#### Linux
Install protobuf-lite (the dev package) and protoc (or protobuf-compiler or whatever it's called in your distro) using your package manager.

Then do: `make`

And it will build the release build (Don't forget to to add something like `-j8` if your computer isn't a piece of shit and you want it to build at a decent speed).

To build the debug build: `make debug`

My makefile sucks so you might need to do: `make clean` if you want to build the debug build after building the release build or the opposite.

For my release build I build it on steamOS using the `build_steamos.sh` script. For it to work you need a x86 version of protobuf installed to: `../protobuf/prefix_x86/` and a x64 version installed to: `../protobuf/prefix/`

#### Windows

The first thing you should do is install git for windows. [Git for Windows](https://git-scm.com/download/win)

Then install visual studio build tools: [Microsoft Build tools](https://visualstudio.microsoft.com/thank-you-downloading-visual-studio/?sku=BuildTools&rel=16) (Make sure you install the C++ build tools. Just select `C++ build tools` in the installer and press install.)


Create a new folder somewhere on your computer.

Go in that folder then right-click open the git command prompt. (Right click in folder->Git Bash Here)

Run the commands:

```
git clone https://github.com/Microsoft/vcpkg
cd vcpkg
./bootstrap-vcpkg.bat
./vcpkg install protobuf --triplet x86-windows-static
./vcpkg install protobuf --triplet x64-windows-static

./vcpkg install curl --triplet x86-windows-static
./vcpkg install curl --triplet x64-windows-static

cd..
git clone https://github.com/BinaryAlien/libssq.git
cd libssq
mkdir build32
:: -G source: https://cmake.org/cmake/help/latest/manual/cmake-generators.7.html
cmake -G "Visual Studio 17 2022" -A Win32 -S . -B "build32"
cmake --build build32 --config Release
mkdir build64
cmake -G "Visual Studio 17 2022" -A x64 -S . -B "build64"
cmake --build build64 --config Release

cd ..
git clone https://gitlab.com/Mr_Goldberg/goldberg_emulator.git
cd goldberg_emulator
```

This should build and install all the dependencies and clone the repo. Some commands like the bootstrap-vcpkg.bat and vcpkg install might take a while.


Then to build the debug experimental version run: `build_win_debug_experimental.bat`

To build the release version run: `build_win_release.bat`

If for some reason you want to set the protobuf directories to something different you can edit: `build_set_protobuf_directories.bat`

##### Pulling the latest code

Go in the goldberg_emulator folder then right-click open the git command prompt. (Right click in folder->Git Bash Here)

Run the command:
```
git pull
```

## Building using CMake

The following targets are included withing the CMake configuration for this project:
- Shared Libraries:
  - steam_api
  - steamclient
  - steamnetworkingsockets
- Executables:
  - lobby_connect
  - generate_interfaces

While all targets are included for all platforms/build variants, there are a couple of points to note:
- All targets are supported for All Windows (x64 and x86) platform/build variant combinations
- Build all inclusion of 'steamclient' and 'steamnetworkingsockets' is disabled for All Linux/WSL platforms (the targets code does currently not support these platforms)
- Experimental build variants will fail on Linux/WSL platforms (this is due to the code of the target experimental 'steam_api' variant not supporting these platforms)

The CMake configuration for this project also includes install support. Installing the project will result in a cleaner set of output files (than the raw build files) and will copy over the appropriate readmes, tools and other support files from the projects directory.
This install is structured as followed:
```
+ install-folder
|- (lib)steam_api(64).[dll|so]
|- (lib)steamclient(64).[dll|so]
|- (lib)steamnetworkingsockets(64).[dll|so]
|- Readme_release.txt
|- Readme_debug.txt // Only for debug build's
|- Readme_experimental.txt // Only for experimental build's
|- steam_appid.EDIT_AND_RENAME.txt
|- steam_interfaces.EXAMPLE.txt
|+ lobby_connect
 |- lobby_connect(64)(.exe)
 |- Readme_lobby_connect.txt
|+ tools
 |- generate_interfaces(64)(.exe)
 |- find_interfaces.ps1
 |- find_interfaces.sh
 |- Readme_generate_interfaces.txt
|+ steam_settings.EXAMPLE
 |- ... // steam_settings example files
```
Note that if no `CMAKE_INSTALL_PREFIX` define is set for CMake generation (or another method of setting a custom installation directory is used) the default OS specific install directories will be used, these are:
- On Windows `c:/Program Files/${PROJECT_NAME}`
- On Linux `/usr/local`

Please see the ['Change the installation directory'](#change-the-installation-directory) section of this readme for more information.

### Windows

#### Prerequisistes
- Visual Studio 2019 Installed or Build Tools for Visual Studio 2019
  - Can both be obtained here: https://visualstudio.microsoft.com/downloads/
  - (Optional) If you are planning to use Visual Studio make sure you include the following workloads during installation:
    - 'Desktop Development with C++'
    - 'C++ CMake tools for Windows' (Optional of the 'Desktop Development with C++' workload)
    - (Optional) If you want build for Linux from Visual studio also include the 'Linux development with C++' workload.
- CMake ^3.6
  - Can be obtained here: https://cmake.org/download/
- VCPKG
  - Can be downloaded here: https://github.com/microsoft/vcpkg/archive/master.zip
  - (Optional) For ease of use I advise you to extract the contents of this zip (the vcpkg-master folder) along side your goldberg_emulator folder and rename the vcpkg-master folder to just vcpkg, resulting in the following folder structure:
  ```
  + some-top-level-folder
  |- vcpkg
  |- goldberg_emulator
  ```
  - Can be installed by running the `bootstrap-vcpkg.bat` from the installation folder
- protobuf ^3.1 && protobuf compiler
  - Can be installed (via VCPKG) by running `vcpkg install protobuf --triplet x64-windows-static && vcpkg install protobuf --triplet x86-windows-static`
  - Alternatively you can try to compile them from the source, for instructions see: https://github.com/protocolbuffers/protobuf )

#### Generate and Build using Visual Studio 2019
This repo includes a CMakeSettings.json file which contains the configurations for the following target platforms and build variants:
- Windows-x64-Release
- Windows-x64-Debug
- Windows-x86-Release
- Windows-x86-Debug
- Windows-x64-ExperimentalRelease
- Windows-x64-ExperimentalDebug
- Windows-x86-ExperimentalRelease
- Windows-x86-ExperimentalDebug
- Linux-x64-Release
- Linux-x64-Debug
- Linux-x86-Release
- Linux-x86-Debug
- Linux-x64-ExperimentalRelease
- Linux-x64-ExperimentalDebug
- Linux-x86-ExperimentalRelease
- Linux-x86-ExperimentalDebug
- WSL-x64-Release
- WSL-x64-Debug
- WSL-x86-Release
- WSL-x86-Debug
- WSL-x64-ExperimentalRelease
- WSL-x64-ExperimentalDebug
- WSL-x86-ExperimentalRelease
- WSL-x86-ExperimentalDebug

These configurations should be automatically loaded when opening the goldberg_emulator folder in Visual Studio.
For more information on how to use these configurations (and CMake project in Visual Studio in general) please see:
https://docs.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio?view=vs-2019

Visual Studio builds for Windows and WSL configurations from will be outputted to the following folder:
`${projectDir}\out\${workspaceHash}\build\<configuration name>`

You can also opt to install directly from Visual Studio. Visual Studio installs for Windows configurations from will be outputted to the following folder:
`${projectDir}\out\install\<configuration name>`

While using these configurations are a couple of points to note:
- If you installed VCPKG into a different folder you might need to update the 'cmakeToolchain' field for each Windows configuration in the CMakeSettings.json to reflect the new VCPKG folder location
- Linux build configurations require a connection to a target Linux system in order to work, more information on how to set this up can be found here:
  https://docs.microsoft.com/en-us/cpp/linux/connect-to-your-remote-linux-computer?view=vs-2019
- WSL build configurations require a Windows Subsystem Linux to be installed:
  https://docs.microsoft.com/en-us/cpp/linux/connect-to-your-remote-linux-computer?view=vs-2019#connect-to-wsl
- Both the Linux build system and the WSL instance require the same prerequisites as a found in the 'Building using CMake - Linux' section below
- Direct installation of the project from Visual Studio is currently only supported for Windows build configuration due to limited support for remote install in Visual Studio.

#### Generate x64
```
call "<Path to Microsoft Visual Studio Installation Folder>\2019\VC\Auxiliary\Build\vcvars64.bat"
cd "<build folder>"
cmake "<goldberg_emulator src folder>" -DVCPKG_TARGET_TRIPLET:STRING="x64-windows-static" -DCMAKE_TOOLCHAIN_FILE:STRING="<vcpkg installation folder>\scripts\buildsystems\vcpkg.cmake"
```

Note that if you are using  the Build Tools for Visual Studio 2019 the path to the vcvars64.bat is slightly diffrent:
```
call "<Path to Build Tools for Visual Studio 2019 Installation Folder>\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
```

#### Build x64
```
call "<Path to Microsoft Visual Studio Installation Folder>\2019\VC\Auxiliary\Build\vcvars64.bat"
cd "<build folder>"
nmake
```

#### Install x64
```
call "<Path to Microsoft Visual Studio Installation Folder>\2019\VC\Auxiliary\Build\vcvars64.bat"
cd "<build folder>"
nmake install
```

#### Generate x86
```
call "<Path to Microsoft Visual Studio Installation Folder>\2019\VC\Auxiliary\Build\vcvars86.bat"
cd "<build folder>"
cmake "<goldberg_emulator src folder>" -DVCPKG_TARGET_TRIPLET:STRING="x86-windows-static" -DCMAKE_TOOLCHAIN_FILE:STRING="<vcpkg installation folder>\scripts\buildsystems\vcpkg.cmake"
```

Note that if you are using  the Build Tools for Visual Studio 2019 the path to the vcvars86.bat is slightly different:
```
call "<Path to Build Tools for Visual Studio 2019 Installation Folder>\2019\BuildTools\VC\Auxiliary\Build\vcvars86.bat"
```

#### Build x86
```
call "<Path to Microsoft Visual Studio Installation Folder>\2019\VC\Auxiliary\Build\vcvars86.bat"
cd "<build folder>"
nmake
```

#### Install x86
```
call "<Path to Microsoft Visual Studio Installation Folder>\2019\VC\Auxiliary\Build\vcvars86.bat"
cd "<build folder>"
nmake install
```

### Linux

#### Prerequisistes
- Compile tools
  - Can usually be obtained via your distro's package manager (e.g. on ubuntu: `sudo apt install build-essential`)
- Cmake ^3.6
  - Can usually be obtained via your distro's package manager (e.g. on ubuntu: `sudo apt install cmake`)
- protobuf ^3.1 && protobuf compiler 
  - Can usually be obtained via your distro's package manager (e.g. on ubuntu(^disco): `sudo apt install libprotobuf-dev protobuf-compiler`)
  - Alternatively you can try to compile them from the source, for instructions see: https://github.com/protocolbuffers/protobuf )

#### Generate x64/x86
```
cd "<build folder>"
cmake "<goldberg_emulator src folder>"
```

#### Build
```
cd "<build folder>"
make
```

#### Install
```
cd "<build folder>"
make install
```

### Additional CMake Related Options

#### Change the target build system
To set the generator, append `-G "<Generator Name>"` e.g.
```
cmake .. -G "Ninja" -DVCPKG_TARGET_TRIPLET:STRING="x64-windows-static" -DCMAKE_TOOLCHAIN_FILE:STRING="..\vcpkg\scripts\buildsystems\vcpkg.cmake"
```
#### Change the target build type
To set the build type, append `-DCMAKE_BUILD_TYPE:STRING="<Build Type>"` e.g.
```
cmake .. -DVCPKG_TARGET_TRIPLET:STRING="x64-windows-static" -DCMAKE_TOOLCHAIN_FILE:STRING="..\vcpkg\scripts\buildsystems\vcpkg.cmake" -DCMAKE_BUILD_TYPE:STRING="RelWithDebInfo"
```

#### Enable the experimental build
To set the experimental build, append `-DEMU_EXPERIMENTAL_BUILD:BOOL=ON` e.g.
```
cmake .. -DVCPKG_TARGET_TRIPLET:STRING="x64-windows-static" -DCMAKE_TOOLCHAIN_FILE:STRING="..\vcpkg\scripts\buildsystems\vcpkg.cmake" -DEMU_EXPERIMENTAL_BUILD:BOOL=ON
```

#### Build with the ninja build system
To build a cmake config generated with Ninja:
```
cd "<build folder>"
ninja
```

#### Change the installation directory
To use a custom installation direction, append `-DCMAKE_INSTALL_PREFIX:STRING="<Custom Installation Directory>"` e.g.
```
cmake .. -DCMAKE_INSTALL_PREFIX:STRING="./install/" -DVCPKG_TARGET_TRIPLET:STRING="x64-windows-static" -DCMAKE_TOOLCHAIN_FILE:STRING="..\vcpkg\scripts\buildsystems\vcpkg.cmake" -DCMAKE_BUILD_TYPE:STRING="RelWithDebInfo"
```

If you do not want to preset the installation directory during the generation step you can also use a build tool or OS specific overwrite, some examples of this are:
- On Windows `nmake install prefix="<Custom Installation Directory>"`
- On Linux `make DESTDIR="<Custom Installation Directory>" install`

## Design Choices / FAQ

##### Why are there no ini files like all the other Steam Emulators?

I think that the way other steam emulators have an ini when you set everything on a per game setting is dumb. The only things that should be set on a per game setting is the things that are specific to that game like the appid, DLC, mods, interface versions, etc...

The rest like your name should be set in a global place because I don't like having to set every fucking name of everyone in an ini for every game I copy to people when I want to copy them games to play on my LAN.

My emu is made in a way that you can just install it on a game and then copy the game to people and they don't have to change anything.

I agree that the fact that I have multiple files might be dumb but it's actually not. Your filesystem is a database so why would you have to replicate that by making one config file when you can just have many. It's a lot easier to manage coding wise.

##### Is there any difference between the Windows and Linux versions?

There is no difference in functionality between the normal windows version and the linux version. Windows has a experimental build which has features that only makes sense on windows. 

##### What is the experimental version?

Read this if you want to know what it is: [The Experimental Readme](Readme_experimental.txt)

##### Is this illegal?

It's as illegal as Wine or any HLE console emulator. All this does is remove the steam dependency from your steam games.

##### But it breaks Steam DRM ?

It doesn't break any DRM. If the game has a protection that doesn't let you use a custom steam api dll it needs to be cracked before you use my emulator. Steam is a DRM as much as any API is a DRM. Steam has actual DRM called steamstub which can easily be cracked but this won't crack it for you.


##### Will you support CS:GO or Dota 2?

No, I don't care about making these games work because they use apis like the game coordinator that no other game uses. Also valve keeps changing them.

However if someone else wastes their time to get them working and I will happily merge their work.
