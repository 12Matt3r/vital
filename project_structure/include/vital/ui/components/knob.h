#pragma once

#include <functional>
#include <string>
#include "button.h"

namespace vital {

class Knob : public Control {
public:
    enum class KnobStyle {
        Analog,
        Digital,
        Minimal,
        Round
    };

    struct Parameters {
        float min_value;
        float max_value;
        float default_value;
        float value;
        KnobStyle style;
        bool show_value;
        bool logarithmic;
        int precision;
    };

    Knob();
    ~Knob();

    void setParameters(const Parameters& params);
    void setValue(float value) override;
    void setRange(float min, float max);
    void setStyle(KnobStyle style);
    void setShowValue(bool show);
    void setLogarithmic(bool logarithmic);
    void setPrecision(int precision);

    float getValue() const override { return params_.value; }
    float getMinValue() const { return params_.min_value; }
    float getMaxValue() const { return params_.max_value; }
    float getNormalizedValue() const;
    
    void setNormalizedValue(float normalized_value);

    // Control overrides
    void onMouseDown(int x, int y) override;
    void onMouseDrag(int x, int y) override;
    void onMouseUp(int x, int y) override;
    void onMouseWheel(int delta) override;
    
    void draw() override;
    void setBounds(float x, float y, float width, float height) override;
    
    // Parameter display
    std::string getValueString() const;
    std::string getLabel() const { return label_; }
    void setLabel(const std::string& label) { label_ = label; }

private:
    float calculateValueFromAngle(float angle);
    float calculateAngleFromValue(float value);
    void updateVisualValue();
    
    float calculateKnobAngle(float mouse_x, float mouse_y);
    bool isPointInKnob(float x, float y) const;
    
    Parameters params_;
    std::string label_;
    
    // Interaction state
    bool dragging_;
    float last_mouse_y_;
    float drag_start_value_;
    float start_angle_;
    
    // Visual state
    float current_angle_;
    bool hover_;
    
    // Style
    KnobStyle style_;
    float border_width_;
    
    // Animation
    float target_value_;
    float animation_progress_;
    bool animating_;
};

} // namespace vital