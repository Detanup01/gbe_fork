<#
.SYNOPSIS
    Downloads and builds third-party dependencies for GBE Fork
.DESCRIPTION
    Handles libssq and ingame_overlay downloads/builds that xmake has trouble with
#>

param(
    [string]$Arch = "x64",
    [string]$Mode = "release"
)

$ErrorActionPreference = "Stop"

$ThirdPartyDir = "third_party"
$BuildDir = "build_deps"

function Write-Status {
    param([string]$Message, [string]$Color = "Cyan")
    Write-Host "[DEPS] $Message" -ForegroundColor $Color
}

function Download-File {
    param([string]$Url, [string]$Output)
    Write-Status "Downloading: $Url"
    curl.exe -fSL -o $Output $Url
    if ($LASTEXITCODE -ne 0) {
        throw "Download failed: $Url"
    }
}

function Expand-ZipArchive {
    param([string]$ZipPath, [string]$Destination)
    Write-Status "Extracting: $ZipPath"
    Expand-Archive -Path $ZipPath -DestinationPath $Destination -Force
}

# Create directories
New-Item -ItemType Directory -Force -Path $ThirdPartyDir | Out-Null
New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null

# ============================================================================
# Build libssq
# ============================================================================
Write-Status "=== Building libssq ===" "Green"

$LibssqZip = "$BuildDir\libssq-main.zip"
$LibssqSrc = "$BuildDir\libssq-main"
$LibssqInstall = "$ThirdPartyDir\libssq"

if (-not (Test-Path "$LibssqInstall\lib\$Mode\ssq.lib")) {
    # Download
    if (-not (Test-Path $LibssqZip)) {
        Download-File "https://github.com/BinaryAlien/libssq/archive/refs/heads/main.zip" $LibssqZip
    }
    
    # Extract
    Expand-ZipArchive $LibssqZip $BuildDir
    
    # Fix MSVC error in error.c
    $ErrorFile = "$LibssqSrc\src\error.c"
    if (Test-Path $ErrorFile) {
        Write-Status "Patching error.c for MSVC compatibility"
        $Content = Get-Content $ErrorFile -Raw
        $Content = $Content -replace 'NULL,\s+\);', "NULL`n    );"
        Set-Content $ErrorFile $Content -NoNewline
    }
    
    # Build with CMake
    $LibssqBuild = "$LibssqSrc\build"
    New-Item -ItemType Directory -Force -Path $LibssqBuild | Out-Null
    
    Push-Location $LibssqBuild
    Write-Status "Configuring libssq with CMake"
    cmake .. -A $Arch -DCMAKE_BUILD_TYPE=$Mode -DCMAKE_INSTALL_PREFIX="$PSScriptRoot\$LibssqInstall"
    
    Write-Status "Building libssq"
    cmake --build . --config $Mode
    
    Write-Status "Installing libssq"
    cmake --install . --config $Mode
    
    # Copy lib to mode-specific subdirectory for xmake
    $LibFile = "$PSScriptRoot\$LibssqInstall\lib\ssq.lib"
    $ModeDir = "$PSScriptRoot\$LibssqInstall\lib\$Mode"
    New-Item -ItemType Directory -Force -Path $ModeDir | Out-Null
    if (Test-Path $LibFile) {
        Copy-Item $LibFile $ModeDir -Force
        Write-Status "Copied ssq.lib to $Mode subdirectory"
    }
    
    Pop-Location
    
    Write-Status "libssq built successfully" "Green"
} else {
    Write-Status "libssq already built, skipping" "Yellow"
}

# ============================================================================
# Build ingame_overlay (Rustbeard86 fork)
# ============================================================================
Write-Status "=== Building ingame_overlay ===" "Green"

$OverlaySrc = "$BuildDir\ingame_overlay"
$OverlayInstall = "$ThirdPartyDir\ingame_overlay"

# Build ingame_overlay if not already built
if (-not (Test-Path "$OverlayInstall\lib\$Mode\ingame_overlay.lib")) {
    # Clone from Rustbeard86 fork with submodules
    if (-not (Test-Path $OverlaySrc)) {
        Write-Status "Cloning ingame_overlay with submodules..."
        git clone --recurse-submodules https://github.com/Rustbeard86/ingame_overlay.git "$OverlaySrc"
        
        if ($LASTEXITCODE -ne 0) {
            Write-Host "[ERROR] Git clone failed for ingame_overlay" -ForegroundColor Red
            exit 1
        }
    }
    
    # Build with CMake
    Push-Location $OverlaySrc
    
    Write-Status "Configuring ingame_overlay"
    if ($Arch -eq "x64") {
        cmake -B build -G "Visual Studio 18 2026" -A x64 `
            -DCMAKE_BUILD_TYPE=$Mode `
            -DCMAKE_INSTALL_PREFIX="$PSScriptRoot\$OverlayInstall" `
            -DBUILD_SHARED_LIBS=OFF `
            -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded"
    } else {
        cmake -B build -G "Visual Studio 18 2026" -A Win32 `
            -DCMAKE_BUILD_TYPE=$Mode `
            -DCMAKE_INSTALL_PREFIX="$PSScriptRoot\$OverlayInstall" `
            -DBUILD_SHARED_LIBS=OFF `
            -DCMAKE_MSVC_RUNTIME_LIBRARY="MultiThreaded"
    }
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "[ERROR] CMake configure failed for ingame_overlay" -ForegroundColor Red
        Pop-Location
        exit 1
    }
    
    Write-Status "Building ingame_overlay"
    cmake --build build --config $Mode
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "[ERROR] Build failed for ingame_overlay" -ForegroundColor Red
        Pop-Location
        exit 1
    }
    
    Write-Status "Installing ingame_overlay"
    cmake --install build --config $Mode
    
    # Copy lib to mode-specific subdirectory for xmake
    $LibFile = "$PSScriptRoot\$OverlayInstall\lib\ingame_overlay.lib"
    $ModeDir = "$PSScriptRoot\$OverlayInstall\lib\$Mode"
    New-Item -ItemType Directory -Force -Path $ModeDir | Out-Null
    if (Test-Path $LibFile) {
        Copy-Item $LibFile $ModeDir -Force
        Write-Status "Copied ingame_overlay.lib to $Mode subdirectory"
    }
    
    Pop-Location
    
    Write-Status "ingame_overlay built successfully" "Green"
} else {
    Write-Status "ingame_overlay already built, skipping" "Yellow"
}

# Copy minhook and system libraries (always check, even if main lib exists)
$ModeDir = "$PSScriptRoot\$OverlayInstall\lib\$Mode"
$MinhookLib = "$OverlaySrc\build\deps\minhook\$Mode\minhook.x64.lib"
$SystemLib = "$OverlaySrc\build\deps\System\$Mode\system.lib"

if ((Test-Path $OverlaySrc) -and (Test-Path "$OverlaySrc\build")) {
    New-Item -ItemType Directory -Force -Path $ModeDir | Out-Null
    
    if (Test-Path $MinhookLib) {
        Copy-Item $MinhookLib $ModeDir -Force
        Write-Status "Copied minhook.x64.lib to $Mode subdirectory"
    } else {
        Write-Status "Warning: minhook.x64.lib not found at $MinhookLib" "Yellow"
    }
    
    if (Test-Path $SystemLib) {
        Copy-Item $SystemLib $ModeDir -Force
        Write-Status "Copied system.lib to $Mode subdirectory"
    } else {
        Write-Status "Warning: system.lib not found at $SystemLib" "Yellow"
    }
}

Write-Status "=== Dependency setup complete ===" "Green"
