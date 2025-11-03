/*
  ==============================================================================
    plugin_midi.h
    Copyright (c) 2025 Vital Plugin Integration Team
    https://vital.audio
    
    Plugin MIDI handling system for Vital
    Provides MIDI input/output processing, MPE support, MIDI learn functionality,
    and comprehensive MIDI mapping for VST3/AU integration.
  ==============================================================================
*/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <map>
#include <vector>
#include <atomic>
#include <mutex>
#include <array>

namespace vital {
namespace plugin {

//==============================================================================
/**
 * @class MidiMessage
 * @brief Extended MIDI message with additional metadata
 */
class PluginMidiMessage {
public:
    enum MessageType {
        NoteOff,
        NoteOn,
        AfterTouch,
        ControlChange,
        ProgramChange,
        PitchBend,
        ChannelPressure,
        PolyPressure,
        SystemExclusive,
        Clock,
        ActiveSensing,
        Reset
    };
    
    struct MidiNote {
        int noteNumber = -1;
        int velocity = 0;
        int channel = 0;
        double timestamp = 0.0;
        bool active = false;
        bool sustained = false;
        int priority = 0;
        
        // Expression data
        float pressure = 0.0f;      // Channel pressure
        float modWheel = 0.0f;      // Modulation wheel
        float pitchBend = 0.0f;     // Pitch bend amount
        float aftertouch = 0.0f;    // Polyphonic aftertouch
        
        // Expression controllers
        float volume = 1.0f;
        float pan = 0.0f;
        float expression = 1.0f;
        float holdPedal = 0.0f;
        float softPedal = 0.0f;
        
        // MPE data
        bool mpeEnabled = false;
        float mpePitchBendRange = 48.0f; // Semitones
        float mpePressureRange = 1.0f;
        float mpeTimbreRange = 1.0f;
    };
    
    PluginMidiMessage() = default;
    PluginMidiMessage(const juce::MidiMessage& message, double timestamp = 0.0);
    PluginMidiMessage(const PluginMidiMessage& other) = default;
    PluginMidiMessage& operator=(const PluginMidiMessage& other) = default;
    ~PluginMidiMessage() = default;
    
    // Message type
    MessageType getType() const;
    bool isNoteMessage() const;
    bool isControlMessage() const;
    bool isSystemMessage() const;
    
    // MIDI data
    const juce::MidiMessage& getMidiMessage() const { return message_; }
    juce::MidiMessage& getMidiMessage() { return message_; }
    void setMidiMessage(const juce::MidiMessage& message) { message_ = message; }
    
    // Timestamp
    double getTimestamp() const { return timestamp_; }
    void setTimestamp(double timestamp) { timestamp_ = timestamp; }
    
    // Channel
    int getChannel() const { return message_.getChannel(); }
    void setChannel(int channel) { message_.setChannel(channel); }
    
    // Note data
    int getNoteNumber() const { return message_.getNoteNumber(); }
    void setNoteNumber(int note) { message_.setNoteNumber(note); }
    
    int getVelocity() const { return message_.getVelocity(); }
    void setVelocity(int velocity) { message_.setVelocity(velocity); }
    
    // Control change data
    int getControllerNumber() const { return message_.getControllerNumber(); }
    void setControllerNumber(int cc) { message_.setControllerNumber(cc); }
    
    int getControllerValue() const { return message_.getControllerValue(); }
    void setControllerValue(int value) { message_.setControllerValue(value); }
    
    // Pitch bend
    int getPitchWheelValue() const { return message_.getPitchWheelValue(); }
    void setPitchWheelValue(int value) { message_.setPitchWheelValue(value); }
    
    // Aftertouch
    int getAfterTouchValue() const { return message_.getAfterTouchValue(); }
    void setAfterTouchValue(int value) { message_.setAfterTouchValue(value); }
    
    // Program change
    int getProgramChangeNumber() const { return message_.getProgramChangeNumber(); }
    void setProgramChangeNumber(int program) { message_.setProgramChangeNumber(program); }
    
    // Extended note data
    MidiNote& getNoteData() { return noteData_; }
    const MidiNote& getNoteData() const { return noteData_; }
    
    // MPE support
    bool isMpeMessage() const;
    bool supportsMpe() const { return mpeSupported_; }
    void setMpeSupported(bool supported) { mpeSupported_ = supported; }
    
    // Expression data
    float getModulation() const { return modulation_; }
    void setModulation(float mod) { modulation_ = mod; }
    
    float getVolume() const { return volume_; }
    void setVolume(float vol) { volume_ = vol; }
    
    float getPan() const { return pan_; }
    void setPan(float p) { pan_ = p; }
    
    float getExpression() const { return expression_; }
    void setExpression(float exp) { expression_ = exp; }
    
    // Utilities
    juce::String getDescription() const;
    bool isValid() const;
    void validate();
    
    // Thread safety
    void lock() const { mutex_.lock(); }
    void unlock() const { mutex_.unlock(); }
    
private:
    juce::MidiMessage message_;
    double timestamp_ = 0.0;
    MidiNote noteData_;
    
    // Expression data
    float modulation_ = 0.0f;
    float volume_ = 1.0f;
    float pan_ = 0.0f;
    float expression_ = 1.0f;
    
    // Feature flags
    bool mpeSupported_ = false;
    
    mutable std::mutex mutex_;
    
    // Internal helpers
    void updateMessageType();
    MessageType messageType_ = NoteOff;
};

//==============================================================================
/**
 * @class MidiInput
 * @brief MIDI input device and message routing
 */
class MidiInput {
public:
    struct MidiDeviceInfo {
        juce::String name;
        juce::String identifier;
        bool isConnected = false;
        bool isVirtual = false;
        int numInputs = 1;
        int numOutputs = 1;
        juce::String manufacturer;
        juce::String type;
    };
    
    struct RoutingSettings {
        bool enabled = true;
        int channel = -1; // -1 for all channels, 0-15 for specific
        bool filterNotes = false;
        bool filterCC = false;
        bool filterAftertouch = false;
        bool filterPitchBend = false;
        bool filterSystem = false;
        bool transpose = false;
        int transposeAmount = 0;
        bool velocityCurve = false;
        int velocityCurveType = 0; // 0=linear, 1=log, 2=exp
        float velocityScaling = 1.0f;
    };
    
    MidiInput();
    explicit MidiInput(const juce::String& deviceName);
    ~MidiInput();
    
    // Device management
    void setDeviceName(const juce::String& deviceName);
    juce::String getDeviceName() const { return deviceName_; }
    
    bool open();
    void close();
    bool isOpen() const { return isOpen_; }
    
    bool start();
    void stop();
    bool isStarted() const { return isStarted_; }
    
    // Device enumeration
    static std::vector<MidiDeviceInfo> getAvailableDevices();
    static std::vector<MidiDeviceInfo> getVirtualDevices();
    static int getNumPhysicalDevices();
    static int getNumVirtualDevices();
    
    // Routing settings
    RoutingSettings& getRouting() { return routing_; }
    const RoutingSettings& getRouting() const { return routing_; }
    void setRouting(const RoutingSettings& routing) { routing_ = routing; }
    
    // Message filtering
    void addMessageFilter(int messageType);
    void removeMessageFilter(int messageType);
    void clearMessageFilters();
    bool isMessageFiltered(const PluginMidiMessage& message) const;
    
    // Virtual MIDI
    void enableVirtualInput(bool enabled);
    bool isVirtualInputEnabled() const { return virtualInputEnabled_; }
    void sendMessage(const PluginMidiMessage& message);
    
    // Callbacks
    std::function<void(const PluginMidiMessage&)> onMessageReceived;
    std::function<void(const juce::String&)> onDeviceDisconnected;
    std::function<void(const juce::String&)> onError;
    
    // Utility functions
    void setInputLatency(int latencySamples);
    int getInputLatency() const { return inputLatency_; }
    
    void setBufferSize(int size);
    int getBufferSize() const { return bufferSize_; }
    
    // Statistics
    struct Statistics {
        int messagesReceived = 0;
        int notesReceived = 0;
        int ccReceived = 0;
        int notesActive = 0;
        int droppedMessages = 0;
        float averageLatency = 0.0f;
    };
    
    Statistics getStatistics() const;
    void resetStatistics();
    
private:
    juce::String deviceName_;
    std::unique_ptr<juce::MidiInput> midiInput_;
    juce::MidiInputCallback* callback_ = nullptr;
    
    // Device state
    bool isOpen_ = false;
    bool isStarted_ = false;
    bool virtualInputEnabled_ = false;
    
    // Routing
    RoutingSettings routing_;
    std::vector<int> messageFilters_;
    
    // Buffer management
    int inputLatency_ = 0;
    int bufferSize_ = 256;
    std::vector<PluginMidiMessage> messageBuffer_;
    juce::AbstractFifo bufferFifo_{1024};
    
    // Statistics
    mutable Statistics statistics_;
    mutable std::mutex statsMutex_;
    
    // Internal methods
    void handleMidiMessage(const juce::MidiMessage& message, double timestamp);
    void processMessageBuffer();
    void applyFiltering(PluginMidiMessage& message);
    void applyVelocityCurve(PluginMidiMessage& message);
    void updateStatistics(const PluginMidiMessage& message);
    
    // JUCE callback implementation
    struct MidiInputCallback : public juce::MidiInputCallback {
        MidiInput* parent = nullptr;
        
        void handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message, double timestamp) override {
            if (parent) {
                parent->handleMidiMessage(message, timestamp);
            }
        }
    };
    
    std::unique_ptr<MidiInputCallback> midiCallback_;
};

//==============================================================================
/**
 * @class MidiLearn
 * @brief MIDI learn functionality
 */
class MidiLearn {
public:
    struct Mapping {
        int parameterId = -1;
        int cc = -1;
        int channel = -1;
        bool enabled = false;
        bool isBipolar = false;
        float minValue = 0.0f;
        float maxValue = 1.0f;
        float currentValue = 0.0f;
        float lastReceivedValue = 0.0f;
        
        // Learn mode
        bool learnMode = false;
        bool waitingForInput = false;
        double lastLearnTime = 0.0;
        
        // Validation
        bool isValid() const { return parameterId >= 0 && cc >= 0 && channel >= 0; }
    };
    
    MidiLearn();
    ~MidiLearn();
    
    // Plugin reference
    void setPlugin(juce::AudioProcessor* plugin);
    juce::AudioProcessor* getPlugin() const { return plugin_; }
    
    // Learn mode
    void enableLearnMode(bool enabled);
    bool isLearnModeEnabled() const { return learnModeEnabled_; }
    
    void startLearnParameter(int parameterId);
    void stopLearnParameter(int parameterId);
    void cancelLearnMode();
    
    bool isParameterInLearnMode(int parameterId) const;
    
    // Mapping management
    void addMapping(const Mapping& mapping);
    void removeMapping(int parameterId);
    void removeMapping(int cc, int channel);
    void clearAllMappings();
    
    Mapping getMapping(int parameterId) const;
    std::vector<Mapping> getMappingsByChannel(int channel) const;
    std::vector<Mapping> getAllMappings() const { return mappings_; }
    
    bool hasMapping(int parameterId) const;
    bool hasMapping(int cc, int channel) const;
    
    // MIDI handling
    void handleMidiMessage(const PluginMidiMessage& message);
    
    // Mapping utilities
    Mapping createMapping(int parameterId, int cc, int channel);
    void updateMapping(int parameterId, const juce::MidiMessage& message);
    void applyMapping(int parameterId, float value);
    
    // Validation
    bool validateMapping(const Mapping& mapping) const;
    void autoRangeMapping(Mapping& mapping);
    
    // Import/Export
    void saveMappings(juce::XmlElement& xml) const;
    void loadMappings(const juce::XmlElement& xml);
    
    // UI feedback
    void setParameterLabel(int parameterId, const juce::String& label);
    juce::String getParameterLabel(int parameterId) const;
    
    void setLearnFeedback(int parameterId, bool active);
    bool isLearnFeedbackActive(int parameterId) const;
    
private:
    juce::AudioProcessor* plugin_ = nullptr;
    std::vector<Mapping> mappings_;
    std::map<int, int> parameterToMapping_;
    std::map<std::pair<int,int>, int> ccChannelToMapping_;
    
    // Learn mode state
    bool learnModeEnabled_ = false;
    int currentLearnParameter_ = -1;
    double learnStartTime_ = 0.0;
    
    // UI feedback
    std::map<int, bool> learnFeedback_;
    std::map<int, juce::String> parameterLabels_;
    
    // Learn timeout
    double learnTimeout_ = 5.0; // 5 seconds
    std::map<int, double> learnTimeouts_;
    
    mutable std::mutex mappingsMutex_;
    
    // Internal methods
    void timeoutLearns();
    void validateMapping(Mapping& mapping);
    void updateMappingIndex();
    void handleControlChange(const PluginMidiMessage& message);
    void handlePitchBend(const PluginMidiMessage& message);
    void handleAftertouch(const PluginMidiMessage& message);
    
    // Utility functions
    int findMappingByCC(int cc, int channel) const;
    void cleanupInvalidMappings();
};

//==============================================================================
/**
 * @class PluginMidiHandler
 * @brief Main MIDI processing handler
 */
class PluginMidiHandler : public juce::MidiInputCallback {
public:
    struct MidiSettings {
        bool enabled = true;
        int numChannels = 16;
        bool enableMPE = true;
        bool enableMPEChannels = false;
        int mpeChannelsPerZone = 6;
        float pitchBendRange = 48.0f; // Semitones
        float pressureRange = 1.0f;
        float timbreRange = 1.0f;
        
        // Input settings
        bool enableInput = true;
        int inputChannel = -1; // -1 for all
        std::vector<int> activeChannels;
        
        // Output settings
        bool enableOutput = false;
        int outputChannel = -1;
        bool echoToOutput = false;
        
        // Note handling
        bool enableMultiChannelNotes = true;
        int maxSimultaneousNotes = 128;
        bool enableNoteStealing = true;
        int notePriority = 0; // 0=oldest, 1=youngest, 2=lowest, 3=highest
        
        // Controller handling
        bool enableCCMapping = true;
        bool enableNRPN = true;
        bool enable14BitCC = true;
        int volumeCC = 7;
        int panCC = 10;
        int expressionCC = 11;
        
        // System exclusive
        bool enableSysEx = false;
        std::vector<uint8> sysExDeviceId;
        
        // Latency and timing
        int inputLatency = 0;
        int outputLatency = 0;
        bool syncToAudioClock = false;
        bool sendClock = false;
    };
    
    struct MidiState {
        bool initialized = false;
        bool processing = false;
        int activeNotes = 0;
        int totalNotesReceived = 0;
        int totalMessagesProcessed = 0;
        float cpuUsage = 0.0f;
        size_t bufferUsage = 0;
        
        // Per-channel state
        std::array<float, 16> channelVolume;
        std::array<float, 16> channelPan;
        std::array<float, 16> channelExpression;
        std::array<float, 16> channelModulation;
        std::array<float, 16> channelPitchBend;
        std::array<float, 16> channelPressure;
        std::array<std::map<int, float>, 16> ccValues;
        
        // Active notes
        std::vector<PluginMidiMessage::MidiNote> activeNoteData;
        
        // Last message times
        double lastMessageTime = 0.0;
        double lastNoteTime = 0.0;
        double lastCCTime = 0.0;
    };
    
    PluginMidiHandler();
    explicit PluginMidiHandler(juce::AudioProcessor* plugin);
    ~PluginMidiHandler();
    
    // Plugin reference
    void setPlugin(juce::AudioProcessor* plugin);
    juce::AudioProcessor* getPlugin() const { return plugin_; }
    
    // Initialization
    void initialize();
    void shutdown();
    
    // Settings
    MidiSettings& getSettings() { return settings_; }
    const MidiSettings& getSettings() const { return settings_; }
    void setSettings(const MidiSettings& settings) { settings_ = settings; }
    
    // State access
    MidiState getState() const { return state_; }
    void updateState();
    
    // MIDI input
    void enableInput(bool enabled);
    bool isInputEnabled() const { return settings_.enableInput; }
    
    void setInputChannel(int channel);
    int getInputChannel() const { return settings_.inputChannel; }
    
    std::vector<MidiInput::MidiDeviceInfo> getAvailableInputs() const;
    void selectInput(const juce::String& deviceName);
    juce::String getCurrentInput() const;
    
    // MIDI output
    void enableOutput(bool enabled);
    bool isOutputEnabled() const { return settings_.enableOutput; }
    
    void setOutputChannel(int channel);
    int getOutputChannel() const { return settings_.outputChannel; }
    
    void sendMessage(const PluginMidiMessage& message);
    void sendNoteOn(int note, int velocity, int channel = 0);
    void sendNoteOff(int note, int channel = 0);
    void sendControlChange(int cc, int value, int channel = 0);
    void sendPitchBend(int value, int channel = 0);
    void sendAftertouch(int value, int channel = 0);
    void sendProgramChange(int program, int channel = 0);
    
    // Message processing
    void processMidiBuffer(const juce::MidiBuffer& buffer);
    void processMessages(const std::vector<PluginMidiMessage>& messages);
    
    // MIDI learn
    MidiLearn& getMidiLearn() { return midiLearn_; }
    const MidiLearn& getMidiLearn() const { return midiLearn_; }
    
    // Active note management
    void noteOn(int note, float velocity, int channel);
    void noteOff(int note, int channel);
    void allNotesOff(int channel = -1);
    int getNumActiveNotes(int channel = -1) const;
    
    // Controller values
    void setCCValue(int cc, float value, int channel);
    float getCCValue(int cc, int channel) const;
    void setPitchBend(float value, int channel);
    float getPitchBend(int channel) const;
    void setChannelPressure(float value, int channel);
    float getChannelPressure(int channel) const;
    
    // MPE support
    void enableMPE(bool enabled);
    bool isMPEEnabled() const { return settings_.enableMPE; }
    
    void setMPEZone(int zone, const std::vector<int>& channels);
    std::vector<int> getMPEZone(int zone) const;
    
    // Utility functions
    juce::String getChannelName(int channel) const;
    int getCCName(int cc) const;
    juce::String getNoteName(int note) const;
    
    // Performance monitoring
    void enablePerformanceMonitoring(bool enabled);
    bool isPerformanceMonitoringEnabled() const { return performanceMonitoring_; }
    
    struct PerformanceStats {
        int messagesPerSecond = 0;
        int notesPerSecond = 0;
        int peakNotes = 0;
        float averageLatency = 0.0f;
        int droppedMessages = 0;
        float bufferUtilization = 0.0f;
    };
    
    PerformanceStats getPerformanceStats() const;
    
    // Debugging
    void enableDebugOutput(bool enabled);
    void logMidiMessage(const PluginMidiMessage& message, const juce::String& source = "");
    
private:
    juce::AudioProcessor* plugin_ = nullptr;
    MidiSettings settings_;
    MidiState state_;
    MidiLearn midiLearn_;
    
    // Input handling
    std::vector<std::unique_ptr<MidiInput>> inputs_;
    std::unique_ptr<MidiInput> selectedInput_;
    juce::Timer* inputTimer_ = nullptr;
    
    // Output handling
    std::unique_ptr<juce::MidiOutput> midiOutput_;
    std::vector<PluginMidiMessage> outputBuffer_;
    juce::AbstractFifo outputFifo_{256};
    
    // Performance monitoring
    bool performanceMonitoring_ = false;
    juce::PerformanceCounter processingCounter_;
    PerformanceStats performanceStats_;
    mutable std::mutex statsMutex_;
    
    // Debugging
    bool debugEnabled_ = false;
    juce::String debugLogFile_;
    
    // JUCE callbacks
    void handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message, double timestamp) override;
    
    // Internal processing
    void processMessage(const PluginMidiMessage& message);
    void processNoteMessage(const PluginMidiMessage& message);
    void processControlMessage(const PluginMidiMessage& message);
    void processSystemMessage(const PluginMidiMessage& message);
    
    // MPE processing
    void processMPEMessage(const PluginMidiMessage& message);
    void updateMPEZones();
    
    // Utility methods
    bool isChannelActive(int channel) const;
    bool isNoteInRange(int note) const;
    void updateActiveNotes();
    void cleanupFinishedNotes();
    
    // Timer callbacks
    void timerCallback() override;
    
    // Statistics
    void updatePerformanceStats();
    void resetPerformanceStats();
    
    // Thread safety
    mutable std::mutex midiMutex_;
    
    // Constants
    static constexpr int kMaxChannels = 16;
    static constexpr int kMaxNotes = 128;
    static constexpr int kMaxCC = 128;
    static constexpr float kMinPitchBend = -8192.0f;
    static constexpr float kMaxPitchBend = 8191.0f;
};

//==============================================================================
/**
 * @namespace vital::plugin::midi
 * @brief Helper functions for MIDI processing
 */
namespace midi {

/**
 * Convert note number to frequency
 */
float noteNumberToFrequency(int noteNumber);

/**
 * Convert frequency to note number
 */
int frequencyToNoteNumber(float frequency);

/**
 * Convert cents to pitch bend value
 */
float centsToPitchBend(float cents);

/**
 * Convert velocity to gain
 */
float velocityToGain(int velocity);

/**
 * Create MIDI learn message
 */
PluginMidiMessage createLearnMessage(int parameterId);

/**
 * Validate MIDI channel
 */
bool isValidChannel(int channel);

/**
 * Validate CC number
 */
bool isValidCC(int cc);

/**
 * Get CC name
 */
juce::String getCCName(int cc);

/**
 * Create velocity curve
 */
float applyVelocityCurve(float velocity, int curveType);

/**
 * Parse MIDI file tempo
 */
double parseMidiFileTempo(const juce::File& midiFile);

} // namespace midi

} // namespace plugin
} // namespace vital