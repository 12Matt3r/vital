/*
  ==============================================================================
    filter_engine.h
    Copyright (c) 2025 Vital Audio Engine Team
    
    Professional filter engine with analog modeling, digital filters,
    and oversampling for the Vital audio engine
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
namespace filtering {

//==============================================================================
/**
 * @class FilterEngine
 * @brief Professional filter processing engine
 * 
 * Provides:
 * - Digital filters (lowpass, highpass, bandpass, notch, peaking, etc.)
 * - Analog modeling filters with virtual components
 * - Oversampling for alias-free filtering
 * - Multi-stage filter chains
 * - Real-time parameter modulation
 */
class FilterEngine
{
public:
    //==============================================================================
    /** Configuration */
    struct Config
    {
        float sampleRate = 44100.0f;
        int numFilters = 8;
        bool enableOversampling = true;
        int oversamplingFactor = 2;
        bool enableAnalogModeling = true;
        int filterStages = 4;
        bool enableNonLinearities = true;
        bool enableDistortion = false;
    };
    
    //==============================================================================
    /** Filter types */
    enum class FilterType {
        LowPass,
        HighPass,
        BandPass,
        Notch,
        Peaking,
        LowShelf,
        HighShelf,
        AllPass,
        AnalogLowPass,
        AnalogHighPass,
        AnalogBandPass,
        Custom
    };
    
    //==============================================================================
    /** Constructor */
    explicit FilterEngine(const Config& config = Config());
    
    /** Destructor */
    ~FilterEngine();
    
    //==============================================================================
    /** Initialize filter engine */
    bool initialize(const Config& config);
    
    /** Shutdown */
    void shutdown();
    
    /** Reset */
    void reset();
    
    //==============================================================================
    /** Main processing */
    void process(int numSamples);
    
    //==============================================================================
    /** Filter control */
    void setFilterType(int filterId, FilterType type);
    void setFilterFrequency(int filterId, float frequency);
    void setFilterResonance(int filterId, float resonance);
    void setFilterGain(int filterId, float gain);
    void setFilterStages(int filterId, int stages);
    void enableFilter(int filterId, bool enabled);
    
    //==============================================================================
    /** Analog modeling */
    void setAnalogModelingEnabled(bool enabled);
    void setAnalogComponentValues(int filterId, float component1, float component2);
    void setAnalogDrift(float driftAmount);
    
    //==============================================================================
    /** Oversampling */
    void enableOversampling(bool enabled);
    void setOversamplingFactor(int factor);
    int getOversamplingFactor() const;
    
    //==============================================================================
    /** Access methods */
    float getFilterFrequency(int filterId) const;
    float getFilterResonance(int filterId) const;
    bool isFilterEnabled(int filterId) const;
    
private:
    //==============================================================================
    /** Configuration */
    Config config_;
    
    /** State */
    std::atomic<bool> isInitialized_{false};
    
    //==============================================================================
    /** Filter implementation */
    class Filter {
    public:
        void initialize(float sampleRate);
        void process(float* samples, int numSamples);
        void setType(FilterType type);
        void setFrequency(float frequency);
        void setResonance(float resonance);
        void setGain(float gain);
        void setStages(int stages);
        void enable(bool enabled);
        
    private:
        float sampleRate_ = 44100.0f;
        FilterType type_ = FilterType::LowPass;
        float frequency_ = 1000.0f;
        float resonance_ = 0.7f;
        float gain_ = 0.0f;
        int stages_ = 1;
        bool enabled_ = true;
        
        // JUCE filter instances for different types
        juce::LinkwitzRileyFilter<float> linkwitzRileyFilter_;
        juce::StateVariableFilter<float> stateVariableFilter_;
        juce::BiquadFilter<float> biquadFilter_;
        
        void updateFilterCoefficients();
    };
    
    //==============================================================================
    /** Filter instances */
    std::vector<Filter> filters_;
    
    //==============================================================================
    /** Processing buffers */
    std::vector<std::vector<float>> inputBuffers_;
    std::vector<std::vector<float>> outputBuffers_;
    std::vector<std::vector<float>> oversampleBuffers_;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FilterEngine)
};

} // namespace filtering
} // namespace audio_engine
} // namespace vital
