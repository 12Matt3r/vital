# Vital Synthesizer - Comprehensive CMake Build System
# 
# A modern, cross-platform build system for the Vital synthesizer with:
# - JUCE framework integration
# - Cross-platform support (Windows, macOS, Linux, WebAssembly)
# - Performance optimizations (SIMD, multithreading)
# - Windows-specific optimizations
# - Multiple dependency management strategies
# - Static analysis and testing integration

cmake_minimum_required(VERSION 3.27)

# Enable modern CMake features
cmake_policy(VERSION 3.27)
if(POLICY CMP0150)
  cmake_policy(SET CMP0150 NEW)  # Hash-based cache keys
endif()
if(POLICY CMP0167)
  cmake_policy(SET CMP0167 NEW)  # FindPkgConfig modules
endif()
if(POLICY CMP0135)
  cmake_policy(SET CMP0135 NEW)  # ExternalProject download timestamp
endif()

# =============================================================================
# PROJECT CONFIGURATION
# =============================================================================

project(
  VitalSynthesizer
  VERSION 1.0.0
  DESCRIPTION "Vital - Powerful, Beautiful, Free, Open-Source Synthesizer"
  LANGUAGES CXX
  HOMEPAGE_URL "https://github.com/Matt-McDaid/vital"
)

# Set default build type if not specified
if(NOT CMAKE_BUILD_TYPE)
  if(DEFINED ENV{CMAKE_BUILD_TYPE})
    set(CMAKE_BUILD_TYPE $ENV{CMAKE_BUILD_TYPE})
  else()
    if(DEFINED CMAKE_CONFIGURATION_TYPES)
      set(CMAKE_BUILD_TYPE "RelWithDebInfo")
    else()
      set(CMAKE_BUILD_TYPE "RelWithDebInfo")
    endif()
  endif()
endif()

# Language standards
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

# =============================================================================
# BUILD CONFIGURATION OPTIONS
# =============================================================================

# Core build options
option(BUILD_SHARED_LIBS "Build shared libraries instead of static" OFF)
option(BUILD_TESTS "Build comprehensive test suite" ON)
option(BUILD_BENCHMARKS "Build performance benchmarks" ON)
option(BUILD_DOCS "Build documentation" ON)
option(BUILD_EXAMPLES "Build example applications" ON)
option(BUILD_PLUGIN "Build VST3/AU plugin" ON)
option(BUILD_STANDALONE "Build standalone application" ON)

# Platform-specific options
option(ENABLE_WINDOWS_ASIO "Enable Windows ASIO support" ON)
option(ENABLE_MACOS_FRAMEWORK "Enable macOS framework integration" ON)
option(ENABLE_LINUX_PULSE "Enable Linux PulseAudio support" ON)

# Performance optimization options
option(ENABLE_SIMD_OPTIMIZATIONS "Enable SIMD optimizations (SSE/AVX/NEON)" ON)
option(ENABLE_MULTITHREADING "Enable multithreading optimizations" ON)
option(ENABLE_VECTORIZATION "Enable auto-vectorization" ON)
option(ENABLE_LTO "Enable Link-Time Optimization" ON)
option(ENABLE_PGO "Enable Profile-Guided Optimization" OFF)

# JUCE integration options
option(USE_JUCE_CMAKE "Use JUCE CMake integration" ON)
option(USE_BUNDLED_JUCE "Use bundled JUCE instead of system JUCE" ON)
option(JUCE_VST3_CAN_REPLACE_PLUGIN "Enable VST3 plugin replacement" ON)
option(JUCE_VST3_CAN_DO_ROLLING_UPDATES "Enable VST3 rolling updates" ON)

# Code quality options
option(ENABLE_ASAN "Enable AddressSanitizer for debugging" OFF)
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)
option(ENABLE_COVERAGE "Enable code coverage analysis" OFF)
option(ENABLE_STATIC_ANALYSIS "Enable static code analysis" OFF)

# Development options
option(ENABLE_PRECOMPILED_HEADERS "Use precompiled headers" ON)
option(ENABLE_CCACHE "Use ccache for faster rebuilds" ON)
option(ENABLE_FAST_DEBUG "Build with fast debug iterations" ON)

# =============================================================================
# CMAKE MODULES AND CONFIGURATION
# =============================================================================

# Include custom CMake modules
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Platform detection and configuration
include(PlatformConfig)

# Compiler-specific configuration
include(CompilerConfig)

# JUCE integration
include(JUCEConfig)

# Dependency management
include(DependencyConfig)

# Performance optimizations
include(PerformanceConfig)

# Static analysis integration
include(StaticAnalysisConfig)

# Package configuration
include(PackageConfig)

# Installation configuration
include(InstallConfig)

# =============================================================================
# BUILD PERFORMANCE SETTINGS
# =============================================================================

# Parallel build configuration
if(NOT DEFINED CMAKE_JOB_POOL_COMPILE)
  math(EXPR MAX_JOBS "${CMAKE_SYSTEM_PROCESSOR_COUNT} * 2")
  if(MAX_JOBS LESS 4)
    set(MAX_JOBS 4)
  endif()
  if(MAX_JOBS GREATER 16)
    set(MAX_JOBS 16)
  endif()
  set(CMAKE_JOB_POOL_COMPILE compile)
  set(CMAKE_JOB_POOL_COMPILE_SIZE ${MAX_JOBS})
  set(CMAKE_JOB_POOL_LINK link)
  set(CMAKE_JOB_POOL_LINK_SIZE 2)
endif()

# Enable incremental linking on MSVC for faster debug builds
if(MSVC)
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /INCREMENTAL")
  endif()
endif()

# =============================================================================
# SOURCE DIRECTORY ORGANIZATION
# =============================================================================

# Define main source directories
set(VITAL_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(VITAL_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(VITAL_RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)
set(VITAL_PLUGINS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/plugins)
set(VITAL_TESTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tests)
set(VITAL_BENCHMARKS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/benchmarks)
set(VITAL_EXAMPLES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/examples)

# Define audio engine directory
set(VITAL_AUDIO_ENGINE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/audio_engine)

# Integration with existing phases
set(VITAL_PHASE1_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../phase1)
set(VITAL_PHASE2_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../phase2)
set(VITAL_PHASE3_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../phase3)
set(VITAL_PHASE4_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../phase4)
set(VITAL_PHASE5_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../phase5)
set(VITAL_PHASE6_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../phase6)
set(VITAL_PHASE7_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../phase7)

# =============================================================================
# MAIN APPLICATION TARGET
# =============================================================================

# Core library target
add_library(VitalCore STATIC
  # Core synthesis engine
  ${VITAL_SRC_DIR}/synthesizer_core.cpp
  ${VITAL_SRC_DIR}/voice_manager.cpp
  ${VITAL_SRC_DIR}/parameter_system.cpp
  ${VITAL_SRC_DIR}/preset_manager.cpp
  ${VITAL_SRC_DIR}/audio_engine.cpp
  
  # DSP modules
  ${VITAL_SRC_DIR}/oscillators.cpp
  ${VITAL_SRC_DIR}/filters.cpp
  ${VITAL_SRC_DIR}/envelopes.cpp
  ${VITAL_SRC_DIR}/effects.cpp
  ${VITAL_SRC_DIR}/modulation.cpp
  
  # Core Audio Engine - Main integration layer
  ${VITAL_AUDIO_ENGINE_DIR}/vital_audio_engine.cpp
  ${VITAL_AUDIO_ENGINE_DIR}/audio_engine_core.cpp
  
  # Core processing modules
  ${VITAL_AUDIO_ENGINE_DIR}/core/
  
  # Oscillator modules (15 advanced oscillator types)
  ${VITAL_AUDIO_ENGINE_DIR}/oscillators/
  ${VITAL_AUDIO_ENGINE_DIR}/new_oscillators.h
  
  # Advanced synthesis modules
  ${VITAL_AUDIO_ENGINE_DIR}/synthesis/
  ${VITAL_AUDIO_ENGINE_DIR}/advanced_synthesis_engine.h
  
  # Effects processing modules
  ${VITAL_AUDIO_ENGINE_DIR}/effects/
  ${VITAL_AUDIO_ENGINE_DIR}/effects_processing_engine.h
  
  # Spectral processing modules
  ${VITAL_AUDIO_ENGINE_DIR}/spectral/
  ${VITAL_AUDIO_ENGINE_DIR}/spectral_warping_engine.h
  
  # Audio quality enhancement modules
  ${VITAL_AUDIO_ENGINE_DIR}/audio_quality/
  ${VITAL_AUDIO_ENGINE_DIR}/audio_quality_processor.h
  
  # Modulation system modules
  ${VITAL_AUDIO_ENGINE_DIR}/modulation/
  ${VITAL_AUDIO_ENGINE_DIR}/modulation_engine.h
  
  # Filter processing modules
  ${VITAL_AUDIO_ENGINE_DIR}/filtering/
  ${VITAL_AUDIO_ENGINE_DIR}/filter_engine.h
  
  # Utility modules
  ${VITAL_AUDIO_ENGINE_DIR}/utility/
  ${VITAL_AUDIO_ENGINE_DIR}/vital_constants.h
  ${VITAL_AUDIO_ENGINE_DIR}/parameter_system.h
  
  # Phase integrations
  ${VITAL_PHASE1_DIR}/juce_modernization/VitalAudioProcessor.cpp
  ${VITAL_PHASE1_DIR}/juce_modernization/VitalSynthesiser.cpp
  ${VITAL_PHASE1_DIR}/juce_modernization/ModernDSP.cpp
  ${VITAL_PHASE1_DIR}/juce_modernization/ModernVoice.cpp
  ${VITAL_PHASE1_DIR}/juce_modernization/SIMDProcessor.cpp
  ${VITAL_PHASE1_DIR}/memory_optimization/memory_pools/memory_pool_impl.cpp
  
  # Phase 2 optimizations
  ${VITAL_PHASE2_DIR}/simd_optimization/simd_vector.cpp
  ${VITAL_PHASE2_DIR}/multithreading/src/ThreadPoolManager.cpp
  ${VITAL_PHASE2_DIR}/multithreading/src/VoiceProcessingManager.cpp
  ${VITAL_PHASE2_DIR}/cache_optimization/src/cache_optimization.cpp
  ${VITAL_PHASE2_DIR}/realtime_optimization/src/callback_optimizer.cpp
  
  # Phase 3 advanced features
  ${VITAL_PHASE3_DIR}/advanced_synthesis/advanced_synthesis_engine.cpp
  ${VITAL_PHASE3_DIR}/effects_processing/effects_processing_engine.cpp
  
  # Phase 4 UI integration
  ${VITAL_PHASE4_DIR}/modern_interface/src/core/ThemeManager.cpp
  ${VITAL_PHASE4_DIR}/modern_interface/src/core/Component.cpp
  ${VITAL_PHASE4_DIR}/visual_enhancements/src/vital_visual_system.cpp
  
  # Phase 5 AI integration
  ${VITAL_PHASE5_DIR}/ai_integration/src/ai_integration_manager.cpp
  ${VITAL_PHASE5_DIR}/cloud_connectivity/src/cloud_connectivity.cpp
  
  # Phase 6 profiling
  ${VITAL_PHASE6_DIR}/advanced_profiling/audio_profiler.cpp
  ${VITAL_PHASE6_DIR}/realtime_monitoring/src/realtime_monitor.cpp
  
  # Phase 7 advanced features
  ${VITAL_PHASE7_DIR}/hardware_acceleration/common/hardware_acceleration_manager.cpp
  ${VITAL_PHASE7_DIR}/environmental_sustainability/carbon_footprint/carbon_tracker.cpp
)

# Target properties
target_include_directories(VitalCore PUBLIC
  $<BUILD_INTERFACE:${VITAL_INCLUDE_DIR}>
  $<BUILD_INTERFACE:${VITAL_PHASE1_DIR}/juce_modernization>
  $<BUILD_INTERFACE:${VITAL_PHASE1_DIR}/cpp20_foundation/include>
  $<BUILD_INTERFACE:${VITAL_PHASE2_DIR}/simd_optimization>
  $<BUILD_INTERFACE:${VITAL_PHASE3_DIR}>
  $<BUILD_INTERFACE:${VITAL_PHASE4_DIR}/modern_interface/include>
  $<BUILD_INTERFACE:${VITAL_AUDIO_ENGINE_DIR}>
  $<BUILD_INTERFACE:${VITAL_AUDIO_ENGINE_DIR}/core>
  $<BUILD_INTERFACE:${VITAL_AUDIO_ENGINE_DIR}/oscillators>
  $<BUILD_INTERFACE:${VITAL_AUDIO_ENGINE_DIR}/synthesis>
  $<BUILD_INTERFACE:${VITAL_AUDIO_ENGINE_DIR}/effects>
  $<BUILD_INTERFACE:${VITAL_AUDIO_ENGINE_DIR}/spectral>
  $<BUILD_INTERFACE:${VITAL_AUDIO_ENGINE_DIR}/audio_quality>
  $<BUILD_INTERFACE:${VITAL_AUDIO_ENGINE_DIR}/modulation>
  $<BUILD_INTERFACE:${VITAL_AUDIO_ENGINE_DIR}/filtering>
  $<BUILD_INTERFACE:${VITAL_AUDIO_ENGINE_DIR}/utility>
  $<INSTALL_INTERFACE:include>
)

target_compile_definitions(VitalCore PUBLIC
  VITAL_VERSION_MAJOR=1
  VITAL_VERSION_MINOR=0
  VITAL_VERSION_PATCH=0
  VITAL_VERSION="${PROJECT_VERSION}"
  JUCE_WEB_BROWSER=0
  JUCE_VST3_CAN_REPLACE_PLUGIN=1
  JUCE_VST3_CAN_DO_ROLLING_UPDATES=1
)

# Apply performance optimizations
if(ENABLE_SIMD_OPTIMIZATIONS)
  target_compile_definitions(VitalCore PUBLIC VITAL_SIMD_OPTIMIZATIONS=1)
endif()

if(ENABLE_MULTITHREADING)
  target_compile_definitions(VitalCore PUBLIC VITAL_MULTITHREADING=1)
endif()

# =============================================================================
# STANDALONE APPLICATION TARGET
# =============================================================================

if(BUILD_STANDALONE)
  add_executable(VitalStandalone
    ${VITAL_SRC_DIR}/main_standalone.cpp
    ${VITAL_SRC_DIR}/standalone_window.cpp
    ${VITAL_RESOURCES_DIR}/resources.cpp
  )
  
  target_link_libraries(VitalStandalone PRIVATE VitalCore)
  target_compile_definitions(VitalStandalone PRIVATE VITAL_STANDALONE=1)
  
  if(MSVC)
    target_compile_options(VitalStandalone PRIVATE /W4 /permissive-)
  else()
    target_compile_options(VitalStandalone PRIVATE -Wall -Wextra -pedantic)
  endif()
endif()

# =============================================================================
# PLUGIN TARGETS
# =============================================================================

if(BUILD_PLUGIN)
  if(USE_JUCE_CMAKE)
    # VST3 Plugin
    if(JUCE_VST3_CAN_REPLACE_PLUGIN)
      juce_add_plugin(VitalVST3
        COMPANY_NAME "The Vital Team"
        IS_SYNTH TRUE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_AFTER_EDITING FALSE
        FORMATS AU VST3
        PRODUCT_NAME "Vital Synthesizer"
      )
      
      target_sources(VitalVST3 PRIVATE
        ${VITAL_SRC_DIR}/plugin_processor.cpp
        ${VITAL_SRC_DIR}/plugin_editor.cpp
        ${VITAL_PHASE1_DIR}/juce_modernization/VitalAudioProcessor.cpp
        ${VITAL_PHASE1_DIR}/juce_modernization/VitalAudioProcessorEditor.cpp
      )
      
      target_link_libraries(VitalVST3 PRIVATE
        VitalCore
        PUBLIC
          juce::juce_audio_basics
          juce::juce_audio_devices
          juce::juce_audio_formats
          juce::juce_audio_plugin_client
          juce::juce_audio_processors
          juce::juce_audio_utils
          juce::juce_recommended_config_flags
          juce::juce_recommended_lto_flags
          juce::juce_recommended_warning_flags
      )
      
      # Additional JUCE modules for advanced audio engine features
      target_link_libraries(VitalVST3 PRIVATE
        PUBLIC
          juce::juce_dsp
          juce::juce_opengl
          juce::juce_cryptography
      )
      
      juce_generate_juce_header(VitalVST3)
    endif()
    
    # AU Plugin for macOS
    if(VITAL_PLATFORM_MACOS)
      juce_add_plugin(VitalAU
        COMPANY_NAME "The Vital Team"
        IS_SYNTH TRUE
        NEEDS_MIDI_INPUT TRUE
        NEEDS_MIDI_OUTPUT FALSE
        EDITOR_WANTS_KEYBOARD_FOCUS FALSE
        COPY_AFTER_EDITING FALSE
        FORMATS AU
        PRODUCT_NAME "Vital Synthesizer"
      )
      
      target_sources(VitalAU PRIVATE
        ${VITAL_SRC_DIR}/plugin_processor.cpp
        ${VITAL_SRC_DIR}/plugin_editor.cpp
        ${VITAL_PHASE1_DIR}/juce_modernization/VitalAudioProcessor.cpp
        ${VITAL_PHASE1_DIR}/juce_modernization/VitalAudioProcessorEditor.cpp
      )
      
      target_link_libraries(VitalAU PRIVATE
        VitalCore
        PUBLIC
          juce::juce_audio_basics
          juce::juce_audio_devices
          juce::juce_audio_formats
          juce::juce_audio_plugin_client
          juce::juce_audio_processors
          juce::juce_audio_utils
          juce::juce_dsp
          juce::juce_opengl
          juce::juce_cryptography
      )
    endif()
  endif()
endif()

# =============================================================================
# TEST TARGETS
# =============================================================================

if(BUILD_TESTS)
  enable_testing()
  
  # Core tests
  add_executable(VitalTests
    ${VITAL_TESTS_DIR}/test_main.cpp
    ${VITAL_TESTS_DIR}/test_synthesizer.cpp
    ${VITAL_TESTS_DIR}/test_dsp.cpp
    ${VITAL_TESTS_DIR}/test_parameters.cpp
    ${VITAL_PHASE1_DIR}/testing_framework/src/sound_engine_test.cpp
    ${VITAL_PHASE6_DIR}/realtime_monitoring/tests/monitoring_test.cpp
  )
  
  target_link_libraries(VitalTests PRIVATE VitalCore)
  target_include_directories(VitalTests PRIVATE ${VITAL_TESTS_DIR})
  
  if(CPM_Catch2_FOUND)
    target_link_libraries(VitalTests PRIVATE Catch2::Catch2)
  endif()
  
  # Performance tests
  add_executable(VitalPerfTests
    ${VITAL_BENCHMARKS_DIR}/benchmark_main.cpp
    ${VITAL_BENCHMARKS_DIR}/synth_performance.cpp
    ${VITAL_PHASE1_DIR}/cpp20_foundation/benchmarks/benchmark_dsp_utils.cpp
  )
  
  target_link_libraries(VitalPerfTests PRIVATE VitalCore)
  
  add_test(NAME VitalCoreTests COMMAND VitalTests)
  add_test(NAME VitalPerfTests COMMAND VitalPerfTests)
endif()

# =============================================================================
# EXAMPLE APPLICATIONS
# =============================================================================

if(BUILD_EXAMPLES)
  # Basic synthesizer example
  add_executable(VitalBasicExample
    ${VITAL_EXAMPLES_DIR}/basic_synth.cpp
  )
  target_link_libraries(VitalBasicExample PRIVATE VitalCore)
  
  # Advanced features example
  add_executable(VitalAdvancedExample
    ${VITAL_EXAMPLES_DIR}/advanced_features.cpp
  )
  target_link_libraries(VitalAdvancedExample PRIVATE VitalCore)
  
  # Performance demo
  add_executable(VitalPerfDemo
    ${VITAL_EXAMPLES_DIR}/performance_demo.cpp
  )
  target_link_libraries(VitalPerfDemo PRIVATE VitalCore)
endif()

# =============================================================================
# CUSTOM TARGETS AND UTILITIES
# =============================================================================

# Static analysis target
if(ENABLE_STATIC_ANALYSIS)
  add_custom_target(static-analysis
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/scripts/static_analysis.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Running static code analysis"
  )
endif()

# Code formatting target
add_custom_target(format-code
  COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/scripts/format_code.py
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Formatting source code"
)

# Documentation generation target
if(BUILD_DOCS)
  add_custom_target(docs
    COMMAND python3 ${CMAKE_CURRENT_SOURCE_DIR}/scripts/generate_docs.py
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMENT "Generating documentation"
  )
endif()

# =============================================================================
# INSTALLATION
# =============================================================================

# Install the core library
install(TARGETS VitalCore
  EXPORT VitalTargets
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
  INCLUDES DESTINATION include
)

# Install headers
install(DIRECTORY ${VITAL_INCLUDE_DIR}/
  DESTINATION include
  FILES_MATCHING PATTERN "*.h"
)

# Install resources
if(EXISTS ${VITAL_RESOURCES_DIR})
  install(DIRECTORY ${VITAL_RESOURCES_DIR}/
    DESTINATION share/vital
    FILES_MATCHING PATTERN "*"
  )
endif()

# Install plugins
if(BUILD_PLUGIN)
  if(EXISTS ${CMAKE_BINARY_DIR}/VitalVST3_artefacts)
    install(DIRECTORY ${CMAKE_BINARY_DIR}/VitalVST3_artefacts/
      DESTINATION lib/vst3
      FILES_MATCHING PATTERN "*.vst3"
    )
  endif()
  
  if(EXISTS ${CMAKE_BINARY_DIR}/VitalAU_artefacts)
    install(DIRECTORY ${CMAKE_BINARY_DIR}/VitalAU_artefacts/
      DESTINATION Library/Audio/Plug-Ins/Components
      FILES_MATCHING PATTERN "*.component"
    )
  endif()
endif()

# =============================================================================
# PACKAGE CONFIGURATION
# =============================================================================

# Export configuration
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/VitalConfigVersion.cmake"
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

install(EXPORT VitalTargets
  FILE VitalTargets.cmake
  NAMESPACE Vital::
  DESTINATION lib/cmake/Vital
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/VitalConfigVersion.cmake"
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/VitalConfig.cmake"
  DESTINATION lib/cmake/Vital
)

# =============================================================================
# BUILD SUMMARY
# =============================================================================

message(STATUS "")
message(STATUS "============================================================")
message(STATUS "  Vital Synthesizer Build Configuration")
message(STATUS "============================================================")
message(STATUS "CMake Version:           ${CMAKE_VERSION}")
message(STATUS "Build Type:              ${CMAKE_BUILD_TYPE}")
message(STATUS "Platform:               ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "Compiler:               ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C++ Standard:           ${CMAKE_CXX_STANDARD}")
message(STATUS "")
message(STATUS "Build Options:")
message(STATUS "  Shared Libraries:      ${BUILD_SHARED_LIBS}")
message(STATUS "  Tests:                 ${BUILD_TESTS}")
message(STATUS "  Benchmarks:            ${BUILD_BENCHMARKS}")
message(STATUS "  Plugin:                ${BUILD_PLUGIN}")
message(STATUS "  Standalone:            ${BUILD_STANDALONE}")
message(STATUS "  Examples:              ${BUILD_EXAMPLES}")
message(STATUS "")
message(STATUS "Performance Optimizations:")
message(STATUS "  SIMD:                  ${ENABLE_SIMD_OPTIMIZATIONS}")
message(STATUS "  Multithreading:        ${ENABLE_MULTITHREADING}")
message(STATUS "  Vectorization:         ${ENABLE_VECTORIZATION}")
message(STATUS "  LTO:                   ${ENABLE_LTO}")
message(STATUS "  PGO:                   ${ENABLE_PGO}")
message(STATUS "")
message(STATUS "JUCE Integration:")
message(STATUS "  JUCE CMake:            ${USE_JUCE_CMAKE}")
message(STATUS "  Bundled JUCE:          ${USE_BUNDLED_JUCE}")
message(STATUS "  VST3 Plugin:           ${BUILD_PLUGIN}")
message(STATUS "")
message(STATUS "Audio Engine Features:")
message(STATUS "  Core Audio Engine:     Built-in")
message(STATUS "  Advanced Oscillators:  15 types")
message(STATUS "  Advanced Synthesis:    Modal, Physical, Granular")
message(STATUS "  Effects Processing:    Convolution, Multi-band, Adaptive")
message(STATUS "  Spectral Warping:      FFT, Phase vocoding, Harmonic")
message(STATUS "  Audio Quality:         Oversampling, Dithering, Anti-aliasing")
message(STATUS "  Modulation System:     LFO, Envelopes, Routing matrix")
message(STATUS "")
message(STATUS "Code Quality:")
message(STATUS "  AddressSanitizer:      ${ENABLE_ASAN}")
message(STATUS "  UndefinedBehaviorSan:  ${ENABLE_UBSAN}")
message(STATUS "  ThreadSanitizer:       ${ENABLE_TSAN}")
message(STATUS "  Coverage:              ${ENABLE_COVERAGE}")
message(STATUS "  Static Analysis:       ${ENABLE_STATIC_ANALYSIS}")
message(STATUS "")
message(STATUS "Parallel Build Jobs:    ${CMAKE_JOB_POOL_COMPILE_SIZE}")
message(STATUS "============================================================")
message(STATUS "")