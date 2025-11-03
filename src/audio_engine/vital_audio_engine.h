/*
  ==============================================================================
    vital_audio_engine.h
    Copyright (c) 2025 Vital Audio Engine Team
    https://vital.audio
    
    Main audio engine header that integrates all phase 3 synthesis improvements
    - 15 oscillator types including chaos, fractal, bio-inspired, and quantum
    - Advanced synthesis engines (modal, physical modeling, granular)
    - Professional effects processing with convolution and adaptive effects
    - Real-time spectral warping and phase vocoding
    - Ultra-low noise audio quality enhancements
    
    This is the central hub for all Vital audio processing functionality.
  ==============================================================================
*/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_plugin_client/juce_audio_plugin_client.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>

#include "core/audio_engine_core.h"
#include "oscillators/new_oscillators.h"
#include "synthesis/advanced_synthesis_engine.h"
#include "effects/effects_processing_engine.h"
#include "spectral/spectral_warping_engine.h"
#include "audio_quality/audio_quality_processor.h"
#include "modulation/modulation_engine.h"
#include "filtering/filter_engine.h"
#include "utility/vital_constants.h"
#include "utility/parameter_system.h"

namespace vital {
namespace audio_engine {

//==============================================================================
/**
 * @class VitalAudioEngine
 * @brief Main audio engine that integrates all Vital synthesis capabilities
 * 
 * This is the central audio engine that coordinates all Vital's advanced
 * synthesis features including:
 * - 15 different oscillator types
 * - Advanced synthesis algorithms
 * - Professional effects processing
 * - Real-time spectral manipulation
 * - Ultra-low noise audio quality
 * 
 * The engine provides a unified interface for all audio processing while
 * maintaining optimal performance through modular design and SIMD optimization.
 */
class VitalAudioEngine
{
public:
    //==============================================================================
    /** Engine configuration structure */
    struct Config
    {
        // Audio settings
        double sampleRate = 44100.0;
        int bufferSize = 512;
        int maxChannels = 2;
        int maxVoices = 32;
        
        // Quality settings
        bool highQualityMode = true;
        bool enableOversampling = true;
        int oversamplingFactor = 2;
        bool enableAntialiasing = true;
        bool enableUltraLowNoise = true;
        
        // Performance settings
        bool enableSIMD = true;
        bool enableMultithreading = true;
        int maxWorkerThreads = 4;
        float cpuLimit = 0.85f;
        bool enableDynamicOptimization = true;
        
        // Synthesis settings
        int numOscillators = 8;
        bool enableNewOscillators = true;
        bool enableAdvancedSynthesis = true;
        bool enableGranularSynthesis = true;
        bool enablePhysicalModeling = true;
        
        // Effects settings
        bool enableEffectsProcessing = true;
        bool enableSpectralWarping = true;
        bool enableConvolution = true;
        bool enableAdaptiveEffects = true;
        
        // Modulation settings
        int numLFOs = 4;
        int numEnvelopes = 4;
        bool enableMacros = true;
        bool enableMidiCC = true;
        
        // Memory settings
        size_t maxMemoryUsage = 512 * 1024 * 1024; // 512MB
        bool enableMemoryOptimization = true;
        bool enableVoiceStealing = true;
    };
    
    //==============================================================================
    /** Engine state information */
    struct EngineState
    {
        bool isInitialized = false;
        bool isProcessing = false;
        bool isSuspended = false;
        bool isBypassed = false;
        
        float cpuUsage = 0.0f;
        size_t memoryUsage = 0;
        int activeVoices = 0;
        int sampleRate = 44100;
        int bufferSize = 512;
        
        std::chrono::steady_clock::time_point lastUpdate;
        
        struct PerformanceStats {
            float averageCPULoad = 0.0f;
            float peakCPULoad = 0.0f;
            size_t peakMemoryUsage = 0;
            int totalVoicesPlayed = 0;
            int totalNotesProcessed = 0;
            float totalUptime = 0.0f;
        } performance;
    };
    
    //==============================================================================
    /** Constructor */
    explicit VitalAudioEngine(const Config& config = Config());
    
    /** Destructor */
    ~VitalAudioEngine();
    
    //==============================================================================
    /** Initialize the audio engine */
    bool initialize();
    
    /** Shutdown and cleanup */
    void shutdown();
    
    /** Reset engine to initial state */
    void reset();
    
    /** Suspend/resume audio processing */
    void suspend();
    void resume();
    
    //==============================================================================
    /** Main audio processing */
    void processBlock(const juce::AudioBuffer<float>& input,
                     juce::AudioBuffer<float>& output,
                     const juce::MidiBuffer& midiMessages);
    
    /** Process single sample */
    float processSample(float input, const juce::MidiBuffer& midiMessages, int channel = 0);
    
    //==============================================================================
    /** Voice management */
    void noteOn(int note, float velocity, int channel = 0, int voiceId = -1);
    void noteOff(int note, int channel = 0, int voiceId = -1);
    void allNotesOff(int channel = 0);
    void setVoicePriority(int voiceId, int priority);
    
    /** Voice information */
    int getNumActiveVoices() const;
    int getMaxVoices() const;
    int allocateVoice(int note, float velocity);
    void deallocateVoice(int voiceId);
    
    //==============================================================================
    /** Oscillator management */
    void setOscillatorType(int oscillatorId, int type);
    void setOscillatorFrequency(int oscillatorId, float frequency);
    void setOscillatorAmplitude(int oscillatorId, float amplitude);
    void setOscillatorPhase(int oscillatorId, float phase);
    int getNumOscillators() const;
    
    /** Access to new oscillator types */
    oscillators::NewOscillatorFactory& getOscillatorFactory() { return oscillatorFactory_; }
    
    //==============================================================================
    /** Synthesis engine access */
    synthesis::AdvancedSynthesisEngine& getSynthesisEngine() { return synthesisEngine_; }
    modulation::ModulationEngine& getModulationEngine() { return modulationEngine_; }
    filtering::FilterEngine& getFilterEngine() { return filterEngine_; }
    
    //==============================================================================
    /** Effects processing */
    effects::EffectsProcessingEngine& getEffectsEngine() { return effectsEngine_; }
    spectral::SpectralWarpingEngine& getSpectralEngine() { return spectralEngine_; }
    
    //==============================================================================
    /** Audio quality processing */
    audio_quality::AudioProcessor& getAudioQualityProcessor() { return audioQualityProcessor_; }
    
    //==============================================================================
    /** Parameter management */
    void setParameter(int paramId, float value);
    float getParameter(int paramId) const;
    void setParameterSmoothing(int paramId, float timeMs);
    void setModulationSource(int paramId, int source, float depth);
    
    /** Parameter automation */
    void setParameterAutomation(int paramId, const std::vector<float>& automation);
    void applyModulation(int paramId, float& value, int sample);
    
    //==============================================================================
    /** Global engine controls */
    void setMasterGain(float gain);
    void setMasterTune(float cents);
    void setMasterBypass(bool bypassed);
    void setGlobalTuning(float tuning);
    
    float getMasterGain() const { return masterGain_; }
    float getMasterTune() const { return masterTuneCents_; }
    bool isBypassed() const { return engineState_.isBypassed; }
    
    //==============================================================================
    /** MIDI processing */
    void processMidiMessage(const juce::MidiMessage& message);
    void processMidiBuffer(const juce::MidiBuffer& buffer);
    
    /** MIDI control */
    void setMidiChannel(int channel);
    int getMidiChannel() const { return midiChannel_; }
    void enableMidiLearn(bool enabled);
    bool isMidiLearnEnabled() const { return midiLearnEnabled_; }
    
    //==============================================================================
    /** Preset management */
    bool loadPreset(const juce::File& file);
    bool savePreset(const juce::File& file, const std::string& name = "");
    void loadDefaultSettings();
    void saveAsDefaultSettings();
    
    /** Preset information */
    std::string getCurrentPresetName() const { return currentPresetName_; }
    std::vector<std::string> getPresetNames() const;
    
    //==============================================================================
    /** Performance monitoring */
    EngineState getEngineState() const;
    Config getConfig() const { return config_; }
    
    /** Real-time performance metrics */
    struct RealTimeMetrics {
        float cpuLoad = 0.0f;
        float voiceUtilization = 0.0f;
        float memoryUsageMB = 0.0f;
        float averageLatencyMs = 0.0f;
        int droppedVoices = 0;
        int xruns = 0;
    };
    
    RealTimeMetrics getRealTimeMetrics() const;
    void enablePerformanceMonitoring(bool enabled);
    bool isPerformanceMonitoringEnabled() const { return performanceMonitoringEnabled_; }
    
    //==============================================================================
    /** Error handling */
    struct ErrorInfo {
        bool hasError = false;
        std::string message;
        std::string component;
        std::chrono::steady_clock::time_point timestamp;
    };
    
    ErrorInfo getLastError() const;
    void clearError();
    
    //==============================================================================
    /** Engine state queries */
    bool isInitialized() const { return engineState_.isInitialized; }
    bool isProcessing() const { return engineState_.isProcessing; }
    bool isSuspended() const { return engineState_.isSuspended; }
    
    //==============================================================================
    /** Utility functions */
    float noteNumberToFrequency(int noteNumber);
    int frequencyToNoteNumber(float frequency);
    float centsToRatio(float cents);
    float ratioToCents(float ratio);
    
    /** Testing and debugging */
    void enableDebugOutput(bool enabled);
    void setTestMode(bool enabled);
    bool isInTestMode() const { return testMode_; }
    
private:
    //==============================================================================
    /** Configuration */
    Config config_;
    mutable Config pendingConfig_; // For thread-safe updates
    
    /** Engine state */
    EngineState engineState_;
    mutable std::mutex stateMutex_;
    
    /** Performance monitoring */
    RealTimeMetrics realTimeMetrics_;
    std::atomic<bool> performanceMonitoringEnabled_{false};
    std::chrono::steady_clock::time_point startTime_;
    
    /** Error handling */
    ErrorInfo lastError_;
    std::atomic<bool> hasError_{false};
    
    /** Control parameters */
    float masterGain_ = 1.0f;
    float masterTuneCents_ = 0.0f;
    int midiChannel_ = 1;
    bool midiLearnEnabled_ = false;
    std::string currentPresetName_;
    
    //==============================================================================
    /** Core engine components */
    std::unique_ptr<core::AudioEngineCore> coreEngine_;
    
    /** Synthesis components */
    synthesis::AdvancedSynthesisEngine synthesisEngine_;
    modulation::ModulationEngine modulationEngine_;
    filtering::FilterEngine filterEngine_;
    
    /** Oscillator system */
    oscillators::NewOscillatorFactory oscillatorFactory_;
    std::vector<std::unique_ptr<oscillators::Oscillator>> oscillators_;
    
    /** Effects processing */
    effects::EffectsProcessingEngine effectsEngine_;
    spectral::SpectralWarpingEngine spectralEngine_;
    
    /** Audio quality */
    audio_quality::AudioProcessor audioQualityProcessor_;
    
    /** Parameter system */
    utility::ParameterSystem parameterSystem_;
    
    //==============================================================================
    /** Threading and scheduling */
    std::unique_ptr<juce::ThreadPool> workerThreadPool_;
    std::vector<std::unique_ptr<juce::Timer>> periodicTasks_;
    std::atomic<bool> shutdownRequested_{false};
    
    /** Processing queues */
    juce::AbstractFifo midiInputQueue_{1024};
    juce::AbstractFifo parameterQueue_{256};
    
    //==============================================================================
    /** Internal initialization methods */
    bool initializeCore();
    bool initializeOscillators();
    bool initializeSynthesis();
    bool initializeEffects();
    bool initializeAudioQuality();
    bool initializeModulation();
    bool initializeFilters();
    
    /** Internal shutdown methods */
    void shutdownCore();
    void shutdownOscillators();
    void shutdownSynthesis();
    void shutdownEffects();
    void shutdownAudioQuality();
    void shutdownModulation();
    void shutdownFilters();
    
    //==============================================================================
    /** Processing methods */
    void updateParameters(int numSamples);
    void processMidiInput(int numSamples);
    void allocateVoices(const juce::MidiBuffer& midiMessages);
    void deallocateFinishedVoices();
    void updateVoiceStates();
    void processSynthesizers(int numSamples);
    void applyEffectsProcessing(int numSamples);
    void applySpectralProcessing(int numSamples);
    void applyAudioQualityProcessing(int numSamples);
    void mixOutput(int numSamples);
    
    //==============================================================================
    /** Voice management internals */
    struct Voice {
        int id = -1;
        int note = -1;
        float velocity = 0.0f;
        int channel = 0;
        bool active = false;
        bool sustained = false;
        int priority = 0;
        std::chrono::steady_clock::time_point startTime;
        
        // Synthesis state
        float frequency = 440.0f;
        float amplitude = 1.0f;
        std::vector<float> parameters;
    };
    
    std::vector<Voice> voices_;
    std::vector<int> freeVoiceIds_;
    std::priority_queue<int> voicePriorityQueue_;
    
    //==============================================================================
    /** Utility methods */
    void updateEngineState();
    void updatePerformanceMetrics();
    void manageMemoryUsage();
    void handleParameterUpdates();
    void processPendingMessages();
    
    /** Logging and debugging */
    void logMessage(const std::string& message, const std::string& level = "INFO");
    void logError(const std::string& error, const std::string& component = "ENGINE");
    bool validateState() const;
    
    //==============================================================================
    /** Debug and test features */
    bool debugOutputEnabled_ = false;
    bool testMode_ = false;
    std::vector<float> testSignalBuffer_;
    
    void generateTestSignal(float* buffer, int numSamples);
    void logPerformanceStats();
    void validateAudioOutput();
    
    //==============================================================================
    /** Static factory methods for external creation */
    static std::unique_ptr<VitalAudioEngine> create(const Config& config);
    static Config getDefaultConfig();
    
    //==============================================================================
    /** Thread safety and atomic operations */
    void updateConfigAtomic(const Config& newConfig);
    Config getConfigAtomic() const;
    
    //==============================================================================
    /** JUCE integration helpers */
    void setupAudioFormats();
    void setupMidiInput();
    void setupParameterSystem();
    
    //==============================================================================
    /** Constants and magic numbers */
    static constexpr int kMaxParameters = 1024;
    static constexpr int kMaxAutomationPoints = 16384;
    static constexpr float kMaxCPUUsage = 0.95f;
    static constexpr size_t kMinMemoryThreshold = 64 * 1024 * 1024; // 64MB
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VitalAudioEngine)
    JUCE_DECLARE_WEAK_REFERENCEABLE(VitalAudioEngine)
};

//==============================================================================
/**
 * @namespace vital::audio_engine::factory
 * @brief Factory functions for creating and configuring VitalAudioEngine instances
 */
namespace factory {

/**
 * Create a VitalAudioEngine with recommended settings for optimal quality
 */
std::unique_ptr<VitalAudioEngine> createHighQualityEngine();

/**
 * Create a VitalAudioEngine optimized for low CPU usage
 */
std::unique_ptr<VitalAudioEngine> createLowCPUEngine();

/**
 * Create a VitalAudioEngine with minimum resource usage
 */
std::unique_ptr<VitalAudioEngine> createMinimalEngine();

/**
 * Create a VitalAudioEngine for testing and development
 */
std::unique_ptr<VitalAudioEngine> createTestEngine();

/**
 * Validate engine configuration
 */
bool validateConfig(const VitalAudioEngine::Config& config);

} // namespace factory

} // namespace audio_engine
} // namespace vital
