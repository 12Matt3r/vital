/*
  ==============================================================================
    plugin_ui.h
    Copyright (c) 2025 Vital Plugin Integration Team
    https://vital.audio
    
    Plugin user interface system for Vital
    Provides modern, responsive UI with parameter controls, real-time visualization,
    preset browser, MIDI visualization, and comprehensive GUI components for VST3/AU.
  ==============================================================================
*/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_opengl/juce_opengl.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_re果per/juce_re果per.h>
#include <juce_dsp/juce_dsp.h>
#include <memory>
#include <vector>
#include <map>
#include <mutex>

namespace vital {
namespace plugin {

//==============================================================================
/**
 * @class UIComponent
 * @brief Base class for all UI components
 */
class UIComponent {
public:
    virtual ~UIComponent() = default;
    
    // Lifecycle
    virtual void initialize() = 0;
    virtual void shutdown() = 0;
    
    // Appearance
    virtual void setLookAndFeel(juce::LookAndFeel* laf);
    juce::LookAndFeel* getLookAndFeel() const { return lookAndFeel_; }
    
    // Layout
    virtual void setBounds(const juce::Rectangle<int>& bounds);
    juce::Rectangle<int> getBounds() const { return bounds_; }
    virtual void setVisible(bool visible);
    bool isVisible() const { return visible_; }
    
    // State
    virtual void setEnabled(bool enabled);
    bool isEnabled() const { return enabled_; }
    
    // Theme
    virtual void setTheme(const juce::String& themeName);
    juce::String getCurrentTheme() const { return currentTheme_; }
    
    // Animation
    virtual void animateIn();
    virtual void animateOut();
    void enableAnimations(bool enabled) { animationsEnabled_ = enabled; }
    bool areAnimationsEnabled() const { return animationsEnabled_; }
    
    // Performance
    virtual void setHighPerformanceMode(bool enabled);
    bool isHighPerformanceMode() const { return highPerformanceMode_; }
    
    // JUCE component
    virtual juce::Component* getComponent() = 0;
    
protected:
    juce::Rectangle<int> bounds_;
    bool visible_ = true;
    bool enabled_ = true;
    bool animationsEnabled_ = true;
    bool highPerformanceMode_ = false;
    juce::String currentTheme_ = "Default";
    juce::LookAndFeel* lookAndFeel_ = nullptr;
};

//==============================================================================
/**
 * @class ParameterControl
 * @brief Interactive parameter control component
 */
class ParameterControl : public UIComponent {
public:
    enum ControlType {
        Knob,
        Slider,
        Button,
        ComboBox,
        TextField,
        Rotary,
        HorizontalSlider,
        VerticalSlider,
        XYPad,
        Envelope,
        Meter
    };
    
    struct ControlStyle {
        juce::Colour backgroundColor = juce::Colours::transparentBlack;
        juce::Colour foregroundColor = juce::Colours::white;
        juce::Colour textColor = juce::Colours::white;
        juce::Colour borderColor = juce::Colours::grey;
        juce::Colour fillColor = juce::Colours::blue;
        
        int borderThickness = 2;
        int cornerRadius = 4;
        bool showValue = true;
        bool showLabel = true;
        bool showUnits = false;
        bool animated = true;
        
        float animationSpeed = 0.3f;
        juce::String fontName = "Default";
        int fontSize = 12;
        
        // Value display
        int decimals = 2;
        juce::String valueSuffix;
        juce::String labelSuffix;
        
        // Interaction
        bool isBipolar = false;
        bool isLogarithmic = false;
        float rangeMin = 0.0f;
        float rangeMax = 1.0f;
        float defaultValue = 0.5f;
        float stepSize = 0.01f;
    };
    
    ParameterControl();
    ParameterControl(int parameterId, const juce::String& parameterName);
    ~ParameterControl() = default;
    
    // Lifecycle
    void initialize() override;
    void shutdown() override;
    
    // Parameter
    int getParameterId() const { return parameterId_; }
    void setParameterId(int id) { parameterId_ = id; }
    
    juce::String getParameterName() const { return parameterName_; }
    void setParameterName(const juce::String& name) { parameterName_ = name; }
    
    // Control type
    ControlType getControlType() const { return controlType_; }
    void setControlType(ControlType type) { controlType_ = type; }
    
    // Values
    float getValue() const { return value_; }
    float getNormalizedValue() const;
    void setValue(float value);
    void setNormalizedValue(float normalizedValue);
    float getDefaultValue() const { return style_.defaultValue; }
    
    // Style
    ControlStyle& getStyle() { return style_; }
    const ControlStyle& getStyle() const { return style_; }
    void setStyle(const ControlStyle& style) { style_ = style; }
    
    // Visual state
    void setValueDisplay(const juce::String& display);
    juce::String getValueDisplay() const { return valueDisplay_; }
    
    // Callback
    std::function<void(float)> onValueChanged;
    std::function<void()> onParameterSelected;
    std::function<void()> onParameterDoubleClicked;
    
    // Visual feedback
    void setHighlighted(bool highlighted);
    bool isHighlighted() const { return highlighted_; }
    
    void setMidiMapped(bool mapped);
    bool isMidiMapped() const { return midiMapped_; }
    
    void setMidiLearning(bool learning);
    bool isMidiLearning() const { return midiLearning_; }
    
    // Range management
    void setRange(float min, float max);
    juce::String getRangeString() const;
    
    // JUCE component
    juce::Component* getComponent() override { return component_.get(); }
    
private:
    int parameterId_ = -1;
    juce::String parameterName_;
    ControlType controlType_ = Knob;
    
    float value_ = 0.0f;
    juce::String valueDisplay_;
    
    ControlStyle style_;
    
    // Visual state
    bool highlighted_ = false;
    bool midiMapped_ = false;
    bool midiLearning_ = false;
    float smoothedValue_ = 0.0f;
    
    // Component
    std::unique_ptr<juce::Component> component_;
    
    // Internal methods
    void createComponent();
    void updateComponent();
    void formatValueDisplay();
    float normalizeValue(float value) const;
    float denormalizeValue(float normalizedValue) const;
};

//==============================================================================
/**
 * @class OscillatorPanel
 * @brief Oscillator controls and visualization
 */
class OscillatorPanel : public UIComponent {
public:
    struct OscillatorVisual {
        juce::Colour waveColor = juce::Colours::blue;
        juce::Colour backgroundColor = juce::Colours::black;
        juce::Colour gridColor = juce::Colours::darkgrey;
        bool showGrid = true;
        bool showHarmonics = false;
        bool showPhase = false;
        int numSamples = 1024;
        float zoom = 1.0f;
        float offset = 0.0f;
    };
    
    struct OscillatorSettings {
        int type = 0; // Sine, Square, Saw, Triangle, Noise, etc.
        float frequency = 440.0f;
        float amplitude = 1.0f;
        float phase = 0.0f;
        float detune = 0.0f;
        bool enabled = true;
        bool syncEnabled = false;
        float syncRatio = 1.0f;
        bool ringModEnabled = false;
        float ringModAmount = 0.0f;
        
        // Advanced settings
        float harmonicAmount = 0.0f;
        int harmonicOrder = 1;
        float wavefolding = 0.0f;
        float saturation = 0.0f;
        float bitCrush = 0.0f;
        float sampleRate = 44100.0f;
    };
    
    OscillatorPanel();
    explicit OscillatorPanel(int oscillatorId);
    ~OscillatorPanel() = default;
    
    // Lifecycle
    void initialize() override;
    void shutdown() override;
    
    // Oscillator
    int getOscillatorId() const { return oscillatorId_; }
    void setOscillatorId(int id) { oscillatorId_ = id; }
    
    OscillatorSettings& getSettings() { return settings_; }
    const OscillatorSettings& getSettings() const { return settings_; }
    void setSettings(const OscillatorSettings& settings) { settings_ = settings; }
    
    // Visualization
    OscillatorVisual& getVisual() { return visual_; }
    const OscillatorVisual& getVisual() const { return visual_; }
    void setVisual(const OscillatorVisual& visual) { visual_ = visual; }
    
    // Waveform data
    void updateWaveformData(const std::vector<float>& data);
    std::vector<float> getWaveformData() const { return waveformData_; }
    
    // Real-time display
    void enableRealtimeDisplay(bool enabled);
    bool isRealtimeDisplayEnabled() const { return realtimeDisplay_; }
    
    // JUCE component
    juce::Component* getComponent() override { return component_.get(); }
    
private:
    int oscillatorId_ = 0;
    OscillatorSettings settings_;
    OscillatorVisual visual_;
    
    std::vector<float> waveformData_;
    bool realtimeDisplay_ = true;
    
    // Component
    std::unique_ptr<juce::Component> component_;
    
    // Audio visualization
    juce::OpenGLContext* getOpenGLContext();
    void renderWaveform();
    void updateOscilloscope();
};

//==============================================================================
/**
 * @class FilterPanel
 * @brief Filter controls and visualization
 */
class FilterPanel : public UIComponent {
public:
    struct FilterType {
        enum Type {
            LowPass,
            HighPass,
            BandPass,
            Notch,
            AllPass,
            LowShelf,
            HighShelf,
            Peaking,
            Tilt,
            Multiband
        };
    };
    
    struct FilterSettings {
        FilterType::Type type = FilterType::LowPass;
        float frequency = 1000.0f;
        float resonance = 0.1f;
        float gain = 0.0f;
        float q = 0.1f;
        bool enabled = true;
        
        // Slope
        int slope = 12; // dB/octave
        bool normalized = true;
        
        // Envelope modulation
        float envelopeAmount = 0.0f;
        bool envelopeEnabled = false;
        int envelopeSource = 0;
        
        // LFO modulation
        float lfoAmount = 0.0f;
        bool lfoEnabled = false;
        int lfoSource = 0;
        float lfoRate = 1.0f;
        
        // Tracking
        bool keyTracking = false;
        float keyTrackAmount = 0.0f;
        float keyTrackNote = 69.0f; // A4
    };
    
    struct FilterVisual {
        juce::Colour responseColor = juce::Colours::green;
        juce::Colour backgroundColor = juce::Colours::black;
        juce::Colour gridColor = juce::Colours::darkgrey;
        juce::Colour markerColor = juce::Colours::yellow;
        bool showMagnitude = true;
        bool showPhase = false;
        bool showGroupDelay = false;
        bool logFrequency = true;
        int numPoints = 512;
        float frequencyRange[2] = {20.0f, 20000.0f};
        float gainRange[2] = {-60.0f, 12.0f};
    };
    
    FilterPanel();
    explicit FilterPanel(int filterId);
    ~FilterPanel() = default;
    
    // Lifecycle
    void initialize() override;
    void shutdown() override;
    
    // Filter
    int getFilterId() const { return filterId_; }
    void setFilterId(int id) { filterId_ = id; }
    
    FilterSettings& getSettings() { return settings_; }
    const FilterSettings& getSettings() const { return settings_; }
    void setSettings(const FilterSettings& settings) { settings_ = settings; }
    
    // Visualization
    FilterVisual& getVisual() { return visual_; }
    const FilterVisual& getVisual() const { return visual_; }
    void setVisual(const FilterVisual& visual) { visual_ = visual; }
    
    // Frequency response
    void updateFrequencyResponse(const std::vector<float>& frequencies,
                                const std::vector<float>& magnitudes,
                                const std::vector<float>& phases = {});
    
    // Real-time display
    void enableRealtimeResponse(bool enabled);
    bool isRealtimeResponseEnabled() const { return realtimeResponse_; }
    
    // JUCE component
    juce::Component* getComponent() override { return component_.get(); }
    
private:
    int filterId_ = 0;
    FilterSettings settings_;
    FilterVisual visual_;
    
    std::vector<float> frequencyData_;
    std::vector<float> magnitudeData_;
    std::vector<float> phaseData_;
    bool realtimeResponse_ = true;
    
    // Component
    std::unique_ptr<juce::Component> component_;
    
    // Internal methods
    void renderFrequencyResponse();
    void updateFilterDisplay();
    juce::String getFilterTypeName(FilterType::Type type) const;
};

//==============================================================================
/**
 * @class EnvelopePanel
 * @brief Envelope generator controls
 */
class EnvelopePanel : public UIComponent {
public:
    struct EnvelopeSettings {
        float attack = 0.01f;
        float decay = 0.3f;
        float sustain = 0.7f;
        float release = 1.0f;
        
        bool enabled = true;
        bool retrigger = false;
        bool loop = false;
        
        // Advanced ADSR
        float hold = 0.0f;
        float slope = 1.0f;
        float curve = 0.0f;
        
        // Velocity sensitivity
        float velocityAmount = 0.0f;
        bool velocityEnabled = false;
        
        // Modulation
        float modAmount = 0.0f;
        int modSource = 0;
    };
    
    struct EnvelopeVisual {
        juce::Colour envelopeColor = juce::Colours::red;
        juce::Colour backgroundColor = juce::Colours::black;
        juce::Colour gridColor = juce::Colours::darkgrey;
        bool showGrid = true;
        int numSamples = 512;
        float timeScale = 1.0f;
        float amplitudeScale = 1.0f;
        
        // Display options
        bool showPoints = true;
        bool showCurves = true;
        bool fillArea = false;
        float lineThickness = 2.0f;
    };
    
    EnvelopePanel();
    explicit EnvelopePanel(int envelopeId);
    ~EnvelopePanel() = default;
    
    // Lifecycle
    void initialize() override;
    void shutdown() override;
    
    // Envelope
    int getEnvelopeId() const { return envelopeId_; }
    void setEnvelopeId(int id) { envelopeId_ = id; }
    
    EnvelopeSettings& getSettings() { return settings_; }
    const EnvelopeSettings& getSettings() const { return settings_; }
    void setSettings(const EnvelopeSettings& settings) { settings_ = settings; }
    
    // Visualization
    EnvelopeVisual& getVisual() { return visual_; }
    const EnvelopeVisual& getVisual() const { return visual_; }
    void setVisual(const EnvelopeVisual& visual) { visual_ = visual; }
    
    // Real-time envelope display
    void enableRealtimeDisplay(bool enabled);
    bool isRealtimeDisplayEnabled() const { return realtimeDisplay_; }
    
    void updateEnvelopeDisplay(float currentLevel, float time, bool playing);
    
    // JUCE component
    juce::Component* getComponent() override { return component_.get(); }
    
private:
    int envelopeId_ = 0;
    EnvelopeSettings settings_;
    EnvelopeVisual visual_;
    
    bool realtimeDisplay_ = true;
    float currentLevel_ = 0.0f;
    float currentTime_ = 0.0f;
    bool isPlaying_ = false;
    
    // Component
    std::unique_ptr<juce::Component> component_;
    
    // Internal methods
    void renderEnvelope();
    void updateDisplay();
    juce::Point<float> calculateCurvePoint(float t, float start, float end, float curve) const;
};

//==============================================================================
/**
 * @class LFOComponent
 * @brief LFO controls and visualization
 */
class LFOComponent : public UIComponent {
public:
    struct LFOSettings {
        float frequency = 1.0f;
        float amplitude = 0.5f;
        float phase = 0.0f;
        int waveform = 0; // Sine, Square, Saw, Triangle, Random, etc.
        
        bool enabled = true;
        bool syncToTempo = false;
        int syncRatio = 1;
        
        // Fade in/out
        float fadeIn = 0.0f;
        float fadeOut = 0.0f;
        bool enableFade = false;
        
        // Random
        float randomAmount = 0.0f;
        int randomType = 0; // Smooth, Step, Binary
        
        // Modulation
        float modAmount = 0.0f;
        int modSource = 0;
        float modDepth = 0.0f;
    };
    
    struct LFOVisual {
        juce::Colour lfoColor = juce::Colours::yellow;
        juce::Colour backgroundColor = juce::Colours::black;
        juce::Colour gridColor = juce::Colours::darkgrey;
        bool showGrid = true;
        int numSamples = 512;
        float timeScale = 1.0f;
        float amplitudeScale = 1.0f;
        
        // Display options
        bool showWaveform = true;
        bool showPhase = false;
        bool fillArea = false;
        float lineThickness = 2.0f;
    };
    
    LFOComponent();
    explicit LFOComponent(int lfoId);
    ~LFOComponent() = default;
    
    // Lifecycle
    void initialize() override;
    void shutdown() override;
    
    // LFO
    int getLFOId() const { return lfoId_; }
    void setLFOId(int id) { lfoId_ = id; }
    
    LFOSettings& getSettings() { return settings_; }
    const LFOSettings& getSettings() const { return settings_; }
    void setSettings(const LFOSettings& settings) { settings_ = settings; }
    
    // Visualization
    LFOVisual& getVisual() { return visual_; }
    const LFOVisual& getVisual() const { return visual_; }
    void setVisual(const LFOVisual& visual) { visual_ = visual; }
    
    // Real-time display
    void enableRealtimeDisplay(bool enabled);
    bool isRealtimeDisplayEnabled() const { return realtimeDisplay_; }
    
    void updateLFODisplay(float phase, float value);
    
    // JUCE component
    juce::Component* getComponent() override { return component_.get(); }
    
private:
    int lfoId_ = 0;
    LFOSettings settings_;
    LFOVisual visual_;
    
    bool realtimeDisplay_ = true;
    float currentPhase_ = 0.0f;
    float currentValue_ = 0.0f;
    
    // Component
    std::unique_ptr<juce::Component> component_;
    
    // Internal methods
    void renderLFO();
    void updateDisplay();
    float calculateWaveform(float phase, int waveform) const;
};

//==============================================================================
/**
 * @class PresetBrowser
 * @brief Preset browsing and management interface
 */
class PresetBrowser : public UIComponent {
public:
    struct PresetEntry {
        juce::String name;
        juce::String author;
        juce::String category;
        juce::String description;
        juce::String filePath;
        int rating = 0;
        int usageCount = 0;
        juce::Time lastUsed;
        bool isFavorite = false;
        juce::Image icon;
        juce::String tags;
    };
    
    struct BrowserSettings {
        bool showCategories = true;
        bool showRatings = true;
        bool showUsageCount = true;
        bool showLastUsed = true;
        bool showTags = true;
        bool showFavorites = false;
        int numColumns = 3;
        int iconSize = 64;
        bool showPreview = true;
        bool enableKeyboard = true;
        bool enableDragDrop = true;
    };
    
    PresetBrowser();
    ~PresetBrowser() = default;
    
    // Lifecycle
    void initialize() override;
    void shutdown() override;
    
    // Preset management
    void refreshPresets();
    void addPreset(const PresetEntry& preset);
    void removePreset(const juce::String& name);
    void loadPreset(const juce::String& name);
    void saveCurrentPreset(const juce::String& name, const juce::String& category = "");
    
    // Search and filter
    void setSearchQuery(const juce::String& query);
    juce::String getSearchQuery() const { return searchQuery_; }
    
    void setCategoryFilter(const juce::String& category);
    juce::String getCategoryFilter() const { return categoryFilter_; }
    
    void setRatingFilter(int minRating);
    int getRatingFilter() const { return ratingFilter_; }
    
    // Display
    BrowserSettings& getSettings() { return settings_; }
    const BrowserSettings& getSettings() const { return settings_; }
    void setSettings(const BrowserSettings& settings) { settings_ = settings; }
    
    // Favorites
    void addToFavorites(const juce::String& name);
    void removeFromFavorites(const juce::String& name);
    void setFavorite(const juce::String& name, bool favorite);
    
    // Callbacks
    std::function<void(const juce::String&)> onPresetSelected;
    std::function<void(const juce::String&)> onPresetLoaded;
    std::function<void(const juce::String&)> onPresetSaved;
    
    // JUCE component
    juce::Component* getComponent() override { return component_.get(); }
    
private:
    std::vector<PresetEntry> presets_;
    juce::String searchQuery_;
    juce::String categoryFilter_;
    int ratingFilter_ = 0;
    
    BrowserSettings settings_;
    
    // Component
    std::unique_ptr<juce::Component> component_;
    
    // Internal methods
    void updatePresetList();
    void createPresetListComponent();
    void createSearchComponent();
    void createFilterComponent();
};

//==============================================================================
/**
 * @class MIDIVisualizer
 * @brief MIDI input visualization
 */
class MIDIVisualizer : public UIComponent {
public:
    struct MIDIEvent {
        int channel = 0;
        int note = -1;
        int velocity = 0;
        int controller = -1;
        int controllerValue = 0;
        double timestamp = 0.0;
        juce::MidiMessage::MidiMessageType type;
    };
    
    struct VisualSettings {
        juce::Colour backgroundColor = juce::Colours::black;
        juce::Colour gridColor = juce::Colours::darkgrey;
        juce::Colour noteColor = juce::Colours::blue;
        juce::Colour ccColor = juce::Colours::green;
        bool showNotes = true;
        bool showCC = true;
        bool showVelocity = true;
        bool showTimestamp = true;
        int numChannels = 16;
        float timeRange = 5.0f; // seconds
        int maxEvents = 1000;
    };
    
    MIDIVisualizer();
    ~MIDIVisualizer() = default;
    
    // Lifecycle
    void initialize() override;
    void shutdown() override;
    
    // Events
    void addEvent(const MIDIEvent& event);
    void clearEvents();
    std::vector<MIDIEvent> getEvents() const { return events_; }
    
    // Visualization settings
    VisualSettings& getSettings() { return settings_; }
    const VisualSettings& getSettings() const { return settings_; }
    void setSettings(const VisualSettings& settings) { settings_ = settings; }
    
    // Display options
    void enableNoteDisplay(bool enabled);
    void enableCCDisplay(bool enabled);
    void setTimeRange(float seconds);
    float getTimeRange() const { return settings_.timeRange; }
    
    // Real-time updates
    void enableRealtimeDisplay(bool enabled);
    bool isRealtimeDisplayEnabled() const { return realtimeDisplay_; }
    
    // Performance
    void setMaxEvents(int maxEvents);
    int getMaxEvents() const { return settings_.maxEvents; }
    
    // JUCE component
    juce::Component* getComponent() override { return component_.get(); }
    
private:
    std::vector<MIDIEvent> events_;
    VisualSettings settings_;
    bool realtimeDisplay_ = true;
    
    // Component
    std::unique_ptr<juce::Component> component_;
    
    // Internal methods
    void renderMIDIEvents();
    void updateDisplay();
    void cleanupOldEvents();
};

//==============================================================================
/**
 * @class PluginUI
 * @brief Main plugin user interface
 */
class PluginUI : public UIComponent {
public:
    struct UISettings {
        int width = 1200;
        int height = 800;
        bool resizable = true;
        bool hasTitleBar = true;
        bool hasMenuBar = false;
        bool hasStatusBar = true;
        
        // Theme
        juce::String theme = "Default";
        juce::Colour backgroundColor = juce::Colour(0xFF2D2D2D);
        juce::Colour accentColor = juce::Colour(0xFF007ACC);
        juce::Colour textColor = juce::Colours::white;
        
        // Layout
        bool showTabs = true;
        bool showTooltips = true;
        bool showKeyboard = true;
        bool showMeters = true;
        bool showSpectrum = true;
        
        // Performance
        bool enableGPUAcceleration = true;
        bool enableVSync = true;
        bool highQualityGraphics = true;
        int refreshRate = 60;
        
        // Accessibility
        bool enableKeyboardNavigation = true;
        bool enableScreenReader = true;
        bool highContrast = false;
        bool largeFonts = false;
    };
    
    PluginUI();
    explicit PluginUI(juce::AudioProcessor* plugin);
    ~PluginUI();
    
    // Lifecycle
    void initialize() override;
    void shutdown() override;
    
    // Plugin reference
    void setPlugin(juce::AudioProcessor* plugin);
    juce::AudioProcessor* getPlugin() const { return plugin_; }
    
    // Settings
    UISettings& getSettings() { return settings_; }
    const UISettings& getSettings() const { return settings_; }
    void setSettings(const UISettings& settings) { settings_ = settings; }
    
    // Panel management
    void addPanel(const juce::String& name, std::shared_ptr<UIComponent> panel);
    void removePanel(const juce::String& name);
    void showPanel(const juce::String& name);
    void hidePanel(const juce::String& name);
    void setActivePanel(const juce::String& name);
    
    std::shared_ptr<UIComponent> getPanel(const juce::String& name) const;
    std::vector<juce::String> getPanelNames() const;
    
    // Parameter controls
    void addParameterControl(std::shared_ptr<ParameterControl> control);
    void removeParameterControl(int parameterId);
    void updateAllParameterDisplays();
    
    // Performance monitoring
    void enablePerformanceDisplay(bool enabled);
    bool isPerformanceDisplayEnabled() const { return performanceDisplay_; }
    
    void updatePerformanceStats(float cpuUsage, int activeVoices, size_t memoryUsage);
    
    // Theme and customization
    void setTheme(const juce::String& themeName) override;
    void setBackgroundColor(const juce::Colour& color);
    void setAccentColor(const juce::Colour& color);
    
    // Modal dialogs
    void showAboutDialog();
    void showSettingsDialog();
    void showPresetManager();
    void showMidiLearnDialog();
    
    // Fullscreen mode
    void toggleFullscreen();
    void enterFullscreen();
    void exitFullscreen();
    bool isFullscreen() const { return fullscreen_; }
    
    // Mobile/touch support
    void enableTouchMode(bool enabled);
    bool isTouchModeEnabled() const { return touchMode_; }
    
    // JUCE component
    juce::Component* getComponent() override { return component_.get(); }
    
    // Real-time updates
    void startRealTimeUpdates();
    void stopRealTimeUpdates();
    bool areRealTimeUpdatesEnabled() const { return realTimeUpdates_; }
    
private:
    juce::AudioProcessor* plugin_ = nullptr;
    UISettings settings_;
    
    // Panel management
    std::map<juce::String, std::shared_ptr<UIComponent>> panels_;
    juce::String activePanel_;
    std::vector<juce::String> panelOrder_;
    
    // Parameter controls
    std::vector<std::shared_ptr<ParameterControl>> parameterControls_;
    std::map<int, std::shared_ptr<ParameterControl>> parameterControlMap_;
    
    // Display state
    bool fullscreen_ = false;
    bool touchMode_ = false;
    bool performanceDisplay_ = false;
    bool realTimeUpdates_ = false;
    
    // OpenGL context
    std::unique_ptr<juce::OpenGLContext> openGLContext_;
    
    // Component
    std::unique_ptr<juce::Component> component_;
    
    // Internal methods
    void createMainComponent();
    void createParameterControls();
    void createPanelComponents();
    void setupLayout();
    void updateLayout();
    
    // Panel creation
    std::shared_ptr<OscillatorPanel> createOscillatorPanel(int oscillatorId);
    std::shared_ptr<FilterPanel> createFilterPanel(int filterId);
    std::shared_ptr<EnvelopePanel> createEnvelopePanel(int envelopeId);
    std::shared_ptr<LFOComponent> createLFOPanel(int lfoId);
    std::shared_ptr<PresetBrowser> createPresetBrowser();
    std::shared_ptr<MIDIVisualizer> createMidiVisualizer();
    
    // Timer callback for real-time updates
    juce::Timer* realTimeTimer_ = nullptr;
    void realTimeTimerCallback();
    
    // Performance monitoring
    struct PerformanceStats {
        float cpuUsage = 0.0f;
        int activeVoices = 0;
        size_t memoryUsage = 0;
        float renderTime = 0.0f;
        int framesPerSecond = 0;
    };
    
    PerformanceStats performanceStats_;
    mutable std::mutex statsMutex_;
    
    // Accessibility
    void setupAccessibility();
    void handleKeyboardNavigation(const juce::KeyPress& key);
    void updateScreenReaderContent();
};

//==============================================================================
/**
 * @namespace vital::plugin::ui
 * @brief UI helper functions and utilities
 */
namespace ui {

/**
 * Create a modern parameter knob
 */
std::shared_ptr<ParameterControl> createParameterKnob(int parameterId, const juce::String& name);

/**
 * Create a horizontal slider
 */
std::shared_ptr<ParameterControl> createHorizontalSlider(int parameterId, const juce::String& name);

/**
 * Create a vertical slider
 */
std::shared_ptr<ParameterControl> createVerticalSlider(int parameterId, const juce::String& name);

/**
 * Create a toggle button
 */
std::shared_ptr<ParameterControl> createToggleButton(int parameterId, const juce::String& name);

/**
 * Create a combo box
 */
std::shared_ptr<ParameterControl> createComboBox(int parameterId, const juce::String& name, 
                                                const juce::StringArray& choices);

/**
 * Create an XY pad control
 */
std::shared_ptr<ParameterControl> createXYPad(int xParameterId, int yParameterId, const juce::String& name);

/**
 * Create a level meter
 */
std::shared_ptr<ParameterControl> createLevelMeter(int meterId, const juce::String& name);

/**
 * Create an oscillator display
 */
std::shared_ptr<OscillatorPanel> createOscillatorDisplay(int oscillatorId);

/**
 * Create a filter response display
 */
std::shared_ptr<FilterPanel> createFilterDisplay(int filterId);

/**
 * Apply modern UI theme
 */
void applyModernTheme(juce::LookAndFeel& Laf);

/**
 * Apply dark theme
 */
void applyDarkTheme(juce::LookAndFeel& Laf);

/**
 * Apply light theme
 */
void applyLightTheme(juce::LookAndFeel& Laf);

/**
 * Create responsive layout
 */
juce::Rectangle<float> createResponsiveBounds(float x, float y, float width, float height, 
                                            float screenWidth, float screenHeight, 
                                            float baseWidth = 1200.0f, float baseHeight = 800.0f);

/**
 * Create high-DPI scaling factor
 */
float getDpiScaling();

/**
 * Create touch-friendly control size
 */
float getTouchControlSize();

} // namespace ui

} // namespace plugin
} // namespace vital