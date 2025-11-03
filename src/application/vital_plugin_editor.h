// Copyright (c) 2025 Vital Application Developers
// 
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from
// the use of this software.
// 
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#pragma once

#include <JuceHeader.h>
#include <memory>
#include <atomic>
#include <vector>
#include <string>
#include <functional>
#include <optional>

// Forward declarations
namespace vital {
    class VitalPluginProcessor;
    class VitalMainWindow;
}

// UI configuration
namespace vital::ui_config {
    constexpr int MIN_WIDTH = 800;
    constexpr int MIN_HEIGHT = 500;
    constexpr int DEFAULT_WIDTH = 1200;
    constexpr int DEFAULT_HEIGHT = 800;
    constexpr int PARAMETER_COMPONENT_HEIGHT = 40;
    constexpr int KNOBS_PER_ROW = 8;
    constexpr int SECTION_MARGIN = 10;
    constexpr int CONTROL_MARGIN = 5;
}

// Plugin editor class
namespace vital {

class VitalPluginEditor : public juce::AudioProcessorEditor,
                          public juce::Timer,
                          public juce::KeyListener {
public:
    // Constructor/Destructor
    explicit VitalPluginEditor(VitalPluginProcessor& processor);
    ~VitalPluginEditor() override;

    //==============================================================================
    // AudioProcessorEditor overrides
    void paint(juce::Graphics& g) override;
    void resized() override;
    void visibilityChanged() override;
    void focusOfChildComponentChanged(FocusChangeType cause) override;

    //==============================================================================
    // Timer overrides
    void timerCallback() override;

    //==============================================================================
    // KeyListener overrides
    bool keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent) override;
    bool keyStateChanged(bool isKeyDown, juce::Component* originatingComponent) override;

    //==============================================================================
    // Custom methods
    void updateParameterDisplay(int parameterIndex);
    void updateProgramDisplay();
    void updatePerformanceDisplay();
    
    // UI state management
    void saveUISTate();
    void loadUISTate();
    void resetToDefaultLayout();
    
    // Preset management
    void loadPresetByName(const juce::String& presetName);
    void savePresetByName(const juce::String& presetName);
    void randomizeParameters();
    void resetAllParameters();
    
    // Keyboard shortcuts
    void addKeyboardShortcuts();
    void handleKeyboardShortcut(const juce::KeyPress& key);
    
    // Accessibility
    void setAccessibilityEnabled(bool enabled);
    void announceParameterChange(int parameterIndex, float value);
    void announceProgramChange(int programIndex);
    
    // Performance monitoring UI
    void showPerformanceOverlay(bool show);
    void updatePerformanceStats(float cpuLoad, float memoryUsage, float latency);
    
    // Window management
    void setEditorSize(int width, int height);
    void setTitleBarVisible(bool visible);
    void setResizable(bool resizable, bool keepAspectRatio = false);
    
    // Tooltip management
    void setTooltipsEnabled(bool enabled);
    void showTooltip(const juce::String& text, juce::Point<int> position);
    void hideTooltip();
    
    // Theme management
    void setTheme(const juce::String& themeName);
    juce::String getCurrentTheme() const;
    void updateTheme();
    
    // High DPI support
    void setHighDPIEnabled(bool enabled);
    float getUIScale() const;
    void setUIScale(float scale);
    
    // Animation
    void enableAnimations(bool enabled);
    void animateParameterChange(int parameterIndex, float fromValue, float toValue);
    
    // Export/Import
    void exportSettings(const juce::File& file);
    void importSettings(const juce::File& file);
    
    // MIDI learn
    void startMIDILearn(int parameterIndex);
    void stopMIDILearn();
    bool isMIDILearning() const;
    
    // Undo/Redo
    void undo();
    void redo();
    bool canUndo() const;
    bool canRedo() const;

private:
    // Processor reference
    VitalPluginProcessor& processor_;
    
    // UI components
    std::unique_ptr<juce::Component> main_panel_;
    std::unique_ptr<juce::Component> parameter_panel_;
    std::unique_ptr<juce::Component> keyboard_panel_;
    std::unique_ptr<juce::Component> status_panel_;
    
    // Parameter controls
    std::vector<std::unique_ptr<juce::Slider>> parameter_sliders_;
    std::vector<std::unique_ptr<juce::Label>> parameter_labels_;
    std::vector<std::unique_ptr<juce::Button>> parameter_buttons_;
    
    // Program controls
    std::unique_ptr<juce::ComboBox> program_selector_;
    std::unique_ptr<juce::TextButton> load_preset_button_;
    std::unique_ptr<juce::TextButton> save_preset_button_;
    std::unique_ptr<juce::TextButton> randomize_button_;
    std::unique_ptr<juce::TextButton> reset_button_;
    
    // Performance monitoring
    std::unique_ptr<juce::Label> cpu_label_;
    std::unique_ptr<juce::Label> memory_label_;
    std::unique_ptr<juce::Label> latency_label_;
    std::unique_ptr<juce::Component> performance_overlay_;
    
    // MIDI keyboard
    std::unique_ptr<juce::MidiKeyboardComponent> virtual_keyboard_;
    
    // Theme
    juce::LookAndFeel_V4 custom_look_and_feel_;
    juce::String current_theme_ = "default";
    
    // UI state
    bool animations_enabled_ = true;
    bool tooltips_enabled_ = true;
    bool accessibility_enabled_ = false;
    bool high_dpi_enabled_ = true;
    float ui_scale_ = 1.0f;
    
    // Performance
    std::atomic<float> current_cpu_load_{0.0f};
    std::atomic<size_t> current_memory_usage_{0};
    std::atomic<float> current_latency_{0.0f};
    
    // MIDI learn
    std::atomic<int> midi_learn_parameter_{-1};
    std::atomic<bool> is_learning_{false};
    
    // Undo/Redo
    struct ParameterState {
        int parameter_index;
        float value;
    };
    std::vector<std::vector<ParameterState>> undo_stack_;
    std::vector<std::vector<ParameterState>> redo_stack_;
    
    // Animation state
    struct AnimationState {
        int parameter_index;
        float from_value;
        float to_value;
        float progress;
        juce::Timer* timer;
    };
    std::vector<AnimationState> animations_;
    
    // Keyboard shortcuts
    std::map<juce::KeyPress, std::function<void()>> shortcuts_;
    
    // Tooltip
    std::unique_ptr<juce::TooltipWindow> tooltip_window_;
    
    // Initialization methods
    void initializeComponents();
    void initializeLayout();
    void initializeTheme();
    void initializeKeyboardShortcuts();
    void initializeAccessibility();
    
    // Layout methods
    void layoutMainPanel();
    void layoutParameterPanel();
    void layoutKeyboardPanel();
    void layoutStatusPanel();
    void updateLayout();
    
    // UI component methods
    void createParameterSliders();
    void createProgramControls();
    void createPerformanceMonitoring();
    void createVirtualKeyboard();
    void createStatusBar();
    
    // Event handlers
    void onParameterChanged(int parameterIndex, float newValue);
    void onProgramChanged(int programIndex);
    void onKeyboardNotePressed(int midiNote, float velocity);
    void onKeyboardNoteReleased(int midiNote);
    
    // Drawing methods
    void drawBackground(juce::Graphics& g);
    void drawParameters(juce::Graphics& g);
    void drawPerformanceOverlay(juce::Graphics& g);
    void drawAnimations(juce::Graphics& g);
    
    // Utility methods
    void updateParameterLabel(int index, const juce::String& text);
    void animateParameter(int parameterIndex, float targetValue);
    void pushUndoState();
    void clearUndoStack();
    void clearRedoStack();
    
    // Theme helpers
    void applyThemeColors();
    juce::Colour getThemeColor(const juce::String& colorName) const;
    void drawKnob(juce::Graphics& g, juce::Rectangle<float> bounds, float value, const juce::String& label);
    void drawSlider(juce::Graphics& g, juce::Rectangle<float> bounds, float value);
    void drawButton(juce::Graphics& g, juce::Rectangle<float> bounds, const juce::String& text, bool isPressed);
    
    // Accessibility helpers
    void setupAccessibilityLabels();
    void updateAccessibilityDescriptions();
    void handleScreenReaderAnnouncement(const juce::String& announcement);
    
    // Performance monitoring
    void updatePerformanceLabels();
    void drawPerformanceGraph(juce::Graphics& g, juce::Rectangle<float> bounds, const std::vector<float>& data);
    
    // MIDI processing
    void handleMidiMessage(const juce::MidiMessage& message);
    void startMidiLearn(int parameterIndex);
    void stopMidiLearn();
    void handleMidiControl(int controller, int value);
    
    // Constants
    static constexpr int UI_UPDATE_FREQUENCY = 30; // Hz
    
    // JUCE leak detection
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VitalPluginEditor)
};

} // namespace vital
