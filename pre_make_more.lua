-- Project SteamClient
project "SteamClient"
    cppdialect("c++17")
    kind "SharedLib"
    language "C++"
    targetdir "bin/SteamClient/%{cfg.buildcfg}_%{cfg.platform}"
    location "GBE_Build/SteamClient"
    staticruntime "on"

    optimize "On"
    symbols "Off"

    -- BASIC FOR WINDOWS 
    filter "options:os=windows"
        buildoptions  {
            "/permissive-", "/MP", "/DYNAMICBASE", "/nologo",
            "/utf-8", "/Zc:char8_t-", "/EHsc", "/GF", "/GL-", "/GS"
        }
        linkoptions  {
            "/DYNAMICBASE", "/ERRORREPORT:NONE", "/NOLOGO", "/emittoolversioninfo:no"
        }
        defines { "STEAMCLIENT_DLL", "EMU_EXPERIMENTAL_BUILD" }    

    -- BASIC FOR LINUX 
    filter "options:os=linux"
        files {
            default_files,
            predefined_libs,
            crash_linux,
            "controller/**"
        }
        buildoptions  {
            "-fvisibility=hidden", "-fexceptions", "-fno-jump-tables", "-fno-char8_t"
        }
        links {
            linux_link,
            overlay_link_linux
        }
        defines { "UTF_CPP_CPLUSPLUS=201703L", "CURL_STATICLIB", "GNUC", "NDEBUG", "STEAMCLIENT_DLL", "CONTROLLER_SUPPORT", "ImTextureID=ImU64" }

        filter { "*Debug", "options:os=linux"}
            defines {"UTF_CPP_CPLUSPLUS=201703L", "CURL_STATICLIB", "GNUC",  "DEBUG", "EMU_RELEASE_BUILD", "CONTROLLER_SUPPORT", "ImTextureID=ImU64", "STEAMCLIENT_DLL" }

    -- SET ARCH
    filter "platforms:x32"
        targetname "steamclient"
        architecture "x86" 

    filter "platforms:x64"
        targetname "steamclient64"
        architecture "x86_64"

    -- WIN 32 DEFAULTS
    filter { "platforms:x32", "options:os=windows" }
        files {
            "steamclient/**",
            "resources/win/client/32/resources.rc"
        }

    -- WIN 64 DEFAULTS
    filter { "platforms:x64", "options:os=windows" }
        files {
            "steamclient/**",
            "resources/win/client/64/resources.rc"
        }

    -- WIN X32 EXP
    filter { "Experimental**", "options:os=windows",  "platforms:x32" }
        files {
            default_files,
            "libs/**",
            crash_win,
            "controller/**",
            "overlay_experimental/**",
            "resources/win/client/32/resources.rc"
        }
        links {
            win_link,
            default_link,
            overlay_link_windows
        }
        libdirs {
            basic_dir_win .. "libssq/build32/Release",
            basic_dir_win .. "curl/install32/lib",
            basic_dir_win .. "protobuf/install32/lib",
            basic_dir_win .. "zlib/install32/lib",
            basic_dir_win .. "mbedtls/install32/lib",
            basic_dir_win .. "ingame_overlay/install32/lib",
            basic_dir_win .. "ingame_overlay/deps/System/install32/lib",
            basic_dir_win .. "ingame_overlay/deps/mini_detour/install32/lib"
        }
        includedirs {
            default_include,
            "dll/proto_gen/win",
            basic_dir_win .. "libssq/include",
            basic_dir_win .. "curl/install32/include",
            basic_dir_win .. "protobuf/install32/include",
            basic_dir_win .. "zlib/install32/include",
            basic_dir_win .. "mbedtls/install32/include",
            basic_dir_win .. "ingame_overlay/install32/include",
            basic_dir_win .. "ingame_overlay/deps/System/install32/include",
            basic_dir_win .. "ingame_overlay/deps/mini_detour/install32/include"
        }
        removefiles { "steamclient/**" }
        removefiles { "libs/detours/uimports.cc" }
        
    -- WIN X64 EXP
    filter { "Experimental**", "options:os=windows",  "platforms:x64" }
        files {
            default_files,
            "libs/**",
            crash_win,
            "controller/**",
            "overlay_experimental/**",
            "resources/win/client/64/resources.rc"
        }
        links {
            win_link,
            default_link,
            overlay_link_windows
        }
        libdirs {
            basic_dir_win .. "libssq/build64/Release",
            basic_dir_win .. "curl/install64/lib",
            basic_dir_win .. "protobuf/install64/lib",
            basic_dir_win .. "zlib/install64/lib",
            basic_dir_win .. "mbedtls/install64/lib",
            basic_dir_win .. "ingame_overlay/install64/lib",
            basic_dir_win .. "ingame_overlay/deps/System/install64/lib",
            basic_dir_win .. "ingame_overlay/deps/mini_detour/install64/lib"
        }
        includedirs {
            default_include,
            "dll/proto_gen/win",
            basic_dir_win .. "libssq/include",
            basic_dir_win .. "curl/install64/include",
            basic_dir_win .. "protobuf/install64/include",
            basic_dir_win .. "zlib/install64/include",
            basic_dir_win .. "mbedtls/install64/include",
            basic_dir_win .. "ingame_overlay/install64/include",
            basic_dir_win .. "ingame_overlay/deps/System/install64/include",
            basic_dir_win .. "ingame_overlay/deps/mini_detour/install64/include"
        }
        removefiles { "steamclient/**" }
        removefiles { "libs/detours/uimports.cc" }

    -- ExperimentalDebug WINDOWS
    filter { "ExperimentalDebug", "options:os=windows" }
        defines { "DEBUG", "EMU_EXPERIMENTAL_BUILD", "CONTROLLER_SUPPORT", "EMU_OVERLAY", "ImTextureID=ImU64", "STEAMCLIENT_DLL", "UTF_CPP_CPLUSPLUS=201703L", "CURL_STATICLIB", "UNICODE", "_UNICODE", "_CRT_SECURE_NO_WARNINGS" }

    -- ExperimentalRelease WINDOWS
    filter { "ExperimentalRelease", "options:os=windows" }
        defines { "NDEBUG", "EMU_RELEASE_BUILD", "EMU_EXPERIMENTAL_BUILD", "CONTROLLER_SUPPORT", "EMU_OVERLAY", "ImTextureID=ImU64", "STEAMCLIENT_DLL", "UTF_CPP_CPLUSPLUS=201703L", "CURL_STATICLIB", "UNICODE", "_UNICODE", "_CRT_SECURE_NO_WARNINGS" }

    -- ExperimentalDebug LINUX
    filter { "ExperimentalDebug", "options:os=linux" }
        files {
            default_files,
            predefined_libs,
            crash_linux,
            "controller/**",
            "overlay_experimental/**"
        }
        defines {"UTF_CPP_CPLUSPLUS=201703L", "CURL_STATICLIB", "GNUC",  "DEBUG", "EMU_RELEASE_BUILD", "CONTROLLER_SUPPORT", "EMU_OVERLAY", "ImTextureID=ImU64", "STEAMCLIENT_DLL" }


    -- ExperimentalRelease LINUX
    filter { "ExperimentalRelease", "options:os=linux" }
        files {
            default_files,
            predefined_libs,
            crash_linux,
            "controller/**",
            "overlay_experimental/**"
        }
        defines {"UTF_CPP_CPLUSPLUS=201703L", "CURL_STATICLIB", "GNUC",  "NDEBUG", "EMU_RELEASE_BUILD", "CONTROLLER_SUPPORT", "EMU_OVERLAY", "ImTextureID=ImU64", "STEAMCLIENT_DLL" }

-- SteamClientExtra
if os.target() == "windows" then
project "SteamClientExtra"
    cppdialect("c++17")
    kind "SharedLib"
    language "C++"
    targetdir "bin/SteamClientExtra/%{cfg.buildcfg}_%{cfg.platform}"
    location "GBE_Build/SteamClientExtra"
    staticruntime "on"

    optimize "On"
    symbols "Off"

    buildoptions  {
        "/permissive-", "/MP4", "/DYNAMICBASE", "/utf-8", "/Zc:char8_t-", "/EHsc", "/GL-"
        }
    linkoptions  {
        "/emittoolversioninfo:no"
        }
    
    links {
        win_link,
        "user32.lib"
    }

        -- SET ARCH
    filter "platforms:x32"
        targetname "steamclient"
        architecture "x86" 

    filter "platforms:x64"
        targetname "steamclient64"
        architecture "x86_64"

    includedirs {
        "helpers",
        "libs",
        "tools/steamclient_loader/win/extra_protection"
    }
    -- WIN 32 DEFAULTS
    filter { "platforms:x32", "options:os=windows" }
        files {
            "helpers/**",
            "libs/detours/**",
            "resources/win/client/32/resources.rc",
            "tools/steamclient_loader/win/**"
        }
        removefiles { "libs/detours/uimports.cc" }

    -- WIN 64 DEFAULTS
    filter { "platforms:x64", "options:os=windows" }
        files {
            "helpers/**",
            "libs/detours/**",
            "resources/win/client/64/resources.rc",
            "tools/steamclient_loader/win/**"
        }
        removefiles { "libs/detours/uimports.cc" }

end
-- End SteamClient + SteamClientExtra
-- Project LobbyConnect
project "LobbyConnect"
    cppdialect("c++17")
    kind "ConsoleApp"
    language "C++"
    targetdir "bin/LobbyConnect/%{cfg.buildcfg}_%{cfg.platform}"
    location "GBE_Build/LobbyConnect"
    staticruntime "on"

-- TODO LOBBYCONNECT

    optimize "On"
    symbols "Off"
-- End LobbyConnect

-- Project GenerateInterfaces
project "GenerateInterfaces"
    cppdialect("c++17")
    kind "ConsoleApp"
    language "C++"
    targetdir "bin/GenerateInterfaces/%{cfg.buildcfg}_%{cfg.platform}"
    location "GBE_Build/GenerateInterfaces"
    targetname "GenerateInterfaces"
    optimize "On"
    symbols "Off"

    files {
        "tools/generate_interfaces/**"
    }
-- End LobbyConnect

-- Project steamnetworkingsockets
project "steamnetworkingsockets"
    cppdialect("c++17")
    kind "ConsoleApp"
    language "C++"
    targetdir "bin/steamnetworkingsockets/%{cfg.buildcfg}_%{cfg.platform}"
    location "GBE_Build/steamnetworkingsockets"
    targetname "steamnetworkingsockets"
    optimize "On"
    symbols "Off"

    files {
        "networking_sockets_lib/**"
    }
-- End LobbyConnect