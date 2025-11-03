#include "accessibility_manager.h"
#include <algorithm>
#include <thread>

namespace vital {
namespace ui {
namespace accessibility {

//==============================================================================
// AccessibilityManager Implementation
//==============================================================================

AccessibilityManager::AccessibilityManager()
    : focused_component_(nullptr),
      previously_focused_component_(nullptr),
      focus_state_(FocusState::None),
      focus_ring_enabled_(true),
      focus_ring_color_(juce::Colours::blue.withAlpha(0.8f)),
      high_contrast_mode_(false),
      high_contrast_scheme_("default"),
      audio_feedback_enabled_(false),
      tts_rate_(1.0f),
      tts_pitch_(1.0f),
      ui_scaling_enabled_(false),
      ui_scale_factor_(1.0f),
      scalable_fonts_enabled_(true),
      reduced_motion_(false),
      animation_duration_multiplier_(0.5f),
      initialized_(false) {
    
    setupDefaultFeatures();
    setupHighContrastColors();
    setupDefaultShortcuts();
}

AccessibilityManager::~AccessibilityManager() = default;

// Feature management
void AccessibilityManager::setFeatureEnabled(AccessibilityFeature feature, bool enabled) {
    feature_states_[feature] = enabled;
}

bool AccessibilityManager::isFeatureEnabled(AccessibilityFeature feature) const {
    if (feature == AccessibilityFeature::AllFeatures) {
        return std::all_of(feature_states_.begin(), feature_states_.end(),
                          [](const auto& pair) { return pair.second; });
    }
    
    auto it = feature_states_.find(feature);
    return (it != feature_states_.end()) ? it->second : false;
}

std::vector<AccessibilityFeature> AccessibilityManager::getEnabledFeatures() const {
    std::vector<AccessibilityFeature> enabled_features;
    
    for (const auto& pair : feature_states_) {
        if (pair.second) {
            enabled_features.push_back(pair.first);
        }
    }
    
    return enabled_features;
}

void AccessibilityManager::enableAllFeatures() {
    for (auto& pair : feature_states_) {
        pair.second = true;
    }
}

void AccessibilityManager::disableAllFeatures() {
    for (auto& pair : feature_states_) {
        pair.second = false;
    }
}

void AccessibilityManager::setSystemPreferences(const juce::String& preferences) {
    system_preferences_ = preferences;
    
    // Parse system preferences and update features
    if (preferences.containsIgnoreCase("high_contrast")) {
        setHighContrastMode(true);
    }
    
    if (preferences.containsIgnoreCase("reduced_motion")) {
        setReducedMotion(true);
    }
    
    if (preferences.containsIgnoreCase("large_fonts")) {
        setUIScalingEnabled(true);
        setUIScaleFactor(1.25f);
    }
}

juce::String AccessibilityManager::getSystemPreferences() const {
    return system_preferences_;
}

// Component registration
void AccessibilityManager::registerComponent(Component* component, 
                                            const ComponentAccessibilityInfo& accessibility_info) {
    if (!componentExists(component)) return;
    
    std::lock_guard<std::mutex> lock(components_mutex_);
    
    registered_components_.push_back(component);
    component_info_map_[component] = accessibility_info;
    component_id_map_[accessibility_info.id] = component;
    
    // Initialize focusability
    if (accessibility_info.is_focusable) {
        setFocusableComponent(component, accessibility_info.id);
    }
}

void AccessibilityManager::unregisterComponent(Component* component) {
    std::lock_guard<std::mutex> lock(components_mutex_);
    
    // Remove from registered components
    registered_components_.erase(
        std::remove(registered_components_.begin(), registered_components_.end(), component),
        registered_components_.end());
    
    // Remove from maps
    component_info_map_.erase(component);
    
    for (auto it = component_id_map_.begin(); it != component_id_map_.end(); ++it) {
        if (it->second == component) {
            component_id_map_.erase(it);
            break;
        }
    }
    
    // Remove from focusable components
    removeFocusableComponent(component);
}

void AccessibilityManager::updateComponentAccessibilityInfo(Component* component,
                                                           const ComponentAccessibilityInfo& accessibility_info) {
    std::lock_guard<std::mutex> lock(components_mutex_);
    component_info_map_[component] = accessibility_info;
}

ComponentAccessibilityInfo AccessibilityManager::getComponentAccessibilityInfo(Component* component) const {
    std::lock_guard<std::mutex> lock(components_mutex_);
    
    auto it = component_info_map_.find(component);
    return (it != component_info_map_.end()) ? it->second : ComponentAccessibilityInfo();
}

bool AccessibilityManager::isComponentRegistered(Component* component) const {
    std::lock_guard<std::mutex> lock(components_mutex_);
    return std::find(registered_components_.begin(), registered_components_.end(), component) 
           != registered_components_.end();
}

std::vector<Component*> AccessibilityManager::getRegisteredComponents() const {
    std::lock_guard<std::mutex> lock(components_mutex_);
    return registered_components_;
}

std::vector<Component*> AccessibilityManager::getComponentsByRole(const juce::String& role) const {
    std::lock_guard<std::mutex> lock(components_mutex_);
    std::vector<Component*> components;
    
    for (const auto& pair : component_info_map_) {
        if (pair.second.role == role) {
            components.push_back(pair.first);
        }
    }
    
    return components;
}

// Screen reader integration
void AccessibilityManager::announceToScreenReader(const ScreenReaderAnnouncement& announcement) {
    if (!isFeatureEnabled(AccessibilityFeature::ScreenReader)) {
        return;
    }
    
    // In a real implementation, this would integrate with platform screen readers
    // For now, we just log the announcement
    juce::Logger::writeToLog("Screen Reader Announcement: " + announcement.text);
    
    // Simulate TTS if enabled
    if (audio_feedback_enabled_) {
        speakText(announcement.text);
    }
}

void AccessibilityManager::announcePolite(const juce::String& message) {
    ScreenReaderAnnouncement announcement(message, "polite");
    announceToScreenReader(announcement);
}

void AccessibilityManager::announceAssertive(const juce::String& message) {
    ScreenReaderAnnouncement announcement(message, "assertive");
    announceToScreenReader(announcement);
}

void AccessibilityManager::updateComponentState(Component* component, 
                                               const juce::String& state_name, 
                                               const juce::String& state_value) {
    auto info = getComponentAccessibilityInfo(component);
    
    if (state_name == "value") {
        info.value_text = state_value;
    } else if (state_name == "selected") {
        info.is_selected = (state_value == "true");
    } else if (state_name == "expanded") {
        info.is_expanded = (state_value == "true");
    } else if (state_name == "disabled") {
        // This would typically be handled by the component itself
    }
    
    updateComponentAccessibilityInfo(component, info);
}

void AccessibilityManager::updateComponentValue(Component* component, const juce::String& value) {
    auto info = getComponentAccessibilityInfo(component);
    info.value_text = value;
    updateComponentAccessibilityInfo(component, info);
}

void AccessibilityManager::updateComponentVisibility(Component* component, bool visible) {
    auto info = getComponentAccessibilityInfo(component);
    // Update visibility state for screen readers
    // In a real implementation, this would update ARIA attributes
    updateComponentAccessibilityInfo(component, info);
}

void AccessibilityManager::updateComponentEnabled(Component* component, bool enabled) {
    auto info = getComponentAccessibilityInfo(component);
    // Update enabled state for screen readers
    // In a real implementation, this would update ARIA attributes
    updateComponentAccessibilityInfo(component, info);
}

void AccessibilityManager::updateComponentSelection(Component* component, bool selected) {
    auto info = getComponentAccessibilityInfo(component);
    info.is_selected = selected;
    updateComponentAccessibilityInfo(component, info);
}

// Focus management
void AccessibilityManager::setFocusableComponent(Component* component, const juce::String& name) {
    if (!componentExists(component)) return;
    
    auto info = getComponentAccessibilityInfo(component);
    info.is_focusable = true;
    updateComponentAccessibilityInfo(component, info);
    
    // Add to focus management (if not already present)
    if (!info.is_focusable) {
        return; // Component is not focusable
    }
    
    // In a real implementation, maintain a list of focusable components
    // and manage focus order based on tab index
}

void AccessibilityManager::removeFocusableComponent(Component* component) {
    if (focused_component_ == component) {
        clearFocus();
    }
}

void AccessibilityManager::setFocusedComponent(Component* component, FocusChangeType cause) {
    std::lock_guard<std::mutex> lock(focus_mutex_);
    
    if (component == focused_component_) return;
    
    // Remove focus from previously focused component
    if (focused_component_) {
        focused_component_->focusLost(cause);
    }
    
    previously_focused_component_ = focused_component_;
    focused_component_ = component;
    
    // Add focus to new component
    if (focused_component_) {
        focused_component_->focusGained(cause);
        announceToScreenReader("Focused: " + getComponentAccessibilityInfo(focused_component_).label);
    }
}

Component* AccessibilityManager::getFocusedComponent() const {
    return focused_component_;
}

void AccessibilityManager::clearFocus() {
    std::lock_guard<std::mutex> lock(focus_mutex_);
    
    if (focused_component_) {
        focused_component_->focusLost(FocusChangeType::focusChangedByWindow);
        focused_component_ = nullptr;
    }
}

void AccessibilityManager::moveFocus(bool forward) {
    Component* next_component = findNextFocusableComponent(forward, focused_component_);
    if (next_component) {
        setFocusedComponent(next_component, FocusChangeType::focusChangedByTabKey);
    }
}

void AccessibilityManager::moveFocusToComponent(Component* component) {
    setFocusedComponent(component, FocusChangeType::focusChangedDirectly);
}

void AccessibilityManager::setFocusState(FocusState state) {
    focus_state_ = state;
}

FocusState AccessibilityManager::getFocusState() const {
    return focus_state_;
}

void AccessibilityManager::setFocusRingEnabled(bool enabled) {
    focus_ring_enabled_ = enabled;
}

bool AccessibilityManager::isFocusRingEnabled() const {
    return focus_ring_enabled_;
}

void AccessibilityManager::setFocusRingColor(const juce::Colour& color) {
    focus_ring_color_ = color;
}

juce::Colour AccessibilityManager::getFocusRingColor() const {
    return focus_ring_color_;
}

// Keyboard navigation
void AccessibilityManager::registerKeyboardShortcut(const KeyboardShortcut& shortcut) {
    std::lock_guard<std::mutex> lock(shortcuts_mutex_);
    
    registered_shortcuts_.push_back(shortcut);
    shortcut_map_[shortcut.key.toString()] = shortcut;
}

void AccessibilityManager::removeKeyboardShortcut(const juce::KeyPress& key) {
    std::lock_guard<std::mutex> lock(shortcuts_mutex_);
    
    // Remove from vector
    registered_shortcuts_.erase(
        std::remove_if(registered_shortcuts_.begin(), registered_shortcuts_.end(),
                      [&key](const KeyboardShortcut& shortcut) {
                          return shortcut.key == key;
                      }),
        registered_shortcuts_.end());
    
    // Remove from map
    shortcut_map_.erase(key.toString());
}

std::vector<KeyboardShortcut> AccessibilityManager::getRegisteredShortcuts() const {
    std::lock_guard<std::mutex> lock(shortcuts_mutex_);
    return registered_shortcuts_;
}

bool AccessibilityManager::processKeyEvent(const juce::KeyPress& key) {
    // Handle tab key navigation
    if (handleTabKey(key == juce::KeyPress::tabKey)) {
        return true;
    }
    
    // Handle arrow key navigation
    if (handleArrowKey(key)) {
        return true;
    }
    
    // Handle custom shortcuts
    if (handleCustomShortcut(key)) {
        return true;
    }
    
    return false;
}

juce::String AccessibilityManager::getShortcutDescription(const juce::KeyPress& key) const {
    auto it = shortcut_map_.find(key.toString());
    return (it != shortcut_map_.end()) ? it->second.description : juce::String();
}

// High contrast mode
void AccessibilityManager::setHighContrastMode(bool enabled) {
    high_contrast_mode_ = enabled;
    
    if (enabled) {
        announceAssertive("High contrast mode enabled");
    } else {
        announcePolite("High contrast mode disabled");
    }
}

bool AccessibilityManager::isHighContrastModeEnabled() const {
    return high_contrast_mode_;
}

void AccessibilityManager::setHighContrastScheme(const juce::String& scheme_name) {
    high_contrast_scheme_ = scheme_name;
    announcePolite("High contrast scheme changed to " + scheme_name);
}

juce::String AccessibilityManager::getHighContrastScheme() const {
    return high_contrast_scheme_;
}

juce::Colour AccessibilityManager::getHighContrastColor(const juce::String& color_name) const {
    auto it = high_contrast_colors_.find(color_name);
    return (it != high_contrast_colors_.end()) ? it->second : juce::Colours::black;
}

bool AccessibilityManager::checkContrastRatio(const juce::Colour& foreground, 
                                              const juce::Colour& background,
                                              const juce::String& level) const {
    float contrast_ratio = calculateContrastRatio(foreground, background);
    
    if (level == "AAA") {
        return contrast_ratio >= 7.0f;
    } else {
        return contrast_ratio >= 4.5f; // AA level
    }
}

// Audio feedback
void AccessibilityManager::setAudioFeedbackEnabled(bool enabled) {
    audio_feedback_enabled_ = enabled;
    
    if (enabled) {
        announcePolite("Audio feedback enabled");
    } else {
        announcePolite("Audio feedback disabled");
    }
}

bool AccessibilityManager::isAudioFeedbackEnabled() const {
    return audio_feedback_enabled_;
}

void AccessibilityManager::setTTSVoice(const juce::String& voice_name) {
    tts_voice_ = voice_name;
}

juce::String AccessibilityManager::getTTSVoice() const {
    return tts_voice_;
}

void AccessibilityManager::setTTSRate(float rate) {
    tts_rate_ = std::clamp(rate, 0.1f, 2.0f);
}

float AccessibilityManager::getTTSRate() const {
    return tts_rate_;
}

void AccessibilityManager::setTTSPitch(float pitch) {
    tts_pitch_ = std::clamp(pitch, 0.1f, 2.0f);
}

float AccessibilityManager::getTTSPitch() const {
    return tts_pitch_;
}

void AccessibilityManager::speakText(const juce::String& text) {
    if (!audio_feedback_enabled_ || text.isEmpty()) {
        return;
    }
    
    // In a real implementation, this would use a TTS engine
    // For now, we just log the text that would be spoken
    juce::Logger::writeToLog("TTS: " + text);
}

void AccessibilityManager::stopSpeaking() {
    // In a real implementation, this would stop the TTS engine
    juce::Logger::writeToLog("TTS stopped");
}

// UI scaling
void AccessibilityManager::setUIScalingEnabled(bool enabled) {
    ui_scaling_enabled_ = enabled;
}

bool AccessibilityManager::isUIScalingEnabled() const {
    return ui_scaling_enabled_;
}

void AccessibilityManager::setUIScaleFactor(float scale) {
    ui_scale_factor_ = std::clamp(scale, 0.5f, 3.0f);
}

float AccessibilityManager::getUIScaleFactor() const {
    return ui_scale_factor_;
}

void AccessibilityManager::setScalableFonts(bool enabled) {
    scalable_fonts_enabled_ = enabled;
}

bool AccessibilityManager::areScalableFontsEnabled() const {
    return scalable_fonts_enabled_;
}

// Reduced motion
void AccessibilityManager::setReducedMotion(bool enabled) {
    reduced_motion_ = enabled;
    
    if (enabled) {
        announcePolite("Reduced motion enabled");
    } else {
        announcePolite("Reduced motion disabled");
    }
}

bool AccessibilityManager::isReducedMotionEnabled() const {
    return reduced_motion_;
}

void AccessibilityManager::setAnimationDurationMultiplier(float multiplier) {
    animation_duration_multiplier_ = std::clamp(multiplier, 0.0f, 1.0f);
}

float AccessibilityManager::getAnimationDurationMultiplier() const {
    return animation_duration_multiplier_;
}

// Configuration and settings
void AccessibilityManager::loadSettings(const juce::String& settings_file) {
    juce::File file(settings_file);
    if (!file.exists()) {
        return;
    }
    
    auto json_content = file.loadFileAsString();
    applySettingsFromJSON(json_content);
}

void AccessibilityManager::saveSettings(const juce::String& settings_file) {
    juce::File file(settings_file);
    auto json_content = getSettingsAsJSON();
    file.replaceWithText(json_content);
}

void AccessibilityManager::resetToDefaults() {
    setupDefaultFeatures();
    setupHighContrastColors();
    setupDefaultShortcuts();
    
    // Reset all settings to default values
    focus_ring_enabled_ = true;
    focus_ring_color_ = juce::Colours::blue.withAlpha(0.8f);
    high_contrast_mode_ = false;
    high_contrast_scheme_ = "default";
    audio_feedback_enabled_ = false;
    tts_voice_ = "";
    tts_rate_ = 1.0f;
    tts_pitch_ = 1.0f;
    ui_scaling_enabled_ = false;
    ui_scale_factor_ = 1.0f;
    scalable_fonts_enabled_ = true;
    reduced_motion_ = false;
    animation_duration_multiplier_ = 0.5f;
    
    announcePolite("Accessibility settings reset to defaults");
}

juce::String AccessibilityManager::getSettingsAsJSON() const {
    // Simplified JSON generation
    juce::String json = "{";
    json += "\"focus_ring_enabled\":" + juce::String(focus_ring_enabled_) + ",";
    json += "\"high_contrast_mode\":" + juce::String(high_contrast_mode_) + ",";
    json += "\"audio_feedback_enabled\":" + juce::String(audio_feedback_enabled_) + ",";
    json += "\"ui_scaling_enabled\":" + juce::String(ui_scaling_enabled_) + ",";
    json += "\"reduced_motion\":" + juce::String(reduced_motion_);
    json += "}";
    
    return json;
}

void AccessibilityManager::applySettingsFromJSON(const juce::String& json_settings) {
    // Simplified JSON parsing - in a real implementation, use proper JSON library
    if (json_settings.contains("\"high_contrast_mode\":true")) {
        setHighContrastMode(true);
    }
    
    if (json_settings.contains("\"audio_feedback_enabled\":true")) {
        setAudioFeedbackEnabled(true);
    }
    
    if (json_settings.contains("\"ui_scaling_enabled\":true")) {
        setUIScalingEnabled(true);
    }
    
    if (json_settings.contains("\"reduced_motion\":true")) {
        setReducedMotion(true);
    }
}

// Lifecycle
void AccessibilityManager::initialize() {
    if (initialized_) return;
    
    initializeScreenReaderSupport();
    initialized_ = true;
    
    announcePolite("Accessibility system initialized");
}

void AccessibilityManager::shutdown() {
    if (!initialized_) return;
    
    clearFocus();
    registered_components_.clear();
    component_info_map_.clear();
    component_id_map_.clear();
    registered_shortcuts_.clear();
    shortcut_map_.clear();
    
    initialized_ = false;
}

bool AccessibilityManager::isInitialized() const {
    return initialized_;
}

// Private methods
void AccessibilityManager::setupDefaultFeatures() {
    feature_states_[AccessibilityFeature::ScreenReader] = false;
    feature_states_[AccessibilityFeature::KeyboardNavigation] = true;
    feature_states_[AccessibilityFeature::HighContrast] = false;
    feature_states_[AccessibilityFeature::AudioFeedback] = false;
    feature_states_[AccessibilityFeature::UIScaling] = false;
    feature_states_[AccessibilityFeature::FocusIndicators] = true;
    feature_states_[AccessibilityFeature::ReducedMotion] = false;
    feature_states_[AccessibilityFeature::TouchOptimization] = true;
    feature_states_[AccessibilityFeature::VoiceControl] = false;
}

void AccessibilityManager::setupHighContrastColors() {
    high_contrast_colors_["background"] = juce::Colours::white;
    high_contrast_colors_["foreground"] = juce::Colours::black;
    high_contrast_colors_["primary"] = juce::Colours::blue;
    high_contrast_colors_["secondary"] = juce::Colours::green;
    high_contrast_colors_["error"] = juce::Colours::red;
    high_contrast_colors_["warning"] = juce::Colours::orange;
    high_contrast_colors_["success"] = juce::Colours::limegreen;
    
    // Dark high contrast scheme
    high_contrast_colors_["dark_background"] = juce::Colours::black;
    high_contrast_colors_["dark_foreground"] = juce::Colours::white;
    high_contrast_colors_["dark_primary"] = juce::Colours::cyan;
    high_contrast_colors_["dark_secondary"] = juce::Colours::yellow;
}

void AccessibilityManager::setupDefaultShortcuts() {
    // Tab navigation
    registerKeyboardShortcut({
        juce::KeyPress(juce::KeyPress::tabKey),
        "Move to next focusable element",
        "tab_navigation"
    });
    
    // Reverse tab navigation
    registerKeyboardShortcut({
        juce::KeyPress(juce::KeyPress::tabKey, ModifierKeys::shiftModifier, false),
        "Move to previous focusable element",
        "reverse_tab_navigation"
    });
    
    // Escape key
    registerKeyboardShortcut({
        juce::KeyPress(juce::KeyPress::escapeKey),
        "Cancel current action",
        "escape_action"
    });
    
    // Enter key
    registerKeyboardShortcut({
        juce::KeyPress(juce::KeyPress::returnKey),
        "Activate current element",
        "activate_element"
    });
    
    // Space key
    registerKeyboardShortcut({
        juce::KeyPress(juce::KeyPress::spaceKey),
        "Toggle current element",
        "toggle_element"
    });
}

Component* AccessibilityManager::findNextFocusableComponent(bool forward, Component* current) const {
    if (registered_components_.empty()) {
        return nullptr;
    }
    
    std::lock_guard<std::mutex> lock(components_mutex_);
    
    // Find current component index
    auto current_it = std::find(registered_components_.begin(), registered_components_.end(), current);
    int current_index = (current_it != registered_components_.end()) ? 
                       std::distance(registered_components_.begin(), current_it) : -1;
    
    // Find next focusable component
    int search_direction = forward ? 1 : -1;
    int index = (current_index + search_direction + registered_components_.size()) % registered_components_.size();
    
    // Search for focusable component
    for (int i = 0; i < registered_components_.size(); i++) {
        Component* component = registered_components_[index];
        auto info = component_info_map_.at(component);
        
        if (info.is_focusable && component->isEnabled()) {
            return component;
        }
        
        index = (index + search_direction + registered_components_.size()) % registered_components_.size();
    }
    
    return nullptr;
}

Component* AccessibilityManager::findComponentInDirection(Component* current, const juce::String& direction) const {
    // This would implement more sophisticated spatial navigation
    // For now, return the next component in tab order
    return findNextFocusableComponent(true, current);
}

bool AccessibilityManager::handleTabKey(bool forward) {
    if (!isFeatureEnabled(AccessibilityFeature::KeyboardNavigation)) {
        return false;
    }
    
    moveFocus(forward);
    return true;
}

bool AccessibilityManager::handleArrowKey(const juce::KeyPress& key) {
    if (!isFeatureEnabled(AccessibilityFeature::KeyboardNavigation)) {
        return false;
    }
    
    Component* current_focus = getFocusedComponent();
    if (!current_focus) return false;
    
    // Handle arrow key navigation for different component types
    auto info = getComponentAccessibilityInfo(current_focus);
    
    if (info.role == "slider" || info.role == "scrollbar") {
        if (key == juce::KeyPress::leftKey || key == juce::KeyPress::downKey) {
            // Decrease value
            // In a real implementation, this would communicate with the slider
            announcePolite("Value decreased");
            return true;
        } else if (key == juce::KeyPress::rightKey || key == juce::KeyPress::upKey) {
            // Increase value
            // In a real implementation, this would communicate with the slider
            announcePolite("Value increased");
            return true;
        }
    }
    
    return false;
}

bool AccessibilityManager::handleCustomShortcut(const juce::KeyPress& key) {
    std::lock_guard<std::mutex> lock(shortcuts_mutex_);
    
    auto it = shortcut_map_.find(key.toString());
    if (it != shortcut_map_.end()) {
        const KeyboardShortcut& shortcut = it->second;
        if (shortcut.action) {
            shortcut.action();
        }
        return true;
    }
    
    return false;
}

void AccessibilityManager::initializeScreenReaderSupport() {
    // Initialize platform-specific screen reader support
    // This would involve platform APIs like Windows Narrator, macOS VoiceOver, etc.
    
    announcePolite("Screen reader support initialized");
}

void AccessibilityManager::updateScreenReaderWithComponent(Component* component) {
    if (!isFeatureEnabled(AccessibilityFeature::ScreenReader)) {
        return;
    }
    
    auto info = getComponentAccessibilityInfo(component);
    
    // Create announcement text
    juce::String announcement = info.label;
    
    if (!info.value_text.isEmpty()) {
        announcement += ", value " + info.value_text;
    }
    
    if (info.is_selected) {
        announcement += ", selected";
    }
    
    if (info.is_expanded) {
        announcement += ", expanded";
    }
    
    announcePolite(announcement);
}

void AccessibilityManager::announceComponentStateChange(Component* component, const juce::String& state) {
    if (!isFeatureEnabled(AccessibilityFeature::ScreenReader)) {
        return;
    }
    
    auto info = getComponentAccessibilityInfo(component);
    announcePolite(info.label + " " + state);
}

bool AccessibilityManager::componentExists(Component* component) const {
    return component != nullptr;
}

void AccessibilityManager::assertComponentExists(Component* component) const {
    // In debug builds, this would assert that the component exists
    jassert(componentExists(component));
}

} // namespace accessibility
} // namespace ui
} // namespace vital