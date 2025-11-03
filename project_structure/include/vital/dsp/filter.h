#pragma once

#include <array>
#include <vector>

namespace vital {

class Filter {
public:
    enum class FilterType {
        LowPass,
        HighPass,
        BandPass,
        Notch,
        AllPass,
        Peaking,
        LowShelf,
        HighShelf
    };

    Filter();
    ~Filter();

    void setType(FilterType type);
    void setCutoff(float cutoff_hz);
    void setResonance(float resonance);
    void setGain(float gain_db); // For peaking and shelving filters
    
    // Processing
    void process(float* input, float* output, int frames);
    void setCoefficients(float a0, float a1, float a2, float b0, float b1, float b2);
    
    // Coefficient calculation
    void calculateCoefficients(double sample_rate);
    void calculateLowPass(double sample_rate, double cutoff, double resonance);
    void calculateHighPass(double sample_rate, double cutoff, double resonance);
    void calculateBandPass(double sample_rate, double center_freq, double q);
    void calculateNotch(double sample_rate, double center_freq, double q);
    void calculatePeaking(double sample_rate, double center_freq, double q, double gain_db);
    
    // Parameters
    FilterType getType() const { return type_; }
    float getCutoff() const { return cutoff_; }
    float getResonance() const { return resonance_; }
    float getGain() const { return gain_; }

private:
    void initializeDelayLine();
    void updateCoefficients(double sample_rate);
    
    FilterType type_;
    float cutoff_;
    float resonance_;
    float gain_;
    
    // IIR coefficients
    double a0_, a1_, a2_;
    double b0_, b1_, b2_;
    
    // Delay line for filter processing
    std::array<float, 4> x_delay_;
    std::array<float, 4> y_delay_;
    int delay_index_;
    
    // Oversampling for better performance
    static constexpr int OVERSAMPLE_FACTOR = 2;
    std::vector<float> oversample_buffer_;
    
    float sample_rate_;
};

} // namespace vital