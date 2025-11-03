#include "oscillator.h"
#include "math_utils.h"

namespace vital {

Oscillator::Oscillator() 
    : frequency_(440.0f)
    , phase_(0.0f)
    , detune_(0.0f)
    , pulse_width_(0.5f)
    , wave_type_(WaveType::Sine)
    , use_band_limiting_(true) {
    
    initializeBandLimitedTables();
}

Oscillator::~Oscillator() = default;

void Oscillator::setWaveType(WaveType wave_type) {
    wave_type_ = wave_type;
    if (use_band_limiting_) {
        initializeBandLimitedTables();
    }
}

void Oscillator::setFrequency(float frequency) {
    frequency_ = std::max(20.0f, std::min(20000.0f, frequency));
}

void Oscillator::setPhase(float phase) {
    phase_ = std::fmod(phase, 1.0f);
    if (phase_ < 0) phase_ += 1.0f;
}

void Oscillator::setDetune(float detune_semitones) {
    detune_ = MathUtils::clamp(detune_semitones, -12.0f, 12.0f);
    // Apply detune via frequency adjustment
    float detune_factor = std::pow(2.0f, detune_ / 12.0f);
    frequency_ *= detune_factor;
}

void Oscillator::setPulseWidth(float width) {
    pulse_width_ = MathUtils::clamp(width, 0.01f, 0.99f);
}

void Oscillator::setCustomWaveform(const std::vector<float>& waveform) {
    if (waveform.size() != TABLE_SIZE) {
        // Resize if necessary
        // This would involve interpolation to fit the waveform to TABLE_SIZE
    }
    wave_type_ = WaveType::Custom;
}

void Oscillator::process(float* output, int frames) {
    if (!output || frames <= 0) return;
    
    generate(wave_type_, output, frames);
    syncPhase();
}

void Oscillator::generate(WaveType wave_type, float* output, int frames) {
    if (!output || frames <= 0) return;
    
    switch (wave_type) {
        case WaveType::Sine:
            generateSine(output, frames);
            break;
        case WaveType::Triangle:
            generateTriangle(output, frames);
            break;
        case WaveType::Sawtooth:
            generateSawtooth(output, frames);
            break;
        case WaveType::Square:
            generateSquare(output, frames);
            break;
        case WaveType::Noise:
            generateNoise(output, frames);
            break;
        case WaveType::Custom:
            generateCustom(output, frames);
            break;
        default:
            generateSine(output, frames);
            break;
    }
}

void Oscillator::generateSine(float* output, int frames) {
    for (int i = 0; i < frames; ++i) {
        output[i] = MathUtils::fastSin(phase_ * TWO_PI);
        phase_ += frequency_ / 44100.0f; // Assuming 44.1kHz sample rate
        if (phase_ >= 1.0f) phase_ -= 1.0f;
    }
}

void Oscillator::generateTriangle(float* output, int frames) {
    for (int i = 0; i < frames; ++i) {
        float phase_angle = phase_ * TWO_PI;
        float normalized_phase = fmod(phase_angle, TWO_PI) / TWO_PI;
        
        if (normalized_phase < 0.25f) {
            output[i] = 4.0f * normalized_phase;
        } else if (normalized_phase < 0.75f) {
            output[i] = 2.0f - 4.0f * normalized_phase;
        } else {
            output[i] = 4.0f * normalized_phase - 4.0f;
        }
        
        phase_ += frequency_ / 44100.0f;
        if (phase_ >= 1.0f) phase_ -= 1.0f;
    }
}

void Oscillator::generateSawtooth(float* output, int frames) {
    for (int i = 0; i < frames; ++i) {
        float phase_angle = phase_ * TWO_PI;
        float normalized_phase = fmod(phase_angle, TWO_PI) / TWO_PI;
        
        output[i] = 2.0f * normalized_phase - 1.0f;
        
        phase_ += frequency_ / 44100.0f;
        if (phase_ >= 1.0f) phase_ -= 1.0f;
    }
}

void Oscillator::generateSquare(float* output, int frames) {
    for (int i = 0; i < frames; ++i) {
        float phase_angle = phase_ * TWO_PI;
        float normalized_phase = fmod(phase_angle, TWO_PI) / TWO_PI;
        
        // Basic square wave with pulse width modulation
        if (normalized_phase < pulse_width_) {
            output[i] = 1.0f;
        } else {
            output[i] = -1.0f;
        }
        
        phase_ += frequency_ / 44100.0f;
        if (phase_ >= 1.0f) phase_ -= 1.0f;
    }
}

void Oscillator::generateNoise(float* output, int frames) {
    for (int i = 0; i < frames; ++i) {
        output[i] = MathUtils::randomFloat(-1.0f, 1.0f);
        // Don't advance phase for noise - it's random
    }
}

void Oscillator::generateCustom(float* output, int frames) {
    // This would interpolate from the custom waveform table
    // For now, fall back to sine
    generateSine(output, frames);
}

void Oscillator::syncPhase() {
    // Keep phase within valid range
    while (phase_ >= 1.0f) phase_ -= 1.0f;
    while (phase_ < 0.0f) phase_ += 1.0f;
}

void Oscillator::initializeBandLimitedTables() {
    if (!use_band_limiting_) return;
    
    // Initialize anti-aliasing tables for different frequencies
    for (int table = 0; table < NUM_TABLES; ++table) {
        band_limited_tables_[table].resize(TABLE_SIZE);
        
        float frequency_ratio = static_cast<float>(table) / NUM_TABLES;
        float fundamental_frequency = 20.0f + frequency_ratio * 1980.0f; // 20Hz to 2kHz
        
        for (int i = 0; i < TABLE_SIZE; ++i) {
            float phase = static_cast<float>(i) / TABLE_SIZE;
            band_limited_tables_[table][i] = MathUtils::fastSin(phase * TWO_PI);
        }
    }
}

float Oscillator::bandLimitedWaveform(WaveType type, float phase, float frequency) const {
    // This would implement band-limited synthesis
    // For now, return regular waveform
    return MathUtils::fastSin(phase * TWO_PI);
}

} // namespace vital