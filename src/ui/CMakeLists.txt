# Vital Modern UI Module
# Implements Material Design 3.0 interface with WCAG 2.2 accessibility compliance

cmake_minimum_required(VERSION 3.20)
project(vital_modern_ui)

# C++20 requirement
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(JUCE 7.0 REQUIRED CONFIG)
find_package(Threads REQUIRED)
find_package(OpenMP)

# JUCE module definitions
juce_add_module(
    NAME vital_modern_ui
    COMPANY_NAME "Vital Audio"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_MIDI_CONVERTER_AFTER_BUILD TRUE
    MIDI_MANAGER_TYPE juce::AudioDeviceManager
    FORMATS AU VST3 LV2
    PRODUCT_NAME "Vital Modern UI"
)

# Module dependencies
juce_use_module(JUCE::juce_audio_basics)
juce_use_module(JUCE::juce_audio_devices)
juce_use_module(JUCE::juce_audio_formats)
juce_use_module(JUCE::juce_audio_plugin_client)
juce_use_module(JUCE::juce_audio_processors)
juce_use_module(JUCE::juce_audio_utils)
juce_use_module(JUCE::juce_recommended_config_flags)
juce_use_module(JUCE::juce_recommended_lto_flags)
juce_use_module(JUCE::juce_recommended_warning_flags)

# Include directories
juce_generate_juce_header(vital_modern_ui)

target_include_directories(vital_modern_ui
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${JUCE_VISUAL_STUDIO_WINDOWS_DIR}/user_code
        ${CMAKE_CURRENT_BINARY_DIR}/vital_modern_ui
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/core
        ${CMAKE_CURRENT_SOURCE_DIR}/material
        ${CMAKE_CURRENT_SOURCE_DIR}/visualizations
        ${CMAKE_CURRENT_SOURCE_DIR}/accessibility
        ${CMAKE_CURRENT_SOURCE_DIR}/responsive
        ${CMAKE_CURRENT_SOURCE_DIR}/theme
        ${CMAKE_CURRENT_SOURCE_DIR}/widgets
        ${CMAKE_CURRENT_SOURCE_DIR}/layouts
)

# Source files
set(VITAL_MODERN_UI_SOURCES
    # Core components
    vital_modern_ui.h
    core/component.h
    core/component.cpp
    core/ui_manager.h
    core/ui_manager.cpp
    core/animation_engine.h
    core/animation_engine.cpp
    core/layout_manager.h
    core/layout_manager.cpp

    # Material Design components
    material/button.h
    material/button.cpp
    material/slider.h
    material/slider.cpp
    material/knob.h
    material/knob.cpp
    material/toggle.h
    material/toggle.cpp
    material/card.h
    material/card.cpp
    material/tabbar.h
    material/tabbar.cpp
    material/drawer.h
    material/drawer.cpp

    # Visualization components
    visualizations/spectrum_analyzer.h
    visualizations/spectrum_analyzer.cpp
    visualizations/oscilloscope.h
    visualizations/oscilloscope.cpp
    visualizations/spectrogram.h
    visualizations/spectrogram.cpp
    visualizations/waveform_viewer.h
    visualizations/waveform_viewer.cpp
    visualizations/level_meter.h
    visualizations/level_meter.cpp
    visualizations/modulation_visualizer.h
    visualizations/modulation_visualizer.cpp
    visualizations/parameter_visualizer.h
    visualizations/parameter_visualizer.cpp
    visualizations/patch_cable_system.h
    visualizations/patch_cable_system.cpp
    visualizations/visual_preset_manager.h
    visualizations/visual_preset_manager.cpp

    # Accessibility components
    accessibility/accessibility_manager.h
    accessibility/accessibility_manager.cpp
    accessibility/focus_manager.h
    accessibility/focus_manager.cpp
    accessibility/screen_reader_interface.h
    accessibility/screen_reader_interface.cpp
    accessibility/text_to_speech_engine.h
    accessibility/text_to_speech_engine.cpp
    accessibility/ui_scaler.h
    accessibility/ui_scaler.cpp
    accessibility/contrast_theme_manager.h
    accessibility/contrast_theme_manager.cpp

    # Responsive design
    responsive/responsive_layout.h
    responsive/responsive_layout.cpp
    responsive/dynamic_scaler.h
    responsive/dynamic_scaler.cpp
    responsive/touch_gesture_manager.h
    responsive/touch_gesture_manager.cpp
    responsive/mobile_controls.h
    responsive/mobile_controls.cpp
    responsive/dpi_aware_scaler.h
    responsive/dpi_aware_scaler.cpp

    # Theme management
    theme/theme_manager.h
    theme/theme_manager.cpp
    theme/theme_data.h
    theme/theme_data.cpp
    theme/material_theme.h
    theme/material_theme.cpp
    theme/high_contrast_theme.h
    theme/high_contrast_theme.cpp

    # Widgets
    widgets/main_window.h
    widgets/main_window.cpp
    widgets/panel.h
    widgets/panel.cpp
    widgets/dock_manager.h
    widgets/dock_manager.cpp
    widgets/workspace_manager.h
    widgets/workspace_manager.cpp

    # Layouts
    layouts/flex_layout.h
    layouts/flex_layout.cpp
    layouts/grid_layout.h
    layouts/grid_layout.cpp
    layouts/responsive_grid.h
    layouts/responsive_grid.cpp
    layouts/split_layout.h
    layouts/split_layout.cpp
)

# Add sources to target
target_sources(vital_modern_ui PRIVATE ${VITAL_MODERN_UI_SOURCES})

# Compiler definitions
target_compile_definitions(vital_modern_ui
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_VST3_CAN_REPLACE_VST2=0
        JUCE_VST3_EMULATE_MIDI_CC_WITH_VST_EVENTS=1
        JUCE_VST3_ENFORCE_IOMAPPING=1
        JUCE_VST3_ENFORCE_KEEP_ORDER=1
        JUCE_VST3_SUPPORTS_MIDI_EFFECTS=1
        JUCE_VST3_USE_XCB=0
        JUCE_VST3_64BIT=1
        JUCE_VST3_STATIC=0
        JUCE_WANKS_UNALIGNED_VST3=1

        # Vital UI specific definitions
        VITAL_MODERN_UI_API=__declspec(dllexport)
        VITAL_UI_ACCESSIBILITY_ENABLED=1
        VITAL_UI_ANIMATION_ENABLED=1
        VITAL_UI_RESPONSIVE_ENABLED=1
        VITAL_UI_TOUCH_ENABLED=1
        VITAL_UI_VISUALIZATION_ENABLED=1
        VITAL_UI_THEME_MANAGEMENT=1

    PRIVATE
        VITAL_UI_DEBUG=0
        VITAL_UI_PROFILE=0
)

# Link libraries
target_link_libraries(vital_modern_ui
    PUBLIC
        JUCE::juce_audio_basics
        JUCE::juce_audio_devices
        JUCE::juce_audio_formats
        JUCE::juce_audio_processors
        JUCE::juce_audio_utils
        Threads::Threads
    PRIVATE
        # Performance libraries
        $<TARGET_NAME_IF_EXISTS:OpenMP::OpenMP_CXX>
        # Math and DSP libraries
        $<TARGET_NAME_IF_EXISTS:FFTW::fftw3>
        $<TARGET_NAME_IF_EXISTS:FFTW::fftw3f>
        # System libraries
        $<TARGET_NAME_IF_EXISTS:Threads::Threads>
)

# Platform-specific libraries
if(WIN32)
    target_link_libraries(vital_modern_ui PRIVATE
        user32
        gdi32
        opengl32
        glu32
    )
elseif(APPLE)
    target_link_libraries(vital_modern_ui PRIVATE
        "-framework OpenGL"
        "-framework Metal"
        "-framework MetalKit"
        "-framework Cocoa"
        "-framework IOKit"
        "-framework CoreFoundation"
    )
elseif(UNIX)
    target_link_libraries(vital_modern_ui PRIVATE
        GL
        GLU
        X11
        pthread
    )
endif()

# Optimization flags
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_options(vital_modern_ui PRIVATE
        $<$<CONFIG:Release>:-O3 -DNDEBUG>
        $<$<CONFIG:Release>:-flto>
        $<$<CONFIG:Release>:-ffast-math>
        $<$<CONFIG:Release>:-funroll-loops>
        $<$<CONFIG:Release>:-march=native>
    )
elseif(CMAKE_BUILD_TYPE STREQUAL "RelWithDebInfo")
    target_compile_options(vital_modern_ui PRIVATE
        $<$<CONFIG:RelWithDebInfo>:-O3 -DNDEBUG -g>
        $<$<CONFIG:RelWithDebInfo>:-flto>
    )
endif()

# Debug flags
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_options(vital_modern_ui PRIVATE
        $<$<CONFIG:Debug>:-O0 -g>
        $<$<CONFIG:Debug>:-DDEBUG>
        $<$<CONFIG:Debug>:-DVITAL_UI_DEBUG=1>
    )
endif()

# Enable hardware acceleration
if(ENABLE_GPU_ACCELERATION)
    target_compile_definitions(vital_modern_ui PRIVATE
        VITAL_UI_GPU_ACCELERATION=1
    )
endif()

# Enable SIMD optimizations
if(ENABLE_SIMD_OPTIMIZATIONS)
    target_compile_definitions(vital_modern_ui PRIVATE
        VITAL_UI_SIMD_OPTIMIZED=1
    )
    
    # Platform-specific SIMD flags
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(vital_modern_ui PRIVATE
            -msse4.2
            -mavx2
            -mfma
        )
    elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
        target_compile_options(vital_modern_ui PRIVATE
            /arch:AVX2
        )
    endif()
endif()

# Enable sanitizers for debugging
if(ENABLE_SANITIZERS)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(vital_modern_ui PRIVATE
            -fsanitize=address
            -fsanitize=undefined
            -fsanitize=leak
        )
        target_link_options(vital_modern_ui PRIVATE
            -fsanitize=address
            -fsanitize=undefined
            -fsanitize=leak
        )
    endif()
endif()

# Installation
install(TARGETS vital_modern_ui
    EXPORT vital_modern_ui_Targets
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Header installation
install(FILES
    vital_modern_ui.h
    core/component.h
    core/ui_manager.h
    core/animation_engine.h
    core/layout_manager.h
    material/button.h
    material/slider.h
    visualizations/spectrum_analyzer.h
    accessibility/accessibility_manager.h
    responsive/responsive_layout.h
    theme/theme_manager.h
    widgets/main_window.h
    DESTINATION include/vital/ui
)

# Documentation installation
install(DIRECTORY docs/
    DESTINATION share/vital/ui/docs
)

# Example files
install(DIRECTORY examples/
    DESTINATION share/vital/ui/examples
)

# Tests
if(BUILD_TESTS)
    enable_testing()
    
    # Test executable
    add_executable(vital_ui_tests
        tests/test_component.cpp
        tests/test_ui_manager.cpp
        tests/test_animation.cpp
        tests/test_theme.cpp
        tests/test_accessibility.cpp
        tests/test_responsive.cpp
    )
    
    target_link_libraries(vital_ui_tests PRIVATE
        vital_modern_ui
        gtest
        gtest_main
    )
    
    target_include_directories(vital_ui_tests PRIVATE
        tests/
    )
    
    add_test(NAME vital_ui_component_tests COMMAND vital_ui_tests --gtest_filter=ComponentTest.*)
    add_test(NAME vital_ui_theme_tests COMMAND vital_ui_tests --gtest_filter=ThemeTest.*)
    add_test(NAME vital_ui_accessibility_tests COMMAND vital_ui_tests --gtest_filter=AccessibilityTest.*)
    add_test(NAME vital_ui_responsive_tests COMMAND vital_ui_tests --gtest_filter=ResponsiveTest.*)
endif()

# Export configuration
if(CMAKE_INSTALL_INTERFACE_DIRECTORY)
    set(VITAL_MODERN_UI_INCLUDE_DIR "${CMAKE_INSTALL_INTERFACE_DIRECTORY}/vital/ui")
else()
    set(VITAL_MODERN_UI_INCLUDE_DIR "include/vital/ui")
endif()

install(EXPORT vital_modern_ui_Targets
    FILE vital_modern_ui_Targets.cmake
    NAMESPACE vital::
    DESTINATION lib/cmake/vital_modern_ui
)

# Package configuration
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/vital_modern_uiConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/vital_modern_uiConfigVersion.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/vital_modern_uiConfig.cmake"
    DESTINATION lib/cmake/vital_modern_ui
)

# Benchmark support
if(BUILD_BENCHMARKS)
    add_executable(vital_ui_benchmarks
        benchmarks/benchmark_component.cpp
        benchmarks/benchmark_animation.cpp
        benchmarks/benchmark_theme.cpp
        benchmarks/benchmark_responsive.cpp
    )
    
    target_link_libraries(vital_ui_benchmarks PRIVATE
        vital_modern_ui
        benchmark::benchmark
    )
    
    target_include_directories(vital_ui_benchmarks PRIVATE
        benchmarks/
    )
endif()

# Summary
message(STATUS "Vital Modern UI Module Configuration:")
message(STATUS "  - C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "  - Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  - GPU Acceleration: ${ENABLE_GPU_ACCELERATION}")
message(STATUS "  - SIMD Optimizations: ${ENABLE_SIMD_OPTIMIZATIONS}")
message(STATUS "  - Sanitizers: ${ENABLE_SANITIZERS}")
message(STATUS "  - Tests: ${BUILD_TESTS}")
message(STATUS "  - Benchmarks: ${BUILD_BENCHMARKS}")
