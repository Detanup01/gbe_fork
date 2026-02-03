-- gbe_fork xmake build configuration
-- Migrated from premake5.lua
-- Targets MSVC 14.50 with C++26

set_project("gbe")
set_version("1.0.0")
set_xmakever("2.8.0")

--------------------------------------------------------------------------------
-- DEPENDENCIES
--------------------------------------------------------------------------------
add_requires("zlib", {system = false, configs = {shared = false}})
add_requires("mbedtls", {system = false, configs = {shared = false}})
add_requires("libcurl", {system = false, configs = {shared = false, tls = "mbedtls"}})
add_requires("protobuf-cpp", {system = false, configs = {shared = false}})
add_requires("libopus", {system = false, configs = {shared = false}})
add_requires("portaudio", {system = false, configs = {shared = false}})
add_requires("utfcpp v3.2.1", {system = false})

--------------------------------------------------------------------------------
-- OPTIONS
--------------------------------------------------------------------------------
option("genproto")
    set_default(false)
    set_showmenu(true)
    set_description("Generate .cc/.h files from .proto file")
option_end()

option("emubuild")
    set_default(os.date("%Y_%m_%d-%H_%M_%S"))
    set_showmenu(true)
    set_description("Set the EMU_BUILD_STRING")
option_end()

if is_plat("windows") then
    option("dosstub")
        set_default(false)
        set_showmenu(true)
        set_description("Change the DOS stub of the Windows builds")
    option_end()

    option("winsign")
        set_default(false)
        set_showmenu(true)
        set_description("Sign Windows builds with a fake certificate")
    option_end()

    option("winrsrc")
        set_default(false)
        set_showmenu(true)
        set_description("Add resources to Windows builds")
    option_end()
end

--------------------------------------------------------------------------------
-- TOOLCHAIN CONFIGURATION FUNCTIONS
--------------------------------------------------------------------------------
function configure_msvc_toolchain()
    set_languages("c++latest", "c17")  -- C++26 via c++latest
    set_runtimes("MT")  -- Static runtime /MT
    
    add_cxxflags(
        "/permissive-",
        "/DYNAMICBASE",
        "/bigobj",
        "/utf-8",
        "/Zc:char8_t-",
        "/EHsc",
        "/GL-",
        {force = true}
    )
    
    add_cflags(
        "/permissive-",
        "/DYNAMICBASE",
        "/bigobj",
        "/utf-8",
        {force = true}
    )
    
    add_ldflags(
        "/NOLOGO",
        "/emittoolversioninfo:no",
        {force = true}
    )
    
    add_defines("_CRT_SECURE_NO_WARNINGS")
end

function configure_gcc_toolchain()
    set_languages("c++2c", "gnu17")  -- C++26 via c++2c
    
    add_cxxflags(
        "-fno-jump-tables",
        "-Wno-switch",
        "-fno-char8_t",
        "-fvisibility=hidden",
        {force = true}
    )
    
    add_ldflags(
        "-Wl,--exclude-libs,ALL",
        {force = true}
    )
    
    add_defines("GNUC")
end

function configure_debug()
    set_symbols("debug")
    set_optimize("none")
    add_defines("DEBUG")
end

function configure_release()
    set_symbols("hidden")
    set_optimize("faster")
    add_defines("NDEBUG", "EMU_RELEASE_BUILD")
end

--------------------------------------------------------------------------------
-- PLATFORM DETECTION
--------------------------------------------------------------------------------
local os_iden = is_plat("windows") and "win" or "linux"

-- Common defines for all targets
local common_emu_defines = {
    "UTF_CPP_CPLUSPLUS=201703L",
    "CURL_STATICLIB",
    "CONTROLLER_SUPPORT",
    "EMU_BUILD_STRING=" .. (get_config("emubuild") or os.date("%Y_%m_%d-%H_%M_%S"))
}

-- Helper to get proto output dir
local function get_proto_dir()
    return path.join(os.projectdir(), "proto_gen", os_iden)
end

-- Note: Proto generation is handled by build.ps1 script
-- Run ".\build.ps1" and select option 2 to generate proto files

-- Common include directories (new structure)
local common_include = {
    ".",  -- Root directory for dll/ includes
    "include",
    "include/gbe",
    "include/gbe/common",
    "include/gbe/overlay",
    "include/sdk",
    "src/core",
    "src/common",
    "proto_gen/" .. os_iden,
    "src/libraries",
    "src/libraries/utfcpp",
    "overlay_experimental",  -- For overlay/steam_overlay.h
}
-- Note: utfcpp might be provided by package now, need to check include path

-- Windows system libraries
local windows_syslibs = {
    "Ws2_32", "Iphlpapi", "Wldap32", "Winmm", "Bcrypt", "Dbghelp",
    "Xinput", "Gdi32", "Dwmapi", "OpenGL32", "Shell32"
}

-- Common source files (new structure)
local common_files = {
    "src/core/**.cpp",
    "proto_gen/" .. os_iden .. "/**.cc",
    "src/libraries/**.cpp", "src/libraries/**.c",
    "src/crash_printer/" .. os_iden .. ".cpp",
    "src/common/common_helpers.cpp",
    "src/common/dbg_log.cpp",
}

local overlay_files = {
    "src/overlay/**.cpp",
}

local overlay_experimental_files = {
    "overlay_experimental/**.cpp",
}

local detours_files = {
    "src/libraries/detours/**.cpp"
}

--------------------------------------------------------------------------------
-- GLOBAL SETTINGS (applied to all targets)
--------------------------------------------------------------------------------

-- Add mode rules for debug/release
add_rules("mode.debug", "mode.release")

-- Language and runtime settings
if is_plat("windows") then
    set_languages("c++latest", "c17")
    set_runtimes("MT")
    
    add_cxxflags("/permissive-", "/DYNAMICBASE", "/bigobj", "/utf-8", "/EHsc", "/GL-", {force = true})
    add_cflags("/permissive-", "/DYNAMICBASE", "/bigobj", "/utf-8", {force = true})
    add_ldflags("/NOLOGO", "/emittoolversioninfo:no", {force = true})
    add_defines("_CRT_SECURE_NO_WARNINGS")
else
    set_languages("c++2c", "gnu17")
    add_cxxflags("-fno-jump-tables", "-Wno-switch", "-fno-char8_t", "-fvisibility=hidden", {force = true})
    add_ldflags("-Wl,--exclude-libs,ALL", {force = true})
    add_defines("GNUC")
end

-- Debug/Release mode settings
if is_mode("debug") then
    set_symbols("debug")
    set_optimize("none")
    add_defines("DEBUG")
else
    set_symbols("hidden")
    set_optimize("faster")
    add_defines("NDEBUG", "EMU_RELEASE_BUILD")
end

--------------------------------------------------------------------------------
-- TARGET: api_regular
--------------------------------------------------------------------------------
target("api_regular")
    set_kind("shared")
    
    -- Output name based on arch
    if is_plat("windows") then
        on_load(function (target)
            if target:is_arch("x86") then
                target:set("basename", "steam_api")
            else
                target:set("basename", "steam_api64")
            end
        end)
    else
        set_basename("libsteam_api")
    end
    
    -- Output directory
    set_targetdir("build/" .. os_iden .. "/$(mode)/regular/$(arch)")
    
    -- Common include directories and defines
    add_includedirs(common_include)
    
    add_defines(common_emu_defines)
    
    -- Source files
    add_files(common_files)
    remove_files(detours_files)
    remove_files("src/core/wrap.cpp")  -- Windows only
    
    -- Link libraries (libssq handled by external script)
    add_packages("zlib", "libcurl", "protobuf-cpp", "mbedtls", "libopus", "portaudio", "utfcpp")
    add_includedirs("third_party/libssq/include")
    add_linkdirs("third_party/libssq/lib/$(mode)")
    add_links("ssq")
    
    if is_plat("windows") then
        add_syslinks(windows_syslibs)
        
        -- Windows resources
        if get_config("winrsrc") then
            on_load(function (target)
                if target:is_arch("x86") then
                    target:add("files", "resources/win/api/32/resources.rc")
                else
                    target:add("files", "resources/win/api/64/resources.rc")
                end
            end)
        end
    else
        add_syslinks("pthread", "dl")
    end
    
    -- Modes
    add_rules("mode.debug", "mode.release")
    if is_mode("debug") then
        configure_debug()
    else
        configure_release()
    end
target_end()

--------------------------------------------------------------------------------
-- TARGET: api_experimental
--------------------------------------------------------------------------------
target("api_experimental")
    set_kind("shared")
    
    -- Output name based on arch
    if is_plat("windows") then
        on_load(function (target)
            if target:is_arch("x86") then
                target:set("basename", "steam_api")
            else
                target:set("basename", "steam_api64")
            end
        end)
    else
        set_basename("libsteam_api")
    end
    
    -- Output directory
    set_targetdir("build/" .. os_iden .. "/$(mode)/experimental/$(arch)")
    
    -- Common include directories and defines
    add_includedirs(common_include)
    
    add_defines(common_emu_defines)
    
    -- Extra defines
    add_defines("EMU_OVERLAY", "EMU_EXPERIMENTAL_BUILD")
    add_cxxflags([[/DIMGUI_USER_CONFIG="InGameOverlay/ImGui/imconfig.h"]])  -- Use overlay's ImGui config
    
    -- Source files
    add_files(common_files)
    add_files(overlay_experimental_files)
    remove_files("third_party/detours/uimports.cc")
    remove_files("src/core/wrap.cpp")  -- Windows only
    
    -- Link libraries (libssq and ingame_overlay handled by external script)
    add_packages("zlib", "libcurl", "protobuf-cpp", "mbedtls", "libopus", "portaudio", "utfcpp")
    add_includedirs("third_party/libssq/include", "third_party/ingame_overlay/include")
    add_linkdirs("third_party/libssq/lib/$(mode)", "third_party/ingame_overlay/lib/$(mode)")
    add_links("ssq", "ingame_overlay", "minhook.x64", "system")

    if is_plat("windows") then
        add_syslinks(windows_syslibs)
        
        -- Windows resources
        if get_config("winrsrc") then
            on_load(function (target)
                if target:is_arch("x86") then
                    target:add("files", "resources/win/api/32/resources.rc")
                else
                    target:add("files", "resources/win/api/64/resources.rc")
                end
            end)
        end
    else
        add_syslinks("pthread", "dl", "X11")
        remove_files(detours_files)
    end
    
    -- Modes
    add_rules("mode.debug", "mode.release")
target_end()

--------------------------------------------------------------------------------
-- TARGET: steamclient_experimental
--------------------------------------------------------------------------------
target("steamclient_experimental")
    set_kind("shared")
    
    -- Output name based on arch
    if is_plat("windows") then
        on_load(function (target)
            if target:is_arch("x86") then
                target:set("basename", "steamclient")
            else
                target:set("basename", "steamclient64")
            end
        end)
        -- Common includes
    add_includedirs(common_include)
    
    add_defines(common_emu_defines)
    
    set_targetdir("build/" .. os_iden .. "/$(mode)/steamclient_experimental")
    else
        set_basename("steamclient")
        set_targetdir("build/" .. os_iden .. "/$(mode)/experimental/$(arch)")
    end
    
    -- Extra defines
    add_defines("STEAMCLIENT_DLL", "EMU_OVERLAY", "EMU_EXPERIMENTAL_BUILD")
    add_cxxflags([[/DIMGUI_USER_CONFIG="InGameOverlay/ImGui/imconfig.h"]])  -- Use overlay's ImGui config
    
    -- Source files
    add_files(common_files)
    add_files(overlay_experimental_files)
    remove_files("third_party/detours/uimports.cc")
    remove_files("src/core/flat.cpp")
    remove_files("src/core/wrap.cpp")  -- Windows only
    
    -- Link libraries (libssq and ingame_overlay handled by external script)
    add_packages("zlib", "libcurl", "protobuf-cpp", "mbedtls", "libopus", "portaudio", "utfcpp")
    add_includedirs("third_party/libssq/include", "third_party/ingame_overlay/include")
    add_linkdirs("third_party/libssq/lib/$(mode)", "third_party/ingame_overlay/lib/$(mode)")
    add_links("ssq", "ingame_overlay", "minhook.x64", "system")
    
    if is_plat("windows") then
        add_syslinks(windows_syslibs)
        
        -- Windows resources
        if get_config("winrsrc") then
            on_load(function (target)
                if target:is_arch("x86") then
                    target:add("files", "resources/win/client/32/resources.rc")
                else
                    target:add("files", "resources/win/client/64/resources.rc")
                end
            end)
        end
    else
        add_syslinks("pthread", "dl")
        remove_files(detours_files)
    end
    
    -- Modes
    add_rules("mode.debug", "mode.release")
target_end()

--------------------------------------------------------------------------------
-- TARGET: tool_lobby_connect
--------------------------------------------------------------------------------
target("tool_lobby_connect")
    set_kind("binary")
    
    set_basename("lobby_connect_$(arch)")
    set_targetdir("build/" .. os_iden .. "/$(mode)/tools/lobby_connect")
    
    -- Include directories
    add_includedirs(common_include)
    
    -- Defines
    add_defines("NO_DISK_WRITES", "LOBBY_CONNECT")
    add_defines("UTF_CPP_CPLUSPLUS=201703L", "CURL_STATICLIB")
    add_defines("EMU_BUILD_STRING=" .. (get_config("emubuild") or os.date("%Y_%m_%d-%H_%M_%S")))
    
    -- Source files
    add_files(common_files)
    add_files("src/tools/lobby_connect/lobby_connect.cpp")
    remove_files("third_party/gamepad/**")
    remove_files("src/libraries/gamepad/**")
    remove_files(detours_files)
    remove_files("src/core/flat.cpp")
    
    -- Link libraries (libssq handled by external script)
    add_packages("zlib", "libcurl", "protobuf-cpp", "mbedtls", "libopus", "portaudio", "utfcpp")
    add_includedirs("third_party/libssq/include")
    add_linkdirs("third_party/libssq/lib/$(mode)")
    add_links("ssq")

    if is_plat("windows") then
        add_syslinks(windows_syslibs)
        add_syslinks("Comdlg32")
        
        -- Windows resources
        if get_config("winrsrc") then
            on_load(function (target)
                if target:is_arch("x86") then
                    target:add("files", "resources/win/launcher/32/resources.rc")
                else
                    target:add("files", "resources/win/launcher/64/resources.rc")
                end
            end)
        end
    else
        add_syslinks("pthread", "dl")
    end
    
    -- Modes
    add_rules("mode.debug", "mode.release")
target_end()

--------------------------------------------------------------------------------
-- TARGET: tool_generate_interfaces
--------------------------------------------------------------------------------
target("tool_generate_interfaces")
    set_kind("binary")
    
    set_basename("generate_interfaces_$(arch)")
    set_targetdir("build/" .. os_iden .. "/$(mode)/tools/generate_interfaces")
    
    -- Source files
    add_files("src/tools/generate_interfaces/generate_interfaces.cpp")
    add_files("src/common/common_helpers.cpp")
    
    add_includedirs("include/gbe/common", "src/common", "third_party", "third_party/utfcpp")
    add_packages("utfcpp")
    
    -- Modes
    add_rules("mode.debug", "mode.release")
target_end()

--------------------------------------------------------------------------------
-- TARGET: lib_steamnetworkingsockets
--------------------------------------------------------------------------------
target("lib_steamnetworkingsockets")
    set_kind("shared")
    
    set_basename("libsteamnetworkingsockets")
    set_targetdir("build/" .. os_iden .. "/$(mode)/steamnetworkingsockets/$(arch)")
    
    -- Source files
    add_files("src/networking/**.cpp")
    add_files("src/common/dbg_log.cpp")
    add_files("src/common/common_helpers.cpp")
    
    add_includedirs("include/sdk", "include/gbe/common", "src/common", "third_party", "third_party/utfcpp")
    add_packages("utfcpp")

    -- Modes
    add_rules("mode.debug", "mode.release")
target_end()

--------------------------------------------------------------------------------
-- TARGET: lib_game_overlay_renderer
--------------------------------------------------------------------------------
target("lib_game_overlay_renderer")
    set_kind("shared")
    
    -- Output name based on arch
    if is_plat("windows") then
        on_load(function (target)
            if target:is_arch("x86") then
                target:set("basename", "GameOverlayRenderer")
            else
                target:set("basename", "GameOverlayRenderer64")
            end
        end)
        -- Common includes
    add_includedirs(common_include)
    
    add_defines(common_emu_defines)
    
    set_targetdir("build/" .. os_iden .. "/$(mode)/steamclient_experimental")
    else
        set_basename("gameoverlayrenderer")
        set_targetdir("build/" .. os_iden .. "/$(mode)/gameoverlayrenderer/$(arch)")
    end
    
    -- Source files
    add_files("src/game_overlay_renderer/**.cpp")
    
    add_includedirs("dll", "include/gbe/common", "src/common", "libs")
    add_packages("zlib", "libcurl", "protobuf-cpp", "mbedtls", "libopus", "portaudio", "utfcpp")
    add_includedirs("third_party/libssq/include")
    add_linkdirs("third_party/libssq/lib/$(mode)")
    add_links("ssq")
    
    -- Windows resources
    if is_plat("windows") and get_config("winrsrc") then
        on_load(function (target)
            if target:is_arch("x86") then
                target:add("files", "resources/win/game_overlay_renderer/32/resources.rc")
            else
                target:add("files", "resources/win/game_overlay_renderer/64/resources.rc")
            end
        end)
    end
    
    -- Modes
    add_rules("mode.debug", "mode.release")
target_end()

--------------------------------------------------------------------------------
-- WINDOWS-ONLY TARGETS
--------------------------------------------------------------------------------
if is_plat("windows") then

--------------------------------------------------------------------------------
-- TARGET: steamclient_experimental_stub
--------------------------------------------------------------------------------
target("steamclient_experimental_stub")
set_kind("shared")
    
on_load(function (target)
    if target:is_arch("x86") then
        target:set("basename", "steamclient")
    else
        target:set("basename", "steamclient64")
    end
end)
    
set_targetdir("build/" .. os_iden .. "/$(mode)/experimental/$(arch)")
    
-- Source files
-- Note: steamclient.cpp doesn't exist in new structure, this target may need updating
-- add_files("steamclient/steamclient.cpp")
    
-- Windows resources
    if get_config("winrsrc") then
        on_load(function (target)
            if target:is_arch("x86") then
                target:add("files", "resources/win/client/32/resources.rc")
            else
                target:add("files", "resources/win/client/64/resources.rc")
            end
        end)
    end
    
    -- Modes
    add_rules("mode.debug", "mode.release")
target_end()

--------------------------------------------------------------------------------
-- TARGET: steamclient_experimental_extra
--------------------------------------------------------------------------------
target("steamclient_experimental_extra")
    set_kind("shared")
    
    set_basename("steamclient_extra_$(arch)")
    -- Common includes
    add_includedirs(common_include)
    
    add_defines(common_emu_defines)
    
    set_targetdir("build/" .. os_iden .. "/$(mode)/steamclient_experimental/extra_dlls")
    
    -- Source files
    add_files("src/tools/steamclient_loader/win/extra_protection/**.cpp")
    add_files("src/common/pe_helpers.cpp")
    add_files("src/common/common_helpers.cpp")
    add_files(detours_files)
    remove_files("third_party/detours/uimports.cc")
    
    add_includedirs("include/gbe/common", "src/common", "third_party", "third_party/utfcpp")
    add_packages("utfcpp")
    
    -- Windows resources
    if get_config("winrsrc") then
        on_load(function (target)
            if target:is_arch("x86") then
                target:add("files", "resources/win/client/32/resources.rc")
            else
                target:add("files", "resources/win/client/64/resources.rc")
            end
        end)
    end
    
    -- Modes
    add_rules("mode.debug", "mode.release")
target_end()

end -- is_plat("windows")
