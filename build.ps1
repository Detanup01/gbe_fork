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
$buildArgs += "-a"  # Build all targets
if ($Target) {
    $buildArgs = @($Target)  # Override to build specific target
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
Write-Host "Output directory: build/win/$Mode/$Arch" -ForegroundColor Cyan

            }
            '3' {
                Show-BuildHeader
                Write-Host "Configuring xmake..." -ForegroundColor $ColorHeader
                if (Invoke-ExternalCommand "xmake" @("f", "-p", "$Global:Plat", "-a", "$Global:Arch", "-m", "$Global:Mode", "-c")) {
                    Write-Host "Configuration applied!" -ForegroundColor $ColorSuccess
                    Start-Sleep -Seconds 1
                }
                Wait-ForUser
            }
            '4' { return }
        }
    }
}

function Invoke-BuildProject {
Show-BuildHeader
Write-Host "Building Project ($Global:Mode / $Global:Arch)..." -ForegroundColor $ColorHeader
    
# Check dependencies
if (-not (Test-Path "third_party\libssq\lib\$Global:Mode\ssq.lib")) {
    Write-Host "Error: Dependencies not installed!" -ForegroundColor $ColorError
    Write-Host "Please run option 1 (Install/Build Dependencies) first" -ForegroundColor $ColorInfo
    Wait-ForUser
    return
}
    
    # Check if project is configured
    if (-not (Test-Path ".xmake\windows\x64\$Global:Mode\cache")) {
        Write-Host "Project not configured for $Global:Mode / $Global:Arch. Running configuration..." -ForegroundColor $ColorInfo
        Write-Host ""
        if (-not (Invoke-ExternalCommand "xmake" @("f", "-p", "$Global:Plat", "-a", "$Global:Arch", "-m", "$Global:Mode", "-c", "-y"))) {
            Write-Host "Configuration failed. Please check dependencies." -ForegroundColor $ColorError
            Wait-ForUser
            return
        }
        Write-Host ""
    }

    # Generate proto files if they don't exist
    if (-not (Test-Path "proto_gen\win\net.pb.h")) {
        Write-Host "Generating protobuf files..." -ForegroundColor $ColorInfo
        Write-Host ""
        
        # Find protoc
        $ProtocPath = Get-ChildItem "$env:LOCALAPPDATA\.xmake\packages\p\protobuf-cpp" -Recurse -Filter "protoc.exe" -ErrorAction SilentlyContinue | Select-Object -First 1
        
        if (-not $ProtocPath) {
            Write-Host "Error: protoc not found. Run 'Install/Build Dependencies' first" -ForegroundColor $ColorError
            Wait-ForUser
            return
        }
        
        New-Item -ItemType Directory -Force -Path "proto_gen\win" | Out-Null
        & $ProtocPath.FullName --proto_path="$PWD\proto" --cpp_out="$PWD\proto_gen\win" "$PWD\proto\net.proto"
        
        if ($LASTEXITCODE -ne 0) {
            Write-Host "Protobuf generation failed." -ForegroundColor $ColorError
            Wait-ForUser
            return
        }
        Write-Host "Protobuf files generated successfully." -ForegroundColor $ColorSuccess
        Write-Host ""
        
        
        # Reconfigure xmake to pick up new proto files
        Write-Host "Reconfiguring xmake..." -ForegroundColor $ColorInfo
        if (-not (Invoke-ExternalCommand "xmake" @("f", "-c", "-y"))) {
            Write-Host "Reconfiguration failed." -ForegroundColor $ColorError
            Wait-ForUser
            return
        }
        Write-Host ""
    }

    # Build all targets
    Write-Host "Building all targets..." -ForegroundColor $ColorInfo
    $buildSuccess = Invoke-ExternalCommand "xmake" @("build", "-a")
    
    Write-Host ""
    
    if ($buildSuccess) {
        Write-Host "Build Complete!" -ForegroundColor $ColorSuccess
    } else {
        Write-Host "Build failed. Check errors above." -ForegroundColor $ColorError
    }
    Wait-ForUser
}

function Invoke-RebuildProject {
    Show-BuildHeader
    Write-Host "Rebuilding Project..." -ForegroundColor $ColorHeader
    if (Invoke-ExternalCommand "xmake" @("-r")) {
        Write-Host "Rebuild Complete!" -ForegroundColor $ColorSuccess
    }
    Wait-ForUser
}

function Clear-BuildArtifacts {
    Show-BuildHeader
    Write-Host "Cleaning Build Artifacts..." -ForegroundColor $ColorHeader
    Invoke-ExternalCommand "xmake" @("c", "-a")
    Write-Host "Cleaned." -ForegroundColor $ColorSuccess
    Wait-ForUser
}

function Clear-Environment {
    Show-BuildHeader
    Write-Host "CLEAN ENVIRONMENT" -ForegroundColor $ColorError
    Write-Host "This will remove:" -ForegroundColor $ColorInfo
    Write-Host "  - Dependencies (third_party, build_deps)" -ForegroundColor $ColorInfo
    Write-Host "  - Build cache (.xmake)" -ForegroundColor $ColorInfo
    Write-Host "  - Generated files (proto_gen)" -ForegroundColor $ColorInfo
    Write-Host "  - Build outputs (build)" -ForegroundColor $ColorInfo
    Write-Host ""
    
    $confirm = Read-Host "Are you sure? (Y/N)"
    if ($confirm -ne 'Y' -and $confirm -ne 'y') {
        Write-Host "Cancelled." -ForegroundColor $ColorInfo
        Wait-ForUser
        return
    }
    
    Write-Host ""
    Write-Host "Removing directories..." -ForegroundColor $ColorHeader
    
    @(".xmake", "build_deps", "third_party", "proto_gen", "build") | ForEach-Object {
        if (Test-Path $_) {
            Write-Host "  Removing $_" -ForegroundColor $ColorInfo
            # Force remove even if it's a git repo
            if ($_ -eq "build_deps") {
                Remove-Item -Recurse -Force $_ -ErrorAction SilentlyContinue
                # Extra cleanup for git repos
                if (Test-Path "$_\.git") {
                    Remove-Item -Recurse -Force "$_\.git" -ErrorAction SilentlyContinue
                }
            } else {
                Remove-Item -Recurse -Force $_ -ErrorAction SilentlyContinue
            }
        }
    }
    
    Write-Host ""
    Write-Host "Environment cleaned successfully!" -ForegroundColor $ColorSuccess
    Wait-ForUser
}

# --- Main Loop ---

Test-XmakeInstalled

while ($true) {
    Show-BuildHeader
    Write-Host "MAIN MENU" -ForegroundColor $ColorHeader
    Write-Host "------------------" -ForegroundColor $ColorInfo
    Write-Host "1. Install/Build Dependencies" -ForegroundColor $ColorOption
    Write-Host "2. Generate Protobuf Source" -ForegroundColor $ColorOption
    Write-Host "3. Configure Build Settings" -ForegroundColor $ColorOption
    Write-Host "------------------" -ForegroundColor $ColorInfo
    Write-Host "4. Build All" -ForegroundColor $ColorSuccess
    Write-Host "5. Rebuild All" -ForegroundColor $ColorOption
    Write-Host "6. Clean" -ForegroundColor $ColorOption
    Write-Host "7. Clean Environment (Full Reset)" -ForegroundColor $ColorError
    Write-Host "------------------" -ForegroundColor $ColorInfo
    Write-Host "Q. Quit" -ForegroundColor $ColorError
    Write-Host ""
    
    $choice = Read-Host "Choose an option"
    
    switch ($choice) {
        '1' { Initialize-BuildDeps }
        '2' { Invoke-Protogen }
        '3' { Edit-BuildConfiguration }
        '4' { Invoke-BuildProject }
        '5' { Invoke-RebuildProject }
        '6' { Clear-BuildArtifacts }
        '7' { Clear-Environment }
        'Q' { exit }
        'q' { exit }
    }
}
