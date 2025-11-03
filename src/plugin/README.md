# Vital Plugin Integration

## Overview

The Vital Plugin Integration system provides comprehensive support for running Vital as a plugin inside Digital Audio Workstations (DAWs) through VST3 and AudioUnit formats. This system bridges the powerful VitalAudioEngine with DAW environments, offering full parameter automation, MIDI handling, state management, and a modern user interface.

## Features

### Plugin Format Support
- **VST3**: Full VST3 plugin format support with advanced features
- **AudioUnit**: Native macOS AudioUnit support with CoreAudio integration
- **VST2**: Legacy VST2 support (optional)

### Core Functionality
- **Parameter Management**: Comprehensive parameter system with automation, smoothing, and MIDI learn
- **MIDI Processing**: Advanced MIDI handling with MPE support, CC mapping, and MIDI learn
- **Plugin State**: Complete state management including presets, programs, and DAW synchronization
- **User Interface**: Modern, responsive UI with real-time visualization
- **Performance Monitoring**: Real-time performance metrics and optimization

### Advanced Features
- **Note Expression**: VST3 note expression support for advanced MIDI expression
- **MPE**: Multidimensional Polyphonic Expression support
- **Sidechain**: Sidechain input support for external audio processing
- **Preset Management**: Factory and user preset system with search and categorization
- **MIDI Visualization**: Real-time MIDI event visualization
- **Custom UI**: Modern, themeable user interface with OpenGL acceleration

## Architecture

### Core Components

#### 1. VitalPlugin (vital_plugin.h/cpp)
The main plugin wrapper that inherits from JUCE's AudioProcessor and provides the bridge between the VitalAudioEngine and DAW environments.

**Key Responsibilities:**
- Plugin lifecycle management
- Parameter synchronization with DAW
- MIDI message routing
- Plugin format detection
- Performance monitoring

#### 2. PluginParameters (plugin_parameters.h)
Comprehensive parameter management system with automation support and MIDI learn functionality.

**Features:**
- Parameter registration and validation
- Real-time parameter smoothing
- Automation point management
- MIDI controller mapping
- Parameter categories and groups
- Smart parameter updates

#### 3. PluginState (plugin_state.h)
Plugin state management with preset loading/saving and DAW synchronization.

**Capabilities:**
- Plugin state serialization
- Preset management (factory and user)
- State history with undo/redo
- Auto-save functionality
- Cross-format compatibility

#### 4. PluginMidi (plugin_midi.h)
Advanced MIDI handling system with MPE support and real-time MIDI processing.

**Functionality:**
- MIDI input/output processing
- MPE zone management
- Note expression support
- MIDI learn functionality
- Performance monitoring
- Virtual MIDI support

#### 5. PluginUI (plugin_ui.h)
Modern user interface system with real-time visualization and responsive design.

**Components:**
- Parameter controls (knobs, sliders, buttons, etc.)
- Oscillator panel with waveform visualization
- Filter panel with frequency response display
- Envelope and LFO visualization
- Preset browser
- MIDI visualizer
- Performance monitoring display

### Platform-Specific Wrappers

#### VST3 Wrapper (vst3_wrapper.h)
VST3-specific implementation providing:
- VST3 host communication
- Note expression support
- Parameter automation
- VST3-compliant messaging
- MPE support

#### AudioUnit Wrapper (au_wrapper.h)
AudioUnit-specific implementation providing:
- CoreAudio integration
- AU parameter management
- MIDI processing
- AU property handling
- MPE support

## Implementation Guide

### Building the Plugin

1. **CMake Configuration**
```bash
mkdir build
cd build
cmake .. -DVITAL_BUILD_VST3=ON -DVITAL_BUILD_AU=ON
make -j$(nproc)
```

2. **Platform-Specific Options**
```cmake
# macOS
cmake .. -DVITAL_BUILD_AU=ON -DVITAL_CODE_SIGN=ON

# Windows
cmake .. -DVITAL_BUILD_VST3=ON -DVITAL_BUILD_WIN_INSTALLER=ON
```

### Integration Steps

1. **Include Plugin Headers**
```cpp
#include "vital_plugin.h"
#include "plugin_parameters.h"
#include "plugin_state.h"
#include "plugin_midi.h"
#include "plugin_ui.h"
```

2. **Create Plugin Instance**
```cpp
auto config = VitalPlugin::PluginConfig();
config.maxVoices = 32;
config.enableMPE = true;
config.enableCustomUI = true;

std::unique_ptr<VitalPlugin> plugin = std::make_unique<VitalPlugin>(config);
```

3. **Initialize Plugin**
```cpp
if (plugin->initialize()) {
    plugin->prepareToPlay(44100.0, 512);
    // Plugin is ready for DAW integration
}
```

### Custom Plugin Development

#### Creating Custom Parameters
```cpp
// Create parameter
auto param = std::make_shared<Parameter>(0, "Master Volume", Parameter::Float);
param->setRange({0.0f, 1.0f, 0.5f, 0.01f});
param->setDisplay({"dB", "0.00", 2});

// Add to parameter system
parameters_.addParameter(param);
```

#### Custom UI Components
```cpp
// Create custom oscillator panel
auto oscPanel = std::make_shared<OscillatorPanel>(0);
oscPanel->setSettings({/* oscillator settings */});
ui.addPanel("Oscillator", oscPanel);
```

#### MIDI Learn Implementation
```cpp
// Enable MIDI learn
plugin->getMidiHandler().getMidiLearn().enableLearnMode(true);

// Map parameter to CC
auto mapping = MidiLearn::Mapping();
mapping.parameterId = 0; // Master Volume
mapping.cc = 7; // Volume CC
mapping.channel = 0;
mapping.enabled = true;

plugin->getMidiHandler().getMidiLearn().addMapping(mapping);
```

## API Reference

### VitalPlugin Class

#### Core Methods
```cpp
// Lifecycle
bool initialize();
void shutdown();
void prepareToPlay(double sampleRate, int samplesPerBlock);
void releaseResources();

// Parameter management
void setParameter(int index, float value);
float getParameter(int index) const;
int getNumParameters() const;

// MIDI processing
void handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message);
void processMidiBuffer(const juce::MidiBuffer& buffer);

// Plugin format detection
bool isVST3() const;
bool isAU() const;
juce::String getCurrentPluginFormat() const;
```

#### State Management
```cpp
void getStateInformation(juce::MemoryBlock& destData);
void setStateInformation(const void* data, int sizeInBytes);
bool loadPreset(const juce::File& file);
bool savePreset(const juce::File& file, const juce::String& name = "");
```

#### Performance Monitoring
```cpp
void enablePerformanceMode(bool enabled);
VitalPlugin::RealTimeMetrics getRealTimeMetrics() const;
```

### PluginParameters Class

#### Parameter Management
```cpp
void addParameter(std::shared_ptr<Parameter> parameter);
std::shared_ptr<Parameter> getParameter(int paramId) const;
float getValue(int paramId) const;
void setValue(int paramId, float value);
```

#### MIDI Learn
```cpp
void enableMidiLearn(bool enabled);
bool learnParameter(int paramId, int cc, int channel);
void unlearnParameter(int paramId);
```

#### Automation
```cpp
void setParameterAutomation(int paramId, const std::vector<float>& automation);
void applyModulation(int paramId, float& value, int sample);
```

### PluginMidiHandler Class

#### MIDI Processing
```cpp
void enableInput(bool enabled);
void setInputChannel(int channel);
void processMidiBuffer(const juce::MidiBuffer& buffer);
void sendMessage(const PluginMidiMessage& message);
```

#### MPE Support
```cpp
void enableMPE(bool enabled);
void setMPEZone(int zone, const std::vector<int>& channels);
```

#### Active Note Management
```cpp
void noteOn(int note, float velocity, int channel);
void noteOff(int note, int channel);
void allNotesOff(int channel = -1);
```

### PluginUI Class

#### Panel Management
```cpp
void addPanel(const juce::String& name, std::shared_ptr<UIComponent> panel);
void showPanel(const juce::String& name);
void setActivePanel(const juce::String& name);
```

#### Parameter Controls
```cpp
void addParameterControl(std::shared_ptr<ParameterControl> control);
void updateAllParameterDisplays();
```

#### Theme and Customization
```cpp
void setTheme(const juce::String& themeName);
void setBackgroundColor(const juce::Colour& color);
void setAccentColor(const juce::Colour& color);
```

## Platform-Specific Features

### VST3 Features
- **Note Expression**: Advanced MIDI note expression support
- **Host Communication**: Direct communication with VST3 hosts
- **Parameter Automation**: Native VST3 automation support
- **Plugin Categories**: Proper plugin categorization
- **MPE Support**: Full Multidimensional Polyphonic Expression

### AudioUnit Features
- **CoreAudio Integration**: Native CoreAudio support
- **AU Parameter Management**: AU-specific parameter handling
- **MIDI Processing**: CoreMIDI integration
- **Property Management**: AU property handling
- **Preset Support**: AU preset management

### Cross-Platform Compatibility
- **Shared State**: Common state format across all plugin types
- **Parameter Synchronization**: Consistent parameter behavior
- **MIDI Mapping**: Standard MIDI controller mapping
- **UI Consistency**: Uniform user interface experience

## Performance Optimization

### Real-Time Considerations
- **Lock-Free Queues**: Non-blocking parameter updates
- **Audio Thread Optimization**: Minimal processing overhead
- **SIMD Acceleration**: Vectorized DSP operations
- **Memory Management**: Efficient memory allocation

### Resource Management
- **Voice Stealing**: Smart polyphony management
- **CPU Limiting**: Dynamic load balancing
- **Memory Usage**: Optimized memory footprint
- **Buffer Management**: Efficient buffer handling

### Monitoring and Debugging
- **Performance Metrics**: Real-time performance monitoring
- **Memory Tracking**: Memory usage analysis
- **Error Handling**: Comprehensive error reporting
- **Debug Mode**: Development mode with additional logging

## Error Handling

### Plugin-Level Errors
```cpp
// Check plugin state
if (!plugin->validateState()) {
    auto error = plugin->getLastError();
    juce::Logger::writeToLog("Plugin Error: " + error.message);
}

// Handle parameter errors
try {
    plugin->setParameter(paramId, value);
} catch (const PluginException& e) {
    // Handle parameter error
}
```

### MIDI Handling Errors
```cpp
// Check MIDI status
auto stats = plugin->getMidiHandler().getPerformanceStats();
if (stats.droppedMessages > 0) {
    // Handle dropped MIDI messages
}
```

### Audio Processing Errors
```cpp
// Monitor audio engine state
if (!plugin->getAudioEngine().isInitialized()) {
    plugin->getAudioEngine().initialize();
}
```

## Testing and Validation

### Plugin Testing
```cpp
// Create test plugin instance
auto config = VitalPlugin::PluginConfig();
config.testMode = true;
config.enableDebugOutput = true;

std::unique_ptr<VitalPlugin> testPlugin = std::make_unique<VitalPlugin>(config);

// Run tests
if (testPlugin->initialize()) {
    // Test plugin functionality
    testPlugin->setParameter(0, 0.5f);
    testPlugin->handleIncomingMidiMessage(nullptr, juce::MidiMessage::noteOn(1, 60, 127.0f));
}
```

### Performance Testing
```cpp
// Enable performance monitoring
plugin->enablePerformanceMode(true);

// Run performance tests
auto metrics = plugin->getRealTimeMetrics();
if (metrics.cpuLoad > 0.8f) {
    // Optimize performance
}
```

## Configuration Options

### Build Configuration
```cmake
# Plugin formats
set(VITAL_BUILD_VST3 ON)
set(VITAL_BUILD_AU ON)
set(VITAL_BUILD_VST2 OFF)

# Platform options
set(VITAL_BUILD_MAC_BUNDLE ON)
set(VITAL_CODE_SIGN OFF)
set(VITAL_NOTARIZATION OFF)

# Features
set(VITAL_ENABLE_MPE ON)
set(VITAL_ENABLE_CUSTOM_UI ON)
set(VITAL_ENABLE_PERFORMANCE_MONITORING ON)
```

### Runtime Configuration
```cpp
// Plugin configuration
VitalPlugin::PluginConfig config;
config.maxVoices = 32;
config.enableMPE = true;
config.enableCustomUI = true;
config.enableOversampling = true;
config.oversamplingFactor = 2;
config.enableSIMD = true;
config.enableMultithreading = true;
```

## Installation and Distribution

### macOS Installation
```bash
# VST3
cp Vital.vst3 /Library/Audio/Plug-Ins/VST3/

# AudioUnit
cp Vital.component /Library/Audio/Plug-Ins/Components/
```

### Windows Installation
```bash
# VST3
copy Vital.dll "C:\Program Files\Common Files\VST3\"

# VST2 (legacy)
copy Vital.dll "C:\Program Files\Vst2\"
```

### Linux Installation
```bash
# VST3
cp Vital.so ~/.vst3/
```

## Troubleshooting

### Common Issues

1. **Plugin Not Loading**
   - Check plugin format compatibility
   - Verify DAW supports the plugin format
   - Check plugin dependencies

2. **Audio Issues**
   - Verify sample rate compatibility
   - Check buffer size settings
   - Monitor CPU usage

3. **MIDI Problems**
   - Check MIDI input/output settings
   - Verify MIDI channel configuration
   - Test MIDI learn functionality

4. **UI Issues**
   - Check OpenGL support
   - Verify display scaling
   - Test different UI themes

### Debug Mode
Enable debug mode for detailed logging:
```cpp
plugin->enableDebugMode(true);
plugin->setDebugLogFile("/path/to/debug.log");
```

### Performance Monitoring
```cpp
// Enable performance monitoring
plugin->enablePerformanceMode(true);

// Check performance metrics
auto metrics = plugin->getRealTimeMetrics();
if (metrics.cpuLoad > 0.8f) {
    // Reduce CPU usage
}
```

## Development Roadmap

### Planned Features
- **Plugin Bridge**: Additional plugin formats (LV2, AAX, VST3)
- **Advanced UI**: 3D visualization and enhanced graphics
- **Cloud Integration**: Preset sharing and cloud synchronization
- **AI Features**: Intelligent parameter optimization
- **Mobile Support**: iOS/Android plugin support

### Platform Expansions
- **iOS AudioUnit**: Native iOS AudioUnit support
- **Android VST3**: Android VST3 plugin support
- **Web Audio**: Web-based plugin interface
- **Embedded Systems**: Minimal resource plugin versions

## Contributing

### Code Style
- Follow JUCE coding standards
- Use C++20 features appropriately
- Maintain cross-platform compatibility
- Include comprehensive documentation

### Testing Requirements
- Unit tests for all new features
- Integration tests for plugin formats
- Performance tests for optimization
- Cross-platform validation

### Documentation
- Update API reference
- Include code examples
- Document breaking changes
- Maintain change logs

## License

This plugin integration system is part of the Vital project. See the main project license for details.

## Support

For plugin integration issues:
1. Check the troubleshooting guide
2. Review the API documentation
3. Test with the provided examples
4. Contact the development team

For DAW-specific issues:
1. Check DAW plugin compatibility
2. Verify plugin installation
3. Test with different DAW versions
4. Contact DAW manufacturer support

---

**Vital Plugin Integration v3.0.0**  
*Professional plugin system for Vital synthesizer*  
*Copyright (c) 2025 Vital Synth Team*