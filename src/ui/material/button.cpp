#include "button.h"
#include <cmath>

namespace vital {
namespace ui {
namespace material {

//==============================================================================
// Button Implementation
//==============================================================================

Button::Button(const juce::String& text, 
               ButtonVariant variant, 
               ButtonSize size)
    : text_(text),
      variant_(variant),
      size_(size),
      custom_color_(),
      ripple_color_(juce::Colours::white.withAlpha(0.2f)),
      shadow_color_(juce::Colours::black.withAlpha(0.3f)),
      ripple_enabled_(true),
      elevation_level_(2),
      min_touch_target_size_(44.0f),
      haptic_feedback_enabled_(false),
      animation_duration_ms_(200.0f),
      animations_enabled_(true),
      use_custom_color_(false),
      theme_color_name_("primary") {
    
    setInterceptsMouseClicks(true, true);
    setWantsKeyboardFocus(true);
    
    // Set up accessibility
    accessibility_info_.aria_role = "button";
    accessibility_info_.is_focusable = true;
    accessibility_info_.is_announced = true;
    
    // Initialize button state
    state_.hovered = false;
    state_.focused = false;
    state_.pressed = false;
    state_.disabled = false;
    state_.ripple_progress = 0.0f;
}

void Button::setText(const juce::String& text) {
    if (text_ != text) {
        text_ = text;
        setAriaLabel(text);
        setNeedsRedraw();
    }
}

void Button::setIcon(const juce::String& icon_path) {
    if (icon_path_ != icon_path) {
        icon_path_ = icon_path;
        setNeedsRedraw();
    }
}

void Button::setTrailingIcon(const juce::String& icon_path) {
    if (trailing_icon_path_ != icon_path) {
        trailing_icon_path_ = icon_path;
        setNeedsRedraw();
    }
}

void Button::setIcons(const juce::String& leading, const juce::String& trailing) {
    icon_path_ = leading;
    trailing_icon_path_ = trailing;
    setNeedsRedraw();
}

void Button::clearIcons() {
    icon_path_.clear();
    trailing_icon_path_.clear();
    setNeedsRedraw();
}

void Button::setVariant(ButtonVariant variant) {
    if (variant_ != variant) {
        variant_ = variant;
        setNeedsRedraw();
    }
}

void Button::setSize(ButtonSize size) {
    if (size_ != size) {
        size_ = size;
        setNeedsRedraw();
    }
}

void Button::setEnabled(bool enabled) {
    if (enabled_ != enabled) {
        enabled_ = enabled;
        state_.disabled = !enabled;
        setInterceptsMouseClicks(enabled, enabled);
        setWantsKeyboardFocus(enabled);
        setState(enabled ? (state_.hovered ? State::Hover : State::Normal) : State::Disabled);
    }
}

void Button::setColor(const juce::Colour& color) {
    custom_color_ = color;
    use_custom_color_ = true;
    setNeedsRedraw();
}

void Button::useThemeColor(const juce::String& color_name) {
    theme_color_name_ = color_name;
    use_custom_color_ = false;
    setNeedsRedraw();
}

void Button::enableRippleEffect(bool enable) {
    ripple_enabled_ = enable;
}

void Button::setRippleColor(const juce::Colour& color) {
    ripple_color_ = color;
}

void Button::triggerRipple(const juce::Point<float>& position) {
    if (!ripple_enabled_ || state_.disabled) return;
    
    state_.ripple_center = position;
    ripple_active_ = true;
    ripple_start_time_ = juce::Time::getMillisecondCounter() / 1000.0f;
    setNeedsRedraw();
}

void Button::triggerRippleAtCenter() {
    auto center = getLocalBounds().getCentre().toFloat();
    triggerRipple(center);
}

void Button::setElevationLevel(int level) {
    elevation_level_ = std::clamp(level, 0, 24);
    setNeedsRedraw();
}

void Button::setShadowColor(const juce::Colour& color) {
    shadow_color_ = color;
    setNeedsRedraw();
}

void Button::setClickCallback(ClickCallback callback) {
    click_callback_ = callback;
}

void Button::setFocusCallback(std::function<void(bool)> callback) {
    focus_callback_ = callback;
}

void Button::setHoverCallback(std::function<void(bool)> callback) {
    hover_callback_ = callback;
}

void Button::setPressCallback(std::function<void()> callback) {
    press_callback_ = callback;
}

void Button::clearCallbacks() {
    click_callback_ = nullptr;
    focus_callback_ = nullptr;
    hover_callback_ = nullptr;
    press_callback_ = nullptr;
}

void Button::setTooltip(const juce::String& tooltip) {
    tooltip_ = tooltip;
    accessibility_info_.aria_description = tooltip;
}

void Button::setKeyboardShortcut(const juce::String& shortcut) {
    keyboard_shortcut_ = shortcut;
    accessibility_info_.keyboard_shortcut = shortcut;
}

void Button::setAriaRole(const juce::String& role) {
    accessibility_info_.aria_role = role;
}

void Button::setPressedState(bool pressed) {
    state_.pressed = pressed;
    setNeedsRedraw();
}

void Button::setMinimumTouchTargetSize(float size) {
    min_touch_target_size_ = std::max(size, 24.0f);
    setNeedsRedraw();
}

void Button::setHapticFeedbackEnabled(bool enabled) {
    haptic_feedback_enabled_ = enabled;
}

void Button::setAnimationDuration(float duration_ms) {
    animation_duration_ms_ = std::max(duration_ms, 0.0f);
}

void Button::setAnimationsEnabled(bool enabled) {
    animations_enabled_ = enabled;
}

// Interaction handlers
bool Button::keyPressed(const juce::KeyPress& key) {
    if (!enabled_) return false;
    
    if (key == juce::KeyPress::spaceKey || 
        key == juce::KeyPress::returnKey ||
        key.getKeyCode() == juce::KeyPress::spaceKey) {
        handleClick();
        return true;
    }
    
    return false;
}

void Button::mouseDown(const juce::MouseEvent& e) {
    if (!enabled_) return;
    
    setState(State::Pressed);
    state_.pressed = true;
    handlePressChange(true);
    
    if (ripple_enabled_) {
        auto local_pos = getLocalPoint(e.getEventRelativeTo(this).getPosition());
        triggerRipple(local_pos);
    }
    
    setNeedsRedraw();
}

void Button::mouseUp(const juce::MouseEvent& e) {
    if (!enabled_) return;
    
    auto was_pressed = state_.pressed;
    state_.pressed = false;
    
    setState(state_.hovered ? State::Hover : State::Normal);
    
    if (was_pressed && getBounds().contains(e.getEventRelativeTo(this).getPosition())) {
        handleClick();
    }
    
    handlePressChange(false);
    setNeedsRedraw();
}

void Button::mouseEnter(const juce::MouseEvent& e) {
    if (!enabled_) return;
    
    state_.hovered = true;
    setState(State::Hover);
    handleHoverChange(true);
    setNeedsRedraw();
}

void Button::mouseExit(const juce::MouseEvent& e) {
    if (!enabled_) return;
    
    state_.hovered = false;
    if (!state_.pressed) {
        setState(State::Normal);
    }
    handleHoverChange(false);
    setNeedsRedraw();
}

// Touch event handlers
void Button::touchStart(const juce::TouchEvent& e) {
    if (!enabled_) return;
    
    setState(State::Pressed);
    state_.pressed = true;
    handlePressChange(true);
    
    if (ripple_enabled_) {
        triggerRipple(e.position);
    }
    
    setNeedsRedraw();
}

void Button::touchMove(const juce::TouchEvent& e) {
    // Handle touch move for press state management
}

void Button::touchEnd(const juce::TouchEvent& e) {
    if (!enabled_) return;
    
    auto was_pressed = state_.pressed;
    state_.pressed = false;
    
    setState(state_.hovered ? State::Hover : State::Normal);
    
    if (was_pressed) {
        handleClick();
    }
    
    handlePressChange(false);
    setNeedsRedraw();
}

void Button::touchCancel(const juce::TouchEvent& e) {
    if (!enabled_) return;
    
    state_.pressed = false;
    setState(state_.hovered ? State::Hover : State::Normal);
    handlePressChange(false);
    setNeedsRedraw();
}

// Component updates
void Button::update() {
    if (needs_redraw_) {
        updateInternal();
        updateButtonState(getAnimationProgress());
        needs_redraw_ = false;
    }
}

void Button::render(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    
    // Apply elevation
    if (elevation_level_ > 0) {
        renderElevation(g);
    }
    
    // Render background
    renderBackground(g);
    
    // Render ripple effect
    if (ripple_active_) {
        renderRippleEffect(g);
    }
    
    // Render content
    renderContent(g);
    
    // Render focus ring
    if (isFocused()) {
        renderFocusRing(g);
    }
}

void Button::updateInternal() {
    // Update button state and animations
}

void Button::renderInternal(juce::Graphics& g) {
    // Main rendering is handled in render()
}

void Button::onMouseEnter() {
    state_.hovered = true;
    if (enabled_ && !state_.pressed) {
        setState(State::Hover);
    }
}

void Button::onMouseExit() {
    state_.hovered = false;
    if (enabled_ && !state_.pressed) {
        setState(State::Normal);
    }
}

void Button::onFocus() {
    setState(State::Focused);
    handleFocusChange(true);
}

void Button::onFocusLost() {
    setState(state_.hovered ? State::Hover : State::Normal);
    handleFocusChange(false);
}

void Button::onPress() {
    setState(State::Pressed);
    state_.pressed = true;
}

void Button::onRelease() {
    state_.pressed = false;
    setState(state_.hovered ? State::Hover : State::Normal);
}

void Button::onValueChanged(float value) {
    // Button doesn't have a value, but this is called when enabled changes
}

void Button::onStateChanged(core::Component::State new_state) {
    setNeedsRedraw();
}

// Content bounds calculations
juce::Rectangle<float> Button::getContentBounds() const {
    auto bounds = getLocalBounds().toFloat();
    float padding = getPadding();
    return bounds.reduced(padding);
}

juce::Rectangle<float> Button::getTextBounds() const {
    if (text_.isEmpty()) return juce::Rectangle<float>();
    
    auto content_bounds = getContentBounds();
    auto font = getFont();
    auto text_bounds = font.getStringBounds(text_, juce::Justification::centred);
    
    // Calculate text position within content bounds
    float text_x = content_bounds.getX();
    float text_y = content_bounds.getY() + 
                   (content_bounds.getHeight() - text_bounds.getHeight()) / 2.0f;
    
    return juce::Rectangle<float>(text_x, text_y, content_bounds.getWidth(), text_bounds.getHeight());
}

juce::Rectangle<float> Button::getIconBounds() const {
    if (icon_path_.isEmpty()) return juce::Rectangle<float>();
    
    auto content_bounds = getContentBounds();
    float icon_size = getIconSize();
    float padding = getPadding() * 0.5f;
    
    return juce::Rectangle<float>(
        content_bounds.getX() + padding,
        content_bounds.getY() + (content_bounds.getHeight() - icon_size) / 2.0f,
        icon_size,
        icon_size
    );
}

juce::Rectangle<float> Button::getTrailingIconBounds() const {
    if (trailing_icon_path_.isEmpty()) return juce::Rectangle<float>();
    
    auto content_bounds = getContentBounds();
    float icon_size = getIconSize();
    float padding = getPadding() * 0.5f;
    
    return juce::Rectangle<float>(
        content_bounds.getRight() - padding - icon_size,
        content_bounds.getY() + (content_bounds.getHeight() - icon_size) / 2.0f,
        icon_size,
        icon_size
    );
}

juce::Rectangle<float> Button::getRippleBounds() const {
    auto bounds = getLocalBounds().toFloat();
    float max_dimension = std::max(bounds.getWidth(), bounds.getHeight());
    float radius = max_dimension * 1.5f; // Ripple extends beyond bounds
    
    return juce::Rectangle<float>(
        state_.ripple_center.getX() - radius,
        state_.ripple_center.getY() - radius,
        radius * 2.0f,
        radius * 2.0f
    );
}

// Rendering methods
void Button::renderBackground(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    float corner_radius = getCornerRadius();
    
    // Background color based on variant and state
    juce::Colour background_color = getBackgroundColor();
    
    g.setColour(background_color);
    
    // Handle FAB variants
    if (variant_ == ButtonVariant::FAB || variant_ == ButtonVariant::FABMini) {
        // FAB is circular or rounded
        float fab_size = (variant_ == ButtonVariant::FABMini) ? 
                        std::min(bounds.getWidth(), 48.0f) : 
                        std::min(bounds.getWidth(), 64.0f);
        auto fab_bounds = bounds.withSizeKeepingCentre(fab_size, fab_size);
        g.fillRoundedRectangle(fab_bounds, fab_size / 2.0f);
    } else {
        // Regular button
        g.fillRoundedRectangle(bounds, corner_radius);
    }
    
    // Border for outlined variants
    juce::Colour border_color = getBorderColor();
    if (border_color != juce::Colours::transparentBlack) {
        g.setColour(border_color);
        float border_width = getBorderWidth();
        g.drawRoundedRectangle(bounds.reduced(border_width / 2.0f), 
                              corner_radius, border_width);
    }
}

void Button::renderRippleEffect(juce::Graphics& g) {
    if (!ripple_active_ || state_.disabled) return;
    
    float current_time = juce::Time::getMillisecondCounter() / 1000.0f;
    float elapsed = current_time - ripple_start_time_;
    float duration = animation_duration_ms_ / 1000.0f;
    
    if (elapsed > duration) {
        ripple_active_ = false;
        state_.ripple_progress = 0.0f;
        return;
    }
    
    state_.ripple_progress = elapsed / duration;
    
    auto ripple_bounds = getRippleBounds();
    float max_radius = std::max(ripple_bounds.getWidth(), ripple_bounds.getHeight()) / 2.0f;
    float current_radius = max_radius * state_.ripple_progress;
    
    // Calculate ripple opacity (fades out over time)
    float opacity = (1.0f - state_.ripple_progress) * 0.3f;
    
    g.setColour(ripple_color_.withAlpha(opacity));
    g.fillEllipse(juce::Rectangle<float>(
        state_.ripple_center.getX() - current_radius,
        state_.ripple_center.getY() - current_radius,
        current_radius * 2.0f,
        current_radius * 2.0f
    ));
}

void Button::renderContent(juce::Graphics& g) {
    renderIcons(g);
    renderText(g);
}

void Button::renderText(juce::Graphics& g) {
    if (text_.isEmpty()) return;
    
    auto text_bounds = getTextBounds();
    juce::Font font = getFont();
    juce::Colour text_color = getTextColor();
    
    g.setColour(text_color);
    g.setFont(font);
    g.drawText(text_, text_bounds, juce::Justification::centred, true);
}

void Button::renderIcons(juce::Graphics& g) {
    // Render leading icon
    if (!icon_path_.isEmpty()) {
        auto icon_bounds = getIconBounds();
        g.setColour(getIconColor());
        // In a real implementation, you would load and render the icon here
        // For now, just draw a placeholder rectangle
        g.fillRoundedRectangle(icon_bounds, 2.0f);
    }
    
    // Render trailing icon
    if (!trailing_icon_path_.isEmpty()) {
        auto icon_bounds = getTrailingIconBounds();
        g.setColour(getIconColor());
        // In a real implementation, you would load and render the icon here
        // For now, just draw a placeholder rectangle
        g.fillRoundedRectangle(icon_bounds, 2.0f);
    }
}

void Button::renderFocusRing(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    float corner_radius = getCornerRadius();
    
    g.setColour(juce::Colours::blue.withAlpha(0.8f));
    g.drawRoundedRectangle(bounds.reduced(2.0f), corner_radius, 2.0f);
}

void Button::renderElevation(juce::Graphics& g) {
    if (elevation_level_ <= 0) return;
    
    // Simple shadow implementation
    float blur = elevation_level_ * 2.0f;
    auto offset = juce::Point<float>(0.0f, elevation_level_ * 0.5f);
    
    g.setColour(shadow_color_.withAlpha(0.3f));
    
    auto bounds = getLocalBounds().toFloat();
    auto shadow_bounds = bounds.translated(offset.getX(), offset.getY());
    g.fillRoundedRectangle(shadow_bounds, getCornerRadius());
}

// Color calculations
juce::Colour Button::getBackgroundColor() const {
    juce::Colour base_color = use_custom_color_ ? 
                             custom_color_ : 
                             getThemeColor(theme_color_name_);
    
    switch (variant_) {
        case ButtonVariant::Filled:
            return getStateColor(state_, base_color);
            
        case ButtonVariant::Outlined:
            if (state_ == State::Pressed) {
                return base_color.withAlpha(0.1f);
            }
            return juce::Colours::transparentBlack;
            
        case ButtonVariant::Text:
            return juce::Colours::transparentBlack;
            
        case ButtonVariant::Elevated:
        case ButtonVariant::FAB:
        case ButtonVariant::FABMini:
            return getStateColor(state_, base_color);
            
        default:
            return base_color;
    }
}

juce::Colour Button::getTextColor() const {
    juce::Colour base_color = use_custom_color_ ? 
                             custom_color_ : 
                             getThemeColor(theme_color_name_);
    
    switch (variant_) {
        case ButtonVariant::Filled:
            // Text color for filled buttons depends on the primary color brightness
            return (base_color.getBrightness() > 0.5f) ? 
                   juce::Colours::black : juce::Colours::white;
                   
        case ButtonVariant::Outlined:
        case ButtonVariant::Text:
        case ButtonVariant::Elevated:
            // Use primary color for text
            return getStateColor(state_, base_color);
            
        case ButtonVariant::FAB:
        case ButtonVariant::FABMini:
            // FAB uses primary color for text/icon
            return juce::Colours::white;
            
        default:
            return juce::Colours::black;
    }
}

juce::Colour Button::getBorderColor() const {
    if (variant_ != ButtonVariant::Outlined) {
        return juce::Colours::transparentBlack;
    }
    
    juce::Colour base_color = use_custom_color_ ? 
                             custom_color_ : 
                             getThemeColor(theme_color_name_);
    
    return getStateColor(state_, base_color).withAlpha(0.5f);
}

juce::Colour Button::getIconColor() const {
    return getTextColor();
}

// Layout calculations
float Button::getPadding() const {
    switch (size_) {
        case ButtonSize::Small: return 8.0f;
        case ButtonSize::Medium: return 12.0f;
        case ButtonSize::Large: return 16.0f;
        default: return 12.0f;
    }
}

float Button::getMinHeight() const {
    switch (size_) {
        case ButtonSize::Small: return 32.0f;
        case ButtonSize::Medium: return 40.0f;
        case ButtonSize::Large: return 48.0f;
        default: return 40.0f;
    }
}

float Button::getMaxWidth() const {
    if (variant_ == ButtonVariant::FAB || variant_ == ButtonVariant::FABMini) {
        return (variant_ == ButtonVariant::FABMini) ? 48.0f : 64.0f;
    }
    return 320.0f; // Material Design recommendation
}

juce::Font Button::getFont() const {
    switch (size_) {
        case ButtonSize::Small: 
            return juce::Font(12.0f);
        case ButtonSize::Medium: 
            return juce::Font(14.0f);
        case ButtonSize::Large: 
            return juce::Font(16.0f);
        default: 
            return juce::Font(14.0f);
    }
}

float Button::getIconSize() const {
    switch (size_) {
        case ButtonSize::Small: return 16.0f;
        case ButtonSize::Medium: return 18.0f;
        case ButtonSize::Large: return 20.0f;
        default: return 18.0f;
    }
}

float Button::getCornerRadius() const {
    switch (variant_) {
        case ButtonVariant::FAB:
        case ButtonVariant::FABMini:
            return 50.0f; // Circular
        default:
            return 4.0f;
    }
}

float Button::getBorderWidth() const {
    return (variant_ == ButtonVariant::Outlined) ? 1.0f : 0.0f;
}

// Animation helpers
void Button::updateRipple(float delta_time) {
    if (ripple_active_) {
        state_.ripple_progress += delta_time * 4.0f; // Ripple speed
        if (state_.ripple_progress >= 1.0f) {
            ripple_active_ = false;
            state_.ripple_progress = 0.0f;
        }
    }
}

void Button::updateButtonState(float delta_time) {
    updateRipple(delta_time);
}

float Button::getAnimationProgress() const {
    return animation_state_.progress;
}

// Event handling
void Button::handleClick() {
    if (click_callback_) {
        click_callback_();
    }
    
    // Announce to screen reader
    if (!text_.isEmpty()) {
        announceToScreenReader("Button clicked: " + text_);
    } else {
        announceToScreenReader("Button clicked");
    }
}

void Button::handleFocusChange(bool gained) {
    if (focus_callback_) {
        focus_callback_(gained);
    }
}

void Button::handleHoverChange(bool entered) {
    if (hover_callback_) {
        hover_callback_(entered);
    }
}

void Button::handlePressChange(bool pressed) {
    if (press_callback_ && pressed) {
        press_callback_();
    }
}

// Utility methods
bool Button::hasIcons() const {
    return !icon_path_.isEmpty() || !trailing_icon_path_.isEmpty();
}

bool Button::shouldShowIconOnly() const {
    return !text_.isEmpty() && text_.length() <= 2 && hasIcons();
}

juce::String Button::getAccessibilityText() const {
    juce::String text = text_;
    if (text.isEmpty()) {
        text = "Button";
    }
    
    if (!keyboard_shortcut_.isEmpty()) {
        text += " (Keyboard shortcut: " + keyboard_shortcut_ + ")";
    }
    
    return text;
}

} // namespace material
} // namespace ui
} // namespace vital