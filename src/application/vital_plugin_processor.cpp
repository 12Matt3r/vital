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

#include "vital_plugin_processor.h"
#include "vital_plugin_editor.h"
#include "audio_engine/vital_audio_engine.h"
#include "juce_modernization/parameter_manager.h"
#include "developer_tools/preset_management.h"

namespace vital {

//==============================================================================
VitalPluginProcessor::VitalPluginProcessor()
    : juce::AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters_(vital::plugin_config::MAX_PARAMETERS, 0.0f),
      parameter_names_(vital::plugin_config::MAX_PARAMETERS),
      parameter_labels_(vital::plugin_config::MAX_PARAMETERS),
      program_names_(vital::plugin_config::MAX_PROGRAMS),
      midi_buffer_(8192) {
    
    initializeParameters();
    initializePrograms();
    setupBuses();
    
    // Add parameter listener
    addParameterListener(this);
    
    // Initialize MIDI keyboard state
    midi_keyboard_state_.addListener(this);
}

VitalPluginProcessor::VitalPluginProcessor(const juce::BusesProperties& ioLayouts)
    : juce::AudioProcessor(ioLayouts),
      parameters_(vital::plugin_config::MAX_PARAMETERS, 0.0f),
      parameter_names_(vital::plugin_config::MAX_PARAMETERS),
      parameter_labels_(vital::plugin_config::MAX_PARAMETERS),
      program_names_(vital::plugin_config::MAX_PROGRAMS),
      midi_buffer_(8192) {
    
    initializeParameters();
    initializePrograms();
    setupBuses();
    
    // Add parameter listener
    addParameterListener(this);
    
    // Initialize MIDI keyboard state
    midi_keyboard_state_.addListener(this);
}

VitalPluginProcessor::~VitalPluginProcessor() {
    // Clean up resources
    if (parameter_manager_) {
        parameter_manager_->cleanup();
    }
    
    if (preset_manager_) {
        preset_manager_->cleanup();
    }
}

//==============================================================================
void VitalPluginProcessor::prepareToPlay(double sampleRate, int samplesPerBlock) {
    try {
        // Initialize audio engine
        audio_engine_ = std::make_shared<VitalAudioEngine>();
        if (!audio_engine_->initialize(static_cast<int>(sampleRate), samplesPerBlock)) {
            DBG("Warning: Audio engine initialization failed in plugin");
        }
        
        // Initialize parameter manager
        parameter_manager_ = std::make_shared<ParameterManager>();
        if (!parameter_manager_->initialize()) {
            DBG("Warning: Parameter manager initialization failed in plugin");
        }
        
        // Initialize preset manager
        preset_manager_ = std::make_shared<PresetManager>();
        if (!preset_manager_->initialize()) {
            DBG("Warning: Preset manager initialization failed in plugin");
        }
        
        // Initialize processing buffers
        dry_buffer_.setSize(2, samplesPerBlock);
        wet_buffer_.setSize(2, samplesPerBlock);
        dry_buffer_.clear();
        wet_buffer_.clear();
        
        // Setup MIDI buffer
        midi_buffer_.clear();
        
        DBG("Plugin prepared to play: sampleRate=" << sampleRate << ", blockSize=" << samplesPerBlock);
        
    } catch (const std::exception& e) {
        DBG("Exception in prepareToPlay: " << e.what());
    }
}

void VitalPluginProcessor::releaseResources() {
    // Release audio engine resources
    if (audio_engine_) {
        audio_engine_->cleanup();
        audio_engine_.reset();
    }
    
    // Clear processing buffers
    dry_buffer_.setSize(0, 0);
    wet_buffer_.setSize(0, 0);
    
    // Clear MIDI buffer
    midi_buffer_.clear();
    
    DBG("Plugin resources released");
}

//==============================================================================
void VitalPluginProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
    
    // Clear unused output channels
    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i) {
        buffer.clear(i, 0, buffer.getNumSamples());
    }
    
    // Process MIDI messages
    if (!midi.isEmpty()) {
        processMidiMessages(midi);
    }
    
    // Update parameters
    updateParameters();
    
    // Process audio through engine
    if (!is_bypassed_ && audio_engine_) {
        // Process through audio engine
        audio_engine_->processBlock(buffer, midi_buffer_);
        
        // Clear MIDI buffer after processing
        midi_buffer_.clear();
    }
    
    // Apply gain and output processing
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        float* output = buffer.getWritePointer(channel);
        float master_volume = parameters_[vital::plugin::MASTER_VOLUME];
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            output[sample] *= master_volume;
        }
    }
}

void VitalPluginProcessor::processBlock(juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midi) {
    // Convert to float processing
    juce::AudioBuffer<float> floatBuffer(buffer.getNumChannels(), buffer.getNumSamples());
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            floatBuffer.setSample(ch, i, static_cast<float>(buffer.getSample(ch, i)));
        }
    }
    
    processBlock(floatBuffer, midi);
    
    // Convert back to double
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch) {
        for (int i = 0; i < buffer.getNumSamples(); ++i) {
            buffer.setSample(ch, i, static_cast<double>(floatBuffer.getSample(ch, i)));
        }
    }
}

void VitalPluginProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                       const juce::MidiBuffer::Iterator& midiIterator) {
    // Alternative processing interface
    juce::MidiBuffer midiBuffer;
    juce::MidiBuffer::Iterator iterator = midiIterator;
    juce::MidiMessage midiMessage;
    int sampleNumber;
    
    while (iterator.getNextEvent(midiMessage, sampleNumber)) {
        midiBuffer.addEvent(midiMessage, sampleNumber);
    }
    
    processBlock(buffer, midiBuffer);
}

void VitalPluginProcessor::processBlockBypassed(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
    // Pass through processing for bypass mode
    // Apply minimal processing (gain, etc.)
    
    float master_volume = parameters_[vital::plugin::MASTER_VOLUME];
    float master_pan = parameters_[vital::plugin::MASTER_PAN];
    
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        float* output = buffer.getWritePointer(channel);
        float gain = (channel == 0) ? (1.0f - master_pan) : (1.0f + master_pan);
        
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            output[sample] *= master_volume * gain;
        }
    }
    
    // Still process MIDI messages for note tracking
    if (!midi.isEmpty()) {
        processMidiMessages(midi);
    }
}

//==============================================================================
bool VitalPluginProcessor::hasEditor() const {
    return true;
}

const juce::String VitalPluginProcessor::getEditorName() const {
    return "Vital Synthesizer Editor";
}

juce::AudioProcessorEditor* VitalPluginProcessor::createEditor() {
    return new vital::VitalPluginEditor(*this);
}

//==============================================================================
bool VitalPluginProcessor::isParameterAutomatable(int parameterIndex) const {
    if (isParameterValid(parameterIndex)) {
        // All Vital parameters are automatable
        return true;
    }
    return false;
}

//==============================================================================
float VitalPluginProcessor::getParameter(int index) {
    if (isParameterValid(index)) {
        return parameters_[index];
    }
    return 0.0f;
}

void VitalPluginProcessor::setParameter(int index, float newValue) {
    if (isParameterValid(index)) {
        float clampedValue = juce::jlimit(0.0f, 1.0f, newValue);
        parameters_[index] = clampedValue;
        
        // Update parameter manager if available
        if (parameter_manager_) {
            parameter_manager_->setParameter(index, clampedValue);
        }
        
        // Mark parameter as changed
        sendParamChangeMessageToListeners(index, clampedValue);
    }
}

const juce::String VitalPluginProcessor::getParameterName(int index) const {
    if (isParameterValid(index)) {
        return parameter_names_[index];
    }
    return "Unknown Parameter";
}

const juce::String VitalPluginProcessor::getParameterText(int index) const {
    if (isParameterValid(index)) {
        return getParameterDisplayText(parameters_[index], index);
    }
    return "0";
}

juce::String VitalPluginProcessor::getParameterLabel(int index) const {
    if (isParameterValid(index)) {
        return parameter_labels_[index];
    }
    return "";
}

//==============================================================================
int VitalPluginProcessor::getCurrentProgram() {
    return current_program_;
}

void VitalPluginProcessor::setCurrentProgram(int index) {
    if (isProgramValid(index)) {
        current_program_ = index;
        
        // Load preset
        if (preset_manager_) {
            preset_manager_->loadProgram(index);
        }
        
        // Notify editor
        sendChangeMessage();
    }
}

const juce::String VitalPluginProcessor::getProgramName(int index) {
    if (isProgramValid(index)) {
        return program_names_[index];
    }
    return "Program " + juce::String(index);
}

void VitalPluginProcessor::changeProgramName(int index, const juce::String& newName) {
    if (isProgramValid(index)) {
        program_names_[index] = newName;
        
        // Update preset manager
        if (preset_manager_) {
            preset_manager_->renameProgram(index, newName);
        }
        
        // Notify editor
        sendChangeMessage();
    }
}

//==============================================================================
void VitalPluginProcessor::getStateInformation(juce::MemoryBlock& destData) {
    auto xml = createStateXml();
    auto string = xml->toString();
    destData.append(string.toRawUTF8(), string.getNumBytesAsUTF8());
}

void VitalPluginProcessor::setStateInformation(const void* data, int sizeInBytes) {
    auto xml = juce::XmlDocument::parse(juce::String::createStringFromData(data, sizeInBytes));
    if (xml && xml->hasTagName("VITAL")) {
        restoreStateFromXml(*xml);
    }
}

//==============================================================================
void VitalPluginProcessor::parameterValueChanged(int parameterIndex, float newValue) {
    // Handle parameter changes from host
    setParameter(parameterIndex, newValue);
}

void VitalPluginProcessor::parameterGestureChanged(int parameterIndex, bool gestureIsStarting) {
    // Handle parameter automation gestures
    DBG("Parameter " << parameterIndex << " gesture " << (gestureIsStarting ? "started" : "ended"));
}

//==============================================================================
void VitalPluginProcessor::handleNoteOn(juce::MidiKeyboardState* state, int midiChannel, int midiNoteNumber, float velocity) {
    handleMidiNoteOn(midiChannel, midiNoteNumber, velocity);
}

void VitalPluginProcessor::handleNoteOff(juce::MidiKeyboardState* state, int midiChannel, int midiNoteNumber, float velocity) {
    handleMidiNoteOff(midiChannel, midiNoteNumber, velocity);
}

//==============================================================================
void VitalPluginProcessor::loadFromXml(const juce::XmlElement& xml) {
    if (xml.hasTagName("VITAL")) {
        restoreStateFromXml(xml);
    }
}

void VitalPluginProcessor::saveToXml(juce::XmlElement& xml) const {
    // Add plugin-specific data to XML
    auto pluginData = xml.createNewChildElement("PluginData");
    pluginData->setAttribute("version", vital::app::VERSION_STRING);
    pluginData->setAttribute("currentProgram", current_program_);
    
    // Add parameters
    auto params = pluginData->createNewChildElement("Parameters");
    for (int i = 0; i < getNumParameters(); ++i) {
        if (parameters_[i] != 0.0f) { // Only save non-default values
            auto param = params->createNewChildElement("Parameter");
            param->setAttribute("index", i);
            param->setAttribute("value", parameters_[i]);
        }
    }
    
    // Add programs
    auto programs = pluginData->createNewChildElement("Programs");
    for (int i = 0; i < getNumPrograms(); ++i) {
        if (program_names_[i].isNotEmpty()) {
            auto program = programs->createNewChildElement("Program");
            program->setAttribute("index", i);
            program->setAttribute("name", program_names_[i]);
        }
    }
}

//==============================================================================
bool VitalPluginProcessor::isBusesLayoutSupported(const juce::BusesLayout& layouts) const {
    // Support stereo input/output
    if (layouts.getMainInputChannelSet() == juce::AudioChannelSet::stereo() &&
        layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()) {
        return true;
    }
    
    // Support mono input/output
    if (layouts.getMainInputChannelSet() == juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() == juce::AudioChannelSet::mono()) {
        return true;
    }
    
    // Support mono input, stereo output
    if (layouts.getMainInputChannelSet() == juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() == juce::AudioChannelSet::stereo()) {
        return true;
    }
    
    return false;
}

//==============================================================================
void VitalPluginProcessor::initializeParameters() {
    // Initialize parameter names and labels
    parameter_names_[vital::plugin::OSC1_WAVE] = "Osc 1 Wave";
    parameter_labels_[vital::plugin::OSC1_WAVE] = "";
    
    parameter_names_[vital::plugin::OSC1_FREQ] = "Osc 1 Freq";
    parameter_labels_[vital::plugin::OSC1_FREQ] = "Hz";
    
    parameter_names_[vital::plugin::OSC1_DETUNE] = "Osc 1 Detune";
    parameter_labels_[vital::plugin::OSC1_DETUNE] = "cents";
    
    parameter_names_[vital::plugin::OSC2_WAVE] = "Osc 2 Wave";
    parameter_labels_[vital::plugin::OSC2_WAVE] = "";
    
    parameter_names_[vital::plugin::OSC2_FREQ] = "Osc 2 Freq";
    parameter_labels_[vital::plugin::OSC2_FREQ] = "Hz";
    
    parameter_names_[vital::plugin::OSC2_DETUNE] = "Osc 2 Detune";
    parameter_labels_[vital::plugin::OSC2_DETUNE] = "cents";
    
    parameter_names_[vital::plugin::FILTER_CUTOFF] = "Filter Cutoff";
    parameter_labels_[vital::plugin::FILTER_CUTOFF] = "Hz";
    
    parameter_names_[vital::plugin::FILTER_RESONANCE] = "Filter Resonance";
    parameter_labels_[vital::plugin::FILTER_RESONANCE] = "";
    
    parameter_names_[vital::plugin::ENV1_ATTACK] = "Env 1 Attack";
    parameter_labels_[vital::plugin::ENV1_ATTACK] = "sec";
    
    parameter_names_[vital::plugin::ENV1_DECAY] = "Env 1 Decay";
    parameter_labels_[vital::plugin::ENV1_DECAY] = "sec";
    
    parameter_names_[vital::plugin::ENV1_SUSTAIN] = "Env 1 Sustain";
    parameter_labels_[vital::plugin::ENV1_SUSTAIN] = "";
    
    parameter_names_[vital::plugin::ENV1_RELEASE] = "Env 1 Release";
    parameter_labels_[vital::plugin::ENV1_RELEASE] = "sec";
    
    parameter_names_[vital::plugin::MASTER_VOLUME] = "Master Volume";
    parameter_labels_[vital::plugin::MASTER_VOLUME] = "dB";
    
    parameter_names_[vital::plugin::MASTER_PAN] = "Master Pan";
    parameter_labels_[vital::plugin::MASTER_PAN] = "";
    
    // Initialize default values
    parameters_[vital::plugin::OSC1_WAVE] = 0.0f; // Sine
    parameters_[vital::plugin::OSC2_WAVE] = 0.0f; // Sine
    parameters_[vital::plugin::MASTER_VOLUME] = 0.707f; // -3dB
    parameters_[vital::plugin::MASTER_PAN] = 0.0f; // Center
    parameters_[vital::plugin::FILTER_CUTOFF] = 0.5f; // Mid position
    parameters_[vital::plugin::FILTER_RESONANCE] = 0.1f; // Low resonance
}

void VitalPluginProcessor::initializePrograms() {
    // Initialize program names
    for (int i = 0; i < vital::plugin_config::MAX_PROGRAMS; ++i) {
        program_names_[i] = "Program " + juce::String(i + 1);
    }
    
    // Set default programs
    program_names_[0] = "Default";
    program_names_[1] = "Lead";
    program_names_[2] = "Bass";
    program_names_[3] = "Pad";
    program_names_[4] = "Pluck";
}

void VitalPluginProcessor::setupBuses() {
    // Setup input/output buses
    setBusLayout(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true));
}

//==============================================================================
void VitalPluginProcessor::processMidiMessages(const juce::MidiBuffer& midiBuffer) {
    juce::MidiBuffer::Iterator midiIterator(midiBuffer);
    juce::MidiMessage midiMessage;
    int sampleNumber;
    
    while (midiIterator.getNextEvent(midiMessage, sampleNumber)) {
        handleMidiMessage(midiMessage);
    }
}

void VitalPluginProcessor::updateParameters() {
    // Update audio engine parameters
    if (parameter_manager_) {
        for (int i = 0; i < vital::plugin_config::MAX_PARAMETERS; ++i) {
            if (isParameterValid(i)) {
                parameter_manager_->setParameter(i, parameters_[i]);
            }
        }
    }
    
    // Update arpeggiator if enabled
    if (arpeggiator_enabled_) {
        updateArpeggiator();
    }
}

void VitalPluginProcessor::updateArpeggiator() {
    // Arpeggiator processing logic
    // This would generate MIDI notes based on arpeggiator pattern
}

void VitalPluginProcessor::applyParameterChanges() {
    // Apply parameter changes to audio engine in real-time
    if (audio_engine_) {
        audio_engine_->updateParameters();
    }
}

//==============================================================================
void VitalPluginProcessor::handleMidiMessage(const juce::MidiMessage& message) {
    if (message.isNoteOn()) {
        handleMidiNoteOn(message.getChannel(), message.getNoteNumber(), message.getVelocity() / 127.0f);
    } else if (message.isNoteOff()) {
        handleMidiNoteOff(message.getChannel(), message.getNoteNumber(), 0.0f);
    } else if (message.isControlChange()) {
        handleMidiControlChange(message.getChannel(), message.getControllerNumber(), message.getControllerValue());
    } else if (message.isPitchWheel()) {
        handleMidiPitchWheel(message.getChannel(), message.getPitchWheelValue());
    }
}

void VitalPluginProcessor::handleMidiNoteOn(int midiChannel, int midiNoteNumber, float velocity) {
    if (audio_engine_) {
        audio_engine_->noteOn(midiChannel, midiNoteNumber, velocity);
    }
    
    // Store in MIDI buffer for processing
    juce::MidiMessage message = juce::MidiMessage::noteOn(midiChannel, midiNoteNumber, velocity);
    midi_buffer_.addEvent(message, 0);
}

void VitalPluginProcessor::handleMidiNoteOff(int midiChannel, int midiNoteNumber, float velocity) {
    if (audio_engine_) {
        audio_engine_->noteOff(midiChannel, midiNoteNumber, velocity);
    }
    
    // Store in MIDI buffer for processing
    juce::MidiMessage message = juce::MidiMessage::noteOff(midiChannel, midiNoteNumber, velocity);
    midi_buffer_.addEvent(message, 0);
}

void VitalPluginProcessor::handleMidiControlChange(int midiChannel, int controlNumber, int value) {
    // Handle MIDI CC messages
    switch (controlNumber) {
        case 1: // Modulation wheel
            handleMidiModulation(midiChannel, value);
            break;
        case 7: // Volume
            setParameter(vital::plugin::MASTER_VOLUME, value / 127.0f);
            break;
        case 10: // Pan
            setParameter(vital::plugin::MASTER_PAN, (value - 64) / 64.0f);
            break;
        case 64: // Sustain pedal
            // Handle sustain pedal
            break;
    }
}

void VitalPluginProcessor::handleMidiPitchWheel(int midiChannel, int pitch) {
    // Handle pitch wheel messages
    if (audio_engine_) {
        audio_engine_->handlePitchWheel(midiChannel, pitch / 8192.0f - 1.0f);
    }
}

void VitalPluginProcessor::handleMidiModulation(int midiChannel, int value) {
    // Handle modulation wheel
    if (audio_engine_) {
        audio_engine_->handleModulation(midiChannel, value / 127.0f);
    }
}

void VitalPluginProcessor::handleMidiAftertouch(int midiChannel, int noteNumber, int value) {
    // Handle aftertouch
    if (audio_engine_) {
        audio_engine_->handleAftertouch(midiChannel, noteNumber, value / 127.0f);
    }
}

//==============================================================================
juce::String VitalPluginProcessor::getParameterDisplayText(float value, int parameterIndex) const {
    switch (parameterIndex) {
        case vital::plugin::OSC1_FREQ:
        case vital::plugin::OSC2_FREQ:
            return juce::String(20.0f + value * 1980.0f, 0) + " Hz";
            
        case vital::plugin::OSC1_DETUNE:
        case vital::plugin::OSC2_DETUNE:
            return juce::String(value * 1200.0f, 0) + " cents";
            
        case vital::plugin::FILTER_CUTOFF:
            return juce::String(20.0f + value * 19800.0f, 0) + " Hz";
            
        case vital::plugin::MASTER_VOLUME:
            return juce::String(juce::Decibels::toGain(value * 100.0f - 60.0f), 1) + " dB";
            
        case vital::plugin::MASTER_PAN:
            if (value < 0.5f) {
                return "L" + juce::String((0.5f - value) * 200.0f, 0);
            } else {
                return "R" + juce::String((value - 0.5f) * 200.0f, 0);
            }
            
        default:
            return juce::String(value * 100.0f, 1) + "%";
    }
}

float VitalPluginProcessor::getParameterNormalizedValue(int parameterIndex) const {
    if (isParameterValid(parameterIndex)) {
        return parameters_[parameterIndex];
    }
    return 0.0f;
}

void VitalPluginProcessor::setParameterNormalizedValue(int parameterIndex, float normalizedValue) {
    setParameter(parameterIndex, normalizedValue);
}

//==============================================================================
juce::XmlElement VitalPluginProcessor::createStateXml() const {
    auto xml = std::make_unique<juce::XmlElement>("VITAL");
    xml->setAttribute("version", vital::app::VERSION_STRING);
    xml->setAttribute("program", current_program_);
    
    // Save parameters
    auto params = xml->createNewChildElement("Parameters");
    for (int i = 0; i < vital::plugin_config::MAX_PARAMETERS; ++i) {
        if (parameters_[i] != 0.0f) {
            auto param = params->createNewChildElement("Parameter");
            param->setAttribute("index", i);
            param->setAttribute("value", parameters_[i]);
        }
    }
    
    // Save programs
    auto programs = xml->createNewChildElement("Programs");
    for (int i = 0; i < vital::plugin_config::MAX_PROGRAMS; ++i) {
        if (program_names_[i].isNotEmpty()) {
            auto program = programs->createNewChildElement("Program");
            program->setAttribute("index", i);
            program->setAttribute("name", program_names_[i]);
        }
    }
    
    return *xml;
}

void VitalPluginProcessor::restoreStateFromXml(const juce::XmlElement& xml) {
    if (xml.hasTagName("VITAL")) {
        // Restore program
        current_program_ = xml.getIntAttribute("program", 0);
        
        // Restore parameters
        auto* params = xml.getChildByName("Parameters");
        if (params) {
            for (auto* param : params->getChildElements()) {
                int index = param->getIntAttribute("index", -1);
                float value = param->getDoubleAttribute("value", 0.0);
                if (isParameterValid(index)) {
                    parameters_[index] = static_cast<float>(value);
                }
            }
        }
        
        // Restore programs
        auto* programs = xml.getChildByName("Programs");
        if (programs) {
            for (auto* program : programs->getChildElements()) {
                int index = program->getIntAttribute("index", -1);
                juce::String name = program->getStringAttribute("name", "");
                if (isProgramValid(index)) {
                    program_names_[index] = name;
                }
            }
        }
        
        sendChangeMessage();
    }
}

//==============================================================================
void VitalPluginProcessor::updateParameterName(int index, const juce::String& name) {
    if (isParameterValid(index)) {
        parameter_names_[index] = name;
        sendChangeMessage();
    }
}

void VitalPluginProcessor::updateProgramName(int index, const juce::String& name) {
    if (isProgramValid(index)) {
        program_names_[index] = name;
        sendChangeMessage();
    }
}

bool VitalPluginProcessor::isParameterValid(int index) const {
    return index >= 0 && index < vital::plugin_config::MAX_PARAMETERS;
}

bool VitalPluginProcessor::isProgramValid(int index) const {
    return index >= 0 && index < vital::plugin_config::MAX_PROGRAMS;
}

//==============================================================================
float VitalPluginProcessor::getCPULoad() const {
    std::lock_guard<std::mutex> lock(performance_mutex_);
    return current_cpu_load_;
}

float VitalPluginProcessor::getCurrentLatency() const {
    std::lock_guard<std::mutex> lock(performance_mutex_);
    return current_latency_;
}

size_t VitalPluginProcessor::getMemoryUsage() const {
    std::lock_guard<std::mutex> lock(performance_mutex_);
    return memory_usage_;
}

//==============================================================================
void VitalPluginProcessor::enableArpeggiator(bool enabled) {
    arpeggiator_enabled_ = enabled;
}

void VitalPluginProcessor::setArpeggiatorPattern(const juce::String& pattern) {
    arpeggiator_pattern_ = pattern;
}

void VitalPluginProcessor::setArpeggiatorRate(float rate) {
    arpeggiator_rate_ = juce::jlimit(20.0f, 300.0f, rate);
}

//==============================================================================
juce::StringArray VitalPluginProcessor::getPresetCategories() const {
    return juce::StringArray("All", "Lead", "Bass", "Pad", "Pluck", "FX");
}

juce::StringArray VitalPluginProcessor::getPresetsInCategory(const juce::String& category) const {
    if (category == "All") {
        juce::StringArray presets;
        for (int i = 0; i < vital::plugin_config::MAX_PROGRAMS; ++i) {
            if (program_names_[i].isNotEmpty()) {
                presets.add(program_names_[i]);
            }
        }
        return presets;
    }
    
    // Return presets filtered by category
    return juce::StringArray();
}

void VitalPluginProcessor::copyFromProcessor(const VitalPluginProcessor& other) {
    parameters_ = other.parameters_;
    program_names_ = other.program_names_;
    current_program_ = other.current_program_;
    sendChangeMessage();
}

void VitalPluginProcessor::pasteToProcessor(VitalPluginProcessor& other) const {
    other.parameters_ = parameters_;
    other.program_names_ = program_names_;
    other.current_program_ = current_program_;
    other.sendChangeMessage();
}

} // namespace vital
