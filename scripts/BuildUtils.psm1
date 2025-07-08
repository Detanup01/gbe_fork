# GBE Fork Build Utilities Module
# Advanced PowerShell functions for build system

function Get-BuildEnvironment {
    <#
    .SYNOPSIS
    Validates and returns build environment information
    #>
    
    $env = @{
        PremakeExists = Test-Path "third-party\common\win\premake\premake5.exe"
        VSWhereExists = Test-Path "third-party\common\win\vswhere\vswhere.exe"
        MSBuildPath = $null
        MaxThreads = 2
        IsValid = $false
    }
    
    # Calculate max threads
    if ($env:NUMBER_OF_PROCESSORS) {
        $env.MaxThreads = [Math]::Max(1, [int]($env:NUMBER_OF_PROCESSORS * 0.7))
    }
    
    # Find MSBuild
    if ($env.VSWhereExists) {
        try {
            $vsPath = & "third-party\common\win\vswhere\vswhere.exe" -prerelease -latest -nocolor -nologo -property installationPath 2>$null
            if ($vsPath) {
                $msbuildPath = Join-Path $vsPath "MSBuild\Current\Bin\MSBuild.exe"
                if (Test-Path $msbuildPath) {
                    $env.MSBuildPath = $msbuildPath
                }
            }
        }
        catch {
            Write-Warning "Failed to locate MSBuild: $_"
        }
    }
    
    $env.IsValid = $env.PremakeExists -and $env.VSWhereExists -and $env.MSBuildPath
    
    return $env
}

function Get-BuildTargets {
    <#
    .SYNOPSIS
    Returns the complete list of available build targets with metadata
    #>
    
    return @(
        @{ Name = "api_regular"; Category = "API"; Description = "Steam API Regular"; Essential = $true },
        @{ Name = "api_experimental"; Category = "API"; Description = "Steam API Experimental"; Essential = $false },
        @{ Name = "steamclient_experimental"; Category = "Client"; Description = "SteamClient Experimental"; Essential = $false },
        @{ Name = "steamclient_hybrid"; Category = "Client"; Description = "SteamClient Hybrid"; Essential = $true },
        @{ Name = "steamclient_experimental_stub"; Category = "Client"; Description = "SteamClient Stub"; Essential = $false },
        @{ Name = "steamclient_experimental_extra"; Category = "Client"; Description = "SteamClient Extra"; Essential = $false },
        @{ Name = "steamclient_experimental_loader"; Category = "Client"; Description = "SteamClient Loader"; Essential = $false },
        @{ Name = "tool_lobby_connect"; Category = "Tools"; Description = "Lobby Connect Tool"; Essential = $false },
        @{ Name = "tool_generate_interfaces"; Category = "Tools"; Description = "Generate Interfaces Tool"; Essential = $true },
        @{ Name = "tool_file_dos_stub_changer"; Category = "Tools"; Description = "DOS Stub Changer"; Essential = $false },
        @{ Name = "lib_steamnetworkingsockets"; Category = "Libraries"; Description = "Steam Networking Sockets"; Essential = $false },
        @{ Name = "lib_game_overlay_renderer"; Category = "Libraries"; Description = "Game Overlay Renderer"; Essential = $false },
        @{ Name = "test_crash_printer"; Category = "Testing"; Description = "Crash Printer Test"; Essential = $false }
    )
}

function Invoke-BuildProcess {
    <#
    .SYNOPSIS
    Executes the build process with given configuration
    .PARAMETER ConfigFile
    Path to JSON configuration file
    #>
    param(
        [Parameter(Mandatory)]
        [string]$ConfigFile
    )
    
    if (-not (Test-Path $ConfigFile)) {
        throw "Configuration file not found: $ConfigFile"
    }
    
    try {
        $config = Get-Content $ConfigFile | ConvertFrom-Json
        
        Write-Host "=" * 50
        Write-Host " Build Process Starting"
        Write-Host "=" * 50
        Write-Host "Configurations: $($config.configs -join ', ')"
        Write-Host "Platforms:      $($config.platforms -join ', ')"  
        Write-Host "Targets:        $($config.targets -join ', ')"
        Write-Host "Timestamp:      $($config.timestamp)"
        Write-Host "=" * 50
        
        $env = Get-BuildEnvironment
        if (-not $env.IsValid) {
            throw "Build environment is not properly configured"
        }
        
        # Generate solution
        Write-Host "Generating Visual Studio solution..."
        $premakeArgs = @("--file=premake5.lua", "--genproto", "--dosstub", "--winrsrc", "--winsign", "--os=windows", "vs2022")
        & "third-party\common\win\premake\premake5.exe" @premakeArgs
        
        if ($LASTEXITCODE -ne 0) {
            throw "Failed to generate solution files"
        }
        
        $solutionFile = "build\project\vs2022\win\gbe.sln"
        if (-not (Test-Path $solutionFile)) {
            throw "Solution file was not created: $solutionFile"
        }
        
        # Calculate total builds
        $totalBuilds = $config.configs.Count * $config.platforms.Count * $config.targets.Count
        $currentBuild = 0
        
        Write-Host "Total builds to process: $totalBuilds"
        Write-Host ""
        
        # Execute builds
        foreach ($buildConfig in $config.configs) {
            foreach ($platform in $config.platforms) {
                foreach ($target in $config.targets) {
                    $currentBuild++
                    
                    Write-Host ""
                    Write-Host "[$currentBuild/$totalBuilds] Building $target $buildConfig $platform"
                    Write-Host ("-" * 50)
                    
                    $msbuildArgs = @(
                        "/nologo",
                        "-m:$($env.MaxThreads)",
                        "-v:n",
                        "/p:Configuration=$buildConfig,Platform=$platform",
                        "/target:$target",
                        $solutionFile
                    )
                    
                    & $env.MSBuildPath @msbuildArgs
                    
                    if ($LASTEXITCODE -ne 0) {
                        Write-Warning "Build failed for $target $buildConfig $platform"
                        Write-Host "Continuing with remaining builds..."
                    }
                    else {
                        Write-Host "✓ Successfully built $target $buildConfig $platform" -ForegroundColor Green
                    }
                }
            }
        }
        
        Write-Host ""
        Write-Host "=" * 50
        Write-Host " Build Process Complete"
        Write-Host "=" * 50
        Write-Host "Total builds attempted: $totalBuilds"
        Write-Host ""
        Write-Host "Build artifacts can be found in:"
        Write-Host "- build\win\vs2022\debug\   (for debug builds)"
        Write-Host "- build\win\vs2022\release\ (for release builds)"
        Write-Host "=" * 50
        
        return $true
    }
    catch {
        Write-Error "Build process failed: $_"
        return $false
    }
}

function New-BuildConfiguration {
    <#
    .SYNOPSIS
    Creates a new build configuration interactively
    #>
    param(
        [string[]]$Configs = @("release"),
        [string[]]$Platforms = @("x64"),
        [string[]]$Targets = @("steamclient_hybrid")
    )
    
    $config = @{
        configs = $Configs
        platforms = $Platforms
        targets = $Targets
        timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    }
    
    return $config
}

function Test-BuildDependencies {
    <#
    .SYNOPSIS
    Tests if all build dependencies are available
    #>
    
    $env = Get-BuildEnvironment
    
    Write-Host "Build Environment Check:"
    Write-Host "------------------------"
    Write-Host "Premake5:    $(if ($env.PremakeExists) { '✓ Found' } else { '✗ Missing' })"
    Write-Host "VSWhere:     $(if ($env.VSWhereExists) { '✓ Found' } else { '✗ Missing' })"
    Write-Host "MSBuild:     $(if ($env.MSBuildPath) { "✓ Found at $($env.MSBuildPath)" } else { '✗ Missing' })"
    Write-Host "Max Threads: $($env.MaxThreads)"
    Write-Host "Overall:     $(if ($env.IsValid) { '✓ Ready' } else { '✗ Not Ready' })"
    
    return $env.IsValid
}

# Export functions
Export-ModuleMember -Function Get-BuildEnvironment, Get-BuildTargets, Invoke-BuildProcess, New-BuildConfiguration, Test-BuildDependencies