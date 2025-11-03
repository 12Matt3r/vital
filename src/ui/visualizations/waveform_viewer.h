#pragma once

#include "../core/component.h"
#include <memory>
#include <vector>
#include <functional>

namespace vital {
namespace ui {
namespace visualizations {

/**
 * @brief Waveform viewer for audio samples
 * 
 * Displays audio waveforms with:
 * - Zoom and pan functionality
 * - Multiple channel support
 * - Selection and playback markers
 * - Peak/RMS level display
 * - Cursor measurements
 */
class WaveformViewer : public Component {
public:
    struct Channel {
        std::string name;
        std::vector<float> data;
        Color color = Colors::primary;
        bool visible = true;
        float opacity = 1.0f;
        bool showPeaks = true;
        bool showRMS = true;
        bool mirrored = false; // For stereo
    };

    struct PlaybackMarker {
        float position = 0.0f;    // Position in seconds
        bool loopPoint = false;
        bool startMarker = false;
        Color color = Colors::primary;
        bool visible = true;
    };

    struct Selection {
        float start = 0.0f;
        float end = 1.0f;
        bool active = false;
        Color color = Colors::secondary;
        float opacity = 0.3f;
    };

    struct WaveformSettings {
        float sampleRate = 44100.0f;
        int channels = 1;
        float timePerPixel = 0.001f; // seconds per pixel
        float verticalScale = 1.0f;
        float centerLineOpacity = 0.3f;
        Color backgroundColor = Colors::surface;
        Color centerLineColor = Colors::outlineVariant;
        Color gridColor = Colors::outlineVariant;
        float gridOpacity = 0.2f;
        bool showGrid = true;
        bool showCenterLine = true;
        bool showCursor = true;
        Color cursorColor = Colors::error;
        float cursorOpacity = 0.8f;
        bool antiAliased = true;
        int maxPoints = 8192; // Points for downsampling
    };

    using SelectionCallback = std::function<void(const Selection& selection)>;
    using CursorCallback = std::function<void(float position)>;
    using PlaybackCallback = std::function<void(float position)>;

    WaveformViewer(const WaveformSettings& settings = {});
    ~WaveformViewer() override = default;

    // Component interface
    void paint(juce::Graphics& g) override;
    void resized() override;
    bool mouseDown(const juce::MouseEvent& e) override;
    bool mouseDrag(const juce::MouseEvent& e) override;
    bool mouseUp(const juce::MouseEvent& e) override;
    bool mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;
    void focusGained() override;
    void focusLost() override;
    
    // Audio data
    void setAudioData(const std::vector<float>& data, int channel = 0);
    void setAudioData(const std::vector<std::vector<float>>& data);
    void clearAudioData();
    
    // Channel management
    void addChannel(const Channel& channel);
    void removeChannel(int index);
    void setChannelData(int index, const std::vector<float>& data);
    void setChannelVisible(int index, bool visible);
    void setChannelColor(int index, const Color& color);
    void setChannelName(int index, const std::string& name);
    
    // Playback markers
    void addPlaybackMarker(const PlaybackMarker& marker);
    void removePlaybackMarker(int index);
    void clearPlaybackMarkers();
    void setMarkerPosition(int index, float position);
    void setMarkerVisible(int index, bool visible);
    
    // Selection
    void setSelection(const Selection& selection);
    void clearSelection();
    Selection getSelection() const { return selection_; }
    void setSelectionActive(bool active);
    
    // Settings
    void setSettings(const WaveformSettings& settings);
    WaveformSettings getSettings() const { return settings_; }
    
    // View controls
    void setTimePerPixel(float timePerPixel);
    void setVerticalScale(float scale);
    void setZoom(float zoom);
    float getZoom() const { return zoom_; }
    void setPan(float pan);
    float getPan() const { return pan_; }
    void resetView();
    void centerOnTime(float time);
    void fitToWindow();
    
    // Cursor
    void setCursorPosition(float position);
    float getCursorPosition() const { return cursorPosition_; }
    
    // Time display
    void setTimeRange(float start, float end);
    std::pair<float, float> getTimeRange() const;
    
    // Measurements
    float getPeakLevel(int channel = 0) const;
    float getRMSLevel(float start, float end, int channel = 0) const;
    float getDuration() const;
    
    // Callbacks
    void setSelectionCallback(SelectionCallback callback);
    void setCursorCallback(CursorCallback callback);
    void setPlaybackCallback(PlaybackCallback callback);
    
    // Export
    void exportSelection(const std::string& filename);
    void exportImage(const std::string& filename, int width = 1920, int height = 1080);
    
    // Accessibility
    juce::String getAccessibilityLabel() const override;

private:
    WaveformSettings settings_;
    std::vector<Channel> channels_;
    std::vector<PlaybackMarker> markers_;
    Selection selection_;
    
    // View state
    float zoom_ = 1.0f;
    float pan_ = 0.0f;
    bool isDragging_ = false;
    bool isSelecting_ = false;
    juce::Point<float> dragStart_;
    juce::Point<float> lastMousePosition_;
    float cursorPosition_ = 0.0f;
    
    // Precomputed data
    std::vector<std::vector<float>> downsampledData_;
    float timePerSample_ = 0.0f;
    
    // Callbacks
    SelectionCallback selectionCallback_;
    CursorCallback cursorCallback_;
    PlaybackCallback playbackCallback_;
    
    // Internal helpers
    void updateDownsampledData();
    void computeDownsampledData(const std::vector<float>& data, std::vector<float>& output);
    juce::Rectangle<float> getDisplayArea() const;
    juce::Point<float> timeToScreen(float time, float amplitude, int channel = 0) const;
    std::pair<float, float> screenToTime(const juce::Point<float>& position) const;
    void drawWaveform(juce::Graphics& g, const Channel& channel);
    void drawGrid(juce::Graphics& g);
    void drawCenterLine(juce::Graphics& g);
    void drawSelection(juce::Graphics& g);
    void drawMarkers(juce::Graphics& g);
    void drawCursor(juce::Graphics& g);
    void drawTimeScale(juce::Graphics& g);
    void drawAmplitudeScale(juce::Graphics& g);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_CHECK(WaveformViewer)
};

} // namespace visualizations
} // namespace ui
} // namespace vital