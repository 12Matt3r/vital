/*
  ==============================================================================
    vital_plugin.cpp
    Copyright (c) 2025 Vital Plugin Integration Team
    https://vital.audio
    
    Main plugin wrapper implementation for Vital synthesizer
    Provides DAW integration with parameter management, MIDI handling,
    and plugin state management for VST3/AU formats.
  ==============================================================================
*/

#include "vital_plugin.h"
#include "../audio_engine/vital_audio_engine.h"
#include "plugin_parameters.h"
#include "plugin_state.h"
#include "plugin_midi.h"
#include "plugin_ui.h"
#include <chrono>
#include <thread>

namespace vital {
namespace plugin {

//==============================================================================
/**
 * @class VitalPluginImplementation
 * @brief Implementation details for VitalPlugin
 */
class VitalPlugin::Implementation {
public:
    Implementation(VitalPlugin* plugin) : plugin_(plugin) {}
    ~Implementation() = default;
    
    // Plugin lifecycle
    bool initialize();
    void shutdown();
    
    // Processing methods
    void processBlockInternal(juce::AudioBuffer<float>& buffer, const juce::MidiBuffer& midi);
    void handleMidiMessage(const juce::MidiMessage& message, double timeStamp);
    
    // Parameter management
    void updateParameters();
    void applyAutomation();
    
    // Performance monitoring
    void updateMetrics();
    void optimizePerformance();
    
private:
    VitalPlugin* plugin_ = nullptr;
};

//==============================================================================
// VitalPlugin Implementation

VitalPlugin::VitalPlugin() : VitalPlugin(PluginConfig()) {}

VitalPlugin::VitalPlugin(const PluginConfig& config) 
    : config_(config)
    , parameters_(this)
    , stateManager_(this)
    , midiHandler_(this)
{
    // Detect plugin format
    detectPluginFormat();
    
    // Initialize engine with configuration
    auto engineConfig = audio_engine::VitalAudioEngine::Config();
    engineConfig.maxVoices = config_.maxVoices;
    engineConfig.enableOversampling = config_.enableOversampling;
    engineConfig.oversamplingFactor = config_.oversamplingFactor;
    engineConfig.enableSIMD = config_.enableSIMD;
    engineConfig.enableMultithreading = config_.enableMultithreading;
    
    audioEngine_ = std::make_unique<audio_engine::VitalAudioEngine>(engineConfig);
    
    // Set plugin properties
    setNumInputChannels(config_.numInputs);
    setNumOutputChannels(config_.numOutputs);
    setHostType(juce::AudioProcessor::getHostType());
    
    // Initialize automation storage
    parameterAutomation_.resize(getNumParameters());
    parameterSmoothing_.resize(getNumParameters(), 1.0f); // 1ms smoothing
    
    // Enable realtime processing
    addListener(this);
    
    logMessage("VitalPlugin initialized successfully");
}

VitalPlugin::~VitalPlugin() {
    shutdown();
}

//==============================================================================
// AudioProcessor Overrides

void VitalPlugin::prepareToPlay(double sampleRate, int samplesPerBlock) {
    logMessage("prepareToPlay called - SampleRate: " + juce::String(sampleRate) + 
               ", BlockSize: " + juce::String(samplesPerBlock));
    
    // Update engine configuration
    auto engineConfig = audioEngine_->getConfig();
    engineConfig.sampleRate = sampleRate;
    engineConfig.bufferSize = samplesPerBlock;
    
    // Initialize engine
    if (!initializeEngine()) {
        setError("Failed to initialize audio engine", "prepareToPlay");
        return;
    }
    
    // Initialize MIDI handling
    initializeMidi();
    
    // Setup performance monitoring
    if (performanceModeEnabled_) {
        performanceTimer_ = std::make_unique<juce::Timer>();
        performanceTimer_->startTimer(100); // 10Hz update rate
    }
    
    // Start processing
    processingActive_ = true;
    state_.isProcessing = true;
    
    logMessage("prepareToPlay completed successfully");
}

void VitalPlugin::releaseResources() {
    logMessage("releaseResources called");
    
    // Stop timers
    if (performanceTimer_) {
        performanceTimer_->stopTimer();
        performanceTimer_.reset();
    }
    
    if (uiUpdateTimer_) {
        uiUpdateTimer_->stopTimer();
        uiUpdateTimer_.reset();
    }
    
    // Stop processing
    processingActive_ = false;
    state_.isProcessing = false;
    
    // Shutdown systems
    shutdownMidi();
    shutdownEngine();
    
    logMessage("releaseResources completed");
}

void VitalPlugin::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi) {
    if (!processingActive_) {
        buffer.clear();
        return;
    }
    
    // Clear any leftover MIDI messages from input channels we don't handle
    juce::MidiBuffer::Iterator midiIterator(midi);
    juce::MidiMessage midiMessage;
    int sampleNumber;
    
    std::vector<std::pair<juce::MidiMessage, int>> filteredMidi;
    
    while (midiIterator.getNextEvent(midiMessage, sampleNumber)) {
        if (midiMessage.isNoteOn() || midiMessage.isNoteOff() || midiMessage.isController()) {
            filteredMidi.emplace_back(midiMessage, sampleNumber);
        }
    }
    
    // Process filtered MIDI
    for (const auto& [msg, sample] : filteredMidi) {
        handleIncomingMidiMessage(nullptr, msg);
    }
    
    // Clear original buffer and add processed messages
    midi.clear();
    for (const auto& [msg, sample] : filteredMidi) {
        midi.addEvent(msg, sample);
    }
    
    // Process audio block
    processAudioBlock(buffer, midi);
    
    // Update performance metrics
    updatePerformanceMetrics();
}

void VitalPlugin::processBlock(juce::AudioBuffer<double>& buffer, juce::MidiBuffer& midi) {
    // Convert to float for processing
    juce::AudioBuffer<float> floatBuffer(buffer.getNumChannels(), buffer.getNumSamples());
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            floatBuffer.setSample(channel, sample, static_cast<float>(buffer.getSample(channel, sample)));
        }
    }
    
    // Process as float buffer
    processBlock(floatBuffer, midi);
    
    // Convert back to double
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
            buffer.setSample(channel, sample, static_cast<double>(floatBuffer.getSample(channel, sample)));
        }
    }
}

//==============================================================================
// Plugin Information

const juce::String VitalPlugin::getName() const {
    return "Vital";
}

bool VitalPlugin::acceptsMidi() const {
    return true;
}

bool VitalPlugin::producesMidi() const {
    return false;
}

bool VitalPlugin::isMidiEffect() const {
    return false;
}

double VitalPlugin::getTailLengthSeconds() const {
    return 0.0;
}

//==============================================================================
// Program/Preset Management

int VitalPlugin::getNumPrograms() {
    return 128; // Maximum preset slots
}

int VitalPlugin::getCurrentProgram() {
    return stateManager_.getCurrentProgram();
}

void VitalPlugin::setCurrentProgram(int index) {
    if (index >= 0 && index < getNumPrograms()) {
        stateManager_.setCurrentProgram(index);
        updateAllParameters();
        sendChangeMessage();
    }
}

const juce::String VitalPlugin::getProgramName(int index) {
    return stateManager_.getProgramName(index);
}

void VitalPlugin::changeProgramName(int index, const juce::String& newName) {
    if (index >= 0 && index < getNumPrograms()) {
        stateManager_.setProgramName(index, newName);
        sendChangeMessage();
    }
}

//==============================================================================
// Parameter Management

int VitalPlugin::getNumParameters() {
    return parameters_.getNumParameters();
}

float VitalPlugin::getParameter(int index) {
    return parameters_.getValue(index);
}

void VitalPlugin::setParameter(int index, float newValue) {
    parameters_.setValue(index, newValue);
    
    // Apply to audio engine
    if (audioEngine_) {
        audioEngine_->setParameter(index, newValue);
    }
    
    // Update automation
    if (getCurrentPosition()) {
        scheduleAutomation(index, newValue, getCurrentPosition()->getPosition().timeInSeconds);
    }
}

const juce::String VitalPlugin::getParameterName(int index) {
    return parameters_.getName(index);
}

const juce::String VitalPlugin::getParameterText(int index) {
    return parameters_.getText(index);
}

bool VitalPlugin::isParameterAutomatable(int index) const {
    return parameters_.isAutomatable(index);
}

bool VitalPlugin::isMetaParameter(int index) const {
    return parameters_.isMetaParameter(index);
}

//==============================================================================
// MIDI Processing

void VitalPlugin::handleIncomingMidiMessage(juce::MidiInput* source, const juce::MidiMessage& message) {
    // Handle different MIDI message types
    if (message.isNoteOn()) {
        int note = message.getNoteNumber();
        float velocity = message.getVelocity() / 127.0f;
        int channel = message.getChannel();
        
        audioEngine_->noteOn(note, velocity, channel);
        
    } else if (message.isNoteOff()) {
        int note = message.getNoteNumber();
        int channel = message.getChannel();
        
        audioEngine_->noteOff(note, channel);
        
    } else if (message.isController()) {
        int cc = message.getControllerNumber();
        int value = message.getControllerValue();
        int channel = message.getChannel();
        
        processCC(cc, value, channel);
        
    } else if (message.isPitchWheel()) {
        int pitch = message.getPitchWheelValue();
        // Apply pitch bend (range ±2 semitones)
        float pitchBend = (pitch - 8192) / 8192.0f * 2.0f;
        audioEngine_->setParameter(0, pitchBend); // Assuming pitch bend is parameter 0
        
    } else if (message.isAftertouch()) {
        int aftertouch = message.getAfterTouchValue();
        float atPressure = aftertouch / 127.0f;
        // Apply channel pressure
        audioEngine_->setParameter(1, atPressure); // Assuming aftertouch is parameter 1
    }
    
    // Handle MPE if enabled
    if (state_.supportsMPE) {
        processMPE(message);
    }
}

//==============================================================================
// Plugin State Management

void VitalPlugin::getStateInformation(juce::MemoryBlock& destData) {
    // Create XML state
    auto xmlState = std::make_unique<juce::XmlElement>("VitalPluginState");
    
    // Plugin metadata
    xmlState->setAttribute("version", "3.0");
    xmlState->setAttribute("format", getCurrentPluginFormat());
    xmlState->setAttribute("timestamp", juce::Time::currentTimeMillis());
    
    // Parameters
    auto parametersXml = xmlState->createNewChildElement("Parameters");
    for (int i = 0; i < getNumParameters(); ++i) {
        auto paramXml = parametersXml->createNewChildElement("Parameter");
        paramXml->setAttribute("id", i);
        paramXml->setAttribute("name", getParameterName(i));
        paramXml->setAttribute("value", getParameter(i));
    }
    
    // Program information
    auto programXml = xmlState->createNewChildElement("Program");
    programXml->setAttribute("current", getCurrentProgram());
    programXml->setAttribute("name", getProgramName(getCurrentProgram()));
    
    // Engine state
    auto engineXml = xmlState->createNewChildElement("Engine");
    engineXml->setAttribute("masterGain", audioEngine_->getMasterGain());
    engineXml->setAttribute("masterTune", audioEngine_->getMasterTune());
    engineXml->setAttribute("bypassed", audioEngine_->isBypassed());
    
    // MIDI settings
    auto midiXml = xmlState->createNewChildElement("MIDI");
    midiXml->setAttribute("channel", audioEngine_->getMidiChannel());
    midiXml->setAttribute("midiLearn", audioEngine_->isMidiLearnEnabled());
    
    // Write to memory block
    copyXmlToBinary(*xmlState, destData);
}

void VitalPlugin::setStateInformation(const void* data, int sizeInBytes) {
    // Load XML state
    auto xmlState = getXmlFromBinary(data, sizeInBytes);
    if (!xmlState || !xmlState->hasTagName("VitalPluginState")) {
        logError("Invalid state data");
        return;
    }
    
    // Load parameters
    auto* parametersXml = xmlState->getChildByName("Parameters");
    if (parametersXml) {
        for (auto* paramXml : parametersXml->getChildIterator()) {
            if (paramXml->hasTagName("Parameter")) {
                int paramId = paramXml->getIntAttribute("id", -1);
                float value = paramXml->getDoubleAttribute("value", 0.0);
                
                if (paramId >= 0 && paramId < getNumParameters()) {
                    setParameter(paramId, value);
                }
            }
        }
    }
    
    // Load program
    auto* programXml = xmlState->getChildByName("Program");
    if (programXml) {
        int programIndex = programXml->getIntAttribute("current", 0);
        setCurrentProgram(programIndex);
    }
    
    // Load engine settings
    auto* engineXml = xmlState->getChildByName("Engine");
    if (engineXml) {
        float masterGain = engineXml->getDoubleAttribute("masterGain", 1.0);
        float masterTune = engineXml->getDoubleAttribute("masterTune", 0.0);
        bool bypassed = engineXml->getBoolAttribute("bypassed", false);
        
        audioEngine_->setMasterGain(masterGain);
        audioEngine_->setMasterTune(masterTune);
        audioEngine_->setMasterBypass(bypassed);
    }
    
    // Load MIDI settings
    auto* midiXml = xmlState->getChildByName("MIDI");
    if (midiXml) {
        int channel = midiXml->getIntAttribute("channel", 1);
        bool midiLearn = midiXml->getBoolAttribute("midiLearn", false);
        
        audioEngine_->setMidiChannel(channel);
        audioEngine_->enableMidiLearn(midiLearn);
    }
    
    sendChangeMessage();
}

//==============================================================================
// Editor Management

bool VitalPlugin::hasEditor() const {
    return config_.enableCustomUI;
}

juce::AudioProcessorEditor* VitalPlugin::createEditor() {
    if (!config_.enableCustomUI) {
        return new juce::GenericAudioProcessorEditor(*this);
    }
    
    editorUI_ = std::make_unique<PluginUI>(this);
    return editorUI_.get();
}

//==============================================================================
// AudioPlayHead Implementation

bool VitalPlugin::getCurrentPosition(juce::AudioPlayHead::CurrentPositionInfo& result) {
    // Clear all fields first
    juce::MemorySet(&result, 0, sizeof(result));
    
    // Update time information
    updateTimeInfo();
    
    result.bpm = state_.tempo;
    result.timeInSamples = static_cast<int64>(state_.currentTime * getSampleRate());
    result.timeInSeconds = state_.currentTime;
    result.ppqPosition = state_.ppqPosition;
    result.ppqPositionOfLastBarStart = state_.ppqPosition - (state_.ppqPosition % 4.0);
    
    // Calculate bar:beat:tick
    result.ppqPositionOfLastBarStart = (state_.ppqPosition / 4.0) * 4.0;
    double beatsIntoBar = state_.ppqPosition - result.ppqPositionOfLastBarStart;
    result.bar = static_cast<int>(state_.ppqPosition / 4.0);
    result.beat = static_cast<int>(beatsIntoBar) + 1;
    result.ticksPerBeat = static_cast<int>((beatsIntoBar - result.beat + 1) * 960.0);
    
    result.isPlaying = state_.isPlaying;
    result.isRecording = state_.isRecording;
    result.isLooping = state_.loopEnabled;
    result.loopPointsInBeats = {0, 16}; // Default 4 bars
    
    return true;
}

//==============================================================================
// Plugin Format Detection

juce::String VitalPlugin::getCurrentPluginFormat() const {
    return state_.currentPluginFormat;
}

bool VitalPlugin::isVST3() const {
    return getCurrentPluginFormat() == "VST3";
}

bool VitalPlugin::isAU() const {
    return getCurrentPluginFormat() == "AU";
}

bool VitalPlugin::isVST2() const {
    return getCurrentPluginFormat() == "VST2";
}

juce::String VitalPlugin::getDAWName() const {
    return state_.dawName;
}

juce::String VitalPlugin::getDAWVersion() const {
    return state_.dawVersion;
}

bool VitalPlugin::isInReaper() const {
    return getDAWName() == "REAPER";
}

bool VitalPlugin::isInLogic() const {
    return getDAWName() == "Logic Pro";
}

bool VitalPlugin::isInProTools() const {
    return getDAWName() == "Pro Tools";
}

bool VitalPlugin::isInCubase() const {
    return getDAWName() == "Cubase";
}

//==============================================================================
// UI Management

void VitalPlugin::showEditor() {
    if (editorUI_) {
        editorUI_->setVisible(true);
    }
}

void VitalPlugin::hideEditor() {
    if (editorUI_) {
        editorUI_->setVisible(false);
    }
}

bool VitalPlugin::isEditorVisible() const {
    return editorUI_ && editorUI_->isVisible();
}

void VitalPlugin::setEditorSize(int width, int height) {
    config_.uiWidth = width;
    config_.uiHeight = height;
    
    if (editorUI_) {
        editorUI_->setSize(width, height);
    }
}

juce::Rectangle<int> VitalPlugin::getEditorBounds() const {
    return state_.lastEditorBounds;
}

//==============================================================================
// Performance Monitoring

void VitalPlugin::enablePerformanceMode(bool enabled) {
    performanceModeEnabled_ = enabled;
    
    if (enabled) {
        // Start performance monitoring
        if (!performanceTimer_) {
            performanceTimer_ = std::make_unique<juce::Timer>();
            performanceTimer_->startTimer(100); // 10Hz
        }
    } else {
        // Stop performance monitoring
        if (performanceTimer_) {
            performanceTimer_->stopTimer();
            performanceTimer_.reset();
        }
    }
}

//==============================================================================
// Batch Processing

void VitalPlugin::updateAllParameters() {
    for (int i = 0; i < getNumParameters(); ++i) {
        float value = getParameter(i);
        audioEngine_->setParameter(i, value);
    }
}

void VitalPlugin::resetEngine() {
    if (audioEngine_) {
        audioEngine_->reset();
    }
    
    // Reset parameters to defaults
    for (int i = 0; i < getNumParameters(); ++i) {
        setParameter(i, getParameterDefaultValue(i));
    }
}

void VitalPlugin::suspendProcessing() {
    processingActive_ = false;
    state_.isSuspended = true;
    
    if (audioEngine_) {
        audioEngine_->suspend();
    }
}

void VitalPlugin::resumeProcessing() {
    processingActive_ = true;
    state_.isSuspended = false;
    
    if (audioEngine_) {
        audioEngine_->resume();
    }
}

//==============================================================================
// Error Handling

VitalPlugin::ErrorInfo VitalPlugin::getLastError() const {
    std::lock_guard<std::mutex> lock(errorMutex_);
    return lastError_;
}

void VitalPlugin::clearError() {
    std::lock_guard<std::mutex> lock(errorMutex_);
    lastError_.hasError = false;
}

//==============================================================================
// Utility Functions

void VitalPlugin::logMessage(const juce::String& message, const juce::String& level) {
    juce::Logger::writeToLog("[" + level + "] VitalPlugin: " + message);
    
    if (debugLogFile_.isNotEmpty()) {
        // Write to debug file if configured
        auto file = juce::File(debugLogFile_);
        if (file.existsAsFile() || file.getParentDirectory().createDirectory()) {
            file.appendText("[" + juce::Time::getCurrentTime().toString() + "] [" + level + "] " + message + "\n");
        }
    }
}

void VitalPlugin::logError(const juce::String& error) {
    logMessage(error, "ERROR");
}

bool VitalPlugin::validateState() const {
    if (!audioEngine_) {
        return false;
    }
    
    if (!audioEngine_->isInitialized()) {
        return false;
    }
    
    return true;
}

//==============================================================================
// Testing and Debugging

void VitalPlugin::enableTestMode(bool enabled) {
    testMode_ = enabled;
    
    if (enabled) {
        logMessage("Test mode enabled");
    } else {
        logMessage("Test mode disabled");
    }
}

bool VitalPlugin::isTestModeEnabled() const {
    return testMode_;
}

//==============================================================================
// Static Factory Methods

VitalPlugin* VitalPlugin::createVST3() {
    auto config = PluginConfig();
    config.enableVST3 = true;
    return new VitalPlugin(config);
}

VitalPlugin* VitalPlugin::createAU() {
    auto config = PluginConfig();
    config.enableAU = true;
    return new VitalPlugin(config);
}

VitalPlugin* VitalPlugin::createVST2() {
    auto config = PluginConfig();
    config.enableVST2 = true;
    return new VitalPlugin(config);
}

//==============================================================================
// Implementation Details

void VitalPlugin::processAudioBlock(juce::AudioBuffer<float>& buffer, const juce::MidiBuffer& midi) {
    if (!audioEngine_) return;
    
    // Get current time info
    updateTimeInfo();
    
    // Process parameters
    processParameters(buffer.getNumSamples());
    
    // Handle automation
    handleAutomation(buffer.getNumSamples());
    
    // Process MIDI
    audioEngine_->processMidiBuffer(midi);
    
    // Process audio through engine
    audioEngine_->processBlock(buffer, buffer, midi);
    
    // Apply master processing
    float masterGain = audioEngine_->getMasterGain();
    if (masterGain != 1.0f) {
        buffer.applyGain(masterGain);
    }
}

void VitalPlugin::processParameters(int numSamples) {
    // Update time-based parameters
    updateTimeInfo();
    
    // Apply parameter smoothing
    for (int i = 0; i < getNumParameters(); ++i) {
        float targetValue = getParameter(i);
        float currentValue = parameters_.getSmoothedValue(i);
        
        if (currentValue != targetValue) {
            float smoothing = parameterSmoothing_[i];
            float delta = (targetValue - currentValue) / std::max(1, numSamples) * smoothing;
            currentValue += delta;
            
            parameters_.setSmoothedValue(i, currentValue);
            audioEngine_->setParameter(i, currentValue);
        }
    }
}

void VitalPlugin::handleAutomation(int numSamples) {
    // Process scheduled automation points
    processScheduledAutomation(numSamples);
}

void VitalPlugin::updatePerformanceMetrics() {
    if (!audioEngine_) return;
    
    // Get engine metrics
    auto engineMetrics = audioEngine_->getRealTimeMetrics();
    
    // Update plugin state
    state_.cpuUsage = engineMetrics.cpuLoad;
    state_.memoryUsage = static_cast<size_t>(engineMetrics.memoryUsageMB * 1024 * 1024);
    state_.activeVoices = audioEngine_->getNumActiveVoices();
}

void VitalPlugin::detectPluginFormat() {
    auto hostType = getHostType();
    
    if (hostType.isAub宿主()) {
        state_.currentPluginFormat = "AU";
    } else if (hostType.isVST3Host()) {
        state_.currentPluginFormat = "VST3";
    } else if (hostType.isVSTHost()) {
        state_.currentPluginFormat = "VST2";
    } else {
        state_.currentPluginFormat = "Unknown";
    }
}

void VitalPlugin::updateDAWInfo() {
    auto hostType = getHostType();
    state_.dawName = hostType.getHostName();
    state_.dawVersion = hostType.getHostVersion();
    
    // Detect MPE support
    state_.supportsMPE = (hostType.isVST3Host() || hostType.isAub宿主()) && 
                        (isInLogic() || isInCubase() || isInReaper());
}

void VitalPlugin::updateTimeInfo() {
    // Update current time from DAW
    if (auto* playHead = getPlayHead()) {
        juce::AudioPlayHead::CurrentPositionInfo position;
        if (playHead->getCurrentPosition(position)) {
            state_.currentTime = position.timeInSeconds;
            state_.ppqPosition = position.ppqPosition;
            state_.tempo = position.bpm;
            state_.isPlaying = position.isPlaying;
            state_.isRecording = position.isRecording;
            state_.loopEnabled = position.isLooping;
            state_.bar = position.bar;
            state_.beat = position.beat;
            state_.tick = position.ticksPerBeat;
        }
    }
}

void VitalPlugin::processMPE(const juce::MidiMessage& message) {
    // Handle MPE (Multidimensional Polyphonic Expression) messages
    if (message.isPitchWheel()) {
        // Per-note pitch bend
        int channel = message.getChannel();
        float pitchBend = (message.getPitchWheelValue() - 8192) / 8192.0f * 2.0f;
        // Apply per-note pitch bend
    }
    
    // Handle per-note pressure (aftertouch)
    if (message.isAftertouch()) {
        // Per-note pressure control
        int channel = message.getChannel();
        float pressure = message.getAfterTouchValue() / 127.0f;
        // Apply per-note pressure
    }
}

void VitalPlugin::processPitchBend(const juce::MidiMessage& message) {
    int pitch = message.getPitchWheelValue();
    float pitchBend = (pitch - 8192) / 8192.0f * 48.0f; // ±48 semitone range
    audioEngine_->setParameter(0, pitchBend);
}

void VitalPlugin::processAftertouch(const juce::MidiMessage& message) {
    float pressure = message.getAfterTouchValue() / 127.0f;
    audioEngine_->setParameter(1, pressure);
}

void VitalPlugin::processCC(int cc, int value, int channel) {
    // Map CC messages to parameters
    switch (cc) {
        case 1: // Mod wheel
            audioEngine_->setParameter(1, value / 127.0f);
            break;
        case 7: // Volume
            audioEngine_->setParameter(7, value / 127.0f);
            break;
        case 74: // Brightness/Cutoff
            audioEngine_->setParameter(74, value / 127.0f);
            break;
        case 75: // Resonance
            audioEngine_->setParameter(75, value / 127.0f);
            break;
        default:
            // Handle other CC mappings
            break;
    }
}

void VitalPlugin::scheduleAutomation(int paramIndex, float value, double time) {
    if (paramIndex >= 0 && paramIndex < parameterAutomation_.size()) {
        AutomationPoint point;
        point.time = static_cast<float>(time);
        point.value = value;
        parameterAutomation_[paramIndex].push_back(point);
    }
}

void VitalPlugin::processScheduledAutomation(int numSamples) {
    double currentTime = getCurrentPosition() ? 
        getCurrentPosition()->getPosition().timeInSeconds : 0.0;
    
    for (auto& automationPoints : parameterAutomation_) {
        auto it = automationPoints.begin();
        while (it != automationPoints.end()) {
            if (it->time <= currentTime) {
                setParameter(automationPoints.begin() - it, it->value);
                it = automationPoints.erase(it);
            } else {
                ++it;
            }
        }
    }
}

//==============================================================================
// Private Implementation Methods

bool VitalPlugin::initializeEngine() {
    if (!audioEngine_) {
        auto engineConfig = audio_engine::VitalAudioEngine::Config();
        audioEngine_ = std::make_unique<audio_engine::VitalAudioEngine>(engineConfig);
    }
    
    if (!audioEngine_->initialize()) {
        setError("Failed to initialize audio engine", "initializeEngine");
        return false;
    }
    
    state_.isInitialized = true;
    return true;
}

bool VitalPlugin::initializeParameters() {
    parameters_.initialize();
    
    // Set parameter defaults
    for (int i = 0; i < getNumParameters(); ++i) {
        setParameter(i, getParameterDefaultValue(i));
    }
    
    return true;
}

bool VitalPlugin::initializeMidi() {
    midiHandler_.initialize();
    return true;
}

bool VitalPlugin::initializeUI() {
    updateDAWInfo();
    return true;
}

bool VitalPlugin::initializeFormats() {
    // Initialize plugin format specific features
    if (isVST3()) {
        // VST3 specific initialization
    } else if (isAU()) {
        // AudioUnit specific initialization
    }
    
    return true;
}

void VitalPlugin::shutdownEngine() {
    if (audioEngine_) {
        audioEngine_->shutdown();
    }
}

void VitalPlugin::shutdownParameters() {
    parameters_.shutdown();
}

void VitalPlugin::shutdownMidi() {
    midiHandler_.shutdown();
}

void VitalPlugin::shutdownUI() {
    if (editorUI_) {
        editorUI_.reset();
    }
}

void VitalPlugin::setError(const juce::String& message, const juce::String& component) {
    std::lock_guard<std::mutex> lock(errorMutex_);
    lastError_.hasError = true;
    lastError_.message = message;
    lastError_.component = component;
    lastError_.timestamp = std::chrono::steady_clock::now();
    
    logError("Error in " + component + ": " + message);
}

//==============================================================================
// Factory Namespace Implementation

namespace factory {

std::unique_ptr<VitalPlugin> createHighQualityPlugin() {
    auto config = VitalPlugin::PluginConfig();
    config.highQualityMode = true;
    config.enableOversampling = true;
    config.oversamplingFactor = 4;
    config.maxVoices = 64;
    config.enableSIMD = true;
    config.enableMultithreading = true;
    
    return std::make_unique<VitalPlugin>(config);
}

std::unique_ptr<VitalPlugin> createLowCPUPlugin() {
    auto config = VitalPlugin::PluginConfig();
    config.lowCPU = true;
    config.enableOversampling = false;
    config.maxVoices = 16;
    config.enableSIMD = false;
    config.enableMultithreading = false;
    
    return std::make_unique<VitalPlugin>(config);
}

std::unique_ptr<VitalPlugin> createMobilePlugin() {
    auto config = VitalPlugin::PluginConfig();
    config.mobile = true;
    config.enableOversampling = false;
    config.maxVoices = 8;
    config.enableCustomUI = false;
    config.numOutputs = 2;
    config.uiWidth = 800;
    config.uiHeight = 600;
    
    return std::make_unique<VitalPlugin>(config);
}

std::unique_ptr<VitalPlugin> createTestPlugin() {
    auto config = VitalPlugin::PluginConfig();
    config.testMode = true;
    config.enableDebugOutput = true;
    config.enableTestSignal = true;
    
    return std::make_unique<VitalPlugin>(config);
}

bool validateConfig(const VitalPlugin::PluginConfig& config) {
    // Validate configuration parameters
    if (config.maxVoices <= 0 || config.maxVoices > 256) {
        return false;
    }
    
    if (config.sampleRate < 8000 || config.sampleRate > 192000) {
        return false;
    }
    
    if (config.blockSize < 16 || config.blockSize > 4096) {
        return false;
    }
    
    if (config.oversamplingFactor != 1 && config.oversamplingFactor != 2 && 
        config.oversamplingFactor != 4 && config.oversamplingFactor != 8) {
        return false;
    }
    
    return true;
}

} // namespace factory

//==============================================================================
// VST3 and AU Specific Implementations
// These would be implemented in separate files for each format

// VST3 specific methods
 Steinberg::tresult PLUGIN_API VitalPlugin::initialize(FUnknown* hostContext) {
    Steinberg::tresult result = AudioProcessor::initialize(hostContext);
    if (result == Steinberg::kResultOk) {
        logMessage("VST3 initialized successfully");
    }
    return result;
}

Steinberg::tresult PLUGIN_API VitalPlugin::terminate() {
    logMessage("VST3 terminated");
    return AudioProcessor::terminate();
}

Steinberg::tresult PLUGIN_API VitalPlugin::setHost(Steinberg::FUnknown* host) {
    logMessage("VST3 host set");
    return AudioProcessor::setHost(host);
}

// AudioUnit specific methods
void VitalPlugin::beginSetParameter(AudioUnitParameterID paramID, AudioUnitScope scope, 
                                  AudioUnitElement element) {
    // AU parameter update start
}

void VitalPlugin::endSetParameter(AudioUnitParameterID paramID, AudioUnitScope scope, 
                                AudioUnitElement element) {
    // AU parameter update end
    if (paramID >= 0 && paramID < getNumParameters()) {
        float value = parameters_.getValue(paramID);
        audioEngine_->setParameter(paramID, value);
    }
}

} // namespace plugin
} // namespace vital