<#
.SYNOPSIS
    Simple Build Wrapper for GBE Fork
.DESCRIPTION
    Lightweight wrapper around xmake for building the project.
    All dependency management and protobuf generation is now handled natively by xmake.
.PARAMETER Arch
    Target architecture (x86 or x64). Default: x64
.PARAMETER Mode
    Build mode (debug or release). Default: release
.PARAMETER Clean
    Clean before building
.PARAMETER CleanCache
    Clean xmake package cache (forces re-fetch of dependencies)
.PARAMETER Rebuild
    Force rebuild all targets
.PARAMETER Target
    Specific target to build (optional)
.EXAMPLE
    .\build.ps1
    .\build.ps1 -Arch x64 -Mode debug
    .\build.ps1 -Clean -Rebuild
    .\build.ps1 -CleanCache
    .\build.ps1 -Target api_experimental
#>

param(
    [ValidateSet("x86", "x64", "both")]
    [string]$Arch = "x64",
    
    [ValidateSet("debug", "release", "both")]
    [string]$Mode = "release",
    
    [switch]$Clean,
    [switch]$CleanCache,
    [switch]$Rebuild,
    [string]$Target = "",
    [switch]$Sign,
    [switch]$DosStub,
    [switch]$Resources,
    [switch]$Help
)

$ErrorActionPreference = "Stop"

# Display help if requested
if ($Help) {
    Write-Host ""
    Write-Host "GBE Fork Build System" -ForegroundColor Cyan
    Write-Host "=====================" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "The build script orchestrates the build process for multiple architectures and modes."
    Write-Host "By default, it builds all projects for the current configuration."
    Write-Host ""
    Write-Host "USAGE:" -ForegroundColor Yellow
    Write-Host "  .\build.ps1 [OPTIONS]"
    Write-Host ""
    Write-Host "OPTIONS (All optional):" -ForegroundColor Yellow
    Write-Host "  -Arch <x86|x64|both>   Target architecture (default: x64)"
    Write-Host "                         'both' will sequentially build for x86 and x64"
    Write-Host ""
    Write-Host "  -Mode <debug|release|both> Build mode (default: release)"
    Write-Host "                         'both' will sequentially build for debug and release"
    Write-Host ""
    Write-Host "  -Target <name>         Build a specific target instead of all (default: all)"
    Write-Host "                         See 'TARGETS' section below for available names"
    Write-Host ""
    Write-Host "  -Sign                  Enable fake certificate signing for binaries"
    Write-Host "  -DosStub               Enable DOS stub manipulation tool"
    Write-Host "  -Resources             Enable Windows resource compiler (embed version info)"
    Write-Host ""
    Write-Host "  -Clean                 Delete existing build artifacts before starting"
    Write-Host "  -CleanCache            Force re-fetch all external dependencies (xmake packages)"
    Write-Host "  -Rebuild               Force re-compilation of all source files"
    Write-Host "  -Help                  Display this help message"
    Write-Host ""
    Write-Host "EXAMPLES:" -ForegroundColor Yellow
    Write-Host "  .\build.ps1"
    Write-Host "    -> Build ALL targets for x64 in Release mode (The Standard Build)"
    Write-Host ""
    Write-Host "  .\build.ps1 -Mode both -Sign"
    Write-Host "    -> Build ALL targets for x64 in both Debug and Release, with signing"
    Write-Host ""
    Write-Host "  .\build.ps1 -Arch both -Mode both"
    Write-Host "    -> Build EVERYTHING: all targets, all architectures, all modes (Comprehensive Build)"
    Write-Host ""
    Write-Host "  .\build.ps1 -Target api_experimental -Mode debug -Arch x86"
    Write-Host "    -> Build ONLY 'api_experimental' for x86 in Debug mode"
    Write-Host ""
    Write-Host "  .\build.ps1 -Clean -Resources -DosStub"
    Write-Host "    -> Clean previous build then build x64 Release with resources and DOS stub manipulation"
    Write-Host ""
    Write-Host "TARGETS:" -ForegroundColor Yellow
    Write-Host "  - api_regular                    : Regular Steam API emulator"
    Write-Host "  - api_experimental               : Experimental Steam API with overlay support"
    Write-Host "  - steamclient_experimental       : Experimental steamclient DLL"
    Write-Host "  - tool_lobby_connect             : Lobby connection tool"
    Write-Host "  - tool_generate_interfaces       : Interface generation tool"
    Write-Host "  - lib_steamnetworkingsockets     : Steam networking sockets library"
    Write-Host "  - lib_game_overlay_renderer      : Game overlay renderer"
    Write-Host "  - steamclient_experimental_extra : Extra protection DLL"
    Write-Host ""
    exit 0
}

# Check if xmake is installed
if (-not (Get-Command "xmake" -ErrorAction SilentlyContinue)) {
    Write-Host "Error: xmake is not found in PATH" -ForegroundColor Red
    Write-Host "Please install xmake: https://xmake.io/#/guide/installation" -ForegroundColor Yellow
    
    if (Get-Command "winget" -ErrorAction SilentlyContinue) {
        $choice = Read-Host "Install xmake via winget? (Y/N)"
        if ($choice -eq 'Y' -or $choice -eq 'y') {
            Write-Host "Installing xmake..." -ForegroundColor Cyan
            winget install xmake
            Write-Host "Please restart your terminal and run this script again" -ForegroundColor Green
            exit 0
        }
    }
    
    exit 1
}

# Check for Ninja
$hasNinja = $false
if (Get-Command "ninja" -ErrorAction SilentlyContinue) {
    $hasNinja = $true
}

Write-Host "================================" -ForegroundColor Cyan
Write-Host "  GBE Fork Build System" -ForegroundColor Cyan
Write-Host "================================" -ForegroundColor Cyan
Write-Host "Architectures to build : $Arch" -ForegroundColor Yellow
Write-Host "Modes to build         : $Mode" -ForegroundColor Yellow
if ($Target) {
    Write-Host "Target filter          : $Target" -ForegroundColor Yellow
}
else {
    Write-Host "Target filter          : All Targets" -ForegroundColor Yellow
}
if ($hasNinja) {
    Write-Host "Build Ninja            : Found (Used for dependencies)" -ForegroundColor Green
}
else {
    Write-Host "Build Ninja            : Not Found (Optional)" -ForegroundColor Gray
}
Write-Host ""

# Clean package cache if requested
if ($CleanCache) {
    Write-Host "Cleaning xmake package cache..." -ForegroundColor Yellow
    Write-Host "This will force re-fetch of all git-based dependencies" -ForegroundColor Cyan
    xmake require --clean
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Cache clean failed" -ForegroundColor Red
        exit $LASTEXITCODE
    }
    Write-Host "Package cache cleaned" -ForegroundColor Green
    Write-Host ""
}

# Clean if requested
if ($Clean) {
    Write-Host "Cleaning build artifacts..." -ForegroundColor Yellow
    xmake clean -a
    if ($LASTEXITCODE -ne 0) {
        Write-Host "Clean failed" -ForegroundColor Red
        exit $LASTEXITCODE
    }
    Write-Host "Clean complete" -ForegroundColor Green
    Write-Host ""
}

# Prepare the list of architectures to build
$archsToBuild = @()
if ($Arch -eq "both") {
    $archsToBuild = @("x86", "x64")
}
else {
    $archsToBuild = @($Arch)
}

# Prepare the list of modes to build
$modesToBuild = @()
if ($Mode -eq "both") {
    $modesToBuild = @("debug", "release")
}
else {
    $modesToBuild = @($Mode)
}

$successPaths = @()

foreach ($currentArch in $archsToBuild) {
    foreach ($currentMode in $modesToBuild) {
        Write-Host "============================" -ForegroundColor Cyan
        Write-Host "  Building: $currentArch | $currentMode" -ForegroundColor Yellow
        Write-Host "============================" -ForegroundColor Cyan

        # Configure xmake
        Write-Host "Configuring build for $currentArch $currentMode..." -ForegroundColor Yellow
        $configArgs = @("f", "-p", "windows", "-a", "$currentArch", "-m", "$currentMode", "-y")
        if ($Rebuild) {
            $configArgs += "-c"
        }
        if ($Sign) {
            $configArgs += "--winsign=y"
        }
        if ($DosStub) {
            $configArgs += "--dosstub=y"
        }
        if ($Resources) {
            $configArgs += "--winrsrc=y"
        }

        xmake @configArgs
        if ($LASTEXITCODE -ne 0) {
            Write-Host "Configuration failed for $currentArch $currentMode" -ForegroundColor Red
            exit $LASTEXITCODE
        }

        Write-Host "Configuration for $currentArch $currentMode complete" -ForegroundColor Green
        Write-Host ""

        # Build
        Write-Host "Building $currentArch $currentMode..." -ForegroundColor Yellow
        $buildArgs = @("build")  # Start with build command

        if ($Rebuild) {
            $buildArgs += "-r"
        }

        if ($Target) {
            $buildArgs += $Target  # Build specific target
        }
        else {
            $buildArgs += "-a"  # Build all targets
        }

        & xmake @buildArgs
        if ($LASTEXITCODE -ne 0) {
            Write-Host "Build failed for $currentArch $currentMode" -ForegroundColor Red
            exit $LASTEXITCODE
        }
        
        $successPaths += ".build/$currentMode/windows/$currentArch"
    }
}

Write-Host ""
Write-Host "================================" -ForegroundColor Green
Write-Host "  Build Successful!" -ForegroundColor Green
Write-Host "================================" -ForegroundColor Green
Write-Host "Output directories:" -ForegroundColor Cyan
foreach ($path in $successPaths) {
    Write-Host "  - $path" -ForegroundColor Gray
}
Write-Host ""

