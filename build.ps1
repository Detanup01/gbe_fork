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
.PARAMETER Rebuild
    Force rebuild all targets
.PARAMETER Target
    Specific target to build (optional)
.EXAMPLE
    .\build.ps1
    .\build.ps1 -Arch x64 -Mode debug
    .\build.ps1 -Clean -Rebuild
    .\build.ps1 -Target api_experimental
#>

param(
    [ValidateSet("x86", "x64")]
    [string]$Arch = "x64",
    
    [ValidateSet("debug", "release")]
    [string]$Mode = "release",
    
    [switch]$Clean,
    [switch]$Rebuild,
    [string]$Target = ""
)

$ErrorActionPreference = "Stop"

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

Write-Host "================================" -ForegroundColor Cyan
Write-Host "  GBE Fork Build System" -ForegroundColor Cyan
Write-Host "================================" -ForegroundColor Cyan
Write-Host "Architecture: $Arch" -ForegroundColor Yellow
Write-Host "Mode: $Mode" -ForegroundColor Yellow
Write-Host ""

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
$buildArgs = @()
if ($Rebuild) {
    $buildArgs += "-r"
}

if ($Target) {
    $buildArgs += $Target  # Build specific target
} else {
    $buildArgs += "-a"  # Build all targets
}

xmake @buildArgs
if ($LASTEXITCODE -ne 0) {
    Write-Host "Build failed" -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Host ""
Write-Host "================================" -ForegroundColor Green
Write-Host "  Build Successful!" -ForegroundColor Green
Write-Host "================================" -ForegroundColor Green
Write-Host "Output directory: .build/$Mode/windows/$Arch" -ForegroundColor Cyan

