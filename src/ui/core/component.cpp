#include "component.h"
#include <cmath>
#include <algorithm>

namespace vital {
namespace ui {
namespace core {

Component::Component() {
    setInterceptsMouseClicks(true, true);
    setWantsKeyboardFocus(true);
    accessible_ = true;
}

void Component::setState(State state) {
    if (state_ != state) {
        state_ = state;
        onStateChanged(state);
        setNeedsRedraw();
    }
}

void Component::setEnabled(bool enabled) {
    if (enabled_ != enabled) {
        enabled_ = enabled;
        setInterceptsMouseClicks(enabled, enabled);
        setWantsKeyboardFocus(enabled);
        setState(enabled ? State::Normal : State::Disabled);
        onValueChanged(enabled ? 1.0f : 0.0f);
    }
}

void Component::setVisible(bool should_be_visible) {
    if (visible_ != should_be_visible) {
        visible_ = should_be_visible;
        juce::Component::setVisible(should_be_visible);
        setNeedsRedraw();
    }
}

void Component::setAlpha(float new_alpha) {
    if (alpha_ != new_alpha) {
        alpha_ = std::clamp(new_alpha, 0.0f, 1.0f);
        juce::Component::setAlpha(alpha_);
        setNeedsRedraw();
    }
}

void Component::animateOpacity(float target_alpha, float duration_ms) {
    // TODO: Implement opacity animation using AnimationEngine
    setAlpha(target_alpha);
}

void Component::setAnimating(bool animating) {
    is_animating_ = animating;
}

void Component::animateBounds(const juce::Rectangle<float>& target_bounds, 
                             float duration_ms,
                             const std::function<void()>& completion_callback) {
    // TODO: Implement bounds animation using AnimationEngine
    setBounds(juce::Rectangle<int>(static_cast<int>(target_bounds.getX()),
                                   static_cast<int>(target_bounds.getY()),
                                   static_cast<int>(target_bounds.getWidth()),
                                   static_cast<int>(target_bounds.getHeight())));
}

void Component::animatePosition(float target_x, float target_y, float duration_ms) {
    // TODO: Implement position animation using AnimationEngine
    setTopLeftPosition(static_cast<int>(target_x), static_cast<int>(target_y));
}

void Component::animateSize(float target_width, float target_height, float duration_ms) {
    // TODO: Implement size animation using AnimationEngine
    setSize(static_cast<int>(target_width), static_cast<int>(target_height));
}

void Component::stopAnimations() {
    animation_state_.is_animating = false;
    animation_state_.progress = 0.0f;
    animation_state_.velocity = 0.0f;
}

void Component::update() {
    if (needs_redraw_) {
        updateInternal();
        updateAnimationState();
        needs_redraw_ = false;
    }
}

void Component::render(juce::Graphics& g) {
    g.saveState();
    
    // Apply alpha
    g.setGlobalAlpha(alpha_);
    
    // Apply transforms if animating
    if (animation_state_.is_animating) {
        g.addTransform(juce::AffineTransform()
            .translation(animation_state_.current_bounds.getX(),
                        animation_state_.current_bounds.getY()));
    }
    
    renderInternal(g);
    renderPost(g);
    
    g.restoreState();
}

void Component::renderPost(juce::Graphics& g) {
    // Base implementation - can be overridden for post-render effects
    if (state_ == State::Focused && accessibility_info_.is_focusable) {
        renderFocusIndicator(g);
    }
}

void Component::applyConstraints() {
    auto bounds = getBounds();
    
    // Apply size constraints
    bounds.setWidth(std::clamp(bounds.getWidth(), 
                              static_cast<int>(constraints_.min_width),
                              static_cast<int>(constraints_.max_width)));
    bounds.setHeight(std::clamp(bounds.getHeight(),
                               static_cast<int>(constraints_.min_height),
                               static_cast<int>(constraints_.max_height)));
    
    // Apply aspect ratio if specified
    if (constraints_.aspect_ratio > 0.0f) {
        float aspect_ratio = constraints_.aspect_ratio;
        float current_ratio = static_cast<float>(bounds.getWidth()) / bounds.getHeight();
        
        if (std::abs(current_ratio - aspect_ratio) > 0.01f) {
            // Adjust to match aspect ratio
            if (current_ratio > aspect_ratio) {
                bounds.setWidth(static_cast<int>(bounds.getHeight() * aspect_ratio));
            } else {
                bounds.setHeight(static_cast<int>(bounds.getWidth() / aspect_ratio));
            }
        }
    }
    
    setBounds(bounds);
}

void Component::setConstraints(const Constraints& constraints) {
    constraints_ = constraints;
    applyConstraints();
}

void Component::onMouseEnter() {
    if (enabled_) {
        setState(State::Hover);
        onGestureDetected("mouseEnter");
    }
}

void Component::onMouseExit() {
    if (enabled_ && !isPressed()) {
        setState(State::Normal);
        onGestureDetected("mouseExit");
    }
}

void Component::onFocus() {
    if (enabled_ && accessibility_info_.is_focusable) {
        setState(State::Focused);
        announceToScreenReader("Focused: " + accessibility_info_.aria_label);
        onGestureDetected("focus");
    }
}

void Component::onFocusLost() {
    if (state_ == State::Focused) {
        setState(State::Normal);
        onGestureDetected("blur");
    }
}

void Component::onPress() {
    if (enabled_) {
        setState(State::Pressed);
        onGestureDetected("press");
    }
}

void Component::onRelease() {
    if (enabled_) {
        setState(isHovered() ? State::Hover : State::Normal);
        onGestureDetected("release");
    }
}

void Component::onValueChanged(float value) {
    // Base implementation - can be overridden
    setNeedsRedraw();
}

void Component::onStateChanged(State new_state) {
    // Base implementation - can be overridden
    setNeedsRedraw();
}

void Component::onGestureDetected(const juce::String& gesture_type) {
    // Base implementation - can be overridden or connected to gesture system
}

void Component::onTouchBegin(const juce::TouchEvent& event) {
    if (enabled_) {
        setState(State::Pressed);
        onGestureDetected("touchBegin");
    }
}

void Component::onTouchMove(const juce::TouchEvent& event) {
    // Base implementation - can be overridden
    onGestureDetected("touchMove");
}

void Component::onTouchEnd(const juce::TouchEvent& event) {
    if (enabled_) {
        setState(isHovered() ? State::Hover : State::Normal);
        onGestureDetected("touchEnd");
    }
}

void Component::onTouchCancel(const juce::TouchEvent& event) {
    if (enabled_) {
        setState(State::Normal);
        onGestureDetected("touchCancel");
    }
}

void Component::setAccessible(bool accessible) {
    accessible_ = accessible;
    setWantsKeyboardFocus(accessible);
}

void Component::setAriaLabel(const juce::String& label) {
    accessibility_info_.aria_label = label;
    setComponentID(label);
}

void Component::setAriaDescription(const juce::String& description) {
    accessibility_info_.aria_description = description;
}

void Component::setAriaRole(const juce::String& role) {
    accessibility_info_.aria_role = role;
}

void Component::setAriaValue(const juce::String& value_now, 
                           const juce::String& min,
                           const juce::String& max) {
    accessibility_info_.value_now = value_now;
    accessibility_info_.value_min = min;
    accessibility_info_.value_max = max;
}

void Component::setFocusable(bool focusable) {
    accessibility_info_.is_focusable = focusable;
    setWantsKeyboardFocus(focusable);
}

void Component::setKeyboardShortcut(const juce::String& shortcut) {
    accessibility_info_.keyboard_shortcut = shortcut;
}

void Component::setTabIndex(int index) {
    accessibility_info_.tab_index = index;
}

void Component::announceToScreenReader(const juce::String& message, const juce::String& priority) {
    // TODO: Implement screen reader announcement through AccessibilityManager
    juce::ignoreUnused(message, priority);
}

void Component::setPosition(const juce::Point<float>& position) {
    position_ = position;
    setTopLeftPosition(static_cast<int>(position.getX()), static_cast<int>(position.getY()));
}

void Component::setSize(float width, float height) {
    juce::Component::setSize(static_cast<int>(width), static_cast<int>(height));
    setNeedsRedraw();
}

void Component::setBounds(const juce::Rectangle<int>& bounds) {
    juce::Component::setBounds(bounds);
    setNeedsRedraw();
}

void Component::setBounds(const juce::Rectangle<float>& bounds) {
    setBounds(juce::Rectangle<int>(static_cast<int>(bounds.getX()),
                                   static_cast<int>(bounds.getY()),
                                   static_cast<int>(bounds.getWidth()),
                                   static_cast<int>(bounds.getHeight())));
}

juce::Point<float> Component::getPosition() const {
    return position_;
}

juce::Rectangle<float> Component::getFloatBounds() const {
    auto bounds = getBounds();
    return juce::Rectangle<float>(bounds.getX(), bounds.getY(), 
                                  bounds.getWidth(), bounds.getHeight());
}

void Component::centerInParent() {
    auto parent_bounds = getParentComponent()->getLocalBounds();
    auto component_bounds = getBounds();
    
    int x = parent_bounds.getCentreX() - component_bounds.getWidth() / 2;
    int y = parent_bounds.getCentreY() - component_bounds.getHeight() / 2;
    
    setTopLeftPosition(x, y);
}

void Component::alignInParent(Alignment alignment) {
    auto parent_bounds = getParentComponent()->getLocalBounds();
    auto component_bounds = getBounds();
    
    int x, y;
    
    switch (alignment) {
        case Alignment::TopLeft:
            x = parent_bounds.getX();
            y = parent_bounds.getY();
            break;
        case Alignment::TopCenter:
            x = parent_bounds.getCentreX() - component_bounds.getWidth() / 2;
            y = parent_bounds.getY();
            break;
        case Alignment::TopRight:
            x = parent_bounds.getRight() - component_bounds.getWidth();
            y = parent_bounds.getY();
            break;
        case Alignment::CenterLeft:
            x = parent_bounds.getX();
            y = parent_bounds.getCentreY() - component_bounds.getHeight() / 2;
            break;
        case Alignment::Center:
            x = parent_bounds.getCentreX() - component_bounds.getWidth() / 2;
            y = parent_bounds.getCentreY() - component_bounds.getHeight() / 2;
            break;
        case Alignment::CenterRight:
            x = parent_bounds.getRight() - component_bounds.getWidth();
            y = parent_bounds.getCentreY() - component_bounds.getHeight() / 2;
            break;
        case Alignment::BottomLeft:
            x = parent_bounds.getX();
            y = parent_bounds.getBottom() - component_bounds.getHeight();
            break;
        case Alignment::BottomCenter:
            x = parent_bounds.getCentreX() - component_bounds.getWidth() / 2;
            y = parent_bounds.getBottom() - component_bounds.getHeight();
            break;
        case Alignment::BottomRight:
            x = parent_bounds.getRight() - component_bounds.getWidth();
            y = parent_bounds.getBottom() - component_bounds.getHeight();
            break;
        default:
            return;
    }
    
    setTopLeftPosition(x, y);
}

juce::Colour Component::getThemeColor(const juce::String& color_name) const {
    // TODO: Implement theme color lookup through ThemeManager
    return juce::Colours::blue; // Placeholder
}

float Component::getThemeValue(const juce::String& value_name) const {
    // TODO: Implement theme value lookup through ThemeManager
    return 1.0f; // Placeholder
}

void Component::setNeedsRedraw() {
    needs_redraw_ = true;
}

void Component::setLayerOptimizationEnabled(bool enabled) {
    layer_optimization_enabled_ = enabled;
}

void Component::setCachingEnabled(bool enabled) {
    caching_enabled_ = enabled;
}

juce::Colour Component::getStateColor(State state, const juce::Colour& normal_color) const {
    switch (state) {
        case State::Hover:
            return normal_color.brighter(0.1f);
        case State::Focused:
            return normal_color.brighter(0.2f);
        case State::Pressed:
            return normal_color.darker(0.2f);
        case State::Disabled:
            return normal_color.withAlpha(0.5f);
        case State::Normal:
        default:
            return normal_color;
    }
}

void Component::renderBackground(juce::Graphics& g, const juce::Colour& color) {
    g.fillAll(color);
}

void Component::renderBorder(juce::Graphics& g, const juce::Colour& color, float thickness) {
    g.setColour(color);
    g.drawRect(getLocalBounds().toFloat(), thickness);
}

void Component::renderShadow(juce::Graphics& g, float blur, const juce::Colour& color, 
                           const juce::Point<float>& offset) {
    // TODO: Implement proper shadow rendering with blur
    g.setColour(color);
    auto bounds = getLocalBounds().toFloat();
    bounds.translate(offset.getX(), offset.getY());
    g.fillRect(bounds);
}

void Component::renderFocusIndicator(juce::Graphics& g) {
    g.setColour(juce::Colours::blue.withAlpha(0.8f));
    g.drawRect(getLocalBounds().toFloat().reduced(1), 2.0f);
}

void Component::renderElevation(juce::Graphics& g, int elevation_level) {
    // TODO: Implement proper elevation system
    float opacity = std::min(0.3f, elevation_level * 0.1f);
    juce::Colour shadow_color = juce::Colours::black.withAlpha(opacity);
    
    auto bounds = getLocalBounds().toFloat();
    g.setColour(shadow_color);
    g.fillRect(bounds.reduced(elevation_level));
}

void Component::updateInternal() {
    // Base implementation - can be overridden by subclasses
}

void Component::renderInternal(juce::Graphics& g) {
    // Base implementation - can be overridden by subclasses
    // Draw a simple background by default
    auto bounds = getLocalBounds().toFloat();
    g.fillAll(getStateColor(state_, getThemeColor("surface")));
}

void Component::onAnimationUpdate() {
    // Base implementation - can be overridden by subclasses
}

void Component::paint(juce::Graphics& g) {
    render(g);
}

void Component::mouseEnter(const juce::MouseEvent& e) {
    juce::Component::mouseEnter(e);
    onMouseEnter();
}

void Component::mouseExit(const juce::MouseEvent& e) {
    juce::Component::mouseExit(e);
    onMouseExit();
}

void Component::focusGained(FocusChangeType cause) {
    juce::Component::focusGained(cause);
    onFocus();
}

void Component::focusLost(FocusChangeType cause) {
    juce::Component::focusLost(cause);
    onFocusLost();
}

void Component::parentHierarchyChanged() {
    juce::Component::parentHierarchyChanged();
    applyConstraints();
}

} // namespace core
} // namespace ui
} // namespace vital
