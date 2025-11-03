/*
  ==============================================================================
    plugin_parameters.h
    Copyright (c) 2025 Vital Plugin Integration Team
    https://vital.audio
    
    Plugin parameter management system for Vital
    Provides parameter registration, automation support, MIDI learning,
    and smooth parameter handling for VST3/AU integration.
  ==============================================================================
*/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <map>
#include <vector>
#include <atomic>
#include <mutex>

namespace vital {
namespace plugin {

//==============================================================================
/**
 * @class Parameter
 * @brief Individual plugin parameter
 */
class Parameter {
public:
    enum Type {
        Float,     // Continuous floating point
        Int,       // Integer
        Bool,      // Boolean toggle
        Choice,    // Choice from predefined list
        Percent,   // Percentage value (0-100)
        Decibels,  // dB value
        Cents,     // Tuning in cents
        Frequency, // Frequency in Hz
        Note       // Musical note number
    };
    
    struct Range {
        float min = 0.0f;
        float max = 1.0f;
        float defaultValue = 0.5f;
        float step = 0.01f;
        bool logarithmic = false;
    };
    
    struct Display {
        juce::String suffix;
        juce::String formatString;
        juce::StringListModel choices;
        int decimals = 2;
        bool showValue = true;
        bool showName = true;
    };
    
    struct Automation {
        bool automatable = true;
        bool record = true;
        float smoothing = 1.0f; // milliseconds
        std::vector<float> points;
    };
    
    struct MidiMapping {
        int cc = -1;
        int channel = -1;
        bool enabled = false;
        bool isBipolar = false;
        float minValue = 0.0f;
        float maxValue = 1.0f;
        bool relative = false;
    };
    
    Parameter() = default;
    Parameter(int id, const juce::String& name, Type type, const Range& range = Range());
    ~Parameter() = default;
    
    // Getters
    int getId() const { return id_; }
    juce::String getName() const { return name_; }
    juce::String getLabel() const { return label_; }
    Type getType() const { return type_; }
    
    // Value management
    float getValue() const { return value_.load(); }
    float getNormalizedValue() const;
    void setValue(float value);
    void setNormalizedValue(float normalizedValue);
    
    // Range management
    Range getRange() const { return range_; }
    void setRange(const Range& range);
    void setMin(float min);
    void setMax(float max);
    void setDefaultValue(float defaultValue);
    
    // Display settings
    Display getDisplay() const { return display_; }
    void setDisplay(const Display& display);
    juce::String getDisplayText() const;
    juce::String getDisplayText(float value) const;
    float parseDisplayText(const juce::String& text) const;
    
    // Automation support
    Automation getAutomation() const { return automation_; }
    void setAutomation(const Automation& automation);
    bool isAutomatable() const { return automation_.automatable; }
    void setValue(float value, double time);
    
    // MIDI mapping
    MidiMapping getMidiMapping() const { return midiMapping_; }
    void setMidiMapping(const MidiMapping& mapping);
    bool hasMidiMapping() const { return midiMapping_.enabled && midiMapping_.cc >= 0; }
    
    // Value conversion
    float normalizeToValue(float normalized) const;
    float valueToNormalized(float value) const;
    float clampValue(float value) const;
    
    // State management
    juce::ValueTree createState() const;
    void restoreState(const juce::ValueTree& state);
    
    // Utility functions
    bool isMetaParameter() const { return isMetaParameter_; }
    void setIsMetaParameter(bool meta) { isMetaParameter_ = meta; }
    
    bool isBypassed() const { return bypassed_; }
    void setBypassed(bool bypassed) { bypassed_ = bypassed; }
    
    int getNumSteps() const;
    float getStepSize() const;
    
    // Thread safety
    void lock() const { mutex_.lock(); }
    void unlock() const { mutex_.unlock(); }
    
private:
    int id_ = -1;
    juce::String name_;
    juce::String label_;
    Type type_ = Float;
    Range range_;
    Display display_;
    Automation automation_;
    MidiMapping midiMapping_;
    
    std::atomic<float> value_{0.0f};
    std::atomic<float> smoothedValue_{0.0f};
    std::atomic<bool> bypassed_{false};
    bool isMetaParameter_ = false;
    
    mutable std::mutex mutex_;
    
    // Value conversion helpers
    float valueToNormalizedLinear(float value) const;
    float normalizedToValueLinear(float normalized) const;
    float valueToNormalizedLog(float value) const;
    float normalizedToValueLog(float normalized) const;
};

//==============================================================================
/**
 * @class ParameterGroup
 * @brief Group of related parameters
 */
class ParameterGroup {
public:
    ParameterGroup() = default;
    ParameterGroup(const juce::String& name, const juce::String& label = "");
    ~ParameterGroup() = default;
    
    // Group management
    juce::String getName() const { return name_; }
    juce::String getLabel() const { return label_; }
    juce::String getDescription() const { return description_; }
    
    void setDescription(const juce::String& description) { description_ = description; }
    
    // Parameter management
    void addParameter(std::shared_ptr<Parameter> parameter);
    void removeParameter(int paramId);
    std::shared_ptr<Parameter> getParameter(int paramId) const;
    std::shared_ptr<Parameter> getParameterByName(const juce::String& name) const;
    
    const std::vector<std::shared_ptr<Parameter>>& getParameters() const { return parameters_; }
    
    int getNumParameters() const { return static_cast<int>(parameters_.size()); }
    
    // State management
    juce::ValueTree createState() const;
    void restoreState(const juce::ValueTree& state);
    
    // Utility
    void setCollapsed(bool collapsed) { collapsed_ = collapsed; }
    bool isCollapsed() const { return collapsed_; }
    
    void setVisible(bool visible) { visible_ = visible; }
    bool isVisible() const { return visible_; }
    
private:
    juce::String name_;
    juce::String label_;
    juce::String description_;
    std::vector<std::shared_ptr<Parameter>> parameters_;
    std::map<int, std::shared_ptr<Parameter>> parametersById_;
    std::map<juce::String, std::shared_ptr<Parameter>> parametersByName_;
    
    bool collapsed_ = false;
    bool visible_ = true;
};

//==============================================================================
/**
 * @class PluginParameters
 * @brief Main parameter management system
 */
class PluginParameters : public juce::ChangeBroadcaster {
public:
    PluginParameters();
    explicit PluginParameters(juce::AudioProcessor* plugin);
    ~PluginParameters() = default;
    
    // Initialization
    void initialize();
    void shutdown();
    
    // Plugin reference
    void setPlugin(juce::AudioProcessor* plugin);
    juce::AudioProcessor* getPlugin() const { return plugin_; }
    
    // Parameter management
    void addParameter(std::shared_ptr<Parameter> parameter);
    void addParameterGroup(std::shared_ptr<ParameterGroup> group);
    void removeParameter(int paramId);
    void removeAllParameters();
    
    std::shared_ptr<Parameter> getParameter(int paramId) const;
    std::shared_ptr<Parameter> getParameterByName(const juce::String& name) const;
    const std::vector<std::shared_ptr<Parameter>>& getAllParameters() const { return parameters_; }
    const std::vector<std::shared_ptr<ParameterGroup>>& getAllGroups() const { return groups_; }
    
    int getNumParameters() const { return static_cast<int>(parameters_.size()); }
    int getNumGroups() const { return static_cast<int>(groups_.size()); }
    
    // Value access
    float getValue(int paramId) const;
    float getNormalizedValue(int paramId) const;
    void setValue(int paramId, float value);
    void setNormalizedValue(int paramId, float normalizedValue);
    
    // Smoothed values for real-time processing
    float getSmoothedValue(int paramId) const;
    void setSmoothedValue(int paramId, float value);
    void updateSmoothedValues(int numSamples);
    
    // Parameter info
    juce::String getName(int paramId) const;
    juce::String getText(int paramId) const;
    bool isAutomatable(int paramId) const;
    bool isMetaParameter(int paramId) const;
    
    // Automation support
    void setDefaultValue(int paramId, float value);
    float getDefaultValue(int paramId) const;
    int getParameterNumSteps(int paramId) const;
    
    // MIDI learning
    void enableMidiLearn(bool enabled);
    bool isMidiLearnEnabled() const { return midiLearnEnabled_; }
    bool learnParameter(int paramId, int cc, int channel);
    void unlearnParameter(int paramId);
    bool isParameterMapped(int paramId) const;
    
    // Batch operations
    void setAllParametersToDefault();
    void resetAllParameters();
    void randomizeParameters();
    void setParameterRange(int paramId, float min, float max);
    
    // State management
    void getParameterState(juce::XmlElement& xml) const;
    void setParameterState(const juce::XmlElement& xml);
    
    // Performance optimization
    void enableSmoothing(bool enabled);
    bool isSmoothingEnabled() const { return smoothingEnabled_; }
    void setSmoothingEnabled(int paramId, bool enabled);
    
    void enableSmartUpdate(bool enabled);
    bool isSmartUpdateEnabled() const { return smartUpdateEnabled_; }
    
    // Real-time parameter updates
    struct ParameterUpdate {
        int paramId;
        float value;
        double timestamp;
        bool isFromUser = false;
        bool isFromMIDI = false;
        bool isFromAutomation = false;
    };
    
    std::vector<ParameterUpdate> getPendingUpdates();
    void clearPendingUpdates();
    
    // Parameter categories
    enum Category {
        Master,
        Oscillator,
        Filter,
        Envelope,
        LFO,
        Effects,
        Modulation,
        Utility
    };
    
    void addParameterToCategory(int paramId, Category category);
    Category getParameterCategory(int paramId) const;
    std::vector<std::shared_ptr<Parameter>> getParametersByCategory(Category category) const;
    
    // Utility functions
    int findParameterByName(const juce::String& name) const;
    void validateParameters();
    bool validateParameter(int paramId) const;
    
    // Thread safety
    void lock() const { parametersMutex_.lock(); }
    void unlock() const { parametersMutex_.unlock(); }
    
private:
    juce::AudioProcessor* plugin_ = nullptr;
    
    // Parameter storage
    std::vector<std::shared_ptr<Parameter>> parameters_;
    std::vector<std::shared_ptr<ParameterGroup>> groups_;
    std::map<int, std::shared_ptr<Parameter>> parametersById_;
    std::map<juce::String, std::shared_ptr<Parameter>> parametersByName_;
    std::map<Category, std::vector<int>> parametersByCategory_;
    
    // State management
    bool midiLearnEnabled_ = false;
    bool smoothingEnabled_ = true;
    bool smartUpdateEnabled_ = true;
    
    // Real-time updates
    std::vector<ParameterUpdate> pendingUpdates_;
    std::mutex updateMutex_;
    
    mutable std::mutex parametersMutex_;
    
    // Internal methods
    void notifyListeners(int paramId);
    void createDefaultParameters();
    void createMasterParameters();
    void createOscillatorParameters();
    void createFilterParameters();
    void createEnvelopeParameters();
    void createLFOParameters();
    void createEffectParameters();
    void createModulationParameters();
    
    // Value normalization helpers
    float normalizeValue(float value, const Parameter::Range& range) const;
    float denormalizeValue(float normalized, const Parameter::Range& range) const;
    
    // MIDI handling
    void handleMidiCC(int cc, int value, int channel);
    void updateMidiMappings();
    
    // Parameter validation
    bool validateParameterRange(const Parameter& param) const;
    bool validateParameterType(const Parameter& param) const;
    bool validateParameterName(const Parameter& param) const;
    
    // Memory management
    void cleanupUnusedParameters();
    void optimizeMemoryUsage();
};

//==============================================================================
/**
 * @namespace vital::plugin::parameters
 * @brief Helper functions for parameter management
 */
namespace parameters {

/**
 * Create a standard oscillator parameter
 */
std::shared_ptr<Parameter> createOscillatorTypeParameter(int oscillatorId);

/**
 * Create a frequency parameter with logarithmic scaling
 */
std::shared_ptr<Parameter> createFrequencyParameter(const juce::String& name, 
                                                   float minFreq = 20.0f,
                                                   float maxFreq = 20000.0f);

/**
 * Create a gain parameter in decibels
 */
std::shared_ptr<Parameter> createGainParameter(const juce::String& name, 
                                             float minDb = -60.0f,
                                             float maxDb = 6.0f);

/**
 * Create a time parameter
 */
std::shared_ptr<Parameter> createTimeParameter(const juce::String& name,
                                             float minTime = 0.001f,
                                             float maxTime = 10.0f);

/**
 * Create a choice parameter
 */
std::shared_ptr<Parameter> createChoiceParameter(const juce::String& name,
                                               const juce::StringArray& choices);

/**
 * Create a percentage parameter
 */
std::shared_ptr<Parameter> createPercentParameter(const juce::String& name);

/**
 * Create a boolean parameter
 */
std::shared_ptr<Parameter> createBooleanParameter(const juce::String& name);

/**
 * Create a tuning parameter in cents
 */
std::shared_ptr<Parameter> createTuningParameter(const juce::String& name,
                                               float minCents = -1200.0f,
                                               float maxCents = 1200.0f);

/**
 * Create a musical note parameter
 */
std::shared_ptr<Parameter> createNoteParameter(const juce::String& name,
                                             int minNote = 0,
                                             int maxNote = 127);

/**
 * Create a parameter group for synthesis parameters
 */
std::shared_ptr<ParameterGroup> createSynthesisGroup(const juce::String& name);

/**
 * Create a parameter group for effects parameters
 */
std::shared_ptr<ParameterGroup> createEffectsGroup(const juce::String& name);

/**
 * Create a parameter group for modulation parameters
 */
std::shared_ptr<ParameterGroup> createModulationGroup(const juce::String& name);

} // namespace parameters

} // namespace plugin
} // namespace vital