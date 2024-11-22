require("premake", ">=5.0.0-beta2")


-- add "-Wl,--whole-archive -Wl,-Bstatic -lmylib -Wl,-Bdynamic -Wl,--no-whole-archive"
-- via: links { 'mylib:static_whole' }
-- https://premake.github.io/docs/Overrides-and-Call-Arrays/#introducing-overrides
premake.override(premake.tools.gcc, "getlinks", function(originalFn, cfg, systemonly, nogroups)
    -- source:
    -- premake.tools.gcc.getlinks(cfg, systemonly, nogroups)
    -- https://github.com/premake/premake-core/blob/d842e671c7bc7e09f2eeaafd199fd01e48b87ee7/src/tools/gcc.lua#L568C15-L568C22

    local result = originalFn(cfg, systemonly, nogroups)
    local whole_syslibs = {"-Wl,--whole-archive"}
    local static_whole_syslibs = {"-Wl,--whole-archive -Wl,-Bstatic"}

    local endswith = function(s, ptrn)
        return ptrn == string.sub(s, -string.len(ptrn))
    end

    local idx_to_remove = {}
    for idx, name in ipairs(result) do
        if endswith(name, ":static_whole") then
            name = string.sub(name, 0, -14)
            table.insert(static_whole_syslibs, name) -- it already includes '-l'
            table.insert(idx_to_remove, idx)
        elseif endswith(name, ":whole_archive") then
            name = string.sub(name, 0, -15)
            table.insert(whole_syslibs, name) -- it already includes '-l'
            table.insert(idx_to_remove, idx)
        end
    end

    -- remove from the end to avoid trouble with table indexes shifting
    for iii = #idx_to_remove, 1, -1 do
        table.remove(result, idx_to_remove[iii])
    end

    local move = function(a1, a2)
        local t = #a2
        for i = 1, #a1 do a2[t + i] = a1[i] end
    end

    local new_result = {}
    if #whole_syslibs > 1 then
        table.insert(whole_syslibs, "-Wl,--no-whole-archive")
        move(whole_syslibs, new_result)
    end
    if #static_whole_syslibs > 1 then
        table.insert(static_whole_syslibs, "-Wl,-Bdynamic -Wl,--no-whole-archive")
        move(static_whole_syslibs, new_result)
    end

    -- https://stackoverflow.com/a/71719579
    -- because of the dumb way linux handles linking, the order becomes important
    -- I've encountered a problem with linking and it was failing with error "undefined reference to `__imp_WSACloseEvent'"
    -- despite 'Ws2_32' being added to the list of libraries, turns out some symbols from 'Ws2_32' were being stripped,
    -- because no library before it (on the command line) mentioned any of its symbols, the static libs were being appended afterwards on the command line,
    -- and they were mentioning some of the now-stripped symbols
    move(result, new_result)
    return new_result
end)

local function table_append(table_dest, table_src)
    local dest_start = #table_dest
    for idx = 1, #table_src do
        table_dest[dest_start + idx] = table_src[idx]
    end
end

local function table_postfix_items(table, postfix)
    local ret = {}
    for idx = 1, #table do
        ret[idx] = table[idx] .. postfix
    end
    return ret
end

-- pre-define stuff

local os_iden = '' -- identifier
if os.target() == "windows" then
    os_iden = 'win'
elseif os.target() == "linux" then
    os_iden = 'linux'
else
    error('Unsupported os target: "' .. os.target() .. '"')
end

local deps_dir = path.getabsolute(path.join('build', 'deps', os_iden, _ACTION), _MAIN_SCRIPT_DIR)

local function genproto()
    local deps_install_prefix = ''
    if os.is64bit() then
        deps_install_prefix = 'install64'
    else
        deps_install_prefix = 'install32'
    end
    local protoc_exe = path.join(deps_dir, 'protobuf', deps_install_prefix, 'bin', 'protoc')

    local out_dir = 'proto_gen/' .. os_iden

    if os.target() == "windows" then
        protoc_exe = protoc_exe .. '.exe'
    end

    if not os.isfile(protoc_exe) then
        error("protoc not found! " .. protoc_exe)
        return
    end

    print("Generating from .proto file!")
    local ok_mk, err_mk = os.mkdir(out_dir)
    if not ok_mk then
        error("Error: " .. err_mk)
        return
    end

    if os.target() == "linux" then
        local ok_chmod, err_chmod = os.chmod(protoc_exe, "777")
        if not ok_chmod then
            error("Error: " .. err_chmod)
            return
        end
    end

    if (os.host() == "linux" and os.target() == "windows") then
        protoc_exe = 'wine ' .. protoc_exe
    end

    return os.execute(protoc_exe .. ' dll/net.proto -I./dll/ --cpp_out=' .. out_dir)
end

newoption {
    category = 'protobuf files',
    trigger = "genproto",
    description = "Generate .cc/.h files from .proto file",
}

newoption {
    category = 'build',
    trigger = "emubuild",
    description = "Set the EMU_BUILD_STRING",
    value = "your_string",
    default = os.date("%Y_%m_%d-%H_%M_%S"),
}

newoption {
    category = 'visual-includes',
    trigger = "incexamples",
    description = "Add all example files in the projects (no impact on build)",
}
newoption {
    category = 'visual-includes',
    trigger = "incdeps",
    description = "Add all header files from the third-party dependencies in the projects (no impact on build)",
}

-- windows options
if os.target() == 'windows' then

newoption {
    category = "build",
    trigger = "dosstub",
    description = "Change the DOS stub of the Windows builds",
}

newoption {
    category = "build",
    trigger = "winsign",
    description = "Sign Windows builds with a fake certificate",
}

newoption {
    category = "build",
    trigger = "winrsrc",
    description = "Add resources to Windows builds",
}

end
-- End windows options


-- common defines
---------
local common_emu_defines = { -- added to all filters, later defines will be appended
    "UTF_CPP_CPLUSPLUS=201703L", "CURL_STATICLIB", "CONTROLLER_SUPPORT", "EMU_BUILD_STRING=" .. _OPTIONS["emubuild"],
}

-- include dirs
---------
local common_include = {
    'dll',
    'proto_gen/' .. os_iden,
    'libs',
    'libs/utfcpp',
    'helpers',
    'crash_printer',
    'sdk',
    "overlay_experimental",
}

local x32_deps_include = {
    path.join(deps_dir, "libssq/include"),
    path.join(deps_dir, "curl/install32/include"),
    path.join(deps_dir, "protobuf/install32/include"),
    path.join(deps_dir, "zlib/install32/include"),
    path.join(deps_dir, "mbedtls/install32/include"),
}

local x32_deps_overlay_include = {
    path.join(deps_dir, "ingame_overlay/install32/include"),
    path.join(deps_dir, "ingame_overlay/deps/System/install32/include"),
    path.join(deps_dir, "ingame_overlay/deps/mini_detour/install32/include"),
}

local x64_deps_include = {
    path.join(deps_dir, "libssq/include"),
    path.join(deps_dir, "curl/install64/include"),
    path.join(deps_dir, "protobuf/install64/include"),
    path.join(deps_dir, "zlib/install64/include"),
    path.join(deps_dir, "mbedtls/install64/include"),
}

local x64_deps_overlay_include = {
    path.join(deps_dir, "ingame_overlay/install64/include"),
    path.join(deps_dir, "ingame_overlay/deps/System/install64/include"),
    path.join(deps_dir, "ingame_overlay/deps/mini_detour/install64/include"),
}


-- source & header files
---------
local common_files = {
    -- dll/
    "dll/**",
    -- proto_gen/
    'proto_gen/' .. os_iden .. '/**',
    -- libs
    "libs/**",
    -- crash_printer/
    'crash_printer/' .. os_iden .. '.cpp', 'crash_printer/crash_printer/' .. os_iden .. '.hpp',
    -- helpers/common_helpers
    "helpers/common_helpers.cpp", "helpers/common_helpers/**",
    -- helpers/dbg_log
    "helpers/dbg_log.cpp", "helpers/dbg_log/**",
}

local overlay_files = {
    "overlay_experimental/**",
}

local detours_files = {
    "libs/detours/**",
}


-- libs to link
---------
local lib_prefix = 'lib'
local static_postfix = ''
-- GCC/Clang add this prefix by default and linking ex: '-lssq' will look for 'libssq'
-- so we have to ommit this prefix since it's automatically added
if _ACTION and string.match(_ACTION, 'gmake.*') then
    lib_prefix = ''
    if os.target() ~= 'windows' then -- for MinGw we compile everything with -static, and this conflicts with it
        static_postfix = ':static'
    end
end

local zlib_archive_name = 'z'
if os.target() == 'windows' then
    zlib_archive_name = 'zlibstatic' -- even on MinGw we need this name
end

local deps_link = {
    "ssq"                .. static_postfix,
    zlib_archive_name    .. static_postfix,
    lib_prefix .. "curl" .. static_postfix,
    "mbedcrypto"         .. static_postfix,
    "mbedtls"            .. static_postfix,
    "mbedx509"           .. static_postfix,
}
-- add protobuf libs
table_append(deps_link, {
    lib_prefix .. "protobuf-lite"                 .. static_postfix,
    "absl_bad_any_cast_impl"                      .. static_postfix,
    "absl_bad_optional_access"                    .. static_postfix,
    "absl_bad_variant_access"                     .. static_postfix,
    "absl_base"                                   .. static_postfix,
    "absl_city"                                   .. static_postfix,
    "absl_civil_time"                             .. static_postfix,
    "absl_cord"                                   .. static_postfix,
    "absl_cordz_functions"                        .. static_postfix,
    "absl_cordz_handle"                           .. static_postfix,
    "absl_cordz_info"                             .. static_postfix,
    "absl_cordz_sample_token"                     .. static_postfix,
    "absl_cord_internal"                          .. static_postfix,
    "absl_crc32c"                                 .. static_postfix,
    "absl_crc_cord_state"                         .. static_postfix,
    "absl_crc_cpu_detect"                         .. static_postfix,
    "absl_crc_internal"                           .. static_postfix,
    "absl_debugging_internal"                     .. static_postfix,
    "absl_demangle_internal"                      .. static_postfix,
    "absl_die_if_null"                            .. static_postfix,
    "absl_examine_stack"                          .. static_postfix,
    "absl_exponential_biased"                     .. static_postfix,
    "absl_failure_signal_handler"                 .. static_postfix,
    "absl_flags_commandlineflag"                  .. static_postfix,
    "absl_flags_commandlineflag_internal"         .. static_postfix,
    "absl_flags_config"                           .. static_postfix,
    "absl_flags_internal"                         .. static_postfix,
    "absl_flags_marshalling"                      .. static_postfix,
    "absl_flags_parse"                            .. static_postfix,
    "absl_flags_private_handle_accessor"          .. static_postfix,
    "absl_flags_program_name"                     .. static_postfix,
    "absl_flags_reflection"                       .. static_postfix,
    "absl_flags_usage"                            .. static_postfix,
    "absl_flags_usage_internal"                   .. static_postfix,
    "absl_graphcycles_internal"                   .. static_postfix,
    "absl_hash"                                   .. static_postfix,
    "absl_hashtablez_sampler"                     .. static_postfix,
    "absl_int128"                                 .. static_postfix,
    "absl_kernel_timeout_internal"                .. static_postfix,
    "absl_leak_check"                             .. static_postfix,
    "absl_log_entry"                              .. static_postfix,
    "absl_log_flags"                              .. static_postfix,
    "absl_log_globals"                            .. static_postfix,
    "absl_log_initialize"                         .. static_postfix,
    "absl_log_internal_check_op"                  .. static_postfix,
    "absl_log_internal_conditions"                .. static_postfix,
    "absl_log_internal_fnmatch"                   .. static_postfix,
    "absl_log_internal_format"                    .. static_postfix,
    "absl_log_internal_globals"                   .. static_postfix,
    "absl_log_internal_log_sink_set"              .. static_postfix,
    "absl_log_internal_message"                   .. static_postfix,
    "absl_log_internal_nullguard"                 .. static_postfix,
    "absl_log_internal_proto"                     .. static_postfix,
    "absl_log_severity"                           .. static_postfix,
    "absl_log_sink"                               .. static_postfix,
    "absl_low_level_hash"                         .. static_postfix,
    "absl_malloc_internal"                        .. static_postfix,
    "absl_periodic_sampler"                       .. static_postfix,
    "absl_random_distributions"                   .. static_postfix,
    "absl_random_internal_distribution_test_util" .. static_postfix,
    "absl_random_internal_platform"               .. static_postfix,
    "absl_random_internal_pool_urbg"              .. static_postfix,
    "absl_random_internal_randen"                 .. static_postfix,
    "absl_random_internal_randen_hwaes"           .. static_postfix,
    "absl_random_internal_randen_hwaes_impl"      .. static_postfix,
    "absl_random_internal_randen_slow"            .. static_postfix,
    "absl_random_internal_seed_material"          .. static_postfix,
    "absl_random_seed_gen_exception"              .. static_postfix,
    "absl_random_seed_sequences"                  .. static_postfix,
    "absl_raw_hash_set"                           .. static_postfix,
    "absl_raw_logging_internal"                   .. static_postfix,
    "absl_scoped_set_env"                         .. static_postfix,
    "absl_spinlock_wait"                          .. static_postfix,
    "absl_stacktrace"                             .. static_postfix,
    "absl_status"                                 .. static_postfix,
    "absl_statusor"                               .. static_postfix,
    "absl_strerror"                               .. static_postfix,
    "absl_strings"                                .. static_postfix,
    "absl_strings_internal"                       .. static_postfix,
    "absl_string_view"                            .. static_postfix,
    "absl_str_format_internal"                    .. static_postfix,
    "absl_symbolize"                              .. static_postfix,
    "absl_synchronization"                        .. static_postfix,
    "absl_throw_delegate"                         .. static_postfix,
    "absl_time"                                   .. static_postfix,
    "absl_time_zone"                              .. static_postfix,
    "absl_vlog_config_internal"                   .. static_postfix,
    "utf8_range"                                  .. static_postfix,
    "utf8_validity"                               .. static_postfix,
})

local common_link_win = {
    -- os specific
    "Ws2_32"   .. static_postfix,
    "Iphlpapi" .. static_postfix,
    "Wldap32"  .. static_postfix,
    "Winmm"    .. static_postfix,
    "Bcrypt"   .. static_postfix,
    "Dbghelp"  .. static_postfix,
    -- gamepad
    "Xinput"   .. static_postfix,
    -- imgui / overlay
    "Gdi32"    .. static_postfix,
    "Dwmapi"   .. static_postfix,
}
-- add deps to win
table_append(common_link_win, deps_link)

local common_link_linux = {
    -- os specific
    "pthread", "dl",
}
-- add deps to linux
table_append(common_link_linux, deps_link)

-- overlay libs
local overlay_link = {
    "ingame_overlay",
    "system", -- ingame_overlay dependency
    "mini_detour", -- ingame_overlay dependency
}
-- we add them later when needed


-- dirs to custom libs
---------
local x32_ssq_libdir = path.join(deps_dir, "libssq/build32")
local x64_ssq_libdir = path.join(deps_dir, "libssq/build64")

local cmake_generator = os.getenv("CMAKE_GENERATOR") or ""
if cmake_generator == "" and os.host() == 'windows' or cmake_generator:find("Visual Studio") then
    x32_ssq_libdir = x32_ssq_libdir .. "/Release"
    x64_ssq_libdir = x64_ssq_libdir .. "/Release"
end

local x32_deps_libdir = {
    x32_ssq_libdir,
    path.join(deps_dir, "curl/install32/lib"),
    path.join(deps_dir, "protobuf/install32/lib"),
    path.join(deps_dir, "zlib/install32/lib"),
    path.join(deps_dir, "mbedtls/install32/lib"),
}

local x32_deps_overlay_libdir = {
    path.join(deps_dir, "ingame_overlay/install32/lib"),
    path.join(deps_dir, "ingame_overlay/deps/System/install32/lib"),
    path.join(deps_dir, "ingame_overlay/deps/mini_detour/install32/lib"),
}

local x64_deps_libdir = {
    x64_ssq_libdir,
    path.join(deps_dir, "curl/install64/lib"),
    path.join(deps_dir, "protobuf/install64/lib"),
    path.join(deps_dir, "zlib/install64/lib"),
    path.join(deps_dir, "mbedtls/install64/lib"),
    path.join(deps_dir, "ingame_overlay/install64/lib"),
}

local x64_deps_overlay_libdir = {
    path.join(deps_dir, "ingame_overlay/install64/lib"),
    path.join(deps_dir, "ingame_overlay/deps/System/install64/lib"),
    path.join(deps_dir, "ingame_overlay/deps/mini_detour/install64/lib"),
}

-- generate proto
if _OPTIONS["genproto"] then
    if genproto() then
        print("Success!")
    else
        error("protoc error")
    end
end
-- End generate proto



-- tokenization
-- https://premake.github.io/docs/Tokens/
-- this means expand the global var 'abc' --> %{abc}
-- this means expand the global var 'abc' and resolve its full path --> %{!abc}
-- this means expand the global var 'abc' as a filepath agnostic to the shell (bash/cmd) --> %[%{abc}]

-- string concat and functions calls
-- https://premake.github.io/docs/Your-First-Script#functions-and-arguments
-- "asd" .. "zxc" --> "asdzxc"
-- when doing string concat, call premake functions/actions with regular brackets
-- this will work: targetdir("build/" .. os_iden)
-- this will fail: targetdir "build/" .. os_iden
-- both are function calls actually, ex: filter({ 'a', 'b' }) is similar to filter { 'a', 'b' }

-- stuff defined globally will affect all workspaces & projects
-- https://premake.github.io/docs/Scopes-and-Inheritance/

filter {} -- reset the filter and remove all active keywords
configurations { "debug", "release", }
platforms { "x64", "x32", }
language "C++"
cppdialect "C++17"
cdialect "C17"
filter { "system:not windows", "action:gmake*" , }
    cdialect("gnu17") -- gamepad.c relies on some linux-specific functions like strdup() and MAX_PATH
filter {} -- reset the filter and remove all active keywords
characterset "Unicode"
staticruntime "on" -- /MT or /MTd
runtime "Release" -- ensure we never link with /MTd, otherwise deps linking will fail
flags {
    "NoPCH", -- no precompiled header on Windows
    "MultiProcessorCompile", -- /MP "Enable Visual Studio to use multiple compiler processes when building"
    "RelativeLinks",
}
targetprefix "" -- prevent adding the prefix libxxx on linux
visibility "Hidden" -- hide all symbols by default on GCC (unless they are marked visible)
linkgroups "On" -- turn off the awful order dependent linking on gcc/clang, causes the linker to go back and forth to find missing symbols
exceptionhandling "On" -- "Enable exception handling. ... although it does not affect execution."
stringpooling "On" -- cache similar strings
vpaths { -- just for visual niceness, see: https://premake.github.io/docs/vpaths/
    ["headers/*"] = {
        "**.h", "**.hxx", "**.hpp",
    },
    ["src/*"] = {
        "**.c", "**.cxx", "**.cpp", "**.cc",
    },
    ["proto/*"] = {
        "**.proto",
    },
    ["docs/*"] = {
        -- post build
        "post_build/**",
        -- licence files
        "**/LICENSE", "**/LICENCE",
        "**.LICENSE", "**.LICENCE",
        "**.mit",
        -- anything else
        "**.txt", "**.md",
    },
}


-- arch
---------
filter { "platforms:x32", }
    architecture "x86" 
filter { "platforms:x64", }
    architecture "x86_64"


-- debug/optimization flags
---------
filter {} -- reset the filter and remove all active keywords
intrinsics "On"
filter { "configurations:*debug", }
    symbols "On"
    optimize "Off"
filter { "configurations:*release", }
    symbols "Off"
    optimize "On"


--- common compiler/linker options
---------
-- Visual Studio common compiler/linker options
filter { "action:vs*", }
    buildoptions  {
        "/permissive-", "/DYNAMICBASE", "/bigobj",
        "/utf-8", "/Zc:char8_t-", "/EHsc", "/GL-"
    }
    linkoptions  {
        -- source of emittoolversioninfo: https://developercommunity.visualstudio.com/t/add-linker-option-to-strip-rich-stamp-from-exe-hea/740443
        "/NOLOGO", "/emittoolversioninfo:no"
    }
-- GNU make common compiler/linker options
filter { "action:gmake*", }
    buildoptions  {
        -- https://gcc.gnu.org/onlinedocs/gcc/Code-Gen-Options.html
        "-fno-jump-tables" , "-Wno-switch",
    }
    linkoptions {
        "-Wl,--exclude-libs,ALL",
    }
-- this is made separate because GCC complains but not CLANG
filter { "action:gmake*" , "files:*.cpp or *.cxx or *.cc or *.hpp or *.hxx", }
    buildoptions  {
        "-fno-char8_t", -- GCC gives a warning when a .c file is compiled with this
    }
filter {} -- reset the filter and remove all active keywords


-- defines
---------
filter {} -- reset the filter and remove all active keywords
defines { -- added to all filters, later defines will be appended
    common_emu_defines,
}
-- release mode defines
filter { "configurations:*release" }
    defines {
        "NDEBUG", "EMU_RELEASE_BUILD"
    }
-- debug mode defines
filter { "configurations:*debug" }
    defines {
        "DEBUG",
    }
-- Windows defines
filter { "system:windows", }
    defines {
        "_CRT_SECURE_NO_WARNINGS",
    }
-- Linux defines
filter { "system:not windows" }
    defines {
        "GNUC",
    }


-- MinGw on Windows
-- common compiler/linker options: source: https://gcc.gnu.org/onlinedocs/gcc/Cygwin-and-MinGW-Options.html
---------
filter { "system:windows", "action:gmake*", }
    -- MinGw on Windows common defines
    -- MinGw on Windows doesn't have a definition for '_S_IFDIR' which is microsoft specific: https://learn.microsoft.com/en-us/cpp/c-runtime-library/reference/stat-functions
    -- this is used in 'base.cpp' -> if ( buffer.st_mode & _S_IFDIR)
    -- instead microsoft has an alternative but only enabled when _CRT_DECLARE_NONSTDC_NAMES is defined
    -- https://learn.microsoft.com/en-us/cpp/c-runtime-library/compatibility
    defines {
        -- '_CRT_NONSTDC_NO_WARNINGS',
        '_CRT_DECLARE_NONSTDC_NAMES',
    }
    linkoptions {
        -- I don't know why but if libgcc/libstdc++ as well as pthreads are not statically linked
        -- none of the output binary .dlls will reach their DllMain() in x64dbg
        -- even when they're force-loaded in any process they immediately unload
        -- '-static-libgcc' ,'-static-libstdc++',
        '-static',
    }
-- MinGw on Windows cannot compile 'creatwth.cpp' from Detours lib (error: 'DWordMult' was not declared in this scope)
-- because intsafe.h isn't included by default
filter { "system:windows", "action:gmake*", "files:**/detours/creatwth.cpp" }
    buildoptions  {
        "-include intsafe.h",
    }


-- add extra files for clearance
filter {} -- reset the filter and remove all active keywords
-- post build docs
filter { 'options:incexamples', }
    files {
        'post_build/**',
    }
filter { 'options:incexamples', 'system:not windows', }
    removefiles {
        'post_build/win/**'
    }

-- deps
filter { 'options:incdeps', "platforms:x32", }
    files {
        table_postfix_items(x32_deps_include, '/**.h'),
        table_postfix_items(x32_deps_include, '/**.hxx'),
        table_postfix_items(x32_deps_include, '/**.hpp'),
    }
filter { 'options:incdeps', "platforms:x64", }
    files {
        table_postfix_items(x64_deps_include, '/**.h'),
        table_postfix_items(x64_deps_include, '/**.hxx'),
        table_postfix_items(x64_deps_include, '/**.hpp'),
    }
filter {} -- reset the filter and remove all active keywords


-- include dirs
---------
-- common include dir
filter {} -- reset the filter and remove all active keywords
includedirs {
    common_include,
}


-- disable warnings for external libraries/deps
filter { 'files:proto_gen/** or libs/** or build/deps/**' }
    warnings 'Off'
filter {} -- reset the filter and remove all active keywords



-- post build change DOS stub + sign
---------
if os.target() == "windows" then

-- token expansion like '%{cfg.platform}' happens later during project build
local dos_stub_exe = path.translate(path.getabsolute('resources/win/file_dos_stub/file_dos_stub_%{cfg.platform}.exe', _MAIN_SCRIPT_DIR), '\\')
local signer_tool = path.translate(path.getabsolute('third-party/build/win/cert/sign_helper.bat', _MAIN_SCRIPT_DIR), '\\')
-- change dos stub
filter { "system:windows", "options:dosstub", }
    postbuildcommands {
        '"' .. dos_stub_exe .. '" %[%{!cfg.buildtarget.abspath}]',
    }
-- sign
filter { "system:windows", "options:winsign", }
    postbuildcommands {
        '"' .. signer_tool .. '" %[%{!cfg.buildtarget.abspath}]',
    }
filter {} -- reset the filter and remove all active keywords
end



workspace "gbe"
    location("build/project/%{_ACTION}/" .. os_iden)


-- Project api_regular
---------
project "api_regular"
    kind "SharedLib"
    location "%{wks.location}/%{prj.name}"
    targetdir("build/" .. os_iden .. "/%{_ACTION}/%{cfg.buildcfg}/regular/%{cfg.platform}")


    -- name
    ---------
    filter { "system:windows", "platforms:x32", }
        targetname "steam_api"
    filter { "system:windows", "platforms:x64", }
        targetname "steam_api64"
    filter { "system:not windows", }
        targetname "libsteam_api"


    -- x32 include dir
    filter { "platforms:x32", }
        includedirs {
            x32_deps_include,
        }

    -- x64 include dir
    filter { "platforms:x64", }
        includedirs {
            x64_deps_include,
        }


    -- common source & header files
    ---------
    filter {} -- reset the filter and remove all active keywords
    files { -- added to all filters, later defines will be appended
        common_files,
    }
    removefiles {
        detours_files,
    }
    -- Windows common source files
    filter { "system:windows", }
        removefiles {
            "dll/wrap.cpp"
        }
    -- Windows x32 common source files
    filter { "system:windows", "platforms:x32", "options:winrsrc", }
        files {
            "resources/win/api/32/resources.rc"
        }
    -- Windows x64 common source files
    filter { "system:windows", "platforms:x64", "options:winrsrc", }
        files {
            "resources/win/api/64/resources.rc"
        }


    -- libs to link
    ---------
    -- Windows libs to link
    filter { "system:windows", }
        links {
            common_link_win,
        }

    -- Linux libs to link
    filter { "system:not windows", }
        links {
            common_link_linux,
        }


    -- libs search dir
    ---------
    -- x32 libs search dir
    filter { "platforms:x32", }
        libdirs {
            x32_deps_libdir,
        }
    -- x64 libs search dir
    filter { "platforms:x64", }
        libdirs {
            x64_deps_libdir,
        }
-- End api_regular


-- Project api_experimental
---------
project "api_experimental"
    kind "SharedLib"
    location "%{wks.location}/%{prj.name}"
    targetdir("build/" .. os_iden .. "/%{_ACTION}/%{cfg.buildcfg}/experimental/%{cfg.platform}")


    -- name
    ---------
    filter { "system:windows", "platforms:x32", }
        targetname "steam_api"
    filter { "system:windows", "platforms:x64", }
        targetname "steam_api64"
    filter { "system:not windows", }
        targetname "libsteam_api"


    -- defines
    ---------
    filter {} -- reset the filter and remove all active keywords
    defines { -- added to all filters, later defines will be appended
        "EMU_OVERLAY", "ImTextureID=ImU64",
    }
    -- Windows defines
    filter { "system:windows" }
        defines {
            "EMU_EXPERIMENTAL_BUILD",
        }


    -- include dir
    ---------
    -- x32 include dir
    filter { "platforms:x32", }
        includedirs {
            x32_deps_include,
            x32_deps_overlay_include,
        }
    -- x64 include dir
    filter { "platforms:x64", }
        includedirs {
            x64_deps_include,
            x64_deps_overlay_include,
        }


    -- common source & header files
    ---------
    filter {} -- reset the filter and remove all active keywords
    files { -- added to all filters, later defines will be appended
        common_files,
        overlay_files,
    }
    removefiles {
        'libs/detours/uimports.cc',
    }
    -- deps
    filter { 'options:incdeps', "platforms:x32", }
        files {
            table_postfix_items(x32_deps_overlay_include, '/**.h'),
            table_postfix_items(x32_deps_overlay_include, '/**.hxx'),
            table_postfix_items(x32_deps_overlay_include, '/**.hpp'),
        }
    filter { 'options:incdeps', "platforms:x64", }
        files {
            table_postfix_items(x64_deps_overlay_include, '/**.h'),
            table_postfix_items(x64_deps_overlay_include, '/**.hxx'),
            table_postfix_items(x64_deps_overlay_include, '/**.hpp'),
        }
    -- Windows common source files
    filter { "system:windows", }
        removefiles {
            "dll/wrap.cpp"
        }
    -- Windows x32 common source files
    filter { "system:windows", "platforms:x32", "options:winrsrc", }
        files {
            "resources/win/api/32/resources.rc"
        }
    -- Windows x64 common source files
    filter { "system:windows", "platforms:x64", "options:winrsrc", }
        files {
            "resources/win/api/64/resources.rc"
        }
    -- Linux common source files
    filter { "system:not windows", }
        removefiles {
            detours_files,
        }


    -- libs to link
    ---------
    filter {} -- reset the filter and remove all active keywords
        links {
            overlay_link,
        }
    -- Windows libs to link
    filter { "system:windows", }
        links {
            common_link_win,
        }

    -- Linux libs to link
    filter { "system:not windows", }
        links {
            common_link_linux,
            "X11"
        }


    -- libs search dir
    ---------
    -- x32 libs search dir
    filter { "platforms:x32", }
        libdirs {
            x32_deps_libdir,
            x32_deps_overlay_libdir,
        }
    -- x64 libs search dir
    filter { "platforms:x64", }
        libdirs {
            x64_deps_libdir,
            x64_deps_overlay_libdir,
        }
-- End api_experimental


-- Project steamclient_experimental
---------
project "steamclient_experimental"
    kind "SharedLib"
    location "%{wks.location}/%{prj.name}"
    
    -- targetdir
    ---------
    filter { "system:windows", }
        targetdir("build/" .. os_iden .. "/%{_ACTION}/%{cfg.buildcfg}/steamclient_experimental")
    filter { "system:not windows", }
        targetdir("build/" .. os_iden .. "/%{_ACTION}/%{cfg.buildcfg}/experimental/%{cfg.platform}")


    -- name
    ---------
    filter { "system:windows", "platforms:x32", }
        targetname "steamclient"
    filter { "system:windows", "platforms:x64", }
        targetname "steamclient64"
    filter { "system:not windows", }
        targetname "steamclient"
    

    -- defines
    ---------
    filter {} -- reset the filter and remove all active keywords
    defines { -- added to all filters, later defines will be appended
        "STEAMCLIENT_DLL", "EMU_OVERLAY", "ImTextureID=ImU64",
    }
    -- Windows defines
    filter { "system:windows" }
        defines {
            "EMU_EXPERIMENTAL_BUILD",
        }


    -- include dir
    ---------
    -- x32 include dir
    filter { "platforms:x32", }
        includedirs {
            x32_deps_include,
            x32_deps_overlay_include,
        }

    -- x64 include dir
    filter { "platforms:x64", }
        includedirs {
            x64_deps_include,
            x64_deps_overlay_include,
        }


    -- common source & header files
    ---------
    filter {} -- reset the filter and remove all active keywords
    files { -- added to all filters, later defines will be appended
        common_files,
        overlay_files,
    }
    removefiles {
        'libs/detours/uimports.cc',
    }
    -- deps
    filter { 'options:incdeps', "platforms:x32", }
        files {
            table_postfix_items(x32_deps_overlay_include, '/**.h'),
            table_postfix_items(x32_deps_overlay_include, '/**.hxx'),
            table_postfix_items(x32_deps_overlay_include, '/**.hpp'),
        }
    filter { 'options:incdeps', "platforms:x64", }
        files {
            table_postfix_items(x64_deps_overlay_include, '/**.h'),
            table_postfix_items(x64_deps_overlay_include, '/**.hxx'),
            table_postfix_items(x64_deps_overlay_include, '/**.hpp'),
        }
    -- Windows common source files
    filter { "system:windows", }
        removefiles {
            "dll/wrap.cpp"
        }
    -- Windows x32 common source files
    filter { "system:windows", "platforms:x32", "options:winrsrc", }
        files {
            "resources/win/client/32/resources.rc"
        }
    -- Windows x64 common source files
    filter { "system:windows", "platforms:x64", "options:winrsrc", }
        files {
            "resources/win/client/64/resources.rc"
        }
    -- Linux common source files
    filter { "system:not windows", }
        removefiles {
            detours_files,
        }


    -- libs to link
    ---------
    filter {} -- reset the filter and remove all active keywords
        links {
            overlay_link,
        }
    -- Windows libs to link
    filter { "system:windows", }
        links {
            common_link_win,
        }

    -- Linux libs to link
    filter { "system:not windows", }
        links {
            common_link_linux,
        }


    -- libs search dir
    ---------
    -- x32 libs search dir
    filter { "platforms:x32", }
        libdirs {
            x32_deps_libdir,
            x32_deps_overlay_libdir,
        }
    -- x64 libs search dir
    filter { "platforms:x64", }
        libdirs {
            x64_deps_libdir,
            x64_deps_overlay_libdir,
        }
-- End steamclient_experimental


-- Project tool_lobby_connect
---------
project "tool_lobby_connect"
    kind "ConsoleApp"
    location "%{wks.location}/%{prj.name}"
    targetdir("build/" .. os_iden .. "/%{_ACTION}/%{cfg.buildcfg}/tools/lobby_connect")
    targetname "lobby_connect_%{cfg.platform}"


    -- defines
    ---------
    filter {} -- reset the filter and remove all active keywords
    defines { -- added to all filters, later defines will be appended
        "NO_DISK_WRITES", "LOBBY_CONNECT",
    }
    removedefines {
        "CONTROLLER_SUPPORT",
    }


    -- include dir
    ---------
    -- common include dir
    -- x32 include dir
    filter { "platforms:x32", }
        includedirs {
            x32_deps_include,
        }

    -- x64 include dir
    filter { "platforms:x64", }
        includedirs {
            x64_deps_include,
        }


    -- common source & header files
    ---------
    filter {} -- reset the filter and remove all active keywords
    files { -- added to all filters, later defines will be appended
        common_files,
        'tools/lobby_connect/lobby_connect.cpp'
    }
    removefiles {
        "libs/gamepad/**",
        detours_files,
    }
    -- Windows x32 common source files
    filter { "system:windows", "platforms:x32", "options:winrsrc", }
        files {
            "resources/win/launcher/32/resources.rc"
        }
    -- Windows x64 common source files
    filter { "system:windows", "platforms:x64", "options:winrsrc", }
        files {
            "resources/win/launcher/64/resources.rc"
        }


    -- libs to link
    ---------
    -- Windows libs to link
    filter { "system:windows", }
        links {
            common_link_win,
            'Comdlg32',
        }

    -- Linux libs to link
    filter { "system:not windows", }
        links {
            common_link_linux,
        }


    -- libs search dir
    ---------
    -- x32 libs search dir
    filter { "platforms:x32", }
        libdirs {
            x32_deps_libdir,
        }
    -- x64 libs search dir
    filter { "platforms:x64", }
        libdirs {
            x64_deps_libdir,
        }
-- End tool_lobby_connect


-- Project tool_generate_interfaces
project "tool_generate_interfaces"
    kind "ConsoleApp"
    location "%{wks.location}/%{prj.name}"
    targetdir("build/" .. os_iden .. "/%{_ACTION}/%{cfg.buildcfg}/tools/generate_interfaces")
    targetname "generate_interfaces_%{cfg.platform}"


    -- common source & header files
    ---------
    files {
        "tools/generate_interfaces/generate_interfaces.cpp"
    }
-- End tool_generate_interfaces


-- Project lib_steamnetworkingsockets START
project "lib_steamnetworkingsockets"
    kind "SharedLib"
    location "%{wks.location}/%{prj.name}"
    targetdir("build/" .. os_iden .. "/%{_ACTION}/%{cfg.buildcfg}/steamnetworkingsockets/%{cfg.platform}")
    targetname "steamnetworkingsockets"


    -- common source & header files
    ---------
    files {
        "networking_sockets_lib/**",
        "helpers/dbg_log.cpp", "helpers/dbg_log/**",
        'helpers/common_helpers.cpp', 'helpers/common_helpers/**',
    }


-- End lib_steamnetworkingsockets


-- Project lib_game_overlay_renderer
project "lib_game_overlay_renderer"
    kind "SharedLib"
    location "%{wks.location}/%{prj.name}"


    -- targetdir
    ---------
    filter { "system:windows", }
        targetdir("build/" .. os_iden .. "/%{_ACTION}/%{cfg.buildcfg}/steamclient_experimental")
    filter { "system:not windows", }
        targetdir("build/" .. os_iden .. "/%{_ACTION}/%{cfg.buildcfg}/gameoverlayrenderer/%{cfg.platform}")


    -- name
    ---------
    filter { "system:windows", "platforms:x32", }
        targetname "GameOverlayRenderer"
    filter { "system:windows", "platforms:x64", }
        targetname "GameOverlayRenderer64"
    filter { "system:not windows", }
        targetname "gameoverlayrenderer"

    
    -- include dir
    ---------
    -- x32 include dir
    filter { "platforms:x32", }
        includedirs {
            x32_deps_include,
        }

    -- x64 include dir
    filter { "platforms:x64", }
        includedirs {
            x64_deps_include,
        }


    -- common source & header files
    ---------
    filter {} -- reset the filter and remove all active keywords
    files {
        "game_overlay_renderer_lib/**"
    }
    -- x32 common source files
    filter { "system:windows", "platforms:x32", "options:winrsrc", }
        files {
            "resources/win/game_overlay_renderer/32/resources.rc"
        }
    -- x64 common source files
    filter { "system:windows", "platforms:x64", "options:winrsrc", }
        files {
            "resources/win/game_overlay_renderer/64/resources.rc"
        }
-- End lib_game_overlay_renderer



-- WINDOWS ONLY TARGETS START
if os.target() == "windows" then


-- Project steamclient_experimental_stub
---------
project "steamclient_experimental_stub"
    -- https://stackoverflow.com/a/63228027
    kind "SharedLib"
    location "%{wks.location}/%{prj.name}"
    targetdir("build/" .. os_iden .. "/%{_ACTION}/%{cfg.buildcfg}/experimental/%{cfg.platform}")


    -- name
    ---------
    filter { "platforms:x32", }
        targetname "steamclient"
    filter { "platforms:x64", }
        targetname "steamclient64"


    -- common source & header files
    ---------
    filter {} -- reset the filter and remove all active keywords
    files { -- added to all filters, later defines will be appended
        "steamclient/steamclient.cpp",
    }
    -- x32 common source files
    filter { "platforms:x32", "options:winrsrc", }
        files {
            "resources/win/client/32/resources.rc"
        }
    -- x64 common source files
    filter { "platforms:x64", "options:winrsrc", }
        files {
            "resources/win/client/64/resources.rc"
        }
-- End steamclient_experimental_stub


-- Project steamclient_experimental_extra
project "steamclient_experimental_extra"
    kind "SharedLib"
    location "%{wks.location}/%{prj.name}"
    targetdir("build/" .. os_iden .. "/%{_ACTION}/%{cfg.buildcfg}/steamclient_experimental/extra_dlls")
    targetname "steamclient_extra_%{cfg.platform}"


    -- include dir
    ---------
    -- x32 include dir
    filter { "platforms:x32", }
        includedirs {
            x32_deps_include,
        }
    -- x64 include dir
    filter { "platforms:x64", }
        includedirs {
            x64_deps_include,
        }


    -- common source & header files
    ---------
    filter {} -- reset the filter and remove all active keywords
    files {
        "tools/steamclient_loader/win/extra_protection/**",
        "helpers/pe_helpers.cpp", "helpers/pe_helpers/**",
        "helpers/common_helpers.cpp", "helpers/common_helpers/**",
        -- detours
        detours_files,
    }
    removefiles {
        'libs/detours/uimports.cc',
    }
    -- x32 common source files
    filter { "platforms:x32", "options:winrsrc", }
        files {
            "resources/win/client/32/resources.rc"
        }
    -- x64 common source files
    filter { "platforms:x64", "options:winrsrc", }
        files {
            "resources/win/client/64/resources.rc"
        }
-- End steamclient_experimental_extra


-- Project steamclient_experimental_loader
project "steamclient_experimental_loader"
    kind "WindowedApp"
    location "%{wks.location}/%{prj.name}"
    targetdir("build/" .. os_iden .. "/%{_ACTION}/%{cfg.buildcfg}/steamclient_experimental")
    targetname "steamclient_loader_%{cfg.platform}"


    --- common compiler/linker options
    ---------
    -- MinGW on Windows
    filter { "action:gmake*", }
        -- source: https://gcc.gnu.org/onlinedocs/gcc/Cygwin-and-MinGW-Options.html
        linkoptions {
            -- MinGW on Windows cannot link wWinMain by default
            "-municode",
        }


    -- common source & header files
    ---------
    filter {} -- reset the filter and remove all active keywords
    files {
        "tools/steamclient_loader/win/*", -- we want the .ini too
        "helpers/pe_helpers.cpp", "helpers/pe_helpers/**",
        "helpers/common_helpers.cpp", "helpers/common_helpers/**",
        "helpers/dbg_log.cpp", "helpers/dbg_log/**",
        "libs/simpleini/**",
    }
    -- x32 common source files
    filter { "platforms:x32", "options:winrsrc", }
        files {
            "resources/win/launcher/32/resources.rc"
        }
    -- x64 common source files
    filter { "platforms:x64", "options:winrsrc", }
        files {
            "resources/win/launcher/64/resources.rc"
        }


    -- libs to link
    ---------
    filter {} -- reset the filter and remove all active keywords
    links {
        -- common_link_win,
        'user32',
    }
-- End steamclient_experimental_loader


-- Project tool_file_dos_stub_changer
project "tool_file_dos_stub_changer"
    kind "ConsoleApp"
    location "%{wks.location}/%{prj.name}"
    targetdir("build/" .. os_iden .. "/%{_ACTION}/%{cfg.buildcfg}/file_dos_stub_changer")
    targetname "file_dos_stub_%{cfg.platform}"


    -- common source & header files
    ---------
    filter {} -- reset the filter and remove all active keywords
    files {
        "resources/win/file_dos_stub/file_dos_stub.cpp",
        "helpers/pe_helpers.cpp", "helpers/pe_helpers/**",
        "helpers/common_helpers.cpp", "helpers/common_helpers/**",
    }
-- End tool_file_dos_stub_changer


-- Project test_crash_printer
---------
project "test_crash_printer"
    kind "ConsoleApp"
    location "%{wks.location}/%{prj.name}"
    targetdir("build/" .. os_iden .. "/%{_ACTION}/%{cfg.buildcfg}/tests/crash_printer")
    targetname "test_crash_printer_%{cfg.platform}"


    -- common source & header files
    ---------
    filter {} -- reset the filter and remove all active keywords
    files { -- added to all filters, later defines will be appended
        'crash_printer/' .. os_iden .. '.cpp', 'crash_printer/crash_printer/' .. os_iden .. '.hpp',
        -- helpers
        'helpers/common_helpers.cpp', 'helpers/common_helpers/**',
        -- test files
        'crash_printer/tests/test_win.cpp',
    }
    removefiles {
        'post_build/**',
        'build/deps/**',
    }


    -- libs to link
    ---------
    filter {} -- reset the filter and remove all active keywords
    -- Windows libs
    links {
        'Dbghelp',
    }


    -- post build
    ---------
    filter {} -- reset the filter and remove all active keywords
    postbuildcommands {
        '%[%{!cfg.buildtarget.abspath}]',
    }
-- End test_crash_printer

end
-- End WINDOWS ONLY TARGETS



-- LINUX ONLY TARGETS START
if os.target() == "linux" then

-- Project steamclient_regular
---------
project "steamclient_regular"
    kind "SharedLib"
    location "%{wks.location}/%{prj.name}"
    targetdir("build/" .. os_iden .. "/%{_ACTION}/%{cfg.buildcfg}/regular/%{cfg.platform}")
    targetname "steamclient"


    -- defines
    ---------
    filter {} -- reset the filter and remove all active keywords
    defines { -- added to all filters, later defines will be appended
        "STEAMCLIENT_DLL",
    }


    -- include dir
    ---------
    -- x32 include dir
    filter { "platforms:x32", }
        includedirs {
            x32_deps_include,
        }
    -- x64 include dir
    filter { "platforms:x64", }
        includedirs {
            x64_deps_include,
        }


    -- common source & header files
    ---------
    filter {} -- reset the filter and remove all active keywords
    files { -- added to all filters, later defines will be appended
        common_files,
    }
    removefiles {
        detours_files,
    }


    -- libs to link
    ---------
    filter {} -- reset the filter and remove all active keywords
    links { -- added to all filters, later defines will be appended
        common_link_linux,
    }

    -- libs search dir
    ---------
    -- x32 libs search dir
    filter { "platforms:x32", }
        libdirs {
            x32_deps_libdir,
        }
    -- x64 libs search dir
    filter { "platforms:x64", }
        libdirs {
            x64_deps_libdir,
        }
-- End steamclient_regular


-- Project test_crash_printer_sa_handler
---------
project "test_crash_printer_sa_handler"
    kind "ConsoleApp"
    location "%{wks.location}/%{prj.name}"
    targetdir("build/" .. os_iden .. "/%{_ACTION}/%{cfg.buildcfg}/tests/crash_printer")
    targetname "test_crash_printer_sa_handler_%{cfg.platform}"


    -- common source & header files
    ---------
    filter {} -- reset the filter and remove all active keywords
    files { -- added to all filters, later defines will be appended
        'crash_printer/' .. os_iden .. '.cpp', 'crash_printer/crash_printer/' .. os_iden .. '.hpp',
        -- helpers
        'helpers/common_helpers.cpp', 'helpers/common_helpers/**',
        -- test files
        'crash_printer/tests/test_linux_sa_handler.cpp',
    }
    removefiles {
        'post_build/**',
        'build/deps/**',
    }


    -- post build
    ---------
    filter {} -- reset the filter and remove all active keywords
    postbuildcommands {
        '%[%{!cfg.buildtarget.abspath}]',
    }

-- End test_crash_printer_sa_handler


-- Project test_crash_printer_sa_sigaction
---------
project "test_crash_printer_sa_sigaction"
    kind "ConsoleApp"
    location "%{wks.location}/%{prj.name}"
    targetdir("build/" .. os_iden .. "/%{_ACTION}/%{cfg.buildcfg}/tests/crash_printer")
    targetname "test_crash_printer_sa_sigaction_%{cfg.platform}"


    -- common source & header files
    ---------
    filter {} -- reset the filter and remove all active keywords
    files { -- added to all filters, later defines will be appended
        'crash_printer/' .. os_iden .. '.cpp', 'crash_printer/crash_printer/' .. os_iden .. '.hpp',
        -- helpers
        'helpers/common_helpers.cpp', 'helpers/common_helpers/**',
        -- test files
        'crash_printer/tests/test_linux_sa_sigaction.cpp',
    }
    removefiles {
        'post_build/**',
        'build/deps/**',
    }


    -- post build
    ---------
    filter {} -- reset the filter and remove all active keywords
    postbuildcommands {
        '%[%{!cfg.buildtarget.abspath}]',
    }
-- End test_crash_printer_sa_sigaction

end
-- End LINUX ONLY TARGETS

-- End Workspace
