#include "waveform_viewer.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <mutex>
#include "../accessibility/accessibility_manager.h"

namespace vital {
namespace ui {
namespace visualizations {

WaveformViewer::WaveformViewer(const WaveformSettings& settings)
    : Component(), settings_(settings) {
    setWantsKeyboardFocus(true);
    setInterceptsMouseClicks(true, true);
    
    // Set ARIA role
    setAriaRole("img");
    setAriaDescription("Audio waveform visualization");
    
    // Initialize time per sample
    timePerSample_ = 1.0f / settings_.sampleRate;
    
    // Initialize selection
    selection_.active = false;
    
    // Initialize animation values for smooth zoom/pan
    // These would be used for smooth transitions
}

void WaveformViewer::paint(juce::Graphics& g) {
    Component::paint(g);
    
    auto bounds = getLocalBounds();
    auto display_area = getDisplayArea();
    
    // Get theme colors
    auto theme = getTheme();
    auto colors = theme->getMaterialColors();
    
    // Draw background
    g.setColour(settings_.backgroundColor.getColor(colors));
    g.fillRect(bounds);
    
    // Draw grid
    if (settings_.showGrid) {
        drawGrid(g);
    }
    
    // Draw center line
    if (settings_.showCenterLine) {
        drawCenterLine(g);
    }
    
    // Draw waveforms for each channel
    for (size_t i = 0; i < channels_.size(); ++i) {
        if (channels_[i].visible && !channels_[i].data.empty()) {
            g.saveState();
            
            // Apply channel opacity
            float opacity = channels_[i].opacity * settings_.antiAliased ? 1.0f : 0.8f;
            g.setColour(channels_[i].color.getColor(colors).withAlpha(opacity));
            
            drawWaveform(g, channels_[i]);
            
            g.restoreState();
        }
    }
    
    // Draw selection
    if (selection_.active) {
        drawSelection(g);
    }
    
    // Draw playback markers
    drawMarkers(g);
    
    // Draw cursor
    if (settings_.showCursor) {
        drawCursor(g);
    }
    
    // Draw time scale
    drawTimeScale(g);
    
    // Draw amplitude scale
    drawAmplitudeScale(g);
    
    // Draw focus indicator
    if (hasKeyboardFocus()) {
        g.setColour(colors.primary);
        g.drawRoundedRectangle(bounds.reduced(1.0f), 4.0f, 2.0f);
    }
}

void WaveformViewer::resized() {
    Component::resized();
    
    // Update downsampled data for new display area
    updateDownsampledData();
}

bool WaveformViewer::mouseDown(const juce::MouseEvent& e) {
    if (!enabled_) return false;
    
    auto position = e.getPosition().toFloat();
    auto display_area = getDisplayArea();
    
    if (display_area.contains(position)) {
        // Check if this is a selection drag or cursor positioning
        if (e.mods.isShiftDown()) {
            isSelecting_ = true;
            dragStart_ = position;
            
            // Start or extend selection
            auto time_info = screenToTime(position);
            float current_time = time_info.first;
            
            if (!selection_.active) {
                selection_.start = current_time;
                selection_.end = current_time;
                selection_.active = true;
            } else {
                selection_.end = current_time;
            }
            
            announceToScreenReader("Selection started");
        } else if (e.mods.isAltDown()) {
            // Set cursor position
            auto time_info = screenToTime(position);
            setCursorPosition(time_info.first);
            
            announceToScreenReader("Cursor position: " + juce::String(time_info.first, 3) + " seconds");
        } else {
            // Start pan drag
            isDragging_ = true;
            dragStart_ = position;
            lastMousePosition_ = position;
        }
        
        setState(State::Pressed);
        return true;
    }
    
    return false;
}

bool WaveformViewer::mouseDrag(const juce::MouseEvent& e) {
    if (!enabled_) return false;
    
    auto position = e.getPosition().toFloat();
    
    if (isSelecting_) {
        auto time_info = screenToTime(position);
        selection_.end = time_info.first;
        
        // Ensure selection is valid (start <= end)
        if (selection_.start > selection_.end) {
            std::swap(selection_.start, selection_.end);
        }
        
        setNeedsRedraw();
        
        if (selectionCallback_) {
            selectionCallback_(selection_);
        }
        
        announceToScreenReader("Selection: " + 
                              juce::String(std::abs(selection_.end - selection_.start), 3) + " seconds");
        
        return true;
    } else if (isDragging_) {
        auto delta = position - lastMousePosition_;
        
        // Pan view
        float pan_speed = 0.001f; // Adjust sensitivity
        pan_ = std::max(-1.0f, std::min(1.0f, pan_ + delta.getX() * pan_speed));
        
        lastMousePosition_ = position;
        setNeedsRedraw();
        
        return true;
    }
    
    return false;
}

bool WaveformViewer::mouseUp(const juce::MouseEvent& e) {
    if (!enabled_) return false;
    
    if (isSelecting_ || isDragging_) {
        isSelecting_ = false;
        isDragging_ = false;
        setState(State::Normal);
        
        if (isSelecting_) {
            announceToScreenReader("Selection complete: " + 
                                  juce::String(std::abs(selection_.end - selection_.start), 3) + " seconds selected");
        } else {
            announceToScreenReader("Pan complete");
        }
    }
    
    return true;
}

bool WaveformViewer::mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) {
    if (!enabled_) return false;
    
    auto display_area = getDisplayArea();
    auto position = e.getPosition().toFloat();
    
    if (display_area.contains(position)) {
        float zoom_factor = wheel.deltaY > 0 ? 0.9f : 1.1f;
        float new_zoom = std::max(0.1f, std::min(10.0f, zoom_ * zoom_factor));
        
        // Zoom around mouse position
        auto time_info = screenToTime(position);
        float zoom_point_time = time_info.first;
        
        // Adjust pan to keep zoom point in place
        float time_per_pixel_new = settings_.timePerPixel / new_zoom;
        float pan_adjustment = (zoom_point_time - (pan_ * getDuration())) * 0.5f;
        
        zoom_ = new_zoom;
        pan_ = std::max(-1.0f, std::min(1.0f, pan_ + pan_adjustment));
        
        setNeedsRedraw();
        announceToScreenReader("Zoom: " + juce::String(zoom_, 1) + "x");
        
        return true;
    }
    
    return false;
}

void WaveformViewer::focusGained() {
    setState(State::Focused);
    announceToScreenReader("Waveform viewer focused. Use mouse wheel to zoom, drag to pan. Shift+drag to select, Alt+click to set cursor.");
}

void WaveformViewer::focusLost() {
    setState(State::Normal);
}

void WaveformViewer::setAudioData(const std::vector<float>& data, int channel) {
    if (channel < 0) return;
    
    // Ensure channel exists
    while (static_cast<int>(channels_.size()) <= channel) {
        Channel new_channel;
        new_channel.name = "Channel " + std::to_string(channels_.size() + 1);
        new_channel.color = Colors::primary;
        new_channel.visible = true;
        new_channel.opacity = 1.0f;
        channels_.push_back(new_channel);
    }
    
    // Set audio data
    channels_[channel].data = data;
    
    // Update time per sample
    if (!data.empty()) {
        timePerSample_ = 1.0f / settings_.sampleRate;
        updateDownsampledData();
    }
    
    setNeedsRedraw();
}

void WaveformViewer::setAudioData(const std::vector<std::vector<float>>& data) {
    channels_.clear();
    channels_.reserve(data.size());
    
    for (size_t i = 0; i < data.size(); ++i) {
        Channel channel;
        channel.name = "Channel " + std::to_string(i + 1);
        channel.data = data[i];
        channel.color = Colors::primary;
        channel.visible = true;
        channel.opacity = 1.0f;
        channels_.push_back(channel);
    }
    
    updateDownsampledData();
    setNeedsRedraw();
}

void WaveformViewer::clearAudioData() {
    channels_.clear();
    downsampledData_.clear();
    selection_.active = false;
    setNeedsRedraw();
}

void WaveformViewer::addChannel(const Channel& channel) {
    channels_.push_back(channel);
    setNeedsRedraw();
}

void WaveformViewer::removeChannel(int index) {
    if (index >= 0 && index < static_cast<int>(channels_.size())) {
        channels_.erase(channels_.begin() + index);
        setNeedsRedraw();
    }
}

void WaveformViewer::setChannelData(int index, const std::vector<float>& data) {
    if (index >= 0 && index < static_cast<int>(channels_.size())) {
        channels_[index].data = data;
        updateDownsampledData();
        setNeedsRedraw();
    }
}

void WaveformViewer::setChannelVisible(int index, bool visible) {
    if (index >= 0 && index < static_cast<int>(channels_.size())) {
        channels_[index].visible = visible;
        setNeedsRedraw();
    }
}

void WaveformViewer::setChannelColor(int index, const Color& color) {
    if (index >= 0 && index < static_cast<int>(channels_.size())) {
        channels_[index].color = color;
        setNeedsRedraw();
    }
}

void WaveformViewer::setChannelName(int index, const std::string& name) {
    if (index >= 0 && index < static_cast<int>(channels_.size())) {
        channels_[index].name = name;
        setNeedsRedraw();
    }
}

void WaveformViewer::addPlaybackMarker(const PlaybackMarker& marker) {
    markers_.push_back(marker);
    setNeedsRedraw();
}

void WaveformViewer::removePlaybackMarker(int index) {
    if (index >= 0 && index < static_cast<int>(markers_.size())) {
        markers_.erase(markers_.begin() + index);
        setNeedsRedraw();
    }
}

void WaveformViewer::clearPlaybackMarkers() {
    markers_.clear();
    setNeedsRedraw();
}

void WaveformViewer::setMarkerPosition(int index, float position) {
    if (index >= 0 && index < static_cast<int>(markers_.size())) {
        markers_[index].position = std::max(0.0f, position);
        setNeedsRedraw();
    }
}

void WaveformViewer::setMarkerVisible(int index, bool visible) {
    if (index >= 0 && index < static_cast<int>(markers_.size())) {
        markers_[index].visible = visible;
        setNeedsRedraw();
    }
}

void WaveformViewer::setSelection(const Selection& selection) {
    selection_ = selection;
    setNeedsRedraw();
}

void WaveformViewer::clearSelection() {
    selection_.active = false;
    setNeedsRedraw();
}

void WaveformViewer::setSelectionActive(bool active) {
    selection_.active = active;
    setNeedsRedraw();
}

void WaveformViewer::setSettings(const WaveformSettings& settings) {
    settings_ = settings;
    timePerSample_ = 1.0f / settings_.sampleRate;
    updateDownsampledData();
    setNeedsRedraw();
}

void WaveformViewer::setTimePerPixel(float timePerPixel) {
    settings_.timePerPixel = std::max(0.0001f, timePerPixel);
    setNeedsRedraw();
}

void WaveformViewer::setVerticalScale(float scale) {
    settings_.verticalScale = std::max(0.001f, scale);
    setNeedsRedraw();
}

void WaveformViewer::setZoom(float zoom) {
    zoom_ = std::max(0.1f, std::min(10.0f, zoom));
    setNeedsRedraw();
}

void WaveformViewer::setPan(float pan) {
    pan_ = std::max(-1.0f, std::min(1.0f, pan));
    setNeedsRedraw();
}

void WaveformViewer::resetView() {
    zoom_ = 1.0f;
    pan_ = 0.0f;
    setNeedsRedraw();
}

void WaveformViewer::centerOnTime(float time) {
    auto display_area = getDisplayArea();
    float total_duration = getDuration();
    float visible_duration = settings_.timePerPixel * display_area.getWidth() / zoom_;
    
    pan_ = (time - visible_duration * 0.5f) / total_duration;
    pan_ = std::max(-1.0f, std::min(1.0f, pan_));
    
    setNeedsRedraw();
}

void WaveformViewer::fitToWindow() {
    if (channels_.empty() || channels_[0].data.empty()) return;
    
    auto display_area = getDisplayArea();
    float total_duration = getDuration();
    
    settings_.timePerPixel = total_duration / display_area.getWidth();
    zoom_ = 1.0f;
    pan_ = 0.0f;
    
    setNeedsRedraw();
}

void WaveformViewer::setCursorPosition(float position) {
    cursorPosition_ = std::max(0.0f, position);
    
    if (cursorCallback_) {
        cursorCallback_(cursorPosition_);
    }
    
    setNeedsRedraw();
}

void WaveformViewer::setTimeRange(float start, float end) {
    if (start < end) {
        auto display_area = getDisplayArea();
        float total_duration = getDuration();
        float new_time_per_pixel = (end - start) / display_area.getWidth();
        
        settings_.timePerPixel = new_time_per_pixel;
        pan_ = (start / total_duration) * 2.0f - 1.0f; // Convert to pan range
    }
    
    setNeedsRedraw();
}

std::pair<float, float> WaveformViewer::getTimeRange() const {
    auto display_area = getDisplayArea();
    float total_duration = getDuration();
    
    float visible_duration = settings_.timePerPixel * display_area.getWidth() / zoom_;
    float start_time = (-pan_ * total_duration * 0.5f) + visible_duration * pan_;
    float end_time = start_time + visible_duration;
    
    return {start_time, end_time};
}

float WaveformViewer::getPeakLevel(int channel) const {
    if (channel < 0 || channel >= static_cast<int>(channels_.size()) || 
        channels_[channel].data.empty()) {
        return 0.0f;
    }
    
    float peak = 0.0f;
    for (const auto& sample : channels_[channel].data) {
        peak = std::max(peak, std::abs(sample));
    }
    
    return peak;
}

float WaveformViewer::getRMSLevel(float start, float end, int channel) const {
    if (channel < 0 || channel >= static_cast<int>(channels_.size()) || 
        channels_[channel].data.empty()) {
        return 0.0f;
    }
    
    auto time_range = getTimeRange();
    if (start < time_range.first) start = time_range.first;
    if (end > time_range.second) end = time_range.second;
    
    // Calculate RMS for the selected time range
    float rms_sum = 0.0f;
    int sample_count = 0;
    
    int start_sample = static_cast<int>(start / timePerSample_);
    int end_sample = static_cast<int>(end / timePerSample_);
    
    start_sample = std::max(0, std::min(start_sample, static_cast<int>(channels_[channel].data.size()) - 1));
    end_sample = std::max(0, std::min(end_sample, static_cast<int>(channels_[channel].data.size())));
    
    for (int i = start_sample; i < end_sample; ++i) {
        float sample = channels_[channel].data[i];
        rms_sum += sample * sample;
        sample_count++;
    }
    
    return sample_count > 0 ? std::sqrt(rms_sum / sample_count) : 0.0f;
}

float WaveformViewer::getDuration() const {
    if (channels_.empty() || channels_[0].data.empty()) {
        return 0.0f;
    }
    
    return channels_[0].data.size() * timePerSample_;
}

void WaveformViewer::setSelectionCallback(SelectionCallback callback) {
    selectionCallback_ = callback;
}

void WaveformViewer::setCursorCallback(CursorCallback callback) {
    cursorCallback_ = callback;
}

void WaveformViewer::setPlaybackCallback(PlaybackCallback callback) {
    playbackCallback_ = callback;
}

void WaveformViewer::exportSelection(const std::string& filename) {
    if (!selection_.active) return;
    
    // Implementation would export the selected audio portion
    announceToScreenReader("Selection exported to " + juce::String(filename));
}

void WaveformViewer::exportImage(const std::string& filename, int width, int height) {
    // Implementation would export the current view as an image
    announceToScreenReader("Image exported to " + juce::String(filename));
}

juce::String WaveformViewer::getAccessibilityLabel() const {
    return juce::String("Waveform viewer");
}

void WaveformViewer::updateDownsampledData() {
    downsampledData_.clear();
    downsampledData_.reserve(channels_.size());
    
    for (const auto& channel : channels_) {
        std::vector<float> downsampled;
        computeDownsampledData(channel.data, downsampled);
        downsampledData_.push_back(downsampled);
    }
}

void WaveformViewer::computeDownsampledData(const std::vector<float>& data, std::vector<float>& output) {
    if (data.empty()) {
        output.clear();
        return;
    }
    
    auto display_area = getDisplayArea();
    int target_points = std::min(settings_.maxPoints, display_area.getWidth());
    
    if (target_points >= static_cast<int>(data.size())) {
        // No downsampling needed
        output = data;
        return;
    }
    
    output.resize(target_points);
    int data_size = static_cast<int>(data.size());
    float samples_per_point = static_cast<float>(data_size) / target_points;
    
    for (int i = 0; i < target_points; ++i) {
        int start_sample = static_cast<int>(i * samples_per_point);
        int end_sample = static_cast<int>((i + 1) * samples_per_point);
        
        start_sample = std::max(0, std::min(start_sample, data_size));
        end_sample = std::max(0, std::min(end_sample, data_size));
        
        if (start_sample >= end_sample) {
            output[i] = 0.0f;
            continue;
        }
        
        // Calculate peak amplitude in this range
        float peak = 0.0f;
        for (int j = start_sample; j < end_sample; ++j) {
            peak = std::max(peak, std::abs(data[j]));
        }
        
        output[i] = peak;
    }
}

juce::Rectangle<float> WaveformViewer::getDisplayArea() const {
    auto bounds = getLocalBounds();
    return bounds.reduced(20.0f); // Leave space for scales
}

juce::Point<float> WaveformViewer::timeToScreen(float time, float amplitude, int channel) const {
    auto display_area = getDisplayArea();
    auto time_range = getTimeRange();
    
    float normalized_time = (time - time_range.first) / (time_range.second - time_range.first);
    float screen_x = display_area.getX() + (normalized_time * display_area.getWidth());
    
    float normalized_amplitude = (amplitude + 1.0f) * 0.5f; // Convert from [-1,1] to [0,1]
    float screen_y = display_area.getY() + (1.0f - normalized_amplitude) * display_area.getHeight();
    
    return juce::Point<float>(screen_x, screen_y);
}

std::pair<float, float> WaveformViewer::screenToTime(const juce::Point<float>& position) const {
    auto display_area = getDisplayArea();
    auto time_range = getTimeRange();
    
    float normalized_x = (position.getX() - display_area.getX()) / display_area.getWidth();
    normalized_x = std::clamp(normalized_x, 0.0f, 1.0f);
    
    float time = time_range.first + (normalized_x * (time_range.second - time_range.first));
    
    // Calculate amplitude (simplified)
    float normalized_y = 1.0f - ((position.getY() - display_area.getY()) / display_area.getHeight());
    float amplitude = (normalized_y * 2.0f) - 1.0f; // Convert from [0,1] to [-1,1]
    
    return {time, amplitude};
}

void WaveformViewer::drawWaveform(juce::Graphics& g, const Channel& channel) {
    auto display_area = getDisplayArea();
    auto time_range = getTimeRange();
    
    if (channel.data.empty() || downsampledData_.empty()) return;
    
    // Find the corresponding downsampled data
    size_t channel_index = &channel - &channels_[0];
    if (channel_index >= downsampledData_.size()) return;
    
    const auto& downsampled = downsampledData_[channel_index];
    if (downsampled.empty()) return;
    
    // Draw waveform as filled area
    juce::Path waveform_path;
    bool path_started = false;
    
    for (int x = 0; x < display_area.getWidth(); ++x) {
        float normalized_x = static_cast<float>(x) / display_area.getWidth();
        int sample_index = static_cast<int>(normalized_x * downsampled.size());
        sample_index = std::clamp(sample_index, 0, static_cast<int>(downsampled.size()) - 1);
        
        float amplitude = downsampled[sample_index];
        float screen_y = display_area.getCentreY() - (amplitude * display_area.getHeight() * 0.8f);
        
        if (!path_started) {
            waveform_path.startNewSubPath(display_area.getX() + x, display_area.getCentreY());
            path_started = true;
        }
        
        waveform_path.lineTo(display_area.getX() + x, screen_y);
    }
    
    if (path_started) {
        waveform_path.lineTo(display_area.getRight(), display_area.getCentreY());
        waveform_path.closeSubPath();
        
        g.fillPath(waveform_path);
        
        // Draw outline
        g.strokePath(waveform_path, juce::PathStrokeType(1.0f));
    }
}

void WaveformViewer::drawGrid(juce::Graphics& g) {
    auto display_area = getDisplayArea();
    auto theme = getTheme();
    auto colors = theme->getMaterialColors();
    
    // Draw time grid lines
    auto time_range = getTimeRange();
    float total_time = time_range.second - time_range.first;
    float grid_spacing = std::pow(10, std::floor(std::log10(total_time / 10))); // Adaptive spacing
    
    g.setColour(settings_.gridColor.getColor(colors).withAlpha(settings_.gridOpacity));
    
    // Vertical lines
    float start_time = std::floor(time_range.first / grid_spacing) * grid_spacing;
    for (float t = start_time; t <= time_range.second; t += grid_spacing) {
        auto screen_pos = timeToScreen(t, 0.0f);
        if (screen_pos.getX() >= display_area.getX() && screen_pos.getX() <= display_area.getRight()) {
            g.drawVerticalLine(static_cast<int>(screen_pos.getX()), 
                              display_area.getY(), display_area.getBottom());
        }
    }
    
    // Horizontal lines
    for (float a = -1.0f; a <= 1.0f; a += 0.2f) {
        auto screen_pos = timeToScreen(time_range.first, a);
        if (screen_pos.getY() >= display_area.getY() && screen_pos.getY() <= display_area.getBottom()) {
            g.drawHorizontalLine(static_cast<int>(screen_pos.getY()), 
                                display_area.getX(), display_area.getRight());
        }
    }
}

void WaveformViewer::drawCenterLine(juce::Graphics& g) {
    auto display_area = getDisplayArea();
    auto theme = getTheme();
    auto colors = theme->getMaterialColors();
    
    g.setColour(settings_.centerLineColor.getColor(colors).withAlpha(settings_.centerLineOpacity));
    g.drawHorizontalLine(static_cast<int>(display_area.getCentreY()), 
                        display_area.getX(), display_area.getRight());
}

void WaveformViewer::drawSelection(juce::Graphics& g) {
    if (!selection_.active) return;
    
    auto display_area = getDisplayArea();
    auto theme = getTheme();
    auto colors = theme->getMaterialColors();
    
    auto start_pos = timeToScreen(selection_.start, 0.0f);
    auto end_pos = timeToScreen(selection_.end, 0.0f);
    
    float selection_x = std::min(start_pos.getX(), end_pos.getX());
    float selection_width = std::abs(end_pos.getX() - start_pos.getX());
    
    auto selection_rect = juce::Rectangle<float>(selection_x, display_area.getY(), 
                                               selection_width, display_area.getHeight());
    
    g.setColour(selection_.color.getColor(colors).withAlpha(selection_.opacity));
    g.fillRect(selection_rect);
    
    // Draw selection border
    g.setColour(selection_.color.getColor(colors));
    g.drawRect(selection_rect, 1.0f);
}

void WaveformViewer::drawMarkers(juce::Graphics& g) {
    auto display_area = getDisplayArea();
    auto theme = getTheme();
    auto colors = theme->getMaterialColors();
    
    for (const auto& marker : markers_) {
        if (!marker.visible) continue;
        
        auto screen_pos = timeToScreen(marker.position, 0.0f);
        
        g.setColour(marker.color.getColor(colors));
        
        if (marker.loopPoint) {
            // Draw loop point marker (triangle)
            juce::Path triangle;
            triangle.addTriangle(screen_pos.getX() - 5.0f, display_area.getY(),
                               screen_pos.getX() + 5.0f, display_area.getY(),
                               screen_pos.getX(), display_area.getY() + 10.0f);
            g.fillPath(triangle);
        } else if (marker.startMarker) {
            // Draw start marker (rectangle)
            g.fillRect(screen_pos.getX() - 2.0f, display_area.getY(), 
                      4.0f, display_area.getHeight());
        } else {
            // Default marker (line)
            g.drawVerticalLine(static_cast<int>(screen_pos.getX()), 
                              display_area.getY(), display_area.getBottom());
        }
    }
}

void WaveformViewer::drawCursor(juce::Graphics& g) {
    auto display_area = getDisplayArea();
    auto theme = getTheme();
    auto colors = theme->getMaterialColors();
    
    auto cursor_pos = timeToScreen(cursorPosition_, 0.0f);
    
    g.setColour(settings_.cursorColor.getColor(colors).withAlpha(settings_.cursorOpacity));
    g.drawVerticalLine(static_cast<int>(cursor_pos.getX()), 
                      display_area.getY(), display_area.getBottom());
    
    // Draw cursor label
    g.setColour(settings_.cursorColor.getColor(colors));
    g.setFont(juce::Font(10.0f));
    g.drawText(juce::String(cursorPosition_, 3) + "s", 
              cursor_pos.getX() - 15.0f, display_area.getY() - 15.0f, 
              30.0f, 12.0f, juce::Justification::centred);
}

void WaveformViewer::drawTimeScale(juce::Graphics& g) {
    auto bounds = getLocalBounds();
    auto display_area = getDisplayArea();
    auto theme = getTheme();
    auto colors = theme->getMaterialColors();
    
    g.setColour(colors.onSurface.withAlpha(0.7f));
    g.setFont(juce::Font(9.0f));
    
    auto time_range = getTimeRange();
    float total_time = time_range.second - time_range.first;
    float grid_spacing = std::pow(10, std::floor(std::log10(total_time / 5)));
    
    float start_time = std::floor(time_range.first / grid_spacing) * grid_spacing;
    for (float t = start_time; t <= time_range.second; t += grid_spacing) {
        auto screen_pos = timeToScreen(t, 0.0f);
        if (screen_pos.getX() >= display_area.getX() && screen_pos.getX() <= display_area.getRight()) {
            juce::String time_text = juce::String(t, 3) + "s";
            g.drawText(time_text, screen_pos.getX() - 15.0f, bounds.getBottom() - 18.0f, 
                      30.0f, 12.0f, juce::Justification::centred);
        }
    }
}

void WaveformViewer::drawAmplitudeScale(juce::Graphics& g) {
    auto bounds = getLocalBounds();
    auto display_area = getDisplayArea();
    auto theme = getTheme();
    auto colors = theme->getMaterialColors();
    
    g.setColour(colors.onSurface.withAlpha(0.7f));
    g.setFont(juce::Font(9.0f));
    
    for (int i = -4; i <= 4; ++i) {
        float amplitude = i * 0.2f; // From -0.8 to +0.8
        auto screen_pos = timeToScreen(0.0f, amplitude);
        
        if (screen_pos.getY() >= display_area.getY() && screen_pos.getY() <= display_area.getBottom()) {
            juce::String amp_text = juce::String(amplitude, 1);
            g.drawText(amp_text, bounds.getX() + 2.0f, screen_pos.getY() - 6.0f, 
                      20.0f, 12.0f, juce::Justification::left);
        }
    }
}

juce::String WaveformViewer::announceToScreenReader(const juce::String& message) {
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