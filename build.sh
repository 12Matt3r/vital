#!/bin/bash

# ===============================================================================
# Vital Synthesizer - Linux Build Script
# ===============================================================================
# This script adapts the Windows build process for Linux development
# Used for testing the build system before Windows deployment
# ===============================================================================

set -e  # Exit on any error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"
BUILD_DIR="$PROJECT_ROOT/build"
INSTALL_DIR="$BUILD_DIR/install"
DIST_DIR="$BUILD_DIR/dist"

echo "==============================================================================="
echo " Vital Synthesizer - Linux Build Script"
echo "==============================================================================="
echo ""

# Check build tools
echo "[1/6] Checking build environment..."

if ! command -v cmake &> /dev/null; then
    echo "ERROR: CMake not found. Please install CMake 3.25 or later."
    exit 1
fi

if ! command -v ninja &> /dev/null; then
    echo "INFO: Ninja not found, will use Makefiles"
    USE_NINJA=false
else
    echo "INFO: Ninja found, will use Ninja for faster builds"
    USE_NINJA=true
fi

echo "Build tools found."
echo ""

# Create build directories
echo "[2/6] Creating build directories..."
mkdir -p "$BUILD_DIR" "$INSTALL_DIR" "$DIST_DIR"
echo ""

# Download JUCE if not present
echo "[3/6] Checking JUCE framework..."
JUCE_DIR="$PROJECT_ROOT/external/juce"
if [ ! -d "$JUCE_DIR" ]; then
    echo "JUCE not found. Downloading JUCE 7.0.12..."
    git clone --depth 1 --branch 7.0.12 https://github.com/juce-framework/JUCE.git "$JUCE_DIR"
    echo "JUCE downloaded successfully."
else
    echo "JUCE found at $JUCE_DIR"
fi
echo ""

# Configure CMake
echo "[4/6] Configuring CMake..."

cd "$BUILD_DIR"

# Build CMake command
CMAKE_CMD="cmake -G Ninja"
CMAKE_CMD="$CMAKE_CMD -DCMAKE_BUILD_TYPE=Release"
CMAKE_CMD="$CMAKE_CMD -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR"
CMAKE_CMD="$CMAKE_CMD -DCMAKE_EXPORT_COMPILE_COMMANDS=ON"
CMAKE_CMD="$CMAKE_CMD -DBUILD_SHARED_LIBS=OFF"
CMAKE_CMD="$CMAKE_CMD -DBUILD_TESTS=OFF"
CMAKE_CMD="$CMAKE_CMD -DBUILD_BENCHMARKS=OFF"
CMAKE_CMD="$CMAKE_CMD -DBUILD_DOCS=OFF"
CMAKE_CMD="$CMAKE_CMD -DBUILD_EXAMPLES=ON"
CMAKE_CMD="$CMAKE_CMD -DBUILD_PLUGIN=OFF"  # Disable plugin building on Linux
CMAKE_CMD="$CMAKE_CMD -DBUILD_STANDALONE=ON"
CMAKE_CMD="$CMAKE_CMD -DENABLE_WINDOWS_ASIO=OFF"
CMAKE_CMD="$CMAKE_CMD -DENABLE_SIMD_OPTIMIZATIONS=ON"
CMAKE_CMD="$CMAKE_CMD -DENABLE_MULTITHREADING=ON"
CMAKE_CMD="$CMAKE_CMD -DENABLE_VECTORIZATION=ON"
CMAKE_CMD="$CMAKE_CMD -DENABLE_LTO=ON"
CMAKE_CMD="$CMAKE_CMD -DUSE_JUCE_CMAKE=ON"
CMAKE_CMD="$CMAKE_CMD -DUSE_BUNDLED_JUCE=ON"
CMAKE_CMD="$CMAKE_CMD -DJUCE_VST3_CAN_REPLACE_PLUGIN=ON"
CMAKE_CMD="$CMAKE_CMD -DJUCE_VST3_CAN_DO_ROLLING_UPDATES=ON"
CMAKE_CMD="$CMAKE_CMD $PROJECT_ROOT"

echo "Running: $CMAKE_CMD"
$CMAKE_CMD

if [ $? -ne 0 ]; then
    echo "ERROR: CMake configuration failed"
    exit 1
fi
echo "CMake configuration successful."
echo ""

# Build the project
echo "[5/6] Building Vital Synthesizer..."
if [ "$USE_NINJA" = true ]; then
    cmake --build . --config Release --parallel 4
else
    cmake --build . --config Release --parallel 4 -- 
fi

if [ $? -ne 0 ]; then
    echo "ERROR: Build failed"
    exit 1
fi
echo "Build completed successfully."
echo ""

# Install the build artifacts
echo "[6/6] Installing..."
cmake --build . --target install --config Release --parallel 4

if [ $? -ne 0 ]; then
    echo "ERROR: Installation failed"
    exit 1
fi
echo "Installation completed."
echo ""

# Create distribution package
echo "[7/7] Creating distribution package..."

# Copy built executable
if [ -f "$INSTALL_DIR/bin/VitalStandalone" ]; then
    cp "$INSTALL_DIR/bin/VitalStandalone" "$DIST_DIR/"
    chmod +x "$DIST_DIR/VitalStandalone"
    echo "Copied standalone executable"
fi

# Copy presets and resources
if [ -d "$PROJECT_ROOT/resources" ]; then
    cp -r "$PROJECT_ROOT/resources" "$DIST_DIR/"
fi

# Copy documentation
if [ -f "$PROJECT_ROOT/README.md" ]; then
    cp "$PROJECT_ROOT/README.md" "$DIST_DIR/"
fi

# Create README for Linux build
cat > "$DIST_DIR/README_LINUX.txt" << 'EOF'
Vital Synthesizer v1.0.0 - Linux Build
========================================

This is a development build of Vital Synthesizer for testing purposes.
For production use, please build the Windows executable using build_windows.bat
in a Windows environment with Visual Studio.

RUNNING:
1. Run ./VitalStandalone to start the synthesizer
2. Configure your audio device in the settings

SYSTEM REQUIREMENTS:
- Linux (Ubuntu 20.04+ or equivalent)
- ALSA or PulseAudio audio system
- 4GB RAM minimum, 8GB recommended

For support and updates, visit:
https://github.com/Matt-McDaid/vital
EOF

echo "Package created successfully in: $DIST_DIR"
echo "Package size: $(du -sh $DIST_DIR | cut -f1)"
echo ""

# Final summary
echo "==============================================================================="
echo " BUILD SUMMARY"
echo "==============================================================================="
echo "Build Type:               Release"
echo "Compiler:                 GCC/Clang"
echo "Architecture:             x86_64"
echo "SIMD Optimizations:       Enabled (SSE4.2, AVX2)"
echo "Multithreading:           Enabled"
echo "Link-Time Optimization:   Enabled"
echo ""
echo "Build completed successfully!"
echo "Executable location:      $DIST_DIR"
echo "Standalone app:           VitalStandalone"
echo ""
echo "Note: This is a Linux build for development testing."
echo "For Windows production build, use build_windows.bat on Windows."
echo "==============================================================================="

cd "$PROJECT_ROOT"
echo "Build script completed."
