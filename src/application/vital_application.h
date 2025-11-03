// Copyright (c) 2025 Vital Application Developers
// 
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from
// the use of this software.
// 
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#pragma once

#include <JuceHeader.h>
#include <memory>
#include <atomic>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <chrono>
#include <functional>
#include <optional>

// Forward declarations
namespace vital {
    class VitalAudioEngine;
    class AudioQualityManager;
    class SIMDProcessor;
    class ParameterManager;
    class ModernDSP;
    class AccessibilityManager;
    class WorkflowManager;
    class PerformanceProfiler;
    class PresetManager;
    class PluginFormatHandler;
    
    namespace ui {
        class VitalMainWindow;
        class VitalEditor;
    }
}

// Core application constants
namespace vital::app {
    // Application version
    constexpr int VERSION_MAJOR = 4;
    constexpr int VERSION_MINOR = 0;
    constexpr int VERSION_PATCH = 0;
    constexpr const char* VERSION_STRING = "4.0.0";
    constexpr const char* VERSION_CODE = "VITAL_4_0_0";
    
    // Performance constants
    constexpr size_t MAX_VOICES = 128;
    constexpr size_t MAX_BUFFERS = 64;
    constexpr size_t AUDIO_BUFFER_SIZE = 512;
    constexpr float SAMPLE_RATE = 44100.0f;
    
    // UI constants
    constexpr int DEFAULT_WINDOW_WIDTH = 1400;
    constexpr int DEFAULT_WINDOW_HEIGHT = 900;
    constexpr int MIN_WINDOW_WIDTH = 1000;
    constexpr int MIN_WINDOW_HEIGHT = 600;
    
    // Thread priorities
    constexpr int AUDIO_THREAD_PRIORITY = 9;
    constexpr int UI_THREAD_PRIORITY = 5;
    constexpr int WORKER_THREAD_PRIORITY = 3;
}

// Core application class
namespace vital {

class VitalApplication : public juce::JUCEApplication {
public:
    // Singleton access
    static std::shared_ptr<VitalApplication> getInstance();
    
    // JUCEApplication overrides
    void initialise(const juce::String& commandLine) override;
    void shutdown() override;
    void suspended() override;
    void resumed() override;
    void systemRequestedQuit() override;
    
    // Application lifecycle
    bool initialiseProject(const juce::String& projectPath = "");
    bool saveProject(const juce::String& projectPath = "");
    void quitApplication();
    
    // Engine access
    std::shared_ptr<VitalAudioEngine> getAudioEngine() const { return audio_engine_; }
    std::shared_ptr<AudioQualityManager> getAudioQualityManager() const { return audio_quality_manager_; }
    std::shared_ptr<SIMDProcessor> getSIMDProcessor() const { return simd_processor_; }
    std::shared_ptr<ParameterManager> getParameterManager() const { return parameter_manager_; }
    std::shared_ptr<ModernDSP> getModernDSP() const { return modern_dsp_; }
    std::shared_ptr<AccessibilityManager> getAccessibilityManager() const { return accessibility_manager_; }
    std::shared_ptr<WorkflowManager> getWorkflowManager() const { return workflow_manager_; }
    std::shared_ptr<PerformanceProfiler> getProfiler() const { return performance_profiler_; }
    std::shared_ptr<PresetManager> getPresetManager() const { return preset_manager_; }
    
    // UI access
    std::shared_ptr<ui::VitalMainWindow> getMainWindow() const { return main_window_; }
    std::shared_ptr<ui::VitalEditor> getEditor() const { return editor_; }
    
    // Plugin formats
    std::shared_ptr<PluginFormatHandler> getPluginFormatHandler() const { return plugin_format_handler_; }
    
    // Configuration
    void loadConfiguration(const juce::File& configFile);
    void saveConfiguration(const juce::File& configFile);
    const juce::PropertiesFile::Options& getPropertiesFileOptions() const;
    
    // Performance monitoring
    float getCPULoad() const;
    size_t getMemoryUsage() const;
    float getAudioLatency() const;
    bool isAudioEngineRunning() const;
    
    // Thread management
    void setThreadPriority(std::thread& thread, int priority);
    void setThreadAffinity(std::thread& thread, int core_id);
    
    // Accessibility
    bool isAccessibilityEnabled() const { return accessibility_enabled_.load(); }
    void setAccessibilityEnabled(bool enabled) { accessibility_enabled_.store(enabled); }
    
    // High DPI support
    bool isHighDPIEnabled() const { return high_dpi_enabled_.load(); }
    void setHighDPIEnabled(bool enabled) { high_dpi_enabled_.store(enabled); }
    
    // Theme and appearance
    juce::String getCurrentTheme() const;
    void setTheme(const juce::String& themeName);
    
    // Presets
    bool loadPreset(const juce::String& presetName);
    bool savePreset(const juce::String& presetName);
    void loadRecentPresets();
    
    // Command line parsing
    struct CommandLineOptions {
        bool standalone = false;
        bool plugin_mode = false;
        bool headless = false;
        juce::String preset_file = "";
        juce::String theme = "default";
        int window_width = vital::app::DEFAULT_WINDOW_WIDTH;
        int window_height = vital::app::DEFAULT_WINDOW_HEIGHT;
        bool enable_accessibility = false;
        bool enable_high_dpi = true;
        float ui_scale = 1.0f;
        bool show_performance_stats = false;
        juce::String log_file = "";
        bool enable_profiling = false;
        int max_voices = vital::app::MAX_VOICES;
        float sample_rate = vital::app::SAMPLE_RATE;
        juce::String plugin_format = "auto"; // auto, vst3, au, lv2
        bool verbose_logging = false;
    };
    
    CommandLineOptions parseCommandLine(const juce::String& commandLine);
    
    // Callbacks
    using ApplicationCallback = std::function<void()>;
    void addCallback(ApplicationCallback callback);
    void removeCallback(ApplicationCallback callback);
    
    // Error handling
    void logError(const juce::String& error_message);
    void handleException(const std::exception& e);
    void reportError(const juce::String& title, const juce::String& message);
    
    // State management
    enum class ApplicationState {
        Uninitialised,
        Initialising,
        Initialised,
        Running,
        Suspended,
        ShuttingDown,
        ShutDown
    };
    
    ApplicationState getState() const { return state_.load(); }
    bool isReady() const { return getState() == ApplicationState::Initialised || getState() == ApplicationState::Running; }
    bool isPlugin() const { return command_line_options_.plugin_mode; }
    bool isStandalone() const { return command_line_options_.standalone; }
    
private:
    // Private constructor for singleton
    VitalApplication();
    
    // Core components
    static std::shared_ptr<VitalApplication> instance_;
    
    // Engine components
    std::shared_ptr<VitalAudioEngine> audio_engine_;
    std::shared_ptr<AudioQualityManager> audio_quality_manager_;
    std::shared_ptr<SIMDProcessor> simd_processor_;
    std::shared_ptr<ParameterManager> parameter_manager_;
    std::shared_ptr<ModernDSP> modern_dsp_;
    std::shared_ptr<AccessibilityManager> accessibility_manager_;
    std::shared_ptr<WorkflowManager> workflow_manager_;
    std::shared_ptr<PerformanceProfiler> performance_profiler_;
    std::shared_ptr<PresetManager> preset_manager_;
    
    // UI components
    std::shared_ptr<ui::VitalMainWindow> main_window_;
    std::shared_ptr<ui::VitalEditor> editor_;
    
    // Plugin integration
    std::shared_ptr<PluginFormatHandler> plugin_format_handler_;
    
    // Configuration
    juce::ApplicationProperties application_properties_;
    juce::PropertiesFile::Options properties_file_options_;
    CommandLineOptions command_line_options_;
    
    // Thread management
    mutable std::mutex callback_mutex_;
    std::vector<ApplicationCallback> callbacks_;
    
    // Performance monitoring
    mutable std::mutex performance_mutex_;
    float cpu_load_ = 0.0f;
    size_t memory_usage_ = 0;
    float audio_latency_ = 0.0f;
    
    // State management
    std::atomic<ApplicationState> state_{ApplicationState::Uninitialised};
    std::atomic<bool> accessibility_enabled_{false};
    std::atomic<bool> high_dpi_enabled_{true};
    
    // Windows-specific optimizations
#ifdef JUCE_WINDOWS
    struct WindowsOptimizations {
        bool enable_hyper_threading_ = true;
        bool enable_prefetch_instructions_ = true;
        bool enable_large_page_support_ = true;
        bool enable_gpu_acceleration_ = true;
        juce::String preferred_audio_driver_ = "WASAPI";
        int audio_buffer_size_ = 256;
        bool enable_audio_resampling_ = true;
        bool enable_debug_layer_ = false;
    } windows_opts_;
#endif
    
    // Initialization methods
    bool initializeCoreSystems();
    bool initializeAudioEngine();
    bool initializeUI();
    bool initializePlugins();
    bool initializePerformanceMonitoring();
    bool initializeAccessibility();
    bool initializeThemes();
    
    // Cleanup methods
    void cleanupCoreSystems();
    void cleanupAudioEngine();
    void cleanupUI();
    void cleanupPlugins();
    void cleanupPerformanceMonitoring();
    void cleanupAccessibility();
    
    // Performance monitoring
    void startPerformanceMonitoring();
    void stopPerformanceMonitoring();
    void updatePerformanceStats();
    
    // Windows-specific initialization
    bool initializeWindowsOptimizations();
    void initializeWindowsAudio();
    void initializeWindowsThemes();
    
    // Platform detection
    bool isWindows() const;
    bool isMacOS() const;
    bool isLinux() const;
    bool isMobile() const;
    
    // Utility methods
    juce::String getApplicationDataDirectory() const;
    juce::String getApplicationLogDirectory() const;
    juce::String getApplicationTempDirectory() const;
    
    // Timer for periodic updates
    juce::Timer performance_timer_;
    void timerCallback() override;
    
    // Exception handling
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VitalApplication)
};

} // namespace vital
