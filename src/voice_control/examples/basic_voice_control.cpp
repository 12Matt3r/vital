/**
 * @file basic_voice_control.cpp
 * @brief Basic voice control demonstration for Vital synthesizer
 * 
 * This example demonstrates the core functionality of the Vital Voice Control System,
 * including voice command recognition, natural language processing, and parameter control.
 */

#include "../vital_voice_control.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <signal.h>
#include <atomic>

// Global flag for graceful shutdown
std::atomic<bool> g_running{true};

void signalHandler(int signal) {
    std::cout << "\nReceived shutdown signal..." << std::endl;
    g_running = false;
}

class BasicVoiceControlDemo {
public:
    BasicVoiceControlDemo() {
        // Set up signal handlers
        signal(SIGINT, signalHandler);
        signal(SIGTERM, signalHandler);
    }

    void run() {
        std::cout << "==================================================" << std::endl;
        std::cout << "   Vital Voice Control System - Basic Demo" << std::endl;
        std::cout << "==================================================" << std::endl;
        std::cout << std::endl;
        
        // Initialize voice control system
        if (!initializeVoiceSystem()) {
            std::cerr << "Failed to initialize voice control system!" << std::endl;
            return;
        }
        
        std::cout << "✅ Voice control system initialized successfully!" << std::endl;
        std::cout << std::endl;
        
        // Main interaction loop
        mainLoop();
        
        // Cleanup
        cleanup();
    }

private:
    std::unique_ptr<vital::voice_control::VitalVoiceControlSystem> voice_system_;
    vital::voice_control::VitalVoiceControlSystem::SystemConfiguration config_;
    
    bool initializeVoiceSystem() {
        // Configure the voice control system
        config_.enable_voice_commands = true;
        config_.enable_tutorials = true;
        config_.enable_preset_navigation = true;
        config_.enable_multilingual = true;
        config_.offline_mode = true;
        config_.default_language = "en-US";
        config_.master_volume = 0.8f;
        config_.max_concurrent_commands = 3;
        config_.enable_voice_training = true;
        config_.enable_adaptation = true;
        
        // Create voice control system
        voice_system_ = std::make_unique<vital::voice_control::VitalVoiceControlSystem>();
        
        // Initialize the system
        if (!voice_system_->initialize(config_)) {
            std::cerr << "Failed to initialize VitalVoiceControlSystem" << std::endl;
            return false;
        }
        
        // Set up callbacks
        setupCallbacks();
        
        // Enable the system
        voice_system_->enableSystem();
        
        return true;
    }
    
    void setupCallbacks() {
        // System state callback
        voice_system_->registerSystemStateCallback([this](vital::voice_control::VitalVoiceControlSystem::SystemState old_state,
                                                       vital::voice_control::VitalVoiceControlSystem::SystemState new_state) {
            std::cout << "[SYSTEM] State changed: " << static_cast<int>(old_state) 
                      << " -> " << static_cast<int>(new_state) << std::endl;
        });
        
        // Error callback
        voice_system_->registerErrorCallback([](const std::string& error) {
            std::cerr << "[ERROR] " << error << std::endl;
        });
        
        // Get component callbacks
        auto recognizer = voice_system_->getCommandRecognizer();
        auto controller = voice_system_->getLanguageController();
        auto tutorials = voice_system_->getTutorialSystem();
        auto navigator = voice_system_->getPresetNavigator();
        auto languages = voice_system_->getLanguageSupport();
        
        if (recognizer) {
            // Voice command callback
            recognizer->registerCommandCallback([](const vital::voice_control::VoiceCommandRecognizer::VoiceCommand& command) {
                std::cout << "[VOICE] Command: \"" << command.text << "\""
                          << " (confidence: " << command.confidence << ")"
                          << " (type: " << static_cast<int>(command.type) << ")" << std::endl;
            });
            
            // State change callback
            recognizer->registerStateChangeCallback([](vital::voice_control::VoiceCommandRecognizer::RecognitionState state) {
                std::cout << "[RECOGNIZER] State: " << static_cast<int>(state) << std::endl;
            });
        }
        
        if (controller) {
            // Parameter control callback
            controller->registerParameterCallback([](const std::string& parameter, float value) {
                std::cout << "[PARAMETER] " << parameter << " -> " << value << std::endl;
            });
            
            // Section change callback
            controller->registerSectionCallback([](const std::string& section) {
                std::cout << "[SECTION] Changed to: " << section << std::endl;
            });
        }
        
        if (tutorials) {
            // Tutorial progress callback
            tutorials->registerProgressCallback([](int current_step, int total_steps, vital::voice_control::VoiceTutorialSystem::TutorialState state) {
                std::cout << "[TUTORIAL] Progress: " << current_step << "/" << total_steps 
                          << " (state: " << static_cast<int>(state) << ")" << std::endl;
            });
        }
        
        if (navigator) {
            // Preset navigation callback
            navigator->registerNavigationCallback([](vital::voice_control::VoicePresetNavigator::NavigationDirection direction,
                                                     const vital::voice_control::VoicePresetNavigator::PresetInfo& preset) {
                std::cout << "[PRESET] Navigated: " << static_cast<int>(direction) 
                          << " -> " << preset.name << std::endl;
            });
        }
    }
    
    void mainLoop() {
        std::cout << "Voice Control System is now active!" << std::endl;
        std::cout << std::endl;
        std::cout << "Available Commands:" << std::endl;
        std::cout << "  - Parameter Control: 'increase volume', 'decrease cutoff', 'set resonance to 50%'" << std::endl;
        std::cout << "  - Preset Navigation: 'next preset', 'previous preset', 'load cosmic pad'" << std::endl;
        std::cout << "  - Tutorials: 'start basic tutorial', 'pause tutorial', 'resume tutorial'" << std::endl;
        std::cout << "  - Navigation: 'go to filter section', 'switch to oscillators'" << std::endl;
        std::cout << std::endl;
        std::cout << "Multi-language Support:" << std::endl;
        std::cout << "  - Spanish: 'aumentar volumen', 'siguiente preset'" << std::endl;
        std::cout << "  - French: 'augmenter le volume', 'preset suivant'" << std::endl;
        std::cout << "  - German: 'lautstärke erhöhen', 'nächstes preset'" << std::endl;
        std::cout << std::endl;
        std::cout << "Press Ctrl+C to exit" << std::endl;
        std::cout << std::endl;
        
        // Display system information
        displaySystemInfo();
        
        // Main loop
        while (g_running) {
            // Simulate audio processing (in a real application, this would be called from the audio callback)
            simulateAudioProcessing();
            
            // Check system status periodically
            static auto last_status_check = std::chrono::steady_clock::now();
            auto now = std::chrono::steady_clock::now();
            
            if (std::chrono::duration_cast<std::chrono::seconds>(now - last_status_check).count() >= 5) {
                displaySystemStatus();
                last_status_check = now;
            }
            
            // Sleep to prevent excessive CPU usage
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }
    
    void simulateAudioProcessing() {
        // Simulate audio buffer processing
        // In a real application, this would be called from Vital's audio callback
        static int sample_count = 0;
        
        // Generate a simple sine wave for demo (440 Hz)
        const int sample_rate = 44100;
        const int buffer_size = 1024;
        std::vector<float> audio_buffer(buffer_size);
        
        for (int i = 0; i < buffer_size; i++) {
            double t = (sample_count + i) / static_cast<double>(sample_rate);
            audio_buffer[i] = 0.1f * std::sin(2.0 * M_PI * 440.0 * t); // 440 Hz sine wave
        }
        
        sample_count += buffer_size;
        
        // Process audio with voice control system
        if (voice_system_ && voice_system_->isSystemEnabled()) {
            voice_system_->processVitalAudioBuffer(audio_buffer.data(), buffer_size, 1);
        }
    }
    
    void displaySystemInfo() {
        std::cout << "System Configuration:" << std::endl;
        std::cout << "  Voice Commands: " << (config_.enable_voice_commands ? "Enabled" : "Disabled") << std::endl;
        std::cout << "  Tutorials: " << (config_.enable_tutorials ? "Enabled" : "Disabled") << std::endl;
        std::cout << "  Preset Navigation: " << (config_.enable_preset_navigation ? "Enabled" : "Disabled") << std::endl;
        std::cout << "  Multi-language: " << (config_.enable_multilingual ? "Enabled" : "Disabled") << std::endl;
        std::cout << "  Offline Mode: " << (config_.offline_mode ? "Enabled" : "Disabled") << std::endl;
        std::cout << "  Default Language: " << config_.default_language << std::endl;
        std::cout << "  Voice Training: " << (config_.enable_voice_training ? "Enabled" : "Disabled") << std::endl;
        std::cout << "  Voice Adaptation: " << (config_.enable_adaptation ? "Enabled" : "Disabled") << std::endl;
        std::cout << std::endl;
        
        // Display supported languages
        auto languages = voice_system_->getLanguageSupport();
        if (languages) {
            auto supported = languages->getSupportedLanguages();
            std::cout << "Supported Languages (" << supported.size() << "):" << std::endl;
            for (size_t i = 0; i < std::min(supported.size(), static_cast<size_t>(8)); i++) {
                std::cout << "  - " << supported[i].name << " (" << supported[i].code << ")" << std::endl;
            }
            if (supported.size() > 8) {
                std::cout << "  ... and " << (supported.size() - 8) << " more" << std::endl;
            }
            std::cout << std::endl;
        }
    }
    
    void displaySystemStatus() {
        auto stats = voice_system_->getStatistics();
        
        std::cout << "System Statistics:" << std::endl;
        std::cout << "  Commands Processed: " << stats.total_commands_processed << std::endl;
        std::cout << "  Success Rate: " << (stats.total_commands_processed > 0 ? 
            (100.0f * stats.successful_commands / stats.total_commands_processed) : 0.0f) << "%" << std::endl;
        std::cout << "  Average Confidence: " << stats.average_recognition_confidence << std::endl;
        std::cout << "  Tutorial Completions: " << stats.tutorial_completions << std::endl;
        std::cout << "  Preset Navigations: " << stats.preset_navigations << std::endl;
        std::cout << "  Voice Samples Trained: " << stats.voice_samples_trained << std::endl;
        std::cout << std::endl;
        
        // Display system state
        auto state = voice_system_->getCurrentState();
        std::cout << "Current State: " << static_cast<int>(state) << std::endl;
        std::cout << std::endl;
    }
    
    void cleanup() {
        std::cout << "Shutting down voice control system..." << std::endl;
        
        if (voice_system_) {
            voice_system_->disableSystem();
            voice_system_->shutdown();
        }
        
        std::cout << "Voice control system stopped." << std::endl;
    }
};

int main(int argc, char* argv[]) {
    try {
        BasicVoiceControlDemo demo;
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