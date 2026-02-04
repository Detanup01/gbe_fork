<#
.SYNOPSIS
    Comprehensive Build Orchestrator for GBE Fork
.DESCRIPTION
    A powerful wrapper around xmake that manages multi-architecture and multi-mode builds.
    Handles configuration, cleaning, signing, and post-build artifact manipulation.
.PARAMETER Arch
    Target architecture: 'x86', 'x64', or 'both' (default: x64).
.PARAMETER Mode
    Build configuration: 'debug', 'release', or 'both' (default: release).
.PARAMETER Target
    Specific target name to build (optional). If omitted, builds all targets.
.PARAMETER Sign
    Switch: Enables fake certificate signing via sign_helper.bat.
.PARAMETER DosStub
    Switch: Enables DOS stub manipulation for Windows binaries.
.PARAMETER Resources
    Switch: Enables compilation and linking of Windows resources (.rc/.res).
.PARAMETER All
    Switch: Meta-switch that enables -Arch both, -Mode both, -Sign, -DosStub, and -Resources.
.PARAMETER Clean
    Switch: Deletes build artifacts before starting.
.PARAMETER CleanCache
    Switch: Forces re-fetch of all external xmake dependencies.
.PARAMETER Rebuild
    Switch: Forces full re-compilation of all source files.
.EXAMPLE
    .\build.ps1
    Build ALL targets for x64 in Release mode.
.EXAMPLE
    .\build.ps1 -All
    Build EVERYTHING: both architectures, both modes, with all post-build steps enabled.
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
    [switch]$All,
    [switch]$Help
)

$ErrorActionPreference = "Stop"

# Detect Platform
$Plat = "windows"
if ($IsLinux -or $PSVersionTable.OS -like "*Linux*") {
    $Plat = "linux"
}

# Handle Meta-switch logic
if ($All) {
    $Arch = "both"
    $Mode = "both"
    $Sign = $true
    $DosStub = $true
    $Resources = $true
}

# Display help if requested
if ($Help) {
    Write-Host ""
    Write-Host "GBE Fork Build System" -ForegroundColor Cyan
    Write-Host "=====================" -ForegroundColor Cyan
    Write-Host "Detected Platform: $Plat" -ForegroundColor Gray
    Write-Host ""
    Write-Host "SYNTAX:" -ForegroundColor Yellow
    Write-Host "  .\build.ps1 [-Arch <x86|x64|both>] [-Mode <debug|release|both>] [-Target <name>]"
    Write-Host "              [-Sign] [-DosStub] [-Resources] [-All] [-Clean] [-CleanCache] [-Rebuild] [-Help]"
    Write-Host ""
    Write-Host "OPTIONS:" -ForegroundColor Yellow
    Write-Host "  -Arch        Target architecture selection (Default: x64)"
    Write-Host "  -Mode        Build configuration selection (Default: release)"
    Write-Host "  -Target      Specific project to build (Default: all targets)"
    Write-Host "  -Sign        [Win] Enable fake certificate signing"
    Write-Host "  -DosStub     [Win] Enable DOS stub manipulation"
    Write-Host "  -Resources   [Win] Enable compilation of Windows resources"
    Write-Host "  -All         Complete Build: builds both archs/modes with all polish steps"
    Write-Host "  -Clean       Wipe build artifacts before starting"
    Write-Host "  -Rebuild     Force full re-compilation"
    Write-Host "  -Help        Display this message"
    Write-Host ""
    Write-Host "EXAMPLES:" -ForegroundColor Yellow
    Write-Host "  .\build.ps1                      Standard Release build"
    Write-Host "  .\build.ps1 -All                 Comprehensive build (All Archs/Modes/Steps)"
    Write-Host "  .\build.ps1 -Target api_experimental -Sign [Win] Build experimental API and sign it"
    Write-Host ""
    Write-Host "TARGETS:" -ForegroundColor Yellow
    Write-Host "  - api_regular, api_experimental, steamclient_experimental, tool_lobby_connect,"
    Write-Host "    tool_generate_interfaces, lib_steamnetworkingsockets, lib_game_overlay_renderer,"
    Write-Host "    steamclient_experimental_extra"
    Write-Host ""
    exit 0
}

# Check if xmake is installed
if (-not (Get-Command "xmake" -ErrorAction SilentlyContinue)) {
    Write-Host "Error: xmake is not found in PATH" -ForegroundColor Red
    Write-Host "Please install xmake: https://xmake.io/#/guide/installation" -ForegroundColor Yellow
    
    if ($Plat -eq "windows" -and (Get-Command "winget" -ErrorAction SilentlyContinue)) {
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
Write-Host "Platform detection     : $Plat" -ForegroundColor Green
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
$archsToBuild = if ($Arch -eq "both") { @("x86", "x64") } else { @($Arch) }

# Prepare the list of modes to build
$modesToBuild = if ($Mode -eq "both") { @("debug", "release") } else { @($Mode) }

$successPaths = @()

foreach ($currentArch in $archsToBuild) {
    foreach ($currentMode in $modesToBuild) {
        Write-Host "============================" -ForegroundColor Cyan
        Write-Host "  Building: $Plat | $currentArch | $currentMode" -ForegroundColor Yellow
        Write-Host "============================" -ForegroundColor Cyan

        # Configure xmake
        Write-Host "Configuring build for $Plat $currentArch $currentMode..." -ForegroundColor Yellow
        $configArgs = @("f", "-p", "$Plat", "-a", "$currentArch", "-m", "$currentMode", "-y")
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
            Write-Host "Configuration failed for $Plat $currentArch $currentMode" -ForegroundColor Red
            exit $LASTEXITCODE
        }

        Write-Host "Configuration for $Plat $currentArch $currentMode complete" -ForegroundColor Green
        Write-Host ""

        # Build
        Write-Host "Building $Plat $currentArch $currentMode..." -ForegroundColor Yellow
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
            Write-Host "Build failed for $Plat $currentArch $currentMode" -ForegroundColor Red
            exit $LASTEXITCODE
        }
        
        $successPaths += ".build/$currentMode/$Plat/$currentArch"
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

