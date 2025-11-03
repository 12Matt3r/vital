# Performance Optimization Configuration
# Handles SIMD, multithreading, vectorization, and other performance optimizations

# =============================================================================
# SIMD OPTIMIZATION SETTINGS
# =============================================================================

if(ENABLE_SIMD_OPTIMIZATIONS)
  message(STATUS "=== SIMD Optimizations Enabled ===")
  
  # CPU feature detection
  include(CheckCXXSourceCompiles)
  include(CheckLibraryExists)
  
  # Check for SSE4.2 support
  check_cxx_source_compiles("
    #include <immintrin.h>
    int main() { 
      __m128i a = _mm_set1_epi32(1);
      __m128i b = _mm_set1_epi32(2);
      __m128i c = _mm_add_epi32(a, b);
      return 0; 
    }
  " VITAL_HAS_SSE42)
  
  # Check for AVX2 support
  check_cxx_source_compiles("
    #include <immintrin.h>
    int main() { 
      __m256i a = _mm256_set1_epi32(1);
      __m256i b = _mm256_set1_epi32(2);
      __m256i c = _mm256_add_epi32(a, b);
      return 0; 
    }
  " VITAL_HAS_AVX2)
  
  # Check for AVX-512 support
  check_cxx_source_compiles("
    #include <immintrin.h>
    int main() { 
      __m512i a = _mm512_set1_epi32(1);
      __m512i b = _mm512_set1_epi32(2);
      __m512i c = _mm512_add_epi32(a, b);
      return 0; 
    }
  " VITAL_HAS_AVX512)
  
  # Check for ARM NEON support
  if(VITAL_ARCH_ARM64)
    check_cxx_source_compiles("
      #include <arm_neon.h>
      int main() { 
        int32x4_t a = vdupq_n_s32(1);
        int32x4_t b = vdupq_n_s32(2);
        int32x4_t c = vaddq_s32(a, b);
        return 0; 
      }
    " VITAL_HAS_NEON)
  endif()
  
  # Apply SIMD flags based on detected support
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # GCC/Clang SIMD flags
    if(VITAL_ARCH_X86_64)
      if(VITAL_HAS_AVX512)
        target_compile_definitions(VitalCore PUBLIC VITAL_SIMD_AVX512=1)
        target_compile_options(VitalCore PRIVATE -mavx512f -mavx512cd -mavx512vl -mavx512bw -mavx512dq)
      endif()
      
      if(VITAL_HAS_AVX2)
        target_compile_definitions(VitalCore PUBLIC VITAL_SIMD_AVX2=1)
        target_compile_options(VitalCore PRIVATE -mavx2 -mfma)
      endif()
      
      if(VITAL_HAS_SSE42)
        target_compile_definitions(VitalCore PUBLIC VITAL_SIMD_SSE42=1)
        target_compile_options(VitalCore PRIVATE -msse4.2)
      endif()
      
      # Enable automatic vectorization
      if(ENABLE_VECTORIZATION)
        target_compile_options(VitalCore PRIVATE -ftree-vectorize -march=native -mtune=native)
      endif()
      
    elseif(VITAL_ARCH_ARM64)
      if(VITAL_HAS_NEON)
        target_compile_definitions(VitalCore PUBLIC VITAL_SIMD_NEON=1)
        target_compile_options(VitalCore PRIVATE -march=armv8-a+simd)
      endif()
    endif()
    
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    # MSVC SIMD flags
    if(VITAL_ARCH_X86_64)
      if(VITAL_HAS_AVX512)
        target_compile_definitions(VitalCore PUBLIC VITAL_SIMD_AVX512=1)
        target_compile_options(VitalCore PRIVATE /arch:AVX512)
      elseif(VITAL_HAS_AVX2)
        target_compile_definitions(VitalCore PUBLIC VITAL_SIMD_AVX2=1)
        target_compile_options(VitalCore PRIVATE /arch:AVX2)
      elseif(VITAL_HAS_SSE42)
        target_compile_definitions(VitalCore PUBLIC VITAL_SIMD_SSE42=1)
        target_compile_options(VitalCore PRIVATE /arch:SSE4.2)
      endif()
    endif()
  endif()
  
  # SIMD-specific source files
  if(EXISTS ${VITAL_PHASE2_DIR}/simd_optimization)
    target_sources(VitalCore PRIVATE
      ${VITAL_PHASE2_DIR}/simd_optimization/simd_vector.cpp
      ${VITAL_PHASE2_DIR}/simd_optimization/sse42_optimizations.h
      ${VITAL_PHASE2_DIR}/simd_optimization/avx2_optimizations.h
      ${VITAL_PHASE2_DIR}/simd_optimization/avx512_optimizations.h
      ${VITAL_PHASE2_DIR}/simd_optimization/neon_optimizations.h
    )
  endif()
  
  # SIMD CPU detection
  target_compile_definitions(VitalCore PUBLIC
    VITAL_HAS_SIMD=1
    $<$<BOOL:${VITAL_HAS_SSE42}>:VITAL_SIMD_SSE42_SUPPORTED=1>
    $<$<BOOL:${VITAL_HAS_AVX2}>:VITAL_SIMD_AVX2_SUPPORTED=1>
    $<$<BOOL:${VITAL_HAS_AVX512}>:VITAL_SIMD_AVX512_SUPPORTED=1>
    $<$<BOOL:${VITAL_HAS_NEON}>:VITAL_SIMD_NEON_SUPPORTED=1>
  )
endif()

# =============================================================================
# MULTITHREADING OPTIMIZATION SETTINGS
# =============================================================================

if(ENABLE_MULTITHREADING)
  message(STATUS "=== Multithreading Optimizations Enabled ===")
  
  target_compile_definitions(VitalCore PUBLIC
    VITAL_MULTITHREADING=1
    JUCE_VIRTUALIZE_CONTROLS=0
  )
  
  # Thread-local storage optimizations
  target_compile_options(VitalCore PRIVATE
    $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:-ftls-model=initial-exec>
  )
  
  # OpenMP support for parallel audio processing
  find_package(OpenMP QUIET)
  if(OpenMP_FOUND)
    target_link_libraries(VitalCore PUBLIC OpenMP::OpenMP_CXX)
    target_compile_definitions(VitalCore PUBLIC VITAL_OPENMP=1)
    message(STATUS "  OpenMP:              Enabled")
  endif()
  
  # Thread pool implementation
  if(EXISTS ${VITAL_PHASE2_DIR}/multithreading)
    target_sources(VitalCore PRIVATE
      ${VITAL_PHASE2_DIR}/multithreading/src/ThreadPoolManager.cpp
      ${VITAL_PHASE2_DIR}/multithreading/src/VoiceProcessingManager.cpp
      ${VITAL_PHASE2_DIR}/multithreading/include/WorkStealingScheduler.h
      ${VITAL_PHASE2_DIR}/multithreading/include/LockFreeRingBuffer.h
    )
  endif()
  
  # CPU affinity management
  if(VITAL_PLATFORM_UNIX)
    target_compile_definitions(VitalCore PUBLIC VITAL_CPU_AFFINITY=1)
  endif()
endif()

# =============================================================================
# MEMORY OPTIMIZATION SETTINGS
# =============================================================================

# Cache optimization settings
target_compile_definitions(VitalCore PUBLIC
  VITAL_CACHE_LINE_SIZE=64
  VITAL_PREFETCH_DISTANCE=4
)

# Memory alignment for audio buffers
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(VitalCore PRIVATE -falign-loops=16 -falign-functions=16)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  target_compile_options(VitalCore PRIVATE /align:16)
endif()

# NUMA-aware allocation (Linux)
if(VITAL_PLATFORM_LINUX)
  find_package(Threads REQUIRED)
  find_library(NUMA_LIBRARY numa)
  if(NUMA_LIBRARY)
    target_link_libraries(VitalCore PUBLIC ${NUMA_LIBRARY})
    target_compile_definitions(VitalCore PUBLIC VITAL_NUMA_AWARE=1)
    message(STATUS "  NUMA Support:       Enabled")
  endif()
endif()

# =============================================================================
# REALTIME OPTIMIZATION SETTINGS
# =============================================================================

# Realtime-safe memory allocation
target_compile_definitions(VitalCore PUBLIC
  VITAL_REALTIME_MEMORY_POOLS=1
  VITAL_LOCK_FREE_CONTAINERS=1
)

# CPU cache optimization
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(VitalCore PRIVATE
    -fprefetch-loop-arrays
    $<$<CONFIG:Release>:-funroll-loops>
  )
endif()

# Branch prediction hints
target_compile_definitions(VitalCore PUBLIC
  $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:VITAL_LIKELY="__attribute__((likely))">
  $<$<COMPILE_LANG_AND_ID:CXX,GNU,Clang>:VITAL_UNLIKELY="__attribute__((unlikely))">
  $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:VITAL_LIKELY="">
  $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:VITAL_UNLIKELY="">
)

# =============================================================================
# FLOATING POINT OPTIMIZATION SETTINGS
# =============================================================================

# Fast math for audio processing
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(VitalCore PRIVATE
    -ffast-math
    -fno-finite-math-only
    -fno-unsafe-math-optimizations
  )
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  target_compile_options(VitalCore PRIVATE /fp:fast)
endif()

# Floating point exception handling
target_compile_definitions(VitalCore PUBLIC
  VITAL_FPE_SIGNALS=1
  $<$<COMPILE_LANG_AND_ID:CXX,GNU>:VITAL_SET_FPU_TRAP="__builtin_fesetround(FE_TONEAREST)">
  $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:VITAL_SET_FPU_TRAP="">
)

# =============================================================================
# LINK-TIME OPTIMIZATION SETTINGS
# =============================================================================

if(ENABLE_LTO AND CMAKE_BUILD_TYPE STREQUAL "Release")
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION ON)
    target_compile_options(VitalCore PRIVATE -flto=auto)
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(VitalCore PRIVATE /GL)
    target_link_options(VitalCore PRIVATE /LTCG)
  endif()
  
  # Remove dead code
  if(VITAL_PLATFORM_UNIX)
    target_link_options(VitalCore PRIVATE -Wl,--gc-sections -Wl,--strip-all)
  elseif(VITAL_PLATFORM_WINDOWS)
    target_link_options(VitalCore PRIVATE /DEBUG:NONE /OPT:REF /OPT:ICF)
  endif()
  
  message(STATUS "  LTO:                Enabled")
endif()

# =============================================================================
# PROFILE-GUIDED OPTIMIZATION SETTINGS
# =============================================================================

if(ENABLE_PGO)
  message(STATUS "=== Profile-Guided Optimization Enabled ===")
  
  if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # PGO compile flags
    target_compile_options(VitalCore PRIVATE -fprofile-generate)
    target_link_options(VitalCore PRIVATE -fprofile-generate)
    
    # Environment variable for PGO data directory
    set(ENV{PROFDATA} "${CMAKE_BINARY_DIR}/pgo_data")
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/pgo_data")
    
    message(STATUS "  PGO Mode:           Instrumentation")
    message(STATUS "  PGO Data Dir:       ${CMAKE_BINARY_DIR}/pgo_data")
    
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(VitalCore PRIVATE /LTCG:PGInstrument)
    target_link_options(VitalCore PRIVATE /LTCG:PGInstrument)
    
    message(STATUS "  PGO Mode:           MSVC Instrumentation")
  endif()
endif()

# =============================================================================
# BRANCHLESS OPTIMIZATION SETTINGS
# =============================================================================

if(EXISTS ${VITAL_PHASE2_DIR}/branchless_optimization)
  target_sources(VitalCore PRIVATE
    ${VITAL_PHASE2_DIR}/branchless_optimization/branchless_audio_algorithms.h
    ${VITAL_PHASE2_DIR}/branchless_optimization/branchless_conditional.h
    ${VITAL_PHASE2_DIR}/branchless_optimization/branchless_interpolation.h
  )
  
  target_compile_definitions(VitalCore PUBLIC VITAL_BRANCHLESS=1)
  message(STATUS "  Branchless Optimizations: Enabled")
endif()

# =============================================================================
# CACHE OPTIMIZATION SETTINGS
# =============================================================================

if(EXISTS ${VITAL_PHASE2_DIR}/cache_optimization)
  target_sources(VitalCore PRIVATE
    ${VITAL_PHASE2_DIR}/cache_optimization/src/cache_optimization.cpp
  )
  
  target_include_directories(VitalCore PRIVATE
    ${VITAL_PHASE2_DIR}/cache_optimization/include
  )
  
  target_compile_definitions(VitalCore PUBLIC VITAL_CACHE_OPTIMIZED=1)
  message(STATUS "  Cache Optimization:     Enabled")
endif()

# =============================================================================
# REALTIME OPTIMIZATION SETTINGS
# =============================================================================

if(EXISTS ${VITAL_PHASE2_DIR}/realtime_optimization)
  target_sources(VitalCore PRIVATE
    ${VITAL_PHASE2_DIR}/realtime_optimization/src/callback_optimizer.cpp
    ${VITAL_PHASE2_DIR}/realtime_optimization/src/cpu_scheduler.cpp
    ${VITAL_PHASE2_DIR}/realtime_optimization/src/deterministic_processor.cpp
  )
  
  target_include_directories(VitalCore PRIVATE
    ${VITAL_PHASE2_DIR}/realtime_optimization/include
  )
  
  target_compile_definitions(VitalCore PUBLIC VITAL_REALTIME_OPTIMIZED=1)
  message(STATUS "  Realtime Optimization:  Enabled")
endif()

# =============================================================================
# HARDWARE ACCELERATION SETTINGS
# =============================================================================

if(VITAL_PLATFORM_MACOS)
  # Metal Performance Shaders for audio processing
  find_library(METAL_LIBRARY Metal)
  find_library(METAL_PERFORMANCE_SHADERS_LIBRARY MetalPerformanceShaders)
  
  if(METAL_LIBRARY AND METAL_PERFORMANCE_SHADERS_LIBRARY)
    target_link_libraries(VitalCore PUBLIC
      "${METAL_LIBRARY}"
      "${METAL_PERFORMANCE_SHADERS_LIBRARY}"
    )
    target_compile_definitions(VitalCore PUBLIC VITAL_METAL_ACCELERATION=1)
    message(STATUS "  Metal Acceleration:     Enabled")
  endif()
  
  # Apple Accelerate framework for vector operations
  find_library(ACCELERATE_LIBRARY Accelerate)
  if(ACCELERATE_LIBRARY)
    target_link_libraries(VitalCore PUBLIC "${ACCELERATE_LIBRARY}")
    target_compile_definitions(VitalCore PUBLIC VITAL_ACCELERATE=1)
    message(STATUS "  Accelerate Framework:   Enabled")
  endif()
endif()

# =============================================================================
# CPU FEATURE SUMMARY
# =============================================================================

message(STATUS "")
message(STATUS "CPU Feature Detection:")
message(STATUS "  Architecture:        ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "  SIMD Support:        ${ENABLE_SIMD_OPTIMIZATIONS}")
if(ENABLE_SIMD_OPTIMIZATIONS)
  message(STATUS "    SSE4.2:            ${VITAL_HAS_SSE42}")
  message(STATUS "    AVX2:              ${VITAL_HAS_AVX2}")
  message(STATUS "    AVX-512:           ${VITAL_HAS_AVX512}")
  if(VITAL_ARCH_ARM64)
    message(STATUS "    NEON:              ${VITAL_HAS_NEON}")
  endif()
endif()
message(STATUS "  Multithreading:      ${ENABLE_MULTITHREADING}")
message(STATUS "  Vectorization:       ${ENABLE_VECTORIZATION}")
message(STATUS "")
message(STATUS "=== End Performance Configuration ===")
message(STATUS "")