#include "machine_learning_engine.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <fstream>
#include <sstream>
#include <random>
#include <future>
#include <chrono>

namespace vital {

// NeuralNetwork implementation
MachineLearningEngine::NeuralNetwork::NeuralNetwork() 
    : learning_rate_(0.001f), activation_function_("relu"), dropout_rate_(0.0f),
      l2_lambda_(0.0f), epochs_(100), batch_size_(32), validation_split_(0.2f) {
}

MachineLearningEngine::NeuralNetwork::NeuralNetwork(const std::vector<size_t>& layer_sizes)
    : NeuralNetwork() {
    layer_sizes_ = layer_sizes;
    initializeWeights();
}

bool MachineLearningEngine::NeuralNetwork::train(const DataSet& data, const Labels& labels) {
    if (!validateTrainingData(data, labels)) return false;
    
    if (data.size() != labels.size()) return false;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    try {
        // Normalize data
        DataSet normalized_data = data;
        normalizeData(normalized_data);
        
        // Create mini-batches
        std::vector<std::pair<DataSet, Labels>> batches;
        createMiniBatches(normalized_data, labels, batches);
        
        // Training loop
        for (size_t epoch = 0; epoch < epochs_; ++epoch) {
            float total_loss = 0.0f;
            
            for (const auto& batch : batches) {
                float batch_loss = 0.0f;
                
                for (size_t i = 0; i < batch.first.size(); ++i) {
                    // Forward pass
                    auto activations = forward(batch.first[i]);
                    
                    // Calculate loss (MSE for regression)
                    float predicted = activations.back()[0];
                    float target = batch.second[i];
                    float loss = (predicted - target) * (predicted - target);
                    batch_loss += loss;
                    
                    // Backward pass
                    backward(activations, {target});
                }
                
                batch_loss /= batch.first.size();
                total_loss += batch_loss;
                
                // Update weights
                updateWeights(learning_rate_);
            }
            
            total_loss /= batches.size();
            
            // Early stopping check
            if (epoch > 10 && total_loss > previous_loss_ * 1.01f) {
                break; // Stop if loss is increasing
            }
            previous_loss_ = total_loss;
        }
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
        
        // Update accuracy
        calculateAccuracy(normalized_data, labels);
        
        return true;
    }
    catch (const std::exception& e) {
        return false;
    }
}

std::vector<float> MachineLearningEngine::NeuralNetwork::predict(const DataPoint& input) {
    if (input.size() != getInputSize()) {
        return std::vector<float>();
    }
    
    auto activations = forward(input);
    std::vector<float> output(activations.back().size());
    
    for (size_t i = 0; i < output.size(); ++i) {
        output[i] = activations.back()[i];
    }
    
    return output;
}

float MachineLearningEngine::NeuralNetwork::predictSingle(const DataPoint& input) {
    auto predictions = predict(input);
    return predictions.empty() ? 0.0f : predictions[0];
}

void MachineLearningEngine::NeuralNetwork::setLearningRate(float rate) {
    learning_rate_ = std::clamp(rate, 0.0001f, 1.0f);
}

void MachineLearningEngine::NeuralNetwork::setActivationFunction(const std::string& activation) {
    if (activation == "relu" || activation == "tanh" || activation == "sigmoid" || activation == "linear") {
        activation_function_ = activation;
    }
}

void MachineLearningEngine::NeuralNetwork::initializeWeights() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-0.1f, 0.1f);
    
    layers_.clear();
    
    for (size_t i = 0; i < layer_sizes_.size() - 1; ++i) {
        Layer layer;
        layer.weights = Eigen::MatrixXf(layer_sizes_[i + 1], layer_sizes_[i]);
        layer.bias = Eigen::VectorXf(layer_sizes_[i + 1]);
        layer.weight_gradient = Eigen::MatrixXf(layer_sizes_[i + 1], layer_sizes_[i]);
        layer.bias_gradient = Eigen::VectorXf(layer_sizes_[i + 1]);
        
        // Xavier initialization
        float fan_in = layer_sizes_[i];
        float fan_out = layer_sizes_[i + 1];
        float std_dev = std::sqrt(2.0f / (fan_in + fan_out));
        
        for (size_t r = 0; r < layer.weights.rows(); ++r) {
            for (size_t c = 0; c < layer.weights.cols(); ++c) {
                layer.weights(r, c) = dist(gen) * std_dev;
            }
        }
        
        for (size_t r = 0; r < layer.bias.size(); ++r) {
            layer.bias(r) = 0.0f;
        }
        
        layer.use_batch_norm = false;
        layers_.push_back(std::move(layer));
    }
}

std::vector<Eigen::VectorXf> MachineLearningEngine::NeuralNetwork::forward(const DataPoint& input) {
    std::vector<Eigen::VectorXf> activations;
    
    // Input layer
    Eigen::VectorXf current(input.data(), input.size());
    activations.push_back(current);
    
    // Hidden layers
    for (const auto& layer : layers_) {
        Eigen::VectorXf next = layer.weights * current + layer.bias;
        
        // Apply activation function
        if (activation_function_ == "relu") {
            next = next.cwiseMax(0.0f);
        } else if (activation_function_ == "tanh") {
            next = next.tanh();
        } else if (activation_function_ == "sigmoid") {
            next = 1.0f / (1.0f + (-next).array().exp());
        }
        // Linear activation for output layer
        
        activations.push_back(next);
        current = next;
    }
    
    return activations;
}

void MachineLearningEngine::NeuralNetwork::backward(const std::vector<Eigen::VectorXf>& activations,
                                                  const DataPoint& target) {
    // Simplified backpropagation
    // In real implementation, would implement full backpropagation
    
    for (auto& layer : layers_) {
        // Simple gradient approximation
        for (auto& w : layer.weight_gradient.data()) {
            w = (std::rand() % 100 - 50) * 0.0001f; // Random gradient for demo
        }
        
        for (auto& b : layer.bias_gradient.data()) {
            b = (std::rand() % 100 - 50) * 0.0001f;
        }
    }
}

void MachineLearningEngine::NeuralNetwork::updateWeights(float learning_rate) {
    for (auto& layer : layers_) {
        layer.weights -= learning_rate * layer.weight_gradient;
        layer.bias -= learning_rate * layer.bias_gradient;
    }
}

bool MachineLearningEngine::NeuralNetwork::save(const std::string& path) {
    try {
        std::ofstream file(path);
        if (!file.is_open()) return false;
        
        file << "neural_network_v1\n";
        file << layer_sizes_.size() << "\n";
        for (size_t size : layer_sizes_) {
            file << size << " ";
        }
        file << "\n";
        file << learning_rate_ << "\n";
        file << activation_function_ << "\n";
        
        for (const auto& layer : layers_) {
            file << layer.weights.rows() << " " << layer.weights.cols() << "\n";
            for (size_t r = 0; r < layer.weights.rows(); ++r) {
                for (size_t c = 0; c < layer.weights.cols(); ++c) {
                    file << layer.weights(r, c) << " ";
                }
                file << "\n";
            }
            
            for (size_t i = 0; i < layer.bias.size(); ++i) {
                file << layer.bias(i) << " ";
            }
            file << "\n";
        }
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool MachineLearningEngine::NeuralNetwork::load(const std::string& path) {
    try {
        std::ifstream file(path);
        if (!file.is_open()) return false;
        
        std::string version;
        std::getline(file, version);
        if (version != "neural_network_v1") return false;
        
        size_t num_layers;
        file >> num_layers;
        layer_sizes_.resize(num_layers);
        
        for (size_t i = 0; i < num_layers; ++i) {
            file >> layer_sizes_[i];
        }
        
        file >> learning_rate_;
        file >> activation_function_;
        
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

void MachineLearningEngine::NeuralNetwork::reset() {
    initializeWeights();
}

float MachineLearningEngine::NeuralNetwork::getAccuracy() const {
    return accuracy_;
}

void MachineLearningEngine::NeuralNetwork::createMiniBatches(const DataSet& data, const Labels& labels,
                                                          std::vector<std::pair<DataSet, Labels>>& batches) {
    size_t num_batches = (data.size() + batch_size_ - 1) / batch_size_;
    
    batches.clear();
    batches.reserve(num_batches);
    
    for (size_t i = 0; i < num_batches; ++i) {
        size_t start = i * batch_size_;
        size_t end = std::min(start + batch_size_, data.size());
        
        DataSet batch_data(data.begin() + start, data.begin() + end);
        Labels batch_labels(labels.begin() + start, labels.begin() + end);
        
        batches.emplace_back(std::move(batch_data), std::move(batch_labels));
    }
}

void MachineLearningEngine::NeuralNetwork::normalizeData(DataSet& data) {
    if (data.empty()) return;
    
    size_t num_features = data[0].size();
    std::vector<float> mins(num_features, std::numeric_limits<float>::max());
    std::vector<float> maxs(num_features, std::numeric_limits<float>::lowest());
    
    // Find min and max for each feature
    for (const auto& sample : data) {
        for (size_t i = 0; i < std::min(sample.size(), num_features); ++i) {
            mins[i] = std::min(mins[i], sample[i]);
            maxs[i] = std::max(maxs[i], sample[i]);
        }
    }
    
    // Normalize
    for (auto& sample : data) {
        for (size_t i = 0; i < std::min(sample.size(), num_features); ++i) {
            float range = maxs[i] - mins[i];
            if (range > 0) {
                sample[i] = (sample[i] - mins[i]) / range;
            }
        }
    }
}

void MachineLearningEngine::NeuralNetwork::calculateAccuracy(const DataSet& data, const Labels& labels) {
    if (data.empty()) {
        accuracy_ = 0.0f;
        return;
    }
    
    float total_error = 0.0f;
    for (size_t i = 0; i < data.size(); ++i) {
        float predicted = predictSingle(data[i]);
        float error = std::abs(predicted - labels[i]);
        total_error += error;
    }
    
    float mean_error = total_error / data.size();
    accuracy_ = std::max(0.0f, 1.0f - mean_error);
}

// KMeansClustering implementation
MachineLearningEngine::KMeansClustering::KMeansClustering(size_t num_clusters)
    : num_clusters_(num_clusters), max_iterations_(100), tolerance_(0.001f) {}

bool MachineLearningEngine::KMeansClustering::train(const DataSet& data, const Labels& labels) {
    if (data.empty()) return false;
    
    num_features_ = data[0].size();
    centroids_.clear();
    assignments_.clear();
    
    // Initialize centroids
    initializeCentroids(data);
    
    // K-means iterations
    for (size_t iter = 0; iter < max_iterations_; ++iter) {
        auto old_centroids = centroids_;
        
        // Update assignments
        updateAssignments(data);
        
        // Update centroids
        updateCentroids(data);
        
        // Check for convergence
        bool converged = true;
        for (size_t i = 0; i < centroids_.size(); ++i) {
            float distance = calculateDistance(centroids_[i], old_centroids[i]);
            if (distance > tolerance_) {
                converged = false;
                break;
            }
        }
        
        if (converged) break;
    }
    
    return true;
}

std::vector<float> MachineLearningEngine::KMeansClustering::predict(const DataPoint& input) {
    std::vector<float> probabilities(num_clusters_, 0.0f);
    
    if (centroids_.empty()) return probabilities;
    
    // Find nearest centroid
    size_t nearest = 0;
    float min_distance = std::numeric_limits<float>::max();
    
    for (size_t i = 0; i < centroids_.size(); ++i) {
        float distance = calculateDistance(input, centroids_[i]);
        if (distance < min_distance) {
            min_distance = distance;
            nearest = i;
        }
    }
    
    // Set probability of 1.0 for nearest cluster, 0.0 for others
    probabilities[nearest] = 1.0f;
    
    return probabilities;
}

float MachineLearningEngine::KMeansClustering::predictSingle(const DataPoint& input) {
    auto probabilities = predict(input);
    if (probabilities.empty()) return 0.0f;
    
    // Return index of highest probability
    size_t max_index = 0;
    for (size_t i = 1; i < probabilities.size(); ++i) {
        if (probabilities[i] > probabilities[max_index]) {
            max_index = i;
        }
    }
    
    return static_cast<float>(max_index);
}

void MachineLearningEngine::KMeansClustering::initializeCentroids(const DataSet& data) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<size_t> dist(0, data.size() - 1);
    
    centroids_.clear();
    
    for (size_t i = 0; i < num_clusters_; ++i) {
        size_t random_index = dist(gen);
        centroids_.push_back(data[random_index]);
    }
}

void MachineLearningEngine::KMeansClustering::updateAssignments(const DataSet& data) {
    assignments_.clear();
    assignments_.reserve(data.size());
    
    for (const auto& sample : data) {
        size_t nearest = 0;
        float min_distance = std::numeric_limits<float>::max();
        
        for (size_t i = 0; i < centroids_.size(); ++i) {
            float distance = calculateDistance(sample, centroids_[i]);
            if (distance < min_distance) {
                min_distance = distance;
                nearest = i;
            }
        }
        
        assignments_.push_back(nearest);
    }
}

void MachineLearningEngine::KMeansClustering::updateCentroids(const DataSet& data) {
    std::vector<std::vector<float>> sums(num_clusters_, std::vector<float>(num_features_, 0.0f));
    std::vector<size_t> counts(num_clusters_, 0);
    
    for (size_t i = 0; i < data.size(); ++i) {
        size_t cluster = assignments_[i];
        counts[cluster]++;
        
        for (size_t j = 0; j < num_features_; ++j) {
            sums[cluster][j] += data[i][j];
        }
    }
    
    // Update centroids
    for (size_t i = 0; i < num_clusters_; ++i) {
        if (counts[i] > 0) {
            for (size_t j = 0; j < num_features_; ++j) {
                centroids_[i][j] = sums[i][j] / counts[i];
            }
        }
    }
}

float MachineLearningEngine::KMeansClustering::calculateDistance(const DataPoint& a, const DataPoint& b) const {
    float sum = 0.0f;
    for (size_t i = 0; i < std::min(a.size(), b.size()); ++i) {
        float diff = a[i] - b[i];
        sum += diff * diff;
    }
    return std::sqrt(sum);
}

bool MachineLearningEngine::KMeansClustering::save(const std::string& path) {
    try {
        std::ofstream file(path);
        if (!file.is_open()) return false;
        
        file << "kmeans_clustering_v1\n";
        file << num_clusters_ << "\n";
        file << num_features_ << "\n";
        
        for (const auto& centroid : centroids_) {
            for (float val : centroid) {
                file << val << " ";
            }
            file << "\n";
        }
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool MachineLearningEngine::KMeansClustering::load(const std::string& path) {
    try {
        std::ifstream file(path);
        if (!file.is_open()) return false;
        
        std::string version;
        std::getline(file, version);
        if (version != "kmeans_clustering_v1") return false;
        
        file >> num_clusters_;
        file >> num_features_;
        
        centroids_.clear();
        centroids_.reserve(num_clusters_);
        
        for (size_t i = 0; i < num_clusters_; ++i) {
            DataPoint centroid(num_features_);
            for (size_t j = 0; j < num_features_; ++j) {
                file >> centroid[j];
            }
            centroids_.push_back(std::move(centroid));
        }
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

void MachineLearningEngine::KMeansClustering::reset() {
    centroids_.clear();
    assignments_.clear();
}

float MachineLearningEngine::KMeansClustering::getAccuracy() const {
    // For clustering, accuracy is based on cluster separation
    if (centroids_.empty()) return 0.0f;
    
    float total_variance = 0.0f;
    for (const auto& centroid : centroids_) {
        for (float val : centroid) {
            total_variance += val * val;
        }
    }
    
    return std::min(1.0f, total_variance / centroids_.size());
}

// RecommendationSystem implementation
MachineLearningEngine::RecommendationSystem::RecommendationSystem() {}

bool MachineLearningEngine::RecommendationSystem::train(const DataSet& data, const Labels& labels) {
    // Simplified training - in real implementation would use collaborative filtering
    // or matrix factorization
    
    if (data.size() != labels.size()) return false;
    
    // For demonstration, treat each sample as a user-item interaction
    for (size_t i = 0; i < data.size(); ++i) {
        std::string user_id = "user_" + std::to_string(i / 10);
        std::string item_id = "item_" + std::to_string(i % 10);
        user_item_ratings_[user_id][item_id] = labels[i];
    }
    
    return true;
}

std::vector<float> MachineLearningEngine::RecommendationSystem::predict(const DataPoint& input) {
    std::vector<float> output(1, 0.5f); // Default prediction
    
    if (input.size() >= 2) {
        // Simple prediction based on input features
        float sum = std::accumulate(input.begin(), input.end(), 0.0f);
        output[0] = sum / input.size();
    }
    
    return output;
}

float MachineLearningEngine::RecommendationSystem::predictSingle(const DataPoint& input) {
    auto predictions = predict(input);
    return predictions.empty() ? 0.5f : predictions[0];
}

bool MachineLearningEngine::RecommendationSystem::save(const std::string& path) {
    try {
        std::ofstream file(path);
        if (!file.is_open()) return false;
        
        file << "recommendation_system_v1\n";
        file << user_item_ratings_.size() << "\n";
        
        for (const auto& [user_id, ratings] : user_item_ratings_) {
            file << user_id << " " << ratings.size() << "\n";
            for (const auto& [item_id, rating] : ratings) {
                file << item_id << " " << rating << "\n";
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool MachineLearningEngine::RecommendationSystem::load(const std::string& path) {
    try {
        std::ifstream file(path);
        if (!file.is_open()) return false;
        
        std::string version;
        std::getline(file, version);
        if (version != "recommendation_system_v1") return false;
        
        size_t num_users;
        file >> num_users;
        
        user_item_ratings_.clear();
        
        for (size_t i = 0; i < num_users; ++i) {
            std::string user_id;
            size_t num_items;
            file >> user_id >> num_items;
            
            for (size_t j = 0; j < num_items; ++j) {
                std::string item_id;
                float rating;
                file >> item_id >> rating;
                user_item_ratings_[user_id][item_id] = rating;
            }
        }
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

void MachineLearningEngine::RecommendationSystem::reset() {
    user_item_ratings_.clear();
    user_features_.clear();
    item_features_.clear();
}

float MachineLearningEngine::RecommendationSystem::getAccuracy() const {
    // Simplified accuracy calculation
    if (user_item_ratings_.empty()) return 0.0f;
    
    float avg_rating = 0.0f;
    size_t count = 0;
    
    for (const auto& [user_id, ratings] : user_item_ratings_) {
        for (const auto& [item_id, rating] : ratings) {
            avg_rating += rating;
            count++;
        }
    }
    
    return count > 0 ? avg_rating / count : 0.0f;
}

size_t MachineLearningEngine::RecommendationSystem::getInputSize() const {
    return 10; // Default input size
}

size_t MachineLearningEngine::RecommendationSystem::getOutputSize() const {
    return 1; // Single prediction output
}

void MachineLearningEngine::RecommendationSystem::addUserPreference(const std::string& user_id, 
                                                                   const std::string& item_id, 
                                                                   float rating) {
    user_item_ratings_[user_id][item_id] = rating;
}

std::vector<std::string> MachineLearningEngine::RecommendationSystem::recommendItems(const std::string& user_id, 
                                                                                     size_t num_recommendations) {
    std::vector<std::string> recommendations;
    
    auto user_ratings = user_item_ratings_.find(user_id);
    if (user_ratings == user_item_ratings_.end()) return recommendations;
    
    // Simple recommendation based on highest rated items
    std::vector<std::pair<std::string, float>> item_ratings;
    for (const auto& [item_id, rating] : user_ratings->second) {
        item_ratings.push_back({item_id, rating});
    }
    
    std::sort(item_ratings.begin(), item_ratings.end(), 
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    for (size_t i = 0; i < std::min(num_recommendations, item_ratings.size()); ++i) {
        recommendations.push_back(item_ratings[i].first);
    }
    
    return recommendations;
}

// MachineLearningEngine implementation
MachineLearningEngine::MachineLearningEngine()
    : use_gpu_(false), memory_limit_mb_(1024), cpu_throttling_(100.0f),
      parallel_processing_(false), num_threads_(std::thread::hardware_concurrency()),
      online_learning_(false), online_learning_rate_(0.01f) {}

MachineLearningEngine::~MachineLearningEngine() {
    std::lock_guard<std::mutex> lock(models_mutex_);
    models_.clear();
}

std::shared_ptr<MachineLearningEngine::MLModel> MachineLearningEngine::createModel(ModelType type, const std::string& name) {
    std::shared_ptr<MLModel> model;
    
    switch (type) {
        case ModelType::NeuralNetwork:
            model = std::make_shared<NeuralNetwork>();
            break;
        case ModelType::KMeansClustering:
            model = std::make_shared<KMeansClustering>(3); // Default 3 clusters
            break;
        case ModelType::RecommendationSystem:
            model = std::make_shared<RecommendationSystem>();
            break;
        case ModelType::LinearRegression:
        case ModelType::DecisionTree:
        case ModelType::ReinforcementLearning:
            // Placeholder implementations
            model = std::make_shared<NeuralNetwork>();
            break;
    }
    
    if (model) {
        std::lock_guard<std::mutex> lock(models_mutex_);
        models_[name] = model;
        
        std::lock_guard<std::mutex> stats_lock(stats_mutex_);
        stats_.total_models_created++;
        stats_.active_models = models_.size();
        stats_.model_type_usage[type]++;
    }
    
    return model;
}

std::shared_ptr<MachineLearningEngine::MLModel> MachineLearningEngine::getModel(const std::string& name) {
    std::lock_guard<std::mutex> lock(models_mutex_);
    auto it = models_.find(name);
    return it != models_.end() ? it->second : nullptr;
}

bool MachineLearningEngine::removeModel(const std::string& name) {
    std::lock_guard<std::mutex> lock(models_mutex_);
    return models_.erase(name) > 0;
}

std::vector<std::string> MachineLearningEngine::getModelNames() const {
    std::lock_guard<std::mutex> lock(models_mutex_);
    std::vector<std::string> names;
    for (const auto& [name, _] : models_) {
        names.push_back(name);
    }
    return names;
}

void MachineLearningEngine::setDefaultModel(const std::string& name) {
    std::lock_guard<std::mutex> lock(models_mutex_);
    if (models_.find(name) != models_.end()) {
        default_model_ = name;
    }
}

// Data management
bool MachineLearningEngine::loadData(const std::string& file_path, DataSet& data, Labels& labels) {
    try {
        std::ifstream file(file_path);
        if (!file.is_open()) return false;
        
        data.clear();
        labels.clear();
        
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;
            
            std::istringstream iss(line);
            std::vector<float> sample;
            float value;
            
            while (iss >> value) {
                sample.push_back(value);
            }
            
            if (!sample.empty()) {
                data.push_back(sample);
            }
        }
        
        // Generate labels (for demonstration)
        labels.resize(data.size());
        for (size_t i = 0; i < labels.size(); ++i) {
            labels[i] = std::accumulate(data[i].begin(), data[i].end(), 0.0f) / data[i].size();
        }
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

bool MachineLearningEngine::saveData(const std::string& file_path, const DataSet& data, const Labels& labels) {
    try {
        std::ofstream file(file_path);
        if (!file.is_open()) return false;
        
        file << "# Data file generated by MachineLearningEngine\n";
        file << "# Format: feature1 feature2 ... featureN label\n\n";
        
        for (size_t i = 0; i < data.size(); ++i) {
            for (float value : data[i]) {
                file << value << " ";
            }
            if (i < labels.size()) {
                file << labels[i];
            }
            file << "\n";
        }
        
        return true;
    } catch (const std::exception& e) {
        return false;
    }
}

void MachineLearningEngine::normalizeData(DataSet& data, const std::string& method) {
    if (data.empty()) return;
    
    if (method == "minmax") {
        size_t num_features = data[0].size();
        std::vector<float> mins(num_features, std::numeric_limits<float>::max());
        std::vector<float> maxs(num_features, std::numeric_limits<float>::lowest());
        
        // Find min and max for each feature
        for (const auto& sample : data) {
            for (size_t i = 0; i < std::min(sample.size(), num_features); ++i) {
                mins[i] = std::min(mins[i], sample[i]);
                maxs[i] = std::max(maxs[i], sample[i]);
            }
        }
        
        // Normalize
        for (auto& sample : data) {
            for (size_t i = 0; i < std::min(sample.size(), num_features); ++i) {
                float range = maxs[i] - mins[i];
                if (range > 0) {
                    sample[i] = (sample[i] - mins[i]) / range;
                }
            }
        }
    } else if (method == "standard") {
        // Z-score normalization
        size_t num_features = data[0].size();
        std::vector<float> means(num_features, 0.0f);
        std::vector<float> stds(num_features, 0.0f);
        
        // Calculate means
        for (const auto& sample : data) {
            for (size_t i = 0; i < std::min(sample.size(), num_features); ++i) {
                means[i] += sample[i];
            }
        }
        
        for (size_t i = 0; i < num_features; ++i) {
            means[i] /= data.size();
        }
        
        // Calculate standard deviations
        for (const auto& sample : data) {
            for (size_t i = 0; i < std::min(sample.size(), num_features); ++i) {
                float diff = sample[i] - means[i];
                stds[i] += diff * diff;
            }
        }
        
        for (size_t i = 0; i < num_features; ++i) {
            stds[i] = std::sqrt(stds[i] / data.size());
        }
        
        // Normalize
        for (auto& sample : data) {
            for (size_t i = 0; i < std::min(sample.size(), num_features); ++i) {
                if (stds[i] > 0) {
                    sample[i] = (sample[i] - means[i]) / stds[i];
                }
            }
        }
    }
}

void MachineLearningEngine::shuffleData(DataSet& data, Labels& labels) {
    std::random_device rd;
    std::mt19937 gen(rd());
    
    size_t n = data.size();
    for (size_t i = n - 1; i > 0; --i) {
        std::uniform_int_distribution<size_t> dist(0, i);
        size_t j = dist(gen);
        
        if (j < labels.size()) {
            std::swap(data[i], data[j]);
            std::swap(labels[i], labels[j]);
        }
    }
}

// Feature selection
std::vector<size_t> MachineLearningEngine::selectBestFeatures(const DataSet& data, const Labels& labels, 
                                                             size_t num_features) {
    if (data.empty() || data[0].empty()) return {};
    
    size_t total_features = data[0].size();
    std::vector<float> importance = calculateFeatureImportance(data, labels);
    
    // Create feature importance pairs
    std::vector<std::pair<size_t, float>> feature_importance;
    for (size_t i = 0; i < total_features; ++i) {
        feature_importance.emplace_back(i, importance[i]);
    }
    
    // Sort by importance
    std::sort(feature_importance.begin(), feature_importance.end(), 
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    // Return top features
    std::vector<size_t> selected_features;
    for (size_t i = 0; i < std::min(num_features, feature_importance.size()); ++i) {
        selected_features.push_back(feature_importance[i].first);
    }
    
    return selected_features;
}

std::vector<float> MachineLearningEngine::calculateFeatureImportance(const DataSet& data, const Labels& labels) {
    if (data.empty() || labels.empty()) return {};
    
    size_t num_features = data[0].size();
    std::vector<float> importance(num_features, 0.0f);
    
    // Calculate correlation between each feature and target
    std::vector<float> target = labels;
    
    for (size_t i = 0; i < num_features; ++i) {
        std::vector<float> feature_values;
        for (const auto& sample : data) {
            if (i < sample.size()) {
                feature_values.push_back(sample[i]);
            }
        }
        
        // Simple correlation calculation
        if (feature_values.size() == target.size() && feature_values.size() > 1) {
            float mean_feature = std::accumulate(feature_values.begin(), feature_values.end(), 0.0f) / feature_values.size();
            float mean_target = std::accumulate(target.begin(), target.end(), 0.0f) / target.size();
            
            float numerator = 0.0f;
            float denom_feature = 0.0f;
            float denom_target = 0.0f;
            
            for (size_t j = 0; j < feature_values.size(); ++j) {
                float diff_feature = feature_values[j] - mean_feature;
                float diff_target = target[j] - mean_target;
                numerator += diff_feature * diff_target;
                denom_feature += diff_feature * diff_feature;
                denom_target += diff_target * diff_target;
            }
            
            float correlation = 0.0f;
            if (denom_feature > 0 && denom_target > 0) {
                correlation = numerator / (std::sqrt(denom_feature) * std::sqrt(denom_target));
            }
            
            importance[i] = std::abs(correlation);
        }
    }
    
    return importance;
}

// Statistics
void MachineLearningEngine::resetStats() {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    stats_ = EngineStats{};
}

// Performance optimization
void MachineLearningEngine::enableGPUAcceleration(bool enable) {
    use_gpu_ = enable;
}

void MachineLearningEngine::setMemoryLimit(size_t max_memory_mb) {
    memory_limit_mb_ = max_memory_mb;
}

void MachineLearningEngine::setCPUThrottling(float percentage) {
    cpu_throttling_ = std::clamp(percentage, 0.0f, 100.0f);
}

void MachineLearningEngine::enableParallelProcessing(bool enable, size_t num_threads) {
    parallel_processing_ = enable;
    num_threads_ = (num_threads == 0) ? std::thread::hardware_concurrency() : num_threads;
}

// ML utility functions
namespace ml_utils {

std::vector<float> normalize(const std::vector<float>& data, const std::string& method) {
    std::vector<float> normalized = data;
    
    if (method == "minmax") {
        float min_val = *std::min_element(normalized.begin(), normalized.end());
        float max_val = *std::max_element(normalized.begin(), normalized.end());
        float range = max_val - min_val;
        
        if (range > 0) {
            for (auto& val : normalized) {
                val = (val - min_val) / range;
            }
        }
    } else if (method == "standard") {
        float mean = std::accumulate(normalized.begin(), normalized.end(), 0.0f) / normalized.size();
        
        float variance = 0.0f;
        for (float val : normalized) {
            float diff = val - mean;
            variance += diff * diff;
        }
        variance /= normalized.size();
        
        float std_dev = std::sqrt(variance);
        if (std_dev > 0) {
            for (auto& val : normalized) {
                val = (val - mean) / std_dev;
            }
        }
    }
    
    return normalized;
}

float calculateEntropy(const std::vector<float>& labels) {
    if (labels.empty()) return 0.0f;
    
    std::map<float, size_t> counts;
    for (float label : labels) {
        counts[label]++;
    }
    
    float entropy = 0.0f;
    for (const auto& [value, count] : counts) {
        float probability = static_cast<float>(count) / labels.size();
        if (probability > 0) {
            entropy -= probability * std::log2(probability);
        }
    }
    
    return entropy;
}

} // namespace ml_utils

} // namespace vital