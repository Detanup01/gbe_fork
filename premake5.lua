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
    "crash_printer/crash_printer/win.hpp"
}

local crash_linux = {
    "crash_printer/linux.cpp",
    "crash_printer/crash_printer/linux.hpp"
}

local win_link = {
    "Ws2_32.lib", "Iphlpapi.lib", "Wldap32.lib", "Winmm.lib", "Bcrypt.lib", "Dbghelp.lib"
}

local linux_link = {
    "pthread",
    "dl",
    "ssq:static",
    "z:static", -- libz library
    "curl:static",
    "protobuf-lite:static",
    "mbedcrypto:static"
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
    "ingame_overlay.lib",
    "system.lib",
    "mini_detour.lib"
}

local overlay_link_linux = {
    "ingame_overlay:static",
    "system:static", -- ingame_overlay dependency
    "mini_detour:static" -- ingame_overlay dependency
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


local linux_files = {
    default_files,
    predefined_libs,
    crash_linux,
    "controller/**"
}
-- 32 
local x32_libsdir_win = {
    basic_dir_win .. "libssq/build32/Release",
    basic_dir_win .. "curl/install32/lib",
    basic_dir_win .. "protobuf/install32/lib",
    basic_dir_win .. "zlib/install32/lib",
    basic_dir_win .. "mbedtls/install32/lib",
    basic_dir_win .. "ingame_overlay/install32/lib",
    basic_dir_win .. "ingame_overlay/deps/System/install32/lib",
    basic_dir_win .. "ingame_overlay/deps/mini_detour/install32/lib"
}

local x32_libsdir_linux = {
    basic_dir_linux .. "libssq/build32",
    basic_dir_linux .. "curl/install32/lib",
    basic_dir_linux .. "protobuf/install32/lib",
    basic_dir_linux .. "zlib/install32/lib",
    basic_dir_linux .. "mbedtls/install32/lib",
    basic_dir_linux .. "ingame_overlay/install32/lib",
    basic_dir_linux .. "ingame_overlay/deps/System/install32/lib",
    basic_dir_linux .. "ingame_overlay/deps/mini_detour/install32/lib"
}

local x32_include_win = {
    basic_dir_win .. "libssq/include",
    basic_dir_win .. "curl/install32/include",
    basic_dir_win .. "protobuf/install32/include",
    basic_dir_win .. "zlib/install32/include",
    basic_dir_win .. "mbedtls/install32/include",
    basic_dir_win .. "ingame_overlay/install32/include",
    basic_dir_win .. "ingame_overlay/deps/System/install32/include",
    basic_dir_win .. "ingame_overlay/deps/mini_detour/install32/include"
}

local x32_include_linux = {
    basic_dir_linux .. "libssq/include",
    basic_dir_linux .. "curl/install32/include",
    basic_dir_linux .. "protobuf/install32/include",
    basic_dir_linux .. "zlib/install32/include",
    basic_dir_linux .. "mbedtls/install32/include",
    basic_dir_linux .. "ingame_overlay/install32/include",
    basic_dir_linux .. "ingame_overlay/deps/System/install32/include",
    basic_dir_linux .. "ingame_overlay/deps/mini_detour/install32/include"
}

-- 32 end
-- 64
local x64_libsdir_win = {
    basic_dir_win .. "libssq/build64/Release",
    basic_dir_win .. "curl/install64/lib",
    basic_dir_win .. "protobuf/install64/lib",
    basic_dir_win .. "zlib/install64/lib",
    basic_dir_win .. "mbedtls/install64/lib",
    basic_dir_win .. "ingame_overlay/install64/lib",
    basic_dir_win .. "ingame_overlay/deps/System/install64/lib",
    basic_dir_win .. "ingame_overlay/deps/mini_detour/install64/lib"
}

local x64_libsdir_linux = {
    basic_dir_linux .. "libssq/build64",
    basic_dir_linux .. "curl/install64/lib",
    basic_dir_linux .. "protobuf/install64/lib",
    basic_dir_linux .. "zlib/install64/lib",
    basic_dir_linux .. "mbedtls/install64/lib",
    basic_dir_linux .. "ingame_overlay/install64/lib",
    basic_dir_linux .. "ingame_overlay/deps/System/install64/lib",
    basic_dir_linux .. "ingame_overlay/deps/mini_detour/install64/lib"
}

local x64_include_win = {
    basic_dir_win .. "libssq/include",
    basic_dir_win .. "curl/install64/include",
    basic_dir_win .. "protobuf/install64/include",
    basic_dir_win .. "zlib/install64/include",
    basic_dir_win .. "mbedtls/install64/include",
    basic_dir_win .. "ingame_overlay/install64/include",
    basic_dir_win .. "ingame_overlay/deps/System/install64/include",
    basic_dir_win .. "ingame_overlay/deps/mini_detour/install64/include"
}

local x64_include_linux = {
    basic_dir_linux .. "libssq/include",
    basic_dir_linux .. "curl/install64/include",
    basic_dir_linux .. "protobuf/install64/include",
    basic_dir_linux .. "zlib/install64/include",
    basic_dir_linux .. "mbedtls/install64/include",
    basic_dir_linux .. "ingame_overlay/install64/include",
    basic_dir_linux .. "ingame_overlay/deps/System/install64/include",
    basic_dir_linux .. "ingame_overlay/deps/mini_detour/install64/include"
}

-- 64 end

-- END of predefines


if _ACTION == "generateproto" then
    print("Generating from .proto file!")
    if os.target() == "windows" then
        os.mkdir("dll/proto_gen/win")
        os.execute('call ' .. _MAIN_SCRIPT_DIR .. '/' .. basic_dir_win ..'protobuf/install64/bin/protoc.exe dll/net.proto -I./dll/ --cpp_out=dll/proto_gen/win')
    end
    if os.target() == "linux" then
        os.mkdir("dll/proto_gen/linux")
        os.chmod(_MAIN_SCRIPT_DIR .. '/' .. basic_dir_linux ..'protobuf/install64/bin/protoc', "777")
        os.execute(_MAIN_SCRIPT_DIR .. '/' .. basic_dir_linux ..'protobuf/install64/bin/protoc dll/net.proto -I./dll/ --cpp_out=dll/proto_gen/linux')
    end
    print("Generation success!")
end

newoption {
    trigger = "emubuild",
    description = "Set the EMU_BUILD_STRING",
    default = "manual"
}
-- GBE Workspace
workspace "GBE"
    configurations { "Debug", "Release", "ExperimentalDebug", "ExperimentalRelease" }
    platforms { "x64", "x32"  }
    location "GBE_Build"

-- Project SteamEmu
project "SteamEmu"
    cppdialect("c++17")
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
            "/permissive-", "/MP", "/DYNAMICBASE",
            "/utf-8", "/Zc:char8_t-", "/EHsc", "/GL-"
        }
        linkoptions  {
            "/NOLOGO", "/emittoolversioninfo:no"
        }

        filter { "options:os=windows", "configurations:Release" }
            defines {
                "NDEBUG", "EMU_RELEASE_BUILD", "UTF_CPP_CPLUSPLUS=201703L", "CURL_STATICLIB", "_CRT_SECURE_NO_WARNINGS", "EMU_BUILD_STRING=".._OPTIONS["emubuild"]
            }

        filter { "options:os=windows", "configurations:Debug" }
            defines {
                "DEBUG", "UTF_CPP_CPLUSPLUS=201703L", "CURL_STATICLIB", "_CRT_SECURE_NO_WARNINGS", "EMU_BUILD_STRING=".._OPTIONS["emubuild"]
            }

        -- WIN 32 DEFAULTS
        filter { "platforms:x32", "options:os=windows" }
            files {
                windows_files,
                "resources/win/api/32/resources.rc"
            }
            links {
                win_link,
                default_link,
                overlay_link_windows
            }
            
            libdirs {
                x32_libsdir_win
            }

            includedirs {
                default_include,
                "dll/proto_gen/win",
                x32_include_win
            }

        -- WIN 64 DEFAULTS
        filter { "platforms:x64", "options:os=windows" }
            files {
                windows_files,
                "resources/win/api/64/resources.rc"
            }
            links {
                win_link,
                default_link,
                overlay_link_windows
            }
            libdirs {
                x64_libsdir_win
            }

            includedirs {
                default_include,
                "dll/proto_gen/win",
                x64_include_win
            }

    -- BASIC FOR LINUX
    filter "options:os=linux"
        files {
            linux_files
        }
        removefiles {
            "libs/simpleini/**",
            "helpers/pe_**"
        }
        buildoptions  {
            "-fvisibility=hidden", "-fexceptions", "-fno-jump-tables", "-fno-char8_t" , "-Wundefined-internal", "-Wno-switch"
        }

        linkoptions {
            "-Wl,--exclude-libs,ALL"
        }

        links {
            linux_link,
            overlay_link_linux
        }

        filter { "options:os=linux", "configurations:Release" }
            defines {
                "NDEBUG", "EMU_RELEASE_BUILD", "UTF_CPP_CPLUSPLUS=201703L", "CURL_STATICLIB", "GNUC", "CONTROLLER_SUPPORT", "EMU_BUILD_STRING=".._OPTIONS["emubuild"]
            }

        filter { "options:os=linux", "configurations:Debug" }
            defines {
                "DEBUG", "UTF_CPP_CPLUSPLUS=201703L", "CURL_STATICLIB", "GNUC", "CONTROLLER_SUPPORT", "EMU_BUILD_STRING=".._OPTIONS["emubuild"]
            }

            -- linux 32 DEFAULTS
        filter { "platforms:x32", "options:os=linux" }
            includedirs {
                default_include,
                "dll/proto_gen/linux",
                x32_include_linux
            }

            libdirs {
                x32_libsdir_linux
            }

        -- linux 64 DEFAULTS
        filter { "platforms:x64", "options:os=linux" }
            includedirs {
                default_include,
                "dll/proto_gen/linux",
                x64_include_linux
            }
            libdirs {
                x64_libsdir_linux
            }

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
        defines {
            "EMU_EXPERIMENTAL_BUILD", "CONTROLLER_SUPPORT", "EMU_OVERLAY", "ImTextureID=ImU64", "UTF_CPP_CPLUSPLUS=201703L", "CURL_STATICLIB",
            "DEBUG", "_CRT_SECURE_NO_WARNINGS", "EMU_BUILD_STRING=".._OPTIONS["emubuild"]
        }
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
        defines {
            "EMU_EXPERIMENTAL_BUILD" , "CONTROLLER_SUPPORT", "EMU_OVERLAY", "ImTextureID=ImU64", "UTF_CPP_CPLUSPLUS=201703L", "CURL_STATICLIB",
            "NDEBUG", "EMU_RELEASE_BUILD", "_CRT_SECURE_NO_WARNINGS", "EMU_BUILD_STRING=".._OPTIONS["emubuild"]
        }
    -- ExperimentalDebug LINUX
    filter { "ExperimentalDebug", "options:os=linux" }
        files {
            linux_files,
            "overlay_experimental/**"
        }

        removefiles {
            "libs/simpleini/**",
            "helpers/pe_**"
        }
        defines {
            "CONTROLLER_SUPPORT", "EMU_OVERLAY", "ImTextureID=ImU64", "UTF_CPP_CPLUSPLUS=201703L", "CURL_STATICLIB",
            "GNUC", "EMU_BUILD_STRING=".._OPTIONS["emubuild"]
        }
    -- ExperimentalRelease LINUX
    filter { "ExperimentalRelease", "options:os=linux" }
        files {
            linux_files,
            "overlay_experimental/**"
        }

        removefiles {
            "libs/simpleini/**",
            "helpers/pe_**"
        }
        defines {
            "CONTROLLER_SUPPORT", "EMU_OVERLAY", "ImTextureID=ImU64", "UTF_CPP_CPLUSPLUS=201703L", "CURL_STATICLIB",
            "NDEBUG", "EMU_RELEASE_BUILD", "GNUC", "EMU_BUILD_STRING=".._OPTIONS["emubuild"]
        }

-- Project SteamEmu END

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

    -- SET ARCH
    filter "platforms:x32"
        targetname "steamclient"
        architecture "x86" 

    filter "platforms:x64"
        targetname "steamclient64"
        architecture "x86_64"

    -- BASIC FOR WINDOWS 
    filter "options:os=windows"
        buildoptions  {
            "/permissive-", "/MP", "/DYNAMICBASE",
            "/utf-8", "/Zc:char8_t-", "/EHsc", "/GL-"
        }
        linkoptions  {
            "/NOLOGO", "/emittoolversioninfo:no"
        }

        filter { "options:os=windows", "configurations:Release" }
            defines {
                "NDEBUG", "EMU_RELEASE_BUILD", "UTF_CPP_CPLUSPLUS=201703L", "CURL_STATICLIB", "_CRT_SECURE_NO_WARNINGS", "EMU_BUILD_STRING=".._OPTIONS["emubuild"], "STEAMCLIENT_DLL", "EMU_EXPERIMENTAL_BUILD"
            }

        filter { "options:os=windows", "configurations:Debug" }
            defines {
                "DEBUG", "UTF_CPP_CPLUSPLUS=201703L", "CURL_STATICLIB", "_CRT_SECURE_NO_WARNINGS", "EMU_BUILD_STRING=".._OPTIONS["emubuild"], "STEAMCLIENT_DLL", "EMU_EXPERIMENTAL_BUILD"
            }

    -- Win32 stuff
    filter { "platforms:x32", "options:os=windows" }
        files {
            "steamclient/**",
            "resources/win/client/32/resources.rc"
        }

        filter { "Experimental**", "options:os=windows",  "platforms:x32" }
            files {
                windows_files,
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
                x32_libsdir_win
            }
            includedirs {
                default_include,
                "dll/proto_gen/win",
                x32_include_win
            }
            removefiles { "steamclient/**" }
            removefiles { "libs/detours/uimports.cc" }

    -- Win64 stuff
    filter { "platforms:x64", "options:os=windows" }
        files {
            "steamclient/**",
            "resources/win/client/64/resources.rc"
        }
    
        filter { "Experimental**", "options:os=windows",  "platforms:x64" }
            files {
                windows_files,
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
                x64_libsdir_win
            }
            includedirs {
                default_include,
                "dll/proto_gen/win",
                x64_include_win
            }
            removefiles { "steamclient/**" }
            removefiles { "libs/detours/uimports.cc" }

    -- ExperimentalDebug WINDOWS
    filter { "ExperimentalDebug", "options:os=windows" }
        defines {
            "DEBUG", "UTF_CPP_CPLUSPLUS=201703L", "CURL_STATICLIB", "_CRT_SECURE_NO_WARNINGS", "EMU_BUILD_STRING=".._OPTIONS["emubuild"], "STEAMCLIENT_DLL", "EMU_EXPERIMENTAL_BUILD", 
            "ImTextureID=ImU64", "EMU_OVERLAY", "CONTROLLER_SUPPORT"
        }

    -- ExperimentalRelease WINDOWS
    filter { "ExperimentalRelease", "options:os=windows" }
        defines {
            "NDEBUG", "EMU_RELEASE_BUILD","UTF_CPP_CPLUSPLUS=201703L", "CURL_STATICLIB", "_CRT_SECURE_NO_WARNINGS", "EMU_BUILD_STRING=".._OPTIONS["emubuild"], "STEAMCLIENT_DLL", "EMU_EXPERIMENTAL_BUILD", 
            "ImTextureID=ImU64", "EMU_OVERLAY", "CONTROLLER_SUPPORT"
        }

    -- linux soon
-- Project SteamClient END

-- Project steamnetworkingsockets START
project "steamnetworkingsockets"
    cppdialect("c++17")
    kind "SharedLib"
    language "C++"
    targetdir "bin/steamnetworkingsockets/%{cfg.buildcfg}_%{cfg.platform}"
    location "GBE_Build/steamnetworkingsockets"
    targetname "steamnetworkingsockets"
    optimize "On"
    symbols "Off"

    files {
        "networking_sockets_lib/**"
    }

    includedirs {
        "dll",
        "sdk"
    }

    filter "options:os=windows"
        buildoptions  {
            "/permissive-", "/MP", "/DYNAMICBASE",
            "/utf-8", "/Zc:char8_t-", "/EHsc", "/GL-"
        }
        linkoptions  {
            "/NOLOGO", "/emittoolversioninfo:no"
        }
        filter { "options:os=windows", "configurations:**Release" }
            defines {
                "NDEBUG", "EMU_RELEASE_BUILD", "UTF_CPP_CPLUSPLUS=201703L", "_CRT_SECURE_NO_WARNINGS", "EMU_BUILD_STRING=".._OPTIONS["emubuild"]
            }

        filter { "options:os=windows", "configurations:**Debug" }
            defines {
                "DEBUG", "UTF_CPP_CPLUSPLUS=201703L", "_CRT_SECURE_NO_WARNINGS", "EMU_BUILD_STRING=".._OPTIONS["emubuild"]
            }

        filter { "platforms:x64", "options:os=windows"}
            files {
                "networking_sockets_lib/**",
                "resources/win/api/64/resources.rc"
            }
        filter { "platforms:x32", "options:os=windows" }
            files {
                "networking_sockets_lib/**",
                "resources/win/api/32/resources.rc"
            }

    filter "options:os=linux"
        buildoptions  {
            "-fvisibility=hidden", "-fexceptions", "-fno-jump-tables", "-fno-char8_t" , "-Wundefined-internal", "-Wno-switch"
        }

        linkoptions {
            "-Wl,--exclude-libs,ALL"
        }
        filter { "options:os=linux", "configurations:**Release" }
            defines {
                "GNUC", "NDEBUG", "EMU_RELEASE_BUILD", "UTF_CPP_CPLUSPLUS=201703L", "_CRT_SECURE_NO_WARNINGS", "EMU_BUILD_STRING=".._OPTIONS["emubuild"]
            }

        filter { "options:os=linux", "configurations:**Debug" }
            defines {
                "GNUC", "DEBUG", "UTF_CPP_CPLUSPLUS=201703L", "_CRT_SECURE_NO_WARNINGS", "EMU_BUILD_STRING=".._OPTIONS["emubuild"]
            }

-- Project steamnetworkingsockets END

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
-- End GenerateInterfaces

-- TODO: SteamClientExtra should make steamclient_loader_BIT.exe and steamclient_extra.dll.

--    Normal (NON EXPERIMENTAL) should make the .exe
--    EXPERIMENTAL should make the _extra.dll
-- ONLY WINDOWS
if os.target() == "windows" then



-- Project GameOverlayRenderer
project "GameOverlayRenderer"
    cppdialect("c++17")
    kind "SharedLib"
    language "C++"
    targetdir "bin/GameOverlayRenderer/%{cfg.buildcfg}_%{cfg.platform}"
    location "GBE_Build/GameOverlayRenderer"
    targetname "GameOverlayRenderer"
    optimize "On"
    symbols "Off"

    files {
        "game_overlay_renderer_lib/**"
    }

    filter { "platforms:x64", "options:os=windows"}
        includedirs {
            "game_overlay_renderer_lib",
            default_include,
            "dll/proto_gen/win",
            x64_include_win
        }
        libdirs {
            x64_libsdir_win
        }
    filter { "platforms:x32", "options:os=windows" }
        includedirs {
            "game_overlay_renderer_lib",
            default_include,
            "dll/proto_gen/win",
            x32_include_win
        }
        libdirs {
            x32_libsdir_win
        }
-- End GameOverlayRenderer

end
-- ONLY WINDOWS END
-- Workspace END