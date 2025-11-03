# Vital Modern UI System

A comprehensive modern UI implementation for the Vital synthesizer, featuring Material Design 3.0 interface, WCAG 2.2 accessibility compliance, responsive design, touch optimization, and real-time visualizations built with the JUCE framework.

## ✨ Features

### 🎨 Material Design 3.0 Interface
- **Semantic Color System**: Material You color roles and dynamic color generation
- **Elevation System**: 24 elevation levels with proper shadows and depth
- **Typography**: Complete text style system with responsive font scaling
- **Shape System**: Corner radius scaling for consistent component geometry
- **Component Library**: Buttons, sliders, knobs, toggles, cards, tabs, and drawers
- **Animation System**: GPU-accelerated animations with 12+ easing functions

### ♿ WCAG 2.2 Accessibility Compliance
- **Screen Reader Support**: Full ARIA label and description integration
- **Keyboard Navigation**: Comprehensive focus management and tab order
- **High Contrast Mode**: Multiple pre-defined high contrast schemes
- **Audio Feedback**: Text-to-speech engine with configurable parameters
- **UI Scaling**: Dynamic DPI detection and font-size based scaling
- **Contrast Validation**: Automatic WCAG AA/AAA compliance checking
- **Reduced Motion**: Accessibility-aware animation controls

### 📱 Responsive Design & Touch Optimization
- **Breakpoint System**: Mobile, tablet, and desktop layout breakpoints
- **Touch Gestures**: Tap, double-tap, long-press, pan, swipe, pinch, and rotate
- **Dynamic Scaling**: Context-aware scaling rules for different device types
- **Mobile Optimization**: Touch-friendly controls with proper sizing
- **DPI Awareness**: Device pixel ratio detection and scaling
- **Orientation Support**: Portrait and landscape layout adaptation

### 📊 Real-Time Visualizations
- **Spectrum Analyzer**: FFT-based frequency analysis with windowing functions
- **Oscilloscope**: High-performance time-domain waveform display
- **Spectrogram**: Time-frequency representation with color mapping
- **Level Meters**: Audio level indicators with peak hold and decay
- **Modulation Visualizer**: Real-time modulation depth and rate display
- **Parameter Visualizer**: Interactive parameter monitoring with graphs
- **Patch Cable System**: Visual routing and connection management
- **Visual Preset Manager**: Themed visualization presets

### 🏗️ Modular Architecture
- **Component System**: Base Component class with extensible functionality
- **Plugin Architecture**: Dynamic component loading and registration
- **Event System**: Comprehensive event handling and distribution
- **Layout Management**: Flexible layout system with constraints
- **Workspace Management**: Layout persistence and restoration
- **Theme Management**: Dynamic theme switching and customization

## 🏛️ Architecture

```
src/ui/
├── vital_modern_ui.h              # Main UI interface
├── core/                          # Core UI components
│   ├── component.h/.cpp           # Base component class
│   ├── ui_manager.h               # UI coordination system
│   ├── animation_engine.h         # GPU-accelerated animations
│   └── layout_manager.h           # Responsive layout system
├── material/                      # Material Design components
│   ├── button.h/.cpp              # Material Design buttons
│   ├── slider.h/.cpp              # Linear and circular sliders
│   ├── knob.h/.cpp                # Rotary controls
│   ├── toggle.h/.cpp              # Switches and checkboxes
│   ├── card.h/.cpp                # Elevated containers
│   ├── tabbar.h/.cpp              # Navigation tabs
│   └── drawer.h/.cpp              # Side navigation panels
├── visualizations/                # Real-time visualization components
│   ├── spectrum_analyzer.h/.cpp   # FFT spectrum analysis
│   ├── oscilloscope.h/.cpp        # Waveform display
│   ├── spectrogram.h/.cpp         # Time-frequency visualization
│   ├── waveform_viewer.h/.cpp     # Audio waveform rendering
│   ├── level_meter.h/.cpp         # Audio level indicators
│   ├── modulation_visualizer.h/.cpp  # Modulation display
│   ├── parameter_visualizer.h/.cpp   # Parameter monitoring
│   ├── patch_cable_system.h/.cpp  # Visual routing system
│   └── visual_preset_manager.h/.cpp  # Themed presets
├── accessibility/                 # Accessibility features
│   ├── accessibility_manager.h/.cpp  # Main accessibility coordinator
│   ├── focus_manager.h/.cpp       # Keyboard navigation
│   ├── screen_reader_interface.h/.cpp  # Screen reader integration
│   ├── text_to_speech_engine.h/.cpp  # Audio feedback
│   ├── ui_scaler.h/.cpp           # UI scaling functionality
│   └── contrast_theme_manager.h/.cpp  # High contrast themes
├── responsive/                    # Responsive design
│   ├── responsive_layout.h/.cpp   # Breakpoint-based layouts
│   ├── dynamic_scaler.h/.cpp      # Context-aware scaling
│   ├── touch_gesture_manager.h/.cpp  # Gesture recognition
│   ├── mobile_controls.h/.cpp     # Touch-optimized controls
│   └── dpi_aware_scaler.h/.cpp    # DPI detection and scaling
├── theme/                         # Theme management
│   ├── theme_manager.h/.cpp       # Material Design theming
│   ├── theme_data.h/.cpp          # Theme data structures
│   ├── material_theme.h/.cpp      # Material Design implementation
│   └── high_contrast_theme.h/.cpp # Accessibility themes
├── widgets/                       # High-level widgets
│   ├── main_window.h/.cpp         # Main application window
│   ├── panel.h/.cpp               # Docking panel system
│   ├── dock_manager.h/.cpp        # Panel docking logic
│   └── workspace_manager.h/.cpp   # Layout persistence
└── layouts/                       # Layout systems
    ├── flex_layout.h/.cpp         # Flexbox-like layouts
    ├── grid_layout.h/.cpp         # Grid-based layouts
    ├── responsive_grid.h/.cpp     # Responsive grid system
    └── split_layout.h/.cpp        # Split view layouts
```

## 🚀 Quick Start

### Basic Setup

```cpp
#include "vital_modern_ui.h"

using namespace vital::ui;

// Create the main UI instance
auto ui = std::make_unique<VitalModernUI>(audio_processor);

// Initialize the UI system
ui->initialize();

// Configure basic settings
ui->setTheme(VitalModernUI::Theme::Dark);
ui->enableAnimations(true);
ui->setQualityLevel(VitalModernUI::QualityLevel::High);
ui->enableAccessibility(true);
```

### Creating Material Design Components

```cpp
// Create buttons with different variants
auto play_button = ui->createButton("Play", 
                                   material::ButtonVariant::Filled,
                                   material::ButtonSize::Medium);

auto settings_button = ui->createButton("⚙️ Settings",
                                       material::ButtonVariant::Outlined,
                                       material::ButtonSize::Small);

// Create sliders
auto volume_slider = ui->createSlider(material::SliderStyle::Linear);
volume_slider->setRange(0.0f, 100.0f, 1.0f);
volume_slider->setValue(75.0f);
volume_slider->setValueChangedCallback([](float value) {
    std::cout << "Volume: " << value << "%" << std::endl;
});

// Create knobs
auto cutoff_knob = ui->createKnob();
cutoff_knob->setRange(100.0f, 10000.0f, 1.0f);
cutoff_knob->setValue(1000.0f);

// Create toggles
auto power_toggle = ui->createToggle(material::ToggleStyle::Switch);
power_toggle->setValueChangedCallback([](float value) {
    std::cout << "Power " << (value > 0.5f ? "ON" : "OFF") << std::endl;
});
```

### Adding Real-Time Visualizations

```cpp
// Create spectrum analyzer
auto spectrum = ui->createSpectrumAnalyzer();
spectrum->setFFTSize(2048);
spectrum->setWindowType(visualizations::WindowType::Hann);
spectrum->setDisplayMode(visualizations::SpectrumDisplayMode::Logarithmic);
spectrum->setFrequencyRange(20.0f, 20000.0f);
spectrum->setAmplitudeRange(-80.0f, 0.0f);
spectrum->setPeakDetectionEnabled(true);
spectrum->setPeakHoldEnabled(true);
spectrum->setPeakHoldTime(3.0f);

// Create oscilloscope
auto oscilloscope = ui->createOscilloscope();
oscilloscope->setTimeScale(0.01f);  // 10ms per division
oscilloscope->setVoltageRange(1.0f);  // 1V range
oscilloscope->setTriggerMode(visualizations::TriggerMode::Auto);
oscilloscope->setPersistenceEnabled(true);

// Create level meters
auto left_level = ui->createLevelMeter();
left_level->setOrientation(material::SliderOrientation::Vertical);
left_level->setShowPeakHold(true);

auto right_level = ui->createLevelMeter();
right_level->setOrientation(material::SliderOrientation::Vertical);
right_level->setShowPeakHold(true);
```

### Implementing Accessibility

```cpp
// Get accessibility manager
auto& accessibility = ui->getAccessibilityManager();

// Enable accessibility features
accessibility.setFeatureEnabled(accessibility::AccessibilityFeature::ScreenReader, true);
accessibility.setFeatureEnabled(accessibility::AccessibilityFeature::KeyboardNavigation, true);
accessibility.setFeatureEnabled(accessibility::AccessibilityFeature::AudioFeedback, true);
accessibility.setFeatureEnabled(accessibility::AccessibilityFeature::HighContrast, true);
accessibility.setFeatureEnabled(accessibility::AccessibilityFeature::UIScaling, true);
accessibility.setFeatureEnabled(accessibility::AccessibilityFeature::ReducedMotion, true);

// Configure keyboard shortcuts
accessibility.registerKeyboardShortcut({
    juce::KeyPress(juce::KeyPress::spaceKey), 
    "Play/Pause", 
    "play_button"
});

accessibility.registerKeyboardShortcut({
    juce::KeyPress('s'), 
    "Stop", 
    "stop_button"
});

// Make announcements to screen readers
accessibility.announcePolite("Synthesizer interface ready");
accessibility.announceAssertive("High contrast mode enabled");

// Enable high contrast mode
accessibility.setHighContrastMode(true);
accessibility.setHighContrastScheme("high_contrast_white");

// Configure UI scaling
accessibility.setUIScalingEnabled(true);
accessibility.setUIScaleFactor(1.25f);
accessibility.setScalableFonts(true);

// Set reduced motion
accessibility.setReducedMotion(true);
accessibility.setAnimationDurationMultiplier(0.5f);
```

### Responsive Design Setup

```cpp
// Get responsive layout manager
auto& responsive = ui->getResponsiveLayoutManager();

// Add breakpoints
responsive::Breakpoint mobile("mobile", 0.0f, 768.0f, "Mobile devices");
responsive::Breakpoint tablet("tablet", 768.0f, 1024.0f, "Tablet devices");
responsive::Breakpoint desktop("desktop", 1024.0f, -1.0f, "Desktop devices");

responsive.addBreakpoint(mobile);
responsive.addBreakpoint(tablet);
responsive.addBreakpoint(desktop);

// Set responsive state for mobile
responsive::ResponsiveState mobile_state("mobile", 375.0f, 667.0f, 2.0f, 2.0f);
mobile_state.is_mobile = true;
mobile_state.is_touch_device = true;
mobile_state.orientation = "portrait";

responsive.setResponsiveState(mobile_state);

// Enable touch optimization
responsive.setTouchOptimizationEnabled(true);
responsive.setMinimumTouchTargetSize(48.0f);
responsive.setTouchFeedbackEnabled(true);
responsive.setHapticFeedbackIntensity(0.7f);

// Register gesture handlers
responsive.registerGestureHandler(responsive::TouchGesture::Tap, 
    [](const responsive::TouchEvent& event) {
        std::cout << "Tap detected at (" 
                  << event.position.getX() << ", " 
                  << event.position.getY() << ")" << std::endl;
    });

responsive.registerGestureHandler(responsive::TouchGesture::DoubleTap,
    [](const responsive::TouchEvent& event) {
        std::cout << "Double-tap detected!" << std::endl;
    });

responsive.registerGestureHandler(responsive::TouchGesture::Pan,
    [](const responsive::TouchEvent& event) {
        std::cout << "Pan gesture in progress..." << std::endl;
    });

// Enable dynamic scaling
responsive.setDynamicScalingEnabled(true);
responsive.setScalingRule("font_size", "clamp(12px, 2.5vw, 18px)");
responsive.setScalingRule("spacing", "clamp(8px, 1.5vw, 16px)");
responsive.setScalingRule("component_size", "clamp(100px, 20vw, 300px)");
```

### Theme Management

```cpp
// Get theme manager
auto& theme_manager = ui->getThemeManager();

// Create themes from seed colors
auto light_theme = theme_manager.createLightTheme(juce::Colours::blue);
auto dark_theme = theme_manager.createDarkTheme(juce::Colours::purple);
auto high_contrast_theme = theme_manager.createHighContrastTheme(theme::ThemeType::HighContrast);

// Generate custom themes
auto custom_theme = theme_manager.generateThemeFromSeed(
    juce::Colours::orange, theme::ThemeType::Light);

// Check contrast compliance
bool meets_aa = theme_manager.meetsWCAGAAContrast(
    juce::Colours::white, juce::Colours::black, false);
bool meets_aaa = theme_manager.meetsWCAGAAAContrast(
    juce::Colours::white, juce::Colours::black, false);

// Switch themes
ui->setTheme(VitalModernUI::Theme::Dark);
ui->setTheme(VitalModernUI::Theme::Light);
ui->setTheme(VitalModernUI::Theme::HighContrast);
```

### Workspace Management

```cpp
// Get workspace manager
auto& workspace_manager = ui->getWorkspaceManager();

// Save current workspace
ui->saveWorkspace("default_layout");
ui->saveWorkspace("minimal_layout");
ui->saveWorkspace("performance_layout");

// Load workspace
ui->loadWorkspace("minimal_layout");

// Get list of saved workspaces
auto workspaces = ui->getSavedWorkspaces();
for (const auto& workspace : workspaces) {
    std::cout << "Workspace: " << workspace << std::endl;
}
```

### Main Window Creation

```cpp
// Create main window
auto main_window = ui->createMainWindow();
main_window->setTitle("Vital Synthesizer");
main_window->setBounds(juce::Rectangle<int>(100, 100, 1200, 800));

// Configure title bar
auto title_config = widgets::TitleBarConfig();
title_config.show_title = true;
title_config.show_icon = true;
title_config.show_minimize_button = true;
title_config.show_maximize_button = true;
title_config.show_close_button = true;
title_config.height = 40.0f;
main_window->setTitleBarConfig(title_config);

// Configure status bar
auto status_config = widgets::StatusBarConfig();
status_config.show_status_bar = true;
status_config.height = 24.0f;
status_config.show_progress = true;
status_config.show_clock = true;
main_window->setStatusBarConfig(status_config);

// Set content
main_window->setContentComponent(content_component);

// Save and load workspaces
main_window->saveWorkspace("my_layout");
main_window->loadWorkspace("my_layout");

// Add docking panels
main_window->addDockingPanel(left_panel, "left", juce::Point<int>(200, 200));
main_window->addDockingPanel(right_panel, "right", juce::Point<int>(200, 200));
main_window->addDockingPanel(bottom_panel, "bottom", juce::Point<int>(800, 200));
```

## 🎯 Performance

### Performance Targets
- **Frame Rate**: 60 FPS sustained at all quality levels (120 FPS on high-end hardware)
- **GPU Acceleration**: OpenGL/Vulkan-based rendering for all graphics
- **Memory Usage**: Optimized vertex buffers and texture atlases
- **CPU Budget**: <5ms per frame for UI rendering
- **Audio Thread**: Non-blocking visualization updates

### Quality Levels

#### Low Quality (30 FPS)
- Reduced shadow blur and opacity
- Simplified animations
- Lower resolution textures
- Disabled complex effects

#### Medium Quality (60 FPS)
- Standard Material Design shadows
- Full animation set
- Normal texture resolution
- Most visual effects enabled

#### High Quality (120 FPS)
- Enhanced shadows and depth
- All animations at highest performance
- High resolution textures
- All visual effects enabled

#### Maximum Quality (Unlimited)
- No performance limits
- Maximum visual fidelity
- All experimental features enabled

## ♿ Accessibility Features

### WCAG 2.2 AA Compliance
- **Color Contrast**: Automatic contrast ratio checking (4.5:1 normal, 3:1 large)
- **Keyboard Navigation**: Full keyboard accessibility with logical tab order
- **Screen Reader Support**: ARIA labels, descriptions, and live regions
- **Focus Management**: Clear focus indicators and focus trapping
- **Text Scaling**: Support for 200% text zoom without horizontal scrolling
- **Motion Preferences**: Respect for system motion preferences
- **Color Independence**: Information not conveyed by color alone

### Screen Reader Integration
```cpp
accessibility.announceToScreenReader("Parameter adjusted to 75%");
accessibility.updateComponentState(component, "value", "75");
accessibility.updateComponentSelection(component, true);
```

### High Contrast Support
```cpp
accessibility.setHighContrastMode(true);
accessibility.setHighContrastScheme("high_contrast_white");
bool valid = theme_manager.meetsWCAGAAContrast(foreground, background);
```

## 📱 Touch and Mobile Support

### Touch Optimization
- **Minimum Touch Targets**: 44-48px minimum touch target size
- **Gesture Recognition**: Comprehensive gesture detection
- **Haptic Feedback**: Configurable haptic feedback for touch interactions
- **Touch-friendly Controls**: Optimized spacing and sizing for mobile
- **Orientation Support**: Automatic layout adaptation for portrait/landscape

### Mobile Layouts
```cpp
// Automatic mobile optimization
responsive.applyMobileOptimizations();

// Custom mobile layout rules
responsive.addLayoutRule(component, {
    .breakpoint_name = "mobile",
    .flex_grow = 1.0f,
    .width_rule = "100%",
    .margin = 8.0f,
    .padding = juce::Rectangle<float>(16.0f)
});
```

## 🎨 Theming System

### Material Design 3.0
- **Dynamic Color**: Automatic color generation from seed colors
- **Semantic Colors**: Primary, secondary, tertiary, error, surface colors
- **Elevation System**: 24 elevation levels with proper shadows
- **Typography Scale**: Complete text style hierarchy
- **Shape System**: Corner radius scaling for consistency

### Custom Theme Creation
```cpp
auto custom_theme = theme_manager.generateThemeFromSeed(
    juce::Colour::fromString("#FF6B35"), theme::ThemeType::Light);

// Save custom theme
theme_manager.registerTheme(custom_theme);
theme_manager.setCurrentTheme("Custom Orange Theme", true);
```

## 🔧 Build Configuration

### CMake Options
```bash
# Enable GPU acceleration
cmake -DENABLE_GPU_ACCELERATION=ON ..

# Enable SIMD optimizations
cmake -DENABLE_SIMD_OPTIMIZATIONS=ON ..

# Enable sanitizers for debugging
cmake -DENABLE_SANITIZERS=ON ..

# Build with tests
cmake -DBUILD_TESTS=ON ..

# Build with benchmarks
cmake -DBUILD_BENCHMARKS=ON ..
```

### Dependencies
- **JUCE 7.0+**: Audio framework and UI system
- **C++20**: Modern C++ features
- **OpenGL/Vulkan**: GPU acceleration (optional)
- **FFTW**: Fast Fourier Transform library (optional)
- **Threads**: Multi-threading support

## 🧪 Testing

### Unit Tests
```bash
# Build and run tests
make test
```

### Example Applications
```bash
# Build example application
make vital_ui_example

# Run the comprehensive demo
./vital_ui_example
```

## 📚 Examples

### Basic Material Design App
See `examples/basic_material_app.cpp` for a complete example of creating a Material Design interface.

### Accessibility Demo
See `examples/accessibility_demo.cpp` for comprehensive accessibility feature demonstration.

### Mobile Interface
See `examples/mobile_interface.cpp` for touch-optimized responsive design.

### Real-time Visualizations
See `examples/visualization_demo.cpp` for advanced real-time visualization examples.

## 🤝 Contributing

### Code Style
- Follow Modern C++20 best practices
- Use RAII and smart pointers
- Implement comprehensive error handling
- Add detailed documentation
- Include unit tests for new features

### Accessibility Standards
- All new components must support keyboard navigation
- ARIA attributes required for screen reader compatibility
- Contrast ratios must meet WCAG AA standards
- Motion alternatives required for animated features

## 📄 License

This implementation is part of the Vital synthesizer project and follows the same licensing terms.

## 🙏 Acknowledgments

- **Material Design Team**: For the excellent design system
- **JUCE Team**: For the powerful audio and UI framework
- **Web Accessibility Initiative**: For WCAG guidelines
- **Audio Community**: For feedback and contributions

---

Built with ❤️ for the audio and accessibility communities
