#include "adaptive_modulation_system.h"
#include <algorithm>
#include <numeric>
#include <cmath>
#include <fstream>
#include <sstream>
#include <random>
#include <chrono>

namespace vital {

AdaptiveModulationSystem::AdaptiveModulationSystem(AIManager* ai_manager) 
    : ai_manager_(ai_manager), next_modulation_id_(1), next_pattern_id_(1),
      learning_rate_(0.01f), adaptation_speed_(0.1f), learning_enabled_(true),
      user_experience_level_(0.5f), predictive_enabled_(false), parallel_processing_(false) {
    
    // Initialize default context
    current_context_.genre = "electronic";
    current_context_.tempo_range = "medium";
    current_context_.complexity_level = 0.5f;
    current_context_.harmonic_content = 0.5f;
    current_context_.rhythmic_intensity = 0.5f;
    current_context_.spectral_features = std::vector<float>(16, 0.5f);
    current_context_.temporal_features = std::vector<float>(16, 0.5f);
    
    // Initialize learning data structures
    parameter_preferences_["cutoff"] = std::vector<float>{0.7f, 0.8f, 0.6f}; // freq, resonance, env_amount
    parameter_preferences_["amplitude"] = std::vector<float>{1.0f, 0.9f}; // env_amount, lfo_amount
    
    if (ai_manager_) {
        ai_manager_->recordEvent(AIManager::FeatureType::AdaptiveModulation, "System initialized");
    }
}

AdaptiveModulationSystem::~AdaptiveModulationSystem() {
    // Cleanup resources
    std::lock_guard<std::mutex> lock(modulations_mutex_);
    modulations_.clear();
    
    std::lock_guard<std::mutex> lock2(patterns_mutex_);
    patterns_.clear();
}

void AdaptiveModulationSystem::setLearningRate(float rate) {
    learning_rate_ = std::clamp(rate, 0.0f, 1.0f);
    
    if (ai_manager_) {
        ai_manager_->recordEvent(AIManager::FeatureType::AdaptiveModulation, 
            "Learning rate set to " + std::to_string(rate));
    }
}

void AdaptiveModulationSystem::setAdaptationSpeed(float speed) {
    adaptation_speed_ = std::clamp(speed, 0.0f, 1.0f);
}

void AdaptiveModulationSystem::enableLearning(bool enable) {
    learning_enabled_ = enable;
    
    if (ai_manager_) {
        ai_manager_->recordEvent(AIManager::FeatureType::AdaptiveModulation, 
            enable ? "Learning enabled" : "Learning disabled");
    }
}

void AdaptiveModulationSystem::setUserExperienceLevel(float level) {
    user_experience_level_ = std::clamp(level, 0.0f, 1.0f);
}

// Modulation management
uint32_t AdaptiveModulationSystem::createModulation(const std::string& target_parameter, ModulationType type) {
    Modulation modulation;
    modulation.id = next_modulation_id_++;
    modulation.target_parameter = target_parameter;
    modulation.type = type;
    modulation.source = ModulationSource::AILearning;
    modulation.amount = 0.5f;
    modulation.phase = 0.0f;
    modulation.frequency = 1.0f;
    modulation.curve_type = CurveType::Linear;
    modulation.enabled = true;
    modulation.user_satisfaction = 0.5f;
    modulation.last_used = std::chrono::high_resolution_clock::now();
    
    // Set default parameters based on type
    switch (type) {
        case ModulationType::LFO:
            modulation.lfo.waveform_sine = 1.0f;
            modulation.lfo.drift = 0.1f;
            break;
        case ModulationType::Envelope:
            modulation.envelope.attack = 0.1f;
            modulation.envelope.decay = 0.1f;
            modulation.envelope.sustain = 0.7f;
            modulation.envelope.release = 0.2f;
            break;
        case ModulationType::Random:
            modulation.random.min_value = 0.0f;
            modulation.random.max_value = 1.0f;
            modulation.random.probability = 0.5f;
            break;
        case ModulationType::StepSequencer:
            modulation.random.pattern = std::vector<float>(16, 0.5f);
            break;
        default:
            break;
    }
    
    {
        std::lock_guard<std::mutex> lock(modulations_mutex_);
        modulations_[modulation.id] = modulation;
        parameter_modulations_[target_parameter].push_back(modulation.id);
    }
    
    {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        stats_.total_modulations_created++;
        stats_.modulation_type_usage[type]++;
    }
    
    if (ai_manager_) {
        ai_manager_->recordEvent(AIManager::FeatureType::AdaptiveModulation, 
            "Modulation created: " + target_parameter + " type " + std::to_string(static_cast<int>(type)));
    }
    
    return modulation.id;
}

bool AdaptiveModulationSystem::removeModulation(uint32_t modulation_id) {
    std::lock_guard<std::mutex> lock(modulations_mutex_);
    auto it = modulations_.find(modulation_id);
    if (it != modulations_.end()) {
        const std::string& param = it->second.target_parameter;
        modulations_.erase(it);
        
        // Remove from parameter mapping
        auto param_it = parameter_modulations_.find(param);
        if (param_it != parameter_modulations_.end()) {
            auto& vec = param_it->second;
            vec.erase(std::remove(vec.begin(), vec.end(), modulation_id), vec.end());
            if (vec.empty()) {
                parameter_modulations_.erase(param_it);
            }
        }
        
        return true;
    }
    return false;
}

bool AdaptiveModulationSystem::updateModulation(uint32_t modulation_id, const Modulation& updated) {
    std::lock_guard<std::mutex> lock(modulations_mutex_);
    auto it = modulations_.find(modulation_id);
    if (it != modulations_.end()) {
        it->second = updated;
        return true;
    }
    return false;
}

Modulation AdaptiveModulationSystem::getModulation(uint32_t modulation_id) const {
    std::lock_guard<std::mutex> lock(modulations_mutex_);
    auto it = modulations_.find(modulation_id);
    return it != modulations_.end() ? it->second : Modulation{};
}

std::vector<uint32_t> AdaptiveModulationSystem::getModulationsForParameter(const std::string& parameter) const {
    std::lock_guard<std::mutex> lock(modulations_mutex_);
    auto it = parameter_modulations_.find(parameter);
    return it != parameter_modulations_.end() ? it->second : std::vector<uint32_t>{};
}

// Real-time processing
float AdaptiveModulationSystem::processModulation(uint32_t modulation_id, float current_value,
                                                const std::vector<float>& audio_input,
                                                const std::vector<float>& midi_input) {
    Modulation modulation;
    {
        std::lock_guard<std::mutex> lock(modulations_mutex_);
        auto it = modulations_.find(modulation_id);
        if (it == modulations_.end() || !it->second.enabled) {
            return current_value;
        }
        modulation = it->second;
    }
    
    float modulation_value = 0.0f;
    
    try {
        switch (modulation.type) {
            case ModulationType::LFO:
                modulation_value = processLFO(modulation.phase, modulation);
                modulation.phase += modulation.frequency * 0.01f;
                if (modulation.phase > 1.0f) modulation.phase -= 1.0f;
                break;
                
            case ModulationType::Envelope:
                modulation_value = processEnvelope(modulation.phase, modulation);
                // Update envelope phase based on trigger
                if (modulation.phase < 1.0f) {
                    modulation.phase += 0.1f;
                }
                break;
                
            case ModulationType::Random:
                modulation_value = processRandom(current_value, modulation);
                break;
                
            case ModulationType::AudioReactive:
                modulation_value = processAudioReactive(audio_input, modulation);
                break;
                
            case ModulationType::Learning:
            case ModulationType::ContextAware:
                // These are generated based on learned patterns
                modulation_value = generateIntelligentModulation(modulation.target_parameter, current_context_).amount;
                break;
                
            default:
                modulation_value = 0.0f;
                break;
        }
        
        // Apply curve processing
        modulation_value = processCurve(modulation_value, modulation);
        
        // Update last used time
        modulation.last_used = std::chrono::high_resolution_clock::now();
        
        // Apply modulation
        float final_value = current_value + (modulation_value - 0.5f) * 2.0f * modulation.amount;
        
        // Store updated modulation
        {
            std::lock_guard<std::mutex> lock(modulations_mutex_);
            auto it = modulations_.find(modulation_id);
            if (it != modulations_.end()) {
                it->second.phase = modulation.phase;
                it->second.last_used = modulation.last_used;
            }
        }
        
        {
            std::lock_guard<std::mutex> lock(stats_mutex_);
            stats_.total_modulations_used++;
        }
        
        return std::clamp(final_value, 0.0f, 1.0f);
        
    } catch (const std::exception& e) {
        if (ai_manager_) {
            ai_manager_->recordEvent(AIManager::FeatureType::AdaptiveModulation, 
                "Modulation processing error: " + std::string(e.what()));
        }
        return current_value;
    }
}

// Pattern management
uint32_t AdaptiveModulationSystem::createPattern(const std::string& name) {
    ModulationPattern pattern;
    pattern.id = next_pattern_id_++;
    pattern.name = name;
    pattern.effectiveness_score = 0.5f;
    pattern.last_effectiveness = 0.5f;
    
    {
        std::lock_guard<std::mutex> lock(patterns_mutex_);
        patterns_[pattern.id] = pattern;
    }
    
    return pattern.id;
}

bool AdaptiveModulationSystem::addModulationToPattern(uint32_t pattern_id, uint32_t modulation_id) {
    std::lock_guard<std::mutex> lock1(modulations_mutex_);
    std::lock_guard<std::mutex> lock2(patterns_mutex_);
    
    auto pattern_it = patterns_.find(pattern_id);
    if (pattern_it == patterns_.end()) return false;
    
    auto modulation_it = modulations_.find(modulation_id);
    if (modulation_it == modulations_.end()) return false;
    
    // Check if modulation is already in pattern
    for (const auto& mod : pattern_it->second.modulations) {
        if (mod.id == modulation_id) return true; // Already exists
    }
    
    pattern_it->second.modulations.push_back(modulation_it->second);
    return true;
}

bool AdaptiveModulationSystem::applyPattern(uint32_t pattern_id, const std::string& context) {
    std::lock_guard<std::mutex> lock(patterns_mutex_);
    auto it = patterns_.find(pattern_id);
    if (it == patterns_.end()) return false;
    
    const auto& pattern = it->second;
    
    // Calculate pattern effectiveness for this context
    MusicalContext context_data = current_context_;
    context_data.genre = context;
    
    float effectiveness = calculatePatternMatch(pattern, context_data);
    it->second.last_effectiveness = effectiveness;
    
    // Update pattern usage
    updatePatternUsage(pattern);
    
    if (ai_manager_) {
        ai_manager_->recordEvent(AIManager::FeatureType::AdaptiveModulation, 
            "Pattern applied: " + pattern.name + " effectiveness = " + std::to_string(effectiveness));
    }
    
    return true;
}

// Learning system
void AdaptiveModulationSystem::learnFromUserAction(const std::string& action, float satisfaction) {
    std::lock_guard<std::mutex> lock(behavior_mutex_);
    
    user_behavior_.recordAction(action);
    user_behavior_.satisfaction_ratings.push_back(satisfaction);
    
    // Keep only recent satisfaction ratings
    if (user_behavior_.satisfaction_ratings.size() > 1000) {
        user_behavior_.satisfaction_ratings.erase(user_behavior_.satisfaction_ratings.begin());
    }
    
    // Update parameter preferences based on action
    if (action.find("parameter_") == 0) {
        std::string param = action.substr(10); // Remove "parameter_" prefix
        user_behavior_.parameter_usage[param] += 1.0f;
    }
    
    if (ai_manager_) {
        ai_manager_->recordEvent(AIManager::FeatureType::AdaptiveModulation, 
            "Learning from action: " + action + " satisfaction = " + std::to_string(satisfaction));
    }
    
    // Trigger learning update
    if (learning_enabled_) {
        analyzeUserBehavior();
    }
}

void AdaptiveModulationSystem::analyzeUserBehavior() {
    std::lock_guard<std::mutex> lock(behavior_mutex_);
    
    // Update quality metrics
    float avg_satisfaction = 0.0f;
    if (!user_behavior_.satisfaction_ratings.empty()) {
        avg_satisfaction = std::accumulate(user_behavior_.satisfaction_ratings.begin(), 
                                          user_behavior_.satisfaction_ratings.end(), 0.0f) / 
                          user_behavior_.satisfaction_ratings.size();
    }
    
    {
        std::lock_guard<std::mutex> lock_q(quality_mutex_);
        quality_metrics_.user_satisfaction_score = avg_satisfaction;
    }
    
    // Update parameter effectiveness
    for (const auto& [param, usage] : user_behavior_.parameter_usage) {
        stats_.parameter_effectiveness[param] = usage;
    }
    
    // Store training data
    training_data_.emplace_back(user_behavior_, current_context_);
    
    // Keep only recent training data
    if (training_data_.size() > 5000) {
        training_data_.erase(training_data_.begin());
    }
}

void AdaptiveModulationSystem::adaptToMusicalContext(const MusicalContext& context) {
    current_context_ = context;
    
    // Store context in history
    context_history_.push_back(context);
    if (context_history_.size() > 100) {
        context_history_.erase(context_history_.begin());
    }
    
    // Update pattern suggestions based on context
    std::lock_guard<std::mutex> lock(patterns_mutex_);
    
    // Score existing patterns for current context
    for (auto& [pattern_id, pattern] : patterns_) {
        float match_score = calculatePatternMatch(pattern, context);
        pattern.last_effectiveness = match_score;
    }
    
    if (ai_manager_) {
        ai_manager_->recordEvent(AIManager::FeatureType::AdaptiveModulation, 
            "Adapted to musical context: " + context.genre);
    }
}

std::vector<std::string> AdaptiveModulationSystem::getSuggestedActions(const std::string& current_state) const {
    std::vector<std::string> suggestions;
    
    // Analyze current state and suggest actions
    if (current_state.find("bass") != std::string::npos) {
        suggestions.push_back("Increase cutoff modulation");
        suggestions.push_argument("Add envelope to filter");
        suggestions.push_argument("Use LFO on amplitude");
    }
    
    if (current_state.find("lead") != std::string::npos) {
        suggestions.push_back("Add vibrato modulation");
        suggestions.push_argument("Increase harmonic content");
        suggestions.push_argument("Use step sequencer for rhythm");
    }
    
    // Add context-based suggestions
    if (current_context_.complexity_level < 0.3f) {
        suggestions.push_back("Simplify modulation");
        suggestions.push_argument("Use fewer parameters");
    } else if (current_context_.complexity_level > 0.7f) {
        suggestions.push_argument("Add complex patterns");
        suggestions.push_argument("Use audio-reactive modulation");
    }
    
    return suggestions;
}

// Intelligent suggestions
std::vector<std::pair<uint32_t, float>> AdaptiveModulationSystem::suggestModulations(
    const std::string& parameter, const MusicalContext& context) {
    
    std::vector<std::pair<uint32_t, float>> suggestions;
    
    // Generate intelligent modulations
    for (size_t i = 0; i < 5; ++i) {
        auto modulation = generateIntelligentModulation(parameter, context);
        modulation.id = next_modulation_id_++;
        
        float effectiveness = assessModulationEffectiveness(modulation, context);
        suggestions.emplace_back(modulation.id, effectiveness);
        
        // Store modulation for later use
        std::lock_guard<std::mutex> lock(modulations_mutex_);
        modulations_[modulation.id] = modulation;
        parameter_modulations_[parameter].push_back(modulation.id);
    }
    
    // Sort by effectiveness
    std::sort(suggestions.begin(), suggestions.end(), 
              [](const auto& a, const auto& b) { return a.second > b.second; });
    
    return suggestions;
}

float AdaptiveModulationSystem::assessModulationEffectiveness(const Modulation& modulation, 
                                                            const MusicalContext& context) {
    float effectiveness = 0.0f;
    
    // Genre appropriateness
    if (modulation.type == ModulationType::LFO && context.genre.find("ambient") != std::string::npos) {
        effectiveness += 0.3f;
    }
    
    if (modulation.type == ModulationType::Random && context.rhythmic_intensity > 0.7f) {
        effectiveness += 0.4f;
    }
    
    // Complexity matching
    if (modulation.type == ModulationType::ContextAware && context.complexity_level > 0.6f) {
        effectiveness += 0.5f;
    }
    
    // Parameter-specific effectiveness
    if (modulation.target_parameter.find("cutoff") != std::string::npos && 
        modulation.type == ModulationType::Envelope) {
        effectiveness += 0.4f;
    }
    
    return effectiveness;
}

// Predictive modulation
void AdaptiveModulationSystem::enablePredictiveModulation(bool enable) {
    predictive_enabled_ = enable;
    
    if (ai_manager_) {
        ai_manager_->recordEvent(AIManager::FeatureType::AdaptiveModulation, 
            enable ? "Predictive modulation enabled" : "Predictive modulation disabled");
    }
}

float AdaptiveModulationSystem::predictUserAction(const std::string& parameter, TimePoint current_time) {
    if (!predictive_enabled_) return 0.0f;
    
    std::lock_guard<std::mutex> lock(behavior_mutex_);
    
    // Analyze usage patterns
    auto recent_actions = std::vector<std::string>();
    for (auto it = user_behavior_.action_history.rbegin(); 
         it != user_behavior_.action_history.rend(); ++it) {
        auto duration = std::chrono::duration_cast<std::chrono::minutes>(current_time - it->first);
        if (duration.count() <= 10) { // Look at last 10 minutes
            if (it->second.find(parameter) != std::string::npos) {
                recent_actions.push_back(it->second);
            }
        } else {
            break;
        }
    }
    
    // Simple prediction based on frequency
    if (recent_actions.size() > 2) {
        return std::min(1.0f, static_cast<float>(recent_actions.size()) / 10.0f);
    }
    
    return 0.0f;
}

// Quality assessment
AdaptiveModulationSystem::QualityMetrics AdaptiveModulationSystem::getQualityMetrics() const {
    std::lock_guard<std::mutex> lock(quality_mutex_);
    return quality_metrics_;
}

void AdaptiveModulationSystem::provideUserFeedback(uint32_t modulation_id, float satisfaction) {
    std::lock_guard<std::mutex> lock(modulations_mutex_);
    auto it = modulations_.find(modulation_id);
    if (it != modulations_.end()) {
        it->second.user_satisfaction = satisfaction;
        
        // Update learning
        if (learning_enabled_) {
            float old_satisfaction = it->second.user_satisfaction;
            it->second.user_satisfaction = (1.0f - learning_rate_) * old_satisfaction + 
                                         learning_rate_ * satisfaction;
        }
    }
    
    {
        std::lock_guard<std::mutex> lock(stats_mutex_);
        float total_sat = stats_.average_user_satisfaction * stats_.total_modulations_used + satisfaction;
        stats_.total_modulations_used++;
        stats_.average_user_satisfaction = total_sat / stats_.total_modulations_used;
    }
}

void AdaptiveModulationSystem::resetStats() {
    std::lock_guard<std::mutex> lock(stats_mutex_);
    stats_ = SystemStats{};
}

// Helper methods - Curve processing
float AdaptiveModulationSystem::processCurve(float input, const Modulation& modulation) const {
    switch (modulation.curve_type) {
        case CurveType::Linear:
            return input;
        case CurveType::Exponential:
            return std::pow(input, 2.0f);
        case CurveType::Logarithmic:
            return std::sqrt(input);
        case CurveType::Sigmoid:
            return 1.0f / (1.0f + std::exp(-6.0f * (input - 0.5f)));
        case CurveType::Sine:
            return 0.5f * (std::sin(2.0f * M_PI * input) + 1.0f);
        case CurveType::Custom:
            if (modulation.curve_points.size() >= 2) {
                float position = input * (modulation.curve_points.size() - 1);
                size_t index = static_cast<size_t>(position);
                float fraction = position - index;
                
                if (index >= modulation.curve_points.size() - 1) {
                    return modulation.curve_points.back();
                }
                
                return modulation.curve_points[index] * (1.0f - fraction) + 
                       modulation.curve_points[index + 1] * fraction;
            }
            return input;
        default:
            return input;
    }
}

// Modulation processing helpers
float AdaptiveModulationSystem::processLFO(float phase, const Modulation& mod) const {
    if (mod.lfo.waveform_sine > 0.5f) {
        return 0.5f * (std::sin(2.0f * M_PI * phase) + 1.0f);
    } else if (mod.lfo.waveform_square > 0.5f) {
        return (phase < 0.5f) ? 1.0f : 0.0f;
    } else if (mod.lfo.waveform_saw > 0.5f) {
        return phase;
    } else if (mod.lfo.waveform_triangle > 0.5f) {
        return (phase < 0.5f) ? 2.0f * phase : 2.0f * (1.0f - phase);
    }
    
    return 0.5f; // Default
}

float AdaptiveModulationSystem::processEnvelope(float phase, const Modulation& mod) const {
    if (phase < mod.envelope.attack) {
        return phase / mod.envelope.attack;
    } else if (phase < mod.envelope.attack + mod.envelope.decay) {
        float t = (phase - mod.envelope.attack) / mod.envelope.decay;
        return 1.0f - t * (1.0f - mod.envelope.sustain);
    } else {
        return mod.envelope.sustain;
    }
}

float AdaptiveModulationSystem::processRandom(float current_value, const Modulation& mod) const {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
    
    float random_value = dist(gen);
    return mod.random.min_value + random_value * (mod.random.max_value - mod.random.min_value);
}

float AdaptiveModulationSystem::processAudioReactive(const std::vector<float>& audio, const Modulation& mod) const {
    if (audio.empty()) return 0.5f;
    
    // Simple audio analysis - calculate RMS
    float rms = 0.0f;
    for (float sample : audio) {
        rms += sample * sample;
    }
    rms = std::sqrt(rms / audio.size());
    
    return std::clamp(rms * 2.0f, 0.0f, 1.0f);
}

// Pattern matching
float AdaptiveModulationSystem::calculatePatternMatch(const ModulationPattern& pattern, 
                                                     const MusicalContext& context) const {
    float match = 0.0f;
    
    // Genre matching
    if (!pattern.genre_preference.empty() && pattern.genre_preference == context.genre) {
        match += 0.3f;
    }
    
    // Complexity matching
    float complexity_diff = std::abs(pattern.modulations.size() / 10.0f - context.complexity_level);
    match += 1.0f - complexity_diff;
    
    // Effectiveness history
    match += pattern.getAverageSatisfaction() * 0.5f;
    
    return std::clamp(match / 2.5f, 0.0f, 1.0f);
}

void AdaptiveModulationSystem::updatePatternUsage(const ModulationPattern& pattern) {
    // Update usage statistics
    std::lock_guard<std::mutex> lock(stats_mutex_);
    stats_.active_patterns = patterns_.size();
    
    // Update pattern effectiveness
    for (const auto& mod : pattern.modulations) {
        stats_.parameter_effectiveness[mod.target_parameter] += pattern.last_effectiveness;
    }
}

// Intelligent modulation generation
AdaptiveModulationSystem::Modulation AdaptiveModulationSystem::generateIntelligentModulation(
    const std::string& parameter, const MusicalContext& context) {
    
    Modulation modulation;
    modulation.id = 0; // Will be set by caller
    modulation.target_parameter = parameter;
    modulation.type = ModulationType::Learning;
    modulation.source = ModulationSource::AILearning;
    modulation.enabled = true;
    modulation.user_satisfaction = 0.5f;
    
    // Analyze user preferences
    std::lock_guard<std::mutex> lock(behavior_mutex_);
    auto param_prefs = parameter_preferences_.find(parameter);
    if (param_prefs != parameter_preferences_.end()) {
        modulation.amount = param_prefs->second[0]; // Use first preference value
    } else {
        modulation.amount = 0.5f;
    }
    
    // Set parameters based on context
    if (context.complexity_level < 0.3f) {
        modulation.type = ModulationType::LFO;
        modulation.frequency = 0.5f;
        modulation.lfo.waveform_sine = 1.0f;
    } else if (context.complexity_level > 0.7f) {
        modulation.type = ModulationType::Random;
        modulation.random.probability = 0.3f;
    } else {
        modulation.type = ModulationType::Envelope;
        modulation.envelope.attack = 0.1f;
        modulation.envelope.decay = 0.2f;
        modulation.envelope.sustain = 0.8f;
    }
    
    return modulation;
}

// Performance optimization
void AdaptiveModulationSystem::optimizeForLatency() {
    adaptation_speed_ = 0.8f; // Faster adaptation
    learning_enabled_ = false; // Disable learning for low latency
    parallel_processing_ = true;
}

void AdaptiveModulationSystem::optimizeForAccuracy() {
    adaptation_speed_ = 0.1f; // Slower but more accurate
    learning_enabled_ = true;
    learning_rate_ = 0.01f;
}

void AdaptiveModulationSystem::enableParallelProcessing(bool enable) {
    parallel_processing_ = enable;
    
    if (enable) {
        size_t num_threads = std::thread::hardware_concurrency();
        for (size_t i = 0; i < num_threads; ++i) {
            processing_threads_.emplace_back([this, i]() {
                // Thread-specific processing
            });
        }
    } else {
        // Stop all processing threads
        for (auto& thread : processing_threads_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        processing_threads_.clear();
    }
}

} // namespace vital