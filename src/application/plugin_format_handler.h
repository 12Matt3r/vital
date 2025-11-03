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
#include <functional>
#include <optional>

// Forward declarations
namespace vital {
    class VitalPluginProcessor;
    class VitalPluginEditor;
}

// Plugin format configuration
namespace vital::plugin_format_config {
    constexpr int MAX_PLUGIN_FORMATS = 4;
    constexpr int MAX_INSTANCES_PER_FORMAT = 64;
    constexpr int MAX_PLUGIN_PARAMETERS = 128;
    constexpr int MAX_PROGRAMS = 128;
    constexpr int MAX_PLUGIN_INPUTS = 16;
    constexpr int MAX_PLUGIN_OUTPUTS = 16;
    
    // Plugin UID configuration
    constexpr juce::String VST3_PLUGIN_UID = "VITa";
    constexpr juce::String AU_PLUGIN_UID = "vital.synth";
    constexpr juce::String LV2_PLUGIN_UID = "urn:vital:synthesizer";
    
    // Plugin categories
    constexpr const char* SYNTHESIZER_CATEGORY = "Instrument/Synthesizer";
    constexpr const char* GENERATOR_CATEGORY = "Generator/Synthesizer";
}

// Plugin format type
namespace vital::plugin_format_type {
    enum class PluginFormat {
        Unknown,
        VST3,
        AudioUnit,
        LV2,
        VST2,
        AAX,
        CLAP
    };
}

// Plugin state
namespace vital::plugin_state {
    enum class PluginState {
        Unloaded,
        Loading,
        Loaded,
        Initializing,
        Ready,
        Processing,
        Error,
        Unloading
    };
}

// Plugin format handler class
namespace vital {

class PluginFormatHandler : public juce::AudioPluginFormat,
                           public juce::AudioPluginFormat::Factory,
                           public juce::Timer {
public:
    // Constructor/Destructor
    PluginFormatHandler();
    ~PluginFormatHandler() override;

    //==============================================================================
    // AudioPluginFormat overrides
    juce::String getName() const override;
    bool fileMightContainThisPluginType(const juce::File& file) const override;
    juce::FileSearchPath getDefaultLocationsToSearch() const override;
    bool doesPluginStillExist(const juce::PluginDescription& description) const override;
    void findAllPluginsToBuild(juce::OwnedArray<juce::PluginDescription>& results) const override;
    std::unique_ptr<juce::AudioPluginInstance> createInstanceFromDescription(
        const juce::PluginDescription& description, 
        double initialSampleRate, 
        int initialBufferSize) const override;
    bool requiresUnblockedMessageThreadForCreation() const override { return false; }
    bool canScanForPlugins() const override { return true; }
    bool isTrivialToScan() const override { return false; }

    //==============================================================================
    // Factory overrides
    juce::String getPluginFormatName() const override;
    juce::StringArray getSearchPaths() const override;
    void getPluginDescriptions(juce::OwnedArray<juce::PluginDescription>& results) const override;
    bool scanAndAddPluginFile(const juce::File& pluginFile, juce::OwnedArray<juce::PluginDescription>& results) const override;

    //==============================================================================
    // Timer overrides
    void timerCallback() override;

    //==============================================================================
    // Custom methods
    bool initialize();
    void cleanup();
    
    // Plugin format registration
    void registerVST3Format();
    void registerAUFormat();
    void registerLV2Format();
    void registerVST2Format();
    void registerAAXFormat();
    void registerCLAPFormat();
    
    // Plugin discovery
    void scanForPlugins();
    juce::Array<juce::PluginDescription> getDiscoveredPlugins(vital::plugin_format_type::PluginFormat format) const;
    juce::PluginDescription getPluginDescription(const juce::String& pluginId) const;
    bool isPluginInstalled(const juce::String& pluginId) const;
    
    // Plugin instantiation
    std::unique_ptr<vital::VitalPluginProcessor> createProcessor(vital::plugin_format_type::PluginFormat format);
    std::unique_ptr<vital::VitalPluginEditor> createEditor(vital::VitalPluginProcessor* processor);
    
    // Plugin management
    void loadPlugin(const juce::String& pluginId);
    void unloadPlugin(const juce::String& pluginId);
    void reloadPlugin(const juce::String& pluginId);
    void getLoadedPlugins(juce::StringArray& pluginList) const;
    void getActiveInstances(vital::plugin_format_type::PluginFormat format, std::vector<std::weak_ptr<vital::VitalPluginProcessor>>& instances) const;
    
    // Plugin state
    vital::plugin_state::PluginState getPluginState(const juce::String& pluginId) const;
    void setPluginState(const juce::String& pluginId, vital::plugin_state::PluginState state);
    
    // Performance monitoring
    float getPluginCPULoad(const juce::String& pluginId) const;
    float getPluginLatency(const juce::String& pluginId) const;
    size_t getPluginMemoryUsage(const juce::String& pluginId) const;
    
    // Plugin configuration
    void setMaxInstancesPerFormat(int maxInstances);
    void setPluginScanPaths(const juce::Array<juce::File>& paths);
    void setPluginCategories(const juce::StringArray& categories);
    void enablePluginFormat(vital::plugin_format_type::PluginFormat format, bool enabled);
    bool isPluginFormatEnabled(vital::plugin_format_type::PluginFormat format) const;
    
    // Plugin validation
    bool validatePluginFile(const juce::File& file) const;
    juce::String getPluginError(const juce::String& pluginId) const;
    bool isPluginSupported(const juce::String& pluginId) const;
    
    // Plugin categories
    juce::StringArray getSupportedCategories() const;
    void setPreferredCategory(const juce::String& category);
    juce::String getPreferredCategory() const;
    
    // Host integration
    bool isRunningInHost() const;
    void setHostName(const juce::String& hostName);
    juce::String getHostName() const;
    void setHostVersion(const juce::String& version);
    juce::String getHostVersion() const;
    
    // Plugin presets
    juce::StringArray getPluginPresets(const juce::String& pluginId) const;
    bool loadPluginPreset(const juce::String& pluginId, const juce::String& presetName);
    bool savePluginPreset(const juce::String& pluginId, const juce::String& presetName);
    bool deletePluginPreset(const juce::String& pluginId, const juce::String& presetName);
    
    // Plugin formats specific
    void setVST3Settings(const juce::XmlElement& settings);
    juce::XmlElement getVST3Settings() const;
    
    void setAUSettings(const juce::XmlElement& settings);
    juce::XmlElement getAUSettings() const;
    
    void setLV2Settings(const juce::XmlElement& settings);
    juce::XmlElement getLV2Settings() const;
    
    // Advanced features
    void enableBypassSupport(bool enabled);
    bool isBypassSupported() const { return bypass_support_enabled_; }
    
    void enableSidechainSupport(bool enabled);
    bool isSidechainSupported() const { return sidechain_support_enabled_; }
    
    void enableMIDIInput(bool enabled);
    bool isMIDIInputSupported() const { return midi_input_enabled_; }
    
    void enableMIDIOutput(bool enabled);
    bool isMIDIOutputSupported() const { return midi_output_enabled_; }
    
    // Plugin categories
    void enableVST3Support();
    void enableAUSupport();
    void enableLV2Support();
    void enableVST2Support();
    void enableAAXSupport();
    void enableCLAPSupport();
    
    // Error handling
    juce::String getLastError() const;
    void clearLastError();
    void logError(const juce::String& error);
    void logWarning(const juce::String& warning);
    void logInfo(const juce::String& info);
    
    // Plugin factory functions
    static juce::String getPluginUID(vital::plugin_format_type::PluginFormat format);
    static vital::plugin_format_type::PluginFormat getFormatFromUID(const juce::String& uid);
    static bool isSupportedFormat(vital::plugin_format_type::PluginFormat format);

private:
    // Plugin format state
    std::atomic<bool> vst3_enabled_{false};
    std::atomic<bool> au_enabled_{false};
    std::atomic<bool> lv2_enabled_{false};
    std::atomic<bool> vst2_enabled_{false};
    std::atomic<bool> aax_enabled_{false};
    std::atomic<bool> clap_enabled_{false};
    
    // Plugin management
    std::map<juce::String, vital::plugin_state::PluginState> plugin_states_;
    std::map<juce::String, juce::String> plugin_errors_;
    std::map<juce::String, std::weak_ptr<VitalPluginProcessor>> active_instances_;
    
    // Plugin discovery
    juce::OwnedArray<juce::PluginDescription> discovered_plugins_[static_cast<int>(vital::plugin_format_type::PluginFormat::CLAP) + 1];
    juce::Array<juce::File> scan_paths_;
    juce::StringArray supported_categories_;
    
    // Configuration
    int max_instances_per_format_ = plugin_format_config::MAX_INSTANCES_PER_FORMAT;
    juce::String preferred_category_ = plugin_format_config::SYNTHESIZER_CATEGORY;
    
    // Host information
    juce::String host_name_ = "Vital Synthesizer";
    juce::String host_version_ = vital::app::VERSION_STRING;
    bool running_in_host_ = false;
    
    // Features
    std::atomic<bool> bypass_support_enabled_{true};
    std::atomic<bool> sidechain_support_enabled_{true};
    std::atomic<bool> midi_input_enabled_{true};
    std::atomic<bool> midi_output_enabled_{false};
    
    // Settings
    juce::XmlElement vst3_settings_;
    juce::XmlElement au_settings_;
    juce::XmlElement lv2_settings_;
    
    // Error handling
    juce::String last_error_;
    juce::StringArray error_log_;
    juce::StringArray warning_log_;
    juce::StringArray info_log_;
    
    // Performance monitoring
    std::map<juce::String, float> plugin_cpu_loads_;
    std::map<juce::String, float> plugin_latencies_;
    std::map<juce::String, size_t> plugin_memory_usage_;
    
    // Scan state
    std::atomic<bool> scanning_in_progress_{false};
    std::atomic<int> scan_progress_{0};
    std::atomic<int> scan_total_{0};
    
    // Initialization methods
    void initializeVST3();
    void initializeAU();
    void initializeLV2();
    void initializeVST2();
    void initializeAAX();
    void initializeCLAP();
    void setupDefaultScanPaths();
    void setupSupportedCategories();
    
    // Scanning methods
    void scanVST3Plugins();
    void scanAUPlugins();
    void scanLV2Plugins();
    void scanVST2Plugins();
    void scanAAXPlugins();
    void scanCLAPPlugins();
    void updateScanProgress();
    
    // Plugin validation methods
    bool validateVST3File(const juce::File& file) const;
    bool validateAUFile(const juce::File& file) const;
    bool validateLV2File(const juce::File& file) const;
    bool validateVST2File(const juce::File& file) const;
    bool validateAAXFile(const juce::File& file) const;
    bool validateCLAPFile(const juce::File& file) const;
    
    // Utility methods
    void logPluginLoad(const juce::String& pluginId);
    void logPluginUnload(const juce::String& pluginId);
    void logPluginError(const juce::String& pluginId, const juce::String& error);
    juce::String getFormatName(vital::plugin_format_type::PluginFormat format) const;
    juce::String getFormatExtension(vital::plugin_format_type::PluginFormat format) const;
    juce::StringArray getFormatSearchPaths(vital::plugin_format_type::PluginFormat format) const;
    
    // Platform-specific methods
    void setupVST3PlatformSpecific();
    void setupAUPlatformSpecific();
    void setupLV2PlatformSpecific();
    
    // Constants
    static constexpr int SCAN_UPDATE_INTERVAL = 100; // ms
    static constexpr int MAX_LOG_ENTRIES = 1000;
    static constexpr int MAX_ERROR_RECENT = 100;
    
    // JUCE leak detection
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginFormatHandler)
};

} // namespace vital
