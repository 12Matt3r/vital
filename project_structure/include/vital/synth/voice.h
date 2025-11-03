#pragma once

#include <array>
#include "oscillator.h"
#include "filter.h"
#include "envelope.h"
#include "lfo.h"

namespace vital {

enum class VoiceStealingMode {
    Oldest,
    Quietest,
    First,
    Random
};

class Voice {
public:
    struct Parameters {
        float frequency;
        int note;
        int velocity;
        bool is_active;
        double age;
        float amplitude;
    };

    Voice();
    ~Voice();

    void initialize();
    void process(float* left_output, float* right_output, int frames);
    
    // Voice control
    void noteOn(int note, int velocity, double frequency);
    void noteOff();
    void setKeyPressure(int pressure);
    void setVoiceStealing(int note);
    
    // Parameter access
    Parameters getParameters() const { return params_; }
    int getNote() const { return params_.note; }
    bool isActive() const { return params_.is_active; }
    double getAge() const { return params_.age; }
    float getAmplitude() const { return params_.amplitude; }
    
    // Oscillators
    void setOscillatorParameters(int oscillator_index, const Oscillator::Parameters& params);
    void setOscillatorWaveform(int oscillator_index, Oscillator::WaveType wave_type);
    void setOscillatorMix(int oscillator_index, float mix);
    
    // Filter
    void setFilterParameters(const Filter::Parameters& params);
    void setFilterCutoff(float cutoff);
    void setFilterResonance(float resonance);
    
    // Envelope
    void setEnvelopeParameters(const Envelope::Parameters& params);
    void setEnvelopeStage(Envelope::Stage stage);
    
    // LFO
    void setLFOParameters(int lfo_index, const LFO::Parameters& params);
    void setLFODestination(int lfo_index, int destination, float amount);
    
    // Modulation
    void addModulation(int source, int destination, float amount);
    void clearModulations();
    
    // Utility
    void reset();
    void updateAge();
    double getSampleRate() const { return sample_rate_; }

private:
    void updateAudioParameters();
    void processOscillators(float* buffer, int frames);
    void processFilter(float* buffer, int frames);
    void processEnvelope(float* buffer, int frames);
    void processLFO(float* buffer, int frames);
    void applyModulation();
    
    // Voice components
    static constexpr int NUM_OSCILLATORS = 4;
    static constexpr int NUM_FILTERS = 2;
    static constexpr int NUM_ENVELOPES = 3; // Amp, Filter, Pitch
    static constexpr int NUM_LFOS = 4;
    
    std::array<std::unique_ptr<Oscillator>, NUM_OSCILLATORS> oscillators_;
    std::array<float, NUM_OSCILLATORS> oscillator_mix_;
    
    std::array<std::unique_ptr<Filter>, NUM_FILTERS> filters_;
    
    std::array<std::unique_ptr<Envelope>, NUM_ENVELOPES> envelopes_;
    Envelope* amp_envelope_;
    Envelope* filter_envelope_;
    Envelope* pitch_envelope_;
    
    std::array<std::unique_ptr<LFO>, NUM_LFOS> lfos_;
    
    // Processing buffers
    std::vector<float> temp_buffer_;
    std::vector<float> filter_buffer_;
    std::vector<float> envelope_buffer_;
    
    // Parameters
    Parameters params_;
    double sample_rate_;
    int block_size_;
    
    // Modulation system
    struct ModulationSource {
        int source_type; // 0=LFO, 1=Envelope, 2=Velocity, 3=Pressure, etc.
        int source_index;
        float amount;
    };
    
    std::vector<ModulationSource> modulations_;
    
    // Performance tracking
    double cpu_usage_;
    bool is_released_;
};

} // namespace vital