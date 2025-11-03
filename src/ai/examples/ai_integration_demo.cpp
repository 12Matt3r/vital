#include "ai_manager.h"
#include "neural_preset_generator.h"
#include "style_transfer_engine.h"
#include "adaptive_modulation_system.h"
#include "machine_learning_engine.h"
#include "intelligent_audio_analyzer.h"
#include "intelligent_preset_generator.h"

#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <random>
#include <fstream>
#include <sstream>

using namespace vital;

// Demo application class that showcases all AI features
class VitalAIDemo {
public:
    VitalAIDemo() : ai_manager_(nullptr) {}
    
    void run() {
        std::cout << "=== Vital AI Integration System Demo ===\n\n";
        
        // Initialize the AI system
        if (!initializeAI()) {
            std::cerr << "Failed to initialize AI system\n";
            return;
        }
        
        // Run comprehensive demo
        demonstrateNeuralPresetGeneration();
        demonstrateStyleTransfer();
        demonstrateAdaptiveModulation();
        demonstrateMachineLearning();
        demonstrateAudioAnalysis();
        demonstrateIntelligentPresetGeneration();
        demonstrateRealTimeProcessing();
        demonstrateUserLearning();
        demonstratePerformanceMonitoring();
        
        std::cout << "\n=== Demo Complete ===\n";
        std::cout << "All AI features have been demonstrated successfully!\n";
    }
    
private:
    AIManager* ai_manager_;
    std::unique_ptr<NeuralPresetGenerator> neural_generator_;
    std::unique_ptr<StyleTransferEngine> style_engine_;
    std::unique_ptr<AdaptiveModulationSystem> modulation_system_;
    std::unique_ptr<MachineLearningEngine> ml_engine_;
    std::unique_ptr<IntelligentAudioAnalyzer> audio_analyzer_;
    std::unique_ptr<IntelligentPresetGenerator> preset_generator_;
    
    bool initializeAI() {
        std::cout << "Initializing AI Integration System...\n";
        
        try {
            // Create AI manager
            ai_manager_ = new AIManager();
            if (!ai_manager_->initialize()) {
                std::cerr << "Failed to initialize AI manager\n";
                return false;
            }
            
            // Create AI components
            neural_generator_ = std::make_unique<NeuralPresetGenerator>(ai_manager_);
            style_engine_ = std::make_unique<StyleTransferEngine>(ai_manager_);
            modulation_system_ = std::make_unique<AdaptiveModulationSystem>(ai_manager_);
            ml_engine_ = std::make_unique<MachineLearningEngine>();
            audio_analyzer_ = std::make_unique<IntelligentAudioAnalyzer>(ai_manager_);
            preset_generator_ = std::make_unique<IntelligentPresetGenerator>(ai_manager_);
            
            // Configure components
            configureComponents();
            
            std::cout << "✓ AI Integration System initialized successfully\n\n";
            return true;
            
        } catch (const std::exception& e) {
            std::cerr << "Initialization error: " << e.what() << "\n";
            return false;
        }
    }
    
    void configureComponents() {
        // Configure neural preset generator
        NeuralPresetGenerator::NetworkConfig config;
        config.layer_sizes = {32, 64, 64, 64, NeuralPresetGenerator::Preset::PARAMETER_COUNT};
        config.activation_functions = {"relu", "relu", "relu", "tanh"};
        config.learning_rate = 0.001f;
        config.dropout_rate = 0.2f;
        neural_generator_->setNetworkConfig(config);
        neural_generator_->setGenerationMode(NeuralPresetGenerator::GenerationMode::Stochastic);
        
        // Configure style transfer engine
        StyleTransferEngine::TransferConfig transfer_config;
        transfer_config.mode = StyleTransferEngine::TransferConfig::Hybrid;
        transfer_config.style_intensity = 0.8f;
        transfer_config.preserve_structure = true;
        transfer_config.quality_threshold = 0.7f;
        style_engine_->setTransferConfig(transfer_config);
        
        // Configure adaptive modulation
        modulation_system_->setLearningRate(0.01f);
        modulation_system_->setAdaptationSpeed(0.1f);
        modulation_system_->enableLearning(true);
        modulation_system_->setUserExperienceLevel(0.5f);
        
        // Configure audio analyzer
        IntelligentAudioAnalyzer::AnalysisConfig analysis_config;
        analysis_config.sample_rate = 44100;
        analysis_config.frame_size = 1024;
        analysis_config.hop_size = 512;
        analysis_config.enable_mfcc = true;
        analysis_config.enable_chroma = true;
        analysis_config.enable_pitch_tracking = true;
        analysis_config.enable_classification = true;
        audio_analyzer_->setAnalysisConfig(analysis_config);
        
        // Configure intelligent preset generator
        IntelligentPresetGenerator::GenerationSettings gen_settings;
        gen_settings.target_genre = IntelligentPresetGenerator::MusicalGenre::Electronic;
        gen_settings.category = IntelligentPresetGenerator::PresetCategory::Bass;
        gen_settings.complexity_level = 0.5f;
        gen_settings.learn_from_user = true;
        gen_settings.use_neural_networks = true;
        preset_generator_->setGenerationSettings(gen_settings);
    }
    
    void demonstrateNeuralPresetGeneration() {
        std::cout << "=== Neural Preset Generation Demo ===\n";
        
        // Generate various presets
        std::vector<NeuralPresetGenerator::Preset> presets;
        
        for (int i = 0; i < 5; ++i) {
            auto preset = neural_generator_->generatePreset();
            presets.push_back(preset);
            
            std::cout << "Preset " << (i+1) << ": " << preset.name 
                     << " (Quality: " << preset.similarity_score << ")\n";
        }
        
        // Learn from user feedback
        std::cout << "\nLearning from user feedback...\n";
        for (size_t i = 0; i < presets.size(); ++i) {
            float rating = (i % 2 == 0) ? 0.8f : 0.4f; // Simulated feedback
            neural_generator_->learnFromUserFeedback(presets[i], rating);
        }
        
        // Generate batch
        auto batch_presets = neural_generator_->generateBatch(3);
        std::cout << "Generated batch of " << batch_presets.size() << " presets\n";
        
        auto stats = neural_generator_->getStats();
        std::cout << "Total generated: " << stats.total_generated 
                 << " (Success rate: " << (float)stats.successful_generations / stats.total_generated << ")\n";
        
        std::cout << "\n";
    }
    
    void demonstrateStyleTransfer() {
        std::cout << "=== Style Transfer Engine Demo ===\n";
        
        // Create a simple audio signal for demonstration
        std::vector<float> input_audio(1024);
        for (size_t i = 0; i < input_audio.size(); ++i) {
            input_audio[i] = std::sin(2.0f * M_PI * i / input_audio.size() * 50.0f) * 0.5f;
        }
        
        // Create style profiles
        StyleTransferEngine::Style style1, style2;
        style1.name = "Warm Analog";
        style1.intensity = 0.8f;
        style1.spectral_profile = std::vector<float>(128, 0.7f);
        style1.harmonic_profile = std::vector<float>(128, 0.6f);
        
        style2.name = "Digital Crystal";
        style2.intensity = 0.9f;
        style2.spectral_profile = std::vector<float>(128, 0.9f);
        style2.harmonic_profile = std::vector<float>(128, 0.8f);
        
        style_engine_->addStyle(style1);
        style_engine_->addStyle(style2);
        
        // Apply style transfer
        std::cout << "Applying style transfer...\n";
        auto transferred_audio = style_engine_->transferStyle(input_audio, "Warm Analog");
        std::cout << "✓ Transferred to 'Warm Analog' style\n";
        
        transferred_audio = style_engine_->transferStyle(input_audio, "Digital Crystal");
        std::cout << "✓ Transferred to 'Digital Crystal' style\n";
        
        // Demonstrate multi-style blending
        std::vector<std::pair<std::string, float>> style_weights = {
            {"Warm Analog", 0.6f},
            {"Digital Crystal", 0.4f}
        };
        
        auto blended_audio = style_engine_->blendStyles(input_audio, style_weights);
        std::cout << "✓ Applied style blending\n";
        
        // Quality assessment
        auto quality = style_engine_->assessTransferQuality(input_audio, transferred_audio);
        std::cout << "Transfer quality - Similarity: " << quality.similarity_score 
                 << ", Preservation: " << quality.preservation_score << "\n";
        
        auto stats = style_engine_->getStats();
        std::cout << "Blocks processed: " << stats.blocks_processed 
                 << " (Success rate: " << (float)stats.successful_transfers / stats.blocks_processed << ")\n";
        
        std::cout << "\n";
    }
    
    void demonstrateAdaptiveModulation() {
        std::cout << "=== Adaptive Modulation System Demo ===\n";
        
        // Create modulations
        auto lfo_id = modulation_system_->createModulation("cutoff", 
            AdaptiveModulationSystem::ModulationType::LFO);
        auto env_id = modulation_system_->createModulation("amplitude",
            AdaptiveModulationSystem::ModulationType::Envelope);
        auto random_id = modulation_system_->createModulation("detune",
            AdaptiveModulationSystem::ModulationType::Random);
        
        std::cout << "Created 3 modulations (IDs: " << lfo_id << ", " << env_id << ", " << random_id << ")\n";
        
        // Process modulations in real-time
        std::cout << "Processing modulations...\n";
        float base_cutoff = 0.5f;
        float base_amplitude = 1.0f;
        float base_detune = 0.0f;
        
        for (int i = 0; i < 10; ++i) {
            base_cutoff = modulation_system_->processModulation(lfo_id, base_cutoff);
            base_amplitude = modulation_system_->processModulation(env_id, base_amplitude);
            base_detune = modulation_system_->processModulation(random_id, base_detune);
            
            if (i % 3 == 0) {
                std::cout << "Step " << i << ": cutoff=" << base_cutoff 
                         << ", amp=" << base_amplitude << ", detune=" << base_detune << "\n";
            }
        }
        
        // Create and apply pattern
        auto pattern_id = modulation_system_->createPattern("Bass Modulation");
        modulation_system_->addModulationToPattern(pattern_id, lfo_id);
        modulation_system_->addModulationToPattern(pattern_id, env_id);
        
        std::cout << "Applied modulation pattern\n";
        
        // Demonstrate learning
        modulation_system_->learnFromUserAction("parameter_cutoff_increase", 0.8f);
        modulation_system_->learnFromUserAction("parameter_amplitude_decrease", 0.6f);
        
        // Get suggestions
        auto suggestions = modulation_system_->suggestModulations("filter_resonance", 
            audio_analyzer_->getRealTimeMetrics().current_class != IntelligentAudioAnalyzer::AudioClass::Unknown ?
            audio_analyzer_->getRealTimeMetrics().current_class :
            IntelligentAudioAnalyzer::AudioClass::Music);
        
        std::cout << "Generated " << suggestions.size() << " modulation suggestions\n";
        
        auto stats = modulation_system_->getStats();
        std::cout << "Total modulations used: " << stats.total_modulations_used 
                 << " (Avg satisfaction: " << stats.average_user_satisfaction << ")\n";
        
        std::cout << "\n";
    }
    
    void demonstrateMachineLearning() {
        std::cout << "=== Machine Learning Engine Demo ===\n";
        
        // Create training data
        MachineLearningEngine::DataSet training_data;
        MachineLearningEngine::DataSet test_data;
        MachineLearningEngine::Labels training_labels;
        MachineLearningEngine::Labels test_labels;
        
        // Generate synthetic data
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        
        for (int i = 0; i < 100; ++i) {
            MachineLearningEngine::DataPoint sample(5);
            for (float& val : sample) val = dist(gen);
            
            float label = (sample[0] + sample[1]) > 1.0f ? 1.0f : 0.0f;
            
            if (i < 80) {
                training_data.push_back(sample);
                training_labels.push_back(label);
            } else {
                test_data.push_back(sample);
                test_labels.push_back(label);
            }
        }
        
        // Create and train neural network
        auto nn_model = ml_engine_->createModel(MachineLearningEngine::ModelType::NeuralNetwork, "demo_nn");
        std::cout << "Training neural network...\n";
        bool success = nn_model->train(training_data, training_labels);
        std::cout << "Training " << (success ? "successful" : "failed") << "\n";
        
        // Test predictions
        std::cout << "Testing predictions:\n";
        for (size_t i = 0; i < std::min(size_t(5), test_data.size()); ++i) {
            float prediction = nn_model->predictSingle(test_data[i]);
            float actual = test_labels[i];
            std::cout << "Sample " << i << ": predicted=" << prediction 
                     << ", actual=" << actual << "\n";
        }
        
        // Create clustering model
        auto kmeans_model = ml_engine_->createModel(MachineLearningEngine::ModelType::KMeansClustering, "demo_kmeans");
        std::cout << "Training K-means clustering...\n";
        success = kmeans_model->train(training_data, training_labels);
        std::cout << "Clustering training " << (success ? "successful" : "failed") << "\n";
        
        // Feature selection demonstration
        auto important_features = ml_engine_->selectBestFeatures(training_data, training_labels, 3);
        std::cout << "Most important features: ";
        for (size_t idx : important_features) {
            std::cout << idx << " ";
        }
        std::cout << "\n";
        
        auto stats = ml_engine_->getStats();
        std::cout << "ML Engine stats - Models created: " << stats.total_models_created 
                 << ", Active models: " << stats.active_models << "\n";
        
        std::cout << "\n";
    }
    
    void demonstrateAudioAnalysis() {
        std::cout << "=== Intelligent Audio Analysis Demo ===\n";
        
        // Generate test audio (simple sine wave with harmonics)
        std::vector<float> test_audio(2048);
        for (size_t i = 0; i < test_audio.size(); ++i) {
            float t = static_cast<float>(i) / 44100.0f;
            test_audio[i] = std::sin(2.0f * M_PI * 440.0f * t) * 0.5f +  // A4
                           std::sin(2.0f * M_PI * 880.0f * t) * 0.3f +  // A5
                           std::sin(2.0f * M_PI * 1320.0f * t) * 0.1f;  // E6
        }
        
        // Analyze audio
        std::cout << "Analyzing audio signal...\n";
        auto features = audio_analyzer_->analyzeAudio(test_audio);
        
        std::cout << "Extracted features:\n";
        std::cout << "  Spectral centroid: " << features.spectral_centroid << "\n";
        std::cout << "  RMS: " << features.rms << "\n";
        std::cout << "  Zero crossing rate: " << features.zero_crossing_rate << "\n";
        std::cout << "  Energy: " << features.energy << "\n";
        
        if (!features.mfcc.empty()) {
            std::cout << "  MFCC coefficients: " << features.mfcc.size() << " values\n";
        }
        
        if (!features.chroma.empty()) {
            std::cout << "  Chroma features: " << features.chroma.size() << " values\n";
        }
        
        // Pitch detection
        float pitch = audio_analyzer_->detectPitch(test_audio);
        std::cout << "Detected pitch: " << pitch << " Hz\n";
        
        // Audio classification
        std::cout << "Classifying audio...\n";
        auto classification = audio_analyzer_->classifyAudio(features);
        std::cout << "Predicted class: " << static_cast<int>(classification.predicted_class) 
                 << " (confidence: " << classification.confidence << ")\n";
        
        // Real-time metrics
        auto metrics = audio_analyzer_->getRealTimeMetrics();
        std::cout << "Real-time metrics:\n";
        std::cout << "  Processing latency: " << metrics.processing_latency_ms << " ms\n";
        std::cout << "  Current RMS: " << metrics.current_rms << "\n";
        
        auto stats = audio_analyzer_->getStats();
        std::cout << "Analysis stats - Blocks processed: " << stats.total_audio_blocks_processed 
                 << ", Classifications: " << stats.total_classifications << "\n";
        
        std::cout << "\n";
    }
    
    void demonstrateIntelligentPresetGeneration() {
        std::cout << "=== Intelligent Preset Generation Demo ===\n";
        
        // Create user profile
        IntelligentPresetGenerator::UserMusicalProfile profile;
        profile.preferred_genres[0] = IntelligentPresetGenerator::MusicalGenre::Electronic;
        profile.preferred_genres[1] = IntelligentPresetGenerator::MusicalGenre::Ambient;
        profile.genre_confidence[0] = 0.9f;
        profile.genre_confidence[1] = 0.7f;
        profile.complexity_preference = 0.6f;
        profile.harmonic_preference = 0.8f;
        profile.years_experience = 5;
        profile.is_professional = false;
        
        preset_generator_->updateUserProfile(profile);
        std::cout << "User profile configured\n";
        
        // Generate presets
        std::cout << "Generating intelligent presets...\n";
        auto presets = preset_generator_->generatePresets(5);
        
        std::cout << "Generated " << presets.size() << " presets:\n";
        for (size_t i = 0; i < presets.size(); ++i) {
            auto& preset = presets[i];
            std::cout << "  " << i+1 << ". " << preset.name 
                     << " (Appeal: " << preset.user_appeal_prediction 
                     << ", Complexity: " << preset.complexity_score << ")\n";
        }
        
        // Demonstrate learning from feedback
        std::cout << "\nLearning from user feedback...\n";
        for (size_t i = 0; i < presets.size(); ++i) {
            float satisfaction = (i % 2 == 0) ? 0.8f : 0.5f; // Simulated feedback
            preset_generator_->learnFromUserFeedback(presets[i], satisfaction);
        }
        
        // Genre fusion generation
        std::cout << "Generating genre fusion presets...\n";
        auto fusion_presets = preset_generator_->generateGenreFusion(
            IntelligentPresetGenerator::MusicalGenre::Electronic,
            IntelligentPresetGenerator::MusicalGenre::Jazz,
            0.5f, 3);
        
        std::cout << "Generated " << fusion_presets.size() << " fusion presets\n";
        
        // Real-time generation
        std::cout << "Real-time preset generation...\n";
        IntelligentPresetGenerator::GenerationContext context;
        context.current_genre = IntelligentPresetGenerator::MusicalGenre::Techno;
        context.current_category = IntelligentPresetGenerator::PresetCategory::Bass;
        
        auto rt_preset = preset_generator_->generateRealTime(context, 25.0f);
        std::cout << "Real-time preset: " << rt_preset.name 
                 << " (generated in <25ms)\n";
        
        auto stats = preset_generator_->getStats();
        std::cout << "Generation stats - Total presets: " << stats.total_presets_generated 
                 << ", Avg appeal: " << stats.average_user_appeal << "\n";
        
        std::cout << "\n";
    }
    
    void demonstrateRealTimeProcessing() {
        std::cout << "=== Real-Time Processing Demo ===\n";
        
        // Start real-time monitoring
        audio_analyzer_->startRealTimeMonitoring();
        
        std::cout << "Processing real-time audio stream...\n";
        
        // Simulate real-time audio processing
        std::vector<float> audio_buffer(512);
        for (int frame = 0; frame < 20; ++frame) {
            // Generate audio frame
            for (size_t i = 0; i < audio_buffer.size(); ++i) {
                float t = (frame * audio_buffer.size() + i) / 44100.0f;
                audio_buffer[i] = std::sin(2.0f * M_PI * 220.0f * t) * 0.3f +
                                 std::sin(2.0f * M_PI * 440.0f * t) * 0.2f;
            }
            
            // Process in real-time
            auto features = audio_analyzer_->analyzeAudioInRealTime(audio_buffer);
            
            // Update modulation system
            if (frame % 5 == 0) {
                auto metrics = audio_analyzer_->getRealTimeMetrics();
                std::cout << "Frame " << frame << ": Pitch=" << metrics.current_pitch 
                         << "Hz, Centroid=" << metrics.current_spectral_centroid 
                         << ", Latency=" << metrics.processing_latency_ms << "ms\n";
            }
            
            // Small delay to simulate real-time processing
            std::this_thread::sleep_for(std::chrono::milliseconds(11)); // ~90fps
        }
        
        audio_analyzer_->stopRealTimeMonitoring();
        std::cout << "Real-time processing demo complete\n\n";
    }
    
    void demonstrateUserLearning() {
        std::cout << "=== User Learning System Demo ===\n";
        
        // Simulate user interaction patterns
        std::vector<std::vector<float>> user_presets;
        std::vector<float> user_ratings;
        
        // Generate sample user behavior
        for (int i = 0; i < 10; ++i) {
            std::vector<float> params(64);
            for (float& p : params) {
                p = (i < 5) ? 0.6f + 0.1f * std::sin(i) : 0.4f + 0.1f * std::cos(i);
            }
            user_presets.push_back(params);
            user_ratings.push_back((i < 5) ? 0.8f : 0.4f);
        }
        
        // Analyze user behavior
        preset_generator_->analyzeUserBehavior(user_presets, user_ratings);
        std::cout << "Analyzed user behavior patterns\n";
        
        // Demonstrate adaptation
        auto profile = preset_generator_->getUserProfile();
        std::cout << "User profile - Avg satisfaction: " << profile.getAverageSatisfaction() << "\n";
        
        // Adaptive modulation learning
        modulation_system_->learnFromUserAction("increased_cutoff_modulation", 0.9f);
        modulation_system_->learnFromUserAction("decreased_lfo_rate", 0.6f);
        modulation_system_->learnFromUserAction("changed_envelope_sustain", 0.7f);
        
        // Neural network learning
        for (size_t i = 0; i < 5; ++i) {
            auto preset = neural_generator_->generatePreset();
            float rating = 0.7f + 0.1f * std::sin(i);
            neural_generator_->learnFromUserFeedback(preset, rating);
        }
        
        std::cout << "AI systems adapted to user preferences\n";
        
        // Generate personalized suggestions
        auto suggestions = modulation_system_->getSuggestedActions("bass_synthesis");
        std::cout << "AI suggestions for bass synthesis:\n";
        for (const auto& suggestion : suggestions) {
            std::cout << "  • " << suggestion << "\n";
        }
        
        std::cout << "\n";
    }
    
    void demonstratePerformanceMonitoring() {
        std::cout << "=== Performance Monitoring Demo ===\n";
        
        // Get performance metrics from all components
        auto ai_metrics = ai_manager_->getMetrics();
        std::cout << "AI Manager:\n";
        std::cout << "  Total processed: " << ai_metrics.total_processed << "\n";
        std::cout << "  Active jobs: " << ai_metrics.active_jobs << "\n";
        std::cout << "  Avg processing time: " << ai_metrics.avg_processing_time_ms << " ms\n";
        std::cout << "  CPU usage: " << ai_metrics.cpu_usage_percent << "%\n";
        std::cout << "  Memory usage: " << ai_metrics.memory_usage_mb << " MB\n";
        
        // Performance profiling
        std::cout << "\nRunning performance profiling...\n";
        ai_manager_->startProfiling();
        
        // Simulate intensive AI operations
        for (int i = 0; i < 100; ++i) {
            auto preset = neural_generator_->generatePreset();
            auto features = audio_analyzer_->analyzeAudio(std::vector<float>(1024, 0.5f));
            
            if (i % 20 == 0) {
                std::cout << "Processed " << i << " operations\n";
            }
        }
        
        ai_manager_->stopProfiling();
        auto profile_report = ai_manager_->getProfilingReport();
        std::cout << "\nProfiling report (first 500 chars):\n";
        std::cout << profile_report.substr(0, 500) << "...\n";
        
        // System optimization recommendations
        std::cout << "\nOptimization recommendations:\n";
        if (ai_metrics.cpu_usage_percent > 80.0f) {
            std::cout << "  • High CPU usage detected - consider reducing AI complexity\n";
        }
        if (ai_metrics.avg_processing_time_ms > 10.0f) {
            std::cout << "  • High latency detected - consider optimizing algorithms\n";
        }
        if (ai_metrics.memory_usage_mb > 500) {
            std::cout << "  • High memory usage detected - consider reducing buffer sizes\n";
        }
        
        // Recent events
        auto recent_events = ai_manager_->getRecentEvents(5);
        std::cout << "\nRecent AI system events:\n";
        for (const auto& event : recent_events) {
            std::cout << "  " << event << "\n";
        }
        
        std::cout << "\n";
    }
};

// Main function
int main() {
    try {
        VitalAIDemo demo;
        demo.run();
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Demo failed with error: " << e.what() << "\n";
        return 1;
    }
}