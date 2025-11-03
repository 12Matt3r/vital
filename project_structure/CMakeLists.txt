cmake_minimum_required(VERSION 3.16)
project(VitalSynthesizer VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic -O2")
endif()

# Find required packages
find_package(PkgConfig REQUIRED)
find_package(Threads REQUIRED)

# Optional dependencies for plugin formats
option(BUILD_VST3 "Build VST3 plugin" ON)
option(BUILD_AU "Build AU plugin (macOS only)" ON)
option(BUILD_LV2 "Build LV2 plugin" ON)

# Include directories
include_directories(include)
include_directories(include/third_party)

# Subdirectories
add_subdirectory(third_party)

# Core components
set(CORE_SOURCES
    src/core/application.cpp
    src/core/version.cpp
    src/core/plugin_interface.cpp
    src/core/plugin_bridge.cpp
)

set(CORE_HEADERS
    include/vital/core/application.h
    include/vital/core/version.h
    include/vital/core/plugin_interface.h
    include/vital/core/plugin_bridge.h
)

# DSP components
set(DSP_SOURCES
    src/dsp/oscillator.cpp
    src/dsp/filter.cpp
    src/dsp/envelope.cpp
    src/dsp/lfo.cpp
    src/dsp/mixer.cpp
    src/dsp/keyboard.cpp
    src/dsp/automation.cpp
)

set(DSP_HEADERS
    include/vital/dsp/oscillator.h
    include/vital/dsp/filter.h
    include/vital/dsp/envelope.h
    include/vital/dsp/lfo.h
    include/vital/dsp/mixer.h
    include/vital/dsp/keyboard.h
    include/vital/dsp/automation.h
)

# Synth components
set(SYNTH_SOURCES
    src/synth/synthesizer.cpp
    src/synth/voice.cpp
    src/synth/voice_manager.cpp
    src/synth/patch.cpp
    src/synth/note_processor.cpp
)

set(SYNTH_HEADERS
    include/vital/synth/synthesizer.h
    include/vital/synth/voice.h
    include/vital/synth/voice_manager.h
    include/vital/synth/patch.h
    include/vital/synth/note_processor.h
)

# Audio effects
set(EFFECTS_SOURCES
    src/effects/reverb.cpp
    src/effects/chorus.cpp
    src/effects/delay.cpp
    src/effects/distortion.cpp
    src/effects/compressor.cpp
    src/effects/eq.cpp
)

set(EFFECTS_HEADERS
    include/vital/effects/reverb.h
    include/vital/effects/chorus.h
    include/vital/effects/delay.h
    include/vital/effects/distortion.h
    include/vital/effects/compressor.h
    include/vital/effects/eq.h
)

# MIDI handling
set(MIDI_SOURCES
    src/midi/midi_input.cpp
    src/midi/midi_output.cpp
    src/midi/midi_parser.cpp
    src/midi/midi_mapping.cpp
)

set(MIDI_HEADERS
    include/vital/midi/midi_input.h
    include/vital/midi/midi_output.h
    include/vital/midi/midi_parser.h
    include/vital/midi/midi_mapping.h
)

# UI components
set(UI_SOURCES
    src/ui/main_window.cpp
    src/ui/editor_window.cpp
    src/ui/components/knob.cpp
    src/ui/components/slider.cpp
    src/ui/components/button.cpp
    src/ui/components/waveform_display.cpp
    src/ui/components/keyboard_widget.cpp
)

set(UI_HEADERS
    include/vital/ui/main_window.h
    include/vital/ui/editor_window.h
    include/vital/ui/components/knob.h
    include/vital/ui/components/slider.h
    include/vital/ui/components/button.h
    include/vital/ui/components/waveform_display.h
    include/vital/ui/components/keyboard_widget.h
)

# Configuration
set(CONFIG_SOURCES
    src/config/settings.cpp
    src/config/file_handler.cpp
    src/config/preset_manager.cpp
)

set(CONFIG_HEADERS
    include/vital/config/settings.h
    include/vital/config/file_handler.h
    include/vital/config/preset_manager.h
)

# Math utilities
set(MATH_SOURCES
    src/math/math_utils.cpp
    src/math/signal_processing.cpp
)

set(MATH_HEADERS
    include/vital/math/math_utils.h
    include/vital/math/signal_processing.h
)

# I/O components
set(IO_SOURCES
    src/io/audio_output.cpp
    src/io/midi_io.cpp
)

set(IO_HEADERS
    include/vital/io/audio_output.h
    include/vital/io/midi_io.h
)

# Combine all sources
set(ALL_SOURCES
    ${CORE_SOURCES}
    ${DSP_SOURCES}
    ${SYNTH_SOURCES}
    ${EFFECTS_SOURCES}
    ${MIDI_SOURCES}
    ${UI_SOURCES}
    ${CONFIG_SOURCES}
    ${MATH_SOURCES}
    ${IO_SOURCES}
)

set(ALL_HEADERS
    ${CORE_HEADERS}
    ${DSP_HEADERS}
    ${SYNTH_HEADERS}
    ${EFFECTS_HEADERS}
    ${MIDI_HEADERS}
    ${UI_HEADERS}
    ${CONFIG_HEADERS}
    ${MATH_HEADERS}
    ${IO_HEADERS}
)

# Create main library
add_library(vital_core ${ALL_SOURCES})
target_link_libraries(vital_core Threads::Threads)

# Create executable (standalone version)
add_executable(vital_standalone src/main.cpp)
target_link_libraries(vital_standalone vital_core)

# Plugin builds
if(BUILD_VST3)
    add_subdirectory(plugin/vst3)
endif()

if(BUILD_AU AND APPLE)
    add_subdirectory(plugin/au)
endif()

if(BUILD_LV2)
    add_subdirectory(plugin/lv2)
endif()

# Testing
enable_testing()
add_subdirectory(tests)

# Installation
install(TARGETS vital_core vital_standalone
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES ${ALL_HEADERS}
    DESTINATION include/vital
)

# Packaging
set(CPACK_PACKAGE_NAME "vital-synthesizer")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_GENERATOR "TGZ")
include(CPack)