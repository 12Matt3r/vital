/*
  ==============================================================================
    modulation_engine.h
    Copyright (c) 2025 Vital Audio Engine Team
    
    Modulation engine for LFOs, envelopes, macros, and MIDI CC control
    integrating with the main Vital audio engine
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
namespace modulation {

//==============================================================================
/**
 * @class ModulationEngine
 * @brief Comprehensive modulation system for Vital
 * 
 * Provides:
 * - Multiple LFO types with phase and frequency control
 * - Advanced envelope generators (ADSR, multi-stage, random walk)
 * - Macro controls for complex parameter modulation
 * - MIDI CC mapping and learn functionality
 * - Parameter modulation routing and mixing
 */
class ModulationEngine
{
public:
    //==============================================================================
    /** Configuration */
    struct Config
    {
        int numLFOs = 4;
        int numEnvelopes = 4;
        int numMacros = 8;
        int maxMIDICC = 128;
        bool enableMacros = true;
        bool enableMidiCC = true;
        bool enableParameterAutomation = true;
        int modulationResolution = 1024; // Bits of modulation precision
        float sampleRate = 44100.0f;
    };
    
    //==============================================================================
    /** LFO types */
    enum class LFOType {
        Sine,
        Square,
        Saw,
        Triangle,
        Noise,
        Random,
        Envelope,
        Custom
    };
    
    /** Envelope types */
    enum class EnvelopeType {
        ADSR,
        MultiStage,
        OneShot,
        Loop,
        RandomWalk,
        Breakable
    };
    
    //==============================================================================
    /** Constructor */
    explicit ModulationEngine(const Config& config = Config());
    
    /** Destructor */
    ~ModulationEngine();
    
    //==============================================================================
    /** Initialize modulation engine */
    bool initialize(const Config& config);
    
    /** Shutdown */
    void shutdown();
    
    /** Reset */
    void reset();
    
    //==============================================================================
    /** Main processing */
    void process(int numSamples);
    
    //==============================================================================
    /** LFO control */
    void setLFOFrequency(int lfoId, float frequency);
    void setLFOGain(int lfoId, float gain);
    void setLFOPeak(int lfoId, float peak);
    void setLFOType(int lfoId, LFOType type);
    void setLFOPhase(int lfoId, float phase);
    void triggerLFO(int lfoId);
    
    //==============================================================================
    /** Envelope control */
    void triggerEnvelope(int envId);
    void releaseEnvelope(int envId);
    void setEnvelopeAttack(int envId, float attack);
    void setEnvelopeDecay(int envId, float decay);
    void setEnvelopeSustain(int envId, float sustain);
    void setEnvelopeRelease(int envId, float release);
    void setEnvelopeType(int envId, EnvelopeType type);
    
    //==============================================================================
    /** Macro control */
    void setMacroValue(int macroId, float value);
    void setMacroRange(int macroId, float min, float max);
    float getMacroValue(int macroId) const;
    
    //==============================================================================
    /** MIDI CC control */
    void setCCValue(int ccNumber, float value);
    float getCCValue(int ccNumber) const;
    void enableMidiLearn(int parameterId);
    void disableMidiLearn(int parameterId);
    void setMidiChannel(int channel);
    
    //==============================================================================
    /** Parameter modulation */
    void setModulationSource(int parameterId, int sourceId, float depth);
    float getModulatedValue(int parameterId, float baseValue);
    void clearModulations(int parameterId);
    
    //==============================================================================
    /** Access methods */
    float getLFOValue(int lfoId) const;
    float getEnvelopeValue(int envId) const;
    float getParameterModulation(int parameterId) const;
    
private:
    //==============================================================================
    /** Configuration */
    Config config_;
    
    /** State */
    std::atomic<bool> isInitialized_{false};
    
    //==============================================================================
    /** LFO implementation */
    class LFO {
    public:
        void initialize(float sampleRate);
        void setFrequency(float frequency);
        void setGain(float gain);
        void setPeak(float peak);
        void setType(LFOType type);
        void setPhase(float phase);
        void trigger();
        float process();
        
    private:
        float sampleRate_ = 44100.0f;
        float frequency_ = 1.0f;
        float gain_ = 1.0f;
        float peak_ = 1.0f;
        float phase_ = 0.0f;
        LFOType type_ = LFOType::Sine;
        float lastValue_ = 0.0f;
        
        float processSine();
        float processSquare();
        float processSaw();
        float processTriangle();
        float processNoise();
        float processRandom();
    };
    
    //==============================================================================
    /** Envelope implementation */
    class Envelope {
    public:
        void initialize(float sampleRate);
        void trigger();
        void release();
        float process();
        void setAttack(float attack);
        void setDecay(float decay);
        void setSustain(float sustain);
        void setRelease(float release);
        void setType(EnvelopeType type);
        
    private:
        float sampleRate_ = 44100.0f;
        float attack_ = 0.1f;
        float decay_ = 0.2f;
        float sustain_ = 0.5f;
        float release_ = 0.3f;
        EnvelopeType type_ = EnvelopeType::ADSR;
        float currentValue_ = 0.0f;
        float targetValue_ = 0.0f;
        bool isActive_ = false;
        bool isReleased_ = false;
        
        enum Stage { Attack, Decay, Sustain, Release };
        Stage currentStage_ = Attack;
    };
    
    //==============================================================================
    /** Modulation routing */
    struct ModulationSource {
        int sourceId = -1;
        float depth = 1.0f;
        bool enabled = true;
    };
    
    struct ModulationRoute {
        int parameterId = -1;
        std::vector<ModulationSource> sources;
        float currentModulation = 0.0f;
    };
    
    //==============================================================================
    /** Engine instances */
    std::vector<LFO> lfos_;
    std::vector<Envelope> envelopes_;
    std::vector<float> macroValues_;
    std::vector<float> midiCCValues_;
    std::vector<ModulationRoute> modulationRoutes_;
    
    //==============================================================================
    /** Current values */
    float lastProcessTime_ = 0.0f;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModulationEngine)
};

} // namespace modulation
} // namespace audio_engine
} // namespace vital
