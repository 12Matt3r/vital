#pragma once

#include <vector>
#include <array>

namespace vital {

class Reverb {
public:
    enum class ReverbType {
        Hall,
        Room,
        Plate,
        Cathedral,
        Custom
    };

    struct Parameters {
        float room_size;      // 0.1 to 1.0
        float damping;        // 0.0 to 1.0
        float wet_level;      // 0.0 to 1.0
        float dry_level;      // 0.0 to 1.0
        float predelay;       // 0 to 100 ms
        float early_reflection_level; // 0.0 to 1.0
        float high_cut;       // 100 Hz to 20 kHz
        float low_cut;        // 20 Hz to 2 kHz
    };

    Reverb();
    ~Reverb();

    void initialize(double sample_rate);
    void setParameters(const Parameters& params);
    void setParameter(int parameter_index, float value);
    void setReverbType(ReverbType type);
    
    // Processing
    void process(float* input_left, float* input_right, 
                float* output_left, float* output_right, 
                int frames);
    
    void processMono(float* input, float* output, int frames);
    
    // Parameter access
    const Parameters& getParameters() const { return params_; }
    float getRoomSize() const { return params_.room_size; }
    float getDamping() const { return params_.damping; }
    float getWetLevel() const { return params_.wet_level; }
    float getDryLevel() const { return params_.dry_level; }
    float getPredelay() const { return params_.predelay; }
    ReverbType getType() const { return type_; }

private:
    void initializeDelays();
    void calculateCoefficients();
    void updateAllpassFilters();
    void processInputBuffer(float* input, int frames);
    void processCombFilters(float* input, float* comb_output);
    void processAllpassFilters(float* input, float* output);
    void applyMixing(float* wet, float* dry, float* output, int frames);
    
    // Schroeder reverb algorithm
    static constexpr int NUM_COMB_FILTERS = 4;
    static constexpr int NUM_ALLPASS_FILTERS = 2;
    static constexpr int MAX_DELAY_SAMPLES = 44100; // 1 second at 44.1kHz
    
    // Delay lines
    struct DelayLine {
        std::vector<float> buffer;
        int delay_samples;
        float feedback;
        float damping;
        int write_index;
        float current_sample;
        float previous_sample;
    };
    
    std::array<DelayLine, NUM_COMB_FILTERS> comb_filters_;
    std::array<DelayLine, NUM_ALLPASS_FILTERS> allpass_filters_;
    
    // Early reflections
    struct EarlyReflection {
        int delay_samples;
        float gain;
        float pan_position; // -1.0 to 1.0
    };
    
    std::vector<EarlyReflection> early_reflections_;
    
    // Filter coefficients
    struct FilterCoefficients {
        float a1, b1; // Low shelf
        float a2, b2; // High shelf
    };
    
    FilterCoefficients low_shelf_coeff_;
    FilterCoefficients high_shelf_coeff_;
    
    // Parameters
    Parameters params_;
    ReverbType type_;
    
    // State
    double sample_rate_;
    float predelay_buffer_[MAX_DELAY_SAMPLES];
    int predelay_write_index_;
    int predelay_samples_;
    
    // Pre-delay mixing
    float* predelay_output_;
    
    // Wet/dry mixing buffers
    std::vector<float> wet_buffer_;
    std::vector<float> temp_buffer_;
};

} // namespace vital