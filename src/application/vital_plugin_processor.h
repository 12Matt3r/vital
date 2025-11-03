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
    class VitalAudioEngine;
    class ParameterManager;
    class PresetManager;
    class VitalMainWindow;
}

// Plugin parameter IDs
namespace vital::plugin {
    enum ParameterIds : juce::AudioProcessorParameter::PID {
        // Oscillator parameters
        OSC1_WAVE = 0,
        OSC1_FREQ = 1,
        OSC1_DETUNE = 2,
        OSC1_PHASE = 3,
        OSC2_WAVE = 4,
        OSC2_FREQ = 5,
        OSC2_DETUNE = 6,
        OSC2_PHASE = 7,
        
        // Filter parameters
        FILTER_CUTOFF = 10,
        FILTER_RESONANCE = 11,
        FILTER_TYPE = 12,
        FILTER_MODE = 13,
        
        // Envelope parameters
        ENV1_ATTACK = 20,
        ENV1_DECAY = 21,
        ENV1_SUSTAIN = 22,
        ENV1_RELEASE = 23,
        ENV2_ATTACK = 24,
        ENV2_DECAY = 25,
        ENV2_SUSTAIN = 26,
        ENV2_RELEASE = 27,
        
        // LFO parameters
        LFO1_RATE = 30,
        LFO1_WAVE = 31,
        LFO1_SYNC = 32,
        LFO2_RATE = 33,
        LFO2_WAVE = 34,
        LFO2_SYNC = 35,
        
        // Master parameters
        MASTER_VOLUME = 40,
        MASTER_PAN = 41,
        MASTER_TUNE = 42,
        
        // FX parameters
        REVERB_ROOMSIZE = 50,
        REVERB_DAMPING = 51,
        REVERB_MIX = 52,
        DELAY_TIME = 53,
        DELAY_FEEDBACK = 54,
        DELAY_MIX = 55,
        
        // Advanced parameters
        UNISON_VOICES = 60,
        UNISON_SPREAD = 61,
        UNISON_DETUNE = 62,
        
        // Performance parameters
        POLYPHONY = 70,
        VOICE_MODE = 71,
        LEGATO_MODE = 72
    };
}

// Plugin configuration
namespace vital::plugin_config {
    // Plugin info
    constexpr int MAX_PARAMETERS = 128;
    constexpr int MAX_PROGRAMS = 128;
    constexpr int MAX_INPUT_CHANNELS = 2;
    constexpr int MAX_OUTPUT_CHANNELS = 2;
    constexpr int MAX_MIDI_CHANNELS = 16;
    
    // Plugin flags
    constexpr bool SUPPORTS_ARPEGGIATOR = true;
    constexpr bool SUPPORTS_SUSTAIN_PEDAL = true;
    constexpr bool SUPPORTS_ALL_PASS_NOTES_OFF = true;
    constexpr bool SUPPORTS_PITCH_WHEEL = true;
    constexpr bool SUPPORTS_MODULATION_WHEEL = true;
    constexpr bool SUPPORTS_CHANNEL_PRESSURE = true;
    constexpr bool SUPPORTS_POLYPHONIC_AFTERTOUCH = true;
}

// Main plugin processor class
namespace vital {

class VitalPluginProcessor : public juce::AudioProcessor,
                             public juce::AudioProcessorParameter::Listener,
                             public juce::MidiKeyboardState::Listener,
                             public juce::ValueTree::Listener {
public:
    // Constructor/Destructor
    VitalPluginProcessor();
    explicit VitalPluginProcessor(const BusesProperties& ioLayouts);
    ~VitalPluginProcessor() override;

    //==============================================================================
    // AudioProcessor overrides
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    void processBlock(juce::AudioBuffer<double>&, juce::MidiBuffer&) override;
    
    // Editor management
    bool hasEditor() const override { return true; }
    const juce::String getEditorName() const override { return "Vital Synthesizer Editor"; }
    juce::AudioProcessorEditor* createEditor() override;
    
    // Plugin information
    const juce::String getName() const override { return "Vital Synthesizer"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    bool isParameterAutomatable(int parameterIndex) const override;
    
    // Parameter management
    int getNumParameters() override { return vital::plugin_config::MAX_PARAMETERS; }
    float getParameter(int index) override;
    void setParameter(int index, float newValue) override;
    const juce::String getParameterName(int index) const override;
    const juce::String getParameterText(int index) const override;
    juce::String getParameterLabel(int index) const override;
    
    // Program management
    int getNumPrograms() override { return vital::plugin_config::MAX_PROGRAMS; }
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;
    
    // State management
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;
    
    // Preset management
    void loadFromXml(const juce::XmlElement& xml);
    void saveToXml(juce::XmlElement& xml) const;
    
    // Buses management
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlockBypassed(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) override;
    
    // Audio processor wrapper
    void processBlock(juce::AudioBuffer<float>& buffer, 
                     const juce::MidiBuffer::Iterator& midiIterator) override;

    //==============================================================================
    // AudioProcessorParameter::Listener overrides
    void parameterValueChanged(int parameterIndex, float newValue) override;
    void parameterGestureChanged(int parameterIndex, bool gestureIsStarting) override;

    //==============================================================================
    // MidiKeyboardState::Listener overrides
    void handleNoteOn(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;
    void handleNoteOff(juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity) override;

    //==============================================================================
    // ValueTree::Listener overrides
    void valueTreePropertyChanged(juce::ValueTree&, const juce::Identifier&) override {}
    void valueTreeChildAdded(juce::ValueTree&, juce::ValueTree&) override {}
    void valueTreeChildRemoved(juce::ValueTree&, juce::ValueTree&, int) override {}
    void valueTreeChildOrderChanged(juce::ValueTree&, int, int) override {}
    void valueTreeParentChanged(juce::ValueTree&) override {}

    //==============================================================================
    // Custom methods
    void setStandaloneMode(bool standalone) { is_standalone_ = standalone; }
    void setIsStandalone(bool standalone) { is_standalone_ = standalone; }
    bool isStandalone() const { return is_standalone_; }
    
    // Engine access
    std::shared_ptr<VitalAudioEngine> getAudioEngine() const { return audio_engine_; }
    std::shared_ptr<ParameterManager> getParameterManager() const { return parameter_manager_; }
    std::shared_ptr<PresetManager> getPresetManager() const { return preset_manager_; }
    
    // MIDI processing
    void handleMidiMessage(const juce::MidiMessage& message);
    void handleMidiSysExMessage(const juce::MidiMessage& message);
    
    // Performance monitoring
    float getCPULoad() const;
    float getCurrentLatency() const;
    size_t getMemoryUsage() const;
    
    // Plugin bypass
    void setBypassed(bool bypassed) { is_bypassed_ = bypassed; }
    bool isBypassed() const { return is_bypassed_; }
    
    // Advanced features
    void enableArpeggiator(bool enabled);
    void setArpeggiatorPattern(const juce::String& pattern);
    void setArpeggiatorRate(float rate);
    
    // Preset categories
    juce::StringArray getPresetCategories() const;
    juce::StringArray getPresetsInCategory(const juce::String& category) const;
    
    // Copy/Paste
    void copyFromProcessor(const VitalPluginProcessor& other);
    void pasteToProcessor(VitalPluginProcessor& other) const;
    
    // Version information
    static const juce::String getVersionString() { return vital::app::VERSION_STRING; }
    static int getVersionNumber() { return vital::app::VERSION_MAJOR * 10000 + vital::app::VERSION_MINOR * 100 + vital::app::VERSION_PATCH; }

private:
    // Audio engine components
    std::shared_ptr<VitalAudioEngine> audio_engine_;
    std::shared_ptr<ParameterManager> parameter_manager_;
    std::shared_ptr<PresetManager> preset_manager_;
    
    // Processing state
    std::atomic<bool> is_bypassed_{false};
    std::atomic<bool> is_standalone_{false};
    std::atomic<float> current_cpu_load_{0.0f};
    
    // MIDI state
    juce::MidiKeyboardState midi_keyboard_state_;
    juce::MidiBuffer midi_buffer_;
    std::vector<juce::MidiMessage> incoming_midi_messages_;
    
    // Parameters
    std::vector<float> parameters_;
    std::vector<juce::String> parameter_names_;
    std::vector<juce::String> parameter_labels_;
    
    // Programs
    std::vector<juce::String> program_names_;
    int current_program_ = 0;
    
    // Arpeggiator state
    bool arpeggiator_enabled_ = false;
    juce::String arpeggiator_pattern_ = "Up";
    float arpeggiator_rate_ = 120.0f; // BPM
    
    // Processing buffers
    juce::AudioBuffer<float> dry_buffer_;
    juce::AudioBuffer<float> wet_buffer_;
    
    // Performance monitoring
    mutable std::mutex performance_mutex_;
    float peak_latency_ = 0.0f;
    float current_latency_ = 0.0f;
    size_t memory_usage_ = 0;
    
    // Initialization
    void initializeParameters();
    void initializePrograms();
    void setupBuses();
    
    // Processing helpers
    void processMidiMessages(const juce::MidiBuffer& midiBuffer);
    void updateParameters();
    void updateArpeggiator();
    void applyParameterChanges();
    
    // MIDI helpers
    void handleMidiNoteOn(int midiChannel, int midiNoteNumber, float velocity);
    void handleMidiNoteOff(int midiChannel, int midiNoteNumber, float velocity);
    void handleMidiControlChange(int midiChannel, int controlNumber, int value);
    void handleMidiPitchWheel(int midiChannel, int pitch);
    void handleMidiModulation(int midiChannel, int value);
    void handleMidiAftertouch(int midiChannel, int noteNumber, int value);
    
    // Parameter helpers
    juce::String getParameterDisplayText(float value, int parameterIndex) const;
    float getParameterNormalizedValue(int parameterIndex) const;
    void setParameterNormalizedValue(int parameterIndex, float normalizedValue);
    
    // State helpers
    juce::XmlElement createStateXml() const;
    void restoreStateFromXml(const juce::XmlElement& xml);
    
    // Utility methods
    void updateParameterName(int index, const juce::String& name);
    void updateProgramName(int index, const juce::String& name);
    bool isParameterValid(int index) const;
    bool isProgramValid(int index) const;
    
    // JUCE leak detection
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VitalPluginProcessor)
    JUCE_DECLARE_NON_COPYABLE(VitalPluginProcessor)
};

} // namespace vital
