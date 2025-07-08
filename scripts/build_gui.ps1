# GBE Fork Interactive Builder GUI
# Professional PowerShell GUI for build target selection

Add-Type -AssemblyName System.Windows.Forms
Add-Type -AssemblyName System.Drawing

# Define all available build targets based on actual GBE Fork structure
$global:BuildTargets = @(
    @{ Name = "api_regular"; Description = "Regular Steam API (steam_api.dll + steam_api64.dll)"; Default = $false; Category = "Complete Packages" },
    @{ Name = "api_experimental"; Description = "Experimental Steam API (steam_api.dll + steam_api64.dll)"; Default = $true; Category = "Experimental Package" },
    @{ Name = "steamclient_experimental_stub"; Description = "Experimental SteamClient (steamclient.dll + steamclient64.dll)"; Default = $true; Category = "Experimental Package" },
    @{ Name = "steamclient_hybrid"; Description = "Hybrid SteamClient (steamclient.dll + steamclient64.dll)"; Default = $false; Category = "Complete Packages" },
    @{ Name = "steamclient_experimental"; Description = "SteamClient Loader (Complete package: DLLs + Protection + Loaders)"; Default = $false; Category = "Advanced SteamClient" },
    @{ Name = "tool_generate_interfaces"; Description = "Generate Interfaces Tool"; Default = $false; Category = "Tools" },
    @{ Name = "tool_lobby_connect"; Description = "Lobby Connect Tool"; Default = $false; Category = "Tools" },
    @{ Name = "tool_file_dos_stub_changer"; Description = "DOS Stub Changer Tool"; Default = $false; Category = "Tools" },
    @{ Name = "lib_steamnetworkingsockets"; Description = "Steam Networking Sockets Library"; Default = $false; Category = "Libraries" },
    @{ Name = "lib_game_overlay_renderer"; Description = "Game Overlay Renderer Library"; Default = $false; Category = "Libraries" },
    @{ Name = "test_crash_printer"; Description = "Crash Printer Test"; Default = $false; Category = "Testing" },
    @{ Name = "package_docs"; Description = "Package Documentation (CHANGELOG, CREDITS, README, steam_settings.EXAMPLE)"; Default = $false; Category = "Documentation" }
)

# Create main form
$form = New-Object System.Windows.Forms.Form
$form.Text = "GBE Fork Interactive Builder v1.0"
$form.Size = New-Object System.Drawing.Size(750, 700)
$form.StartPosition = "CenterScreen"
$form.MaximizeBox = $false
$form.FormBorderStyle = "FixedDialog"

# Create header panel
$headerPanel = New-Object System.Windows.Forms.Panel
$headerPanel.Size = New-Object System.Drawing.Size(750, 60)
$headerPanel.Location = New-Object System.Drawing.Point(0, 0)
$headerPanel.BackColor = [System.Drawing.Color]::FromArgb(45, 45, 48)

$headerLabel = New-Object System.Windows.Forms.Label
$headerLabel.Text = "GBE Fork Professional Build System"
$headerLabel.Font = New-Object System.Drawing.Font("Segoe UI", 16, [System.Drawing.FontStyle]::Bold)
$headerLabel.ForeColor = [System.Drawing.Color]::White
$headerLabel.Size = New-Object System.Drawing.Size(500, 30)
$headerLabel.Location = New-Object System.Drawing.Point(20, 15)
$headerPanel.Controls.Add($headerLabel)

$form.Controls.Add($headerPanel)

# Configuration Section
$configGroupBox = New-Object System.Windows.Forms.GroupBox
$configGroupBox.Text = "Build Configuration"
$configGroupBox.Font = New-Object System.Drawing.Font("Segoe UI", 10, [System.Drawing.FontStyle]::Bold)
$configGroupBox.Size = New-Object System.Drawing.Size(350, 120)
$configGroupBox.Location = New-Object System.Drawing.Point(20, 80)

# Debug/Release checkboxes
$debugCheck = New-Object System.Windows.Forms.CheckBox
$debugCheck.Text = "Debug Build"
$debugCheck.Size = New-Object System.Drawing.Size(120, 25)
$debugCheck.Location = New-Object System.Drawing.Point(20, 30)
$debugCheck.Font = New-Object System.Drawing.Font("Segoe UI", 9)

$releaseCheck = New-Object System.Windows.Forms.CheckBox
$releaseCheck.Text = "Release Build"
$releaseCheck.Size = New-Object System.Drawing.Size(120, 25)
$releaseCheck.Location = New-Object System.Drawing.Point(20, 60)
$releaseCheck.Checked = $true
$releaseCheck.Font = New-Object System.Drawing.Font("Segoe UI", 9)

$configGroupBox.Controls.AddRange(@($debugCheck, $releaseCheck))
$form.Controls.Add($configGroupBox)

# Platform Section
$platformGroupBox = New-Object System.Windows.Forms.GroupBox
$platformGroupBox.Text = "Target Platform"
$platformGroupBox.Font = New-Object System.Drawing.Font("Segoe UI", 10, [System.Drawing.FontStyle]::Bold)
$platformGroupBox.Size = New-Object System.Drawing.Size(350, 120)
$platformGroupBox.Location = New-Object System.Drawing.Point(380, 80)

# Platform checkboxes
$x64Check = New-Object System.Windows.Forms.CheckBox
$x64Check.Text = "x64 (64-bit)"
$x64Check.Size = New-Object System.Drawing.Size(120, 25)
$x64Check.Location = New-Object System.Drawing.Point(20, 30)
$x64Check.Checked = $true
$x64Check.Font = New-Object System.Drawing.Font("Segoe UI", 9)

$x32Check = New-Object System.Windows.Forms.CheckBox
$x32Check.Text = "Win32 (32-bit)"
$x32Check.Size = New-Object System.Drawing.Size(120, 25)
$x32Check.Location = New-Object System.Drawing.Point(20, 60)
$x32Check.Font = New-Object System.Drawing.Font("Segoe UI", 9)

$platformGroupBox.Controls.AddRange(@($x64Check, $x32Check))
$form.Controls.Add($platformGroupBox)

# Build Targets Section
$targetsGroupBox = New-Object System.Windows.Forms.GroupBox
$targetsGroupBox.Text = "Build Targets"
$targetsGroupBox.Font = New-Object System.Drawing.Font("Segoe UI", 10, [System.Drawing.FontStyle]::Bold)
$targetsGroupBox.Size = New-Object System.Drawing.Size(710, 350)
$targetsGroupBox.Location = New-Object System.Drawing.Point(20, 220)

# Quick selection buttons
$selectAllBtn = New-Object System.Windows.Forms.Button
$selectAllBtn.Text = "Select All"
$selectAllBtn.Size = New-Object System.Drawing.Size(90, 30)
$selectAllBtn.Location = New-Object System.Drawing.Point(20, 25)
$selectAllBtn.Font = New-Object System.Drawing.Font("Segoe UI", 9)

$selectNoneBtn = New-Object System.Windows.Forms.Button
$selectNoneBtn.Text = "Select None"
$selectNoneBtn.Size = New-Object System.Drawing.Size(90, 30)
$selectNoneBtn.Location = New-Object System.Drawing.Point(120, 25)
$selectNoneBtn.Font = New-Object System.Drawing.Font("Segoe UI", 9)

$selectHybridBtn = New-Object System.Windows.Forms.Button
$selectHybridBtn.Text = "Hybrid Only"
$selectHybridBtn.Size = New-Object System.Drawing.Size(90, 30)
$selectHybridBtn.Location = New-Object System.Drawing.Point(220, 25)
$selectHybridBtn.Font = New-Object System.Drawing.Font("Segoe UI", 9)

$selectEssentialBtn = New-Object System.Windows.Forms.Button
$selectEssentialBtn.Text = "Essential"
$selectEssentialBtn.Size = New-Object System.Drawing.Size(90, 30)
$selectEssentialBtn.Location = New-Object System.Drawing.Point(320, 25)
$selectEssentialBtn.Font = New-Object System.Drawing.Font("Segoe UI", 9)

$targetsGroupBox.Controls.AddRange(@($selectAllBtn, $selectNoneBtn, $selectHybridBtn, $selectEssentialBtn))

# Create scrollable panel for targets
$targetsPanel = New-Object System.Windows.Forms.Panel
$targetsPanel.Size = New-Object System.Drawing.Size(680, 280)
$targetsPanel.Location = New-Object System.Drawing.Point(15, 65)
$targetsPanel.AutoScroll = $true
$targetsPanel.BorderStyle = "FixedSingle"

# Create target checkboxes dynamically
$global:TargetCheckboxes = @()
$currentCategory = ""
$yPosition = 10

foreach ($target in $global:BuildTargets) {
    # Add category header if changed
    if ($target.Category -ne $currentCategory) {
        $categoryLabel = New-Object System.Windows.Forms.Label
        $categoryLabel.Text = "--- " + $target.Category + " ---"
        $categoryLabel.Font = New-Object System.Drawing.Font("Segoe UI", 9, [System.Drawing.FontStyle]::Bold)
        $categoryLabel.ForeColor = [System.Drawing.Color]::FromArgb(0, 122, 204)
        $categoryLabel.Size = New-Object System.Drawing.Size(300, 20)
        $categoryLabel.Location = New-Object System.Drawing.Point(10, $yPosition)
        $targetsPanel.Controls.Add($categoryLabel)
        $yPosition += 25
        $currentCategory = $target.Category
    }
    
    # Create checkbox for target
    $checkbox = New-Object System.Windows.Forms.CheckBox
    $checkbox.Text = $target.Description
    $checkbox.Size = New-Object System.Drawing.Size(600, 25)
    $checkbox.Location = New-Object System.Drawing.Point(30, $yPosition)
    $checkbox.Checked = $target.Default
    $checkbox.Font = New-Object System.Drawing.Font("Segoe UI", 9)
    $checkbox.Tag = $target.Name
    
    $targetsPanel.Controls.Add($checkbox)
    $global:TargetCheckboxes += $checkbox
    $yPosition += 30
}

$targetsGroupBox.Controls.Add($targetsPanel)
$form.Controls.Add($targetsGroupBox)

# Button event handlers
$selectAllBtn.Add_Click({
    # Select all targets
    foreach ($checkbox in $global:TargetCheckboxes) {
        $checkbox.Checked = $true
    }
    # Select both Debug and Release
    $debugCheck.Checked = $true
    $releaseCheck.Checked = $true
    # Select both platforms
    $x64Check.Checked = $true
    $x32Check.Checked = $true
})

$selectNoneBtn.Add_Click({
    # Deselect all targets
    foreach ($checkbox in $global:TargetCheckboxes) {
        $checkbox.Checked = $false
    }
    # Deselect build configurations
    $debugCheck.Checked = $false
    $releaseCheck.Checked = $false
    # Deselect platforms
    $x64Check.Checked = $false
    $x32Check.Checked = $false
    # Also deselect package_docs
})

$selectHybridBtn.Add_Click({
    foreach ($checkbox in $global:TargetCheckboxes) {
        $checkbox.Checked = ($checkbox.Tag -eq "steamclient_hybrid")
    }
})

$selectEssentialBtn.Add_Click({
    # Reset all first
    foreach ($checkbox in $global:TargetCheckboxes) {
        $checkbox.Checked = $false
    }
    # Select essentials: Steam API Experimental + SteamClient Hybrid + Documentation
    $essentials = @("api_experimental", "steamclient_experimental_stub", "package_docs")
    foreach ($checkbox in $global:TargetCheckboxes) {
        $checkbox.Checked = ($essentials -contains $checkbox.Tag)
    }
    # Set Release build as default for essentials
    $debugCheck.Checked = $false
    $releaseCheck.Checked = $true
    # Set x64 as default platform
    $x64Check.Checked = $true
    $x32Check.Checked = $false
})

# Action buttons
$buildBtn = New-Object System.Windows.Forms.Button
$buildBtn.Text = "Start Build"
$buildBtn.Size = New-Object System.Drawing.Size(120, 40)
$buildBtn.Location = New-Object System.Drawing.Point(450, 590)
$buildBtn.Font = New-Object System.Drawing.Font("Segoe UI", 11, [System.Drawing.FontStyle]::Bold)
$buildBtn.BackColor = [System.Drawing.Color]::FromArgb(0, 122, 204)
$buildBtn.ForeColor = [System.Drawing.Color]::White
$buildBtn.FlatStyle = "Flat"

$cancelBtn = New-Object System.Windows.Forms.Button
$cancelBtn.Text = "Cancel"
$cancelBtn.Size = New-Object System.Drawing.Size(100, 40)
$cancelBtn.Location = New-Object System.Drawing.Point(580, 590)
$cancelBtn.Font = New-Object System.Drawing.Font("Segoe UI", 11)
$cancelBtn.BackColor = [System.Drawing.Color]::FromArgb(120, 120, 120)
$cancelBtn.ForeColor = [System.Drawing.Color]::White
$cancelBtn.FlatStyle = "Flat"

# Credit label
$creditLabel = New-Object System.Windows.Forms.Label
# Use char code for reliable heart symbol
$heart = [char]0x2665
$creditLabel.Text = "Made with $heart by GittyGittyKit"
$creditLabel.Size = New-Object System.Drawing.Size(250, 20)
$creditLabel.Location = New-Object System.Drawing.Point(20, 600)
$creditLabel.Font = New-Object System.Drawing.Font("Segoe UI", 9, [System.Drawing.FontStyle]::Italic)
$creditLabel.ForeColor = [System.Drawing.Color]::FromArgb(100, 100, 100)

$form.Controls.AddRange(@($buildBtn, $cancelBtn, $creditLabel))

# Build button click handler
$buildBtn.Add_Click({
    # Check if only documentation is selected
    $selectedTargets = @()
    foreach ($checkbox in $global:TargetCheckboxes) {
        if ($checkbox.Checked) {
            $selectedTargets += $checkbox.Tag
        }
    }
    
    $onlyDocsSelected = ($selectedTargets.Count -eq 1) -and ($selectedTargets -contains "package_docs")
    
    # Validate selections (relaxed for docs-only)
    $hasConfig = $debugCheck.Checked -or $releaseCheck.Checked
    $hasPlatform = $x64Check.Checked -or $x32Check.Checked
    $hasTargets = $selectedTargets.Count -gt 0
    
    if (-not $hasTargets) {
        [System.Windows.Forms.MessageBox]::Show("Please select at least one build target.", "Invalid Selection", "OK", "Warning")
        return
    }
    
    if (-not $hasConfig) {
        [System.Windows.Forms.MessageBox]::Show("Please select at least one build configuration (Debug/Release).", "Invalid Selection", "OK", "Warning")
        return
    }
    
    # For docs-only builds, skip platform validation
    if (-not $onlyDocsSelected -and -not $hasPlatform) {
        [System.Windows.Forms.MessageBox]::Show("Please select at least one target platform (x64/Win32).", "Invalid Selection", "OK", "Warning")
        return
    }
    
    # Collect selections
    $configs = @()
    if ($debugCheck.Checked) { $configs += "debug" }
    if ($releaseCheck.Checked) { $configs += "release" }
    
    $platforms = @()
    if ($x64Check.Checked) { $platforms += "x64" }
    if ($x32Check.Checked) { $platforms += "Win32" }
    
    # For docs-only, set default platform if none selected
    if ($onlyDocsSelected -and $platforms.Count -eq 0) {
        $platforms += "x64"  # default platform for docs
    }
    
    $selectedTargets = @()
    foreach ($checkbox in $global:TargetCheckboxes) {
        if ($checkbox.Checked) {
            $selectedTargets += $checkbox.Tag
        }
    }
    
    # Auto-add dependencies for steamclient_experimental
    if ($selectedTargets -contains "steamclient_experimental") {
        if ($selectedTargets -notcontains "steamclient_experimental_extra") {
            $selectedTargets += "steamclient_experimental_extra"
        }
        if ($selectedTargets -notcontains "steamclient_experimental_loader") {
            $selectedTargets += "steamclient_experimental_loader"
        }
        if ($selectedTargets -notcontains "lib_game_overlay_renderer") {
            $selectedTargets += "lib_game_overlay_renderer"
        }
    }
    
    # Create selection object
    $selection = @{
        configs = $configs
        platforms = $platforms
        targets = $selectedTargets
        timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    }
    
    # Save selection to JSON file
    try {
        $selection | ConvertTo-Json -Depth 3 | Out-File -FilePath "build_selection.json" -Encoding UTF8
        Write-Host "Build configuration saved successfully."
        $form.Close()
    }
    catch {
        [System.Windows.Forms.MessageBox]::Show("Error saving build configuration: $_", "Error", "OK", "Error")
    }
})

# Cancel button click handler
$cancelBtn.Add_Click({
    $form.Close()
})

# Show the form
$form.ShowDialog() | Out-Null