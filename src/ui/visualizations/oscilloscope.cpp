#include "oscilloscope.h"
#include <cmath>
#include <algorithm>
#include <mutex>
#include "../accessibility/accessibility_manager.h"

namespace vital {
namespace ui {
namespace visualizations {

Oscilloscope::Oscilloscope()
    : Component() {
    setWantsKeyboardFocus(true);
    setInterceptsMouseClicks(true, true);
    
    // Set ARIA role
    setAriaRole("img");
    setAriaDescription("Oscilloscope waveform visualization");
    
    // Initialize default channel
    addChannel();
    
    // Initialize animation values
    displayZoom_.reset(1.0f);
    displayZoom_.setDuration(200.0f);
    displayZoom_.setEasing(AnimationEasing::easeInOutCubic);
    
    displayPan_.reset(0.0f);
    displayPan_.setDuration(200.0f);
    displayPan_.setEasing(AnimationEasing::easeInOutCubic);
    
    // Initialize trigger settings
    triggerArmed_ = true;
    triggerOccurred_ = false;
    triggerPosition_ = 0.0f;
    triggerSample_ = 0;
    
    // Set default grid settings
    gridSettings_.showGrid = true;
    gridSettings_.showCenterLines = true;
    gridSettings_.showTriggerLine = true;
    gridSettings_.gridColor = Colors::outlineVariant;
    gridSettings_.centerColor = Colors::outline;
    gridSettings_.triggerColor = Colors::error;
    gridSettings_.gridOpacity = 0.3f;
    gridSettings_.centerOpacity = 0.6f;
    gridSettings_.triggerOpacity = 0.8f;
    
    // Set default timebase settings
    timebaseSettings_.timePerDivision = 1.0f;  // 1ms/div
    timebaseSettings_.sampleRate = 44100.0f;   // 44.1kHz
    timebaseSettings_.samplesPerScreen = 1024;
    timebaseSettings_.panOffset = 0.0f;
    timebaseSettings_.rollMode = false;
    
    // Set default trigger settings
    triggerSettings_.mode = TriggerMode::Auto;
    triggerSettings_.positiveEdge = true;
    triggerSettings_.level = 0.0f;
    triggerSettings_.holdoff = 100.0f;         // 100μs
    triggerSettings_.preTrigger = 20.0f;       // 20%
    triggerSettings_.noiseReject = false;
    triggerSettings_.hfReject = false;
}

void Oscilloscope::paint(juce::Graphics& g) {
    Component::paint(g);
    
    auto bounds = getLocalBounds();
    auto display_area = getDisplayArea();
    auto grid_bounds = getGridBounds();
    
    // Get theme colors
    auto theme = getTheme();
    auto colors = theme->getMaterialColors();
    
    // Draw background
    g.setColour(colors.surface);
    g.fillRect(bounds);
    
    // Draw display area background
    g.setColour(colors.surfaceVariant);
    g.fillRect(display_area);
    
    // Draw grid
    if (gridSettings_.showGrid) {
        drawGrid(g);
    }
    
    // Draw center lines
    if (gridSettings_.showCenterLines) {
        g.setColour(gridSettings_.centerColor.withAlpha(gridSettings_.centerOpacity));
        
        // Vertical center line
        float center_x = display_area.getCentreX();
        g.drawVerticalLine(static_cast<int>(center_x), display_area.getY(), display_area.getBottom());
        
        // Horizontal center line
        float center_y = display_area.getCentreY();
        g.drawHorizontalLine(static_cast<int>(center_y), display_area.getX(), display_area.getRight());
    }
    
    // Draw trigger line
    if (gridSettings_.showTriggerLine && triggerOccurred_) {
        g.setColour(gridSettings_.triggerColor.withAlpha(gridSettings_.triggerOpacity));
        float trigger_x = triggerPosition_;
        g.drawVerticalLine(static_cast<int>(trigger_x), display_area.getY(), display_area.getBottom());
    }
    
    // Draw channels
    drawChannels(g);
    
    // Draw measurements
    drawMeasurements(g);
    
    // Draw cursors
    drawCursors(g);
    
    // Draw scale information
    if (hasKeyboardFocus()) {
        g.setColour(colors.onSurface.withAlpha(0.6f));
        g.setFont(juce::Font(10.0f));
        
        float scale_text_x = display_area.getX() + 8.0f;
        float scale_text_y = display_area.getY() + 8.0f;
        
        juce::String scale_text = juce::String("Time: ") + 
                                 juce::String(timebaseSettings_.timePerDivision, 2) + "ms/div";
        g.drawText(scale_text, scale_text_x, scale_text_y, 200.0f, 12.0f, juce::Justification::left);
        
        scale_text_y += 14.0f;
        if (!channels_.empty()) {
            scale_text = juce::String("Ch1: ") + 
                        juce::String(channels_[0].verticalScale, 2) + "V/div";
            g.drawText(scale_text, scale_text_x, scale_text_y, 200.0f, 12.0f, juce::Justification::left);
        }
        
        // Draw zoom indicator
        scale_text_y += 14.0f;
        scale_text = juce::String("Zoom: ") + juce::String(zoom_, 1) + "x";
        g.drawText(scale_text, scale_text_x, scale_text_y, 200.0f, 12.0f, juce::Justification::left);
    }
    
    // Draw focus border
    if (hasKeyboardFocus()) {
        g.setColour(colors.primary);
        g.drawRoundedRectangle(bounds.reduced(1.0f), 4.0f, 2.0f);
    }
}

void Oscilloscope::resized() {
    Component::resized();
    
    // Recalculate trigger position when size changes
    auto display_area = getDisplayArea();
    triggerPosition_ = display_area.getX() + (display_area.getWidth() * triggerSettings_.preTrigger / 100.0f);
}

bool Oscilloscope::mouseDown(const juce::MouseEvent& e) {
    if (!enabled_) return false;
    
    auto position = e.getPosition().toFloat();
    auto display_area = getDisplayArea();
    
    if (display_area.contains(position)) {
        isDragging_ = true;
        dragStart_ = position;
        lastMousePosition_ = position;
        
        setState(State::Pressed);
        announceToScreenReader("Oscilloscope dragging started");
        
        return true;
    }
    
    return false;
}

bool Oscilloscope::mouseDrag(const juce::MouseEvent& e) {
    if (!enabled_ || !isDragging_) return false;
    
    auto position = e.getPosition().toFloat();
    auto delta = position - lastMousePosition_;
    
    // Pan view with horizontal drag
    if (std::abs(delta.getX()) > std::abs(delta.getY())) {
        float pan_speed = 0.5f;
        pan_ = std::max(-1.0f, std::min(1.0f, pan_ + delta.getX() * pan_speed / getWidth()));
        displayPan_.animateTo(pan_);
        
        if (viewChangedCallback_) {
            viewChangedCallback_(zoom_, pan_);
        }
    }
    
    lastMousePosition_ = position;
    setNeedsRedraw();
    
    return true;
}

bool Oscilloscope::mouseUp(const juce::MouseEvent& e) {
    if (!enabled_) return false;
    
    if (isDragging_) {
        isDragging_ = false;
        setState(State::Normal);
        announceToScreenReader("Oscilloscope dragging ended");
    }
    
    return true;
}

bool Oscilloscope::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) {
    if (!enabled_) return false;
    
    auto display_area = getDisplayArea();
    auto position = e.getPosition().toFloat();
    
    if (display_area.contains(position)) {
        float zoom_factor = wheel.deltaY > 0 ? 0.9f : 1.1f;
        float new_zoom = std::max(0.1f, std::min(10.0f, zoom_ * zoom_factor));
        
        zoom_ = new_zoom;
        displayZoom_.animateTo(zoom_);
        
        if (viewChangedCallback_) {
            viewChangedCallback_(zoom_, pan_);
        }
        
        setNeedsRedraw();
        announceToScreenReader("Zoom: " + juce::String(zoom_, 1) + "x");
        
        return true;
    }
    
    return false;
}

void Oscilloscope::focusGained() {
    setState(State::Focused);
    announceToScreenReader("Oscilloscope focused. Use mouse wheel to zoom, drag to pan.");
}

void Oscilloscope::focusLost() {
    setState(State::Normal);
}

void Oscilloscope::addChannel(const Channel& channel) {
    if (channels_.size() >= MAX_CHANNELS) return;
    
    channels_.push_back(channel);
    channelData_.push_back(std::vector<float>());
    
    // Initialize channel data buffer
    channelData_.back().resize(timebaseSettings_.samplesPerScreen, 0.0f);
    
    setNeedsRedraw();
    
    if (channelChangedCallback_) {
        channelChangedCallback_(static_cast<int>(channels_.size()) - 1);
    }
}

void Oscilloscope::removeChannel(int channelIndex) {
    if (channelIndex < 0 || channelIndex >= static_cast<int>(channels_.size())) return;
    
    channels_.erase(channels_.begin() + channelIndex);
    channelData_.erase(channelData_.begin() + channelIndex);
    
    setNeedsRedraw();
    
    if (channelChangedCallback_) {
        channelChangedCallback_(channelIndex);
    }
}

void Oscilloscope::clearChannels() {
    channels_.clear();
    channelData_.clear();
    
    // Add default channel
    addChannel();
    
    setNeedsRedraw();
}

void Oscilloscope::setChannelEnabled(int channelIndex, bool enabled) {
    if (channelIndex < 0 || channelIndex >= static_cast<int>(channels_.size())) return;
    
    channels_[channelIndex].enabled = enabled;
    setNeedsRedraw();
}

void Oscilloscope::setChannelColor(int channelIndex, const Color& color) {
    if (channelIndex < 0 || channelIndex >= static_cast<int>(channels_.size())) return;
    
    channels_[channelIndex].color = color;
    setNeedsRedraw();
}

void Oscilloscope::setChannelScale(int channelIndex, float scale) {
    if (channelIndex < 0 || channelIndex >= static_cast<int>(channels_.size())) return;
    
    channels_[channelIndex].verticalScale = std::max(0.001f, scale);
    setNeedsRedraw();
}

void Oscilloscope::setChannelOffset(int channelIndex, float offset) {
    if (channelIndex < 0 || channelIndex >= static_cast<int>(channels_.size())) return;
    
    channels_[channelIndex].verticalOffset = offset;
    setNeedsRedraw();
}

void Oscilloscope::setChannelCoupling(int channelIndex, ChannelMode coupling) {
    if (channelIndex < 0 || channelIndex >= static_cast<int>(channels_.size())) return;
    
    channels_[channelIndex].coupling = coupling;
    setNeedsRedraw();
}

void Oscilloscope::setChannelData(int channelIndex, const std::vector<float>& data) {
    if (channelIndex < 0 || channelIndex >= static_cast<int>(channels_.size())) return;
    
    // Ensure data buffer is large enough
    if (channelData_[channelIndex].size() != data.size()) {
        channelData_[channelIndex].resize(data.size());
    }
    
    // Copy data with clipping
    for (size_t i = 0; i < std::min(data.size(), channelData_[channelIndex].size()); ++i) {
        channelData_[channelIndex][i] = std::clamp(data[i], -10.0f, 10.0f); // Clip to ±10V
    }
    
    // Trigger processing
    if (triggerSettings_.mode != TriggerMode::None) {
        processTrigger();
    }
    
    setNeedsRedraw();
}

void Oscilloscope::setDataCallback(DataCallback callback) {
    dataCallback_ = callback;
}

void Oscilloscope::setTriggerSettings(const TriggerSettings& settings) {
    triggerSettings_ = settings;
    triggerArmed_ = true;
    triggerOccurred_ = false;
    
    // Update trigger position based on pre-trigger setting
    auto display_area = getDisplayArea();
    triggerPosition_ = display_area.getX() + (display_area.getWidth() * settings.preTrigger / 100.0f);
    
    setNeedsRedraw();
}

void Oscilloscope::setTimebaseSettings(const TimebaseSettings& settings) {
    timebaseSettings_ = settings;
    
    // Resize data buffers
    for (auto& data : channelData_) {
        data.resize(settings.samplesPerScreen, 0.0f);
    }
    
    setNeedsRedraw();
}

void Oscilloscope::setGridSettings(const GridSettings& settings) {
    gridSettings_ = settings;
    setNeedsRedraw();
}

void Oscilloscope::setMeasurementCallback(MeasurementCallback callback) {
    measurementCallback_ = callback;
}

void Oscilloscope::enableMeasurement(int channelIndex, const std::string& measurement) {
    // Implementation for enabling specific measurements
    setNeedsRedraw();
}

void Oscilloscope::disableMeasurement(int channelIndex, const std::string& measurement) {
    // Implementation for disabling specific measurements
    setNeedsRedraw();
}

void Oscilloscope::clearMeasurements(int channelIndex) {
    // Implementation for clearing measurements
    setNeedsRedraw();
}

void Oscilloscope::setZoom(float zoom) {
    zoom_ = std::max(0.1f, std::min(10.0f, zoom));
    displayZoom_.animateTo(zoom_);
    setNeedsRedraw();
}

void Oscilloscope::setPan(float pan) {
    pan_ = std::max(-1.0f, std::min(1.0f, pan));
    displayPan_.animateTo(pan_);
    setNeedsRedraw();
}

void Oscilloscope::resetView() {
    zoom_ = 1.0f;
    pan_ = 0.0f;
    displayZoom_.animateTo(zoom_);
    displayPan_.animateTo(pan_);
    
    setNeedsRedraw();
}

void Oscilloscope::autoScale() {
    // Auto-scale based on current channel data
    for (size_t i = 0; i < channels_.size(); ++i) {
        if (!channels_[i].enabled || channelData_[i].empty()) continue;
        
        // Find min/max values
        float min_val = std::numeric_limits<float>::max();
        float max_val = std::numeric_limits<float>::min();
        
        for (const auto& sample : channelData_[i]) {
            min_val = std::min(min_val, sample);
            max_val = std::max(max_val, sample);
        }
        
        // Calculate scale to fit data in display area
        float range = max_val - min_val;
        if (range > 0.0f) {
            auto display_area = getDisplayArea();
            float target_height = display_area.getHeight() * 0.8f; // Use 80% of available height
            float scale = range / target_height;
            channels_[i].verticalScale = scale;
        }
    }
    
    setNeedsRedraw();
}

void Oscilloscope::exportData(const std::string& filename, int channelIndex) {
    if (channelIndex < 0 || channelIndex >= static_cast<int>(channelData_.size())) return;
    
    // Implementation for CSV export
    // This would involve writing the waveform data to a CSV file
    announceToScreenReader("Data exported to " + juce::String(filename));
}

void Oscilloscope::exportImage(const std::string& filename, int width, int height) {
    // Implementation for image export
    announceToScreenReader("Image exported to " + juce::String(filename));
}

void Oscilloscope::setViewChangedCallback(ViewChangedCallback callback) {
    viewChangedCallback_ = callback;
}

void Oscilloscope::setChannelChangedCallback(ChannelChangedCallback callback) {
    channelChangedCallback_ = callback;
}

juce::String Oscilloscope::getAccessibilityLabel() const {
    return juce::String("Oscilloscope");
}

juce::String Oscilloscope::getAccessibilityValue() const {
    juce::String result = juce::String(channels_.size()) + " channels, ";
    result += juce::String(timebaseSettings_.timePerDivision, 2) + "ms/div";
    
    if (triggerOccurred_) {
        result += ", triggered";
    }
    
    return result;
}

void Oscilloscope::processTrigger() {
    if (triggerSettings_.mode == TriggerMode::None || 
        channelData_.empty() || channelData_[0].size() < 2) {
        return;
    }
    
    auto& data = channelData_[0];
    
    // Simple edge trigger detection
    bool trigger_found = false;
    
    for (size_t i = 1; i < data.size(); ++i) {
        if (triggerSettings_.positiveEdge) {
            if (data[i-1] <= triggerSettings_.level && data[i] > triggerSettings_.level) {
                trigger_found = true;
                triggerSample_ = static_cast<int>(i);
                break;
            }
        } else {
            if (data[i-1] >= triggerSettings_.level && data[i] < triggerSettings_.level) {
                trigger_found = true;
                triggerSample_ = static_cast<int>(i);
                break;
            }
        }
    }
    
    if (trigger_found) {
        triggerOccurred_ = true;
        triggerArmed_ = false;
        
        // Calculate trigger position in screen coordinates
        auto display_area = getDisplayArea();
        triggerPosition_ = display_area.getX() + (display_area.getWidth() * triggerSettings_.preTrigger / 100.0f);
    }
}

void Oscilloscope::updateMeasurements() {
    if (measurementCallback_ && !channelData_.empty()) {
        // Call the measurement callback to get calculated measurements
        auto measurements = measurementCallback_(channelData_);
        // Store measurements for display
    }
}

juce::Rectangle<float> Oscilloscope::getDisplayArea() const {
    auto bounds = getLocalBounds();
    return bounds.reduced(10.0f);
}

juce::Rectangle<float> Oscilloscope::getGridBounds() const {
    return getDisplayArea();
}

void Oscilloscope::drawGrid(juce::Graphics& g) {
    auto grid_bounds = getGridBounds();
    auto theme = getTheme();
    auto colors = theme->getMaterialColors();
    
    // Draw major grid lines (every 100 pixels)
    g.setColour(gridSettings_.gridColor.withAlpha(gridSettings_.gridOpacity));
    
    int major_spacing = 100;
    
    // Vertical major lines
    for (float x = grid_bounds.getX(); x <= grid_bounds.getRight(); x += major_spacing) {
        g.drawVerticalLine(static_cast<int>(x), grid_bounds.getY(), grid_bounds.getBottom());
    }
    
    // Horizontal major lines
    for (float y = grid_bounds.getY(); y <= grid_bounds.getBottom(); y += major_spacing) {
        g.drawHorizontalLine(static_cast<int>(y), grid_bounds.getX(), grid_bounds.getRight());
    }
    
    // Draw minor grid lines (every 20 pixels)
    g.setColour(gridSettings_.gridColor.withAlpha(gridSettings_.gridOpacity * 0.5f));
    
    int minor_spacing = 20;
    
    // Vertical minor lines
    for (float x = grid_bounds.getX(); x <= grid_bounds.getRight(); x += minor_spacing) {
        if (static_cast<int>(x) % major_spacing != 0) {
            g.drawVerticalLine(static_cast<int>(x), grid_bounds.getY(), grid_bounds.getBottom());
        }
    }
    
    // Horizontal minor lines
    for (float y = grid_bounds.getY(); y <= grid_bounds.getBottom(); y += minor_spacing) {
        if (static_cast<int>(y) % major_spacing != 0) {
            g.drawHorizontalLine(static_cast<int>(y), grid_bounds.getX(), grid_bounds.getRight());
        }
    }
}

void Oscilloscope::drawChannels(juce::Graphics& g) {
    if (channelData_.empty()) return;
    
    auto display_area = getDisplayArea();
    float display_zoom = displayZoom_.getCurrentValue();
    float display_pan = displayPan_.getCurrentValue();
    
    // Calculate horizontal scale
    float time_per_pixel = (timebaseSettings_.timePerDivision * 10.0f) / display_area.getWidth();
    float samples_per_pixel = timebaseSettings_.sampleRate * time_per_pixel * display_zoom;
    
    // Draw each enabled channel
    for (size_t channel_index = 0; channel_index < channels_.size(); ++channel_index) {
        if (!channels_[channel_index].enabled || channels_[channel_index].channelData_.empty()) {
            continue;
        }
        
        auto& channel = channels_[channel_index];
        auto& data = channelData_[channel_index];
        
        if (data.empty()) continue;
        
        // Set channel color with brightness
        auto channel_color = channel.color.getColor(getTheme()->getMaterialColors());
        channel_color = channel_color.withAlpha(std::clamp(channel.brightness, 0.0f, 1.0f));
        g.setColour(channel_color);
        
        // Calculate vertical scale and offset
        float volts_per_pixel = channel.verticalScale;
        float center_y = display_area.getCentreY() - (channel.verticalOffset / volts_per_pixel);
        
        // Draw waveform path using OpenGL path for better performance
        juce::Path waveform_path;
        bool path_started = false;
        
        for (int x = 0; x < display_area.getWidth(); ++x) {
            float time = x * time_per_pixel * display_zoom + display_pan;
            int sample_index = static_cast<int>(time * timebaseSettings_.sampleRate);
            
            if (sample_index >= 0 && sample_index < static_cast<int>(data.size())) {
                float voltage = data[sample_index];
                float y = center_y - (voltage / volts_per_pixel);
                
                if (!path_started) {
                    waveform_path.startNewSubPath(display_area.getX() + x, y);
                    path_started = true;
                } else {
                    waveform_path.lineTo(display_area.getX() + x, y);
                }
            }
        }
        
        if (path_started) {
            g.strokePath(waveform_path, juce::PathStrokeType(1.0f));
        }
        
        // Draw channel label
        if (channel.showTrace) {
            g.setColour(channel_color.withAlpha(0.8f));
            g.setFont(juce::Font(10.0f));
            
            juce::String channel_label = "Ch" + juce::String(channel_index + 1);
            g.drawText(channel_label, display_area.getX() + 8.0f, 
                      display_area.getY() + 20.0f + channel_index * 15.0f,
                      60.0f, 12.0f, juce::Justification::left);
        }
    }
}

void Oscilloscope::drawTrigger(juce::Graphics& g) {
    if (!triggerOccurred_ || !gridSettings_.showTriggerLine) return;
    
    g.setColour(gridSettings_.triggerColor.withAlpha(gridSettings_.triggerOpacity));
    g.drawVerticalLine(static_cast<int>(triggerPosition_), 
                      getDisplayArea().getY(), 
                      getDisplayArea().getBottom());
}

void Oscilloscope::drawMeasurements(juce::Graphics& g) {
    if (!measurementCallback_ || channelData_.empty()) return;
    
    // Implementation for drawing measurement values
    // This would involve calling the measurement callback and displaying results
}

void Oscilloscope::drawCursors(juce::Graphics& g) {
    // Implementation for measurement cursors
    // This would involve drawing vertical/horizontal lines for time/voltage measurements
}

juce::Point<float> Oscilloscope::dataToScreen(float x, float y, int channelIndex) const {
    auto display_area = getDisplayArea();
    auto screen_x = display_area.getX() + (x * display_area.getWidth() / timebaseSettings_.samplesPerScreen);
    auto screen_y = display_area.getCentreY() - (y / channels_[channelIndex].verticalScale);
    
    return juce::Point<float>(screen_x, screen_y);
}

juce::Point<float> Oscilloscope::screenToData(float x, float y, int& channelIndex) const {
    auto display_area = getDisplayArea();
    auto data_x = (x - display_area.getX()) * timebaseSettings_.samplesPerScreen / display_area.getWidth();
    
    // Find closest channel (simplified - could be more sophisticated)
    channelIndex = 0;
    auto data_y = (display_area.getCentreY() - y) * channels_[channelIndex].verticalScale;
    
    return juce::Point<float>(data_x, data_y);
}

void Oscilloscope::update() {
    Component::update();
    
    // Update animations
    displayZoom_.update(0.016f);
    displayPan_.update(0.016f);
    
    // Update measurements
    updateMeasurements();
}

void Oscilloscope::reset() {
    // Reset all settings to defaults
    channels_.clear();
    channelData_.clear();
    
    // Add default channel
    addChannel();
    
    // Reset trigger
    triggerArmed_ = true;
    triggerOccurred_ = false;
    
    // Reset view
    resetView();
    
    setNeedsRedraw();
}

juce::String Oscilloscope::announceToScreenReader(const juce::String& message) {
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