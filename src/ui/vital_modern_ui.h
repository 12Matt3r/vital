#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <JuceHeader.h>

// Forward declarations
namespace vital {
namespace ui {
    class VitalModernUI;
    class Component;
    class ThemeManager;
    class AnimationEngine;
    class AccessibilityManager;
    class ResponsiveLayoutManager;
}
}

// Core component includes
#include "core/component.h"
#include "core/ui_manager.h"
#include "core/animation_engine.h"
#include "core/layout_manager.h"

// Material Design components
#include "material/button.h"
#include "material/slider.h"
#include "material/knob.h"
#include "material/toggle.h"
#include "material/card.h"
#include "material/tabbar.h"
#include "material/drawer.h"

// Visualization components
#include "visualizations/spectrum_analyzer.h"
#include "visualizations/oscilloscope.h"
#include "visualizations/spectrogram.h"
#include "visualizations/waveform_viewer.h"
#include "visualizations/level_meter.h"
#include "visualizations/modulation_visualizer.h"
#include "visualizations/parameter_visualizer.h"
#include "visualizations/patch_cable_system.h"
#include "visualizations/visual_preset_manager.h"

// Accessibility components
#include "accessibility/accessibility_manager.h"
#include "accessibility/focus_manager.h"
#include "accessibility/screen_reader_interface.h"
#include "accessibility/text_to_speech_engine.h"
#include "accessibility/ui_scaler.h"
#include "accessibility/contrast_theme_manager.h"

// Responsive design
#include "responsive/responsive_layout.h"
#include "responsive/dynamic_scaler.h"
#include "responsive/touch_gesture_manager.h"
#include "responsive/mobile_controls.h"
#include "responsive/dpi_aware_scaler.h"

// Theme management
#include "theme/theme_manager.h"
#include "theme/theme_data.h"
#include "theme/material_theme.h"
#include "theme/high_contrast_theme.h"

// Widgets
#include "widgets/main_window.h"
#include "widgets/panel.h"
#include "widgets/dock_manager.h"
#include "widgets/workspace_manager.h"

// Layouts
#include "layouts/flex_layout.h"
#include "layouts/grid_layout.h"
#include "layouts/responsive_grid.h"
#include "layouts/split_layout.h"

namespace vital {
namespace ui {

/**
 * @brief Main modern UI interface for Vital synthesizer
 * 
 * This is the main entry point for the Vital modern UI system.
 * Provides:
 * - Material Design 3.0 interface
 * - WCAG 2.2 accessibility compliance
 * - Responsive design with touch optimization
 * - Real-time visualizations
 * - Modular component architecture using JUCE framework
 */
class VITAL_MODERN_UI_API VitalModernUI : public juce::Component {
public:
    /**
     * @brief Theme options
     */
    enum class Theme {
        Light,
        Dark,
        HighContrast,
        Custom
    };

    /**
     * @brief Animation quality levels
     */
    enum class QualityLevel {
        Low,      // 30 FPS, reduced effects
        Medium,   // 60 FPS, standard effects
        High,     // 120 FPS, full effects
        Maximum   // Unlimited FPS, all effects
    };

    /**
     * @brief Constructor
     * @param audio_processor Reference to audio processor for visualizations
     */
    explicit VitalModernUI(juce::AudioProcessor& audio_processor);

    /**
     * @brief Destructor
     */
    ~VitalModernUI() override;

    //==============================================================================
    // Initialization and Configuration
    void initialize();
    void shutdown();

    /**
     * @brief Set theme
     * @param theme Theme to apply
     */
    void setTheme(Theme theme);

    /**
     * @brief Get current theme
     */
    Theme getTheme() const { return current_theme_; }

    /**
     * @brief Enable/disable animations
     * @param enabled Whether animations are enabled
     */
    void enableAnimations(bool enabled);

    /**
     * @brief Set animation quality level
     * @param quality Quality level
     */
    void setQualityLevel(QualityLevel quality);

    /**
     * @brief Get animation quality level
     */
    QualityLevel getQualityLevel() const { return quality_level_; }

    //==============================================================================
    // Material Design Components
    /**
     * @brief Create a Material Design button
     * @param text Button text
     * @param variant Button variant
     * @param size Button size
     * @return Created button component
     */
    std::unique_ptr<material::Button> createButton(
        const juce::String& text = "",
        material::ButtonVariant variant = material::ButtonVariant::Filled,
        material::ButtonSize size = material::ButtonSize::Medium
    );

    /**
     * @brief Create a Material Design slider
     * @param style Slider style
     * @return Created slider component
     */
    std::unique_ptr<material::Slider> createSlider(material::SliderStyle style = material::SliderStyle::Linear);

    /**
     * @brief Create a Material Design knob
     * @return Created knob component
     */
    std::unique_ptr<material::Knob> createKnob();

    /**
     * @brief Create a Material Design toggle
     * @param style Toggle style
     * @return Created toggle component
     */
    std::unique_ptr<material::Toggle> createToggle(material::ToggleStyle style = material::ToggleStyle::Switch);

    /**
     * @brief Create a Material Design card
     * @param elevation Elevation level
     * @return Created card component
     */
    std::unique_ptr<material::Card> createCard(int elevation = 2);

    /**
     * @ Create a Material Design tab bar
     * @return Created tab bar component
     */
    std::unique_ptr<material::TabBar> createTabBar();

    /**
     * @brief Create a Material Design drawer
     * @param position Drawer position
     * @return Created drawer component
     */
    std::unique_ptr<material::Drawer> createDrawer(material::DrawerPosition position);

    //==============================================================================
    // Visualization Components
    /**
     * @brief Create a spectrum analyzer
     * @return Created spectrum analyzer component
     */
    std::unique_ptr<visualizations::SpectrumAnalyzer> createSpectrumAnalyzer();

    /**
     * @brief Create an oscilloscope
     * @return Created oscilloscope component
     */
    std::unique_ptr<visualizations::Oscilloscope> createOscilloscope();

    /**
     * @brief Create a spectrogram
     * @return Created spectrogram component
     */
    std::unique_ptr<visualizations::Spectrogram> createSpectrogram();

    /**
     * @brief Create a waveform viewer
     * @return Created waveform viewer component
     */
    std::unique_ptr<visualizations::WaveformViewer> createWaveformViewer();

    /**
     * @brief Create a level meter
     * @return Created level meter component
     */
    std::unique_ptr<visualizations::LevelMeter> createLevelMeter();

    /**
     * @brief Create a modulation visualizer
     * @return Created modulation visualizer component
     */
    std::unique_ptr<visualizations::ModulationVisualizer> createModulationVisualizer();

    /**
     * @brief Create a parameter visualizer
     * @return Created parameter visualizer component
     */
    std::unique_ptr<visualizations::ParameterVisualizer> createParameterVisualizer();

    /**
     * @brief Create a patch cable system
     * @return Created patch cable system component
     */
    std::unique_ptr<visualizations::PatchCableSystem> createPatchCableSystem();

    //==============================================================================
    // Widgets and Layouts
    /**
     * @brief Create a main window
     * @return Created main window component
     */
    std::unique_ptr<widgets::MainWindow> createMainWindow();

    /**
     * @brief Create a dockable panel
     * @param title Panel title
     * @return Created panel component
     */
    std::unique_ptr<widgets::Panel> createPanel(const juce::String& title = "");

    /**
     * @brief Create a flexible layout
     * @return Created flex layout component
     */
    std::unique_ptr<layouts::FlexLayout> createFlexLayout();

    /**
     * @brief Create a grid layout
     * @return Created grid layout component
     */
    std::unique_ptr<layouts::GridLayout> createGridLayout();

    /**
     * @brief Create a responsive grid
     * @return Created responsive grid component
     */
    std::unique_ptr<layouts::ResponsiveGrid> createResponsiveGrid();

    //==============================================================================
    // Accessibility
    /**
     * @brief Get accessibility manager
     * @return Accessibility manager reference
     */
    accessibility::AccessibilityManager& getAccessibilityManager();

    /**
     * @brief Get focus manager
     * @return Focus manager reference
     */
    accessibility::FocusManager& getFocusManager();

    /**
     * @brief Enable/disable accessibility features
     * @param enabled Whether accessibility is enabled
     */
    void enableAccessibility(bool enabled);

    /**
     * @brief Check if accessibility is enabled
     */
    bool isAccessibilityEnabled() const { return accessibility_enabled_; }

    //==============================================================================
    // Workspace Management
    /**
     * @brief Get workspace manager
     * @return Workspace manager reference
     */
    widgets::WorkspaceManager& getWorkspaceManager();

    /**
     * @brief Save current workspace
     * @param name Workspace name
     * @return True if saved successfully
     */
    bool saveWorkspace(const juce::String& name);

    /**
     * @brief Load workspace
     * @param name Workspace name
     * @return True if loaded successfully
     */
    bool loadWorkspace(const juce::String& name);

    /**
     * @brief Get list of saved workspaces
     */
    std::vector<juce::String> getSavedWorkspaces() const;

    //==============================================================================
    // Responsive Design
    /**
     * @brief Get responsive layout manager
     * @return Responsive layout manager reference
     */
    responsive::ResponsiveLayoutManager& getResponsiveLayoutManager();

    /**
     * @brief Set responsive design breakpoints
     * @param breakpoints Breakpoint configurations
     */
    void setBreakpoints(const responsive::Breakpoints& breakpoints);

    /**
     * @brief Get current responsive state
     */
    responsive::ResponsiveState getResponsiveState() const;

    //==============================================================================
    // JUCE Component Overrides
    void paint(juce::Graphics& g) override;
    void resized() override;
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;
    bool keyPressed(const juce::KeyPress& key) override;
    void focusGained(FocusChangeType cause) override;
    void focusLost(FocusChangeType cause) override;

    //==============================================================================
    // Component Registration
    /**
     * @brief Register component with UI system
     * @param component Component to register
     */
    void registerComponent(Component* component);

    /**
     * @brief Unregister component from UI system
     * @param component Component to unregister
     */
    void unregisterComponent(Component* component);

    //==============================================================================
    // Performance Monitoring
    /**
     * @brief Enable performance monitoring
     * @param enabled Whether monitoring is enabled
     */
    void enablePerformanceMonitoring(bool enabled);

    /**
     * @brief Get frame rate
     */
    float getFrameRate() const;

    /**
     * @brief Get memory usage
     */
    size_t getMemoryUsage() const;

private:
    //==============================================================================
    // Private member variables
    juce::AudioProcessor& audio_processor_;
    Theme current_theme_ = Theme::Light;
    QualityLevel quality_level_ = QualityLevel::Medium;
    bool animations_enabled_ = true;
    bool accessibility_enabled_ = true;
    bool performance_monitoring_enabled_ = false;

    // Core managers
    std::unique_ptr<core::UIManager> ui_manager_;
    std::unique_ptr<theme::ThemeManager> theme_manager_;
    std::unique_ptr<core::AnimationEngine> animation_engine_;
    std::unique_ptr<accessibility::AccessibilityManager> accessibility_manager_;
    std::unique_ptr<responsive::ResponsiveLayoutManager> responsive_manager_;
    std::unique_ptr<widgets::WorkspaceManager> workspace_manager_;

    // Performance tracking
    mutable std::atomic<float> frame_rate_{60.0f};
    mutable std::atomic<size_t> memory_usage_{0};

    // Component tracking
    std::vector<Component*> registered_components_;
    mutable juce::CriticalSection component_list_mutex_;

    //==============================================================================
    // Private methods
    void setupDefaultTheme();
    void setupDefaultBreakpoints();
    void updateAllComponents();
    void updateFrameRate();
    void updateMemoryUsage();

    // Theme switching
    void applyThemeColors();
    void applyHighContrastMode();
    void applyCustomTheme();

    // Performance optimizations
    void optimizeRendering();
    void batchUpdateComponents();

    // Accessibility integration
    void setupAccessibilityDefaults();
    void integrateWithScreenReader();

    // Responsive design setup
    void setupResponsiveDefaults();
    void updateResponsiveLayouts();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VitalModernUI)
};

} // namespace ui
} // namespace vital
