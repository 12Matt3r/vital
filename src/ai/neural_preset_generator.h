#pragma once

#include "ai_manager.h"
#include <vector>
#include <array>
#include <memory>
#include <random>
#include <Eigen/Dense>

namespace vital {

/**
 * @class NeuralPresetGenerator
 * @brief Deep learning-based intelligent preset generation system
 * 
 * Uses neural networks to generate synthesizer presets based on user preferences,
 * audio analysis, and learned patterns from musical data.
 */
class NeuralPresetGenerator {
public:
    // Preset data structure
    struct Preset {
        std::string name;
        std::vector<float> parameters;
        float similarity_score;
        std::vector<float> audio_features;
        std::string genre_tags;
        float user_rating;
        TimePoint creation_time;
        
        static constexpr size_t PARAMETER_COUNT = 64;
    };
    
    // Neural network architecture
    struct NetworkConfig {
        std::vector<size_t> layer_sizes;
        std::vector<std::string> activation_functions;
        float learning_rate;
        float dropout_rate;
        size_t batch_size;
        size_t epochs;
    };
    
    // Generation modes
    enum class GenerationMode {
        Deterministic,    // Same input always generates same output
        Stochastic,       // Introduces controlled randomness
        Variational,      // Uses variational autoencoder approach
        Conditional       // Generates based on conditions (genre, mood, etc.)
    };
    
    NeuralPresetGenerator(AIManager* ai_manager);
    ~NeuralPresetGenerator();
    
    // Configuration
    void setNetworkConfig(const NetworkConfig& config);
    void setGenerationMode(GenerationMode mode);
    void setUserPreferences(const std::vector<float>& preferences);
    
    // Preset generation
    Preset generatePreset();
    Preset generatePreset(const std::vector<float>& conditions);
    std::vector<Preset> generateBatch(size_t count);
    Preset mutatePreset(const Preset& original, float mutation_strength = 0.1f);
    
    // Learning from user behavior
    void learnFromUserFeedback(const Preset& preset, float user_rating);
    void learnFromAudioAnalysis(const std::vector<float>& audio_features, const Preset& preset);
    void learnFromGenrePreference(const std::string& genre, const Preset& preset);
    
    // Audio analysis integration
    std::vector<float> analyzeAudioForPreset(const std::vector<float>& audio_data);
    void setTargetAudioFeatures(const std::vector<float>& features);
    
    // Model management
    bool loadModel(const std::string& model_path);
    bool saveModel(const std::string& model_path);
    void retrainModel(const std::vector<Preset>& training_data);
    
    // Quality assessment
    float assessPresetQuality(const Preset& preset);
    std::vector<float> getPresetSimilarity(const Preset& a, const Preset& b);
    
    // Statistics
    struct GenerationStats {
        uint64_t total_generated = 0;
        uint64_t successful_generations = 0;
        float average_rating = 0.0f;
        std::vector<float> generation_times;
        std::unordered_map<std::string, int> genre_distribution;
    };
    
    GenerationStats getStats() const { return stats_; }
    void resetStats();
    
    // Constraints and validation
    void addParameterConstraint(size_t param_index, float min_val, float max_val);
    void addSoftConstraint(size_t param_index, float preferred_val, float weight);
    void validatePreset(Preset& preset);
    
    // Batch processing
    std::vector<Preset> generateWithConstraints(const std::vector<float>& conditions,
                                               const std::vector<size_t>& required_params);
    
private:
    // Neural network implementation
    class NeuralNetwork {
    public:
        NeuralNetwork();
        void initialize(const NetworkConfig& config);
        
        std::vector<float> forward(const std::vector<float>& input);
        void backward(const std::vector<float>& target);
        void updateWeights(float learning_rate);
        
        bool save(const std::string& path);
        bool load(const std::string& path);
        
    private:
        struct Layer {
            Eigen::MatrixXf weights;
            Eigen::VectorXf bias;
            std::vector<float> activation;
            std::vector<float> delta;
            std::string activation_type;
        };
        
        std::vector<Layer> layers_;
        NetworkConfig config_;
        std::random_device rd_;
        std::mt19937 rng_;
    };
    
    // Member variables
    AIManager* ai_manager_;
    std::unique_ptr<NeuralNetwork> network_;
    NetworkConfig config_;
    GenerationMode mode_;
    
    // User learning data
    std::vector<Preset> training_data_;
    std::vector<float> user_preferences_;
    std::vector<std::pair<float, float>> parameter_constraints_;
    std::vector<std::tuple<size_t, float, float>> soft_constraints_;
    
    // Audio features
    std::vector<float> target_audio_features_;
    
    // Statistics and learning
    mutable std::mutex data_mutex_;
    GenerationStats stats_;
    std::vector<float> quality_weights_;
    
    // Helper methods
    std::vector<float> encodeCondition(const std::vector<float>& conditions);
    std::vector<float> decodePreset(const std::vector<float>& network_output);
    void normalizeParameters(std::vector<float>& parameters);
    void denormalizeParameters(std::vector<float>& parameters);
    
    // Random generation helpers
    std::vector<float> generateRandomPreset();
    std::vector<float> interpolatePresets(const Preset& a, const Preset& b, float t);
    
    // Quality scoring
    float calculateSimilarityScore(const Preset& a, const Preset& b);
    float calculateGenreMatch(const Preset& preset, const std::string& target_genre);
    float calculateUserPreferenceScore(const Preset& preset);
};

} // namespace vital