@echo off
setlocal EnableDelayedExpansion

REM ===============================================================================
REM Vital Synthesizer - Windows Build Script
REM ===============================================================================
REM Comprehensive build script for Vital synthesizer Windows executable
REM Includes: Audio engine, AI integration, voice control, UI, plugin support
REM Optimizations: SIMD, multithreading, LTO, MSVC compiler settings
REM ===============================================================================

set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%
set BUILD_DIR=%PROJECT_ROOT%build
set INSTALL_DIR=%BUILD_DIR%\install
set DIST_DIR=%BUILD_DIR%\dist

echo ===============================================================================
echo  Vital Synthesizer - Windows Build Script
echo ===============================================================================
echo.

REM Check for required tools
echo [1/8] Checking build environment...

where cmake >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake not found. Please install CMake 3.27 or later.
    echo Download from: https://cmake.org/download/
    exit /b 1
)

where msbuild >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: MSBuild not found. Please install Visual Studio 2019 or later.
    echo Or use the Visual Studio Developer Command Prompt.
    exit /b 1
)

echo Build tools found.
echo.

REM Set build configuration
set BUILD_TYPE=Release
set CMAKE_GENERATOR=Visual Studio 17 2022
set CMAKE_ARCH=x64
set BUILD_JOBS=8

REM Parse command line arguments
:parse_args
if "%1"=="-h" goto :usage
if "%1"=="--help" goto :usage
if "%1"=="--debug" set BUILD_TYPE=Debug&shift&goto :parse_args
if "%1"=="--release" set BUILD_TYPE=Release&shift&goto :parse_args
if "%1"=="--relwithdebinfo" set BUILD_TYPE=RelWithDebInfo&shift&goto :parse_args
if "%1"=="--jobs" set BUILD_JOBS=%2&shift&shift&goto :parse_args
if "%1"=="--clean" set CLEAN_BUILD=1&shift&goto :parse_args
if "%1"=="--no-plugin" set BUILD_PLUGIN=0&shift&goto :parse_args
if "%1"=="--no-standalone" set BUILD_STANDALONE=0&shift&goto :parse_args
if "%1"=="--no-simd" set ENABLE_SIMD=0&shift&goto :parse_args
if "%1"=="--no-lto" set ENABLE_LTO=0&shift&goto :parse_args
if "%1"=="--pg" set ENABLE_PGO=1&shift&goto :parse_args
if "%1"=="--asan" set ENABLE_ASAN=1&shift&goto :parse_args
if "%1"=="--install-only" set INSTALL_ONLY=1&shift&goto :parse_args
if "%1"=="--package-only" set PACKAGE_ONLY=1&shift&goto :parse_args
if not "%1"=="" (
    echo Unknown option: %1
    goto :usage
)
goto :args_parsed

:usage
echo Usage: %~nx0 [OPTIONS]
echo.
echo Options:
echo   --debug              Build with debug configuration
echo   --release            Build with release configuration (default)
echo   --relwithdebinfo     Build with release with debug info
echo   --jobs N             Number of parallel jobs (default: 8)
echo   --clean              Clean build directory before building
echo   --no-plugin          Skip building VST3 plugin
echo   --no-standalone      Skip building standalone application
echo   --no-simd            Disable SIMD optimizations
echo   --no-lto             Disable Link-Time Optimization
echo   --pg                 Enable Profile-Guided Optimization
echo   --asan               Enable AddressSanitizer
echo   --install-only       Only install (skip build)
echo   --package-only       Only create package (skip build)
echo   -h, --help           Show this help message
echo.
echo Examples:
echo   %~nx0                           # Release build with defaults
echo   %~nx0 --debug --jobs 4          # Debug build with 4 parallel jobs
echo   %~nx0 --pg --install-only       # Install with PGO
echo.
exit /b 0

:args_parsed

REM Set default values if not specified
if not defined BUILD_PLUGIN set BUILD_PLUGIN=1
if not defined BUILD_STANDALONE set BUILD_STANDALONE=1
if not defined ENABLE_SIMD set ENABLE_SIMD=1
if not defined ENABLE_LTO set ENABLE_LTO=1
if not defined CLEAN_BUILD set CLEAN_BUILD=0
if not defined ENABLE_PGO set ENABLE_PGO=0
if not defined ENABLE_ASAN set ENABLE_ASAN=0

echo Build Configuration:
echo   Build Type:              %BUILD_TYPE%
echo   Generator:               %CMAKE_GENERATOR%
echo   Architecture:            %CMAKE_ARCH%
echo   Parallel Jobs:           %BUILD_JOBS%
echo   Build Plugin:            %BUILD_PLUGIN%
echo   Build Standalone:        %BUILD_STANDALONE%
echo   SIMD Optimizations:      %ENABLE_SIMD%
echo   Link-Time Optimization:  %ENABLE_LTO%
echo   Profile-Guided Opt:      %ENABLE_PGO%
echo   AddressSanitizer:        %ENABLE_ASAN%
echo.

REM Clean build directory if requested
if %CLEAN_BUILD% EQU 1 (
    echo [2/8] Cleaning build directory...
    if exist "%BUILD_DIR%" (
        rmdir /s /q "%BUILD_DIR%"
        echo Build directory cleaned.
    )
    echo.
)

REM Create build directories
echo [3/8] Creating build directories...
if not exist "%BUILD_DIR%" mkdir "%BUILD_DIR%"
if not exist "%INSTALL_DIR%" mkdir "%INSTALL_DIR%"
if not exist "%DIST_DIR%" mkdir "%DIST_DIR%"
echo.

REM Download JUCE if not present
echo [4/8] Checking JUCE framework...
set JUCE_DIR=%PROJECT_ROOT%external\juce
if not exist "%JUCE_DIR%" (
    echo JUCE not found. Downloading...
    git clone --depth 1 --branch 7.0.12 https://github.com/juce-framework/JUCE.git "%JUCE_DIR%"
    if !ERRORLEVEL! NEQ 0 (
        echo ERROR: Failed to download JUCE
        exit /b 1
    )
    echo JUCE downloaded successfully.
) else (
    echo JUCE found at %JUCE_DIR%
)
echo.

REM Configure CMake
echo [5/8] Configuring CMake...

cd /d "%BUILD_DIR%"

REM Build CMake command
set CMAKE_CMD=cmake -G "%CMAKE_GENERATOR%" -A %CMAKE_ARCH% ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DCMAKE_INSTALL_PREFIX="%INSTALL_DIR%" ^
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ^
    -DBUILD_SHARED_LIBS=OFF ^
    -DBUILD_TESTS=OFF ^
    -DBUILD_BENCHMARKS=OFF ^
    -DBUILD_DOCS=OFF ^
    -DBUILD_EXAMPLES=ON ^
    -DBUILD_PLUGIN=%BUILD_PLUGIN% ^
    -DBUILD_STANDALONE=%BUILD_STANDALONE% ^
    -DENABLE_WINDOWS_ASIO=ON ^
    -DENABLE_SIMD_OPTIMIZATIONS=%ENABLE_SIMD% ^
    -DENABLE_MULTITHREADING=ON ^
    -DENABLE_VECTORIZATION=ON ^
    -DENABLE_LTO=%ENABLE_LTO% ^
    -DENABLE_PGO=%ENABLE_PGO% ^
    -DENABLE_ASAN=%ENABLE_ASAN% ^
    -DENABLE_UBSAN=OFF ^
    -DENABLE_TSAN=OFF ^
    -DENABLE_COVERAGE=OFF ^
    -DENABLE_STATIC_ANALYSIS=OFF ^
    -DENABLE_PRECOMPILED_HEADERS=ON ^
    -USE_JUCE_CMAKE=ON ^
    -USE_BUNDLED_JUCE=ON ^
    -JUCE_VST3_CAN_REPLACE_PLUGIN=ON ^
    -JUCE_VST3_CAN_DO_ROLLING_UPDATES=ON ^
    -DENABLE_FAST_DEBUG=%ENABLE_ASAN% ^
    "%PROJECT_ROOT%"

echo Running: %CMAKE_CMD%
%CMAKE_CMD%
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake configuration failed
    exit /b 1
)
echo CMake configuration successful.
echo.

REM Install only if requested
if defined INSTALL_ONLY (
    echo [6/8] Installing...
    cmake --build . --target install --config %BUILD_TYPE% --parallel %BUILD_JOBS%
    if !ERRORLEVEL! NEQ 0 (
        echo ERROR: Installation failed
        exit /b 1
    )
    echo Installation completed.
    goto :end
)

REM Build the project
echo [6/8] Building Vital Synthesizer...
cmake --build . --config %BUILD_TYPE% --parallel %BUILD_JOBS%
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Build failed
    exit /b 1
)
echo Build completed successfully.
echo.

REM Install the build artifacts
echo [7/8] Installing...
cmake --build . --target install --config %BUILD_TYPE% --parallel %BUILD_JOBS%
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: Installation failed
    exit /b 1
)
echo Installation completed.
echo.

REM Package only if requested
if defined PACKAGE_ONLY (
    goto :package
)

REM Create distribution package
:package
echo [8/8] Creating distribution package...

REM Copy built executables
if exist "%INSTALL_DIR%\bin\VitalStandalone.exe" (
    copy "%INSTALL_DIR%\bin\VitalStandalone.exe" "%DIST_DIR%\"
    echo Copied standalone executable
)

if exist "%INSTALL_DIR%\bin\VitalVST3.vst3" (
    xcopy /E /I /Y "%INSTALL_DIR%\bin\VitalVST3.vst3" "%DIST_DIR%\VST3\"
    echo Copied VST3 plugin
)

REM Copy JUCE runtime dependencies (DLLs)
echo Copying runtime dependencies...
robocopy "%JUCE_DIR%\bin\resources" "%DIST_DIR%\resources" /E /NFL /NDL /NJH /NJS
if exist "%INSTALL_DIR%\bin\*.dll" (
    copy "%INSTALL_DIR%\bin\*.dll" "%DIST_DIR%\"
)

REM Copy presets and resources
if exist "%PROJECT_ROOT%\resources" (
    xcopy /E /I /Y "%PROJECT_ROOT%\resources" "%DIST_DIR%\resources"
)

REM Create plugin directory structure
if %BUILD_PLUGIN% EQU 1 (
    if not exist "%DIST_DIR%\VST3" mkdir "%DIST_DIR%\VST3"
)

REM Copy documentation
if exist "%PROJECT_ROOT%\README.md" (
    copy "%PROJECT_ROOT%\README.md" "%DIST_DIR%\"
)
if exist "%PROJECT_ROOT%\LICENSE" (
    copy "%PROJECT_ROOT%\LICENSE" "%DIST_DIR%\"
)

REM Create installer script
echo @echo off > "%DIST_DIR%\install_vst3.bat"
echo echo Installing VST3 plugin... >> "%DIST_DIR%\install_vst3.bat"
echo xcopy /Y "%~dp0VST3\VitalVST3.vst3" "%%VST3PLUGINSDIR%%\" >> "%DIST_DIR%\install_vst3.bat"
echo echo VST3 plugin installed successfully! >> "%DIST_DIR%\install_vst3.bat"
echo pause >> "%DIST_DIR%\install_vst3.bat"

REM Create installation instructions
echo Vital Synthesizer v1.0.0 - Windows Distribution > "%DIST_DIR%\INSTALL.txt"
echo ============================================== >> "%DIST_DIR%\INSTALL.txt"
echo. >> "%DIST_DIR%\INSTALL.txt"
echo INSTALLATION INSTRUCTIONS >> "%DIST_DIR%\INSTALL.txt"
echo. >> "%DIST_DIR%\INSTALL.txt"
echo STANDALONE APPLICATION: >> "%DIST_DIR%\INSTALL.txt"
echo 1. Run VitalStandalone.exe to start the synthesizer >> "%DIST_DIR%\INSTALL.txt"
echo 2. Configure your audio device in the settings >> "%DIST_DIR%\INSTALL.txt"
echo. >> "%DIST_DIR%\INSTALL.txt"
echo VST3 PLUGIN: >> "%DIST_DIR%\INSTALL.txt"
echo 1. Run install_vst3.bat as administrator >> "%DIST_DIR%\INSTALL.txt"
echo 2. Or manually copy VitalVST3.vst3 to your VST3 folder >> "%DIST_DIR%\INSTALL.txt"
echo    Default locations: >> "%DIST_DIR%\INSTALL.txt"
echo    - C:\Program Files\Common Files\VST3\ >> "%DIST_DIR%\INSTALL.txt"
echo    - C:\Users\%USERNAME%\Documents\VST3\ >> "%DIST_DIR%\INSTALL.txt"
echo. >> "%DIST_DIR%\INSTALL.txt"
echo SYSTEM REQUIREMENTS: >> "%DIST_DIR%\INSTALL.txt"
echo - Windows 10/11 (64-bit) >> "%DIST_DIR%\INSTALL.txt"
echo - AVX2 support recommended for best performance >> "%DIST_DIR%\INSTALL.txt"
echo - 4GB RAM minimum, 8GB recommended >> "%DIST_DIR%\INSTALL.txt"
echo. >> "%DIST_DIR%\INSTALL.txt"
echo For support and updates, visit: >> "%DIST_DIR%\INSTALL.txt"
echo https://github.com/Matt-McDaid/vital >> "%DIST_DIR%\INSTALL.txt"

REM Calculate package size
for %%A in ("%DIST_DIR%") do set PACKAGE_SIZE=%%~zA

echo Package created successfully in: %DIST_DIR%
echo Package size: %PACKAGE_SIZE% bytes
echo.

REM Final summary
echo ===============================================================================
echo  BUILD SUMMARY
echo ===============================================================================
echo Build Type:               %BUILD_TYPE%
echo Compiler:                 MSVC (Visual Studio 2022)
echo Architecture:             %CMAKE_ARCH%
echo SIMD Optimizations:       %ENABLE_SIMD% (AVX2)
echo Multithreading:           Enabled
echo Link-Time Optimization:   %ENABLE_LTO%
echo Profile-Guided Opt:       %ENABLE_PGO%
echo.
echo Build completed successfully!
echo Executables location:     %DIST_DIR%
echo Standalone app:           VitalStandalone.exe
if %BUILD_PLUGIN% EQU 1 echo VST3 Plugin:              VitalVST3.vst3
echo.
echo Run %DIST_DIR%\INSTALL.txt for installation instructions.
echo ===============================================================================

:end
cd /d "%PROJECT_ROOT%"
echo Build script completed.
pause
