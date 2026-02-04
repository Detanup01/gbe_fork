-- gbe_fork xmake build configuration
-- C++26 with MSVC or GCC/Clang

set_project("gbe")
set_version("1.0.0")
set_xmakever("2.8.0")

-- Centralize all build artifacts to .build/ directory
set_targetdir(".build/$(mode)/$(plat)/$(arch)")
set_objectdir(".build/.objs")

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

-- Git-based dependencies with build scripts
add_requires("libssq latest", {system = false, configs = {cmake = true, shared = false}})
add_requires("ingame_overlay latest", {system = false, configs = {cmake = true, shared = false}})

--------------------------------------------------------------------------------
-- CUSTOM PACKAGE DEFINITIONS
--------------------------------------------------------------------------------

package("libssq")
    set_homepage("https://github.com/BinaryAlien/libssq")
    set_description("Source Query library")
    set_license("MIT")
    
    add_urls("https://github.com/BinaryAlien/libssq.git")
    add_versions("latest", "main")
    
    add_deps("cmake")
    
    on_install(function (package)
        -- Apply MSVC compatibility patch
        io.gsub("src/error.c", "NULL,%s+%);", "NULL);")
        
        local configs = {"-DBUILD_SHARED_LIBS=OFF"}
        import("package.tools.cmake").install(package, configs)
    end)
package_end()

package("ingame_overlay")
    set_homepage("https://github.com/Rustbeard86/ingame_overlay")
    set_description("In-game overlay with ImGui")
    set_license("MIT")
    
    add_urls("https://github.com/Rustbeard86/ingame_overlay.git", {submodules = true})
    add_versions("latest", "master")
    
    add_deps("cmake")

    on_install("windows", function (package)
        local configs = {
            "-DBUILD_SHARED_LIBS=OFF",
            "-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded"
        }
        
        import("package.tools.cmake").install(package, configs)
        
        -- 1. Correct Header Copy
        -- Your tree shows headers are already in 'include/InGameOverlay' in source
        os.cp("include/InGameOverlay", package:installdir("include"))
        
        -- 2. Robust Library Copy
        -- We found: ingame_overlay.lib, minhook.x64.lib, and system.lib
        for _, filepath in ipairs(os.files("**.lib")) do
            local filename = path.filename(filepath):lower()
            if filename:find("ingame_overlay") or 
               filename:find("minhook") or 
               filename:find("system") then
                os.trycp(filepath, package:installdir("lib"))
            end
        end
    end)

    on_test(function (package)
        assert(package:has_cxxincludes("InGameOverlay/RendererHook.h"))
    end)
package_end()

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

-- Common include directories
local common_include = {
    ".",  -- Root directory for src/core/ includes
    "include",
    "include/gbe",
    "include/gbe/common",
    "include/gbe/overlay",
    "include/sdk",
    "src/core",
    "src/common",
    "src/libraries",
    "src/libraries/utfcpp",
    "src/overlay/experimental",
    "$(builddir)",  -- For generated protobuf files
}

-- Windows system libraries
local windows_syslibs = {
    "Ws2_32", "Iphlpapi", "Wldap32", "Winmm", "Bcrypt", "Dbghelp",
    "Xinput", "Gdi32", "Dwmapi", "OpenGL32", "Shell32", "User32", "Advapi32"
}

-- Common source files
local common_files = {
    "src/core/**.cpp",
    "src/proto/*.proto",  -- Protobuf files (will be auto-generated)
    "src/libraries/**.cpp", "src/libraries/**.c",
    "src/crash_printer/" .. os_iden .. ".cpp",
    "src/common/common_helpers.cpp",
    "src/common/dbg_log.cpp",
}

local overlay_files = {
    "src/overlay/**.cpp",
}

local overlay_experimental_files = {
    "src/overlay/experimental/**.cpp",
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
    
    -- Add protobuf rule for automatic .proto compilation
    add_rules("protobuf.cpp")
    
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
    set_targetdir(".build/$(mode)/regular/$(arch)")
    
    -- Common include directories and defines
    add_includedirs(common_include)
    
    add_defines(common_emu_defines)
    
    -- Source files
    add_files(common_files)
    remove_files(detours_files)
    remove_files("src/core/wrap.cpp")  -- Windows only
    
    -- Link libraries (using xmake packages)
    add_packages("zlib", "libcurl", "protobuf-cpp", "mbedtls", "libopus", "portaudio", "utfcpp", "libssq")
    
    if is_plat("windows") then
        add_syslinks(windows_syslibs)
        
        -- Windows resources
        if get_config("winrsrc") then
            on_load(function (target)
                if target:is_arch("x86") then
                    target:add("files", "src/resources/win/api/32/resources.rc")
                else
                    target:add("files", "src/resources/win/api/64/resources.rc")
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
    
    -- Add protobuf rule for automatic .proto compilation
    add_rules("protobuf.cpp")
    
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
    set_targetdir(".build/$(mode)/experimental/$(arch)")
    
    -- Common include directories and defines
    add_includedirs(common_include)
    
    add_defines(common_emu_defines)
    
    -- Extra defines
    add_defines("EMU_OVERLAY", "EMU_EXPERIMENTAL_BUILD")
    add_cxxflags([[/DIMGUI_USER_CONFIG="InGameOverlay/ImGuiConfig.h"]])  -- Use overlay's ImGui config
    
    -- Source files
    add_files(common_files)
    add_files(overlay_experimental_files)
    remove_files("src/third_party/detours/uimports.cc")
    remove_files("src/core/wrap.cpp")  -- Windows only
    
    -- Link libraries (using xmake packages)
    add_packages("zlib", "libcurl", "protobuf-cpp", "mbedtls", "libopus", "portaudio", "utfcpp", "libssq", "ingame_overlay")

    if is_plat("windows") then
        add_syslinks(windows_syslibs)
        add_syslinks("Ntdll")
        
        -- Windows resources
        if get_config("winrsrc") then
            on_load(function (target)
                if target:is_arch("x86") then
                    target:add("files", "src/resources/win/api/32/resources.rc")
                else
                    target:add("files", "src/resources/win/api/64/resources.rc")
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
    
    -- Add protobuf rule for automatic .proto compilation
    add_rules("protobuf.cpp")
    
    
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
    
    set_targetdir(".build/$(mode)/steamclient_experimental")
    else
        set_basename("steamclient")
        set_targetdir(".build/$(mode)/experimental/$(arch)")
    end
    
    -- Extra defines
    add_defines("STEAMCLIENT_DLL", "EMU_OVERLAY", "EMU_EXPERIMENTAL_BUILD")
    add_cxxflags([[/DIMGUI_USER_CONFIG="InGameOverlay/ImGuiConfig.h"]])  -- Use overlay's ImGui config
    
    -- Source files
    add_files(common_files)
    add_files(overlay_experimental_files)
    remove_files("src/third_party/detours/uimports.cc")
    remove_files("src/core/flat.cpp")
    remove_files("src/core/wrap.cpp")  -- Windows only
    
    -- Link libraries (using xmake packages)
    add_packages("zlib", "libcurl", "protobuf-cpp", "mbedtls", "libopus", "portaudio", "utfcpp", "libssq", "ingame_overlay")
    
    if is_plat("windows") then
        add_syslinks(windows_syslibs)
        add_syslinks("Ntdll")
        
        -- Windows resources
        if get_config("winrsrc") then
            on_load(function (target)
                if target:is_arch("x86") then
                    target:add("files", "src/resources/win/client/32/resources.rc")
                else
                    target:add("files", "src/resources/win/client/64/resources.rc")
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
    
    -- Add protobuf rule for automatic .proto compilation
    add_rules("protobuf.cpp")
    
    set_basename("lobby_connect_$(arch)")
    set_targetdir(".build/$(mode)/tools/lobby_connect")
    
    -- Include directories
    add_includedirs(common_include)
    
    -- Defines
    add_defines("NO_DISK_WRITES", "LOBBY_CONNECT")
    add_defines("UTF_CPP_CPLUSPLUS=201703L", "CURL_STATICLIB")
    add_defines("EMU_BUILD_STRING=" .. (get_config("emubuild") or os.date("%Y_%m_%d-%H_%M_%S")))
    
    -- Source files
    add_files(common_files)
    add_files("src/tools/lobby_connect/lobby_connect.cpp")
    remove_files("src/third_party/gamepad/**")
    remove_files("src/libraries/gamepad/**")
    remove_files(detours_files)
    remove_files("src/core/flat.cpp")
    
    -- Link libraries (using xmake packages)
    add_packages("zlib", "libcurl", "protobuf-cpp", "mbedtls", "libopus", "portaudio", "utfcpp", "libssq")

    if is_plat("windows") then
        add_syslinks(windows_syslibs)
        add_syslinks("Comdlg32")
        
        -- Windows resources
        if get_config("winrsrc") then
            on_load(function (target)
                if target:is_arch("x86") then
                    target:add("files", "src/src/resources/win/launcher/32/resources.rc")
                else
                    target:add("files", "src/src/resources/win/launcher/64/resources.rc")
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
    set_targetdir(".build/$(mode)/tools/generate_interfaces")
    
    -- Source files
    add_files("src/tools/generate_interfaces/generate_interfaces.cpp")
    add_files("src/common/common_helpers.cpp")
    
    add_includedirs("include/gbe/common", "src/common")
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
    set_targetdir(".build/$(mode)/steamnetworkingsockets/$(arch)")
    
    -- Source files
    add_files("src/networking/**.cpp")
    add_files("src/common/dbg_log.cpp")
    add_files("src/common/common_helpers.cpp")
    
    add_includedirs("include/sdk", "include/gbe/common", "src/common")
    add_packages("utfcpp")

    -- Modes
    add_rules("mode.debug", "mode.release")
target_end()

--------------------------------------------------------------------------------
-- TARGET: lib_game_overlay_renderer
--------------------------------------------------------------------------------
target("lib_game_overlay_renderer")
    set_kind("shared")
    
    -- Add protobuf rule for automatic .proto compilation
    add_rules("protobuf.cpp")
    
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
    
    set_targetdir(".build/$(mode)/steamclient_experimental")
    else
        set_basename("gameoverlayrenderer")
        set_targetdir(".build/$(mode)/gameoverlayrenderer/$(arch)")
    end
    
    -- Source files
    add_files("src/game_overlay_renderer/**.cpp")
    add_files("src/proto/*.proto")  -- Need protobuf generation
    
    add_includedirs("src/core", "include/gbe/common", "src/common")
    add_packages("zlib", "libcurl", "protobuf-cpp", "mbedtls", "libopus", "portaudio", "utfcpp", "libssq")
    
    -- Windows resources
    if is_plat("windows") and get_config("winrsrc") then
        on_load(function (target)
            if target:is_arch("x86") then
                target:add("files", "src/src/resources/win/game_overlay_renderer/32/resources.rc")
            else
                target:add("files", "src/src/resources/win/game_overlay_renderer/64/resources.rc")
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



--[[
--------------------------------------------------------------------------------
-- TARGET: steamclient_experimental_stub (DISABLED)
-- This target is currently disabled as it's not needed in the current build
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
    
set_targetdir(".build/$(mode)/experimental/$(arch)")
    
-- This target builds an empty stub DLL
-- The actual steamclient implementation is in steamclient_experimental target
    
-- Windows resources
    if get_config("winrsrc") then
        on_load(function (target)
            if target:is_arch("x86") then
                target:add("files", "src/resources/win/client/32/resources.rc")
            else
                target:add("files", "src/resources/win/client/64/resources.rc")
            end
        end)
    end
    
    -- Modes
    add_rules("mode.debug", "mode.release")
target_end()
--]]


--------------------------------------------------------------------------------
-- TARGET: steamclient_experimental_extra
--------------------------------------------------------------------------------
target("steamclient_experimental_extra")
    set_kind("shared")
    
    set_basename("steamclient_extra_$(arch)")
    -- Common includes
    add_includedirs(common_include)
    
    add_defines(common_emu_defines)
    
    set_targetdir(".build/$(mode)/steamclient_experimental/extra_dlls")
    
    -- Source files
    add_files("src/tools/steamclient_loader/win/extra_protection/**.cpp")
    add_files("src/common/pe_helpers.cpp")
    add_files("src/common/common_helpers.cpp")
    add_files(detours_files)
    remove_files("src/third_party/detours/uimports.cc")
    
    add_includedirs("include/gbe/common", "src/common")
    add_packages("utfcpp")
    
    if is_plat("windows") then
        add_syslinks(windows_syslibs)
        
        -- Windows resources
        if get_config("winrsrc") then
            on_load(function (target)
                if target:is_arch("x86") then
                    target:add("files", "src/resources/win/client/32/resources.rc")
                else
                    target:add("files", "src/resources/win/client/64/resources.rc")
                end
            end)
        end
    end
    
    -- Modes
    add_rules("mode.debug", "mode.release")
target_end()

end -- is_plat("windows")
