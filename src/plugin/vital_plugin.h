/*
  ==============================================================================
    vital_plugin.h
    Copyright (c) 2025 Vital Plugin Integration Team
    https://vital.audio
    
    Main plugin wrapper that integrates VitalAudioEngine as a VST3/AU plugin
    Provides complete DAW integration with parameter management, MIDI handling,
    and plugin state management for both VST3 and AudioUnit formats.
  ==============================================================================
*/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_audio_plugin_client/juce_audio_plugin_client.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_data_structures/juce_data_structures.h>
#include <juce_dsp/juce_dsp.h>
#include <juce_opengl/juce_opengl.h>
#include <juce_re果per/juce_re果per.h>
#include <juce_vst3/juce_vst3.h>
#include <juce_xapian/juce_xapian.h>

#include "../audio_engine/vital_audio_engine.h"
#include "plugin_parameters.h"
#include "plugin_state.h"
#include "plugin_midi.h"
#include "vst3_wrapper.h"
#include "au_wrapper.h"
#include "plugin_ui.h"

namespace vital {
namespace plugin {

//==============================================================================
/**
 * @class VitalPlugin
 * @brief Main plugin wrapper for Vital synthesizer
 * 
 * This class provides the bridge between the VitalAudioEngine and DAW environments.
 * It handles:
 * - Parameter management and automation
 * - MIDI input processing
 * - Plugin state saving/loading
 * - Audio processing
 * - Plugin format detection (VST3/AU/VST2)
 */
class VitalPlugin : public juce::AudioProcessor,
                   public juce::AudioProcessorListener,
                   public juce::MidiInputCallback,
                   public juce::AudioPlayHead,
                   public juce::VST3ClientCallback,
                   public juce::AUClientCallback,
                   public juce::ChangeBroadcaster
{
public:
    //==============================================================================
    /** Plugin configuration */
    struct PluginConfig
    {
        // Audio settings
        int numInputs = 0;
        int numOutputs = 2;
        int maxVoices = 32;
        double sampleRate = 44100.0;
        int blockSize = 512;
        
        // Plugin format support
        bool enableVST3 = true;
        bool enableAU = true;
        bool enableVST2 = false;
        bool enableLV2 = false;
        
        // UI settings
        bool enableCustomUI = true;
        int uiWidth = 1200;
        int uiHeight = 800;
        bool resizable = true;
        
        // Parameter settings
        bool enableParameterAutomation = true;
        bool enableMidiLearn = true;
        int maxAutomationPoints = 16384;
        
        // Performance settings
        bool enableMultithreading = true;
        bool enableSIMD = true;
        bool enableOversampling = true;
        int oversamplingFactor = 2;
        
        // State management
        bool enableStateManagement = true;
        bool enablePresetBrowser = true;
        int maxPresets = 1000;
        
        // MIDI settings
        bool enableMidiInput = true;
        bool enableMidiOutput = false;
        bool enableMidiChannelMode = true;
        int maxMidiChannels = 16;
    };
    
    //==============================================================================
    /** Plugin state information */
    struct PluginState
    {
        bool isInitialized = false;
        bool isProcessing = false;
        bool isSuspended = false;
        bool isBypassed = false;
        bool hasCustomUI = false;
        bool supportsMPE = false;
        
        juce::String currentPluginFormat;
        juce::String dawName;
        juce::String dawVersion;
        
        // Performance metrics
        float cpuUsage = 0.0f;
        size_t memoryUsage = 0;
        int activeVoices = 0;
        
        // Time information
        double currentTime = 0.0;
        double ppqPosition = 0.0;
        int bar = 0;
        int beat = 0;
        int tick = 0;
        double tempo = 120.0;
        bool isPlaying = false;
        bool isRecording = false;
        bool loopEnabled = false;
        
        juce::Rectangle<int> lastEditorBounds;
    };
    
    //==============================================================================
    /** Constructor and destructor */
    VitalPlugin();
    VitalPlugin(const PluginConfig& config);
    virtual ~VitalPlugin();
    
    //==============================================================================
    /** AudioProcessor overrides */
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlock(juce::AudioBuffer<double>&, juce::MidiBuffer&) override;
    
    //==============================================================================
    /** Plugin information */
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    
    //==============================================================================
    /** Program/preset management */
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;
    
    //==============================================================================
    /** Parameter management */
    int getNumParameters() override;
    float getParameter(int index) override;
    void setParameter(int index, float newValue) override;
    const juce::String getParameterName(int index) override;
    const juce::String getParameterText(int index) override;
    bool isParameterAutomatable(int index) const override;
    bool isMetaParameter(int index) const override;
    
    /** Parameter automation support */
    void setParameterDefaultValue(int index, float value) override;
    float getParameterDefaultValue(int index) const override;
    int getParameterNumSteps(int index) const override;
    
    //==============================================================================
    /** MIDI processing */
    void handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message) override;
    void processMidiBuffer(const juce::MidiBuffer& buffer);
    
    //==============================================================================
    /** Plugin state management */
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;
    bool hasEditor() const override;
    juce::AudioProcessorEditor* createEditor() override;
    
    //==============================================================================
    /** AudioPlayHead implementation */
    bool getCurrentPosition(juce::AudioPlayHead::CurrentPositionInfo& result) override;
    
    //==============================================================================
    /** VST3 specific methods */
    Steinberg::tresult PLUGIN_API initialize(FUnknown* hostContext) override;
    Steinberg::tresult PLUGIN_API terminate() override;
    Steinberg::tresult PLUGIN_API setHost(Steinberg::FUnknown* host) override;
    
    //==============================================================================
    /** AudioUnit specific methods */
    void beginSetParameter(AudioUnitParameterID paramID, AudioUnitScope scope, 
                          AudioUnitElement element) override;
    void endSetParameter(AudioUnitParameterID paramID, AudioUnitScope scope, 
                        AudioUnitElement element) override;
    
    //==============================================================================
    /** Plugin format detection */
    juce::String getCurrentPluginFormat() const;
    bool isVST3() const;
    bool isAU() const;
    bool isVST2() const;
    
    /** DAW integration */
    juce::String getDAWName() const;
    juce::String getDAWVersion() const;
    bool isInReaper() const;
    bool isInLogic() const;
    bool isInProTools() const;
    bool isInCubase() const;
    
    //==============================================================================
    /** VitalAudioEngine access */
    audio_engine::VitalAudioEngine& getAudioEngine() { return *audioEngine_; }
    const audio_engine::VitalAudioEngine& getAudioEngine() const { return *audioEngine_; }
    
    /** Plugin configuration access */
    PluginConfig getConfig() const { return config_; }
    
    /** Plugin state access */
    PluginState getState() const { return state_; }
    
    //==============================================================================
    /** UI management */
    void showEditor();
    void hideEditor();
    bool isEditorVisible() const;
    void setEditorSize(int width, int height);
    juce::Rectangle<int> getEditorBounds() const;
    
    //==============================================================================
    /** Performance monitoring */
    void enablePerformanceMode(bool enabled);
    bool isPerformanceModeEnabled() const { return performanceModeEnabled_; }
    
    //==============================================================================
    /** Batch processing */
    void updateAllParameters();
    void resetEngine();
    void suspendProcessing();
    void resumeProcessing();
    
    //==============================================================================
    /** Error handling */
    struct ErrorInfo {
        bool hasError = false;
        juce::String message;
        juce::String component;
        std::chrono::steady_clock::time_point timestamp;
    };
    
    ErrorInfo getLastError() const;
    void clearError();
    
    //==============================================================================
    /** Utility functions */
    void logMessage(const juce::String& message, const juce::String& level = "INFO");
    void logError(const juce::String& error);
    bool validateState() const;
    
    //==============================================================================
    /** Testing and debugging */
    void enableTestMode(bool enabled);
    bool isTestModeEnabled() const { return testMode_; }
    
    //==============================================================================
    /** Static factory methods */
    static VitalPlugin* createVST3();
    static VitalPlugin* createAU();
    static VitalPlugin* createVST2();
    
    //==============================================================================
    /** Constants */
    static constexpr int kDefaultNumVoices = 32;
    static constexpr int kDefaultBlockSize = 512;
    static constexpr int kDefaultEditorWidth = 1200;
    static constexpr int kDefaultEditorHeight = 800;
    
private:
    //==============================================================================
    /** Configuration */
    PluginConfig config_;
    
    /** Plugin state */
    PluginState state_;
    
    /** Audio engine */
    std::unique_ptr<audio_engine::VitalAudioEngine> audioEngine_;
    
    /** Plugin parameters */
    PluginParameters parameters_;
    
    /** Plugin state manager */
    PluginStateManager stateManager_;
    
    /** MIDI handler */
    PluginMidiHandler midiHandler_;
    
    /** Editor UI */
    std::unique_ptr<PluginUI> editorUI_;
    
    //==============================================================================
    /** Internal initialization */
    bool initializeEngine();
    bool initializeParameters();
    bool initializeMidi();
    bool initializeUI();
    bool initializeFormats();
    
    /** Internal cleanup */
    void shutdownEngine();
    void shutdownParameters();
    void shutdownMidi();
    void shutdownUI();
    
    //==============================================================================
    /** Processing methods */
    void processAudioBlock(juce::AudioBuffer<float>& buffer, const juce::MidiBuffer& midi);
    void processParameters(int numSamples);
    void handleAutomation(int numSamples);
    void updatePerformanceMetrics();
    
    //==============================================================================
    /** Plugin format detection */
    void detectPluginFormat();
    void updateDAWInfo();
    
    //==============================================================================
    /** Time and tempo processing */
    void updateTimeInfo();
    double calculatePPQ(double timeInSeconds) const;
    
    //==============================================================================
    /** MIDI processing helpers */
    void processMPE(const juce::MidiMessage& message);
    void processPitchBend(const juce::MidiMessage& message);
    void processAftertouch(const juce::MidiMessage& message);
    void processCC(int cc, int value, int channel);
    
    //==============================================================================
    /** Automation handling */
    struct AutomationPoint {
        float time = 0.0f;
        float value = 0.0f;
    };
    
    std::vector<std::vector<AutomationPoint>> parameterAutomation_;
    std::vector<float> parameterSmoothing_;
    
    void scheduleAutomation(int paramIndex, float value, double time);
    void processScheduledAutomation(int numSamples);
    
    //==============================================================================
    /** Performance optimization */
    void optimizeForPerformance();
    void optimizeForMemory();
    void optimizeForLatency();
    
    //==============================================================================
    /** State synchronization */
    void synchronizeState();
    void updateFromDAW();
    void pushToDAW();
    
    //==============================================================================
    /** Error handling */
    mutable std::mutex errorMutex_;
    ErrorInfo lastError_;
    void setError(const juce::String& message, const juce::String& component);
    
    //==============================================================================
    /** Threading and scheduling */
    std::unique_ptr<juce::Timer> performanceTimer_;
    std::unique_ptr<juce::Timer> uiUpdateTimer_;
    std::atomic<bool> processingActive_{false};
    
    //==============================================================================
    /** Debug and test features */
    bool testMode_ = false;
    bool performanceModeEnabled_ = false;
    juce::String debugLogFile_;
    
    //==============================================================================
    /** JUCE macros */
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VitalPlugin)
    JUCE_DECLARE_WEAK_REFERENCEABLE(VitalPlugin)
};

//==============================================================================
/**
 * @namespace vital::plugin::factory
 * @brief Factory functions for creating VitalPlugin instances
 */
namespace factory {

/**
 * Create a VitalPlugin optimized for high quality audio
 */
std::unique_ptr<VitalPlugin> createHighQualityPlugin();

/**
 * Create a VitalPlugin optimized for low CPU usage
 */
std::unique_ptr<VitalPlugin> createLowCPUPlugin();

/**
 * Create a VitalPlugin for mobile/embedded systems
 */
std::unique_ptr<VitalPlugin> createMobilePlugin();

/**
 * Create a VitalPlugin for testing and development
 */
std::unique_ptr<VitalPlugin> createTestPlugin();

/**
 * Validate plugin configuration
 */
bool validateConfig(const VitalPlugin::PluginConfig& config);

} // namespace factory

} // namespace plugin
} // namespace vital