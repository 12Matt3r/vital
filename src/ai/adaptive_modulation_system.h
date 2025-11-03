#pragma once

#include "ai_manager.h"
#include <vector>
#include <array>
#include <map>
#include <unordered_map>
#include <memory>
#include <functional>
#include <mutex>

namespace vital {

/**
 * @class AdaptiveModulationSystem
 * @brief AI-powered adaptive modulation system for intelligent parameter modulation
 * 
 * Learns from user behavior and musical context to provide intelligent modulation
 * suggestions and automated parameter control.
 */
class AdaptiveModulationSystem {
public:
    // Modulation types and sources
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
    
    // Modulation curve types
    enum class CurveType {
        Linear,
        Exponential,
        Logarithmic,
        Sigmoid,
        Sine,
        Custom
    };
    
    // Individual modulation
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
        
        // Envelope parameters
        struct EnvelopeParams {
            float attack = 0.1f;
            float decay = 0.1f;
            float sustain = 0.7f;
            float release = 0.2f;
        } envelope;
        
        // LFO parameters
        struct LFOParams {
            float waveform_sine = 1.0f;
            float waveform_square = 0.0f;
            float waveform_saw = 0.0f;
            float waveform_triangle = 0.0f;
            float drift = 0.0f;
            float chaos = 0.0f;
        } lfo;
        
        // Random modulation parameters
        struct RandomParams {
            float min_value = 0.0f;
            float max_value = 1.0f;
            float probability = 0.5f;
            float smoothing = 0.1f;
            size_t pattern_size = 16;
            std::vector<float> pattern;
        } random;
    };
    
    // Adaptive modulation pattern
    struct ModulationPattern {
        std::string name;
        std::vector<Modulation> modulations;
        float effectiveness_score;
        std::vector<float> usage_history;
        std::string genre_preference;
        float last_effectiveness;
        
        bool isActive() const { return !modulations.empty(); }
        float getAverageSatisfaction() const {
            if (modulations.empty()) return 0.0f;
            float total = 0.0f;
            for (const auto& mod : modulations) {
                total += mod.user_satisfaction;
            }
            return total / modulations.size();
        }
    };
    
    // User behavior tracking
    struct UserBehavior {
        std::map<std::string, float> parameter_usage;
        std::map<std::string, float> modulation_frequency;
        std::vector<std::pair<TimePoint, std::string>> action_history;
        std::vector<float> satisfaction_ratings;
        std::map<std::string, std::vector<float>> genre_preferences;
        
        void recordAction(const std::string& action) {
            auto now = std::chrono::high_resolution_clock::now();
            action_history.emplace_back(now, action);
            
            // Keep only recent actions
            if (action_history.size() > 1000) {
                action_history.erase(action_history.begin());
            }
        }
        
        float getRecentActivityScore(size_t window_minutes = 30) const {
            auto now = std::chrono::high_resolution_clock::now();
            size_t recent_count = 0;
            
            for (auto it = action_history.rbegin(); it != action_history.rend(); ++it) {
                auto duration = std::chrono::duration_cast<std::chrono::minutes>(now - it->first);
                if (duration.count() <= window_minutes) {
                    recent_count++;
                } else {
                    break;
                }
            }
            
            return static_cast<float>(recent_count) / window_minutes;
        }
    };
    
    // Context analysis
    struct MusicalContext {
        std::string genre;
        std::string tempo_range;
        float complexity_level;
        std::vector<float> spectral_features;
        std::vector<float> temporal_features;
        float harmonic_content;
        float rhythmic_intensity;
        
        // Context similarity to other pieces
        float similarityTo(const MusicalContext& other) const {
            float similarity = 0.0f;
            
            // Genre similarity
            if (genre == other.genre) similarity += 0.3f;
            
            // Feature similarity
            size_t feature_count = std::min(spectral_features.size(), other.spectral_features.size());
            if (feature_count > 0) {
                float feature_sim = 0.0f;
                for (size_t i = 0; i < feature_count; ++i) {
                    feature_sim += 1.0f - std::abs(spectral_features[i] - other.spectral_features[i]);
                }
                feature_sim /= feature_count;
                similarity += feature_sim * 0.4f;
            }
            
            // Complexity similarity
            similarity += 1.0f - std::abs(complexity_level - other.complexity_level);
            
            return similarity / 2.0f; // Normalize
        }
    };
    
    AdaptiveModulationSystem(AIManager* ai_manager);
    ~AdaptiveModulationSystem();
    
    // System configuration
    void setLearningRate(float rate);
    void setAdaptationSpeed(float speed);
    void enableLearning(bool enable);
    void setUserExperienceLevel(float level); // 0.0 = beginner, 1.0 = expert
    
    // Modulation management
    uint32_t createModulation(const std::string& target_parameter, ModulationType type);
    bool removeModulation(uint32_t modulation_id);
    bool updateModulation(uint32_t modulation_id, const Modulation& updated);
    Modulation getModulation(uint32_t modulation_id) const;
    std::vector<uint32_t> getModulationsForParameter(const std::string& parameter) const;
    
    // Real-time processing
    float processModulation(uint32_t modulation_id, float current_value, 
                           const std::vector<float>& audio_input = {},
                           const std::vector<float>& midi_input = {});
    
    // Pattern management
    uint32_t createPattern(const std::string& name);
    bool addModulationToPattern(uint32_t pattern_id, uint32_t modulation_id);
    bool removeModulationFromPattern(uint32_t pattern_id, uint32_t modulation_id);
    bool applyPattern(uint32_t pattern_id, const std::string& context);
    void savePattern(uint32_t pattern_id, const std::string& file_path);
    bool loadPattern(uint32_t pattern_id, const std::string& file_path);
    
    // Learning system
    void learnFromUserAction(const std::string& action, float satisfaction);
    void analyzeUserBehavior();
    void adaptToMusicalContext(const MusicalContext& context);
    std::vector<std::string> getSuggestedActions(const std::string& current_state) const;
    
    // Intelligent suggestions
    std::vector<std::pair<uint32_t, float>> suggestModulations(
        const std::string& parameter, const MusicalContext& context);
    
    std::vector<uint32_t> suggestParameterModulations(
        const std::vector<std::string>& parameters, float satisfaction_threshold = 0.7f);
    
    // Predictive modulation
    void enablePredictiveModulation(bool enable);
    float predictUserAction(const std::string& parameter, TimePoint current_time);
    std::vector<uint32_t> getPredictedModulations(TimePoint future_time);
    
    // Quality assessment
    struct QualityMetrics {
        float user_satisfaction_score;
        float modulation_effectiveness;
        float pattern_success_rate;
        float learning_convergence;
        float prediction_accuracy;
    };
    
    QualityMetrics getQualityMetrics() const;
    void provideUserFeedback(uint32_t modulation_id, float satisfaction);
    
    // Statistics and analysis
    struct SystemStats {
        uint64_t total_modulations_created = 0;
        uint64_t total_modulations_used = 0;
        float average_user_satisfaction = 0.0f;
        size_t active_patterns = 0;
        std::map<ModulationType, size_t> modulation_type_usage;
        std::map<std::string, float> parameter_effectiveness;
        std::vector<float> learning_progress;
    };
    
    SystemStats getStats() const { return stats_; }
    void resetStats();
    
    // Export/Import
    void exportUserPreferences(const std::string& file_path);
    bool importUserPreferences(const std::string& file_path);
    std::string generateUserProfile() const;
    
    // Performance optimization
    void optimizeForLatency();
    void optimizeForAccuracy();
    void enableParallelProcessing(bool enable);
    
private:
    // Learning algorithms
    void updateUserPreferences(const UserBehavior& behavior);
    void updateModulationEffectiveness();
    void updatePatternSuccessRates();
    void updatePredictiveModels();
    
    // Modulation generation
    Modulation generateIntelligentModulation(const std::string& parameter, 
                                           const MusicalContext& context);
    
    Modulation generateContextualModulation(const std::string& parameter,
                                          const MusicalContext& context);
    
    // Curve processing
    float processCurve(float input, const Modulation& modulation) const;
    std::vector<float> generateCustomCurve(const std::vector<float>& control_points);
    
    // Real-time processing helpers
    float processLFO(float phase, const Modulation& mod) const;
    float processEnvelope(float phase, const Modulation& mod) const;
    float processRandom(float current_value, const Modulation& mod) const;
    float processAudioReactive(const std::vector<float>& audio, const Modulation& mod) const;
    
    // Pattern matching
    float calculatePatternMatch(const ModulationPattern& pattern, const MusicalContext& context) const;
    void updatePatternUsage(const ModulationPattern& pattern);
    
    // Predictive modeling
    std::vector<float> extractTemporalFeatures(const UserBehavior& behavior) const;
    std::vector<float> predictFutureActions(const std::vector<float>& features) const;
    
    // Member variables
    AIManager* ai_manager_;
    
    // System settings
    float learning_rate_;
    float adaptation_speed_;
    bool learning_enabled_;
    float user_experience_level_;
    bool predictive_enabled_;
    bool parallel_processing_;
    
    // Modulations
    mutable std::mutex modulations_mutex_;
    std::unordered_map<uint32_t, Modulation> modulations_;
    std::map<std::string, std::vector<uint32_t>> parameter_modulations_;
    uint32_t next_modulation_id_;
    
    // Patterns
    mutable std::mutex patterns_mutex_;
    std::unordered_map<uint32_t, ModulationPattern> patterns_;
    uint32_t next_pattern_id_;
    
    // User behavior
    mutable std::mutex behavior_mutex_;
    UserBehavior user_behavior_;
    std::unordered_map<std::string, std::vector<float>> parameter_preferences_;
    std::vector<float> satisfaction_history_;
    
    // Musical context
    MusicalContext current_context_;
    std::vector<MusicalContext> context_history_;
    std::map<std::string, std::vector<ModulationPattern>> genre_patterns_;
    
    // Learning data
    std::vector<std::pair<UserBehavior, MusicalContext>> training_data_;
    std::map<std::string, float> modulation_effectiveness_;
    std::map<uint32_t, float> pattern_success_rates_;
    
    // Quality tracking
    mutable std::mutex quality_mutex_;
    QualityMetrics quality_metrics_;
    
    // Statistics
    mutable std::mutex stats_mutex_;
    SystemStats stats_;
    
    // Performance optimization
    std::vector<std::thread> processing_threads_;
    std::vector<std::vector<float>> processing_buffers_;
};

} // namespace vital