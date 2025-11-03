#pragma once

#include <functional>
#include <JuceHeader.h>
#include "../core/component.h"

namespace vital {
namespace ui {
namespace material {

/**
 * @brief Button variants following Material Design 3.0
 */
enum class ButtonVariant {
    Filled,        // Primary action button with filled background
    Outlined,      // Secondary action button with outline
    Text,          // Tertiary action button with text only
    Elevated,      // Button with elevation shadow
    FAB,           // Floating Action Button
    FABMini        // Small FAB
};

/**
 * @brief Button size modes
 */
enum class ButtonSize {
    Small,
    Medium,
    Large
};

/**
 * @brief Button state
 */
struct ButtonState {
    bool hovered = false;
    bool focused = false;
    bool pressed = false;
    bool disabled = false;
    float ripple_progress = 0.0f;
    juce::Point<float> ripple_center;
    juce::Colour ripple_color;
};

/**
 * @brief Material Design 3.0 Button component
 * 
 * Implements buttons following Material Design 3.0 guidelines
 * with support for:
 * - Elevation and shadows
 * - Ripple effects
 * - Multiple variants
 * - Accessibility features
 * - Touch optimization
 */
class VITAL_MODERN_UI_API Button : public core::Component {
public:
    /**
     * @brief Click callback function
     */
    using ClickCallback = std::function<void()>;

    /**
     * @brief Constructor
     * @param text Button text
     * @param variant Button variant
     * @param size Button size
     */
    Button(const juce::String& text = "",
           ButtonVariant variant = ButtonVariant::Filled,
           ButtonSize size = ButtonSize::Medium);

    /**
     * @brief Destructor
     */
    ~Button() override = default;

    //==============================================================================
    // Content management
    void setText(const juce::String& text);
    const juce::String& getText() const { return text_; }

    void setIcon(const juce::String& icon_path);
    const juce::String& getIcon() const { return icon_path_; }

    void setTrailingIcon(const juce::String& icon_path);
    const juce::String& getTrailingIcon() const { return trailing_icon_path_; }

    /**
     * @brief Set both leading and trailing icons
     */
    void setIcons(const juce::String& leading, const juce::String& trailing);

    /**
     * @brief Remove all icons
     */
    void clearIcons();

    //==============================================================================
    // Appearance
    void setVariant(ButtonVariant variant);
    ButtonVariant getVariant() const { return variant_; }

    void setSize(ButtonSize size);
    ButtonSize getSize() const { return size_; }

    void setEnabled(bool enabled) override;

    /**
     * @brief Set button color
     * @param color Button color (applies to background or text depending on variant)
     */
    void setColor(const juce::Colour& color);

    /**
     * @brief Get button color
     */
    juce::Colour getColor() const { return custom_color_; }

    /**
     * @brief Use theme color
     * @param color_name Theme color name
     */
    void useThemeColor(const juce::String& color_name);

    //==============================================================================
    // Ripple effect
    void enableRippleEffect(bool enable);
    bool isRippleEffectEnabled() const { return ripple_enabled_; }

    void setRippleColor(const juce::Colour& color);
    juce::Colour getRippleColor() const { return ripple_color_; }

    /**
     * @brief Trigger ripple effect at specific position
     * @param position Ripple center position
     */
    void triggerRipple(const juce::Point<float>& position);

    /**
     * @brief Trigger ripple at center
     */
    void triggerRippleAtCenter();

    //==============================================================================
    // Elevation
    void setElevationLevel(int level);
    int getElevationLevel() const { return elevation_level_; }

    /**
     * @brief Set shadow color
     */
    void setShadowColor(const juce::Colour& color);

    /**
     * @brief Get shadow color
     */
    juce::Colour getShadowColor() const { return shadow_color_; }

    //==============================================================================
    // State callbacks
    void setClickCallback(ClickCallback callback);
    void setFocusCallback(std::function<void(bool)> callback);
    void setHoverCallback(std::function<void(bool)> callback);
    void setPressCallback(std::function<void()> callback);

    /**
     * @brief Clear all callbacks
     */
    void clearCallbacks();

    //==============================================================================
    // Accessibility
    void setTooltip(const juce::String& tooltip);
    const juce::String& getTooltip() const { return tooltip_; }

    /**
     * @brief Set keyboard shortcut
     */
    void setKeyboardShortcut(const juce::String& shortcut);

    /**
     * @brief Get keyboard shortcut
     */
    juce::String getKeyboardShortcut() const { return keyboard_shortcut_; }

    /**
     * @brief Set role for accessibility
     */
    void setAriaRole(const juce::String& role);

    /**
     * @brief Enable/disable button
     */
    void setPressedState(bool pressed);

    /**
     * @brief Check if button is in pressed state
     */
    bool isPressed() const { return state_.pressed; }

    //==============================================================================
    // Touch optimization
    /**
     * @brief Set minimum touch target size
     */
    void setMinimumTouchTargetSize(float size);

    /**
     * @brief Get minimum touch target size
     */
    float getMinimumTouchTargetSize() const { return min_touch_target_size_; }

    /**
     * @brief Enable haptic feedback (if supported)
     */
    void setHapticFeedbackEnabled(bool enabled);

    /**
     * @brief Check if haptic feedback is enabled
     */
    bool isHapticFeedbackEnabled() const { return haptic_feedback_enabled_; }

    //==============================================================================
    // Animation
    /**
     * @brief Set animation duration
     */
    void setAnimationDuration(float duration_ms);

    /**
     * @brief Get animation duration
     */
    float getAnimationDuration() const { return animation_duration_ms_; }

    /**
     * @brief Enable/disable button animations
     */
    void setAnimationsEnabled(bool enabled);

    /**
     * @brief Check if animations are enabled
     */
    bool areAnimationsEnabled() const { return animations_enabled_; }

    //==============================================================================
    // Interaction
    bool keyPressed(const juce::KeyPress& key) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;

    // Touch events
    void touchStart(const juce::TouchEvent& e) override;
    void touchMove(const juce::TouchEvent& e) override;
    void touchEnd(const juce::TouchEvent& e) override;
    void touchCancel(const juce::TouchEvent& e) override;

    //==============================================================================
    // Component overrides
    void update() override;
    void render(juce::Graphics& g) override;

protected:
    /**
     * @brief Core::Component overrides
     */
    void updateInternal() override;
    void renderInternal(juce::Graphics& g) override;

    void onMouseEnter() override;
    void onMouseExit() override;
    void onFocus() override;
    void onFocusLost() override;
    void onPress() override;
    void onRelease() override;
    void onValueChanged(float value) override;
    void onStateChanged(core::Component::State new_state) override;

private:
    //==============================================================================
    // Member variables
    juce::String text_;
    juce::String icon_path_;
    juce::String trailing_icon_path_;
    juce::String tooltip_;
    juce::String keyboard_shortcut_;

    ButtonVariant variant_ = ButtonVariant::Filled;
    ButtonSize size_ = ButtonSize::Medium;

    // Visual properties
    juce::Colour custom_color_;
    juce::Colour ripple_color_ = juce::Colours::white.withAlpha(0.2f);
    juce::Colour shadow_color_ = juce::Colours::black.withAlpha(0.3f);

    // Ripple effect
    bool ripple_enabled_ = true;
    ButtonState state_;
    bool ripple_active_ = false;
    float ripple_start_time_ = 0.0f;

    // Elevation
    int elevation_level_ = 2;

    // Touch optimization
    float min_touch_target_size_ = 44.0f;  // Material Design recommendation
    bool haptic_feedback_enabled_ = false;

    // Animation
    float animation_duration_ms_ = 200.0f;
    bool animations_enabled_ = true;

    // Callbacks
    ClickCallback click_callback_;
    std::function<void(bool)> focus_callback_;
    std::function<void(bool)> hover_callback_;
    std::function<void()> press_callback_;

    // Theme
    bool use_custom_color_ = false;
    juce::String theme_color_name_;

    //==============================================================================
    // Private methods
    juce::Rectangle<float> getContentBounds() const;
    juce::Rectangle<float> getTextBounds() const;
    juce::Rectangle<float> getIconBounds() const;
    juce::Rectangle<float> getTrailingIconBounds() const;
    juce::Rectangle<float> getRippleBounds() const;

    // Rendering
    void renderBackground(juce::Graphics& g);
    void renderRippleEffect(juce::Graphics& g);
    void renderContent(juce::Graphics& g);
    void renderText(juce::Graphics& g);
    void renderIcons(juce::Graphics& g);
    void renderFocusRing(juce::Graphics& g);
    void renderElevation(juce::Graphics& g);

    // Color calculations
    juce::Colour getBackgroundColor() const;
    juce::Colour getTextColor() const;
    juce::Colour getBorderColor() const;
    juce::Colour getIconColor() const;

    // Layout calculations
    float getPadding() const;
    float getMinHeight() const;
    float getMaxWidth() const;
    juce::Font getFont() const;
    float getIconSize() const;
    float getCornerRadius() const;
    float getBorderWidth() const;

    // Animation helpers
    void updateRipple(float delta_time);
    void updateButtonState(float delta_time);
    float getAnimationProgress() const;

    // Event handling
    void handleClick();
    void handleFocusChange(bool gained);
    void handleHoverChange(bool entered);
    void handlePressChange(bool pressed);

    // Utility
    bool hasIcons() const;
    bool shouldShowIconOnly() const;
    juce::String getAccessibilityText() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Button)
};

} // namespace material
} // namespace ui
} // namespace vital
