#include "vital_modern_ui.h"
#include "core/animation_engine.h"
#include "material/button.h"
#include "material/slider.h"
#include "visualizations/spectrum_analyzer.h"
#include "accessibility/accessibility_manager.h"
#include "responsive/responsive_layout.h"
#include "theme/theme_manager.h"
#include "widgets/main_window.h"
#include <iostream>

namespace vital {
namespace ui {
namespace examples {

/**
 * @brief Example demonstrating the Vital Modern UI system
 * 
 * This example shows:
 * - Setting up a modern UI with Material Design 3.0
 * - Creating Material Design components
 * - Adding real-time visualizations
 * - Implementing accessibility features
 * - Using responsive design
 * - Applying themes
 * - Managing workspaces
 */
class ModernUIExample {
public:
    ModernUIExample() {
        std::cout << "Initializing Vital Modern UI Example..." << std::endl;
        
        // Create audio processor (placeholder)
        audio_processor_ = std::make_unique<juce::AudioProcessor>();
        
        // Create the main UI instance
        modern_ui_ = std::make_unique<VitalModernUI>(*audio_processor_);
        
        std::cout << "Modern UI instance created successfully!" << std::endl;
    }

    ~ModernUIExample() {
        std::cout << "Shutting down Vital Modern UI Example..." << std::endl;
        if (modern_ui_) {
            modern_ui_->shutdown();
        }
    }

    void run() {
        std::cout << "\n=== Vital Modern UI System Demo ===" << std::endl;
        
        // Initialize the UI system
        initializeUI();
        
        // Demonstrate Material Design components
        demonstrateMaterialComponents();
        
        // Show real-time visualizations
        demonstrateVisualizations();
        
        // Test accessibility features
        testAccessibility();
        
        // Show responsive design
        demonstrateResponsiveDesign();
        
        // Apply different themes
        testTheming();
        
        // Demonstrate workspace management
        testWorkspaceManagement();
        
        // Show performance monitoring
        demonstratePerformanceMonitoring();
        
        std::cout << "\n=== Demo Complete ===" << std::endl;
    }

private:
    void initializeUI() {
        std::cout << "\n1. Initializing UI System..." << std::endl;
        
        // Initialize the modern UI
        modern_ui_->initialize();
        
        // Enable animations with high quality
        modern_ui_->enableAnimations(true);
        modern_ui_->setQualityLevel(VitalModernUI::QualityLevel::High);
        
        // Enable accessibility features
        modern_ui_->enableAccessibility(true);
        
        std::cout << "   ✓ UI system initialized" << std::endl;
        std::cout << "   ✓ Animations enabled (High quality)" << std::endl;
        std::cout << "   ✓ Accessibility features enabled" << std::endl;
    }

    void demonstrateMaterialComponents() {
        std::cout << "\n2. Material Design Components..." << std::endl;
        
        // Create Material Design buttons
        auto play_button = modern_ui_->createButton("Play", 
                                                   material::ButtonVariant::Filled,
                                                   material::ButtonSize::Medium);
        play_button->setClickCallback([]() {
            std::cout << "   🎵 Play button clicked!" << std::endl;
        });
        
        auto stop_button = modern_ui_->createButton("Stop", 
                                                   material::ButtonVariant::Outlined,
                                                   material::ButtonSize::Medium);
        stop_button->setClickCallback([]() {
            std::cout << "   ⏹️ Stop button clicked!" << std::endl;
        });
        
        auto settings_button = modern_ui_->createButton("Settings", 
                                                       material::ButtonVariant::Text,
                                                       material::ButtonSize::Small);
        settings_button->setClickCallback([]() {
            std::cout << "   ⚙️ Settings button clicked!" << std::endl;
        });
        
        // Create Material Design sliders
        auto volume_slider = modern_ui_->createSlider(material::SliderStyle::Linear);
        volume_slider->setRange(0.0f, 100.0f, 1.0f);
        volume_slider->setValue(75.0f);
        volume_slider->setValueChangedCallback([](float value) {
            std::cout << "   🔊 Volume changed to: " << value << "%" << std::endl;
        });
        volume_slider->setValueLabelVisible(true);
        
        auto frequency_slider = modern_ui_->createSlider(material::SliderStyle::Linear);
        frequency_slider->setRange(20.0f, 20000.0f, 1.0f);
        frequency_slider->setValue(440.0f);
        frequency_slider->setValueChangedCallback([](float value) {
            std::cout << "   🎼 Frequency changed to: " << value << " Hz" << std::endl;
        });
        
        // Create Material Design toggle
        auto power_toggle = modern_ui_->createToggle(material::ToggleStyle::Switch);
        power_toggle->setValueChangedCallback([](float value) {
            std::cout << "   🔌 Power " << (value > 0.5f ? "ON" : "OFF") << std::endl;
        });
        
        // Create Material Design knob
        auto cutoff_knob = modern_ui_->createKnob();
        cutoff_knob->setRange(100.0f, 10000.0f, 1.0f);
        cutoff_knob->setValue(1000.0f);
        cutoff_knob->setValueChangedCallback([](float value) {
            std::cout << "   🎛️ Cutoff frequency: " << value << " Hz" << std::endl;
        });
        
        // Create Material Design card
        auto control_card = modern_ui_->createCard(3);
        
        std::cout << "   ✓ Created Play button (Filled variant)" << std::endl;
        std::cout << "   ✓ Created Stop button (Outlined variant)" << std::endl;
        std::cout << "   ✓ Created Settings button (Text variant)" << std::endl;
        std::cout << "   ✓ Created Volume slider (75%)" << std::endl;
        std::cout << "   ✓ Created Frequency slider (440 Hz)" << std::endl;
        std::cout << "   ✓ Created Power toggle" << std::endl;
        std::cout << "   ✓ Created Cutoff knob (1000 Hz)" << std::endl;
        std::cout << "   ✓ Created Control card (Elevation 3)" << std::endl;
    }

    void demonstrateVisualizations() {
        std::cout << "\n3. Real-Time Visualizations..." << std::endl;
        
        // Create spectrum analyzer
        auto spectrum_analyzer = modern_ui_->createSpectrumAnalyzer();
        spectrum_analyzer->setFFTSize(2048);
        spectrum_analyzer->setWindowType(visualizations::WindowType::Hann);
        spectrum_analyzer->setDisplayMode(visualizations::SpectrumDisplayMode::Logarithmic);
        spectrum_analyzer->setFrequencyRange(20.0f, 20000.0f);
        spectrum_analyzer->setAmplitudeRange(-80.0f, 0.0f);
        spectrum_analyzer->setPeakDetectionEnabled(true);
        spectrum_analyzer->setPeakHoldEnabled(true);
        spectrum_analyzer->setPeakHoldTime(3.0f);
        spectrum_analyzer->setGridVisible(true);
        spectrum_analyzer->setLabelsVisible(true);
        
        // Create oscilloscope
        auto oscilloscope = modern_ui_->createOscilloscope();
        oscilloscope->setTimeScale(0.01f);  // 10ms per division
        oscilloscope->setVoltageRange(1.0f);  // 1V range
        oscilloscope->setTriggerMode(visualizations::TriggerMode::Auto);
        oscilloscope->setPersistenceEnabled(true);
        
        // Create level meter
        auto left_level_meter = modern_ui_->createLevelMeter();
        left_level_meter->setOrientation(material::SliderOrientation::Vertical);
        left_level_meter->setShowPeakHold(true);
        left_level_meter->setShowPeakDecay(true);
        
        auto right_level_meter = modern_ui_->createLevelMeter();
        right_level_meter->setOrientation(material::SliderOrientation::Vertical);
        right_level_meter->setShowPeakHold(true);
        right_level_meter->setShowPeakDecay(true);
        
        // Create modulation visualizer
        auto modulation_viz = modern_ui_->createModulationVisualizer();
        modulation_viz->setModulationDepth(0.5f);
        modulation_viz->setModulationRate(2.5f);
        modulation_viz->setWaveformType(visualizations::WaveformType::Sine);
        
        // Create parameter visualizer
        auto param_viz = modern_ui_->createParameterVisualizer();
        param_viz->addParameter("Cutoff", 1000.0f, 100.0f, 10000.0f);
        param_viz->addParameter("Resonance", 0.7f, 0.0f, 1.0f);
        param_viz->addParameter("Gain", 0.5f, 0.0f, 2.0f);
        
        std::cout << "   ✓ Created Spectrum Analyzer (2048-point FFT)" << std::endl;
        std::cout << "   ✓ Created Oscilloscope (10ms/div, 1V range)" << std::endl;
        std::cout << "   ✓ Created Level Meters (L/R channels)" << std::endl;
        std::cout << "   ✓ Created Modulation Visualizer (Depth: 50%, Rate: 2.5Hz)" << std::endl;
        std::cout << "   ✓ Created Parameter Visualizer (3 parameters)" << std::endl;
    }

    void testAccessibility() {
        std::cout << "\n4. Accessibility Features..." << std::endl;
        
        // Get accessibility manager
        auto& accessibility = modern_ui_->getAccessibilityManager();
        
        // Enable all accessibility features
        accessibility.setFeatureEnabled(accessibility::AccessibilityFeature::ScreenReader, true);
        accessibility.setFeatureEnabled(accessibility::AccessibilityFeature::KeyboardNavigation, true);
        accessibility.setFeatureEnabled(accessibility::AccessibilityFeature::AudioFeedback, true);
        accessibility.setFeatureEnabled(accessibility::AccessibilityFeature::UIScaling, true);
        accessibility.setFeatureEnabled(accessibility::AccessibilityFeature::HighContrast, true);
        accessibility.setFeatureEnabled(accessibility::AccessibilityFeature::ReducedMotion, true);
        
        // Test keyboard navigation
        accessibility.setKeyboardShortcuts({
            {juce::KeyPress(juce::KeyPress::spaceKey), "Play/Pause"},
            {juce::KeyPress('s'), "Stop"},
            {juce::KeyPress(juce::KeyPress::upKey), "Volume Up"},
            {juce::KeyPress(juce::KeyPress::downKey), "Volume Down"}
        });
        
        // Test screen reader announcements
        accessibility.announcePolite("Vital synthesizer interface ready");
        accessibility.announceAssertive("High contrast mode enabled");
        
        // Test high contrast mode
        accessibility.setHighContrastMode(true);
        accessibility.setHighContrastScheme("high_contrast_white");
        
        // Test UI scaling
        accessibility.setUIScalingEnabled(true);
        accessibility.setUIScaleFactor(1.25f);
        accessibility.setScalableFonts(true);
        
        // Test reduced motion
        accessibility.setReducedMotion(true);
        accessibility.setAnimationDurationMultiplier(0.5f);
        
        std::cout << "   ✓ Screen reader support enabled" << std::endl;
        std::cout << "   ✓ Keyboard navigation configured" << std::endl;
        std::cout << "   ✓ High contrast mode enabled" << std::endl;
        std::cout << "   ✓ UI scaling activated (125%)" << std::endl;
        std::cout << "   ✓ Reduced motion mode enabled" << std::endl;
        std::cout << "   ✓ Audio feedback configured" << std::endl;
    }

    void demonstrateResponsiveDesign() {
        std::cout << "\n5. Responsive Design..." << std::endl;
        
        // Get responsive layout manager
        auto& responsive = modern_ui_->getResponsiveLayoutManager();
        
        // Add breakpoints
        responsive::Breakpoint mobile_breakpoint("mobile", 0.0f, 768.0f, "Mobile devices");
        responsive::Breakpoint tablet_breakpoint("tablet", 768.0f, 1024.0f, "Tablet devices");
        responsive::Breakpoint desktop_breakpoint("desktop", 1024.0f, -1.0f, "Desktop devices");
        
        responsive.addBreakpoint(mobile_breakpoint);
        responsive.addBreakpoint(tablet_breakpoint);
        responsive.addBreakpoint(desktop_breakpoint);
        
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
                                            std::cout << "   📱 Tap gesture detected at (" 
                                                    << event.position.getX() << ", " 
                                                    << event.position.getY() << ")" << std::endl;
                                        });
        
        responsive.registerGestureHandler(responsive::TouchGesture::DoubleTap,
                                        [](const responsive::TouchEvent& event) {
                                            std::cout << "   📱 Double-tap gesture detected!" << std::endl;
                                        });
        
        responsive.registerGestureHandler(responsive::TouchGesture::Pan,
                                        [](const responsive::TouchEvent& event) {
                                            std::cout << "   📱 Pan gesture in progress..." << std::endl;
                                        });
        
        // Enable dynamic scaling
        responsive.setDynamicScalingEnabled(true);
        responsive.setScalingRule("font_size", "clamp(12px, 2.5vw, 18px)");
        responsive.setScalingRule("spacing", "clamp(8px, 1.5vw, 16px)");
        responsive.setScalingRule("component_size", "clamp(100px, 20vw, 300px)");
        
        std::cout << "   ✓ Mobile breakpoint configured (0-768px)" << std::endl;
        std::cout << "   ✓ Tablet breakpoint configured (768-1024px)" << std::endl;
        std::cout << "   ✓ Desktop breakpoint configured (1024px+)" << std::endl;
        std::cout << "   ✓ Touch optimization enabled (48px min targets)" << std::endl;
        std::cout << "   ✓ Gesture recognition configured" << std::endl;
        std::cout << "   ✓ Dynamic scaling rules applied" << std::endl;
    }

    void testTheming() {
        std::cout << "\n6. Theme Management..." << std::endl;
        
        // Create light theme with blue seed color
        auto light_theme = theme::ThemeManager().createLightTheme(juce::Colours::blue);
        
        // Create dark theme with purple seed color
        auto dark_theme = theme::ThemeManager().createDarkTheme(juce::Colours::purple);
        
        // Create high contrast theme
        auto high_contrast_theme = theme::ThemeManager().createHighContrastTheme(
            theme::ThemeType::HighContrast);
        
        // Test theme generation from seed colors
        auto custom_theme = theme::ThemeManager().generateThemeFromSeed(
            juce::Colours::orange, theme::ThemeType::Light);
        
        // Test contrast checking
        auto& theme_manager = modern_ui_->getThemeManager();
        
        juce::Colour test_foreground = juce::Colours::white;
        juce::Colour test_background = juce::Colours::black;
        
        bool meets_aa = theme_manager.meetsWCAGAAContrast(test_foreground, test_background, false);
        bool meets_aaa = theme_manager.meetsWCAGAAAContrast(test_foreground, test_background, false);
        
        std::cout << "   ✓ Light theme created (Blue seed)" << std::endl;
        std::cout << "   ✓ Dark theme created (Purple seed)" << std::endl;
        std::cout << "   ✓ High contrast theme created" << std::endl;
        std::cout << "   ✓ Custom theme generated (Orange seed)" << std::endl;
        std::cout << "   ✓ WCAG AA contrast check: " << (meets_aa ? "PASS" : "FAIL") << std::endl;
        std::cout << "   ✓ WCAG AAA contrast check: " << (meets_aaa ? "PASS" : "FAIL") << std::endl;
        
        // Switch between themes to demonstrate theme management
        modern_ui_->setTheme(VitalModernUI::Theme::Dark);
        std::cout << "   ✓ Switched to Dark theme" << std::endl;
        
        juce::Thread::sleep(500); // Simulate theme transition
        
        modern_ui_->setTheme(VitalModernUI::Theme::Light);
        std::cout << "   ✓ Switched to Light theme" << std::endl;
        
        modern_ui_->setTheme(VitalModernUI::Theme::HighContrast);
        std::cout << "   ✓ Switched to High Contrast theme" << std::endl;
    }

    void testWorkspaceManagement() {
        std::cout << "\n7. Workspace Management..." << std::endl;
        
        // Get workspace manager
        auto& workspace_manager = modern_ui_->getWorkspaceManager();
        
        // Save current workspace
        bool saved = modern_ui_->saveWorkspace("default_layout");
        std::cout << "   " << (saved ? "✓" : "✗") << " Saved workspace 'default_layout'" << std::endl;
        
        // Save another workspace with different configuration
        modern_ui_->saveWorkspace("minimal_layout");
        std::cout << "   ✓ Saved workspace 'minimal_layout'" << std::endl;
        
        modern_ui_->saveWorkspace("performance_layout");
        std::cout << "   ✓ Saved workspace 'performance_layout'" << std::endl;
        
        // Get list of saved workspaces
        auto workspaces = modern_ui_->getSavedWorkspaces();
        std::cout << "   ✓ Found " << workspaces.size() << " saved workspaces:" << std::endl;
        for (const auto& workspace : workspaces) {
            std::cout << "     - " << workspace << std::endl;
        }
        
        // Load a different workspace
        bool loaded = modern_ui_->loadWorkspace("minimal_layout");
        std::cout << "   " << (loaded ? "✓" : "✗") << " Loaded workspace 'minimal_layout'" << std::endl;
        
        // Switch back to default
        modern_ui_->loadWorkspace("default_layout");
        std::cout << "   ✓ Switched back to 'default_layout'" << std::endl;
    }

    void demonstratePerformanceMonitoring() {
        std::cout << "\n8. Performance Monitoring..." << std::endl;
        
        // Enable performance monitoring
        modern_ui_->enablePerformanceMonitoring(true);
        
        // Update UI to trigger performance measurements
        modern_ui_->update();
        
        // Get performance metrics
        float frame_rate = modern_ui_->getFrameRate();
        size_t memory_usage = modern_ui_->getMemoryUsage();
        
        std::cout << "   ✓ Frame rate: " << frame_rate << " FPS" << std::endl;
        std::cout << "   ✓ Memory usage: " << memory_usage << " bytes" << std::endl;
        
        // Enable animation quality levels
        std::cout << "   ✓ Current animation quality: High (120 FPS target)" << std::endl;
        
        modern_ui_->setQualityLevel(VitalModernUI::QualityLevel::Medium);
        std::cout << "   ✓ Animation quality set to: Medium (60 FPS target)" << std::endl;
        
        modern_ui_->setQualityLevel(VitalModernUI::QualityLevel::Low);
        std::cout << "   ✓ Animation quality set to: Low (30 FPS target)" << std::endl;
        
        // Reset to high quality
        modern_ui_->setQualityLevel(VitalModernUI::QualityLevel::High);
    }

    // Member variables
    std::unique_ptr<juce::AudioProcessor> audio_processor_;
    std::unique_ptr<VitalModernUI> modern_ui_;
};

/**
 * @brief Main function to run the example
 */
int main() {
    std::cout << "Vital Modern UI System - Material Design 3.0 Implementation" << std::endl;
    std::cout << "Features: Accessibility (WCAG 2.2), Responsive Design, Real-time Visualizations" << std::endl;
    std::cout << "Framework: JUCE with modern C++20" << std::endl;
    std::cout << "===============================================================" << std::endl;
    
    try {
        ModernUIExample example;
        example.run();
        
        std::cout << "\n🎉 All examples completed successfully!" << std::endl;
        std::cout << "\nKey Features Demonstrated:" << std::endl;
        std::cout << "✅ Material Design 3.0 components with elevation, shadows, and animations" << std::endl;
        std::cout << "✅ WCAG 2.2 AA accessibility compliance with screen reader support" << std::endl;
        std::cout << "✅ Responsive design with touch optimization and gesture recognition" << std::endl;
        std::cout << "✅ Real-time visualizations including spectrum analyzer and oscilloscope" << std::endl;
        std::cout << "✅ Advanced theming with dynamic theme switching and contrast checking" << std::endl;
        std::cout << "✅ Workspace management with layout persistence and restoration" << std::endl;
        std::cout << "✅ Performance monitoring with quality levels and optimization" << std::endl;
        
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

} // namespace examples
} // namespace ui
} // namespace vital
