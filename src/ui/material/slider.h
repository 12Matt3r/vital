#pragma once

#include <functional>
#include <JuceHeader.h>
#include "../core/component.h"

namespace vital {
namespace ui {
namespace material {

/**
 * @brief Slider style variants
 */
enum class SliderStyle {
    Linear,           // Horizontal/vertical linear slider
    Circular,         // Circular/semi-circular slider
    Discrete,         // Discrete steps with snap
    TickMark,         // Slider with tick marks
    FloatingLabel     // Slider with floating value label
};

/**
 * @brief Slider orientation
 */
enum class SliderOrientation {
    Horizontal,
    Vertical
};

/**
 * @brief Slider state information
 */
struct SliderState {
    bool hovering = false;
    bool dragging = false;
    bool focused = false;
    float drag_start_value = 0.0f;
    juce::Point<float> drag_start_position;
    juce::Point<float> current_position;
    float active_thumb_scale = 1.0f;
    float track_progress = 0.0f;
    int active_tick_index = -1;
};

/**
 * @brief Tick mark configuration
 */
struct TickMark {
    float position;      // Position (0.0 to 1.0)
    juce::String label;  // Optional label
    juce::Colour color;  // Tick color
    bool major = false;  // Major or minor tick
};

/**
 * @brief Material Design 3.0 Slider component
 * 
 * Implements sliders following Material Design 3.0 guidelines
 * with support for:
 * - Multiple styles (linear, circular, discrete)
 * - Tick marks and value display
 * - Accessibility features
 * - Touch and mouse interaction
 * - Real-time value feedback
 */
class VITAL_MODERN_UI_API Slider : public core::Component {
public:
    /**
     * @brief Value change callback
     */
    using ValueChangedCallback = std::function<void(float)>;
    using ValueChangeStartCallback = std::function<void(float)>;
    using ValueChangeEndCallback = std::function<void(float)>;
    using SliderDragCallback = std::function<void(float, const juce::Point<float>&)>;

    /**
     * @brief Constructor
     * @param style Slider style
     * @param orientation Slider orientation
     */
    Slider(SliderStyle style = SliderStyle::Linear,
           SliderOrientation orientation = SliderOrientation::Horizontal);

    /**
     * @brief Destructor
     */
    ~Slider() override = default;

    //==============================================================================
    // Value Management
    void setValue(float value, bool notify = true);
    float getValue() const { return value_; }

    void setRange(float min_value, float max_value, float step_value = 0.0f);
    
    /**
     * @brief Set value range
     */
    struct ValueRange {
        float minimum = 0.0f;
        float maximum = 100.0f;
        float step = 0.0f;  // 0.0 for continuous

        ValueRange() = default;
        ValueRange(float min, float max, float step_val = 0.0f)
            : minimum(min), maximum(max), step(step_val) {}
    };

    void setRange(const ValueRange& range);
    const ValueRange& getRange() const { return range_; }

    /**
     * @brief Get minimum value
     */
    float getMinimum() const { return range_.minimum; }

    /**
     * @brief Get maximum value
     */
    float getMaximum() const { return range_.maximum; }

    /**
     * @brief Get step size
     */
    float getStep() const { return range_.step; }

    /**
     * @brief Snap value to nearest step
     */
    float snapToStep(float value) const;

    /**
     * @brief Check if value is within range
     */
    bool isValueInRange(float value) const;

    /**
     * @brief Convert value to normalized position (0.0 to 1.0)
     */
    float valueToPosition(float value) const;

    /**
     * @brief Convert position to value
     */
    float positionToValue(float position) const;

    //==============================================================================
    // Appearance
    void setStyle(SliderStyle style);
    SliderStyle getStyle() const { return style_; }

    void setOrientation(SliderOrientation orientation);
    SliderOrientation getOrientation() const { return orientation_; }

    void setEnabled(bool enabled) override;

    /**
     * @brief Set slider color
     */
    void setColor(const juce::Colour& color);

    /**
     * @brief Get slider color
     */
    juce::Colour getColor() const { return slider_color_; }

    /**
     * @brief Set track color
     */
    void setTrackColor(const juce::Colour& color);

    /**
     * @brief Get track color
     */
    juce::Colour getTrackColor() const { return track_color_; }

    /**
     * @brief Set thumb color
     */
    void setThumbColor(const juce::Colour& color);

    /**
     * @brief Get thumb color
     */
    juce::Colour getThumbColor() const { return thumb_color_; }

    /**
     * @brief Set value label visibility
     */
    void setValueLabelVisible(bool visible);

    /**
     * @brief Check if value label is visible
     */
    bool isValueLabelVisible() const { return show_value_label_; }

    /**
     * @brief Set value label precision
     */
    void setValueLabelPrecision(int precision);

    /**
     * @brief Get value label precision
     */
    int getValueLabelPrecision() const { return value_label_precision_; }

    /**
     * @brief Set custom value formatter
     */
    void setValueFormatter(std::function<juce::String(float)> formatter);

    //==============================================================================
    // Tick Marks (for discrete sliders)
    /**
     * @brief Add tick mark
     * @param position Position (0.0 to 1.0)
     * @param label Optional label
     * @param major Whether this is a major tick
     */
    void addTickMark(float position, const juce::String& label = "", bool major = false);

    /**
     * @brief Remove tick mark at position
     */
    void removeTickMark(float position);

    /**
     * @brief Remove all tick marks
     */
    void clearTickMarks();

    /**
     * @brief Set tick marks from value positions
     */
    void setTickMarksFromValues(const std::vector<float>& positions);

    /**
     * @brief Get tick marks
     */
    const std::vector<TickMark>& getTickMarks() const { return tick_marks_; }

    /**
     * @brief Show tick marks
     */
    void setTickMarksVisible(bool visible);

    /**
     * @brief Check if tick marks are visible
     */
    bool areTickMarksVisible() const { return show_tick_marks_; }

    //==============================================================================
    // Discrete Slider Behavior
    /**
     * @brief Enable discrete mode
     */
    void setDiscreteMode(bool discrete);

    /**
     * @brief Check if in discrete mode
     */
    bool isDiscreteMode() const { return discrete_mode_; }

    /**
     * @brief Enable tick mark snapping
     */
    void setTickMarkSnapping(bool snapping);

    /**
     * @brief Check if tick mark snapping is enabled
     */
    bool isTickMarkSnapping() const { return tick_mark_snapping_; }

    //==============================================================================
    // Size and Dimensions
    void setHeight(float height);
    void setWidth(float width) override;

    /**
     * @brief Set track thickness
     */
    void setTrackThickness(float thickness);

    /**
     * @brief Get track thickness
     */
    float getTrackThickness() const { return track_thickness_; }

    /**
     * @brief Set thumb size
     */
    void setThumbSize(float size);

    /**
     * @brief Get thumb size
     */
    float getThumbSize() const { return thumb_size_; }

    /**
     * @ Set minimum slider length
     */
    void setMinimumSliderLength(float length);

    /**
     * @brief Get minimum slider length
     */
    float getMinimumSliderLength() const { return min_slider_length_; }

    //==============================================================================
    // Callbacks
    void setValueChangedCallback(ValueChangedCallback callback);
    void setValueChangeStartCallback(ValueChangeStartCallback callback);
    void setValueChangeEndCallback(ValueChangeEndCallback callback);
    void setDragCallback(SliderDragCallback callback);

    /**
     * @brief Clear all callbacks
     */
    void clearCallbacks();

    //==============================================================================
    // Accessibility
    void setAriaRole(const juce::String& role) override;
    
    /**
     * @brief Set slider description for screen readers
     */
    void setDescription(const juce::String& description);

    /**
     * @brief Get slider description
     */
    juce::String getDescription() const { return description_; }

    /**
     * @brief Set keyboard increments
     */
    void setKeyboardIncrements(float small_increment, float large_increment);

    /**
     * @brief Get keyboard increments
     */
    std::pair<float, float> getKeyboardIncrements() const { 
        return {small_increment_, large_increment_}; 
    }

    /**
     * @brief Set keyboard shortcuts
     */
    void setKeyboardShortcuts(const juce::String& increment_key,
                             const juce::String& decrement_key);

    //==============================================================================
    // Interaction
    bool keyPressed(const juce::KeyPress& key) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
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
    float value_ = 0.0f;
    ValueRange range_{0.0f, 100.0f, 0.0f};

    SliderStyle style_ = SliderStyle::Linear;
    SliderOrientation orientation_ = SliderOrientation::Horizontal;

    // Colors
    juce::Colour slider_color_;
    juce::Colour track_color_;
    juce::Colour thumb_color_;
    juce::Colour tick_mark_color_;

    // State
    SliderState state_;

    // Dimensions
    float track_thickness_ = 4.0f;
    float thumb_size_ = 20.0f;
    float min_slider_length_ = 100.0f;

    // Tick marks
    std::vector<TickMark> tick_marks_;
    bool show_tick_marks_ = false;
    bool discrete_mode_ = false;
    bool tick_mark_snapping_ = false;

    // Value label
    bool show_value_label_ = false;
    int value_label_precision_ = 2;
    std::function<juce::String(float)> value_formatter_;

    // Keyboard interaction
    float small_increment_ = 1.0f;
    float large_increment_ = 10.0f;
    juce::String increment_key_;
    juce::String decrement_key_;

    // Accessibility
    juce::String description_;

    // Callbacks
    ValueChangedCallback value_changed_callback_;
    ValueChangeStartCallback value_change_start_callback_;
    ValueChangeEndCallback value_change_end_callback_;
    SliderDragCallback drag_callback_;

    //==============================================================================
    // Private methods
    juce::Rectangle<float> getTrackBounds() const;
    juce::Rectangle<float> getThumbBounds() const;
    juce::Rectangle<float> getValueLabelBounds() const;
    juce::Rectangle<float> getTickMarkBounds(const TickMark& tick) const;

    // Rendering
    void renderLinearSlider(juce::Graphics& g);
    void renderCircularSlider(juce::Graphics& g);
    void renderDiscreteSlider(juce::Graphics& g);
    void renderTrack(juce::Graphics& g);
    void renderThumb(juce::Graphics& g);
    void renderTickMarks(juce::Graphics& g);
    void renderValueLabel(juce::Graphics& g);
    void renderFocusRing(juce::Graphics& g);
    void renderActiveArea(juce::Graphics& g);

    // Value calculation
    float calculateValueFromPosition(const juce::Point<float>& position) const;
    juce::Point<float> calculatePositionFromValue(float value) const;
    int findNearestTickMark(float position) const;

    // Interaction handling
    void handleMouseDown(const juce::MouseEvent& e);
    void handleMouseDrag(const juce::MouseEvent& e);
    void handleTouchStart(const juce::TouchEvent& e);
    void handleTouchMove(const juce::TouchEvent& e);
    void handleKeyPress(const juce::KeyPress& key);

    // State management
    void updateDragState(const juce::Point<float>& position);
    void updateHoverState(const juce::Point<float>& position);
    void updateFocusState(bool focused);

    // Animation
    void updateThumbAnimation(float delta_time);
    void updateTrackProgress();

    // Utility
    bool isPositionInThumb(const juce::Point<float>& position) const;
    bool isPositionInTrack(const juce::Point<float>& position) const;
    juce::String formatValue(float value) const;
    float getTrackLength() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Slider)
};

} // namespace material
} // namespace ui
} // namespace vital
