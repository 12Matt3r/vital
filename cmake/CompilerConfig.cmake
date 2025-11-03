# Compiler-Specific Configuration and Optimization Flags
# Supports GCC, Clang, MSVC, Intel, and other modern C++ compilers

# =============================================================================
# COMMON COMPILER SETTINGS
# =============================================================================

# Standard library selection
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  # Use libstdc++ with modern C++ standard library features
  target_compile_definitions(VitalCore PUBLIC
    _GLIBCXX_USE_CXX11_ABI=1
  )
endif()

# C++20/23 feature availability
target_compile_definitions(VitalCore PUBLIC
  $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:HAVE_STD_FILESYSTEM=1>
  $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:HAVE_STD_OPTIONAL=1>
  $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:HAVE_STD_VARIANT=1>
  $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:HAVE_STD_CONCEPTS=$<VERSION_GREATER_EQUAL:${CMAKE_CXX_STANDARD},20>>
  $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:HAVE_STD_COROUTINES=$<VERSION_GREATER_EQUAL:${CMAKE_CXX_STANDARD},20>>
)

# =============================================================================
# GCC COMPILER CONFIGURATION
# =============================================================================

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  message(STATUS "=== GCC Compiler Configuration ===")
  
  # GCC version requirements
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "11.0")
    message(WARNING "GCC ${CMAKE_CXX_COMPILER_VERSION} detected. GCC 11+ recommended for full C++20 support.")
  endif()
  
  # C++ standard library
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
  
  # Modern GCC warnings for audio software
  list(APPEND CMAKE_CXX_FLAGS
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wsign-conversion
    -Wcast-qual
    -Wzero-as-null-pointer-constant
    -Wold-style-cast
    -Wnon-virtual-dtor
    -Woverloaded-virtual
    -Wredundant-decls
    -Winline
  )
  
  # C++20/23 specific warnings
  if(CMAKE_CXX_STANDARD GREATER_EQUAL 20)
    list(APPEND CMAKE_CXX_FLAGS
      -Wctad-maybe-unsupported
      -Wcoroutines
    )
  endif()
  
  if(CMAKE_CXX_STANDARD GREATER_EQUAL 23)
    list(APPEND CMAKE_CXX_FLAGS
      -Wdeduced-template-class
      -Wmultidimensional-subscript
    )
  endif()
  
  # GCC-specific optimizations for audio processing
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    # Aggressive optimization for release
    list(APPEND CMAKE_CXX_FLAGS_RELEASE
      -O3
      -DNDEBUG
      -march=native
      -mtune=native
      -funroll-loops
      -ffast-math
      -ftree-vectorize
      -fprefetch-loop-arrays
      -finline-functions
      -finline-limit=1000
    )
    
    # Link-time optimization
    if(ENABLE_LTO)
      list(APPEND CMAKE_CXX_FLAGS_RELEASE -flto=auto)
      list(APPEND CMAKE_SHARED_LINKER_FLAGS_RELEASE -flto=auto)
    endif()
    
    # No-semantic-interposition for better optimization
    list(APPEND CMAKE_CXX_FLAGS_RELEASE -fno-semantic-interposition)
    
  elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    list(APPEND CMAKE_CXX_FLAGS_RELWITHDEBINFO
      -O2
      -DNDEBUG
      -g
      -fvar-tracking-assignments
    )
  endif()
  
  # Debug configuration
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND CMAKE_CXX_FLAGS_DEBUG
      -O0
      -DDEBUG
      -D_DEBUG
      -g
      -fno-omit-frame-pointer
      -fno-inline
      -fno-optimize-sibling-calls
    )
  endif()
  
  # Sanitizer configurations
  if(ENABLE_ASAN)
    list(APPEND CMAKE_CXX_FLAGS
      -fsanitize=address
      -fno-omit-frame-pointer
      -fno-optimize-sibling-calls
    )
  endif()
  
  if(ENABLE_UBSAN)
    list(APPEND CMAKE_CXX_FLAGS
      -fsanitize=undefined
      -fno-omit-frame-pointer
    )
  endif()
  
  if(ENABLE_TSAN)
    list(APPEND CMAKE_CXX_FLAGS
      -fsanitize=thread
      -fno-omit-frame-pointer
    )
  endif()
  
  # Code coverage
  if(ENABLE_COVERAGE)
    list(APPEND CMAKE_CXX_FLAGS --coverage -DCOVERAGE_BUILD)
    list(APPEND CMAKE_EXE_LINKER_FLAGS --coverage)
  endif()
  
  # ccache support for faster rebuilds
  if(ENABLE_CCACHE)
    find_program(CCACHE_PROGRAM ccache)
    if(CCACHE_PROGRAM)
      set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM} CACHE FILEPATH "C compiler launcher")
      set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM} CACHE FILEPATH "C++ compiler launcher")
      message(STATUS "  ccache:            Enabled")
    endif()
  endif()
  
  # GCC-specific defines
  target_compile_definitions(VitalCore PUBLIC
    __GNUC__=${CMAKE_CXX_COMPILER_VERSION_MAJOR}
    __GNUC_MINOR__=${CMAKE_CXX_COMPILER_VERSION_MINOR}
    __GNUC_PATCHLEVEL__=${CMAKE_CXX_COMPILER_VERSION_PATCHLEVEL}
    __GNUC_GNU_INLINE__=1
  )

# =============================================================================
# CLANG COMPILER CONFIGURATION
# =============================================================================

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  message(STATUS "=== Clang Compiler Configuration ===")
  
  # Clang version requirements
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "13.0")
    message(WARNING "Clang ${CMAKE_CXX_COMPILER_VERSION} detected. Clang 13+ recommended for full C++20 support.")
  endif()
  
  # C++ standard library
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
  
  # Modern Clang warnings for audio software
  list(APPEND CMAKE_CXX_FLAGS
    -Wall
    -Wextra
    -Wpedantic
    -Wshadow
    -Wconversion
    -Wsign-conversion
    -Wcast-qual
    -Wzero-as-null-pointer-constant
    -Wold-style-cast
    -Wnon-virtual-dtor
    -Woverloaded-virtual
    -Wredundant-decls
    -Winline
  )
  
  # Clang-specific warnings
  list(APPEND CMAKE_CXX_FLAGS
    -Wmost
    -Werror=implicit-int-float-conversion
    -Wno-unused-private-field
    -Wno-unused-variable
  )
  
  # C++20/23 specific features
  if(CMAKE_CXX_STANDARD GREATER_EQUAL 20)
    list(APPEND CMAKE_CXX_FLAGS
      -Wctad-maybe-unsupported
      -Wcoroutines
      -Wdeprecated-copy-with-user-provided-copy
    )
  endif()
  
  if(CMAKE_CXX_STANDARD GREATER_EQUAL 23)
    list(APPEND CMAKE_CXX_FLAGS
      -Wdeduced-template-class
      -Wmultidimensional-subscript
    )
  endif()
  
  # Clang optimizations for audio processing
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    list(APPEND CMAKE_CXX_FLAGS_RELEASE
      -O3
      -DNDEBUG
      -march=native
      -mtune=native
      -funroll-loops
      -ffast-math
      -ftree-vectorize
      -fprefetch-loop-arrays
      -finline-functions
      -mllvm
      -polly
      -polly-vectorizer=stripmine
    )
    
    # Clang LTO (thin LTO is usually better for Clang)
    if(ENABLE_LTO)
      list(APPEND CMAKE_CXX_FLAGS_RELEASE -flto=thin)
      list(APPEND CMAKE_SHARED_LINKER_FLAGS_RELEASE -flto=thin)
    endif()
    
  elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    list(APPEND CMAKE_CXX_FLAGS_RELWITHDEBINFO
      -O2
      -DNDEBUG
      -g
      -fno-omit-frame-pointer
    )
  endif()
  
  # Debug configuration
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND CMAKE_CXX_FLAGS_DEBUG
      -O0
      -DDEBUG
      -D_DEBUG
      -g
      -fno-omit-frame-pointer
      -fno-inline
    )
  endif()
  
  # Sanitizer configurations
  if(ENABLE_ASAN)
    list(APPEND CMAKE_CXX_FLAGS
      -fsanitize=address
      -fno-omit-frame-pointer
    )
  endif()
  
  if(ENABLE_UBSAN)
    list(APPEND CMAKE_CXX_FLAGS
      -fsanitize=undefined
      -fno-omit-frame-pointer
    )
  endif()
  
  if(ENABLE_TSAN)
    list(APPEND CMAKE_CXX_FLAGS
      -fsanitize=thread
      -fno-omit-frame-pointer
    )
  endif()
  
  # Code coverage
  if(ENABLE_COVERAGE)
    list(APPEND CMAKE_CXX_FLAGS --coverage -DCOVERAGE_BUILD)
    list(APPEND CMAKE_EXE_LINKER_FLAGS --coverage)
  endif()
  
  # Clang-specific defines
  target_compile_definitions(VitalCore PUBLIC
    __clang__=1
    __clang_major__=${CMAKE_CXX_COMPILER_VERSION_MAJOR}
    __clang_minor__=${CMAKE_CXX_COMPILER_VERSION_MINOR}
    __clang_patchlevel__=${CMAKE_CXX_COMPILER_VERSION_PATCHLEVEL}
  )
  
  # ccache support (default on for Clang)
  if(ENABLE_CCACHE)
    find_program(CCACHE_PROGRAM ccache)
    if(CCACHE_PROGRAM)
      set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM} CACHE FILEPATH "C compiler launcher")
      set(CMAKE_CXX_COMPILER_LAUNCHER ${CCACHE_PROGRAM} CACHE FILEPATH "C++ compiler launcher")
      message(STATUS "  ccache:            Enabled")
    endif()
  endif()

# =============================================================================
# MSVC COMPILER CONFIGURATION
# =============================================================================

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  message(STATUS "=== MSVC Compiler Configuration ===")
  
  # MSVC version requirements
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "1920")
    message(FATAL_ERROR "MSVC 2019 (16.0) or later required. Detected version: ${CMAKE_CXX_COMPILER_VERSION}")
  endif()
  
  # C++ standard library
  set(CMAKE_CXX_STANDARD 20)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS ON)  # MSVC extensions are useful for Windows development
  
  # MSVC warning levels and specific warnings
  list(APPEND CMAKE_CXX_FLAGS
    /W4
    /permissive-
    /Zc:__cplusplus
    /Zc:strictStrings
    /Zc:throwingNew
    /Zc:inline
    /utf-8
  )
  
  # Disable specific warnings that are common in JUCE and third-party code
  list(APPEND CMAKE_CXX_FLAGS
    /wd4251  # 'identifier': class 'type' needs to have dll-interface to be used by clients of class 'type2'
    /wd4275  # non-DLL-interface class 'identifier' used as base for DLL-interface class 'identifier'
    /wd4351  # default behavior may differ from standard behavior
    /wd4589  # Constructor for class 'type' does not initialize a reference member
    /wd4819  # The file contains a character that cannot be represented in the current code page
    /wd4996  # 'function': was declared deprecated
  )
  
  # C++20/23 features
  add_definitions(-D_HAS_CXX20=1)
  add_definitions(-D_HAS_CXX23=1)
  if(CMAKE_CXX_STANDARD GREATER_EQUAL 20)
    add_definitions(-D_HAS_CONCEPTS_LANG=1)
    add_definitions(-D_HAS_COROUTINES=1)
    add_definitions(-D_HAS_SPACESHIP_OPERATOR=1)
  endif()
  
  # MSVC optimization flags
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    list(APPEND CMAKE_CXX_FLAGS_RELEASE
      /O2
      /Oy-
      /Ob2
      /DNDEBUG
      /GL
      /Gm-
      /Zc:inline
      /fp:fast
      /permissive-
    )
    
    # Link-time code generation
    if(ENABLE_LTO)
      list(APPEND CMAKE_CXX_FLAGS_RELEASE /LTCG)
    endif()
    
    # Profile-guided optimization
    if(ENABLE_PGO)
      list(APPEND CMAKE_CXX_FLAGS_RELEASE /LTCG:PGInstrument)
      list(APPEND CMAKE_EXE_LINKER_FLAGS_RELEASE /LTCG:PGInstrument)
    endif()
    
    # Static analysis (only in release builds)
    list(APPEND CMAKE_CXX_FLAGS_RELEASE /analyze)
    
  elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    list(APPEND CMAKE_CXX_FLAGS_RELWITHDEBINFO
      /O2
      /DNDEBUG
      /Zi
      /Oy-
    )
    list(APPEND CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO /DEBUG:FULL)
  endif()
  
  # Debug configuration
  if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND CMAKE_CXX_FLAGS_DEBUG
      /Od
      /D_DEBUG
      /DDEBUG
      /Zi
      /Oy-
      /RTC1
    )
    
    # Enhanced debug information
    list(APPEND CMAKE_EXE_LINKER_FLAGS_DEBUG /DEBUG:FULL)
  endif()
  
  # Fast debug builds for rapid iteration
  if(ENABLE_FAST_DEBUG AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    list(APPEND CMAKE_CXX_FLAGS_DEBUG
      /DNDEBUG
      /Oy
    )
    target_compile_definitions(VitalCore PUBLIC VITAL_FAST_DEBUG=1)
    message(STATUS "  Fast Debug:        Enabled")
  endif()
  
  # Security enhancements
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    list(APPEND CMAKE_CXX_FLAGS_RELEASE
      /sdl
      /GS
      /guard:cf
      /d1 FSA:Enable
    )
  endif()
  
  # MSVC-specific defines
  target_compile_definitions(VitalCore PUBLIC
    _MSC_VER=${CMAKE_CXX_COMPILER_VERSION}
    _MSC_FULL_VER=${CMAKE_CXX_COMPILER_VERSION}
    _WIN32
    _WINDOWS
    NOMINMAX
    WIN32_LEAN_AND_MEAN
    UNICODE
    _UNICODE
  )
  
  # Windows-specific optimizations
  if(VITAL_ARCH_X86_64)
    # Enable 64-bit optimizations
    list(APPEND CMAKE_CXX_FLAGS_RELEASE /arch:AVX2)
    message(STATUS "  64-bit Optimizations: Enabled")
  endif()

# =============================================================================
# INTEL COMPILER CONFIGURATION
# =============================================================================

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
  message(STATUS "=== Intel Compiler Configuration ===")
  
  # Intel-specific flags for audio processing
  list(APPEND CMAKE_CXX_FLAGS
    -Wall
    -Wextra
    -Wpedantic
    -qopenmp-simd
  )
  
  # Intel optimizations for audio/DSP
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    list(APPEND CMAKE_CXX_FLAGS_RELEASE
      -O3
      -DNDEBUG
      -ipo
      -no-prec-div
      -fast-transcendentals
      -qopenmp
      -xHost
    )
  endif()
  
  # Intel-specific defines
  target_compile_definitions(VitalCore PUBLIC
    __INTEL_COMPILER=${CMAKE_CXX_COMPILER_VERSION}
    __INTEL_COMPILER_BUILD_DATE=${CMAKE_CXX_COMPILER_VERSION}
  )

# =============================================================================
# UNKNOWN COMPILER FALLBACK
# =============================================================================

else()
  message(WARNING "Unknown compiler: ${CMAKE_CXX_COMPILER_ID}. Using conservative settings.")
  
  # Basic warning settings for unknown compilers
  list(APPEND CMAKE_CXX_FLAGS -Wall -Wextra -Wpedantic)
  
  # Basic optimization settings
  if(CMAKE_BUILD_TYPE STREQUAL "Release")
    list(APPEND CMAKE_CXX_FLAGS_RELEASE -O3 -DNDEBUG)
  endif()
  
  target_compile_definitions(VitalCore PUBLIC UNKNOWN_COMPILER=1)
endif()

# =============================================================================
# PRECOMPILED HEADERS SUPPORT
# =============================================================================

if(ENABLE_PRECOMPILED_HEADERS)
  if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # MSVC precompiled headers
    target_precompiled_headers(VitalCore REUSE_FROM vital_pch.h)
  else()
    # GCC/Clang precompiled headers using gccxml or similar
    # Note: GCC/Clang don't have native PCH support in CMake yet
    message(STATUS "  Precompiled Headers: Manual configuration required for ${CMAKE_CXX_COMPILER_ID}")
  endif()
endif()

# =============================================================================
# EXPORT COMPILER INFORMATION
# =============================================================================

set(VITAL_COMPILER_ID ${CMAKE_CXX_COMPILER_ID} CACHE INTERNAL "Compiler ID")
set(VITAL_COMPILER_VERSION ${CMAKE_CXX_COMPILER_VERSION} CACHE INTERNAL "Compiler version")

# Mark advanced variables
mark_as_advanced(
  CMAKE_C_COMPILER_LAUNCHER
  CMAKE_CXX_COMPILER_LAUNCHER
  ENABLE_CCACHE
)

# =============================================================================
# COMPILER SUMMARY
# =============================================================================

message(STATUS "")
message(STATUS "=== Compiler Configuration Summary ===")
message(STATUS "Compiler:              ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "C++ Standard:          ${CMAKE_CXX_STANDARD}")
message(STATUS "Standard Extensions:   ${CMAKE_CXX_EXTENSIONS}")
message(STATUS "Build Type:            ${CMAKE_BUILD_TYPE}")
message(STATUS "LTO:                   ${ENABLE_LTO}")
message(STATUS "PGO:                   ${ENABLE_PGO}")
message(STATUS "Sanitizers:")
message(STATUS "  AddressSanitizer:     ${ENABLE_ASAN}")
message(STATUS "  UndefinedBehaviorSan: ${ENABLE_UBSAN}")
message(STATUS "  ThreadSanitizer:      ${ENABLE_TSAN}")
message(STATUS "Code Coverage:         ${ENABLE_COVERAGE}")
message(STATUS "======================================")
message(STATUS "")