#pragma once

#include <vector>
#include <array>
#include <map>
#include <memory>
#include <functional>
#include <random>
#include <Eigen/Dense>
#include <atomic>
#include <mutex>
#include <chrono>

namespace vital {

/**
 * @class MachineLearningEngine
 * @brief Core machine learning engine for Vital AI features
 * 
 * Provides unified interface for various ML algorithms including neural networks,
 * clustering, regression, and reinforcement learning for musical AI applications.
 */
class MachineLearningEngine {
public:
    // Model types
    enum class ModelType {
        NeuralNetwork,
        KMeansClustering,
        LinearRegression,
        DecisionTree,
        ReinforcementLearning,
        RecommendationSystem
    };
    
    // Learning types
    enum class LearningType {
        Supervised,
        Unsupervised,
        Reinforcement,
        Online,
        Transfer
    };
    
    // Data types
    using DataPoint = std::vector<float>;
    using DataSet = std::vector<DataPoint>;
    using Labels = std::vector<float>;
    
    // Model interface
    class MLModel {
    public:
        virtual ~MLModel() = default;
        virtual bool train(const DataSet& data, const Labels& labels) = 0;
        virtual std::vector<float> predict(const DataPoint& input) = 0;
        virtual float predictSingle(const DataPoint& input) = 0;
        virtual bool save(const std::string& path) = 0;
        virtual bool load(const std::string& path) = 0;
        virtual void reset() = 0;
        virtual float getAccuracy() const = 0;
        virtual size_t getInputSize() const = 0;
        virtual size_t getOutputSize() const = 0;
    };
    
    // Neural Network implementation
    class NeuralNetwork : public MLModel {
    public:
        NeuralNetwork();
        explicit NeuralNetwork(const std::vector<size_t>& layer_sizes);
        
        // MLModel interface
        bool train(const DataSet& data, const Labels& labels) override;
        std::vector<float> predict(const DataPoint& input) override;
        float predictSingle(const DataPoint& input) override;
        bool save(const std::string& path) override;
        bool load(const std::string& path) override;
        void reset() override;
        float getAccuracy() const override;
        size_t getInputSize() const override { return layer_sizes_.front(); }
        size_t getOutputSize() const override { return layer_sizes_.back(); }
        
        // Configuration
        void setLearningRate(float rate);
        void setActivationFunction(const std::string& activation);
        void setDropout(float dropout_rate);
        void enableBatchNormalization(bool enable);
        void setRegularization(float l2_lambda);
        
        // Training configuration
        void setEpochs(size_t epochs);
        void setBatchSize(size_t batch_size);
        void setValidationSplit(float split);
        
    private:
        struct Layer {
            Eigen::MatrixXf weights;
            Eigen::VectorXf bias;
            Eigen::MatrixXf weight_gradient;
            Eigen::VectorXf bias_gradient;
            Eigen::MatrixXf batch_norm_scale;
            Eigen::MatrixXf batch_norm_shift;
            bool use_batch_norm;
        };
        
        std::vector<Layer> layers_;
        std::vector<size_t> layer_sizes_;
        float learning_rate_;
        std::string activation_function_;
        float dropout_rate_;
        float l2_lambda_;
        size_t epochs_;
        size_t batch_size_;
        float validation_split_;
        
        // Activation functions
        Eigen::VectorXf activate(const Eigen::VectorXf& input) const;
        Eigen::VectorXf activateDerivative(const Eigen::VectorXf& input) const;
        
        // Forward and backward propagation
        std::vector<Eigen::VectorXf> forward(const DataPoint& input);
        void backward(const std::vector<Eigen::VectorXf>& activations,
                     const DataPoint& target);
        void updateWeights(float learning_rate);
        
        // Utility methods
        void initializeWeights();
        void normalizeData(DataSet& data);
        void createMiniBatches(const DataSet& data, const Labels& labels,
                              std::vector<std::pair<DataSet, Labels>>& batches);
    };
    
    // K-Means Clustering implementation
    class KMeansClustering : public MLModel {
    public:
        KMeansClustering(size_t num_clusters);
        
        // MLModel interface
        bool train(const DataSet& data, const Labels& labels) override;
        std::vector<float> predict(const DataPoint& input) override;
        float predictSingle(const DataPoint& input) override;
        bool save(const std::string& path) override;
        bool load(const std::string& path) override;
        void reset() override;
        float getAccuracy() const override;
        size_t getInputSize() const override { return num_features_; }
        size_t getOutputSize() const override { return num_clusters_; }
        
        // Clustering methods
        void setMaxIterations(size_t iterations);
        void setTolerance(float tolerance);
        std::vector<size_t> predictClusters(const DataSet& data);
        std::vector<size_t> getClusterAssignments() const;
        std::vector<DataPoint> getClusterCenters() const;
        
    private:
        size_t num_clusters_;
        size_t num_features_;
        std::vector<DataPoint> centroids_;
        std::vector<size_t> assignments_;
        size_t max_iterations_;
        float tolerance_;
        
        void initializeCentroids(const DataSet& data);
        void updateAssignments(const DataSet& data);
        void updateCentroids(const DataSet& data);
        float calculateDistance(const DataPoint& a, const DataPoint& b) const;
    };
    
    // Recommendation System implementation
    class RecommendationSystem : public MLModel {
    public:
        RecommendationSystem();
        
        // MLModel interface
        bool train(const DataSet& data, const Labels& labels) override;
        std::vector<float> predict(const DataPoint& input) override;
        float predictSingle(const DataPoint& input) override;
        bool save(const std::string& path) override;
        bool load(const std::string& path) override;
        void reset() override;
        float getAccuracy() const override;
        size_t getInputSize() const override;
        size_t getOutputSize() const override;
        
        // Recommendation methods
        void addUserPreference(const std::string& user_id, const std::string& item_id, float rating);
        std::vector<std::string> recommendItems(const std::string& user_id, size_t num_recommendations);
        std::vector<std::string> recommendUsers(const std::string& item_id, size_t num_recommendations);
        void updateWithFeedback(const std::string& user_id, const std::string& item_id, float rating);
        
        // User profiling
        std::vector<float> getUserProfile(const std::string& user_id) const;
        std::vector<float> getItemProfile(const std::string& item_id) const;
        
    private:
        std::map<std::string, std::map<std::string, float>> user_item_ratings_;
        std::map<std::string, std::vector<float>> user_features_;
        std::map<std::string, std::vector<float>> item_features_;
        std::map<std::string, std::string> user_item_mapping_;
        
        void calculateUserSimilarity(const std::string& user1, const std::string& user2);
        void calculateItemSimilarity(const std::string& item1, const std::string& item2);
        float predictRating(const std::string& user_id, const std::string& item_id);
    };
    
    // Main engine interface
    MachineLearningEngine();
    ~MachineLearningEngine();
    
    // Model management
    std::shared_ptr<MLModel> createModel(ModelType type, const std::string& name);
    std::shared_ptr<MLModel> getModel(const std::string& name);
    bool removeModel(const std::string& name);
    std::vector<std::string> getModelNames() const;
    void setDefaultModel(const std::string& name);
    
    // Data management
    bool loadData(const std::string& file_path, DataSet& data, Labels& labels);
    bool saveData(const std::string& file_path, const DataSet& data, const Labels& labels);
    void normalizeData(DataSet& data, const std::string& method = "minmax");
    void shuffleData(DataSet& data, Labels& labels);
    DataSet splitData(const DataSet& data, float train_ratio = 0.8f);
    
    // Cross-validation
    struct CrossValidationResult {
        float mean_accuracy;
        float std_accuracy;
        std::vector<float> fold_accuracies;
        float mean_squared_error;
        float r_squared;
    };
    
    CrossValidationResult crossValidate(std::shared_ptr<MLModel> model, 
                                      const DataSet& data, const Labels& labels,
                                      size_t num_folds = 5);
    
    // Model comparison
    struct ModelComparison {
        std::string model_name;
        float accuracy;
        float training_time_ms;
        float inference_time_ms;
        size_t model_size_bytes;
        float memory_usage_mb;
    };
    
    std::vector<ModelComparison> compareModels(const std::vector<std::string>& model_names,
                                              const DataSet& test_data, const Labels& test_labels);
    
    // Feature selection
    std::vector<size_t> selectBestFeatures(const DataSet& data, const Labels& labels, 
                                          size_t num_features);
    std::vector<float> calculateFeatureImportance(const DataSet& data, const Labels& labels);
    
    // Hyperparameter optimization
    struct HyperParameter {
        std::string name;
        std::vector<float> values;
    };
    
    struct OptimizationResult {
        std::map<std::string, float> best_parameters;
        float best_score;
        std::vector<std::pair<std::map<std::string, float>, float>> history;
    };
    
    OptimizationResult optimizeHyperParameters(std::shared_ptr<MLModel> model,
                                              const DataSet& data, const Labels& labels,
                                              const std::vector<HyperParameter>& parameters,
                                              size_t max_iterations = 100);
    
    // Statistics and monitoring
    struct EngineStats {
        size_t total_models_created = 0;
        size_t total_training_sessions = 0;
        size_t total_predictions = 0;
        float total_training_time_ms = 0.0f;
        float total_inference_time_ms = 0.0f;
        size_t active_models = 0;
        std::map<ModelType, size_t> model_type_usage;
    };
    
    EngineStats getStats() const { return stats_; }
    void resetStats();
    
    // Performance optimization
    void enableGPUAcceleration(bool enable);
    void setMemoryLimit(size_t max_memory_mb);
    void setCPUThrottling(float percentage);
    void enableParallelProcessing(bool enable, size_t num_threads = 0);
    
    // Real-time learning
    void enableOnlineLearning(bool enable);
    void setOnlineLearningRate(float rate);
    
private:
    // Model storage
    mutable std::mutex models_mutex_;
    std::map<std::string, std::shared_ptr<MLModel>> models_;
    std::string default_model_;
    
    // Performance tracking
    mutable std::mutex stats_mutex_;
    EngineStats stats_;
    
    // Configuration
    bool use_gpu_;
    size_t memory_limit_mb_;
    float cpu_throttling_;
    bool parallel_processing_;
    size_t num_threads_;
    bool online_learning_;
    float online_learning_rate_;
    
    // Utility methods
    void updateStats(const std::string& operation, float time_ms);
    bool checkMemoryUsage();
    void cleanInactiveModels();
    
    // Validation methods
    bool validateModelInput(const std::shared_ptr<MLModel>& model, const DataPoint& input);
    bool validateTrainingData(const DataSet& data, const Labels& labels);
};

// Utility functions for data preprocessing
namespace ml_utils {
    std::vector<float> normalize(const std::vector<float>& data, const std::string& method = "minmax");
    std::vector<float> standardize(const std::vector<float>& data);
    std::vector<float> pcaTransform(const std::vector<std::vector<float>>& data, size_t num_components);
    float calculateEntropy(const std::vector<float>& labels);
    float calculateInformationGain(const std::vector<float>& labels, const std::vector<size_t>& feature_values);
    std::vector<float> calculateCorrelation(const std::vector<std::vector<float>>& features, const std::vector<float>& target);
}

} // namespace vital