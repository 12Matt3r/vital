#include "vital_voice_control.h"
#include <algorithm>
#include <fstream>
#include <sstream>

namespace vital {
namespace voice_control {

/**
 * @brief Dynamic tutorial content generator
 */
class DynamicContentGenerator {
public:
    static std::string generateContextualHint(const std::string& context, const std::string& parameter) {
        std::string hint = "To control " + parameter + " in the " + context + " section: ";
        
        if (context == "filter") {
            if (parameter == "filter_cutoff") {
                hint += "Use 'increase cutoff' to make the sound brighter, or 'decrease cutoff' for a darker tone.";
            } else if (parameter == "filter_resonance") {
                hint += "Say 'increase resonance' for more dramatic filtering effects, or 'decrease resonance' for a smoother sound.";
            }
        } else if (context == "oscillators") {
            if (parameter == "oscillator_frequency") {
                hint += "Use 'increase frequency' to make the sound higher, or 'decrease frequency' for lower tones.";
            }
        } else if (context == "envelope") {
            if (parameter == "envelope_attack") {
                hint += "Say 'increase attack' for a slower buildup, or 'decrease attack' for an instant response.";
            } else if (parameter == "envelope_release") {
                hint += "Use 'increase release' for a longer tail, or 'decrease release' for a shorter decay.";
            }
        }
        
        return hint;
    }
    
    static std::vector<std::string> generateProgressiveSteps(const std::string& tutorial_type) {
        std::vector<std::string> steps;
        
        if (tutorial_type == "basic_operations") {
            steps = {
                "Welcome to Vital! Let's start with the basics.",
                "The master volume controls the overall loudness. Try saying 'increase volume'.",
                "Now try 'decrease volume' to lower the sound.",
                "The filter section shapes your sound's frequency content.",
                "Say 'go to filter section' to navigate there.",
                "Try 'increase cutoff' to brighten the sound.",
                "Use 'decrease cutoff' to darken the sound.",
                "Great! You've mastered the basic controls."
            };
        } else if (tutorial_type == "parameter_control") {
            steps = {
                "Let's learn precise parameter control.",
                "You can set exact values: 'set volume to fifty percent'.",
                "Try 'set cutoff to maximum' for the brightest sound.",
                "Use 'set resonance to zero' for no filtering effect.",
                "Relative control is also available: 'increase volume by ten percent'.",
                "Great work mastering parameter control!"
            };
        } else if (tutorial_type == "preset_management") {
            steps = {
                "Presets are saved sounds you can load instantly.",
                "Say 'next preset' to browse through sounds.",
                "Use 'previous preset' to go back.",
                "Try 'random preset' for a surprise sound.",
                "Say 'load cosmic pad' to load a specific preset.",
                "You've learned preset navigation!"
            };
        } else if (tutorial_type == "effects_processing") {
            steps = {
                "Effects add character and depth to your sound.",
                "Navigate to the effects section: 'go to effects'.",
                "Each effect can be controlled with voice commands.",
                "Try adjusting the delay time: 'increase delay time'.",
                "Experiment with the reverb: 'increase reverb amount'.",
                "Effects can transform your sound completely!"
            };
        }
        
        return steps;
    }
};

/**
 * @brief User behavior analyzer for adaptive tutorials
 */
class UserBehaviorAnalyzer {
public:
    struct BehaviorMetrics {
        float completion_rate = 0.0f;
        int commands_per_step = 0;
        float average_response_time_ms = 0.0f;
        std::vector<std::string> struggling_parameters;
        float skill_level = 0.5f; // 0.0 to 1.0
    };

    void recordUserAction(const std::string& action) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        action_history_.push_back({
            action,
            std::chrono::steady_clock::now()
        });
        
        // Keep only recent actions
        if (action_history_.size() > 100) {
            action_history_.erase(action_history_.begin());
        }
        
        updateMetrics();
    }

    void recordCommandSuccess(bool success) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        success_history_.push_back(success);
        
        // Keep only recent history
        if (success_history_.size() > 50) {
            success_history_.erase(success_history_.begin());
        }
        
        updateMetrics();
    }

    float getSkillLevel() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return behavior_metrics_.skill_level;
    }

    BehaviorMetrics getBehaviorMetrics() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return behavior_metrics_;
    }

    std::vector<std::string> getStrugglingAreas() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return behavior_metrics_.struggling_parameters;
    }

private:
    struct ActionRecord {
        std::string action;
        std::chrono::steady_clock::time_point timestamp;
    };

    void updateMetrics() {
        // Calculate completion rate
        if (!success_history_.empty()) {
            int successes = std::count(success_history_.begin(), success_history_.end(), true);
            behavior_metrics_.completion_rate = static_cast<float>(successes) / success_history_.size();
        }
        
        // Calculate skill level based on completion rate and response patterns
        float base_skill = behavior_metrics_.completion_rate;
        
        // Adjust for response time (faster = higher skill)
        if (!action_history_.empty() && action_history_.size() > 1) {
            auto recent_actions = std::vector<ActionRecord>(
                action_history_.end() - std::min(10, (int)action_history_.size()), 
                action_history_.end()
            );
            
            float avg_interval = 0.0f;
            for (size_t i = 1; i < recent_actions.size(); i++) {
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                    recent_actions[i].timestamp - recent_actions[i-1].timestamp
                ).count();
                avg_interval += duration;
            }
            avg_interval /= (recent_actions.size() - 1);
            
            // Faster response indicates higher skill
            if (avg_interval < 2000.0f) { // Under 2 seconds
                base_skill += 0.2f;
            } else if (avg_interval > 5000.0f) { // Over 5 seconds
                base_skill -= 0.1f;
            }
        }
        
        behavior_metrics_.skill_level = std::clamp(base_skill, 0.0f, 1.0f);
    }

    std::vector<ActionRecord> action_history_;
    std::vector<bool> success_history_;
    BehaviorMetrics behavior_metrics_;
    mutable std::mutex mutex_;
};

/**
 * @brief Tutorial progress tracker
 */
class TutorialProgressTracker {
public:
    struct StepProgress {
        int step_id;
        TutorialState state = TutorialState::NotStarted;
        std::chrono::steady_clock::time_point start_time;
        std::chrono::milliseconds completion_time{0};
        int attempts = 0;
        bool criteria_met = false;
    };

    void startStep(int step_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto& progress = step_progress_[step_id];
        progress.step_id = step_id;
        progress.state = TutorialState::InProgress;
        progress.start_time = std::chrono::steady_clock::now();
        progress.attempts++;
    }

    void completeStep(int step_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto& progress = step_progress_[step_id];
        if (progress.state == TutorialState::InProgress) {
            progress.state = TutorialState::Completed;
            progress.completion_time = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - progress.start_time
            );
            progress.criteria_met = true;
        }
    }

    void skipStep(int step_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        step_progress_[step_id].state = TutorialState::Skipped;
    }

    void pauseStep(int step_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (step_progress_[step_id].state == TutorialState::InProgress) {
            step_progress_[step_id].state = TutorialState::Paused;
        }
    }

    void resumeStep(int step_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        if (step_progress_[step_id].state == TutorialState::Paused) {
            step_progress_[step_id].state = TutorialState::InProgress;
            step_progress_[step_id].start_time = std::chrono::steady_clock::now();
        }
    }

    TutorialState getStepState(int step_id) const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = step_progress_.find(step_id);
        return (it != step_progress_.end()) ? it->second.state : TutorialState::NotStarted;
    }

    int getCurrentStep() const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        for (const auto& pair : step_progress_) {
            if (pair.second.state == TutorialState::InProgress) {
                return pair.first;
            }
        }
        
        // Find first not started step
        for (const auto& pair : step_progress_) {
            if (pair.second.state == TutorialState::NotStarted) {
                return pair.first;
            }
        }
        
        return -1; // All steps completed
    }

    float getCompletionPercentage() const {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (step_progress_.empty()) return 0.0f;
        
        int completed = 0;
        int total = step_progress_.size();
        
        for (const auto& pair : step_progress_) {
            if (pair.second.state == TutorialState::Completed) {
                completed++;
            }
        }
        
        return static_cast<float>(completed) / total;
    }

private:
    std::unordered_map<int, StepProgress> step_progress_;
    mutable std::mutex mutex_;
};

/**
 * @brief Implementation class for VoiceTutorialSystem
 */
class VoiceTutorialSystem::Impl {
public:
    Impl()
        : current_state_(TutorialState::NotStarted)
        , current_step_(0)
        , voice_guidance_enabled_(true)
        , voice_speed_(1.0f)
        , voice_volume_(0.8f)
        , voice_language_("en-US")
        , adaptive_tutorials_enabled_(true)
        , user_skill_level_(0.5f)
        , progress_tracker_(std::make_unique<TutorialProgressTracker>())
        , behavior_analyzer_(std::make_unique<UserBehaviorAnalyzer>()) {
        
        loadDefaultTutorials();
    }

    bool loadTutorial(const TutorialDefinition& tutorial) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        tutorials_[tutorial.title] = tutorial;
        
        // Initialize progress tracking for this tutorial
        for (const auto& step : tutorial.steps) {
            // Step progress is managed by the progress tracker
        }
        
        return true;
    }

    bool loadTutorialFromFile(const std::string& file_path) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        std::ifstream file(file_path);
        if (!file.is_open()) {
            return false;
        }
        
        // Simple JSON-like parsing for tutorial file
        TutorialDefinition tutorial;
        std::string line;
        
        // Parse tutorial definition from file
        while (std::getline(file, line)) {
            if (line.find("\"title\"") != std::string::npos) {
                size_t start = line.find(":") + 2;
                size_t end = line.find("\"", start + 1);
                tutorial.title = line.substr(start + 1, end - start - 2);
            }
            // Parse other fields...
        }
        
        tutorials_[tutorial.title] = tutorial;
        return true;
    }

    void startTutorial(TutorialType type) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto tutorial = findTutorialByType(type);
        if (tutorial) {
            startTutorial(tutorial->title);
        }
    }

    void startTutorial(const std::string& tutorial_id) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = tutorials_.find(tutorial_id);
        if (it != tutorials_.end()) {
            current_tutorial_ = it->second;
            current_state_ = TutorialState::InProgress;
            current_step_ = 0;
            
            // Start first step
            if (!current_tutorial_.steps.empty()) {
                progress_tracker_->startStep(current_tutorial_.steps[0].step_id);
                announceStep(current_tutorial_.steps[0]);
            }
        }
    }

    void pauseTutorial() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (current_state_ == TutorialState::InProgress) {
            int current_step_id = getCurrentStepId();
            if (current_step_id >= 0) {
                progress_tracker_->pauseStep(current_step_id);
            }
            current_state_ = TutorialState::Paused;
        }
    }

    void resumeTutorial() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (current_state_ == TutorialState::Paused) {
            int current_step_id = getCurrentStepId();
            if (current_step_id >= 0) {
                progress_tracker_->resumeStep(current_step_id);
            }
            current_state_ = TutorialState::InProgress;
        }
    }

    void skipTutorial() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        int current_step_id = getCurrentStepId();
        if (current_step_id >= 0) {
            progress_tracker_->skipStep(current_step_id);
        }
        
        advanceToNextStep();
    }

    void stopTutorial() {
        std::lock_guard<std::mutex> lock(mutex_);
        current_state_ = TutorialState::Completed;
    }

    TutorialState getCurrentState() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return current_state_;
    }

    int getCurrentStep() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return current_step_;
    }

    int getTotalSteps() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return current_tutorial_.steps.size();
    }

    TutorialType getCurrentTutorialType() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return current_tutorial_.type;
    }

    std::string getCurrentTutorialTitle() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return current_tutorial_.title;
    }

    void enableVoiceGuidance(bool enabled) {
        std::lock_guard<std::mutex> lock(mutex_);
        voice_guidance_enabled_ = enabled;
    }

    bool isVoiceGuidanceEnabled() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return voice_guidance_enabled_;
    }

    void setVoiceSpeed(float speed) {
        std::lock_guard<std::mutex> lock(mutex_);
        voice_speed_ = std::clamp(speed, 0.5f, 2.0f);
    }

    float getVoiceSpeed() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return voice_speed_;
    }

    void setVoiceVolume(float volume) {
        std::lock_guard<std::mutex> lock(mutex_);
        voice_volume_ = std::clamp(volume, 0.0f, 1.0f);
    }

    float getVoiceVolume() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return voice_volume_;
    }

    void setVoiceLanguage(const std::string& language_code) {
        std::lock_guard<std::mutex> lock(mutex_);
        voice_language_ = language_code;
    }

    void registerTutorialCallback(TutorialCallback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        tutorial_callbacks_.push_back(callback);
    }

    void registerProgressCallback(ProgressCallback callback) {
        std::lock_guard<std::mutex> lock(mutex_);
        progress_callbacks_.push_back(callback);
    }

    void createContextualTutorial(const std::string& context) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        TutorialDefinition tutorial;
        tutorial.type = TutorialType::Custom;
        tutorial.title = "Contextual: " + context;
        tutorial.description = "Learn " + context + " specific controls";
        
        // Generate steps based on context
        auto steps = DynamicContentGenerator::generateProgressiveSteps(context);
        for (size_t i = 0; i < steps.size(); i++) {
            TutorialStep step;
            step.step_id = static_cast<int>(i + 1);
            step.title = "Step " + std::to_string(i + 1);
            step.instruction_text = steps[i];
            step.voice_instruction = steps[i];
            step.is_optional = false;
            step.estimated_duration_seconds = 30;
            
            tutorial.steps.push_back(step);
        }
        
        tutorial.requires_voice_guidance = true;
        tutorials_[tutorial.title] = tutorial;
    }

    void addCustomStep(const TutorialStep& step) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!current_tutorial_.title.empty()) {
            current_tutorial_.steps.push_back(step);
        }
    }

    void enableAdaptiveTutorials(bool enabled) {
        std::lock_guard<std::mutex> lock(mutex_);
        adaptive_tutorials_enabled_ = enabled;
    }

    void setUserSkillLevel(float skill_level) {
        std::lock_guard<std::mutex> lock(mutex_);
        user_skill_level_ = std::clamp(skill_level, 0.0f, 1.0f);
    }

    void analyzeUserBehavior(const std::string& action) {
        behavior_analyzer_->recordUserAction(action);
        
        if (adaptive_tutorials_enabled_) {
            adaptToUserBehavior();
        }
    }

private:
    TutorialDefinition* findTutorialByType(TutorialType type) {
        for (auto& pair : tutorials_) {
            if (pair.second.type == type) {
                return &pair.second;
            }
        }
        return nullptr;
    }

    void announceStep(const TutorialStep& step) {
        if (voice_guidance_enabled_) {
            // In a real implementation, this would use text-to-speech
            std::string voice_text = step.voice_instruction.empty() ? step.instruction_text : step.voice_instruction;
            
            // Notify callbacks about voice guidance
            for (const auto& callback : tutorial_callbacks_) {
                if (callback) {
                    callback(step, TutorialState::InProgress);
                }
            }
        } else {
            // Text-only guidance
            for (const auto& callback : tutorial_callbacks_) {
                if (callback) {
                    callback(step, TutorialState::InProgress);
                }
            }
        }
    }

    int getCurrentStepId() const {
        if (current_tutorial_.steps.empty() || current_step_ >= current_tutorial_.steps.size()) {
            return -1;
        }
        
        return current_tutorial_.steps[current_step_].step_id;
    }

    void advanceToNextStep() {
        current_step_++;
        
        if (current_step_ >= current_tutorial_.steps.size()) {
            // Tutorial completed
            current_state_ = TutorialState::Completed;
            
            // Notify progress callbacks
            for (const auto& callback : progress_callbacks_) {
                if (callback) {
                    callback(getTotalSteps(), getTotalSteps(), current_state_);
                }
            }
        } else {
            // Move to next step
            int next_step_id = current_tutorial_.steps[current_step_].step_id;
            progress_tracker_->startStep(next_step_id);
            announceStep(current_tutorial_.steps[current_step_]);
        }
    }

    void adaptToUserBehavior() {
        auto metrics = behavior_analyzer_->getBehaviorMetrics();
        setUserSkillLevel(metrics.skill_level);
        
        // Adapt tutorial complexity based on skill level
        if (metrics.skill_level < 0.3f) {
            // User is struggling - provide more guidance
            for (const auto& callback : progress_callbacks_) {
                if (callback) {
                    callback(current_step_, getTotalSteps(), TutorialState::Paused);
                }
            }
        }
    }

    void loadDefaultTutorials() {
        // Basic Operations Tutorial
        TutorialDefinition basic_ops;
        basic_ops.type = TutorialType::BasicOperations;
        basic_ops.title = "Basic Operations";
        basic_ops.description = "Learn the fundamental controls of Vital";
        
        auto basic_steps = DynamicContentGenerator::generateProgressiveSteps("basic_operations");
        for (size_t i = 0; i < basic_steps.size(); i++) {
            TutorialStep step;
            step.step_id = static_cast<int>(i + 1);
            step.title = "Basic Step " + std::to_string(i + 1);
            step.instruction_text = basic_steps[i];
            step.voice_instruction = basic_steps[i];
            step.is_optional = false;
            step.estimated_duration_seconds = 45;
            
            basic_ops.steps.push_back(step);
        }
        basic_ops.requires_voice_guidance = true;
        tutorials_[basic_ops.title] = basic_ops;
        
        // Parameter Control Tutorial
        TutorialDefinition param_control;
        param_control.type = TutorialType::ParameterControl;
        param_control.title = "Parameter Control";
        param_control.description = "Master precise parameter adjustments";
        
        auto param_steps = DynamicContentGenerator::generateProgressiveSteps("parameter_control");
        for (size_t i = 0; i < param_steps.size(); i++) {
            TutorialStep step;
            step.step_id = static_cast<int>(i + 1);
            step.title = "Parameter Step " + std::to_string(i + 1);
            step.instruction_text = param_steps[i];
            step.voice_instruction = param_steps[i];
            step.is_optional = false;
            step.estimated_duration_seconds = 40;
            
            param_control.steps.push_back(step);
        }
        param_control.requires_voice_guidance = true;
        tutorials_[param_control.title] = param_control;
        
        // Preset Management Tutorial
        TutorialDefinition preset_mgmt;
        preset_mgmt.type = TutorialType::PresetManagement;
        preset_mgmt.title = "Preset Management";
        preset_mgmt.description = "Navigate and manage sound presets";
        
        auto preset_steps = DynamicContentGenerator::generateProgressiveSteps("preset_management");
        for (size_t i = 0; i < preset_steps.size(); i++) {
            TutorialStep step;
            step.step_id = static_cast<int>(i + 1);
            step.title = "Preset Step " + std::to_string(i + 1);
            step.instruction_text = preset_steps[i];
            step.voice_instruction = preset_steps[i];
            step.is_optional = false;
            step.estimated_duration_seconds = 35;
            
            preset_mgmt.steps.push_back(step);
        }
        preset_mgmt.requires_voice_guidance = true;
        tutorials_[preset_mgmt.title] = preset_mgmt;
        
        // Effects Processing Tutorial
        TutorialDefinition effects;
        effects.type = TutorialType::EffectsProcessing;
        effects.title = "Effects Processing";
        effects.description = "Add depth and character with effects";
        
        auto effects_steps = DynamicContentGenerator::generateProgressiveSteps("effects_processing");
        for (size_t i = 0; i < effects_steps.size(); i++) {
            TutorialStep step;
            step.step_id = static_cast<int>(i + 1);
            step.title = "Effects Step " + std::to_string(i + 1);
            step.instruction_text = effects_steps[i];
            step.voice_instruction = effects_steps[i];
            step.is_optional = false;
            step.estimated_duration_seconds = 50;
            
            effects.steps.push_back(step);
        }
        effects.requires_voice_guidance = true;
        tutorials_[effects.title] = effects;
        
        // Synthesis Techniques Tutorial
        TutorialDefinition synthesis;
        synthesis.type = TutorialType::SynthesisTechniques;
        synthesis.title = "Advanced Synthesis";
        synthesis.description = "Learn advanced synthesis techniques";
        
        // Create advanced synthesis steps
        std::vector<std::string> synthesis_steps = {
            "Welcome to advanced synthesis techniques!",
            "Let's explore spectral manipulation and complex modulation.",
            "Try combining multiple oscillators for rich harmonic content.",
            "Experiment with FM synthesis by modulating oscillator frequency.",
            "Use the filter's envelope to create dynamic spectral changes.",
            "Master advanced techniques for unique sounds!"
        };
        
        for (size_t i = 0; i < synthesis_steps.size(); i++) {
            TutorialStep step;
            step.step_id = static_cast<int>(i + 1);
            step.title = "Synthesis Step " + std::to_string(i + 1);
            step.instruction_text = synthesis_steps[i];
            step.voice_instruction = synthesis_steps[i];
            step.is_optional = false;
            step.estimated_duration_seconds = 60;
            
            synthesis.steps.push_back(step);
        }
        synthesis.requires_voice_guidance = true;
        tutorials_[synthesis.title] = synthesis;
    }

    std::unordered_map<std::string, TutorialDefinition> tutorials_;
    TutorialDefinition current_tutorial_;
    
    TutorialState current_state_;
    int current_step_;
    
    bool voice_guidance_enabled_;
    float voice_speed_;
    float voice_volume_;
    std::string voice_language_;
    
    bool adaptive_tutorials_enabled_;
    float user_skill_level_;
    
    std::vector<TutorialCallback> tutorial_callbacks_;
    std::vector<ProgressCallback> progress_callbacks_;
    
    std::unique_ptr<TutorialProgressTracker> progress_tracker_;
    std::unique_ptr<UserBehaviorAnalyzer> behavior_analyzer_;
    
    mutable std::mutex mutex_;
};

// VoiceTutorialSystem implementation
VoiceTutorialSystem::VoiceTutorialSystem() 
    : p_impl_(std::make_unique<Impl>()) {
}

VoiceTutorialSystem::~VoiceTutorialSystem() = default;

bool VoiceTutorialSystem::loadTutorial(const TutorialDefinition& tutorial) {
    return p_impl_->loadTutorial(tutorial);
}

bool VoiceTutorialSystem::loadTutorialFromFile(const std::string& file_path) {
    return p_impl_->loadTutorialFromFile(file_path);
}

void VoiceTutorialSystem::startTutorial(TutorialType type) {
    p_impl_->startTutorial(type);
}

void VoiceTutorialSystem::startTutorial(const std::string& tutorial_id) {
    p_impl_->startTutorial(tutorial_id);
}

void VoiceTutorialSystem::pauseTutorial() {
    p_impl_->pauseTutorial();
}

void VoiceTutorialSystem::resumeTutorial() {
    p_impl_->resumeTutorial();
}

void VoiceTutorialSystem::skipTutorial() {
    p_impl_->skipTutorial();
}

void VoiceTutorialSystem::stopTutorial() {
    p_impl_->stopTutorial();
}

VoiceTutorialSystem::TutorialState VoiceTutorialSystem::getCurrentState() const {
    return p_impl_->getCurrentState();
}

int VoiceTutorialSystem::getCurrentStep() const {
    return p_impl_->getCurrentStep();
}

int VoiceTutorialSystem::getTotalSteps() const {
    return p_impl_->getTotalSteps();
}

VoiceTutorialSystem::TutorialType VoiceTutorialSystem::getCurrentTutorialType() const {
    return p_impl_->getCurrentTutorialType();
}

std::string VoiceTutorialSystem::getCurrentTutorialTitle() const {
    return p_impl_->getCurrentTutorialTitle();
}

void VoiceTutorialSystem::enableVoiceGuidance(bool enabled) {
    p_impl_->enableVoiceGuidance(enabled);
}

bool VoiceTutorialSystem::isVoiceGuidanceEnabled() const {
    return p_impl_->isVoiceGuidanceEnabled();
}

void VoiceTutorialSystem::setVoiceSpeed(float speed) {
    p_impl_->setVoiceSpeed(speed);
}

float VoiceTutorialSystem::getVoiceSpeed() const {
    return p_impl_->getVoiceSpeed();
}

void VoiceTutorialSystem::setVoiceVolume(float volume) {
    p_impl_->setVoiceVolume(volume);
}

float VoiceTutorialSystem::getVoiceVolume() const {
    return p_impl_->getVoiceVolume();
}

void VoiceTutorialSystem::setVoiceLanguage(const std::string& language_code) {
    p_impl_->setVoiceLanguage(language_code);
}

void VoiceTutorialSystem::registerTutorialCallback(TutorialCallback callback) {
    p_impl_->registerTutorialCallback(callback);
}

void VoiceTutorialSystem::registerProgressCallback(ProgressCallback callback) {
    p_impl_->registerProgressCallback(callback);
}

void VoiceTutorialSystem::createContextualTutorial(const std::string& context) {
    p_impl_->createContextualTutorial(context);
}

void VoiceTutorialSystem::addCustomStep(const TutorialStep& step) {
    p_impl_->addCustomStep(step);
}

void VoiceTutorialSystem::enableAdaptiveTutorials(bool enabled) {
    p_impl_->enableAdaptiveTutorials(enabled);
}

void VoiceTutorialSystem::setUserSkillLevel(float skill_level) {
    p_impl_->setUserSkillLevel(skill_level);
}

void VoiceTutorialSystem::analyzeUserBehavior(const std::string& action) {
    p_impl_->analyzeUserBehavior(action);
}

} // namespace voice_control
} // namespace vital