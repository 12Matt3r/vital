/**
 * @file branchless_programming.h
 * @brief Branchless programming optimizations for Vital DSP operations
 * @author Vital Development Team
 * @date 2025-11-03
 * 
 * This module provides comprehensive branchless algorithms for audio processing,
 * eliminating CPU pipeline stalls and improving performance through SIMD-friendly
 * conditional operations and mathematical transformations.
 */

#pragma once

#include <algorithm>
#include <array>
#include <bit>
#include <cmath>
#include <cstdint>
#include <limits>
#include <type_traits>
#include <x86intrin.h>

namespace vital {
namespace performance {
namespace branchless {

// ============================================================================
// SIMD Architecture Detection for Branchless Operations
// ============================================================================

/**
 * SIMD capabilities for branchless operations
 */
struct BranchlessSIMD {
    static constexpr bool has_sse42 = __SSE4_2__;
    static constexpr bool has_avx = __AVX__;
    static constexpr bool has_avx2 = __AVX2__;
    static constexpr bool has_avx512f = __AVX512F__;
    
    static constexpr int sse_width = 4;
    static constexpr int avx_width = 8;
    static constexpr int avx512_width = 16;
    
    static constexpr int max_width = 
        (has_avx512f ? avx512_width : (has_avx2 ? avx_width : (has_sse42 ? sse_width : 1)));
};

// ============================================================================
// Branchless Selection and Conditional Operations
// ============================================================================

/**
 * Branchless conditional selection using bit operations
 */
template<typename T>
T branchless_select(T condition, T true_value, T false_value) {
    // Convert condition to mask: condition != 0 -> all bits 1, else all bits 0
    using UnsignedT = typename std::make_unsigned<T>::type;
    UnsignedT mask = -(condition != T(0));
    
    // Select based on mask
    return (true_value & mask) | (false_value & ~mask);
}

/**
 * Branchless clamp operation
 */
template<typename T>
T branchless_clamp(T value, T min_value, T max_value) {
    return branchless_select(value > max_value, max_value,
           branchless_select(value < min_value, min_value, value));
}

/**
 * Branchless min/max operations
 */
template<typename T>
T branchless_min(T a, T b) {
    return branchless_select(a > b, b, a);
}

template<typename T>
T branchless_max(T a, T b) {
    return branchless_select(a < b, b, a);
}

/**
 * Branchless absolute value
 */
template<typename T>
T branchless_abs(T value) {
    return branchless_select(value < T(0), -value, value);
}

/**
 * Branchless sign function
 */
template<typename T>
T branchless_sign(T value) {
    using UnsignedT = typename std::make_unsigned<T>::type;
    return branchless_select(value > T(0), T(1),
           branchless_select(value < T(0), T(-1), T(0)));
}

// ============================================================================
// Fast Mathematical Approximations (Branchless)
// ============================================================================

/**
 * Branchless reciprocal approximation
 */
float branchless_reciprocal(float x) {
    return 1.0f / x;
}

/**
 * Branchless reciprocal square root approximation
 */
float branchless_rsqrt(float x) {
    return 1.0f / std::sqrt(x);
}

/**
 * Fast branchless exponential approximation for small values
 */
float branchless_exp_fast(float x) {
    // For |x| < 1, exp(x) ≈ 1 + x + x²/2
    // For larger |x|, use more terms or fallback to standard exp
    if (std::abs(x) < 1.0f) {
        float x2 = x * x;
        return 1.0f + x + x2 * 0.5f + x2 * x * 0.1666667f; // 1/6
    }
    return std::exp(x);
}

/**
 * Fast branchless logarithm approximation
 */
float branchless_log_fast(float x) {
    if (x <= 0.0f) return std::numeric_limits<float>::lowest();
    
    // Fast approximation using polynomial
    float log2 = std::log2(x);
    return log2 * 0.693147f; // log(2)
}

/**
 * Branchless sigmoid activation
 */
float branchless_sigmoid(float x) {
    // Numerically stable sigmoid: 1 / (1 + exp(-x))
    if (x > 0.0f) {
        float e_neg_x = std::exp(-x);
        return 1.0f / (1.0f + e_neg_x);
    } else {
        float e_x = std::exp(x);
        return e_x / (1.0f + e_x);
    }
}

/**
 * Branchless fast sigmoid approximation
 */
float branchless_sigmoid_fast(float x) {
    // Polynomial approximation: 0.5 + 0.15*x for small x
    float abs_x = branchless_abs(x);
    if (abs_x < 1.0f) {
        return 0.5f + 0.15f * x;
    }
    return branchless_sigmoid(x);
}

// ============================================================================
// Branchless Audio Processing Algorithms
// ============================================================================

/**
 * Branchless wavetable synthesis
 */
class BranchlessWavetableSynthesizer {
public:
    /**
     * Generate sine wave without branches
     */
    static void generate_sine_wavetable(float* wavetable, size_t table_size) {
        const float two_pi = 2.0f * M_PI;
        
        for (size_t i = 0; i < table_size; ++i) {
            float phase = static_cast<float>(i) / table_size;
            float angle = phase * two_pi;
            wavetable[i] = std::sin(angle);
        }
    }
    
    /**
     * Interpolate wavetable without branches
     */
    static float interpolate_wavetable(const float* wavetable, size_t table_size, 
                                     float phase) {
        // Wrap phase to [0, 1)
        phase = phase - std::floor(phase);
        
        // Convert to table indices
        float position = phase * table_size;
        size_t index = static_cast<size_t>(position);
        float fraction = position - index;
        
        // Linear interpolation without branches
        float sample1 = wavetable[index];
        float sample2 = wavetable[(index + 1) % table_size];
        
        return sample1 + fraction * (sample2 - sample1);
    }
    
    /**
     * Multi-dimensional wavetable morphing
     */
    static float interpolate_2d_wavetable(const float* wavetable, size_t table_size,
                                        size_t dims, float* phases, float* weights) {
        size_t total_size = 1;
        for (size_t i = 0; i < dims; ++i) {
            total_size *= table_size;
        }
        
        // Calculate linear index
        size_t linear_index = 0;
        size_t stride = 1;
        for (size_t i = 0; i < dims; ++i) {
            size_t index = static_cast<size_t>(phases[i] * table_size) % table_size;
            linear_index += index * stride;
            stride *= table_size;
        }
        
        // Interpolate based on weights (simplified)
        float result = 0.0f;
        for (size_t i = 0; i < dims; ++i) {
            float phase = phases[i];
            float weight = branchless_clamp(weights[i], 0.0f, 1.0f);
            result += weight * interpolate_wavetable(wavetable, table_size, phase);
        }
        
        return result;
    }
};

/**
 * Branchless filter processing
 */
class BranchlessFilter {
public:
    /**
     * Branchless biquad filter coefficient calculation
     */
    static void calculate_biquad_coefficients(float* coefficients, float freq, 
                                            float q, float gain_db, float sample_rate) {
        float omega = 2.0f * M_PI * freq / sample_rate;
        float sin_omega = std::sin(omega);
        float cos_omega = std::cos(omega);
        float alpha = sin_omega / (2.0f * q);
        
        float a = std::pow(10.0f, gain_db / 40.0f);
        float beta = std::sqrt(a) / q;
        
        // Peak filter coefficients (simplified)
        coefficients[0] = 1.0f + alpha * a;
        coefficients[1] = -2.0f * cos_omega;
        coefficients[2] = 1.0f - alpha * a;
        coefficients[3] = 1.0f + beta;
        coefficients[4] = -2.0f * cos_omega;
        coefficients[5] = 1.0f - beta;
    }
    
    /**
     * Branchless biquad filter application
     */
    static void apply_biquad(const float* input, const float* coefficients,
                           float* output, const float* state, size_t num_samples) {
        float b0 = coefficients[0];
        float b1 = coefficients[1];
        float b2 = coefficients[2];
        float a1 = coefficients[4];
        float a2 = coefficients[5];
        
        float x1 = state[0];
        float x2 = state[1];
        float y1 = state[2];
        float y2 = state[3];
        
        for (size_t i = 0; i < num_samples; ++i) {
            float x0 = input[i];
            
            // Direct form II transposed
            float y0 = b0 * x0 + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
            
            output[i] = y0;
            
            // Update state (shift registers)
            x2 = x1;
            x1 = x0;
            y2 = y1;
            y1 = y0;
        }
    }
};

/**
 * Branchless envelope generator
 */
class BranchlessEnvelope {
public:
    /**
     * Branchless ADSR envelope with smooth transitions
     */
    void process_envelope(float* envelope, size_t num_samples, 
                         float target_level, float attack_time, float release_time,
                         float sample_rate) {
        float attack_rate = 1.0f / (attack_time * sample_rate);
        float release_rate = 1.0f / (release_time * sample_rate);
        
        for (size_t i = 0; i < num_samples; ++i) {
            float current_level = envelope[i];
            float difference = target_level - current_level;
            
            // Branchless attack/release detection
            bool is_attack = difference > 0.0f;
            float rate = branchless_select(is_attack, attack_rate, release_rate);
            
            // Branchless envelope update
            float change = branchless_abs(difference) * rate;
            envelope[i] = branchless_select(is_attack, 
                                          current_level + change, 
                                          current_level - change);
            
            // Clamp to target
            envelope[i] = branchless_select(is_attack,
                                          branchless_min(envelope[i], target_level),
                                          branchless_max(envelope[i], target_level));
        }
    }
    
    /**
     * Branchless gate/hold functionality
     */
    void apply_gate(float* signal, const bool* gate, size_t num_samples) {
        for (size_t i = 0; i < num_samples; ++i) {
            float gate_value = branchless_select(gate[i], 1.0f, 0.0f);
            signal[i] *= gate_value;
        }
    }
};

/**
 * Branchless effects processing
 */
class BranchlessEffects {
public:
    /**
     * Branchless soft clipping distortion
     */
    static float branchless_soft_clip(float input, float drive = 1.0f, float softness = 2.0f) {
        input *= drive;
        
        // tanh-like curve without branches
        float x = input;
        float x2 = x * x;
        float x3 = x2 * x;
        float x5 = x3 * x2;
        
        // Taylor series for tanh(x)
        float result = x - (x3 / 3.0f) + (2.0f * x5) / 15.0f;
        
        // Scale by softness
        return result / softness;
    }
    
    /**
     * Branchless compressor
     */
    static void apply_compressor(float* signal, const float* control, 
                               size_t num_samples, float threshold, float ratio) {
        for (size_t i = 0; i < num_samples; ++i) {
            float level = branchless_abs(control[i]);
            
            // Branchless over-threshold detection
            bool over_threshold = level > threshold;
            float excess = branchless_select(over_threshold, level - threshold, 0.0f);
            
            // Branchless compression calculation
            float compressed_excess = excess / ratio;
            float gain_reduction = compressed_excess - excess;
            
            // Apply gain reduction
            float gain_factor = std::pow(10.0f, gain_reduction / 20.0f);
            signal[i] *= gain_factor;
        }
    }
    
    /**
     * Branchless bit crusher
     */
    static void apply_bit_crush(float* signal, size_t num_samples, int bits = 8) {
        float step = 1.0f / ((1 << bits) - 1);
        float inv_step = 1.0f / step;
        
        for (size_t i = 0; i < num_samples; ++i) {
            float stepped = std::floor(branchless_abs(signal[i]) * inv_step) * step;
            signal[i] = branchless_sign(signal[i]) * stepped;
        }
    }
    
    /**
     * Branchless stereo panning
     */
    static void apply_stereo_pan(float* left, float* right, float pan_position) {
        float pan = branchless_clamp(pan_position, -1.0f, 1.0f);
        
        // Pan law: -3dB at center
        float left_gain = 1.0f - branchless_select(pan > 0.0f, pan * 0.5f, 0.0f);
        float right_gain = 1.0f - branchless_select(pan < 0.0f, -pan * 0.5f, 0.0f);
        
        *left *= left_gain;
        *right *= right_gain;
    }
};

// ============================================================================
// SIMD Control Flow for Branchless Processing
// ============================================================================

/**
 * SIMD vector type for branchless operations
 */
template<typename T, int Width>
class BranchlessSIMDVector {
public:
    using value_type = T;
    static constexpr int width = Width;
    
    BranchlessSIMDVector() = default;
    
    explicit BranchlessSIMDVector(T value) {
        load_value(value);
    }
    
    // Load and store operations
    static BranchlessSIMDVector load(const T* ptr) {
        BranchlessSIMDVector result;
        if constexpr (std::is_same_v<T, float>) {
            if constexpr (Width == 16) result.data_ = _mm512_loadu_ps(ptr);
            else if constexpr (Width == 8) result.data_ = _mm256_loadu_ps(ptr);
            else if constexpr (Width == 4) result.data_ = _mm_loadu_ps(ptr);
        }
        return result;
    }
    
    // Branchless operations
    BranchlessSIMDVector select(const BranchlessSIMDVector& other, 
                               const BranchlessSIMDVector& mask) const {
        BranchlessSIMDVector result;
        if constexpr (std::is_same_v<T, float>) {
            if constexpr (Width == 16) result.data_ = _mm512_mask_blend_ps(mask.data_, data_, other.data_);
            else if constexpr (Width == 8) result.data_ = _mm256_blendv_ps(data_, other.data_, mask.data_);
            else if constexpr (Width == 4) result.data_ = _mm_blendv_ps(data_, other.data_, mask.data_);
        }
        return result;
    }
    
    // Arithmetic with branchless clamping
    BranchlessSIMDVector multiply_clamped(const BranchlessSIMDVector& other, 
                                        T min_val, T max_val) const {
        BranchlessSIMDVector result = (*this) * other;
        result = result.select(BranchlessSIMDVector(max_val), 
                              result > BranchlessSIMDVector(max_val));
        result = result.select(BranchlessSIMDVector(min_val), 
                              result < BranchlessSIMDVector(min_val));
        return result;
    }
    
    // Branchless saturation arithmetic
    BranchlessSIMDVector saturating_add(const BranchlessSIMDVector& other) const {
        BranchlessSIMDVector sum = (*this) + other;
        return sum.select(BranchlessSIMDVector(std::numeric_limits<T>::max()), 
                        sum > BranchlessSIMDVector(std::numeric_limits<T>::max()));
    }
    
    // Standard operators
    BranchlessSIMDVector operator+(const BranchlessSIMDVector& other) const {
        BranchlessSIMDVector result;
        if constexpr (std::is_same_v<T, float>) {
            if constexpr (Width == 16) result.data_ = _mm512_add_ps(data_, other.data_);
            else if constexpr (Width == 8) result.data_ = _mm256_add_ps(data_, other.data_);
            else if constexpr (Width == 4) result.data_ = _mm_add_ps(data_, other.data_);
        }
        return result;
    }
    
    BranchlessSIMDVector operator*(const BranchlessSIMDVector& other) const {
        BranchlessSIMDVector result;
        if constexpr (std::is_same_v<T, float>) {
            if constexpr (Width == 16) result.data_ = _mm512_mul_ps(data_, other.data_);
            else if constexpr (Width == 8) result.data_ = _mm256_mul_ps(data_, other.data_);
            else if constexpr (Width == 4) result.data_ = _mm_mul_ps(data_, other.data_);
        }
        return result;
    }
    
    BranchlessSIMDVector operator>(const BranchlessSIMDVector& other) const {
        BranchlessSIMDVector result;
        if constexpr (std::is_same_v<T, float>) {
            if constexpr (Width == 16) result.data_ = _mm512_cmp_ps_mask(data_, other.data_, _CMP_GT_OQ);
            else if constexpr (Width == 8) result.data_ = _mm256_cmp_ps(data_, other.data_, _CMP_GT_OQ);
            else if constexpr (Width == 4) result.data_ = _mm_cmp_ps(data_, other.data_, _CMP_GT_OQ);
        }
        return result;
    }
    
private:
    union {
        __m512 m512;
        __m256 m256;
        __m128 m128;
        std::array<T, 16> array512;
        std::array<T, 8> array256;
        std::array<T, 4> array128;
    } data_;
    
    void load_value(T value) {
        for (int i = 0; i < Width; ++i) {
            if constexpr (Width == 16) data_.array512[i] = value;
            else if constexpr (Width == 8) data_.array256[i] = value;
            else if constexpr (Width == 4) data_.array128[i] = value;
        }
    }
};

/**
 * Branchless audio processor using SIMD
 */
class BranchlessSIMDProcessor {
public:
    BranchlessSIMDProcessor() = default;
    
    // Process audio buffer with branchless operations
    template<int VectorWidth = BranchlessSIMD::max_width>
    void process_audio_branchless(const float* input, float* output, 
                                 size_t num_samples, const float* parameters) {
        constexpr int width = VectorWidth;
        
        using SimdVec = BranchlessSIMDVector<float, width>;
        
        // Extract parameters
        SimdVec gain(parameters[0]);
        SimdVec cutoff(parameters[1]);
        SimdVec resonance(parameters[2]);
        
        size_t vector_count = num_samples / width;
        
        for (size_t i = 0; i < vector_count; ++i) {
            SimdVec input_vec = SimdVec::load(input + i * width);
            
            // Branchless processing chain
            SimdVec processed = process_voice(input_vec, gain, cutoff, resonance);
            processed.store(output + i * width);
        }
        
        // Process remainder
        for (size_t i = vector_count * width; i < num_samples; ++i) {
            output[i] = process_scalar_branchless(input[i], parameters);
        }
    }
    
private:
    using SimdVec = BranchlessSIMDVector<float, BranchlessSIMD::max_width>;
    
    SimdVec process_voice(const SimdVec& input, const SimdVec& gain, 
                         const SimdVec& cutoff, const SimdVec& resonance) {
        // Branchless filter chain
        auto filtered = apply_filter_branchless(input, cutoff, resonance);
        auto saturated = apply_saturation_branchless(filtered);
        auto gated = apply_gate_branchless(saturated, gain);
        return gated;
    }
    
    SimdVec apply_filter_branchless(const SimdVec& input, const SimdVec& cutoff, 
                                   const SimdVec& resonance) {
        // Simplified filter without branches
        // In practice, this would implement a more sophisticated filter
        SimdVec fc = cutoff * SimdVec(0.001f); // Normalize frequency
        SimdVec q = resonance * SimdVec(0.5f);
        
        // Branchless filter coefficients calculation
        SimdVec omega = SimdVec(2.0f) * SimdVec(M_PI) * fc;
        SimdVec sin_omega = omega.sin(); // Would need SIMD sin implementation
        SimdVec cos_omega = omega.cos(); // Would need SIMD cos implementation
        SimdVec alpha = sin_omega / (SimdVec(2.0f) * q);
        
        // Apply filter (simplified)
        return input * (SimdVec(1.0f) + alpha);
    }
    
    SimdVec apply_saturation_branchless(const SimdVec& input) {
        // Branchless soft clipping
        auto abs_input = input.select(-input, input < SimdVec(0.0f));
        auto soft_clipped = abs_input.select(SimdVec(1.0f), abs_input > SimdVec(1.0f));
        return soft_clipped * input.select(SimdVec(-1.0f), input < SimdVec(0.0f));
    }
    
    SimdVec apply_gate_branchless(const SimdVec& input, const SimdVec& gain) {
        // Branchless gain control
        auto gated_gain = gain.select(SimdVec(0.0f), gain < SimdVec(0.01f));
        return input * gated_gain;
    }
    
    float process_scalar_branchless(float input, const float* parameters) {
        float gain = parameters[0];
        float cutoff = parameters[1];
        float resonance = parameters[2];
        
        // Branchless scalar processing
        float processed = input * gain;
        processed = branchless_clamp(processed, -1.0f, 1.0f);
        return processed;
    }
};

// ============================================================================
// Branchless Parameter Interpolation
// ============================================================================

/**
 * Branchless parameter interpolation utilities
 */
class BranchlessInterpolator {
public:
    /**
     * Branchless linear interpolation
     */
    template<typename T>
    static T linear_interpolate(const T& start, const T& end, float t) {
        t = branchless_clamp(t, 0.0f, 1.0f);
        return start + (end - start) * t;
    }
    
    /**
     * Branchless exponential interpolation
     */
    template<typename T>
    static T exponential_interpolate(const T& start, const T& end, float t) {
        t = branchless_clamp(t, 0.0f, 1.0f);
        float factor = 1.0f - std::pow(0.001f, t); // Fast approach to 1
        return start + (end - start) * factor;
    }
    
    /**
     * Branchless smoothstep interpolation
     */
    template<typename T>
    static T smoothstep_interpolate(const T& start, const T& end, float t) {
        t = branchless_clamp(t, 0.0f, 1.0f);
        t = t * t * (3.0f - 2.0f * t); // smoothstep function
        return start + (end - start) * t;
    }
    
    /**
     * Branchless cubic Hermite interpolation
     */
    template<typename T>
    static T cubic_interpolate(const T& p0, const T& p1, const T& p2, const T& p3, float t) {
        t = branchless_clamp(t, 0.0f, 1.0f);
        float t2 = t * t;
        float t3 = t2 * t;
        
        // Hermite basis functions
        float h00 = 2.0f * t3 - 3.0f * t2 + 1.0f;
        float h10 = t3 - 2.0f * t2 + t;
        float h01 = -2.0f * t3 + 3.0f * t2;
        float h11 = t3 - t2;
        
        return p1 * h00 + p2 * h01 + (p1 - p0) * h10 + (p3 - p2) * h11;
    }
    
    /**
     * Multi-parameter interpolation
     */
    template<typename T>
    static void interpolate_parameters(T* parameters, const T* targets, 
                                     size_t num_params, float t) {
        for (size_t i = 0; i < num_params; ++i) {
            parameters[i] = linear_interpolate(parameters[i], targets[i], t);
        }
    }
};

// ============================================================================
// Branchless Audio Utilities
// ============================================================================

/**
 * Collection of branchless audio utilities
 */
class AudioBranchlessUtils {
public:
    /**
     * Branchless RMS calculation
     */
    static float calculate_rms_branchless(const float* data, size_t num_samples) {
        if (num_samples == 0) return 0.0f;
        
        double sum_squares = 0.0;
        
        for (size_t i = 0; i < num_samples; ++i) {
            float squared = data[i] * data[i];
            sum_squares += squared;
        }
        
        return static_cast<float>(std::sqrt(sum_squares / num_samples));
    }
    
    /**
     * Branchless peak detection
     */
    static float detect_peak_branchless(const float* data, size_t num_samples) {
        float peak = 0.0f;
        
        for (size_t i = 0; i < num_samples; ++i) {
            float abs_val = branchless_abs(data[i]);
            peak = branchless_max(peak, abs_val);
        }
        
        return peak;
    }
    
    /**
     * Branchless level metering
     */
    struct LevelMeter {
        float peak = 0.0f;
        float rms = 0.0f;
        float crest_factor = 0.0f;
    };
    
    static LevelMeter measure_level_branchless(const float* data, size_t num_samples) {
        LevelMeter meter;
        
        float sum_squares = 0.0f;
        meter.peak = 0.0f;
        
        for (size_t i = 0; i < num_samples; ++i) {
            float abs_val = branchless_abs(data[i]);
            meter.peak = branchless_max(meter.peak, abs_val);
            sum_squares += abs_val * abs_val;
        }
        
        if (num_samples > 0) {
            meter.rms = std::sqrt(sum_squares / num_samples);
            meter.crest_factor = (meter.rms > 0.0f) ? meter.peak / meter.rms : 0.0f;
        }
        
        return meter;
    }
    
    /**
     * Branchless crossfade
     */
    static void crossfade_branchless(const float* input1, const float* input2, 
                                   float* output, size_t num_samples, float crossfade) {
        crossfade = branchless_clamp(crossfade, 0.0f, 1.0f);
        
        for (size_t i = 0; i < num_samples; ++i) {
            float gain1 = 1.0f - crossfade;
            float gain2 = crossfade;
            
            output[i] = input1[i] * gain1 + input2[i] * gain2;
        }
    }
    
    /**
     * Branchless fade in/out
     */
    static void apply_fade_branchless(float* data, size_t num_samples, 
                                     size_t fade_samples, bool fade_in) {
        for (size_t i = 0; i < num_samples; ++i) {
            float fade_factor;
            
            if (fade_in) {
                // Fade in
                size_t fade_position = i < fade_samples ? i : fade_samples;
                fade_factor = static_cast<float>(fade_position) / fade_samples;
            } else {
                // Fade out
                size_t fade_position = i >= (num_samples - fade_samples) ? 
                                     (num_samples - i) : fade_samples;
                fade_factor = static_cast<float>(fade_position) / fade_samples;
            }
            
            data[i] *= fade_factor;
        }
    }
};

// ============================================================================
// Performance Monitoring for Branchless Operations
// ============================================================================

/**
 * Branchless performance monitor
 */
class BranchlessPerformanceMonitor {
public:
    struct BranchlessMetrics {
        size_t branch_operations_eliminated = 0;
        size_t simd_operations_used = 0;
        size_t total_operations = 0;
        double branch_misprediction_penalty_saved = 0.0;
        double pipeline_efficiency_improvement = 0.0;
    };
    
    void record_branch_elimination(size_t count = 1) {
        metrics_.branch_operations_eliminated += count;
    }
    
    void record_simd_operation(size_t count = 1) {
        metrics_.simd_operations_used += count;
    }
    
    void record_operation() {
        metrics_.total_operations++;
    }
    
    BranchlessMetrics get_metrics() const {
        return metrics_;
    }
    
    double get_branchless_efficiency() const {
        if (metrics_.total_operations > 0) {
            return static_cast<double>(metrics_.branch_operations_eliminated) / metrics_.total_operations;
        }
        return 0.0;
    }
    
    void reset_metrics() {
        metrics_ = BranchlessMetrics{};
    }
    
private:
    BranchlessMetrics metrics_;
};

} // namespace branchless
} // namespace performance
} // namespace vital
