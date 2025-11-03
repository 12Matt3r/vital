#pragma once

#include "ai_manager.h"
#include <vector>
#include <array>
#include <complex>
#include <mutex>
#include <queue>
#include <atomic>
#include <functional>

namespace vital {

/**
 * @class IntelligentAudioAnalyzer
 * @brief AI-powered real-time audio analysis and learning system
 * 
 * Provides comprehensive audio analysis including spectral features, pitch tracking,
 * timbre analysis, and intelligent classification with machine learning capabilities.
 */
class IntelligentAudioAnalyzer {
public:
    // Audio features
    struct AudioFeatures {
        // Spectral features
        std::vector<float> mfcc;
        std::vector<float> chroma;
        float spectral_centroid;
        float spectral_rolloff;
        float spectral_flatness;
        float spectral_bandwidth;
        std::vector<float> spectral_slope;
        
        // Temporal features
        float zero_crossing_rate;
        float energy;
        float rms;
        float temporal_centroid;
        std::vector<float> temporal_slope;
        
        // Harmonic features
        float fundamental_frequency;
        float harmonic_ratio;
        std::vector<float> harmonic_strength;
        float inharmonicity;
        
        // Perceptual features
        float brightness;
        float warmth;
        float roughness;
        float clarity;
        float richness;
        
        // Musical features
        std::vector<float> key_profile;
        float tempo_estimate;
        float time_signature_estimate;
        float dynamic_range;
        
        // Statistical features
        float variance;
        float skewness;
        float kurtosis;
        std::vector<float> autocorrelation;
        
        // Custom features
        std::vector<float> custom_features;
        
        static constexpr size_t MFCC_BINS = 13;
        static constexpr size_t CHROMA_BINS = 12;
        static constexpr size_t NUM_FEATURES = 50;
        
        void clear() {
            mfcc.clear();
            chroma.clear();
            spectral_slope.clear();
            harmonic_strength.clear();
            key_profile.clear();
            temporal_slope.clear();
            autocorrelation.clear();
            custom_features.clear();
            
            spectral_centroid = 0.0f;
            spectral_rolloff = 0.0f;
            spectral_flatness = 0.0f;
            spectral_bandwidth = 0.0f;
            zero_crossing_rate = 0.0f;
            energy = 0.0f;
            rms = 0.0f;
            temporal_centroid = 0.0f;
            fundamental_frequency = 0.0f;
            harmonic_ratio = 0.0f;
            inharmonicity = 0.0f;
            brightness = 0.0f;
            warmth = 0.0f;
            roughness = 0.0f;
            clarity = 0.0f;
            richness = 0.0f;
            tempo_estimate = 0.0f;
            time_signature_estimate = 0.0f;
            dynamic_range = 0.0f;
            variance = 0.0f;
            skewness = 0.0f;
            kurtosis = 0.0f;
        }
    };
    
    // Audio classification
    enum class AudioClass {
        Unknown,
        Speech,
        Music,
        Drum,
        Bass,
        Lead,
        Pad,
        Percussion,
        Noise,
        Silence
    };
    
    struct ClassificationResult {
        AudioClass predicted_class;
        float confidence;
        std::vector<std::pair<AudioClass, float>> class_probabilities;
        std::string description;
        TimePoint timestamp;
    };
    
    // Analysis configuration
    struct AnalysisConfig {
        size_t sample_rate = 44100;
        size_t frame_size = 1024;
        size_t hop_size = 512;
        size_t window_type = 1; // 0: rectangular, 1: hann, 2: hamming, 3: blackman
        bool enable_mfcc = true;
        bool enable_chroma = true;
        bool enable_pitch_tracking = true;
        bool enable_classification = true;
        bool enable_learning = true;
        float feature_smoothing = 0.1f;
        size_t analysis_history_size = 100;
    };
    
    IntelligentAudioAnalyzer(AIManager* ai_manager);
    ~IntelligentAudioAnalyzer();
    
    // Configuration
    void setAnalysisConfig(const AnalysisConfig& config);
    AnalysisConfig getAnalysisConfig() const { return config_; }
    void updateSampleRate(size_t sample_rate);
    
    // Real-time processing
    AudioFeatures analyzeAudio(const std::vector<float>& audio_block);
    AudioFeatures analyzeAudioInRealTime(const std::vector<float>& audio_block);
    
    // Batch processing
    std::vector<AudioFeatures> analyzeBatch(const std::vector<std::vector<float>>& audio_blocks);
    AudioFeatures analyzeAudioFile(const std::string& file_path);
    
    // Classification
    ClassificationResult classifyAudio(const AudioFeatures& features);
    ClassificationResult classifyAudioRealTime(const std::vector<float>& audio_block);
    
    // Pitch tracking
    float detectPitch(const std::vector<float>& audio_block);
    float detectPitchAutocorrelation(const std::vector<float>& audio_block);
    float detectPitchYIN(const std::vector<float>& audio_block);
    
    // Spectral analysis
    void computeSpectrum(const std::vector<float>& audio_block, std::vector<std::complex<float>>& spectrum);
    void computeMelSpectrum(const std::vector<float>& audio_block, std::vector<float>& mel_spectrum);
    void computeChromaFeatures(const std::vector<std::complex<float>>& spectrum, std::vector<float>& chroma);
    
    // Feature extraction
    void extractMFCC(const std::vector<std::complex<float>>& spectrum, std::vector<float>& mfcc);
    void extractSpectralFeatures(const std::vector<std::complex<float>>& spectrum, AudioFeatures& features);
    void extractTemporalFeatures(const std::vector<float>& audio_block, AudioFeatures& features);
    void extractHarmonicFeatures(const std::vector<float>& audio_block, AudioFeatures& features);
    void extractPerceptualFeatures(const AudioFeatures& features);
    
    // Learning and adaptation
    void learnFromUserFeedback(const AudioFeatures& features, AudioClass correct_class);
    void adaptToMusicalContext(const std::string& genre, const std::string& style);
    void updateClassificationModel(const std::vector<AudioFeatures>& features, 
                                  const std::vector<AudioClass>& labels);
    
    // Feature analysis and comparison
    float compareFeatures(const AudioFeatures& a, const AudioFeatures& b);
    std::vector<float> calculateFeatureDistance(const AudioFeatures& a, const AudioFeatures& b);
    std::vector<size_t> findSimilarAudio(const AudioFeatures& query, size_t num_results);
    
    // Musical intelligence
    float estimateTempo(const std::vector<float>& audio_block);
    std::vector<float> estimateKey(const AudioFeatures& features);
    float estimateKeyStrength(const std::vector<float>& key_profile);
    
    // Real-time monitoring
    struct RealTimeMetrics {
        float current_spectral_centroid = 0.0f;
        float current_rms = 0.0f;
        float current_pitch = 0.0f;
        float current_tempo = 0.0f;
        AudioClass current_class = AudioClass::Unknown;
        float current_confidence = 0.0f;
        size_t buffer_size = 0;
        float cpu_usage = 0.0f;
        float processing_latency_ms = 0.0f;
    };
    
    RealTimeMetrics getRealTimeMetrics() const;
    void startRealTimeMonitoring();
    void stopRealTimeMonitoring();
    
    // Quality and reliability
    struct QualityMetrics {
        float feature_stability = 0.0f;
        float classification_reliability = 0.0f;
        float pitch_tracking_accuracy = 0.0f;
        float spectral_consistency = 0.0f;
        float temporal_consistency = 0.0f;
        size_t total_analyzed_frames = 0;
        size_t failed_classifications = 0;
        float average_confidence = 0.0f;
    };
    
    QualityMetrics getQualityMetrics() const;
    
    // Statistics
    struct AnalysisStats {
        uint64_t total_audio_blocks_processed = 0;
        uint64_t total_features_extracted = 0;
        uint64_t total_classifications = 0;
        size_t active_features = 0;
        float average_processing_time_ms = 0.0f;
        std::map<AudioClass, uint64_t> classification_counts;
        std::vector<float> processing_times;
        std::map<std::string, float> genre_confidence;
    };
    
    AnalysisStats getStats() const { return stats_; }
    void resetStats();
    
    // Model management
    bool saveAnalysisModel(const std::string& file_path);
    bool loadAnalysisModel(const std::string& file_path);
    void resetClassificationModel();
    void exportFeatures(const std::string& file_path, const std::vector<AudioFeatures>& features);
    
    // Utility functions
    std::vector<float> applyWindow(const std::vector<float>& audio_block, size_t window_type);
    std::vector<float> generateMelFilterBank(size_t num_filters, size_t fft_size, float sample_rate);
    
private:
    // FFT implementation (simplified Cooley-Tukey)
    void performFFT(const std::vector<float>& input, std::vector<std::complex<float>>& output);
    void performIFFT(const std::vector<std::complex<float>>& input, std::vector<float>& output);
    
    // Window functions
    std::vector<float> generateWindow(size_t size, size_t window_type);
    float calculateWindowCorrection(size_t size, size_t window_type);
    
    // Feature smoothing
    void smoothFeatures(AudioFeatures& current_features, const AudioFeatures& previous_features);
    
    // Pitch detection algorithms
    float autocorrelatePitch(const std::vector<float>& audio_block);
    float yinPitch(const std::vector<float>& audio_block);
    
    // Classification algorithms
    AudioClass classifyKNN(const AudioFeatures& features, size_t k = 5);
    AudioClass classifyBayes(const AudioFeatures& features);
    AudioClass classifyNeural(const AudioFeatures& features);
    
    // Feature normalization
    void normalizeFeatures(AudioFeatures& features);
    void updateFeatureStatistics(const AudioFeatures& features);
    
    // Quality assessment
    float assessFeatureQuality(const AudioFeatures& features);
    void detectAnomalies(const AudioFeatures& features);
    
    // Member variables
    AIManager* ai_manager_;
    AnalysisConfig config_;
    
    // Audio processing buffers
    std::vector<float> window_;
    std::vector<float> mel_filter_bank_;
    std::vector<float> dct_matrix_;
    
    // Analysis state
    std::vector<AudioFeatures> feature_history_;
    std::vector<std::pair<AudioFeatures, AudioClass>> training_data_;
    std::mutex history_mutex_;
    mutable std::mutex processing_mutex_;
    
    // Real-time monitoring
    mutable std::mutex metrics_mutex_;
    RealTimeMetrics real_time_metrics_;
    std::atomic<bool> real_time_monitoring_active_;
    
    // Statistics and quality
    mutable std::mutex stats_mutex_;
    AnalysisStats stats_;
    QualityMetrics quality_metrics_;
    
    // Feature statistics
    struct FeatureStatistics {
        std::vector<float> means;
        std::vector<float> standard_deviations;
        std::vector<float> minima;
        std::vector<float> maxima;
        size_t update_count = 0;
    } feature_stats_;
    
    // Window function cache
    std::vector<std::vector<float>> window_cache_;
    
    // Helper methods
    size_t getNextPowerOfTwo(size_t n);
    float interpolateValue(const std::vector<float>& data, float position);
    
    // Performance optimization
    void optimizeForLatency();
    void optimizeForAccuracy();
};

} // namespace vital