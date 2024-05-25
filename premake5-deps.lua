require("premake", ">=5.0.0-beta2")

local os_iden = '' -- identifier
if os.target() == "windows" then
    os_iden = 'win'
elseif os.target() == "linux" then
    os_iden = 'linux'
else
    error('Unsupported os target: "' .. os.target() .. '"')
end


-- options
---------
-- verbose
newoption {
    trigger = "verbose",
    description = "Verbose output",
}

-- local tools
newoption {
    category = "tools",
    trigger = "custom-cmake",
    description = "Use custom cmake",
    value = 'path/to/cmake.exe',
    default = nil
}

-- deps extraction
newoption {
    category = "extract",
    trigger = "all-ext",
    description = "Extract all deps",
}
newoption {
    category = "extract",
    trigger = "clean-ext",
    description = "Clean folder before extraction",
}

newoption {
    category = "extract",
    trigger = "ext-ssq",
    description = "Extract ssq",
}
newoption {
    category = "extract",
    trigger = "ext-zlib",
    description = "Extract zlib",
}
newoption {
    category = "extract",
    trigger = "ext-curl",
    description = "Extract curl",
}
newoption {
    category = "extract",
    trigger = "ext-protobuf",
    description = "Extract protobuf",
}
newoption {
    category = "extract",
    trigger = "ext-mbedtls",
    description = "Extract mbedtls",
}
newoption {
    category = "extract",
    trigger = "ext-ingame_overlay",
    description = "Extract ingame_overlay",
}

-- build
newoption {
    category = "build",
    trigger = "all-build",
    description = "Build all deps",
}
newoption {
    category = "build",
    trigger = "clean-build",
    description = "Clean folder before building",
}
newoption {
    category = "build",
    trigger = "32-build",
    description = "Build for 32-bit arch",
}
newoption {
    category = "build",
    trigger = "64-build",
    description = "Build for 64-bit arch",
}

newoption {
    category = "build",
    trigger = "build-ssq",
    description = "Build ssq",
}
newoption {
    category = "build",
    trigger = "build-zlib",
    description = "Build zlib",
}
newoption {
    category = "build",
    trigger = "build-curl",
    description = "Build curl",
}
newoption {
    category = "build",
    trigger = "build-protobuf",
    description = "Build protobuf",
}
newoption {
    category = "build",
    trigger = "build-mbedtls",
    description = "Build mbedtls",
}
newoption {
    category = "build",
    trigger = "build-ingame_overlay",
    description = "Build ingame_overlay",
}


local function merge_list(src, dest)
    local src_count = #src
    local res = {}

    for idx = 1, src_count do
        res[idx] = src[idx]
    end

    for idx = 1, #dest do
        res[src_count + idx] = dest[idx]
    end
    return res
end



-- common defs
---------

local deps_dir = os.realpath(path.join('build', 'deps', os_iden))
local third_party_dir = os.realpath('third-party')
local third_party_deps_dir = os.realpath(path.join(third_party_dir, 'deps', os_iden))
local third_party_common_dir = os.realpath(path.join(third_party_dir, 'deps', 'common'))
local extractor = os.realpath(path.join(third_party_deps_dir, '7za', '7za'))
local mycmake = os.realpath(path.join(third_party_deps_dir, 'cmake', 'bin', 'cmake'))

if _OPTIONS["custom-cmake"] then
    mycmake = _OPTIONS["custom-cmake"]
    print('using custom cmake: ' .. _OPTIONS["custom-cmake"])
end


-- ############## common CMAKE args ##############
-- https://cmake.org/cmake/help/latest/variable/CMAKE_LANG_FLAGS_CONFIG.html#variable:CMAKE_%3CLANG%3E_FLAGS_%3CCONFIG%3E
local cmake_common_defs = {
    'CMAKE_BUILD_TYPE=Release',
    'CMAKE_POSITION_INDEPENDENT_CODE=True',
    'BUILD_SHARED_LIBS=OFF',
    'CMAKE_MSVC_RUNTIME_LIBRARY=MultiThreaded',
}
if os.target() == 'windows' then
    table.insert(cmake_common_defs, 'CMAKE_C_FLAGS_RELEASE="-MT -D_MT"')
    table.insert(cmake_common_defs, 'CMAKE_CXX_FLAGS_RELEASE="-MT -D_MT"')
end

local function cmake_build(dep_folder, is_32, extra_defs)
    dep_base = os.realpath(path.join(deps_dir, dep_folder))
    local arch_iden = ''
    if is_32 then
        arch_iden = '32'
    else
        arch_iden = '64'
    end
    
    local build_dir = os.realpath(path.join(dep_base, 'build' .. arch_iden))
    
    -- clean if required
    if _OPTIONS["clean-build"] then
        print('cleaning dir: ' .. build_dir)
        os.rmdir(build_dir)
    end

    if not os.mkdir(build_dir) then
        error("failed to create build dir: " .. build_dir)
        return
    end

    local install_dir = os.realpath(path.join(dep_base, 'install' .. arch_iden))
    local cmake_common_defs_str = '-D' .. table.concat(cmake_common_defs, ' -D') .. ' -DCMAKE_INSTALL_PREFIX="' .. install_dir .. '"'
    local cmd_gen = mycmake .. ' -S "' .. dep_base .. '" -B "' .. build_dir .. '" ' .. cmake_common_defs_str

    -- arch
    if string.match(_ACTION, 'gmake.*') then
        if is_32 then
            local toolchain_file = os.realpath(path.join(deps_dir, 'toolchain_32.cmake'))
            if not os.isfile(toolchain_file) then
                if not io.writefile(toolchain_file, [[
                    set(CMAKE_C_FLAGS_INIT   "-m32")
                    set(CMAKE_CXX_FLAGS_INIT "-m32")
                ]]) then
                    error("failed to create 32-bit cmake toolchain")
                    return
                end
            end

            cmd_gen = cmd_gen .. ' -DCMAKE_TOOLCHAIN_FILE="' .. toolchain_file .. '"'
        end
    elseif string.match(_ACTION, 'vs.+') then
        if is_32 then
            cmd_gen = cmd_gen .. ' -A Win32'
        else
            cmd_gen = cmd_gen .. ' -A x64'
        end
    else
        error("unsupported action: " .. _ACTION)
        return
    end

    -- add extra defs
    if extra_defs then
        if type(extra_defs) == 'table' then
            if #extra_defs > 0 then
                local extra_defs_str = ' -D' .. table.concat(extra_defs, ' -D')
                cmd_gen = cmd_gen .. extra_defs_str
            end
        elseif type(extra_defs) == 'string' then
            cmd_gen = cmd_gen .. ' ' .. extra_defs
        else
            error("unsupported type for extra_defs: " .. type(extra_defs))
            return
        end
    end

    -- verbose generation
    if _OPTIONS['verbose'] then
        print(cmd_gen)
    end

    -- generate cmake config
    local ok = os.execute(cmd_gen)
    if not ok then
        error("failed to generate")
        return
    end

    -- verbose build
    local verbose_build_str = ''
    if _OPTIONS['verbose'] then
        verbose_build_str = ' -v'
    end

    -- build with cmake
    local ok = os.execute(mycmake .. ' --build "' .. build_dir .. '" --config Release --parallel' .. verbose_build_str)
    if not ok then
        error("failed to build")
        return
    end

    -- clean if required
    if _OPTIONS["clean-build"] then
        print('cleaning dir: ' .. install_dir)
        os.rmdir(install_dir)
    end
    -- create dir
    if not os.mkdir(install_dir) then
        error("failed to create install dir: " .. install_dir)
        return
    end

    local cmd_install = mycmake.. ' --install "' .. build_dir .. '" --prefix "' .. install_dir
    print(cmd_install)
    local ok = os.execute(cmd_install)
    if not ok then
        error("failed to install")
        return
    end
end

if os.host() == "linux" then
    local ok_chmod, err_chmod = os.chmod(extractor, "777")
    if not ok_chmod then
        error("cannot chmod: " .. err_chmod)
        return
    end

    if not _OPTIONS["custom-cmake"] then
        local ok_chmod, err_chmod = os.chmod(mycmake, "777")
        if not ok_chmod then
            error("cannot chmod: " .. err_chmod)
            return
        end
    end
end


-- extract action
-------
deps_to_extract = {} -- { 'path/to/archive', 'path/to/extraction folder'
if _OPTIONS["ext-ssq"] or _OPTIONS["all-ext"] then
    table.insert(deps_to_extract, { 'libssq/libssq.tar.gz', 'libssq' })
end
if _OPTIONS["ext-zlib"] or _OPTIONS["all-ext"] then
    table.insert(deps_to_extract, { 'zlib/zlib.tar.gz', 'zlib' })
end
if _OPTIONS["ext-curl"] or _OPTIONS["all-ext"] then
    table.insert(deps_to_extract, { 'curl/curl.tar.gz', 'curl' })
end
if _OPTIONS["ext-protobuf"] or _OPTIONS["all-ext"] then
    table.insert(deps_to_extract, { 'protobuf/protobuf.tar.gz', 'protobuf' })
end
if _OPTIONS["ext-mbedtls"] or _OPTIONS["all-ext"] then
    table.insert(deps_to_extract, { 'mbedtls/mbedtls.tar.gz', 'mbedtls' })
end
if _OPTIONS["ext-ingame_overlay"] or _OPTIONS["all-ext"] then
    table.insert(deps_to_extract, { 'ingame_overlay/ingame_overlay.tar.gz', 'ingame_overlay' })
end


for _, v in pairs(deps_to_extract) do
    -- check archive
    local archive_file = os.realpath(third_party_common_dir .. '/' .. v[1])
    if not os.isfile(archive_file) then
        error("archive not found: " .. archive_file)
        return
    end

    local out_folder = os.realpath(deps_dir .. '/' .. v[2])

    -- clean if required
    if _OPTIONS["clean-ext"] then
        print('cleaning dir: ' .. out_folder)
        os.rmdir(out_folder)
    end

    -- create out folder
    print("creating dir: " .. out_folder)
    local ok_mk, err_mk = os.mkdir(out_folder)
    if not ok_mk then
        error("Error: " .. err_mk)
        return
    end

    -- extract
    print("extracting: '" .. archive_file .. "'")
    local ext = string.lower(string.sub(archive_file, -7)) -- ".tar.gz"
    local ok_cmd = false
    if ext == ".tar.gz" then
        ok_cmd = os.execute(extractor .. ' x "' .. archive_file .. '" -so | "' .. extractor .. '" x -si -ttar -y -aoa -o"' .. out_folder .. '"')
    else
        ok_cmd = os.execute(extractor .. ' x "' .. archive_file .. '" -y -aoa -o"' .. out_folder .. '"')
    end
    if not ok_cmd then
        error('extraction failed')
    end

    local folders = os.matchdirs(out_folder .. '/*')
    for _, vv in pairs(folders) do
        local inner_folder = os.realpath(vv)
        local ok = os.execute('{COPYDIR} "' .. inner_folder  .. '" "' .. out_folder .. '"')
        if not ok then
            error('copy dir failed, src=' .. inner_folder .. ', dest=' .. out_folder)
        end
        os.rmdir(inner_folder)
    end

end


-- build action
-------
if _OPTIONS["build-ssq"] or _OPTIONS["all-build"] then
    if _OPTIONS["32-build"] then
        cmake_build('libssq', true)
    end
    if _OPTIONS["64-build"] then
        cmake_build('libssq', false)
    end
end
if _OPTIONS["build-zlib"] or _OPTIONS["all-build"] then
    if _OPTIONS["32-build"] then
        cmake_build('zlib', true)
    end
    if _OPTIONS["64-build"] then
        cmake_build('zlib', false)
    end
end

-- ############## zlib is painful ##############
-- lib curl uses the default search paths, even when ZLIB_INCLUDE_DIR and ZLIB_LIBRARY_RELEASE are defined
-- check thir CMakeLists.txt line #573
--     optional_dependency(ZLIB)
--     if(ZLIB_FOUND)
--       set(HAVE_LIBZ ON)
--       set(USE_ZLIB ON)
--     
--       # Depend on ZLIB via imported targets if supported by the running
--       # version of CMake.  This allows our dependents to get our dependencies
--       # transitively.
--       if(NOT CMAKE_VERSION VERSION_LESS 3.4)
--         list(APPEND CURL_LIBS ZLIB::ZLIB)    <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< evil
--       else()
--         list(APPEND CURL_LIBS ${ZLIB_LIBRARIES})
--         include_directories(${ZLIB_INCLUDE_DIRS})
--       endif()
--       list(APPEND CMAKE_REQUIRED_INCLUDES ${ZLIB_INCLUDE_DIRS})
--     endif()
-- we have to set the ZLIB_ROOT so that it is prepended to the search list
-- we have to set ZLIB_LIBRARY NOT ZLIB_LIBRARY_RELEASE in order to override the FindZlib module
-- we also should set ZLIB_USE_STATIC_LIBS since we want to force static builds
-- https://github.com/Kitware/CMake/blob/a6853135f569f0b040a34374a15a8361bb73901b/Modules/FindZLIB.cmake#L98C4-L98C13

local zlib_name = ''
-- name
if os.target() == 'windows' then
    zlib_name = 'zlibstatic'
else
    zlib_name = 'libz'
end
-- ext
if string.match(_ACTION, 'vs.+') then
    zlib_name = zlib_name .. '.lib'
else
    zlib_name = zlib_name .. '.a'
end

local wild_zlib_path_32 = os.realpath(path.join(deps_dir, 'zlib', 'install32', 'lib', zlib_name))
local wild_zlib_32 = {
    'ZLIB_USE_STATIC_LIBS=ON',
    'ZLIB_ROOT="' .. os.realpath(path.join(deps_dir, 'zlib', 'install32')) .. '"',
    'ZLIB_INCLUDE_DIR="' .. os.realpath(path.join(deps_dir, 'zlib', 'install32', 'include')) .. '"',
    'ZLIB_LIBRARY="' .. wild_zlib_path_32 .. '"',
}
local wild_zlib_path_64 = os.realpath(path.join(deps_dir, 'zlib', 'install64', 'lib', zlib_name))
local wild_zlib_64 = {
    'ZLIB_USE_STATIC_LIBS=ON',
    'ZLIB_ROOT="' .. os.realpath(path.join(deps_dir, 'zlib', 'install64')) .. '"',
    'ZLIB_INCLUDE_DIR="' .. os.realpath(path.join(deps_dir, 'zlib', 'install64', 'include')) .. '"',
    'ZLIB_LIBRARY="' .. wild_zlib_path_64 .. '"',
}

if _OPTIONS["build-curl"] or _OPTIONS["all-build"] then
    local curl_common_defs = {
        "BUILD_CURL_EXE=OFF",
        "BUILD_SHARED_LIBS=OFF",
        "BUILD_STATIC_CURL=OFF",
        "BUILD_STATIC_LIBS=ON",
        "CURL_USE_OPENSSL=OFF",
        "CURL_ZLIB=ON",
        "ENABLE_UNICODE=ON",
        "CURL_STATIC_CRT=ON",
        "CURL_USE_LIBSSH2=OFF",
        "CURL_USE_LIBPSL=OFF",
        "USE_LIBIDN2=OFF",
        "CURL_DISABLE_LDAP=ON",
    }
    if _OPTIONS["32-build"] then
        cmake_build('curl', true, merge_list(merge_list(curl_common_defs, wild_zlib_32), {
            'CMAKE_SHARED_LINKER_FLAGS_INIT="' .. wild_zlib_path_32 .. '"',
            'CMAKE_MODULE_LINKER_FLAGS_INIT="' .. wild_zlib_path_32 .. '"',
            'CMAKE_EXE_LINKER_FLAGS_INIT="' .. wild_zlib_path_32 .. '"',
        }))
    end
    if _OPTIONS["64-build"] then
        cmake_build('curl', false, merge_list(merge_list(curl_common_defs, wild_zlib_64), {
            'CMAKE_SHARED_LINKER_FLAGS_INIT="' .. wild_zlib_path_64 .. '"',
            'CMAKE_MODULE_LINKER_FLAGS_INIT="' .. wild_zlib_path_64 .. '"',
            'CMAKE_EXE_LINKER_FLAGS_INIT="' .. wild_zlib_path_64 .. '"',
        }))
    end
end

if _OPTIONS["build-protobuf"] or _OPTIONS["all-build"] then
    local proto_common_defs = {
        "protobuf_BUILD_TESTS=OFF",
        "protobuf_BUILD_SHARED_LIBS=OFF",
        "protobuf_WITH_ZLIB=ON",
    }
    if _OPTIONS["32-build"] then
        cmake_build('protobuf', true, merge_list(merge_list(proto_common_defs, wild_zlib_32), {
            'CMAKE_SHARED_LINKER_FLAGS_INIT="' .. wild_zlib_path_32 .. '"',
            'CMAKE_MODULE_LINKER_FLAGS_INIT="' .. wild_zlib_path_32 .. '"',
            'CMAKE_EXE_LINKER_FLAGS_INIT="' .. wild_zlib_path_32 .. '"',
        }))
    end
    if _OPTIONS["64-build"] then
        cmake_build('protobuf', false, merge_list(merge_list(proto_common_defs, wild_zlib_64), {
            'CMAKE_SHARED_LINKER_FLAGS_INIT="' .. wild_zlib_path_64 .. '"',
            'CMAKE_MODULE_LINKER_FLAGS_INIT="' .. wild_zlib_path_64 .. '"',
            'CMAKE_EXE_LINKER_FLAGS_INIT="' .. wild_zlib_path_64 .. '"',
        }))
    end
end
-- TODO COMPLETE THIS
if _OPTIONS["build-mbedtls"] or _OPTIONS["all-build"] then
    local mbedtls_common_defs = {
        "USE_STATIC_MBEDTLS_LIBRARY=ON",
        "USE_SHARED_MBEDTLS_LIBRARY=OFF",
        "MSVC_STATIC_RUNTIME=ON",
        "ENABLE_TESTING=OFF",
        "ENABLE_PROGRAMS=OFF",
        "LINK_WITH_PTHREAD=ON",
    }
    if _OPTIONS["32-build"] then
        local mbedtls_all_defs_32 = table.deepcopy(mbedtls_common_defs) -- we want a copy to avoid changing the original list
        if string.match(_ACTION, 'gmake.*') then
            table.insert(mbedtls_all_defs_32, 'CMAKE_C_FLAGS_INIT="-mpclmul -msse2 -maes"')
            table.insert(mbedtls_all_defs_32, 'CMAKE_CXX_FLAGS_INIT="-mpclmul -msse2 -maes"')
        end
        cmake_build('mbedtls', true, mbedtls_all_defs_32)
    end
    if _OPTIONS["64-build"] then
        cmake_build('mbedtls', false, mbedtls_common_defs)
    end
end
