cmake_minimum_required(VERSION 3.20)
project(VitalVoiceControl LANGUAGES CXX)

# Set C++20 standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build configuration options
option(VITAL_VOICE_BUILD_EXAMPLES "Build example applications" ON)
option(VITAL_VOICE_BUILD_TESTS "Build unit tests" ON)
option(VITAL_VOICE_ENABLE_PORTAUDIO "Enable PortAudio support" OFF)
option(VITAL_VOICE_ENABLE_JSON "Enable JSON configuration support" ON)
option(VITAL_VOICE_ENABLE_COVERAGE "Enable code coverage reporting" OFF)

# Compiler-specific optimizations
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    add_compile_options(-Wall -Wextra -Wpedantic)
    add_compile_options(-O3 -march=native)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    add_compile_options(/W4)
    add_compile_options(/O2)
endif()

# Find required packages
find_package(Threads REQUIRED)
find_package(PkgConfig QUIET)

# Optional dependencies
if(VITAL_VOICE_ENABLE_PORTAUDIO)
    pkg_check_modules(PORTAUDIO REQUIRED portaudio-2.0)
endif()

if(VITAL_VOICE_ENABLE_JSON)
    find_package(nlohmann_json 3.9.0 QUIET)
    if(NOT nlohmann_json_FOUND)
        message(WARNING "nlohmann_json not found, using simple JSON parser")
    endif()
endif()

# Create library
set(VOICE_CONTROL_SOURCES
    vital_voice_control_system.cpp
    voice_command_recognizer.cpp
    natural_language_controller.cpp
    voice_tutorial_system.cpp
    voice_preset_navigator.cpp
    multi_language_support.cpp
)

set(VOICE_CONTROL_HEADERS
    vital_voice_control.h
)

# Create the main library
add_library(vital_voice_control STATIC ${VOICE_CONTROL_SOURCES} ${VOICE_CONTROL_HEADERS})

# Set library properties
set_target_properties(vital_voice_control PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 1
    PUBLIC_HEADER "${VOICE_CONTROL_HEADERS}"
)

# Include directories
target_include_directories(vital_voice_control
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}
)

# Link libraries
target_link_libraries(vital_voice_control
    PRIVATE
        Threads::Threads
)

# Add PortAudio if enabled
if(VITAL_VOICE_ENABLE_PORTAUDIO AND PORTAUDIO_FOUND)
    target_include_directories(vital_voice_control PRIVATE ${PORTAUDIO_INCLUDE_DIRS})
    target_link_libraries(vital_voice_control PRIVATE ${PORTAUDIO_LIBRARIES})
    target_compile_definitions(vital_voice_control PRIVATE VITAL_VOICE_ENABLE_PORTAUDIO=1)
endif()

# Add JSON support if available
if(VITAL_VOICE_ENABLE_JSON AND nlohmann_json_FOUND)
    target_link_libraries(vital_voice_control PRIVATE nlohmann_json::nlohmann_json)
    target_compile_definitions(vital_voice_control PRIVATE VITAL_VOICE_ENABLE_JSON=1)
endif()

# Add code coverage if enabled
if(VITAL_VOICE_ENABLE_COVERAGE)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        add_compile_options(--coverage -fprofile-arcs -ftest-coverage)
        link_libraries(--coverage)
    endif()
endif()

# Installation
include(GNUInstallDirs)

install(TARGETS vital_voice_control
    EXPORT VitalVoiceControlTargets
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/vital/voice_control
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(EXPORT VitalVoiceControlTargets
    FILE VitalVoiceControlTargets.cmake
    NAMESPACE Vital::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VitalVoiceControl
)

# Create cmake config file
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/VitalVoiceControlConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

export(EXPORT VitalVoiceControlTargets
    FILE "${CMAKE_CURRENT_BINARY_DIR}/VitalVoiceControlTargets.cmake"
    NAMESPACE Vital::
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/VitalVoiceControlConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/VitalVoiceControlConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VitalVoiceControl
)

# Install cmake config files
install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/VitalVoiceControlConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/VitalVoiceControlConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/VitalVoiceControl
)

# Example applications
if(VITAL_VOICE_BUILD_EXAMPLES)
    # Basic voice control example
    add_executable(vital_voice_control_basic
        examples/basic_voice_control.cpp
    )
    
    target_link_libraries(vital_voice_control_basic
        PRIVATE
            vital_voice_control
            Threads::Threads
    )
    
    target_include_directories(vital_voice_control_basic
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
    )
    
    # Tutorial system demo
    add_executable(vital_voice_tutorial_demo
        examples/tutorial_system_demo.cpp
    )
    
    target_link_libraries(vital_voice_tutorial_demo
        PRIVATE
            vital_voice_control
            Threads::Threads
    )
    
    target_include_directories(vital_voice_tutorial_demo
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
    )
    
    # Multi-language demo
    add_executable(vital_voice_multilang_demo
        examples/multilang_demo.cpp
    )
    
    target_link_libraries(vital_voice_multilang_demo
        PRIVATE
            vital_voice_control
            Threads::Threads
    )
    
    target_include_directories(vital_voice_multilang_demo
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
    )
    
    # Create example directory structure
    add_custom_target(copy_voice_assets ALL
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/examples/assets
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/examples/assets/voice_models
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/examples/assets/language_packs
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/examples/assets/tutorials
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/examples/assets/* ${CMAKE_BINARY_DIR}/examples/assets/
        DEPENDS vital_voice_control_basic vital_voice_tutorial_demo vital_voice_multilang_demo
    )
endif()

# Testing
if(VITAL_VOICE_BUILD_TESTS)
    # Enable testing
    enable_testing()
    
    # Find Google Test
    find_package(GTest QUIET)
    if(NOT GTEST_FOUND)
        # Download and build Google Test
        include(FetchContent)
        FetchContent_Declare(
            googletest
            URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        )
        set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
        FetchContent_MakeAvailable(googletest)
        set(GTEST_FOUND TRUE)
    endif()
    
    # Create test executable
    add_executable(vital_voice_control_tests
        tests/test_voice_control.cpp
    )
    
    target_link_libraries(vital_voice_control_tests
        PRIVATE
            vital_voice_control
            GTest::gtest_main
            GTest::gmock
            Threads::Threads
    )
    
    target_include_directories(vital_voice_control_tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}
            ${CMAKE_CURRENT_BINARY_DIR}
    )
    
    # Add test cases
    add_test(NAME VoiceControl_Initialization COMMAND vital_voice_control_tests)
    add_test(NAME VoiceControl_CommandRecognition COMMAND vital_voice_control_tests)
    add_test(NAME VoiceControl_LanguageSupport COMMAND vital_voice_control_tests)
    add_test(NAME VoiceControl_TutorialSystem COMMAND vital_voice_control_tests)
    add_test(NAME VoiceControl_PresetNavigator COMMAND vital_voice_control_tests)
    
    # Code coverage (requires VITAL_VOICE_ENABLE_COVERAGE)
    if(VITAL_VOICE_ENABLE_COVERAGE)
        add_custom_target(coverage
            COMMAND lcov --directory . --capture --output-file coverage.info
            COMMAND genhtml coverage.info --output-directory coverage_report
            DEPENDS vital_voice_control_tests
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        )
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

# Create package
set(CPACK_PACKAGE_NAME "vital-voice-control")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Vital Voice Control System")
set(CPACK_PACKAGE_VENDOR "Vital Audio")
set(CPACK_GENERATOR "TGZ;ZIP")
set(CPACK_PACKAGE_CONTACT "support@vital.audio")
set(CPACK_PACKAGE_LICENSE "Proprietary")

include(CPack)

# Print configuration summary
message(STATUS "==================================================")
message(STATUS "Vital Voice Control System Build Configuration")
message(STATUS "==================================================")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Build Examples: ${VITAL_VOICE_BUILD_EXAMPLES}")
message(STATUS "Build Tests: ${VITAL_VOICE_BUILD_TESTS}")
message(STATUS "PortAudio Support: ${VITAL_VOICE_ENABLE_PORTAUDIO}")
message(STATUS "JSON Support: ${VITAL_VOICE_ENABLE_JSON}")
message(STATUS "Code Coverage: ${VITAL_VOICE_ENABLE_COVERAGE}")
message(STATUS "Installation Prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "==================================================")

# Add a dummy target that does nothing but allows for dependency tracking
add_custom_target(vital_voice_control DEPENDS vital_voice_control)