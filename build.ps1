<#
.SYNOPSIS
    Interactive Build Script for GBE Fork (TUI Style)
.DESCRIPTION
    Provides a menu-driven interface for building the project, managing dependencies,
    and configuring build options using xmake.
#>

$ErrorActionPreference = "Stop"

# --- Configuration ---
$Global:XmakePath = "xmake" 
$Global:Arch = "x64"
$Global:Mode = "release"
$Global:Plat = "windows"

# --- Colors ---
$ColorHeader = "Cyan"
$ColorOption = "Yellow"
$ColorSuccess = "Green"
$ColorError = "Red"
$ColorInfo = "Gray"

# --- Helper Functions ---

function Show-BuildHeader {
    Clear-Host
    Write-Host "============================================================" -ForegroundColor $ColorHeader
    Write-Host "                GBE FORK BUILD SYSTEM                       " -ForegroundColor $ColorHeader
    Write-Host "============================================================" -ForegroundColor $ColorHeader
    Write-Host ""
    Write-Host " Architecture: " -NoNewline
    Write-Host $Global:Arch -ForegroundColor $ColorSuccess -NoNewline
    Write-Host " | Mode: " -NoNewline
    Write-Host $Global:Mode -ForegroundColor $ColorSuccess
    Write-Host ""
}

function Test-XmakeInstalled {
    if (-not (Get-Command "xmake" -ErrorAction SilentlyContinue)) {
        Write-Host "Error: xmake is not found in PATH." -ForegroundColor $ColorError
        Write-Host "Please install xmake (checking winget...)" -ForegroundColor $ColorInfo
        if (Get-Command "winget" -ErrorAction SilentlyContinue) {
            $choice = Read-Host "Install xmake via winget? (Y/N)"
            if ($choice -eq 'Y' -or $choice -eq 'y') {
                winget install xmake
                Write-Host "Please restart the script after installation." -ForegroundColor $ColorHeader
                exit
            }
        }
        Write-Host "Aborting." -ForegroundColor $ColorError
        exit 1
    }
}

function Invoke-ExternalCommand {
    param($Cmd, [string[]]$Arguments)
    $ArgsStr = $Arguments -join " "
    Write-Host "> $Cmd $ArgsStr" -ForegroundColor $ColorInfo
    Write-Host ""
    & $Cmd $Arguments
    if ($LASTEXITCODE -ne 0) {
        Write-Host ""
        Write-Host "Command failed with exit code $LASTEXITCODE" -ForegroundColor $ColorError
        return $false
    }
    return $true
}

function Wait-ForUser {
    Write-Host ""
    Read-Host "Press Enter to continue..."
}

# --- Actions ---

function Initialize-BuildDeps {
    Show-BuildHeader
    Write-Host "Installing Dependencies..." -ForegroundColor $ColorHeader
    Write-Host "Running custom dependency setup..." -ForegroundColor $ColorInfo
    Write-Host ""
    
    # Run custom dependency script
    & "$PSScriptRoot\setup_deps.ps1" -Arch $Global:Arch -Mode $Global:Mode
    
    # Run xmake package installation (will prompt if not using -y)
    Write-Host ""
    Write-Host "Installing xmake packages..." -ForegroundColor $ColorInfo
    if (Invoke-ExternalCommand "xmake" @("f", "-p", "$Global:Plat", "-a", "$Global:Arch", "-m", "$Global:Mode", "-c", "-y")) {
        Write-Host "Dependencies installed successfully!" -ForegroundColor $ColorSuccess
    } else {
        Write-Host "Note: Some dependencies may have failed. Check output above." -ForegroundColor $ColorError
    }
    Wait-ForUser
}

function Invoke-Protogen {
    Show-BuildHeader
    Write-Host "Generating Protobuf Files..." -ForegroundColor $ColorHeader
    
    # Find protoc from xmake packages
    $ProtocPath = Get-ChildItem "$env:LOCALAPPDATA\.xmake\packages\p\protobuf-cpp" -Recurse -Filter "protoc.exe" -ErrorAction SilentlyContinue | Select-Object -First 1
    
    if (-not $ProtocPath) {
        Write-Host "Error: protoc not found in xmake packages" -ForegroundColor $ColorError
        Write-Host "Please run 'Install/Build Dependencies' first" -ForegroundColor $ColorInfo
        Wait-ForUser
        return
    }
    
    Write-Host "Using protoc: $($ProtocPath.FullName)" -ForegroundColor $ColorInfo
    
    # Create output directory
    $ProtoOutDir = "proto_gen\win"
    New-Item -ItemType Directory -Force -Path $ProtoOutDir | Out-Null
    
    # Generate proto files
    Get-ChildItem proto\*.proto | ForEach-Object {
        Write-Host "Generating: $($_.Name)" -ForegroundColor $ColorInfo
        & $ProtocPath.FullName --proto_path="$PWD\proto" --cpp_out="$PWD\$ProtoOutDir" "$($_.FullName)"
        if ($LASTEXITCODE -ne 0) {
            Write-Host "Failed to generate proto for $($_.Name)" -ForegroundColor $ColorError
            Wait-ForUser
            return
        }
    }
    
    Write-Host "Protobufs generated successfully!" -ForegroundColor $ColorSuccess
    Wait-ForUser
}

function Edit-BuildConfiguration {
    while ($true) {
        Show-BuildHeader
        Write-Host "CONFIGURATION MENU" -ForegroundColor $ColorHeader
        Write-Host "------------------" -ForegroundColor $ColorInfo
        Write-Host "1. Toggle Architecture (Current: $($Global:Arch))" -ForegroundColor $ColorOption
        Write-Host "2. Toggle Mode (Current: $($Global:Mode))" -ForegroundColor $ColorOption
        Write-Host "3. Apply Configuration (Run xmake f)" -ForegroundColor $ColorSuccess
        Write-Host "4. Back to Main Menu" -ForegroundColor $ColorInfo
        Write-Host ""
        
        $choice = Read-Host "Choose an option"
        
        switch ($choice) {
            '1' { 
                if ($Global:Arch -eq "x64") { $Global:Arch = "x86" } else { $Global:Arch = "x64" }
            }
            '2' {
                if ($Global:Mode -eq "release") { $Global:Mode = "debug" } else { $Global:Mode = "release" }
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
