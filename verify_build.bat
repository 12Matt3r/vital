@echo off
REM ===============================================================================
REM Vital Synthesizer - Pre-Build Verification Script
REM ===============================================================================
REM This script checks if all required files and components are present
REM before attempting to build the Windows executable.
REM ===============================================================================

set SCRIPT_DIR=%~dp0
set PROJECT_ROOT=%SCRIPT_DIR%

echo ===============================================================================
echo  Vital Synthesizer - Pre-Build Verification
echo ===============================================================================
echo.

set VERIFICATION_PASSED=1

REM Check if we're in the right directory
if not exist "%PROJECT_ROOT%CMakeLists.txt" (
    echo ERROR: CMakeLists.txt not found in %PROJECT_ROOT%
    echo Please run this script from the vital_application directory
    set VERIFICATION_PASSED=0
    goto :verification_end
)

echo [1/6] Checking project structure...
if not exist "%PROJECT_ROOT%src\application\main.cpp" (
    echo ERROR: Main application source not found
    set VERIFICATION_PASSED=0
) else (
    echo   ✓ Main application source found
)

if not exist "%PROJECT_ROOT%src\audio_engine\" (
    echo ERROR: Audio engine directory not found
    set VERIFICATION_PASSED=0
) else (
    echo   ✓ Audio engine directory found
)

if not exist "%PROJECT_ROOT%src\plugin\" (
    echo ERROR: Plugin directory not found
    set VERIFICATION_PASSED=0
) else (
    echo   ✓ Plugin directory found
)

if not exist "%PROJECT_ROOT%src\ui\" (
    echo ERROR: UI directory not found
    set VERIFICATION_PASSED=0
) else (
    echo   ✓ UI directory found
)

echo.
echo [2/6] Checking build configuration...
if not exist "%PROJECT_ROOT%build_windows.bat" (
    echo ERROR: Windows build script not found
    set VERIFICATION_PASSED=0
) else (
    echo   ✓ Windows build script found
)

if not exist "%PROJECT_ROOT%cmake\PlatformConfig.cmake" (
    echo ERROR: Platform configuration not found
    set VERIFICATION_PASSED=0
) else (
    echo   ✓ Platform configuration found
)

if not exist "%PROJECT_ROOT%cmake\CompilerConfig.cmake" (
    echo ERROR: Compiler configuration not found
    set VERIFICATION_PASSED=0
) else (
    echo   ✓ Compiler configuration found
)

echo.
echo [3/6] Checking JUCE framework...
set JUCE_DIR=%PROJECT_ROOT%external\juce
if not exist "%JUCE_DIR%" (
    echo WARNING: JUCE directory not found
    echo The build script will download JUCE automatically
    echo   ⚠ JUCE will be downloaded during build
) else (
    if not exist "%JUCE_DIR%\CMakeLists.txt" (
        echo WARNING: JUCE appears incomplete
        echo   ⚠ JUCE download may be corrupted
    ) else (
        echo   ✓ JUCE framework found
    )
)

echo.
echo [4/6] Checking build dependencies...
where cmake >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: CMake not found in PATH
    echo Please install CMake 3.27 or later from: https://cmake.org/download/
    set VERIFICATION_PASSED=0
) else (
    echo   ✓ CMake found
    cmake --version | findstr "CMake"
)

where msbuild >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo ERROR: MSBuild not found in PATH
    echo Please install Visual Studio 2019 or later
    set VERIFICATION_PASSED=0
) else (
    echo   ✓ MSBuild found
)

where git >nul 2>&1
if %ERRORLEVEL% NEQ 0 (
    echo WARNING: Git not found in PATH
    echo Git is required for downloading JUCE framework
    echo Please install Git from: https://git-scm.com/download/win
) else (
    echo   ✓ Git found
)

echo.
echo [5/6] Checking system requirements...
for /f "tokens=4-5 delims=. " %%i in ('ver') do set VERSION=%%i.%%j
if "%VERSION%" LSS "10.0" (
    echo ERROR: Windows version too old
    echo Vital requires Windows 10 or later
    set VERIFICATION_PASSED=0
) else (
    echo   ✓ Windows version: !VERSION!
)

systeminfo | find "OS Name" >nul
if %ERRORLEVEL% EQU 0 (
    systeminfo | find "OS Name"
    systeminfo | find "Total Physical Memory"
) else (
    echo   ⚠ Could not retrieve system information
)

echo.
echo [6/6] Checking available disk space...
for /f "tokens=3" %%a in ('dir /-c "%PROJECT_ROOT%" ^| find "bytes free"') do set FREE_SPACE=%%a
echo   Available disk space: %FREE_SPACE% bytes
echo   (Minimum 2GB recommended for build)

echo.
echo ===============================================================================
echo  VERIFICATION SUMMARY
echo ===============================================================================

if %VERIFICATION_PASSED% EQU 1 (
    echo ✓ ALL CHECKS PASSED
    echo.
    echo Your system is ready to build Vital Synthesizer!
    echo.
    echo Next steps:
    echo   1. Run: build_windows.bat
    echo   2. Wait for build completion (may take 10-30 minutes)
    echo   3. Find executables in: build\dist\
    echo.
    echo For detailed instructions, see: BUILD_INSTRUCTIONS.md
) else (
    echo ✗ VERIFICATION FAILED
    echo.
    echo Some requirements are missing or incorrect.
    echo Please address the errors above before building.
    echo.
    echo Common solutions:
    echo   - Install Visual Studio with C++ workload
    echo   - Install CMake 3.27+ and add to PATH
    echo   - Install Git for downloading dependencies
    echo   - Ensure you're running as Administrator if needed
)

echo ===============================================================================
echo.
pause