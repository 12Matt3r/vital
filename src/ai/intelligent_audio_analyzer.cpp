#include "intelligent_audio_analyzer.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <fstream>
#include <sstream>
#include <thread>
#include <future>

namespace vital {

IntelligentAudioAnalyzer::IntelligentAudioAnalyzer(AIManager* ai_manager) 
    : ai_manager_(ai_manager), real_time_monitoring_active_(false) {
    
    // Default configuration
    config_.sample_rate = 44100;
    config_.frame_size = 1024;
    config_.hop_size = 512;
    config_.window_type = 1; // Hann window
    config_.enable_mfcc = true;
    config_.enable_chroma = true;
    config_.enable_pitch_tracking = true;
    config_.enable_classification = true;
    config_.enable_learning = true;
    config_.feature_smoothing = 0.1f;
    config_.analysis_history_size = 100;
    
    // Initialize window function
    window_ = generateWindow(config_.frame_size, config_.window_type);
    
    // Initialize feature statistics
    feature_stats_.means = std::vector<float>(AudioFeatures::NUM_FEATURES, 0.0f);
    feature_stats_.standard_deviations = std::vector<float>(AudioFeatures::NUM_FEATURES, 0.0f);
    feature_stats_.minima = std::vector<float>(AudioFeatures::NUM_FEATURES, 0.0f);
    feature_stats_.maxima = std::vector<float>(AudioFeatures::NUM_FEATURES, 0.0f);
    
    // Initialize real-time metrics
    real_time_metrics_ = RealTimeMetrics{};
    
    if (ai_manager_) {
        ai_manager_->recordEvent(AIManager::FeatureType::AudioAnalysis, "Analyzer initialized");
    }
}

IntelligentAudioAnalyzer::~IntelligentAudioAnalyzer() {
    real_time_monitoring_active_ = false;
}

void IntelligentAudioAnalyzer::setAnalysisConfig(const AnalysisConfig& config) {
    config_ = config;
    
    // Update dependent components
    window_ = generateWindow(config_.frame_size, config_.window_type);
    
    if (ai_manager_) {
        ai_manager_->recordEvent(AIManager::FeatureType::AudioAnalysis, "Analysis config updated");
    }
}

void IntelligentAudioAnalyzer::updateSampleRate(size_t sample_rate) {
    config_.sample_rate = sample_rate;
    mel_filter_bank_ = generateMelFilterBank(26, config_.frame_size, sample_rate);
}

// Real-time processing
IntelligentAudioAnalyzer::AudioFeatures IntelligentAudioAnalyzer::analyzeAudio(const std::vector<float>& audio_block) {
    AudioFeatures features;
    auto start_time = std::chrono::high_resolution_clock::now();
    
    try {
        // Apply window
        std::vector<float> windowed_audio = applyWindow(audio_block, config_.window_type);
        
        // Compute FFT
        std::vector<std::complex<float>> spectrum;
        computeSpectrum(windowed_audio, spectrum);
        
        // Extract features
        extractSpectralFeatures(spectrum, features);
        extractTemporalFeatures(windowed_audio, features);
        
        if (config_.enable_harmonic_features) {
            extractHarmonicFeatures(windowed_audio, features);
        }
        
        extractPerceptualFeatures(features);
        
        // Smooth features if we have history
        if (!feature_history_.empty()) {
            smoothFeatures(features, feature_history_.back());
        }
        
        // Add to history
        {
            std::lock_guard<std::mutex> lock(history_mutex_);
            feature_history_.push_back(features);
            if (feature_history_.size() > config_.analysis_history_size) {
                feature_history_.erase(feature_history_.begin());
            }
        }
        
        // Update statistics
        {
            std::lock_guard<std::mutex> lock(stats_mutex_);
            stats_.total_audio_blocks_processed++;
            stats_.total_features_extracted += 1;
            
            auto end_time = std::chrono::high_resolution_clock::now();
            float processing_time = std::chrono::duration<float>(end_time - start_time).count() * 1000.0f;
            stats_.processing_times.push_back(processing_time);
            
            if (stats_.processing_times.size() > 1000) {
                stats_.processing_times.erase(stats_.processing_times.begin());
            }
            
            float avg_time = std::accumulate(stats_.processing_times.begin(), stats_.processing_times.end(), 0.0f) / 
                           stats_.processing_times.size();
            stats_.average_processing_time_ms = avg_time;
        }
        
        // Update real-time metrics
        if (real_time_monitoring_active_) {
            std::lock_guard<std::mutex> lock(metrics_mutex_);
            real_time_metrics_.current_spectral_centroid = features.spectral_centroid;
            real_time_metrics_.current_rms = features.rms;
            real_time_metrics_.current_pitch = features.fundamental_frequency;
            real_time_metrics_.buffer_size = audio_block.size();
        }
        
    } catch (const std::exception& e) {
        if (ai_manager_) {
            ai_manager_->recordEvent(AIManager::FeatureType::AudioAnalysis, 
                "Analysis error: " + std::string(e.what()));
        }
    }
    
    return features;
}

IntelligentAudioAnalyzer::AudioFeatures IntelligentAudioAnalyzer::analyzeAudioInRealTime(const std::vector<float>& audio_block) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    AudioFeatures features = analyzeAudio(audio_block);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    float latency = std::chrono::duration<float>(end_time - start_time).count() * 1000.0f;
    
    if (real_time_monitoring_active_) {
        std::lock_guard<std::mutex> lock(metrics_mutex_);
        real_time_metrics_.processing_latency_ms = latency;
    }
    
    return features;
}

// Classification
IntelligentAudioAnalyzer::ClassificationResult IntelligentAudioAnalyzer::classifyAudio(const AudioFeatures& features) {
    ClassificationResult result;
    result.timestamp = std::chrono::high_resolution_clock::now();
    
    try {
        // Apply multiple classification methods and ensemble
        AudioClass knn_result = classifyKNN(features);
        AudioClass bayes_result = classifyBayes(features);
        AudioClass neural_result = classifyNeural(features);
        
        // Ensemble voting
        std::map<AudioClass, int> votes;
        votes[knn_result]++;
        votes[bayes_result]++;
        votes[neural_result]++;
        
        // Find winner
        AudioClass best_class = AudioClass::Unknown;
        int max_votes = 0;
        float total_confidence = 0.0f;
        
        for (const auto& [cls, vote_count] : votes) {
            float confidence = static_cast<float>(vote_count) / 3.0f;
            if (vote_count > max_votes) {
                max_votes = vote_count;
                best_class = cls;
                total_confidence = confidence;
            }
        }
        
        result.predicted_class = best_class;
        result.confidence = total_confidence;
        result.description = "Ensemble classification";
        
        // Calculate class probabilities
        for (int i = 0; i < static_cast<int>(AudioClass::Unknown); ++i) {
            AudioClass cls = static_cast<AudioClass>(i);
            float prob = static_cast<float>(votes[cls]) / 3.0f;
            result.class_probabilities.emplace_back(cls, prob);
        }
        
        // Sort by probability
        std::sort(result.class_probabilities.begin(), result.class_probabilities.end(),
                  [](const auto& a, const auto& b) { return a.second > b.second; });
        
        {
            std::lock_guard<std::mutex> lock(stats_mutex_);
            stats_.total_classifications++;
            stats_.classification_counts[best_class]++;
        }
        
        if (ai_manager_) {
            ai_manager_->recordEvent(AIManager::FeatureType::AudioAnalysis, 
                "Audio classified: " + std::to_string(static_cast<int>(best_class)) + 
                " confidence: " + std::to_string(total_confidence));
        }
        
    } catch (const std::exception& e) {
        result.predicted_class = AudioClass::Unknown;
        result.confidence = 0.0f;
        result.description = "Classification failed: " + std::string(e.what());
    }
    
    return result;
}

// Pitch detection
float IntelligentAudioAnalyzer::detectPitch(const std::vector<float>& audio_block) {
    if (!config_.enable_pitch_tracking || audio_block.empty()) {
        return 0.0f;
    }
    
    // Use autocorrelation method for pitch detection
    return detectPitchAutocorrelation(audio_block);
}

float IntelligentAudioAnalyzer::detectPitchAutocorrelation(const std::vector<float>& audio_block) {
    if (audio_block.size() < 64) return 0.0f;
    
    size_t min_period = static_cast<size_t>(config_.sample_rate / 2000.0f); // 2kHz max
    size_t max_period = static_cast<size_t>(config_.sample_rate / 50.0f);   // 50Hz min
    
    min_period = std::max(min_period, static_cast<size_t>(1));
    max_period = std::min(max_period, audio_block.size() / 2);
    
    float max_correlation = 0.0f;
    size_t best_period = min_period;
    
    // Compute autocorrelation
    for (size_t period = min_period; period < max_period; ++period) {
        float correlation = 0.0f;
        size_t valid_samples = audio_block.size() - period;
        
        for (size_t i = 0; i < valid_samples; ++i) {
            correlation += audio_block[i] * audio_block[i + period];
        }
        
        correlation /= valid_samples;
        
        if (correlation > max_correlation) {
            max_correlation = correlation;
            best_period = period;
        }
    }
    
    // Convert period to frequency
    float frequency = (best_period > 0) ? static_cast<float>(config_.sample_rate) / best_period : 0.0f;
    
    // Return 0 if correlation is too low (likely silence or noise)
    return (max_correlation > 0.001f) ? frequency : 0.0f;
}

float IntelligentAudioAnalyzer::detectPitchYIN(const std::vector<float>& audio_block) {
    // Simplified YIN algorithm implementation
    // In real implementation, would use complete YIN algorithm
    return detectPitchAutocorrelation(audio_block);
}

// Spectral analysis
void IntelligentAudioAnalyzer::computeSpectrum(const std::vector<float>& audio_block, std::vector<std::complex<float>>& spectrum) {
    performFFT(audio_block, spectrum);
}

void IntelligentAudioAnalyzer::computeMelSpectrum(const std::vector<float>& audio_block, std::vector<float>& mel_spectrum) {
    std::vector<std::complex<float>> fft_spectrum;
    computeSpectrum(audio_block, fft_spectrum);
    
    size_t num_mel_filters = 26;
    mel_spectrum.resize(num_mel_filters);
    
    // Apply mel filter bank
    for (size_t i = 0; i < num_mel_filters; ++i) {
        float mel_value = 0.0f;
        size_t start = static_cast<size_t>(i * fft_spectrum.size() / num_mel_filters);
        size_t end = static_cast<size_t>((i + 1) * fft_spectrum.size() / num_mel_filters);
        
        for (size_t j = start; j < end && j < fft_spectrum.size(); ++j) {
            mel_value += std::abs(fft_spectrum[j]);
        }
        
        mel_spectrum[i] = mel_value / (end - start);
    }
    
    // Convert to log scale
    for (auto& value : mel_spectrum) {
        value = std::log(std::max(value, 1e-10f));
    }
}

void IntelligentAudioAnalyzer::computeChromaFeatures(const std::vector<std::complex<float>>& spectrum, std::vector<float>& chroma) {
    chroma.resize(AudioFeatures::CHROMA_BINS, 0.0f);
    
    size_t fft_size = spectrum.size();
    
    // Map FFT bins to pitch classes
    for (size_t i = 1; i < fft_size / 2; ++i) {
        float frequency = static_cast<float>(i) * config_.sample_rate / fft_size;
        
        // Convert frequency to pitch class (C, C#, D, etc.)
        if (frequency > 0) {
            float pitch_class = 12.0f * std::log2(frequency / 440.0f) + 57.0f; // A4 = 57
            int class_index = static_cast<int>(std::round(pitch_class)) % AudioFeatures::CHROMA_BINS;
            
            if (class_index < 0) class_index += AudioFeatures::CHROMA_BINS;
            if (class_index >= 0 && class_index < static_cast<int>(AudioFeatures::CHROMA_BINS)) {
                chroma[class_index] += std::abs(spectrum[i]);
            }
        }
    }
}

// Feature extraction
void IntelligentAudioAnalyzer::extractMFCC(const std::vector<std::complex<float>>& spectrum, std::vector<float>& mfcc) {
    mfcc.resize(AudioFeatures::MFCC_BINS, 0.0f);
    
    // Get magnitude spectrum
    std::vector<float> magnitude_spectrum(spectrum.size());
    for (size_t i = 0; i < spectrum.size(); ++i) {
        magnitude_spectrum[i] = std::abs(spectrum[i]);
    }
    
    // Apply mel filter bank
    std::vector<float> mel_spectrum;
    computeMelSpectrum(std::vector<float>(magnitude_spectrum.begin(), magnitude_spectrum.end()), mel_spectrum);
    
    // Apply DCT to get MFCC coefficients
    for (size_t i = 0; i < AudioFeatures::MFCC_BINS; ++i) {
        for (size_t j = 0; j < mel_spectrum.size(); ++j) {
            mfcc[i] += mel_spectrum[j] * std::cos(M_PI * i * (j + 0.5f) / mel_spectrum.size());
        }
    }
}

void IntelligentAudioAnalyzer::extractSpectralFeatures(const std::vector<std::complex<float>>& spectrum, AudioFeatures& features) {
    std::vector<float> magnitude(spectrum.size() / 2);
    for (size_t i = 0; i < magnitude.size(); ++i) {
        magnitude[i] = std::abs(spectrum[i]);
    }
    
    // Spectral centroid
    float weighted_sum = 0.0f;
    float magnitude_sum = 0.0f;
    
    for (size_t i = 0; i < magnitude.size(); ++i) {
        float frequency = static_cast<float>(i) * config_.sample_rate / magnitude.size();
        weighted_sum += frequency * magnitude[i];
        magnitude_sum += magnitude[i];
    }
    
    features.spectral_centroid = (magnitude_sum > 0) ? weighted_sum / magnitude_sum : 0.0f;
    
    // Spectral rolloff (85% of energy)
    float energy_threshold = 0.85f * magnitude_sum;
    float running_sum = 0.0f;
    features.spectral_rolloff = 0.0f;
    
    for (size_t i = 0; i < magnitude.size(); ++i) {
        running_sum += magnitude[i];
        if (running_sum >= energy_threshold) {
            features.spectral_rolloff = static_cast<float>(i) * config_.sample_rate / magnitude.size();
            break;
        }
    }
    
    // Spectral flatness
    float geometric_mean = 1.0f;
    float arithmetic_mean = 0.0f;
    
    for (float mag : magnitude) {
        geometric_mean *= std::pow(mag + 1e-10f, 1.0f / magnitude.size());
        arithmetic_mean += mag;
    }
    arithmetic_mean /= magnitude.size();
    
    features.spectral_flatness = (arithmetic_mean > 0) ? geometric_mean / arithmetic_mean : 0.0f;
    
    // Spectral bandwidth
    float bandwidth_sum = 0.0f;
    for (size_t i = 0; i < magnitude.size(); ++i) {
        float freq = static_cast<float>(i) * config_.sample_rate / magnitude.size();
        float diff = std::abs(freq - features.spectral_centroid);
        bandwidth_sum += diff * magnitude[i];
    }
    
    features.spectral_bandwidth = (magnitude_sum > 0) ? bandwidth_sum / magnitude_sum : 0.0f;
    
    // MFCC features
    if (config_.enable_mfcc) {
        extractMFCC(spectrum, features.mfcc);
    }
    
    // Chroma features
    if (config_.enable_chroma) {
        computeChromaFeatures(spectrum, features.chroma);
    }
}

void IntelligentAudioAnalyzer::extractTemporalFeatures(const std::vector<float>& audio_block, AudioFeatures& features) {
    if (audio_block.empty()) return;
    
    // Zero crossing rate
    size_t zero_crossings = 0;
    for (size_t i = 1; i < audio_block.size(); ++i) {
        if ((audio_block[i-1] >= 0.0f) != (audio_block[i] >= 0.0f)) {
            zero_crossings++;
        }
    }
    features.zero_crossing_rate = static_cast<float>(zero_crossings) / audio_block.size();
    
    // Energy and RMS
    float sum_squares = 0.0f;
    for (float sample : audio_block) {
        sum_squares += sample * sample;
    }
    
    features.energy = sum_squares;
    features.rms = std::sqrt(sum_squares / audio_block.size());
    
    // Statistical moments
    float mean = std::accumulate(audio_block.begin(), audio_block.end(), 0.0f) / audio_block.size();
    
    float variance = 0.0f;
    for (float sample : audio_block) {
        float diff = sample - mean;
        variance += diff * diff;
    }
    variance /= audio_block.size();
    
    features.variance = variance;
    features.skewness = 0.0f; // Simplified
    features.kurtosis = 0.0f; // Simplified
}

void IntelligentAudioAnalyzer::extractHarmonicFeatures(const std::vector<float>& audio_block, AudioFeatures& features) {
    if (audio_block.empty() || !config_.enable_pitch_tracking) return;
    
    // Fundamental frequency (pitch)
    features.fundamental_frequency = detectPitch(audio_block);
    
    // Harmonic ratio
    if (features.fundamental_frequency > 0) {
        float harmonic_energy = 0.0f;
        float total_energy = 0.0f;
        
        // Look for harmonics up to 10th
        for (size_t harmonic = 1; harmonic <= 10; ++harmonic) {
            // Simplified harmonic detection
            // In real implementation, would use more sophisticated methods
            float expected_bin = features.fundamental_frequency * harmonic * audio_block.size() / config_.sample_rate;
            size_t bin_index = static_cast<size_t>(expected_bin);
            
            if (bin_index < audio_block.size()) {
                float sample_val = audio_block[bin_index];
                harmonic_energy += sample_val * sample_val;
            }
            
            total_energy += audio_block[bin_index] * audio_block[bin_index];
        }
        
        features.harmonic_ratio = (total_energy > 0) ? harmonic_energy / total_energy : 0.0f;
    }
}

void IntelligentAudioAnalyzer::extractPerceptualFeatures(const AudioFeatures& features) {
    // Brightness (high-frequency content)
    features.brightness = features.spectral_centroid / (config_.sample_rate / 2.0f);
    
    // Warmth (low-mid frequency content)
    features.warmth = 1.0f - features.brightness;
    
    // Clarity (inverse of spectral flatness)
    features.clarity = 1.0f - features.spectral_flatness;
    
    // Roughness (spectral irregularity)
    features.roughness = features.spectral_bandwidth / features.spectral_centroid;
    
    // Richness (harmonic content)
    features.richness = features.harmonic_ratio;
}

// Classification algorithms
IntelligentAudioAnalyzer::AudioClass IntelligentAudioAnalyzer::classifyKNN(const AudioFeatures& features, size_t k) {
    if (training_data_.size() < k) return AudioClass::Unknown;
    
    // Calculate distances to all training samples
    std::vector<std::pair<float, AudioClass>> distances;
    distances.reserve(training_data_.size());
    
    for (const auto& [train_features, label] : training_data_) {
        float distance = compareFeatures(features, train_features);
        distances.emplace_back(distance, label);
    }
    
    // Sort by distance
    std::partial_sort(distances.begin(), distances.begin() + k, distances.end(),
                     [](const auto& a, const auto& b) { return a.first < b.first; });
    
    // Vote on k nearest neighbors
    std::map<AudioClass, int> votes;
    for (size_t i = 0; i < k; ++i) {
        votes[distances[i].second]++;
    }
    
    // Find majority class
    AudioClass best_class = AudioClass::Unknown;
    int max_votes = 0;
    
    for (const auto& [cls, vote_count] : votes) {
        if (vote_count > max_votes) {
            max_votes = vote_count;
            best_class = cls;
        }
    }
    
    return best_class;
}

IntelligentAudioAnalyzer::AudioClass IntelligentAudioAnalyzer::classifyBayes(const AudioFeatures& features) {
    // Simplified Naive Bayes classifier
    // In real implementation, would maintain feature statistics per class
    
    // For demonstration, use simple rules based on spectral features
    if (features.spectral_centroid > 2000.0f) {
        if (features.energy > 0.1f) return AudioClass::Lead;
        return AudioClass::Speech;
    } else if (features.spectral_centroid > 1000.0f) {
        return AudioClass::Pad;
    } else {
        if (features.zero_crossing_rate > 0.1f) return AudioClass::Drum;
        return AudioClass::Bass;
    }
}

IntelligentAudioAnalyzer::AudioClass IntelligentAudioAnalyzer::classifyNeural(const AudioFeatures& features) {
    // Simplified neural network classification
    // In real implementation, would use trained neural network
    
    // For demonstration, use combined feature analysis
    float combined_score = features.spectral_centroid * 0.3f + 
                          features.energy * 0.2f + 
                          features.harmonic_ratio * 0.3f + 
                          (1.0f - features.spectral_flatness) * 0.2f;
    
    if (combined_score > 0.8f) return AudioClass::Lead;
    if (combined_score > 0.6f) return AudioClass::Pad;
    if (combined_score > 0.4f) return AudioClass::Bass;
    if (combined_score > 0.2f) return AudioClass::Drum;
    
    return AudioClass::Noise;
}

// Feature comparison
float IntelligentAudioAnalyzer::compareFeatures(const AudioFeatures& a, const AudioFeatures& b) {
    // Simplified Euclidean distance
    float distance = 0.0f;
    
    // Compare spectral features
    distance += (a.spectral_centroid - b.spectral_centroid) * (a.spectral_centroid - b.spectral_centroid);
    distance += (a.spectral_rolloff - b.spectral_rolloff) * (a.spectral_rolloff - b.spectral_rolloff);
    distance += (a.spectral_flatness - b.spectral_flatness) * (a.spectral_flatness - b.spectral_flatness);
    distance += (a.energy - b.energy) * (a.energy - b.energy);
    distance += (a.rms - b.rms) * (a.rms - b.rms);
    
    return std::sqrt(distance);
}

// Learning
void IntelligentAudioAnalyzer::learnFromUserFeedback(const AudioFeatures& features, AudioClass correct_class) {
    if (!config_.enable_learning) return;
    
    {
        std::lock_guard<std::mutex> lock(history_mutex_);
        training_data_.push_back({features, correct_class});
        
        // Keep only recent training data
        if (training_data_.size() > 1000) {
            training_data_.erase(training_data_.begin());
        }
    }
    
    if (ai_manager_) {
        ai_manager_->recordEvent(AIManager::FeatureType::AudioAnalysis, 
            "Learned from user feedback: class " + std::to_string(static_cast<int>(correct_class)));
    }
}

// Real-time monitoring
IntelligentAudioAnalyzer::RealTimeMetrics IntelligentAudioAnalyzer::getRealTimeMetrics() const {
    std::lock_guard<std::mutex> lock(metrics_mutex_);
    return real_time_metrics_;
}

void IntelligentAudioAnalyzer::startRealTimeMonitoring() {
    real_time_monitoring_active_ = true;
}

void IntelligentAudioAnalyzer::stopRealTimeMonitoring() {
    real_time_monitoring_active_ = false;
}

// Quality metrics
IntelligentAudioAnalyzer::QualityMetrics IntelligentAudioAnalyzer::getQualityMetrics() const {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    return quality_metrics_;
}

// Statistics
void IntelligentAudioAnalyzer::resetStats() {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    stats_ = AnalysisStats{};
    
    {
        std::lock_guard<std::mutex> lock2(history_mutex_);
        feature_history_.clear();
        training_data_.clear();
    }
}

// Utility functions
std::vector<float> IntelligentAudioAnalyzer::applyWindow(const std::vector<float>& audio_block, size_t window_type) {
    std::vector<float> windowed = audio_block;
    
    for (size_t i = 0; i < std::min(windowed.size(), window_.size()); ++i) {
        windowed[i] *= window_[i];
    }
    
    return windowed;
}

std::vector<float> IntelligentAudioAnalyzer::generateWindow(size_t size, size_t window_type) {
    std::vector<float> window(size);
    
    switch (window_type) {
        case 0: // Rectangular
            for (size_t i = 0; i < size; ++i) {
                window[i] = 1.0f;
            }
            break;
            
        case 1: // Hann
            for (size_t i = 0; i < size; ++i) {
                window[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (size - 1)));
            }
            break;
            
        case 2: // Hamming
            for (size_t i = 0; i < size; ++i) {
                window[i] = 0.54f - 0.46f * std::cos(2.0f * M_PI * i / (size - 1));
            }
            break;
            
        case 3: // Blackman
            for (size_t i = 0; i < size; ++i) {
                window[i] = 0.42f - 0.5f * std::cos(2.0f * M_PI * i / (size - 1)) + 
                           0.08f * std::cos(4.0f * M_PI * i / (size - 1));
            }
            break;
            
        default:
            // Default to Hann
            return generateWindow(size, 1);
    }
    
    return window;
}

std::vector<float> IntelligentAudioAnalyzer::generateMelFilterBank(size_t num_filters, size_t fft_size, float sample_rate) {
    std::vector<float> filter_bank(num_filters * fft_size / 2, 0.0f);
    
    // Convert to mel scale
    float mel_max = 2595.0f * std::log10(1.0f + sample_rate / 2 / 700.0f);
    std::vector<float> mel_centers(num_filters + 2);
    
    for (size_t i = 0; i < mel_centers.size(); ++i) {
        mel_centers[i] = i * mel_max / (num_filters + 1);
    }
    
    // Convert back to frequency
    std::vector<float> freq_centers(mel_centers.size());
    for (size_t i = 0; i < mel_centers.size(); ++i) {
        freq_centers[i] = 700.0f * (std::pow(10.0f, mel_centers[i] / 2595.0f) - 1.0f);
    }
    
    // Convert to FFT bin numbers
    std::vector<size_t> bin_centers(freq_centers.size());
    for (size_t i = 0; i < freq_centers.size(); ++i) {
        bin_centers[i] = static_cast<size_t>((fft_size + 1) * freq_centers[i] / sample_rate);
    }
    
    // Create triangular filters
    for (size_t m = 1; m <= num_filters; ++m) {
        size_t left = bin_centers[m - 1];
        size_t center = bin_centers[m];
        size_t right = bin_centers[m + 1];
        
        for (size_t k = left; k < center; ++k) {
            filter_bank[(m - 1) * fft_size / 2 + k] = (k - left) / (center - left);
        }
        
        for (size_t k = center; k < right; ++k) {
            filter_bank[(m - 1) * fft_size / 2 + k] = (right - k) / (right - center);
        }
    }
    
    return filter_bank;
}

// Feature smoothing
void IntelligentAudioAnalyzer::smoothFeatures(AudioFeatures& current, const AudioFeatures& previous) {
    float alpha = config_.feature_smoothing;
    
    current.spectral_centroid = alpha * current.spectral_centroid + (1.0f - alpha) * previous.spectral_centroid;
    current.energy = alpha * current.energy + (1.0f - alpha) * previous.energy;
    current.rms = alpha * current.rms + (1.0f - alpha) * previous.rms;
    
    // Add more smoothing for other features as needed
}

// Musical intelligence
float IntelligentAudioAnalyzer::estimateTempo(const std::vector<float>& audio_block) {
    // Simplified tempo estimation based on energy peaks
    // In real implementation, would use beat tracking algorithms
    
    float rms_values[16] = {0};
    size_t window_size = audio_block.size() / 16;
    
    for (size_t i = 0; i < 16; ++i) {
        float sum = 0.0f;
        size_t start = i * window_size;
        size_t end = std::min(start + window_size, audio_block.size());
        
        for (size_t j = start; j < end; ++j) {
            sum += audio_block[j] * audio_block[j];
        }
        
        rms_values[i] = std::sqrt(sum / (end - start));
    }
    
    // Find peaks (simplified)
    float peak_threshold = 1.5f * std::accumulate(rms_values, rms_values + 16, 0.0f) / 16.0f;
    size_t peak_count = 0;
    
    for (size_t i = 1; i < 15; ++i) {
        if (rms_values[i] > peak_threshold && 
            rms_values[i] > rms_values[i-1] && 
            rms_values[i] > rms_values[i+1]) {
            peak_count++;
        }
    }
    
    // Estimate BPM based on peak patterns
    float bpm = peak_count * 60.0f * 16 / (audio_block.size() / config_.sample_rate);
    
    return std::clamp(bpm, 60.0f, 180.0f);
}

// Performance optimization
void IntelligentAudioAnalyzer::optimizeForLatency() {
    config_.frame_size = 512;
    config_.hop_size = 256;
    config_.enable_mfcc = false; // Disable expensive features
    config_.feature_smoothing = 0.2f; // More aggressive smoothing
}

void IntelligentAudioAnalyzer::optimizeForAccuracy() {
    config_.frame_size = 2048;
    config_.hop_size = 512;
    config_.enable_mfcc = true;
    config_.enable_chroma = true;
    config_.enable_pitch_tracking = true;
    config_.feature_smoothing = 0.05f; // Less aggressive smoothing
}

// FFT implementation (simplified)
void IntelligentAudioAnalyzer::performFFT(const std::vector<float>& input, std::vector<std::complex<float>>& output) {
    size_t n = getNextPowerOfTwo(input.size());
    output.resize(n);
    
    // Simplified FFT implementation
    for (size_t k = 0; k < n; ++k) {
        std::complex<float> sum(0.0f, 0.0f);
        for (size_t j = 0; j < n; ++j) {
            float angle = -2.0f * M_PI * k * j / n;
            std::complex<float> w(std::cos(angle), std::sin(angle));
            sum += input[j] * w;
        }
        output[k] = sum;
    }
}

size_t IntelligentAudioAnalyzer::getNextPowerOfTwo(size_t n) {
    size_t power = 1;
    while (power < n) {
        power <<= 1;
    }
    return power;
}

} // namespace vital