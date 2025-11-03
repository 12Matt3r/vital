# Vital Synthesizer - Windows Build Instructions

## Overview
This build system creates a complete Windows executable for Vital Synthesizer including:
- Standalone application (VitalStandalone.exe)
- VST3 plugin (VitalVST3.vst3)
- All audio engine, AI integration, voice control, and UI components
- Performance optimizations (SIMD, multithreading, LTO)

## Prerequisites
Before building, ensure you have the following installed on Windows:

### Required Tools
1. **Visual Studio 2019 or later** with C++ workload
   - Download: https://visualstudio.microsoft.com/vs/
   - Required components:
     - MSVC v143 compiler toolset
     - Windows 10/11 SDK
     - CMake tools for Visual Studio

2. **CMake 3.27 or later**
   - Download: https://cmake.org/download/
   - Add to PATH

3. **Git** (for JUCE framework download)
   - Download: https://git-scm.com/download/win

### Optional Tools
- **Ninja** (for faster builds): https://ninja-build.org/
- **7-Zip** (for package compression)

## Building the Executable

### Quick Build (Recommended)
```cmd
cd path\to\vital_application
build_windows.bat
```

This creates:
- Release build with all optimizations enabled
- Both standalone and VST3 plugin
- Distribution package in `build/dist/`

### Custom Build Options
```cmd
build_windows.bat [OPTIONS]
```

**Configuration Options:**
- `--debug` - Debug build configuration
- `--release` - Release build (default)
- `--relwithdebinfo` - Release with debug info
- `--jobs N` - Parallel build jobs (default: 8)

**Build Control:**
- `--clean` - Clean build directory before building
- `--no-plugin` - Skip VST3 plugin build
- `--no-standalone` - Skip standalone app build

**Performance Options:**
- `--no-simd` - Disable SIMD optimizations
- `--no-lto` - Disable Link-Time Optimization
- `--pg` - Enable Profile-Guided Optimization
- `--asan` - Enable AddressSanitizer (debug builds)

**Utility Options:**
- `--install-only` - Only install (skip build)
- `--package-only` - Only create package (skip build)

### Example Commands
```cmd
# Debug build with 4 parallel jobs
build_windows.bat --debug --jobs 4

# Release build without VST3 plugin
build_windows.bat --no-plugin

# Clean release build with PGO
build_windows.bat --clean --pg

# Build with sanitizers for testing
build_windows.bat --asan --debug
```

## Build Output

After successful build, you'll find:

### `build/dist/` Directory
- `VitalStandalone.exe` - Standalone synthesizer application
- `VST3/` - VST3 plugin directory
  - `VitalVST3.vst3` - VST3 plugin file
- `install_vst3.bat` - VST3 installation script
- `resources/` - Presets, themes, and other assets
- `INSTALL.txt` - Installation instructions
- Various DLL files - Runtime dependencies

### `build/install/` Directory
- `bin/` - All built executables and libraries
- `lib/` - Static libraries
- `include/` - Headers (if building as library)

## Installation

### Standalone Application
1. Run `VitalStandalone.exe` directly from `build/dist/`
2. Configure your audio device in the application settings
3. No additional installation required

### VST3 Plugin
1. **Automatic Installation:**
   - Run `install_vst3.bat` as Administrator
   - This copies the plugin to the system VST3 directory

2. **Manual Installation:**
   - Copy `VitalVST3.vst3` to your VST3 folder:
     - System: `C:\Program Files\Common Files\VST3\`
     - User: `C:\Users\%USERNAME%\Documents\VST3\`

## System Requirements

### Minimum Requirements
- Windows 10/11 (64-bit)
- 4GB RAM
- DirectX compatible audio device

### Recommended Requirements
- Windows 11 (64-bit)
- 8GB+ RAM
- AVX2 support for optimal performance
- ASIO-compatible audio interface

## Troubleshooting

### Build Errors
1. **"CMake not found"**
   - Install CMake 3.27+ and add to PATH
   - Restart command prompt after installation

2. **"MSBuild not found"**
   - Install Visual Studio with C++ workload
   - Use "Developer Command Prompt for VS"

3. **"JUCE download failed"**
   - Check internet connection
   - Ensure Git is installed and in PATH

### Runtime Errors
1. **"Missing DLL" errors**
   - Install Visual C++ Redistributable
   - Run from Developer Command Prompt for dependency info

2. **Audio device errors**
   - Check Windows audio device settings
   - Try running as Administrator
   - Install ASIO driver for better performance

### Performance Issues
1. **Audio dropouts or latency**
   - Increase audio buffer size in settings
   - Disable antivirus real-time scanning for the app
   - Close other audio applications

2. **Slow performance**
   - Ensure AVX2 is supported and enabled
   - Close unnecessary background applications
   - Check RAM usage in Task Manager

## Build Configuration Details

### Compiler Settings
- **MSVC v143** (Visual Studio 2022)
- **C++20** standard
- **x64** architecture only

### Optimization Features
- **SIMD Vectorization:** SSE4.2, AVX, AVX2
- **Multithreading:** Parallel audio processing
- **Link-Time Optimization:** Cross-module optimization
- **Profile-Guided Optimization:** Runtime-optimized builds

### Plugin Features
- **VST3 SDK:** Latest Steinberg VST3 interface
- **MIDI Support:** Full MIDI input/processing
- **Automation:** Parameter automation support
- **State Management:** Plugin state save/restore

## Support

For issues and support:
- GitHub: https://github.com/Matt-McDaid/vital
- Documentation: See individual module README files
- Build Issues: Check CMake logs in `build/CMakeFiles/`

## Advanced Configuration

### Custom CMake Options
To modify CMake settings, edit `build_windows.bat` around line 161-189. Common customizations:

```cmake
# Enable additional features
-DENABLE_EXPERIMENTAL_FEATURES=ON
-DENABLE_DEBUG_CONSOLE=ON
-DENABLE_PROFILING=ON

# Modify optimization levels
-DCMAKE_CXX_OPTIMIZATION_LEVEL=3
-DCMAKE_C_OPTIMIZATION_LEVEL=3
```

### Environment Variables
- `CMAKE_BUILD_TYPE` - Build configuration
- `CMAKE_CXX_FLAGS` - Additional compiler flags
- `JUCE_VST3_SDK_DIR` - Custom VST3 SDK path

---

**Note:** This build system creates both standalone and plugin versions. The VST3 plugin requires Steinberg VST3 SDK which is included via JUCE integration.