/*
  ==============================================================================
    au_wrapper.h
    Copyright (c) 2025 Vital Plugin Integration Team
    https://vital.audio
    
    AudioUnit wrapper implementation for Vital plugin
    Provides AU-specific functionality including CoreAudio integration,
    AU parameter management, MIDI handling, and AU-compliant features for macOS.
  ==============================================================================
*/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_re果per/juce_re果per.h>
#include <AudioUnit/AudioUnit.h>
#include <AudioToolbox/AudioToolbox.h>
#include <CoreAudio/CoreAudio.h>
#include <AvailabilityMacros.h>

namespace vital {
namespace plugin {

//==============================================================================
/**
 * @class AUAudioProcessor
 * @brief AudioUnit AudioProcessor implementation
 */
class AUAudioProcessor : public juce::AudioProcessor {
public:
    AUAudioProcessor();
    ~AUAudioProcessor() override;
    
    // AudioProcessor interface
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) override;
    void processBlock(juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midi) override;
    
    // AU-specific methods
    void setAudioUnit(AudioUnit audioUnit);
    AudioUnit getAudioUnit() const { return audioUnit_; }
    
    // Parameter management
    void setParameterValue(AudioUnitParameterID paramID, AudioUnitParameterValue value);
    AudioUnitParameterValue getParameterValue(AudioUnitParameterID paramID) const;
    
    // Parameter history
    void addParameterHistoryPoint(AudioUnitParameterID paramID, AudioUnitParameterValue value, UInt32 sampleFrame);
    void clearParameterHistory(AudioUnitParameterID paramID);
    std::vector<std::pair<UInt32, AudioUnitParameterValue>> getParameterHistory(AudioUnitParameterID paramID) const;
    
    // Parameter ramping
    void setParameterRampTime(AudioUnitParameterID paramID, Float32 rampTime);
    Float32 getParameterRampTime(AudioUnitParameterID paramID) const;
    bool isParameterRamping(AudioUnitParameterID paramID) const;
    
    // Stream format handling
    OSStatus setStreamFormat(AudioUnitScope scope, AudioUnitElement element, const AudioStreamBasicDescription& format);
    OSStatus getStreamFormat(AudioUnitScope scope, AudioUnitElement element, AudioStreamBasicDescription& format) const;
    
    // MIDI processing
    OSStatus processMIDIEvents(AUPixelBuffer*& outputChannelList, const MusicDeviceNoteParams& notes, UInt32 inNumberOfNotes);
    OSStatus handleMIDISysEx(const UInt8* inData, UInt32 inLength);
    
    // Note off handling
    void scheduleNoteOff(AudioUnitElement element, UInt32 noteID, UInt32 offsetSampleFrame);
    
    // Render callback
    static OSStatus renderCallback(void* inRefCon,
                                   AudioUnitRenderActionFlags* ioActionFlags,
                                   const AudioTimeStamp* inTimeStamp,
                                   UInt32 inBusNumber,
                                   UInt32 inNumberFrames,
                                   AudioBufferList* ioData);
    
    // AU connection handling
    void setInputCallback(AudioUnitInputCallback inputCallback, void* userData);
    void setRenderCallback(AudioUnitRenderCallback renderCallback, void* userData);
    
    // Preset management
    OSStatus loadAUPreset(const AUPreset& preset);
    OSStatus saveAUPreset(AUPreset& preset) const;
    OSStatus getCurrentAUPreset(AUPreset& preset) const;
    
    // Custom property handling
    OSStatus setProperty(AudioUnitPropertyID propertyID, AudioUnitScope scope, AudioUnitElement element,
                        const void* data, UInt32 dataSize);
    OSStatus getProperty(AudioUnitPropertyID propertyID, AudioUnitScope scope, AudioUnitElement element,
                        void* data, UInt32& dataSize) const;
    OSStatus getPropertyInfo(AudioUnitPropertyID propertyID, AudioUnitScope scope, AudioUnitElement element,
                           UInt32& dataSize, Boolean& writable) const;
    
    // Component information
    void setComponentDescription(const AudioComponentDescription& desc);
    AudioComponentDescription getComponentDescription() const { return componentDesc_; }
    
    // AU state
    void setInitialized(bool initialized) { isInitialized_ = initialized; }
    bool isInitialized() const { return isInitialized_; }
    
    void setActive(bool active) { isActive_ = active; }
    bool isActive() const { return isActive_; }
    
    // Utility functions
    static OSStatus validateAUVersion(UInt32 version);
    static juce::String getAUErrorString(OSStatus error);
    
private:
    AudioUnit audioUnit_ = nullptr;
    AudioComponentDescription componentDesc_;
    
    // AU state
    bool isInitialized_ = false;
    bool isActive_ = false;
    
    // Parameter management
    std::map<AudioUnitParameterID, Float32> currentValues_;
    std::map<AudioUnitParameterID, std::vector<std::pair<UInt32, AudioUnitParameterValue>>> parameterHistory_;
    std::map<AudioUnitParameterID, Float32> rampTimes_;
    
    // Stream formats
    AudioStreamBasicDescription inputFormat_;
    AudioStreamBasicDescription outputFormat_;
    
    // MIDI handling
    std::vector<UInt8> sysExBuffer_;
    
    // Internal methods
    void initializeParameters();
    void initializeStreamFormats();
    void updateParametersFromAU();
    void pushParametersToAU();
    
    // Custom properties
    static constexpr AudioUnitPropertyID kCustomPropertyBase = 0x1000;
    static constexpr AudioUnitPropertyID kParameterAutomation = kCustomPropertyBase + 1;
    static constexpr AudioUnitPropertyID kMIDISettings = kCustomPropertyBase + 2;
    static constexpr AudioUnitPropertyID kPerformanceMetrics = kCustomPropertyBase + 3;
    static constexpr AudioUnitPropertyID kPresetInfo = kCustomPropertyBase + 4;
};

//==============================================================================
/**
 * @class AUParameterListener
 * @brief AU parameter change listener
 */
class AUParameterListener {
public:
    AUParameterListener();
    ~AUParameterListener();
    
    // Parameter change callbacks
    void parameterDidChange(AudioUnitParameterID paramID, AudioUnitParameterValue newValue, UInt32 sampleOffset);
    void parameterWillChange(AudioUnitParameterID paramID, AudioUnitParameterValue newValue, UInt32 sampleOffset);
    
    // Parameter queries
    AudioUnitParameterValue getParameterValue(AudioUnitParameterID paramID) const;
    UInt32 getParameterChangeTime(AudioUnitParameterID paramID) const;
    
    // History access
    std::vector<std::pair<UInt32, AudioUnitParameterValue>> getParameterHistory(AudioUnitParameterID paramID) const;
    
    // Range validation
    bool validateParameterRange(AudioUnitParameterID paramID, AudioUnitParameterValue value) const;
    AudioUnitParameterValue normalizeParameterValue(AudioUnitParameterID paramID, AudioUnitParameterValue value) const;
    AudioUnitParameterValue denormalizeParameterValue(AudioUnitParameterID paramID, AudioUnitParameterValue value) const;
    
    // Callback registration
    void addParameterCallback(AudioUnitParameterID paramID, std::function<void(AudioUnitParameterID, AudioUnitParameterValue, UInt32)> callback);
    void removeParameterCallback(AudioUnitParameterID paramID);
    void clearParameterCallbacks();
    
    // Parameter mapping
    void mapParameterToProperty(AudioUnitParameterID paramID, AudioUnitPropertyID propID, AudioUnitScope scope);
    void unmapParameterFromProperty(AudioUnitParameterID paramID, AudioUnitPropertyID propID, AudioUnitScope scope);
    
private:
    std::map<AudioUnitParameterID, AudioUnitParameterValue> currentValues_;
    std::map<AudioUnitParameterID, std::vector<std::pair<UInt32, AudioUnitParameterValue>>> changeHistory_;
    std::map<AudioUnitParameterID, std::function<void(AudioUnitParameterID, AudioUnitParameterValue, UInt32)>> callbacks_;
    std::map<AudioUnitParameterID, std::map<std::pair<AudioUnitPropertyID, AudioUnitScope>, bool>> propertyMappings_;
    
    mutable std::mutex parameterMutex_;
    
    // Internal helpers
    void recordParameterChange(AudioUnitParameterID paramID, AudioUnitParameterValue value, UInt32 sampleOffset);
    void notifyCallbacks(AudioUnitParameterID paramID, AudioUnitParameterValue value, UInt32 sampleOffset);
};

//==============================================================================
/**
 * @class AUMIDIManager
 * @brief AudioUnit MIDI management
 */
class AUMIDIManager {
public:
    struct MIDISettings {
        bool enabled = true;
        int numChannels = 16;
        bool enableMPE = false;
        bool enableMPEChannels = false;
        int mpeChannelsPerZone = 6;
        float pitchBendRange = 48.0f;
        float pressureRange = 1.0f;
        float timbreRange = 1.0f;
        
        // Note handling
        bool enableNoteOnOff = true;
        bool enableAftertouch = true;
        bool enablePitchBend = true;
        bool enableControlChange = true;
        
        // Velocity sensitivity
        bool velocityEnabled = false;
        float velocityCurve[128];
        float velocityOffset = 0.0f;
        
        // Expression
        bool enableExpression = true;
        bool enableVolume = true;
        bool enablePan = true;
        bool enableModulation = true;
    };
    
    struct MIDISource {
        AudioUnitInputCallback inputCallback = nullptr;
        void* userData = nullptr;
        UInt32 maxFramesPerSlice = 0;
        AudioStreamBasicDescription streamFormat;
        bool active = false;
    };
    
    AUMIDIManager();
    ~AUMIDIManager();
    
    // Initialization
    void initialize();
    void shutdown();
    
    // MIDI sources
    void addMIDISource(const MIDISource& source);
    void removeMIDISource(int sourceIndex);
    void clearMIDISources();
    
    MIDISource getMIDISource(int sourceIndex) const;
    int getNumMIDISources() const { return static_cast<int>(midiSources_.size()); }
    
    // MIDI processing
    OSStatus processMIDIEvents(const MusicDeviceNoteParams* noteParams, UInt32 inNumberOfNotes, 
                             const UInt8* midiData, UInt32 midiLength, UInt32 sampleOffset);
    
    OSStatus processMIDISysEx(const UInt8* sysExData, UInt32 sysExLength, UInt32 sampleOffset);
    
    // MIDI channel management
    void setMIDIActiveChannels(UInt32 channelBitmap);
    UInt32 getMIDIActiveChannels() const { return midiActiveChannels_; }
    
    void setMIDILearnEnabled(bool enabled) { midiLearnEnabled_ = enabled; }
    bool isMIDILearnEnabled() const { return midiLearnEnabled_; }
    
    // Settings
    MIDISettings& getSettings() { return settings_; }
    const MIDISettings& getSettings() const { return settings_; }
    void setSettings(const MIDISettings& settings) { settings_ = settings; }
    
    // MPE support
    void enableMPE(bool enabled);
    bool isMPEEnabled() const { return settings_.enableMPE; }
    
    void setMPEZone(int zone, const std::vector<UInt32>& channels);
    std::vector<UInt32> getMPEZone(int zone) const;
    
    // Note management
    void scheduleNoteOn(UInt32 note, Float32 velocity, UInt32 channel, UInt32 sampleOffset);
    void scheduleNoteOff(UInt32 note, UInt32 channel, UInt32 sampleOffset);
    void allNotesOff(UInt32 channel);
    
    // Control change handling
    void handleControlChange(UInt32 controller, UInt32 value, UInt32 channel, UInt32 sampleOffset);
    void handlePitchBend(UInt32 value, UInt32 channel, UInt32 sampleOffset);
    void handleAftertouch(UInt32 value, UInt32 channel, UInt32 sampleOffset);
    void handleChannelPressure(UInt32 value, UInt32 channel, UInt32 sampleOffset);
    
    // Utility functions
    static UInt32 noteToFrequency(UInt32 note);
    static UInt32 frequencyToNote(UInt32 frequency);
    static Float32 velocityToGain(UInt32 velocity);
    static Float32 applyVelocityCurve(UInt32 velocity, int curveType);
    
    // Performance metrics
    struct PerformanceMetrics {
        UInt32 messagesProcessed = 0;
        UInt32 notesProcessed = 0;
        UInt32 sysExProcessed = 0;
        Float32 averageLatency = 0.0f;
        UInt32 peakNotes = 0;
        UInt32 droppedMessages = 0;
    };
    
    PerformanceMetrics getPerformanceMetrics() const;
    void resetPerformanceMetrics();
    
private:
    MIDISettings settings_;
    std::vector<MIDISource> midiSources_;
    
    UInt32 midiActiveChannels_ = 0xFFFF; // All channels active by default
    bool midiLearnEnabled_ = false;
    
    // Active notes tracking
    struct ActiveNote {
        UInt32 note = 0;
        UInt32 velocity = 0;
        UInt32 channel = 0;
        UInt32 sampleOffset = 0;
        bool isNoteOn = false;
    };
    
    std::map<UInt32, ActiveNote> activeNotes_; // Key: note + channel
    std::vector<ActiveNote> noteQueue_;
    
    // Performance metrics
    PerformanceMetrics performanceMetrics_;
    mutable std::mutex metricsMutex_;
    
    // Internal methods
    void initializeVelocityCurve();
    void updateActiveNotes();
    void processNoteParams(const MusicDeviceNoteParams* noteParams, UInt32 inNumberOfNotes, UInt32 sampleOffset);
    void processMIDIData(const UInt8* midiData, UInt32 midiLength, UInt32 sampleOffset);
    void updatePerformanceMetrics();
    
    // MPE zone management
    std::vector<std::vector<UInt32>> mpeZones_;
    void updateMPEZones();
    
    // Constants
    static constexpr UInt32 kMIDIMaxChannels = 16;
    static constexpr UInt32 kMIDIMaxNotes = 128;
    static constexpr UInt32 kMIDIMaxControllers = 128;
};

//==============================================================================
/**
 * @class AUWrapper
 * @brief Main AudioUnit wrapper class
 */
class AUWrapper {
public:
    AUWrapper();
    ~AUWrapper();
    
    // Plugin reference
    void setPlugin(juce::AudioProcessor* plugin);
    juce::AudioProcessor* getPlugin() const { return plugin_; }
    
    // AudioUnit processor
    AUAudioProcessor* getAudioProcessor() const { return audioProcessor_.get(); }
    
    // Initialization
    OSStatus initialize(AudioUnit audioUnit);
    OSStatus shutdown();
    
    // Component information
    void setComponentType(AudioUnitType type) { componentDesc_.componentType = type; }
    void setComponentSubtype(AudioUnitSubtype subtype) { componentDesc_.componentSubtype = subtype; }
    void setComponentManufacturer(AudioUnitManufacturer manufacturer) { componentDesc_.componentManufacturer = manufacturer; }
    
    AudioComponentDescription getComponentDescription() const { return componentDesc_; }
    
    // Property handling
    OSStatus setProperty(AudioUnitPropertyID propertyID, AudioUnitScope scope, AudioUnitElement element,
                        const void* data, UInt32 dataSize);
    OSStatus getProperty(AudioUnitPropertyID propertyID, AudioUnitScope scope, AudioUnitElement element,
                        void* data, UInt32& dataSize) const;
    OSStatus getPropertyInfo(AudioUnitPropertyID propertyID, AudioUnitScope scope, AudioUnitElement element,
                           UInt32& dataSize, Boolean& writable) const;
    
    // Parameter management
    void setParameterMapping(int juceParamID, AudioUnitParameterID auParamID, AudioUnitParameterUnit unit);
    AudioUnitParameterID getParameterMapping(int juceParamID) const;
    int getJUCEParameterMapping(AudioUnitParameterID auParamID) const;
    
    // Preset management
    OSStatus saveAUPreset(const juce::String& presetName, bool isFactory = false);
    OSStatus loadAUPreset(const juce::String& presetName);
    std::vector<juce::String> getAUPresetList() const;
    
    // Host communication
    OSStatus sendAUEvent(AudioUnitEventType eventType, AudioUnitParameterID paramID, 
                        AudioUnitParameterValue value, UInt32 scope, UInt32 element);
    
    // Parameter automation
    void enableParameterAutomation(bool enabled);
    bool isParameterAutomationEnabled() const { return parameterAutomationEnabled_; }
    
    void setParameterRampTimes(const std::map<int, Float32>& rampTimes);
    Float32 getParameterRampTime(int paramID) const;
    
    // MIDI handling
    void enableMIDISupport(bool enabled);
    bool isMIDISupportEnabled() const { return midiSupportEnabled_; }
    
    AUMIDIManager* getMIDIManager() const { return midiManager_.get(); }
    
    // Note expression support
    void enableNoteExpression(bool enabled);
    bool isNoteExpressionEnabled() const { return noteExpressionEnabled_; }
    
    OSStatus setNoteExpression(UInt32 noteID, UInt32 channel, Float32 value, UInt32 type);
    OSStatus getNoteExpression(UInt32 noteID, UInt32 channel, Float32& value, UInt32& type) const;
    
    // MPE support
    void enableMPE(bool enabled);
    bool isMPEEnabled() const { return mpeEnabled_; }
    
    void setMPESettings(UInt32 channels, Float32 pitchBendRange, Float32 pressureRange, Float32 timbreRange);
    
    // Sidechain support
    void enableSidechain(bool enabled);
    bool isSidechainEnabled() const { return sidechainEnabled_; }
    
    // Custom properties
    OSStatus registerCustomProperty(AudioUnitPropertyID propID, AudioUnitScope scope, 
                                  AudioUnitElement element, void* data, UInt32 dataSize);
    OSStatus unregisterCustomProperty(AudioUnitPropertyID propID, AudioUnitScope scope, AudioUnitElement element);
    
    // Performance monitoring
    void enablePerformanceReporting(bool enabled);
    bool isPerformanceReportingEnabled() const { return performanceReportingEnabled_; }
    
    struct PerformanceMetrics {
        Float32 cpuUsage = 0.0f;
        UInt32 voiceCount = 0;
        UInt32 xruns = 0;
        Float32 memoryUsage = 0.0f;
        UInt32 messagesProcessed = 0;
    };
    
    PerformanceMetrics getPerformanceMetrics() const;
    
    // Error handling
    OSStatus getLastError(juce::String& error) const;
    void clearError();
    
    // Utility functions
    static OSStatus validateAUVersion(UInt32 version);
    static juce::String getAUErrorString(OSStatus error);
    static AudioUnitComponent* findAUComponent(const AudioComponentDescription& desc);
    
private:
    juce::AudioProcessor* plugin_ = nullptr;
    AudioUnit audioUnit_ = nullptr;
    AudioComponentDescription componentDesc_;
    
    // Core components
    std::unique_ptr<AUAudioProcessor> audioProcessor_;
    std::unique_ptr<AUParameterListener> parameterListener_;
    std::unique_ptr<AUMIDIManager> midiManager_;
    
    // Parameter mapping
    std::map<int, AudioUnitParameterID> juceToAUParamMap_;
    std::map<AudioUnitParameterID, int> auToJuceParamMap_;
    
    // Custom properties
    std::map<std::tuple<AudioUnitPropertyID, AudioUnitScope, AudioUnitElement>, std::vector<uint8>> customProperties_;
    
    // Feature flags
    bool parameterAutomationEnabled_ = true;
    bool midiSupportEnabled_ = true;
    bool noteExpressionEnabled_ = false;
    bool mpeEnabled_ = false;
    bool sidechainEnabled_ = false;
    bool performanceReportingEnabled_ = false;
    
    // Error handling
    mutable juce::String lastError_;
    mutable std::mutex errorMutex_;
    
    // Performance metrics
    PerformanceMetrics performanceMetrics_;
    mutable std::mutex metricsMutex_;
    
    // Internal methods
    OSStatus initializeProperties();
    OSStatus initializeParameters();
    OSStatus initializePresets();
    OSStatus initializeCustomProperties();
    
    void setError(OSStatus error, const juce::String& context);
    OSStatus checkError(OSStatus error, const juce::String& context);
    
    // Property handlers
    static OSStatus propertyListener(void* inRefCon, AudioUnit ci, AudioUnitPropertyID propID, 
                                   AudioUnitScope scope, AudioUnitElement element, const void* data, UInt32 dataSize);
    
    // Custom property handlers
    OSStatus handleCustomPropertyGet(AudioUnitPropertyID propID, AudioUnitScope scope, AudioUnitElement element,
                                   void* data, UInt32& dataSize) const;
    OSStatus handleCustomPropertySet(AudioUnitPropertyID propID, AudioUnitScope scope, AudioUnitElement element,
                                   const void* data, UInt32 dataSize);
    
    // Component information
    AudioComponent component_ = nullptr;
    
    // Version information
    UInt32 versionMajor_ = 0x00030000; // Version 3.0.0
    UInt32 versionMinor_ = 0;
    UInt32 versionPatch_ = 0;
    
    // Constants
    static constexpr UInt32 kAUVersionMajor = 1;
    static constexpr UInt32 kAUVersionMinor = 0;
    static constexpr UInt32 kAUVersionPatch = 0;
};

//==============================================================================
/**
 * @class AUPluginFormat
 * @brief AudioUnit plugin format handler
 */
class AUPluginFormat : public juce::AudioPluginFormat {
public:
    AUPluginFormat();
    ~AUPluginFormat() override;
    
    // AudioPluginFormat interface
    juce::String getName() const override;
    bool canHandleDescription(const juce::PluginDescription& description) const override;
    bool isTrivialToScan() const override;
    bool requiresUnblockedMessageThreadDuringCreation(const juce::PluginDescription&) const override;
    juce::StringArray getSearchPaths() const override;
    juce::FileSearchPath getDefaultLocationsToSearch() const override;
    juce::PluginDescription createPluginDescription(const juce::File& file) const override;
    void findAllPossibleDescriptions(std::vector<juce::PluginDescription>& results) override;
    juce::AudioPluginInstance* createInstanceFromDescription(const juce::PluginDescription& description, 
                                                            double initialSampleRate,
                                                            int initialBufferSize) override;
    void createPluginFromDescription(const juce::PluginDescription& description,
                                   juce::AudioPluginInstance*& instance,
                                   const juce::String& errorMessage) override;
    
    // AU-specific methods
    void setScanProgressCallback(std::function<void(double)> callback);
    bool isScanning() const { return isScanning_; }
    void cancelScanning();
    
    // Component registration
    void registerComponent(const AudioComponentDescription& desc, const juce::String& name);
    void unregisterComponent(const AudioComponentDescription& desc);
    
private:
    std::function<void(double)> scanProgressCallback_;
    std::atomic<bool> isScanning_{false};
    std::atomic<bool> cancelScan_{false};
    
    // Component registration
    std::map<AudioComponentDescription, juce::String> registeredComponents_;
    
    // Internal scanning methods
    void scanAUComponents(std::vector<juce::PluginDescription>& results);
    juce::PluginDescription createDescriptionFromAU(const AudioComponentDescription& desc) const;
    juce::String getComponentName(const AudioComponentDescription& desc) const;
    juce::String getComponentManufacturer(const AudioComponentDescription& desc) const;
    UInt32 getComponentVersion(const AudioComponentDescription& desc) const;
    juce::String getComponentCategory(const AudioComponentDescription& desc) const;
    
    // Validation
    bool validateAUComponent(const AudioComponentDescription& desc) const;
    bool isVST3Component(const AudioComponentDescription& desc) const;
};

//==============================================================================
/**
 * @namespace vital::plugin::au
 * @brief AudioUnit specific helper functions and utilities
 */
namespace au {

/**
 * Convert AudioComponentDescription to JUCE PluginDescription
 */
juce::PluginDescription toAUDescription(const juce::PluginDescription& description);

/**
 * Convert JUCE PluginDescription to AudioComponentDescription
 */
AudioComponentDescription fromAUDescription(const juce::PluginDescription& description);

/**
 * Get AU plugin format information
 */
juce::String getAUFormatInfo();

/**
 * Check AudioUnit compliance
 */
bool validateAUCompliance(const AudioComponentDescription& desc);

/**
 * Create AudioComponentDescription
 */
AudioComponentDescription createComponentDescription(AudioUnitType type, AudioUnitSubtype subtype,
                                                   AudioUnitManufacturer manufacturer);

/**
 * Generate AU plugin type
 */
juce::String getPluginTypeString(AudioUnitType type);

/**
 * AU parameter unit conversion
 */
juce::String parameterUnitToString(AudioUnitParameterUnit unit);
AudioUnitParameterUnit stringToParameterUnit(const juce::String& str);

/**
 * AU error handling
 */
namespace errors {
    static const OSStatus InvalidParameter = -10879;
    static const OSStatus InvalidProperty = -10880;
    static const OSStatus InvalidPropertyValue = -10881;
    static const OSStatus InvalidPropertySize = -10882;
    static const OSStatus InvalidScope = -10883;
    static const OSStatus InvalidElement = -10884;
    static const OSStatus PropertyNotWritable = -10885;
    static const OSStatus PropertyInUse = -10886;
    static const OSStatus TooManyPropsToSet = -10887;
    static const OSStatus CantSetFromAsk = -10888;
    static const OSStatus CantGetFromSet = -10889;
    static const OSStatus PropertyOffLine = -10890;
    static const OSStatus InvalidChannelMap = -10891;
    static const OSStatus InvalidChannelCapabilities = -10892;
    static const OSStatus InvalidAndUnreadableProperty = -10893;
}

} // namespace au

} // namespace plugin
} // namespace vital