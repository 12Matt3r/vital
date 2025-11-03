#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <JuceHeader.h>
#include "../core/component.h"

namespace vital {
namespace ui {
namespace accessibility {

/**
 * @brief Accessibility feature types
 */
enum class AccessibilityFeature {
    ScreenReader,
    KeyboardNavigation,
    HighContrast,
    AudioFeedback,
    UIScaling,
    FocusIndicators,
    ReducedMotion,
    TouchOptimization,
    VoiceControl,
    AllFeatures
};

/**
 * @brief Focus management states
 */
enum class FocusState {
    None,
    ParameterControl,
    TransportControl,
    Navigation,
    PresetBrowser,
    Modulation,
    Effects,
    Custom
};

/**
 * @brief Screen reader announcement priorities
 */
enum class AnnouncementPriority {
    Polite,     // Non-interrupting
    Assertive,  // Interrupting
    Status      // Status updates
};

/**
 * @brief Screen reader announcement
 */
struct ScreenReaderAnnouncement {
    juce::String text;
    juce::String priority = "polite";  // "polite", "assertive", "status"
    juce::String aria_live_region;
    juce::String component_id;
    juce::String timestamp;

    ScreenReaderAnnouncement() = default;
    ScreenReaderAnnouncement(const juce::String& message, 
                           const juce::String& prior = "polite")
        : text(message), priority(prior) {}
};

/**
 * @brief Component accessibility information
 */
struct ComponentAccessibilityInfo {
    juce::String id;
    juce::String role;                    // "button", "slider", "checkbox", etc.
    juce::String label;
    juce::String description;
    juce::String value_text;              // Current value text
    juce::String value_min;
    juce::String value_max;
    juce::String value_now;
    bool is_focusable = true;
    bool is_announced = true;
    juce::String keyboard_shortcut;
    int tab_index = 0;
    juce::String group_name;
    bool is_expanded = false;
    bool is_selected = false;
    juce::String error_message;
    juce::String help_text;

    ComponentAccessibilityInfo() = default;
    ComponentAccessibilityInfo(const juce::String& id_str, const juce::String& role_str)
        : id(id_str), role(role_str) {}
};

/**
 * @brief Keyboard shortcut configuration
 */
struct KeyboardShortcut {
    juce::KeyPress key;
    juce::String description;
    juce::String component_id;
    std::function<void()> action;

    KeyboardShortcut() = default;
    KeyboardShortcut(const juce::KeyPress& keypress, 
                    const juce::String& desc,
                    const juce::String& comp_id)
        : key(keypress), description(desc), component_id(comp_id) {}
};

/**
 * @brief AccessibilityManager - Comprehensive accessibility system
 * 
 * Provides WCAG 2.2 AA compliance including:
 * - Screen reader support with ARIA attributes
 * - Full keyboard navigation
 * - High contrast mode
 * - Audio feedback and text-to-speech
 * - UI scaling for visually impaired users
 * - Focus management and indicators
 * - Reduced motion support
 * - Touch optimization
 */
class VITAL_MODERN_UI_API AccessibilityManager {
public:
    /**
     * @brief Constructor
     */
    AccessibilityManager();

    /**
     * @brief Destructor
     */
    ~AccessibilityManager();

    //==============================================================================
    // Feature Management
    /**
     * @brief Enable/disable accessibility feature
     * @param feature Feature to toggle
     * @param enabled Whether feature is enabled
     */
    void setFeatureEnabled(AccessibilityFeature feature, bool enabled);

    /**
     * @brief Check if feature is enabled
     * @param feature Feature to check
     */
    bool isFeatureEnabled(AccessibilityFeature feature) const;

    /**
     * @brief Get all enabled features
     */
    std::vector<AccessibilityFeature> getEnabledFeatures() const;

    /**
     * @brief Enable all accessibility features
     */
    void enableAllFeatures();

    /**
     * @brief Disable all accessibility features
     */
    void disableAllFeatures();

    /**
     * @brief Set system accessibility preferences
     * @param preferences System accessibility preferences
     */
    void setSystemPreferences(const juce::String& preferences);

    /**
     * @brief Get system accessibility preferences
     */
    juce::String getSystemPreferences() const;

    //==============================================================================
    // Component Registration
    /**
     * @brief Register component for accessibility
     * @param component Component to register
     * @param accessibility_info Accessibility information
     */
    void registerComponent(Component* component, 
                          const ComponentAccessibilityInfo& accessibility_info);

    /**
     * @brief Unregister component from accessibility
     * @param component Component to unregister
     */
    void unregisterComponent(Component* component);

    /**
     * @brief Update component accessibility information
     * @param component Component to update
     * @param accessibility_info New accessibility information
     */
    void updateComponentAccessibilityInfo(Component* component,
                                         const ComponentAccessibilityInfo& accessibility_info);

    /**
     * @brief Get component accessibility information
     * @param component Component to query
     * @return Accessibility information or default if not found
     */
    ComponentAccessibilityInfo getComponentAccessibilityInfo(Component* component) const;

    /**
     * @brief Check if component is registered
     * @param component Component to check
     */
    bool isComponentRegistered(Component* component) const;

    /**
     * @brief Get all registered components
     */
    std::vector<Component*> getRegisteredComponents() const;

    /**
     * @brief Get registered components by role
     * @param role Component role to filter by
     */
    std::vector<Component*> getComponentsByRole(const juce::String& role) const;

    //==============================================================================
    // Screen Reader Integration
    /**
     * @brief Announce message to screen reader
     * @param announcement Announcement to make
     */
    void announceToScreenReader(const ScreenReaderAnnouncement& announcement);

    /**
     * @brief Make polite announcement
     * @param message Message to announce
     */
    void announcePolite(const juce::String& message);

    /**
     * @brief Make assertive announcement
     * @param message Message to announce
     */
    void announceAssertive(const juce::String& message);

    /**
     * @brief Update component state for screen readers
     * @param component Component to update
     * @param state_name State name
     * @param state_value State value
     */
    void updateComponentState(Component* component, 
                             const juce::String& state_name, 
                             const juce::String& state_value);

    /**
     * @brief Update component value
     * @param component Component to update
     * @param value Component value
     */
    void updateComponentValue(Component* component, const juce::String& value);

    /**
     * @brief Update component visibility
     * @param component Component to update
     * @param visible Whether component is visible
     */
    void updateComponentVisibility(Component* component, bool visible);

    /**
     * @brief Update component enabled state
     * @param component Component to update
     * @param enabled Whether component is enabled
     */
    void updateComponentEnabled(Component* component, bool enabled);

    /**
     * @brief Update component selection state
     * @param component Component to update
     * @param selected Whether component is selected
     */
    void updateComponentSelection(Component* component, bool selected);

    //==============================================================================
    // Focus Management
    /**
     * @brief Set focusable component
     * @param component Component to make focusable
     * @param name Component name for focus management
     */
    void setFocusableComponent(Component* component, const juce::String& name = "");

    /**
     * @brief Remove focus from component
     * @param component Component to unfocus
     */
    void removeFocusableComponent(Component* component);

    /**
     * @brief Set focused component
     * @param component Component to focus
     * @param cause Reason for focus change
     */
    void setFocusedComponent(Component* component, FocusChangeType cause);

    /**
     * @brief Get currently focused component
     */
    Component* getFocusedComponent() const { return focused_component_; }

    /**
     * @brief Clear focus
     */
    void clearFocus();

    /**
     * @brief Move focus in specified direction
     * @param forward Direction (true = forward, false = backward)
     */
    void moveFocus(bool forward = true);

    /**
     * @brief Move focus to specific component
     * @param component Target component
     */
    void moveFocusToComponent(Component* component);

    /**
     * @brief Set focus state for navigation
     * @param state Focus state for context
     */
    void setFocusState(FocusState state);

    /**
     * @brief Get current focus state
     */
    FocusState getFocusState() const { return focus_state_; }

    /**
     * @brief Enable focus ring rendering
     */
    void setFocusRingEnabled(bool enabled);

    /**
     * @brief Check if focus ring is enabled
     */
    bool isFocusRingEnabled() const { return focus_ring_enabled_; }

    /**
     * @brief Set focus ring color
     */
    void setFocusRingColor(const juce::Colour& color);

    /**
     * @brief Get focus ring color
     */
    juce::Colour getFocusRingColor() const { return focus_ring_color_; }

    //==============================================================================
    // Keyboard Navigation
    /**
     * @brief Register keyboard shortcut
     * @param shortcut Keyboard shortcut configuration
     */
    void registerKeyboardShortcut(const KeyboardShortcut& shortcut);

    /**
     * @brief Remove keyboard shortcut
     * @param key Key press to remove
     */
    void removeKeyboardShortcut(const juce::KeyPress& key);

    /**
     * @brief Get all registered shortcuts
     */
    std::vector<KeyboardShortcut> getRegisteredShortcuts() const;

    /**
     * @brief Process keyboard event
     * @param key Key press event
     * @return True if handled
     */
    bool processKeyEvent(const juce::KeyPress& key);

    /**
     * @brief Get shortcut description for key
     * @param key Key press
     * @return Shortcut description or empty string
     */
    juce::String getShortcutDescription(const juce::KeyPress& key) const;

    //==============================================================================
    // High Contrast Mode
    /**
     * @ Enable high contrast mode
     * @param enabled Whether high contrast is enabled
     */
    void setHighContrastMode(bool enabled);

    /**
     * @brief Check if high contrast mode is enabled
     */
    bool isHighContrastModeEnabled() const { return high_contrast_mode_; }

    /**
     * @brief Set high contrast color scheme
     * @param scheme_name Color scheme name
     */
    void setHighContrastScheme(const juce::String& scheme_name);

    /**
     * @brief Get high contrast color scheme
     */
    juce::String getHighContrastScheme() const { return high_contrast_scheme_; }

    /**
     * @brief Get high contrast color
     * @param color_name Color name
     * @return High contrast color
     */
    juce::Colour getHighContrastColor(const juce::String& color_name) const;

    /**
     * @brief Check if color contrast meets WCAG guidelines
     * @param foreground Foreground color
     * @param background Background color
     * @param level Contrast level ("AA" or "AAA")
     * @return True if contrast meets guidelines
     */
    bool checkContrastRatio(const juce::Colour& foreground, 
                           const juce::Colour& background,
                           const juce::String& level = "AA") const;

    //==============================================================================
    // Audio Feedback
    /**
     * @brief Enable audio feedback
     * @param enabled Whether audio feedback is enabled
     */
    void setAudioFeedbackEnabled(bool enabled);

    /**
     * @brief Check if audio feedback is enabled
     */
    bool isAudioFeedbackEnabled() const { return audio_feedback_enabled_; }

    /**
     * @brief Set text-to-speech voice
     * @param voice_name Voice name
     */
    void setTTSVoice(const juce::String& voice_name);

    /**
     * @brief Get text-to-speech voice
     */
    juce::String getTTSVoice() const { return tts_voice_; }

    /**
     * @brief Set TTS rate
     * @param rate Speech rate (0.1 to 2.0)
     */
    void setTTSRate(float rate);

    /**
     * @brief Get TTS rate
     */
    float getTTSRate() const { return tts_rate_; }

    /**
     * @brief Set TTS pitch
     * @param pitch Speech pitch (0.1 to 2.0)
     */
    void setTTSPitch(float pitch);

    /**
     * @brief Get TTS pitch
     */
    float getTTSPitch() const { return tts_pitch_; }

    /**
     * @brief Speak text
     * @param text Text to speak
     */
    void speakText(const juce::String& text);

    /**
     * @brief Stop speaking
     */
    void stopSpeaking();

    //==============================================================================
    // UI Scaling
    /**
     * @brief Enable UI scaling
     * @param enabled Whether UI scaling is enabled
     */
    void setUIScalingEnabled(bool enabled);

    /**
     * @brief Check if UI scaling is enabled
     */
    bool isUIScalingEnabled() const { return ui_scaling_enabled_; }

    /**
     * @brief Set UI scale factor
     * @param scale Scale factor
     */
    void setUIScaleFactor(float scale);

    /**
     * @brief Get UI scale factor
     */
    float getUIScaleFactor() const { return ui_scale_factor_; }

    /**
     * @brief Enable/disable scalable fonts
     */
    void setScalableFonts(bool enabled);

    /**
     * @brief Check if scalable fonts are enabled
     */
    bool areScalableFontsEnabled() const { return scalable_fonts_enabled_; }

    //==============================================================================
    // Reduced Motion
    /**
     * @brief Enable/disable reduced motion
     * @param enabled Whether reduced motion is preferred
     */
    void setReducedMotion(bool enabled);

    /**
     * @brief Check if reduced motion is enabled
     */
    bool isReducedMotionEnabled() const { return reduced_motion_; }

    /**
     * @brief Set animation duration multiplier
     * @param multiplier Duration multiplier (0.0 to 1.0)
     */
    void setAnimationDurationMultiplier(float multiplier);

    /**
     * @brief Get animation duration multiplier
     */
    float getAnimationDurationMultiplier() const { return animation_duration_multiplier_; }

    //==============================================================================
    // Configuration and Settings
    /**
     * @brief Load accessibility settings
     * @param settings_file Settings file path
     */
    void loadSettings(const juce::String& settings_file);

    /**
     * @brief Save accessibility settings
     * @param settings_file Settings file path
     */
    void saveSettings(const juce::String& settings_file);

    /**
     * @brief Reset to default settings
     */
    void resetToDefaults();

    /**
     * @brief Get settings as JSON
     */
    juce::String getSettingsAsJSON() const;

    /**
     * @brief Apply settings from JSON
     * @param json_settings JSON settings string
     */
    void applySettingsFromJSON(const juce::String& json_settings);

    //==============================================================================
    // Lifecycle
    /**
     * @brief Initialize accessibility system
     */
    void initialize();

    /**
     * @brief Shutdown accessibility system
     */
    void shutdown();

    /**
     * @brief Check if initialized
     */
    bool isInitialized() const { return initialized_; }

private:
    //==============================================================================
    // Private member variables
    std::unordered_map<AccessibilityFeature, bool> feature_states_;
    bool initialized_ = false;

    // Component tracking
    std::vector<Component*> registered_components_;
    std::unordered_map<Component*, ComponentAccessibilityInfo> component_info_map_;
    std::unordered_map<juce::String, Component*> component_id_map_;

    // Focus management
    Component* focused_component_ = nullptr;
    Component* previously_focused_component_ = nullptr;
    FocusState focus_state_ = FocusState::None;
    bool focus_ring_enabled_ = true;
    juce::Colour focus_ring_color_ = juce::Colours::blue.withAlpha(0.8f);

    // Keyboard shortcuts
    std::vector<KeyboardShortcut> registered_shortcuts_;
    std::unordered_map<juce::String, KeyboardShortcut> shortcut_map_;

    // High contrast
    bool high_contrast_mode_ = false;
    juce::String high_contrast_scheme_ = "default";
    std::unordered_map<juce::String, juce::Colour> high_contrast_colors_;

    // Audio feedback
    bool audio_feedback_enabled_ = false;
    juce::String tts_voice_;
    float tts_rate_ = 1.0f;
    float tts_pitch_ = 1.0f;

    // UI scaling
    bool ui_scaling_enabled_ = false;
    float ui_scale_factor_ = 1.0f;
    bool scalable_fonts_enabled_ = true;

    // Reduced motion
    bool reduced_motion_ = false;
    float animation_duration_multiplier_ = 0.5f;

    // System preferences
    juce::String system_preferences_;

    // Threading
    mutable std::mutex components_mutex_;
    mutable std::mutex focus_mutex_;
    mutable std::mutex shortcuts_mutex_;

    //==============================================================================
    // Private methods
    void setupDefaultFeatures();
    void setupHighContrastColors();
    void setupDefaultShortcuts();
    void sortComponentsByTabOrder();
    
    // Screen reader helpers
    void updateScreenReaderAnnouncements();
    void cleanupExpiredAnnouncements();
    
    // Focus management helpers
    Component* findNextFocusableComponent(bool forward, Component* current = nullptr);
    Component* findComponentInDirection(Component* current, const juce::String& direction);
    
    // Keyboard navigation helpers
    bool handleTabKey(bool forward);
    bool handleArrowKey(const juce::KeyPress& key);
    bool handleCustomShortcut(const juce::KeyPress& key);
    
    // Screen reader integration
    void initializeScreenReaderSupport();
    void updateScreenReaderWithComponent(Component* component);
    void announceComponentStateChange(Component* component, const juce::String& state);
    
    // Utility methods
    bool componentExists(Component* component) const;
    void assertComponentExists(Component* component) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AccessibilityManager)
};

} // namespace accessibility
} // namespace ui
} // namespace vital
