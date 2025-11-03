/**
 * @file multilang_demo.cpp
 * @brief Multi-language voice control demonstration
 * 
 * This example demonstrates the multi-language capabilities of the Vital Voice Control System,
 * including language detection, real-time translation, and cultural adaptation.
 */

#include "../vital_voice_control.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>
#include <random>

class MultiLanguageVoiceDemo {
public:
    MultiLanguageVoiceDemo() {
        std::random_device rd;
        rng_.seed(rd());
    }

    void run() {
        std::cout << "==================================================" << std::endl;
        std::cout << "   Vital Voice Control - Multi-Language Demo" << std::endl;
        std::cout << "==================================================" << std::endl;
        std::cout << std::endl;
        
        if (!initializeVoiceSystem()) {
            std::cerr << "Failed to initialize voice control system!" << std::endl;
            return;
        }
        
        std::cout << "✅ Multi-language voice control system initialized!" << std::endl;
        std::cout << std::endl;
        
        demonstrateLanguageSupport();
        demonstrateLanguageDetection();
        demonstrateRealTimeTranslation();
        demonstrateCulturalAdaptation();
        
        cleanup();
    }

private:
    std::unique_ptr<vital::voice_control::VitalVoiceControlSystem> voice_system_;
    std::mt19937 rng_;
    
    bool initializeVoiceSystem() {
        vital::voice_control::VitalVoiceControlSystem::SystemConfiguration config;
        config.enable_voice_commands = true;
        config.enable_tutorials = true;
        config.enable_preset_navigation = true;
        config.enable_multilingual = true;
        config.offline_mode = true;
        config.default_language = "en-US";
        config.enable_voice_training = true;
        config.enable_adaptation = true;
        
        voice_system_ = std::make_unique<vital::voice_control::VitalVoiceControlSystem>();
        
        if (!voice_system_->initialize(config)) {
            return false;
        }
        
        // Set up multi-language callbacks
        setupMultiLanguageCallbacks();
        
        // Enable real-time translation
        auto languages = voice_system_->getLanguageSupport();
        if (languages) {
            languages->enableRealTimeTranslation(true);
            languages->enableAutoLanguageDetection(true);
        }
        
        voice_system_->enableSystem();
        return true;
    }
    
    void setupMultiLanguageCallbacks() {
        // Translation callback
        auto languages = voice_system_->getLanguageSupport();
        if (languages) {
            languages->registerTranslationCallback([](const vital::voice_control::MultiLanguageSupport::TranslationMap& map) {
                std::cout << "[TRANSLATION] " << map.source_language << " -> " << map.target_language << std::endl;
                std::cout << "  Original: \"" << map.original_text << "\"" << std::endl;
                std::cout << "  Translated: \"" << map.translated_text << "\"" << std::endl;
                std::cout << "  Confidence: " << map.confidence << std::endl;
                std::cout << std::endl;
            });
        }
    }
    
    void demonstrateLanguageSupport() {
        std::cout << "🌍 Supported Languages:" << std::endl;
        std::cout << "======================" << std::endl;
        
        auto languages = voice_system_->getLanguageSupport();
        if (!languages) return;
        
        auto supported = languages->getSupportedLanguages();
        
        std::cout << "Total supported languages: " << supported.size() << std::endl;
        std::cout << std::endl;
        
        // Display languages by region
        std::vector<std::string> european_languages = {"en-US", "en-GB", "es-ES", "fr-FR", "de-DE", "it-IT", "pt-BR", "ru-RU"};
        std::vector<std::string> asian_languages = {"ja-JP", "ko-KR", "zh-CN", "zh-TW"};
        std::vector<std::string> other_languages = {"ar-SA", "hi-IN"};
        
        displayLanguageGroup("European Languages", european_languages, supported);
        displayLanguageGroup("Asian Languages", asian_languages, supported);
        displayLanguageGroup("Other Languages", other_languages, supported);
        
        // Test language switching
        std::cout << "Testing language switching..." << std::endl;
        testLanguageSwitching();
        std::cout << std::endl;
    }
    
    void displayLanguageGroup(const std::string& group_name, 
                             const std::vector<std::string>& language_codes,
                             const std::vector<vital::voice_control::MultiLanguageSupport::Language>& all_languages) {
        std::cout << group_name << ":" << std::endl;
        
        for (const auto& code : language_codes) {
            for (const auto& lang : all_languages) {
                if (lang.code == code) {
                    std::cout << "  " << lang.name << " (" << lang.native_name << ")" << std::endl;
                    std::cout << "    Code: " << lang.code << std::endl;
                    std::cout << "    RTL: " << (lang.is_right_to_left ? "Yes" : "No") << std::endl;
                    std::cout << "    Phonetic: " << lang.phonetic_alphabet << std::endl;
                    std::cout << "    Recognition: " << (lang.recognition_confidence * 100) << "%" << std::endl;
                    std::cout << std::endl;
                    break;
                }
            }
        }
    }
    
    void testLanguageSwitching() {
        auto languages = voice_system_->getLanguageSupport();
        if (!languages) return;
        
        std::vector<std::string> test_languages = {"en-US", "es-ES", "fr-FR", "de-DE", "ja-JP"};
        
        for (const auto& lang_code : test_languages) {
            std::cout << "Switching to: " << lang_code << std::endl;
            languages->setActiveLanguage(lang_code);
            
            // Test parameter localization
            std::string localized_volume = languages->localizeParameter("master_volume");
            std::string localized_cutoff = languages->localizeParameter("filter_cutoff");
            
            std::cout << "  'master_volume' -> '" << localized_volume << "'" << std::endl;
            std::cout << "  'filter_cutoff' -> '" << localized_cutoff << "'" << std::endl;
            std::cout << std::endl;
        }
    }
    
    void demonstrateLanguageDetection() {
        std::cout << "🔍 Language Detection Demo:" << std::endl;
        std::cout << "===========================" << std::endl;
        
        auto languages = voice_system_->getLanguageSupport();
        if (!languages) return;
        
        // Test phrases in different languages
        std::vector<std::pair<std::string, std::string>> test_phrases = {
            {"increase volume", "en-US"},
            {"aumentar volumen", "es-ES"},
            {"augmenter le volume", "fr-FR"},
            {"lautstärke erhöhen", "de-DE"},
            {"ボリュームを上げる", "ja-JP"},
            {"增加音量", "zh-CN"},
            {"볼륨 증가", "ko-KR"}
        };
        
        std::cout << "Testing automatic language detection..." << std::endl;
        std::cout << std::endl;
        
        for (const auto& [phrase, expected_lang] : test_phrases) {
            float confidence;
            std::string detected_lang = languages->detectLanguage(phrase, confidence);
            
            std::cout << "Phrase: \"" << phrase << "\"" << std::endl;
            std::cout << "  Expected: " << expected_lang << std::endl;
            std::cout << "  Detected: " << detected_lang << std::endl;
            std::cout << "  Confidence: " << (confidence * 100) << "%" << std::endl;
            std::cout << "  Accuracy: " << (detected_lang == expected_lang ? "✅" : "❌") << std::endl;
            std::cout << std::endl;
            
            // Simulate processing the language input
            languages->processInputLanguage(phrase);
        }
    }
    
    void demonstrateRealTimeTranslation() {
        std::cout << "🔄 Real-Time Translation Demo:" << std::endl;
        std::cout << "===============================" << std::endl;
        
        auto languages = voice_system_->getLanguageSupport();
        if (!languages) return;
        
        // Test translation between languages
        std::vector<std::string> source_texts = {
            "increase volume",
            "decrease cutoff",
            "set resonance to maximum",
            "next preset",
            "start basic tutorial"
        };
        
        std::vector<std::string> target_languages = {"es-ES", "fr-FR", "de-DE", "ja-JP"};
        
        for (const auto& source_text : source_texts) {
            std::cout << "Source text: \"" << source_text << "\"" << std::endl;
            
            for (const auto& target_lang : target_languages) {
                std::string translated = languages->translateCommand(source_text, target_lang);
                std::cout << "  " << target_lang << ": \"" << translated << "\"" << std::endl;
            }
            std::cout << std::endl;
            
            // Simulate processing
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    void demonstrateCulturalAdaptation() {
        std::cout << "🎭 Cultural Adaptation Demo:" << std::endl;
        std::cout << "=============================" << std::endl;
        
        auto languages = voice_system_->getLanguageSupport();
        if (!languages) return;
        
        // Test phonetic adaptation
        std::vector<std::string> commands = {
            "increase volume",
            "decrease cutoff",
            "next preset",
            "start tutorial"
        };
        
        std::vector<std::string> languages_to_test = {"ja-JP", "zh-CN", "ar-SA"};
        
        for (const auto& lang_code : languages_to_test) {
            std::cout << "Language: " << lang_code << std::endl;
            
            for (const auto& command : commands) {
                std::string adapted = languages->phoneticallyAdaptCommand(command, lang_code);
                std::cout << "  \"" << command << "\" -> \"" << adapted << "\"" << std::endl;
            }
            std::cout << std::endl;
        }
        
        // Test accessibility features
        std::cout << "Accessibility Features:" << std::endl;
        
        languages->setReadingSpeed(120.0f);
        std::cout << "  Reading speed: 120 words/min" << std::endl;
        
        languages->enableSimplifiedLanguage(true);
        std::cout << "  Simplified language: Enabled" << std::endl;
        
        languages->enableHighContrastMode(true);
        std::cout << "  High contrast mode: Enabled" << std::endl;
        
        languages->setLanguageForAccessibility("en-US");
        std::cout << "  Accessibility language: en-US" << std::endl;
        std::cout << std::endl;
    }
    
    void demonstrateVoiceCommands() {
        std::cout << "🎤 Voice Command Examples:" << std::endl;
        std::cout << "==========================" << std::endl;
        
        // Examples in different languages
        std::vector<std::vector<std::string>> language_commands = {
            // English
            {"increase volume", "decrease cutoff", "next preset", "start tutorial"},
            // Spanish
            {"aumentar volumen", "disminuir corte", "siguiente preset", "comenzar tutorial"},
            // French
            {"augmenter le volume", "diminuer la coupure", "preset suivant", "commencer tutoriel"},
            // German
            {"lautstärke erhöhen", "cutoff verringern", "nächstes preset", "tutorial starten"},
            // Japanese
            {"ボリュームを上げる", "カットオフを減らす", "次のプリセット", "チュートリアルを開始"}
        };
        
        std::vector<std::string> language_codes = {"en-US", "es-ES", "fr-FR", "de-DE", "ja-JP"};
        
        for (size_t lang_idx = 0; lang_idx < language_codes.size(); lang_idx++) {
            std::cout << language_codes[lang_idx] << ":" << std::endl;
            
            for (const auto& command : language_commands[lang_idx]) {
                std::cout << "  \"" << command << "\"" << std::endl;
            }
            std::cout << std::endl;
        }
    }
    
    void simulateVoiceTraining() {
        std::cout << "🎓 Voice Training Simulation:" << std::endl;
        std::cout << "=============================" << std::endl;
        
        // Simulate voice training for different users
        std::vector<std::string> user_profiles = {"user_001", "user_002", "user_003"};
        std::vector<std::string> training_phrases = {
            "increase volume",
            "decrease cutoff",
            "set resonance to fifty percent",
            "next preset",
            "go to filter section"
        };
        
        for (const auto& user_id : user_profiles) {
            std::cout << "Training for user: " << user_id << std::endl;
            
            // Start voice training
            voice_system_->startVoiceTraining(user_id);
            
            // Simulate adding voice samples
            std::uniform_int_distribution<int> phrase_dist(0, training_phrases.size() - 1);
            std::uniform_real_distribution<float> amplitude_dist(0.1f, 0.3f);
            
            for (int i = 0; i < 5; i++) {
                std::string phrase = training_phrases[phrase_dist(rng_)];
                int num_samples = 16000; // 1 second at 16kHz
                std::vector<float> audio_data(num_samples);
                
                // Generate synthetic audio data
                for (int j = 0; j < num_samples; j++) {
                    audio_data[j] = amplitude_dist(rng_) * std::sin(2.0 * M_PI * 440.0 * j / 16000.0);
                }
                
                voice_system_->addVoiceSample(phrase, audio_data);
                std::cout << "  Added sample: \"" << phrase << "\"" << std::endl;
            }
            
            std::cout << std::endl;
        }
    }
    
    void displayPerformanceMetrics() {
        std::cout << "📊 Performance Metrics:" << std::endl;
        std::cout << "=======================" << std::endl;
        
        auto stats = voice_system_->getStatistics();
        
        std::cout << "Recognition Performance:" << std::endl;
        std::cout << "  Total commands processed: " << stats.total_commands_processed << std::endl;
        std::cout << "  Successful commands: " << stats.successful_commands << std::endl;
        std::cout << "  Average confidence: " << (stats.average_recognition_confidence * 100) << "%" << std::endl;
        std::cout << "  Average response time: " << stats.average_response_time.count() << "ms" << std::endl;
        std::cout << std::endl;
        
        std::cout << "Language Support Metrics:" << std::endl;
        std::cout << "  Voice samples trained: " << stats.voice_samples_trained << std::endl;
        std::cout << "  Adaptation score: " << (stats.adaptation_score * 100) << "%" << std::endl;
        std::cout << "  Tutorial completions: " << stats.tutorial_completions << std::endl;
        std::cout << "  Preset navigations: " << stats.preset_navigations << std::endl;
        std::cout << std::endl;
    }
    
    void cleanup() {
        std::cout << "Cleaning up multi-language voice control system..." << std::endl;
        
        if (voice_system_) {
            // Save statistics
            voice_system_->saveConfiguration("multilang_demo_config.json");
            
            voice_system_->disableSystem();
            voice_system_->shutdown();
        }
        
        std::cout << "Demo completed successfully!" << std::endl;
    }
};

int main(int argc, char* argv[]) {
    try {
        MultiLanguageVoiceDemo demo;
        demo.run();
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
        
    } catch (...) {
        std::cerr << "Unknown exception occurred" << std::endl;
        return 1;
    }
}