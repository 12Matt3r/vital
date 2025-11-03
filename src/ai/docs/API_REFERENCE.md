# Vital AI Integration System - API Reference

## Table of Contents
1. [AIManager](#aimanager)
2. [NeuralPresetGenerator](#neuralpresetgenerator)
3. [StyleTransferEngine](#styletransferengine)
4. [AdaptiveModulationSystem](#adaptivemodulationsystem)
5. [MachineLearningEngine](#machinelearningengine)
6. [IntelligentAudioAnalyzer](#intelligentaudioanalyzer)
7. [IntelligentPresetGenerator](#intelligentpresetgenerator)
8. [Core Data Structures](#core-data-structures)

---

## AIManager

Central AI integration manager that coordinates all AI features in the Vital synthesizer.

### Constructors

```cpp
AIManager();
```

### Methods

#### Initialization and Configuration

```cpp
bool initialize(int num_threads = std::thread::hardware_concurrency());
```
Initializes the AI manager with specified number of worker threads.

**Parameters:**
- `num_threads`: Number of worker threads (default: hardware concurrency)

**Returns:** `true` if initialization successful

```cpp
void shutdown();
```
Gracefully shuts down the AI manager and all worker threads.

#### Resource Management

```cpp
void setCpuUsageLimit(float percentage);
```
Sets CPU usage limit as percentage (0-100).

```cpp
void setMemoryLimit(size_t max_memory_mb);
```
Sets memory usage limit in megabytes.

```cpp
void enableFeature(FeatureType type, bool enable = true);
```
Enables or disables specific AI features.

**Parameters:**
- `type`: Feature type to enable/disable
- `enable`: Enable flag

```cpp
bool isFeatureEnabled(FeatureType type) const;
```
Checks if a specific feature is enabled.

#### Job Submission

```cpp
template<typename Func>
std::future<void> submitJob(FeatureType type, Func&& task, AIJob::Priority priority = AIJob::Priority::Normal);
```
Submits an AI task for asynchronous execution.

**Template Parameters:**
- `Func`: Callable function type

**Parameters:**
- `type`: Feature type for the job
- `task`: Function to execute
- `priority`: Job priority

**Returns:** Future object for the job result

#### Performance Monitoring

```cpp
PerformanceMetrics getMetrics() const;
```
Returns current performance metrics.

```cpp
void startProfiling();
void stopProfiling();
```
Controls performance profiling.

```cpp
std::string getProfilingReport() const;
```
Generates a performance profiling report.

#### Settings Persistence

```cpp
void saveSettings(const std::string& path);
bool loadSettings(const std::string& path);
```
Save/load AI manager settings to/from file.

### Types

```cpp
enum class FeatureType {
    NeuralPreset,
    StyleTransfer,
    AdaptiveModulation,
    MachineLearning,
    AudioAnalysis,
    UserLearning,
    IntelligentGeneration
};

struct PerformanceMetrics {
    std::atomic<uint64_t> total_processed;
    std::atomic<uint64_t> active_jobs;
    std::atomic<uint64_t> completed_jobs;
    std::atomic<uint64_t> failed_jobs;
    std::atomic<float> avg_processing_time_ms;
    std::atomic<float> cpu_usage_percent;
    std::atomic<uint64_t> memory_usage_mb;
};
```

---

## NeuralPresetGenerator

Deep learning-based intelligent preset generation system.

### Constructors

```cpp
NeuralPresetGenerator(AIManager* ai_manager);
```

### Methods

#### Configuration

```cpp
void setNetworkConfig(const NetworkConfig& config);
```
Sets neural network architecture and training parameters.

```cpp
void setGenerationMode(GenerationMode mode);
```
Sets preset generation mode.

**Parameters:**
- `mode`: Generation mode (Deterministic, Stochastic, Variational, Conditional)

```cpp
void setUserPreferences(const std::vector<float>& preferences);
```
Sets user parameter preferences for generation.

#### Preset Generation

```cpp
Preset generatePreset();
```
Generates a single preset with default settings.

```cpp
Preset generatePreset(const std::vector<float>& conditions);
```
Generates a preset with specific conditions.

```cpp
std::vector<Preset> generateBatch(size_t count);
```
Generates a batch of presets asynchronously.

```cpp
Preset mutatePreset(const Preset& original, float mutation_strength = 0.1f);
```
Creates a mutated version of an existing preset.

#### Learning System

```cpp
void learnFromUserFeedback(const Preset& preset, float user_rating);
```
Learns from user satisfaction ratings (0.0-1.0).

```cpp
void learnFromAudioAnalysis(const std::vector<float>& audio_features, const Preset& preset);
```
Learns from audio analysis results.

```cpp
void learnFromGenrePreference(const std::string& genre, const Preset& preset);
```
Learns genre-specific preferences.

#### Model Management

```cpp
bool loadModel(const std::string& model_path);
bool saveModel(const std::string& model_path);
```
Load/save trained neural network models.

```cpp
void retrainModel(const std::vector<Preset>& training_data);
```
Retrains the neural network with new data.

### Types

```cpp
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

enum class GenerationMode {
    Deterministic,  // Same input = same output
    Stochastic,     // Controlled randomness
    Variational,    // Variational autoencoder
    Conditional     // Condition-based generation
};

struct NetworkConfig {
    std::vector<size_t> layer_sizes;
    std::vector<std::string> activation_functions;
    float learning_rate;
    float dropout_rate;
    size_t batch_size;
    size_t epochs;
};
```

---

## StyleTransferEngine

Real-time audio style transfer and timbre modification system.

### Constructors

```cpp
StyleTransferEngine(AIManager* ai_manager);
```

### Methods

#### Configuration

```cpp
void setTransferConfig(const TransferConfig& config);
```
Sets style transfer processing parameters.

```cpp
void setProcessingParameters(size_t frame_size, size_t hop_size);
```
Sets audio processing frame and hop sizes.

#### Style Management

```cpp
bool loadStyle(const std::string& style_path);
bool saveStyle(const std::string& style_path, const Style& style);
```
Load/save style profiles to/from files.

```cpp
void addStyle(const Style& style);
void removeStyle(const std::string& style_name);
```
Add or remove style profiles from the library.

```cpp
Style getStyle(const std::string& style_name) const;
```
Retrieve a style profile by name.

```cpp
std::vector<std::string> getAvailableStyles() const;
```
Returns list of all available style names.

#### Style Transfer Processing

```cpp
std::vector<float> transferStyle(const std::vector<float>& input_audio, const std::string& target_style);
```
Transfers audio to target style.

```cpp
std::vector<float> transferStyle(const std::vector<float>& input_audio, const std::vector<float>& style_features);
```
Transfers audio using custom style features.

```cpp
std::vector<float> blendStyles(const std::vector<float>& input_audio, const std::vector<std::pair<std::string, float>>& style_weights);
```
Blends multiple styles together with weighted contributions.

#### Real-time Processing

```cpp
bool processBlock(const std::vector<float>& input_block, std::vector<float>& output_block);
```
Processes a single audio block for real-time style transfer.

```cpp
bool initializeRealTime(size_t sample_rate);
```
Initializes real-time processing with specific sample rate.

#### Quality Assessment

```cpp
QualityMetrics assessTransferQuality(const std::vector<float>& original, const std::vector<float>& processed);
```
Assesses quality of style transfer operation.

### Types

```cpp
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

struct QualityMetrics {
    float similarity_score;
    float preservation_score;
    float artifact_level;
    float spectral_smoothness;
};
```

---

## AdaptiveModulationSystem

AI-powered adaptive modulation system for intelligent parameter control.

### Constructors

```cpp
AdaptiveModulationSystem(AIManager* ai_manager);
```

### Methods

#### System Configuration

```cpp
void setLearningRate(float rate);
```
Sets learning rate for user adaptation (0.0-1.0).

```cpp
void setAdaptationSpeed(float speed);
```
Sets adaptation speed for real-time learning.

```cpp
void enableLearning(bool enable);
```
Enables/disables the learning system.

```cpp
void setUserExperienceLevel(float level);
```
Sets user experience level (0.0 = beginner, 1.0 = expert).

#### Modulation Management

```cpp
uint32_t createModulation(const std::string& target_parameter, ModulationType type);
```
Creates a new modulation for a target parameter.

```cpp
bool removeModulation(uint32_t modulation_id);
```
Removes a modulation by ID.

```cpp
bool updateModulation(uint32_t modulation_id, const Modulation& updated);
```
Updates modulation parameters.

```cpp
Modulation getModulation(uint32_t modulation_id) const;
```
Retrieves modulation by ID.

```cpp
std::vector<uint32_t> getModulationsForParameter(const std::string& parameter) const;
```
Returns all modulations affecting a parameter.

#### Real-time Processing

```cpp
float processModulation(uint32_t modulation_id, float current_value, 
                       const std::vector<float>& audio_input = {},
                       const std::vector<float>& midi_input = {});
```
Processes modulation for a single parameter value.

#### Pattern Management

```cpp
uint32_t createPattern(const std::string& name);
```
Creates a new modulation pattern.

```cpp
bool addModulationToPattern(uint32_t pattern_id, uint32_t modulation_id);
bool removeModulationFromPattern(uint32_t pattern_id, uint32_t modulation_id);
```
Adds/removes modulations to/from patterns.

```cpp
bool applyPattern(uint32_t pattern_id, const std::string& context);
```
Applies a modulation pattern with context.

#### Learning System

```cpp
void learnFromUserAction(const std::string& action, float satisfaction);
```
Learns from user actions and satisfaction ratings.

```cpp
void analyzeUserBehavior();
```
Analyzes stored user behavior data.

```cpp
void adaptToMusicalContext(const MusicalContext& context);
```
Adapts system to current musical context.

#### Intelligent Suggestions

```cpp
std::vector<std::pair<uint32_t, float>> suggestModulations(const std::string& parameter, const MusicalContext& context);
```
Suggests modulations for a parameter based on context.

```cpp
std::vector<uint32_t> suggestParameterModulations(const std::vector<std::string>& parameters, float satisfaction_threshold = 0.7f);
```
Suggests parameter modulations based on user history.

### Types

```cpp
enum class ModulationType {
    LFO,              // Low Frequency Oscillator
    Envelope,         // ADSR envelope
    StepSequencer,    // Step sequencer
    Random,           // Random modulation
    AudioReactive,    // Audio reactive modulation
    Learning,         // AI-learned modulation
    ContextAware      // Context-aware modulation
};

enum class ModulationSource {
    Internal,         // Internal LFO/envelope
    MIDI,             // MIDI CC/note data
    Audio,            // Audio analysis
    UserAction,       // User interaction
    AILearning,       // AI-generated
    Predictive        // Predictive modulation
};

struct Modulation {
    uint32_t id;
    std::string target_parameter;
    ModulationType type;
    ModulationSource source;
    float amount;
    float phase;
    float frequency;
    std::vector<float> curve_points;
    CurveType curve_type;
    bool enabled;
    float user_satisfaction;
    TimePoint last_used;
    
    // Type-specific parameters
    union {
        EnvelopeParams envelope;
        LFOParams lfo;
        RandomParams random;
    };
};

struct MusicalContext {
    std::string genre;
    std::string tempo_range;
    float complexity_level;
    std::vector<float> spectral_features;
    std::vector<float> temporal_features;
    float harmonic_content;
    float rhythmic_intensity;
};
```

---

## MachineLearningEngine

Core machine learning engine supporting multiple ML algorithms.

### Constructors

```cpp
MachineLearningEngine();
```

### Methods

#### Model Management

```cpp
std::shared_ptr<MLModel> createModel(ModelType type, const std::string& name);
```
Creates a new ML model of specified type.

```cpp
std::shared_ptr<MLModel> getModel(const std::string& name);
```
Retrieves a model by name.

```cpp
bool removeModel(const std::string& name);
```
Removes a model from the engine.

```cpp
std::vector<std::string> getModelNames() const;
```
Returns list of all model names.

```cpp
void setDefaultModel(const std::string& name);
```
Sets the default model for operations.

#### Data Management

```cpp
bool loadData(const std::string& file_path, DataSet& data, Labels& labels);
bool saveData(const std::string& file_path, const DataSet& data, const Labels& labels);
```
Load/save training data from/to files.

```cpp
void normalizeData(DataSet& data, const std::string& method = "minmax");
```
Normalizes data using specified method ("minmax" or "standard").

```cpp
void shuffleData(DataSet& data, Labels& labels);
```
Randomly shuffles data and labels.

#### Cross-validation

```cpp
CrossValidationResult crossValidate(std::shared_ptr<MLModel> model, 
                                   const DataSet& data, const Labels& labels,
                                   size_t num_folds = 5);
```
Performs k-fold cross-validation on a model.

#### Model Comparison

```cpp
std::vector<ModelComparison> compareModels(const std::vector<std::string>& model_names,
                                          const DataSet& test_data, const Labels& test_labels);
```
Compares multiple models on test data.

#### Feature Selection

```cpp
std::vector<size_t> selectBestFeatures(const DataSet& data, const Labels& labels, 
                                      size_t num_features);
```
Selects most important features using information gain.

```cpp
std::vector<float> calculateFeatureImportance(const DataSet& data, const Labels& labels);
```
Calculates feature importance scores.

### Types

```cpp
enum class ModelType {
    NeuralNetwork,
    KMeansClustering,
    LinearRegression,
    DecisionTree,
    ReinforcementLearning,
    RecommendationSystem
};

using DataPoint = std::vector<float>;
using DataSet = std::vector<DataPoint>;
using Labels = std::vector<float>;

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

struct CrossValidationResult {
    float mean_accuracy;
    float std_accuracy;
    std::vector<float> fold_accuracies;
    float mean_squared_error;
    float r_squared;
};

struct ModelComparison {
    std::string model_name;
    float accuracy;
    float training_time_ms;
    float inference_time_ms;
    size_t model_size_bytes;
    float memory_usage_mb;
};
```

---

## IntelligentAudioAnalyzer

AI-powered real-time audio analysis and learning system.

### Constructors

```cpp
IntelligentAudioAnalyzer(AIManager* ai_manager);
```

### Methods

#### Configuration

```cpp
void setAnalysisConfig(const AnalysisConfig& config);
```
Sets audio analysis parameters.

```cpp
void updateSampleRate(size_t sample_rate);
```
Updates sample rate for analysis.

#### Real-time Processing

```cpp
AudioFeatures analyzeAudio(const std::vector<float>& audio_block);
```
Analyzes a single audio block.

```cpp
AudioFeatures analyzeAudioInRealTime(const std::vector<float>& audio_block);
```
Analyzes audio with real-time performance tracking.

#### Batch Processing

```cpp
std::vector<AudioFeatures> analyzeBatch(const std::vector<std::vector<float>>& audio_blocks);
```
Analyzes multiple audio blocks in batch.

```cpp
AudioFeatures analyzeAudioFile(const std::string& file_path);
```
Analyzes a complete audio file.

#### Classification

```cpp
ClassificationResult classifyAudio(const AudioFeatures& features);
```
Classifies audio based on extracted features.

```cpp
ClassificationResult classifyAudioRealTime(const std::vector<float>& audio_block);
```
Performs real-time audio classification.

#### Pitch Detection

```cpp
float detectPitch(const std::vector<float>& audio_block);
```
Detects fundamental frequency in audio.

```cpp
float detectPitchAutocorrelation(const std::vector<float>& audio_block);
```
Detects pitch using autocorrelation method.

```cpp
float detectPitchYIN(const std::vector<float>& audio_block);
```
Detects pitch using YIN algorithm.

#### Feature Extraction

```cpp
void extractMFCC(const std::vector<std::complex<float>>& spectrum, std::vector<float>& mfcc);
```
Extracts Mel-frequency cepstral coefficients.

```cpp
void extractSpectralFeatures(const std::vector<std::complex<float>>& spectrum, AudioFeatures& features);
```
Extracts comprehensive spectral features.

```cpp
void extractTemporalFeatures(const std::vector<float>& audio_block, AudioFeatures& features);
```
Extracts temporal domain features.

#### Learning and Adaptation

```cpp
void learnFromUserFeedback(const AudioFeatures& features, AudioClass correct_class);
```
Learns from user feedback on classifications.

```cpp
void adaptToMusicalContext(const std::string& genre, const std::string& style);
```
Adapts to current musical context.

#### Real-time Monitoring

```cpp
void startRealTimeMonitoring();
void stopRealTimeMonitoring();
```
Controls real-time monitoring.

```cpp
RealTimeMetrics getRealTimeMetrics() const;
```
Returns current real-time processing metrics.

### Types

```cpp
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
};

struct ClassificationResult {
    AudioClass predicted_class;
    float confidence;
    std::vector<std::pair<AudioClass, float>> class_probabilities;
    std::string description;
    TimePoint timestamp;
};

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
```

---

## IntelligentPresetGenerator

Advanced AI-powered preset generation with user learning and musical intelligence.

### Constructors

```cpp
IntelligentPresetGenerator(AIManager* ai_manager);
```

### Methods

#### Configuration

```cpp
void setGenerationSettings(const GenerationSettings& settings);
```
Sets preset generation parameters.

#### User Profile Management

```cpp
void updateUserProfile(const UserMusicalProfile& profile);
```
Updates user musical profile.

```cpp
UserMusicalProfile getUserProfile() const;
```
Returns current user profile.

```cpp
void analyzeUserBehavior(const std::vector<std::vector<float>>& preset_history,
                        const std::vector<float>& satisfaction_ratings);
```
Analyzes user behavior patterns from preset usage history.

#### Preset Generation

```cpp
std::vector<IntelligentPreset> generatePresets(size_t count, const GenerationContext& context = {});
```
Generates multiple presets with specified context.

```cpp
IntelligentPreset generateSinglePreset(const GenerationContext& context = {});
```
Generates a single preset.

#### Advanced Generation Modes

```cpp
std::vector<IntelligentPreset> generateFromAudio(const std::vector<float>& audio_data,
                                                const std::string& style_name = "auto");
```
Generates presets based on audio analysis.

```cpp
std::vector<IntelligentPreset> generateFromDescription(const std::string& description,
                                                      size_t count = 5);
```
Generates presets from natural language description.

```cpp
std::vector<IntelligentPreset> generateRemixVariations(const std::vector<float>& base_parameters,
                                                      size_t count = 3,
                                                      float variation_strength = 0.3f);
```
Creates remix variations of existing presets.

```cpp
std::vector<IntelligentPreset> generateGenreFusion(MusicalGenre genre_a, MusicalGenre genre_b,
                                                  float fusion_ratio = 0.5f,
                                                  size_t count = 3);
```
Generates presets by fusing two musical genres.

#### Learning and Adaptation

```cpp
void learnFromUserFeedback(const IntelligentPreset& preset, float user_satisfaction);
```
Learns from user satisfaction ratings.

```cpp
void learnFromPresetUsage(const IntelligentPreset& preset, size_t usage_count, float duration_seconds);
```
Learns from preset usage patterns.

```cpp
void adaptToMusicalContext(const std::vector<float>& audio_analysis,
                         const std::string& context_description);
```
Adapts to current musical context.

#### Quality and Personalization

```cpp
float predictUserAppeal(const IntelligentPreset& preset);
```
Predicts user appeal score for a preset.

```cpp
float calculateUniqueness(const IntelligentPreset& preset, 
                        const std::vector<IntelligentPreset>& existing_presets);
```
Calculates uniqueness score compared to existing presets.

```cpp
std::vector<float> analyzePresetCompatibility(const IntelligentPreset& preset,
                                             const std::vector<float>& reference_presets);
```
Analyzes compatibility with reference presets.

#### Musical Intelligence

```cpp
std::vector<float> extractMusicalFeatures(const std::vector<float>& parameters);
```
Extracts musical features from parameters.

```cpp
float estimateComplexity(const std::vector<float>& parameters);
```
Estimates musical complexity of parameters.

```cpp
float estimateHarmonicRichness(const std::vector<float>& parameters);
```
Estimates harmonic richness.

```cpp
float estimateRhythmicIntensity(const std::vector<float>& parameters);
```
Estimates rhythmic intensity.

#### Preset Analysis

```cpp
PresetAnalysis analyzePreset(const IntelligentPreset& preset);
```
Performs comprehensive preset analysis.

```cpp
std::vector<float> comparePresets(const IntelligentPreset& preset_a, 
                                const IntelligentPreset& preset_b);
```
Compares two presets across multiple dimensions.

#### Real-time Generation

```cpp
IntelligentPreset generateRealTime(const GenerationContext& context,
                                 float time_budget_ms = 50.0f);
```
Generates preset within time budget for real-time applications.

### Types

```cpp
enum class MusicalGenre {
    Electronic,
    Ambient,
    Techno,
    House,
    Trance,
    Dubstep,
    Jazz,
    Classical,
    Rock,
    Pop,
    Experimental,
    Unknown
};

enum class PresetCategory {
    Bass,
    Lead,
    Pad,
    Pluck,
    Arp,
    Drone,
    Effect,
    Percussion,
    Ambient,
    Rhythmic
};

struct IntelligentPreset {
    std::string name;
    std::string description;
    std::vector<float> parameters;
    MusicalGenre genre;
    PresetCategory category;
    float complexity_score;
    float harmonic_score;
    float user_appeal_prediction;
    float uniqueness_score;
    std::vector<float> audio_features;
    std::vector<float> similarity_scores;
    TimePoint creation_time;
    
    static constexpr size_t PARAMETER_COUNT = 128;
};

struct GenerationSettings {
    MusicalGenre target_genre = MusicalGenre::Electronic;
    PresetCategory category = PresetCategory::Bass;
    float complexity_level = 0.5f;
    float harmonic_complexity = 0.5f;
    float rhythmic_intensity = 0.5f;
    float emotional_content = 0.5f;
    bool learn_from_user = true;
    bool use_neural_networks = true;
    bool use_genetic_algorithm = false;
    bool preserve_character = false;
    size_t generation_iterations = 10;
    float mutation_rate = 0.1f;
    float crossover_rate = 0.7f;
};

struct PresetAnalysis {
    float similarity_to_reference;
    float musical_coherence;
    float parameter_balance;
    float creative_originality;
    float technical_quality;
    float user_preference_match;
    std::vector<float> feature_vector;
    std::map<std::string, float> detailed_metrics;
};
```

---

## Core Data Structures

### Common Types

```cpp
using Clock = std::chrono::high_resolution_clock;
using TimePoint = Clock::time_point;
```

### Utility Functions

```cpp
// Data preprocessing utilities
namespace ml_utils {
    std::vector<float> normalize(const std::vector<float>& data, const std::string& method = "minmax");
    std::vector<float> standardize(const std::vector<float>& data);
    std::vector<float> pcaTransform(const std::vector<std::vector<float>>& data, size_t num_components);
    float calculateEntropy(const std::vector<float>& labels);
    float calculateInformationGain(const std::vector<float>& labels, const std::vector<size_t>& feature_values);
    std::vector<float> calculateCorrelation(const std::vector<std::vector<float>>& features, const std::vector<float>& target);
}
```

---

## Error Handling

All AI components throw exceptions derived from `std::runtime_error` for error conditions. Always wrap AI calls in try-catch blocks:

```cpp
try {
    auto preset = preset_generator->generatePreset();
    // Use preset
} catch (const std::runtime_error& e) {
    std::cerr << "AI operation failed: " << e.what() << "\n";
    // Handle error
}
```

## Thread Safety

- All public methods are thread-safe unless specified otherwise
- Internal state is protected by mutexes
- Use separate instances for different threads when high concurrency is needed
- Real-time processing methods must be called from audio thread

## Performance Considerations

- **Real-time Safety**: All audio processing methods are real-time safe
- **Memory Management**: Objects use RAII and automatic memory management
- **CPU Usage**: Monitor with AIManager::getMetrics()
- **Latency**: Keep processing times <10ms for real-time applications

---

*This API reference covers all public interfaces of the Vital AI Integration System. For implementation details and internal architecture, refer to the source code and technical documentation.*