/**
 * @file tutorial_system_demo.cpp
 * @brief Voice-guided tutorial system demonstration
 * 
 * This example demonstrates the interactive voice-guided tutorial system
 * for learning Vital synthesizer controls and techniques.
 */

#include "../vital_voice_control.h"
#include <iostream>
#include <thread>
#include <chrono>
#include <map>

class VoiceTutorialDemo {
public:
    VoiceTutorialDemo() {
        setupDemoScenarios();
    }

    void run() {
        std::cout << "==================================================" << std::endl;
        std::cout << "   Vital Voice Control - Tutorial System Demo" << std::endl;
        std::cout << "==================================================" << std::endl;
        std::cout << std::endl;
        
        if (!initializeVoiceSystem()) {
            std::cerr << "Failed to initialize voice control system!" << std::endl;
            return;
        }
        
        std::cout << "✅ Tutorial system initialized!" << std::endl;
        std::cout << std::endl;
        
        showMainMenu();
        runInteractiveDemo();
        
        cleanup();
    }

private:
    std::unique_ptr<vital::voice_control::VitalVoiceControlSystem> voice_system_;
    std::map<std::string, std::string> demo_scenarios_;
    bool tutorial_active_ = false;
    int current_scenario_ = 0;
    
    void setupDemoScenarios() {
        demo_scenarios_ = {
            {"1", "basic_operations", "Basic synthesizer operations"},
            {"2", "parameter_control", "Precise parameter control"},
            {"3", "preset_management", "Preset navigation and management"},
            {"4", "effects_processing", "Effects and sound design"},
            {"5", "synthesis_techniques", "Advanced synthesis methods"},
            {"6", "full_demo", "Complete interactive tutorial"},
            {"7", "adaptive_learning", "Adaptive tutorial demonstration"}
        };
    }
    
    bool initializeVoiceSystem() {
        vital::voice_control::VitalVoiceControlSystem::SystemConfiguration config;
        config.enable_voice_commands = true;
        config.enable_tutorials = true;
        config.enable_preset_navigation = true;
        config.enable_multilingual = true;
        config.offline_mode = true;
        config.enable_voice_training = true;
        config.enable_adaptation = true;
        
        voice_system_ = std::make_unique<vital::voice_control::VitalVoiceControlSystem>();
        
        if (!voice_system_->initialize(config)) {
            return false;
        }
        
        setupTutorialCallbacks();
        voice_system_->enableSystem();
        return true;
    }
    
    void setupTutorialCallbacks() {
        auto tutorials = voice_system_->getTutorialSystem();
        if (!tutorials) return;
        
        // Tutorial step callback
        tutorials->registerTutorialCallback([this](vital::voice_control::VoiceTutorialSystem::TutorialStep step,
                                                  vital::voice_control::VoiceTutorialSystem::TutorialState state) {
            handleTutorialStep(step, state);
        });
        
        // Progress callback
        tutorials->registerProgressCallback([this](int current_step, int total_steps, 
                                                   vital::voice_control::VoiceTutorialSystem::TutorialState state) {
            handleTutorialProgress(current_step, total_steps, state);
        });
        
        // Enable voice guidance
        tutorials->enableVoiceGuidance(true);
        tutorials->setVoiceSpeed(1.0f);
        tutorials->setVoiceVolume(0.8f);
    }
    
    void showMainMenu() {
        std::cout << "Available Tutorial Scenarios:" << std::endl;
        std::cout << "=============================" << std::endl;
        
        for (const auto& [key, scenario] : demo_scenarios_) {
            std::cout << key << ". " << scenario.second << std::endl;
        }
        
        std::cout << std::endl;
    }
    
    void runInteractiveDemo() {
        while (true) {
            std::cout << "Select a tutorial scenario (1-7) or 'q' to quit: ";
            std::string choice;
            std::getline(std::cin, choice);
            
            if (choice == "q" || choice == "Q") {
                break;
            }
            
            if (choice == "basic_operations" || choice == "1") {
                runBasicOperationsDemo();
            } else if (choice == "parameter_control" || choice == "2") {
                runParameterControlDemo();
            } else if (choice == "preset_management" || choice == "3") {
                runPresetManagementDemo();
            } else if (choice == "effects_processing" || choice == "4") {
                runEffectsProcessingDemo();
            } else if (choice == "synthesis_techniques" || choice == "5") {
                runSynthesisTechniquesDemo();
            } else if (choice == "full_demo" || choice == "6") {
                runFullDemo();
            } else if (choice == "adaptive_learning" || choice == "7") {
                runAdaptiveLearningDemo();
            } else {
                std::cout << "Invalid choice. Please select 1-7 or 'q' to quit." << std::endl;
                std::cout << std::endl;
            }
        }
    }
    
    void runBasicOperationsDemo() {
        std::cout << "🎹 Basic Operations Tutorial" << std::endl;
        std::cout << "=============================" << std::endl;
        
        auto tutorials = voice_system_->getTutorialSystem();
        if (!tutorials) return;
        
        tutorials->enableAdaptiveTutorials(true);
        tutorials->setUserSkillLevel(0.3f); // Beginner level
        
        // Start basic operations tutorial
        tutorials->startTutorial(vital::voice_control::VoiceTutorialSystem::TutorialType::BasicOperations);
        tutorial_active_ = true;
        
        simulateTutorialExecution("basic_operations");
        
        tutorials->stopTutorial();
        tutorial_active_ = false;
    }
    
    void runParameterControlDemo() {
        std::cout << "🎛️  Parameter Control Tutorial" << std::endl;
        std::cout << "================================" << std::endl;
        
        auto tutorials = voice_system_->getTutorialSystem();
        if (!tutorials) return;
        
        tutorials->enableAdaptiveTutorials(true);
        tutorials->setUserSkillLevel(0.5f); // Intermediate level
        
        tutorials->startTutorial(vital::voice_control::VoiceTutorialSystem::TutorialType::ParameterControl);
        tutorial_active_ = true;
        
        simulateTutorialExecution("parameter_control");
        
        tutorials->stopTutorial();
        tutorial_active_ = false;
    }
    
    void runPresetManagementDemo() {
        std::cout << "🎵 Preset Management Tutorial" << std::endl;
        std::cout << "===============================" << std::endl;
        
        auto tutorials = voice_system_->getTutorialSystem();
        if (!tutorials) return;
        
        tutorials->enableAdaptiveTutorials(true);
        tutorials->setUserSkillLevel(0.4f); // Beginner-intermediate
        
        tutorials->startTutorial(vital::voice_control::VoiceTutorialSystem::TutorialType::PresetManagement);
        tutorial_active_ = true;
        
        simulateTutorialExecution("preset_management");
        
        tutorials->stopTutorial();
        tutorial_active_ = false;
    }
    
    void runEffectsProcessingDemo() {
        std::cout << "✨ Effects Processing Tutorial" << std::endl;
        std::cout << "===============================" << std::endl;
        
        auto tutorials = voice_system_->getTutorialSystem();
        if (!tutorials) return;
        
        tutorials->enableAdaptiveTutorials(true);
        tutorials->setUserSkillLevel(0.6f); // Intermediate-advanced
        
        tutorials->startTutorial(vital::voice_control::VoiceTutorialSystem::TutorialType::EffectsProcessing);
        tutorial_active_ = true;
        
        simulateTutorialExecution("effects_processing");
        
        tutorials->stopTutorial();
        tutorial_active_ = false;
    }
    
    void runSynthesisTechniquesDemo() {
        std::cout << "🔬 Advanced Synthesis Tutorial" << std::endl;
        std::cout << "================================" << std::endl;
        
        auto tutorials = voice_system_->getTutorialSystem();
        if (!tutorials) return;
        
        tutorials->enableAdaptiveTutorials(true);
        tutorials->setUserSkillLevel(0.8f); // Advanced level
        
        tutorials->startTutorial(vital::voice_control::VoiceTutorialSystem::TutorialType::SynthesisTechniques);
        tutorial_active_ = true;
        
        simulateTutorialExecution("synthesis_techniques");
        
        tutorials->stopTutorial();
        tutorial_active_ = false;
    }
    
    void runFullDemo() {
        std::cout << "🎯 Complete Interactive Tutorial" << std::endl;
        std::cout << "=================================" << std::endl;
        
        auto tutorials = voice_system_->getTutorialSystem();
        if (!tutorials) return;
        
        // Demonstrate contextual tutorial creation
        std::vector<std::string> contexts = {"filter control", "oscillator tuning", "envelope shaping"};
        
        for (const auto& context : contexts) {
            std::cout << "Creating contextual tutorial for: " << context << std::endl;
            tutorials->createContextualTutorial(context);
            
            std::cout << "Tutorial title: " << tutorials->getCurrentTutorialTitle() << std::endl;
            std::cout << "Total steps: " << tutorials->getTotalSteps() << std::endl;
            std::cout << std::endl;
        }
        
        // Run adaptive learning demonstration
        runAdaptiveLearningDemo();
    }
    
    void runAdaptiveLearningDemo() {
        std::cout << "🧠 Adaptive Learning Demonstration" << std::endl;
        std::cout << "===================================" << std::endl;
        
        auto tutorials = voice_system_->getTutorialSystem();
        if (!tutorials) return;
        
        tutorials->enableAdaptiveTutorials(true);
        
        // Simulate different user skill levels
        std::vector<float> skill_levels = {0.2f, 0.5f, 0.8f};
        std::vector<std::string> skill_descriptions = {"Beginner", "Intermediate", "Advanced"};
        
        for (size_t i = 0; i < skill_levels.size(); i++) {
            std::cout << "Simulating " << skill_descriptions[i] << " user (skill level: " 
                      << skill_levels[i] << ")" << std::endl;
            
            tutorials->setUserSkillLevel(skill_levels[i]);
            
            // Simulate user behavior analysis
            std::vector<std::string> user_actions = {
                "voice command", "parameter adjustment", "preset navigation", "help request"
            };
            
            for (const auto& action : user_actions) {
                tutorials->analyzeUserBehavior(action);
                std::cout << "  Analyzed action: " << action << std::endl;
            }
            
            // Get updated skill level
            float updated_skill = tutorials->getUserSkillLevel();
            std::cout << "  Updated skill level: " << updated_skill << std::endl;
            std::cout << std::endl;
        }
    }
    
    void simulateTutorialExecution(const std::string& tutorial_type) {
        auto tutorials = voice_system_->getTutorialSystem();
        if (!tutorials) return;
        
        std::cout << "Simulating tutorial execution..." << std::endl;
        
        // Simulate tutorial progression
        for (int step = 0; step < tutorials->getTotalSteps(); step++) {
            std::cout << "Step " << (step + 1) << "/" << tutorials->getTotalSteps() << std::endl;
            
            // Simulate user interaction
            simulateUserInteraction(tutorial_type, step);
            
            // Progress tutorial
            tutorials->analyzeUserBehavior("completed_step");
            
            // Simulate step completion delay
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            
            if (!tutorial_active_) break;
        }
        
        std::cout << "Tutorial completed!" << std::endl;
        std::cout << std::endl;
    }
    
    void simulateUserInteraction(const std::string& tutorial_type, int step) {
        std::vector<std::string> example_commands;
        
        if (tutorial_type == "basic_operations") {
            example_commands = {
                "increase volume",
                "decrease volume", 
                "go to filter section",
                "increase cutoff"
            };
        } else if (tutorial_type == "parameter_control") {
            example_commands = {
                "set volume to fifty percent",
                "set cutoff to maximum",
                "increase resonance by twenty percent"
            };
        } else if (tutorial_type == "preset_management") {
            example_commands = {
                "next preset",
                "previous preset",
                "load cosmic pad"
            };
        } else if (tutorial_type == "effects_processing") {
            example_commands = {
                "go to effects section",
                "increase delay time",
                "add reverb"
            };
        }
        
        if (!example_commands.empty()) {
            std::string command = example_commands[step % example_commands.size()];
            std::cout << "User says: \"" << command << "\"" << std::endl;
            
            // Simulate voice recognition
            auto recognizer = voice_system_->getCommandRecognizer();
            if (recognizer) {
                vital::voice_control::VoiceCommandRecognizer::VoiceCommand cmd;
                cmd.text = command;
                cmd.type = vital::voice_control::VoiceCommandRecognizer::CommandType::ParameterControl;
                cmd.confidence = 0.9f;
                cmd.is_valid = true;
                cmd.language = "en-US";
                cmd.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now().time_since_epoch());
                
                recognizer->registerCommandCallback([](const vital::voice_control::VoiceCommandRecognizer::VoiceCommand&) {
                    // Command processed successfully
                });
            }
            
            // Simulate parameter changes
            auto controller = voice_system_->getLanguageController();
            if (controller) {
                if (command.find("volume") != std::string::npos) {
                    controller->processParameterCommand("master_volume", 0.5f);
                } else if (command.find("cutoff") != std::string::npos) {
                    controller->processParameterCommand("filter_cutoff", 0.7f);
                }
            }
        }
    }
    
    void handleTutorialStep(vital::voice_control::VoiceTutorialSystem::TutorialStep step,
                           vital::voice_control::VoiceTutorialSystem::TutorialState state) {
        if (state == vital::voice_control::VoiceTutorialSystem::TutorialState::InProgress) {
            std::cout << "🎓 Tutorial Step " << step.step_id << ": " << step.title << std::endl;
            std::cout << "   Instruction: " << step.instruction_text << std::endl;
            if (!step.voice_instruction.empty()) {
                std::cout << "   Voice: " << step.voice_instruction << std::endl;
            }
            std::cout << std::endl;
        } else if (state == vital::voice_control::VoiceTutorialSystem::TutorialState::Completed) {
            std::cout << "✅ Step " << step.step_id << " completed!" << std::endl;
            std::cout << std::endl;
        }
    }
    
    void handleTutorialProgress(int current_step, int total_steps, 
                               vital::voice_control::VoiceTutorialSystem::TutorialState state) {
        float progress = (total_steps > 0) ? (100.0f * current_step / total_steps) : 0.0f;
        
        std::cout << "[PROGRESS] " << current_step << "/" << total_steps 
                  << " (" << std::fixed << std::setprecision(1) << progress << "%)" << std::endl;
        
        if (state == vital::voice_control::VoiceTutorialSystem::TutorialState::Completed) {
            std::cout << "🎉 Tutorial completed successfully!" << std::endl;
            
            // Show tutorial statistics
            showTutorialStatistics();
        } else if (state == vital::voice_control::VoiceTutorialSystem::TutorialState::Paused) {
            std::cout << "⏸️  Tutorial paused" << std::endl;
        } else if (state == vital::voice_control::VoiceTutorialSystem::TutorialState::Skipped) {
            std::cout << "⏭️  Tutorial step skipped" << std::endl;
        }
        
        std::cout << std::endl;
    }
    
    void showTutorialStatistics() {
        auto tutorials = voice_system_->getTutorialSystem();
        if (!tutorials) return;
        
        std::cout << "Tutorial Statistics:" << std::endl;
        std::cout << "  Current tutorial: " << tutorials->getCurrentTutorialTitle() << std::endl;
        std::cout << "  Tutorial type: " << static_cast<int>(tutorials->getCurrentTutorialType()) << std::endl;
        std::cout << "  Total steps: " << tutorials->getTotalSteps() << std::endl;
        std::cout << "  Voice guidance: " << (tutorials->isVoiceGuidanceEnabled() ? "Enabled" : "Disabled") << std::endl;
        std::cout << "  Voice speed: " << tutorials->getVoiceSpeed() << std::endl;
        std::cout << "  Voice volume: " << tutorials->getVoiceVolume() << std::endl;
        std::cout << "  Adaptive tutorials: Enabled" << std::endl;
        std::cout << "  User skill level: " << tutorials->getUserSkillLevel() << std::endl;
        std::cout << std::endl;
    }
    
    void demonstrateCustomTutorials() {
        std::cout << "📚 Custom Tutorial Creation" << std::endl;
        std::cout << "============================" << std::endl;
        
        auto tutorials = voice_system_->getTutorialSystem();
        if (!tutorials) return;
        
        // Create a custom tutorial
        vital::voice_control::VoiceTutorialSystem::TutorialDefinition custom_tutorial;
        custom_tutorial.type = vital::voice_control::VoiceTutorialSystem::TutorialType::Custom;
        custom_tutorial.title = "My Custom Vital Tutorial";
        custom_tutorial.description = "Learn custom synthesis techniques";
        custom_tutorial.author = "Voice Control Demo";
        custom_tutorial.version = "1.0";
        custom_tutorial.difficulty_level = "Advanced";
        custom_tutorial.requires_voice_guidance = true;
        
        // Add custom steps
        std::vector<std::string> custom_steps = {
            "Welcome to your custom tutorial!",
            "First, let's set up a basic patch",
            "Adjust the oscillators to create harmony",
            "Use the filter to shape the sound",
            "Add modulation for movement",
            "Apply effects for the finishing touch"
        };
        
        for (size_t i = 0; i < custom_steps.size(); i++) {
            vital::voice_control::VoiceTutorialSystem::TutorialStep step;
            step.step_id = static_cast<int>(i + 1);
            step.title = "Custom Step " + std::to_string(i + 1);
            step.instruction_text = custom_steps[i];
            step.voice_instruction = custom_steps[i];
            step.is_optional = false;
            step.estimated_duration_seconds = 45;
            
            custom_tutorial.steps.push_back(step);
        }
        
        // Load the custom tutorial
        tutorials->loadTutorial(custom_tutorial);
        
        std::cout << "Custom tutorial created!" << std::endl;
        std::cout << "  Title: " << custom_tutorial.title << std::endl;
        std::cout << "  Steps: " << custom_tutorial.steps.size() << std::endl;
        std::cout << "  Difficulty: " << custom_tutorial.difficulty_level << std::endl;
        std::cout << std::endl;
        
        // Add additional custom step
        vital::voice_control::VoiceTutorialSystem::TutorialStep extra_step;
        extra_step.step_id = static_cast<int>(custom_steps.size() + 1);
        extra_step.title = "Extra Tips";
        extra_step.instruction_text = "Here are some additional tips and tricks";
        extra_step.voice_instruction = "Listen for these helpful tips";
        extra_step.is_optional = true;
        extra_step.estimated_duration_seconds = 30;
        
        tutorials->addCustomStep(extra_step);
        
        std::cout << "Added extra step!" << std::endl;
        std::cout << "  Total steps: " << tutorials->getTotalSteps() << std::endl;
        std::cout << std::endl;
    }
    
    void cleanup() {
        std::cout << "Cleaning up tutorial system..." << std::endl;
        
        if (voice_system_) {
            voice_system_->saveConfiguration("tutorial_demo_config.json");
            voice_system_->disableSystem();
            voice_system_->shutdown();
        }
        
        std::cout << "Tutorial demo completed!" << std::endl;
    }
};

int main(int argc, char* argv[]) {
    try {
        VoiceTutorialDemo demo;
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