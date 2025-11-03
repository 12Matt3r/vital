/*
  ==============================================================================
    CMakeLists.txt
    Copyright (c) 2025 Vital Plugin Integration Team
    https://vital.audio
    
    CMake configuration for Vital plugin integration
    Builds VST3 and AudioUnit plugin versions with proper dependencies and
    platform-specific configurations.
  ==============================================================================
*/

cmake_minimum_required(VERSION 3.21)
project(VitalPlugin VERSION 3.0.0 LANGUAGES CXX)

#==============================================================================
# Plugin Module Configuration

# Module name and version
set(VITAL_PLUGIN_NAME "VitalPlugin")
set(VITAL_PLUGIN_VERSION_MAJOR 3)
set(VITAL_PLUGIN_VERSION_MINOR 0)
set(VITAL_PLUGIN_VERSION_PATCH 0)

# Plugin configuration
set(VITAL_PLUGIN_ID "vital.vital.plugin")
set(VITAL_PLUGIN_MANUFACTURER "Vital Synth Team")
set(VITAL_PLUGIN_DESCRIPTION "Powerful wavetable synthesizer plugin")

# Supported plugin formats
option(VITAL_BUILD_VST3 "Build VST3 plugin format" ON)
option(VITAL_BUILD_AU "Build AudioUnit plugin format" ON)
option(VITAL_BUILD_VST2 "Build VST2 plugin format (legacy)" OFF)

# Platform-specific options
if(APPLE)
    option(VITAL_BUILD_MAC_BUNDLE "Build macOS bundle" ON)
    option(VITAL_CODE_SIGN "Code sign macOS build" OFF)
    option(VITAL_NOTARIZATION "Notarize macOS build" OFF)
endif()

if(WIN32)
    option(VITAL_BUILD_WIN_INstaller "Build Windows installer" ON)
    option(VITAL_CODE_SIGN_WINDOWS "Code sign Windows build" OFF)
endif()

#==============================================================================
# Plugin Build Configuration

# C++ standard and features
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Plugin-specific compiler flags
set(VITAL_PLUGIN_COMPILE_FLAGS
    -Wall
    -Wextra
    -Wpedantic
    -Werror
    -Wno-unused-parameter
    -Wno-unused-variable
    -Wno-unused-function
    -Wshadow
    -Wformat=2
    -Wformat-security
    -Wundef
    -Wcast-align
    -Wcast-qual
    -Wconversion
    -Wsign-compare
    -Wsign-promo
    -Woverloaded-virtual
    -Winvalid-pch
    -Winvalid-offsetof
    -Wdocumentation
    -Wdocumentation-unknown-command
    -Wheader-guard
    -Wheader-hygiene
    -Wnewline-eof
    -Wnon-virtual-dtor
    -Wredundant-decls
    -Wvla
    -Wfloat-conversion
    -Wimplicit-fallthrough
    -Wdate-time
)

# Platform-specific flags
if(APPLE)
    list(APPEND VITAL_PLUGIN_COMPILE_FLAGS
        -fPIC
        -fvisibility=hidden
        -fvisibility-inlines-hidden
        -Wno-c++11-narrowing
        -Wno-c++11-narrowing-constant-conversion
    )
elseif(WIN32)
    list(APPEND VITAL_PLUGIN_COMPILE_FLAGS
        -Wno-microsoft-include
        -Wno-documentation
    )
endif()

#==============================================================================
# JUCE Dependencies

# Find JUCE components
find_package(JUCE CONFIG REQUIRED)

# JUCE modules
juce_add_modules(
    PUBLIC
        juce_audio_basics
        juce_audio_devices
        juce_audio_formats
        juce_audio_plugin_client
        juce_audio_processors
        juce_audio_utils
        juce_data_structures
        juce_dsp
        juce_events
        juce_graphics
        juce_gui_basics
        juce_gui_extra_components
        PRIVATE
        juce_re果per
        juce_opengl
        juce_opengl_opengl3
        juce_vst3
        juce_xcode
        juce_curl
)

# JUCE configuration
juce_set_globals(
    JUCE_VST3_CAN_REPLACE_VST2 TRUE
    JUCE_VST3_EMULATE_MIDI_CC_USING_TWO_BYTES FALSE
    JUCE_VST3_EDITOR_NEEDS_OPENGL TRUE
    JUCE_VST3_INCLUDE_REPLACEMENT_VST2_MESSAGES TRUE
    JUCE_WEB_BROWSER FALSE
)

#==============================================================================
# Source Files

# Core plugin files
set(VITAL_PLUGIN_SOURCES
    vital_plugin.cpp
    vital_plugin.h
    plugin_parameters.cpp
    plugin_parameters.h
    plugin_state.cpp
    plugin_state.h
    plugin_midi.cpp
    plugin_midi.h
    plugin_ui.cpp
    plugin_ui.h
)

# VST3 wrapper (if enabled)
if(VITAL_BUILD_VST3)
    list(APPEND VITAL_PLUGIN_SOURCES
        vst3_wrapper.cpp
        vst3_wrapper.h
    )
endif()

# AudioUnit wrapper (if enabled on macOS)
if(VITAL_BUILD_AU AND APPLE)
    list(APPEND VITAL_PLUGIN_SOURCES
        au_wrapper.cpp
        au_wrapper.h
    )
endif()

# Audio engine dependency
if(TARGET vital_audio_engine)
    target_link_libraries(${VITAL_PLUGIN_NAME} PRIVATE vital_audio_engine)
else()
    # If audio engine not found, provide stub
    message(WARNING "vital_audio_engine target not found, creating stub")
    add_library(vital_audio_engine STATIC IMPORTED)
    set_target_properties(vital_audio_engine PROPERTIES
        IMPORTED_LOCATION "${CMAKE_SOURCE_DIR}/src/audio_engine/libvital_audio_engine.a"
        INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_SOURCE_DIR}/src/audio_engine"
    )
    target_link_libraries(${VITAL_PLUGIN_NAME} PRIVATE vital_audio_engine)
endif()

#==============================================================================
# Plugin Target

# Create plugin library
juce_add_plugin(${VITAL_PLUGIN_NAME}
    COMPANY_NAME ${VITAL_PLUGIN_MANUFACTURER}
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_MACOS_RESOURCES FALSE
    COPY_WIN32_RESOURCES FALSE
    MSVC_ALL_WARNINGS TRUE
    MSVC_WARNINGS_AS_ERRORS TRUE
    UNFORMATTED_CODE FALSE
    ENABLE_JUCE_VERSION_COMMENTING TRUE
    USE_LEAK_DETECTOR JUCE
)

# Set plugin metadata
set_target_properties(${VITAL_PLUGIN_NAME} PROPERTIES
    OUTPUT_NAME "Vital"
    SUFFIX ".dll"
    PREFIX ""
)

# Add sources to plugin
target_sources(${VITAL_PLUGIN_NAME} PRIVATE ${VITAL_PLUGIN_SOURCES})

# Compiler flags for plugin
target_compile_options(${VITAL_PLUGIN_NAME} PRIVATE ${VITAL_PLUGIN_COMPILE_FLAGS})

# Link JUCE modules
target_link_libraries(${VITAL_PLUGIN_NAME} PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_formats
    juce::juce_audio_plugin_client
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_data_structures
    juce::juce_dsp
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra_components
)

# Platform-specific dependencies
if(APPLE)
    target_link_libraries(${VITAL_PLUGIN_NAME} PRIVATE
        juce::juce_re果per
        juce::juce_opengl
        "-framework AudioToolbox"
        "-framework CoreAudio"
        "-framework CoreMIDI"
        "-framework Metal"
        "-framework MetalKit"
        "-framework AVFoundation"
        "-framework Foundation"
        "-framework AppKit"
    )
elseif(WIN32)
    target_link_libraries(${VITAL_PLUGIN_NAME} PRIVATE
        winmm
        user32
        gdi32
    )
endif()

# VST3-specific linking
if(VITAL_BUILD_VST3)
    target_link_libraries(${VITAL_PLUGIN_NAME} PRIVATE
        juce::juce_vst3
    )
endif()

#==============================================================================
# Plugin Format Targets

# VST3 target
if(VITAL_BUILD_VST3)
    set_target_properties(${VITAL_PLUGIN_NAME} PROPERTIES
        JUCE_VST3_MANUFACTURER_CODE ${VITAL_PLUGIN_MANUFACTURER}
        JUCE_VST3_CAN_REPLACE_VST2 TRUE
        JUCE_VST3_FORMAT TRUE
        JUCE_VST3_CLASS_NAME "VitalVST3"
    )
endif()

# AudioUnit target (macOS only)
if(VITAL_BUILD_AU AND APPLE)
    set_target_properties(${VITAL_PLUGIN_NAME} PROPERTIES
        JUCE_AUDIO_UNIT_MANUFACTURER_CODE ${VITAL_PLUGIN_MANUFACTURER}
        JUCE_AUDIO_UNIT_TYPE kMusicDeviceClassID
        JUCE_AUDIO_UNIT_SUBTYPE 'VITL'
        JUCE_AUDIO_UNIT_IS_SYNTH TRUE
        JUCE_AUDIO_UNIT_NEEDS_MIDI_INPUT TRUE
        JUCE_AUDIO_UNIT_FORMAT TRUE
        JUCE_AUDIO_UNIT_CLASS_NAME "VitalAU"
    )
endif()

#==============================================================================
# Platform-Specific Configurations

# macOS bundle configuration
if(VITAL_BUILD_MAC_BUNDLE AND APPLE)
    set_target_properties(${VITAL_PLUGIN_NAME} PROPERTIES
        BUNDLE TRUE
        BUNDLE_EXTENSION "component"
        BUNDLE_NAME "Vital"
        BUNDLE_DISPLAY_NAME "Vital"
        BUNDLE_IDENTIFIER "${VITAL_PLUGIN_ID}.AU"
        MACOSX_BUNDLE_GUI_IDENTIFIER "${VITAL_PLUGIN_ID}.AU"
        MACOSX_BUNDLE_BUNDLE_NAME "Vital"
        MACOSX_BUNDLE_DISPLAY_NAME "Vital Synthesizer"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${VITAL_PLUGIN_VERSION_MAJOR}.${VITAL_PLUGIN_VERSION_MINOR}"
        MACOSX_BUNDLE_LONG_VERSION_STRING "Version ${VITAL_PLUGIN_VERSION_MAJOR}.${VITAL_PLUGIN_VERSION_MINOR}.${VITAL_PLUGIN_VERSION_PATCH}"
        MACOSX_BUNDLE_COPYRIGHT "Copyright © ${VITAL_PLUGIN_MANUFACTURER}"
    )
endif()

# Windows configuration
if(WIN32)
    set_target_properties(${VITAL_PLUGIN_NAME} PROPERTIES
        OUTPUT_NAME "VitalSynthesizer"
        SUFFIX ".dll"
        PREFIX ""
        WINDOWS_EXPORT_ALL_SYMBOLS FALSE
    )
endif()

#==============================================================================
# Code Signing (macOS)

if(VITAL_CODE_SIGN AND APPLE)
    # Find code signing identity
    find_program(CODESIGN_TOOL codesign)
    if(CODESIGN_TOOL)
        find_program(SECURITY_TOOL security)
        
        # Get signing identity from keychain
        if(SECURITY_TOOL)
            execute_process(
                COMMAND ${SECURITY_TOOL} find-identity -v -p codesigning
                OUTPUT_VARIABLE identities
            )
            
            # Extract first identity if multiple exist
            string(REGEX MATCH "[^\n]+" signing_identity ${identities})
            string(REPLACE ": " "" signing_identity ${signing_identity})
            
            if(signing_identity)
                set_target_properties(${VITAL_PLUGIN_NAME} PROPERTIES
                    MACOSX_BUNDLE_CODESIGN_IDENTITY ${signing_identity}
                    MACOSX_BUNDLE_CODESIGN_ENTITLEMENTS "${CMAKE_CURRENT_SOURCE_DIR}/entitlements.plist"
                )
                
                target_link_options(${VITAL_PLUGIN_NAME} PRIVATE
                    "-Wl,-sectalign,__TEXT,__entitlements,0x1000"
                    "-Wl,-sectcreate,__TEXT,__entitlements,${CMAKE_CURRENT_SOURCE_DIR}/entitlements.plist"
                )
            endif()
        endif()
    endif()
endif()

#==============================================================================
# Installation

# Define installation paths
if(APPLE)
    set(VST3_INSTALL_PATH "/Library/Audio/Plug-Ins/VST3")
    set(AU_INSTALL_PATH "/Library/Audio/Plug-Ins/Components")
elseif(WIN32)
    set(VST3_INSTALL_PATH "C:/Program Files/Common Files/VST3")
    set(VST2_INSTALL_PATH "C:/Program Files/Vst2")
endif()

# Installation rules
if(VITAL_BUILD_VST3)
    install(TARGETS ${VITAL_PLUGIN_NAME}
        RUNTIME DESTINATION ${VST3_INSTALL_PATH}
        LIBRARY DESTINATION ${VST3_INSTALL_PATH}
        ARCHIVE DESTINATION ${VST3_INSTALL_PATH}
        BUNDLE DESTINATION ${VST3_INSTALL_PATH}
    )
endif()

if(VITAL_BUILD_AU AND APPLE)
    install(TARGETS ${VITAL_PLUGIN_NAME}
        RUNTIME DESTINATION ${AU_INSTALL_PATH}
        LIBRARY DESTINATION ${AU_INSTALL_PATH}
        ARCHIVE DESTINATION ${AU_INSTALL_PATH}
        BUNDLE DESTINATION ${AU_INSTALL_PATH}
    )
endif()

if(VITAL_BUILD_VST2 AND WIN32)
    install(TARGETS ${VITAL_PLUGIN_NAME}
        RUNTIME DESTINATION ${VST2_INSTALL_PATH}
        LIBRARY DESTINATION ${VST2_INSTALL_PATH}
        ARCHIVE DESTINATION ${VST2_INSTALL_PATH}
    )
endif()

#==============================================================================
# Testing

# Plugin unit tests
if(BUILD_TESTING)
    enable_testing()
    
    add_subdirectory(tests)
endif()

#==============================================================================
# Documentation

# Generate plugin documentation
add_custom_target(documentation
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/docs
    COMMAND echo "Vital Plugin Integration Documentation" > ${CMAKE_BINARY_DIR}/docs/plugin_integration.md
    COMMAND echo "" >> ${CMAKE_BINARY_DIR}/docs/plugin_integration.md
    COMMAND echo "Plugin Formats:" >> ${CMAKE_BINARY_DIR}/docs/plugin_integration.md
    if(VITAL_BUILD_VST3)
        COMMAND echo "- VST3: Enabled" >> ${CMAKE_BINARY_DIR}/docs/plugin_integration.md
    else()
        COMMAND echo "- VST3: Disabled" >> ${CMAKE_BINARY_DIR}/docs/plugin_integration.md
    endif()
    if(VITAL_BUILD_AU)
        COMMAND echo "- AudioUnit: Enabled (macOS only)" >> ${CMAKE_BINARY_DIR}/docs/plugin_integration.md
    else()
        COMMAND echo "- AudioUnit: Disabled" >> ${CMAKE_BINARY_DIR}/docs/plugin_integration.md
    endif()
    if(VITAL_BUILD_VST2)
        COMMAND echo "- VST2: Enabled (legacy)" >> ${CMAKE_BINARY_DIR}/docs/plugin_integration.md
    else()
        COMMAND echo "- VST2: Disabled" >> ${CMAKE_BINARY_DIR}/docs/plugin_integration.md
    endif()
    COMMAND echo "" >> ${CMAKE_BINARY_DIR}/docs/plugin_integration.md
    COMMAND echo "Built on: $(date)" >> ${CMAKE_BINARY_DIR}/docs/plugin_integration.md
    COMMAND echo "Plugin Version: ${VITAL_PLUGIN_VERSION_MAJOR}.${VITAL_PLUGIN_VERSION_MINOR}.${VITAL_PLUGIN_VERSION_PATCH}" >> ${CMAKE_BINARY_DIR}/docs/plugin_integration.md
    COMMAND echo "Plugin ID: ${VITAL_PLUGIN_ID}" >> ${CMAKE_BINARY_DIR}/docs/plugin_integration.md
    COMMAND echo "Manufacturer: ${VITAL_PLUGIN_MANUFACTURER}" >> ${CMAKE_BINARY_DIR}/docs/plugin_integration.md
)

#==============================================================================
# Summary

message(STATUS "")
message(STATUS "=== Vital Plugin Integration Configuration ===")
message(STATUS "Plugin Name: ${VITAL_PLUGIN_NAME}")
message(STATUS "Plugin Version: ${VITAL_PLUGIN_VERSION_MAJOR}.${VITAL_PLUGIN_VERSION_MINOR}.${VITAL_PLUGIN_VERSION_PATCH}")
message(STATUS "Plugin ID: ${VITAL_PLUGIN_ID}")
message(STATUS "Manufacturer: ${VITAL_PLUGIN_MANUFACTURER}")
message(STATUS "")
message(STATUS "Plugin Formats:")
if(VITAL_BUILD_VST3)
    message(STATUS "  ✓ VST3: Enabled")
else()
    message(STATUS "  ✗ VST3: Disabled")
endif()

if(VITAL_BUILD_AU)
    if(APPLE)
        message(STATUS "  ✓ AudioUnit: Enabled")
    else()
        message(STATUS "  ✗ AudioUnit: Disabled (macOS only)")
    endif()
else()
    message(STATUS "  ✗ AudioUnit: Disabled")
endif()

if(VITAL_BUILD_VST2)
    message(STATUS "  ✓ VST2: Enabled (legacy)")
else()
    message(STATUS "  ✗ VST2: Disabled")
endif()

message(STATUS "")
message(STATUS "Platform Configuration:")
if(APPLE)
    message(STATUS "  Platform: macOS")
    if(VITAL_BUILD_MAC_BUNDLE)
        message(STATUS "  ✓ macOS Bundle: Enabled")
    else()
        message(STATUS "  ✗ macOS Bundle: Disabled")
    endif()
    if(VITAL_CODE_SIGN)
        message(STATUS "  ✓ Code Signing: Enabled")
    else()
        message(STATUS "  ✗ Code Signing: Disabled")
    endif()
elseif(WIN32)
    message(STATUS "  Platform: Windows")
    if(VITAL_BUILD_WIN_INSTALLER)
        message(STATUS "  ✓ Windows Installer: Enabled")
    else()
        message(STATUS "  ✗ Windows Installer: Disabled")
    endif()
else()
    message(STATUS "  Platform: Linux")
endif()

message(STATUS "")
message(STATUS "Installation Paths:")
if(VITAL_BUILD_VST3)
    message(STATUS "  VST3: ${VST3_INSTALL_PATH}")
endif()
if(VITAL_BUILD_AU AND APPLE)
    message(STATUS "  AudioUnit: ${AU_INSTALL_PATH}")
endif()
if(VITAL_BUILD_VST2 AND WIN32)
    message(STATUS "  VST2: ${VST2_INSTALL_PATH}")
endif()

message(STATUS "")
message(STATUS "=== Build Configuration Complete ===")
message(STATUS "")