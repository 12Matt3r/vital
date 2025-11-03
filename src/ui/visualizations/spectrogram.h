#pragma once

#include "../core/component.h"
#include <memory>
#include <vector>
#include <functional>

namespace vital {
namespace ui {
namespace visualizations {

/**
 * @brief Real-time spectrogram visualization
 * 
 * Displays frequency spectrum over time with:
 * - Color-coded intensity mapping
 * - Time-frequency analysis
 * - Adjustable window functions
 * - Frequency scaling options
 * - Scroll and zoom capabilities
 */
class Spectrogram : public Component {
public:
    enum class FrequencyScale {
        Linear,         // Linear frequency scale
        Logarithmic,    // Logarithmic frequency scale
        Mel,            // Mel frequency scale
        Octave          // Octave-based scale
    };

    enum class ColorMap {
        Viridis,        // Viridis color map
        Plasma,         // Plasma color map
        Inferno,        // Inferno color map
        Magma,          // Magma color map
        Turbo,          // Google Turbo
        Custom          // Custom colormap
    };

    struct SpectrogramSettings {
        int fftSize = 1024;
        int hopSize = 256;
        float sampleRate = 44100.0f;
        FrequencyScale frequencyScale = FrequencyScale::Linear;
        ColorMap colorMap = ColorMap::Viridis;
        float minFreq = 20.0f;
        float maxFreq = 20000.0f;
        int timeBins = 256;
        bool normalize = true;
        float gamma = 1.0f;              // Gamma correction
        float dynamicRange = 60.0f;      // dB range
        float referenceLevel = 0.0f;     // Reference level in dB
        bool autoScale = true;
        bool showColorBar = true;
        bool showFrequencyAxis = true;
        bool showTimeAxis = true;
        float opacity = 1.0f;
        Color backgroundColor = Colors::surface;
        Color axisColor = Colors::onSurface;
        float axisOpacity = 0.7f;
    };

    using SpectrumCallback = std::function<void(const std::vector<float>& spectrum)>;
    using TimeSeriesCallback = std::function<void(const std::vector<std::vector<float>>& spectrogram)>;

    Spectrogram(const SpectrogramSettings& settings = {});
    ~Spectrogram() override = default;

    // Component interface
    void paint(juce::Graphics& g) override;
    void resized() override;
    bool mouseDown(const juce::MouseEvent& e) override;
    bool mouseDrag(const juce::MouseEvent& e) override;
    bool mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;
    
    // Settings
    void setSettings(const SpectrogramSettings& settings);
    SpectrogramSettings getSettings() const { return settings_; }
    
    // Data input
    void setSpectrumData(const std::vector<float>& spectrum);
    void setSpectrumCallback(SpectrumCallback callback);
    void addTimeBin(const std::vector<float>& spectrum);
    void clearSpectrogram();
    
    // Frequency settings
    void setFrequencyRange(float minFreq, float maxFreq);
    void setSampleRate(float sampleRate);
    void setFFTSize(int fftSize);
    void setHopSize(int hopSize);
    void setFrequencyScale(FrequencyScale scale);
    
    // Display settings
    void setColorMap(ColorMap colorMap);
    void setDynamicRange(float range);
    void setReferenceLevel(float level);
    void setGamma(float gamma);
    void setTimeBins(int timeBins);
    void setNormalize(bool normalize);
    void setOpacity(float opacity);
    
    // Axes
    void setShowFrequencyAxis(bool show);
    void setShowTimeAxis(bool show);
    void setShowColorBar(bool show);
    
    // Zoom and pan
    void setZoom(float zoom);
    float getZoom() const { return zoom_; }
    void setPan(float pan);
    float getPan() const { return pan_; }
    void resetView();
    
    // Export
    void exportImage(const std::string& filename, int width = 1920, int height = 1080);
    void exportData(const std::string& filename);
    
    // Accessibility
    juce::String getAccessibilityLabel() const override;

private:
    SpectrogramSettings settings_;
    std::vector<std::vector<float>> spectrogram_;
    
    // Display state
    float zoom_ = 1.0f;
    float pan_ = 0.0f;
    bool isDragging_ = false;
    juce::Point<float> dragStart_;
    
    // Processing
    float lastSpectrumTimestamp_ = 0.0f;
    int currentTimeBin_ = 0;
    
    // Callbacks
    SpectrumCallback spectrumCallback_;
    
    // Internal helpers
    void setupColormap();
    void updateSpectrogram();
    void addSpectrumRow(const std::vector<float>& spectrum);
    Color getColorFromValue(float value) const;
    juce::Rectangle<float> getSpectrogramArea() const;
    juce::Rectangle<float> getColorBarArea() const;
    juce::Rectangle<float> getFrequencyAxisArea() const;
    juce::Rectangle<float> getTimeAxisArea() const;
    void drawSpectrogram(juce::Graphics& g);
    void drawAxes(juce::Graphics& g);
    void drawColorBar(juce::Graphics& g);
    float frequencyToX(float frequency) const;
    float timeToY(int timeBin) const;
    float xToFrequency(float x) const;
    int yToTimeBin(float y) const;
    void applyFrequencyScale(float& freq) const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_CHECK(Spectrogram)
};

} // namespace visualizations
} // namespace ui
} // namespace vital