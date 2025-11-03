#pragma once

#include "../core/component.h"
#include "../theme/theme_manager.h"
#include <functional>
#include <memory>

namespace vital {
namespace ui {
namespace material {

/**
 * @brief Material Design 3.0 rotary knob control
 * 
 * A circular, rotary control for adjusting numeric values with:
 * - Visual feedback through rotation
 * - Touch/mouse interaction with smooth dragging
 * - Optional value display
 * - Motorized control support
 * - Multiple rotation modes (circular, linear)
 */
class Knob : public Component {
public:
    enum class Style {
        Standard,       // Standard circular knob
        Linear,         // Vertical linear representation
        Virtual,        // Virtual analog knob
        Motorized       // Physical motorized knob
    };

    enum class RotationMode {
        Circular,       // 270° rotation range
        FullCircle,     // 360° rotation range
        Semicircle,     // 180° rotation range
        Custom          // Custom range
    };

    struct KnobStyle {
        Style style = Style::Standard;
        RotationMode rotationMode = RotationMode::Circular;
        float rotationRange = 270.0f; // degrees
        bool showValueLabel = true;
        bool enableMotor = false;
        Color accentColor = Colors::primary;
        float indicatorThickness = 4.0f;
        float tickMarkThickness = 2.0f;
        int tickMarkCount = 12;
        bool snapToTicks = false;
    };

    using ValueChangedCallback = std::function<void(float)>;
    using DoubleClickCallback = std::function<void()>;
    using MotorCallback = std::function<void(float)>;

    Knob(const std::string& label = "", const KnobStyle& style = {});
    ~Knob() override = default;

    // Component interface
    void paint(juce::Graphics& g) override;
    void resized() override;
    bool mouseDown(const juce::MouseEvent& e) override;
    bool mouseDrag(const juce::MouseEvent& e) override;
    bool mouseUp(const juce::MouseEvent& e) override;
    bool mouseDoubleClick(const juce::MouseEvent& e) override;
    
    // Value management
    void setValue(float value, bool sendCallback = true);
    float getValue() const { return currentValue_; }
    void setRange(float min, float max, float step = 0.0f);
    
    // Style configuration
    void setStyle(const KnobStyle& style);
    const KnobStyle& getStyle() const { return style_; }
    
    // Visual properties
    void setLabel(const std::string& label);
    void setValueFormat(const std::string& format); // printf-style format
    void setDisplayPrecision(int precision);
    
    // Motor support
    void enableMotorizedControl(bool enable);
    void setMotorPosition(float position);
    void setMotorCallback(MotorCallback callback);
    
    // Tick marks
    void setTickMarkCount(int count);
    void setTickMarkStyle(const std::vector<float>& tickValues);
    
    // Callbacks
    void setValueChangedCallback(ValueChangedCallback callback);
    void setDoubleClickCallback(DoubleClickCallback callback);
    
    // Accessibility
    juce::String getAccessibilityLabel() const override;
    juce::String getAccessibilityValue() const override;

private:
    float minValue_ = 0.0f;
    float maxValue_ = 1.0f;
    float stepValue_ = 0.0f;
    float currentValue_ = 0.5f;
    float displayValue_ = 0.5f;
    
    bool isDragging_ = false;
    juce::Point<float> dragStart_;
    float startValue_;
    
    KnobStyle style_;
    std::string label_;
    std::string valueFormat_ = "%.3f";
    int displayPrecision_ = 3;
    
    // Motor state
    bool motorEnabled_ = false;
    float motorTargetPosition_ = 0.0f;
    MotorCallback motorCallback_;
    
    // Animation
    AnimationValue displayAnimation_;
    
    // Internal helpers
    float valueToAngle(float value) const;
    float angleToValue(float angle) const;
    void startDrag(const juce::MouseEvent& e);
    void updateDrag(const juce::MouseEvent& e);
    void endDrag();
    void updateDisplayValue();
    void sendValueChanged();
    juce::Rectangle<float> getKnobBounds() const;
    juce::Point<float> getKnobCenter() const;
    float getKnobRadius() const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_CHECK(Knob)
};

} // namespace material
} // namespace ui
} // namespace vital