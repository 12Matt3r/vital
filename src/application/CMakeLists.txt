# Copyright (c) 2025 Vital Application Developers
# 
# This software is provided 'as-is', without any express or implied warranty.
# In no event will the authors be held liable for any damages arising from
# the use of this software.
# 
# Permission is granted to anyone to use this software for any purpose,
# including commercial applications, and to alter it and redistribute it
# freely, subject to the following restrictions:
# 
# 1. The origin of this software must not be misrepresented; you must not
#    claim that you wrote the original software. If you use this software
#    in a product, an acknowledgment in the product documentation would be
#    appreciated but is not required.
# 2. Altered source versions must be plainly marked as such, and must not be
#    misrepresented as being the original software.
# 3. This notice may not be removed or altered from any source distribution.

cmake_minimum_required(VERSION 3.16)

# Project configuration
project(vital_application VERSION 4.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build configuration
set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "" FORCE)

# Enable testing
enable_testing()

# Source files
set(VITAL_APPLICATION_SOURCES
    main.cpp
    vital_application.cpp
    vital_plugin_processor.cpp
    vital_plugin_editor.cpp
    vital_main_window.cpp
    plugin_format_handler.cpp
)

set(VITAL_APPLICATION_HEADERS
    vital_application.h
    vital_plugin_processor.h
    vital_plugin_editor.h
    vital_main_window.h
    plugin_format_handler.h
)

# Add library target
add_library(vital_application STATIC ${VITAL_APPLICATION_SOURCES} ${VITAL_APPLICATION_HEADERS})

# Set target properties
set_target_properties(vital_application PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 4
    OUTPUT_NAME "vital_application"
    CXX_STANDARD 20
    POSITION_INDEPENDENT_CODE ON
    WINDOWS_EXPORT_ALL_SYMBOLS ON
)

# Platform-specific settings
if(WIN32)
    target_compile_definitions(vital_application PRIVATE
        JUCE_WINDOWS=1
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        UNICODE
        _UNICODE
    )
    
    # Set Windows-specific compiler flags
    target_compile_options(vital_application PRIVATE
        /W4
        /MP
        /permissive-
        /Zc:__cplusplus
        /Zc:strictStrings
        /Zc:throwingNew
    )
    
    # Link Windows libraries
    target_link_libraries(vital_application PRIVATE
        kernel32
        user32
        gdi32
        winspool
        comdlg32
        advapi32
        shell32
        ole32
        oleaut32
        uuid
        odbc32
        odbccp32
        setupapi
        psapi
        version
    )
    
    # Windows resource file
    set(WIN32_RESOURCE_FILE "${CMAKE_CURRENT_SOURCE_DIR}/resources/vital.rc")
    if(EXISTS ${WIN32_RESOURCE_FILE})
        target_sources(vital_application PRIVATE ${WIN32_RESOURCE_FILE})
    endif()
    
elseif(APPLE)
    target_compile_definitions(vital_application PRIVATE
        JUCE_MAC=1
        __MACOSX_CORE__
        __MACOSX__
    )
    
    # Set macOS-specific compiler flags
    target_compile_options(vital_application PRIVATE
        -Wall
        -Wextra
        -pedantic
        -fvisibility=hidden
        -fvisibility-inlines-hidden
    )
    
    # Link macOS frameworks
    find_library(COCOA_FRAMEWORK Cocoa)
    find_library(CORE_AUDIO_FRAMEWORK CoreAudio)
    find_library(AUDIO_UNIT_FRAMEWORK AudioUnit)
    find_library(AUDIO_TOOLBOX_FRAMEWORK AudioToolbox)
    find_library(CORE_FOUNDATION_FRAMEWORK CoreFoundation)
    find_library(CORE_SERVICES_FRAMEWORK CoreServices)
    find_library(FOUNDATION_FRAMEWORK Foundation)
    find_library(APP_KIT_FRAMEWORK AppKit)
    find_library(CORE_GRAPHICS_FRAMEWORK CoreGraphics)
    find_library(QUARTZ_CORE_FRAMEWORK QuartzCore)
    find_library(CORE_TEXT_FRAMEWORK CoreText)
    
    target_link_libraries(vital_application PRIVATE
        ${COCOA_FRAMEWORK}
        ${CORE_AUDIO_FRAMEWORK}
        ${AUDIO_UNIT_FRAMEWORK}
        ${AUDIO_TOOLBOX_FRAMEWORK}
        ${CORE_FOUNDATION_FRAMEWORK}
        ${CORE_SERVICES_FRAMEWORK}
        ${FOUNDATION_FRAMEWORK}
        ${APP_KIT_FRAMEWORK}
        ${CORE_GRAPHICS_FRAMEWORK}
        ${QUARTZ_CORE_FRAMEWORK}
        ${CORE_TEXT_FRAMEWORK}
    )
    
    # Bundle utilities for macOS
    find_program(CODESIGN codesign)
    if(CODESIGN)
        target_link_options(vital_application PRIVATE
            -Wl,-dead_strip
            -Wl,-search_paths_first
        )
    endif()
    
elseif(UNIX AND NOT APPLE)
    target_compile_definitions(vital_application PRIVATE
        JUCE_LINUX=1
        _GNU_SOURCE
        _POSIX_C_SOURCE=200809L
    )
    
    # Set Linux-specific compiler flags
    target_compile_options(vital_application PRIVATE
        -Wall
        -Wextra
        -pedantic
        -fPIC
        -pthread
    )
    
    # Link Linux libraries
    find_package(Threads REQUIRED)
    target_link_libraries(vital_application PRIVATE
        Threads::Threads
        dl
        pthread
        rt
        m
        X11
        Xrandr
        Xinerama
        Xxf86vm
        Xext
    )
endif()

# Plugin format support
option(VITAL_ENABLE_VST3 "Enable VST3 plugin format support" ON)
option(VITAL_ENABLE_AU "Enable AudioUnit plugin format support" ON)
option(VITAL_ENABLE_LV2 "Enable LV2 plugin format support" ON)
option(VITAL_ENABLE_VST2 "Enable VST2 plugin format support" OFF)
option(VITAL_ENABLE_AAX "Enable AAX plugin format support" OFF)
option(VITAL_ENABLE_CLAP "Enable CLAP plugin format support" OFF)

# Plugin format compilation definitions
if(VITAL_ENABLE_VST3)
    target_compile_definitions(vital_application PRIVATE
        JUCE_VST3_CAN_REPLACE_VST2=1
        VITAL_ENABLE_VST3_FORMAT=1
    )
endif()

if(VITAL_ENABLE_AU)
    if(APPLE)
        target_compile_definitions(vital_application PRIVATE
            JUCE_PLUGINHOST_AU=1
            VITAL_ENABLE_AU_FORMAT=1
        )
    else()
        message(WARNING "AudioUnit format only available on macOS, disabling VITAL_ENABLE_AU")
        set(VITAL_ENABLE_AU OFF)
    endif()
endif()

if(VITAL_ENABLE_LV2)
    target_compile_definitions(vital_application PRIVATE
        JUCE_PLUGINHOST_LV2=1
        VITAL_ENABLE_LV2_FORMAT=1
    )
endif()

if(VITAL_ENABLE_VST2)
    target_compile_definitions(vital_application PRIVATE
        JUCE_PLUGINHOST_VST2=1
        VITAL_ENABLE_VST2_FORMAT=1
    )
endif()

if(VITAL_ENABLE_AAX)
    target_compile_definitions(vital_application PRIVATE
        JUCE_PLUGINHOST_AAX=1
        VITAL_ENABLE_AAX_FORMAT=1
    )
endif()

if(VITAL_ENABLE_CLAP)
    target_compile_definitions(vital_application PRIVATE
        JUCE_PLUGINHOST_CLAP=1
        VITAL_ENABLE_CLAP_FORMAT=1
    )
endif()

# Compiler-specific optimizations
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # GCC/Clang optimizations
    target_compile_options(vital_application PRIVATE
        $<$<CONFIG:Release>:-O3 -DNDEBUG>
        $<$<CONFIG:Release>:-flto>
        $<$<CONFIG:Release>:-ffast-math>
        $<$<CONFIG:Release>:-march=native>
        $<$<CONFIG:Debug>:-O0 -g -DDEBUG>
        $<$<CONFIG:Debug>:-fsanitize=address -fsanitize=undefined>
    )
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    # MSVC optimizations
    target_compile_options(vital_application PRIVATE
        $<$<CONFIG:Release>:/O2 /DNDEBUG>
        $<$<CONFIG:Release>:/GL>
        $<$<CONFIG:Release>:/LTCG>
        $<$<CONFIG:Debug>:/Od /Zi /D_DEBUG /RTC1>
        $<$<CONFIG:Debug>:/fsanitize=address>
    )
endif()

# SIMD optimizations
option(VITAL_ENABLE_SSE42 "Enable SSE4.2 optimizations" ON)
option(VITAL_ENABLE_AVX2 "Enable AVX2 optimizations" ON)
option(VITAL_ENABLE_AVX512 "Enable AVX-512 optimizations" ON)
option(VITAL_ENABLE_NEON "Enable NEON optimizations" ON)

if(VITAL_ENABLE_SSE42)
    target_compile_definitions(vital_application PRIVATE VITAL_ENABLE_SSE42=1)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(vital_application PRIVATE -msse4.2)
    endif()
endif()

if(VITAL_ENABLE_AVX2)
    target_compile_definitions(vital_application PRIVATE VITAL_ENABLE_AVX2=1)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(vital_application PRIVATE -mavx2)
    endif()
endif()

if(VITAL_ENABLE_AVX512)
    target_compile_definitions(vital_application PRIVATE VITAL_ENABLE_AVX512=1)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(vital_application PRIVATE -mavx512f -mavx512cd)
    endif()
endif()

if(VITAL_ENABLE_NEON)
    if(APPLE)
        target_compile_definitions(vital_application PRIVATE VITAL_ENABLE_NEON=1)
        target_compile_options(vital_application PRIVATE -mfpu=neon)
    endif()
endif()

# Link with JUCE
find_package(JUCE CONFIG REQUIRED)
target_link_libraries(vital_application PRIVATE
    JUCE::JUCE_Audio_basics
    JUCE::JUCE_Audio_devices
    JUCE::JUCE_Audio_formats
    JUCE::JUCE_Audio_plugin_client
    JUCE::JUCE_Audio_processors
    JUCE::JUCE_Audio_utils
    JUCE::JUCE_Cryptography
    JUCE::JUCE_DataStructures
    JUCE::JUCE_Dsp
    JUCE::JUCE_Events
    JUCE::JUCE_Graphics
    JUCE::JUCE_Logging
    JUCE::JUCE_MIDI
    JUCE::JUCE_OpenGL
    JUCE::JUCE_ParallelUtils
    JUCE::JUCE_PluginFormats
    JUCE::JUCE_Properties
    JUCE::JUCE_RealtimeAudio
    JUCE::JUCE_RingBuffer
    JUCE::JUCE_Ropes
    JUCE::JUCE_Text
    JUCE::JUCE_Threading
    JUCE::JUCE_TracktionEngine
    JUCE::JUCE_Utilities
    JUCE::JUCE_Video
    JUCE::JUCE_Xml
)

# Include directories
target_include_directories(vital_application
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src/audio_engine
        ${CMAKE_SOURCE_DIR}/src/performance
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_SOURCE_DIR}/phase1/juce_modernization
        ${CMAKE_SOURCE_DIR}/phase1/developer_tools
        ${CMAKE_SOURCE_DIR}/phase2/simd_optimization
        ${CMAKE_SOURCE_DIR}/phase2/multithreading
        ${CMAKE_SOURCE_DIR}/phase4/accessibility
        ${CMAKE_SOURCE_DIR}/phase4/workflow_improvements
        ${CMAKE_SOURCE_DIR}/phase6/advanced_profiling
)

# Platform-specific include directories
if(WIN32)
    target_include_directories(vital_application PRIVATE
        $<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/../resources>
    )
elseif(APPLE)
    target_include_directories(vital_application PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../resources
    )
elseif(UNIX AND NOT APPLE)
    target_include_directories(vital_application PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/../resources
    )
endif()

# Source groups for IDE organization
source_group("Source Files" FILES ${VITAL_APPLICATION_SOURCES})
source_group("Header Files" FILES ${VITAL_APPLICATION_HEADERS})

# Install targets
install(TARGETS vital_application
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

install(FILES ${VITAL_APPLICATION_HEADERS}
    DESTINATION include/vital_application
)

# Documentation
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/../docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endif()

# Testing
find_package(GTest)
if(GTEST_FOUND)
    # Test sources would be added here
    # For now, no specific tests for the application layer
endif()

# CPack configuration
set(CPACK_PACKAGE_NAME "vital_application")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Vital Synthesizer Application Layer")
set(CPACK_PACKAGE_VENDOR "Vital Application Developers")
set(CPACK_PACKAGE_CONTACT "developers@vital-synthesizer.com")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/../LICENSE")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/../README.md")

# Platform-specific CPack settings
if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
    set(CPACK_NSIS_DISPLAY_NAME "Vital Synthesizer")
    set(CPACK_NSIS_PACKAGE_NAME "Vital Synthesizer")
    set(CPACK_NSIS_HELP_LINK "https://vital-synthesizer.com/support")
    set(CPACK_NSIS_URL_INFO_ABOUT "https://vital-synthesizer.com")
    set(CPACK_NSIS_CONTACT "support@vital-synthesizer.com")
    set(CPACK_NSIS_MODIFY_PATH ON)
    set(CPACK_NSIS_EXECUTABLES_DIRECTORY "bin")
    set(CPACK_NSIS_MUI_ICON "${CMAKE_CURRENT_SOURCE_DIR}/../resources/icons/vital_icon.ico")
    set(CPACK_NSIS_MUI_UNIICON "${CMAKE_CURRENT_SOURCE_DIR}/../resources/icons/vital_icon.ico")
    
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop;Bundle")
    set(CPACK_BUNDLE_NAME "Vital Synthesizer")
    set(CPACK_BUNDLE_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/../resources/macos/Info.plist")
    set(CPACK_BUNDLE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/../resources/macos/vital_icon.icns")
    set(CPACK_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION})
    set(CPACK_BUNDLE_LONG_VERSION_STRING ${PROJECT_VERSION})
    set(CPACK_BUNDLE_COPYRIGHT "Copyright (c) 2025 Vital Application Developers")
    
elseif(UNIX AND NOT APPLE)
    set(CPACK_GENERATOR "TGZ;DEB;RPM")
    set(CPACK_DEBIAN_PACKAGE_NAME "vital-application")
    set(CPACK_DEBIAN_PACKAGE_VERSION ${PROJECT_VERSION})
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "libjuce6, libstdc++6")
    set(CPACK_RPM_PACKAGE_NAME "vital-application")
    set(CPACK_RPM_PACKAGE_VERSION ${PROJECT_VERSION})
    set(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")
    set(CPACK_RPM_PACKAGE_REQUIRES "libjuce6, libstdc++6")
endif()

include(CPack)

# Print configuration summary
message(STATUS "=== Vital Application Configuration ===")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Platform: ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "")
message(STATUS "Plugin Format Support:")
message(STATUS "  VST3: ${VITAL_ENABLE_VST3}")
message(STATUS "  AudioUnit: ${VITAL_ENABLE_AU}")
message(STATUS "  LV2: ${VITAL_ENABLE_LV2}")
message(STATUS "  VST2: ${VITAL_ENABLE_VST2}")
message(STATUS "  AAX: ${VITAL_ENABLE_AAX}")
message(STATUS "  CLAP: ${VITAL_ENABLE_CLAP}")
message(STATUS "")
message(STATUS "SIMD Optimizations:")
message(STATUS "  SSE4.2: ${VITAL_ENABLE_SSE42}")
message(STATUS "  AVX2: ${VITAL_ENABLE_AVX2}")
message(STATUS "  AVX-512: ${VITAL_ENABLE_AVX512}")
message(STATUS "  NEON: ${VITAL_ENABLE_NEON}")
message(STATUS "====================================")
