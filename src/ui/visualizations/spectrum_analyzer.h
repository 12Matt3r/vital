#pragma once

#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <JuceHeader.h>
#include "../core/component.h"

namespace vital {
namespace ui {
namespace visualizations {

/**
 * @brief FFT window types
 */
enum class WindowType {
    None,           // No windowing
    Hann,           // Hann window
    Hamming,        // Hamming window
    Blackman,       // Blackman window
    BlackmanHarris, // Blackman-Harris window
    Nuttall,        // Nuttall window
    FlatTop         // Flat top window
};

/**
 * @brief Spectrum display modes
 */
enum class SpectrumDisplayMode {
    Linear,         // Linear frequency scale
    Logarithmic,    // Logarithmic frequency scale
    Mel,            // Mel scale
    Octave          // Octave bands
};

/**
 * @brief Peak detection and hold
 */
struct PeakInfo {
    float frequency = 0.0f;    // Peak frequency in Hz
    float magnitude = 0.0f;    // Peak magnitude in dB
    float amplitude = 0.0f;    // Linear amplitude
    bool hold = false;         // Whether to hold this peak
    float hold_time = 0.0f;    // Time to hold (0 = infinite)
    float decay_rate = 1.0f;   // Decay rate for non-held peaks
    
    PeakInfo() = default;
    PeakInfo(float freq, float mag) 
        : frequency(freq), magnitude(mag), amplitude(juce::Decibels::decibelsToGain(mag)) {}
};

/**
 * @brief Spectrum analyzer configuration
 */
struct AnalyzerConfig {
    int fft_size = 2048;                    // FFT size (power of 2)
    int hop_size = -1;                      // Hop size (-1 = overlap)
    WindowType window_type = WindowType::Hann;
    SpectrumDisplayMode display_mode = SpectrumDisplayMode::Logarithmic;
    bool peak_detection = true;             // Enable peak detection
    bool peak_hold = true;                  // Enable peak hold
    float peak_hold_time = 2.0f;            // Peak hold time in seconds
    float smoothing_factor = 0.8f;          // Smoothing factor (0-1)
    float decay_rate = 2.0f;                // Peak decay rate
    float min_frequency = 20.0f;            // Minimum frequency to display
    float max_frequency = 20000.0f;         // Maximum frequency to display
    float min_amplitude = -80.0f;           // Minimum amplitude in dB
    float max_amplitude = 0.0f;             // Maximum amplitude in dB
    bool normalize_amplitudes = false;      // Normalize amplitudes
    bool show_grid = true;                  // Show frequency grid
    bool show_labels = true;                // Show frequency labels
    bool show_peaks = true;                 // Show detected peaks
    int grid_lines = 10;                    // Number of grid lines
    juce::Colour spectrum_color = juce::Colours::cyan;
    juce::Colour grid_color = juce::Colours::grey.withAlpha(0.3f);
    juce::Colour peak_color = juce::Colours::yellow;

    AnalyzerConfig() = default;
};

/**
 * @brief SpectrumAnalyzer - Real-time FFT spectrum analysis component
 * 
 * Provides high-performance real-time spectrum analysis with:
 * - Configurable FFT sizes and window functions
 * - Multiple display modes (linear, logarithmic, mel scale)
 * - Peak detection and hold functionality
 * - Smooth animation and decay effects
 * - Optimized rendering for real-time performance
 */
class VITAL_MODERN_UI_API SpectrumAnalyzer : public core::Component {
public:
    /**
     * @brief Constructor
     * @param config Analyzer configuration
     */
    explicit SpectrumAnalyzer(const AnalyzerConfig& config = AnalyzerConfig());

    /**
     * @brief Destructor
     */
    ~SpectrumAnalyzer() override;

    //==============================================================================
    // Configuration
    /**
     * @brief Update analyzer configuration
     */
    void setConfig(const AnalyzerConfig& config);

    /**
     * @brief Get current analyzer configuration
     */
    const AnalyzerConfig& getConfig() const { return config_; }

    /**
     * @brief Set FFT size
     * @param size FFT size (must be power of 2)
     */
    void setFFTSize(int size);

    /**
     * @brief Get FFT size
     */
    int getFFTSize() const { return config_.fft_size; }

    /**
     * @brief Set window type
     */
    void setWindowType(WindowType window_type);

    /**
     * @brief Get window type
     */
    WindowType getWindowType() const { return config_.window_type; }

    /**
     * @brief Set display mode
     */
    void setDisplayMode(SpectrumDisplayMode mode);

    /**
     * @brief Get display mode
     */
    SpectrumDisplayMode getDisplayMode() const { return config_.display_mode; }

    //==============================================================================
    // Audio Buffer Input
    /**
     * @brief Set audio buffer for analysis
     * @param buffer Audio buffer
     * @param num_channels Number of channels
     * @param num_samples Number of samples
     */
    void setAudioBuffer(const float* const* buffer, int num_channels, int num_samples);

    /**
     * @brief Set audio buffer (JUCE AudioBuffer)
     */
    void setAudioBuffer(const juce::AudioBuffer<float>& buffer);

    /**
     * @brief Process audio buffer in real-time
     * @param buffer Input buffer
     * @param num_samples Number of samples to process
     * @param channel Channel to analyze (-1 for average)
     */
    void processAudioBuffer(const float* buffer, int num_samples, int channel = -1);

    /**
     * @brief Add samples directly to analysis buffer
     * @param samples Sample data
     * @param num_samples Number of samples
     */
    void addSamples(const float* samples, int num_samples);

    /**
     * @brief Clear analysis buffer
     */
    void clearBuffer();

    //==============================================================================
    // Display Parameters
    /**
     * @ Set frequency range
     * @param min_freq Minimum frequency
     * @param max_freq Maximum frequency
     */
    void setFrequencyRange(float min_freq, float max_freq);

    /**
     * @brief Get frequency range
     */
    std::pair<float, float> getFrequencyRange() const { 
        return {config_.min_frequency, config_.max_frequency}; 
    }

    /**
     * @brief Set amplitude range
     * @param min_amp Minimum amplitude in dB
     * @param max_amp Maximum amplitude in dB
     */
    void setAmplitudeRange(float min_amp, float max_amp);

    /**
     * @brief Get amplitude range
     */
    std::pair<float, float> getAmplitudeRange() const { 
        return {config_.min_amplitude, config_.max_amplitude}; 
    }

    /**
     * @brief Set smoothing factor
     * @param factor Smoothing factor (0.0 to 1.0)
     */
    void setSmoothingFactor(float factor);

    /**
     * @brief Get smoothing factor
     */
    float getSmoothingFactor() const { return config_.smoothing_factor; }

    //==============================================================================
    // Peak Detection
    /**
     * @brief Enable/disable peak detection
     */
    void setPeakDetectionEnabled(bool enabled);

    /**
     * @brief Check if peak detection is enabled
     */
    bool isPeakDetectionEnabled() const { return config_.peak_detection; }

    /**
     * @brief Enable/disable peak hold
     */
    void setPeakHoldEnabled(bool enabled);

    /**
     * @brief Check if peak hold is enabled
     */
    bool isPeakHoldEnabled() const { return config_.peak_hold; }

    /**
     * @brief Set peak hold time
     * @param time Hold time in seconds
     */
    void setPeakHoldTime(float time);

    /**
     * @brief Get peak hold time
     */
    float getPeakHoldTime() const { return config_.peak_hold_time; }

    /**
     * @brief Clear all peaks
     */
    void clearPeaks();

    /**
     * @brief Get detected peaks
     */
    const std::vector<PeakInfo>& getPeaks() const { return peaks_; }

    /**
     * @brief Get maximum peak
     */
    PeakInfo getMaximumPeak() const;

    //==============================================================================
    // Visual Properties
    /**
     * @brief Set spectrum color
     */
    void setSpectrumColor(const juce::Colour& color);

    /**
     * @brief Get spectrum color
     */
    juce::Colour getSpectrumColor() const { return config_.spectrum_color; }

    /**
     * @brief Set grid color
     */
    void setGridColor(const juce::Colour& color);

    /**
     * @brief Get grid color
     */
    juce::Colour getGridColor() const { return config_.grid_color; }

    /**
     * @brief Set peak color
     */
    void setPeakColor(const juce::Colour& color);

    /**
     * @brief Get peak color
     */
    juce::Colour getPeakColor() const { return config_.peak_color; }

    /**
     * @brief Show/hide grid
     */
    void setGridVisible(bool visible);

    /**
     * @brief Check if grid is visible
     */
    bool isGridVisible() const { return config_.show_grid; }

    /**
     * @brief Show/hide labels
     */
    void setLabelsVisible(bool visible);

    /**
     * @brief Check if labels are visible
     */
    bool areLabelsVisible() const { return config_.show_labels; }

    /**
     * @brief Show/hide peaks
     */
    void setPeaksVisible(bool visible);

    /**
     * @brief Check if peaks are visible
     */
    bool arePeaksVisible() const { return config_.show_peaks; }

    //==============================================================================
    // Performance
    /**
     * @brief Get current frame rate
     */
    float getFrameRate() const { return frame_rate_.load(); }

    /**
     * @brief Get FFT processing time
     */
    float getFFTProcessingTime() const { return fft_processing_time_ms_; }

    /**
     * @brief Get rendering time
     */
    float getRenderingTime() const { return rendering_time_ms_; }

    /**
     * @brief Enable performance monitoring
     */
    void setPerformanceMonitoring(bool enabled);

    /**
     * @brief Check if performance monitoring is enabled
     */
    bool isPerformanceMonitoringEnabled() const { return performance_monitoring_; }

    //==============================================================================
    // Component Overrides
    void update() override;
    void render(juce::Graphics& g) override;

protected:
    /**
     * @brief Core::Component overrides
     */
    void updateInternal() override;
    void renderInternal(juce::Graphics& g) override;

private:
    //==============================================================================
    // Private member variables
    AnalyzerConfig config_;

    // Audio buffer
    std::vector<float> input_buffer_;
    std::vector<float> window_buffer_;
    std::vector<std::complex<float>> fft_buffer_;
    std::vector<float> spectrum_magnitude_;
    std::vector<float> spectrum_magnitude_smoothed_;
    int buffer_position_ = 0;
    int samples_available_ = 0;

    // FFT processing
    std::unique_ptr<juce::dsp::FFT> fft_;
    juce::dsp::WindowingFunction<float>::WindowingMethod window_method_;
    std::vector<float> window_coefficients_;

    // Peaks
    std::vector<PeakInfo> peaks_;
    std::vector<PeakInfo> held_peaks_;
    mutable std::mutex peaks_mutex_;

    // Performance tracking
    mutable std::atomic<float> frame_rate_{60.0f};
    float fft_processing_time_ms_ = 0.0f;
    float rendering_time_ms_ = 0.0f;
    juce::uint64 last_frame_time_ = 0;
    juce::uint64 frame_count_ = 0;
    bool performance_monitoring_ = false;

    // Rendering helpers
    juce::Rectangle<float> spectrum_area_;
    juce::Rectangle<float> grid_area_;
    std::vector<juce::Point<float>> spectrum_points_;
    std::vector<float> frequency_grid_lines_;
    std::vector<float> amplitude_grid_lines_;

    //==============================================================================
    // Private methods
    void initializeFFT();
    void updateWindowCoefficients();
    void processFFT();
    void detectPeaks();
    void updatePeakHold(float delta_time);
    void updateSpectrumSmoothing();
    void updateFrequencyGrid();
    void updateAmplitudeGrid();
    void updateSpectrumPoints();

    // Rendering methods
    void renderBackground(juce::Graphics& g);
    void renderGrid(juce::Graphics& g);
    void renderSpectrum(juce::Graphics& g);
    void renderPeaks(juce::Graphics& g);
    void renderLabels(juce::Graphics& g);
    void renderPerformanceInfo(juce::Graphics& g);

    // Utility methods
    float frequencyToX(float frequency) const;
    float amplitudeToY(float amplitude) const;
    float xToFrequency(float x) const;
    float yToAmplitude(float y) const;
    int frequencyToBin(float frequency) const;
    float binToFrequency(int bin) const;

    // Window function implementations
    void applyHannWindow(float* data, int size);
    void applyHammingWindow(float* data, int size);
    void applyBlackmanWindow(float* data, int size);
    void applyBlackmanHarrisWindow(float* data, int size);
    void applyNuttallWindow(float* data, int size);
    void applyFlatTopWindow(float* data, int size);

    // Frequency scale conversions
    float hzToMel(float hz) const;
    float melToHz(float mel) const;
    float hzToOctave(float hz, float reference_hz = 440.0f) const;
    float octaveToHz(float octave, float reference_hz = 440.0f) const;

    // Performance monitoring
    void updatePerformanceMetrics();
    void updateFrameRate();

    // Layout
    void calculateLayout();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectrumAnalyzer)
};

} // namespace visualizations
} // namespace ui
} // namespace vital
