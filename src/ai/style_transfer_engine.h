#pragma once

#include "ai_manager.h"
#include <vector>
#include <array>
#include <complex>
#include <memory>
#include <mutex>
#include <queue>

namespace vital {

/**
 * @class StyleTransferEngine
 * @brief Real-time audio style transfer and timbre modification system
 * 
 * Provides neural style transfer for audio, allowing real-time transformation
 * of audio characteristics based on learned style models.
 */
class StyleTransferEngine {
public:
    // Style definition
    struct Style {
        std::string name;
        std::vector<float> spectral_profile;
        std::vector<float> harmonic_profile;
        std::vector<float> temporal_profile;
        std::vector<float> perceptual_features;
        float intensity;
        std::vector<std::string> tags;
        
        static constexpr size_t FEATURE_DIMENSION = 128;
    };
    
    // Transfer configuration
    struct TransferConfig {
        StyleTransferMode mode;
        float style_intensity;
        float blend_rate;
        bool preserve_structure;
        float quality_threshold;
        size_t frame_size;
        size_t hop_size;
        
        enum StyleTransferMode {
            Spectral,      // Frequency domain transfer
            Harmonic,      // Harmonic content transfer
            Temporal,      // Time domain transfer
            Perceptual,    // Perception-based transfer
            Hybrid         // Combination of all modes
        };
    };
    
    // Audio processing buffers
    struct AudioBuffer {
        std::vector<float> real;
        std::vector<std::complex<float>> spectral;
        std::vector<float> features;
        size_t size;
        
        void resize(size_t new_size) {
            real.resize(new_size);
            spectral.resize(new_size);
            features.resize(Style::FEATURE_DIMENSION);
            size = new_size;
        }
    };
    
    StyleTransferEngine(AIManager* ai_manager);
    ~StyleTransferEngine();
    
    // Configuration
    void setTransferConfig(const TransferConfig& config);
    void setProcessingParameters(size_t frame_size, size_t hop_size);
    
    // Style management
    bool loadStyle(const std::string& style_path);
    bool saveStyle(const std::string& style_path, const Style& style);
    bool createStyleFromAudio(const std::vector<float>& audio_data, const std::string& style_name);
    void addStyle(const Style& style);
    void removeStyle(const std::string& style_name);
    Style getStyle(const std::string& style_name) const;
    std::vector<std::string> getAvailableStyles() const;
    
    // Style transfer processing
    std::vector<float> transferStyle(const std::vector<float>& input_audio, const std::string& target_style);
    std::vector<float> transferStyle(const std::vector<float>& input_audio, 
                                   const std::vector<float>& style_features);
    
    // Multi-style blending
    std::vector<float> blendStyles(const std::vector<float>& input_audio,
                                 const std::vector<std::pair<std::string, float>>& style_weights);
    
    // Real-time processing
    bool processBlock(const std::vector<float>& input_block, std::vector<float>& output_block);
    bool initializeRealTime(size_t sample_rate);
    void updateStyleWeights(const std::vector<float>& new_weights);
    
    // Style interpolation
    std::vector<float> interpolateBetweenStyles(const std::string& style_a, const std::string& style_b, float t);
    void setInterpolationSpeed(float speed);
    
    // Quality assessment
    struct QualityMetrics {
        float similarity_score;
        float preservation_score;
        float artifact_level;
        float spectral_smoothness;
    };
    
    QualityMetrics assessTransferQuality(const std::vector<float>& original, 
                                       const std::vector<float>& processed);
    
    // Learning and adaptation
    void learnFromUserFeedback(const std::string& style_name, float user_rating);
    void adaptToAudioContent(const std::vector<float>& audio_features);
    void updateStyleProfile(const std::string& style_name, const std::vector<float>& new_features);
    
    // Statistics and monitoring
    struct ProcessingStats {
        uint64_t blocks_processed = 0;
        uint64_t successful_transfers = 0;
        float average_latency_ms = 0.0f;
        float cpu_usage_percent = 0.0f;
        std::vector<float> quality_scores;
        std::unordered_map<std::string, int> style_usage_count;
    };
    
    ProcessingStats getStats() const { return stats_; }
    void resetStats();
    
    // Performance optimization
    void enableGPUAcceleration(bool enable);
    void setCpuBudget(float percentage);
    void optimizeForLatency();
    void optimizeForQuality();
    
private:
    // FFT implementation (simplified)
    void performFFT(const std::vector<float>& input, std::vector<std::complex<float>>& output);
    void performIFFT(const std::vector<std::complex<float>>& input, std::vector<float>& output);
    
    // Feature extraction
    std::vector<float> extractSpectralFeatures(const std::vector<std::complex<float>>& spectral);
    std::vector<float> extractHarmonicFeatures(const std::vector<std::complex<float>>& spectral);
    std::vector<float> extractTemporalFeatures(const std::vector<float>& audio);
    std::vector<float> extractPerceptualFeatures(const std::vector<float>& spectral_features);
    
    // Style transfer algorithms
    void applySpectralTransfer(const std::vector<std::complex<float>>& input, 
                             const Style& target_style,
                             std::vector<std::complex<float>>& output);
    
    void applyHarmonicTransfer(const std::vector<std::complex<float>>& input,
                             const Style& target_style,
                             std::vector<std::complex<float>>& output);
    
    void applyTemporalTransfer(const std::vector<float>& input,
                             const Style& target_style,
                             std::vector<float>& output);
    
    void applyPerceptualTransfer(const std::vector<float>& features,
                               const Style& target_style,
                               std::vector<float>& output_features);
    
    // Windowing and buffering
    void applyWindow(std::vector<float>& audio, const std::string& window_type = "hann");
    void overlapAdd(const std::vector<float>& processed_block, 
                   std::vector<float>& output_buffer);
    
    // Quality control
    float calculateSimilarity(const std::vector<float>& a, const std::vector<float>& b);
    void applyAntiAliasing(std::vector<std::complex<float>>& spectral);
    void detectArtifacts(const std::vector<float>& audio);
    
    // Member variables
    AIManager* ai_manager_;
    TransferConfig config_;
    
    // Audio processing
    size_t sample_rate_;
    AudioBuffer input_buffer_;
    AudioBuffer output_buffer_;
    std::vector<float> window_;
    std::queue<std::vector<float>> processing_queue_;
    
    // Styles
    mutable std::mutex styles_mutex_;
    std::unordered_map<std::string, Style> styles_;
    std::vector<float> current_style_weights_;
    
    // Processing state
    bool is_initialized_;
    bool is_processing_;
    mutable std::mutex processing_mutex_;
    
    // Performance optimization
    bool use_gpu_;
    float cpu_budget_;
    std::vector<std::thread> processing_threads_;
    
    // Statistics
    mutable std::mutex stats_mutex_;
    ProcessingStats stats_;
    std::vector<float> latency_history_;
    
    // Window functions
    std::vector<float> generateWindow(size_t size, const std::string& type);
};

} // namespace vital