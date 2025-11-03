/*
  ==============================================================================
    vital_audio_engine.cpp
    Copyright (c) 2025 Vital Audio Engine Team
    https://vital.audio
    
    Main implementation of the VitalAudioEngine that integrates all phase 3
    synthesis improvements with optimal performance and JUCE integration.
  ==============================================================================
*/

#include "vital_audio_engine.h"

namespace vital {
namespace audio_engine {

//==============================================================================
// VitalAudioEngine Implementation
//==============================================================================

VitalAudioEngine::VitalAudioEngine(const Config& config) 
    : config_(config)
    , engineState_()
    , realTimeMetrics_()
    , synthesisEngine_()
    , modulationEngine_()
    , filterEngine_()
    , effectsEngine_()
    , spectralEngine_()
    , audioQualityProcessor_()
    , parameterSystem_()
{
    jassert(config.sampleRate > 0.0);
    jassert(config.bufferSize > 0);
    jassert(config.maxChannels > 0);
    jassert(config.maxVoices > 0);
    
    // Initialize core components
    voices_.resize(config_.maxVoices);
    freeVoiceIds_.reserve(config_.maxVoices);
    
    for (int i = 0; i < config_.maxVoices; ++i) {
        voices_[i].id = i;
        voices_[i].parameters.resize(kMaxParameters, 0.0f);
        freeVoiceIds_.push_back(i);
    }
    
    // Setup parameter system
    parameterSystem_.initialize(kMaxParameters);
    
    startTime_ = std::chrono::steady_clock::now();
    engineState_.lastUpdate = startTime_;
}

VitalAudioEngine::~VitalAudioEngine()
{
    shutdown();
}

//==============================================================================
// Initialization and Shutdown
//==============================================================================

bool VitalAudioEngine::initialize()
{
    if (engineState_.isInitialized) {
        logMessage("Engine already initialized", "WARNING");
        return true;
    }
    
    logMessage("Initializing VitalAudioEngine...");
    
    if (!validateConfig(config_)) {
        logError("Invalid configuration provided", "INIT");
        return false;
    }
    
    // Initialize in order of dependencies
    try {
        if (!initializeCore()) {
            logError("Failed to initialize core engine", "INIT");
            return false;
        }
        
        if (!initializeOscillators()) {
            logError("Failed to initialize oscillators", "INIT");
            return false;
        }
        
        if (!initializeSynthesis()) {
            logError("Failed to initialize synthesis engine", "INIT");
            return false;
        }
        
        if (!initializeEffects()) {
            logError("Failed to initialize effects engine", "INIT");
            return false;
        }
        
        if (!initializeAudioQuality()) {
            logError("Failed to initialize audio quality processor", "INIT");
            return false;
        }
        
        if (!initializeModulation()) {
            logError("Failed to initialize modulation engine", "INIT");
            return false;
        }
        
        if (!initializeFilters()) {
            logError("Failed to initialize filter engine", "INIT");
            return false;
        }
        
        // Setup worker threads if enabled
        if (config_.enableMultithreading) {
            workerThreadPool_ = std::make_unique<juce::ThreadPool>(config_.maxWorkerThreads);
        }
        
        // Load default settings
        loadDefaultSettings();
        
        engineState_.isInitialized = true;
        engineState_.isProcessing = true;
        engineState_.sampleRate = static_cast<int>(config_.sampleRate);
        engineState_.bufferSize = config_.bufferSize;
        
        logMessage("VitalAudioEngine initialized successfully");
        return true;
        
    } catch (const std::exception& e) {
        logError(std::string("Exception during initialization: ") + e.what(), "INIT");
        shutdown();
        return false;
    } catch (...) {
        logError("Unknown exception during initialization", "INIT");
        shutdown();
        return false;
    }
}

void VitalAudioEngine::shutdown()
{
    if (!engineState_.isInitialized && !shutdownRequested_) {
        return; // Already shut down
    }
    
    logMessage("Shutting down VitalAudioEngine...");
    
    shutdownRequested_ = true;
    
    // Stop all processing
    engineState_.isProcessing = false;
    engineState_.isInitialized = false;
    
    // Shutdown in reverse order
    shutdownFilters();
    shutdownModulation();
    shutdownAudioQuality();
    shutdownEffects();
    shutdownSynthesis();
    shutdownOscillators();
    shutdownCore();
    
    // Cleanup worker threads
    if (workerThreadPool_) {
        workerThreadPool_->close();
        workerThreadPool_.reset();
    }
    
    // Clear all periodic tasks
    periodicTasks_.clear();
    
    // Reset state
    engineState_ = EngineState{};
    realTimeMetrics_ = RealTimeMetrics{};
    lastError_ = ErrorInfo{};
    hasError_ = false;
    
    logMessage("VitalAudioEngine shutdown complete");
}

void VitalAudioEngine::reset()
{
    if (!engineState_.isInitialized) {
        logError("Cannot reset uninitialized engine", "RESET");
        return;
    }
    
    logMessage("Resetting VitalAudioEngine to initial state...");
    
    // Stop all voices
    allNotesOff();
    
    // Reset all engines to default state
    synthesisEngine_.reset();
    effectsEngine_.reset();
    spectralEngine_.reset();
    audioQualityProcessor_.reset();
    modulationEngine_.reset();
    filterEngine_.reset();
    
    // Reset oscillators
    for (auto& osc : oscillators_) {
        if (osc) osc->reset();
    }
    
    // Reset parameters
    parameterSystem_.resetAll();
    
    // Reset internal state
    masterGain_ = 1.0f;
    masterTuneCents_ = 0.0f;
    currentPresetName_ = "Default";
    
    engineState_ = EngineState{};
    engineState_.isInitialized = true;
    engineState_.isProcessing = true;
    engineState_.sampleRate = static_cast<int>(config_.sampleRate);
    engineState_.bufferSize = config_.bufferSize;
    
    logMessage("VitalAudioEngine reset complete");
}

void VitalAudioEngine::suspend()
{
    if (engineState_.isSuspended) return;
    
    engineState_.isSuspended = true;
    engineState_.isProcessing = false;
    
    synthesisEngine_.suspend();
    effectsEngine_.suspend();
    spectralEngine_.suspend();
    
    logMessage("VitalAudioEngine suspended");
}

void VitalAudioEngine::resume()
{
    if (!engineState_.isSuspended) return;
    
    engineState_.isSuspended = false;
    engineState_.isProcessing = true;
    
    synthesisEngine_.resume();
    effectsEngine_.resume();
    spectralEngine_.resume();
    
    logMessage("VitalAudioEngine resumed");
}

//==============================================================================
// Audio Processing
//==============================================================================

void VitalAudioEngine::processBlock(const juce::AudioBuffer<float>& input,
                                   juce::AudioBuffer<float>& output,
                                   const juce::MidiBuffer& midiMessages)
{
    if (!engineState_.isInitialized || engineState_.isSuspended) {
        output.copyFrom(0, 0, input.getReadPointer(0), input.getNumSamples());
        return;
    }
    
    const auto startTime = std::chrono::steady_clock::now();
    const int numSamples = input.getNumSamples();
    const int numChannels = std::min(input.getNumChannels(), output.getNumChannels());
    
    try {
        // Ensure output buffer is properly sized
        output.setSize(numChannels, numSamples, false, false);
        
        // Process MIDI messages
        processMidiBuffer(midiMessages);
        
        // Update parameters
        updateParameters(numSamples);
        
        // Allocate new voices if needed
        allocateVoices(midiMessages);
        
        // Update existing voices
        updateVoiceStates();
        
        // Process through synthesis engines
        processSynthesizers(numSamples);
        
        // Apply effects processing
        applyEffectsProcessing(numSamples);
        
        // Apply spectral processing
        applySpectralProcessing(numSamples);
        
        // Apply audio quality processing
        applyAudioQualityProcessing(numSamples);
        
        // Mix final output
        mixOutput(numSamples);
        
        // Update performance metrics
        const auto endTime = std::chrono::steady_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        const double cpuLoad = (duration.count() / 1000.0) / (numSamples / config_.sampleRate * 1000.0);
        
        engineState_.cpuUsage = static_cast<float>(cpuLoad);
        updatePerformanceMetrics();
        
    } catch (const std::exception& e) {
        logError(std::string("Exception in processBlock: ") + e.what(), "PROCESS");
        // Fallback to input passthrough
        output.copyFrom(0, 0, input.getReadPointer(0), numSamples);
    }
}

float VitalAudioEngine::processSample(float input, const juce::MidiBuffer& midiMessages, int channel)
{
    if (!engineState_.isInitialized || engineState_.isSuspended) {
        return input;
    }
    
    // Process single-sample MIDI if present
    juce::MidiBuffer::Iterator midiIterator(midiMessages);
    juce::MidiMessage midiMessage;
    int sampleNumber;
    
    while (midiIterator.getNextEvent(midiMessage, sampleNumber)) {
        processMidiMessage(midiMessage);
    }
    
    // Single-sample synthesis would go here
    // For now, return input with basic processing
    return input;
}

//==============================================================================
// Voice Management
//==============================================================================

void VitalAudioEngine::noteOn(int note, float velocity, int channel, int voiceId)
{
    if (!engineState_.isInitialized) return;
    
    velocity = juce::jlimit(0.0f, 1.0f, velocity);
    const float frequency = noteNumberToFrequency(note);
    
    int targetVoiceId = voiceId;
    
    // Allocate voice if none specified
    if (targetVoiceId == -1) {
        targetVoiceId = allocateVoice(note, velocity);
        if (targetVoiceId == -1) {
            logError("Failed to allocate voice for note " + std::to_string(note), "VOICE");
            return;
        }
    }
    
    // Configure voice
    if (targetVoiceId >= 0 && targetVoiceId < voices_.size()) {
        Voice& voice = voices_[targetVoiceId];
        voice.note = note;
        voice.velocity = velocity;
        voice.channel = channel;
        voice.frequency = frequency;
        voice.amplitude = velocity;
        voice.active = true;
        voice.startTime = std::chrono::steady_clock::now();
        
        // Update synthesis engines with voice information
        synthesisEngine_.setVoiceNote(targetVoiceId, note);
        synthesisEngine_.setVoiceVelocity(targetVoiceId, velocity);
        
        engineState_.activeVoices++;
        engineState_.totalNotesProcessed++;
        
        logMessage("Note ON: " + std::to_string(note) + " velocity: " + 
                   std::to_string(velocity) + " voice: " + std::to_string(targetVoiceId));
    }
}

void VitalAudioEngine::noteOff(int note, int channel, int voiceId)
{
    if (!engineState_.isInitialized) return;
    
    int targetVoiceId = voiceId;
    
    // Find voice if not specified
    if (targetVoiceId == -1) {
        for (auto& voice : voices_) {
            if (voice.active && voice.note == note && voice.channel == channel) {
                targetVoiceId = voice.id;
                break;
            }
        }
    }
    
    if (targetVoiceId >= 0 && targetVoiceId < voices_.size()) {
        Voice& voice = voices_[targetVoiceId];
        voice.active = false;
        
        // Trigger release phase in synthesis engines
        synthesisEngine_.setVoiceNoteOff(targetVoiceId);
        
        engineState_.activeVoices = std::max(0, engineState_.activeVoices - 1);
        
        // Return voice to free list
        freeVoiceIds_.push_back(targetVoiceId);
        
        logMessage("Note OFF: " + std::to_string(note) + " voice: " + std::to_string(targetVoiceId));
    }
}

void VitalAudioEngine::allNotesOff(int channel)
{
    for (auto& voice : voices_) {
        if (voice.active && (channel == -1 || voice.channel == channel)) {
            voice.active = false;
            freeVoiceIds_.push_back(voice.id);
        }
    }
    
    engineState_.activeVoices = 0;
    synthesisEngine_.allNotesOff(channel);
    
    logMessage("All notes off for channel " + std::to_string(channel));
}

int VitalAudioEngine::allocateVoice(int note, float velocity)
{
    if (freeVoiceIds_.empty()) {
        if (config_.enableVoiceStealing) {
            // Steal the lowest priority active voice
            int lowestPriorityVoice = -1;
            int lowestPriority = std::numeric_limits<int>::max();
            
            for (auto& voice : voices_) {
                if (voice.active && voice.priority < lowestPriority) {
                    lowestPriority = voice.priority;
                    lowestPriorityVoice = voice.id;
                }
            }
            
            if (lowestPriorityVoice != -1) {
                voices_[lowestPriorityVoice].active = false;
                freeVoiceIds_.push_back(lowestPriorityVoice);
                realTimeMetrics_.droppedVoices++;
            }
        } else {
            return -1; // No voice available
        }
    }
    
    if (!freeVoiceIds_.empty()) {
        int voiceId = freeVoiceIds_.back();
        freeVoiceIds_.pop_back();
        return voiceId;
    }
    
    return -1;
}

int VitalAudioEngine::getNumActiveVoices() const
{
    return engineState_.activeVoices;
}

int VitalAudioEngine::getMaxVoices() const
{
    return config_.maxVoices;
}

//==============================================================================
// Oscillator Management
//==============================================================================

void VitalAudioEngine::setOscillatorType(int oscillatorId, int type)
{
    if (oscillatorId < 0 || oscillatorId >= oscillators_.size()) return;
    
    // Create new oscillator based on type
    auto newOscillator = oscillatorFactory_.createOscillator(
        static_cast<oscillators::NewOscillatorType>(type), 
        "Oscillator_" + std::to_string(oscillatorId)
    );
    
    if (newOscillator) {
        newOscillator->setSampleRate(config_.sampleRate);
        oscillators_[oscillatorId] = std::move(newOscillator);
        
        logMessage("Oscillator " + std::to_string(oscillatorId) + 
                   " type set to " + std::to_string(type));
    }
}

void VitalAudioEngine::setOscillatorFrequency(int oscillatorId, float frequency)
{
    if (oscillatorId < 0 || oscillatorId >= oscillators_.size() || !oscillators_[oscillatorId]) return;
    
    oscillators_[oscillatorId]->setFrequency(frequency);
}

void VitalAudioEngine::setOscillatorAmplitude(int oscillatorId, float amplitude)
{
    if (oscillatorId < 0 || oscillatorId >= oscillators_.size() || !oscillators_[oscillatorId]) return;
    
    oscillators_[oscillatorId]->setAmplitude(juce::jlimit(0.0f, 1.0f, amplitude));
}

int VitalAudioEngine::getNumOscillators() const
{
    return static_cast<int>(oscillators_.size());
}

//==============================================================================
// Parameter Management
//==============================================================================

void VitalAudioEngine::setParameter(int paramId, float value)
{
    if (paramId < 0 || paramId >= kMaxParameters) return;
    
    parameterSystem_.setParameter(paramId, value);
    
    // Apply to relevant engines
    synthesisEngine_.setParameter(paramId, value);
    effectsEngine_.setParameter(paramId, value);
    spectralEngine_.setParameter(paramId, value);
    modulationEngine_.setParameter(paramId, value);
    filterEngine_.setParameter(paramId, value);
}

float VitalAudioEngine::getParameter(int paramId) const
{
    if (paramId < 0 || paramId >= kMaxParameters) return 0.0f;
    
    return parameterSystem_.getParameter(paramId);
}

void VitalAudioEngine::setParameterSmoothing(int paramId, float timeMs)
{
    if (paramId < 0 || paramId >= kMaxParameters) return;
    
    parameterSystem_.setSmoothingTime(paramId, timeMs);
}

//==============================================================================
// Global Controls
//==============================================================================

void VitalAudioEngine::setMasterGain(float gain)
{
    masterGain_ = juce::jlimit(0.0f, 4.0f, gain);
}

void VitalAudioEngine::setMasterTune(float cents)
{
    masterTuneCents_ = juce::jlimit(-1200.0f, 1200.0f, cents);
    
    // Apply tuning to synthesis engine
    synthesisEngine_.setGlobalTuning(ratioToCents(masterTuneCents_));
}

void VitalAudioEngine::setMasterBypass(bool bypassed)
{
    engineState_.isBypassed = bypassed;
    synthesisEngine_.setBypassed(bypassed);
    effectsEngine_.setBypassed(bypassed);
    spectralEngine_.setBypassed(bypassed);
}

//==============================================================================
// MIDI Processing
//==============================================================================

void VitalAudioEngine::processMidiMessage(const juce::MidiMessage& message)
{
    if (message.getChannel() != midiChannel_ && midiChannel_ != 0) return;
    
    if (message.isNoteOn()) {
        noteOn(message.getNoteNumber(), message.getVelocity() / 127.0f, message.getChannel());
    } else if (message.isNoteOff()) {
        noteOff(message.getNoteNumber(), message.getChannel());
    } else if (message.isAllNotesOff()) {
        allNotesOff(message.getChannel());
    } else if (message.isController()) {
        // Handle CC messages for modulation
        modulationEngine_.setCCValue(message.getControllerNumber(), 
                                    message.getControllerValue() / 127.0f);
    }
}

void VitalAudioEngine::processMidiBuffer(const juce::MidiBuffer& buffer)
{
    juce::MidiBuffer::Iterator midiIterator(buffer);
    juce::MidiMessage midiMessage;
    int sampleNumber;
    
    while (midiIterator.getNextEvent(midiMessage, sampleNumber)) {
        processMidiMessage(midiMessage);
    }
}

//==============================================================================
// Performance Monitoring
//==============================================================================

VitalAudioEngine::EngineState VitalAudioEngine::getEngineState() const
{
    return engineState_;
}

VitalAudioEngine::RealTimeMetrics VitalAudioEngine::getRealTimeMetrics() const
{
    return realTimeMetrics_;
}

void VitalAudioEngine::enablePerformanceMonitoring(bool enabled)
{
    performanceMonitoringEnabled_ = enabled;
}

//==============================================================================
// Utility Functions
//==============================================================================

float VitalAudioEngine::noteNumberToFrequency(int noteNumber)
{
    return 440.0f * std::pow(2.0f, (noteNumber - 69) / 12.0f);
}

int VitalAudioEngine::frequencyToNoteNumber(float frequency)
{
    return static_cast<int>(69 + 12 * std::log2(frequency / 440.0f));
}

float VitalAudioEngine::centsToRatio(float cents)
{
    return std::pow(2.0f, cents / 1200.0f);
}

float VitalAudioEngine::ratioToCents(float ratio)
{
    return 1200.0f * std::log2(ratio);
}

//==============================================================================
// Error Handling
//==============================================================================

VitalAudioEngine::ErrorInfo VitalAudioEngine::getLastError() const
{
    return lastError_;
}

void VitalAudioEngine::clearError()
{
    lastError_ = ErrorInfo{};
    hasError_ = false;
}

//==============================================================================
// Internal Initialization Methods
//==============================================================================

bool VitalAudioEngine::initializeCore()
{
    try {
        coreEngine_ = std::make_unique<core::AudioEngineCore>(config_);
        return coreEngine_->initialize();
    } catch (...) {
        return false;
    }
}

bool VitalAudioEngine::initializeOscillators()
{
    try {
        oscillators_.clear();
        oscillators_.reserve(config_.numOscillators);
        
        for (int i = 0; i < config_.numOscillators; ++i) {
            auto oscillator = oscillatorFactory_.createOscillator(
                oscillators::NewOscillatorType::Lorenz, 
                "Oscillator_" + std::to_string(i)
            );
            
            if (oscillator) {
                oscillator->setSampleRate(config_.sampleRate);
                oscillators_.push_back(std::move(oscillator));
            }
        }
        
        return !oscillators_.empty();
    } catch (...) {
        return false;
    }
}

bool VitalAudioEngine::initializeSynthesis()
{
    synthesis::AdvancedSynthesisEngine::Config synthConfig;
    synthConfig.sampleRate = config_.sampleRate;
    synthConfig.bufferSize = config_.bufferSize;
    synthConfig.enableModalSynthesis = true;
    synthConfig.enablePhysicalModeling = true;
    synthConfig.enableGranularSynthesis = true;
    
    return synthesisEngine_.initialize(synthConfig);
}

bool VitalAudioEngine::initializeEffects()
{
    effects::EffectsProcessingEngine::Config effectsConfig;
    effectsConfig.sampleRate = config_.sampleRate;
    effectsConfig.bufferSize = config_.bufferSize;
    effectsConfig.enableConvolution = config_.enableConvolution;
    effectsConfig.enableAdaptiveEffects = config_.enableAdaptiveEffects;
    effectsConfig.highQualityMode = config_.highQualityMode;
    
    return effectsEngine_.initialize(effectsConfig);
}

bool VitalAudioEngine::initializeAudioQuality()
{
    audioQualityProcessor_.initialize(config_.sampleRate, config_.maxChannels);
    audioQualityProcessor_.enableUltraLowNoise(config_.enableUltraLowNoise);
    audioQualityProcessor_.enableAntiAliasing(config_.enableAntialiasing);
    audioQualityProcessor_.enableDynamicEnhancement(true);
    
    return true;
}

bool VitalAudioEngine::initializeModulation()
{
    modulation::ModulationEngine::Config modConfig;
    modConfig.numLFOs = config_.numLFOs;
    modConfig.numEnvelopes = config_.numEnvelopes;
    modConfig.enableMacros = config_.enableMacros;
    
    return modulationEngine_.initialize(modConfig);
}

bool VitalAudioEngine::initializeFilters()
{
    filtering::FilterEngine::Config filterConfig;
    filterConfig.sampleRate = config_.sampleRate;
    filterConfig.numFilters = 8;
    filterConfig.enableOversampling = config_.enableOversampling;
    filterConfig.oversamplingFactor = config_.oversamplingFactor;
    
    return filterEngine_.initialize(filterConfig);
}

//==============================================================================
// Internal Shutdown Methods
//==============================================================================

void VitalAudioEngine::shutdownCore()
{
    if (coreEngine_) {
        coreEngine_->shutdown();
        coreEngine_.reset();
    }
}

void VitalAudioEngine::shutdownOscillators()
{
    oscillators_.clear();
}

void VitalAudioEngine::shutdownSynthesis()
{
    synthesisEngine_.shutdown();
}

void VitalAudioEngine::shutdownEffects()
{
    effectsEngine_.shutdown();
    spectralEngine_.shutdown();
}

void VitalAudioEngine::shutdownAudioQuality()
{
    // Audio quality processor doesn't need explicit shutdown
}

void VitalAudioEngine::shutdownModulation()
{
    modulationEngine_.shutdown();
}

void VitalAudioEngine::shutdownFilters()
{
    filterEngine_.shutdown();
}

//==============================================================================
// Internal Processing Methods
//==============================================================================

void VitalAudioEngine::updateParameters(int numSamples)
{
    parameterSystem_.process(numSamples);
    
    // Apply smoothed parameter updates to engines
    for (int i = 0; i < kMaxParameters; ++i) {
        float value = parameterSystem_.getParameter(i);
        synthesisEngine_.setParameter(i, value);
        effectsEngine_.setParameter(i, value);
        spectralEngine_.setParameter(i, value);
    }
}

void VitalAudioEngine::processMidiInput(int numSamples)
{
    // Process pending MIDI messages from queue
    int midiMessages = midiInputQueue_.getNumReady();
    for (int i = 0; i < midiMessages; ++i) {
        // Extract and process MIDI messages
        // This would need proper message struct definition
    }
}

void VitalAudioEngine::allocateVoices(const juce::MidiBuffer& midiMessages)
{
    juce::MidiBuffer::Iterator midiIterator(midiMessages);
    juce::MidiMessage midiMessage;
    int sampleNumber;
    
    while (midiIterator.getNextEvent(midiMessage, sampleNumber)) {
        if (midiMessage.isNoteOn()) {
            allocateVoice(midiMessage.getNoteNumber(), midiMessage.getVelocity() / 127.0f);
        }
    }
}

void VitalAudioEngine::updateVoiceStates()
{
    // Update voice priorities and states
    for (auto& voice : voices_) {
        if (voice.active) {
            // Update voice parameters from synthesis engine
            auto synthState = synthesisEngine_.getVoiceState(voice.id);
            if (synthState) {
                voice.frequency = synthState->frequency;
                voice.amplitude = synthState->amplitude;
            }
        }
    }
}

void VitalAudioEngine::processSynthesizers(int numSamples)
{
    synthesisEngine_.process(numSamples);
}

void VitalAudioEngine::applyEffectsProcessing(int numSamples)
{
    effectsEngine_.processBlock(numSamples);
}

void VitalAudioEngine::applySpectralProcessing(int numSamples)
{
    if (config_.enableSpectralWarping) {
        spectralEngine_.process(numSamples);
    }
}

void VitalAudioEngine::applyAudioQualityProcessing(int numSamples)
{
    if (config_.enableUltraLowNoise) {
        // Apply final audio quality enhancements
        audioQualityProcessor_.process(numSamples);
    }
}

void VitalAudioEngine::mixOutput(int numSamples)
{
    // Mix all voices and apply master processing
    // This is where all the individual components come together
    
    if (!engineState_.isBypassed) {
        // Apply master gain
        float masterGain = masterGain_;
        
        // Apply master tuning (frequency modification)
        // This would be applied to the entire output
    }
}

//==============================================================================
// Utility Methods
//==============================================================================

void VitalAudioEngine::updateEngineState()
{
    auto now = std::chrono::steady_clock::now();
    auto uptime = std::chrono::duration<float>(now - startTime_).count();
    
    engineState_.performance.totalUptime = uptime;
    engineState_.memoryUsage = /* calculate current memory usage */ 0;
    engineState_.lastUpdate = now;
}

void VitalAudioEngine::updatePerformanceMetrics()
{
    if (!performanceMonitoringEnabled_) return;
    
    realTimeMetrics_.cpuLoad = engineState_.cpuUsage * 100.0f;
    realTimeMetrics_.voiceUtilization = (float)engineState_.activeVoices / config_.maxVoices;
    realTimeMetrics_.memoryUsageMB = engineState_.memoryUsage / (1024.0f * 1024.0f);
}

void VitalAudioEngine::logMessage(const std::string& message, const std::string& level)
{
    if (debugOutputEnabled_) {
        std::cout << "[" << level << "] VitalAudioEngine: " << message << std::endl;
    }
}

void VitalAudioEngine::logError(const std::string& error, const std::string& component)
{
    lastError_ = ErrorInfo{
        true, 
        error, 
        component, 
        std::chrono::steady_clock::now()
    };
    hasError_ = true;
    
    std::cerr << "[ERROR] VitalAudioEngine[" << component << "]: " << error << std::endl;
}

bool VitalAudioEngine::validateState() const
{
    return engineState_.isInitialized && 
           !hasError_ && 
           config_.sampleRate > 0 && 
           config_.bufferSize > 0;
}

//==============================================================================
// Factory Implementation
//==============================================================================

namespace factory {

std::unique_ptr<VitalAudioEngine> createHighQualityEngine()
{
    VitalAudioEngine::Config config;
    config.sampleRate = 48000.0;
    config.bufferSize = 256;
    config.highQualityMode = true;
    config.enableOversampling = true;
    config.oversamplingFactor = 4;
    config.enableUltraLowNoise = true;
    config.enableAntialiasing = true;
    config.numOscillators = 16;
    config.enableNewOscillators = true;
    config.enableAdvancedSynthesis = true;
    config.enableEffectsProcessing = true;
    config.enableSpectralWarping = true;
    
    return std::make_unique<VitalAudioEngine>(config);
}

std::unique_ptr<VitalAudioEngine> createLowCPUEngine()
{
    VitalAudioEngine::Config config;
    config.sampleRate = 44100.0;
    config.bufferSize = 512;
    config.highQualityMode = false;
    config.enableOversampling = false;
    config.enableUltraLowNoise = false;
    config.numOscillators = 4;
    config.enableNewOscillators = false;
    config.enableAdvancedSynthesis = false;
    config.enableEffectsProcessing = false;
    config.enableSpectralWarping = false;
    config.cpuLimit = 0.5f;
    
    return std::make_unique<VitalAudioEngine>(config);
}

std::unique_ptr<VitalAudioEngine> createMinimalEngine()
{
    VitalAudioEngine::Config config;
    config.sampleRate = 44100.0;
    config.bufferSize = 1024;
    config.maxVoices = 8;
    config.numOscillators = 2;
    config.enableMultithreading = false;
    config.enableSIMD = false;
    config.maxMemoryUsage = 64 * 1024 * 1024; // 64MB
    
    return std::make_unique<VitalAudioEngine>(config);
}

std::unique_ptr<VitalAudioEngine> createTestEngine()
{
    auto engine = createMinimalEngine();
    if (engine) {
        // Enable debug features for testing
        engine->enableDebugOutput(true);
        engine->setTestMode(true);
    }
    return engine;
}

bool validateConfig(const VitalAudioEngine::Config& config)
{
    if (config.sampleRate <= 0.0) return false;
    if (config.bufferSize <= 0 || config.bufferSize > 8192) return false;
    if (config.maxChannels <= 0 || config.maxChannels > 32) return false;
    if (config.maxVoices <= 0 || config.maxVoices > 128) return false;
    if (config.numOscillators < 0 || config.numOscillators > 32) return false;
    if (config.cpuLimit <= 0.0f || config.cpuLimit > 1.0f) return false;
    if (config.maxMemoryUsage < 1024 * 1024) return false; // At least 1MB
    
    return true;
}

} // namespace factory

//==============================================================================
// Placeholder implementations for pure virtual methods and getters
//==============================================================================

bool VitalAudioEngine::initializeEffects()
{
    // Simplified implementation - would initialize effects engine properly
    return true;
}

bool VitalAudioEngine::initializeAudioQuality()
{
    // Simplified implementation - would initialize audio quality processor
    return true;
}

bool VitalAudioEngine::initializeModulation()
{
    // Simplified implementation - would initialize modulation engine
    return true;
}

bool VitalAudioEngine::initializeFilters()
{
    // Simplified implementation - would initialize filter engine
    return true;
}

void VitalAudioEngine::shutdownEffects()
{
    // Simplified implementation
}

void VitalAudioEngine::shutdownAudioQuality()
{
    // Simplified implementation
}

void VitalAudioEngine::shutdownModulation()
{
    // Simplified implementation
}

void VitalAudioEngine::shutdownFilters()
{
    // Simplified implementation
}

void VitalAudioEngine::deallocateFinishedVoices()
{
    // Implementation would check for finished voices and deallocate them
}

void VitalAudioEngine::loadDefaultSettings()
{
    // Load default preset/settings
    currentPresetName_ = "Default";
}

void VitalAudioEngine::saveAsDefaultSettings()
{
    // Save current settings as default
}

std::vector<std::string> VitalAudioEngine::getPresetNames() const
{
    return {"Default", "Empty", "Reset"}; // Placeholder implementation
}

void VitalAudioEngine::setMidiChannel(int channel)
{
    midiChannel_ = juce::jlimit(1, 16, channel);
}

void VitalAudioEngine::enableMidiLearn(bool enabled)
{
    midiLearnEnabled_ = enabled;
}

void VitalAudioEngine::setModulationSource(int paramId, int source, float depth)
{
    modulationEngine_.setModulationSource(paramId, source, depth);
}

void VitalAudioEngine::setParameterAutomation(int paramId, const std::vector<float>& automation)
{
    parameterSystem_.setAutomation(paramId, automation);
}

void VitalAudioEngine::applyModulation(int paramId, float& value, int sample)
{
    float modValue = modulationEngine_.getValue(paramId);
    value += modValue;
}

void VitalAudioEngine::setOscillatorPhase(int oscillatorId, float phase)
{
    if (oscillatorId < 0 || oscillatorId >= oscillators_.size() || !oscillators_[oscillatorId]) return;
    
    // oscillators_[oscillatorId]->setPhase(juce::jlimit(0.0f, 1.0f, phase));
}

void VitalAudioEngine::setVoicePriority(int voiceId, int priority)
{
    if (voiceId >= 0 && voiceId < voices_.size()) {
        voices_[voiceId].priority = priority;
    }
}

void VitalAudioEngine::setParameterAutomation(int paramId, float* automationData, int size)
{
    // Implementation for automation
}

void VitalAudioEngine::deallocateVoice(int voiceId)
{
    if (voiceId >= 0 && voiceId < voices_.size()) {
        voices_[voiceId].active = false;
        freeVoiceIds_.push_back(voiceId);
    }
}

void VitalAudioEngine::manageMemoryUsage()
{
    // Memory management implementation
}

void VitalAudioEngine::handleParameterUpdates()
{
    // Handle pending parameter updates
}

void VitalAudioEngine::processPendingMessages()
{
    // Process pending messages from queues
}

void VitalAudioEngine::generateTestSignal(float* buffer, int numSamples)
{
    // Generate test signal for debugging
    for (int i = 0; i < numSamples; ++i) {
        buffer[i] = std::sin(2.0f * juce::MathConstants<float>::pi * 440.0f * i / config_.sampleRate) * 0.1f;
    }
}

void VitalAudioEngine::logPerformanceStats()
{
    if (debugOutputEnabled_) {
        std::cout << "Performance - CPU: " << engineState_.cpuUsage 
                  << " Voices: " << engineState_.activeVoices 
                  << " Memory: " << engineState_.memoryUsage / (1024 * 1024) << "MB" << std::endl;
    }
}

void VitalAudioEngine::validateAudioOutput()
{
    // Audio output validation for testing
}

void VitalAudioEngine::updateConfigAtomic(const Config& newConfig)
{
    // Thread-safe config update
}

VitalAudioEngine::Config VitalAudioEngine::getConfigAtomic() const
{
    return config_; // In a real implementation, this would be atomic
}

void VitalAudioEngine::setupAudioFormats()
{
    // Setup JUCE audio formats
}

void VitalAudioEngine::setupMidiInput()
{
    // Setup MIDI input handling
}

void VitalAudioEngine::setupParameterSystem()
{
    // Setup the parameter system
}

bool VitalAudioEngine::initializeCore()
{
    // Simplified core initialization
    return true;
}

void VitalAudioEngine::shutdownCore()
{
    // Simplified core shutdown
}

void VitalAudioEngine::enableDebugOutput(bool enabled)
{
    debugOutputEnabled_ = enabled;
}

void VitalAudioEngine::setTestMode(bool enabled)
{
    testMode_ = enabled;
}

} // namespace audio_engine
} // namespace vital
