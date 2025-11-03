/*
  ==============================================================================
    audio_quality_processor.h
    Copyright (c) 2025 Vital Audio Engine Team
    
    Ultra-low noise audio quality processor with dithering,
    anti-aliasing filters, dynamic range enhancement,
    stereo imaging, and high-quality sample rate conversion
  ==============================================================================
*/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <random>

namespace vital {
namespace audio_engine {
namespace audio_quality {

//==============================================================================
/**
 * @class AudioProcessor
 * @brief Ultra-low noise audio quality enhancement processor
 * 
 * Provides professional-grade audio quality improvements including:
 * - Ultra-low noise oscillators with advanced dithering
 * - Anti-aliasing filters with oversampling and interpolation
 * - Dynamic range enhancement with psychoacoustic noise shaping
 * - Stereo imaging with phase alignment and correlation control
 * - High-quality sample rate conversion with phase preservation
 * 
 * Optimized for real-time performance with measurable quality improvements.
 */
class AudioProcessor
{
public:
    //==============================================================================
    /** Configuration structure */
    struct Config
    {
        double sampleRate = 44100.0;
        int channels = 2;
        
        // Ultra-low noise settings
        bool enableUltraLowNoise = true;
        bool enableAdvancedDithering = true;
        DitherType ditherType = DitherType::kTriangular;
        float ditherAmount = 0.25f;
        bool addHarmonicDistortion = false;
        float harmonicLevel = 0.001f;
        bool enableDCCorrection = true;
        
        // Anti-aliasing settings
        bool enableAntiAliasing = true;
        AntiAliasType filterType = AntiAliasType::kWindowedSinc;
        int oversampleFactor = 4;
        float cutoffFrequency = 0.45f;  // Nyquist normalized
        float transitionWidth = 0.1f;
        float stopbandAttenuation = 100.0f; // dB
        bool enableOversampling = false;
        
        // Noise shaping settings
        bool enableNoiseShaping = true;
        NoiseShapingType shapingType = NoiseShapingType::kPsychoacoustic;
        int wordLength = 16;
        float shapingStrength = 0.5f;
        int feedbackDelay = 1;
        bool enableErrorFeedback = true;
        
        // Stereo imaging settings
        bool enableStereoProcessing = true;
        float phaseOffset = 0.0f; // radians
        float stereoWidth = 1.0f;
        float correlation = 1.0f;
        bool autoPhaseAlign = true;
        float calibrationTolerance = 0.001f;
        bool enableMidSideProcessing = false;
        
        // Sample rate conversion settings
        bool enableSampleRateConversion = false;
        ResampleQuality quality = ResampleQuality::kHigh;
        float targetSampleRate = 44100.0f;
        bool preservePhase = true;
        bool enableAntialias = true;
        int filterLength = 128;
        float resampleCutoff = 0.95f; // Nyquist normalized
        
        // Performance settings
        bool enableSIMD = true;
        bool enableMultithreading = false;
        float cpuLimit = 0.2f;
        bool enableRealTimeOptimization = true;
        
        // Quality modes
        bool highQualityMode = true;
        bool ultraQualityMode = false;
        bool performanceMode = false;
    };
    
    //==============================================================================
    /** Dithering types */
    enum class DitherType {
        kNoDither = 0,
        kRectangular,
        kTriangular,
        kGaussian,
        kBlueNoise,
        kErrorFeedback,
        kTriangularPDF,
        kWGN
    };
    
    /** Anti-aliasing filter types */
    enum class AntiAliasType {
        kNoAlias = 0,
        kLinear,
        kHermite,
        kCubic,
        kWindowedSinc,
        kFIR,
        kButterworth,
        kChebyshev
    };
    
    /** Noise shaping types */
    enum class NoiseShapingType {
        kNoShaping = 0,
        kSimple,
        kPsychoacoustic,
        kWeightedNoise,
        kSpectral,
        kAdaptive
    };
    
    /** Sample rate conversion quality */
    enum class ResampleQuality {
        kFast = 0,
        kMedium,
        kHigh,
        kUltra,
        kMaximum
    };
    
    //==============================================================================
    /** Performance metrics */
    struct PerformanceMetrics
    {
        float cpuUsagePercent = 0.0f;
        float latencySamples = 0.0f;
        float dynamicRangeImprovementDb = 0.0f;
        float thdReductionDb = 0.0f;
        float snrImprovementDb = 0.0f;
        float stereoImagingScore = 0.0f;
        int totalSamplesProcessed = 0;
        int totalDitherBits = 0;
        int aliasingArtifactsReduced = 0;
        float averageProcessingTimeUs = 0.0f;
        
        struct ComponentMetrics {
            float cpuUsage = 0.0f;
            float qualityScore = 0.0f;
            int samplesProcessed = 0;
            float averageLatency = 0.0f;
        } dithering, antiAliasing, noiseShaping, stereoProcessing, resampling;
    };
    
    //==============================================================================
    /** Engine state */
    struct EngineState
    {
        bool isInitialized = false;
        bool isProcessing = false;
        bool isSuspended = false;
        bool isBypassed = false;
        
        float currentCPUUsage = 0.0f;
        int currentLatencySamples = 0;
        int activeChannels = 0;
        float memoryUsage = 0.0f;
        
        std::chrono::steady_clock::time_point lastUpdate;
        
        struct ComponentState {
            bool enabled = true;
            bool active = false;
            float processingTime = 0.0f;
            std::string status;
            int samplesProcessed = 0;
        };
        
        ComponentState ultraLowNoise;
        ComponentState antiAliasing;
        ComponentState noiseShaping;
        ComponentState stereoProcessing;
        ComponentState sampleRateConversion;
    };
    
    //==============================================================================
    /** Constructor */
    explicit AudioProcessor(const Config& config = Config());
    
    /** Destructor */
    ~AudioProcessor();
    
    //==============================================================================
    /** Initialize the audio quality processor */
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
    /** Ultra-low noise oscillator control */
    void enableUltraLowNoise(bool enable);
    void setDitherType(DitherType type, float amount = 0.25f);
    void setDitherAmount(float amount);
    void enableHarmonicDistortion(bool enable, float level = 0.001f);
    void enableDCCorrection(bool enable);
    void autoCalibrate();
    float getDCOffset() const;
    
    //==============================================================================
    /** Anti-aliasing filter control */
    void enableAntiAliasing(bool enable);
    void setFilterType(AntiAliasType type);
    void setOversampleFactor(int factor);
    void setCutoffFrequency(float cutoff); // Nyquist normalized
    void setTransitionWidth(float width);
    void setStopbandAttenuation(float attenuation);
    void designFilter();
    
    /** Interpolation control */
    void setInterpolationType(AntiAliasType type);
    void enableOversampling(bool enable);
    int getOversampleFactor() const;
    
    //==============================================================================
    /** Dynamic range enhancement control */
    void enableNoiseShaping(bool enable);
    void setNoiseShapingType(NoiseShapingType type);
    void setWordLength(int bits);
    void setShapingStrength(float strength);
    void setFeedbackDelay(int delaySamples);
    void enableErrorFeedback(bool enable);
    void resetFeedbackBuffer();
    
    //==============================================================================
    /** Stereo imaging control */
    void enableStereoProcessing(bool enable);
    void setPhaseOffset(float offsetRadians);
    void setStereoWidth(float width);
    void setCorrelation(float correlation);
    void enableAutoPhaseAlignment(bool enable);
    void setCalibrationTolerance(float tolerance);
    void enableMidSideProcessing(bool enable);
    
    /** Phase alignment */
    void autoAlignPhase(float* left, float* right, int numSamples);
    float measurePhaseDifference(const float* left, const float* right, int numSamples);
    
    //==============================================================================
    /** Sample rate conversion control */
    void enableSampleRateConversion(bool enable);
    void setQuality(ResampleQuality quality);
    void setTargetSampleRate(float sampleRate);
    void enablePhasePreservation(bool enable);
    void enableResampleAntialias(bool enable);
    void setFilterLength(int length);
    
    //==============================================================================
    /** Global controls */
    void setMasterGain(float gain);
    void setBypass(bool bypassed);
    void setQualityMode(int mode); // 0=performance, 1=balanced, 2=quality, 3=ultra
    void setProcessingChain(const std::vector<bool>& enabledComponents);
    
    float getMasterGain() const { return masterGain_; }
    bool isBypassed() const { return engineState_.isBypassed; }
    
    //==============================================================================
    /** Analysis and measurement */
    float measureDynamicRange(const float* samples, int numSamples);
    float measureTHD(const float* samples, int numSamples, float fundamental);
    float measureSNR(const float* samples, int numSamples);
    float measureStereoImaging(float* left, float* right, int numSamples);
    float measureAliasingReduction(const float* before, const float* after, int numSamples);
    
    /** Spectral analysis */
    void enableSpectralAnalysis(bool enable);
    void setAnalysisWindow(int windowType);
    void setFrequencyBins(int bins);
    const std::vector<float>& getSpectrum(int channel = 0) const;
    
    //==============================================================================
    /** Performance monitoring */
    EngineState getEngineState() const;
    PerformanceMetrics getPerformanceMetrics() const;
    
    /** Real-time metrics */
    struct RealTimeMetrics {
        float currentCPUUsage = 0.0f;
        float currentLatencyMs = 0.0f;
        float currentDynamicRange = 0.0f;
        float currentStereoImaging = 0.0f;
        int currentOversampleFactor = 1;
        float currentTHD = 0.0f;
        float currentSNR = 0.0f;
    };
    
    RealTimeMetrics getRealTimeMetrics() const;
    void enablePerformanceMonitoring(bool enabled);
    void resetMetrics();
    
    //==============================================================================
    /** Quality presets */
    void loadQualityPreset(int presetId);
    void saveQualityPreset(int presetId, const std::string& name);
    std::vector<std::string> getQualityPresetNames() const;
    
    /** Quality reporting */
    struct QualityReport {
        float overallScore = 0.0f;
        float snrScore = 0.0f;
        float thdScore = 0.0f;
        float stereoScore = 0.0f;
        float aliasingScore = 0.0f;
        float noiseScore = 0.0f;
        std::string recommendations;
    };
    
    QualityReport generateQualityReport();
    
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
    
private:
    //==============================================================================
    /** Configuration */
    Config config_;
    
    /** Engine state */
    EngineState engineState_;
    
    /** Control parameters */
    float masterGain_ = 1.0f;
    float currentDCOffset_ = 0.0f;
    int currentQualityMode_ = 2; // 2 = balanced quality
    
    //==============================================================================
    /** Processing components - simplified implementations */
    
    class UltraLowNoiseProcessor {
    public:
        void initialize(const Config& config);
        void process(float* samples, int numSamples, int channel);
        void setDitherType(DitherType type, float amount);
        void setHarmonicDistortion(bool enable, float level);
        void autoCalibrate();
        
    private:
        DitherType ditherType_ = DitherType::kTriangular;
        float ditherAmount_ = 0.25f;
        bool enableHarmonicDistortion_ = false;
        float harmonicLevel_ = 0.001f;
        float dcOffset_ = 0.0f;
        
        void generateDither(float* ditherBuffer, int numSamples);
        void applyDithering(float* samples, int numSamples);
        void correctDC(float* samples, int numSamples);
    };
    
    class AntiAliasingProcessor {
    public:
        void initialize(const Config& config);
        void processOversampled(const float* input, float* output, int numSamples);
        void setFilterType(AntiAliasType type);
        void setOversampleFactor(int factor);
        void designFilter();
        
    private:
        AntiAliasType filterType_ = AntiAliasType::kWindowedSinc;
        int oversampleFactor_ = 4;
        std::vector<float> filterCoeffs_;
        std::vector<float> filterState_;
        int filterLength_ = 0;
        
        void applyLinearInterpolation(const float* input, float* output, int numSamples);
        void applyHermiteInterpolation(const float* input, float* output, int numSamples);
        void applyCubicInterpolation(const float* input, float* output, int numSamples);
        void applyWindowedSinc(const float* input, float* output, int numSamples);
    };
    
    class NoiseShapingProcessor {
    public:
        void initialize(const Config& config);
        void process(float* samples, int numSamples);
        void setShapingType(NoiseShapingType type);
        void setFeedbackDelay(int delay);
        void resetFeedback();
        
    private:
        NoiseShapingType shapingType_ = NoiseShapingType::kPsychoacoustic;
        int feedbackDelay_ = 1;
        std::vector<float> feedbackBuffer_;
        float previousError_ = 0.0f;
        int feedbackIndex_ = 0;
        
        void applySimpleShaping(float* samples, int numSamples);
        void applyPsychoacousticShaping(float* samples, int numSamples);
        float calculateErrorSignal(float sample, float quantized);
    };
    
    class StereoProcessor {
    public:
        void initialize(const Config& config);
        void process(const float* leftIn, const float* rightIn,
                    float* leftOut, float* rightOut, int numSamples);
        void setPhaseOffset(float offset);
        void setStereoWidth(float width);
        void setCorrelation(float correlation);
        void autoAlignPhase(float* left, float* right, int numSamples);
        
    private:
        float phaseOffset_ = 0.0f;
        float stereoWidth_ = 1.0f;
        float correlation_ = 1.0f;
        std::vector<float> delayBuffer_;
        int delayLength_ = 0;
        
        void applyPhaseShift(float* samples, int numSamples, float phaseShift);
        void widenStereo(float* left, float* right, int numSamples);
    };
    
    class ResamplingProcessor {
    public:
        void initialize(const Config& config);
        void process(const float* input, float* output, int numInput, int numOutput);
        void setQuality(ResampleQuality quality);
        void enablePhasePreservation(bool enable);
        
    private:
        ResampleQuality quality_ = ResampleQuality::kHigh;
        bool preservePhase_ = true;
        std::vector<float> resampleFilter_;
        std::vector<float> phaseTable_;
        float ratio_ = 1.0f;
        
        void designPolyphaseFilter();
        void applyPolyphaseResampling(const float* input, const float* filter,
                                     float* output, int numInput, int numOutput);
    };
    
    //==============================================================================
    /** Component instances */
    std::unique_ptr<UltraLowNoiseProcessor> ultraLowNoise_;
    std::unique_ptr<AntiAliasingProcessor> antiAlias_;
    std::unique_ptr<NoiseShapingProcessor> noiseShaping_;
    std::unique_ptr<StereoProcessor> stereo_;
    std::unique_ptr<ResamplingProcessor> resampling_;
    
    //==============================================================================
    /** Processing buffers */
    std::vector<std::vector<float>> inputBuffers_;
    std::vector<std::vector<float>> outputBuffers_;
    std::vector<std::vector<float>> tempBuffers_;
    std::vector<std::vector<float>> oversampleBuffers_;
    
    /** Spectral analysis buffers */
    std::vector<std::vector<float>> spectrumBuffers_;
    std::vector<std::vector<std::complex<float>>> fftBuffers_;
    std::unique_ptr<juce::FFT> fft_;
    
    //==============================================================================
    /** Performance monitoring */
    PerformanceMetrics metrics_;
    std::atomic<bool> performanceMonitoringEnabled_{false};
    std::chrono::steady_clock::time_point startTime_;
    
    /** Error tracking */
    ErrorInfo lastError_;
    std::atomic<bool> hasError_{false};
    
    //==============================================================================
    /** Internal processing methods */
    void initializeComponents();
    void shutdownComponents();
    void initializeBuffers();
    void validateConfiguration();
    
    /** Main processing chain */
    void processUltraLowNoise(int numSamples);
    void processAntiAliasing(int numSamples);
    void processNoiseShaping(int numSamples);
    void processStereo(int numSamples);
    void processResampling(int numSamples);
    
    /** Quality measurement */
    void updatePerformanceMetrics();
    void measureAudioQuality();
    void calculateOverallScore();
    
    /** Utility methods */
    void updateEngineState();
    void validateBuffers();
    void logMessage(const std::string& message, const std::string& level = "INFO");
    void logError(const std::string& error, const std::string& component = "AUDIO_QUALITY");
    
    //==============================================================================
    /** Random number generation for dithering */
    std::random_device randomDevice_;
    std::mt19937 randomGenerator_;
    
    void initializeRandomGenerators();
    float generateWhiteNoise();
    float generateTriangularNoise();
    float generateGaussianNoise();
    
    //==============================================================================
    /** Threading */
    mutable std::mutex processMutex_;
    mutable std::mutex configMutex_;
    std::atomic<bool> shutdownRequested_{false};
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioProcessor)
};

} // namespace audio_quality
} // namespace audio_engine
} // namespace vital
