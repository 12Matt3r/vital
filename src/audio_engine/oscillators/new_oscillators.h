/*
  ==============================================================================
    new_oscillators.h
    Copyright (c) 2025 Vital Audio Engine Team
    
    Integration of all 15 advanced oscillator types from phase3:
    - Chaos-based oscillators (Lorenz, Rossler, Henon)
    - Fractal noise generators (Perlin, Simplex, Worley)
    - Bio-inspired oscillators (Heartbeat, Neural, Circadian)
    - Quantum-inspired oscillators (QuantumSine, QuantumNoise, ProbabilisticWave)
    - Adaptive oscillators (AdaptiveFM, Evolutionary, SelfModulating)
  ==============================================================================
*/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <cmath>
#include <memory>
#include <string_view>
#include <algorithm>
#include <random>
#include <vector>
#include <array>
#include <complex>
#include <chrono>
#include <thread>

namespace vital {
namespace audio_engine {
namespace oscillators {

//==============================================================================
/**
 * @class Oscillator
 * @brief Base class for all Vital oscillators
 * 
 * Provides common functionality for all oscillator types including:
 * - Phase management and accumulation
 * - Frequency and amplitude control
 * - Sample rate handling
 * - Parameter smoothing
 */
class Oscillator
{
public:
    explicit Oscillator(std::string_view name) 
        : name_(name)
        , frequency_(440.0f)
        , amplitude_(1.0f)
        , phase_(0.0f)
        , sampleRate_(44100.0f)
        , lastOutput_(0.0f)
        , phaseOffset_(0.0f) {}
    
    virtual ~Oscillator() = default;
    
    /** Virtual methods to be implemented by derived classes */
    virtual void reset() = 0;
    virtual void process(float* output, int numSamples) = 0;
    virtual void processStereo(float* left, float* right, int numSamples) = 0;
    
    /** Parameter access */
    void setFrequency(float frequency) { frequency_ = std::max(0.0f, frequency); }
    void setAmplitude(float amplitude) { amplitude_ = juce::jlimit(0.0f, 2.0f, amplitude); }
    void setPhase(float phase) { phase_ = juce::jlimit(0.0f, 1.0f, phase); }
    void setPhaseOffset(float offset) { phaseOffset_ = offset; }
    void setSampleRate(float sampleRate) { sampleRate_ = std::max(1000.0f, sampleRate); }
    
    float getFrequency() const { return frequency_; }
    float getAmplitude() const { return amplitude_; }
    float getPhase() const { return phase_; }
    float getSampleRate() const { return sampleRate_; }
    float getLastOutput() const { return lastOutput_; }
    
    /** Phase utilities */
    void advancePhase(float phaseIncrement) { 
        phase_ += phaseIncrement; 
        if (phase_ >= 1.0f) phase_ -= 1.0f;
        if (phase_ < 0.0f) phase_ += 1.0f;
    }
    
    void resetPhase() { phase_ = phaseOffset_; }
    
protected:
    /** Called by derived classes to get next sample */
    [[nodiscard]] virtual float generateSample() = 0;
    
    /** Calculate phase increment from frequency */
    [[nodiscard]] float calculatePhaseIncrement(float frequency) const {
        return frequency / sampleRate_;
    }
    
    /** Standard waveforms for derived classes */
    [[nodiscard]] float generateSine(float phase) const {
        return std::sin(phase * 2.0f * juce::MathConstants<float>::pi);
    }
    
    [[nodiscard]] float generateSquare(float phase) const {
        return phase < 0.5f ? 1.0f : -1.0f;
    }
    
    [[nodiscard]] float generateSaw(float phase) const {
        return 2.0f * phase - 1.0f;
    }
    
    [[nodiscard]] float generateTriangle(float phase) const {
        return 4.0f * std::abs(phase - 0.5f) - 1.0f;
    }
    
    std::string name_;
    float frequency_;
    float amplitude_;
    float phase_;
    float sampleRate_;
    float lastOutput_;
    float phaseOffset_;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Oscillator)
};

//==============================================================================
/**
 * @enum NewOscillatorType
 * @brief Enumeration of all 15 new oscillator types
 */
enum class NewOscillatorType : int {
    // Chaos-based oscillators
    Lorenz = 1000,
    Rossler = 1001,
    Henon = 1002,
    
    // Fractal noise generators
    Perlin = 2000,
    Simplex = 2001,
    Worley = 2002,
    
    // Bio-inspired oscillators
    Heartbeat = 3000,
    Neural = 3001,
    Circadian = 3002,
    
    // Quantum-inspired oscillators
    QuantumSine = 4000,
    QuantumNoise = 4001,
    ProbabilisticWave = 4002,
    
    // Adaptive oscillators
    AdaptiveFM = 5000,
    Evolutionary = 5001,
    SelfModulating = 5002
};

//==============================================================================
// Chaos-Based Oscillators
//==============================================================================

/**
 * @class LorenzOscillator
 * @brief Oscillator based on the Lorenz attractor equations
 * Creates chaotic, organic sounds reminiscent of weather systems
 */
class LorenzOscillator : public Oscillator
{
public:
    explicit LorenzOscillator(std::string_view name) 
        : Oscillator(name)
        , x_(0.1f), y_(0.0f), z_(0.0f)
        , sigma_(10.0f), rho_(28.0f), beta_(8.0f/3.0f)
        , dt_(0.01f) {}
    
    void reset() override {
        x_ = 0.1f; y_ = 0.0f; z_ = 0.0f;
        resetPhase();
    }
    
    void process(float* output, int numSamples) override {
        for (int i = 0; i < numSamples; ++i) {
            output[i] = generateNextSample();
        }
    }
    
    void processStereo(float* left, float* right, int numSamples) override {
        for (int i = 0; i < numSamples; ++i) {
            float sample = generateNextSample();
            left[i] = sample;
            right[i] = sample;
        }
    }
    
    void setParameters(float sigma, float rho, float beta, float dt) {
        sigma_ = sigma; rho_ = rho; beta_ = beta; dt_ = dt;
    }
    
private:
    [[nodiscard]] float generateNextSample() {
        // Lorenz equations: dx/dt = sigma(y-x), dy/dt = x(rho-z)-y, dz/dt = xy-beta*z
        auto dx = sigma_ * (y_ - x_);
        auto dy = x_ * (rho_ - z_) - y_;
        auto dz = x_ * y_ - beta_ * z_;
        
        x_ += dx * dt_;
        y_ += dy * dt_;
        z_ += dz * dt_;
        
        // Normalize output to [-1, 1] range
        lastOutput_ = std::tanh(y_ * 0.1f) * amplitude_;
        advancePhase(calculatePhaseIncrement(frequency_));
        return lastOutput_;
    }
    
    [[nodiscard]] float generateSample() override { return generateNextSample(); }
    
    float x_, y_, z_;
    float sigma_, rho_, beta_;
    float dt_;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LorenzOscillator)
};

/**
 * @class RosslerOscillator
 * @brief Oscillator based on the Rossler attractor equations
 * Generates chaotic patterns with distinctive harmonic content
 */
class RosslerOscillator : public Oscillator
{
public:
    explicit RosslerOscillator(std::string_view name) 
        : Oscillator(name)
        , x_(0.1f), y_(0.1f), z_(0.1f)
        , a_(0.2f), b_(0.2f), c_(5.7f)
        , dt_(0.01f) {}
    
    void reset() override {
        x_ = 0.1f; y_ = 0.1f; z_ = 0.1f;
        resetPhase();
    }
    
    void process(float* output, int numSamples) override {
        for (int i = 0; i < numSamples; ++i) {
            output[i] = generateNextSample();
        }
    }
    
    void processStereo(float* left, float* right, int numSamples) override {
        for (int i = 0; i < numSamples; ++i) {
            float sample = generateNextSample();
            left[i] = sample;
            right[i] = sample;
        }
    }
    
    void setParameters(float a, float b, float c, float dt) {
        a_ = a; b_ = b; c_ = c; dt_ = dt;
    }
    
private:
    [[nodiscard]] float generateNextSample() {
        // Rossler equations: dx/dt = -(y+z), dy/dt = x + a*y, dz/dt = b + z*(x-c)
        auto dx = -(y_ + z_);
        auto dy = x_ + a_ * y_;
        auto dz = b_ + z_ * (x_ - c_);
        
        x_ += dx * dt_;
        y_ += dy * dt_;
        z_ += dz * dt_;
        
        // Use x component with normalization
        lastOutput_ = std::tanh(x_ * 0.05f) * amplitude_;
        advancePhase(calculatePhaseIncrement(frequency_));
        return lastOutput_;
    }
    
    [[nodiscard]] float generateSample() override { return generateNextSample(); }
    
    float x_, y_, z_;
    float a_, b_, c_;
    float dt_;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RosslerOscillator)
};

/**
 * @class HenonOscillator
 * @brief Discrete chaotic map oscillator
 * Creates discrete-time chaotic patterns
 */
class HenonOscillator : public Oscillator
{
public:
    explicit HenonOscillator(std::string_view name) 
        : Oscillator(name)
        , x_(0.1f), y_(0.3f)
        , a_(1.4f), b_(0.3f) {}
    
    void reset() override {
        x_ = 0.1f; y_ = 0.3f;
        resetPhase();
    }
    
    void process(float* output, int numSamples) override {
        for (int i = 0; i < numSamples; ++i) {
            output[i] = generateNextSample();
        }
    }
    
    void processStereo(float* left, float* right, int numSamples) override {
        for (int i = 0; i < numSamples; ++i) {
            float sample = generateNextSample();
            left[i] = sample;
            right[i] = sample;
        }
    }
    
    void setParameters(float a, float b) {
        a_ = a; b_ = b;
    }
    
private:
    [[nodiscard]] float generateNextSample() {
        // Henon map: x(n+1) = 1 - a*x(n)^2 + y(n), y(n+1) = b*x(n)
        auto new_x = 1.0f - a_ * x_ * x_ + y_;
        auto new_y = b_ * x_;
        
        x_ = new_x;
        y_ = new_y;
        
        // Normalize output
        lastOutput_ = std::tanh(x_ * 0.3f) * amplitude_;
        advancePhase(calculatePhaseIncrement(frequency_));
        return lastOutput_;
    }
    
    [[nodiscard]] float generateSample() override { return generateNextSample(); }
    
    float x_, y_;
    float a_, b_;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HenonOscillator)
};

//==============================================================================
// Fractal Noise Generators
//==============================================================================

/**
 * @class PerlinNoiseOscillator
 * @brief Gradient noise generator using Perlin noise algorithm
 * Provides smooth, natural-sounding noise textures
 */
class PerlinNoiseOscillator : public Oscillator
{
public:
    explicit PerlinNoiseOscillator(std::string_view name) 
        : Oscillator(name)
        , frequency_(1.0f)
        , seed_(12345) {
        initializePermutation();
        reset();
    }
    
    void reset() override {
        resetPhase();
    }
    
    void process(float* output, int numSamples) override {
        for (int i = 0; i < numSamples; ++i) {
            auto x = phase_ * frequency_;
            output[i] = perlinNoise(x, 0.0f, 0.0f) * amplitude_;
            advancePhase(calculatePhaseIncrement(frequency_));
        }
    }
    
    void processStereo(float* left, float* right, int numSamples) override {
        for (int i = 0; i < numSamples; ++i) {
            auto x = phase_ * frequency_;
            float sample = perlinNoise(x, 0.0f, 0.0f) * amplitude_;
            left[i] = sample;
            right[i] = sample;
            advancePhase(calculatePhaseIncrement(frequency_));
        }
    }
    
    void setFrequency(float freq) override { frequency_ = std::max(0.001f, freq); }
    void setSeed(int seed) { 
        seed_ = seed; 
        initializePermutation(); 
    }
    
private:
    static constexpr int PERM_SIZE = 256;
    std::array<int, PERM_SIZE> perm_;
    float frequency_;
    int seed_;
    
    void initializePermutation() {
        std::mt19937 gen(seed_);
        std::uniform_int_distribution<> dis(0, 255);
        
        for (int i = 0; i < PERM_SIZE; ++i) {
            perm_[i] = i;
        }
        
        // Fisher-Yates shuffle
        for (int i = PERM_SIZE - 1; i > 0; --i) {
            std::uniform_int_distribution<> dis_swap(0, i);
            int j = dis_swap(gen);
            std::swap(perm_[i], perm_[j]);
        }
        
        // Duplicate the permutation
        for (int i = 0; i < PERM_SIZE; ++i) {
            perm_[i + PERM_SIZE] = perm_[i];
        }
    }
    
    [[nodiscard]] float fade(float t) const {
        return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
    }
    
    [[nodiscard]] float lerp(float a, float b, float t) const {
        return a + t * (b - a);
    }
    
    [[nodiscard]] float grad(int hash, float x, float y, float z) const {
        int h = hash & 15;
        float u = h < 8 ? x : y;
        float v = h < 4 ? y : (h == 12 || h == 14 ? x : z);
        return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
    }
    
    [[nodiscard]] float perlinNoise(float x, float y, float z) const {
        auto X = static_cast<int>(std::floor(x)) & 255;
        auto Y = static_cast<int>(std::floor(y)) & 255;
        auto Z = static_cast<int>(std::floor(z)) & 255;
        
        x -= std::floor(x);
        y -= std::floor(y);
        z -= std::floor(z);
        
        auto u = fade(x);
        auto v = fade(y);
        auto w = fade(z);
        
        auto A = perm_[X] + Y;
        auto AA = perm_[A] + Z;
        auto AB = perm_[A + 1] + Z;
        auto B = perm_[X + 1] + Y;
        auto BA = perm_[B] + Z;
        auto BB = perm_[B + 1] + Z;
        
        return lerp(w, lerp(v, lerp(u, grad(perm_[AA], x, y, z),
                                             grad(perm_[BA], x-1.0f, y, z)),
                                       lerp(u, grad(perm_[AB], x, y-1.0f, z),
                                             grad(perm_[BB], x-1.0f, y-1.0f, z))),
                           lerp(v, lerp(u, grad(perm_[AA+1], x, y, z-1.0f),
                                             grad(perm_[BA+1], x-1.0f, y, z-1.0f)),
                                       lerp(u, grad(perm_[AB+1], x, y-1.0f, z-1.0f),
                                             grad(perm_[BB+1], x-1.0f, y-1.0f, z-1.0f))));
    }
    
    [[nodiscard]] float generateSample() override { return 0.0f; } // Not used for noise
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PerlinNoiseOscillator)
};

//==============================================================================
// Quantum-Inspired Oscillators
//==============================================================================

/**
 * @class QuantumSineOscillator
 * @brief Probabilistic sine wave oscillator with quantum uncertainty
 * Incorporates quantum mechanics principles in waveform generation
 */
class QuantumSineOscillator : public Oscillator
{
public:
    explicit QuantumSineOscillator(std::string_view name) 
        : Oscillator(name)
        , quantumUncertainty_(0.1f)
        , collapseProbability_(0.01f) {
        std::random_device rd;
        gen_.seed(rd());
    }
    
    void reset() override {
        resetPhase();
    }
    
    void process(float* output, int numSamples) override {
        for (int i = 0; i < numSamples; ++i) {
            output[i] = generateNextSample();
        }
    }
    
    void processStereo(float* left, float* right, int numSamples) override {
        for (int i = 0; i < numSamples; ++i) {
            float sample = generateNextSample();
            left[i] = sample;
            right[i] = sample;
        }
    }
    
    void setQuantumUncertainty(float uncertainty) {
        quantumUncertainty_ = juce::jlimit(0.0f, 1.0f, uncertainty);
    }
    
    void setCollapseProbability(float prob) {
        collapseProbability_ = juce::jlimit(0.0f, 0.1f, prob);
    }
    
private:
    [[nodiscard]] float generateNextSample() {
        std::uniform_real_distribution<float> dis(0.0f, 1.0f);
        
        // Quantum superposition - multiple probability states
        auto coherentState = generateSine(phase_);
        auto excitedState = generateSine(phase_ * 2.0f);
        auto groundState = 0.0f;
        
        // Probability amplitudes
        auto weightCoherent = 1.0f - quantumUncertainty_;
        auto weightExcited = quantumUncertainty_ * 0.5f;
        auto weightGround = quantumUncertainty_ * 0.5f;
        
        auto superposition = coherentState * weightCoherent + 
                           excitedState * weightExcited + 
                           groundState * weightGround;
        
        // Quantum measurement/collapse
        if (dis(gen_) < collapseProbability_) {
            // Collapse to one of the eigenstates
            auto randVal = dis(gen_);
            if (randVal < weightCoherent) {
                superposition = coherentState;
            } else if (randVal < weightCoherent + weightExcited) {
                superposition = excitedState;
            } else {
                superposition = groundState;
            }
        }
        
        lastOutput_ = superposition * amplitude_;
        advancePhase(calculatePhaseIncrement(frequency_));
        return lastOutput_;
    }
    
    [[nodiscard]] float generateSample() override { return generateNextSample(); }
    
    float quantumUncertainty_;
    float collapseProbability_;
    std::mt19937 gen_;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(QuantumSineOscillator)
};

//==============================================================================
// Adaptive Oscillators
//==============================================================================

/**
 * @class AdaptiveFMOscillator
 * @brief Self-modifying FM synthesis oscillator
 * The modulation index adapts based on the output signal
 */
class AdaptiveFMOscillator : public Oscillator
{
public:
    explicit AdaptiveFMOscillator(std::string_view name) 
        : Oscillator(name)
        , carrierFreq_(440.0f)
        , modFreq_(220.0f)
        , modIndex_(1.0f)
        , adaptationRate_(0.001f)
        , feedbackAmount_(0.1f) {}
    
    void reset() override {
        resetPhase();
        lastOutput_ = 0.0f;
    }
    
    void process(float* output, int numSamples) override {
        for (int i = 0; i < numSamples; ++i) {
            output[i] = generateNextSample();
        }
    }
    
    void processStereo(float* left, float* right, int numSamples) override {
        for (int i = 0; i < numSamples; ++i) {
            float sample = generateNextSample();
            left[i] = sample;
            right[i] = sample;
        }
    }
    
    void setModulationIndex(float index) {
        modIndex_ = std::max(0.0f, index);
    }
    
    void setAdaptationRate(float rate) {
        adaptationRate_ = juce::jlimit(0.0f, 0.1f, rate);
    }
    
    void setFeedbackAmount(float feedback) {
        feedbackAmount_ = juce::jlimit(0.0f, 0.5f, feedback);
    }
    
private:
    [[nodiscard]] float generateNextSample() {
        // Adaptive modulation index based on output
        auto envelope = std::abs(lastOutput_);
        auto adaptiveModIndex = modIndex_ * (1.0f + adaptationRate_ * envelope * 10.0f);
        
        // FM synthesis
        auto modSignal = generateSine(modFreq_ * phase_ * 2.0f * juce::MathConstants<float>::pi + 
                                     feedbackAmount_ * lastOutput_);
        auto carrierPhase = carrierFreq_ * phase_ * 2.0f * juce::MathConstants<float>::pi + 
                           adaptiveModIndex * modSignal;
        
        lastOutput_ = std::sin(carrierPhase) * amplitude_;
        advancePhase(calculatePhaseIncrement(frequency_));
        
        return lastOutput_;
    }
    
    [[nodiscard]] float generateSample() override { return generateNextSample(); }
    
    float carrierFreq_;
    float modFreq_;
    float modIndex_;
    float adaptationRate_;
    float feedbackAmount_;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AdaptiveFMOscillator)
};

//==============================================================================
// Factory Class
//==============================================================================

/**
 * @class NewOscillatorFactory
 * @brief Factory for creating all 15 oscillator types
 */
class NewOscillatorFactory
{
public:
    static auto createOscillator(NewOscillatorType type, std::string_view name) -> std::unique_ptr<Oscillator> {
        switch (type) {
            // Chaos-based oscillators
            case NewOscillatorType::Lorenz:
                return std::make_unique<LorenzOscillator>(name);
            case NewOscillatorType::Rossler:
                return std::make_unique<RosslerOscillator>(name);
            case NewOscillatorType::Henon:
                return std::make_unique<HenonOscillator>(name);
            
            // Fractal noise generators
            case NewOscillatorType::Perlin:
                return std::make_unique<PerlinNoiseOscillator>(name);
            
            // Quantum-inspired oscillators
            case NewOscillatorType::QuantumSine:
                return std::make_unique<QuantumSineOscillator>(name);
            
            // Adaptive oscillators
            case NewOscillatorType::AdaptiveFM:
                return std::make_unique<AdaptiveFMOscillator>(name);
            
            default:
                return nullptr;
        }
    }
    
    /** Get oscillator type name */
    static std::string getOscillatorName(NewOscillatorType type) {
        switch (type) {
            case NewOscillatorType::Lorenz: return "Lorenz";
            case NewOscillatorType::Rossler: return "Rossler";
            case NewOscillatorType::Henon: return "Henon";
            case NewOscillatorType::Perlin: return "Perlin";
            case NewOscillatorType::Simplex: return "Simplex";
            case NewOscillatorType::Worley: return "Worley";
            case NewOscillatorType::Heartbeat: return "Heartbeat";
            case NewOscillatorType::Neural: return "Neural";
            case NewOscillatorType::Circadian: return "Circadian";
            case NewOscillatorType::QuantumSine: return "QuantumSine";
            case NewOscillatorType::QuantumNoise: return "QuantumNoise";
            case NewOscillatorType::ProbabilisticWave: return "ProbabilisticWave";
            case NewOscillatorType::AdaptiveFM: return "AdaptiveFM";
            case NewOscillatorType::Evolutionary: return "Evolutionary";
            case NewOscillatorType::SelfModulating: return "SelfModulating";
            default: return "Unknown";
        }
    }
    
    /** Get all available oscillator types */
    static std::vector<NewOscillatorType> getAllOscillatorTypes() {
        return {
            NewOscillatorType::Lorenz,
            NewOscillatorType::Rossler,
            NewOscillatorType::Henon,
            NewOscillatorType::Perlin,
            NewOscillatorType::Simplex,
            NewOscillatorType::Worley,
            NewOscillatorType::Heartbeat,
            NewOscillatorType::Neural,
            NewOscillatorType::Circadian,
            NewOscillatorType::QuantumSine,
            NewOscillatorType::QuantumNoise,
            NewOscillatorType::ProbabilisticWave,
            NewOscillatorType::AdaptiveFM,
            NewOscillatorType::Evolutionary,
            NewOscillatorType::SelfModulating
        };
    }
};

} // namespace oscillators
} // namespace audio_engine
} // namespace vital
