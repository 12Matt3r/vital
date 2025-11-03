cmake_minimum_required(VERSION 3.20)
project(VitalAI VERSION 1.0.0 LANGUAGES CXX)

# C++20 requirement for modern features
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find required packages
find_package(Eigen3 3.4.0 REQUIRED)
find_package(Threads REQUIRED)

# Compiler-specific optimizations
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-O3 -march=native -flto)
endif()

# Source files
set(AI_SOURCES
    ai_manager.cpp
    neural_preset_generator.cpp
    style_transfer_engine.cpp
    adaptive_modulation_system.cpp
    machine_learning_engine.cpp
    intelligent_audio_analyzer.cpp
    intelligent_preset_generator.cpp
)

# Header files
set(AI_HEADERS
    ai_manager.h
    neural_preset_generator.h
    style_transfer_engine.h
    adaptive_modulation_system.h
    machine_learning_engine.h
    intelligent_audio_analyzer.h
    intelligent_preset_generator.h
)

# Create library
add_library(vital_ai STATIC ${AI_SOURCES} ${AI_HEADERS})

# Set library properties
set_target_properties(vital_ai PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    OUTPUT_NAME "vital_ai"
)

# Include directories
target_include_directories(vital_ai PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${EIGEN3_INCLUDE_DIRS}
)

# Link libraries
target_link_libraries(vital_ai
    Eigen3::Eigen
    Threads::Threads
)

# Compiler definitions
target_compile_definitions(vital_ai PRIVATE
    VITAL_AI_VERSION_MAJOR=1
    VITAL_AI_VERSION_MINOR=0
    VITAL_AI_VERSION_PATCH=0
)

# Performance optimizations
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(vital_ai PRIVATE
        NDEBUG
        VITAL_AI_ENABLE_ASSERTS=0
        VITAL_AI_ENABLE_DEBUG_LOGGING=0
    )
else()
    target_compile_definitions(vital_ai PRIVATE
        VITAL_AI_ENABLE_ASSERTS=1
        VITAL_AI_ENABLE_DEBUG_LOGGING=1
    )
endif()

# Platform-specific settings
if(WIN32)
    target_link_libraries(vital_ai ws2_32)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(vital_ai pthread)
endif()

# Generate export header
set_target_properties(vital_ai PROPERTIES
    WINDOWS_EXPORT_ALL_SYMBOLS ON
    CXX_VISIBILITY_PRESET hidden
    VISIBILITY_INLINES_HIDDEN ON
)

# Set compilation flags based on optimization level
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(vital_ai PRIVATE -g -O0)
elseif(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(vital_ai PRIVATE -O3 -DNDEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    target_compile_options(vital_ai PRIVATE -O2 -g -DNDEBUG)
elseif(CMAKE_BUILD_TYPE STREQUAL "MinSizeRel")
    target_compile_options(vital_ai PRIVATE -Os -DNDEBUG)
endif()

# Code quality settings
if(ENABLE_STATIC_ANALYSIS)
    target_compile_options(vital_ai PRIVATE 
        -Wall -Wextra -Wpedantic
        -Werror -Wcast-align -Wcast-qual
        -Wconversion -Wsign-conversion
    )
endif()

# Link time optimization
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
        target_link_options(vital_ai PRIVATE -flto)
    elseif(MSVC)
        target_link_options(vital_ai PRIVATE /LTCG)
    endif()
endif()

# Package configuration
install(TARGETS vital_ai
    EXPORT VitalAITargets
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
    INCLUDES DESTINATION include
)

# Install headers
install(FILES ${AI_HEADERS}
    DESTINATION include/vital/ai
)

# Create export set
install(EXPORT VitalAITargets
    FILE VitalAITargets.cmake
    NAMESPACE Vital::
    DESTINATION lib/cmake/vital_ai
)

# Package configuration
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(CPACK_PACKAGE_NAME "libvital-ai")
    set(CPACK_PACKAGE_VENDOR "Vital Audio")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "AI-powered audio synthesis library")
    set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
    set(CPACK_GENERATOR "TGZ;ZIP")
    
    set(CPACK_COMPONENT_UNSPECIFIED_DISPLAY_NAME "Core AI Library")
    set(CPACK_COMPONENT_UNSPECIFIED_DESCRIPTION "Core AI functionality for audio synthesis")
    
    include(CPack)
endif()

# Testing (if enabled)
if(BUILD_TESTING)
    enable_testing()
    
    # Find test framework
    find_package(GTest QUIET)
    
    if(GTest_FOUND)
        # Create test executable
        add_executable(vital_ai_tests
            tests/test_ai_manager.cpp
            tests/test_neural_preset_generator.cpp
            tests/test_machine_learning_engine.cpp
            tests/test_intelligent_audio_analyzer.cpp
            tests/test_adaptive_modulation_system.cpp
            tests/test_style_transfer_engine.cpp
            tests/test_intelligent_preset_generator.cpp
        )
        
        target_link_libraries(vital_ai_tests
            vital_ai
            GTest::gtest_main
            GTest::gmock_main
        )
        
        target_include_directories(vital_ai_tests PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
        )
        
        # Add tests
        add_test(NAME AI_Tests COMMAND vital_ai_tests)
        
        # Coverage (if available)
        if(ENABLE_COVERAGE)
            find_program(LCOV_EXE lcov)
            find_program(GENHTML_EXE genhtml)
            
            if(LCOV_EXE AND GENHTML_EXE)
                set(COVERAGE_DIR "${CMAKE_BINARY_DIR}/coverage")
                
                add_custom_target(coverage
                    COMMAND ${LCOV_EXE} --directory . --capture --output-file coverage.info
                    COMMAND ${GENHTML_EXE} coverage.info --output-directory ${COVERAGE_DIR}
                    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
                    COMMENT "Generating coverage report"
                )
            endif()
        endif()
    else()
        # Simple manual test
        add_executable(vital_ai_test tests/manual_test.cpp)
        target_link_libraries(vital_ai_test vital_ai)
        target_include_directories(vital_ai_test PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
        add_test(NAME Manual_AI_Test COMMAND vital_ai_test)
    endif()
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

# Benchmarks
if(BUILD_BENCHMARKS)
    add_executable(vital_ai_benchmarks
        benchmarks/benchmark_neural_presets.cpp
        benchmarks/benchmark_audio_analysis.cpp
        benchmarks/benchmark_modulation_system.cpp
    )
    
    target_link_libraries(vital_ai_benchmarks vital_ai)
    target_include_directories(vital_ai_benchmarks PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
endif()

# Example applications
add_executable(intelligent_preset_demo examples/preset_generation_demo.cpp)
target_link_libraries(intelligent_preset_demo vital_ai)

add_executable(style_transfer_demo examples/style_transfer_demo.cpp)
target_link_libraries(style_transfer_demo vital_ai)

add_executable(ai_integration_demo examples/ai_integration_demo.cpp)
target_link_libraries(ai_integration_demo vital_ai)

# Summary
message(STATUS "")
message(STATUS "=== Vital AI Configuration Summary ===")
message(STATUS "Version: ${PROJECT_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Compiler: ${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "Build tests: ${BUILD_TESTING}")
message(STATUS "Build benchmarks: ${BUILD_BENCHMARKS}")
message(STATUS "Enable coverage: ${ENABLE_COVERAGE}")
message(STATUS "Static analysis: ${ENABLE_STATIC_ANALYSIS}")
message(STATUS "Optimization level: ${CMAKE_BUILD_TYPE}")
message(STATUS "")

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    message(STATUS "Performance optimizations enabled:")
    message(STATUS "  - Link Time Optimization: ON")
    message(STATUS "  - Vectorization: Enabled")
    message(STATUS "  - Inlining: Aggressive")
    message(STATUS "  - Dead code elimination: Enabled")
endif()

message(STATUS "======================================")
message(STATUS "")