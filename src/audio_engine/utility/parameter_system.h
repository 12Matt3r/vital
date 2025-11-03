/*
  ==============================================================================
    parameter_system.h
    Copyright (c) 2025 Vital Audio Engine Team
    
    Comprehensive parameter management system for the Vital audio engine
    with automation, modulation, and smoothing support
  ==============================================================================
*/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <functional>

namespace vital {
namespace audio_engine {
namespace utility {

//==============================================================================
/**
 * @class ParameterSystem
 * @brief Comprehensive parameter management system
 * 
 * Provides:
 * - Parameter creation, management, and access
 * - Real-time parameter smoothing
 * - Parameter automation and recording
 * - Modulation routing and mixing
 * - Parameter range validation and clamping
 * - Thread-safe parameter updates
 */
class ParameterSystem
{
public:
    //==============================================================================
    /** Parameter configuration */
    struct ParameterConfig {
        std::string name;
        std::string shortName;
        std::string unit;
        float minValue = 0.0f;
        float maxValue = 1.0f;
        float defaultValue = 0.5f;
        float smoothingTimeMs = 0.0f; // 0 = no smoothing
        bool automationEnabled = true;
        bool modulationEnabled = true;
        bool isBipolar = false; // true = ranges from -1 to 1
        int precision = 3; // decimal places for display
        std::string valueToString(float value) const;
        float stringToValue(const std::string& str) const;
    };
    
    //==============================================================================
    /** Parameter automation data */
    struct AutomationPoint {
        double time;      // Time in seconds
        float value;      // Parameter value at this time
        float curve;      // Curve shape (-1 to 1)
    };
    
    //==============================================================================
    /** Modulation source */
    struct ModulationSource {
        enum class Type {
            None,
            LFO,
            Envelope,
            Macro,
            MIDI_CC,
            Automation,
            External
        };
        
        Type type = Type::None;
        int sourceId = -1;
        float depth = 1.0f;
        bool enabled = true;
        std::function<float(double)> externalFunction;
    };
    
    //==============================================================================
    /** Individual parameter */
    class Parameter {
    public:
        explicit Parameter(const ParameterConfig& config);
        
        /** Value access */
        void setValue(float value);
        float getValue() const { return currentValue_; }
        float getTargetValue() const { return targetValue_; }
        float getDefaultValue() const { return config_.defaultValue; }
        
        /** Range access */
        float getMinValue() const { return config_.minValue; }
        float getMaxValue() const { return config_.maxValue; }
        bool isBipolar() const { return config_.isBipolar; }
        
        /** Smoothing */
        void setSmoothingTime(float timeMs);
        float getSmoothingTime() const { return config_.smoothingTimeMs; }
        void setSmoothingEnabled(bool enabled) { smoothingEnabled_ = enabled; }
        bool isSmoothingEnabled() const { return smoothingEnabled_; }
        
        /** Automation */
        void setAutomationPoints(const std::vector<AutomationPoint>& points);
        const std::vector<AutomationPoint>& getAutomationPoints() const { return automationPoints_; }
        void clearAutomation();
        void recordAutomationPoint(double time, float value);
        float getAutomatedValue(double time) const;
        
        /** Modulation */
        void addModulationSource(const ModulationSource& source);
        void removeModulationSource(int sourceId);
        void setModulationDepth(int sourceId, float depth);
        float getModulationValue() const { return modulationValue_; }
        void updateModulationValue(double time);
        
        /** Processing */
        void process(int numSamples);
        void processAutomation(double currentTime);
        void reset();
        
        /** Display */
        std::string getName() const { return config_.name; }
        std::string getShortName() const { return config_.shortName; }
        std::string getUnit() const { return config_.unit; }
        int getPrecision() const { return config_.precision; }
        std::string getValueString() const;
        
        /** State queries */
        bool isAutomationActive() const { return !automationPoints_.empty(); }
        bool hasModulation() const { return modulationValue_ != 0.0f; }
        bool isChanging() const { return std::abs(currentValue_ - targetValue_) > 0.0001f; }
        
    private:
        ParameterConfig config_;
        float currentValue_ = 0.0f;
        float targetValue_ = 0.0f;
        float modulationValue_ = 0.0f;
        bool smoothingEnabled_ = true;
        
        std::vector<AutomationPoint> automationPoints_;
        std::vector<ModulationSource> modulationSources_;
        
        double lastAutomationTime_ = 0.0;
        std::atomic<bool> automationDirty_{false};
        
        void updateSmoothing(int numSamples);
        float interpolateAutomation(double time) const;
        void validateValue(float& value) const;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Parameter)
    };
    
    //==============================================================================
    /** Constructor */
    explicit ParameterSystem(int maxParameters = 1024);
    
    /** Destructor */
    ~ParameterSystem();
    
    //==============================================================================
    /** System initialization */
    void initialize(int maxParameters);
    void resetAll();
    
    //==============================================================================
    /** Parameter management */
    int createParameter(const ParameterConfig& config);
    void removeParameter(int parameterId);
    Parameter* getParameter(int parameterId);
    const Parameter* getParameter(int parameterId) const;
    int getNumParameters() const;
    
    /** Parameter access */
    void setParameter(int parameterId, float value);
    float getParameter(int parameterId) const;
    std::string getParameterName(int parameterId) const;
    
    /** Batch parameter access */
    void setParameters(const std::vector<float>& values);
    std::vector<float> getParameters() const;
    
    //==============================================================================
    /** Smoothing control */
    void setParameterSmoothing(int parameterId, float timeMs);
    void setGlobalSmoothingEnabled(bool enabled);
    bool isGlobalSmoothingEnabled() const { return globalSmoothingEnabled_; }
    void setSmoothingQuality(int quality); // 0=fast, 1=medium, 2=high
    
    //==============================================================================
    /** Automation control */
    void setParameterAutomation(int parameterId, const std::vector<AutomationPoint>& points);
    void clearParameterAutomation(int parameterId);
    void recordParameterAutomation(int parameterId, double time, float value);
    void setAutomationPlaybackRate(float rate);
    void setAutomationLoop(bool enabled);
    void startAutomationPlayback(double startTime = 0.0);
    void stopAutomationPlayback();
    
    /** Automation queries */
    bool isParameterAutomated(int parameterId) const;
    bool isAnyAutomationActive() const;
    double getAutomationLength() const;
    
    //==============================================================================
    /** Modulation control */
    void setModulationSource(int parameterId, int sourceId, const ModulationSource& source);
    void setModulationDepth(int parameterId, int sourceId, float depth);
    void clearParameterModulation(int parameterId);
    float getModulationValue(int parameterId) const;
    
    /** Global modulation */
    void setGlobalModulationDepth(float depth);
    float getGlobalModulationDepth() const { return globalModulationDepth_; }
    void enableAllModulation(bool enabled);
    
    //==============================================================================
    /** Real-time processing */
    void process(int numSamples);
    void updateModulationValues(double currentTime);
    
    /** Time management */
    void setCurrentTime(double time);
    double getCurrentTime() const { return currentTime_; }
    void setTimeScale(float scale) { timeScale_ = scale; }
    
    //==============================================================================
    /** State queries */
    bool isParameterValid(int parameterId) const;
    bool areParametersChanging() const;
    int getNumChangedParameters() const;
    std::vector<int> getChangedParameterIds() const;
    
    //==============================================================================
    /** Parameter presets */
    void saveParameterState(const std::string& presetName);
    void loadParameterState(const std::string& presetName);
    void deleteParameterState(const std::string& presetName);
    std::vector<std::string> getParameterStateNames() const;
    void clearAllPresets();
    
    //==============================================================================
    /** Serialization */
    std::string serializeParameters() const;
    void deserializeParameters(const std::string& data);
    
    /** MIDI integration */
    void setMidiChannel(int channel);
    int getMidiChannel() const { return midiChannel_; }
    void setMidiCCMapping(int parameterId, int ccNumber);
    int getMidiCCMapping(int parameterId) const;
    void setMIDIControlValue(int ccNumber, float value);
    
    //==============================================================================
    /** Performance monitoring */
    struct PerformanceStats {
        int totalParameters = 0;
        int activeParameters = 0;
        int automatedParameters = 0;
        int modulatedParameters = 0;
        float averageSmoothingTime = 0.0f;
        double processingTimeMs = 0.0f;
        int automationEvents = 0;
        int modulationUpdates = 0;
    };
    
    PerformanceStats getPerformanceStats() const;
    void resetPerformanceStats();
    
    //==============================================================================
    /** Error handling */
    struct ErrorInfo {
        bool hasError = false;
        std::string message;
        std::string parameterName;
        std::chrono::steady_clock::time_point timestamp;
    };
    
    ErrorInfo getLastError() const;
    void clearError();
    
private:
    //==============================================================================
    /** Configuration */
    int maxParameters_;
    int currentParameterCount_ = 0;
    
    /** State */
    std::atomic<bool> isInitialized_{false};
    std::atomic<bool> globalSmoothingEnabled_{true};
    std::atomic<double> currentTime_{0.0};
    std::atomic<float> timeScale_{1.0f};
    std::atomic<float> globalModulationDepth_{1.0f};
    std::atomic<int> midiChannel_{1};
    
    /** Parameter storage */
    std::vector<std::unique_ptr<Parameter>> parameters_;
    std::vector<int> freeParameterIds_;
    std::vector<int> changedParameterIds_;
    
    /** Automation system */
    bool automationPlaying_ = false;
    double automationStartTime_ = 0.0;
    float automationPlaybackRate_ = 1.0f;
    bool automationLoop_ = false;
    
    /** Performance monitoring */
    PerformanceStats performanceStats_;
    std::chrono::steady_clock::time_point lastProcessTime_;
    
    /** Error tracking */
    ErrorInfo lastError_;
    std::atomic<bool> hasError_{false};
    
    /** Threading */
    mutable std::mutex parameterMutex_;
    mutable std::mutex automationMutex_;
    mutable std::mutex modulationMutex_;
    
    //==============================================================================
    /** Internal methods */
    int allocateParameterId();
    void deallocateParameterId(int parameterId);
    void updatePerformanceStats();
    void validateParameterId(int parameterId) const;
    void logError(const std::string& error, const std::string& paramName = "");
    
    /** Utility methods */
    void initializePresets();
    void cleanupParameters();
    void optimizeMemoryUsage();
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParameterSystem)
};

} // namespace utility
} // namespace audio_engine
} // namespace vital
