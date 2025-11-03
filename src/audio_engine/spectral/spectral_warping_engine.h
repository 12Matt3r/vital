/*
  ==============================================================================
    spectral_warping_engine.h
    Copyright (c) 2025 Vital Audio Engine Team
    
    Real-time spectral warping engine with phase vocoding,
    harmonic phase manipulation, and spectral envelope shaping
    from phase3 spectral_warping module
  ==============================================================================
*/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_dsp/FFT/juce_FFT.h>
#include <vector>
#include <memory>
#include <atomic>
#include <complex>
#include <mutex>

namespace vital {
namespace audio_engine {
namespace spectral {

//==============================================================================
/**
 * @class SpectralWarpingEngine
 * @brief Real-time spectral processing and warping engine
 * 
 * Provides real-time spectral manipulation including:
 * - Phase vocoding for time-stretching and pitch-shifting
 * - Harmonic phase manipulation for spectral transformations
 * - Spectral envelope shaping with formant preservation
 * - Real-time FFT processing with optimized algorithms
 * - Spectral interpolation for smooth morphing
 * 
 * Optimized for low-latency real-time performance with <10ms processing delay.
 */
class SpectralWarpingEngine
{
public:
    //==============================================================================
    /** Engine configuration */
    struct Config
    {
        double sampleRate = 44100.0;
        int bufferSize = 512;
        int fftSize = 1024;
        int overlapFactor = 4;
        int numChannels = 2;
        
        // Quality settings
        bool highQualityMode = true;
        bool enableOversampling = false;
        int oversamplingFactor = 2;
        bool preserveFormants = true;
        
        // Performance settings
        bool enableSIMD = true;
        bool enableMultithreading = false;
        int maxWorkerThreads = 2;
        float cpuLimit = 0.3f; // Conservative limit for real-time use
        
        // Phase vocoder settings
        bool enablePhaseVocoder = true;
        float phaseLockThreshold = 0.5f;
        bool enableSpectralSmoothing = true;
        float spectralSmoothingFactor = 0.5f;
        
        // Warping parameters
        bool enableSpectralWarping = true;
        float warpIntensity = 0.5f; // 0.0 to 1.0
        float warpFrequency = 0.1f; // Hz
        int warpMode = 0; // 0=linear, 1=logarithmic, 2=exponential
        
        // Harmonic manipulation
        bool enableHarmonicManipulation = true;
        int numHarmonics = 8;
        float harmonicPhaseOffset = 0.0f;
        bool enableHarmonicLocking = true;
        
        // Envelope shaping
        bool enableEnvelopeShaping = true;
        float envelopeSpeed = 1.0f;
        bool preserveSpectralCentroid = true;
        bool enableFormantPreservation = true;
        
        // Interpolation settings
        bool enableSpectralInterpolation = true;
        float interpolationSpeed = 1.0f;
        int interpolationQuality = 2; // 0=fast, 1=medium, 2=high
    };
    
    //==============================================================================
    /** Warping modes */
    enum class WarpMode {
        Linear = 0,
        Logarithmic = 1,
        Exponential = 2,
        Custom = 3
    };
    
    /** Phase processing modes */
    enum class PhaseMode {
        Standard = 0,
        PhaseLocked = 1,
        HarmonicLocked = 2,
        Adaptive = 3
    };
    
    /** Interpolation types */
    enum class InterpolationType {
        Nearest = 0,
        Linear = 1,
        Cubic = 2,
        Hermite = 3,
        Spline = 4
    };
    
    //==============================================================================
    /** Spectral analysis data */
    struct SpectralFrame {
        std::vector<float> magnitude;
        std::vector<float> phase;
        std::vector<float> frequency;
        std::vector<float> amplitude;
        float spectralCentroid = 0.0f;
        float spectralRolloff = 0.0f;
        float spectralFlatness = 0.0f;
        int numBins = 0;
        double timestamp = 0.0;
        
        SpectralFrame() = default;
        explicit SpectralFrame(int bins) : magnitude(bins), phase(bins), 
                                         frequency(bins), amplitude(bins), numBins(bins) {}
    };
    
    /** Warping parameters for real-time control */
    struct WarpParameters {
        bool enabled = true;
        float intensity = 0.5f;
        float frequency = 0.1f;
        WarpMode mode = WarpMode::Linear;
        std::vector<float> customCurve;
        
        // Harmonic parameters
        bool harmonicLocking = true;
        int numHarmonics = 8;
        float harmonicPhaseOffset = 0.0f;
        
        // Envelope parameters
        float envelopeSpeed = 1.0f;
        bool preserveFormants = true;
        bool preserveCentroid = true;
    };
    
    //==============================================================================
    /** Performance statistics */
    struct PerformanceStats {
        float averageCPULoad = 0.0f;
        float peakCPULoad = 0.0f;
        float averageLatencyMs = 0.0f;
        int totalFramesProcessed = 0;
        int totalSamplesProcessed = 0;
        float totalUptime = 0.0f;
        int xruns = 0;
        float memoryUsage = 0.0f;
        
        struct FFTStats {
            float averageFFTTime = 0.0f;
            float peakFFTTime = 0.0f;
            int totalFFTs = 0;
            float efficiency = 0.0f;
        } fft;
        
        struct WarpStats {
            float averageWarpTime = 0.0f;
            int totalWarps = 0;
            float averageIntensity = 0.0f;
        } warping;
    };
    
    //==============================================================================
    /** Engine state information */
    struct EngineState {
        bool isInitialized = false;
        bool isProcessing = false;
        bool isSuspended = false;
        bool isBypassed = false;
        
        int currentLatencySamples = 0;
        float currentCPUUsage = 0.0f;
        int activeChannels = 0;
        int framesProcessed = 0;
        
        std::chrono::steady_clock::time_point lastUpdate;
        
        struct ComponentState {
            bool enabled = true;
            bool active = false;
            float processingTime = 0.0f;
            std::string status;
        };
        
        ComponentState fft;
        ComponentState phaseVocoder;
        ComponentState warping;
        ComponentState harmonicManipulation;
        ComponentState envelopeShaping;
        ComponentState interpolation;
    };
    
    //==============================================================================
    /** Constructor */
    explicit SpectralWarpingEngine(const Config& config = Config());
    
    /** Destructor */
    ~SpectralWarpingEngine();
    
    //==============================================================================
    /** Initialize the spectral engine */
    bool initialize();
    
    /** Shutdown and cleanup */
    void shutdown();
    
    /** Reset to initial state */
    void reset();
    
    /** Suspend/resume processing */
    void suspend();
    void resume();
    
    //==============================================================================
    /** Main audio processing */
    void process(int numSamples);
    
    /** Process audio buffer */
    void processBuffer(const juce::AudioBuffer<float>& input,
                      juce::AudioBuffer<float>& output);
    
    /** Process single sample */
    float processSample(float input, int channel = 0);
    
    //==============================================================================
    /** Spectral warping control */
    void setWarpingEnabled(bool enabled);
    void setWarpIntensity(float intensity);
    void setWarpFrequency(float frequency);
    void setWarpMode(WarpMode mode);
    void setCustomWarpCurve(const std::vector<float>& curve);
    
    /** Warping presets */
    void loadWarpPreset(int presetId);
    void saveWarpPreset(int presetId, const std::string& name);
    std::vector<std::string> getWarpPresetNames() const;
    
    //==============================================================================
    /** Phase vocoder control */
    void setPhaseVocoderEnabled(bool enabled);
    void setPhaseMode(PhaseMode mode);
    void setPhaseLockThreshold(float threshold);
    void setSpectralSmoothing(bool enabled, float factor = 0.5f);
    
    /** Time and pitch manipulation */
    void setTimeStretchFactor(float factor); // 0.5 = half speed, 2.0 = double speed
    void setPitchShiftFactor(float factor);  // 0.5 = one octave down, 2.0 = up
    void setFormantPreservation(bool enabled);
    
    //==============================================================================
    /** Harmonic manipulation control */
    void setHarmonicManipulationEnabled(bool enabled);
    void setNumHarmonics(int num);
    void setHarmonicPhaseOffset(float offset);
    void setHarmonicLocking(bool enabled);
    void setHarmonicAmplitude(int harmonicIndex, float amplitude);
    void setHarmonicPhase(int harmonicIndex, float phase);
    
    //==============================================================================
    /** Envelope shaping control */
    void setEnvelopeShapingEnabled(bool enabled);
    void setEnvelopeSpeed(float speed);
    void setFormantPreservation(bool enabled);
    void setSpectralCentroidPreservation(bool enabled);
    void setEnvelopeAttack(float attack);
    void setEnvelopeRelease(float release);
    
    //==============================================================================
    /** Spectral interpolation control */
    void setInterpolationEnabled(bool enabled);
    void setInterpolationType(InterpolationType type);
    void setInterpolationSpeed(float speed);
    void setInterpolationQuality(int quality);
    
    /** Morphing control */
    void startSpectralMorph(float targetIntensity, float duration = 1.0f);
    void setMorphingCurve(const std::vector<std::pair<float, float>>& curve);
    float getMorphingProgress() const;
    
    //==============================================================================
    /** Spectral analysis access */
    const SpectralFrame* getCurrentFrame(int channel = 0) const;
    const SpectralFrame* getPreviousFrame(int channel = 0) const;
    float getSpectralCentroid(int channel = 0) const;
    float getSpectralRolloff(int channel = 0) const;
    float getSpectralFlatness(int channel = 0) const;
    
    /** Analysis utilities */
    void enableSpectralAnalysis(bool enabled);
    void setAnalysisWindow(int windowType); // 0=rectangular, 1=hanning, 2=blackman, etc.
    void setFrequencyRange(float minFreq, float maxFreq);
    
    //==============================================================================
    /** Global controls */
    void setMasterGain(float gain);
    void setMix(float mix);
    void setBypass(bool bypassed);
    void setWetDryBalance(float wet, float dry);
    
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
        float currentLatencyMs = 0.0f;
        int currentFrameSize = 0;
        int currentOverlap = 0;
        float spectralCentroid = 0.0f;
        float warpIntensity = 0.0f;
        float phaseLockQuality = 0.0f;
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
    /** State queries */
    bool isInitialized() const { return engineState_.isInitialized; }
    bool isProcessing() const { return engineState_.isProcessing; }
    bool isSuspended() const { return engineState_.isSuspended; }
    
    //==============================================================================
    /** MIDI integration */
    void setMIDIControlledParameter(int paramId);
    void setMIDIControlRange(int paramId, float minValue, float maxValue);
    void processMIDIControl(int controller, float value);
    
    //==============================================================================
    /** Parameter automation */
    void setParameterAutomation(int paramId, const std::vector<float>& automation);
    void setParameterModulation(int paramId, int source, float depth);
    void enableParameterSmoothing(int paramId, float timeMs);
    
private:
    //==============================================================================
    /** Configuration */
    Config config_;
    
    /** Engine state */
    EngineState engineState_;
    
    /** Control parameters */
    float masterGain_ = 1.0f;
    float mix_ = 1.0f;
    float timeStretchFactor_ = 1.0f;
    float pitchShiftFactor_ = 1.0f;
    std::vector<float> customWarpCurve_;
    
    /** Warping parameters */
    WarpParameters currentWarpParams_;
    WarpParameters targetWarpParams_;
    float morphingProgress_ = 0.0f;
    float morphingDuration_ = 1.0f;
    std::vector<std::pair<float, float>> morphingCurve_;
    
    /** Phase processing */
    PhaseMode currentPhaseMode_ = PhaseMode::Standard;
    float phaseLockThreshold_ = 0.5f;
    bool spectralSmoothingEnabled_ = true;
    float spectralSmoothingFactor_ = 0.5f;
    
    //==============================================================================
    /** FFT and spectral processing */
    std::unique_ptr<juce::FFT> fft_;
    std::vector<std::unique_ptr<juce::FFT>> channelFFTs_;
    
    /** Spectral frames for each channel */
    std::vector<std::vector<SpectralFrame>> spectralFrames_;
    std::vector<std::vector<SpectralFrame>> spectralHistory_;
    
    /** Windowing */
    std::vector<float> analysisWindow_;
    std::vector<float> synthesisWindow_;
    
    /** Phase vocoder state */
    std::vector<std::vector<float>> previousPhases_;
    std::vector<std::vector<float>> accumulatedPhases_;
    std::vector<std::vector<float>> instantaneousFreq_;
    
    //==============================================================================
    /** Processing buffers */
    std::vector<std::vector<float>> inputBuffers_;
    std::vector<std::vector<float>> outputBuffers_;
    std::vector<std::vector<float>> overlapBuffers_;
    std::vector<std::vector<float>> hannWindowBuffer_;
    
    /** Analysis buffers */
    std::vector<std::vector<float>> magnitudeBuffer_;
    std::vector<std::vector<float>> phaseBuffer_;
    std::vector<std::vector<float>> frequencyBuffer_;
    
    //==============================================================================
    /** Performance monitoring */
    PerformanceStats performanceStats_;
    std::atomic<bool> performanceMonitoringEnabled_{false};
    std::chrono::steady_clock::time_point startTime_;
    
    /** Error tracking */
    ErrorInfo lastError_;
    std::atomic<bool> hasError_{false};
    
    //==============================================================================
    /** Internal processing methods */
    void initializeFFT();
    void initializeBuffers();
    void initializeWindows();
    void validateConfiguration();
    
    /** FFT processing */
    void performFFTAnalysis(int channel);
    void performFFTSynthesis(int channel);
    void applyWindowing(float* data, int size, const std::vector<float>& window);
    
    /** Phase vocoder processing */
    void updatePhaseVocoder(int channel);
    void applyPhaseLocking(int channel);
    void applySpectralSmoothing(int channel);
    
    /** Spectral warping */
    void applySpectralWarping(int channel);
    void applyHarmonicManipulation(int channel);
    void applyEnvelopeShaping(int channel);
    
    /** Interpolation */
    void applySpectralInterpolation(int channel);
    float interpolateValue(float value1, float value2, float progress, InterpolationType type);
    
    /** Overlap-add processing */
    void performOverlapAdd(int channel);
    void manageOverlapBuffers(int channel);
    
    /** Parameter processing */
    void updateWarpParameters();
    void applyParameterSmoothing();
    void handleMorphingProgress();
    
    /** Performance monitoring */
    void updatePerformanceStats();
    void measureFFTPerformance();
    void measureWarpPerformance();
    
    /** Utility methods */
    void updateEngineState();
    void validateBuffers();
    void logMessage(const std::string& message, const std::string& level = "INFO");
    void logError(const std::string& error, const std::string& component = "SPECTRAL");
    
    //==============================================================================
    /** Threading */
    mutable std::mutex processMutex_;
    mutable std::mutex configMutex_;
    std::atomic<bool> shutdownRequested_{false};
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SpectralWarpingEngine)
};

} // namespace spectral
} // namespace audio_engine
} // namespace vital
