# Performance Optimization Modules for Vital Audio Synthesizer
# CMakeLists.txt

cmake_minimum_required(VERSION 3.15)
project(VitalPerformanceModules VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Compiler-specific options
if(MSVC)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /permissive-")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /Oi /Ot /GL /MP")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /O2 /Oi /Ot /GL /Zi")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -march=native -mtune=native -flto")
    set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} -O3 -march=native -mtune=native -flto -g")
endif()

# Performance optimization options
option(VITAL_PERF_ENABLE_SIMD "Enable SIMD optimizations" ON)
option(VITAL_PERF_ENABLE_MULTITHREADING "Enable multithreading optimizations" ON)
option(VITAL_PERF_ENABLE_CACHE_OPT "Enable cache optimizations" ON)
option(VITAL_PERF_ENABLE_BRANCHLESS "Enable branchless programming" ON)
option(VITAL_PERF_ENABLE_REALTIME "Enable real-time optimizations" ON)
option(VITAL_PERF_BUILD_TESTS "Build test applications" ON)
option(VITAL_PERF_BUILD_BENCHMARKS "Build benchmark applications" ON)
option(VITAL_PERF_ENABLE_PROFILING "Enable performance profiling" ON)

# SIMD configuration
if(VITAL_PERF_ENABLE_SIMD)
    # Check for supported SIMD instruction sets
    include(CheckCXXSourceCompiles)
    
    # SSE4.2
    check_cxx_source_compiles("
        #include <emmintrin.h>
        int main() {
            __m128i a = _mm_setzero_si128();
            return 0;
        }
    " HAS_SSE42)
    
    # AVX
    check_cxx_source_compiles("
        #include <immintrin.h>
        int main() {
            __m256 a = _mm256_setzero_ps();
            return 0;
        }
    " HAS_AVX)
    
    # AVX2
    check_cxx_source_compiles("
        #include <immintrin.h>
        int main() {
            __m256i a = _mm256_setzero_si256();
            return 0;
        }
    " HAS_AVX2)
    
    # AVX-512
    check_cxx_source_compiles("
        #include <immintrin.h>
        int main() {
            __m512 a = _mm512_setzero_ps();
            return 0;
        }
    " HAS_AVX512F)
    
    # ARM NEON
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm|aarch64")
        check_cxx_source_compiles("
            #if defined(__ARM_NEON__) || defined(__aarch64__)
            #include <arm_neon.h>
            int main() {
                float32x4_t a = vdupq_n_f32(0.0f);
                return 0;
            }
            #endif
        " HAS_NEON)
    endif()
    
    # Set SIMD flags
    if(HAS_SSE42)
        add_compile_options(-msse4.2)
    endif()
    
    if(HAS_AVX2)
        add_compile_options(-mavx2)
    endif()
    
    if(HAS_AVX512F)
        add_compile_options(-mavx512f)
    endif()
    
    if(HAS_NEON)
        add_compile_options(-mfpu=neon -march=armv8-a+simd)
    endif()
endif()

# Platform-specific settings
if(WIN32)
    # Windows-specific libraries
    find_library(PSAPI psapi)
    find_library(KERNEL32 kernel32)
elseif(UNIX AND NOT APPLE)
    # Linux-specific libraries
    find_package(Threads REQUIRED)
    find_package(PkgConfig QUIET)
    
    # NUMA support
    pkg_check_modules(NUMA numa)
    if(NUMA_FOUND)
        add_compile_options(${NUMA_CFLAGS_OTHER})
        link_libraries(${NUMA_LIBRARIES})
        add_definitions(-DHAS_NUMA_SUPPORT=1)
    endif()
    
    # Hardware performance counters
    find_package(PkgConfig QUIET)
    pkg_check_modules(PERF perf_event)
    if(PERF_FOUND)
        add_compile_options(${PERF_CFLAGS_OTHER})
        link_libraries(${PERF_LIBRARIES})
        add_definitions(-DHAS_PERF_SUPPORT=1)
    endif()
endif()

# JUCE framework integration (optional)
option(VITAL_PERF_ENABLE_JUCE "Enable JUCE framework integration" ON)
if(VITAL_PERF_ENABLE_JUCE)
    find_package(JUCE CONFIG QUIET)
    if(JUCE_FOUND)
        add_definitions(-DHAS_JUCE_SUPPORT=1)
        message(STATUS "JUCE framework found - JUCE integration enabled")
    else()
        message(STATUS "JUCE framework not found - JUCE integration disabled")
        set(VITAL_PERF_ENABLE_JUCE OFF)
    endif()
endif()

# Source files
set(PERFORMANCE_HEADERS
    performance.h
    simd_vectorization.h
    multithreading.h
    cache_optimization.h
    branchless_programming.h
    real_time_optimization.h
)

set(PERFORMANCE_SOURCES
    # Implementation files would go here
    # performance.cpp
)

# Main performance library
add_library(vital_performance STATIC ${PERFORMANCE_SOURCES} ${PERFORMANCE_HEADERS})

# Set library properties
set_target_properties(vital_performance PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    OUTPUT_NAME "vital_performance"
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    POSITION_INDEPENDENT_CODE ON
)

# Target include directories
target_include_directories(vital_performance PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:include>
)

# Target compile definitions
target_compile_definitions(vital_performance PRIVATE
    VITAL_PERFORMANCE_VERSION=${PROJECT_VERSION}
    VITAL_PERF_ENABLE_SIMD=$<BOOL:${VITAL_PERF_ENABLE_SIMD}>
    VITAL_PERF_ENABLE_MULTITHREADING=$<BOOL:${VITAL_PERF_ENABLE_MULTITHREADING}>
    VITAL_PERF_ENABLE_CACHE_OPT=$<BOOL:${VITAL_PERF_ENABLE_CACHE_OPT}>
    VITAL_PERF_ENABLE_BRANCHLESS=$<BOOL:${VITAL_PERF_ENABLE_BRANCHLESS}>
    VITAL_PERF_ENABLE_REALTIME=$<BOOL:${VITAL_PERF_ENABLE_REALTIME}>
    VITAL_PERF_ENABLE_PROFILING=$<BOOL:${VITAL_PERF_ENABLE_PROFILING}>
    HAS_SSE42=$<BOOL:${HAS_SSE42}>
    HAS_AVX=$<BOOL:${HAS_AVX}>
    HAS_AVX2=$<BOOL:${HAS_AVX2}>
    HAS_AVX512F=$<BOOL:${HAS_AVX512F}>
    HAS_NEON=$<BOOL:${HAS_NEON}>
)

# Target link libraries
target_link_libraries(vital_performance PUBLIC
    Threads::Threads
    $<$<BOOL:${PSAPI}>:psapi>
    $<$<BOOL:${KERNEL32}>:kernel32>
    $<$<BOOL:${NUMA_FOUND}>:${NUMA_LIBRARIES}>
)

# Installation
install(TARGETS vital_performance
    EXPORT VitalPerformanceTargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${PERFORMANCE_HEADERS}
    DESTINATION include/vital/performance
)

# CMake package configuration
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "VitalPerformanceConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(EXPORT VitalPerformanceTargets
    FILE VitalPerformanceConfig.cmake
    NAMESPACE VitalPerformance::
    DESTINATION lib/cmake/VitalPerformance
)

# Testing
if(VITAL_PERF_BUILD_TESTS)
    enable_testing()
    
    # Test executable
    add_executable(test_performance
        test/test_performance.cpp
        test/test_simd.cpp
        test/test_multithreading.cpp
        test/test_cache_optimization.cpp
        test/test_branchless.cpp
        test/test_realtime.cpp
    )
    
    target_link_libraries(test_performance PRIVATE
        vital_performance
        Threads::Threads
    )
    
    target_include_directories(test_performance PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/test
    )
    
    # Add test
    add_test(NAME PerformanceTests COMMAND test_performance)
    
    # Benchmark executable
    if(VITAL_PERF_BUILD_BENCHMARKS)
        add_executable(benchmark_performance
            benchmark/benchmark_performance.cpp
            benchmark/benchmark_simd.cpp
            benchmark/benchmark_multithreading.cpp
            benchmark/benchmark_cache.cpp
            benchmark/benchmark_branchless.cpp
            benchmark/benchmark_realtime.cpp
        )
        
        target_link_libraries(benchmark_performance PRIVATE
            vital_performance
            Threads::Threads
        )
        
        target_include_directories(benchmark_performance PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/benchmark
        )
    endif()
endif()

# Example applications
option(VITAL_PERF_BUILD_EXAMPLES "Build example applications" ON)
if(VITAL_PERF_BUILD_EXAMPLES)
    # Basic example
    add_executable(example_basic_optimization
        examples/basic_optimization.cpp
    )
    
    target_link_libraries(example_basic_optimization PRIVATE
        vital_performance
        Threads::Threads
    )
    
    # Audio processing example
    add_executable(example_audio_processing
        examples/audio_processing.cpp
    )
    
    target_link_libraries(example_audio_processing PRIVATE
        vital_performance
        Threads::Threads
    )
    
    # Multithreaded example
    add_executable(example_multithreaded
        examples/multithreaded_processing.cpp
    )
    
    target_link_libraries(example_multithreaded PRIVATE
        vital_performance
        Threads::Threads
    )
endif()

# Documentation
find_package(Doxygen QUIET)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_SOURCE_DIR}/docs/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile)
    
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)
    
    add_custom_target(docs
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
        COMMENT "Generating API documentation with Doxygen"
        VERBATIM
    )
endif()

# CPack packaging
set(CPACK_PACKAGE_NAME "vital-performance-modules")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "High-performance optimization modules for Vital audio synthesizer")
set(CPACK_PACKAGE_VENDOR "Vital Development Team")
set(CPACK_GENERATOR "TGZ;ZIP")

set(CPACK_PACKAGE_CONTACT "dev@vital.audio")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://vital.audio")

include(CPack)

# Summary
message(STATUS "=== Vital Performance Modules Build Summary ===")
message(STATUS "SIMD Optimization: ${VITAL_PERF_ENABLE_SIMD}")
message(STATUS "Multithreading: ${VITAL_PERF_ENABLE_MULTITHREADING}")
message(STATUS "Cache Optimization: ${VITAL_PERF_ENABLE_CACHE_OPT}")
message(STATUS "Branchless Programming: ${VITAL_PERF_ENABLE_BRANCHLESS}")
message(STATUS "Real-time Optimization: ${VITAL_PERF_ENABLE_REALTIME}")
message(STATUS "JUCE Integration: ${VITAL_PERF_ENABLE_JUCE}")

if(VITAL_PERF_ENABLE_SIMD)
    message(STATUS "SSE4.2 Support: ${HAS_SSE42}")
    message(STATUS "AVX Support: ${HAS_AVX}")
    message(STATUS "AVX2 Support: ${HAS_AVX2}")
    message(STATUS "AVX-512 Support: ${HAS_AVX512F}")
    message(STATUS "NEON Support: ${HAS_NEON}")
endif()

message(STATUS "Build Tests: ${VITAL_PERF_BUILD_TESTS}")
message(STATUS "Build Benchmarks: ${VITAL_PERF_BUILD_BENCHMARKS}")
message(STATUS "Build Examples: ${VITAL_PERF_BUILD_EXAMPLES}")
message(STATUS "Build Documentation: ${DOXYGEN_FOUND}")

message(STATUS "Installation prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "===========================================")
