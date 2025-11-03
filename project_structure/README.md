# Vital Synthesizer

A professional C++ synthesizer with advanced DSP capabilities and cross-platform support.

## Features

- **High-Quality Synthesis**: Multiple oscillator types with anti-aliasing
- **Flexible Effects**: Reverb, chorus, delay, distortion, compressor, and EQ
- **MIDI Support**: Full MIDI processing with CC mapping and automation
- **Modern UI**: Cross-platform GUI with customizable themes
- **Plugin Formats**: VST3, Audio Units (macOS), and LV2 support
- **Low Latency**: Optimized for real-time performance
- **Preset Management**: Comprehensive patch loading and saving
- **Modulation Matrix**: Advanced routing and modulation capabilities

## Architecture

The Vital synthesizer is built with a modular architecture:

### Core Components

- **Synthesizer** (`synth/synthesizer.h`): Main synthesis engine
- **Voice Manager** (`synth/voice_manager.h`): Polyphonic voice allocation
- **Voice** (`synth/voice.h`): Individual voice processing
- **Patch** (`synth/patch.h`): Sound parameter management

### DSP Components

- **Oscillator** (`dsp/oscillator.h`): Waveform generation with anti-aliasing
- **Filter** (`dsp/filter.h`): Multi-mode filtering with oversampling
- **Envelope** (`dsp/envelope.h`): ADSR envelope generator
- **LFO** (`dsp/lfo.h`): Low-frequency oscillator for modulation
- **Mixer** (`dsp/mixer.h`): Audio signal mixing and routing

### Audio Effects

- **Reverb** (`effects/reverb.h`): Schroeder reverb algorithm
- **Chorus** (`effects/chorus.h`): Modulated delay line chorus
- **Delay** (`effects/delay.h`): Multi-tap delay system
- **Distortion** (`effects/distortion.h`): Wave-shaping distortion
- **Compressor** (`effects/compressor.h`): Dynamic range compressor
- **EQ** (`effects/eq.h`): Parametric equalizer

### MIDI Processing

- **MIDI Input** (`midi/midi_input.h`): Real-time MIDI message processing
- **MIDI Output** (`midi/midi_output.h`): MIDI data output and clock
- **MIDI Mapping** (`midi/midi_mapping.h`): Parameter mapping and automation

### User Interface

- **Main Window** (`ui/main_window.h`): Application main window
- **Editor Window** (`ui/editor_window.h`): Synthesizer interface
- **UI Components** (`ui/components/`): Reusable UI elements

### Configuration

- **Settings** (`config/settings.h`): Application and audio settings
- **Preset Manager** (`config/preset_manager.h`): Patch storage and management
- **File Handler** (`config/file_handler.h`): File I/O operations

## Building

### Prerequisites

- C++17 compatible compiler (GCC 8+, Clang 8+, MSVC 2019+)
- CMake 3.16 or higher
- Thread library (POSIX threads or Win32 threads)

### Build Instructions

1. **Clone the repository:**
   ```bash
   git clone <repository-url>
   cd vital_application/project_structure
   ```

2. **Create build directory:**
   ```bash
   mkdir build
   cd build
   ```

3. **Configure with CMake:**
   ```bash
   cmake .. -DCMAKE_BUILD_TYPE=Release \
            -DBUILD_VST3=ON \
            -DBUILD_AU=ON \
            -DBUILD_LV2=ON
   ```

4. **Build:**
   ```bash
   cmake --build . -j$(nproc)
   ```

### CMake Options

- `CMAKE_BUILD_TYPE`: Debug or Release
- `BUILD_VST3`: Build VST3 plugin (default: ON)
- `BUILD_AU`: Build Audio Units plugin (macOS only, default: ON)
- `BUILD_LV2`: Build LV2 plugin (default: ON)

## Project Structure

```
vital_application/project_structure/
├── CMakeLists.txt          # Main build configuration
├── README.md               # This file
├── src/                    # Source files
│   ├── core/              # Core application components
│   ├── synth/             # Synthesis engine
│   ├── dsp/               # Digital signal processing
│   ├── effects/           # Audio effects
│   ├── midi/              # MIDI processing
│   ├── ui/                # User interface
│   ├── config/            # Configuration management
│   ├── math/              # Math utilities
│   ├── io/                # I/O operations
│   └── main.cpp           # Application entry point
├── include/                # Header files
│   └── vital/             # Vital library headers
├── build/                  # Build output directory
├── resources/              # Application resources
│   ├── presets/           # Preset patches
│   ├── themes/            # UI themes
│   ├── assets/            # Graphics and assets
│   └── fonts/             # Typefaces
├── tests/                  # Unit and integration tests
├── docs/                   # Documentation
├── scripts/                # Build and deployment scripts
└── third_party/            # External dependencies
```

## Usage

### Standalone Application

Run the standalone version:
```bash
./vital_standalone
```

### Plugin Usage

The synthesizer works as a plugin in DAWs that support:
- VST3 (Windows, macOS, Linux)
- Audio Units (macOS only)
- LV2 (Linux)

### MIDI Control

- Connect MIDI devices for real-time control
- Map MIDI CC to synthesizer parameters
- Use velocity sensitivity for dynamic playing
- MIDI clock sync for tempo-dependent effects

## Performance Optimization

- **Voice Management**: Intelligent polyphony control with voice stealing
- **Anti-Aliasing**: Band-limited synthesis to prevent aliasing
- **Oversampling**: High-quality processing with sample rate multiplication
- **Memory Management**: Efficient buffer allocation and reuse
- **CPU Optimization**: Fixed-point math where appropriate

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new functionality
5. Ensure all tests pass
6. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Dependencies

- **PortAudio**: Cross-platform audio I/O
- **RtMidi**: MIDI I/O library  
- **JUCE**: Audio application framework (optional)
- **Native GUI Libraries**: Platform-specific windowing and graphics

## Roadmap

- [ ] Advanced modulation matrix
- [ ] Spectral effects
- [ ] MPE (MIDI Polyphonic Expression) support
- [ ] Macro controls and snapshots
- [ ] Modulator synchronization
- [ ] Advanced preset management
- [ ] Plugin sidechain input
- [ ] Multi-out support

## Contact

For questions, bug reports, or contributions, please open an issue on the project repository.