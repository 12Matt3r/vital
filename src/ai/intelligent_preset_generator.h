#pragma once

#include "ai_manager.h"
#include "neural_preset_generator.h"
#include "machine_learning_engine.h"
#include <vector>
#include <map>
#include <memory>
#include <mutex>
#include <chrono>
#include <random>
#include <functional>

namespace vital {

/**
 * @class IntelligentPresetGenerator
 * @brief Advanced AI-powered preset generation with user learning and musical intelligence
 * 
 * Combines neural networks, machine learning, and user behavior analysis to generate
 * intelligent and personalized synthesizer presets.
 */
class IntelligentPresetGenerator {
public:
    // Musical genres and styles
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
    
    // User musical profile
    struct UserMusicalProfile {
        MusicalGenre preferred_genres[3];
        float genre_confidence[3];
        std::vector<float> preferred_parameters;
        std::vector<float> parameter_variations;
        float complexity_preference;
        float harmonic_preference;
        float rhythmic_preference;
        size_t years_experience;
        bool is_professional;
        
        // Behavior patterns
        std::map<std::string, float> parameter_usage_frequency;
        std::map<std::string, std::vector<float>> parameter_ranges_used;
        std::vector<float> satisfaction_history;
        
        void updateFromPreset(const std::vector<float>& parameters, float satisfaction) {
            satisfaction_history.push_back(satisfaction);
            if (satisfaction_history.size() > 100) {
                satisfaction_history.erase(satisfaction_history.begin());
            }
        }
        
        float getAverageSatisfaction() const {
            if (satisfaction_history.empty()) return 0.5f;
            return std::accumulate(satisfaction_history.begin(), satisfaction_history.end(), 0.0f) / 
                   satisfaction_history.size();
        }
    };
    
    // Intelligent preset generation parameters
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
    
    // Generated preset with metadata
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
        
        bool operator<(const IntelligentPreset& other) const {
            return user_appeal_prediction > other.user_appeal_prediction;
        }
    };
    
    // Generation context
    struct GenerationContext {
        MusicalGenre current_genre;
        PresetCategory current_category;
        std::vector<float> reference_features;
        std::vector<float> target_features;
        std::map<std::string, float> constraints;
        bool is_remix_mode;
        std::string remix_base_preset;
        float remix_influence;
    };
    
    IntelligentPresetGenerator(AIManager* ai_manager);
    ~IntelligentPresetGenerator();
    
    // Configuration
    void setGenerationSettings(const GenerationSettings& settings);
    GenerationSettings getGenerationSettings() const { return settings_; }
    
    // User profile management
    void updateUserProfile(const UserMusicalProfile& profile);
    UserMusicalProfile getUserProfile() const;
    void analyzeUserBehavior(const std::vector<std::vector<float>>& preset_history,
                           const std::vector<float>& satisfaction_ratings);
    
    // Preset generation
    std::vector<IntelligentPreset> generatePresets(size_t count, const GenerationContext& context = {});
    IntelligentPreset generateSinglePreset(const GenerationContext& context = {});
    
    // Advanced generation modes
    std::vector<IntelligentPreset> generateFromAudio(const std::vector<float>& audio_data,
                                                    const std::string& style_name = "auto");
    
    std::vector<IntelligentPreset> generateFromDescription(const std::string& description,
                                                          size_t count = 5);
    
    std::vector<IntelligentPreset> generateRemixVariations(const std::vector<float>& base_parameters,
                                                          size_t count = 3,
                                                          float variation_strength = 0.3f);
    
    std::vector<IntelligentPreset> generateGenreFusion(MusicalGenre genre_a, MusicalGenre genre_b,
                                                      float fusion_ratio = 0.5f,
                                                      size_t count = 3);
    
    // Learning and adaptation
    void learnFromUserFeedback(const IntelligentPreset& preset, float user_satisfaction);
    void learnFromPresetUsage(const IntelligentPreset& preset, size_t usage_count, float duration_seconds);
    void adaptToMusicalContext(const std::vector<float>& audio_analysis,
                             const std::string& context_description);
    
    // Quality and personalization
    float predictUserAppeal(const IntelligentPreset& preset);
    float calculateUniqueness(const IntelligentPreset& preset, 
                            const std::vector<IntelligentPreset>& existing_presets);
    
    std::vector<float> analyzePresetCompatibility(const IntelligentPreset& preset,
                                                 const std::vector<float>& reference_presets);
    
    // Musical intelligence
    std::vector<float> extractMusicalFeatures(const std::vector<float>& parameters);
    float estimateComplexity(const std::vector<float>& parameters);
    float estimateHarmonicRichness(const std::vector<float>& parameters);
    float estimateRhythmicIntensity(const std::vector<float>& parameters);
    
    // Preset analysis and comparison
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
    
    PresetAnalysis analyzePreset(const IntelligentPreset& preset);
    std::vector<float> comparePresets(const IntelligentPreset& preset_a, 
                                    const IntelligentPreset& preset_b);
    
    // Batch operations
    std::vector<IntelligentPreset> generatePresetCollection(const std::string& theme,
                                                          size_t count_per_style = 5);
    
    std::map<MusicalGenre, std::vector<IntelligentPreset>> generateGenreCollection(
        const std::vector<MusicalGenre>& genres, size_t count_per_genre = 10);
    
    // Real-time generation
    IntelligentPreset generateRealTime(const GenerationContext& context,
                                     float time_budget_ms = 50.0f);
    
    // Statistics and monitoring
    struct GeneratorStats {
        uint64_t total_presets_generated = 0;
        uint64_t successful_generations = 0;
        float average_user_appeal = 0.0f;
        float average_generation_time_ms = 0.0f;
        std::map<MusicalGenre, uint64_t> genre_generation_count;
        std::map<PresetCategory, uint64_t> category_generation_count;
        std::vector<float> user_satisfaction_scores;
        size_t learning_iterations = 0;
    };
    
    GeneratorStats getStats() const { return stats_; }
    void resetStats();
    
    // Model management
    bool saveUserProfile(const std::string& file_path);
    bool loadUserProfile(const std::string& file_path);
    bool exportGeneratedPresets(const std::string& file_path, 
                              const std::vector<IntelligentPreset>& presets);
    
    // Performance optimization
    void optimizeForSpeed();
    void optimizeForQuality();
    void optimizeForUserPreference();
    
private:
    // Core generation algorithms
    std::vector<IntelligentPreset> generateNeuralPresets(const GenerationContext& context);
    std::vector<IntelligentPreset> generateGeneticPresets(const GenerationContext& context);
    std::vector<IntelligentPreset> generateRuleBasedPresets(const GenerationContext& context);
    
    // Musical intelligence
    float calculateGenreFit(const IntelligentPreset& preset, MusicalGenre genre);
    float calculateCategoryFit(const IntelligentPreset& preset, PresetCategory category);
    float calculateComplexityFit(const IntelligentPreset& preset, float target_complexity);
    
    // User learning
    void updateParameterPreferences(const std::vector<float>& preferred_parameters);
    void updateGenrePreferences();
    void updateComplexityPreferences();
    
    // Quality assessment
    float assessPresetQuality(const IntelligentPreset& preset);
    float assessMusicalCoherence(const std::vector<float>& parameters);
    float assessParameterBalance(const std::vector<float>& parameters);
    
    // Utility methods
    std::vector<float> encodeMusicalContext(const GenerationContext& context);
    std::vector<float> decodeToParameters(const std::vector<float>& encoded_context);
    
    void normalizePreset(IntelligentPreset& preset);
    void addMetadata(IntelligentPreset& preset, const GenerationContext& context);
    
    // Genetic algorithm helpers
    std::vector<float> mutateParameters(const std::vector<float>& parameters, float mutation_strength);
    std::vector<float> crossoverParameters(const std::vector<float>& parent_a, 
                                         const std::vector<float>& parent_b);
    
    // Member variables
    AIManager* ai_manager_;
    
    // Generation components
    std::unique_ptr<NeuralPresetGenerator> neural_generator_;
    std::unique_ptr<MachineLearningEngine> ml_engine_;
    
    // Settings and profiles
    GenerationSettings settings_;
    UserMusicalProfile user_profile_;
    
    // Generation context
    GenerationContext current_context_;
    
    // Data storage
    mutable std::mutex presets_mutex_;
    std::vector<IntelligentPreset> generated_presets_;
    std::vector<IntelligentPreset> user_liked_presets_;
    std::vector<IntelligentPreset> user_disliked_presets_;
    
    // Learning data
    std::vector<std::pair<std::vector<float>, float>> parameter_satisfaction_pairs_;
    std::vector<std::pair<MusicalGenre, float>> genre_preference_pairs_;
    
    // Performance tracking
    mutable std::mutex stats_mutex_;
    GeneratorStats stats_;
    
    // Random number generation
    std::random_device rd_;
    std::mt19937 rng_;
    
    // Parameter names (for meaningful analysis)
    static constexpr size_t NUM_PARAMETERS = IntelligentPreset::PARAMETER_COUNT;
    static const std::vector<std::string> parameter_names_;
};

// Parameter name mapping
constexpr std::array<const char*, IntelligentPresetGenerator::NUM_PARAMETERS> parameter_names = {
    "osc1_waveform", "osc1_detune", "osc1_sync", "osc1_fm_amount", "osc1_pwm_amount",
    "osc2_waveform", "osc2_detune", "osc2_sync", "osc2_fm_amount", "osc2_pwm_amount",
    "sub_osc_level", "sub_osc_waveform", "sub_osc_octave", "noise_level", "noise_color",
    "filter_type", "filter_cutoff", "filter_resonance", "filter_env_amount", "filter_lfo_amount",
    "filter_keyboard_tracking", "filter_slope", "filter_saturation", "filter_drive", "filter_mix",
    "amp_env_attack", "amp_env_decay", "amp_env_sustain", "amp_env_release", "amp_vel_sensitivity",
    "filter_env_attack", "filter_env_decay", "filter_env_sustain", "filter_env_release", "filter_env_curve",
    "lfo1_rate", "lfo1_amount", "lfo1_waveform", "lfo1_sync", "lfo1_fade_in",
    "lfo2_rate", "lfo2_amount", "lfo2_waveform", "lfo2_sync", "lfo2_fade_in",
    "lfo3_rate", "lfo3_amount", "lfo3_waveform", "lfo3_sync", "lfo3_fade_in",
    "mod_env_attack", "mod_env_decay", "mod_env_sustain", "mod_env_release", "mod_env_curve",
    "chorus_rate", "chorus_depth", "chorus_feedback", "chorus_mix", "chorus_delay",
    "delay_time", "delay_feedback", "delay_mix", "delay_wet_dry", "delay_sync",
    "reverb_size", "reverb_decay", "reverb_damping", "reverb_mix", "reverb_wet_dry",
    "distortion_amount", "distortion_type", "distortion_saturation", "distortion_mix", "distortion_bypass",
    "compressor_threshold", "compressor_ratio", "compressor_attack", "compressor_release", "compressor_makeup",
    "eq_low_gain", "eq_mid_gain", "eq_high_gain", "eq_low_freq", "eq_mid_freq",
    "effects_mix", "master_volume", "polyphony", "unison_voices", "unison_detune",
    "arpeggiator_rate", "arpeggiator_pattern", "arpeggiator_gate", "arpeggiator_sync", "arpeggiator_octaves",
    "step_sequencer_rate", "step_sequencer_gate", "step_sequencer_sync", "step_sequencer_length", "step_sequencer_swing",
    "mod_matrix_source1", "mod_matrix_dest1", "mod_matrix_amount1", "mod_matrix_source2", "mod_matrix_dest2",
    "mod_matrix_amount2", "mod_matrix_source3", "mod_matrix_dest3", "mod_matrix_amount3", "macro1_value",
    "macro2_value", "macro3_value", "macro4_value", "performance_mode", "legato_mode",
    "portamento_time", "sustain_mode", "aftertouch_sensitivity", "velocity_curve", "release_velocity",
    "key_transpose", "key_scale", "key_root", "chord_mode", "chord_inversion",
    "randomization_amount", "randomization_seed", "preset_variation", "morph_position", "spectral_shift",
    "harmonizer_ratio", "vocoder_bands", "vocoder_carrier", "vocoder_unvoiced_threshold", "sidechain_amount",
    "pulsed_output", "sample_rate_conversion", "bit_depth_reduction", "dither_amount", "latency_compensation",
    "output_impedance", "input_sensitivity", "phase_inversion", "polarity", "ground_lift",
    "sync_reference", "midi_channel", "midi_cc_mapping", "midi_program_change", "midi_aftertouch_mode",
    "automation_rate", "automation_curve", "automation_resolution", "automation_quantize", "automation_smoothing",
    "cpu_usage_limit", "memory_usage_limit", "buffer_size", "oversampling", "aliasing_suppression",
    "user_defined1", "user_defined2", "user_defined3", "user_defined4", "user_defined5"
};

} // namespace vital