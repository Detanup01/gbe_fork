#!/usr/bin/env bash

set -e

echo "======================================================="
echo " GBE Fork Interactive Builder for Linux"
echo " Professional Build System with GUI"
echo "======================================================="
echo

# Check for zenity (GUI toolkit)
if ! command -v zenity &> /dev/null; then
    echo "Installing zenity for GUI support..."
    sudo apt update
    sudo apt install -y zenity
fi

# Check for required tools
premake_exe="./third-party/common/linux/premake/premake5"
if [[ ! -f "$premake_exe" ]]; then
    echo "ERROR: Premake5 not found at $premake_exe"
    echo "Please build dependencies first: ./build_linux_premake.sh --deps"
    zenity --error --text="Premake5 not found!\n\nPlease build dependencies first:\n./build_linux_premake.sh --deps"
    exit 1
fi

chmod +x "$premake_exe"

# Calculate build threads
build_threads="$(( $(getconf _NPROCESSORS_ONLN 2>/dev/null || echo 2) * 70 / 100 ))"
[[ $build_threads -lt 2 ]] && build_threads=2

echo "Using $build_threads build threads"
echo

# Define build targets
targets=(
    "FALSE" "api_regular" "Steam API Regular"
    "FALSE" "api_experimental" "Steam API Experimental" 
    "FALSE" "steamclient_experimental" "SteamClient Experimental"
    "TRUE" "steamclient_hybrid" "SteamClient Hybrid"
    "FALSE" "steamclient_experimental_stub" "SteamClient Stub"
    "FALSE" "steamclient_experimental_extra" "SteamClient Extra"
    "FALSE" "steamclient_experimental_loader" "SteamClient Loader"
    "FALSE" "tool_lobby_connect" "Lobby Connect Tool"
    "FALSE" "tool_generate_interfaces" "Generate Interfaces Tool"
    "FALSE" "tool_file_dos_stub_changer" "DOS Stub Changer"
    "FALSE" "lib_steamnetworkingsockets" "Steam Networking Sockets"
    "FALSE" "lib_game_overlay_renderer" "Game Overlay Renderer"
    "FALSE" "test_crash_printer" "Crash Printer Test"
)

# Show build configuration dialog
config_choice=$(zenity --list --checklist \
    --title="GBE Fork Builder - Build Configuration" \
    --text="Select build configurations:" \
    --column="Select" --column="Configuration" --column="Description" \
    FALSE "debug" "Debug Build (with symbols)" \
    TRUE "release" "Release Build (optimized)" \
    --width=500 --height=300)

if [[ -z "$config_choice" ]]; then
    echo "Build cancelled by user."
    exit 0
fi

# Show platform selection dialog  
platform_choice=$(zenity --list --checklist \
    --title="GBE Fork Builder - Target Platform" \
    --text="Select target platforms:" \
    --column="Select" --column="Platform" --column="Description" \
    TRUE "x64" "64-bit build" \
    FALSE "x32" "32-bit build" \
    --width=400 --height=250)

if [[ -z "$platform_choice" ]]; then
    echo "Build cancelled by user."
    exit 0
fi

# Show targets selection dialog
target_choice=$(zenity --list --checklist \
    --title="GBE Fork Builder - Build Targets" \
    --text="Select build targets:" \
    --column="Select" --column="Target" --column="Description" \
    "${targets[@]}" \
    --width=700 --height=500)

if [[ -z "$target_choice" ]]; then
    echo "Build cancelled by user."
    exit 0
fi

# Convert choices to arrays
IFS='|' read -ra selected_configs <<< "$config_choice"
IFS='|' read -ra selected_platforms <<< "$platform_choice" 
IFS='|' read -ra selected_targets <<< "$target_choice"

echo "======================================================="
echo " Build Configuration Summary"
echo "======================================================="
echo " Configurations: ${selected_configs[*]}"
echo " Platforms:      ${selected_platforms[*]}"
echo " Targets:        ${selected_targets[*]}"
echo "======================================================="
echo

# Generate makefiles
echo "Generating makefiles..."
"$premake_exe" --genproto --os=linux gmake2 || {
    zenity --error --text="Failed to generate makefiles!"
    exit 1
}

cd ./build/project/gmake2/linux

# Calculate total builds
total_builds=0
for config in "${selected_configs[@]}"; do
    for platform in "${selected_platforms[@]}"; do
        for target in "${selected_targets[@]}"; do
            ((total_builds++))
        done
    done
done

echo "Total builds to process: $total_builds"
echo

# Execute builds with progress
current_build=0
for config in "${selected_configs[@]}"; do
    for platform in "${selected_platforms[@]}"; do
        for target in "${selected_targets[@]}"; do
            ((current_build++))
            
            echo
            echo "[$current_build/$total_builds] Building $target $config $platform"
            echo "------------------------------------------------------"
            
            # Convert platform name for make
            make_platform="$platform"
            [[ "$platform" == "x32" ]] && make_platform="x32"
            [[ "$platform" == "x64" ]] && make_platform="x64"
            
            make_config="${config}_${make_platform}"
            
            # Show progress in GUI
            progress=$((current_build * 100 / total_builds))
            echo "$progress" | zenity --progress \
                --title="Building GBE Fork" \
                --text="Building $target ($config $platform)..." \
                --percentage=0 --auto-close --no-cancel &
            
            # Execute build
            make -j $build_threads config="$make_config" "$target" || {
                zenity --error --text="Build failed for:\n$target $config $platform\n\nCheck console for details."
                echo
                echo "ERROR: Build failed for $target $config $platform"
                echo "Continuing with remaining builds..."
                echo
            }
            
            # Kill progress dialog
            pkill -f "zenity.*progress" 2>/dev/null || true
        done
    done
done

cd ../../../../

echo
echo "======================================================="
echo " Build Process Summary"
echo "======================================================="
echo " Total builds attempted: $total_builds"
echo
echo " Build artifacts can be found in:"
echo " - build/linux/gmake2/debug/   (for debug builds)"
echo " - build/linux/gmake2/release/ (for release builds)"
echo "======================================================="

zenity --info --text="Build process completed!\n\nTotal builds: $total_builds\n\nCheck build/linux/gmake2/ for artifacts."

echo "Build process completed successfully!"