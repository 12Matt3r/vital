#pragma once

#include <array>
#include <vector>
#include <functional>

namespace vital {

class Oscillator {
public:
    enum class WaveType {
        Sine,
        Triangle,
        Sawtooth,
        Square,
        Noise,
        Custom
    };

    Oscillator();
    ~Oscillator();

    void setWaveType(WaveType wave_type);
    void setFrequency(float frequency);
    void setPhase(float phase);
    void setDetune(float detune_semitones);
    void setPulseWidth(float width); // 0.0 to 1.0 for square wave
    
    // Custom waveform support
    void setCustomWaveform(const std::vector<float>& waveform);
    
    // Processing
    void process(float* output, int frames);
    void generate(WaveType wave_type, float* output, int frames);
    
    // Utility functions
    void syncPhase(); // Sync to current frequency
    float getFrequency() const { return frequency_; }
    float getPhase() const { return phase_; }
    WaveType getWaveType() const { return wave_type_; }

private:
    void generateSine(float* output, int frames);
    void generateTriangle(float* output, int frames);
    void generateSawtooth(float* output, int frames);
    void generateSquare(float* output, int frames);
    void generateNoise(float* output, int frames);
    void generateCustom(float* output, int frames);
    
    // Anti-aliasing
    void initializeBandLimitedTables();
    float bandLimitedWaveform(WaveType type, float phase, float frequency) const;
    
    float frequency_;
    float phase_;
    float detune_;
    float pulse_width_;
    WaveType wave_type_;
    
    // Band-limited waveform tables for anti-aliasing
    static constexpr int NUM_TABLES = 8;
    static constexpr int TABLE_SIZE = 1024;
    std::array<std::vector<float>, NUM_TABLES> band_limited_tables_;
    
    bool use_band_limiting_;
};

} // namespace vital