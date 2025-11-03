#pragma once

#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <atomic>
#include <mutex>
#include <chrono>
#include <unordered_map>
#include <queue>
#include <array>
#include <cmath>

namespace vital {
namespace voice_control {

/**
 * @brief Voice command recognition system with offline speech processing
 * Supports 12+ languages with real-time audio processing and MFCC feature extraction
 */
class VoiceCommandRecognizer {
public:
    enum class RecognitionState {
        Idle,
        Listening,
        Processing,
        Recognizing,
        Active,
        Error
    };

    enum class CommandType {
        ParameterControl,
        PresetNavigation,
        Tutorial,
        Help,
        ModeSwitch,
        UndoRedo,
        SaveLoad,
        Playback,
        Custom
    };

    struct VoiceCommand {
        std::string text;
        CommandType type;
        std::vector<std::string> parameters;
        float confidence;
        bool is_valid;
        std::string language;
        std::chrono::milliseconds timestamp;
    };

    struct RecognitionSettings {
        bool offline_mode = true;
        int sample_rate = 16000;
        int buffer_size = 1024;
        float confidence_threshold = 0.7f;
        int max_commands_per_minute = 30;
        bool noise_reduction = true;
        bool echo_cancellation = true;
        bool enable_vad = true;
    };

    using CommandCallback = std::function<void(const VoiceCommand&)>;
    using StateChangeCallback = std::function<void(RecognitionState)>;

    VoiceCommandRecognizer();
    ~VoiceCommandRecognizer();

    // Core recognition methods
    bool initialize(const RecognitionSettings& settings);
    void startListening();
    void stopListening();
    void pauseListening();
    void resumeListening();

    // Command handling
    void registerCommandCallback(CommandCallback callback);
    void registerStateChangeCallback(StateChangeCallback callback);
    void processAudioBuffer(const float* audio_data, int num_samples);
    
    // Settings management
    void setRecognitionSettings(const RecognitionSettings& settings);
    RecognitionSettings getRecognitionSettings() const;
    
    // Language and model management
    bool loadLanguageModel(const std::string& language_code);
    void setActiveLanguage(const std::string& language_code);
    std::vector<std::string> getSupportedLanguages() const;
    
    // Status and monitoring
    RecognitionState getCurrentState() const;
    float getAudioLevel() const;
    int getCommandCount() const;
    std::vector<VoiceCommand> getRecentCommands() const;

    // Voice Activity Detection
    bool isVoiceDetected() const;
    void setVADSettings(float threshold, int min_duration_ms);

private:
    class Impl;
    std::unique_ptr<Impl> p_impl_;
    mutable std::mutex mutex_;
};

/**
 * @brief Natural language parameter control system with AI-powered understanding
 */
class NaturalLanguageController {
public:
    struct ParameterMapping {
        std::string command_phrase;
        std::string parameter_name;
        float normalized_value;
        std::string unit_type;
        std::vector<std::string> synonyms;
        float weight;
        std::vector<std::string> contexts;
    };

    struct ControlContext {
        std::string current_section;
        std::vector<std::string> active_parameters;
        std::unordered_map<std::string, float> parameter_states;
        std::chrono::steady_clock::time_point last_update;
        std::vector<std::string> recent_commands;
    };

    using ParameterCallback = std::function<void(const std::string&, float)>;
    using SectionCallback = std::function<void(const std::string&)>;

    NaturalLanguageController();
    ~NaturalLanguageController();

    // Parameter mapping
    void registerParameterMapping(const ParameterMapping& mapping);
    void registerParameterMapping(const std::string& command, const std::string& parameter);
    bool removeParameterMapping(const std::string& command);
    std::vector<ParameterMapping> getParameterMappings() const;
    
    // Context management
    void setCurrentSection(const std::string& section);
    std::string getCurrentSection() const;
    void updateParameterStates(const std::unordered_map<std::string, float>& states);
    
    // Command processing
    bool processNaturalLanguageCommand(const std::string& text);
    void processParameterCommand(const std::string& parameter, float value);
    void processSectionCommand(const std::string& section);
    
    // Callbacks
    void registerParameterCallback(ParameterCallback callback);
    void registerSectionCallback(SectionCallback callback);
    
    // Learning and adaptation
    void learnFromUserInput(const std::string& command, const std::string& parameter);
    void adaptToUserPreferences(const std::vector<std::string>& frequent_commands);
    void enableAdaptiveLearning(bool enabled);

    // Intent recognition
    enum class IntentType {
        Increase,
        Decrease,
        Set,
        Navigate,
        Play,
        Stop,
        Save,
        Load,
        Help,
        Unknown
    };

    IntentType recognizeIntent(const std::string& text);
    std::vector<std::string> extractParameters(const std::string& text, IntentType intent);

private:
    class Impl;
    std::unique_ptr<Impl> p_impl_;
    mutable std::mutex mutex_;
};

/**
 * @brief Voice-guided tutorials and assistance system with adaptive learning
 */
class VoiceTutorialSystem {
public:
    enum class TutorialType {
        BasicOperations,
        ParameterControl,
        PresetManagement,
        EffectsProcessing,
        SynthesisTechniques,
        Custom
    };

    enum class TutorialState {
        NotStarted,
        InProgress,
        Paused,
        Completed,
        Skipped
    };

    struct TutorialStep {
        int step_id;
        std::string title;
        std::string instruction_text;
        std::string voice_instruction;
        std::vector<std::string> actions_required;
        std::vector<std::string> success_criteria;
        bool is_optional;
        int estimated_duration_seconds;
        std::string audio_file_path;
    };

    struct TutorialDefinition {
        TutorialType type;
        std::string title;
        std::string description;
        std::vector<TutorialStep> steps;
        std::string difficulty_level;
        std::vector<std::string> prerequisites;
        bool requires_voice_guidance;
        std::string author;
        std::string version;
    };

    using TutorialCallback = std::function<void(TutorialStep, TutorialState)>;
    using ProgressCallback = std::function<void(int, int, TutorialState)>;

    VoiceTutorialSystem();
    ~VoiceTutorialSystem();

    // Tutorial management
    bool loadTutorial(const TutorialDefinition& tutorial);
    bool loadTutorialFromFile(const std::string& file_path);
    void startTutorial(TutorialType type);
    void startTutorial(const std::string& tutorial_id);
    void pauseTutorial();
    void resumeTutorial();
    void skipTutorial();
    void stopTutorial();

    // Progress and control
    TutorialState getCurrentState() const;
    int getCurrentStep() const;
    int getTotalSteps() const;
    TutorialType getCurrentTutorialType() const;
    std::string getCurrentTutorialTitle() const;
    
    // Voice guidance
    void enableVoiceGuidance(bool enabled);
    bool isVoiceGuidanceEnabled() const;
    void setVoiceSpeed(float speed); // 0.5 to 2.0
    float getVoiceSpeed() const;
    void setVoiceVolume(float volume); // 0.0 to 1.0
    float getVoiceVolume() const;
    void setVoiceLanguage(const std::string& language_code);
    
    // Callbacks
    void registerTutorialCallback(TutorialCallback callback);
    void registerProgressCallback(ProgressCallback callback);
    
    // Dynamic tutorial creation
    void createContextualTutorial(const std::string& context);
    void addCustomStep(const TutorialStep& step);
    
    // AI-powered adaptation
    void enableAdaptiveTutorials(bool enabled);
    void setUserSkillLevel(float skill_level); // 0.0 to 1.0
    void analyzeUserBehavior(const std::string& action);

private:
    class Impl;
    std::unique_ptr<Impl> p_impl_;
    mutable std::mutex mutex_;
};

/**
 * @brief Voice-based preset navigation with intelligent recommendations
 */
class VoicePresetNavigator {
public:
    enum class NavigationDirection {
        Next,
        Previous,
        First,
        Last,
        Random,
        Similar,
        Favorite,
        Recent
    };

    enum class PresetCategory {
        Synth,
        Bass,
        Lead,
        Pad,
        Percussion,
        Effects,
        Atmospheric,
        Experimental,
        Custom
    };

    struct PresetInfo {
        std::string id;
        std::string name;
        std::string author;
        PresetCategory category;
        std::string tags;
        std::string description;
        float rating;
        int usage_count;
        std::chrono::system_clock::time_point last_used;
        bool is_favorite;
        std::string file_path;
        std::vector<float> audio_features;
    };

    struct NavigationContext {
        PresetCategory current_category;
        std::string current_tags;
        std::vector<PresetInfo> filtered_presets;
        int current_index;
        std::vector<std::string> search_history;
    };

    using PresetCallback = std::function<void(const PresetInfo&)>;
    using NavigationCallback = std::function<void(NavigationDirection, const PresetInfo&)>;
    using SearchCallback = std::function<void(const std::vector<PresetInfo>&)>;

    VoicePresetNavigator();
    ~VoicePresetNavigator();

    // Preset management
    void loadPresetDatabase(const std::string& database_path);
    void scanPresetDirectories(const std::vector<std::string>& directories);
    void addPreset(const PresetInfo& preset);
    void removePreset(const std::string& preset_id);
    std::vector<PresetInfo> getAllPresets() const;
    std::vector<PresetInfo> getPresetsByCategory(PresetCategory category) const;
    
    // Voice navigation
    void navigateToPreset(const std::string& preset_name);
    void navigateDirection(NavigationDirection direction);
    void navigateToCategory(PresetCategory category);
    void searchPresets(const std::string& query);
    void navigateByTags(const std::vector<std::string>& tags);
    
    // Favorites and history
    void toggleFavorite(const std::string& preset_id);
    void markAsUsed(const std::string& preset_id);
    std::vector<PresetInfo> getFavoritePresets() const;
    std::vector<PresetInfo> getRecentPresets(int count = 10) const;
    
    // Smart navigation
    void enableSmartNavigation(bool enabled);
    void setSmartPreferences(float similarity_threshold = 0.7f, int max_recommendations = 5);
    std::vector<PresetInfo> getSimilarPresets(const std::string& preset_id) const;
    std::vector<PresetInfo> getRecommendations() const;
    
    // Callbacks
    void registerPresetCallback(PresetCallback callback);
    void registerNavigationCallback(NavigationCallback callback);
    void registerSearchCallback(SearchCallback callback);

private:
    class Impl;
    std::unique_ptr<Impl> p_impl_;
    mutable std::mutex mutex_;
};

/**
 * @brief Multi-language support for global accessibility with 12+ languages
 */
class MultiLanguageSupport {
public:
    struct Language {
        std::string code;
        std::string name;
        std::string native_name;
        bool is_right_to_left;
        std::string voice_model_path;
        std::vector<std::string> supported_commands;
        std::string phonetic_alphabet;
        float recognition_confidence;
    };

    struct LocalizedString {
        std::string key;
        std::string language_code;
        std::string text;
        std::string phonetics;
        std::string voice_prompt;
    };

    struct TranslationMap {
        std::string original_text;
        std::string translated_text;
        float confidence;
        bool is_phonetic;
        std::string source_language;
        std::string target_language;
    };

    using TranslationCallback = std::function<void(const TranslationMap&)>;
    using LanguageCallback = std::function<void(const std::string&)>;

    MultiLanguageSupport();
    ~MultiLanguageSupport();

    // Language management
    bool loadLanguagePack(const std::string& language_code, const std::string& pack_path);
    void setActiveLanguage(const std::string& language_code);
    std::string getActiveLanguage() const;
    std::vector<Language> getSupportedLanguages() const;
    
    // Translation and localization
    std::string translateCommand(const std::string& text, const std::string& target_language);
    std::string localizeParameter(const std::string& parameter_name);
    std::string phoneticallyAdaptCommand(const std::string& text, const std::string& language);
    
    // Voice model management
    bool loadVoiceModel(const std::string& language_code, const std::string& model_path);
    void setVoiceModel(const std::string& language_code);
    std::vector<std::string> getAvailableVoiceModels(const std::string& language_code) const;
    
    // Dynamic translation
    void enableRealTimeTranslation(bool enabled);
    void registerTranslationCallback(TranslationCallback callback);
    void processInputLanguage(const std::string& text);
    
    // Language detection
    std::string detectLanguage(const std::string& text, float& confidence);
    void enableAutoLanguageDetection(bool enabled);
    
    // Accessibility features
    void setLanguageForAccessibility(const std::string& language_code);
    void enableSimplifiedLanguage(bool enabled);
    void setReadingSpeed(float words_per_minute);
    void enableHighContrastMode(bool enabled);

    // Supported languages
    static const std::vector<std::string>& getDefaultLanguages() {
        static const std::vector<std::string> languages = {
            "en-US", "en-GB", "es-ES", "fr-FR", "de-DE", "it-IT", "pt-BR",
            "ja-JP", "ko-KR", "zh-CN", "zh-TW", "ar-SA", "hi-IN", "ru-RU"
        };
        return languages;
    }

private:
    class Impl;
    std::unique_ptr<Impl> p_impl_;
    mutable std::mutex mutex_;
};

/**
 * @brief Main voice control system that coordinates all components
 */
class VitalVoiceControlSystem {
public:
    enum class SystemState {
        Uninitialized,
        Initializing,
        Ready,
        Active,
        Error,
        Disabled
    };

    struct SystemConfiguration {
        bool enable_voice_commands = true;
        bool enable_tutorials = true;
        bool enable_preset_navigation = true;
        bool enable_multilingual = true;
        bool offline_mode = true;
        std::string default_language = "en-US";
        float master_volume = 0.8f;
        int max_concurrent_commands = 3;
        bool enable_voice_training = true;
        bool enable_adaptation = true;
    };

    using SystemStateCallback = std::function<void(SystemState, SystemState)>;
    using ErrorCallback = std::function<void(const std::string&)>;

    VitalVoiceControlSystem();
    ~VitalVoiceControlSystem();

    // System initialization
    bool initialize(const SystemConfiguration& config);
    void shutdown();
    
    // System state management
    void enableSystem();
    void disableSystem();
    SystemState getCurrentState() const;
    bool isSystemEnabled() const;
    
    // Component access
    VoiceCommandRecognizer* getCommandRecognizer();
    NaturalLanguageController* getLanguageController();
    VoiceTutorialSystem* getTutorialSystem();
    VoicePresetNavigator* getPresetNavigator();
    MultiLanguageSupport* getLanguageSupport();
    
    // Integration
    void setVitalParameters(void* parameters); // Generic pointer for Vital integration
    void processVitalAudioBuffer(const float* audio_data, int num_samples, int num_channels);
    
    // System callbacks
    void registerSystemStateCallback(SystemStateCallback callback);
    void registerErrorCallback(ErrorCallback callback);
    
    // Configuration management
    void saveConfiguration(const std::string& file_path);
    bool loadConfiguration(const std::string& file_path);
    SystemConfiguration getConfiguration() const;
    void updateConfiguration(const SystemConfiguration& config);
    
    // Voice training and adaptation
    void startVoiceTraining(const std::string& user_profile_id);
    void addVoiceSample(const std::string& phrase, const std::vector<float>& audio_data);
    void enableVoiceAdaptation(bool enabled);
    
    // Statistics and monitoring
    struct SystemStatistics {
        std::chrono::system_clock::time_point start_time;
        int total_commands_processed;
        int successful_commands;
        int failed_commands;
        float average_recognition_confidence;
        int tutorial_completions;
        int preset_navigations;
        std::chrono::milliseconds average_response_time;
        int voice_samples_trained;
        float adaptation_score;
    };
    
    SystemStatistics getStatistics() const;
    void resetStatistics();

private:
    class Impl;
    std::unique_ptr<Impl> p_impl_;
    std::atomic<SystemState> current_state_;
    mutable std::mutex mutex_;
};

} // namespace voice_control
} // namespace vital