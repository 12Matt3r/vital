# JUCE Framework Integration Configuration
# Handles JUCE library integration, plugin building, and audio framework setup

# JUCE version requirements
set(JUCE_MIN_VERSION 7.0)
set(JUCE_RECOMMENDED_VERSION 7.0.12)

# JUCE discovery and setup
if(USE_JUCE_CMAKE)
  if(USE_BUNDLED_JUCE)
    # Use bundled JUCE (recommended for cross-platform consistency)
    message(STATUS "Using bundled JUCE from ${CMAKE_CURRENT_SOURCE_DIR}/external/juce")
    
    # Check for JUCE in external directory
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/external/juce/JUCEConfig.cmake)
      list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/external/juce/cmake")
      set(JUCE_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/juce")
    else()
      # Download JUCE using FetchContent
      include(FetchContent)
      
      FetchContent_Declare(
        juce
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG origin/master
        GIT_PROGRESS TRUE
      )
      
      set(JUCE_BUILD_EXAMPLES OFF CACHE BOOL "Build JUCE examples")
      set(JUCE_BUILD_DOCS OFF CACHE BOOL "Build JUCE documentation")
      set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build shared libraries")
      
      FetchContent_MakeAvailable(juce)
      set(JUCE_ROOT_DIR ${juce_SOURCE_DIR})
    endif()
    
  else()
    # Use system-installed JUCE
    find_package(JUCE CONFIG QUIET)
    
    if(NOT JUCE_FOUND)
      message(WARNING "JUCE not found. Trying to find JUCEConfig.cmake...")
      
      # Try to find JUCEConfig.cmake in common locations
      find_path(JUCE_CONFIG_DIR
        NAMES JUCEConfig.cmake
        HINTS
          /usr/local/lib/cmake/JUCE
          /usr/lib/cmake/JUCE
          ~/Library/Application\ Support/JUCE/cmake
        DOC "JUCE CMake configuration directory"
      )
      
      if(JUCE_CONFIG_DIR)
        list(APPEND CMAKE_MODULE_PATH ${JUCE_CONFIG_DIR})
        find_package(JUCE CONFIG REQUIRED)
        message(STATUS "Found JUCE at: ${JUCE_CONFIG_DIR}")
      else()
        message(FATAL_ERROR "JUCE not found. Please install JUCE or enable USE_BUNDLED_JUCE.")
      endif()
    endif()
  endif()
  
  # Include JUCE module
  if(EXISTS ${JUCE_ROOT_DIR}/cmake/JUCEConfig.cmake)
    include(${JUCE_ROOT_DIR}/cmake/JUCEConfig.cmake)
  endif()
  
  # Verify JUCE version
  if(JUCE_VERSION AND JUCE_MIN_VERSION)
    if(JUCE_VERSION VERSION_LESS ${JUCE_MIN_VERSION})
      message(WARNING "JUCE version ${JUCE_VERSION} is older than recommended ${JUCE_RECOMMENDED_VERSION}")
    endif()
  endif()
endif()

# JUCE-specific compiler flags
if(JUCE_FOUND OR JUCE_ROOT_DIR)
  # Audio processing optimizations
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # Use -ffast-math for audio processing (acceptable for plugins)
    target_compile_options(VitalCore PUBLIC -ffast-math)
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # Enable floating point optimizations on MSVC
    target_compile_definitions(VitalCore PUBLIC /fp:fast)
  endif()
  
  # JUCE module integration
  if(USE_JUCE_CMAKE)
    # Audio modules
    find_package(JUCE REQUIRED CONFIG
      COMPONENTS
        juce_audio_basics
        juce_audio_devices
        juce_audio_formats
        juce_audio_processors
        juce_audio_utils
        juce_audio_widgets
    )
    
    # Plugin client module (for VST3/AU building)
    find_package(JUCE REQUIRED CONFIG
      COMPONENTS juce_audio_plugin_client
    )
    
    # Optional modules based on platform
    if(VITAL_PLATFORM_WINDOWS AND ENABLE_WINDOWS_ASIO)
      find_package(JUCE REQUIRED CONFIG COMPONENTS juce_audio_host)
    endif()
    
    if(VITAL_PLATFORM_MACOS AND ENABLE_MACOS_FRAMEWORK)
      find_package(JUCE REQUIRED CONFIG COMPONENTS juce_opengl)
    endif()
  endif()
  
  # JUCE-specific definitions
  target_compile_definitions(VitalCore PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_VST3_CAN_REPLACE_PLUGIN=1
    JUCE_VST3_CAN_DO_ROLLING_UPDATES=1
    JUCE_ENABLE_REPAINT_WHILE_AUDIO_IS_PLAYING=1
  )
endif()

# Plugin-specific configurations
if(BUILD_PLUGIN)
  # VST3 Plugin settings
  if(JUCE_VST3_CAN_REPLACE_PLUGIN)
    set(VITAL_VST3_FOLDER_PATH "${CMAKE_BINARY_DIR}/VST3")
    
    # VST3 SDK paths
    if(NOT DEFINED VST3_SDK_PATH)
      # Common VST3 SDK locations
      find_path(VST3_SDK_PATH
        NAMES "VST3SDK"
        HINTS
          "/usr/include/vst3sdk"
          "/usr/local/include/vst3sdk"
          "C:/VST3 SDK"
          "C:/Program Files/VST3 SDK"
          "C:/SDKs/VST3SDK"
      )
    endif()
    
    if(VST3_SDK_PATH)
      target_include_directories(VitalCore PRIVATE "${VST3_SDK_PATH}")
      message(STATUS "Found VST3 SDK at: ${VST3_SDK_PATH}")
    else()
      message(WARNING "VST3 SDK not found. VST3 plugin building may fail.")
    endif()
  endif()
  
  # Audio Unit (AU) Plugin settings for macOS
  if(VITAL_PLATFORM_MACOS)
    # AU SDK is typically provided by Xcode
    find_library(AUDIO_TOOLBOX_LIBRARY AudioToolbox)
    find_library(CORE_AUDIO_LIBRARY CoreAudio)
    find_library(AUDIO_UNIT_LIBRARY AudioUnit)
    
    if(AUDIO_TOOLBOX_LIBRARY AND CORE_AUDIO_LIBRARY AND AUDIO_UNIT_LIBRARY)
      target_link_libraries(VitalCore PUBLIC
        "${AUDIO_TOOLBOX_LIBRARY}"
        "${CORE_AUDIO_LIBRARY}"
        "${AUDIO_UNIT_LIBRARY}"
      )
      message(STATUS "Found AU frameworks")
    endif()
  endif()
endif()

# Audio driver configuration
set(VITAL_AUDIO_DRIVERS
  JUCE
  PortAudio
  ASIO
  CoreAudio
  WASAPI
  DirectSound
  ALSA
  PulseAudio
  JACK
  CORS
)

# Platform-specific audio backend selection
if(VITAL_PLATFORM_WINDOWS)
  if(ENABLE_WINDOWS_ASIO)
    set(VITAL_PREFERRED_AUDIO_BACKEND "ASIO")
    target_compile_definitions(VitalCore PUBLIC VITAL_ENABLE_ASIO=1)
  else()
    set(VITAL_PREFERRED_AUDIO_BACKEND "WASAPI")
  endif()
elseif(VITAL_PLATFORM_MACOS)
  set(VITAL_PREFERRED_AUDIO_BACKEND "CoreAudio")
elseif(VITAL_PLATFORM_LINUX)
  if(ENABLE_LINUX_PULSE)
    set(VITAL_PREFERRED_AUDIO_BACKEND "PulseAudio")
    find_package(PulseAudio QUIET)
    if(PulseAudio_FOUND)
      target_link_libraries(VitalCore PUBLIC PulseAudio::PulseAudio)
    endif()
  else()
    set(VITAL_PREFERRED_AUDIO_BACKEND "ALSA")
    find_package(ALSA QUIET)
    if(ALSA_FOUND)
      target_link_libraries(VitalCore PUBLIC ALSA::ALSA)
    endif()
  endif()
endif()

# MIDI support
target_compile_definitions(VitalCore PUBLIC
  JUCE_MIDI_INPUT=1
  JUCE_MIDI_OUTPUT=1
)

# Platform-specific JUCE optimizations
if(CMAKE_BUILD_TYPE STREQUAL "Release")
  if(VITAL_PLATFORM_WINDOWS)
    # MSVC-specific JUCE optimizations
    target_compile_definitions(VitalCore PUBLIC JUCE_MSVC_DISABLE_ALLwarnings=1)
  elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # GCC/Clang JUCE optimizations
    target_compile_options(VitalCore PRIVATE -Wno-unused-parameter)
    target_compile_options(VitalCore PRIVATE -Wno-attributes)
  endif()
endif()

# Plugin export settings
set(VITAL_PLUGIN_MANUFACTURER_CODE_VST3 "VtS1")  # Vital Synthesizer
set(VITAL_PLUGIN_MANUFACTURER_CODE_AU "VtS1")
set(VITAL_PLUGIN_CODE "VtS1")

target_compile_definitions(VitalCore PUBLIC
  JUCER_VS2022_78A508D6=1
  JUCE_WEB_BROWSER=0
  JUCE_VST3_CAN_REPLACE_PLUGIN=1
  JUCE_VST3_CAN_DO_ROLLING_UPDATES=1
  JUCE_VST3_NUM_PROGRAMS=128
  JUCE_VST3_NUM_PARAMETERS=256
  JUCE_VST3_NUM_INPUTS=2
  JUCE_VST3_NUM_OUTPUTS=2
)

# Export JUCE configuration for targets
set(JUCE_CONFIGURED ON CACHE INTERNAL "JUCE configuration complete")
mark_as_advanced(JUCE_ROOT_DIR VST3_SDK_PATH)

message(STATUS "JUCE Integration Status:")
if(JUCE_FOUND OR JUCE_ROOT_DIR)
  message(STATUS "  JUCE Version:           ${JUCE_VERSION}")
  message(STATUS "  JUCE Path:             ${JUCE_ROOT_DIR}")
  message(STATUS "  Preferred Audio:       ${VITAL_PREFERRED_AUDIO_BACKEND}")
  if(VST3_SDK_PATH)
    message(STATUS "  VST3 SDK:              ${VST3_SDK_PATH}")
  endif()
  message(STATUS "  MIDI Support:          Enabled")
else()
  message(STATUS "  JUCE Status:          NOT FOUND")
endif()