-- pre-define stuffs
local default_files = {
    "dll/**",
    "sdk/**",
    "helpers/**"
   }

local predefined_libs = {
       "libs/fifo_map/**",
       "libs/json/**",
       "libs/sha/**",
       "libs/simpleini/**",
       "libs/stb/**",
       "libs/utfcpp/**"
   }

local crash_win = {
       "crash_printer/win.cpp",
       "crash_printer/crash_printer/win.hpp",
       "crash_printer/tests/test_win.cpp",
       "crash_printer/tests/test_helper.hpp"
   }

local crash_linux = {
       "crash_printer/linux.cpp",
       "crash_printer/crash_printer/linux.hpp",
       "crash_printer/tests/test_linux*",
       "crash_printer/tests/test_helper.hpp"
   }

local win_link = {
       "Ws2_32.lib", "Iphlpapi.lib", "Wldap32.lib", "Winmm.lib", "Bcrypt.lib", "Dbghelp.lib"
   }

local linux_link = {
    "pthread",
    "dl",
    "ssq",
    "z", -- libz library
    "curl",
    "protobuf-lite",
    "mbedcrypto"
}

local default_link = {
       "ssq.lib",
       "libcurl.lib",
       "libprotobuf-lite.lib",
       "zlibstatic.lib",
       "mbedcrypto.lib"
   }

local basic_dir_win = "build/deps/win/"
local basic_dir_linux = "build/deps/linux/"

local overlay_link_windows = {
       "ingame_overlay.lib", "system.lib", "mini_detour.lib"
   }

local overlay_link_linux = {
    "ingame_overlay",
    "system", -- ingame_overlay dependency
    "mini_detour" -- ingame_overlay dependency
}

local default_include = {
    "dll",
    "sdk",
    "libs",
    "helpers",
    "crash_printer",
    "overlay_experimental",
    "controller"
}

local windows_files = {
    default_files,
    predefined_libs,
    crash_win
}

-- END of predefines


if _ACTION == "generateproto" then
    print("Generating from .proto file!")
    -- todo edit this path!
    os.execute('call ' .. _MAIN_SCRIPT_DIR ..'/build/deps/win/protobuf/install32/bin/protoc.exe dll/net.proto -I./dll/ --cpp_out=dll/proto_gen/win')
    print("Generation success!")
 end

workspace "GBE"
    configurations { "Debug", "Release", "ExperimentalDebug", "ExperimentalRelease" }
    platforms { "x64", "x32"  }
    location "GBE_Build"

-- Project SteamEmu
project "SteamEmu"
    cppdialect("C++latest")
    kind "SharedLib"
    language "C++"
    targetdir "bin/SteamEmu/%{cfg.buildcfg}_%{cfg.platform}"
    location "GBE_Build/SteamEmu"
    staticruntime "on"

    optimize "On"
    symbols "Off"

    -- SET ARCH
    filter "platforms:x32"
        targetname "steam_api"
        architecture "x86" 

    filter "platforms:x64"
        targetname "steam_api64"
        architecture "x86_64"

    -- BASIC FOR WINDOWS 
    filter "options:os=windows"
        buildoptions  {
            "/permissive-", "/MP4", "/DYNAMICBASE", "/utf-8", "/Zc:char8_t-", "/EHsc", "/GL-"
        }
        linkoptions  {
            "/emittoolversioninfo:no"
        }
        defines {"UTF_CPP_CPLUSPLUS=201703L", "CURL_STATICLIB", "UNICODE", "_UNICODE", "_CRT_SECURE_NO_WARNINGS" }

    -- BASIC FOR LINUX
    filter "options:os=linux"
        files {
            default_files,
            predefined_libs,
            crash_linux
        }
        defines {"UTF_CPP_CPLUSPLUS=201703L", "CURL_STATICLIB", "GNUC" }
        buildoptions  {
            "-fvisibility=hidden", "-fexceptions", "-fno-jump-tables", "-fno-char8_t"
        }
        links {
            linux_link,
            overlay_link_linux
        }

    -- WIN 32 DEFAULTS
    filter { "platforms:x32", "options:os=windows" }
        files {
            windows_files
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
      

    -- WIN 64 DEFAULTS
    filter { "platforms:x64", "options:os=windows" }
        files {
            windows_files
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

    -- DEBUG ALL
    filter "configurations:Debug"
        defines { "DEBUG" }
    -- Release ALL
    filter "configurations:Release"
        defines { "NDEBUG", "EMU_RELEASE_BUILD" }

    -- ExperimentalDebug WINDOWS
    filter { "ExperimentalDebug", "options:os=windows" }
        files {
            default_files,
            "libs/**",
            crash_win,
            "controller/**",
            "overlay_experimental/**",
            "resources/win/api/32/resources.rc"
        }
        removefiles { "libs/detours/uimports.cc" }
        defines { "DEBUG", "EMU_EXPERIMENTAL_BUILD", "ImTextureID=ImU64" }
    -- ExperimentalRelease WINDOWS
    filter { "ExperimentalRelease", "options:os=windows" }
        files {
            default_files,
            "libs/**",
            crash_win,
            "controller/**",
            "overlay_experimental/**",
            "resources/win/api/32/resources.rc"
        }
        removefiles { "libs/detours/uimports.cc" }
        defines { "NDEBUG", "EMU_RELEASE_BUILD", "EMU_EXPERIMENTAL_BUILD" ,"CONTROLLER_SUPPORT", "EMU_OVERLAY", "ImTextureID=ImU64" }
    -- ExperimentalDebug LINUX
    filter { "ExperimentalDebug", "options:os=linux" }
        files {
            default_files,
            predefined_libs,
            crash_linux,
            "controller/**",
            "overlay_experimental/**"
        }
        defines { "DEBUG", "CONTROLLER_SUPPORT", "EMU_OVERLAY", "ImTextureID=ImU64" }
    -- ExperimentalRelease LINUX
    filter { "ExperimentalRelease", "options:os=linux" }
        files {
            default_files,
            predefined_libs,
            crash_linux,
            "controller/**",
            "overlay_experimental/**"
        }
        defines { "NDEBUG", "EMU_RELEASE_BUILD", "CONTROLLER_SUPPORT", "EMU_OVERLAY", "ImTextureID=ImU64" }

-- end SteamEmu

-- Project SteamClient
project "SteamClient"
    cppdialect("C++latest")
    kind "SharedLib"
    language "C++"
    targetdir "bin/SteamClient/%{cfg.buildcfg}_%{cfg.platform}"
    location "GBE_Build/SteamClient"
    staticruntime "on"

    optimize "On"
    symbols "Off"

    files {
        "steamclient/**"
    }

    -- BASIC FOR WINDOWS 
    filter "options:os=windows"
        buildoptions  {
            "/permissive-", "/MP4", "/DYNAMICBASE", "/utf-8", "/Zc:char8_t-", "/EHsc", "/GL-"
        }
        linkoptions  {
            "/emittoolversioninfo:no"
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
    cppdialect("C++latest")
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


-- .