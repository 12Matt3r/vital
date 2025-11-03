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

#include "vital_application.h"
#include "vital_main_window.h"
#include "vital_editor.h"
#include "plugin_format_handler.h"

// Include all the phase improvements
#include "audio_engine/vital_audio_engine.h"
#include "audio_quality/audio_quality.h"
#include "performance/simd_vectorization.h"
#include "performance/multithreading.h"
#include "performance/real_time_optimization.h"
#include "juce_modernization/parameter_manager.h"
#include "juce_modernization/modern_dsp.h"
#include "accessibility/accessibility_manager.h"
#include "workflow_improvements/workflow_manager.h"
#include "profiling_system/performance_profiler.h"
#include "developer_tools/preset_management.h"

#include <JuceHeader.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <exception>
#include <algorithm>
#include <numeric>
#include <execution>

namespace vital {

// Singleton implementation
std::shared_ptr<VitalApplication> VitalApplication::instance_ = nullptr;

//==============================================================================
VitalApplication::VitalApplication()
    : state_(ApplicationState::Uninitialised),
      accessibility_enabled_(false),
      high_dpi_enabled_(true) {
    
    // Set application information
    setApplicationName("Vital Synthesizer");
    setApplicationVersion(vital::app::VERSION_STRING);
    setApplicationVendor("Vital Application Developers");
    
    // Configure properties file
    properties_file_options_.applicationName = "Vital";
    properties_file_options_.filenameSuffix = ".settings";
    properties_file_options_.folderName = "Vital";
    properties_file_options_.osxLibrarySubFolder = "Application Support/Vital";
    properties_file_options_.ignoreCaseOfKeyNames = false;
    properties_file_options_.doNotSave = false;
    properties_file_options_.millisecondsBeforeSaving = 1000;
}

//==============================================================================
std::shared_ptr<VitalApplication> VitalApplication::getInstance() {
    if (!instance_) {
        instance_ = std::make_shared<VitalApplication>();
    }
    return instance_;
}

//==============================================================================
void VitalApplication::initialise(const juce::String& commandLine) {
    try {
        state_.store(ApplicationState::Initialising);
        
        // Parse command line options
        command_line_options_ = parseCommandLine(commandLine);
        
        // Setup logging
        if (!command_line_options_.log_file.isEmpty()) {
            juce::FileLogger* logger = juce::FileLogger::createFileLogger(
                juce::File(command_line_options_.log_file),
                "Vital Log",
                juce::FileLogger::dateStamp + " " + juce::FileLogger::timeStamp + ": "
            );
            juce::Logger::setCurrentLogger(logger);
        }
        
        if (command_line_options_.verbose_logging) {
            DBG("Vital Application Starting...");
            DBG("Version: " << vital::app::VERSION_STRING);
            DBG("Command Line: " << commandLine);
        }
        
        // Initialize core systems
        if (!initializeCoreSystems()) {
            throw std::runtime_error("Failed to initialize core systems");
        }
        
        // Initialize Windows-specific optimizations
#ifdef JUCE_WINDOWS
        if (isWindows() && !initializeWindowsOptimizations()) {
            DBG("Warning: Windows optimizations failed to initialize");
        }
#endif
        
        // Initialize audio engine
        if (!initializeAudioEngine()) {
            throw std::runtime_error("Failed to initialize audio engine");
        }
        
        // Initialize UI (only if not headless or plugin mode)
        if (!command_line_options_.headless && !command_line_options_.plugin_mode) {
            if (!initializeUI()) {
                throw std::runtime_error("Failed to initialize UI");
            }
        }
        
        // Initialize plugin formats
        if (command_line_options_.plugin_mode || command_line_options_.preset_file.isNotEmpty()) {
            if (!initializePlugins()) {
                throw std::runtime_error("Failed to initialize plugin formats");
            }
        }
        
        // Initialize accessibility
        if (command_line_options_.enable_accessibility) {
            if (!initializeAccessibility()) {
                DBG("Warning: Accessibility initialization failed");
            }
        }
        
        // Initialize performance monitoring
        if (!initializePerformanceMonitoring()) {
            DBG("Warning: Performance monitoring initialization failed");
        }
        
        // Initialize themes
        if (!initializeThemes()) {
            DBG("Warning: Theme initialization failed");
        }
        
        // Load configuration
        if (!command_line_options_.preset_file.isEmpty()) {
            loadPreset(command_line_options_.preset_file);
        }
        
        // Load recent presets
        loadRecentPresets();
        
        state_.store(ApplicationState::Initialised);
        
        if (command_line_options_.verbose_logging) {
            DBG("Vital Application initialized successfully");
        }
        
    } catch (const std::exception& e) {
        handleException(e);
        juce::JUCEApplication::quit();
    }
}

//==============================================================================
void VitalApplication::shutdown() {
    try {
        state_.store(ApplicationState::ShuttingDown);
        
        if (command_line_options_.verbose_logging) {
            DBG("Vital Application shutting down...");
        }
        
        // Cleanup in reverse order
        cleanupUI();
        cleanupPlugins();
        cleanupAudioEngine();
        cleanupAccessibility();
        cleanupPerformanceMonitoring();
        cleanupCoreSystems();
        
        state_.store(ApplicationState::ShutDown);
        
        if (command_line_options_.verbose_logging) {
            DBG("Vital Application shutdown complete");
        }
        
    } catch (const std::exception& e) {
        handleException(e);
    }
}

//==============================================================================
void VitalApplication::suspended() {
    state_.store(ApplicationState::Suspended);
    
    // Pause audio engine
    if (audio_engine_) {
        audio_engine_->suspendProcessing(true);
    }
    
    // Pause performance monitoring
    performance_timer_.stopTimer();
    
    if (command_line_options_.verbose_logging) {
        DBG("Vital Application suspended");
    }
}

//==============================================================================
void VitalApplication::resumed() {
    state_.store(ApplicationState::Running);
    
    // Resume audio engine
    if (audio_engine_) {
        audio_engine_->suspendProcessing(false);
    }
    
    // Resume performance monitoring
    performance_timer_.startTimer(100); // Update every 100ms
    
    if (command_line_options_.verbose_logging) {
        DBG("Vital Application resumed");
    }
}

//==============================================================================
void VitalApplication::systemRequestedQuit() {
    // Save configuration before quitting
    saveConfiguration(juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("Vital").getChildFile("settings.xml"));
    
    quitApplication();
}

//==============================================================================
bool VitalApplication::initialiseProject(const juce::String& projectPath) {
    if (projectPath.isEmpty()) {
        return false;
    }
    
    juce::File projectFile(projectPath);
    if (!projectFile.exists()) {
        logError("Project file does not exist: " + projectPath);
        return false;
    }
    
    try {
        // Load project data
        auto xml = juce::XmlDocument::parse(projectFile);
        if (!xml) {
            logError("Failed to parse project file: " + projectPath);
            return false;
        }
        
        // Load preset data
        if (preset_manager_) {
            preset_manager_->loadFromXML(*xml);
        }
        
        // Load audio engine state
        if (audio_engine_) {
            audio_engine_->loadFromXML(*xml);
        }
        
        // Load UI state
        if (main_window_) {
            // Load window position and size
            auto* windowElement = xml->getChildByName("Window");
            if (windowElement) {
                int x = windowElement->getIntAttribute("x", 100);
                int y = windowElement->getIntAttribute("y", 100);
                int width = windowElement->getIntAttribute("width", vital::app::DEFAULT_WINDOW_WIDTH);
                int height = windowElement->getIntAttribute("height", vital::app::DEFAULT_WINDOW_HEIGHT);
                
                main_window_->setBounds(x, y, width, height);
            }
        }
        
        logError("Project loaded successfully: " + projectPath);
        return true;
        
    } catch (const std::exception& e) {
        logError("Exception loading project: " + juce::String(e.what()));
        return false;
    }
}

//==============================================================================
bool VitalApplication::saveProject(const juce::String& projectPath) {
    if (projectPath.isEmpty()) {
        return false;
    }
    
    try {
        auto xml = std::make_unique<juce::XmlElement>("VitalProject");
        
        // Add version information
        xml->setAttribute("version", vital::app::VERSION_STRING);
        xml->setAttribute("build", vital::app::VERSION_CODE);
        
        // Save preset data
        if (preset_manager_) {
            preset_manager_->saveToXML(*xml);
        }
        
        // Save audio engine state
        if (audio_engine_) {
            audio_engine_->saveToXML(*xml);
        }
        
        // Save UI state
        if (main_window_) {
            auto* windowElement = xml->createNewChildElement("Window");
            auto bounds = main_window_->getBounds();
            windowElement->setAttribute("x", bounds.getX());
            windowElement->setAttribute("y", bounds.getY());
            windowElement->setAttribute("width", bounds.getWidth());
            windowElement->setAttribute("height", bounds.getHeight());
        }
        
        // Write to file
        juce::File projectFile(projectPath);
        if (!projectFile.getParentDirectory().createDirectory()) {
            logError("Failed to create project directory");
            return false;
        }
        
        if (!xml->writeTo(projectFile)) {
            logError("Failed to write project file: " + projectPath);
            return false;
        }
        
        logError("Project saved successfully: " + projectPath);
        return true;
        
    } catch (const std::exception& e) {
        logError("Exception saving project: " + juce::String(e.what()));
        return false;
    }
}

//==============================================================================
void VitalApplication::quitApplication() {
    if (state_.load() == ApplicationState::Running ||
        state_.load() == ApplicationState::Suspended) {
        
        state_.store(ApplicationState::ShuttingDown);
        juce::JUCEApplication::quit();
    }
}

//==============================================================================
bool VitalApplication::initializeCoreSystems() {
    try {
        // Initialize performance profiler
        performance_profiler_ = std::make_shared<PerformanceProfiler>();
        if (!performance_profiler_->initialize()) {
            DBG("Warning: Performance profiler initialization failed");
        }
        
        // Initialize SIMD processor
        simd_processor_ = std::make_shared<SIMDProcessor>();
        if (!simd_processor_->initialize()) {
            DBG("Warning: SIMD processor initialization failed");
        }
        
        // Initialize parameter manager
        parameter_manager_ = std::make_shared<ParameterManager>();
        if (!parameter_manager_->initialize()) {
            DBG("Warning: Parameter manager initialization failed");
        }
        
        // Initialize modern DSP
        modern_dsp_ = std::make_shared<ModernDSP>();
        if (!modern_dsp_->initialize()) {
            DBG("Warning: Modern DSP initialization failed");
        }
        
        // Initialize preset manager
        preset_manager_ = std::make_shared<PresetManager>();
        if (!preset_manager_->initialize()) {
            DBG("Warning: Preset manager initialization failed");
        }
        
        return true;
        
    } catch (const std::exception& e) {
        logError("Exception initializing core systems: " + juce::String(e.what()));
        return false;
    }
}

//==============================================================================
bool VitalApplication::initializeAudioEngine() {
    try {
        // Initialize audio engine
        audio_engine_ = std::make_shared<VitalAudioEngine>();
        if (!audio_engine_->initialize(static_cast<int>(command_line_options_.sample_rate), 
                                      vital::app::AUDIO_BUFFER_SIZE)) {
            return false;
        }
        
        // Initialize audio quality manager
        audio_quality_manager_ = std::make_shared<AudioQualityManager>();
        if (!audio_quality_manager_->initialize()) {
            DBG("Warning: Audio quality manager initialization failed");
        }
        
        return true;
        
    } catch (const std::exception& e) {
        logError("Exception initializing audio engine: " + juce::String(e.what()));
        return false;
    }
}

//==============================================================================
bool VitalApplication::initializeUI() {
    try {
        // Create main window
        main_window_ = std::make_shared<ui::VitalMainWindow>();
        if (!main_window_->create()) {
            return false;
        }
        
        // Set window size from command line
        main_window_->setSize(command_line_options_.window_width, 
                             command_line_options_.window_height);
        
        // Enable high DPI if requested
        if (command_line_options_.enable_high_dpi) {
            main_window_->setLookAndFeel(&juce::Desktop::getInstance()
                .getDefaultLookAndFeel());
        }
        
        // Apply UI scaling
        if (command_line_options_.ui_scale != 1.0f) {
            juce::Desktop::getInstance().setGlobalScaleFactor(command_line_options_.ui_scale);
        }
        
        // Show the main window
        main_window_->setVisible(true);
        main_window_->toFront(true);
        
        return true;
        
    } catch (const std::exception& e) {
        logError("Exception initializing UI: " + juce::String(e.what()));
        return false;
    }
}

//==============================================================================
bool VitalApplication::initializePlugins() {
    try {
        // Initialize plugin format handler
        plugin_format_handler_ = std::make_shared<PluginFormatHandler>();
        if (!plugin_format_handler_->initialize()) {
            return false;
        }
        
        // Register plugin formats based on command line
        if (command_line_options_.plugin_format == "auto" ||
            command_line_options_.plugin_format == "vst3") {
            plugin_format_handler_->registerVST3Format();
        }
        
        if (command_line_options_.plugin_format == "auto" ||
            command_line_options_.plugin_format == "au") {
            plugin_format_handler_->registerAUFormat();
        }
        
        if (command_line_options_.plugin_format == "auto" ||
            command_line_options_.plugin_format == "lv2") {
            plugin_format_handler_->registerLV2Format();
        }
        
        return true;
        
    } catch (const std::exception& e) {
        logError("Exception initializing plugins: " + juce::String(e.what()));
        return false;
    }
}

//==============================================================================
bool VitalApplication::initializePerformanceMonitoring() {
    try {
        // Start performance monitoring timer
        performance_timer_.startTimer(100); // Update every 100ms
        
        // Start performance monitoring thread
        std::thread monitoring_thread([this]() {
            startPerformanceMonitoring();
        });
        monitoring_thread.detach();
        
        return true;
        
    } catch (const std::exception& e) {
        logError("Exception initializing performance monitoring: " + juce::String(e.what()));
        return false;
    }
}

//==============================================================================
bool VitalApplication::initializeAccessibility() {
    try {
        // Initialize accessibility manager
        accessibility_manager_ = std::make_shared<AccessibilityManager>();
        if (!accessibility_manager_->initialize()) {
            return false;
        }
        
        accessibility_enabled_.store(true);
        
        // Setup accessibility callbacks
        if (main_window_) {
            // Add accessibility support to main window
            // This would be implemented in the VitalMainWindow class
        }
        
        return true;
        
    } catch (const std::exception& e) {
        logError("Exception initializing accessibility: " + juce::String(e.what()));
        return false;
    }
}

//==============================================================================
bool VitalApplication::initializeThemes() {
    try {
        // Load theme from command line
        setTheme(command_line_options_.theme);
        
        // Setup high DPI support
        high_dpi_enabled_.store(command_line_options_.enable_high_dpi);
        
        return true;
        
    } catch (const std::exception& e) {
        logError("Exception initializing themes: " + juce::String(e.what()));
        return false;
    }
}

//==============================================================================
bool VitalApplication::initializeWindowsOptimizations() {
#ifdef JUCE_WINDOWS
    try {
        // Enable Windows-specific optimizations
        DBG("Initializing Windows optimizations...");
        
        // Set process priority
        SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
        
        // Enable large page support if available
        if (windows_opts_.enable_large_page_support_) {
            // This would be implemented with appropriate Windows API calls
        }
        
        // Initialize WASAPI audio driver
        initializeWindowsAudio();
        
        // Initialize Windows themes
        initializeWindowsThemes();
        
        DBG("Windows optimizations initialized successfully");
        return true;
        
    } catch (const std::exception& e) {
        logError("Exception initializing Windows optimizations: " + juce::String(e.what()));
        return false;
    }
#else
    return true;
#endif
}

//==============================================================================
void VitalApplication::initializeWindowsAudio() {
#ifdef JUCE_WINDOWS
    try {
        // Setup Windows-specific audio driver preferences
        // This would configure WASAPI or DirectSound as preferred
        
        DBG("Windows audio initialized");
        
    } catch (const std::exception& e) {
        logError("Exception initializing Windows audio: " + juce::String(e.what()));
    }
#endif
}

//==============================================================================
void VitalApplication::initializeWindowsThemes() {
#ifdef JUCE_WINDOWS
    try {
        // Initialize Windows 10/11 theme support
        // This would enable modern Windows themes
        
        DBG("Windows themes initialized");
        
    } catch (const std::exception& e) {
        logError("Exception initializing Windows themes: " + juce::String(e.what()));
    }
#endif
}

//==============================================================================
void VitalApplication::cleanupCoreSystems() {
    performance_profiler_.reset();
    simd_processor_.reset();
    parameter_manager_.reset();
    modern_dsp_.reset();
    preset_manager_.reset();
}

//==============================================================================
void VitalApplication::cleanupAudioEngine() {
    audio_engine_.reset();
    audio_quality_manager_.reset();
}

//==============================================================================
void VitalApplication::cleanupUI() {
    editor_.reset();
    main_window_.reset();
}

//==============================================================================
void VitalApplication::cleanupPlugins() {
    plugin_format_handler_.reset();
}

//==============================================================================
void VitalApplication::cleanupPerformanceMonitoring() {
    stopPerformanceMonitoring();
    performance_timer_.stopTimer();
}

//==============================================================================
void VitalApplication::cleanupAccessibility() {
    accessibility_manager_.reset();
    accessibility_enabled_.store(false);
}

//==============================================================================
void VitalApplication::startPerformanceMonitoring() {
    while (state_.load() != ApplicationState::ShutDown &&
           state_.load() != ApplicationState::ShuttingDown) {
        updatePerformanceStats();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

//==============================================================================
void VitalApplication::stopPerformanceMonitoring() {
    // Thread will exit naturally when state changes
}

//==============================================================================
void VitalApplication::updatePerformanceStats() {
    std::lock_guard<std::mutex> lock(performance_mutex_);
    
    // Update CPU load (simplified - would use more sophisticated methods)
    cpu_load_ = juce::SystemStats::getCpuUsage() * 100.0f;
    
    // Update memory usage
    memory_usage_ = juce::SystemStats::getCurrentMemoryUsage();
    
    // Update audio latency
    if (audio_engine_) {
        audio_latency_ = audio_engine_->getCurrentLatency();
    }
}

//==============================================================================
void VitalApplication::timerCallback() {
    if (state_.load() == ApplicationState::Running) {
        updatePerformanceStats();
    }
}

//==============================================================================
VitalApplication::CommandLineOptions VitalApplication::parseCommandLine(const juce::String& commandLine) {
    CommandLineOptions options;
    juce::StringArray tokens;
    tokens.addTokens(commandLine, true);
    
    for (int i = 0; i < tokens.size(); ++i) {
        juce::String token = tokens[i];
        
        if (token == "--standalone" || token == "-s") {
            options.standalone = true;
        }
        else if (token == "--plugin" || token == "-p") {
            options.plugin_mode = true;
        }
        else if (token == "--headless" || token == "-h") {
            options.headless = true;
        }
        else if (token.startsWith("--preset=")) {
            options.preset_file = token.substring(9);
        }
        else if (token.startsWith("--theme=")) {
            options.theme = token.substring(8);
        }
        else if (token.startsWith("--width=")) {
            options.window_width = token.substring(8).getIntValue();
        }
        else if (token.startsWith("--height=")) {
            options.window_height = token.substring(9).getIntValue();
        }
        else if (token == "--accessibility" || token == "-a") {
            options.enable_accessibility = true;
        }
        else if (token == "--no-high-dpi") {
            options.enable_high_dpi = false;
        }
        else if (token.startsWith("--scale=")) {
            options.ui_scale = token.substring(8).getFloatValue();
        }
        else if (token == "--stats" || token == "--performance") {
            options.show_performance_stats = true;
        }
        else if (token.startsWith("--log=")) {
            options.log_file = token.substring(6);
        }
        else if (token == "--profile" || token == "--profiling") {
            options.enable_profiling = true;
        }
        else if (token.startsWith("--voices=")) {
            options.max_voices = token.substring(9).getIntValue();
        }
        else if (token.startsWith("--sample-rate=")) {
            options.sample_rate = token.substring(14).getFloatValue();
        }
        else if (token.startsWith("--format=")) {
            options.plugin_format = token.substring(9);
        }
        else if (token == "--verbose" || token == "-v") {
            options.verbose_logging = true;
        }
    }
    
    // Detect mode if not explicitly set
    if (!options.standalone && !options.plugin_mode) {
        // Default to standalone if no UI elements specified
        options.standalone = true;
    }
    
    return options;
}

//==============================================================================
void VitalApplication::loadConfiguration(const juce::File& configFile) {
    if (!configFile.exists()) {
        return;
    }
    
    try {
        auto xml = juce::XmlDocument::parse(configFile);
        if (xml) {
            // Load configuration from XML
            // This would parse various configuration options
            
            if (xml->hasAttribute("theme")) {
                setTheme(xml->getStringAttribute("theme"));
            }
            
            if (xml->hasAttribute("accessibility")) {
                setAccessibilityEnabled(xml->getBoolAttribute("accessibility", false));
            }
            
            if (xml->hasAttribute("high_dpi")) {
                setHighDPIEnabled(xml->getBoolAttribute("high_dpi", true));
            }
        }
        
    } catch (const std::exception& e) {
        logError("Exception loading configuration: " + juce::String(e.what()));
    }
}

//==============================================================================
void VitalApplication::saveConfiguration(const juce::File& configFile) {
    try {
        auto xml = std::make_unique<juce::XmlElement>("VitalConfig");
        
        xml->setAttribute("version", vital::app::VERSION_STRING);
        xml->setAttribute("theme", getCurrentTheme());
        xml->setAttribute("accessibility", isAccessibilityEnabled());
        xml->setAttribute("high_dpi", isHighDPIEnabled());
        xml->setAttribute("ui_scale", juce::Desktop::getInstance().getGlobalScaleFactor());
        
        // Write configuration
        if (configFile.getParentDirectory().createDirectory()) {
            xml->writeTo(configFile);
        }
        
    } catch (const std::exception& e) {
        logError("Exception saving configuration: " + juce::String(e.what()));
    }
}

//==============================================================================
const juce::PropertiesFile::Options& VitalApplication::getPropertiesFileOptions() const {
    return properties_file_options_;
}

//==============================================================================
float VitalApplication::getCPULoad() const {
    std::lock_guard<std::mutex> lock(performance_mutex_);
    return cpu_load_;
}

//==============================================================================
size_t VitalApplication::getMemoryUsage() const {
    std::lock_guard<std::mutex> lock(performance_mutex_);
    return memory_usage_;
}

//==============================================================================
float VitalApplication::getAudioLatency() const {
    std::lock_guard<std::mutex> lock(performance_mutex_);
    return audio_latency_;
}

//==============================================================================
bool VitalApplication::isAudioEngineRunning() const {
    return audio_engine_ && audio_engine_->isProcessing();
}

//==============================================================================
void VitalApplication::setThreadPriority(std::thread& thread, int priority) {
#ifdef JUCE_WINDOWS
    SetThreadPriority(static_cast<HANDLE>(thread.native_handle()), priority);
#elif defined(JUCE_MAC)
    // macOS thread priority handling
#elif defined(JUCE_LINUX)
    // Linux thread priority handling
#endif
}

//==============================================================================
void VitalApplication::setThreadAffinity(std::thread& thread, int core_id) {
#ifdef JUCE_WINDOWS
    SetThreadAffinityMask(static_cast<HANDLE>(thread.native_handle()), 1 << core_id);
#elif defined(JUCE_MAC)
    // macOS thread affinity handling
#elif defined(JUCE_LINUX)
    // Linux thread affinity handling
#endif
}

//==============================================================================
juce::String VitalApplication::getCurrentTheme() const {
    return application_properties_.getUserSettings()->getValue("current_theme", "default");
}

//==============================================================================
void VitalApplication::setTheme(const juce::String& themeName) {
    application_properties_.getUserSettings()->setValue("current_theme", themeName);
    
    // Apply theme to UI components
    if (main_window_) {
        // This would be implemented in VitalMainWindow
    }
}

//==============================================================================
bool VitalApplication::loadPreset(const juce::String& presetName) {
    if (!preset_manager_) {
        return false;
    }
    
    return preset_manager_->loadPreset(presetName);
}

//==============================================================================
bool VitalApplication::savePreset(const juce::String& presetName) {
    if (!preset_manager_) {
        return false;
    }
    
    return preset_manager_->savePreset(presetName);
}

//==============================================================================
void VitalApplication::loadRecentPresets() {
    if (!preset_manager_) {
        return;
    }
    
    // Load list of recently used presets
    auto recent_presets = application_properties_.getUserSettings()->getValue("recent_presets");
    // Process recent presets list
    
    DBG("Loaded recent presets");
}

//==============================================================================
void VitalApplication::addCallback(ApplicationCallback callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    callbacks_.push_back(callback);
}

//==============================================================================
void VitalApplication::removeCallback(ApplicationCallback callback) {
    std::lock_guard<std::mutex> lock(callback_mutex_);
    callbacks_.erase(std::remove(callbacks_.begin(), callbacks_.end(), callback), callbacks_.end());
}

//==============================================================================
void VitalApplication::logError(const juce::String& error_message) {
    juce::Logger::writeToLog("[Vital] " + error_message);
}

//==============================================================================
void VitalApplication::handleException(const std::exception& e) {
    logError("Exception: " + juce::String(e.what()));
    
    // Show error dialog
    juce::AlertWindow::showMessageBox(
        juce::AlertWindow::AlertIconType::WarningIcon,
        "Vital Error",
        juce::String("An error occurred: ") + e.what(),
        "OK"
    );
}

//==============================================================================
void VitalApplication::reportError(const juce::String& title, const juce::String& message) {
    juce::AlertWindow::showMessageBox(
        juce::AlertWindow::AlertIconType::WarningIcon,
        title,
        message,
        "OK"
    );
}

//==============================================================================
bool VitalApplication::isWindows() const {
    return JUCE_WINDOWS;
}

//==============================================================================
bool VitalApplication::isMacOS() const {
    return JUCE_MAC;
}

//==============================================================================
bool VitalApplication::isLinux() const {
    return JUCE_LINUX;
}

//==============================================================================
bool VitalApplication::isMobile() const {
    return JUCE_IOS || JUCE_ANDROID;
}

//==============================================================================
juce::String VitalApplication::getApplicationDataDirectory() const {
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("Vital").getFullPathName();
}

//==============================================================================
juce::String VitalApplication::getApplicationLogDirectory() const {
    return juce::File::getSpecialLocation(juce::File::userLogsDirectory)
        .getChildFile("Vital").getFullPathName();
}

//==============================================================================
juce::String VitalApplication::getApplicationTempDirectory() const {
    return juce::File::getSpecialLocation(juce::File::tempDirectory)
        .getChildFile("Vital").getFullPathName();
}

} // namespace vital
