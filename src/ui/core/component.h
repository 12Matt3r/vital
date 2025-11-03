#pragma once

#include <memory>
#include <functional>
#include <JuceHeader.h>

namespace vital {
namespace ui {
namespace core {

/**
 * @brief Base class for all modern UI components
 * 
 * Provides common functionality for positioning, rendering,
 * state management, and accessibility following Material Design 3.0
 * and WCAG 2.2 guidelines.
 */
class VITAL_MODERN_UI_API Component : public juce::Component {
public:
    /**
     * @brief Component state enumeration
     */
    enum class State {
        Normal,
        Hover,
        Focused,
        Pressed,
        Disabled,
        Selected,
        Indeterminate
    };

    /**
     * @brief Component size constraints
     */
    struct Constraints {
        juce::Rectangle<int> bounds;
        int min_width = 0;
        int max_width = INT_MAX;
        int min_height = 0;
        int max_height = INT_MAX;
        float aspect_ratio = -1.0f;  // -1 means no constraint
        
        Constraints() = default;
        Constraints(int min_w, int min_h, int max_w = INT_MAX, int max_h = INT_MAX)
            : min_width(min_w), max_width(max_w), min_height(min_h), max_height(max_h) {}
    };

    /**
     * @brief Animation state
     */
    struct AnimationState {
        float progress = 0.0f;
        float velocity = 0.0f;
        bool is_animating = false;
        juce::Rectangle<float> current_bounds;
        juce::Rectangle<float> target_bounds;
        
        AnimationState() = default;
    };

    /**
     * @brief Accessibility information
     */
    struct AccessibilityInfo {
        juce::String aria_label;
        juce::String aria_description;
        juce::String aria_role = "generic";
        juce::String aria_value_now;
        juce::String aria_value_min;
        juce::String aria_value_max;
        bool is_focusable = true;
        bool is_announced = true;
        juce::String keyboard_shortcut;
        int tab_index = 0;
        
        AccessibilityInfo() = default;
    };

    /**
     * @brief Constructor
     */
    Component();

    /**
     * @brief Destructor
     */
    virtual ~Component() = default;

    //==============================================================================
    // State management
    void setState(State state);
    State getState() const { return state_; }
    bool isHovered() const { return state_ == State::Hover; }
    bool isFocused() const { return state_ == State::Focused; }
    bool isPressed() const { return state_ == State::Pressed; }
    bool isDisabled() const { return state_ == State::Disabled; }
    bool isSelected() const { return state_ == State::Selected; }

    /**
     * @brief Set enabled state
     */
    virtual void setEnabled(bool enabled) override;

    /**
     * @brief Get enabled state
     */
    bool isEnabled() const { return enabled_; }

    //==============================================================================
    // Visibility and opacity
    void setVisible(bool should_be_visible) override;
    void setAlpha(float new_alpha) override;
    float getAlpha() const { return alpha_; }

    /**
     * @brief Set component opacity with animation
     */
    void animateOpacity(float target_alpha, float duration_ms = 300.0f);

    //==============================================================================
    // Animation support
    void setAnimating(bool animating);
    bool isAnimating() const { return animation_state_.is_animating; }

    /**
     * @brief Animate position and size
     */
    void animateBounds(const juce::Rectangle<float>& target_bounds, 
                      float duration_ms = 300.0f, 
                      const std::function<void()>& completion_callback = nullptr);

    /**
     * @brief Animate to specific position
     */
    void animatePosition(float target_x, float target_y, float duration_ms = 300.0f);

    /**
     * @brief Animate to specific size
     */
    void animateSize(float target_width, float target_height, float duration_ms = 300.0f);

    /**
     * @brief Stop current animations
     */
    void stopAnimations();

    /**
     * @brief Base update method called before rendering
     */
    virtual void update();

    /**
     * @brief Main rendering method
     */
    virtual void render(juce::Graphics& g);

    /**
     * @brief Render method called after main render
     */
    virtual void renderPost(juce::Graphics& g);

    /**
     * @brief Calculate and apply size constraints
     */
    void applyConstraints();

    /**
     * @brief Set component constraints
     */
    void setConstraints(const Constraints& constraints);
    const Constraints& getConstraints() const { return constraints_; }

    /**
     * @brief Get current animation state
     */
    const AnimationState& getAnimationState() const { return animation_state_; }

    //==============================================================================
    // Event handling
    virtual void onMouseEnter();
    virtual void onMouseExit();
    virtual void onFocus();
    virtual void onFocusLost();
    virtual void onPress();
    virtual void onRelease();
    virtual void onValueChanged(float value);
    virtual void onStateChanged(State new_state);
    virtual void onGestureDetected(const juce::String& gesture_type);

    /**
     * @brief Touch event handlers for mobile interfaces
     */
    virtual void onTouchBegin(const juce::TouchEvent& event);
    virtual void onTouchMove(const juce::TouchEvent& event);
    virtual void onTouchEnd(const juce::TouchEvent& event);
    virtual void onTouchCancel(const juce::TouchEvent& event);

    //==============================================================================
    // Accessibility
    void setAccessible(bool accessible);
    bool isAccessible() const { return accessible_; }

    void setAriaLabel(const juce::String& label);
    const juce::String& getAriaLabel() const { return accessibility_info_.aria_label; }

    void setAriaDescription(const juce::String& description);
    const juce::String& getAriaDescription() const { return accessibility_info_.aria_description; }

    void setAriaRole(const juce::String& role);
    const juce::String& getAriaRole() const { return accessibility_info_.aria_role; }

    void setAriaValue(const juce::String& value_now, 
                     const juce::String& min = "",
                     const juce::String& max = "");
    
    void setFocusable(bool focusable);
    bool isFocusable() const { return accessibility_info_.is_focusable; }

    void setKeyboardShortcut(const juce::String& shortcut);
    const juce::String& getKeyboardShortcut() const { return accessibility_info_.keyboard_shortcut; }

    void setTabIndex(int index);
    int getTabIndex() const { return accessibility_info_.tab_index; }

    /**
     * @brief Get accessibility information
     */
    const AccessibilityInfo& getAccessibilityInfo() const { return accessibility_info_; }

    /**
     * @brief Announce to screen reader
     */
    void announceToScreenReader(const juce::String& message, 
                               const juce::String& priority = "polite");

    //==============================================================================
    // Layout and positioning
    void setPosition(const juce::Point<float>& position);
    void setSize(float width, float height) override;
    void setBounds(const juce::Rectangle<int>& bounds) override;
    void setBounds(const juce::Rectangle<float>& bounds);

    juce::Point<float> getPosition() const;
    juce::Rectangle<float> getFloatBounds() const;

    /**
     * @brief Center component in parent
     */
    void centerInParent();

    /**
     * @brief Align component to parent edges
     */
    enum class Alignment {
        TopLeft, TopCenter, TopRight,
        CenterLeft, Center, CenterRight,
        BottomLeft, BottomCenter, BottomRight
    };
    
    void alignInParent(Alignment alignment);

    //==============================================================================
    // Theme integration
    /**
     * @brief Get theme color for current state
     */
    virtual juce::Colour getThemeColor(const juce::String& color_name) const;

    /**
     * @brief Get theme value for current state
     */
    virtual float getThemeValue(const juce::String& value_name) const;

    //==============================================================================
    // Performance optimization
    void setNeedsRedraw();
    bool needsRedraw() const { return needs_redraw_; }

    void setLayerOptimizationEnabled(bool enabled);
    bool isLayerOptimizationEnabled() const { return layer_optimization_enabled_; }

    void setCachingEnabled(bool enabled);
    bool isCachingEnabled() const { return caching_enabled_; }

protected:
    //==============================================================================
    // Member variables
    State state_ = State::Normal;
    float alpha_ = 1.0f;
    bool enabled_ = true;
    bool visible_ = true;
    bool is_animating_ = false;
    bool accessible_ = true;
    bool needs_redraw_ = true;
    bool layer_optimization_enabled_ = true;
    bool caching_enabled_ = true;

    AnimationState animation_state_;
    Constraints constraints_;
    AccessibilityInfo accessibility_info_;

    juce::Point<float> position_{0, 0};
    juce::CriticalSection state_mutex_;

    //==============================================================================
    // Helper methods
    juce::Colour getStateColor(State state, const juce::Colour& normal_color) const;
    void renderBackground(juce::Graphics& g, const juce::Colour& color);
    void renderBorder(juce::Graphics& g, const juce::Colour& color, float thickness = 1.0f);
    void renderShadow(juce::Graphics& g, float blur, const juce::Colour& color, 
                     const juce::Point<float>& offset = juce::Point<float>(0, 2));

    /**
     * @brief Render focus indicator
     */
    void renderFocusIndicator(juce::Graphics& g);

    /**
     * @brief Render elevation/shadow
     */
    void renderElevation(juce::Graphics& g, int elevation_level);

    //==============================================================================
    // Override points for subclasses
    virtual void updateInternal();
    virtual void renderInternal(juce::Graphics& g);
    virtual void onAnimationUpdate();

    //==============================================================================
    // JUCE Component overrides
    void paint(juce::Graphics& g) override;
    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;
    void focusGained(FocusChangeType cause) override;
    void focusLost(FocusChangeType cause) override;
    void parentHierarchyChanged() override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Component)
};

} // namespace core
} // namespace ui
} // namespace vital
