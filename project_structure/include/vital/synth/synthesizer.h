#pragma once

#include <array>
#include <memory>
#include "voice_manager.h"
#include "patch.h"
#include "audio_output.h"
#include "midi_input.h"

namespace vital {

class Synthesizer {
public:
    Synthesizer();
    ~Synthesizer();

    void initialize(double sample_rate);
    void shutdown();

    // Audio processing
    void process(float** inputs, float** outputs, int frames, int channels);
    void processBlock(float* output_left, float* output_right, int frames);

    // MIDI input
    void processMidi(int status, int data1, int data2);
    void processMidiNote(int channel, int note, int velocity);
    void processMidiControlChange(int channel, int controller, int value);
    void processMidiPitchBend(int channel, int value);
    void processMidiAfterTouch(int channel, int note, int pressure);

    // Voice management
    void setMaxVoices(int max_voices);
    int getMaxVoices() const { return voice_manager_.getMaxVoices(); }
    int getActiveVoices() const { return voice_manager_.getActiveVoices(); }
    int getPolyphony() const { return voice_manager_.getPolyphony(); }

    // Patch management
    void setPatch(const Patch& patch);
    const Patch& getPatch() const { return current_patch_; }
    void loadPatchFromFile(const std::string& filename);
    void savePatchToFile(const std::string& filename);

    // Parameter modulation
    void setParameterModulation(int param_index, float modulation_amount);
    void clearParameterModulations();

    // Effect processing
    void setGlobalEffectsEnabled(bool enabled);
    bool getGlobalEffectsEnabled() const { return global_effects_enabled_; }

    // Utility
    double getSampleRate() const { return sample_rate_; }
    void setSampleRate(double sample_rate);
    void setMasterVolume(float volume);
    float getMasterVolume() const { return master_volume_; }

private:
    void updateParameters();
    void processGlobalEffects(float* input, float* output, int frames);
    void voiceStealing();
    void initializeVoices();

    // Core components
    VoiceManager voice_manager_;
    Patch current_patch_;
    
    // Audio output
    AudioOutput audio_output_;
    
    // MIDI processing
    MidiInput midi_input_;
    
    // Global state
    double sample_rate_;
    int block_size_;
    int num_channels_;
    
    // Global effects chain
    struct GlobalEffects {
        // Reverb reverb_;
        // Chorus chorus_;
        // Compressor compressor_;
    } global_effects_;
    
    // Performance monitoring
    float cpu_usage_;
    float master_volume_;
    bool global_effects_enabled_;
    
    // Modulation buses
    std::vector<float> modulation_bus_;
    int modulation_buses_[64]; // Maximum modulation buses
    
    // Parameter smoothing
    std::vector<float> smoothed_parameters_;
    std::vector<float> parameter_smoothing_speeds_;
};

} // namespace vital