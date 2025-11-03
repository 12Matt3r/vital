#pragma once

#include "../core/component.h"
#include <memory>
#include <vector>
#include <thread>
#include <atomic>
#include <functional>

namespace vital {
namespace ui {
namespace visualizations {

/**
 * @brief Real-time oscilloscope visualization
 * 
 * Displays time-domain waveform data with:
 * - Multiple channel support
 * - Trigger modes (auto, normal, single)
 * - Timebase controls
 * - Vertical sensitivity controls
 * - Zoom and pan functionality
 * - Grid and measurement tools
 */
class Oscilloscope : public Component {
public:
    enum class TriggerMode {
        Auto,           // Automatic trigger
        Normal,         // Normal trigger
        Single,         // Single shot
        None            // No trigger
    };

    enum class ChannelMode {
        DC,             // DC coupling
        AC,             // AC coupling
        Ground          // Ground reference
    };

    struct Channel {
        bool enabled = true;
        Color color = Colors::primary;
        float verticalScale = 1.0f;     // Volts/div
        float verticalOffset = 0.0f;    // Volts
        ChannelMode coupling = ChannelMode::DC;
        float brightness = 1.0f;
        bool showTrace = true;
        bool average = false;
        int averageSamples = 16;
    };

    struct TriggerSettings {
        TriggerMode mode = TriggerMode::Auto;
        bool positiveEdge = true;       // true = positive, false = negative
        float level = 0.0f;             // Trigger level in volts
        float holdoff = 100.0f;         // Holdoff time in microseconds
        float preTrigger = 20.0f;       // Pre-trigger percentage (0-100)
        bool noiseReject = false;
        bool hfReject = false;
    };

    struct TimebaseSettings {
        float timePerDivision = 1.0f;   // Time per grid division (ms/div)
        float sampleRate = 44100.0f;    // Sample rate in Hz
        int samplesPerScreen = 1024;    // Number of samples displayed
        float panOffset = 0.0f;         // Time offset in seconds
        bool rollMode = false;          // Rolling display mode
    };

    struct GridSettings {
        bool showGrid = true;
        bool showCenterLines = true;
        bool showTriggerLine = true;
        Color gridColor = Colors::outlineVariant;
        Color centerColor = Colors::outline;
        Color triggerColor = Colors::error;
        float gridOpacity = 0.3f;
        float centerOpacity = 0.6f;
        float triggerOpacity = 0.8f;
    };

    struct Measurement {
        std::string name;
        float value;
        std::string unit;
        Color color;
        bool show = true;
    };

    using DataCallback = std::function<void(const std::vector<float>& channelData, int channelIndex)>;
    using MeasurementCallback = std::function<std::vector<Measurement>(const std::vector<std::vector<float>>& channelData)>;

    Oscilloscope();
    ~Oscilloscope() override = default;

    // Component interface
    void paint(juce::Graphics& g) override;
    void resized() override;
    bool mouseDown(const juce::MouseEvent& e) override;
    bool mouseDrag(const juce::MouseEvent& e) override;
    bool mouseUp(const juce::MouseEvent& e) override;
    bool mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;
    void focusGained() override;
    void focusLost() override;
    
    // Channel management
    void addChannel(const Channel& channel = {});
    void removeChannel(int channelIndex);
    void clearChannels();
    void setChannelEnabled(int channelIndex, bool enabled);
    void setChannelColor(int channelIndex, const Color& color);
    void setChannelScale(int channelIndex, float scale);
    void setChannelOffset(int channelIndex, float offset);
    void setChannelCoupling(int channelIndex, ChannelMode coupling);
    
    // Data input
    void setChannelData(int channelIndex, const std::vector<float>& data);
    void setDataCallback(DataCallback callback);
    
    // Trigger settings
    void setTriggerSettings(const TriggerSettings& settings);
    TriggerSettings getTriggerSettings() const { return triggerSettings_; }
    
    // Timebase settings
    void setTimebaseSettings(const TimebaseSettings& settings);
    TimebaseSettings getTimebaseSettings() const { return timebaseSettings_; }
    
    // Grid settings
    void setGridSettings(const GridSettings& settings);
    GridSettings getGridSettings() const { return gridSettings_; }
    
    // Measurements
    void setMeasurementCallback(MeasurementCallback callback);
    void enableMeasurement(int channelIndex, const std::string& measurement);
    void disableMeasurement(int channelIndex, const std::string& measurement);
    void clearMeasurements(int channelIndex);
    
    // Display controls
    void setZoom(float zoom);
    float getZoom() const { return zoom_; }
    void setPan(float pan);
    float getPan() const { return pan_; }
    void resetView();
    void autoScale();
    
    // Export functions
    void exportData(const std::string& filename, int channelIndex);
    void exportImage(const std::string& filename, int width = 1920, int height = 1080);
    
    // Callbacks
    void setViewChangedCallback(std::function<void(float zoom, float pan)> callback);
    void setChannelChangedCallback(std::function<void(int channelIndex)> callback);
    
    // Accessibility
    juce::String getAccessibilityLabel() const override;
    juce::String getAccessibilityValue() const override;

private:
    static constexpr int MAX_CHANNELS = 8;
    static constexpr int MAX_SAMPLES = 8192;
    static constexpr int MIN_SAMPLES = 64;
    
    std::vector<Channel> channels_;
    std::vector<std::vector<float>> channelData_;
    
    TriggerSettings triggerSettings_;
    TimebaseSettings timebaseSettings_;
    GridSettings gridSettings_;
    
    // Display state
    float zoom_ = 1.0f;
    float pan_ = 0.0f;
    bool isDragging_ = false;
    juce::Point<float> dragStart_;
    juce::Point<float> lastMousePosition_;
    
    // Trigger state
    bool triggerArmed_ = true;
    bool triggerOccurred_ = false;
    float triggerPosition_ = 0.0f;
    int triggerSample_ = 0;
    
    // Animation
    AnimationValue displayZoom_;
    AnimationValue displayPan_;
    
    // Callbacks
    DataCallback dataCallback_;
    MeasurementCallback measurementCallback_;
    ViewChangedCallback viewChangedCallback_;
    ChannelChangedCallback channelChangedCallback_;
    
    // Internal helpers
    void processTrigger();
    void processData();
    void updateMeasurements();
    juce::Rectangle<float> getDisplayArea() const;
    juce::Rectangle<float> getGridBounds() const;
    void drawGrid(juce::Graphics& g);
    void drawChannels(juce::Graphics& g);
    void drawTrigger(juce::Graphics& g);
    void drawMeasurements(juce::Graphics& g);
    void drawCursors(juce::Graphics& g);
    juce::Point<float> dataToScreen(float x, float y, int channelIndex) const;
    juce::Point<float> screenToData(float x, float y, int& channelIndex) const;
    float getTriggerLevel() const;
    bool checkTrigger(const std::vector<float>& data) const;
    void reset();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_CHECK(Oscilloscope)
};

} // namespace visualizations
} // namespace ui
} // namespace vital