#pragma once

#include "../core/component.h"
#include <memory>
#include <vector>
#include <functional>

namespace vital {
namespace ui {
namespace visualizations {

/**
 * @brief Digital level meter display
 * 
 * Shows audio level with:
 * - Peak and RMS indicators
 * - Multiple channel support
 * - Peak hold functionality
 * - Overload indication
 * - Bar graph and numeric display
 */
class LevelMeter : public Component {
public:
    enum class MeterType {
        Bar,            // Vertical bar meter
        Horizontal,     // Horizontal bar
        Numeric,        // Numeric only
        Combined        // Bar + numeric
    };

    enum class MeterMode {
        Peak,           // Peak level only
        RMS,            // RMS level only
        PeakHold,       // Peak with hold
        PeakRMS         // Both peak and RMS
    };

    struct MeterChannel {
        std::string name;
        float peakLevel = -60.0f;     // Current peak level in dB
        float rmsLevel = -60.0f;      // Current RMS level in dB
        float peakHold = -60.0f;      // Peak hold level in dB
        float peakHoldTime = 0.0f;    // Time since peak hold
        bool overload = false;        // Overload flag
        Color color = Colors::primary;
        bool visible = true;
    };

    struct LevelMeterSettings {
        MeterType type = MeterType::Bar;
        MeterMode mode = MeterMode::PeakRMS;
        int channels = 1;
        float minLevel = -60.0f;      // Minimum level in dB
        float maxLevel = 0.0f;        // Maximum level in dB
        float peakHoldTime = 1.5f;    // Peak hold duration in seconds
        float falloffTime = 2.0f;     // Falloff time in seconds
        bool showOverload = true;
        bool showPeakHold = true;
        bool showLabels = true;
        bool showTickMarks = true;
        float tickMarkInterval = 6.0f; // dB intervals
        Color backgroundColor = Colors::surface;
        Color barColor = Colors::primary;
        Color rmsColor = Colors::secondary;
        Color peakHoldColor = Colors::error;
        Color overloadColor = Colors::error;
        Color textColor = Colors::onSurface;
        float opacity = 1.0f;
        float cornerRadius = 4.0f;
        bool smoothAnimation = true;
        float animationSpeed = 10.0f; // Interpolation speed
    };

    using LevelCallback = std::function<void(float peak, float rms, int channel)>;

    LevelMeter(const LevelMeterSettings& settings = {});
    ~LevelMeter() override = default;

    // Component interface
    void paint(juce::Graphics& g) override;
    void resized() override;
    void timerCallback() override;
    
    // Settings
    void setSettings(const LevelMeterSettings& settings);
    LevelMeterSettings getSettings() const { return settings_; }
    
    // Channel management
    void setChannelCount(int count);
    void addChannel(const MeterChannel& channel = {});
    void removeChannel(int index);
    void clearChannels();
    void setChannelName(int index, const std::string& name);
    void setChannelVisible(int index, bool visible);
    void setChannelColor(int index, const Color& color);
    
    // Level input
    void setPeakLevel(float level, int channel = 0);
    void setRMSLevel(float level, int channel = 0);
    void setLevels(float peak, float rms, int channel = 0);
    void setLevels(const std::vector<float>& peakLevels, const std::vector<float>& rmsLevels);
    
    // Peak hold control
    void resetPeakHold(int channel = -1);
    void setPeakHoldEnabled(bool enabled);
    bool isPeakHoldEnabled() const { return settings_.showPeakHold; }
    
    // Level callbacks
    void setLevelCallback(LevelCallback callback);
    
    // External control
    void setExternalLevel(float level, int channel = 0);
    void startMeter();
    void stopMeter();
    void resetMeter();
    
    // Accessibility
    juce::String getAccessibilityLabel() const override;
    juce::String getAccessibilityValue() const override;

private:
    LevelMeterSettings settings_;
    std::vector<MeterChannel> channels_;
    
    // Animation state
    AnimationValue displayPeakLevel_;
    AnimationValue displayRMSLevel_;
    AnimationValue displayPeakHold_;
    
    // Timer
    juce::Timer updateTimer_;
    bool isRunning_ = false;
    
    // Callbacks
    LevelCallback levelCallback_;
    
    // Internal helpers
    void setupChannels();
    void updateLevels();
    void processPeakHold();
    juce::Rectangle<float> getChannelBounds(int index) const;
    juce::Rectangle<float> getBarBounds(int index) const;
    juce::Rectangle<float> getLabelBounds(int index) const;
    void drawBarMeter(juce::Graphics& g, const MeterChannel& channel, const juce::Rectangle<float>& bounds);
    void drawHorizontalMeter(juce::Graphics& g, const MeterChannel& channel, const juce::Rectangle<float>& bounds);
    void drawNumericDisplay(juce::Graphics& g, const MeterChannel& channel, const juce::Rectangle<float>& bounds);
    void drawCombinedDisplay(juce::Graphics& g, const MeterChannel& channel, const juce::Rectangle<float>& bounds);
    void drawTickMarks(juce::Graphics& g, const juce::Rectangle<float>& bounds);
    void drawLabels(juce::Graphics& g, const MeterChannel& channel, const juce::Rectangle<float>& bounds);
    float levelToNormalized(float level) const;
    float normalizedToLevel(float normalized) const;
    float getLevelFromNormalized(float normalized, int channel) const;
    Color getLevelColor(float level) const;
    bool isOverloaded(float level) const;
    juce::String formatLevel(float level) const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_CHECK(LevelMeter)
};

} // namespace visualizations
} // namespace ui
} // namespace vital