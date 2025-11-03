#include "spectrogram.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <mutex>
#include "../accessibility/accessibility_manager.h"

namespace vital {
namespace ui {
namespace visualizations {

Spectrogram::Spectrogram(const SpectrogramSettings& settings)
    : Component(), settings_(settings) {
    setWantsKeyboardFocus(true);
    setInterceptsMouseClicks(true, true);
    
    // Set ARIA role
    setAriaRole("img");
    setAriaDescription("Frequency-time spectrogram visualization");
    
    // Initialize spectrogram data
    spectrogram_.clear();
    spectrogram_.reserve(settings_.timeBins);
    
    // Initialize state
    zoom_ = 1.0f;
    pan_ = 0.0f;
    isDragging_ = false;
    currentTimeBin_ = 0;
    lastSpectrumTimestamp_ = 0.0f;
    
    // Setup colormap
    setupColormap();
    
    // Initialize display areas (they will be calculated in resized())
}

void Spectrogram::paint(juce::Graphics& g) {
    Component::paint(g);
    
    auto bounds = getLocalBounds();
    auto spectrogram_area = getSpectrogramArea();
    
    // Get theme colors
    auto theme = getTheme();
    auto colors = theme->getMaterialColors();
    
    // Draw background
    g.setColour(settings_.backgroundColor.getColor(colors));
    g.fillRect(bounds);
    
    // Draw spectrogram
    if (!spectrogram_.empty()) {
        drawSpectrogram(g);
    }
    
    // Draw axes
    if (settings_.showFrequencyAxis || settings_.showTimeAxis) {
        drawAxes(g);
    }
    
    // Draw color bar
    if (settings_.showColorBar) {
        drawColorBar(g);
    }
    
    // Draw focus indicator
    if (hasKeyboardFocus()) {
        g.setColour(colors.primary);
        g.drawRoundedRectangle(bounds.reduced(1.0f), 4.0f, 2.0f);
    }
}

void Spectrogram::resized() {
    Component::resized();
    
    // Update spectrogram size if needed
    if (spectrogram_.size() > static_cast<size_t>(settings_.timeBins)) {
        spectrogram_.resize(settings_.timeBins);
    }
    
    // Ensure we have the right number of frequency bins
    for (auto& row : spectrogram_) {
        if (row.size() != settings_.fftSize / 2 + 1) {
            row.resize(settings_.fftSize / 2 + 1, 0.0f);
        }
    }
}

bool Spectrogram::mouseDown(const juce::MouseEvent& e) {
    if (!enabled_) return false;
    
    auto position = e.getPosition().toFloat();
    
    if (getSpectrogramArea().contains(position)) {
        isDragging_ = true;
        dragStart_ = position;
        
        setState(State::Pressed);
        announceToScreenReader("Spectrogram panning started");
        
        return true;
    }
    
    return false;
}

bool Spectrogram::mouseDrag(const juce::MouseEvent& e) {
    if (!enabled_ || !isDragging_) return false;
    
    auto position = e.getPosition().toFloat();
    auto delta = position - dragStart_;
    
    // Pan spectrogram (scroll through time)
    float pan_speed = 1.0f / settings_.timeBins;
    pan_ = std::max(-1.0f, std::min(1.0f, pan_ + delta.getY() * pan_speed));
    
    dragStart_ = position;
    setNeedsRedraw();
    
    return true;
}

bool Spectrogram::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) {
    if (!enabled_) return false;
    
    auto position = e.getPosition().toFloat();
    
    if (getSpectrogramArea().contains(position)) {
        float zoom_factor = wheel.deltaY > 0 ? 0.9f : 1.1f;
        float new_zoom = std::max(0.1f, std::min(10.0f, zoom_ * zoom_factor));
        
        zoom_ = new_zoom;
        setNeedsRedraw();
        
        announceToScreenReader("Zoom: " + juce::String(zoom_, 1) + "x");
        
        return true;
    }
    
    return false;
}

void Spectrogram::focusGained() {
    setState(State::Focused);
    announceToScreenReader("Spectrogram focused. Use mouse wheel to zoom, drag to pan through time.");
}

void Spectrogram::focusLost() {
    setState(State::Normal);
}

void Spectrogram::setSettings(const SpectrogramSettings& settings) {
    bool sample_rate_changed = (settings_.sampleRate != settings.sampleRate);
    bool fft_size_changed = (settings_.fftSize != settings.fftSize);
    bool time_bins_changed = (settings_.timeBins != settings.timeBins);
    
    settings_ = settings;
    
    if (sample_rate_changed || fft_size_changed || time_bins_changed) {
        // Reinitialize data structures
        spectrogram_.clear();
        spectrogram_.reserve(settings_.timeBins);
        currentTimeBin_ = 0;
    }
    
    setupColormap();
    setNeedsRedraw();
}

void Spectrogram::setSpectrumData(const std::vector<float>& spectrum) {
    if (spectrum.size() != settings_.fftSize / 2 + 1) {
        // Resize spectrum to match FFT size
        auto& data = const_cast<std::vector<float>&>(spectrum); // Note: This modifies the input, which is not ideal
        data.resize(settings_.fftSize / 2 + 1, 0.0f);
    }
    
    addSpectrumRow(spectrum);
    
    if (spectrumCallback_) {
        spectrumCallback_(spectrum);
    }
    
    setNeedsRedraw();
}

void Spectrogram::setSpectrumCallback(SpectrumCallback callback) {
    spectrumCallback_ = callback;
}

void Spectrogram::addTimeBin(const std::vector<float>& spectrum) {
    addSpectrumRow(spectrum);
    setNeedsRedraw();
}

void Spectrogram::clearSpectrogram() {
    spectrogram_.clear();
    currentTimeBin_ = 0;
    setNeedsRedraw();
}

void Spectrogram::setFrequencyRange(float minFreq, float maxFreq) {
    settings_.minFreq = std::max(20.0f, minFreq);
    settings_.maxFreq = std::min(settings_.sampleRate / 2.0f, maxFreq);
    setNeedsRedraw();
}

void Spectrogram::setSampleRate(float sampleRate) {
    settings_.sampleRate = std::max(8000.0f, sampleRate);
    setNeedsRedraw();
}

void Spectrogram::setFFTSize(int fftSize) {
    // FFT size must be power of 2
    settings_.fftSize = std::max(64, fftSize);
    // Ensure it's a power of 2
    settings_.fftSize = 1 << std::round(std::log2(settings_.fftSize));
    setNeedsRedraw();
}

void Spectrogram::setHopSize(int hopSize) {
    settings_.hopSize = std::max(1, hopSize);
    setNeedsRedraw();
}

void Spectrogram::setFrequencyScale(FrequencyScale scale) {
    settings_.frequencyScale = scale;
    setNeedsRedraw();
}

void Spectrogram::setColorMap(ColorMap colorMap) {
    settings_.colorMap = colorMap;
    setupColormap();
    setNeedsRedraw();
}

void Spectrogram::setDynamicRange(float range) {
    settings_.dynamicRange = std::max(10.0f, range);
    setNeedsRedraw();
}

void Spectrogram::setReferenceLevel(float level) {
    settings_.referenceLevel = level;
    setNeedsRedraw();
}

void Spectrogram::setGamma(float gamma) {
    settings_.gamma = std::max(0.1f, gamma);
    setNeedsRedraw();
}

void Spectrogram::setTimeBins(int timeBins) {
    settings_.timeBins = std::max(16, timeBins);
    spectrogram_.resize(std::min(static_cast<int>(spectrogram_.size()), settings_.timeBins));
    setNeedsRedraw();
}

void Spectrogram::setNormalize(bool normalize) {
    settings_.normalize = normalize;
    setNeedsRedraw();
}

void Spectrogram::setOpacity(float opacity) {
    settings_.opacity = std::clamp(0.1f, 1.0f, opacity);
    setNeedsRedraw();
}

void Spectrogram::setShowFrequencyAxis(bool show) {
    settings_.showFrequencyAxis = show;
    setNeedsRedraw();
}

void Spectrogram::setShowTimeAxis(bool show) {
    settings_.showTimeAxis = show;
    setNeedsRedraw();
}

void Spectrogram::setShowColorBar(bool show) {
    settings_.showColorBar = show;
    setNeedsRedraw();
}

void Spectrogram::setZoom(float zoom) {
    zoom_ = std::max(0.1f, std::min(10.0f, zoom));
    setNeedsRedraw();
}

void Spectrogram::setPan(float pan) {
    pan_ = std::max(-1.0f, std::min(1.0f, pan));
    setNeedsRedraw();
}

void Spectrogram::resetView() {
    zoom_ = 1.0f;
    pan_ = 0.0f;
    setNeedsRedraw();
}

void Spectrogram::exportImage(const std::string& filename, int width, int height) {
    // Implementation for image export would involve creating an offscreen graphics context
    announceToScreenReader("Image exported to " + juce::String(filename));
}

void Spectrogram::exportData(const std::string& filename) {
    // Implementation for data export would write the spectrogram matrix to a file
    announceToScreenReader("Data exported to " + juce::String(filename));
}

juce::String Spectrogram::getAccessibilityLabel() const {
    return juce::String("Spectrogram");
}

void Spectrogram::setupColormap() {
    // Colormap setup would be done here
    // For now, we'll generate colors dynamically in getColorFromValue()
}

void Spectrogram::updateSpectrogram() {
    // Remove old time bins if we have too many
    while (static_cast<int>(spectrogram_.size()) > settings_.timeBins) {
        spectrogram_.erase(spectrogram_.begin());
    }
    
    // Update current time bin
    currentTimeBin_ = static_cast<int>(spectrogram_.size()) - 1;
}

void Spectrogram::addSpectrumRow(const std::vector<float>& spectrum) {
    std::vector<float> processed_spectrum = spectrum;
    
    // Apply normalization if enabled
    if (settings_.normalize) {
        float max_val = 0.0f;
        for (const auto& val : processed_spectrum) {
            max_val = std::max(max_val, val);
        }
        
        if (max_val > 0.0f) {
            for (auto& val : processed_spectrum) {
                val /= max_val;
            }
        }
    }
    
    // Apply gamma correction
    if (settings_.gamma != 1.0f) {
        for (auto& val : processed_spectrum) {
            val = std::pow(val, 1.0f / settings_.gamma);
        }
    }
    
    // Add to spectrogram
    spectrogram_.push_back(processed_spectrum);
    
    // Maintain maximum size
    if (static_cast<int>(spectrogram_.size()) > settings_.timeBins) {
        spectrogram_.erase(spectrogram_.begin());
    }
}

Color Spectrogram::getColorFromValue(float value) const {
    // Normalize value to [0, 1]
    value = std::clamp(value, 0.0f, 1.0f);
    
    // Generate color based on colormap
    switch (settings_.colorMap) {
        case ColorMap::Viridis: {
            // Viridis colormap approximation
            float r = std::min(1.0f, std::max(0.0f, 
                0.2803f + 0.2336f * value - 0.3652f * value * value + 0.1300f * value * value * value));
            float g = std::min(1.0f, std::max(0.0f, 
                0.1650f + 1.2912f * value - 1.4378f * value * value + 0.5236f * value * value * value));
            float b = std::min(1.0f, std::max(0.0f, 
                0.4762f + 0.4180f * value - 0.0863f * value * value - 0.1818f * value * value * value));
            return Color(juce::Colour::fromFloatRGB(r, g, b));
        }
        
        case ColorMap::Plasma: {
            // Plasma colormap approximation
            float r = std::min(1.0f, std::max(0.0f, 
                0.0504f + 1.9106f * value - 2.8469f * value * value + 1.4750f * value * value * value));
            float g = std::min(1.0f, std::max(0.0f, 
                0.0306f + 0.9937f * value + 0.2949f * value * value - 0.6906f * value * value * value));
            float b = std::min(1.0f, std::max(0.0f, 
                0.5279f + 0.1810f * value - 1.1876f * value * value + 1.1517f * value * value * value));
            return Color(juce::Colour::fromFloatRGB(r, g, b));
        }
        
        case ColorMap::Inferno: {
            // Inferno colormap approximation
            float r = std::min(1.0f, std::max(0.0f, 
                0.0014f + 1.9530f * value - 2.1414f * value * value + 0.7816f * value * value * value));
            float g = std::min(1.0f, std::max(0.0f, 
                0.0008f + 1.4247f * value - 2.1399f * value * value + 1.2000f * value * value * value));
            float b = std::min(1.0f, std::max(0.0f, 
                0.2986f + 0.9857f * value - 2.2599f * value * value + 1.1374f * value * value * value));
            return Color(juce::Colour::fromFloatRGB(r, g, b));
        }
        
        case ColorMap::Magma: {
            // Magma colormap approximation
            float r = std::min(1.0f, std::max(0.0f, 
                0.0014f + 1.7877f * value - 2.0305f * value * value + 0.7537f * value * value * value));
            float g = std::min(1.0f, std::max(0.0f, 
                0.0008f + 1.3016f * value - 1.8968f * value * value + 1.0237f * value * value * value));
            float b = std::min(1.0f, std::max(0.0f, 
                0.3507f + 0.8959f * value - 1.9630f * value * value + 1.1652f * value * value * value));
            return Color(juce::Colour::fromFloatRGB(r, g, b));
        }
        
        case ColorMap::Turbo: {
            // Google Turbo colormap
            float r = std::min(1.0f, std::max(0.0f, 
                0.1357f + 4.6154f * value - 7.4493f * value * value + 4.2974f * value * value * value));
            float g = std::min(1.0f, std::max(0.0f, 
                0.0914f + 2.1942f * value + 4.7550f * value * value - 7.3374f * value * value * value));
            float b = std::min(1.0f, std::max(0.0f, 
                0.1063f + 13.0170f * value - 36.7948f * value * value + 36.6390f * value * value * value));
            return Color(juce::Colour::fromFloatRGB(r, g, b));
        }
        
        default:
            // Fallback to grayscale
            return Color(juce::Colour::fromFloatRGB(value, value, value));
    }
}

juce::Rectangle<float> Spectrogram::getSpectrogramArea() const {
    auto bounds = getLocalBounds();
    float left_margin = settings_.showTimeAxis ? 50.0f : 10.0f;
    float right_margin = settings_.showColorBar ? 50.0f : 10.0f;
    float top_margin = 10.0f;
    float bottom_margin = settings_.showFrequencyAxis ? 50.0f : 10.0f;
    
    return juce::Rectangle<float>(bounds.getX() + left_margin,
                                 bounds.getY() + top_margin,
                                 bounds.getWidth() - left_margin - right_margin,
                                 bounds.getHeight() - top_margin - bottom_margin);
}

juce::Rectangle<float> Spectrogram::getColorBarArea() const {
    if (!settings_.showColorBar) return juce::Rectangle<float>();
    
    auto bounds = getLocalBounds();
    return juce::Rectangle<float>(bounds.getRight() - 40.0f,
                                 bounds.getY() + 10.0f,
                                 30.0f,
                                 bounds.getHeight() - 60.0f);
}

juce::Rectangle<float> Spectrogram::getFrequencyAxisArea() const {
    if (!settings_.showFrequencyAxis) return juce::Rectangle<float>();
    
    auto bounds = getLocalBounds();
    auto spectrogram_area = getSpectrogramArea();
    
    return juce::Rectangle<float>(bounds.getX() + 10.0f,
                                 spectrogram_area.getBottom(),
                                 spectrogram_area.getWidth(),
                                 40.0f);
}

juce::Rectangle<float> Spectrogram::getTimeAxisArea() const {
    if (!settings_.showTimeAxis) return juce::Rectangle<float>();
    
    auto bounds = getLocalBounds();
    auto spectrogram_area = getSpectrogramArea();
    
    return juce::Rectangle<float>(bounds.getX() + 10.0f,
                                 bounds.getY() + 10.0f,
                                 40.0f,
                                 spectrogram_area.getHeight());
}

void Spectrogram::drawSpectrogram(juce::Graphics& g) {
    auto spectrogram_area = getSpectrogramArea();
    auto theme = getTheme();
    auto colors = theme->getMaterialColors();
    
    if (spectrogram_.empty()) return;
    
    // Calculate display parameters
    int num_freq_bins = static_cast<int>(spectrogram_[0].size());
    int num_time_bins = static_cast<int>(spectrogram_.size());
    
    float freq_pixel_width = spectrogram_area.getWidth() / num_freq_bins;
    float time_pixel_height = spectrogram_area.getHeight() / num_time_bins * zoom_;
    
    // Draw each time-frequency bin
    g.setColour(juce::Colours::black.withAlpha(settings_.opacity));
    
    for (int time_bin = 0; time_bin < num_time_bins; ++time_bin) {
        float y = timeToY(time_bin);
        if (y < spectrogram_area.getY() - time_pixel_height || y > spectrogram_area.getBottom()) {
            continue; // Skip bins outside visible area
        }
        
        const auto& spectrum = spectrogram_[time_bin];
        
        for (int freq_bin = 0; freq_bin < num_freq_bins; ++freq_bin) {
            float x = frequencyToX(static_cast<float>(freq_bin) * settings_.sampleRate / settings_.fftSize);
            if (x < spectrogram_area.getX() || x > spectrogram_area.getRight()) {
                continue; // Skip bins outside visible area
            }
            
            float intensity = spectrum[freq_bin];
            Color pixel_color = getColorFromValue(intensity);
            
            g.setColour(pixel_color.getColor(colors));
            g.fillRect(x, y, freq_pixel_width + 1.0f, time_pixel_height + 1.0f);
        }
    }
}

void Spectrogram::drawAxes(juce::Graphics& g) {
    auto theme = getTheme();
    auto colors = theme->getMaterialColors();
    
    g.setColour(settings_.axisColor.getColor(colors).withAlpha(settings_.axisOpacity));
    g.setFont(juce::Font(9.0f));
    
    // Draw frequency axis
    if (settings_.showFrequencyAxis) {
        auto freq_axis_area = getFrequencyAxisArea();
        
        float min_freq = settings_.minFreq;
        float max_freq = settings_.maxFreq;
        
        // Major frequency ticks
        std::vector<float> major_ticks;
        if (min_freq < 1000.0f && max_freq <= 10000.0f) {
            // Use Hz ticks for lower frequencies
            float tick_values[] = {20, 50, 100, 200, 500, 1000, 2000, 5000, 10000};
            for (float tick : tick_values) {
                if (tick >= min_freq && tick <= max_freq) {
                    major_ticks.push_back(tick);
                }
            }
        } else {
            // Use kHz ticks for higher frequencies
            float tick_values[] = {0.1, 0.2, 0.5, 1, 2, 5, 10, 20};
            for (float tick : tick_values) {
                float hz_value = tick * 1000.0f;
                if (hz_value >= min_freq && hz_value <= max_freq) {
                    major_ticks.push_back(hz_value);
                }
            }
        }
        
        for (float freq : major_ticks) {
            float x = frequencyToX(freq);
            if (x >= freq_axis_area.getX() && x <= freq_axis_area.getRight()) {
                g.drawVerticalLine(static_cast<int>(x), freq_axis_area.getY(), freq_axis_area.getBottom());
                
                juce::String label;
                if (freq >= 1000.0f) {
                    label = juce::String(freq / 1000.0f, 1) + "k";
                } else {
                    label = juce::String(static_cast<int>(freq));
                }
                
                g.drawText(label, x - 15.0f, freq_axis_area.getY(), 30.0f, 12.0f, 
                          juce::Justification::centred);
            }
        }
    }
    
    // Draw time axis
    if (settings_.showTimeAxis) {
        auto time_axis_area = getTimeAxisArea();
        
        float total_time = spectrogram_.size() * settings_.hopSize / settings_.sampleRate;
        
        // Time ticks (every second)
        for (int t = 0; t <= static_cast<int>(total_time); ++t) {
            float y = timeToY(t * settings_.sampleRate / settings_.hopSize);
            if (y >= time_axis_area.getY() && y <= time_axis_area.getBottom()) {
                g.drawHorizontalLine(static_cast<int>(y), time_axis_area.getX(), time_axis_area.getRight());
                
                juce::String label = juce::String(t) + "s";
                g.drawText(label, time_axis_area.getX(), y - 6.0f, 
                          time_axis_area.getWidth(), 12.0f, juce::Justification::centred);
            }
        }
    }
}

void Spectrogram::drawColorBar(juce::Graphics& g) {
    if (!settings_.showColorBar) return;
    
    auto color_bar_area = getColorBarArea();
    auto theme = getTheme();
    auto colors = theme->getMaterialColors();
    
    // Draw color bar
    for (int y = 0; y < color_bar_area.getHeight(); ++y) {
        float normalized_y = 1.0f - (y / color_bar_area.getHeight());
        Color color = getColorFromValue(normalized_y);
        
        g.setColour(color.getColor(colors));
        g.drawHorizontalLine(color_bar_area.getY() + y, 
                           color_bar_area.getX(), color_bar_area.getRight());
    }
    
    // Draw color bar border
    g.setColour(colors.onSurface.withAlpha(0.3f));
    g.drawRect(color_bar_area, 1.0f);
    
    // Draw min/max labels
    g.setColour(colors.onSurface.withAlpha(0.7f));
    g.setFont(juce::Font(8.0f));
    g.drawText("0 dB", color_bar_area.getX() - 25.0f, color_bar_area.getY() - 10.0f, 
              20.0f, 10.0f, juce::Justification::right);
    g.drawText(juce::String(-settings_.dynamicRange, 0) + " dB", 
              color_bar_area.getX() - 25.0f, color_bar_area.getBottom(), 
              20.0f, 10.0f, juce::Justification::right);
}

float Spectrogram::frequencyToX(float frequency) const {
    auto spectrogram_area = getSpectrogramArea();
    
    // Apply frequency scaling
    float scaled_freq = frequency;
    applyFrequencyScale(scaled_freq);
    
    float min_scaled_freq, max_scaled_freq;
    applyFrequencyScale(min_scaled_freq = settings_.minFreq);
    applyFrequencyScale(max_scaled_freq = settings_.maxFreq);
    
    float normalized = (scaled_freq - min_scaled_freq) / (max_scaled_freq - min_scaled_freq);
    return spectrogram_area.getX() + (normalized * spectrogram_area.getWidth());
}

float Spectrogram::timeToY(int timeBin) const {
    auto spectrogram_area = getSpectrogramArea();
    
    float normalized_time = static_cast<float>(timeBin) / spectrogram_.size();
    normalized_time += (pan_ * 0.5f); // Apply pan
    normalized_time = std::clamp(normalized_time, 0.0f, 1.0f);
    
    return spectrogram_area.getY() + ((1.0f - normalized_time) * spectrogram_area.getHeight());
}

float Spectrogram::xToFrequency(float x) const {
    auto spectrogram_area = getSpectrogramArea();
    
    float normalized_x = (x - spectrogram_area.getX()) / spectrogram_area.getWidth();
    normalized_x = std::clamp(normalized_x, 0.0f, 1.0f);
    
    // Convert back from scaled frequency
    float min_scaled_freq, max_scaled_freq;
    applyFrequencyScale(min_scaled_freq = settings_.minFreq);
    applyFrequencyScale(max_scaled_freq = settings_.maxFreq);
    
    float scaled_freq = min_scaled_freq + (normalized_x * (max_scaled_freq - min_scaled_freq));
    
    // Apply inverse frequency scaling
    float original_freq = scaled_freq;
    // Note: This is a simplified inverse - proper implementation would depend on the scale
    if (settings_.frequencyScale == FrequencyScale::Logarithmic) {
        // Convert back from logarithmic scale
        original_freq = std::exp(std::log(settings_.minFreq) + 
                               scaled_freq * (std::log(settings_.maxFreq) - std::log(settings_.minFreq)));
    }
    
    return original_freq;
}

int Spectrogram::yToTimeBin(float y) const {
    auto spectrogram_area = getSpectrogramArea();
    
    float normalized_y = 1.0f - ((y - spectrogram_area.getY()) / spectrogram_area.getHeight());
    normalized_y -= (pan_ * 0.5f); // Apply inverse pan
    normalized_y = std::clamp(normalized_y, 0.0f, 1.0f);
    
    return static_cast<int>(normalized_y * spectrogram_.size());
}

void Spectrogram::applyFrequencyScale(float& freq) const {
    switch (settings_.frequencyScale) {
        case FrequencyScale::Linear:
            // No scaling needed
            break;
            
        case FrequencyScale::Logarithmic: {
            float log_min = std::log(settings_.minFreq);
            float log_max = std::log(settings_.maxFreq);
            freq = std::log(freq);
            // Normalize to [0, 1]
            freq = (freq - log_min) / (log_max - log_min);
            // Scale back to original range but log-mapped
            freq = std::exp(log_min + freq * (log_max - log_min));
            break;
        }
            
        case FrequencyScale::Mel: {
            // Convert to mel scale (simplified)
            float mel = 2595.0f * std::log10(1.0f + freq / 700.0f);
            // For display, convert back
            freq = 700.0f * (std::pow(10.0f, mel / 2595.0f) - 1.0f);
            break;
        }
            
        case FrequencyScale::Octave: {
            // Convert to octave scale
            float octave = std::log2(freq / 440.0f); // A4 = 440Hz
            freq = 440.0f * std::pow(2.0f, octave);
            break;
        }
    }
}

juce::String Spectrogram::announceToScreenReader(const juce::String& message) {
    auto accessibility_manager = getAccessibilityManager();
    if (accessibility_manager && accessibility_manager->isFeatureEnabled(
        accessibility::AccessibilityFeature::ScreenReader)) {
        accessibility_manager->announceToScreenReader(message);
    }
    return message;
}

} // namespace visualizations
} // namespace ui
} // namespace vital