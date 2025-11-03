#include "knob.h"
#include <cmath>
#include <algorithm>
#include "../accessibility/accessibility_manager.h"
#include "../core/animation_engine.h"

namespace vital {
namespace ui {
namespace material {

Knob::Knob(const std::string& label, const KnobStyle& style)
    : Component(), label_(label), style_(style) {
    setWantsKeyboardFocus(true);
    setInterceptsMouseClicks(true, true);
    
    // Set ARIA role
    setAriaRole("slider");
    setAriaDescription(label_);
    setAriaLabel(label_);
    
    // Initialize display value animation
    displayAnimation_.reset(0.0f);
    displayAnimation_.setDuration(150.0f); // 150ms animation
    displayAnimation_.setEasing(AnimationEasing::easeOutCubic);
    
    // Default size
    setSize(80, 100); // Leave space for label
    
    // Initialize motor state
    motorTargetPosition_ = currentValue_;
}

void Knob::paint(juce::Graphics& g) {
    Component::paint(g);
    
    auto bounds = getLocalBounds();
    auto knob_bounds = getKnobBounds();
    auto center = getKnobCenter();
    auto radius = getKnobRadius();
    
    // Get theme colors
    auto theme = getTheme();
    auto colors = theme->getMaterialColors();
    
    // Calculate current angle based on value
    float angle = valueToAngle(displayValue_);
    float normalized_value = (currentValue_ - minValue_) / (maxValue_ - minValue_);
    
    // Draw background circle with elevation
    if (style_.style == Style::Standard || style_.style == Style::Virtual) {
        // Elevation shadow
        float elevation = theme->getElevation(elevation_);
        auto shadow_color = juce::Colours::black.withAlpha(elevation * 0.2f);
        auto shadow_offset = juce::Point<float>(elevation * 0.1f, elevation * 0.1f);
        
        g.setColour(shadow_color);
        g.fillEllipse(knob_bounds.translated(shadow_offset.getX(), shadow_offset.getY()));
        
        // Main knob background
        auto knob_color = enabled_ ? colors.surface : colors.surface.withAlpha(0.6f);
        if (state_ == State::Hover) {
            knob_color = knob_color.brighter(0.05f);
        } else if (state_ == State::Pressed) {
            knob_color = knob_color.darker(0.05f);
        }
        
        g.setColour(knob_color);
        g.fillEllipse(knob_bounds);
        
        // Border
        g.setColour(colors.outline.withAlpha(enabled_ ? 0.5f : 0.3f));
        g.drawEllipse(knob_bounds, 1.0f);
    }
    
    // Draw tick marks
    if (style_.tickMarkCount > 0) {
        g.saveState();
        g.addTransform(juce::AffineTransform::rotation(angleToAngle(angle, style_.rotationMode), 
                                                       center.getX(), center.getY()));
        
        auto tick_color = colors.onSurface.withAlpha(0.3f);
        g.setColour(tick_color);
        
        float start_angle = -juce::MathConstants<float>::pi / 2.0f - 
                           juce::MathConstants<float>::pi * 0.35f; // Start from -225°
        float angle_step = (juce::MathConstants<float>::pi * 1.5f) / (style_.tickMarkCount - 1);
        
        float tick_length = radius * 0.15f;
        float tick_radius = radius * 0.85f;
        
        for (int i = 0; i < style_.tickMarkCount; ++i) {
            float current_angle = start_angle + (i * angle_step);
            float x1 = center.getX() + std::cos(current_angle) * tick_radius;
            float y1 = center.getY() + std::sin(current_angle) * tick_radius;
            float x2 = center.getX() + std::cos(current_angle) * (tick_radius - tick_length);
            float y2 = center.getY() + std::sin(current_angle) * (tick_radius - tick_length);
            
            float thickness = (i % 4 == 0) ? 2.0f : 1.0f; // Major ticks every 4th position
            g.drawLine(x1, y1, x2, y2, thickness);
        }
        
        g.restoreState();
    }
    
    // Draw value indicator
    g.saveState();
    g.addTransform(juce::AffineTransform::rotation(angle, center.getX(), center.getY()));
    
    auto indicator_color = style_.accentColor.getColor(colors);
    float indicator_radius = radius * 0.85f;
    float indicator_start = radius * 0.2f;
    
    g.setColour(indicator_color);
    g.drawLine(center.getX() + indicator_start, center.getY(),
               center.getX() + indicator_radius, center.getY(),
               style_.indicatorThickness);
    
    // Draw indicator knob
    g.setColour(indicator_color);
    g.fillEllipse(center.getX() + indicator_radius - 3.0f, center.getY() - 3.0f, 6.0f, 6.0f);
    
    g.restoreState();
    
    // Draw value label
    if (style_.showValueLabel) {
        auto label_bounds = juce::Rectangle<float>(bounds.getX(), knob_bounds.getBottom() + 10.0f,
                                                  bounds.getWidth(), 20.0f);
        
        g.setColour(colors.onSurface);
        g.setFont(juce::Font(12.0f));
        
        juce::String value_text = formatValueString(displayValue_);
        juce::Rectangle<float> text_bounds = g.getCurrentFont().getStringBounds(value_text, 
                                                                               juce::Justification::centred);
        
        g.drawText(value_text, label_bounds, juce::Justification::centred);
    }
    
    // Draw label
    if (!label_.empty()) {
        auto label_bounds = juce::Rectangle<float>(bounds.getX(), knob_bounds.getBottom() + 30.0f,
                                                  bounds.getWidth(), 16.0f);
        
        g.setColour(colors.onSurface.withAlpha(0.7f));
        g.setFont(juce::Font(11.0f));
        g.drawText(label_, label_bounds, juce::Justification::centred);
    }
    
    // Draw focus ring if focused
    if (state_ == State::Focused && enabled_) {
        auto focus_bounds = knob_bounds.reduced(3.0f);
        g.setColour(colors.primary);
        g.drawRoundedRectangle(focus_bounds, knob_bounds.getWidth() * 0.5f, 2.0f);
    }
    
    // Draw disabled overlay
    if (!enabled_) {
        g.setColour(juce::Colours::black.withAlpha(0.3f));
        g.fillEllipse(knob_bounds);
    }
}

void Knob::resized() {
    Component::resized();
    
    // Recalculate layout for new size
    auto bounds = getLocalBounds();
    
    // Leave space for label at bottom
    if (style_.showValueLabel || !label_.empty()) {
        auto knob_area = bounds;
        knob_area.removeFromBottom(style_.showValueLabel ? 35 : 20);
        setKnobBounds(knob_area.reduced(5));
    } else {
        setKnobBounds(bounds.reduced(5));
    }
}

bool Knob::mouseDown(const juce::MouseEvent& e) {
    if (!enabled_) return false;
    
    auto position = e.getPosition().toFloat();
    
    // Check if click is within knob bounds
    auto knob_bounds = getKnobBounds();
    if (knob_bounds.contains(position)) {
        startDrag(e);
        
        // Announce to screen reader
        announceToScreenReader(juce::String("Knob ") + label_ + 
                              ". Value: " + formatValueString(currentValue_));
        
        return true;
    }
    
    return false;
}

bool Knob::mouseDrag(const juce::MouseEvent& e) {
    if (!enabled_ || !isDragging_) return false;
    
    updateDrag(e);
    return true;
}

bool Knob::mouseUp(const juce::MouseEvent& e) {
    if (!enabled_) return false;
    
    if (isDragging_) {
        endDrag();
        return true;
    }
    
    return false;
}

bool Knob::mouseDoubleClick(const juce::MouseEvent& e) {
    if (!enabled_) return false;
    
    if (double_click_callback_) {
        double_click_callback_();
        return true;
    }
    
    return false;
}

void Knob::setValue(float value, bool sendCallback) {
    // Clamp value to range
    value = std::clamp(value, minValue_, maxValue_);
    
    // Apply step snapping if enabled
    if (stepValue_ > 0.0f && style_.snapToTicks) {
        float steps = std::round((value - minValue_) / stepValue_);
        value = minValue_ + (steps * stepValue_);
    }
    
    if (std::abs(value - currentValue_) > 0.0001f) {
        float old_value = currentValue_;
        currentValue_ = value;
        
        // Start animation for display value
        displayAnimation_.animateTo(currentValue_);
        
        if (sendCallback && value_changed_callback_) {
            value_changed_callback_(currentValue_);
        }
        
        // Update motor position
        if (motorEnabled_ && motorCallback_) {
            motorCallback_(value);
        }
        
        setNeedsRedraw();
        
        // Announce to screen reader
        announceToScreenReader(juce::String("Knob ") + label_ + 
                              ". Value changed to: " + formatValueString(currentValue_));
    }
}

void Knob::setRange(float min, float max, float step) {
    minValue_ = min;
    maxValue_ = max;
    stepValue_ = step;
    
    // Ensure current value is within new range
    currentValue_ = std::clamp(currentValue_, minValue_, maxValue_);
    
    setNeedsRedraw();
}

void Knob::setStyle(const KnobStyle& style) {
    style_ = style;
    setNeedsRedraw();
}

void Knob::setLabel(const std::string& label) {
    label_ = label;
    setAriaLabel(label);
    setAriaDescription(label);
    setNeedsRedraw();
}

void Knob::setValueFormat(const std::string& format) {
    valueFormat_ = format;
    setNeedsRedraw();
}

void Knob::setDisplayPrecision(int precision) {
    displayPrecision_ = std::max(0, precision);
    setNeedsRedraw();
}

void Knob::enableMotorizedControl(bool enable) {
    motorEnabled_ = enable;
    if (!enable) {
        motorTargetPosition_ = currentValue_;
    }
}

void Knob::setMotorPosition(float position) {
    motorTargetPosition_ = std::clamp(position, minValue_, maxValue_);
    setNeedsRedraw();
}

void Knob::setMotorCallback(MotorCallback callback) {
    motorCallback_ = callback;
}

void Knob::setTickMarkCount(int count) {
    style_.tickMarkCount = std::max(0, count);
    setNeedsRedraw();
}

void Knob::setTickMarkStyle(const std::vector<float>& tickValues) {
    // Implementation for custom tick marks
    // For now, just update count based on values
    style_.tickMarkCount = static_cast<int>(tickValues.size());
    setNeedsRedraw();
}

void Knob::setValueChangedCallback(ValueChangedCallback callback) {
    value_changed_callback_ = callback;
}

void Knob::setDoubleClickCallback(DoubleClickCallback callback) {
    double_click_callback_ = callback;
}

juce::String Knob::getAccessibilityLabel() const {
    return juce::String(label_);
}

juce::String Knob::getAccessibilityValue() const {
    return formatValueString(currentValue_);
}

void Knob::startDrag(const juce::MouseEvent& e) {
    isDragging_ = true;
    dragStart_ = e.getPosition().toFloat();
    startValue_ = currentValue_;
    
    setState(State::Pressed);
}

void Knob::updateDrag(const juce::MouseEvent& e) {
    auto current_position = e.getPosition().toFloat();
    auto delta = current_position - dragStart_;
    
    float sensitivity = 0.005f; // Adjust sensitivity as needed
    float value_change = -delta.getY() * sensitivity * (maxValue_ - minValue_);
    float new_value = startValue_ + value_change;
    
    setValue(new_value);
}

void Knob::endDrag() {
    isDragging_ = false;
    setState(State::Normal);
}

void Knob::updateDisplayValue() {
    displayValue_ = displayAnimation_.getCurrentValue();
}

float Knob::valueToAngle(float value) const {
    float normalized = (value - minValue_) / (maxValue_ - minValue_);
    normalized = std::clamp(normalized, 0.0f, 1.0f);
    
    // Map to rotation range
    float start_angle = -juce::MathConstants<float>::pi * 0.75f; // -135°
    float end_angle = juce::MathConstants<float>::pi * 0.75f;    // 135°
    
    switch (style_.rotationMode) {
        case RotationMode::Circular:
            start_angle = -juce::MathConstants<float>::pi * 0.875f; // -157.5°
            end_angle = juce::MathConstants<float>::pi * 0.875f;     // 157.5°
            break;
        case RotationMode::FullCircle:
            start_angle = -juce::MathConstants<float>::pi;           // -180°
            end_angle = juce::MathConstants<float>::pi;              // 180°
            break;
        case RotationMode::Semicircle:
            start_angle = -juce::MathConstants<float>::pi * 0.5f;    // -90°
            end_angle = juce::MathConstants<float>::pi * 0.5f;       // 90°
            break;
        case RotationMode::Custom:
            // Use style_.rotationRange
            float range_radians = juce::MathConstants<float>::pi * style_.rotationRange / 180.0f;
            start_angle = -range_radians * 0.5f;
            end_angle = range_radians * 0.5f;
            break;
    }
    
    return start_angle + (normalized * (end_angle - start_angle));
}

float Knob::angleToValue(float angle) const {
    // Normalize angle to [0, 1] range based on rotation mode
    float start_angle, end_angle;
    
    switch (style_.rotationMode) {
        case RotationMode::Circular:
            start_angle = -juce::MathConstants<float>::pi * 0.875f;
            end_angle = juce::MathConstants<float>::pi * 0.875f;
            break;
        case RotationMode::FullCircle:
            start_angle = -juce::MathConstants<float>::pi;
            end_angle = juce::MathConstants<float>::pi;
            break;
        case RotationMode::Semicircle:
            start_angle = -juce::MathConstants<float>::pi * 0.5f;
            end_angle = juce::MathConstants<float>::pi * 0.5f;
            break;
        case RotationMode::Custom:
            float range_radians = juce::MathConstants<float>::pi * style_.rotationRange / 180.0f;
            start_angle = -range_radians * 0.5f;
            end_angle = range_radians * 0.5f;
            break;
        default:
            start_angle = -juce::MathConstants<float>::pi * 0.875f;
            end_angle = juce::MathConstants<float>::pi * 0.875f;
            break;
    }
    
    // Clamp angle to valid range
    angle = std::clamp(angle, start_angle, end_angle);
    
    // Convert to normalized value
    float normalized = (angle - start_angle) / (end_angle - start_angle);
    return minValue_ + (normalized * (maxValue_ - minValue_));
}

float Knob::angleToAngle(float angle, RotationMode mode) const {
    // Convert any angle to the standard coordinate system
    return angle;
}

juce::String Knob::formatValueString(float value) const {
    char buffer[64];
    std::snprintf(buffer, sizeof(buffer), valueFormat_.c_str(), value);
    return juce::String(buffer);
}

juce::Rectangle<float> Knob::getKnobBounds() const {
    auto bounds = getLocalBounds();
    auto center = getKnobCenter();
    float radius = getKnobRadius();
    
    return juce::Rectangle<float>(center.getX() - radius, center.getY() - radius,
                                 radius * 2.0f, radius * 2.0f);
}

juce::Point<float> Knob::getKnobCenter() const {
    auto bounds = getLocalBounds();
    return juce::Point<float>(bounds.getCentreX(), bounds.getCentreY() - 15.0f);
}

float Knob::getKnobRadius() const {
    auto bounds = getLocalBounds();
    return std::min(bounds.getWidth(), bounds.getHeight() * 0.7f) * 0.35f;
}

void Knob::setKnobBounds(const juce::Rectangle<float>& bounds) {
    knob_bounds_ = bounds;
}

void Knob::update() {
    Component::update();
    
    // Update display value animation
    displayAnimation_.update(0.016f); // Assume 60 FPS
    updateDisplayValue();
    
    // Update motor position if enabled
    if (motorEnabled_ && std::abs(motorTargetPosition_ - currentValue_) > 0.001f) {
        float lerp_factor = 0.1f; // Smooth motor movement
        currentValue_ += (motorTargetPosition_ - currentValue_) * lerp_factor;
        displayValue_ = currentValue_;
        setNeedsRedraw();
    }
}

juce::String Knob::announceToScreenReader(const juce::String& message) {
    auto accessibility_manager = getAccessibilityManager();
    if (accessibility_manager && accessibility_manager->isFeatureEnabled(
        accessibility::AccessibilityFeature::ScreenReader)) {
        accessibility_manager->announceToScreenReader(message);
    }
    return message;
}

} // namespace material
} // namespace ui
} // namespace vital