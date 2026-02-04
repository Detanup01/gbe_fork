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
    [ValidateSet("x86", "x64")]
    [string]$Arch = "x64",
    
    [ValidateSet("debug", "release")]
    [string]$Mode = "release",
    
    [switch]$Clean,
    [switch]$CleanCache,
    [switch]$Rebuild,
    [string]$Target = "",
    [switch]$Help
)

$ErrorActionPreference = "Stop"

# Display help if requested
if ($Help) {
    Write-Host ""
    Write-Host "GBE Fork Build System" -ForegroundColor Cyan
    Write-Host "=====================" -ForegroundColor Cyan
    Write-Host ""
    Write-Host "USAGE:" -ForegroundColor Yellow
    Write-Host "  .\build.ps1 [OPTIONS]"
    Write-Host ""
    Write-Host "OPTIONS:" -ForegroundColor Yellow
    Write-Host "  -Arch <x86|x64>        Target architecture (default: x64)"
    Write-Host "  -Mode <debug|release>  Build mode (default: release)"
    Write-Host "  -Clean                 Clean build artifacts before building"
    Write-Host "  -CleanCache            Clean xmake package cache (forces re-fetch of dependencies)"
    Write-Host "  -Rebuild               Force rebuild all targets"
    Write-Host "  -Target <name>         Build specific target (default: all targets)"
    Write-Host "  -Help                  Display this help message"
    Write-Host ""
    Write-Host "EXAMPLES:" -ForegroundColor Yellow
    Write-Host "  .\build.ps1"
    Write-Host "    Build all targets in release mode for x64"
    Write-Host ""
    Write-Host "  .\build.ps1 -Arch x86 -Mode debug"
    Write-Host "    Build all targets in debug mode for x86"
    Write-Host ""
    Write-Host "  .\build.ps1 -Clean -Rebuild"
    Write-Host "    Clean and rebuild all targets"
    Write-Host ""
    Write-Host "  .\build.ps1 -CleanCache -Rebuild"
    Write-Host "    Clear dependency cache and rebuild (useful after updating git dependencies)"
    Write-Host ""
    Write-Host "  .\build.ps1 -Target api_experimental"
    Write-Host "    Build only the api_experimental target"
    Write-Host ""
    Write-Host "TARGETS:" -ForegroundColor Yellow
    Write-Host "  api_regular                    - Regular Steam API emulator"
    Write-Host "  api_experimental               - Experimental Steam API with overlay support"
    Write-Host "  steamclient_experimental       - Experimental steamclient DLL"
    Write-Host "  tool_lobby_connect             - Lobby connection tool"
    Write-Host "  tool_generate_interfaces       - Interface generation tool"
    Write-Host "  lib_steamnetworkingsockets     - Steam networking sockets library"
    Write-Host "  lib_game_overlay_renderer      - Game overlay renderer"
    Write-Host "  steamclient_experimental_extra - Extra protection DLL"
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
Write-Host "Architecture: $Arch" -ForegroundColor Yellow
Write-Host "Mode: $Mode" -ForegroundColor Yellow
if ($hasNinja) {
    Write-Host "Build Ninja:  Found (Used for dependencies)" -ForegroundColor Green
}
else {
    Write-Host "Build Ninja:  Not Found (Optional)" -ForegroundColor Gray
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

# Configure xmake
Write-Host "Configuring build..." -ForegroundColor Yellow
$configArgs = @("f", "-p", "windows", "-a", "$Arch", "-m", "$Mode", "-y")
if ($Rebuild) {
    $configArgs += "-c"
}

xmake @configArgs
if ($LASTEXITCODE -ne 0) {
    Write-Host "Configuration failed" -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host "Configuration complete" -ForegroundColor Green
Write-Host ""

# Build
Write-Host "Building..." -ForegroundColor Yellow
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
    Write-Host "Build failed" -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host ""
Write-Host "================================" -ForegroundColor Green
Write-Host "  Build Successful!" -ForegroundColor Green
Write-Host "================================" -ForegroundColor Green
Write-Host "Output directory: .build/$Mode/windows/$Arch" -ForegroundColor Cyan

