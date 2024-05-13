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

local default_link = {
       "ssq.lib",
       "libcurl.lib",
       "libprotobuf-lite.lib",
       "zlibstatic.lib",
       "mbedcrypto.lib"
   }

local basic_dir_win = "build/deps/win/"
local basic_dir_linux = "build/deps/linux/"

local overlay_link = {
       "ingame_overlay.lib", "system.lib", "mini_detour.lib"
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

-- END of predefines


if _ACTION == "generateproto" then
    print("Generating from .proto file!")
    -- todo edit this path!
    os.execute('call ' .. _MAIN_SCRIPT_DIR ..'/build/deps/win/protobuf/install32/bin/protoc.exe dll/net.proto -I./dll/ --cpp_out=dll/proto_gen/win')
    print("Generation success!")
 end

workspace "GBE"
    configurations { "Debug", "Release", "ExperimentalDebug", "ExperimentalRelease" }
    location "GBE_Build"

project "SteamEmu"
    cppdialect("C++17")
    kind "SharedLib"
    language "C++"
    targetdir "bin/steamEmu/%{cfg.buildcfg}_%{cfg.platform}"
    location "GBE_Build/SteamEmu"
    staticruntime "on"

    optimize "On"
    symbols "Off"


    links {
        default_link
    }



-- BASIC FOR WINDOWS 
    filter "options:os=windows"
        files {
            default_files,
            predefined_libs,
            crash_win
        }
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

-- BUILD X64 and X85 on VS. (soon more!)
    filter "action:vs*"
        platforms { "x86_64", "x86" }

-- SET ARCH
    filter "platforms:x86"
        targetname "steam_api"
        architecture "x86" 

    filter "platforms:x86_64"
        targetname "steam_api64"
        architecture "x86_64"

-- WIN 32 DEFAULTS
    filter { "platforms:x86", "options:os=windows" }
        links {
            win_link,
            default_link,
            overlay_link
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
    filter { "platforms:x86_64", "options:os=windows" }
        links {
            win_link,
            default_link,
            overlay_link
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
            "overlay_experimental/**"
        }
        removefiles { "libs/detours/uimports.cc" }
        defines { "NDEBUG", "EMU_EXPERIMENTAL_BUILD", "ImTextureID=ImU64" }
-- ExperimentalRelease WINDOWS
    filter { "ExperimentalRelease", "options:os=windows" }
        files {
            default_files,
            "libs/**",
            crash_win,
            "controller/**",
            "overlay_experimental/**"
        }
        removefiles { "libs/detours/uimports.cc" }
        defines { "DEBUG", "EMU_RELEASE_BUILD", "EMU_EXPERIMENTAL_BUILD" ,"CONTROLLER_SUPPORT", "EMU_OVERLAY", "ImTextureID=ImU64" }
-- ExperimentalDebug LINUX
    filter { "ExperimentalDebug", "options:os=linux" }
        files {
            default_files,
            predefined_libs,
            crash_linux,
            "controller/**",
            "overlay_experimental/**"
        }
        defines { "NDEBUG", "CONTROLLER_SUPPORT", "EMU_OVERLAY", "ImTextureID=ImU64" }
-- ExperimentalRelease LINUX
    filter { "ExperimentalRelease", "options:os=linux" }
        files {
            default_files,
            predefined_libs,
            crash_linux,
            "controller/**",
            "overlay_experimental/**"
        }
        defines { "DEBUG", "EMU_RELEASE_BUILD", "CONTROLLER_SUPPORT", "EMU_OVERLAY", "ImTextureID=ImU64" }
