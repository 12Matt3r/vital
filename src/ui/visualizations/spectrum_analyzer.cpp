#include "spectrum_analyzer.h"
#include <algorithm>
#include <cmath>
#include <complex>

namespace vital {
namespace ui {
namespace visualizations {

//==============================================================================
// SpectrumAnalyzer Implementation
//==============================================================================

SpectrumAnalyzer::SpectrumAnalyzer(const AnalyzerConfig& config)
    : config_(config),
      buffer_position_(0),
      samples_available_(0),
      fft_processing_time_ms_(0.0f),
      rendering_time_ms_(0.0f),
      last_frame_time_(0),
      frame_count_(0),
      performance_monitoring_(false),
      spectrum_area_(0, 0, 100, 100),
      grid_area_(0, 0, 100, 100),
      is_transitioning_(false) {
    
    // Initialize buffers
    initializeBuffers();
    
    // Initialize FFT
    initializeFFT();
    
    // Initialize window coefficients
    updateWindowCoefficients();
    
    // Initialize performance monitoring
    last_frame_time_ = juce::Time::getHighResolutionTicks();
}

SpectrumAnalyzer::~SpectrumAnalyzer() = default;

void SpectrumAnalyzer::setConfig(const AnalyzerConfig& config) {
    config_ = config;
    initializeFFT();
    updateWindowCoefficients();
    calculateLayout();
}

void SpectrumAnalyzer::setFFTSize(int size) {
    if (size < 64 || size > 16384 || (size & (size - 1)) != 0) {
        return; // Invalid FFT size
    }
    
    config_.fft_size = size;
    initializeFFT();
    updateWindowCoefficients();
    clearBuffer();
}

void SpectrumAnalyzer::setWindowType(WindowType window_type) {
    config_.window_type = window_type;
    updateWindowCoefficients();
}

void SpectrumAnalyzer::setDisplayMode(SpectrumDisplayMode mode) {
    config_.display_mode = mode;
    calculateLayout();
}

void SpectrumAnalyzer::setAudioBuffer(const float* const* buffer, int num_channels, int num_samples) {
    // Use average of all channels for spectrum analysis
    std::vector<float> averaged_samples(num_samples);
    
    for (int i = 0; i < num_samples; i++) {
        float sum = 0.0f;
        for (int channel = 0; channel < num_channels; channel++) {
            sum += buffer[channel][i];
        }
        averaged_samples[i] = sum / static_cast<float>(num_channels);
    }
    
    addSamples(averaged_samples.data(), num_samples);
}

void SpectrumAnalyzer::setAudioBuffer(const juce::AudioBuffer<float>& buffer) {
    if (buffer.getNumChannels() > 0 && buffer.getNumSamples() > 0) {
        setAudioBuffer(buffer.getArrayOfWriteablePointers(), 
                      buffer.getNumChannels(), 
                      buffer.getNumSamples());
    }
}

void SpectrumAnalyzer::processAudioBuffer(const float* buffer, int num_samples, int channel) {
    addSamples(buffer, num_samples);
}

void SpectrumAnalyzer::addSamples(const float* samples, int num_samples) {
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    
    for (int i = 0; i < num_samples; i++) {
        if (samples_available_ >= config_.fft_size) {
            // Process existing buffer if it's full
            break;
        }
        
        input_buffer_[buffer_position_] = samples[i];
        buffer_position_ = (buffer_position_ + 1) % input_buffer_.size();
        samples_available_++;
    }
}

void SpectrumAnalyzer::clearBuffer() {
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    
    std::fill(input_buffer_.begin(), input_buffer_.end(), 0.0f);
    std::fill(spectrum_magnitude_.begin(), spectrum_magnitude_.end(), 0.0f);
    std::fill(spectrum_magnitude_smoothed_.begin(), spectrum_magnitude_smooth_.end(), 0.0f);
    
    buffer_position_ = 0;
    samples_available_ = 0;
    
    clearPeaks();
}

void SpectrumAnalyzer::setFrequencyRange(float min_freq, float max_freq) {
    config_.min_frequency = min_freq;
    config_.max_frequency = max_freq;
    updateFrequencyGrid();
}

void SpectrumAnalyzer::setAmplitudeRange(float min_amp, float max_amp) {
    config_.min_amplitude = min_amp;
    config_.max_amplitude = max_amp;
    updateAmplitudeGrid();
}

void SpectrumAnalyzer::setSmoothingFactor(float factor) {
    config_.smoothing_factor = std::clamp(factor, 0.0f, 1.0f);
}

void SpectrumAnalyzer::setPeakDetectionEnabled(bool enabled) {
    config_.peak_detection = enabled;
}

void SpectrumAnalyzer::setPeakHoldEnabled(bool enabled) {
    config_.peak_hold = enabled;
}

void SpectrumAnalyzer::setPeakHoldTime(float time) {
    config_.peak_hold_time = std::max(time, 0.0f);
}

void SpectrumAnalyzer::clearPeaks() {
    peaks_.clear();
    held_peaks_.clear();
}

PeakInfo SpectrumAnalyzer::getMaximumPeak() const {
    std::lock_guard<std::mutex> lock(peaks_mutex_);
    
    if (peaks_.empty()) {
        return PeakInfo();
    }
    
    auto max_peak = std::max_element(peaks_.begin(), peaks_.end(),
        [](const PeakInfo& a, const PeakInfo& b) {
            return a.magnitude < b.magnitude;
        });
    
    return *max_peak;
}

void SpectrumAnalyzer::setSpectrumColor(const juce::Colour& color) {
    config_.spectrum_color = color;
}

void SpectrumAnalyzer::setGridColor(const juce::Colour& color) {
    config_.grid_color = color;
}

void SpectrumAnalyzer::setPeakColor(const juce::Colour& color) {
    config_.peak_color = color;
}

void SpectrumAnalyzer::setGridVisible(bool visible) {
    config_.show_grid = visible;
}

void SpectrumAnalyzer::setLabelsVisible(bool visible) {
    config_.show_labels = visible;
}

void SpectrumAnalyzer::setPeaksVisible(bool visible) {
    config_.show_peaks = visible;
}

void SpectrumAnalyzer::setPerformanceMonitoring(bool enabled) {
    performance_monitoring_ = enabled;
}

// Component updates
void SpectrumAnalyzer::update() {
    if (needs_redraw_) {
        updateInternal();
        updatePerformanceMetrics();
        needs_redraw_ = false;
    }
}

void SpectrumAnalyzer::render(juce::Graphics& g) {
    auto render_start = juce::Time::getHighResolutionTicks();
    
    renderBackground(g);
    
    if (config_.show_grid) {
        renderGrid(g);
    }
    
    renderSpectrum(g);
    
    if (config_.show_peaks) {
        renderPeaks(g);
    }
    
    if (config_.show_labels) {
        renderLabels(g);
    }
    
    if (performance_monitoring_) {
        renderPerformanceInfo(g);
    }
    
    if (performance_monitoring_) {
        auto render_end = juce::Time::getHighResolutionTicks();
        rendering_time_ms_ = juce::Time::highResolutionTicksToSeconds(render_end - render_start) * 1000.0f;
    }
}

void SpectrumAnalyzer::updateInternal() {
    // Process FFT if we have enough samples
    if (samples_available_ >= config_.fft_size) {
        processFFT();
        
        if (config_.peak_detection) {
            detectPeaks();
        }
        
        updateSpectrumSmoothing();
        updateSpectrumPoints();
        
        // Clear processed samples
        samples_available_ = 0;
        buffer_position_ = 0;
    }
    
    updatePeakHold(getDeltaTime());
    
    calculateLayout();
}

void SpectrumAnalyzer::renderInternal(juce::Graphics& g) {
    // Main rendering is handled in render()
}

// Private methods
void SpectrumAnalyzer::initializeBuffers() {
    int buffer_size = config_.fft_size * 4; // 4x buffer for overlap
    
    input_buffer_.resize(buffer_size);
    window_buffer_.resize(config_.fft_size);
    fft_buffer_.resize(config_.fft_size);
    spectrum_magnitude_.resize(config_.fft_size / 2);
    spectrum_magnitude_smoothed_.resize(config_.fft_size / 2);
    
    std::fill(input_buffer_.begin(), input_buffer_.end(), 0.0f);
    std::fill(spectrum_magnitude_.begin(), spectrum_magnitude_.end(), 0.0f);
    std::fill(spectrum_magnitude_smoothed_.begin(), spectrum_magnitude_.end(), 0.0f);
}

void SpectrumAnalyzer::initializeFFT() {
    try {
        fft_ = std::make_unique<juce::dsp::FFT>(static_cast<int>(std::log2(config_.fft_size)));
    } catch (const std::exception& e) {
        // Fallback to smaller FFT size
        config_.fft_size = 1024;
        fft_ = std::make_unique<juce::dsp::FFT>(10);
    }
    
    window_coefficients_.resize(config_.fft_size);
    updateWindowCoefficients();
}

void SpectrumAnalyzer::updateWindowCoefficients() {
    switch (config_.window_type) {
        case WindowType::None:
            // No windowing - all coefficients are 1.0
            std::fill(window_coefficients_.begin(), window_coefficients_.end(), 1.0f);
            break;
            
        case WindowType::Hann:
            applyHannWindow(window_coefficients_.data(), config_.fft_size);
            break;
            
        case WindowType::Hamming:
            applyHammingWindow(window_coefficients_.data(), config_.fft_size);
            break;
            
        case WindowType::Blackman:
            applyBlackmanWindow(window_coefficients_.data(), config_.fft_size);
            break;
            
        case WindowType::BlackmanHarris:
            applyBlackmanHarrisWindow(window_coefficients_.data(), config_.fft_size);
            break;
            
        case WindowType::Nuttall:
            applyNuttallWindow(window_coefficients_.data(), config_.fft_size);
            break;
            
        case WindowType::FlatTop:
            applyFlatTopWindow(window_coefficients_.data(), config_.fft_size);
            break;
    }
}

void SpectrumAnalyzer::processFFT() {
    auto fft_start = juce::Time::getHighResolutionTicks();
    
    std::lock_guard<std::mutex> lock(buffer_mutex_);
    
    // Copy and window the input buffer
    for (int i = 0; i < config_.fft_size; i++) {
        int buffer_index = (buffer_position_ + i) % input_buffer_.size();
        window_buffer_[i] = input_buffer_[buffer_index] * window_coefficients_[i];
    }
    
    // Perform FFT
    fft_->perform(window_buffer_.data(), fft_buffer_.data(), false);
    
    // Calculate magnitude spectrum
    for (int i = 0; i < config_.fft_size / 2; i++) {
        float real = fft_buffer_[i].real();
        float imag = fft_buffer_[i].imag();
        float magnitude = std::sqrt(real * real + imag * imag);
        
        // Convert to dB
        if (magnitude > 0.0f) {
            spectrum_magnitude_[i] = 20.0f * std::log10(magnitude);
        } else {
            spectrum_magnitude_[i] = config_.min_amplitude - 20.0f;
        }
    }
    
    auto fft_end = juce::Time::getHighResolutionTicks();
    if (performance_monitoring_) {
        fft_processing_time_ms_ = juce::Time::highResolutionTicksToSeconds(fft_end - fft_start) * 1000.0f;
    }
}

void SpectrumAnalyzer::detectPeaks() {
    std::lock_guard<std::mutex> lock(peaks_mutex_);
    
    peaks_.clear();
    
    // Simple peak detection using local maxima
    int window_size = std::max(3, config_.fft_size / 100);
    
    for (int i = window_size; i < spectrum_magnitude_.size() - window_size; i++) {
        bool is_peak = true;
        float current_magnitude = spectrum_magnitude_[i];
        
        // Check if this is a local maximum
        for (int j = -window_size; j <= window_size; j++) {
            if (j != 0 && spectrum_magnitude_[i + j] >= current_magnitude) {
                is_peak = false;
                break;
            }
        }
        
        // Additional peak criteria
        if (is_peak && current_magnitude > config_.min_amplitude + 10.0f) {
            float frequency = binToFrequency(i);
            PeakInfo peak(frequency, current_magnitude);
            peaks_.push_back(peak);
        }
    }
    
    // Sort peaks by magnitude (descending)
    std::sort(peaks_.begin(), peaks_.end(),
        [](const PeakInfo& a, const PeakInfo& b) {
            return a.magnitude > b.magnitude;
        });
    
    // Limit number of peaks
    if (peaks_.size() > 20) {
        peaks_.resize(20);
    }
}

void SpectrumAnalyzer::updatePeakHold(float delta_time) {
    std::lock_guard<std::mutex> lock(peaks_mutex_);
    
    if (!config_.peak_hold) {
        return;
    }
    
    // Update held peaks
    for (auto it = held_peaks_.begin(); it != held_peaks_.end();) {
        it->hold_time -= delta_time;
        
        if (it->hold_time <= 0.0f) {
            it = held_peaks_.erase(it);
        } else {
            ++it;
        }
    }
    
    // Add new peaks to held peaks
    for (const auto& peak : peaks_) {
        bool already_held = false;
        
        for (const auto& held_peak : held_peaks_) {
            if (std::abs(held_peak.frequency - peak.frequency) < 50.0f) { // Within 50 Hz
                already_held = true;
                break;
            }
        }
        
        if (!already_held) {
            PeakInfo held_peak = peak;
            held_peak.hold = true;
            held_peak.hold_time = config_.peak_hold_time;
            held_peaks_.push_back(held_peak);
        }
    }
}

void SpectrumAnalyzer::updateSpectrumSmoothing() {
    float alpha = config_.smoothing_factor;
    
    for (size_t i = 0; i < spectrum_magnitude_.size(); i++) {
        spectrum_magnitude_smoothed_[i] = alpha * spectrum_magnitude_smoothed_[i] + 
                                         (1.0f - alpha) * spectrum_magnitude_[i];
    }
}

void SpectrumAnalyzer::updateFrequencyGrid() {
    frequency_grid_lines_.clear();
    
    if (!config_.show_grid) return;
    
    int num_lines = config_.grid_lines;
    float range = config_.max_frequency - config_.min_frequency;
    
    for (int i = 0; i <= num_lines; i++) {
        float frequency = config_.min_frequency + (range * i / num_lines);
        frequency_grid_lines_.push_back(frequency);
    }
}

void SpectrumAnalyzer::updateAmplitudeGrid() {
    amplitude_grid_lines_.clear();
    
    if (!config_.show_grid) return;
    
    int num_lines = config_.grid_lines;
    float range = config_.max_amplitude - config_.min_amplitude;
    
    for (int i = 0; i <= num_lines; i++) {
        float amplitude = config_.min_amplitude + (range * i / num_lines);
        amplitude_grid_lines_.push_back(amplitude);
    }
}

void SpectrumAnalyzer::updateSpectrumPoints() {
    spectrum_points_.clear();
    
    int num_points = config_.fft_size / 2;
    float spectrum_width = spectrum_area_.getWidth();
    float spectrum_height = spectrum_area_.getHeight();
    
    for (int i = 0; i < num_points; i++) {
        float frequency = binToFrequency(i);
        float x = frequencyToX(frequency);
        float y = amplitudeToY(spectrum_magnitude_smoothed_[i]);
        
        spectrum_points_.push_back(juce::Point<float>(x, y));
    }
}

// Rendering methods
void SpectrumAnalyzer::renderBackground(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    
    // Background
    g.setColour(getThemeColor("surface"));
    g.fillRect(bounds);
    
    // Spectrum area
    float margin = 20.0f;
    spectrum_area_ = bounds.reduced(margin);
    
    grid_area_ = spectrum_area_;
}

void SpectrumAnalyzer::renderGrid(juce::Graphics& g) {
    g.setColour(config_.grid_color);
    
    // Vertical frequency lines
    for (float frequency : frequency_grid_lines_) {
        float x = frequencyToX(frequency);
        if (x >= grid_area_.getX() && x <= grid_area_.getRight()) {
            g.drawVerticalLine(x, grid_area_.getY(), grid_area_.getBottom());
        }
    }
    
    // Horizontal amplitude lines
    for (float amplitude : amplitude_grid_lines_) {
        float y = amplitudeToY(amplitude);
        if (y >= grid_area_.getY() && y <= grid_area_.getBottom()) {
            g.drawHorizontalLine(y, grid_area_.getX(), grid_area_.getRight());
        }
    }
}

void SpectrumAnalyzer::renderSpectrum(juce::Graphics& g) {
    if (spectrum_points_.size() < 2) return;
    
    g.setColour(config_.spectrum_color);
    
    // Draw spectrum line
    juce::Path spectrum_path;
    spectrum_path.startNewSubPath(spectrum_points_[0]);
    
    for (size_t i = 1; i < spectrum_points_.size(); i++) {
        spectrum_path.lineTo(spectrum_points_[i]);
    }
    
    g.strokePath(spectrum_path, juce::PathStrokeType(2.0f));
    
    // Fill spectrum area
    juce::Path filled_path = spectrum_path;
    filled_path.lineTo(spectrum_points_.back().getX(), grid_area_.getBottom());
    filled_path.lineTo(spectrum_points_[0].getX(), grid_area_.getBottom());
    filled_path.closeSubPath();
    
    g.setColour(config_.spectrum_color.withAlpha(0.2f));
    g.fillPath(filled_path);
}

void SpectrumAnalyzer::renderPeaks(juce::Graphics& g) {
    std::lock_guard<std::mutex> lock(peaks_mutex_);
    
    g.setColour(config_.peak_color);
    
    // Render current peaks
    for (const auto& peak : peaks_) {
        float x = frequencyToX(peak.frequency);
        float y = amplitudeToY(peak.magnitude);
        
        if (x >= grid_area_.getX() && x <= grid_area_.getRight() &&
            y >= grid_area_.getY() && y <= grid_area_.getBottom()) {
            
            g.fillEllipse(x - 3, y - 3, 6, 6);
        }
    }
    
    // Render held peaks
    for (const auto& held_peak : held_peaks_) {
        float x = frequencyToX(held_peak.frequency);
        float y = amplitudeToY(held_peak.magnitude);
        
        if (x >= grid_area_.getX() && x <= grid_area_.getRight() &&
            y >= grid_area_.getY() && y <= grid_area_.getBottom()) {
            
            g.setColour(config_.peak_color.withAlpha(0.7f));
            g.fillEllipse(x - 4, y - 4, 8, 8);
            
            // Draw hold indicator
            g.setColour(config_.peak_color.withAlpha(0.3f));
            g.drawEllipse(x - 6, y - 6, 12, 12, 1.0f);
        }
    }
}

void SpectrumAnalyzer::renderLabels(juce::Graphics& g) {
    if (!config_.show_labels) return;
    
    g.setColour(getThemeColor("on_surface"));
    g.setFont(juce::Font(10.0f));
    
    // Frequency labels
    juce::String freq_unit = "Hz";
    for (float frequency : frequency_grid_lines_) {
        float x = frequencyToX(frequency);
        if (x >= grid_area_.getX() && x <= grid_area_.getRight()) {
            juce::String label;
            if (frequency >= 1000.0f) {
                label = juce::String(frequency / 1000.0f, 1) + "k" + freq_unit;
            } else {
                label = juce::String((int)frequency) + freq_unit;
            }
            
            g.drawText(label, juce::Rectangle<float>(x - 20, grid_area_.getBottom() + 2, 40, 12),
                      juce::Justification::centred);
        }
    }
    
    // Amplitude labels
    juce::String amp_unit = "dB";
    for (float amplitude : amplitude_grid_lines_) {
        float y = amplitudeToY(amplitude);
        if (y >= grid_area_.getY() && y <= grid_area_.getBottom()) {
            juce::String label = juce::String((int)amplitude) + amp_unit;
            
            g.drawText(label, juce::Rectangle<float>(grid_area_.getX() - 35, y - 6, 30, 12),
                      juce::Justification::right);
        }
    }
}

void SpectrumAnalyzer::renderPerformanceInfo(juce::Graphics& g) {
    g.setColour(juce::Colours::red);
    g.setFont(juce::Font(10.0f));
    
    juce::String info = "FFT: " + juce::String(fft_processing_time_ms_, 2) + "ms, " +
                       "Render: " + juce::String(rendering_time_ms_, 2) + "ms, " +
                       "FPS: " + juce::String(getFrameRate(), 1);
    
    g.drawText(info, getLocalBounds().toFloat().removeFromTop(20),
              juce::Justification::topRight);
}

// Utility methods
float SpectrumAnalyzer::frequencyToX(float frequency) const {
    float normalized_freq = (frequency - config_.min_frequency) / 
                           (config_.max_frequency - config_.min_frequency);
    return grid_area_.getX() + normalized_freq * grid_area_.getWidth();
}

float SpectrumAnalyzer::amplitudeToY(float amplitude) const {
    float normalized_amp = (amplitude - config_.min_amplitude) / 
                          (config_.max_amplitude - config_.min_amplitude);
    return grid_area_.getBottom() - normalized_amp * grid_area_.getHeight();
}

float SpectrumAnalyzer::xToFrequency(float x) const {
    float normalized_x = (x - grid_area_.getX()) / grid_area_.getWidth();
    return config_.min_frequency + normalized_x * 
           (config_.max_frequency - config_.min_frequency);
}

float SpectrumAnalyzer::yToAmplitude(float y) const {
    float normalized_y = (grid_area_.getBottom() - y) / grid_area_.getHeight();
    return config_.min_amplitude + normalized_y * 
           (config_.max_amplitude - config_.min_amplitude);
}

int SpectrumAnalyzer::frequencyToBin(float frequency) const {
    float bin_frequency = frequencyToX(frequency);
    float normalized_freq = (bin_frequency - grid_area_.getX()) / grid_area_.getWidth();
    int bin = static_cast<int>(normalized_freq * (config_.fft_size / 2));
    return std::clamp(bin, 0, static_cast<int>(spectrum_magnitude_.size()) - 1);
}

float SpectrumAnalyzer::binToFrequency(int bin) const {
    float sample_rate = 44100.0f; // Assuming 44.1kHz sample rate
    float bin_hz = (sample_rate / 2.0f) * bin / (config_.fft_size / 2);
    return bin_hz;
}

// Window function implementations
void SpectrumAnalyzer::applyHannWindow(float* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * i / (size - 1)));
    }
}

void SpectrumAnalyzer::applyHammingWindow(float* data, int size) {
    for (int i = 0; i < size; i++) {
        data[i] = 0.54f - 0.46f * std::cos(2.0f * juce::MathConstants<float>::pi * i / (size - 1));
    }
}

void SpectrumAnalyzer::applyBlackmanWindow(float* data, int size) {
    for (int i = 0; i < size; i++) {
        float a0 = 0.42f;
        float a1 = 0.5f;
        float a2 = 0.08f;
        
        data[i] = a0 - a1 * std::cos(2.0f * juce::MathConstants<float>::pi * i / (size - 1)) +
                  a2 * std::cos(4.0f * juce::MathConstants<float>::pi * i / (size - 1));
    }
}

void SpectrumAnalyzer::applyBlackmanHarrisWindow(float* data, int size) {
    for (int i = 0; i < size; i++) {
        float a0 = 0.35875f;
        float a1 = 0.48829f;
        float a2 = 0.14128f;
        float a3 = 0.01168f;
        
        data[i] = a0 - a1 * std::cos(2.0f * juce::MathConstants<float>::pi * i / (size - 1)) +
                  a2 * std::cos(4.0f * juce::MathConstants<float>::pi * i / (size - 1)) -
                  a3 * std::cos(6.0f * juce::MathConstants<float>::pi * i / (size - 1));
    }
}

void SpectrumAnalyzer::applyNuttallWindow(float* data, int size) {
    for (int i = 0; i < size; i++) {
        float a0 = 0.355768f;
        float a1 = 0.487396f;
        float a2 = 0.144232f;
        float a3 = 0.012604f;
        
        data[i] = a0 - a1 * std::cos(2.0f * juce::MathConstants<float>::pi * i / (size - 1)) +
                  a2 * std::cos(4.0f * juce::MathConstants<float>::pi * i / (size - 1)) -
                  a3 * std::cos(6.0f * juce::MathConstants<float>::pi * i / (size - 1));
    }
}

void SpectrumAnalyzer::applyFlatTopWindow(float* data, int size) {
    for (int i = 0; i < size; i++) {
        float a0 = 1.0f;
        float a1 = 1.93f;
        float a2 = 1.29f;
        float a3 = 0.388f;
        float a4 = 0.028f;
        
        data[i] = a0 - a1 * std::cos(2.0f * juce::MathConstants<float>::pi * i / (size - 1)) +
                  a2 * std::cos(4.0f * juce::MathConstants<float>::pi * i / (size - 1)) -
                  a3 * std::cos(6.0f * juce::MathConstants<float>::pi * i / (size - 1)) +
                  a4 * std::cos(8.0f * juce::MathConstants<float>::pi * i / (size - 1));
    }
}

// Frequency scale conversions
float SpectrumAnalyzer::hzToMel(float hz) const {
    return 2595.0f * std::log10(1.0f + hz / 700.0f);
}

float SpectrumAnalyzer::melToHz(float mel) const {
    return 700.0f * (std::pow(10.0f, mel / 2595.0f) - 1.0f);
}

float SpectrumAnalyzer::hzToOctave(float hz, float reference_hz) const {
    return std::log2(hz / reference_hz);
}

float SpectrumAnalyzer::octaveToHz(float octave, float reference_hz) const {
    return reference_hz * std::pow(2.0f, octave);
}

// Performance monitoring
void SpectrumAnalyzer::updatePerformanceMetrics() {
    updateFrameRate();
}

void SpectrumAnalyzer::updateFrameRate() {
    auto current_time = juce::Time::getHighResolutionTicks();
    auto delta_time = current_time - last_frame_time_;
    
    if (delta_time > 0) {
        float frame_rate = 1.0f / juce::Time::highResolutionTicksToSeconds(delta_time);
        frame_rate_.store(frame_rate);
    }
    
    last_frame_time_ = current_time;
    frame_count_++;
}

// Layout
void SpectrumAnalyzer::calculateLayout() {
    updateFrequencyGrid();
    updateAmplitudeGrid();
    updateSpectrumPoints();
}

} // namespace visualizations
} // namespace ui
} // namespace vital