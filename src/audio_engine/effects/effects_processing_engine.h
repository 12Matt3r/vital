/*
  ==============================================================================
    effects_processing_engine.h
    Copyright (c) 2025 Vital Audio Engine Team
    
    Effects processing engine integrating partitioned convolution,
    dynamic convolution, multi-band processing, adaptive effects,
    and parameter morphing from phase3
  ==============================================================================
*/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>

namespace vital {
namespace audio_engine {
namespace effects {

//==============================================================================
/**
 * @class EffectsProcessingEngine
 * @brief Comprehensive effects processing engine with all phase3 improvements
 * 
 * This engine provides:
 * - Partitioned convolution reverb with low-latency processing
 * - Dynamic convolution for real-time impulse response changes
 * - Multi-band processing for frequency-dependent effects
 * - Adaptive effects that respond to input content
 * - Parameter morphing between different effect settings
 * - Professional-grade audio quality with oversampling
 */
class EffectsProcessingEngine
{
public:
    //==============================================================================
    /** Engine configuration */
    struct Config
    {
        double sampleRate = 44100.0;
        int bufferSize = 512;
        int maxChannels = 2;
        
        // Effect enable/disable flags
        bool enableConvolution = true;
        bool enableDynamicConvolution = false;
        bool enableMultiBand = true;
        bool enableAdaptiveEffects = true;
        bool enableParameterMorphing = true;
        
        // Quality settings
        bool highQualityMode = true;
        bool enableOversampling = true;
        int oversamplingFactor = 2;
        bool enableLatencyCompensation = true;
        
        // Performance settings
        bool enableMultithreading = true;
        int maxConcurrentEffects = 4;
        float cpuLimit = 0.8f;
        
        // Memory management
        size_t maxMemoryUsage = 256 * 1024 * 1024; // 256MB
        bool enableMemoryOptimization = true;
        
        // Convolution settings
        bool partitionedConvolution = true;
        int partitionSize = 512;
        int maxImpulseLength = 44100; // 1 second at 44.1kHz
        
        // Multi-band settings
        int numBands = 4;
        float crossoverFrequencies[4] = {200.0f, 1000.0f, 4000.0f, 8000.0f};
        FilterType filterType = FilterType::LinkwitzRiley;
        
        // Adaptive effects settings
        float adaptationRate = 0.01f;
        float detectionThreshold = 0.1f;
        bool enableContentAnalysis = true;
        
        // Parameter morphing settings
        float morphSpeed = 1.0f;
        bool enableBezierMorphing = true;
        int maxMorphPoints = 16;
    };
    
    enum class FilterType {
        LinkwitzRiley,
        Butterworth,
        Chebyshev,
        Elliptic
    };
    
    //==============================================================================
    /** Effect types */
    enum class EffectType {
        Reverb,
        Delay,
        Chorus,
        Flanger,
        Phaser,
        Distortion,
        Compressor,
        EQ,
        Filter,
        Custom
    };
    
    /** Morphing curve types */
    enum class MorphingCurve {
        Linear,
        Exponential,
        Logarithmic,
        Sigmoid,
        Bezier,
        CatmullRom
    };
    
    //==============================================================================
    /** Effect preset information */
    struct EffectPreset
    {
        std::string name;
        std::string description;
        std::map<std::string, float> parameters;
        std::vector<float> parameterCurves;
        bool supportsMorphing = true;
        bool supportsAdaptation = true;
        std::map<std::string, std::string> metadata;
        
        EffectPreset() = default;
        EffectPreset(const std::string& n, const std::string& d) 
            : name(n), description(d) {}
    };
    
    //==============================================================================
    /** Performance statistics */
    struct PerformanceStats
    {
        float averageCPUUsage = 0.0f;
        float peakCPUUsage = 0.0f;
        size_t memoryUsage = 0;
        size_t peakMemoryUsage = 0;
        int totalEffectsProcessed = 0;
        int totalSamplesProcessed = 0;
        float averageProcessingTime = 0.0f;
        float totalUptime = 0.0f;
        
        struct EffectStats {
            float cpuUsage = 0.0f;
            float processingTime = 0.0f;
            size_t memoryUsage = 0;
            int callsCount = 0;
        } convolution, dynamicConvolution, multiBand, adaptiveEffects, parameterMorphing;
    };
    
    //==============================================================================
    /** Engine state information */
    struct EngineState
    {
        bool isInitialized = false;
        bool isProcessing = false;
        bool isSuspended = false;
        bool isBypassed = false;
        
        float currentCPUUsage = 0.0f;
        size_t currentMemoryUsage = 0;
        int totalLatencySamples = 0;
        
        std::chrono::steady_clock::time_point lastUpdate;
        
        struct EffectState {
            bool enabled = true;
            bool active = false;
            float processingTime = 0.0f;
            std::string status;
            int callsCount = 0;
        };
        
        EffectState convolution;
        EffectState dynamicConvolution;
        EffectState multiBand;
        EffectState adaptiveEffects;
        EffectState parameterMorphing;
    };
    
    //==============================================================================
    /** Constructor */
    explicit EffectsProcessingEngine(const Config& config = Config());
    
    /** Destructor */
    ~EffectsProcessingEngine();
    
    //==============================================================================
    /** Initialize the effects engine */
    bool initialize();
    
    /** Shutdown and cleanup */
    void shutdown();
    
    /** Reset to default state */
    void reset();
    
    /** Suspend/resume processing */
    void suspend();
    void resume();
    
    //==============================================================================
    /** Main audio processing */
    void processBlock(int numSamples);
    
    /** Process single sample */
    float processSample(float input, int channel = 0);
    
    /** Process audio buffer with effects chain */
    void processBuffer(const juce::AudioBuffer<float>& input,
                      juce::AudioBuffer<float>& output);
    
    //==============================================================================
    /** Effect enable/disable controls */
    void setConvolutionEnabled(bool enabled);
    void setDynamicConvolutionEnabled(bool enabled);
    void setMultiBandEnabled(bool enabled);
    void setAdaptiveEffectsEnabled(bool enabled);
    void setParameterMorphingEnabled(bool enabled);
    
    /** Get effect enable status */
    bool isConvolutionEnabled() const;
    bool isDynamicConvolutionEnabled() const;
    bool isMultiBandEnabled() const;
    bool isAdaptiveEffectsEnabled() const;
    bool isParameterMorphingEnabled() const;
    
    //==============================================================================
    /** Convolution reverb control */
    void loadImpulseResponse(const juce::File& impulseFile);
    void loadImpulseResponse(const std::vector<float>& impulseResponse);
    void setConvolutionGain(float gain);
    void setConvolutionMix(float mix);
    void setRoomSize(float size);
    void setDamping(float damping);
    
    /** Dynamic convolution control */
    void setDynamicConvolutionEnabled(bool enabled, float blendRate = 0.1f);
    void setInterpolationMode(int mode);
    void setMaxImpulseLength(int length);
    
    //==============================================================================
    /** Multi-band processing control */
    void setNumBands(int numBands);
    void setCrossoverFrequency(int bandIndex, float frequency);
    void setBandGain(int bandIndex, float gain);
    void setBandQ(int bandIndex, float q);
    void setBandEnabled(int bandIndex, bool enabled);
    
    /** Band-specific effects */
    void setBandEffectType(int bandIndex, EffectType effect);
    void setBandEffectParameters(int bandIndex, const std::map<std::string, float>& params);
    
    //==============================================================================
    /** Adaptive effects control */
    void setAdaptationRate(float rate);
    void setDetectionThreshold(float threshold);
    void setAdaptationMode(int mode);
    void setContentAnalysisEnabled(bool enabled);
    
    /** Analysis features */
    void setSpectralAnalysisEnabled(bool enabled);
    void setTemporalAnalysisEnabled(bool enabled);
    void setLevelAnalysisEnabled(bool enabled);
    
    //==============================================================================
    /** Parameter morphing control */
    void startMorphing(const std::string& targetPreset, float duration = 1.0f);
    void setMorphingCurve(MorphingCurve curve);
    void setMorphingSpeed(float speed);
    void addMorphingPoint(float time, float value, int parameter = -1);
    void clearMorphingPoints();
    
    /** Preset management */
    void savePreset(const std::string& name, const std::map<std::string, float>& parameters);
    void loadPreset(const std::string& name);
    std::vector<std::string> getPresetNames() const;
    void deletePreset(const std::string& name);
    
    //==============================================================================
    /** Global effect parameters */
    void setMasterGain(float gain);
    void setMix(float mix);
    void setBypass(bool bypassed);
    void setGlobalWetDry(float wet, float dry);
    
    float getMasterGain() const { return masterGain_; }
    float getMix() const { return mix_; }
    bool isBypassed() const { return engineState_.isBypassed; }
    
    //==============================================================================
    /** Performance monitoring */
    EngineState getEngineState() const;
    PerformanceStats getPerformanceStats() const;
    
    /** Real-time metrics */
    struct RealTimeMetrics {
        float currentCPUUsage = 0.0f;
        float currentMemoryUsage = 0.0f;
        int currentLatency = 0;
        int activeEffects = 0;
        float morphingProgress = 0.0f;
        float adaptiveResponse = 0.0f;
    };
    
    RealTimeMetrics getRealTimeMetrics() const;
    void enablePerformanceMonitoring(bool enabled);
    
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
    /** Utility functions */
    void setInputGain(float gain);
    void setOutputGain(float gain);
    int getTotalLatency() const;
    double getSampleRate() const { return config_.sampleRate; }
    int getBufferSize() const { return config_.bufferSize; }
    
    //==============================================================================
    /** State queries */
    bool isInitialized() const { return engineState_.isInitialized; }
    bool isProcessing() const { return engineState_.isProcessing; }
    bool isSuspended() const { return engineState_.isSuspended; }
    
    //==============================================================================
    /** Parameter access */
    void setParameter(int paramId, float value);
    float getParameter(int paramId) const;
    std::string getParameterName(int paramId) const;
    std::vector<int> getParameterIds() const;
    
private:
    //==============================================================================
    /** Configuration */
    Config config_;
    
    /** Engine state */
    EngineState engineState_;
    
    /** Control parameters */
    float masterGain_ = 1.0f;
    float mix_ = 1.0f;
    float inputGain_ = 1.0f;
    float outputGain_ = 1.0f;
    std::string currentPreset_;
    
    /** Performance monitoring */
    PerformanceStats performanceStats_;
    std::atomic<bool> performanceMonitoringEnabled_{false};
    std::chrono::steady_clock::time_point startTime_;
    
    /** Error tracking */
    ErrorInfo lastError_;
    std::atomic<bool> hasError_{false};
    
    //==============================================================================
    /** Effect components - simplified implementations for demonstration */
    class ConvolutionReverb {
    public:
        void initialize(const Config& config);
        void process(float* input, float* output, int numSamples);
        void loadImpulse(const std::vector<float>& impulse);
        void setGain(float gain) { gain_ = gain; }
        void setMix(float mix) { mix_ = mix; }
        
    private:
        std::vector<float> impulseResponse_;
        std::vector<float> delayLine_;
        float gain_ = 1.0f;
        float mix_ = 1.0f;
        int delayLength_ = 0;
        int writeIndex_ = 0;
    };
    
    class MultiBandProcessor {
    public:
        void initialize(const Config& config);
        void process(float* input, float* output, int numSamples);
        void setNumBands(int numBands);
        void setCrossoverFrequency(int bandIndex, float frequency);
        void setBandGain(int bandIndex, float gain);
        void setBandEnabled(int bandIndex, bool enabled);
        
    private:
        std::vector<juce::LinkwitzRileyFilter<float>> filters_;
        std::vector<float> bandGains_;
        std::vector<bool> bandEnabled_;
        int numBands_ = 4;
    };
    
    class AdaptiveEffects {
    public:
        void initialize(const Config& config);
        void process(float* input, float* output, int numSamples);
        void setAdaptationRate(float rate);
        void setDetectionThreshold(float threshold);
        void analyzeContent(const float* input, int numSamples);
        
    private:
        float adaptationRate_ = 0.01f;
        float detectionThreshold_ = 0.1f;
        float adaptiveGain_ = 1.0f;
        std::vector<float> analysisBuffer_;
        int analysisBufferSize_ = 0;
    };
    
    class ParameterMorph {
    public:
        void initialize(const Config& config);
        void processParameters(const std::map<int, float>& current, 
                              const std::map<int, float>& target, 
                              float progress);
        void setMorphingCurve(MorphingCurve curve);
        void setMorphingSpeed(float speed);
        
    private:
        MorphingCurve curve_ = MorphingCurve::Linear;
        float morphingSpeed_ = 1.0f;
        std::map<int, float> startValues_;
        std::map<int, float> targetValues_;
    };
    
    //==============================================================================
    /** Effect instances */
    std::unique_ptr<ConvolutionReverb> convolution_;
    std::unique_ptr<MultiBandProcessor> multiBand_;
    std::unique_ptr<AdaptiveEffects> adaptive_;
    std::unique_ptr<ParameterMorph> morph_;
    
    /** Preset management */
    std::map<std::string, EffectPreset> presets_;
    EffectPreset* currentPreset_ = nullptr;
    
    //==============================================================================
    /** Internal processing methods */
    void initializeEffects();
    void shutdownEffects();
    void processEffectChain(int numSamples);
    
    /** Effect-specific processing */
    void processConvolution(int numSamples);
    void processMultiBand(int numSamples);
    void processAdaptiveEffects(int numSamples);
    void processParameterMorphing(int numSamples);
    
    /** Parameter management */
    void updateParameters();
    void applyParameterSmoothing(int numSamples);
    void handleMorphingProgress();
    
    /** Performance and monitoring */
    void updatePerformanceStats();
    void validateMemoryUsage();
    void handleResourceManagement();
    
    /** Utility methods */
    void validateConfiguration();
    void allocateBuffers();
    void updateEngineState();
    void logMessage(const std::string& message, const std::string& level = "INFO");
    void logError(const std::string& error, const std::string& component = "ENGINE");
    
    //==============================================================================
    /** Threading */
    mutable std::mutex processMutex_;
    mutable std::mutex configMutex_;
    std::atomic<bool> shutdownRequested_{false};
    
    //==============================================================================
    /** Internal buffer management */
    std::vector<std::vector<float>> effectBuffers_;
    std::vector<float> tempBuffer_;
    juce::AudioBuffer<float> inputBuffer_;
    juce::AudioBuffer<float> outputBuffer_;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectsProcessingEngine)
};

} // namespace effects
} // namespace audio_engine
} // namespace vital
