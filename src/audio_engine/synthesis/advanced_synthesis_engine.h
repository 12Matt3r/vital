/*
  ==============================================================================
    advanced_synthesis_engine.h
    Copyright (c) 2025 Vital Audio Engine Team
    
    Advanced synthesis engine integrating modal, physical modeling,
    granular synthesis, and hybrid analog modeling from phase3
  ==============================================================================
*/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <memory>
#include <array>
#include <atomic>
#include <mutex>

namespace vital {
namespace audio_engine {
namespace synthesis {

//==============================================================================
/**
 * @class AdvancedSynthesisEngine
 * @brief Main advanced synthesis engine coordinating all synthesis types
 * 
 * This engine provides:
 * - Modal synthesis with multiple resonant modes
 * - Physical modeling synthesis (Karplus-Strong, impedance modeling)
 * - Granular synthesis with cloud processing
 * - Hybrid analog modeling with virtual components
 * - Advanced envelope generation
 * 
 * Integrates all phase3 synthesis improvements with optimal performance.
 */
class AdvancedSynthesisEngine
{
public:
    //==============================================================================
    /** Engine configuration */
    struct Config
    {
        double sampleRate = 44100.0;
        int bufferSize = 512;
        int maxChannels = 2;
        int maxVoices = 32;
        
        // Synthesis types
        bool enableModalSynthesis = true;
        bool enablePhysicalModeling = true;
        bool enableGranularSynthesis = true;
        bool enableHybridAnalog = true;
        bool enableAdvancedEnvelopes = true;
        
        // Modal synthesis settings
        int maxModes = 16;
        float modeDecay = 0.995f;
        
        // Physical modeling settings
        int stringCount = 4;
        int filterOrder = 4;
        bool enableNonLinearities = true;
        
        // Granular synthesis settings
        int grainSize = 1024;
        int grainOverlap = 4;
        int maxGrains = 64;
        bool enablePitchShifting = true;
        
        // Hybrid analog settings
        int oscillatorCount = 8;
        int filterStages = 4;
        bool enableOversampling = true;
        int oversamplingFactor = 2;
        
        // Advanced envelopes
        int envelopeStages = 8;
        bool enableRandomWalk = true;
        bool enableBreaking = true;
        
        // Quality settings
        bool highQualityMode = true;
        bool enableSIMD = true;
        bool enableMultithreading = false;
    };
    
    //==============================================================================
    /** Voice state information */
    struct VoiceState
    {
        int id = -1;
        int note = -1;
        float velocity = 0.0f;
        float frequency = 440.0f;
        float amplitude = 1.0f;
        int channel = 0;
        bool active = false;
        bool sustained = false;
        
        // Synthesis type
        int synthesisType = 0;
        
        // State parameters
        std::vector<float> parameters;
        std::chrono::steady_clock::time_point startTime;
        
        VoiceState() {
            parameters.resize(128, 0.0f);
        }
    };
    
    //==============================================================================
    /** Synthesis types */
    enum class SynthesisType {
        Modal,
        PhysicalModeling,
        Granular,
        HybridAnalog,
        AdvancedEnvelopes,
        MultiHybrid
    };
    
    /** Processing modes */
    enum class ProcessingMode {
        Parallel,
        Sequential,
        Layered,
        CrossModulated,
        Adaptive
    };
    
    //==============================================================================
    /** Constructor */
    explicit AdvancedSynthesisEngine();
    
    /** Destructor */
    ~AdvancedSynthesisEngine();
    
    //==============================================================================
    /** Initialize synthesis engine */
    bool initialize(const Config& config);
    
    /** Shutdown and cleanup */
    void shutdown();
    
    /** Reset to initial state */
    void reset();
    
    /** Suspend/resume processing */
    void suspend();
    void resume();
    
    //==============================================================================
    /** Main processing */
    void process(int numSamples);
    
    /** Process single sample */
    float processSample(float input, int channel = 0);
    
    //==============================================================================
    /** Voice management */
    void allocateVoice(int voiceId);
    void deallocateVoice(int voiceId);
    void setVoiceNote(int voiceId, int note);
    void setVoiceVelocity(int voiceId, float velocity);
    void setVoiceChannel(int voiceId, int channel);
    void setVoiceMute(int voiceId, bool muted);
    void setVoiceNoteOff(int voiceId);
    
    /** Voice state access */
    const VoiceState* getVoiceState(int voiceId) const;
    int getNumActiveVoices() const;
    
    //==============================================================================
    /** Synthesis type management */
    void enableSynthesisType(SynthesisType type, bool enabled);
    void setProcessingMode(ProcessingMode mode);
    void setSynthesisWeight(SynthesisType type, float weight);
    void setCrossfadeTime(float time);
    
    //==============================================================================
    /** Parameter management */
    void setParameter(int paramId, float value);
    float getParameter(int paramId) const;
    void setGlobalParameters(const std::vector<float>& parameters);
    
    /** Parameter modulation */
    void setParameterModulation(int paramId, int source, float depth);
    void setParameterCurve(int paramId, int curveType, const std::vector<float>& points);
    
    //==============================================================================
    /** Global controls */
    void setMasterGain(float gain);
    void setMasterTune(float cents);
    void setGlobalTuning(float tuning);
    void setBypassed(bool bypassed);
    
    float getMasterGain() const { return masterGain_; }
    float getMasterTune() const { return masterTuneCents_; }
    bool isBypassed() const { return bypassed_; }
    
    //==============================================================================
    /** MIDI handling */
    void allNotesOff(int channel = 0);
    
    //==============================================================================
    /** Performance monitoring */
    struct PerformanceStats {
        float cpuUsage = 0.0f;
        float voiceUtilization = 0.0f;
        int activeVoices = 0;
        float averageLatencyMs = 0.0f;
        size_t memoryUsage = 0;
        
        struct SynthesisStats {
            float modalCPU = 0.0f;
            float physicalCPU = 0.0f;
            float granularCPU = 0.0f;
            float analogCPU = 0.0f;
            float envelopeCPU = 0.0f;
        } synthesis;
    };
    
    PerformanceStats getPerformanceStats() const;
    
    //==============================================================================
    /** Utility functions */
    float noteNumberToFrequency(int noteNumber);
    int frequencyToNoteNumber(float frequency);
    float centsToRatio(float cents);
    float ratioToCents(float ratio);
    
private:
    //==============================================================================
    /** Configuration */
    Config config_;
    
    /** Engine state */
    std::atomic<bool> isInitialized_{false};
    std::atomic<bool> isProcessing_{false};
    std::atomic<bool> isSuspended_{false};
    std::atomic<bool> bypassed_{false};
    
    /** Control parameters */
    float masterGain_ = 1.0f;
    float masterTuneCents_ = 0.0f;
    ProcessingMode processingMode_ = ProcessingMode::Parallel;
    
    //==============================================================================
    /** Voice management */
    std::vector<VoiceState> voices_;
    std::vector<bool> activeVoices_;
    std::vector<bool> synthesisEnabled_;
    std::vector<float> synthesisWeights_;
    
    /** Parameter management */
    std::vector<float> globalParameters_;
    std::vector<std::vector<float>> parameterAutomation_;
    std::vector<std::vector<std::pair<int, float>>> parameterModulation_;
    std::vector<std::vector<float>> parameterCurves_;
    
    //==============================================================================
    /** Processing buffers */
    std::vector<std::vector<float>> synthesisOutputs_;
    std::vector<std::vector<float>> voiceOutputs_;
    std::vector<std::vector<float>> mixedOutputs_;
    
    //==============================================================================
    /** Modal synthesis components */
    class ModalSynthesizer {
    public:
        struct Mode {
            float frequency = 440.0f;
            float amplitude = 1.0f;
            float decay = 0.99f;
            float phase = 0.0f;
        };
        
        void initialize(int maxModes, float sampleRate);
        void process(float* output, int numSamples, const std::vector<Mode>& modes);
        void setMode(int modeId, const Mode& mode);
        
    private:
        std::vector<Mode> modes_;
        float sampleRate_ = 44100.0f;
        int maxModes_ = 0;
    };
    
    //==============================================================================
    /** Physical modeling components */
    class PhysicalModelingSynthesizer {
    public:
        void initialize(int stringCount, float sampleRate);
        void processKarplusStrong(float* output, int numSamples, float frequency, float excitation);
        void processImpedance(float* output, int numSamples, const std::vector<float>& impedance);
        
    private:
        std::vector<std::vector<float>> delayLines_;
        std::vector<float> stringFrequencies_;
        int stringCount_ = 0;
        float sampleRate_ = 44100.0f;
    };
    
    //==============================================================================
    /** Granular synthesis components */
    class GranularSynthesizer {
    public:
        struct Grain {
            float position = 0.0f;
            float duration = 1.0f;
            float amplitude = 1.0f;
            float pitch = 1.0f;
            bool active = false;
        };
        
        void initialize(int maxGrains, int grainSize, float sampleRate);
        void process(float* output, int numSamples, const std::vector<float>& source);
        void addGrain(const Grain& grain);
        
    private:
        std::vector<Grain> grains_;
        std::vector<float> grainBuffer_;
        int maxGrains_ = 0;
        int grainSize_ = 1024;
        float sampleRate_ = 44100.0f;
    };
    
    //==============================================================================
    /** Hybrid analog components */
    class HybridAnalogSynthesizer {
    public:
        void initialize(int oscillatorCount, float sampleRate);
        void process(float* output, int numSamples);
        void setOscillatorType(int oscId, int type);
        void setOscillatorFrequency(int oscId, float frequency);
        
    private:
        struct VirtualOscillator {
            float frequency = 440.0f;
            float amplitude = 1.0f;
            int type = 0;
            float phase = 0.0f;
        };
        
        std::vector<VirtualOscillator> oscillators_;
        float sampleRate_ = 44100.0f;
        int oscillatorCount_ = 0;
    };
    
    //==============================================================================
    /** Advanced envelope components */
    class AdvancedEnvelopeSynthesizer {
    public:
        void initialize(int envelopeStages, float sampleRate);
        void process(float* output, int numSamples, const std::vector<float>& stages);
        void triggerEnvelope(int voiceId);
        
    private:
        std::vector<std::vector<float>> envelopes_;
        int envelopeStages_ = 8;
        float sampleRate_ = 44100.0f;
    };
    
    //==============================================================================
    /** Synthesis engine instances */
    ModalSynthesizer modalSynthesizer_;
    PhysicalModelingSynthesizer physicalSynthesizer_;
    GranularSynthesizer granularSynthesizer_;
    HybridAnalogSynthesizer analogSynthesizer_;
    AdvancedEnvelopeSynthesizer envelopeSynthesizer_;
    
    //==============================================================================
    /** Internal processing methods */
    void updateVoices(int numSamples);
    void processModalSynthesis(int numSamples);
    void processPhysicalModeling(int numSamples);
    void processGranularSynthesis(int numSamples);
    void processHybridAnalog(int numSamples);
    void processAdvancedEnvelopes(int numSamples);
    
    /** Synthesis mixing */
    void mixSynthesisOutputs(int numSamples);
    void applySynthesisWeights(int numSamples);
    void applyCrossfading(int numSamples);
    
    /** Parameter processing */
    void updateGlobalParameters();
    void applyParameterAutomation(int numSamples);
    void applyParameterCurves(int numSamples);
    
    /** Performance monitoring */
    void updatePerformanceStats();
    void measureSynthesisCPUUsage();
    
    //==============================================================================
    /** Utility methods */
    void validateConfiguration();
    void allocateBuffers();
    void validateVoices();
    void optimizeProcessing();
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AdvancedSynthesisEngine)
};

} // namespace synthesis
} // namespace audio_engine
} // namespace vital
