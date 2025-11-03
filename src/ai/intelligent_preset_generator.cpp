#include "intelligent_preset_generator.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <fstream>
#include <sstream>
#include <chrono>

namespace vital {

const std::vector<std::string> IntelligentPresetGenerator::parameter_names_ = {
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

IntelligentPresetGenerator::IntelligentPresetGenerator(AIManager* ai_manager)
    : ai_manager_(ai_manager), rng_(42) {
    
    // Initialize default settings
    settings_.target_genre = MusicalGenre::Electronic;
    settings_.category = PresetCategory::Bass;
    settings_.complexity_level = 0.5f;
    settings_.harmonic_complexity = 0.5f;
    settings_.rhythmic_intensity = 0.5f;
    settings_.emotional_content = 0.5f;
    settings_.learn_from_user = true;
    settings_.use_neural_networks = true;
    settings_.use_genetic_algorithm = false;
    settings_.preserve_character = false;
    settings_.generation_iterations = 10;
    settings_.mutation_rate = 0.1f;
    settings_.crossover_rate = 0.7f;
    
    // Initialize user profile
    user_profile_.preferred_genres[0] = MusicalGenre::Electronic;
    user_profile_.preferred_genres[1] = MusicalGenre::Ambient;
    user_profile_.preferred_genres[2] = MusicalGenre::Techno;
    user_profile_.genre_confidence[0] = 0.8f;
    user_profile_.genre_confidence[1] = 0.6f;
    user_profile_.genre_confidence[2] = 0.4f;
    user_profile_.preferred_parameters = std::vector<float>(NUM_PARAMETERS, 0.5f);
    user_profile_.complexity_preference = 0.5f;
    user_profile_.harmonic_preference = 0.5f;
    user_profile_.rhythmic_preference = 0.5f;
    user_profile_.years_experience = 0;
    user_profile_.is_professional = false;
    
    // Initialize generation context
    current_context_.current_genre = MusicalGenre::Electronic;
    current_context_.current_category = PresetCategory::Bass;
    current_context_.is_remix_mode = false;
    current_context_.remix_influence = 0.0f;
    
    // Initialize AI components
    neural_generator_ = std::make_unique<NeuralPresetGenerator>(ai_manager);
    ml_engine_ = std::make_unique<MachineLearningEngine>();
    
    if (ai_manager_) {
        ai_manager_->recordEvent(AIManager::FeatureType::IntelligentGeneration, "Generator initialized");
    }
}

IntelligentPresetGenerator::~IntelligentPresetGenerator() {
    // Cleanup
}

void IntelligentPresetGenerator::setGenerationSettings(const GenerationSettings& settings) {
    settings_ = settings;
    
    // Update neural generator settings if available
    if (neural_generator_) {
        NeuralPresetGenerator::NetworkConfig config;
        config.layer_sizes = {32, 64, 64, IntelligentPreset::PARAMETER_COUNT};
        config.learning_rate = 0.001f;
        config.dropout_rate = 0.2f;
        neural_generator_->setNetworkConfig(config);
        
        neural_generator_->setGenerationMode(
            settings_.use_genetic_algorithm ? 
            NeuralPresetGenerator::GenerationMode::Stochastic :
            NeuralPresetGenerator::GenerationMode::Deterministic);
    }
    
    if (ai_manager_) {
        ai_manager_->recordEvent(AIManager::FeatureType::IntelligentGeneration, "Generation settings updated");
    }
}

void IntelligentPresetGenerator::updateUserProfile(const UserMusicalProfile& profile) {
    user_profile_ = profile;
    
    // Update neural generator with user preferences
    if (neural_generator_) {
        std::vector<float> preferences(NUM_PARAMETERS, 0.5f);
        for (size_t i = 0; i < std::min(profile.preferred_parameters.size(), NUM_PARAMETERS); ++i) {
            preferences[i] = profile.preferred_parameters[i];
        }
        neural_generator_->setUserPreferences(preferences);
    }
    
    if (ai_manager_) {
        ai_manager_->recordEvent(AIManager::FeatureType::IntelligentGeneration, "User profile updated");
    }
}

IntelligentPresetGenerator::UserMusicalProfile IntelligentPresetGenerator::getUserProfile() const {
    return user_profile_;
}

void IntelligentPresetGenerator::analyzeUserBehavior(const std::vector<std::vector<float>>& preset_history,
                                                   const std::vector<float>& satisfaction_ratings) {
    if (preset_history.size() != satisfaction_ratings.size()) return;
    
    std::lock_guard<std::mutex> lock(presets_mutex_);
    
    for (size_t i = 0; i < preset_history.size(); ++i) {
        const auto& parameters = preset_history[i];
        float satisfaction = satisfaction_ratings[i];
        
        // Update parameter preferences
        if (parameters.size() == NUM_PARAMETERS) {
            for (size_t j = 0; j < NUM_PARAMETERS; ++j) {
                user_profile_.preferred_parameters[j] = 
                    (user_profile_.preferred_parameters[j] * 0.9f) + (parameters[j] * 0.1f);
            }
        }
        
        // Store for learning
        parameter_satisfaction_pairs_.emplace_back(parameters, satisfaction);
        
        // Update user profile
        user_profile_.updateFromPreset(parameters, satisfaction);
        
        // Categorize presets based on satisfaction
        if (satisfaction > 0.7f) {
            IntelligentPreset preset;
            preset.parameters = parameters;
            preset.user_appeal_prediction = satisfaction;
            user_liked_presets_.push_back(preset);
        } else if (satisfaction < 0.3f) {
            IntelligentPreset preset;
            preset.parameters = parameters;
            preset.user_appeal_prediction = satisfaction;
            user_disliked_presets_.push_back(preset);
        }
    }
    
    // Update user profile statistics
    user_profile_.complexity_preference = calculateComplexityPreference();
    user_profile_.harmonic_preference = calculateHarmonicPreference();
    user_profile_.rhythmic_preference = calculateRhythmicPreference();
    
    if (ai_manager_) {
        ai_manager_->recordEvent(AIManager::FeatureType::IntelligentGeneration, 
            "Analyzed " + std::to_string(preset_history.size()) + " user behaviors");
    }
}

// Preset generation
std::vector<IntelligentPresetGenerator::IntelligentPreset> IntelligentPresetGenerator::generatePresets(
    size_t count, const GenerationContext& context) {
    
    auto start_time = std::chrono::high_resolution_clock::now();
    std::vector<IntelligentPreset> presets;
    
    try {
        current_context_ = context;
        
        // Generate presets using multiple approaches
        if (settings_.use_neural_networks) {
            auto neural_presets = generateNeuralPresets(context);
            presets.insert(presets.end(), neural_presets.begin(), neural_presets.end());
        }
        
        if (settings_.use_genetic_algorithm) {
            auto genetic_presets = generateGeneticPresets(context);
            presets.insert(presets.end(), genetic_presets.begin(), genetic_presets.end());
        }
        
        // Add rule-based presets for diversity
        auto rule_based_presets = generateRuleBasedPresets(context);
        presets.insert(presets.end(), rule_based_presets.begin(), rule_based_presets.end());
        
        // Limit to requested count and sort by user appeal
        if (presets.size() > count) {
            std::partial_sort(presets.begin(), presets.begin() + count, presets.end(),
                             [](const auto& a, const auto& b) {
                                 return a.user_appeal_prediction > b.user_appeal_prediction;
                             });
            presets.resize(count);
        } else {
            std::sort(presets.begin(), presets.end());
        }
        
        // Update statistics
        {
            std::lock_guard<std::mutex> lock(stats_mutex_);
            stats_.total_presets_generated += presets.size();
            stats_.successful_generations += presets.size();
            
            auto end_time = std::chrono::high_resolution_clock::now();
            float generation_time = std::chrono::duration<float>(end_time - start_time).count() * 1000.0f;
            
            float avg_time = (stats_.average_generation_time_ms * (stats_.successful_generations - presets.size()) + 
                            generation_time) / stats_.successful_generations;
            stats_.average_generation_time_ms = avg_time;
            
            stats_.genre_generation_count[current_context_.current_genre] += presets.size();
            stats_.category_generation_count[current_context_.current_category] += presets.size();
        }
        
        // Store generated presets
        std::lock_guard<std::mutex> store_lock(presets_mutex_);
        generated_presets_.insert(generated_presets_.end(), presets.begin(), presets.end());
        
        if (ai_manager_) {
            ai_manager_->recordEvent(AIManager::FeatureType::IntelligentGeneration, 
                "Generated " + std::to_string(presets.size()) + " presets");
        }
        
    } catch (const std::exception& e) {
        if (ai_manager_) {
            ai_manager_->recordEvent(AIManager::FeatureType::IntelligentGeneration, 
                "Preset generation failed: " + std::string(e.what()));
        }
    }
    
    return presets;
}

IntelligentPresetGenerator::IntelligentPreset IntelligentPresetGenerator::generateSinglePreset(
    const GenerationContext& context) {
    
    auto presets = generatePresets(1, context);
    return presets.empty() ? IntelligentPreset{} : presets[0];
}

// Neural preset generation
std::vector<IntelligentPresetGenerator::IntelligentPreset> 
IntelligentPresetGenerator::generateNeuralPresets(const GenerationContext& context) {
    
    std::vector<IntelligentPreset> presets;
    
    if (!neural_generator_) return presets;
    
    // Generate conditions based on context
    std::vector<float> conditions = encodeMusicalContext(context);
    
    for (size_t i = 0; i < settings_.generation_iterations; ++i) {
        auto neural_preset = neural_generator_->generatePreset(conditions);
        
        IntelligentPreset preset;
        preset.name = "Neural " + std::to_string(i + 1);
        preset.description = "AI-generated preset using neural networks";
        preset.parameters = neural_preset.parameters;
        preset.genre = context.current_genre;
        preset.category = context.current_category;
        preset.complexity_score = estimateComplexity(preset.parameters);
        preset.harmonic_score = estimateHarmonicRichness(preset.parameters);
        preset.user_appeal_prediction = predictUserAppeal(preset);
        preset.uniqueness_score = calculateUniqueness(preset, presets);
        preset.creation_time = std::chrono::high_resolution_clock::now();
        
        // Extract audio features
        preset.audio_features = extractMusicalFeatures(preset.parameters);
        
        addMetadata(preset, context);
        presets.push_back(preset);
    }
    
    return presets;
}

// Genetic preset generation
std::vector<IntelligentPresetGenerator::IntelligentPreset> 
IntelligentPresetGenerator::generateGeneticPresets(const GenerationContext& context) {
    
    std::vector<IntelligentPreset> presets;
    
    // Initialize population
    std::vector<std::vector<float>> population;
    for (size_t i = 0; i < 20; ++i) {
        population.push_back(generateRuleBasedParameters(context));
    }
    
    // Evolve for generations
    for (size_t generation = 0; generation < settings_.generation_iterations; ++generation) {
        std::vector<IntelligentPreset> generation_presets;
        
        for (size_t i = 0; i < population.size(); ++i) {
            IntelligentPreset preset;
            preset.parameters = population[i];
            preset.genre = context.current_genre;
            preset.category = context.current_category;
            preset.user_appeal_prediction = predictUserAppeal(preset);
            generation_presets.push_back(preset);
        }
        
        // Sort by fitness (user appeal)
        std::sort(generation_presets.begin(), generation_presets.end());
        
        // Keep best individuals
        size_t elite_count = population.size() / 2;
        
        // Create next generation
        std::vector<std::vector<float>> new_population;
        
        // Elite selection
        for (size_t i = 0; i < elite_count && i < generation_presets.size(); ++i) {
            new_population.push_back(generation_presets[i].parameters);
        }
        
        // Crossover and mutation
        while (new_population.size() < population.size()) {
            // Select parents
            size_t parent1_idx = std::uniform_int_distribution<size_t>(0, elite_count - 1)(rng_);
            size_t parent2_idx = std::uniform_int_distribution<size_t>(0, elite_count - 1)(rng_);
            
            auto parent1 = generation_presets[parent1_idx].parameters;
            auto parent2 = generation_presets[parent2_idx].parameters;
            
            // Crossover
            std::vector<float> child;
            if (std::uniform_real_distribution<float>(0.0f, 1.0f)(rng_) < settings_.crossover_rate) {
                child = crossoverParameters(parent1, parent2);
            } else {
                child = parent1;
            }
            
            // Mutation
            if (std::uniform_real_distribution<float>(0.0f, 1.0f)(rng_) < settings_.mutation_rate) {
                child = mutateParameters(child, 0.1f);
            }
            
            new_population.push_back(child);
        }
        
        population = new_population;
    }
    
    // Return best presets
    for (size_t i = 0; i < std::min(settings_.generation_iterations, population.size()); ++i) {
        IntelligentPreset preset;
        preset.name = "Genetic " + std::to_string(i + 1);
        preset.description = "Evolutionary generated preset";
        preset.parameters = population[i];
        preset.genre = context.current_genre;
        preset.category = context.current_category;
        preset.complexity_score = estimateComplexity(preset.parameters);
        preset.harmonic_score = estimateHarmonicRichness(preset.parameters);
        preset.user_appeal_prediction = predictUserAppeal(preset);
        preset.uniqueness_score = calculateUniqueness(preset, presets);
        preset.creation_time = std::chrono::high_resolution_clock::now();
        
        preset.audio_features = extractMusicalFeatures(preset.parameters);
        addMetadata(preset, context);
        presets.push_back(preset);
    }
    
    return presets;
}

// Rule-based preset generation
std::vector<IntelligentPresetGenerator::IntelligentPreset> 
IntelligentPresetGenerator::generateRuleBasedPresets(const GenerationContext& context) {
    
    std::vector<IntelligentPreset> presets;
    std::vector<std::string> preset_names;
    
    // Generate different preset variations
    switch (context.current_category) {
        case PresetCategory::Bass:
            preset_names = {"Sub Bass", "Analog Bass", "FM Bass", "Resonant Bass", "Aggressive Bass"};
            break;
        case PresetCategory::Lead:
            preset_names = {"Classic Lead", "Vintage Lead", "Digital Lead", "Warm Lead", "Bright Lead"};
            break;
        case PresetCategory::Pad:
            preset_names = {"Ambient Pad", "Warm Pad", "Evolving Pad", "Crystal Pad", "Dark Pad"};
            break;
        case PresetCategory::Pluck:
            preset_names = {"Fast Pluck", "Soft Pluck", "Metallic Pluck", "Bouncy Pluck", "Delicate Pluck"};
            break;
        default:
            preset_names = {"Preset 1", "Preset 2", "Preset 3", "Preset 4", "Preset 5"};
            break;
    }
    
    for (size_t i = 0; i < preset_names.size(); ++i) {
        IntelligentPreset preset;
        preset.name = preset_names[i];
        preset.description = "Rule-based generated preset for " + std::to_string(static_cast<int>(context.current_category));
        preset.parameters = generateRuleBasedParameters(context);
        preset.genre = context.current_genre;
        preset.category = context.current_category;
        preset.complexity_score = estimateComplexity(preset.parameters);
        preset.harmonic_score = estimateHarmonicRichness(preset.parameters);
        preset.user_appeal_prediction = predictUserAppeal(preset);
        preset.uniqueness_score = 0.5f; // Default for rule-based
        preset.creation_time = std::chrono::high_resolution_clock::now();
        
        preset.audio_features = extractMusicalFeatures(preset.parameters);
        addMetadata(preset, context);
        presets.push_back(preset);
    }
    
    return presets;
}

// User learning
void IntelligentPresetGenerator::learnFromUserFeedback(const IntelligentPreset& preset, float user_satisfaction) {
    {
        std::lock_guard<std::mutex> lock(presets_mutex_);
        user_profile_.updateFromPreset(preset.parameters, user_satisfaction);
        
        // Update parameter preferences
        for (size_t i = 0; i < preset.parameters.size() && i < user_profile_.preferred_parameters.size(); ++i) {
            user_profile_.preferred_parameters[i] = 
                (user_profile_.preferred_parameters[i] * 0.95f) + (preset.parameters[i] * 0.05f);
        }
        
        parameter_satisfaction_pairs_.emplace_back(preset.parameters, user_satisfaction);
        
        if (user_satisfaction > 0.7f) {
            user_liked_presets_.push_back(preset);
        } else if (user_satisfaction < 0.3f) {
            user_disliked_presets_.push_back(preset);
        }
    }
    
    // Update neural generator
    if (neural_generator_) {
        NeuralPresetGenerator::Preset neural_preset;
        neural_preset.parameters = preset.parameters;
        neural_preset.user_rating = user_satisfaction;
        neural_generator_->learnFromUserFeedback(neural_preset, user_satisfaction);
    }
    
    // Update statistics
    {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.user_satisfaction_scores.push_back(user_satisfaction);
        if (stats_.user_satisfaction_scores.size() > 1000) {
            stats_.user_satisfaction_scores.erase(stats_.user_satisfaction_scores.begin());
        }
        
        float avg_satisfaction = std::accumulate(stats_.user_satisfaction_scores.begin(), 
                                               stats_.user_satisfaction_scores.end(), 0.0f) /
                               stats_.user_satisfaction_scores.size();
        stats_.average_user_appeal = avg_satisfaction;
        
        stats_.learning_iterations++;
    }
    
    if (ai_manager_) {
        ai_manager_->recordEvent(AIManager::FeatureType::IntelligentGeneration, 
            "Learned from user feedback: satisfaction = " + std::to_string(user_satisfaction));
    }
}

// Quality prediction
float IntelligentPresetGenerator::predictUserAppeal(const IntelligentPreset& preset) {
    float appeal_score = 0.0f;
    
    // Genre appropriateness
    float genre_appropriateness = calculateGenreFit(preset, preset.genre);
    appeal_score += genre_appropriateness * 0.25f;
    
    // Category appropriateness
    float category_appropriateness = calculateCategoryFit(preset, preset.category);
    appeal_score += category_appropriateness * 0.25f;
    
    // User preference match
    float user_preference_match = 0.0f;
    for (size_t i = 0; i < preset.parameters.size() && i < user_profile_.preferred_parameters.size(); ++i) {
        user_preference_match += 1.0f - std::abs(preset.parameters[i] - user_profile_.preferred_parameters[i]);
    }
    user_preference_match /= preset.parameters.size();
    appeal_score += user_preference_match * 0.3f;
    
    // Musical quality
    float musical_quality = assessMusicalCoherence(preset.parameters);
    appeal_score += musical_quality * 0.2f;
    
    return std::clamp(appeal_score, 0.0f, 1.0f);
}

// Musical intelligence
std::vector<float> IntelligentPresetGenerator::extractMusicalFeatures(const std::vector<float>& parameters) {
    std::vector<float> features(16, 0.0f);
    
    if (parameters.size() < NUM_PARAMETERS) return features;
    
    // Extract key parameter groups
    // Oscillator features
    features[0] = parameters[0]; // osc1_waveform
    features[1] = parameters[5]; // osc2_waveform
    features[2] = parameters[11]; // sub_osc_waveform
    features[3] = parameters[13]; // noise_level
    
    // Filter features
    features[4] = parameters[15]; // filter_cutoff
    features[5] = parameters[16]; // filter_resonance
    features[6] = parameters[18]; // filter_env_amount
    
    // Envelope features
    features[7] = parameters[25]; // amp_env_attack
    features[8] = parameters[26]; // amp_env_decay
    features[9] = parameters[27]; // amp_env_sustain
    features[10] = parameters[28]; // amp_env_release
    
    // LFO features
    features[11] = parameters[36]; // lfo1_rate
    features[12] = parameters[37]; // lfo1_amount
    
    // Effects features
    features[13] = parameters[56]; // reverb_mix
    features[14] = parameters[61]; // delay_mix
    features[15] = parameters[86]; // master_volume
    
    return features;
}

float IntelligentPresetGenerator::estimateComplexity(const std::vector<float>& parameters) {
    if (parameters.size() < NUM_PARAMETERS) return 0.0f;
    
    float complexity_score = 0.0f;
    size_t active_features = 0;
    
    // LFO usage
    for (size_t i = 36; i < 45; ++i) { // LFO parameters
        if (parameters[i] > 0.1f) {
            complexity_score += 0.1f;
            active_features++;
        }
    }
    
    // Modulation matrix usage
    for (size_t i = 82; i < 90; ++i) { // Mod matrix parameters
        if (parameters[i] > 0.1f) {
            complexity_score += 0.15f;
            active_features++;
        }
    }
    
    // Effect usage
    for (size_t i = 55; i < 80; ++i) { // Effect parameters
        if (parameters[i] > 0.1f) {
            complexity_score += 0.1f;
            active_features++;
        }
    }
    
    // Envelope complexity
    float env_variance = 0.0f;
    float env_mean = (parameters[25] + parameters[26] + parameters[27] + parameters[28]) / 4.0f;
    env_variance += std::pow(parameters[25] - env_mean, 2);
    env_variance += std::pow(parameters[26] - env_mean, 2);
    env_variance += std::pow(parameters[27] - env_mean, 2);
    env_variance += std::pow(parameters[28] - env_mean, 2);
    
    complexity_score += env_variance * 0.5f;
    
    return std::clamp(complexity_score, 0.0f, 1.0f);
}

float IntelligentPresetGenerator::estimateHarmonicRichness(const std::vector<float>& parameters) {
    if (parameters.size() < NUM_PARAMETERS) return 0.0f;
    
    float richness = 0.0f;
    
    // Oscillator richness
    richness += (parameters[0] + parameters[5] + parameters[11]) / 3.0f; // Waveform diversity
    richness += parameters[10]; // Sub oscillator level
    richness += parameters[13]; // Noise level
    
    // Filter complexity
    richness += parameters[16] * 0.5f; // Resonance adds richness
    richness += parameters[17] * 0.3f; // Filter env amount
    
    // Effects contribution
    richness += parameters[56] * 0.2f; // Reverb
    richness += parameters[52] * 0.2f; // Chorus
    
    return std::clamp(richness / 2.0f, 0.0f, 1.0f);
}

// Quality assessment
float IntelligentPresetGenerator::assessMusicalCoherence(const std::vector<float>& parameters) {
    if (parameters.size() < NUM_PARAMETERS) return 0.0f;
    
    float coherence = 0.0f;
    
    // Filter cutoff and envelope correlation
    float filter_cutoff = parameters[15];
    float filter_env_amount = parameters[18];
    if (filter_env_amount > 0.5f) {
        // If filter env is active, cutoff should be reasonable
        coherence += (filter_cutoff > 0.2f && filter_cutoff < 0.8f) ? 0.3f : 0.1f;
    }
    
    // Amplitude envelope consistency
    float amp_attack = parameters[25];
    float amp_decay = parameters[26];
    float amp_sustain = parameters[27];
    float amp_release = parameters[28];
    
    // Natural envelope flow
    if (amp_attack + amp_decay > 0.3f && amp_sustain > 0.1f) {
        coherence += 0.3f;
    }
    
    // Level consistency
    float master_level = parameters[86];
    if (master_level > 0.1f && master_level < 0.9f) {
        coherence += 0.2f;
    }
    
    // LFO and modulation balance
    float lfo_total = 0.0f;
    for (size_t i = 36; i < 45; ++i) {
        lfo_total += parameters[i];
    }
    
    if (lfo_total > 0.0f && lfo_total < 2.0f) {
        coherence += 0.2f;
    }
    
    return std::clamp(coherence, 0.0f, 1.0f);
}

// Helper methods
std::vector<float> IntelligentPresetGenerator::encodeMusicalContext(const GenerationContext& context) {
    std::vector<float> encoded(32, 0.0f);
    
    // Genre encoding
    encoded[static_cast<int>(context.current_genre)] = 1.0f;
    
    // Category encoding
    encoded[16 + static_cast<int>(context.current_category)] = 1.0f;
    
    // Reference features
    for (size_t i = 0; i < std::min(context.reference_features.size(), encoded.size() - 20); ++i) {
        encoded[20 + i] = context.reference_features[i];
    }
    
    return encoded;
}

std::vector<float> IntelligentPresetGenerator::decodeToParameters(const std::vector<float>& encoded_context) {
    // Simple parameter generation based on context
    std::vector<float> parameters(NUM_PARAMETERS, 0.5f);
    
    // Use neural generator if available
    if (neural_generator_) {
        auto preset = neural_generator_->generatePreset(encoded_context);
        return preset.parameters;
    }
    
    return generateRuleBasedParameters(current_context_);
}

void IntelligentPresetGenerator::addMetadata(IntelligentPreset& preset, const GenerationContext& context) {
    preset.complexity_score = estimateComplexity(preset.parameters);
    preset.harmonic_score = estimateHarmonicRichness(preset.parameters);
    preset.user_appeal_prediction = predictUserAppeal(preset);
    preset.creation_time = std::chrono::high_resolution_clock::now();
}

// Rule-based parameter generation
std::vector<float> IntelligentPresetGenerator::generateRuleBasedParameters(const GenerationContext& context) {
    std::vector<float> parameters(NUM_PARAMETERS, 0.5f);
    
    // Generate based on category
    switch (context.current_category) {
        case PresetCategory::Bass:
            // Lower cutoff, more sub oscillator
            parameters[15] = 0.3f; // filter_cutoff
            parameters[16] = 0.7f; // filter_resonance
            parameters[10] = 0.8f; // sub_osc_level
            parameters[11] = 0.3f; // sub_osc_waveform
            parameters[25] = 0.1f; // amp_env_attack
            parameters[26] = 0.2f; // amp_env_decay
            parameters[27] = 0.6f; // amp_env_sustain
            parameters[28] = 0.4f; // amp_env_release
            break;
            
        case PresetCategory::Lead:
            // Higher cutoff, more modulation
            parameters[15] = 0.7f; // filter_cutoff
            parameters[16] = 0.5f; // filter_resonance
            parameters[36] = 0.3f; // lfo1_rate
            parameters[37] = 0.6f; // lfo1_amount
            parameters[0] = 0.6f; // osc1_waveform
            parameters[25] = 0.2f; // amp_env_attack
            parameters[26] = 0.3f; // amp_env_decay
            break;
            
        case PresetCategory::Pad:
            // Wide filter, slow envelopes, reverb
            parameters[15] = 0.6f; // filter_cutoff
            parameters[16] = 0.3f; // filter_resonance
            parameters[25] = 0.6f; // amp_env_attack
            parameters[26] = 0.8f; // amp_env_decay
            parameters[27] = 0.9f; // amp_env_sustain
            parameters[56] = 0.7f; // reverb_mix
            break;
            
        default:
            // Default balanced settings
            break;
    }
    
    // Apply genre-specific modifications
    switch (context.current_genre) {
        case MusicalGenre::Ambient:
            parameters[56] = std::max(parameters[56], 0.6f); // More reverb
            parameters[61] = std::max(parameters[61], 0.4f); // More delay
            break;
            
        case MusicalGenre::Techno:
            parameters[16] = std::max(parameters[16], 0.7f); // More resonance
            parameters[25] = std::min(parameters[25], 0.1f); // Faster attack
            break;
            
        case MusicalGenre::Jazz:
            parameters[0] = 0.3f; // More sine/smooth waveforms
            parameters[36] = std::min(parameters[36], 0.2f); // Slower LFO
            break;
            
        default:
            break;
    }
    
    return parameters;
}

// Genetic algorithm helpers
std::vector<float> IntelligentPresetGenerator::mutateParameters(const std::vector<float>& parameters, float mutation_strength) {
    std::vector<float> mutated = parameters;
    std::uniform_real_distribution<float> mutation_dist(-mutation_strength, mutation_strength);
    
    for (size_t i = 0; i < mutated.size(); ++i) {
        if (std::uniform_real_distribution<float>(0.0f, 1.0f)(rng_) < 0.1f) { // 10% mutation rate
            mutated[i] = std::clamp(mutated[i] + mutation_dist(rng_), 0.0f, 1.0f);
        }
    }
    
    return mutated;
}

std::vector<float> IntelligentPresetGenerator::crossoverParameters(const std::vector<float>& parent_a, 
                                                                   const std::vector<float>& parent_b) {
    std::vector<float> child = parent_a;
    
    size_t crossover_point = std::uniform_int_distribution<size_t>(1, parent_a.size() - 1)(rng_);
    
    for (size_t i = crossover_point; i < child.size(); ++i) {
        child[i] = parent_b[i];
    }
    
    return child;
}

// Statistics
void IntelligentPresetGenerator::resetStats() {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    stats_ = GeneratorStats{};
}

} // namespace vital