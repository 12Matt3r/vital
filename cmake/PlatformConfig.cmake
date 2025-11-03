# Platform Configuration for Cross-Platform Support
# Handles different operating systems, architectures, and platform-specific optimizations

# =============================================================================
# OPERATING SYSTEM DETECTION
# =============================================================================

# Primary platform detection
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(VITAL_PLATFORM_LINUX ON)
  set(VITAL_PLATFORM_UNIX ON)
  set(VITAL_PLATFORM_NAME "Linux")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(VITAL_PLATFORM_MACOS ON)
  set(VITAL_PLATFORM_UNIX ON)
  set(VITAL_PLATFORM_NAME "macOS")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(VITAL_PLATFORM_WINDOWS ON)
  set(VITAL_PLATFORM_NAME "Windows")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Emscripten")
  set(VITAL_PLATFORM_WASM ON)
  set(VITAL_PLATFORM_NAME "WebAssembly")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
  set(VITAL_PLATFORM_ANDROID ON)
  set(VITAL_PLATFORM_UNIX ON)
  set(VITAL_PLATFORM_NAME "Android")
elseif(CMAKE_SYSTEM_NAME STREQUAL "iOS")
  set(VITAL_PLATFORM_IOS ON)
  set(VITAL_PLATFORM_NAME "iOS")
endif()

# =============================================================================
# ARCHITECTURE DETECTION
# =============================================================================

# CPU architecture detection
if(CMAKE_SYSTEM_PROCESSOR MATCHES "x86_64|AMD64")
  set(VITAL_ARCH_X86_64 ON)
  set(VITAL_ARCH_BITS 64)
  set(VITAL_ARCH_NAME "x86_64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
  set(VITAL_ARCH_ARM64 ON)
  set(VITAL_ARCH_BITS 64)
  set(VITAL_ARCH_NAME "ARM64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "armv7|armv7l")
  set(VITAL_ARCH_ARM ON)
  set(VITAL_ARCH_BITS 32)
  set(VITAL_ARCH_NAME "ARMv7")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "armv6|armv6l")
  set(VITAL_ARCH_ARM ON)
  set(VITAL_ARCH_BITS 32)
  set(VITAL_ARCH_NAME "ARMv6")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "wasm32")
  set(VITAL_ARCH_WASM ON)
  set(VITAL_ARCH_BITS 32)
  set(VITAL_ARCH_NAME "WebAssembly")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "wasm64")
  set(VITAL_ARCH_WASM ON)
  set(VITAL_ARCH_BITS 64)
  set(VITAL_ARCH_NAME "WebAssembly")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "riscv64")
  set(VITAL_ARCH_RISCV64 ON)
  set(VITAL_ARCH_BITS 64)
  set(VITAL_ARCH_NAME "RISC-V64")
endif()

# Cross-compilation architecture settings
if(DEFINED CMAKE_SYSTEM_PROCESSOR)
  message(STATUS "Detected CPU architecture: ${CMAKE_SYSTEM_PROCESSOR}")
endif()

# =============================================================================
# PLATFORM-SPECIFIC COMPILER SETTINGS
# =============================================================================

# Linux-specific settings
if(VITAL_PLATFORM_LINUX)
  set(CMAKE_CXX_FLAGS_INIT "-Wall -Wextra -pedantic")
  set(CMAKE_EXE_LINKER_FLAGS_INIT "-Wl,--as-needed -Wl,--gc-sections")
  
  # Dynamic linking preferences
  set(CMAKE_SHARED_LIBRARY_SUFFIX ".so")
  set(CMAKE_SHARED_MODULE_SUFFIX ".so")
  
  # Position-independent code for shared libraries
  set(CMAKE_POSITION_INDEPENDENT_CODE ON)
  
  # Audio system libraries
  set(VITAL_PLATFORM_LIBS pthread dl m)
  
  # ALSA support
  find_package(ALSA QUIET)
  if(ALSA_FOUND)
    list(APPEND VITAL_PLATFORM_LIBS ALSA::ALSA)
    target_compile_definitions(VitalCore PUBLIC VITAL_ALSA_SUPPORT=1)
  endif()
  
  # PulseAudio support
  if(ENABLE_LINUX_PULSE)
    find_package(PulseAudio QUIET)
    if(PulseAudio_FOUND)
      list(APPEND VITAL_PLATFORM_LIBS PulseAudio::PulseAudio)
      target_compile_definitions(VitalCore PUBLIC VITAL_PULSEAUDIO_SUPPORT=1)
    endif()
  endif()
  
  # JACK Audio Connection Kit support
  find_package(JACK QUIET)
  if(JACK_FOUND)
    list(APPEND VATFORM_LIBS JACK:: JACK)
    target_compile_definitions(VitalCore PUBLIC VITAL_JACK_SUPPORT=1)
  endif()
  
  # Linux-specific optimizations
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(VitalCore PRIVATE
      -march=native
      -mtune=native
      -fomit-frame-pointer
    )
  endif()

# macOS-specific settings
elseif(VITAL_PLATFORM_MACOS)
  set(CMAKE_CXX_FLAGS_INIT "-Wall -Wextra -pedantic")
  
  # Multi-architecture build support
  set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build architectures for macOS")
  
  # Framework linking
  set(CMAKE_SHARED_LIBRARY_SUFFIX ".dylib")
  set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum macOS version")
  
  # macOS-specific frameworks
  find_library(CORE_AUDIO_FRAMEWORK CoreAudio)
  find_library(AUDIO_TOOLBOX_FRAMEWORK AudioToolbox)
  find_library(AUDIO_UNIT_FRAMEWORK AudioUnit)
  find_library(CORE_MIDI_FRAMEWORK CoreMIDI)
  find_library(OPENGL_LIBRARY OpenGL)
  find_library(QUARTZCORE_LIBRARY QuartzCore)
  find_library(METAL_LIBRARY Metal)
  find_library(METAL_PERFORMANCE_SHADERS_LIBRARY MetalPerformanceShaders)
  find_library(ACCELERATE_LIBRARY Accelerate)
  
  # Framework availability checks
  if(CMAKE_OSX_DEPLOYMENT_TARGET VERSION_GREATER "10.14")
    set(VITAL_HAS_METAL ON)
    set(VITAL_HAS_MPS ON)
  endif()
  
  if(CMAKE_OSX_DEPLOYMENT_TARGET VERSION_GREATER "10.13")
    set(VITAL_HAS_ACCELERATE ON)
  endif()
  
  # Link frameworks
  if(CORE_AUDIO_FRAMEWORK)
    target_link_libraries(VitalCore PUBLIC "${CORE_AUDIO_FRAMEWORK}")
  endif()
  if(AUDIO_TOOLBOX_FRAMEWORK)
    target_link_libraries(VitalCore PUBLIC "${AUDIO_TOOLBOX_FRAMEWORK}")
  endif()
  if(AUDIO_UNIT_FRAMEWORK)
    target_link_libraries(VitalCore PUBLIC "${AUDIO_UNIT_FRAMEWORK}")
  endif()
  if(CORE_MIDI_FRAMEWORK)
    target_link_libraries(VitalCore PUBLIC "${CORE_MIDI_FRAMEWORK}")
  endif()
  if(METAL_LIBRARY)
    target_link_libraries(VitalCore PUBLIC "${METAL_LIBRARY}")
  endif()
  if(METAL_PERFORMANCE_SHADERS_LIBRARY)
    target_link_libraries(VitalCore PUBLIC "${METAL_PERFORMANCE_SHADERS_LIBRARY}")
  endif()
  if(ACCELERATE_LIBRARY)
    target_link_libraries(VitalCore PUBLIC "${ACCELERATE_LIBRARY}")
  endif()
  
  # macOS-specific compiler flags
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(VitalCore PRIVATE -flto)
    target_link_options(VitalCore PRIVATE -flto)
  endif()
  
  # Bundle resources for macOS applications
  set_target_properties(VitalStandalone PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_BUNDLE_NAME "Vital Synthesizer"
    MACOSX_BUNDLE_GUI_IDENTIFIER "com.vital.synthesizer"
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/resources/macos/Info.plist
  )

# Windows-specific settings
elseif(VITAL_PLATFORM_WINDOWS)
  set(CMAKE_CXX_FLAGS_INIT "/W4 /MP /permissive-")
  set(CMAKE_EXE_LINKER_FLAGS_INIT "/SUBSYSTEM:WINDOWS")
  
  # Default to static runtime for better compatibility
  if(NOT CMAKE_MSVC_RUNTIME_LIBRARY)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>" CACHE STRING "MSVC runtime library")
  endif()
  
  # Unicode support
  add_definitions(-DUNICODE -D_UNICODE)
  add_definitions(-DWIN32_LEAN_AND_MEAN -DNOMINMAX)
  
  # Windows-specific libraries
  list(APPEND VITAL_PLATFORM_LIBS
    winmm
    kernel32
    user32
    gdi32
    advapi32
    shell32
    ole32
    oleaut32
    uuid
  )
  
  # Windows Audio APIs
  if(ENABLE_WINDOWS_ASIO)
    find_package(ASIO QUIET)
    if(ASIO_FOUND)
      target_compile_definitions(VitalCore PUBLIC VITAL_ASIO_SUPPORT=1)
      target_include_directories(VitalCore PRIVATE ${ASIO_INCLUDE_DIR})
    endif()
  endif()
  
  # WASAPI and DirectSound are included with Windows SDK
  target_compile_definitions(VitalCore PUBLIC
    VITAL_WASAPI_SUPPORT=1
    VITAL_DIRECTSOUND_SUPPORT=1
  )
  
  # Windows-specific optimizations
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(VitalCore PRIVATE
      /O2
      /Oy-
      /Ob2
      /GL
      /Gm-
      /Zc:inline
      /fp:fast
      /permissive-
    )
    target_link_options(VitalCore PRIVATE
      /LTCG:incremental
      /SUBSYSTEM:WINDOWS
      /OPT:REF
      /OPT:ICF
    )
  elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    target_compile_options(VitalCore PRIVATE /O2 /DNDEBUG /Zi)
    target_link_options(VitalCore PRIVATE /DEBUG:FULL /LTCG)
  endif()
  
  # Enable 64-bit optimizations
  if(VITAL_ARCH_X86_64)
    target_compile_options(VitalCore PRIVATE /arch:AVX2)
  endif()

# WebAssembly (Emscripten) settings
elseif(VITAL_PLATFORM_WASM)
  set(CMAKE_CXX_FLAGS_INIT "-Wall -Wextra -O3 -s MODULARIZE=1 -s EXPORT_NAME=VitalEngine")
  set(CMAKE_EXE_LINKER_FLAGS_INIT
    "-s ALLOW_MEMORY_GROWTH=1"
    "-s EXPORTED_FUNCTIONS=_main,_malloc,_free"
    "-s EXPORTED_RUNTIME_METHODS=ccall,cwrap"
    "-s WASM=1"
  )
  
  # WebAssembly-specific optimizations
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(VitalCore PRIVATE -O3 -flto)
    target_link_options(VitalCore PRIVATE -O3 -flto)
  endif()
  
  # WebAssembly-specific defines
  target_compile_definitions(VitalCore PUBLIC
    __EMSCRIPTEN__
    EMSCRIPTEN
    VITAL_WEBASSEMBLY=1
  )
  
  # Disable some features for web
  target_compile_definitions(VitalCore PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_VST3_CAN_REPLACE_PLUGIN=0
  )

# Android-specific settings
elseif(VITAL_PLATFORM_ANDROID)
  set(CMAKE_CXX_FLAGS_INIT "-Wall -Wextra -pedantic")
  
  # Android-specific optimizations
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(VitalCore PRIVATE -flto)
    target_link_options(VitalCore PRIVATE -flto)
  endif()
  
  # Android audio
  target_compile_definitions(VitalCore PUBLIC
    VITAL_OPENSL_ES=1
    VITAL_ANDROID_AUDIO=1
  )

# iOS-specific settings
elseif(VITAL_PLATFORM_IOS)
  set(CMAKE_CXX_FLAGS_INIT "-Wall -Wextra -pedantic")
  
  # iOS-specific framework linking
  set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "Build architectures for iOS")
  set(CMAKE_OSX_DEPLOYMENT_TARGET "13.0" CACHE STRING "Minimum iOS version")
  
  # iOS frameworks
  find_library(CORE_AUDIO_FRAMEWORK CoreAudio)
  find_library(AUDIO_TOOLBOX_FRAMEWORK AudioToolbox)
  find_library(AUDIO_UNIT_FRAMEWORK AudioUnit)
  find_library(OPENGLES_LIBRARY OpenGLES)
  
  if(CORE_AUDIO_FRAMEWORK)
    target_link_libraries(VitalCore PUBLIC "${CORE_AUDIO_FRAMEWORK}")
  endif()
  if(AUDIO_TOOLBOX_FRAMEWORK)
    target_link_libraries(VitalCore PUBLIC "${AUDIO_TOOLBOX_FRAMEWORK}")
  endif()
  if(AUDIO_UNIT_FRAMEWORK)
    target_link_libraries(VitalCore PUBLIC "${AUDIO_UNIT_FRAMEWORK}")
  endif()
  
  # iOS bundle settings
  set_target_properties(VitalStandalone PROPERTIES
    MACOSX_BUNDLE TRUE
    IOS_INSTALL_COMBINED TRUE
    XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET ${CMAKE_OSX_DEPLOYMENT_TARGET}
  )
endif()

# =============================================================================
# ARCHITECTURE-SPECIFIC OPTIMIZATIONS
# =============================================================================

# 64-bit architecture optimizations
if(VITAL_ARCH_BITS EQUAL 64)
  target_compile_definitions(VitalCore PUBLIC VITAL_64BIT=1)
  
  # Prefer 64-bit system libraries where available
  if(VITAL_PLATFORM_LINUX)
    target_link_options(VitalCore PRIVATE -L/usr/lib64)
  endif()
endif()

# SIMD availability based on architecture
if(VITAL_ARCH_X86_64 OR VITAL_ARCH_ARM64)
  set(VITAL_HAS_SIMD ON)
  target_compile_definitions(VitalCore PUBLIC VITAL_SIMD_CAPABLE=1)
endif()

# =============================================================================
# POSITION-INDEPENDENT CODE
# =============================================================================

# Enable PIC/PIC for all platforms (required for shared libraries)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# =============================================================================
# PLATFORM-SPECIFIC LIBRARIES AND INCLUDES
# =============================================================================

# Apply platform-specific libraries
if(VITAL_PLATFORM_LIBS)
  target_link_libraries(VitalCore PUBLIC ${VITAL_PLATFORM_LIBS})
endif()

# =============================================================================
# THREADING MODEL CONFIGURATION
# =============================================================================

# Enable threading support on all platforms
find_package(Threads REQUIRED)
target_link_libraries(VitalCore PUBLIC Threads::Threads)

# Thread-local storage
target_compile_definitions(VitalCore PUBLIC
  VITAL_THREADS_ENABLED=1
  $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:VITAL_TLS_MODEL="initial-exec">
  $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:VITAL_TLS_MODEL="">
)

# =============================================================================
# PLATFORM PACKAGING SETTINGS
# =============================================================================

# Library versioning
set(VITAL_LIBRARY_VERSION ${PROJECT_VERSION})

# Shared library version scheme (for Unix-like systems)
if(VITAL_PLATFORM_UNIX)
  set(VITAL_LIBRARY_SOVERSION ${PROJECT_VERSION_MAJOR})
  set(VITAL_LIBRARY_CURRENT ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})
endif()

# =============================================================================
# EXPORT PLATFORM CONFIGURATION
# =============================================================================

# Export platform information as cache variables
set(VITAL_PLATFORM_NAME ${VITAL_PLATFORM_NAME} CACHE INTERNAL "Platform name")
set(VITAL_ARCH_NAME ${VITAL_ARCH_NAME} CACHE INTERNAL "Architecture name")
set(VITAL_ARCH_BITS ${VITAL_ARCH_BITS} CACHE INTERNAL "Architecture bits")

# Mark advanced variables
mark_as_advanced(
  CMAKE_OSX_ARCHITECTURES
  CMAKE_OSX_DEPLOYMENT_TARGET
  CMAKE_MSVC_RUNTIME_LIBRARY
  VST3_SDK_PATH
)

# =============================================================================
# PLATFORM SUMMARY
# =============================================================================

message(STATUS "=== Platform Configuration ===")
message(STATUS "Platform:               ${VITAL_PLATFORM_NAME}")
message(STATUS "Architecture:           ${VITAL_ARCH_NAME} (${VITAL_ARCH_BITS}-bit)")
message(STATUS "Compiler:               ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Build Type:             ${CMAKE_BUILD_TYPE}")
message(STATUS "Position Independent:   ${CMAKE_POSITION_INDEPENDENT_CODE}")
message(STATUS "Thread Support:         Enabled")
message(STATUS "==========================")