/*
  ==============================================================================
    plugin_state.h
    Copyright (c) 2025 Vital Plugin Integration Team
    https://vital.audio
    
    Plugin state management system for Vital
    Handles preset loading/saving, plugin state serialization, program management,
    and state synchronization between DAW and plugin for VST3/AU integration.
  ==============================================================================
*/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_cryptography/juce_cryptography.h>
#include <map>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>

namespace vital {
namespace plugin {

//==============================================================================
/**
 * @class ProgramInfo
 * @brief Information about a plugin program/preset
 */
class ProgramInfo {
public:
    struct Metadata {
        juce::String name;
        juce::String author;
        juce::String description;
        juce::String category;
        juce::String tags;
        juce::String filePath;
        juce::String iconPath;
        
        // Technical information
        int version = 1;
        juce::String createdBy;
        juce::Time createdAt;
        juce::Time modifiedAt;
        
        // Rating and popularity
        int rating = 0; // 0-5 stars
        int usageCount = 0;
        bool isUserPreset = true;
        bool isFactoryPreset = false;
        
        // Compatibility
        juce::String requiredPluginVersion;
        juce::String dawCompatibility;
        std::vector<juce::String> compatibleFormats;
        
        // Analysis data
        std::map<juce::String, float> parameterAnalysis;
        juce::String hashSignature;
    };
    
    ProgramInfo() = default;
    ProgramInfo(const juce::String& name, const juce::String& filePath = "");
    ~ProgramInfo() = default;
    
    // Metadata access
    Metadata& getMetadata() { return metadata_; }
    const Metadata& getMetadata() const { return metadata_; }
    
    void setName(const juce::String& name) { metadata_.name = name; }
    juce::String getName() const { return metadata_.name; }
    
    void setFilePath(const juce::String& filePath) { metadata_.filePath = filePath; }
    juce::String getFilePath() const { return metadata_.filePath; }
    
    void setCategory(const juce::String& category) { metadata_.category = category; }
    juce::String getCategory() const { return metadata_.category; }
    
    void setAuthor(const juce::String& author) { metadata_.author = author; }
    juce::String getAuthor() const { return metadata_.author; }
    
    void setDescription(const juce::String& description) { metadata_.description = description; }
    juce::String getDescription() const { return metadata_.description; }
    
    // Rating and usage
    void setRating(int rating) { metadata_.rating = juce::jlimit(0, 5, rating); }
    int getRating() const { return metadata_.rating; }
    
    void incrementUsageCount() { metadata_.usageCount++; }
    int getUsageCount() const { return metadata_.usageCount; }
    
    // State management
    juce::ValueTree createState() const;
    void restoreState(const juce::ValueTree& state);
    
    juce::MemoryBlock createStateData() const;
    bool restoreStateData(const void* data, int size);
    
    // File operations
    bool saveToFile(const juce::File& file) const;
    bool loadFromFile(const juce::File& file);
    
    // Validation
    bool isValid() const;
    bool isCompatible() const;
    
    // Utility
    juce::String getFormattedInfo() const;
    void generateHash();
    
private:
    Metadata metadata_;
    
    // Internal helpers
    bool validateMetadata() const;
    void updateTimestamps();
};

//==============================================================================
/**
 * @class StateManager
 * @brief Manages plugin state and presets
 */
class StateManager {
public:
    enum StateVersion {
        Legacy1 = 1,
        Legacy2 = 2,
        Current = 3
    };
    
    struct StateMetadata {
        int version = Current;
        juce::String pluginName = "Vital";
        juce::String pluginVersion = "3.0.0";
        juce::String format = "VitalState";
        juce::Time createdAt;
        juce::String sourceFormat; // VST3, AU, VST2
        
        // Checksum for validation
        juce::String checksum;
        bool isValid = false;
    };
    
    struct StateData {
        StateMetadata metadata;
        std::map<int, float> parameterValues;
        juce::String programName;
        int currentProgram = 0;
        std::map<juce::String, juce::String> customData;
        
        // MIDI mappings
        std::map<int, int> midiMappings;
        
        // Engine state
        float masterGain = 1.0f;
        float masterTune = 0.0f;
        bool bypassed = false;
        int midiChannel = 1;
        bool midiLearn = false;
        
        // UI state
        juce::Rectangle<int> editorBounds;
        bool editorVisible = true;
        
        // Performance settings
        std::map<juce::String, bool> featureFlags;
        std::map<juce::String, float> engineSettings;
    };
    
    StateManager() = default;
    explicit StateManager(juce::AudioProcessor* plugin);
    ~StateManager() = default;
    
    // Plugin reference
    void setPlugin(juce::AudioProcessor* plugin);
    juce::AudioProcessor* getPlugin() const { return plugin_; }
    
    // State management
    StateData getCurrentState() const;
    void setCurrentState(const StateData& state);
    
    // Parameter state
    void saveParameterState(std::map<int, float>& parameterValues) const;
    void loadParameterState(const std::map<int, float>& parameterValues);
    
    // Program management
    void setCurrentProgram(int programIndex);
    int getCurrentProgram() const { return currentProgram_; }
    void setProgramName(int programIndex, const juce::String& name);
    juce::String getProgramName(int programIndex) const;
    int getNumPrograms() const;
    
    // Preset management
    bool savePreset(const juce::File& file, const juce::String& name = "");
    bool loadPreset(const juce::File& file);
    bool saveCurrentStateAsPreset(const juce::String& name, const juce::String& category = "");
    
    // Factory presets
    void loadFactoryPresets();
    const std::vector<ProgramInfo>& getFactoryPresets() const { return factoryPresets_; }
    const std::vector<ProgramInfo>& getUserPresets() const { return userPresets_; }
    
    // Preset browser
    ProgramInfo* findPresetByName(const juce::String& name) const;
    std::vector<ProgramInfo> searchPresets(const juce::String& query) const;
    std::vector<ProgramInfo> getPresetsByCategory(const juce::String& category) const;
    std::vector<juce::String> getPresetCategories() const;
    
    // State serialization
    juce::MemoryBlock createStateBlock() const;
    bool restoreStateBlock(const void* data, int size);
    
    // XML state management
    juce::XmlElement createStateXml() const;
    bool restoreStateXml(const juce::XmlElement& xml);
    
    // Binary state management (for DAW compatibility)
    juce::MemoryBlock createBinaryState() const;
    bool restoreBinaryState(const void* data, int size);
    
    // Import/Export
    bool exportState(const juce::File& file, const juce::String& format = "xml");
    bool importState(const juce::File& file);
    
    // Validation and compatibility
    bool validateState(const juce::MemoryBlock& state) const;
    bool isCompatibleState(const juce::MemoryBlock& state) const;
    int getStateVersion(const juce::MemoryBlock& state) const;
    
    // State history and undo/redo
    void saveStateHistory();
    bool undoState();
    bool redoState();
    void clearStateHistory();
    bool canUndo() const { return !undoHistory_.empty(); }
    bool canRedo() const { return !redoHistory_.empty(); }
    
    // Auto-save
    void enableAutoSave(bool enabled);
    void setAutoSaveInterval(int seconds);
    void triggerAutoSave();
    
    // State synchronization
    void synchronizeWithDAW();
    void pushStateToDAW();
    void pullStateFromDAW();
    
    // State comparison
    int compareStates(const StateData& state1, const StateData& state2) const;
    juce::String getStateDifference(const StateData& state1, const StateData& state2) const;
    
    // Preset management utilities
    void addUserPreset(const ProgramInfo& preset);
    void removeUserPreset(const juce::String& name);
    void updatePresetRating(const juce::String& name, int rating);
    void incrementPresetUsage(const juce::String& name);
    
    // Custom data
    void setCustomData(const juce::String& key, const juce::String& value);
    juce::String getCustomData(const juce::String& key) const;
    std::map<juce::String, juce::String> getAllCustomData() const;
    
    // Performance and memory management
    void optimizeMemoryUsage();
    void cleanupUnusedStates();
    size_t getStateMemoryUsage() const;
    
private:
    juce::AudioProcessor* plugin_ = nullptr;
    
    // State data
    StateData currentState_;
    int currentProgram_ = 0;
    
    // Preset storage
    std::vector<ProgramInfo> factoryPresets_;
    std::vector<ProgramInfo> userPresets_;
    std::map<juce::String, juce::String> presetCategoryMap_;
    
    // State history for undo/redo
    std::vector<StateData> undoHistory_;
    std::vector<StateData> redoHistory_;
    size_t maxHistorySize_ = 50;
    
    // Auto-save
    bool autoSaveEnabled_ = false;
    int autoSaveInterval_ = 300; // 5 minutes
    juce::Timer* autoSaveTimer_ = nullptr;
    
    // State synchronization
    bool autoSyncEnabled_ = true;
    mutable std::mutex stateMutex_;
    juce::Time lastStateUpdate_;
    
    // Utility methods
    void addToHistory(const StateData& state);
    void createFactoryPresets();
    void loadUserPresets();
    void saveUserPresets();
    juce::String generateStateChecksum(const StateData& state) const;
    bool validateStateData(const StateData& state) const;
    
    // File operations
    juce::File getPresetsDirectory() const;
    juce::File getFactoryPresetsDirectory() const;
    juce::File getUserPresetsDirectory() const;
    juce::String getPresetFileName(const juce::String& name) const;
    
    // State migration
    StateData migrateLegacyState(const void* data, int size, int version) const;
    void updateStateVersion();
    
    // Callback handlers
    void handleParameterChange(int paramId, float value);
    void handleProgramChange(int programIndex);
    void handleMidiMappingChange(int paramId, int cc, int channel);
    
    // Helper functions
    juce::String hashState(const juce::String& data) const;
    bool isValidFilePath(const juce::File& file) const;
    void logStateOperation(const juce::String& operation, const juce::String& details = "");
    
    // Memory management
    void cleanupHistory();
    void optimizePresetStorage();
};

//==============================================================================
/**
 * @class PluginStateManager
 * @brief Main plugin state management class
 */
class PluginStateManager {
public:
    PluginStateManager();
    explicit PluginStateManager(juce::AudioProcessor* plugin);
    ~PluginStateManager();
    
    // Plugin reference
    void setPlugin(juce::AudioProcessor* plugin);
    juce::AudioProcessor* getPlugin() const { return plugin_; }
    
    // State management
    void initialize();
    void shutdown();
    
    // Program management
    void setCurrentProgram(int programIndex);
    int getCurrentProgram() const;
    void setProgramName(int index, const juce::String& name);
    juce::String getProgramName(int index) const;
    int getNumPrograms() const;
    
    // Preset management
    bool loadPreset(const juce::File& file);
    bool savePreset(const juce::File& file, const juce::String& name = "");
    
    // State serialization for DAW
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;
    
    // Parameter state
    void saveParameterValues(std::map<int, float>& values) const;
    void loadParameterValues(const std::map<int, float>& values);
    
    // UI state
    void setEditorBounds(const juce::Rectangle<int>& bounds);
    juce::Rectangle<int> getEditorBounds() const;
    void setEditorVisible(bool visible);
    bool isEditorVisible() const;
    
    // Performance monitoring
    void enablePerformanceLogging(bool enabled);
    void logPerformanceStats() const;
    
    // State validation
    bool isStateValid() const;
    juce::String getStateError() const;
    
private:
    juce::AudioProcessor* plugin_ = nullptr;
    std::unique_ptr<StateManager> stateManager_;
    
    // State synchronization
    juce::Timer* syncTimer_ = nullptr;
    bool autoSyncEnabled_ = true;
    
    // Performance tracking
    mutable juce::PerformanceCounter stateLoadCounter_;
    mutable juce::PerformanceCounter stateSaveCounter_;
    
    // Internal methods
    void startStateSync();
    void stopStateSync();
    void syncState();
    void handleStateChange();
    
    // Callback implementations
    void audioProcessorParameterChanged(juce::AudioProcessor* processor, int parameterIndex, float newValue);
    void audioProcessorChanged(juce::AudioProcessor* processor, const juce::String& changeType);
};

//==============================================================================
/**
 * @namespace vital::plugin::state
 * @brief Helper functions for state management
 */
namespace state {

/**
 * Create a default program name
 */
juce::String createProgramName(int index);

/**
 * Validate preset file format
 */
bool validatePresetFile(const juce::File& file);

/**
 * Get preset file extension
 */
juce::String getPresetExtension();

/**
 * Create preset metadata from filename
 */
juce::String extractPresetNameFromFilename(const juce::String& filename);

/**
 * Get valid preset categories
 */
juce::StringArray getPresetCategories();

/**
 * Create state validation checksum
 */
juce::String createChecksum(const juce::String& data);

/**
 * Compare parameter values for similarity
 */
float compareParameterValues(float value1, float value2);

/**
 * Generate preset preview data
 */
juce::String generatePresetPreview(const std::map<int, float>& parameters);

} // namespace state

} // namespace plugin
} // namespace vital