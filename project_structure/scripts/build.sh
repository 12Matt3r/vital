#!/bin/bash

# Vital Synthesizer Build Script
# This script builds the Vital synthesizer for multiple platforms

set -e

# Configuration
PROJECT_NAME="vital-synthesizer"
VERSION="1.0.0"
BUILD_TYPE="Release"
PARALLEL_JOBS=$(nproc)

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Helper functions
log_info() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

log_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check prerequisites
check_prerequisites() {
    log_info "Checking prerequisites..."
    
    if ! command -v cmake &> /dev/null; then
        log_error "CMake is not installed. Please install CMake 3.16 or higher."
        exit 1
    fi
    
    local cmake_version=$(cmake --version | head -n1 | awk '{print $3}')
    log_info "Found CMake version: $cmake_version"
    
    if ! command -v g++ &> /dev/null && ! command -v clang++ &> /dev/null && ! command -v cl &> /dev/null; then
        log_error "No C++ compiler found. Please install GCC, Clang, or MSVC."
        exit 1
    fi
    
    log_success "All prerequisites found"
}

# Parse command line arguments
parse_arguments() {
    while [[ $# -gt 0 ]]; do
        case $1 in
            -h|--help)
                show_help
                exit 0
                ;;
            -d|--debug)
                BUILD_TYPE="Debug"
                shift
                ;;
            -r|--release)
                BUILD_TYPE="Release"
                shift
                ;;
            -j|--jobs)
                PARALLEL_JOBS="$2"
                shift 2
                ;;
            --vst3-only)
                PLUGIN_FORMATS="-DBUILD_VST3=ON -DBUILD_AU=OFF -DBUILD_LV2=OFF"
                shift
                ;;
            --au-only)
                PLUGIN_FORMATS="-DBUILD_VST3=OFF -DBUILD_AU=ON -DBUILD_LV2=OFF"
                shift
                ;;
            --lv2-only)
                PLUGIN_FORMATS="-DBUILD_VST3=OFF -DBUILD_AU=OFF -DBUILD_LV2=ON"
                shift
                ;;
            --no-plugins)
                PLUGIN_FORMATS="-DBUILD_VST3=OFF -DBUILD_AU=OFF -DBUILD_LV2=OFF"
                shift
                ;;
            --clean)
                CLEAN_BUILD=true
                shift
                ;;
            *)
                log_error "Unknown option: $1"
                show_help
                exit 1
                ;;
        esac
    done
}

# Show help message
show_help() {
    echo "Vital Synthesizer Build Script"
    echo ""
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -h, --help          Show this help message"
    echo "  -d, --debug         Build in Debug mode"
    echo "  -r, --release       Build in Release mode (default)"
    echo "  -j, --jobs N        Use N parallel jobs (default: auto)"
    echo "  --vst3-only         Build only VST3 plugin"
    echo "  --au-only           Build only Audio Units plugin (macOS)"
    echo "  --lv2-only          Build only LV2 plugin"
    echo "  --no-plugins        Build standalone only"
    echo "  --clean             Clean build directory before building"
    echo ""
    echo "Examples:"
    echo "  $0                  # Build in release mode with all plugins"
    echo "  $0 --debug          # Build in debug mode"
    echo "  $0 --vst3-only      # Build only VST3 plugin"
    echo "  $0 --clean --j 8    # Clean and build with 8 parallel jobs"
}

# Setup build directory
setup_build_directory() {
    log_info "Setting up build directory..."
    
    mkdir -p build
    cd build
    
    if [ "$CLEAN_BUILD" = true ]; then
        log_info "Cleaning build directory..."
        rm -rf ./*
    fi
}

# Configure with CMake
configure_cmake() {
    log_info "Configuring with CMake..."
    
    local cmake_args=(
        ..
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE"
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
        -DCMAKE_CXX_STANDARD=17
        -DCMAKE_CXX_STANDARD_REQUIRED=ON
        -DCMAKE_CXX_EXTENSIONS=OFF
    )
    
    # Add plugin format options
    if [ -n "$PLUGIN_FORMATS" ]; then
        for format_option in $PLUGIN_FORMATS; do
            cmake_args+=("$format_option")
        done
    else
        cmake_args+=(-DBUILD_VST3=ON)
        cmake_args+=(-DBUILD_AU=ON)
        cmake_args+=(-DBUILD_LV2=ON)
    fi
    
    log_info "CMake arguments: ${cmake_args[*]}"
    
    if ! cmake "${cmake_args[@]}"; then
        log_error "CMake configuration failed"
        exit 1
    fi
    
    log_success "CMake configuration completed"
}

# Build the project
build_project() {
    log_info "Building project..."
    
    local build_args=(
        . 
        --config "$BUILD_TYPE"
        --parallel "$PARALLEL_JOBS"
        --verbose
    )
    
    if ! cmake --build "${build_args[@]}"; then
        log_error "Build failed"
        exit 1
    fi
    
    log_success "Build completed successfully"
}

# Run tests
run_tests() {
    if [ "$BUILD_TYPE" = "Debug" ]; then
        log_info "Running tests (debug build only)..."
        
        if ctest --output-on-failure; then
            log_success "All tests passed"
        else
            log_warning "Some tests failed"
        fi
    else
        log_info "Skipping tests (release build)"
    fi
}

# Generate package
generate_package() {
    log_info "Generating package..."
    
    cd build
    cpack --config CPackConfig.cmake
    
    log_success "Package generated: $PROJECT_NAME-$VERSION-$BUILD_TYPE"
}

# Main execution
main() {
    log_info "Starting build for $PROJECT_NAME v$VERSION"
    
    # Initialize plugin format options
    PLUGIN_FORMATS=""
    CLEAN_BUILD=false
    
    check_prerequisites
    parse_arguments "$@"
    setup_build_directory
    configure_cmake
    build_project
    run_tests
    generate_package
    
    log_success "Build process completed successfully!"
    log_info "Build directory: $(pwd)"
    
    # Show build artifacts
    find . -name "*.exe" -o -name "*.vst3" -o -name "*.component" -o -name "*.tar.gz" | while read file; do
        log_info "Built artifact: $file"
    done
}

# Run main function with all arguments
main "$@"