#include "slider.h"
#include <cmath>
#include <algorithm>

namespace vital {
namespace ui {
namespace material {

Slider::Slider(SliderStyle style, SliderOrientation orientation)
    : style_(style), orientation_(orientation) {
    setWantsKeyboardFocus(true);
    setInterceptsMouseClicks(true, true);
    
    // Initialize colors
    slider_color_ = juce::Colours::blue;
    track_color_ = juce::Colours::grey.withAlpha(0.3f);
    thumb_color_ = juce::Colours::white;
    tick_mark_color_ = juce::Colours::grey;
    
    // Initialize dimensions
    track_thickness_ = 4.0f;
    thumb_size_ = 20.0f;
    min_slider_length_ = 100.0f;
    
    // Initialize callbacks to nullptr
    value_changed_callback_ = nullptr;
    value_change_start_callback_ = nullptr;
    value_change_end_callback_ = nullptr;
    drag_callback_ = nullptr;
}

void Slider::setValue(float value, bool notify) {
    value = std::clamp(value, range_.minimum, range_.maximum);
    
    if (value_ != value) {
        float old_value = value_;
        value_ = value;
        
        if (notify) {
            if (value_change_start_callback_ && old_value != value_) {
                value_change_start_callback_(old_value);
            }
            
            if (value_changed_callback_) {
                value_changed_callback_(value_);
            }
            
            if (value_change_end_callback_) {
                value_change_end_callback_(value_);
            }
        }
        
        onValueChanged(value_);
        setNeedsRedraw();
    }
}

void Slider::setRange(float min_value, float max_value, float step_value) {
    range_.minimum = min_value;
    range_.maximum = max_value;
    range_.step = step_value;
    
    if (range_.step > 0.0f) {
        discrete_mode_ = true;
        value_ = snapToStep(value_);
    }
    
    setNeedsRedraw();
}

void Slider::setRange(const ValueRange& range) {
    range_ = range;
    if (range_.step > 0.0f) {
        discrete_mode_ = true;
    }
    setNeedsRedraw();
}

float Slider::snapToStep(float value) const {
    if (range_.step <= 0.0f) return value;
    
    float steps = std::round((value - range_.minimum) / range_.step);
    return range_.minimum + (steps * range_.step);
}

bool Slider::isValueInRange(float value) const {
    return value >= range_.minimum && value <= range_.maximum;
}

float Slider::valueToPosition(float value) const {
    float normalized = (value - range_.minimum) / (range_.maximum - range_.minimum);
    return std::clamp(normalized, 0.0f, 1.0f);
}

float Slider::positionToValue(float position) const {
    position = std::clamp(position, 0.0f, 1.0f);
    float value = range_.minimum + (position * (range_.maximum - range_.minimum));
    
    if (discrete_mode_) {
        return snapToStep(value);
    }
    
    return value;
}

void Slider::setStyle(SliderStyle style) {
    if (style_ != style) {
        style_ = style;
        setNeedsRedraw();
    }
}

void Slider::setOrientation(SliderOrientation orientation) {
    if (orientation_ != orientation) {
        orientation_ = orientation;
        setNeedsRedraw();
    }
}

void Slider::setEnabled(bool enabled) {
    if (enabled_ != enabled) {
        enabled_ = enabled;
        setInterceptsMouseClicks(enabled, enabled);
        setWantsKeyboardFocus(enabled);
        setState(enabled ? core::Component::State::Normal : core::Component::State::Disabled);
        onValueChanged(enabled ? 1.0f : 0.0f);
    }
}

void Slider::setColor(const juce::Colour& color) {
    slider_color_ = color;
    setNeedsRedraw();
}

void Slider::setTrackColor(const juce::Colour& color) {
    track_color_ = color;
    setNeedsRedraw();
}

void Slider::setThumbColor(const juce::Colour& color) {
    thumb_color_ = color;
    setNeedsRedraw();
}

void Slider::setValueLabelVisible(bool visible) {
    show_value_label_ = visible;
    setNeedsRedraw();
}

void Slider::setValueLabelPrecision(int precision) {
    value_label_precision_ = std::max(0, precision);
    setNeedsRedraw();
}

void Slider::setValueFormatter(std::function<juce::String(float)> formatter) {
    value_formatter_ = formatter;
    setNeedsRedraw();
}

void Slider::addTickMark(float position, const juce::String& label, bool major) {
    position = std::clamp(position, 0.0f, 1.0f);
    
    TickMark tick;
    tick.position = position;
    tick.label = label;
    tick.major = major;
    tick.color = tick_mark_color_;
    
    tick_marks_.push_back(tick);
    
    // Sort tick marks by position
    std::sort(tick_marks_.begin(), tick_marks_.end(), 
              [](const TickMark& a, const TickMark& b) {
                  return a.position < b.position;
              });
    
    show_tick_marks_ = true;
    setNeedsRedraw();
}

void Slider::removeTickMark(float position) {
    position = std::clamp(position, 0.0f, 1.0f);
    
    auto it = std::find_if(tick_marks_.begin(), tick_marks_.end(),
                          [position](const TickMark& tick) {
                              return std::abs(tick.position - position) < 0.01f;
                          });
    
    if (it != tick_marks_.end()) {
        tick_marks_.erase(it);
    }
    
    if (tick_marks_.empty()) {
        show_tick_marks_ = false;
    }
    
    setNeedsRedraw();
}

void Slider::clearTickMarks() {
    tick_marks_.clear();
    show_tick_marks_ = false;
    setNeedsRedraw();
}

void Slider::setTickMarksFromValues(const std::vector<float>& positions) {
    tick_marks_.clear();
    
    for (float value : positions) {
        float position = valueToPosition(value);
        addTickMark(position);
    }
}

void Slider::setTickMarksVisible(bool visible) {
    show_tick_marks_ = visible;
    setNeedsRedraw();
}

void Slider::setDiscreteMode(bool discrete) {
    discrete_mode_ = discrete;
    if (discrete && range_.step <= 0.0f) {
        range_.step = 1.0f; // Default step for discrete mode
    }
    setNeedsRedraw();
}

void Slider::setTickMarkSnapping(bool snapping) {
    tick_mark_snapping_ = snapping;
    setNeedsRedraw();
}

void Slider::setHeight(float height) {
    // Slider height depends on orientation
    if (orientation_ == SliderOrientation::Horizontal) {
        setSize(getWidth(), static_cast<int>(height));
    } else {
        // For vertical sliders, height becomes width
        setSize(static_cast<int>(height), getHeight());
    }
}

void Slider::setWidth(float width) {
    // Slider width depends on orientation
    if (orientation_ == SliderOrientation::Horizontal) {
        setSize(static_cast<int>(width), getHeight());
    } else {
        // For vertical sliders, width becomes height
        setSize(getWidth(), static_cast<int>(width));
    }
}

void Slider::setTrackThickness(float thickness) {
    track_thickness_ = std::max(1.0f, thickness);
    setNeedsRedraw();
}

void Slider::setThumbSize(float size) {
    thumb_size_ = std::max(10.0f, size);
    setNeedsRedraw();
}

void Slider::setMinimumSliderLength(float length) {
    min_slider_length_ = std::max(50.0f, length);
    setNeedsRedraw();
}

void Slider::setValueChangedCallback(ValueChangedCallback callback) {
    value_changed_callback_ = callback;
}

void Slider::setValueChangeStartCallback(ValueChangeStartCallback callback) {
    value_change_start_callback_ = callback;
}

void Slider::setValueChangeEndCallback(ValueChangeEndCallback callback) {
    value_change_end_callback_ = callback;
}

void Slider::setDragCallback(SliderDragCallback callback) {
    drag_callback_ = callback;
}

void Slider::clearCallbacks() {
    value_changed_callback_ = nullptr;
    value_change_start_callback_ = nullptr;
    value_change_end_callback_ = nullptr;
    drag_callback_ = nullptr;
}

void Slider::setAriaRole(const juce::String& role) {
    core::Component::setAriaRole(role);
    if (role.isEmpty()) {
        setAriaRole("slider");
    }
}

void Slider::setDescription(const juce::String& description) {
    description_ = description;
    setAriaDescription(description);
}

void Slider::setKeyboardIncrements(float small_increment, float large_increment) {
    small_increment_ = std::max(0.0f, small_increment);
    large_increment_ = std::max(0.0f, large_increment);
}

void Slider::setKeyboardShortcuts(const juce::String& increment_key,
                                 const juce::String& decrement_key) {
    increment_key_ = increment_key;
    decrement_key_ = decrement_key;
}

bool Slider::keyPressed(const juce::KeyPress& key) {
    if (!enabled_) return false;
    
    float new_value = value_;
    bool handled = false;
    
    if (key == juce::KeyPress::leftKey || key == juce::KeyPress::downKey) {
        new_value -= small_increment_;
        handled = true;
    } else if (key == juce::KeyPress::rightKey || key == juce::KeyPress::upKey) {
        new_value += small_increment_;
        handled = true;
    } else if (key == juce::KeyPress::pageUpKey) {
        new_value += large_increment_;
        handled = true;
    } else if (key == juce::KeyPress::pageDownKey) {
        new_value -= large_increment_;
        handled = true;
    } else if (key == juce::KeyPress::homeKey) {
        new_value = range_.minimum;
        handled = true;
    } else if (key == juce::KeyPress::endKey) {
        new_value = range_.maximum;
        handled = true;
    }
    
    if (handled) {
        new_value = std::clamp(new_value, range_.minimum, range_.maximum);
        if (discrete_mode_) {
            new_value = snapToStep(new_value);
        }
        
        if (new_value != value_) {
            setValue(new_value);
            announceToScreenReader("Value: " + formatValue(value_));
        }
        
        return true;
    }
    
    return false;
}

void Slider::mouseDown(const juce::MouseEvent& e) {
    if (!enabled_) return;
    
    handleMouseDown(e);
}

void Slider::mouseDrag(const juce::MouseEvent& e) {
    if (!enabled_ || !state_.dragging) return;
    
    handleMouseDrag(e);
}

void Slider::mouseUp(const juce::MouseEvent& e) {
    if (!enabled_) return;
    
    state_.dragging = false;
    setState(state_.hovering ? core::Component::State::Hover : core::Component::State::Normal);
    
    // Announce final value
    announceToScreenReader("Value: " + formatValue(value_));
}

void Slider::mouseEnter(const juce::MouseEvent& e) {
    if (!enabled_) return;
    
    state_.hovering = true;
    setState(core::Component::State::Hover);
}

void Slider::mouseExit(const juce::MouseEvent& e) {
    if (!enabled_) return;
    
    state_.hovering = false;
    if (!state_.dragging) {
        setState(core::Component::State::Normal);
    }
}

void Slider::touchStart(const juce::TouchEvent& e) {
    if (!enabled_) return;
    
    handleTouchStart(e);
}

void Slider::touchMove(const juce::TouchEvent& e) {
    if (!enabled_ || !state_.dragging) return;
    
    handleTouchMove(e);
}

void Slider::touchEnd(const juce::TouchEvent& e) {
    if (!enabled_) return;
    
    state_.dragging = false;
    setState(state_.hovering ? core::Component::State::Hover : core::Component::State::Normal);
    
    // Announce final value
    announceToScreenReader("Value: " + formatValue(value_));
}

void Slider::touchCancel(const juce::TouchEvent& e) {
    if (!enabled_) return;
    
    state_.dragging = false;
    setState(core::Component::State::Normal);
    setValue(state_.drag_start_value);
}

void Slider::update() {
    core::Component::update();
    
    updateThumbAnimation(0.016f); // Assume 60 FPS
}

void Slider::render(juce::Graphics& g) {
    core::Component::render(g);
}

void Slider::updateInternal() {
    // Update track progress
    state_.track_progress = valueToPosition(value_);
}

void Slider::renderInternal(juce::Graphics& g) {
    switch (style_) {
        case SliderStyle::Linear:
            renderLinearSlider(g);
            break;
        case SliderStyle::Circular:
            renderCircularSlider(g);
            break;
        case SliderStyle::Discrete:
            renderDiscreteSlider(g);
            break;
        case SliderStyle::TickMark:
            renderLinearSlider(g);
            break;
        case SliderStyle::FloatingLabel:
            renderLinearSlider(g);
            break;
    }
}

void Slider::onMouseEnter() {
    if (enabled_) {
        setState(core::Component::State::Hover);
    }
}

void Slider::onMouseExit() {
    if (enabled_ && !state_.dragging) {
        setState(core::Component::State::Normal);
    }
}

void Slider::onFocus() {
    if (enabled_) {
        setState(core::Component::State::Focused);
        announceToScreenReader("Slider focused. Value: " + formatValue(value_));
    }
}

void Slider::onFocusLost() {
    if (state_ == core::Component::State::Focused) {
        setState(core::Component::State::Normal);
    }
}

void Slider::onPress() {
    if (enabled_) {
        setState(core::Component::State::Pressed);
    }
}

void Slider::onRelease() {
    if (enabled_) {
        setState(state_.hovering ? core::Component::State::Hover : core::Component::State::Normal);
    }
}

void Slider::onValueChanged(float value) {
    setNeedsRedraw();
}

void Slider::onStateChanged(core::Component::State new_state) {
    setNeedsRedraw();
}

void Slider::renderLinearSlider(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    auto track_bounds = getTrackBounds();
    auto thumb_bounds = getThumbBounds();
    
    // Render track
    renderTrack(g);
    
    // Render active track portion
    renderActiveArea(g);
    
    // Render tick marks if enabled
    if (show_tick_marks_ && !tick_marks_.empty()) {
        renderTickMarks(g);
    }
    
    // Render thumb
    renderThumb(g);
    
    // Render value label if enabled
    if (show_value_label_) {
        renderValueLabel(g);
    }
    
    // Render focus ring if focused
    if (state_ == core::Component::State::Focused) {
        renderFocusRing(g);
    }
}

void Slider::renderCircularSlider(juce::Graphics& g) {
    // TODO: Implement circular slider rendering
    renderLinearSlider(g); // Fallback to linear for now
}

void Slider::renderDiscreteSlider(juce::Graphics& g) {
    renderLinearSlider(g);
    
    // Add discrete-specific visual indicators
    // TODO: Implement discrete slider specific features
}

void Slider::renderTrack(juce::Graphics& g) {
    auto track_bounds = getTrackBounds();
    
    g.setColour(track_color_);
    g.fillRoundedRectangle(track_bounds, track_thickness_ * 0.5f);
}

void Slider::renderThumb(juce::Graphics& g) {
    auto thumb_bounds = getThumbBounds();
    auto scale = state_.active_thumb_scale;
    
    // Apply scaling for active state
    juce::AffineTransform transform = juce::AffineTransform::identity;
    if (scale != 1.0f) {
        auto center = thumb_bounds.getCentre();
        transform = transform.translated(center.x, center.y)
                           .scaled(scale)
                           .translated(-center.x, -center.y);
    }
    
    g.saveState();
    g.addTransform(transform);
    
    // Thumb shadow
    if (state_ == core::Component::State::Hover || state_ == core::Component::State::Pressed) {
        g.setColour(thumb_color_.withAlpha(0.3f));
        g.fillEllipse(thumb_bounds.reduced(2));
    }
    
    // Thumb main circle
    g.setColour(thumb_color_);
    g.fillEllipse(thumb_bounds);
    
    // Thumb border
    g.setColour(slider_color_.withAlpha(0.8f));
    g.drawEllipse(thumb_bounds, 2.0f);
    
    g.restoreState();
}

void Slider::renderTickMarks(juce::Graphics& g) {
    auto track_bounds = getTrackBounds();
    
    for (const auto& tick : tick_marks_) {
        auto tick_bounds = getTickMarkBounds(tick);
        
        g.setColour(tick.color);
        float thickness = tick.major ? 2.0f : 1.0f;
        float length = tick.major ? 8.0f : 4.0f;
        
        if (orientation_ == SliderOrientation::Horizontal) {
            g.drawVerticalLine(static_cast<int>(tick_bounds.getX()),
                             tick_bounds.getY(),
                             tick_bounds.getY() + length);
        } else {
            g.drawHorizontalLine(static_cast<int>(tick_bounds.getY()),
                               tick_bounds.getX(),
                               tick_bounds.getX() + length);
        }
        
        // Draw label if provided
        if (!tick.label.isEmpty()) {
            g.setColour(tick.color);
            g.setFont(juce::Font(10.0f));
            g.drawText(tick.label, tick_bounds, juce::Justification::centred);
        }
    }
}

void Slider::renderValueLabel(juce::Graphics& g) {
    auto label_bounds = getValueLabelBounds();
    juce::String value_text = formatValue(value_);
    
    // Background
    g.setColour(slider_color_.withAlpha(0.9f));
    g.fillRoundedRectangle(label_bounds.reduced(4), 4.0f);
    
    // Text
    g.setColour(juce::Colours::white);
    g.setFont(juce::Font(12.0f));
    g.drawText(value_text, label_bounds, juce::Justification::centred);
}

void Slider::renderFocusRing(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    g.setColour(juce::Colours::blue.withAlpha(0.8f));
    g.drawRoundedRectangle(bounds.reduced(2), 4.0f, 2.0f);
}

void Slider::renderActiveArea(juce::Graphics& g) {
    auto track_bounds = getTrackBounds();
    auto active_bounds = track_bounds;
    
    if (orientation_ == SliderOrientation::Horizontal) {
        float active_width = track_bounds.getWidth() * state_.track_progress;
        active_bounds.setWidth(active_width);
    } else {
        float active_height = track_bounds.getHeight() * state_.track_progress;
        active_bounds.setBottom(active_bounds.getBottom() - (track_bounds.getHeight() - active_height));
    }
    
    g.setColour(slider_color_);
    g.fillRoundedRectangle(active_bounds, track_thickness_ * 0.5f);
}

juce::Rectangle<float> Slider::getTrackBounds() const {
    auto bounds = getLocalBounds().toFloat();
    
    if (orientation_ == SliderOrientation::Horizontal) {
        float padding = thumb_size_ * 0.5f;
        return juce::Rectangle<float>(bounds.getX() + padding,
                                     bounds.getCentreY() - track_thickness_ * 0.5f,
                                     bounds.getWidth() - padding * 2.0f,
                                     track_thickness_);
    } else {
        float padding = thumb_size_ * 0.5f;
        return juce::Rectangle<float>(bounds.getCentreX() - track_thickness_ * 0.5f,
                                     bounds.getY() + padding,
                                     track_thickness_,
                                     bounds.getHeight() - padding * 2.0f);
    }
}

juce::Rectangle<float> Slider::getThumbBounds() const {
    auto track_bounds = getTrackBounds();
    auto position = calculatePositionFromValue(value_);
    
    if (orientation_ == SliderOrientation::Horizontal) {
        return juce::Rectangle<float>(position.getX() - thumb_size_ * 0.5f,
                                     track_bounds.getCentreY() - thumb_size_ * 0.5f,
                                     thumb_size_,
                                     thumb_size_);
    } else {
        return juce::Rectangle<float>(track_bounds.getCentreX() - thumb_size_ * 0.5f,
                                     position.getY() - thumb_size_ * 0.5f,
                                     thumb_size_,
                                     thumb_size_);
    }
}

juce::Rectangle<float> Slider::getValueLabelBounds() const {
    auto thumb_bounds = getThumbBounds();
    juce::String value_text = formatValue(value_);
    
    // Measure text
    juce::Font font(12.0f);
    auto text_bounds = font.getStringBounds(value_text, juce::Justification::centred);
    
    // Add padding
    float padding = 8.0f;
    auto label_bounds = text_bounds.expand(padding);
    
    // Position above thumb
    if (orientation_ == SliderOrientation::Horizontal) {
        label_bounds.setPosition(thumb_bounds.getCentreX() - label_bounds.getWidth() * 0.5f,
                                thumb_bounds.getY() - label_bounds.getHeight() - 5.0f);
    } else {
        label_bounds.setPosition(thumb_bounds.getRight() + 5.0f,
                                thumb_bounds.getCentreY() - label_bounds.getHeight() * 0.5f);
    }
    
    return label_bounds;
}

juce::Rectangle<float> Slider::getTickMarkBounds(const TickMark& tick) const {
    auto track_bounds = getTrackBounds();
    float tick_position;
    
    if (orientation_ == SliderOrientation::Horizontal) {
        tick_position = track_bounds.getX() + (track_bounds.getWidth() * tick.position);
        return juce::Rectangle<float>(tick_position, track_bounds.getBottom(), 1.0f, 8.0f);
    } else {
        tick_position = track_bounds.getY() + (track_bounds.getHeight() * tick.position);
        return juce::Rectangle<float>(track_bounds.getRight(), tick_position, 8.0f, 1.0f);
    }
}

float Slider::calculateValueFromPosition(const juce::Point<float>& position) const {
    auto track_bounds = getTrackBounds();
    float position_ratio;
    
    if (orientation_ == SliderOrientation::Horizontal) {
        position_ratio = (position.getX() - track_bounds.getX()) / track_bounds.getWidth();
    } else {
        position_ratio = (track_bounds.getBottom() - position.getY()) / track_bounds.getHeight();
    }
    
    position_ratio = std::clamp(position_ratio, 0.0f, 1.0f);
    return positionToValue(position_ratio);
}

juce::Point<float> Slider::calculatePositionFromValue(float value) const {
    auto track_bounds = getTrackBounds();
    float position_ratio = valueToPosition(value);
    
    if (orientation_ == SliderOrientation::Horizontal) {
        return juce::Point<float>(track_bounds.getX() + (track_bounds.getWidth() * position_ratio),
                                 track_bounds.getCentreY());
    } else {
        return juce::Point<float>(track_bounds.getCentreX(),
                                 track_bounds.getBottom() - (track_bounds.getHeight() * position_ratio));
    }
}

void Slider::handleMouseDown(const juce::MouseEvent& e) {
    auto position = e.getPosition().toFloat();
    
    if (isPositionInThumb(position)) {
        // Click on thumb - start dragging
        state_.dragging = true;
        state_.drag_start_value = value_;
        state_.drag_start_position = e.getPosition();
        setState(core::Component::State::Pressed);
    } else if (isPositionInTrack(position)) {
        // Click on track - jump to position
        float new_value = calculateValueFromPosition(position);
        setValue(new_value);
        announceToScreenReader("Value: " + formatValue(value_));
    }
}

void Slider::handleMouseDrag(const juce::MouseEvent& e) {
    auto position = e.getPosition().toFloat();
    float new_value = calculateValueFromPosition(position);
    
    if (new_value != value_) {
        setValue(new_value);
        
        if (drag_callback_) {
            drag_callback_(value_, position);
        }
    }
}

void Slider::handleTouchStart(const juce::TouchEvent& e) {
    auto position = juce::Point<float>(e.x, e.y);
    
    if (isPositionInThumb(position) || isPositionInTrack(position)) {
        state_.dragging = true;
        state_.drag_start_value = value_;
        setState(core::Component::State::Pressed);
    }
}

void Slider::handleTouchMove(const juce::TouchEvent& e) {
    auto position = juce::Point<float>(e.x, e.y);
    float new_value = calculateValueFromPosition(position);
    
    if (new_value != value_) {
        setValue(new_value);
        
        if (drag_callback_) {
            drag_callback_(value_, position);
        }
    }
}

bool Slider::isPositionInThumb(const juce::Point<float>& position) const {
    auto thumb_bounds = getThumbBounds();
    return thumb_bounds.contains(position);
}

bool Slider::isPositionInTrack(const juce::Point<float>& position) const {
    auto track_bounds = getTrackBounds();
    float tolerance = track_thickness_ + 10.0f; // Add some tolerance
    
    if (orientation_ == SliderOrientation::Horizontal) {
        return position.getY() >= track_bounds.getY() - tolerance &&
               position.getY() <= track_bounds.getBottom() + tolerance &&
               position.getX() >= track_bounds.getX() &&
               position.getX() <= track_bounds.getRight();
    } else {
        return position.getX() >= track_bounds.getX() - tolerance &&
               position.getX() <= track_bounds.getRight() + tolerance &&
               position.getY() >= track_bounds.getY() &&
               position.getY() <= track_bounds.getBottom();
    }
}

juce::String Slider::formatValue(float value) const {
    if (value_formatter_) {
        return value_formatter_(value);
    }
    
    // Default formatting based on range
    float range_span = range_.maximum - range_.minimum;
    int precision = 0;
    
    if (range_span <= 10.0f) {
        precision = 2;
    } else if (range_span <= 100.0f) {
        precision = 1;
    } else {
        precision = 0;
    }
    
    precision = std::min(precision, value_label_precision_);
    
    return juce::String(value, precision);
}

float Slider::getTrackLength() const {
    auto track_bounds = getTrackBounds();
    return (orientation_ == SliderOrientation::Horizontal) ? 
           track_bounds.getWidth() : track_bounds.getHeight();
}

void Slider::updateThumbAnimation(float delta_time) {
    float target_scale = 1.0f;
    
    if (state_ == core::Component::State::Pressed) {
        target_scale = 0.9f;
    } else if (state_ == core::Component::State::Hover) {
        target_scale = 1.1f;
    }
    
    // Smooth transition
    float lerp_factor = std::min(delta_time * 10.0f, 1.0f);
    state_.active_thumb_scale = state_.active_thumb_scale + 
                               (target_scale - state_.active_thumb_scale) * lerp_factor;
}

void Slider::updateTrackProgress() {
    state_.track_progress = valueToPosition(value_);
}

} // namespace material
} // namespace ui
} // namespace vital
