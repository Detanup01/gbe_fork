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
    trigger = "b-all",
    description = "Build all deps",
}

newoption {
    category = "build",
    trigger = "b-ssq",
    description = "Build ssq",
}
newoption {
    category = "build",
    trigger = "b-zlib",
    description = "Build zlib",
}
newoption {
    category = "build",
    trigger = "b-curl",
    description = "Build curl",
}
newoption {
    category = "build",
    trigger = "b-protobuf",
    description = "Build protobuf",
}
newoption {
    category = "build",
    trigger = "b-mbedtls",
    description = "Build mbedtls",
}
newoption {
    category = "build",
    trigger = "b-ingame_overlay",
    description = "Build ingame_overlay",
}




-- common defs
---------

local deps_dir = os.realpath('build/deps/' .. os_iden)
local third_party_dir = os.realpath('third-party')
local third_party_deps_dir = os.realpath(third_party_dir .. '/deps/' .. os_iden)
local third_party_common_dir = os.realpath(third_party_dir .. '/deps/common')
local extractor = os.realpath(third_party_deps_dir .. '/7za/7za')
local mycmake = os.realpath(third_party_deps_dir .. 'cmake/bin/cmake')

if _OPTIONS["custom-cmake"] then
    mycmake = _OPTIONS["custom-cmake"]
    print('using custom cmake: ' .. _OPTIONS["custom-cmake"])
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
