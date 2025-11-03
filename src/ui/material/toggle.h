#pragma once

#include "../core/component.h"
#include "../theme/theme_manager.h"
#include <functional>
#include <memory>

namespace vital {
namespace ui {
namespace material {

/**
 * @brief Material Design 3.0 toggle switch
 * 
 * A binary state control with smooth animations and:
 * - On/off states with visual feedback
 * - Smooth sliding animation
 * - Touch optimization
 * - Keyboard navigation support
 * - Disabled state handling
 */
class Toggle : public Component {
public:
    enum class Style {
        Standard,       // Standard Material Design switch
        Large,          // Large size for better accessibility
        Small,          // Compact size
        Custom          // Custom styling
    };

    struct ToggleStyle {
        Style style = Style::Standard;
        bool showLabel = true;
        bool animateStateChange = true;
        float animationDuration = 0.2f; // seconds
        Color onColor = Colors::primary;
        Color offColor = Colors::surfaceVariant;
        Color thumbColor = Colors::surface;
        Color thumbOnColor = Colors::onPrimary;
        float cornerRadius = 16.0f;
        float thumbRadius = 10.0f;
        float trackHeight = 20.0f;
        float trackWidth = 36.0f;
        bool useHapticFeedback = true;
    };

    using StateChangedCallback = std::function<void(bool)>;
    using ValueChangedCallback = std::function<void(bool)>;

    Toggle(const std::string& label = "", bool initialState = false, const ToggleStyle& style = {});
    ~Toggle() override = default;

    // Component interface
    void paint(juce::Graphics& g) override;
    void resized() override;
    bool mouseDown(const juce::MouseEvent& e) override;
    bool mouseUp(const juce::MouseEvent& e) override;
    bool keyPressed(const juce::KeyPress& key) override;
    void focusGained() override;
    void focusLost() override;
    
    // State management
    void setEnabled(bool enabled) override;
    void setChecked(bool checked, bool sendCallback = true);
    bool isChecked() const { return checked_; }
    
    // Style configuration
    void setStyle(const ToggleStyle& style);
    const ToggleStyle& getStyle() const { return style_; }
    
    // Visual properties
    void setLabel(const std::string& label);
    void setLabelPosition(juce::RelativeCoordinate position);
    
    // Callbacks
    void setStateChangedCallback(StateChangedCallback callback);
    void setValueChangedCallback(ValueChangedCallback callback);
    
    // Animation control
    void animateToState(bool targetState, float duration = 0.2f);
    
    // Accessibility
    juce::String getAccessibilityLabel() const override;
    juce::String getAccessibilityValue() const override;
    void performAccessibilityAction(const juce::String& actionName) override;

private:
    ToggleStyle style_;
    std::string label_;
    juce::RelativeCoordinate labelPosition_{juce::RelativePoint(50, 0)};
    
    bool checked_ = false;
    bool isAnimating_ = false;
    float animationProgress_ = 0.0f;
    float targetAnimationProgress_ = 0.0f;
    
    // Animation values
    AnimationValue thumbPosition_;
    AnimationValue trackColor_;
    AnimationValue thumbColor_;
    
    // Callbacks
    StateChangedCallback stateCallback_;
    ValueChangedCallback valueCallback_;
    
    // Internal helpers
    void setCheckedInternal(bool checked);
    void startAnimation(bool targetState, float duration);
    void updateAnimation();
    juce::Rectangle<float> getTrackBounds() const;
    juce::Rectangle<float> getThumbBounds(float progress) const;
    Color getTrackColor(float progress) const;
    Color getThumbColor(float progress) const;
    float getThumbPosition(float progress) const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_CHECK(Toggle)
};

} // namespace material
} // namespace ui
} // namespace vital