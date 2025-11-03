# Dependency Management Configuration
# Modern C++ package management with multiple strategies: CPM, Conan, vcpkg, and system packages

# =============================================================================
# DEPENDENCY MANAGEMENT OPTIONS
# =============================================================================

# Package manager selections
option(USE_CONAN "Use Conan for dependency management" OFF)
option(USE_VCPKG "Use vcpkg for dependency management" OFF)
option(USE_CPM "Use CPM.cmake for dependency management" ON)
option(USE_SYSTEM_PACKAGES "Use system-installed packages when available" ON)
option(FETCHCONTENT_QUIET "Make FetchContent quiet" ON)
option(ENABLE_VENDOR_DEPS "Enable vendored dependencies" OFF)

# Dependency options
set(VITAL_DEPS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external CACHE PATH "External dependencies directory")
set(VENDOR_DEPS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/vendor CACHE PATH "Vendored dependencies directory")

# =============================================================================
# CPM.CMAKE SETUP (C++ PACKAGE MANAGER)
# =============================================================================

if(USE_CPM)
  message(STATUS "=== CPM.cmake Dependency Management ===")
  
  # CPM.cmake version and source
  set(CPM_VERSION "0.38.2")
  set(CPM_DIRECTORY "${VITAL_DEPS_DIR}/CPM")
  set(CPM_FILE "${CPM_DIRECTORY}/CPM.cmake")
  
  # Download CPM if not present
  if(NOT EXISTS ${CPM_FILE})
    message(STATUS "Downloading CPM.cmake v${CPM_VERSION}...")
    file(DOWNLOAD
      "https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_VERSION}/CPM.cmake"
      ${CPM_FILE}
      EXPECTED_HASH SHA256=ca1a9a7b2a5d07a1bd60d3d1a0a8dabe477d00a1b2b1f5b7d8f2fd44ba2b4a9c
      SHOW_PROGRESS
    )
  endif()
  
  # Load CPM
  include(${CPM_FILE})
  
  # CPM options for consistent builds
  set(CPM_DOWNLOAD_ALL_CACHE FALSE)
  set(CPM_USE_LOCAL_PACKAGES FALSE)
  set(CPM_DONT_INSTALL_ON_FIND TRUE)
  
  if(FETCHCONTENT_QUIET)
    set(CPM_QUIET TRUE)
  endif()
endif()

# =============================================================================
# CORE AUDIO DEPENDENCIES
# =============================================================================

# JUCE Framework (handled in JUCEConfig.cmake, but listed here for completeness)
# JUCE is typically handled by JUCEConfig.cmake

# JSON Library - nlohmann/json
if(USE_CPM)
  CPMAddPackage(
    NAME nlohmann_json
    GITHUB_REPOSITORY nlohmann/json
    VERSION 3.11.3
    EXCLUDE_FROM_ALL
    OPTIONS
      "JSON_BuildTests OFF"
      "JSON_BuildBenchmarks OFF"
      "JSON_Install ON"
      "JSON_DefaultDumpComments ON"
      "JSON_DisableIntel guards=ON"
  )
endif()

# Logging Library - spdlog
if(USE_CPM)
  CPMAddPackage(
    NAME spdlog
    GITHUB_REPOSITORY gabrielrabensteiner/spdlog
    VERSION 1.12.0
    EXCLUDE_FROM_ALL
    OPTIONS
      "SPDLOG_BUILD_SHARED_LIBS OFF"
      "SPDLOG_BUILD_EXAMPLES OFF"
      "SPDLOG_BUILD_TESTS OFF"
      "SPDLOG_BUILD_PCH ON"
      "SPDLOG_NO_EXCEPTIONS OFF"
  )
endif()

# =============================================================================
# MATH AND DSP LIBRARIES
# =============================================================================

# Eigen3 - Linear algebra library
if(USE_CPM)
  CPMAddPackage(
    NAME Eigen
    GIT_TAG 3.4.90
    EXCLUDE_FROM_ALL
    OPTIONS
      "EIGEN_BUILD_DOCUMENTATION OFF"
      "EIGEN_BUILD_TESTING OFF"
      "EIGEN_BUILD_EXAMPLES OFF"
      "EIGEN_BUILD_PCH ON"
  )
endif()

# FFTW - Fast Fourier Transform library (optional)
find_package(FFTW QUIET)
if(FFTW_FOUND AND USE_SYSTEM_PACKAGES)
  target_link_libraries(VitalCore PUBLIC FFTW::FFTW)
  target_compile_definitions(VitalCore PUBLIC VITAL_FFTW_SYSTEM=1)
  message(STATUS "  FFTW:              System (${FFTW_VERSION})")
elseif(USE_CPM)
  CPMAddPackage(
    NAME FFTW
    URL https://www.fftw.org/fftw-3.3.10.tar.gz
    EXCLUDE_FROM_ALL
    OPTIONS
      "FFTW_BUILD_SHARED_LIBS OFF"
      "FFTW_BUILD_FLOAT ON"
      "FFTW_BUILD_DOUBLE ON"
      "FFTW_BUILD_LONG_DOUBLE ON"
      "FFTW_BUILD_QUAD_PRECISION ON"
  )
endif()

# =============================================================================
# TESTING FRAMEWORKS
# =============================================================================

if(BUILD_TESTS)
  # Catch2 - Modern C++ test framework
  if(USE_CPM)
    CPMAddPackage(
      NAME Catch2
      GITHUB_REPOSITORY catchorg/Catch2
      VERSION 3.5.0
      EXCLUDE_FROM_ALL
      OPTIONS
        "CATCH_INSTALL_DOCS OFF"
        "CATCH_INSTALL_EXAMPLES OFF"
        "CATCH_BUILD_TESTING OFF"
    )
  endif()
  
  # GoogleTest - Alternative testing framework
  if(USE_SYSTEM_PACKAGES)
    find_package(GTest QUIET)
    if(GTest_FOUND)
      target_link_libraries(VitalCore PUBLIC GTest::gtest_main)
      target_compile_definitions(VitalCore PUBLIC VITAL_GOOGLE_TEST=1)
    endif()
  endif()
endif()

# =============================================================================
# AUDIO-SPECIFIC DEPENDENCIES
# =============================================================================

# PortAudio - Cross-platform audio I/O
if(VITAL_PLATFORM_UNIX)
  if(USE_SYSTEM_PACKAGES)
    find_package(PortAudio QUIET)
    if(PortAudio_FOUND)
      target_link_libraries(VitalCore PUBLIC PortAudio::PortAudio)
      target_compile_definitions(VitalCore PUBLIC VITAL_PORTAUDIO_SYSTEM=1)
      message(STATUS "  PortAudio:         System")
    endif()
  endif()
  
  # Fallback to CPM if system package not found
  if(NOT PortAudio_FOUND AND USE_CPM)
    CPMAddPackage(
      NAME PortAudio
      GIT_REPOSITORY PortAudio/portaudio
      GIT_TAG v19.7.0
      EXCLUDE_FROM_ALL
      OPTIONS
        "PA_BUILD_SHARED_LIBS OFF"
        "PA_BUILD_STATIC_LIBS ON"
        "PA_BUILD_EXAMPLES OFF"
        "PA_BUILD_TESTS OFF"
    )
  endif()
endif()

# OpenAL - 3D audio library
if(USE_SYSTEM_PACKAGES)
  find_package(OpenAL QUIET)
  if(OpenAL_FOUND)
    target_link_libraries(VitalCore PUBLIC OpenAL::OpenAL)
    target_compile_definitions(VitalCore PUBLIC VITAL_OPENAL_SYSTEM=1)
    message(STATUS "  OpenAL:            System")
  endif()
endif()

# =============================================================================
# CRYPTOGRAPHIC AND SECURITY LIBRARIES
# =============================================================================

# OpenSSL - For secure connections (cloud features)
if(USE_SYSTEM_PACKAGES)
  find_package(OpenSSL QUIET)
  if(OpenSSL_FOUND)
    target_link_libraries(VitalCore PUBLIC OpenSSL::SSL OpenSSL::Crypto)
    target_compile_definitions(VitalCore PUBLIC VITAL_OPENSSL_SYSTEM=1)
  endif()
endif()

# =============================================================================
# CONAN INTEGRATION
# =============================================================================

if(USE_CONAN)
  message(STATUS "=== Conan Dependency Management ===")
  
  # Install Conan if not available
  find_program(CONAN conan)
  if(NOT CONAN)
    message(WARNING "Conan not found. Attempting to install...")
    execute_process(
      COMMAND pip install conan>=2.0
      RESULT_VARIABLE CONAN_INSTALL_RESULT
    )
    
    if(CONAN_INSTALL_RESULT EQUAL 0)
      find_program(CONAN conan)
      message(STATUS "  Conan installed successfully")
    else()
      message(WARNING "Failed to install Conan. Install manually with: pip install conan")
    endif()
  endif()
  
  if(CONAN)
    # Create Conan profile for Vital
    set(CONAN_PROFILE_FILE "${CMAKE_BINARY_DIR}/vital_profile")
    file(WRITE ${CONAN_PROFILE_FILE}
      "[settings]
os=Linux
os_build=Linux
arch=x86_64
arch_build=x86_64
compiler=gcc
compiler.version=11
compiler.libcxx=libstdc++11
build_type=Release

[env]
CC=gcc
CXX=g++
AR=ar
STRIP=strip

[options]
nlohmann_json:build_tests=False
nlohmann_json:build_examples=False
spdlog:build_tests=False
spdlog:build_examples=False

[conf]
tools.build:cflags=["-Wall", "-Wextra"]
tools.build:cxxflags=["-std=c++20", "-Wall", "-Wextra"]
tools.build:defines=["VITAL_CONAN_BUILD=1"]

[build_requires]
")
    
    # Create conanfile.txt for dependencies
    file(WRITE ${CMAKE_BINARY_DIR}/conanfile.txt
      "[requires]
nlohmann_json/3.11.3
spdlog/1.12.0
Eigen/3.4.90

[options]
nlohmann_json:build_tests=False
nlohmann_json:build_examples=False
spdlog:build_tests=False
spdlog:build_examples=False
spdlog:build_pch=True

[generators]
CMakeToolchain

[imports]
bin, *.dll -> ./bin # Copy DLLs from packages bin directory to the final application directory
lib, *.dylib* -> ./bin # Copy dylibs from packages lib directory to the final application directory
")
    
    # Run Conan install
    add_custom_command(
      COMMAND ${CONAN} install ${CMAKE_BINARY_DIR}/conanfile.txt
              --profile:host=${CONAN_PROFILE_FILE}
              --profile:build=${CONAN_PROFILE_FILE}
              --build=missing
              --update
      COMMAND ${CONAN} install ${CMAKE_BINARY_DIR}/conanfile.txt
              --profile:host=${CONAN_PROFILE_FILE}
              --profile:build=${CONAN_PROFILE_FILE}
              --build=missing
              --update
      COMMENT "Installing dependencies with Conan"
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    )
    
    add_custom_target(conan-deps ALL
      COMMAND ${CONAN} install ${CMAKE_BINARY_DIR}/conanfile.txt
              --profile:host=${CONAN_PROFILE_FILE}
              --profile:build=${CONAN_PROFILE_FILE}
              --build=missing
      COMMENT "Running Conan dependency installation"
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      DEPENDS ${CMAKE_BINARY_DIR}/conanfile.txt ${CONAN_PROFILE_FILE}
    )
    
    # Include Conan configuration if available
    if(EXISTS ${CMAKE_BINARY_DIR}/conan/build/Release/generators/conan_toolchain.cmake)
      include(${CMAKE_BINARY_DIR}/conan/build/Release/generators/conan_toolchain.cmake)
    endif()
    
    if(EXISTS ${CMAKE_BINARY_DIR}/conan/conanbuildinfo.cmake)
      include(${CMAKE_BINARY_DIR}/conan/conanbuildinfo.cmake)
      conan_basic_setup(TARGETS)
    endif()
  endif()
endif()

# =============================================================================
# VCPKG INTEGRATION
# =============================================================================

if(USE_VCPKG)
  message(STATUS "=== vcpkg Dependency Management ===")
  
  # Check if vcpkg is available
  find_program(VCPKG vcpkg)
  if(NOT VCPKG)
    message(WARNING "vcpkg not found. Please install vcpkg or disable USE_VCPKG.")
  else()
    # Install vcpkg dependencies
    set(VCPKG_DEPENDENCIES
      nlohmann-json
      spdlog
      eigen3
      fftw3
      portaudio
      openal-soft
    )
    
    # Triplet selection
    if(VITAL_PLATFORM_WINDOWS)
      set(VCPKG_TARGET_TRIPLET "x64-windows")
    elseif(VITAL_PLATFORM_MACOS)
      set(VCPKG_TARGET_TRIPLET "x64-osx")
    elseif(VITAL_PLATFORM_LINUX)
      set(VCPKG_TARGET_TRIPLET "x64-linux")
    endif()
    
    add_custom_command(
      COMMAND ${VCPKG} install ${VCPKG_DEPENDENCIES}:${VCPKG_TARGET_TRIPLET} --x-feature=manifest
      COMMENT "Installing dependencies with vcpkg"
      WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    )
    
    # Find vcpkg packages
    find_package(nlohmann_json CONFIG REQUIRED)
    find_package(spdlog CONFIG REQUIRED)
    find_package(Eigen3 CONFIG QUIET)
    find_package(FFTW3 CONFIG QUIET)
    find_package(portaudio CONFIG QUIET)
    find_package(OpenAL CONFIG QUIET)
  endif()
endif()

# =============================================================================
# SYSTEM PACKAGE INTEGRATION
# =============================================================================

if(USE_SYSTEM_PACKAGES)
  message(STATUS "=== System Package Integration ===")
  
  # Package configuration
  find_package(PkgConfig QUIET)
  
  # Audio system libraries
  find_package(ALSA QUIET)         # Advanced Linux Sound Architecture
  find_package(PulseAudio QUIET)   # PulseAudio sound server
  find_package(JACK QUIET)         # JACK Audio Connection Kit
  find_package(PortAudio QUIET)    # Portable audio I/O
  find_package(OpenAL QUIET)       # Open Audio Library
  
  # Platform-specific audio support
  if(VITAL_PLATFORM_MACOS)
    find_package(COREAUDIO QUIET)
    find_package(COREMIDI QUIET)
    find_package(COCOA QUIET)
  elseif(VITAL_PLATFORM_WINDOWS)
    find_package(MMDEVAPI QUIET)
    find_package(DIRECTSOUND QUIET)
    find_package(KERNEL32 QUIET)
  endif()
  
  # Link found system packages
  if(ALSA_FOUND)
    target_link_libraries(VitalCore PUBLIC ALSA::ALSA)
    target_compile_definitions(VitalCore PUBLIC VITAL_ALSA_SYSTEM=1)
    message(STATUS "  ALSA:              System")
  endif()
  
  if(PulseAudio_FOUND)
    target_link_libraries(VitalCore PUBLIC PulseAudio::PulseAudio)
    target_compile_definitions(VitalCore PUBLIC VITAL_PULSEAUDIO_SYSTEM=1)
    message(STATUS "  PulseAudio:        System")
  endif()
  
  if(JACK_FOUND)
    target_link_libraries(VitalCore PUBLIC JACK::JACK)
    target_compile_definitions(VitalCore PUBLIC VITAL_JACK_SYSTEM=1)
    message(STATUS "  JACK:              System")
  endif()
  
  if(PortAudio_FOUND)
    target_link_libraries(VitalCore PUBLIC PortAudio::PortAudio)
    target_compile_definitions(VitalCore PUBLIC VITAL_PORTAUDIO_SYSTEM=1)
    message(STATUS "  PortAudio:         System")
  endif()
  
  if(OpenAL_FOUND)
    target_link_libraries(VitalCore PUBLIC OpenAL::OpenAL)
    target_compile_definitions(VitalCore PUBLIC VITAL_OPENAL_SYSTEM=1)
    message(STATUS "  OpenAL:            System")
  endif()
endif()

# =============================================================================
# VENDORED DEPENDENCIES
# =============================================================================

if(ENABLE_VENDOR_DEPS)
  message(STATUS "=== Vendored Dependencies ===")
  
  # Copy vendored dependencies to build directory
  file(GLOB VENDOR_LIBS "${VENDOR_DEPS_DIR}/*.a" "${VENDOR_DEPS_DIR}/*.lib" "${VENDOR_DEPS_DIR}/*.dll" "${VENDOR_DEPS_DIR}/*.so" "${VENDOR_DEPS_DIR}/*.dylib")
  foreach(LIBRARY ${VENDOR_LIBS})
    add_library(VendorLib STATIC IMPORTED)
    set_target_properties(VendorLib PROPERTIES IMPORTED_LOCATION ${LIBRARY})
    target_link_libraries(VitalCore PUBLIC VendorLib)
  endforeach()
endif()

# =============================================================================
# DEPENDENCY RESOLUTION HELPERS
# =============================================================================

# Helper function to resolve dependency with multiple strategies
function(vital_resolve_dependency target_name pkg_name)
  set(options SYSTEM CPM VCPKG CONAN)
  set(one_value_args HEADER_VAR LIB_VAR DEFINITION_VAR)
  cmake_parse_arguments(VITAL_DEP "${options}" "${one_value_args}" "" ${ARGN})
  
  string(TOLOWER "${pkg_name}" pkg_lower)
  
  # Strategy 1: System package
  if(VITAL_DEP_SYSTEM)
    find_package(${pkg_name} QUIET)
    if(${pkg_name}_FOUND)
      if(VITAL_DEP_HEADER_VAR)
        set(${VITAL_DEP_HEADER_VAR} ${${pkg_name}_INCLUDE_DIRS} PARENT_SCOPE)
      endif()
      if(VITAL_DEP_LIB_VAR)
        set(${VITAL_DEP_LIB_VAR} ${${pkg_name}_LIBRARIES} PARENT_SCOPE)
      endif()
      if(VITAL_DEP_DEFINITION_VAR)
        set(${VITAL_DEP_DEFINITION_VAR} "VITAL_${pkg_upper}_SYSTEM" PARENT_SCOPE)
      endif()
    endif()
  endif()
  
  # Strategy 2: CPM package
  if(VITAL_DEP_CPM)
    if(CPM_${pkg_name}_FOUND)
      if(VITAL_DEP_HEADER_VAR)
        set(${VITAL_DEP_HEADER_VAR} ${CPM_${pkg_name}_SOURCE_DIR} PARENT_SCOPE)
      endif()
      if(VITAL_DEP_LIB_VAR)
        set(${VITAL_DEP_LIB_VAR} ${pkg_name}::${pkg_name} PARENT_SCOPE)
      endif()
      if(VITAL_DEP_DEFINITION_VAR)
        set(${VITAL_DEP_DEFINITION_VAR} "VITAL_${pkg_upper}_CPM" PARENT_SCOPE)
      endif()
    endif()
  endif()
  
  # Strategy 3: vcpkg package
  if(VITAL_DEP_VCPKG)
    find_package(${pkg_name} CONFIG QUIET)
    if(${pkg_name}_FOUND)
      if(VITAL_DEP_HEADER_VAR)
        set(${VITAL_DEP_HEADER_VAR} ${${pkg_name}_INCLUDE_DIRS} PARENT_SCOPE)
      endif()
      if(VITAL_DEP_LIB_VAR)
        set(${VITAL_DEP_LIB_VAR} ${${pkg_name}_LIBRARIES} PARENT_SCOPE)
      endif()
      if(VITAL_DEP_DEFINITION_VAR)
        set(${VITAL_DEP_DEFINITION_VAR} "VITAL_${pkg_upper}_VCPKG" PARENT_SCOPE)
      endif()
    endif()
  endif()
  
  # Strategy 4: Conan package
  if(VITAL_DEP_CONAN)
    # Conan packages are handled through CMake toolchain
    if(${pkg_name}_FOUND)
      if(VITAL_DEP_HEADER_VAR)
        set(${VITAL_DEP_HEADER_VAR} ${${pkg_name}_INCLUDE_DIRS} PARENT_SCOPE)
      endif()
      if(VITAL_DEP_LIB_VAR)
        set(${VITAL_DEP_LIB_VAR} ${${pkg_name}_LIBRARIES} PARENT_SCOPE)
      endif()
      if(VITAL_DEP_DEFINITION_VAR)
        set(${VITAL_DEP_DEFINITION_VAR} "VITAL_${pkg_upper}_CONAN" PARENT_SCOPE)
      endif()
    endif()
  endif()
endfunction()

# =============================================================================
# DEPENDENCY STATUS AND VALIDATION
# =============================================================================

# Validate critical dependencies
set(VITAL_REQUIRED_DEPS JUCE)

foreach(dep ${VITAL_REQUIRED_DEPS})
  if(NOT TARGET ${dep} AND NOT ${dep}_FOUND)
    message(FATAL_ERROR "Required dependency ${dep} not found!")
  endif()
endforeach()

# Generate dependency report
set(DEPENDENCY_REPORT_FILE "${CMAKE_BINARY_DIR}/dependency_report.txt")
file(WRITE ${DEPENDENCY_REPORT_FILE}
"=== Vital Synthesizer Dependency Report ===
Generated: $(date)

Package Managers:
  CPM: ${USE_CPM}
  Conan: ${USE_CONAN}
  vcpkg: ${USE_VCPKG}
  System: ${USE_SYSTEM_PACKAGES}

Dependencies Found:
")

if(CPM_nlohmann_json_FOUND)
  file(APPEND ${DEPENDENCY_REPORT_FILE} "  nlohmann_json: ${CPM_nlohmann_json_VERSION}\n")
endif()

if(CPM_spdlog_FOUND)
  file(APPEND ${DEPENDENCY_REPORT_FILE} "  spdlog: ${CPM_spdlog_VERSION}\n")
endif()

if(ALSA_FOUND)
  file(APPEND ${DEPENDENCY_REPORT_FILE} "  ALSA: System\n")
endif()

if(PortAudio_FOUND)
  file(APPEND ${DEPENDENCY_REPORT_FILE} "  PortAudio: System\n")
endif()

if(OpenAL_FOUND)
  file(APPEND ${DEPENDENCY_REPORT_FILE} "  OpenAL: System\n")
endif()

file(APPEND ${DEPENDENCY_REPORT_FILE} "\n=== End Dependency Report ===\n")

# =============================================================================
# DEPENDENCY SUMMARY
# =============================================================================

message(STATUS "")
message(STATUS "=== Dependency Management Status ===")
if(USE_CPM)
  message(STATUS "CPM.cmake:             ENABLED")
  if(CPM_nlohmann_json_FOUND)
    message(STATUS "  nlohmann_json:       ${CPM_nlohmann_json_VERSION}")
  endif()
  if(CPM_spdlog_FOUND)
    message(STATUS "  spdlog:              ${CPM_spdlog_VERSION}")
  endif()
  if(CPM_Eigen_FOUND)
    message(STATUS "  Eigen3:              ${CPM_Eigen_VERSION}")
  endif()
endif()

if(USE_CONAN)
  message(STATUS "Conan:                 ENABLED")
endif()

if(USE_VCPKG)
  message(STATUS "vcpkg:                 ENABLED")
endif()

if(USE_SYSTEM_PACKAGES)
  message(STATUS "System Packages:       ENABLED")
  if(ALSA_FOUND)
    message(STATUS "  ALSA:                ${ALSA_VERSION}")
  endif()
  if(PortAudio_FOUND)
    message(STATUS "  PortAudio:           ${PortAudio_VERSION}")
  endif()
  if(OpenAL_FOUND)
    message(STATUS "  OpenAL:              ${OpenAL_VERSION}")
  endif()
endif()

message(STATUS "============================")
message(STATUS "")