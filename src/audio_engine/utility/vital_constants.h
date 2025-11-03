/*
  ==============================================================================
    vital_constants.h
    Copyright (c) 2025 Vital Audio Engine Team
    
    Vital-specific constants and utilities for the audio engine
  ==============================================================================
*/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <cmath>

namespace vital {
namespace audio_engine {
namespace utility {

//==============================================================================
/**
 * @class VitalConstants
 * @brief Centralized constants and utility functions for Vital
 * 
 * Provides all essential constants used throughout the Vital audio engine
 * including mathematical constants, performance limits, and configuration defaults.
 */
class VitalConstants
{
public:
    //==============================================================================
    /** Mathematical constants */
    static constexpr float PI = juce::MathConstants<float>::pi;
    static constexpr float TWO_PI = juce::MathConstants<float>::twoPi;
    static constexpr float E = juce::MathConstants<float>::e;
    static constexpr float LN_2 = std::log(2.0f);
    static constexpr float LN_10 = std::log(10.0f);
    static constexpr float SQRT_2 = std::sqrt(2.0f);
    
    //==============================================================================
    /** Audio processing limits */
    static constexpr float MAX_FREQUENCY = 22050.0f;    // Nyquist at 44.1kHz
    static constexpr float MIN_FREQUENCY = 20.0f;       // Minimum audible frequency
    static constexpr float MAX_AMPLITUDE = 1.0f;        // Full scale
    static constexpr float MIN_AMPLITUDE = 0.0f;        // Silence
    
    //==============================================================================
    /** MIDI and tuning constants */
    static constexpr int MIDI_NOTES = 128;
    static constexpr int MIDI_CHANNELS = 16;
    static constexpr float MIDI_A4 = 440.0f;           // A4 frequency
    static constexpr int MIDI_A4_NOTE = 69;            // A4 note number
    static constexpr float CENTS_PER_OCTAVE = 1200.0f;
    
    //==============================================================================
    /** Performance limits */
    static constexpr int MAX_VOICES = 128;
    static constexpr int MAX_OSCILLATORS = 32;
    static constexpr int MAX_FILTERS = 16;
    static constexpr int MAX_LFOS = 8;
    static constexpr int MAX_ENVELOPES = 8;
    static constexpr int MAX_MACROS = 16;
    
    //==============================================================================
    /** Buffer and processing limits */
    static constexpr int MIN_BUFFER_SIZE = 64;
    static constexpr int MAX_BUFFER_SIZE = 8192;
    static constexpr int DEFAULT_BUFFER_SIZE = 512;
    static constexpr int MIN_SAMPLE_RATE = 44100;
    static constexpr int MAX_SAMPLE_RATE = 192000;
    static constexpr int DEFAULT_SAMPLE_RATE = 44100;
    
    //==============================================================================
    /** FFT and spectral processing */
    static constexpr int MIN_FFT_SIZE = 256;
    static constexpr int MAX_FFT_SIZE = 8192;
    static constexpr int DEFAULT_FFT_SIZE = 1024;
    static constexpr int MIN_OVERLAP = 2;
    static constexpr int MAX_OVERLAP = 16;
    static constexpr int DEFAULT_OVERLAP = 4;
    
    //==============================================================================
    /** Quality settings */
    static constexpr int MIN_OVERSAMPLING = 1;
    static constexpr int MAX_OVERSAMPLING = 8;
    static constexpr int DEFAULT_OVERSAMPLING = 2;
    
    static constexpr float MIN_FILTER_Q = 0.1f;
    static constexpr float MAX_FILTER_Q = 30.0f;
    static constexpr float DEFAULT_FILTER_Q = 1.0f;
    
    static constexpr float MIN_FILTER_FREQ = 20.0f;
    static constexpr float MAX_FILTER_FREQ = 22050.0f;
    
    //==============================================================================
    /** Parameter ranges */
    static constexpr float MIN_ATTACK_TIME = 0.001f;    // 1ms
    static constexpr float MAX_ATTACK_TIME = 10.0f;     // 10s
    static constexpr float MIN_DECAY_TIME = 0.001f;
    static constexpr float MAX_DECAY_TIME = 10.0f;
    static constexpr float MIN_RELEASE_TIME = 0.001f;
    static constexpr float MAX_RELEASE_TIME = 10.0f;
    
    static constexpr float MIN_SUSTAIN_LEVEL = 0.0f;
    static constexpr float MAX_SUSTAIN_LEVEL = 1.0f;
    
    //==============================================================================
    /** LFO constants */
    static constexpr float MIN_LFO_FREQ = 0.01f;        // 0.01 Hz (100 seconds per cycle)
    static constexpr float MAX_LFO_FREQ = 100.0f;       // 100 Hz
    static constexpr float DEFAULT_LFO_FREQ = 1.0f;     // 1 Hz
    
    //==============================================================================
    /** CPU and memory limits */
    static constexpr float MIN_CPU_LIMIT = 0.1f;        // 10% minimum
    static constexpr float MAX_CPU_LIMIT = 0.95f;       // 95% maximum
    static constexpr float DEFAULT_CPU_LIMIT = 0.8f;    // 80% default
    
    static constexpr size_t MIN_MEMORY_LIMIT = 1024 * 1024;      // 1MB minimum
    static constexpr size_t MAX_MEMORY_LIMIT = 1024 * 1024 * 1024; // 1GB maximum
    static constexpr size_t DEFAULT_MEMORY_LIMIT = 256 * 1024 * 1024; // 256MB default
    
    //==============================================================================
    /** Error and warning thresholds */
    static constexpr float CPU_WARNING_THRESHOLD = 0.85f;  // 85% CPU usage
    static constexpr float CPU_ERROR_THRESHOLD = 0.95f;    // 95% CPU usage
    
    static constexpr float LATENCY_WARNING_MS = 10.0f;      // 10ms latency warning
    static constexpr float LATENCY_ERROR_MS = 20.0f;        // 20ms latency error
    
    //==============================================================================
    /** Quality scores */
    static constexpr float PERFECT_QUALITY = 100.0f;
    static constexpr float GOOD_QUALITY = 80.0f;
    static constexpr float ACCEPTABLE_QUALITY = 60.0f;
    static constexpr float POOR_QUALITY = 40.0f;
    
    //==============================================================================
    /** Static utility functions */
    
    /**
     * Convert MIDI note number to frequency
     */
    [[nodiscard]] static float midiToFrequency(int noteNumber) {
        return MIDI_A4 * std::pow(2.0f, (noteNumber - MIDI_A4_NOTE) / 12.0f);
    }
    
    /**
     * Convert frequency to MIDI note number
     */
    [[nodiscard]] static int frequencyToMidi(float frequency) {
        return static_cast<int>(MIDI_A4_NOTE + 12 * std::log2(frequency / MIDI_A4));
    }
    
    /**
     * Convert cents to frequency ratio
     */
    [[nodiscard]] static float centsToRatio(float cents) {
        return std::pow(2.0f, cents / CENTS_PER_OCTAVE);
    }
    
    /**
     * Convert frequency ratio to cents
     */
    [[nodiscard]] static float ratioToCents(float ratio) {
        return CENTS_PER_OCTAVE * std::log2(ratio);
    }
    
    /**
     * Convert decibels to linear gain
     */
    [[nodiscard]] static float dbToLinear(float db) {
        return std::pow(10.0f, db / 20.0f);
    }
    
    /**
     * Convert linear gain to decibels
     */
    [[nodiscard]] static float linearToDb(float linear) {
        return 20.0f * std::log10(std::max(linear, 1e-10f));
    }
    
    /**
     * Clamp value to range
     */
    [[nodiscard]] static float clamp(float value, float min, float max) {
        return std::clamp(value, min, max);
    }
    
    /**
     * Clamp integer to range
     */
    [[nodiscard]] static int clamp(int value, int min, int max) {
        return std::clamp(value, min, max);
    }
    
    /**
     * Calculate phase increment from frequency
     */
    [[nodiscard]] static float phaseIncrement(float frequency, float sampleRate) {
        return frequency / sampleRate;
    }
    
    /**
     * Calculate normalized frequency (0.0 to 1.0)
     */
    [[nodiscard]] static float normalizedFrequency(float frequency, float sampleRate) {
        return frequency / (sampleRate * 0.5f);
    }
    
    /**
     * Apply exponential smoothing
     */
    [[nodiscard]] static float smooth(float current, float target, float alpha) {
        return current + alpha * (target - current);
    }
    
    /**
     * Calculate RMS (Root Mean Square)
     */
    [[nodiscard]] static float calculateRMS(const float* samples, int numSamples) {
        double sum = 0.0;
        for (int i = 0; i < numSamples; ++i) {
            sum += samples[i] * samples[i];
        }
        return static_cast<float>(std::sqrt(sum / numSamples));
    }
    
    /**
     * Calculate peak value
     */
    [[nodiscard]] static float calculatePeak(const float* samples, int numSamples) {
        float peak = 0.0f;
        for (int i = 0; i < numSamples; ++i) {
            peak = std::max(peak, std::abs(samples[i]));
        }
        return peak;
    }
    
    /**
     * Calculate dynamic range (difference between peak and RMS)
     */
    [[nodiscard]] static float calculateDynamicRange(const float* samples, int numSamples) {
        float peak = calculatePeak(samples, numSamples);
        float rms = calculateRMS(samples, numSamples);
        return linearToDb(peak / std::max(rms, 1e-10f));
    }
    
    /**
     * Generate Hann window
     */
    [[nodiscard]] static std::vector<float> generateHannWindow(int size) {
        std::vector<float> window(size);
        for (int i = 0; i < size; ++i) {
            window[i] = 0.5f * (1.0f - std::cos(2.0f * PI * i / (size - 1)));
        }
        return window;
    }
    
    /**
     * Generate Hamming window
     */
    [[nodiscard]] static std::vector<float> generateHammingWindow(int size) {
        std::vector<float> window(size);
        for (int i = 0; i < size; ++i) {
            window[i] = 0.54f - 0.46f * std::cos(2.0f * PI * i / (size - 1));
        }
        return window;
    }
    
    /**
     * Check if value is power of 2
     */
    [[nodiscard]] static bool isPowerOf2(int value) {
        return (value > 0) && ((value & (value - 1)) == 0);
    }
    
    /**
     * Round up to next power of 2
     */
    [[nodiscard]] static int nextPowerOf2(int value) {
        if (isPowerOf2(value)) return value;
        
        int power = 1;
        while (power < value) {
            power <<= 1;
        }
        return power;
    }
    
    /**
     * Calculate greatest common divisor
     */
    [[nodiscard]] static int gcd(int a, int b) {
        while (b != 0) {
            int temp = b;
            b = a % b;
            a = temp;
        }
        return a;
    }
    
    /**
     * Calculate least common multiple
     */
    [[nodiscard]] static int lcm(int a, int b) {
        return (a / gcd(a, b)) * b;
    }
    
    /**
     * Linear interpolation
     */
    [[nodiscard]] static float lerp(float a, float b, float t) {
        return a + t * (b - a);
    }
    
    /**
     * Cosine interpolation
     */
    [[nodiscard]] static float cosineInterp(float a, float b, float t) {
        float t2 = (1.0f - std::cos(t * PI)) * 0.5f;
        return a * (1.0f - t2) + b * t2;
    }
    
    /**
     * Cubic interpolation
     */
    [[nodiscard]] static float cubicInterp(float y0, float y1, float y2, float y3, float t) {
        float t2 = t * t;
        float a0 = -0.5f * y0 + 1.5f * y1 - 1.5f * y2 + 0.5f * y3;
        float a1 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
        float a2 = -0.5f * y0 + 0.5f * y2;
        float a3 = y1;
        
        return a0 * t * t2 + a1 * t2 + a2 * t + a3;
    }
    
private:
    VitalConstants() = default; // Static class - no instantiation
};

} // namespace utility
} // namespace audio_engine
} // namespace vital
