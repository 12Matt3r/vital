# Vital Synthesizer CMake Configuration File
# This file is used by find_package(Vital CONFIG REQUIRED)

# =============================================================================
# PACKAGE CONFIGURATION
# =============================================================================

# Set package version
if(NOT VitalCore_VERSION)
  set(VitalCore_VERSION 1.0.0)
endif()

# Create version configuration
include(CMakePackageConfigHelpers)

write_basic_package_version_file(
  "${CMAKE_CURRENT_LIST_DIR}/VitalConfigVersion.cmake"
  VERSION ${VitalCore_VERSION}
  COMPATIBILITY SameMajorVersion
)

# =============================================================================
# TARGET EXPORTS AND ALIASES
# =============================================================================

# Core library target
if(TARGET VitalCore AND NOT TARGET Vital::VitalCore)
  add_library(Vital::VitalCore ALIAS VitalCore)
endif()

# =============================================================================
# IMPORTED LOCATION VARIABLES
# =============================================================================

# Windows
if(WIN32)
  set(VitalCore_INCLUDE_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/../../../include"
  )
  
  set(VitalCore_LIBRARIES
    "${CMAKE_CURRENT_LIST_DIR}/../../../lib/VitalCore.lib"
  )
  
  set(VitalCore_RUNTIME_LIBRARIES
    "${CMAKE_CURRENT_LIST_DIR}/../../../bin/VitalCore.dll"
  )

# macOS
elseif(APPLE)
  set(VitalCore_INCLUDE_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/../../../include"
  )
  
  if(BUILD_SHARED_LIBS)
    set(VitalCore_LIBRARIES
      "${CMAKE_CURRENT_LIST_DIR}/../../../lib/libVitalCore.dylib"
    )
    
    set(VitalCore_RUNTIME_LIBRARIES
      "${CMAKE_CURRENT_LIST_DIR}/../../../lib/libVitalCore.dylib"
    )
  else()
    set(VitalCore_LIBRARIES
      "${CMAKE_CURRENT_LIST_DIR}/../../../lib/libVitalCore.a"
    )
  endif()

# Linux/Unix
else()
  set(VitalCore_INCLUDE_DIRS
    "${CMAKE_CURRENT_LIST_DIR}/../../../include"
  )
  
  if(BUILD_SHARED_LIBS)
    set(VitalCore_LIBRARIES
      "${CMAKE_CURRENT_LIST_DIR}/../../../lib/libVitalCore.so"
    )
    
    set(VitalCore_RUNTIME_LIBRARIES
      "${CMAKE_CURRENT_LIST_DIR}/../../../lib/libVitalCore.so"
    )
  else()
    set(VitalCore_LIBRARIES
      "${CMAKE_CURRENT_LIST_DIR}/../../../lib/libVitalCore.a"
    )
  endif()
endif()

# =============================================================================
# COMPONENT CONFIGURATION
# =============================================================================

# Standalone application component
if(BUILD_STANDALONE)
  if(WIN32)
    set(VitalStandalone_EXECUTABLE "${CMAKE_CURRENT_LIST_DIR}/../../../bin/VitalStandalone.exe")
  elseif(APPLE)
    set(VitalStandalone_EXECUTABLE "${CMAKE_CURRENT_LIST_DIR}/../../../bin/Vital.app")
  else()
    set(VitalStandalone_EXECUTABLE "${CMAKE_CURRENT_LIST_DIR}/../../../bin/VitalStandalone")
  endif()
endif()

# Plugin components
if(BUILD_PLUGIN)
  # VST3 Plugin
  if(WIN32)
    set(VitalVST3_PLUGIN "${CMAKE_CURRENT_LIST_DIR}/../../../lib/vst3/VitalVST3.vst3")
  elseif(APPLE)
    set(VitalVST3_PLUGIN "${CMAKE_CURRENT_LIST_DIR}/../../../lib/vst3/VitalVST3.vst3")
  else()
    set(VitalVST3_PLUGIN "${CMAKE_CURRENT_LIST_DIR}/../../../lib/vst3/VitalVST3.vst3")
  endif()
  
  # Audio Unit Plugin (macOS only)
  if(APPLE)
    set(VitalAU_PLUGIN "/Library/Audio/Plug-Ins/Components/VitalAU.component")
  endif()
endif()

# =============================================================================
# DEPENDENCY RESOLUTION
# =============================================================================

# JUCE Framework dependency
find_package(JUCE 7.0 QUIET CONFIG)

if(JUCE_FOUND)
  set(VitalCore_DEPENDENCIES "JUCE::juce_audio_basics;JUCE::juce_audio_devices;JUCE::juce_audio_formats")
  message(STATUS "VitalSynthesizer: Found JUCE ${JUCE_VERSION}")
else()
  # Fallback for systems without JUCE
  set(VitalCore_DEPENDENCIES "")
  message(WARNING "VitalSynthesizer: JUCE not found. Some features may be limited.")
endif()

# =============================================================================
# IMPORTED TARGET PROPERTIES
# =============================================================================

# Set properties on the imported target if it exists
if(TARGET VitalCore)
  set_target_properties(VitalCore PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${VitalCore_INCLUDE_DIRS}"
    IMPORTED_LOCATION "${VitalCore_LIBRARIES}"
    VERSION ${VitalCore_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
  )
  
  # Set runtime library for Windows
  if(WIN32)
    set_target_properties(VitalCore PROPERTIES
      IMPORTED_LOCATION_RELEASE "${VitalCore_LIBRARIES}"
      IMPORTED_LOCATION_DEBUG "${VitalCore_LIBRARIES}"
    )
  endif()
  
  # Link dependencies
  if(JUCE_FOUND)
    target_link_libraries(VitalCore INTERFACE ${VitalCore_DEPENDENCIES})
  endif()
endif()

# =============================================================================
# COMPONENT DEFINITIONS
# =============================================================================

# Define components for find_package
set(VitalCore_COMPONENTS Core)

if(BUILD_STANDALONE)
  list(APPEND VitalCore_COMPONENTS Standalone)
endif()

if(BUILD_PLUGIN)
  list(APPEND VitalCore_COMPONENTS Plugin VST3 AU)
endif()

if(BUILD_DOCS)
  list(APPEND VitalCore_COMPONENTS Documentation)
endif()

# =============================================================================
# UTILITY FUNCTIONS
# =============================================================================

# Function to import Vital target
function(vital_import_target component)
  if(component STREQUAL "Core")
    if(TARGET VitalCore)
      set(VITAL_TARGET VitalCore)
    else()
      set(VITAL_TARGET Vital::VitalCore)
    endif()
    set(${VITAL_TARGET_VAR} ${VITAL_TARGET} PARENT_SCOPE)
  elseif(component STREQUAL "Standalone")
    if(EXISTS "${VitalStandalone_EXECUTABLE}")
      set(${VITAL_TARGET_VAR} ${VitalStandalone_EXECUTABLE} PARENT_SCOPE)
    endif()
  elseif(component STREQUAL "Plugin")
    if(EXISTS "${VitalVST3_PLUGIN}")
      set(${VITAL_TARGET_VAR} ${VitalVST3_PLUGIN} PARENT_SCOPE)
    endif()
  elseif(component STREQUAL "VST3")
    if(EXISTS "${VitalVST3_PLUGIN}")
      set(${VITAL_TARGET_VAR} ${VitalVST3_PLUGIN} PARENT_SCOPE)
    endif()
  elseif(component STREQUAL "AU")
    if(EXISTS "${VitalAU_PLUGIN}")
      set(${VITAL_TARGET_VAR} ${VitalAU_PLUGIN} PARENT_SCOPE)
    endif()
  endif()
endfunction()

# Function to check feature availability
function(vital_has_feature feature available_var)
  if(feature STREQUAL "JUCE")
    if(JUCE_FOUND)
      set(${available_var} TRUE PARENT_SCOPE)
    endif()
  elseif(feature STREQUAL "SIMD")
    if(ENABLE_SIMD_OPTIMIZATIONS)
      set(${available_var} TRUE PARENT_SCOPE)
    endif()
  elseif(feature STREQUAL "MULTITHREADING")
    if(ENABLE_MULTITHREADING)
      set(${available_var} TRUE PARENT_SCOPE)
    endif()
  elseif(feature STREQUAL "STANDALONE")
    if(BUILD_STANDALONE AND EXISTS "${VitalStandalone_EXECUTABLE}")
      set(${available_var} TRUE PARENT_SCOPE)
    endif()
  elseif(feature STREQUAL "VST3")
    if(BUILD_PLUGIN AND EXISTS "${VitalVST3_PLUGIN}")
      set(${available_var} TRUE PARENT_SCOPE)
    endif()
  elseif(feature STREQUAL "AU")
    if(BUILD_PLUGIN AND APPLE AND EXISTS "${VitalAU_PLUGIN}")
      set(${available_var} TRUE PARENT_SCOPE)
    endif()
  endif()
endfunction()

# =============================================================================
# EXPORT CONFIGURATION
# =============================================================================

# Set cache variables for backward compatibility
set(VitalCore_INCLUDE_DIRS ${VitalCore_INCLUDE_DIRS} CACHE PATH "Vital include directories")
set(VitalCore_LIBRARIES ${VitalCore_LIBRARIES} CACHE PATH "Vital library")
set(VitalCore_VERSION ${VitalCore_VERSION} CACHE PATH "Vital version")

# =============================================================================
# DEPENDENCY CHECKS
# =============================================================================

# Check for required dependencies
if(NOT JUCE_FOUND AND NOT TARGET JUCE::juce_audio_basics)
  message(WARNING
    "VitalSynthesizer: JUCE framework not found.\n"
    "Some features may not be available. Consider installing JUCE or\n"
    "using the bundled JUCE in the Vital source directory."
  )
endif()

# =============================================================================
# COMPATIBILITY SHIMS
# =============================================================================

# Handle different naming conventions
if(NOT TARGET VitalCore AND TARGET Vital::VitalCore)
  add_library(VitalCore ALIAS Vital::VitalCore)
endif()

# =============================================================================
# FEATURE DETECTION
# =============================================================================

# Detect available features
vital_has_feature("JUCE" VITAL_HAS_JUCE)
vital_has_feature("SIMD" VITAL_HAS_SIMD)
vital_has_feature("MULTITHREADING" VITAL_HAS_MULTITHREADING)
vital_has_feature("STANDALONE" VITAL_HAS_STANDALONE)
vital_has_feature("VST3" VITAL_HAS_VST3)
vital_has_feature("AU" VITAL_HAS_AU)

# =============================================================================
# IMPORTED INTERFACE
# =============================================================================

# Create imported interface target for components
if(NOT TARGET Vital::VitalSynthesizer)
  add_library(Vital::VitalSynthesizer INTERFACE IMPORTED)
  
  set_target_properties(Vital::VitalSynthesizer PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${VitalCore_INCLUDE_DIRS}"
  )
  
  if(TARGET VitalCore)
    target_link_libraries(Vital::VitalSynthesizer INTERFACE VitalCore)
  endif()
  
  if(JUCE_FOUND)
    target_link_libraries(Vital::VitalSynthesizer INTERFACE ${VitalCore_DEPENDENCIES})
  endif()
endif()

# =============================================================================
# CACHE INvalidation FOR DEVELOPMENT
# =============================================================================

if(CMAKE_VERSION VERSION_GREATER_EQUAL "3.27")
  # Use new cache key hashing for better invalidation
  cmake_policy(SET CMP0150 NEW)
endif()

# =============================================================================
# SUMMARY MESSAGE
# =============================================================================

message(STATUS "VitalSynthesizer Config:")
message(STATUS "  Version:            ${VitalCore_VERSION}")
message(STATUS "  Include dirs:       ${VitalCore_INCLUDE_DIRS}")
message(STATUS "  Libraries:          ${VitalCore_LIBRARIES}")
if(BUILD_STANDALONE)
  message(STATUS "  Standalone:         ${VitalStandalone_EXECUTABLE}")
endif()
if(BUILD_PLUGIN)
  message(STATUS "  VST3 Plugin:        ${VitalVST3_PLUGIN}")
endif()
if(APPLE AND BUILD_PLUGIN)
  message(STATUS "  AU Plugin:          ${VitalAU_PLUGIN}")
endif()
message(STATUS "  JUCE Support:       ${VITAL_HAS_JUCE}")
message(STATUS "  SIMD Support:       ${VITAL_HAS_SIMD}")
message(STATUS "  Multithreading:     ${VITAL_HAS_MULTITHREADING}")

# =============================================================================
# EXPORT PACKAGE CONFIGURATION
# =============================================================================

# No-op - actual export is handled by install(EXPORT...) in InstallConfig.cmake