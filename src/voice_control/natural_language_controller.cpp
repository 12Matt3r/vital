#include "vital_voice_control.h"
#include <algorithm>
#include <sstream>
#include <fstream>

namespace vital {
namespace voice_control {

/**
 * @brief Intent recognition engine for natural language processing
 */
class IntentRecognizer {
public:
    struct Intent {
        NaturalLanguageController::IntentType type;
        float confidence;
        std::vector<std::string> parameters;
        std::string action;
    };

    IntentRecognizer() {
        initializeIntentPatterns();
    }

    Intent recognizeIntent(const std::string& text) {
        std::string lower_text = text;
        std::transform(lower_text.begin(), lower_text.end(), lower_text.begin(), ::tolower);
        
        std::vector<std::pair<Intent, float>> candidates;
        
        // Check each intent pattern
        for (const auto& pattern : intent_patterns_) {
            float confidence = computePatternConfidence(lower_text, pattern.pattern);
            if (confidence > 0.3f) {
                Intent intent = pattern.intent;
                intent.confidence = confidence;
                candidates.push_back({intent, confidence});
            }
        }
        
        // Sort by confidence
        std::sort(candidates.begin(), candidates.end(), [](const auto& a, const auto& b) {
            return a.second > b.second;
        });
        
        // Return best match or unknown intent
        if (!candidates.empty() && candidates[0].second > 0.5f) {
            return candidates[0].first;
        } else {
            return {NaturalLanguageController::IntentType::Unknown, 0.0f, {}, ""};
        }
    }

private:
    struct IntentPattern {
        std::string pattern;
        Intent intent;
    };

    void initializeIntentPatterns() {
        // Increase intent patterns
        intent_patterns_.push_back({
            "increase",
            {NaturalLanguageController::IntentType::Increase, 1.0f, {}, "increase"}
        });
        intent_patterns_.push_back({
            "raise",
            {NaturalLanguageController::IntentType::Increase, 1.0f, {}, "raise"}
        });
        intent_patterns_.push_back({
            "boost",
            {NaturalLanguageController::IntentType::Increase, 1.0f, {}, "boost"}
        });
        intent_patterns_.push_back({
            "turn up",
            {NaturalLanguageController::IntentType::Increase, 1.0f, {}, "increase"}
        });

        // Decrease intent patterns
        intent_patterns_.push_back({
            "decrease",
            {NaturalLanguageController::IntentType::Decrease, 1.0f, {}, "decrease"}
        });
        intent_patterns_.push_back({
            "lower",
            {NaturalLanguageController::IntentType::Decrease, 1.0f, {}, "lower"}
        });
        intent_patterns_.push_back({
            "reduce",
            {NaturalLanguageController::IntentType::Decrease, 1.0f, {}, "reduce"}
        });
        intent_patterns_.push_back({
            "turn down",
            {NaturalLanguageController::IntentType::Decrease, 1.0f, {}, "decrease"}
        });

        // Set intent patterns
        intent_patterns_.push_back({
            "set",
            {NaturalLanguageController::IntentType::Set, 1.0f, {}, "set"}
        });
        intent_patterns_.push_back({
            "adjust",
            {NaturalLanguageController::IntentType::Set, 1.0f, {}, "set"}
        });
        intent_patterns_.push_back({
            "change to",
            {NaturalLanguageController::IntentType::Set, 1.0f, {}, "set"}
        });

        // Navigate intent patterns
        intent_patterns_.push_back({
            "go to",
            {NaturalLanguageController::IntentType::Navigate, 1.0f, {}, "navigate"}
        });
        intent_patterns_.push_back({
            "switch to",
            {NaturalLanguageController::IntentType::Navigate, 1.0f, {}, "navigate"}
        });
        intent_patterns_.push_back({
            "navigate to",
            {NaturalLanguageController::IntentType::Navigate, 1.0f, {}, "navigate"}
        });

        // Play intent patterns
        intent_patterns_.push_back({
            "play",
            {NaturalLanguageController::IntentType::Play, 1.0f, {}, "play"}
        });
        intent_patterns_.push_back({
            "start",
            {NaturalLanguageController::IntentType::Play, 1.0f, {}, "play"}
        });
        intent_patterns_.push_back({
            "begin",
            {NaturalLanguageController::IntentType::Play, 1.0f, {}, "play"}
        });

        // Stop intent patterns
        intent_patterns_.push_back({
            "stop",
            {NaturalLanguageController::IntentType::Stop, 1.0f, {}, "stop"}
        });
        intent_patterns_.push_back({
            "pause",
            {NaturalLanguageController::IntentType::Stop, 1.0f, {}, "pause"}
        });
        intent_patterns_.push_back({
            "end",
            {NaturalLanguageController::IntentType::Stop, 1.0f, {}, "stop"}
        });

        // Save intent patterns
        intent_patterns_.push_back({
            "save",
            {NaturalLanguageController::IntentType::Save, 1.0f, {}, "save"}
        });
        intent_patterns_.push_back({
            "store",
            {NaturalLanguageController::IntentType::Save, 1.0f, {}, "save"}
        });
        intent_patterns_.push_back({
            "keep",
            {NaturalLanguageController::IntentType::Save, 1.0f, {}, "save"}
        });

        // Load intent patterns
        intent_patterns_.push_back({
            "load",
            {NaturalLanguageController::IntentType::Load, 1.0f, {}, "load"}
        });
        intent_patterns_.push_back({
            "open",
            {NaturalLanguageController::IntentType::Load, 1.0f, {}, "load"}
        });
        intent_patterns_.push_back({
            "load preset",
            {NaturalLanguageController::IntentType::Load, 1.0f, {}, "load"}
        });

        // Help intent patterns
        intent_patterns_.push_back({
            "help",
            {NaturalLanguageController::IntentType::Help, 1.0f, {}, "help"}
        });
        intent_patterns_.push_back({
            "what",
            {NaturalLanguageController::IntentType::Help, 0.8f, {}, "help"}
        });
        intent_patterns_.push_back({
            "how",
            {NaturalLanguageController::IntentType::Help, 0.8f, {}, "help"}
        });
    }

    float computePatternConfidence(const std::string& text, const std::string& pattern) {
        // Simple token-based matching
        std::istringstream text_stream(text);
        std::istringstream pattern_stream(pattern);
        
        std::vector<std::string> text_tokens, pattern_tokens;
        std::string token;
        
        while (pattern_stream >> token) {
            pattern_tokens.push_back(token);
        }
        
        while (text_stream >> token) {
            text_tokens.push_back(token);
        }
        
        if (pattern_tokens.empty() || text_tokens.empty()) {
            return 0.0f;
        }
        
        // Count matching tokens
        int matches = 0;
        for (const auto& pattern_token : pattern_tokens) {
            for (const auto& text_token : text_tokens) {
                if (pattern_token == text_token || 
                    (pattern_token.length() > 3 && text_token.find(pattern_token) != std::string::npos)) {
                    matches++;
                    break;
                }
            }
        }
        
        return static_cast<float>(matches) / pattern_tokens.size();
    }

    std::vector<IntentPattern> intent_patterns_;
};

/**
 * @brief Parameter extraction and value parsing
 */
class ParameterExtractor {
public:
    struct ExtractedParameter {
        std::string parameter_name;
        float value;
        bool has_value;
        std::string unit;
        std::string raw_text;
    };

    std::vector<ExtractedParameter> extractParameters(const std::string& text, NaturalLanguageController::IntentType intent) {
        std::vector<ExtractedParameter> parameters;
        
        // Extract parameter name
        auto parameter = extractParameterName(text);
        if (!parameter.empty()) {
            parameters.push_back({parameter, 0.0f, false, "", parameter});
        }
        
        // Extract value if intent involves setting a value
        if (intent == NaturalLanguageController::IntentType::Set || 
            intent == NaturalLanguageController::IntentType::Increase ||
            intent == NaturalLanguageController::IntentType::Decrease) {
            
            auto value_info = extractValue(text);
            if (value_info.has_value) {
                parameters.back().value = value_info.value;
                parameters.back().has_value = true;
                parameters.back().unit = value_info.unit;
                parameters.back().raw_text = value_info.raw_text;
            }
        }
        
        return parameters;
    }

private:
    std::string extractParameterName(const std::string& text) {
        std::string lower_text = text;
        std::transform(lower_text.begin(), lower_text.end(), lower_text.begin(), ::tolower);
        
        // Check for known parameter names
        if (lower_text.find("volume") != std::string::npos) return "master_volume";
        if (lower_text.find("cutoff") != std::string::npos) return "filter_cutoff";
        if (lower_text.find("resonance") != std::string::npos) return "filter_resonance";
        if (lower_text.find("attack") != std::string::npos) return "envelope_attack";
        if (lower_text.find("decay") != std::string::npos) return "envelope_decay";
        if (lower_text.find("sustain") != std::string::npos) return "envelope_sustain";
        if (lower_text.find("release") != std::string::npos) return "envelope_release";
        if (lower_text.find("oscillator") != std::string::npos) return "oscillator_frequency";
        if (lower_text.find("filter") != std::string::npos) return "filter_cutoff";
        if (lower_text.find("modulation") != std::string::npos) return "modulation_amount";
        
        return "";
    }

    struct ValueInfo {
        float value;
        bool has_value;
        std::string unit;
        std::string raw_text;
    };

    ValueInfo extractValue(const std::string& text) {
        ValueInfo info = {0.0f, false, "", ""};
        
        // Extract percentage values
        auto percent_pos = text.find('%');
        if (percent_pos != std::string::npos) {
            auto start = percent_pos;
            while (start > 0 && text[start - 1] == ' ') start--;
            
            auto number_start = start;
            while (number_start > 0 && (isdigit(text[number_start - 1]) || text[number_start - 1] == '.')) {
                number_start--;
            }
            
            std::string number_str = text.substr(number_start, start - number_start);
            try {
                info.value = std::stof(number_str) / 100.0f;
                info.has_value = true;
                info.unit = "percent";
                info.raw_text = number_str + "%";
            } catch (...) {
                // Invalid number
            }
        }
        
        // Extract decimal values (0.0 to 1.0 range)
        if (!info.has_value) {
            auto decimal_pos = text.find("point");
            if (decimal_pos != std::string::npos) {
                auto start = decimal_pos + 6; // After "point"
                std::string number_str;
                
                for (size_t i = start; i < text.length() && isdigit(text[i]); i++) {
                    number_str += text[i];
                }
                
                if (!number_str.empty()) {
                    info.value = std::stof("0." + number_str);
                    info.has_value = true;
                    info.unit = "decimal";
                    info.raw_text = "point " + number_str;
                }
            }
        }
        
        // Extract verbal values
        if (!info.has_value) {
            auto verbal_value = extractVerbalValue(text);
            if (verbal_value.has_value) {
                info = verbal_value;
            }
        }
        
        return info;
    }

    ValueInfo extractVerbalValue(const std::string& text) {
        ValueInfo info = {0.0f, false, "", ""};
        std::string lower_text = text;
        std::transform(lower_text.begin(), lower_text.end(), lower_text.begin(), ::tolower);
        
        // Common verbal values
        if (lower_text.find("maximum") != std::string::npos || lower_text.find("max") != std::string::npos) {
            info.value = 1.0f;
            info.has_value = true;
            info.unit = "verbal";
            info.raw_text = "maximum";
        } else if (lower_text.find("minimum") != std::string::npos || lower_text.find("min") != std::string::npos) {
            info.value = 0.0f;
            info.has_value = true;
            info.unit = "verbal";
            info.raw_text = "minimum";
        } else if (lower_text.find("half") != std::string::npos || lower_text.find("50%") != std::string::npos) {
            info.value = 0.5f;
            info.has_value = true;
            info.unit = "verbal";
            info.raw_text = "half";
        } else if (lower_text.find("quarter") != std::string::npos || lower_text.find("25%") != std::string::npos) {
            info.value = 0.25f;
            info.has_value = true;
            info.unit = "verbal";
            info.raw_text = "quarter";
        } else if (lower_text.find("three quarters") != std::string::npos || lower_text.find("75%") != std::string::npos) {
            info.value = 0.75f;
            info.has_value = true;
            info.unit = "verbal";
            info.raw_text = "three quarters";
        } else if (lower_text.find("zero") != std::string::npos || lower_text.find("0%") != std::string::npos) {
            info.value = 0.0f;
            info.has_value = true;
            info.unit = "verbal";
            info.raw_text = "zero";
        }
        
        return info;
    }
};

/**
 * @brief User preference learning system
 */
class PreferenceLearning {
public:
    struct UserProfile {
        std::unordered_map<std::string, int> command_frequency;
        std::unordered_map<std::string, float> parameter_preferences;
        std::unordered_map<std::string, std::vector<float>> value_history;
        float learning_rate = 0.1f;
        int total_commands = 0;
    };

    void recordCommandUsage(const std::string& command) {
        std::lock_guard<std::mutex> lock(mutex_);
        user_profile_.command_frequency[command]++;
        user_profile_.total_commands++;
    }

    void recordParameterValue(const std::string& parameter, float value) {
        std::lock_guard<std::mutex> lock(mutex_);
        user_profile_.value_history[parameter].push_back(value);
        
        // Keep only recent values
        if (user_profile_.value_history[parameter].size() > 100) {
            user_profile_.value_history[parameter].erase(
                user_profile_.value_history[parameter].begin()
            );
        }
        
        // Update preference
        float avg_value = getAverageValue(parameter);
        user_profile_.parameter_preferences[parameter] = avg_value;
    }

    float getPreferenceWeight(const std::string& command) const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = user_profile_.command_frequency.find(command);
        if (it != user_profile_.command_frequency.end()) {
            // Frequency-based weighting
            return static_cast<float>(it->second) / user_profile_.total_commands;
        }
        
        return 0.0f;
    }

    float getParameterPreference(const std::string& parameter) const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = user_profile_.parameter_preferences.find(parameter);
        if (it != user_profile_.parameter_preferences.end()) {
            return it->second;
        }
        
        return 0.5f; // Default neutral preference
    }

    std::vector<std::string> getFrequentCommands(int count = 10) const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::vector<std::pair<std::string, int>> commands;
        for (const auto& pair : user_profile_.command_frequency) {
            commands.push_back(pair);
        }
        
        std::sort(commands.begin(), commands.end(), [](const auto& a, const auto& b) {
            return a.second > b.second;
        });
        
        std::vector<std::string> frequent_commands;
        for (int i = 0; i < std::min(count, static_cast<int>(commands.size())); i++) {
            frequent_commands.push_back(commands[i].first);
        }
        
        return frequent_commands;
    }

private:
    float getAverageValue(const std::string& parameter) const {
        const auto& values = user_profile_.value_history.at(parameter);
        if (values.empty()) return 0.5f;
        
        float sum = 0.0f;
        for (float value : values) {
            sum += value;
        }
        return sum / values.size();
    }

    UserProfile user_profile_;
    mutable std::mutex mutex_;
};

/**
 * @brief Implementation class for NaturalLanguageController
 */
class NaturalLanguageController::Impl {
public:
    Impl()
        : intent_recognizer_(std::make_unique<IntentRecognizer>())
        , parameter_extractor_(std::make_unique<ParameterExtractor>())
        , preference_learning_(std::make_unique<PreferenceLearning>())
        , adaptive_learning_enabled_(true) {
        
        initializeDefaultMappings();
    }

    void registerParameterMapping(const ParameterMapping& mapping) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        ParameterMapping new_mapping = mapping;
        new_mapping.weight = preference_learning_->getPreferenceWeight(mapping.command_phrase);
        
        parameter_mappings_[mapping.command_phrase] = new_mapping;
        
        // Add synonyms
        for (const auto& synonym : mapping.synonyms) {
            parameter_mappings_[synonym] = new_mapping;
        }
    }

    void registerParameterMapping(const std::string& command, const std::string& parameter) {
        ParameterMapping mapping;
        mapping.command_phrase = command;
        mapping.parameter_name = parameter;
        mapping.normalized_value = 0.5f;
        mapping.weight = 1.0f;
        
        registerParameterMapping(mapping);
    }

    bool removeParameterMapping(const std::string& command) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = parameter_mappings_.find(command);
        if (it != parameter_mappings_.end()) {
            parameter_mappings_.erase(it);
            return true;
        }
        
        return false;
    }

    std::vector<ParameterMapping> getParameterMappings() const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::vector<ParameterMapping> mappings;
        for (const auto& pair : parameter_mappings_) {
            mappings.push_back(pair.second);
        }
        
        return mappings;
    }

    void setCurrentSection(const std::string& section) {
        std::lock_guard<std::mutex> lock(mutex_);
        context_.current_section = section;
        context_.last_update = std::chrono::steady_clock::now();
    }

    std::string getCurrentSection() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return context_.current_section;
    }

    void updateParameterStates(const std::unordered_map<std::string, float>& states) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        for (const auto& pair : states) {
            context_.parameter_states[pair.first] = pair.second;
            preference_learning_->recordParameterValue(pair.first, pair.second);
        }
    }

    bool processNaturalLanguageCommand(const std::string& text) {
        if (text.empty()) return false;
        
        // Record command usage for learning
        preference_learning_->recordCommandUsage(text);
        
        // Recognize intent
        auto intent = intent_recognizer_->recognizeIntent(text);
        
        // Extract parameters
        auto extracted_params = parameter_extractor_->extractParameters(text, intent.type);
        
        // Find best matching parameter mapping
        auto best_mapping = findBestMapping(text, extracted_params);
        
        if (best_mapping) {
            // Process the command based on intent
            return processIntent(intent, text, best_mapping);
        }
        
        return false;
    }

    void processParameterCommand(const std::string& parameter, float value) {
        // Direct parameter control
        for (const auto& callback : parameter_callbacks_) {
            if (callback) {
                callback(parameter, value);
            }
        }
    }

    void processSectionCommand(const std::string& section) {
        // Section navigation
        for (const auto& callback : section_callbacks_) {
            if (callback) {
                callback(section);
            }
        }
    }

    void registerParameterCallback(ParameterCallback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        parameter_callbacks_.push_back(callback);
    }

    void registerSectionCallback(SectionCallback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        section_callbacks_.push_back(callback);
    }

    void learnFromUserInput(const std::string& command, const std::string& parameter) {
        // Create new mapping based on user feedback
        ParameterMapping mapping;
        mapping.command_phrase = command;
        mapping.parameter_name = parameter;
        mapping.normalized_value = 0.5f;
        mapping.weight = 1.0f;
        
        registerParameterMapping(mapping);
    }

    void adaptToUserPreferences(const std::vector<std::string>& frequent_commands) {
        if (!adaptive_learning_enabled_) return;
        
        // Adjust weights based on frequency
        for (const auto& command : frequent_commands) {
            auto it = parameter_mappings_.find(command);
            if (it != parameter_mappings_.end()) {
                it->second.weight = preference_learning_->getPreferenceWeight(command);
            }
        }
    }

    void enableAdaptiveLearning(bool enabled) {
        std::lock_guard<std::mutex> lock(mutex_);
        adaptive_learning_enabled_ = enabled;
    }

    NaturalLanguageController::IntentType recognizeIntent(const std::string& text) {
        auto intent = intent_recognizer_->recognizeIntent(text);
        return intent.type;
    }

    std::vector<std::string> extractParameters(const std::string& text, NaturalLanguageController::IntentType intent) {
        auto extracted = parameter_extractor_->extractParameters(text, intent);
        
        std::vector<std::string> parameters;
        for (const auto& param : extracted) {
            if (param.has_value) {
                parameters.push_back(param.parameter_name + "=" + std::to_string(param.value));
            } else {
                parameters.push_back(param.parameter_name);
            }
        }
        
        return parameters;
    }

private:
    void initializeDefaultMappings() {
        // Core parameter mappings
        registerParameterMapping("increase volume", "master_volume");
        registerParameterMapping("decrease volume", "master_volume");
        registerParameterMapping("set volume to", "master_volume");
        registerParameterMapping("maximum volume", "master_volume");
        registerParameterMapping("minimum volume", "master_volume");
        
        registerParameterMapping("increase cutoff", "filter_cutoff");
        registerParameterMapping("decrease cutoff", "filter_cutoff");
        registerParameterMapping("set cutoff to", "filter_cutoff");
        registerParameterMapping("maximum cutoff", "filter_cutoff");
        registerParameterMapping("minimum cutoff", "filter_cutoff");
        
        registerParameterMapping("increase resonance", "filter_resonance");
        registerParameterMapping("decrease resonance", "filter_resonance");
        registerParameterMapping("set resonance to", "filter_resonance");
        registerParameterMapping("maximum resonance", "filter_resonance");
        registerParameterMapping("minimum resonance", "filter_resonance");
        
        // Section navigation
        registerParameterMapping("go to filter section", "filter");
        registerParameterMapping("go to oscillators", "oscillators");
        registerParameterMapping("go to envelope", "envelope");
        registerParameterMapping("go to effects", "effects");
        registerParameterMapping("switch to filter", "filter");
        registerParameterMapping("switch to oscillators", "oscillators");
        registerParameterMapping("switch to envelope", "envelope");
        registerParameterMapping("switch to effects", "effects");
        
        // Envelope controls
        registerParameterMapping("increase attack", "envelope_attack");
        registerParameterMapping("decrease attack", "envelope_attack");
        registerParameterMapping("set attack to", "envelope_attack");
        
        registerParameterMapping("increase decay", "envelope_decay");
        registerParameterMapping("decrease decay", "envelope_decay");
        registerParameterMapping("set decay to", "envelope_decay");
        
        registerParameterMapping("increase sustain", "envelope_sustain");
        registerParameterMapping("decrease sustain", "envelope_sustain");
        registerParameterMapping("set sustain to", "envelope_sustain");
        
        registerParameterMapping("increase release", "envelope_release");
        registerParameterMapping("decrease release", "envelope_release");
        registerParameterMapping("set release to", "envelope_release");
    }

    ParameterMapping* findBestMapping(const std::string& command, const std::vector<ParameterExtractor::ExtractedParameter>& extracted_params) {
        float best_score = 0.0f;
        ParameterMapping* best_mapping = nullptr;
        
        for (auto& pair : parameter_mappings_) {
            float score = computeMappingScore(command, extracted_params, pair.second);
            if (score > best_score) {
                best_score = score;
                best_mapping = &pair.second;
            }
        }
        
        return best_score > 0.5f ? best_mapping : nullptr;
    }

    float computeMappingScore(const std::string& command, const std::vector<ParameterExtractor::ExtractedParameter>& extracted_params, const ParameterMapping& mapping) {
        float score = 0.0f;
        
        // Direct text match
        if (command.find(mapping.command_phrase) != std::string::npos) {
            score += 0.8f;
        }
        
        // Check synonyms
        for (const auto& synonym : mapping.synonyms) {
            if (command.find(synonym) != std::string::npos) {
                score += 0.6f;
                break;
            }
        }
        
        // Context weight
        if (!mapping.contexts.empty() && !context_.current_section.empty()) {
            for (const auto& context : mapping.contexts) {
                if (context == context_.current_section) {
                    score += 0.3f;
                    break;
                }
            }
        }
        
        // User preference weight
        score += mapping.weight * 0.2f;
        
        return std::min(score, 1.0f);
    }

    bool processIntent(Intent intent, const std::string& command, ParameterMapping* mapping) {
        if (!mapping) return false;
        
        float current_value = context_.parameter_states.count(mapping->parameter_name) ? 
                             context_.parameter_states[mapping->parameter_name] : 0.5f;
        
        float new_value = current_value;
        
        switch (intent.type) {
            case IntentType::Increase:
                new_value = std::min(1.0f, current_value + 0.1f);
                break;
                
            case IntentType::Decrease:
                new_value = std::max(0.0f, current_value - 0.1f);
                break;
                
            case IntentType::Set:
                // Extract and apply specific value from command
                for (const auto& param : parameter_extractor_->extractParameters(command, intent.type)) {
                    if (param.parameter_name == mapping->parameter_name && param.has_value) {
                        new_value = param.value;
                        break;
                    }
                }
                break;
                
            case IntentType::Navigate:
                processSectionCommand(mapping->parameter_name);
                return true;
                
            default:
                break;
        }
        
        // Apply the parameter change
        processParameterCommand(mapping->parameter_name, new_value);
        
        // Update context
        context_.parameter_states[mapping->parameter_name] = new_value;
        
        return true;
    }

    ControlContext context_;
    std::unordered_map<std::string, ParameterMapping> parameter_mappings_;
    std::vector<ParameterCallback> parameter_callbacks_;
    std::vector<SectionCallback> section_callbacks_;
    
    std::unique_ptr<IntentRecognizer> intent_recognizer_;
    std::unique_ptr<ParameterExtractor> parameter_extractor_;
    std::unique_ptr<PreferenceLearning> preference_learning_;
    
    bool adaptive_learning_enabled_;
    mutable std::mutex mutex_;
};

// NaturalLanguageController implementation
NaturalLanguageController::NaturalLanguageController() 
    : p_impl_(std::make_unique<Impl>()) {
}

NaturalLanguageController::~NaturalLanguageController() = default;

void NaturalLanguageController::registerParameterMapping(const ParameterMapping& mapping) {
    p_impl_->registerParameterMapping(mapping);
}

void NaturalLanguageController::registerParameterMapping(const std::string& command, const std::string& parameter) {
    p_impl_->registerParameterMapping(command, parameter);
}

bool NaturalLanguageController::removeParameterMapping(const std::string& command) {
    return p_impl_->removeParameterMapping(command);
}

std::vector<NaturalLanguageController::ParameterMapping> NaturalLanguageController::getParameterMappings() const {
    return p_impl_->getParameterMappings();
}

void NaturalLanguageController::setCurrentSection(const std::string& section) {
    p_impl_->setCurrentSection(section);
}

std::string NaturalLanguageController::getCurrentSection() const {
    return p_impl_->getCurrentSection();
}

void NaturalLanguageController::updateParameterStates(const std::unordered_map<std::string, float>& states) {
    p_impl_->updateParameterStates(states);
}

bool NaturalLanguageController::processNaturalLanguageCommand(const std::string& text) {
    return p_impl_->processNaturalLanguageCommand(text);
}

void NaturalLanguageController::processParameterCommand(const std::string& parameter, float value) {
    p_impl_->processParameterCommand(parameter, value);
}

void NaturalLanguageController::processSectionCommand(const std::string& section) {
    p_impl_->processSectionCommand(section);
}

void NaturalLanguageController::registerParameterCallback(ParameterCallback callback) {
    p_impl_->registerParameterCallback(callback);
}

void NaturalLanguageController::registerSectionCallback(SectionCallback callback) {
    p_impl_->registerSectionCallback(callback);
}

void NaturalLanguageController::learnFromUserInput(const std::string& command, const std::string& parameter) {
    p_impl_->learnFromUserInput(command, parameter);
}

void NaturalLanguageController::adaptToUserPreferences(const std::vector<std::string>& frequent_commands) {
    p_impl_->adaptToUserPreferences(frequent_commands);
}

void NaturalLanguageController::enableAdaptiveLearning(bool enabled) {
    p_impl_->enableAdaptiveLearning(enabled);
}

NaturalLanguageController::IntentType NaturalLanguageController::recognizeIntent(const std::string& text) {
    return p_impl_->recognizeIntent(text);
}

std::vector<std::string> NaturalLanguageController::extractParameters(const std::string& text, IntentType intent) {
    return p_impl_->extractParameters(text, intent);
}

} // namespace voice_control
} // namespace vital