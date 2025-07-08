# GBE Fork GUI Builder System

**Professional Interactive Build System with Graphical User Interface**

## Overview

The GUI Builder System provides a user-friendly interface for building GBE Fork components. Instead of memorizing command-line parameters, developers can use intuitive graphical dialogs to select exactly what they want to build.

## Features

### 🖥️ **Cross-Platform GUI Support**
- **Windows**: Professional PowerShell-based GUI with modern design
- **Linux**: Zenity-based dialogs with progress tracking

### ⚙️ **Flexible Build Configuration**
- **Build Types**: Debug, Release, or both
- **Platforms**: x64, Win32/x32, or both  
- **Individual Target Selection**: Choose exactly which components to build

### 🎯 **Smart Target Management**
- **Quick Selection Buttons**:
  - `Select All` - Build everything
  - `Select None` - Clear all selections
  - `Hybrid Only` - Just the hybrid client (fastest)
  - `Essential` - Core components only

### 📊 **Professional Features**
- **Progress Tracking**: Real-time build progress with completion counters
- **Error Handling**: Graceful error recovery with detailed reporting
- **Build Summary**: Complete overview of what was built
- **Validation**: Prevents invalid build configurations

## Available Build Targets

### **API Components**
- `api_regular` - Standard Steam API implementation
- `api_experimental` - Experimental Steam API features

### **Client Components**
- `steamclient_hybrid` - **Main hybrid client** (recommended)
- `steamclient_experimental` - Experimental client features
- `steamclient_experimental_stub` - Minimal client stub
- `steamclient_experimental_extra` - Extended client functionality
- `steamclient_experimental_loader` - Client loading utilities

### **Developer Tools**
- `tool_lobby_connect` - Lobby connection testing tool
- `tool_generate_interfaces` - Interface generation utility
- `tool_file_dos_stub_changer` - DOS stub modification tool

### **Libraries**
- `lib_steamnetworkingsockets` - Steam networking implementation
- `lib_game_overlay_renderer` - Game overlay rendering engine

### **Testing**
- `test_crash_printer` - Crash reporting test utilities

## Quick Start

### Windows
```batch
# Run the interactive builder
build_win_interactive.bat
```

### Linux
```bash
# Make executable and run
chmod +x build_linux_interactive.sh
./build_linux_interactive.sh
```

## Build Process Flow

1. **Dependency Check**: Verifies all required tools are available
2. **GUI Launch**: Opens the interactive selection interface
3. **Configuration**: User selects build options through intuitive GUI
4. **Validation**: System validates selections for consistency
5. **Generation**: Creates build files (solution/makefiles)
6. **Compilation**: Executes builds with progress tracking
7. **Summary**: Displays completion status and artifact locations

## Requirements

### Windows
- Windows 10/11
- PowerShell 5.0+
- Visual Studio 2022 with C++ workload
- Pre-built dependencies (`build_win_premake.bat --deps`)

### Linux
- Ubuntu 20.04+ or compatible
- Zenity (GUI toolkit) - auto-installed if missing
- GCC with multilib support
- Pre-built dependencies (`./build_linux_premake.sh --deps`)

## Usage Examples

### Build Hybrid Client Only (Fastest)
1. Run interactive builder
2. Select "Release" configuration
3. Select "x64" platform  
4. Click "Hybrid Only" button
5. Click "Start Build"

**Result**: Single optimized hybrid client in ~2-5 minutes

### Build Essential Components
1. Run interactive builder
2. Select "Release" configuration
3. Select "x64" platform
4. Click "Essential" button (selects hybrid client + API + interfaces tool)
5. Click "Start Build"

**Result**: Core development kit with all essentials

### Build Everything (Full Development Setup)
1. Run interactive builder
2. Select both "Debug" and "Release"
3. Select both "x64" and "Win32"
4. Click "Select All" button
5. Click "Start Build"

**Result**: Complete build matrix - all targets, all configs, all platforms

## Advanced Features

### **Multi-Threading**
- Automatically detects CPU cores
- Uses 70% of available cores for optimal performance
- Configurable via system environment

### **Error Recovery**
- Individual build failures don't stop the entire process
- Detailed error reporting for each failed target
- Build summary shows successful vs failed builds

### **Output Organization**
```
build/
├── win/vs2022/
│   ├── debug/      # Debug builds
│   └── release/    # Release builds
└── linux/gmake2/
    ├── debug/      # Debug builds  
    └── release/    # Release builds
```

### **JSON Configuration**
The GUI saves selections as JSON for programmatic access:
```json
{
  "configs": ["release"],
  "platforms": ["x64"],
  "targets": ["steamclient_hybrid"],
  "timestamp": "2025-01-05 15:30:00"
}
```

## Performance Tips

### **For Development**
- Use "Hybrid Only" + "Release" + "x64" for fastest iteration
- Build debug versions only when debugging is needed

### **For Distribution**
- Use "Essential" selection for most use cases
- Include both x64 and Win32 for maximum compatibility

### **For Testing**
- Use "Select All" to ensure comprehensive coverage
- Include debug builds for detailed error analysis

## Troubleshooting

### **Windows Issues**
```batch
# If PowerShell execution is restricted:
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser

# If Visual Studio not found:
# Install Visual Studio 2022 Community with C++ workload
```

### **Linux Issues**  
```bash
# If zenity not available:
sudo apt install zenity

# If build tools missing:
sudo apt install build-essential gcc-multilib g++-multilib
```

### **Common Build Errors**
- **"Premake not found"**: Run `--deps` build first to install dependencies
- **"MSBuild not found"**: Install Visual Studio 2022 with C++ workload
- **"Permission denied"**: Ensure scripts are executable (`chmod +x`)

## Integration

### **CI/CD Integration**
The GUI builder can be automated using the JSON configuration:
```batch
# Generate config programmatically, then:
call scripts\build_custom.bat
```

### **IDE Integration**
Add custom build configurations in your IDE that call:
- `build_win_interactive.bat` for Windows
- `./build_linux_interactive.sh` for Linux

---

**Made with ❤️ by GittyGittyKit**

*Professional build system for the GBE Fork project - making Steam emulation development accessible to everyone.*