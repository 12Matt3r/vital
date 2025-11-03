#include "neural_preset_generator.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <fstream>
#include <sstream>

namespace vital {

// NeuralNetwork implementation
NeuralPresetGenerator::NeuralNetwork::NeuralNetwork() : rng_(42) {}

void NeuralPresetGenerator::NeuralNetwork::initialize(const NetworkConfig& config) {
    config_ = config;
    layers_.clear();
    
    std::uniform_real_distribution<float> weight_dist(-0.1f, 0.1f);
    
    for (size_t i = 0; i < config_.layer_sizes.size() - 1; ++i) {
        Layer layer;
        layer.weights = Eigen::MatrixXf(config_.layer_sizes[i + 1], config_.layer_sizes[i]);
        layer.bias = Eigen::VectorXf(config_.layer_sizes[i + 1]);
        
        // Initialize weights
        for (auto w : layer.weights.data()) {
            w = weight_dist(rng_);
        }
        for (auto b : layer.bias.data()) {
            b = weight_dist(rng_);
        }
        
        layer.activation_type = config_.activation_functions[i];
        layers_.push_back(std::move(layer));
    }
}

std::vector<float> NeuralPresetGenerator::NeuralNetwork::forward(const std::vector<float>& input) {
    std::vector<float> current_activation = input;
    
    for (auto& layer : layers_) {
        Eigen::VectorXf input_vec = Eigen::Map<const Eigen::VectorXf>(current_activation.data(), current_activation.size());
        Eigen::VectorXf output = layer.weights * input_vec + layer.bias;
        
        // Apply activation function
        layer.activation.resize(output.size());
        if (layer.activation_type == "relu") {
            for (size_t i = 0; i < output.size(); ++i) {
                layer.activation[i] = std::max(0.0f, output(i));
            }
        } else if (layer.activation_type == "tanh") {
            for (size_t i = 0; i < output.size(); ++i) {
                layer.activation[i] = std::tanh(output(i));
            }
        } else if (layer.activation_type == "sigmoid") {
            for (size_t i = 0; i < output.size(); ++i) {
                layer.activation[i] = 1.0f / (1.0f + std::exp(-output(i)));
            }
        } else {
            // Linear activation
            for (size_t i = 0; i < output.size(); ++i) {
                layer.activation[i] = output(i);
            }
        }
        
        current_activation = layer.activation;
    }
    
    return current_activation;
}

void NeuralPresetGenerator::NeuralNetwork::backward(const std::vector<float>& target) {
    if (layers_.empty()) return;
    
    // Simple gradient descent for demonstration
    // In real implementation, use more sophisticated backpropagation
    for (auto& layer : layers_) {
        for (auto& w : layer.weights.data()) {
            w -= config_.learning_rate * (std::rand() % 100 - 50) * 0.001f;
        }
        
        for (auto& b : layer.bias.data()) {
            b -= config_.learning_rate * (std::rand() % 100 - 50) * 0.001f;
        }
    }
}

void NeuralPresetGenerator::NeuralNetwork::updateWeights(float learning_rate) {
    // Implementation for weight updates during training
    for (auto& layer : layers_) {
        layer.weights *= (1.0f - learning_rate * 0.001f);
    }
}

bool NeuralPresetGenerator::NeuralNetwork::save(const std::string& path) {
    try {
        std::ofstream file(path);
        if (!file.is_open()) return false;
        
        file << "neural_network_v1\n";
        file << config_.layer_sizes.size() << "\n";
        for (size_t size : config_.layer_sizes) {
            file << size << " ";
        }
        file << "\n";
        
        for (const auto& layer : layers_) {
            file << layer.weights.rows() << " " << layer.weights.cols() << "\n";
            for (size_t i = 0; i < layer.weights.rows(); ++i) {
                for (size_t j = 0; j < layer.weights.cols(); ++j) {
                    file << layer.weights(i, j) << " ";
                }
                file << "\n";
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool NeuralPresetGenerator::NeuralNetwork::load(const std::string& path) {
    try {
        std::ifstream file(path);
        if (!file.is_open()) return false;
        
        std::string version;
        std::getline(file, version);
        if (version != "neural_network_v1") return false;
        
        size_t num_layers;
        file >> num_layers;
        config_.layer_sizes.resize(num_layers);
        
        for (size_t i = 0; i < num_layers; ++i) {
            file >> config_.layer_sizes[i];
        }
        
        layers_.clear();
        for (size_t i = 0; i < num_layers - 1; ++i) {
            size_t rows, cols;
            file >> rows >> cols;
            
            Layer layer;
            layer.weights = Eigen::MatrixXf(rows, cols);
            layer.bias = Eigen::VectorXf(rows);
            
            for (size_t r = 0; r < rows; ++r) {
                for (size_t c = 0; c < cols; ++c) {
                    file >> layer.weights(r, c);
                }
            }
            
            for (size_t r = 0; r < rows; ++r) {
                file >> layer.bias(r);
            }
            
            layers_.push_back(std::move(layer));
        }
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

// NeuralPresetGenerator implementation
NeuralPresetGenerator::NeuralPresetGenerator(AIManager* ai_manager) 
    : ai_manager_(ai_manager), mode_(GenerationMode::Stochastic) {
    
    // Default network configuration
    config_.layer_sizes = {32, 64, 64, Preset::PARAMETER_COUNT};
    config_.activation_functions = {"relu", "relu", "tanh"};
    config_.learning_rate = 0.001f;
    config_.dropout_rate = 0.2f;
    config_.batch_size = 32;
    config_.epochs = 100;
    
    network_ = std::make_unique<NeuralNetwork>();
    network_->initialize(config_);
    
    // Initialize quality weights
    quality_weights_ = std::vector<float>(Preset::PARAMETER_COUNT, 1.0f);
    
    // Initialize user preferences
    user_preferences_ = std::vector<float>(16, 0.5f); // Default neutral preferences
}

NeuralPresetGenerator::~NeuralPresetGenerator() {
    // Cleanup
}

void NeuralPresetGenerator::setNetworkConfig(const NetworkConfig& config) {
    config_ = config;
    network_->initialize(config_);
    
    if (ai_manager_) {
        ai_manager_->recordEvent(AIManager::FeatureType::NeuralPreset, "Network configuration updated");
    }
}

void NeuralPresetGenerator::setGenerationMode(GenerationMode mode) {
    mode_ = mode;
    
    if (ai_manager_) {
        ai_manager_->recordEvent(AIManager::FeatureType::NeuralPreset, 
            "Generation mode set to " + std::to_string(static_cast<int>(mode)));
    }
}

void NeuralPresetGenerator::setUserPreferences(const std::vector<float>& preferences) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    user_preferences_ = preferences;
}

NeuralPresetGenerator::Preset NeuralPresetGenerator::generatePreset() {
    return generatePreset({});
}

NeuralPresetGenerator::Preset NeuralPresetGenerator::generatePreset(const std::vector<float>& conditions) {
    auto start_time = std::chrono::high_resolution_clock::now();
    
    Preset preset;
    preset.parameters.resize(Preset::PARAMETER_COUNT);
    preset.creation_time = std::chrono::high_resolution_clock::now();
    
    try {
        // Encode conditions
        std::vector<float> encoded_conditions = encodeCondition(conditions);
        
        // Generate based on mode
        std::vector<float> network_input = encoded_conditions;
        if (mode_ == GenerationMode::Stochastic) {
            // Add noise for stochastic generation
            std::uniform_real_distribution<float> noise_dist(-0.1f, 0.1f);
            for (auto& val : network_input) {
                val += noise_dist(rng_);
            }
        }
        
        // Forward pass through network
        std::vector<float> network_output = network_->forward(network_input);
        
        // Decode to preset parameters
        preset.parameters = decodePreset(network_output);
        
        // Apply constraints
        validatePreset(preset);
        
        // Calculate quality score
        preset.similarity_score = assessPresetQuality(preset);
        
        // Update statistics
        {
            std::lock_guard<std::mutex> lock(data_mutex_);
            stats_.total_generated++;
            stats_.successful_generations++;
            
            auto end_time = std::chrono::high_resolution_clock::now();
            float generation_time = std::chrono::duration<float>(end_time - start_time).count() * 1000.0f;
            stats_.generation_times.push_back(generation_time);
            
            // Keep only recent generation times (for performance)
            if (stats_.generation_times.size() > 1000) {
                stats_.generation_times.erase(stats_.generation_times.begin());
            }
        }
        
        if (ai_manager_) {
            ai_manager_->recordEvent(AIManager::FeatureType::NeuralPreset, "Preset generated successfully");
        }
        
    } catch (const std::exception& e) {
        {
            std::lock_guard<std::mutex> lock(data_mutex_);
            stats_.failed_generations++;
        }
        
        if (ai_manager_) {
            ai_manager_->recordEvent(AIManager::FeatureType::NeuralPreset, "Preset generation failed: " + std::string(e.what()));
        }
        
        // Return a default preset on failure
        preset = Preset{"Default Generated", std::vector<float>(Preset::PARAMETER_COUNT, 0.5f), 0.0f, {}, "", 0.0f, start_time};
    }
    
    return preset;
}

std::vector<NeuralPresetGenerator::Preset> NeuralPresetGenerator::generateBatch(size_t count) {
    std::vector<Preset> presets;
    presets.reserve(count);
    
    // Submit batch job to AI manager if available
    if (ai_manager_) {
        ai_manager_->submitJob(AIManager::FeatureType::NeuralPreset, 
            [this, count, &presets]() {
                for (size_t i = 0; i < count; ++i) {
                    presets.push_back(generatePreset());
                }
            }, AIManager::AIJob::Priority::Normal);
    } else {
        // Direct generation if no AI manager
        for (size_t i = 0; i < count; ++i) {
            presets.push_back(generatePreset());
        }
    }
    
    return presets;
}

NeuralPresetGenerator::Preset NeuralPresetGenerator::mutatePreset(const Preset& original, float mutation_strength) {
    Preset mutated = original;
    mutated.name += " (Mutated)";
    mutated.creation_time = std::chrono::high_resolution_clock::now();
    
    std::uniform_real_distribution<float> mutation_dist(-mutation_strength, mutation_strength);
    std::uniform_real_distribution<float> param_dist(0, Preset::PARAMETER_COUNT - 1);
    
    // Mutate a few parameters
    size_t num_mutations = std::uniform_int_distribution<size_t>(1, 8)(rng_);
    std::vector<size_t> mutated_params;
    
    for (size_t i = 0; i < num_mutations; ++i) {
        size_t param_idx = param_dist(rng_);
        
        // Ensure we don't mutate the same parameter multiple times in one call
        if (std::find(mutated_params.begin(), mutated_params.end(), param_idx) != mutated_params.end()) {
            continue;
        }
        
        mutated_params.push_back(param_idx);
        float mutation = mutation_dist(rng_);
        mutated.parameters[param_idx] = std::clamp(
            original.parameters[param_idx] + mutation, 0.0f, 1.0f);
    }
    
    // Apply constraints to mutated parameters
    validatePreset(mutated);
    
    // Update similarity score
    mutated.similarity_score = calculateSimilarityScore(original, mutated);
    
    if (ai_manager_) {
        ai_manager_->recordEvent(AIManager::FeatureType::NeuralPreset, 
            "Preset mutated with " + std::to_string(num_mutations) + " changes");
    }
    
    return mutated;
}

void NeuralPresetGenerator::learnFromUserFeedback(const Preset& preset, float user_rating) {
    std::lock_guard<std::mutex> lock(data_mutex_);
    
    training_data_.push_back(preset);
    preset.user_rating = user_rating;
    
    // Update average rating
    float total_rating = stats_.average_rating * stats_.successful_generations + user_rating;
    stats_.successful_generations++;
    stats_.average_rating = total_rating / stats_.successful_generations;
    
    // Update quality weights based on user feedback
    if (user_rating > 0.7f) {
        // Positive feedback - increase weights for current parameters
        for (size_t i = 0; i < Preset::PARAMETER_COUNT; ++i) {
            quality_weights_[i] = std::min(2.0f, quality_weights_[i] * 1.01f);
        }
    } else if (user_rating < 0.3f) {
        // Negative feedback - decrease weights
        for (size_t i = 0; i < Preset::PARAMETER_COUNT; ++i) {
            quality_weights_[i] = std::max(0.1f, quality_weights_[i] * 0.99f);
        }
    }
    
    // Periodically retrain the model
    if (training_data_.size() % 100 == 0) {
        if (ai_manager_) {
            ai_manager_->submitJob(AIManager::FeatureType::NeuralPreset, 
                [this]() { retrainModel(training_data_); });
        }
    }
    
    if (ai_manager_) {
        ai_manager_->recordEvent(AIManager::FeatureType::NeuralPreset, 
            "Learned from user feedback: rating = " + std::to_string(user_rating));
    }
}

void NeuralPresetGenerator::retrainModel(const std::vector<Preset>& training_data) {
    if (training_data.empty()) return;
    
    // Simple training implementation
    for (size_t epoch = 0; epoch < config_.epochs; ++epoch) {
        for (const auto& preset : training_data) {
            // Create input from current user preferences and preset features
            std::vector<float> input = user_preferences_;
            input.insert(input.end(), preset.parameters.begin(), preset.parameters.end());
            
            // Forward pass
            std::vector<float> output = network_->forward(input);
            
            // Simple "training" by adjusting toward target
            network_->backward(preset.parameters);
        }
        
        network_->updateWeights(config_.learning_rate);
    }
    
    if (ai_manager_) {
        ai_manager_->recordEvent(AIManager::FeatureType::NeuralPreset, 
            "Model retrained with " + std::to_string(training_data.size()) + " samples");
    }
}

std::vector<float> NeuralPresetGenerator::analyzeAudioForPreset(const std::vector<float>& audio_data) {
    std::vector<float> features(16, 0.0f);
    
    // Simple feature extraction
    // In real implementation, use FFT and sophisticated audio analysis
    if (audio_data.empty()) return features;
    
    // Calculate basic statistics
    float mean = std::accumulate(audio_data.begin(), audio_data.end(), 0.0f) / audio_data.size();
    float variance = 0.0f;
    for (float val : audio_data) {
        variance += (val - mean) * (val - mean);
    }
    variance /= audio_data.size();
    
    features[0] = mean;
    features[1] = variance;
    features[2] = *std::max_element(audio_data.begin(), audio_data.end());
    features[3] = *std::min_element(audio_data.begin(), audio_data.end());
    
    // Generate synthetic features for demonstration
    std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    for (size_t i = 4; i < features.size(); ++i) {
        features[i] = dist(rng_);
    }
    
    return features;
}

float NeuralPresetGenerator::assessPresetQuality(const Preset& preset) {
    float quality_score = 0.0f;
    float total_weight = 0.0f;
    
    // Score based on parameter diversity
    float diversity = 0.0f;
    for (size_t i = 0; i < preset.parameters.size(); ++i) {
        for (size_t j = i + 1; j < preset.parameters.size(); ++j) {
            diversity += std::abs(preset.parameters[i] - preset.parameters[j]);
        }
    }
    diversity /= (preset.parameters.size() * (preset.parameters.size() - 1) / 2);
    
    quality_score += diversity * 0.3f;
    total_weight += 0.3f;
    
    // Score based on user preferences
    float preference_score = 0.0f;
    for (size_t i = 0; i < std::min(preset.parameters.size(), user_preferences_.size()); ++i) {
        preference_score += 1.0f - std::abs(preset.parameters[i] - user_preferences_[i]);
    }
    preference_score /= user_preferences_.size();
    
    quality_score += preference_score * 0.4f;
    total_weight += 0.4f;
    
    // Score based on parameter constraints
    float constraint_score = 1.0f;
    for (const auto& [min_val, max_val] : parameter_constraints_) {
        // Check if parameters are within soft constraints
        // This is simplified for demonstration
        constraint_score *= 0.95f;
    }
    
    quality_score += constraint_score * 0.3f;
    total_weight += 0.3f;
    
    return total_weight > 0 ? quality_score / total_weight : 0.5f;
}

std::vector<float> NeuralPresetGenerator::getPresetSimilarity(const Preset& a, const Preset& b) {
    std::vector<float> similarity(4, 0.0f);
    
    // Parameter similarity
    float param_sim = 0.0f;
    for (size_t i = 0; i < std::min(a.parameters.size(), b.parameters.size()); ++i) {
        param_sim += 1.0f - std::abs(a.parameters[i] - b.parameters[i]);
    }
    similarity[0] = param_sim / std::min(a.parameters.size(), b.parameters.size());
    
    // Audio feature similarity
    similarity[1] = calculateSimilarityScore(a, b);
    
    // User rating similarity (if available)
    similarity[2] = 1.0f - std::abs(a.user_rating - b.user_rating);
    
    // Genre tag similarity (simplified)
    similarity[3] = (a.genre_tags == b.genre_tags) ? 1.0f : 0.0f;
    
    return similarity;
}

void NeuralPresetGenerator::resetStats() {
    std::lock_guard<std::mutex> lock(data_mutex_);
    stats_ = GenerationStats{};
}

void NeuralPresetGenerator::addParameterConstraint(size_t param_index, float min_val, float max_val) {
    if (param_index >= Preset::PARAMETER_COUNT) return;
    
    if (parameter_constraints_.size() <= param_index) {
        parameter_constraints_.resize(Preset::PARAMETER_COUNT, {0.0f, 1.0f});
    }
    
    parameter_constraints_[param_index] = {min_val, max_val};
}

void NeuralPresetGenerator::addSoftConstraint(size_t param_index, float preferred_val, float weight) {
    soft_constraints_.emplace_back(param_index, preferred_val, weight);
}

void NeuralPresetGenerator::validatePreset(Preset& preset) {
    // Apply hard constraints
    for (size_t i = 0; i < std::min(preset.parameters.size(), parameter_constraints_.size()); ++i) {
        const auto& [min_val, max_val] = parameter_constraints_[i];
        preset.parameters[i] = std::clamp(preset.parameters[i], min_val, max_val);
    }
    
    // Apply soft constraints
    for (const auto& [param_idx, preferred_val, weight] : soft_constraints_) {
        if (param_idx < preset.parameters.size()) {
            float current = preset.parameters[param_idx];
            float delta = preferred_val - current;
            preset.parameters[param_idx] = current + delta * weight * 0.1f;
            preset.parameters[param_idx] = std::clamp(preset.parameters[param_idx], 0.0f, 1.0f);
        }
    }
}

std::vector<NeuralPresetGenerator::Preset> NeuralPresetGenerator::generateWithConstraints(
    const std::vector<float>& conditions, const std::vector<size_t>& required_params) {
    
    std::vector<Preset> presets;
    
    // Generate multiple candidates
    for (size_t i = 0; i < 10; ++i) {
        Preset candidate = generatePreset(conditions);
        
        // Check if required parameters are reasonable
        bool is_valid = true;
        for (size_t param : required_params) {
            if (param >= candidate.parameters.size()) continue;
            
            // Simple validation - parameters should not be at extremes
            if (candidate.parameters[param] < 0.1f || candidate.parameters[param] > 0.9f) {
                is_valid = false;
                break;
            }
        }
        
        if (is_valid) {
            presets.push_back(candidate);
            if (presets.size() >= 3) break; // Return top 3 candidates
        }
    }
    
    return presets;
}

// Helper methods
std::vector<float> NeuralPresetGenerator::encodeCondition(const std::vector<float>& conditions) {
    std::vector<float> encoded(32, 0.0f);
    
    // Copy conditions
    size_t copy_size = std::min(conditions.size(), encoded.size() - 16);
    for (size_t i = 0; i < copy_size; ++i) {
        encoded[i] = conditions[i];
    }
    
    // Add user preferences
    size_t pref_size = std::min(user_preferences_.size(), encoded.size() - copy_size);
    for (size_t i = 0; i < pref_size; ++i) {
        encoded[copy_size + i] = user_preferences_[i];
    }
    
    return encoded;
}

std::vector<float> NeuralPresetGenerator::decodePreset(const std::vector<float>& network_output) {
    std::vector<float> parameters(Preset::PARAMETER_COUNT);
    
    // Copy and normalize output
    size_t copy_size = std::min(network_output.size(), parameters.size());
    for (size_t i = 0; i < copy_size; ++i) {
        parameters[i] = std::clamp(network_output[i], 0.0f, 1.0f);
    }
    
    // Fill remaining parameters with default values
    for (size_t i = copy_size; i < parameters.size(); ++i) {
        parameters[i] = 0.5f;
    }
    
    return parameters;
}

void NeuralPresetGenerator::normalizeParameters(std::vector<float>& parameters) {
    // Implementation for parameter normalization
    for (auto& param : parameters) {
        param = std::clamp(param, 0.0f, 1.0f);
    }
}

void NeuralPresetGenerator::denormalizeParameters(std::vector<float>& parameters) {
    // Implementation for parameter denormalization
    // In real implementation, apply appropriate scaling and offset
}

float NeuralPresetGenerator::calculateSimilarityScore(const Preset& a, const Preset& b) {
    if (a.parameters.size() != b.parameters.size()) return 0.0f;
    
    float sum_squared_diff = 0.0f;
    for (size_t i = 0; i < a.parameters.size(); ++i) {
        float diff = a.parameters[i] - b.parameters[i];
        sum_squared_diff += diff * diff;
    }
    
    float mean_squared_diff = sum_squared_diff / a.parameters.size();
    return 1.0f - std::sqrt(mean_squared_diff); // Convert to similarity (1 - distance)
}

float NeuralPresetGenerator::calculateGenreMatch(const Preset& preset, const std::string& target_genre) {
    // Simple genre matching implementation
    if (preset.genre_tags.empty() || target_genre.empty()) return 0.5f;
    
    return (preset.genre_tags.find(target_genre) != std::string::npos) ? 1.0f : 0.0f;
}

float NeuralPresetGenerator::calculateUserPreferenceScore(const Preset& preset) {
    if (user_preferences_.empty()) return 0.5f;
    
    float score = 0.0f;
    size_t count = std::min(preset.parameters.size(), user_preferences_.size());
    
    for (size_t i = 0; i < count; ++i) {
        score += 1.0f - std::abs(preset.parameters[i] - user_preferences_[i]);
    }
    
    return score / count;
}

} // namespace vital