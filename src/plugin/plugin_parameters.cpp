/*
  ==============================================================================
    plugin_parameters.cpp
    Copyright (c) 2025 Vital Plugin Integration Team
    https://vital.audio
    
    Plugin parameter management implementation
    Provides parameter registration, automation, and MIDI learn functionality.
  ==============================================================================
*/

#include "plugin_parameters.h"
#include <cmath>
#include <algorithm>

namespace vital {
namespace plugin {

//==============================================================================
// Parameter Implementation

Parameter::Parameter(int id, const juce::String& name, Type type, const Range& range)
    : id_(id), name_(name), type_(type), range_(range) {
    setRange(range);
    setValue(range.defaultValue);
}

void Parameter::setRange(const Range& range) {
    range_ = range;
    value_.store(clampValue(value_.load()));
}

void Parameter::setDisplay(const Display& display) {
    display_ = display;
    // Update display text for current value
    auto newDisplayText = getDisplayText();
    // Trigger any display change listeners
}

float Parameter::getValue() const {
    return value_.load();
}

void Parameter::setValue(float value) {
    float clampedValue = clampValue(value);
    float oldValue = value_.exchange(clampedValue);
    
    if (oldValue != clampedValue) {
        smoothedValue_.store(clampedValue);
        // Trigger parameter change listeners
    }
}

float Parameter::getNormalizedValue() const {
    return valueToNormalized(value_.load());
}

void Parameter::setNormalizedValue(float normalizedValue) {
    float value = normalizeToValue(normalizedValue);
    setValue(value);
}

float Parameter::normalizeToValue(float normalized) const {
    normalized = juce::jlimit(0.0f, 1.0f, normalized);
    return range_.logarithmic ? normalizedToValueLog(normalized) : normalizedToValueLinear(normalized);
}

float Parameter::valueToNormalized(float value) const {
    float clampedValue = clampValue(value);
    return range_.logarithmic ? valueToNormalizedLog(clampedValue) : valueToNormalizedLinear(clampedValue);
}

float Parameter::clampValue(float value) const {
    return juce::jlimit(range_.min, range_.max, value);
}

juce::String Parameter::getDisplayText() const {
    return getDisplayText(value_.load());
}

juce::String Parameter::getDisplayText(float value) const {
    juce::String text;
    
    switch (type_) {
        case Float:
        case Percent: {
            int decimals = display_.decimals;
            text = juce::String(value, decimals);
            break;
        }
        case Int:
        case Note: {
            text = juce::String(static_cast<int>(std::round(value)));
            break;
        }
        case Decibels: {
            if (value <= 0.0f) {
                text = "-∞";
            } else {
                text = juce::String(20.0f * std::log10(value), 1) + " dB";
            }
            break;
        }
        case Frequency: {
            if (value < 1000.0f) {
                text = juce::String(value, 1) + " Hz";
            } else {
                text = juce::String(value / 1000.0f, 2) + " kHz";
            }
            break;
        }
        case Cents: {
            text = juce::String(value, 1) + " cents";
            break;
        }
        case Choice: {
            int index = static_cast<int>(std::round(value));
            if (index >= 0 && index < display_.choices.size()) {
                text = display_.choices[index];
            }
            break;
        }
        case Bool: {
            text = value >= 0.5f ? "On" : "Off";
            break;
        }
        default:
            text = juce::String(value, 2);
    }
    
    if (display_.suffix.isNotEmpty() && type_ != Decibels && type_ != Frequency) {
        text += " " + display_.suffix;
    }
    
    return text;
}

float Parameter::parseDisplayText(const juce::String& text) const {
    auto trimmedText = text.trim();
    
    switch (type_) {
        case Decibels: {
            if (trimmedText == "-∞" || trimmedText == "-inf") {
                return 0.0f;
            }
            
            // Remove "dB" suffix if present
            if (trimmedText.endsWithIgnoreCase("dB")) {
                trimmedText = trimmedText.dropLastCharacters(2).trim();
            }
            
            float dbValue = trimmedText.getFloatValue();
            return std::pow(10.0f, dbValue / 20.0f);
        }
        case Frequency: {
            juce::String numericPart = trimmedText;
            bool isKHz = false;
            
            if (trimmedText.endsWithIgnoreCase("kHz")) {
                numericPart = trimmedText.dropLastCharacters(3).trim();
                isKHz = true;
            } else if (trimmedText.endsWithIgnoreCase("Hz")) {
                numericPart = trimmedText.dropLastCharacters(2).trim();
            }
            
            float hzValue = numericPart.getFloatValue();
            return isKHz ? hzValue * 1000.0f : hzValue;
        }
        case Cents: {
            if (trimmedText.endsWithIgnoreCase("cents")) {
                trimmedText = trimmedText.dropLastCharacters(5).trim();
            }
            return trimmedText.getFloatValue();
        }
        case Choice: {
            int index = display_.choices.indexOf(trimmedText);
            return index >= 0 ? static_cast<float>(index) : 0.0f;
        }
        case Bool: {
            return trimmedText.equalsIgnoreCase("on") ? 1.0f : 0.0f;
        }
        default: {
            float parsedValue = trimmedText.getFloatValue();
            // Remove any suffix for numeric parameters
            for (const auto& suffix : display_.suffix) {
                if (trimmedText.endsWithIgnoreCase(suffix)) {
                    parsedValue = trimmedText.dropLastCharacters(suffix.length()).trim().getFloatValue();
                    break;
                }
            }
            return parsedValue;
        }
    }
}

void Parameter::setAutomation(const Automation& automation) {
    automation_ = automation;
}

void Parameter::setMidiMapping(const MidiMapping& mapping) {
    midiMapping_ = mapping;
}

int Parameter::getNumSteps() const {
    if (type_ == Bool) return 2;
    if (type_ == Choice) return display_.choices.size();
    
    float range = range_.max - range_.min;
    int steps = static_cast<int>(range / range_.step + 0.5f);
    
    return juce::jmax(1, steps);
}

float Parameter::getStepSize() const {
    if (type_ == Bool) return 1.0f;
    if (type_ == Choice) return 1.0f;
    
    return range_.step;
}

juce::ValueTree Parameter::createState() const {
    auto state = juce::ValueTree("Parameter");
    state.setProperty("id", id_, nullptr);
    state.setProperty("name", name_, nullptr);
    state.setProperty("type", static_cast<int>(type_), nullptr);
    state.setProperty("value", getValue(), nullptr);
    state.setProperty("range_min", range_.min, nullptr);
    state.setProperty("range_max", range_.max, nullptr);
    state.setProperty("range_default", range_.defaultValue, nullptr);
    state.setProperty("range_step", range_.step, nullptr);
    state.setProperty("range_logarithmic", range_.logarithmic, nullptr);
    
    // Display properties
    state.setProperty("suffix", display_.suffix, nullptr);
    state.setProperty("decimals", display_.decimals, nullptr);
    state.setProperty("show_value", display_.showValue, nullptr);
    state.setProperty("show_name", display_.showName, nullptr);
    
    // MIDI mapping
    state.setProperty("midi_cc", midiMapping_.cc, nullptr);
    state.setProperty("midi_channel", midiMapping_.channel, nullptr);
    state.setProperty("midi_enabled", midiMapping_.enabled, nullptr);
    state.setProperty("midi_bipolar", midiMapping_.isBipolar, nullptr);
    state.setProperty("midi_min", midiMapping_.minValue, nullptr);
    state.setProperty("midi_max", midiMapping_.maxValue, nullptr);
    
    return state;
}

void Parameter::restoreState(const juce::ValueTree& state) {
    if (!state.hasProperty("id") || state.getProperty("id") != id_) {
        return;
    }
    
    Range range = range_;
    range.min = state.getProperty("range_min", range.min);
    range.max = state.getProperty("range_max", range.max);
    range.defaultValue = state.getProperty("range_default", range.defaultValue);
    range.step = state.getProperty("range_step", range.step);
    range.logarithmic = state.getProperty("range_logarithmic", range.logarithmic);
    setRange(range);
    
    Display display = display_;
    display.suffix = state.getProperty("suffix", display.suffix);
    display.decimals = state.getProperty("decimals", display.decimals);
    display.showValue = state.getProperty("show_value", display.showValue);
    display.showName = state.getProperty("show_name", display.showName);
    setDisplay(display);
    
    MidiMapping mapping = midiMapping_;
    mapping.cc = state.getProperty("midi_cc", mapping.cc);
    mapping.channel = state.getProperty("midi_channel", mapping.channel);
    mapping.enabled = state.getProperty("midi_enabled", mapping.enabled);
    mapping.isBipolar = state.getProperty("midi_bipolar", mapping.isBipolar);
    mapping.minValue = state.getProperty("midi_min", mapping.minValue);
    mapping.maxValue = state.getProperty("midi_max", mapping.maxValue);
    setMidiMapping(mapping);
    
    float value = state.getProperty("value", getValue());
    setValue(value);
}

float Parameter::valueToNormalizedLinear(float value) const {
    return (value - range_.min) / (range_.max - range_.min);
}

float Parameter::normalizedToValueLinear(float normalized) const {
    return range_.min + normalized * (range_.max - range_.min);
}

float Parameter::valueToNormalizedLog(float value) const {
    if (value <= 0.0f) return 0.0f;
    if (range_.min <= 0.0f) range_.min = 0.001f; // Prevent log(0)
    
    float logValue = std::log(value);
    float logMin = std::log(range_.min);
    float logMax = std::log(range_.max);
    
    return (logValue - logMin) / (logMax - logMin);
}

float Parameter::normalizedToValueLog(float normalized) const {
    if (normalized <= 0.0f) return range_.min;
    if (range_.min <= 0.0f) range_.min = 0.001f; // Prevent log(0)
    
    float logMin = std::log(range_.min);
    float logMax = std::log(range_.max);
    float logValue = logMin + normalized * (logMax - logMin);
    
    return std::exp(logValue);
}

//==============================================================================
// ParameterGroup Implementation

ParameterGroup::ParameterGroup(const juce::String& name, const juce::String& label)
    : name_(name), label_(label.isEmpty() ? name : label) {
}

void ParameterGroup::addParameter(std::shared_ptr<Parameter> parameter) {
    if (!parameter) return;
    
    parameters_.push_back(parameter);
    parametersById_[parameter->getId()] = parameter;
    parametersByName_[parameter->getName()] = parameter;
}

void ParameterGroup::removeParameter(int paramId) {
    auto it = parametersById_.find(paramId);
    if (it != parametersById_.end()) {
        auto parameter = it->second;
        
        // Remove from vector
        parameters_.erase(std::remove_if(parameters_.begin(), parameters_.end(),
            [parameter](const auto& p) { return p == parameter; }), parameters_.end());
        
        // Remove from maps
        parametersById_.erase(it);
        parametersByName_.erase(parameter->getName());
    }
}

std::shared_ptr<Parameter> ParameterGroup::getParameter(int paramId) const {
    auto it = parametersById_.find(paramId);
    return it != parametersById_.end() ? it->second : nullptr;
}

std::shared_ptr<Parameter> ParameterGroup::getParameterByName(const juce::String& name) const {
    auto it = parametersByName_.find(name);
    return it != parametersByName_.end() ? it->second : nullptr;
}

juce::ValueTree ParameterGroup::createState() const {
    auto state = juce::ValueTree("ParameterGroup");
    state.setProperty("name", name_, nullptr);
    state.setProperty("label", label_, nullptr);
    state.setProperty("description", description_, nullptr);
    state.setProperty("collapsed", collapsed_, nullptr);
    state.setProperty("visible", visible_, nullptr);
    
    auto paramsState = state.createNewChildElement("Parameters");
    for (const auto& parameter : parameters_) {
        paramsState->addChildElement(parameter->createState().createCopy());
    }
    
    return state;
}

void ParameterGroup::restoreState(const juce::ValueTree& state) {
    if (state.hasProperty("name") && state.getProperty("name") == name_) {
        label_ = state.getProperty("label", label_);
        description_ = state.getProperty("description", description_);
        collapsed_ = state.getProperty("collapsed", collapsed_);
        visible_ = state.getProperty("visible", visible_);
        
        auto* paramsState = state.getChildByName("Parameters");
        if (paramsState) {
            for (auto* paramState : paramsState->getChildIterator()) {
                int paramId = paramState->getProperty("id", -1);
                auto parameter = getParameter(paramId);
                if (parameter) {
                    parameter->restoreState(*paramState);
                }
            }
        }
    }
}

//==============================================================================
// PluginParameters Implementation

PluginParameters::PluginParameters()
    : plugin_(nullptr) {
}

PluginParameters::PluginParameters(juce::AudioProcessor* plugin)
    : plugin_(plugin) {
}

void PluginParameters::initialize() {
    createDefaultParameters();
}

void PluginParameters::shutdown() {
    clearPendingUpdates();
    removeAllParameters();
}

void PluginParameters::setPlugin(juce::AudioProcessor* plugin) {
    plugin_ = plugin;
}

void PluginParameters::addParameter(std::shared_ptr<Parameter> parameter) {
    if (!parameter) return;
    
    std::lock_guard<std::mutex> lock(parametersMutex_);
    
    int paramId = parameter->getId();
    if (parametersById_.find(paramId) != parametersById_.end()) {
        // Parameter ID already exists, generate new one
        paramId = getNumParameters();
        parameter = std::make_shared<Parameter>(paramId, parameter->getName(), 
                                              parameter->getType(), parameter->getRange());
    }
    
    parameters_.push_back(parameter);
    parametersById_[paramId] = parameter;
    parametersByName_[parameter->getName()] = parameter;
    
    // Initialize smoothed value
    float value = parameter->getValue();
    parameter->setValue(value); // This will set both value_ and smoothedValue_
    
    sendChangeMessage();
}

void PluginParameters::addParameterGroup(std::shared_ptr<ParameterGroup> group) {
    if (!group) return;
    
    std::lock_guard<std::mutex> lock(parametersMutex_);
    groups_.push_back(group);
    
    // Add all parameters from the group
    for (const auto& parameter : group->getParameters()) {
        addParameter(parameter);
    }
}

void PluginParameters::removeParameter(int paramId) {
    std::lock_guard<std::mutex> lock(parametersMutex_);
    
    auto it = parametersById_.find(paramId);
    if (it != parametersById_.end()) {
        auto parameter = it->second;
        
        // Remove from main list
        parameters_.erase(std::remove_if(parameters_.begin(), parameters_.end(),
            [parameter](const auto& p) { return p == parameter; }), parameters_.end());
        
        // Remove from maps
        parametersById_.erase(it);
        parametersByName_.erase(parameter->getName());
        
        // Remove from groups
        for (auto& group : groups_) {
            group->removeParameter(paramId);
        }
        
        sendChangeMessage();
    }
}

void PluginParameters::removeAllParameters() {
    std::lock_guard<std::mutex> lock(parametersMutex_);
    
    parameters_.clear();
    parametersById_.clear();
    parametersByName_.clear();
    groups_.clear();
    parametersByCategory_.clear();
    
    clearPendingUpdates();
}

std::shared_ptr<Parameter> PluginParameters::getParameter(int paramId) const {
    std::lock_guard<std::mutex> lock(parametersMutex_);
    
    auto it = parametersById_.find(paramId);
    return it != parametersById_.end() ? it->second : nullptr;
}

std::shared_ptr<Parameter> PluginParameters::getParameterByName(const juce::String& name) const {
    std::lock_guard<std::mutex> lock(parametersMutex_);
    
    auto it = parametersByName_.find(name);
    return it != parametersByName_.end() ? it->second : nullptr;
}

float PluginParameters::getValue(int paramId) const {
    auto parameter = getParameter(paramId);
    return parameter ? parameter->getValue() : 0.0f;
}

float PluginParameters::getNormalizedValue(int paramId) const {
    auto parameter = getParameter(paramId);
    return parameter ? parameter->getNormalizedValue() : 0.0f;
}

void PluginParameters::setValue(int paramId, float value) {
    auto parameter = getParameter(paramId);
    if (parameter) {
        float oldValue = parameter->getValue();
        parameter->setValue(value);
        
        // Record update for real-time processing
        {
            std::lock_guard<std::mutex> lock(updateMutex_);
            ParameterUpdate update;
            update.paramId = paramId;
            update.value = value;
            update.timestamp = juce::Time::getCurrentTime().toDouble();
            pendingUpdates_.push_back(update);
        }
        
        if (oldValue != value) {
            notifyListeners(paramId);
        }
    }
}

void PluginParameters::setNormalizedValue(int paramId, float normalizedValue) {
    auto parameter = getParameter(paramId);
    if (parameter) {
        float value = parameter->normalizeToValue(normalizedValue);
        setValue(paramId, value);
    }
}

float PluginParameters::getSmoothedValue(int paramId) const {
    auto parameter = getParameter(paramId);
    return parameter ? parameter->smoothedValue_.load() : 0.0f;
}

void PluginParameters::setSmoothedValue(int paramId, float value) {
    auto parameter = getParameter(paramId);
    if (parameter) {
        parameter->smoothedValue_.store(value);
    }
}

void PluginParameters::updateSmoothedValues(int numSamples) {
    if (!smoothingEnabled_) return;
    
    std::lock_guard<std::mutex> lock(parametersMutex_);
    
    for (const auto& parameter : parameters_) {
        float currentValue = parameter->getValue();
        float smoothedValue = parameter->smoothedValue_.load();
        
        if (currentValue != smoothedValue) {
            float smoothingCoeff = 1.0f - std::pow(0.001f, static_cast<float>(numSamples) / 1000.0f);
            smoothedValue = smoothedValue + smoothingCoeff * (currentValue - smoothedValue);
            parameter->smoothedValue_.store(smoothedValue);
        }
    }
}

juce::String PluginParameters::getName(int paramId) const {
    auto parameter = getParameter(paramId);
    return parameter ? parameter->getName() : juce::String();
}

juce::String PluginParameters::getText(int paramId) const {
    auto parameter = getParameter(paramId);
    return parameter ? parameter->getDisplayText() : juce::String();
}

bool PluginParameters::isAutomatable(int paramId) const {
    auto parameter = getParameter(paramId);
    return parameter && parameter->isAutomatable();
}

bool PluginParameters::isMetaParameter(int paramId) const {
    auto parameter = getParameter(paramId);
    return parameter && parameter->isMetaParameter();
}

void PluginParameters::setDefaultValue(int paramId, float value) {
    auto parameter = getParameter(paramId);
    if (parameter) {
        auto range = parameter->getRange();
        range.defaultValue = value;
        parameter->setRange(range);
    }
}

float PluginParameters::getDefaultValue(int paramId) const {
    auto parameter = getParameter(paramId);
    return parameter ? parameter->getRange().defaultValue : 0.0f;
}

int PluginParameters::getParameterNumSteps(int paramId) const {
    auto parameter = getParameter(paramId);
    return parameter ? parameter->getNumSteps() : 1;
}

void PluginParameters::enableMidiLearn(bool enabled) {
    midiLearnEnabled_ = enabled;
}

bool PluginParameters::learnParameter(int paramId, int cc, int channel) {
    auto parameter = getParameter(paramId);
    if (!parameter || !midiLearnEnabled_) return false;
    
    Parameter::MidiMapping mapping;
    mapping.cc = cc;
    mapping.channel = channel;
    mapping.enabled = true;
    mapping.minValue = parameter->getRange().min;
    mapping.maxValue = parameter->getRange().max;
    
    parameter->setMidiMapping(mapping);
    return true;
}

void PluginParameters::unlearnParameter(int paramId) {
    auto parameter = getParameter(paramId);
    if (parameter) {
        Parameter::MidiMapping mapping;
        parameter->setMidiMapping(mapping);
    }
}

bool PluginParameters::isParameterMapped(int paramId) const {
    auto parameter = getParameter(paramId);
    return parameter && parameter->hasMidiMapping();
}

void PluginParameters::setAllParametersToDefault() {
    std::lock_guard<std::mutex> lock(parametersMutex_);
    
    for (const auto& parameter : parameters_) {
        float defaultValue = parameter->getRange().defaultValue;
        parameter->setValue(defaultValue);
    }
}

void PluginParameters::resetAllParameters() {
    setAllParametersToDefault();
}

void PluginParameters::randomizeParameters() {
    std::lock_guard<std::mutex> lock(parametersMutex_);
    
    juce::Random random;
    for (const auto& parameter : parameters_) {
        auto range = parameter->getRange();
        float randomValue = range.min + random.nextFloat() * (range.max - range.min);
        parameter->setValue(randomValue);
    }
}

void PluginParameters::setParameterRange(int paramId, float min, float max) {
    auto parameter = getParameter(paramId);
    if (parameter) {
        auto range = parameter->getRange();
        range.min = min;
        range.max = max;
        parameter->setRange(range);
    }
}

void PluginParameters::getParameterState(juce::XmlElement& xml) const {
    xml.setTagName("Parameters");
    xml.setAttribute("count", getNumParameters());
    
    std::lock_guard<std::mutex> lock(parametersMutex_);
    
    for (const auto& parameter : parameters_) {
        auto* paramXml = xml.createNewChildElement("Parameter");
        paramXml->setAttribute("id", parameter->getId());
        paramXml->setAttribute("name", parameter->getName());
        paramXml->setAttribute("value", parameter->getValue());
    }
}

void PluginParameters::setParameterState(const juce::XmlElement& xml) {
    if (!xml.hasTagName("Parameters")) return;
    
    for (auto* paramXml : xml.getChildIterator()) {
        if (paramXml->hasTagName("Parameter")) {
            int paramId = paramXml->getIntAttribute("id", -1);
            float value = paramXml->getDoubleAttribute("value", 0.0);
            
            setValue(paramId, value);
        }
    }
}

void PluginParameters::enableSmoothing(bool enabled) {
    smoothingEnabled_ = enabled;
}

void PluginParameters::enableSmartUpdate(bool enabled) {
    smartUpdateEnabled_ = enabled;
}

std::vector<PluginParameters::ParameterUpdate> PluginParameters::getPendingUpdates() {
    std::lock_guard<std::mutex> lock(updateMutex_);
    auto updates = pendingUpdates_;
    clearPendingUpdates();
    return updates;
}

void PluginParameters::clearPendingUpdates() {
    std::lock_guard<std::mutex> lock(updateMutex_);
    pendingUpdates_.clear();
}

void PluginParameters::addParameterToCategory(int paramId, Category category) {
    parametersByCategory_[category].push_back(paramId);
}

PluginParameters::Category PluginParameters::getParameterCategory(int paramId) const {
    for (const auto& [category, paramIds] : parametersByCategory_) {
        if (std::find(paramIds.begin(), paramIds.end(), paramId) != paramIds.end()) {
            return category;
        }
    }
    return Utility;
}

std::vector<std::shared_ptr<Parameter>> PluginParameters::getParametersByCategory(Category category) const {
    std::vector<std::shared_ptr<Parameter>> results;
    
    auto it = parametersByCategory_.find(category);
    if (it != parametersByCategory_.end()) {
        for (int paramId : it->second) {
            auto parameter = getParameter(paramId);
            if (parameter) {
                results.push_back(parameter);
            }
        }
    }
    
    return results;
}

int PluginParameters::findParameterByName(const juce::String& name) const {
    auto parameter = getParameterByName(name);
    return parameter ? parameter->getId() : -1;
}

void PluginParameters::validateParameters() {
    std::lock_guard<std::mutex> lock(parametersMutex_);
    
    for (const auto& parameter : parameters_) {
        if (!validateParameterRange(*parameter) ||
            !validateParameterType(*parameter) ||
            !validateParameterName(*parameter)) {
            // Handle invalid parameter
            removeParameter(parameter->getId());
        }
    }
}

bool PluginParameters::validateParameter(int paramId) const {
    auto parameter = getParameter(paramId);
    return parameter && validateParameterRange(*parameter) &&
           validateParameterType(*parameter) && validateParameterName(*parameter);
}

void PluginParameters::notifyListeners(int paramId) {
    sendChangeMessage();
}

void PluginParameters::createDefaultParameters() {
    createMasterParameters();
    createOscillatorParameters();
    createFilterParameters();
    createEnvelopeParameters();
    createLFOParameters();
    createEffectParameters();
    createModulationParameters();
}

void PluginParameters::createMasterParameters() {
    // Master Volume
    auto masterVolume = std::make_shared<Parameter>(0, "Master Volume", Parameter::Float);
    masterVolume->setRange({0.0f, 1.0f, 0.8f, 0.01f});
    masterVolume->setDisplay({"%", 0, true, true, false});
    addParameter(masterVolume);
    addParameterToCategory(0, Category::Master);
    
    // Master Tune
    auto masterTune = std::make_shared<Parameter>(1, "Master Tune", Parameter::Cents);
    masterTune->setRange({-1200.0f, 1200.0f, 0.0f, 1.0f});
    masterTune->setDisplay({"cents", 0, true, true, false});
    addParameter(masterTune);
    addParameterToCategory(1, Category::Master);
    
    // Master Bypass
    auto masterBypass = std::make_shared<Parameter>(2, "Bypass", Parameter::Bool);
    masterBypass->setDisplay({"", 0, true, true, false});
    addParameter(masterBypass);
    addParameterToCategory(2, Category::Master);
}

void PluginParameters::createOscillatorParameters() {
    // Oscillator 1 Type
    auto osc1Type = std::make_shared<Parameter>(10, "Osc 1 Type", Parameter::Choice);
    osc1Type->setDisplay({"", 0, true, true, false, 0, "", false});
    juce::StringArray oscTypes = {"Sine", "Square", "Saw", "Triangle", "Noise", "Custom"};
    // osc1Type->getDisplay().choices = oscTypes; // Would need public accessor
    addParameter(osc1Type);
    addParameterToCategory(10, Category::Oscillator);
    
    // Oscillator 1 Frequency
    auto osc1Freq = std::make_shared<Parameter>(11, "Osc 1 Frequency", Parameter::Frequency);
    osc1Freq->setRange({20.0f, 20000.0f, 440.0f, 0.01f, true});
    osc1Freq->setDisplay({"Hz", 1, true, true, false});
    addParameter(osc1Freq);
    addParameterToCategory(11, Category::Oscillator);
    
    // Oscillator 1 Level
    auto osc1Level = std::make_shared<Parameter>(12, "Osc 1 Level", Parameter::Float);
    osc1Level->setRange({0.0f, 1.0f, 0.7f, 0.01f});
    osc1Level->setDisplay({"%", 0, true, true, false});
    addParameter(osc1Level);
    addParameterToCategory(12, Category::Oscillator);
}

void PluginParameters::createFilterParameters() {
    // Filter Type
    auto filterType = std::make_shared<Parameter>(20, "Filter Type", Parameter::Choice);
    juce::StringArray filterTypes = {"Low Pass", "High Pass", "Band Pass", "Notch", "Peaking"};
    // filterType->getDisplay().choices = filterTypes;
    addParameter(filterType);
    addParameterToCategory(20, Category::Filter);
    
    // Filter Cutoff
    auto filterCutoff = std::make_shared<Parameter>(21, "Filter Cutoff", Parameter::Frequency);
    filterCutoff->setRange({20.0f, 20000.0f, 1000.0f, 0.01f, true});
    filterCutoff->setDisplay({"Hz", 1, true, true, false});
    addParameter(filterCutoff);
    addParameterToCategory(21, Category::Filter);
    
    // Filter Resonance
    auto filterResonance = std::make_shared<Parameter>(22, "Filter Resonance", Parameter::Float);
    filterResonance->setRange({0.0f, 1.0f, 0.1f, 0.01f});
    filterResonance->setDisplay({"%", 0, true, true, false});
    addParameter(filterResonance);
    addParameterToCategory(22, Category::Filter);
}

void PluginParameters::createEnvelopeParameters() {
    // ADSR Envelope
    auto envAttack = std::make_shared<Parameter>(30, "Env Attack", Parameter::Time);
    envAttack->setRange({0.001f, 10.0f, 0.01f, 0.001f, false});
    envAttack->setDisplay({"s", 3, true, true, false});
    addParameter(envAttack);
    addParameterToCategory(30, Category::Envelope);
    
    auto envDecay = std::make_shared<Parameter>(31, "Env Decay", Parameter::Time);
    envDecay->setRange({0.001f, 10.0f, 0.3f, 0.001f, false});
    envDecay->setDisplay({"s", 3, true, true, false});
    addParameter(envDecay);
    addParameterToCategory(31, Category::Envelope);
    
    auto envSustain = std::make_shared<Parameter>(32, "Env Sustain", Parameter::Float);
    envSustain->setRange({0.0f, 1.0f, 0.7f, 0.01f});
    envSustain->setDisplay({"%", 0, true, true, false});
    addParameter(envSustain);
    addParameterToCategory(32, Category::Envelope);
    
    auto envRelease = std::make_shared<Parameter>(33, "Env Release", Parameter::Time);
    envRelease->setRange({0.001f, 10.0f, 1.0f, 0.001f, false});
    envRelease->setDisplay({"s", 3, true, true, false});
    addParameter(envRelease);
    addParameterToCategory(33, Category::Envelope);
}

void PluginParameters::createLFOParameters() {
    // LFO Rate
    auto lfoRate = std::make_shared<Parameter>(40, "LFO Rate", Parameter::Float);
    lfoRate->setRange({0.1f, 20.0f, 1.0f, 0.01f});
    lfoRate->setDisplay({"Hz", 2, true, true, false});
    addParameter(lfoRate);
    addParameterToCategory(40, Category::LFO);
    
    // LFO Amount
    auto lfoAmount = std::make_shared<Parameter>(41, "LFO Amount", Parameter::Float);
    lfoAmount->setRange({0.0f, 1.0f, 0.5f, 0.01f});
    lfoAmount->setDisplay({"%", 0, true, true, false});
    addParameter(lfoAmount);
    addParameterToCategory(41, Category::LFO);
}

void PluginParameters::createEffectParameters() {
    // Reverb Mix
    auto reverbMix = std::make_shared<Parameter>(50, "Reverb Mix", Parameter::Float);
    reverbMix->setRange({0.0f, 1.0f, 0.3f, 0.01f});
    reverbMix->setDisplay({"%", 0, true, true, false});
    addParameter(reverbMix);
    addParameterToCategory(50, Category::Effects);
    
    // Delay Mix
    auto delayMix = std::make_shared<Parameter>(51, "Delay Mix", Parameter::Float);
    delayMix->setRange({0.0f, 1.0f, 0.2f, 0.01f});
    delayMix->setDisplay({"%", 0, true, true, false});
    addParameter(delayMix);
    addParameterToCategory(51, Category::Effects);
}

void PluginParameters::createModulationParameters() {
    // Mod Wheel
    auto modWheel = std::make_shared<Parameter>(60, "Mod Wheel", Parameter::Float);
    modWheel->setRange({0.0f, 1.0f, 0.0f, 0.01f});
    modWheel->setDisplay({"%", 0, true, true, false});
    addParameter(modWheel);
    addParameterToCategory(60, Category::Modulation);
    
    // Aftertouch
    auto aftertouch = std::make_shared<Parameter>(61, "Aftertouch", Parameter::Float);
    aftertouch->setRange({0.0f, 1.0f, 0.0f, 0.01f});
    aftertouch->setDisplay({"%", 0, true, true, false});
    addParameter(aftertouch);
    addParameterToCategory(61, Category::Modulation);
}

float PluginParameters::normalizeValue(float value, const Parameter::Range& range) const {
    if (range.logarithmic) {
        return (std::log(std::max(value, 0.001f)) - std::log(range.min)) / 
               (std::log(range.max) - std::log(range.min));
    } else {
        return (value - range.min) / (range.max - range.min);
    }
}

float PluginParameters::denormalizeValue(float normalized, const Parameter::Range& range) const {
    normalized = juce::jlimit(0.0f, 1.0f, normalized);
    
    if (range.logarithmic) {
        float logMin = std::log(range.min);
        float logMax = std::log(range.max);
        return std::exp(logMin + normalized * (logMax - logMin));
    } else {
        return range.min + normalized * (range.max - range.min);
    }
}

void PluginParameters::handleMidiCC(int cc, int value, int channel) {
    // Find parameters mapped to this CC
    std::lock_guard<std::mutex> lock(parametersMutex_);
    
    for (const auto& parameter : parameters_) {
        auto mapping = parameter->getMidiMapping();
        if (mapping.enabled && mapping.cc == cc && mapping.channel == channel) {
            float normalizedValue = value / 127.0f;
            float parameterValue = mapping.isBipolar ? 
                (normalizedValue * 2.0f - 1.0f) * (mapping.maxValue - mapping.minValue) + mapping.minValue :
                normalizedValue * (mapping.maxValue - mapping.minValue) + mapping.minValue;
            
            setValue(parameter->getId(), parameterValue);
        }
    }
}

void PluginParameters::updateMidiMappings() {
    // Update MIDI mappings based on current settings
}

bool PluginParameters::validateParameterRange(const Parameter& param) const {
    auto range = param.getRange();
    return range.min < range.max && range.defaultValue >= range.min && range.defaultValue <= range.max;
}

bool PluginParameters::validateParameterType(const Parameter& param) const {
    // Validate parameter type-specific constraints
    return true;
}

bool PluginParameters::validateParameterName(const Parameter& param) const {
    juce::String name = param.getName();
    return name.isNotEmpty() && !name.containsNonPrintableChars();
}

void PluginParameters::cleanupUnusedParameters() {
    // Remove parameters that are no longer needed
}

void PluginParameters::optimizeMemoryUsage() {
    // Optimize parameter system memory usage
    cleanupUnusedParameters();
}

//==============================================================================
// Helper Functions Namespace

namespace parameters {

std::shared_ptr<Parameter> createOscillatorTypeParameter(int oscillatorId) {
    auto param = std::make_shared<Parameter>(oscillatorId * 100 + 10, "Oscillator Type", Parameter::Choice);
    // Set up oscillator type choices
    return param;
}

std::shared_ptr<Parameter> createFrequencyParameter(const juce::String& name, 
                                                   float minFreq, float maxFreq) {
    auto param = std::make_shared<Parameter>(0, name, Parameter::Frequency);
    param->setRange({minFreq, maxFreq, 440.0f, 0.01f, true});
    param->setDisplay({"Hz", 2, true, true, false});
    return param;
}

std::shared_ptr<Parameter> createGainParameter(const juce::String& name, 
                                             float minDb, float maxDb) {
    auto param = std::make_shared<Parameter>(0, name, Parameter::Decibels);
    float minGain = std::pow(10.0f, minDb / 20.0f);
    float maxGain = std::pow(10.0f, maxDb / 20.0f);
    float defaultGain = std::pow(10.0f, (minDb + maxDb) / 2.0f / 20.0f);
    
    param->setRange({minGain, maxGain, defaultGain, 0.01f, true});
    param->setDisplay({"dB", 1, true, true, false});
    return param;
}

std::shared_ptr<Parameter> createTimeParameter(const juce::String& name,
                                             float minTime, float maxTime) {
    auto param = std::make_shared<Parameter>(0, name, Parameter::Float);
    param->setRange({minTime, maxTime, (minTime + maxTime) / 2.0f, 0.001f, false});
    param->setDisplay({"s", 3, true, true, false});
    return param;
}

std::shared_ptr<Parameter> createChoiceParameter(const juce::String& name,
                                               const juce::StringArray& choices) {
    auto param = std::make_shared<Parameter>(0, name, Parameter::Choice);
    // param->getDisplay().choices = choices;
    param->setRange({0.0f, static_cast<float>(choices.size() - 1), 0.0f, 1.0f, false});
    return param;
}

std::shared_ptr<Parameter> createPercentParameter(const juce::String& name) {
    auto param = std::make_shared<Parameter>(0, name, Parameter::Float);
    param->setRange({0.0f, 100.0f, 50.0f, 0.1f, false});
    param->setDisplay({"%", 1, true, true, false});
    return param;
}

std::shared_ptr<Parameter> createBooleanParameter(const juce::String& name) {
    auto param = std::make_shared<Parameter>(0, name, Parameter::Bool);
    param->setDisplay({"", 0, true, true, false});
    return param;
}

std::shared_ptr<Parameter> createTuningParameter(const juce::String& name,
                                               float minCents, float maxCents) {
    auto param = std::make_shared<Parameter>(0, name, Parameter::Cents);
    param->setRange({minCents, maxCents, 0.0f, 1.0f, false});
    param->setDisplay({"cents", 0, true, true, false});
    return param;
}

std::shared_ptr<Parameter> createNoteParameter(const juce::String& name,
                                             int minNote, int maxNote) {
    auto param = std::make_shared<Parameter>(0, name, Parameter::Note);
    param->setRange({static_cast<float>(minNote), static_cast<float>(maxNote), 
                    static_cast<float>((minNote + maxNote) / 2), 1.0f, false});
    param->setDisplay({"", 0, true, true, false});
    return param;
}

std::shared_ptr<ParameterGroup> createSynthesisGroup(const juce::String& name) {
    return std::make_shared<ParameterGroup>(name, name + " Synthesis");
}

std::shared_ptr<ParameterGroup> createEffectsGroup(const juce::String& name) {
    return std::make_shared<ParameterGroup>(name, name + " Effects");
}

std::shared_ptr<ParameterGroup> createModulationGroup(const juce::String& name) {
    return std::make_shared<ParameterGroup>(name, name + " Modulation");
}

} // namespace parameters

} // namespace plugin
} // namespace vital