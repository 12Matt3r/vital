/*
  ==============================================================================
    vst3_wrapper.h
    Copyright (c) 2025 Vital Plugin Integration Team
    https://vital.audio
    
    VST3 wrapper implementation for Vital plugin
    Provides VST3-specific functionality including host communication,
    parameter automation, MIDI handling, and VST3-compliant features.
  ==============================================================================
*/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_vst3/juce_vst3.h>
#include <juce_re果per/juce_re果per.h>
#include <pluginterfaces/vst/ivstparameterchanges.h>
#include <pluginterfaces/vst/ivstnoteexpression.h>
#include <pluginterfaces/vst/ivstmidiccout.h>
#include <pluginterfaces/vst/ivstaudioprocessor.h>
#include <pluginterfaces/vst/ivsteditcontroller.h>
#include <pluginterfaces/vst/ivstconnection.h>
#include <pluginterfaces/vst/ivsthostapplication.h>
#include <pluginterfaces/vst/ivstpluginterfacesupport.h>
#include <pluginterfaces/vst/ivstunits.h>
#include <pluginterfaces/base/funknown.h>

namespace vital {
namespace plugin {

//==============================================================================
/**
 * @class VST3HostApplication
 * @brief VST3 Host Application interface implementation
 */
class VST3HostApplication : public Steinberg::FObject,
                          public Steinberg::Vst::IHostApplication,
                          public Steinberg::Vst::IAttributeList,
                          public Steinberg::Vst::IMessage,
                          public Steinberg::Vst::IUnitInfo,
                          public Steinberg::Vst::IProgramList,
                          public Steinberg::Vst::INoteExpressionPhysicalUIMapping {
public:
    // FUnknown methods
    Steinberg::tresult PLUGIN_API queryInterface(const Steinberg::TUID iid, void** obj) override;
    Steinberg::uint32 PLUGIN_API addRef() override;
    Steinberg::uint32 PLUGIN_API release() override;
    
    // IHostApplication methods
    Steinberg::tresult PLUGIN_API getName(Steinberg::Vst::String128 name) override;
    Steinberg::tresult PLUGIN_API createInstance(Steinberg::TUID cid, Steinberg::TUID iid, void** obj) override;
    Steinberg::tresult PLUGIN_API setOwner(Steinberg::FUnknown* owner) override;
    
    // IAttributeList methods
    Steinberg::tresult PLUGIN_API setInt(Steinberg::FID iid, Steinberg::int64 value) override;
    Steinberg::tresult PLUGIN_API getInt(Steinberg::FID iid, Steinberg::int64& value) override;
    Steinberg::tresult PLUGIN_API setFloat(Steinberg::FID iid, double value) override;
    Steinberg::tresult PLUGIN_API getFloat(Steinberg::FID iid, double& value) override;
    Steinberg::tresult PLUGIN_API setString(Steinberg::FID iid, Steinberg::Vst::String128 string) override;
    Steinberg::tresult PLUGIN_API getString(Steinberg::FID iid, Steinberg::Vst::String128 string) override;
    Steinberg::tresult PLUGIN_API setBinary(Steinberg::FID iid, const void* data, Steinberg::int32 byteSize) override;
    Steinberg::tresult PLUGIN_API getBinary(Steinberg::FID iid, const void*& data, Steinberg::int32& byteSize) override;
    Steinberg::tresult PLUGIN_API remove(Steinberg::FID iid) override;
    
    // IMessage methods
    Steinberg::tresult PLUGIN_API getMessageID(Steinberg::FID id) override;
    Steinberg::tresult PLUGIN_API setMessageID(Steinberg::FID id) override;
    Steinberg::Vst::IAttributeList* PLUGIN_API getAttributes() override;
    
    // IUnitInfo methods
    Steinberg::int32 PLUGIN_API getUnitCount() override;
    Steinberg::tresult PLUGIN_API getUnitInfo(Steinberg::int32 unitId, Steinberg::Vst::UnitInfo& info) override;
    Steinberg::int32 PLUGIN_API getProgramListCount() override;
    Steinberg::tresult PLUGIN_API getProgramListInfo(Steinberg::int32 listId, Steinberg::Vst::ProgramListInfo& info) override;
    Steinberg::tresult PLUGIN_API getProgramName(Steinberg::int32 listId, Steinberg::int32 programIndex, Steinberg::Vst::String128 name) override;
    Steinberg::tresult PLUGIN_API getProgramInfo(Steinberg::int32 listId, Steinberg::int32 programIndex, Steinberg::FID attributeId, Steinberg::Vst::String128 attributeValue) override;
    Steinberg::tresult PLUGIN_API hasProgramPitchNames(Steinberg::int32 listId, Steinberg::int32 programIndex) override;
    Steinberg::tresult PLUGIN_API getProgramPitchName(Steinberg::int32 listId, Steinberg::int32 programIndex, Steinberg::int16 pitch, Steinberg::Vst::String128 name) override;
    
    // IProgramList methods
    Steinberg::tresult PLUGIN_API setProgramName(Steinberg::int32 listId, Steinberg::int32 programIndex, const Steinberg::Vst::String128 name) override;
    Steinberg::tresult PLUGIN_API setProgramData(Steinberg::int32 listId, Steinberg::int32 programIndex, Steinberg::int32 payloadSize, const void* payload) override;
    
    // INoteExpressionPhysicalUIMapping methods
    Steinberg::int32 PLUGIN_API getNoteExpressionCount(Steinberg::Vst::NoteExpressionTypeID typeId) override;
    Steinberg::tresult PLUGIN_API getNoteExpressionInfo(Steinberg::int32 noteExpressionIndex, Steinberg::Vst::NoteExpressionTypeInfo& info) override;
    Steinberg::tresult PLUGIN_API getNoteExpressionStringByValue(Steinberg::Vst::NoteExpressionTypeID typeId, Steinberg::Vst::NoteExpressionValue valueNormalized, Steinberg::Vst::String128 string) override;
    Steinberg::tresult PLUGIN_API getNoteExpressionValueByString(Steinberg::Vst::NoteExpressionTypeID typeId, const Steinberg::Vst::String128 string, Steinberg::Vst::NoteExpressionValue& valueNormalized) override;
    
private:
    Steinberg::FUnknown* owner_ = nullptr;
    std::map<Steinberg::FID, Steinberg::int64> intAttributes_;
    std::map<Steinberg::FID, double> floatAttributes_;
    std::map<Steinberg::FID, juce::String> stringAttributes_;
    std::map<Steinberg::FID, std::vector<uint8>> binaryAttributes_;
    
    Steinberg::FID messageId_;
    std::unique_ptr<Steinberg::Vst::IAttributeList> messageAttributes_;
    
    std::vector<Steinberg::Vst::UnitInfo> units_;
    std::vector<Steinberg::Vst::ProgramListInfo> programLists_;
    std::vector<std::vector<juce::String>> programNames_;
};

//==============================================================================
/**
 * @class VST3Processor
 * @brief VST3 Audio Processor implementation
 */
class VST3Processor : public Steinberg::Vst::IAudioProcessor,
                     public Steinberg::Vst::IComponent,
                     public Steinberg::Vst::IConnectionPoint,
                     public Steinberg::Vst::IParamValueQueue,
                     public Steinberg::Vst::IMidiMapping,
                     public Steinberg::Vst::INoteExpression,
                     public Steinberg::Vst::IEditController,
                     public Steinberg::Vst::IUnitInfo,
                     public Steinberg::Vst::IProgramList {
public:
    VST3Processor();
    ~VST3Processor() override;
    
    // FUnknown methods
    Steinberg::tresult PLUGIN_API queryInterface(const Steinberg::TUID iid, void** obj) override;
    Steinberg::uint32 PLUGIN_API addRef() override;
    Steinberg::uint32 PLUGIN_API release() override;
    
    // IPluginBase methods
    Steinberg::tresult PLUGIN_API initialize(FUnknown* context) override;
    Steinberg::tresult PLUGIN_API terminate() override;
    Steinberg::tresult PLUGIN_API setComponentState(Steinberg::IBStream* state) override;
    
    // IComponent methods
    Steinberg::tresult PLUGIN_API connect(IConnectionPoint* other) override;
    Steinberg::tresult PLUGIN_API disconnect(IConnectionPoint* other) override;
    Steinberg::tresult PLUGIN_API notify(IMessage* message) override;
    
    Steinberg::int32 PLUGIN_API getBusCount(Steinberg::Vst::MediaType type, Steinberg::Vst::BusDirection dir) override;
    Steinberg::tresult PLUGIN_API getBusInfo(Steinberg::Vst::MediaType type, Steinberg::Vst::BusDirection dir, Steinberg::int32 index, Steinberg::Vst::BusInfo& info) override;
    Steinberg::tresult PLUGIN_API getRoutingInfo(Steinberg::Vst::RoutingInfo& inInfo, Steinberg::Vst::RoutingInfo& outInfo) override;
    Steinberg::tresult PLUGIN_API activateBus(Steinberg::Vst::MediaType type, Steinberg::Vst::BusDirection dir, Steinberg::int32 index, Steinberg::TBool state) override;
    Steinberg::tresult PLUGIN_API setBusArrangements(Steinberg::Vst::SpeakerArrangement* inputs, Steinberg::int32 numIns, Steinberg::Vst::SpeakerArrangement* outputs, Steinberg::int32 numOuts) override;
    Steinberg::tresult PLUGIN_API getBusArrangements(Steinberg::Vst::SpeakerArrangement* inputs, Steinberg::int32& numIns, Steinberg::Vst::SpeakerArrangement* outputs, Steinberg::int32& numOuts) override;
    Steinberg::tresult PLUGIN_API setActive(TBool state) override;
    Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream* state) override;
    Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream* state) override;
    
    // IAudioProcessor methods
    Steinberg::tresult PLUGIN_API process(Steinberg::Vst::ProcessData& data) override;
    Steinberg::int32 PLUGIN_API getLatencySamples() override;
    Steinberg::tresult PLUGIN_API setupProcessing(Steinberg::Vst::ProcessSetup& setup) override;
    Steinberg::tresult PLUGIN_API getTailSamples(Steinberg::int32& samples) override;
    
    // IParamValueQueue methods
    Steinberg::int32 PLUGIN_API getParameterCount() override;
    Steinberg::tresult PLUGIN_API getParameterInfo(Steinberg::int32 paramIndex, Steinberg::Vst::ParameterInfo& info) override;
    Steinberg::int32 PLUGIN_API getPointCount(Steinberg::int32 paramId) override;
    Steinberg::tresult PLUGIN_API getPoint(Steinberg::int32 paramId, Steinberg::int32 index, Steinberg::int32& sampleOffset, Steinberg::Vst::ParamValue& value) override;
    Steinberg::tresult PLUGIN_API addPoint(Steinberg::int32 paramId, Steinberg::int32 sampleOffset, Steinberg::Vst::ParamValue value, Steinberg::int32& index) override;
    
    // IMidiMapping methods
    Steinberg::int32 PLUGIN_API getMidiControllerCount() override;
    Steinberg::tresult PLUGIN_API getMidiControllerAssignment(Steinberg::int32 busIndex, Steinberg::int16 channel, Steinberg::Vst::ControllerAssignment& id) override;
    
    // INoteExpression methods
    Steinberg::int32 PLUGIN_API getNoteExpressionCount(Steinberg::Vst::NoteExpressionTypeID typeId) override;
    Steinberg::tresult PLUGIN_API getNoteExpressionInfo(Steinberg::int32 noteExpressionIndex, Steinberg::Vst::NoteExpressionTypeInfo& info) override;
    Steinberg::tresult PLUGIN_API getNoteExpressionStringByValue(Steinberg::Vst::NoteExpressionTypeID typeId, Steinberg::Vst::NoteExpressionValue valueNormalized, Steinberg::Vst::String128 string) override;
    Steinberg::tresult PLUGIN_API getNoteExpressionValueByString(Steinberg::Vst::NoteExpressionTypeID typeId, const Steinberg::Vst::String128 string, Steinberg::Vst::NoteExpressionValue& valueNormalized) override;
    
    // IEditController methods
    Steinberg::int32 PLUGIN_API getParameterCount() override;
    Steinberg::tresult PLUGIN_API getParameterInfo(Steinberg::int32 paramIndex, Steinberg::Vst::ParameterInfo& info) override;
    Steinberg::tresult PLUGIN_API getParameterStringFromValue(Steinberg::Vst::ParamID paramId, Steinberg::Vst::ParamValue valueNormalized, Steinberg::Vst::String128 string) override;
    Steinberg::tresult PLUGIN_API getParameterValueFromString(Steinberg::Vst::ParamID paramId, const Steinberg::Vst::String128 string, Steinberg::Vst::ParamValue& valueNormalized) override;
    Steinberg::tresult PLUGIN_API setComponentState(Steinberg::IBStream* state) override;
    Steinberg::tresult PLUGIN_API setState(Steinberg::IBStream* state) override;
    Steinberg::tresult PLUGIN_API getState(Steinberg::IBStream* state) override;
    Steinberg::tresult PLUGIN_API openEditor(Steinberg::FID iid, void* parent, Steinberg::Vst::PlatformType platformType) override;
    Steinberg::tresult PLUGIN_API closeEditor() override;
    Steinberg::tresult PLUGIN_API getEditorSize(Steinberg::Vst::ViewRect* size) override;
    Steinberg::tresult PLUGIN_API onSize(Steinberg::Vst::ViewRect* newSize) override;
    Steinberg::tresult PLUGIN_API canResize() override;
    Steinberg::tresult PLUGIN_API checkSizeConstraint(Steinberg::Vst::ViewRect* rect) override;
    
    // IUnitInfo methods
    Steinberg::int32 PLUGIN_API getUnitCount() override;
    Steinberg::tresult PLUGIN_API getUnitInfo(Steinberg::int32 unitId, Steinberg::Vst::UnitInfo& info) override;
    Steinberg::int32 PLUGIN_API getProgramListCount() override;
    Steinberg::tresult PLUGIN_API getProgramListInfo(Steinberg::int32 listId, Steinberg::Vst::ProgramListInfo& info) override;
    Steinberg::tresult PLUGIN_API getProgramName(Steinberg::int32 listId, Steinberg::int32 programIndex, Steinberg::Vst::String128 name) override;
    Steinberg::tresult PLUGIN_API getProgramInfo(Steinberg::int32 listId, Steinberg::int32 programIndex, Steinberg::FID attributeId, Steinberg::Vst::String128 attributeValue) override;
    Steinberg::tresult PLUGIN_API hasProgramPitchNames(Steinberg::int32 listId, Steinberg::int32 programIndex) override;
    Steinberg::tresult PLUGIN_API getProgramPitchName(Steinberg::int32 listId, Steinberg::int32 programIndex, Steinberg::int16 pitch, Steinberg::Vst::String128 name) override;
    
    // IProgramList methods
    Steinberg::tresult PLUGIN_API setProgramName(Steinberg::int32 listId, Steinberg::int32 programIndex, const Steinberg::Vst::String128 name) override;
    Steinberg::tresult PLUGIN_API setProgramData(Steinberg::int32 listId, Steinberg::int32 programIndex, Steinberg::int32 payloadSize, const void* payload) override;
    
    // Plugin integration
    void setAudioProcessor(juce::AudioProcessor* processor);
    juce::AudioProcessor* getAudioProcessor() const { return processor_; }
    
    // Host communication
    Steinberg::tresult sendMessage(const Steinberg::FID id, const void* data, Steinberg::int32 size);
    Steinberg::tresult receiveMessage(const Steinberg::FID id, const void* data, Steinberg::int32 size);
    
    // Parameter management
    Steinberg::tresult beginParameterEdit(Steinberg::Vst::ParamID paramId);
    Steinberg::tresult performParameterEdit(Steinberg::Vst::ParamID paramId, Steinberg::Vst::ParamValue value, Steinberg::int32 sampleOffset);
    Steinberg::tresult endParameterEdit(Steinberg::Vst::ParamID paramId);
    
    // MIDI handling
    Steinberg::tresult processMidiEvent(const Steinberg::Vst::MIDIMessage& message, Steinberg::int32 sampleOffset);
    
    // Note expression
    Steinberg::tresult setNoteExpression(Steinberg::int32 noteId, Steinberg::Vst::NoteExpressionTypeID typeId, Steinberg::Vst::NoteExpressionValue value);
    Steinberg::tresult getNoteExpression(Steinberg::int32 noteId, Steinberg::Vst::NoteExpressionTypeID typeId, Steinberg::Vst::NoteExpressionValue& value);
    
private:
    juce::AudioProcessor* processor_ = nullptr;
    
    // VST3 messaging
    Steinberg::FUnknown* connectionPoint_ = nullptr;
    std::map<Steinberg::FID, std::vector<uint8>> pendingMessages_;
    
    // Parameter queues
    std::vector<std::unique_ptr<Steinberg::Vst::IParamValueQueue>> parameterQueues_;
    
    // Note expressions
    std::map<std::pair<int32_t, Steinberg::Vst::NoteExpressionTypeID>, Steinberg::Vst::NoteExpressionValue> noteExpressions_;
    
    // Unit and program information
    std::vector<Steinberg::Vst::UnitInfo> units_;
    std::vector<Steinberg::Vst::ProgramListInfo> programLists_;
    std::vector<std::vector<juce::String>> programNames_;
    
    // Processing state
    Steinberg::Vst::ProcessSetup processSetup_;
    Steinberg::int32 latencySamples_ = 0;
    Steinberg::int32 tailSamples_ = 0;
    
    // Internal methods
    void initializeUnits();
    void initializeProgramLists();
    Steinberg::tresult createParameterInfo();
    Steinberg::tresult createNoteExpressionInfo();
    
    // Stream handling
    Steinberg::tresult writeStateToStream(Steinberg::IBStream* stream);
    Steinberg::tresult readStateFromStream(Steinberg::IBStream* stream);
    
    // Message handling
    Steinberg::tresult handleMessage(Steinberg::IMessage* message);
    Steinberg::tresult createMessage(const Steinberg::FID id, Steinberg::IMessage*& message);
};

//==============================================================================
/**
 * @class VST3Wrapper
 * @brief Main VST3 wrapper class
 */
class VST3Wrapper {
public:
    VST3Wrapper();
    ~VST3Wrapper();
    
    // Plugin reference
    void setPlugin(juce::AudioProcessor* plugin);
    juce::AudioProcessor* getPlugin() const { return plugin_; }
    
    // VST3 processor
    Steinberg::Vst::IAudioProcessor* getProcessor() const { return processor_.get(); }
    
    // VST3 component
    Steinberg::Vst::IComponent* getComponent() const { return processor_.get(); }
    
    // Host application
    Steinberg::Vst::IHostApplication* getHostApplication() const { return hostApplication_.get(); }
    
    // Initialization
    Steinberg::tresult initialize(Steinberg::FUnknown* context);
    Steinberg::tresult shutdown();
    
    // Host communication
    Steinberg::tresult setHost(Steinberg::Vst::IHostApplication* host);
    Steinberg::tresult getHost(Steinberg::Vst::IHostApplication*& host) const;
    
    // Plugin state
    Steinberg::tresult savePluginState(Steinberg::IBStream* stream);
    Steinberg::tresult loadPluginState(Steinberg::IBStream* stream);
    
    // Parameter automation
    Steinberg::tresult setupParameterAutomation();
    Steinberg::tresult processParameterChanges(Steinberg::Vst::IParameterChanges* changes);
    
    // MIDI processing
    Steinberg::tresult processMIDIEvents(Steinberg::Vst::IMidiEvents* events);
    
    // Note expression support
    Steinberg::tresult enableNoteExpression(bool enabled);
    Steinberg::tresult getSupportedNoteExpressionTypes(std::vector<Steinberg::Vst::NoteExpressionTypeID>& types);
    
    // MPE support
    Steinberg::tresult enableMPE(bool enabled);
    Steinberg::tresult setMPESettings(int channels, float pitchBendRange, float pressureRange, float timbreRange);
    
    // Sidechain support
    Steinberg::tresult enableSidechain(bool enabled);
    Steinberg::tresult getSidechainFormat(Steinberg::Vst::SpeakerArrangement& arrangement);
    
    // Preset support
    Steinberg::tresult setupPresetManagement();
    Steinberg::tresult savePreset(const Steinberg::Vst::String128 name, Steinberg::IBStream* stream);
    Steinberg::tresult loadPreset(const Steinberg::Vst::String128 name, Steinberg::IBStream* stream);
    
    // Performance monitoring
    Steinberg::tresult enablePerformanceReporting(bool enabled);
    Steinberg::tresult getPerformanceMetrics(float& cpuUsage, Steinberg::int32& voiceCount, Steinberg::int32& xruns);
    
    // Error handling
    Steinberg::tresult getLastError(juce::String& error) const;
    Steinberg::tresult clearError();
    
    // Utility functions
    Steinberg::tresult validateVST3Compliance();
    Steinberg::tresult generateUUID(Steinberg::TUID uid);
    Steinberg::tresult getPluginInformation(juce::String& info) const;
    
private:
    juce::AudioProcessor* plugin_ = nullptr;
    
    // VST3 components
    std::unique_ptr<VST3Processor> processor_;
    std::unique_ptr<VST3HostApplication> hostApplication_;
    
    // Host interface
    Steinberg::Vst::IHostApplication* host_ = nullptr;
    
    // Feature flags
    bool noteExpressionEnabled_ = false;
    bool mpeEnabled_ = false;
    bool sidechainEnabled_ = false;
    bool presetManagementEnabled_ = false;
    bool performanceReportingEnabled_ = false;
    
    // Error handling
    mutable juce::String lastError_;
    mutable std::mutex errorMutex_;
    
    // Internal methods
    Steinberg::tresult initializeHost();
    Steinberg::tresult initializeProcessor();
    Steinberg::tresult initializeFeatures();
    
    void setError(const Steinberg::tresult result, const juce::String& context);
    Steinberg::tresult checkError(const Steinberg::tresult result, const juce::String& context);
    
    // Plugin information
    Steinberg::TUID pluginUID_;
    Steinberg::TUID processorUID_;
    Steinberg::TUID controllerUID_;
    
    // Version information
    Steinberg::int32 versionMajor_ = 3;
    Steinberg::int32 versionMinor_ = 0;
    Steinberg::int32 versionPatch_ = 0;
};

//==============================================================================
/**
 * @class VST3PluginFormat
 * @brief VST3 plugin format handler
 */
class VST3PluginFormat : public juce::AudioPluginFormat {
public:
    VST3PluginFormat();
    ~VST3PluginFormat() override;
    
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
    
    // VST3 specific
    void setScanProgressCallback(std::function<void(double)> callback);
    bool isScanning() const { return isScanning_; }
    void cancelScanning();
    
private:
    std::function<void(double)> scanProgressCallback_;
    std::atomic<bool> isScanning_{false};
    std::atomic<bool> cancelScan_{false};
    
    // Internal scanning methods
    void scanDirectory(const juce::File& directory, std::vector<juce::PluginDescription>& results);
    bool validateVST3Plugin(const juce::File& file) const;
    juce::PluginDescription createDescriptionFromVST3(const juce::File& file) const;
    juce::String extractPluginName(const juce::File& file) const;
    juce::String extractManufacturer(const juce::File& file) const;
    Steinberg::int32 extractVersion(const juce::File& file) const;
    juce::String extractCategory(const juce::File& file) const;
};

//==============================================================================
/**
 * @namespace vital::plugin::vst3
 * @brief VST3 specific helper functions and utilities
 */
namespace vst3 {

/**
 * Convert JUCE plugin description to VST3 plugin description
 */
juce::PluginDescription toVST3Description(const juce::PluginDescription& description);

/**
 * Convert VST3 plugin description to JUCE plugin description
 */
juce::PluginDescription fromVST3Description(const juce::PluginDescription& description);

/**
 * Extract plugin information from VST3 module
 */
bool extractVST3Info(const juce::File& file, juce::String& name, juce::String& manufacturer, juce::String& version);

/**
 * Create VST3 parameter info from JUCE parameter
 */
Steinberg::Vst::ParameterInfo createParameterInfo(const juce::AudioProcessorParameter* parameter);

/**
 * Convert VST3 parameter ID to JUCE parameter ID
 */
int vst3ParamIdToJuce(Steinberg::Vst::ParamID vst3Id);

/**
 * Convert JUCE parameter ID to VST3 parameter ID
 */
Steinberg::Vst::ParamID juceParamIdToVST3(int juceId);

/**
 * Get VST3 plugin format information
 */
juce::String getVST3FormatInfo();

/**
 * Check VST3 compliance
 */
bool validateVST3Compliance(const juce::File& file);

/**
 * Create VST3 host application
 */
Steinberg::Vst::IHostApplication* createHostApplication();

/**
 * Generate VST3 plugin UUID
 */
Steinberg::TUID generatePluginUUID();

/**
 * VST3 error codes
 */
namespace errors {
    static const Steinberg::tresult InvalidHostContext = Steinberg::kInvalidArgument;
    static const Steinberg::tresult PluginNotFound = Steinberg::kResultFalse;
    static const Steinberg::tresult InvalidParameter = Steinberg::kInvalidArgument;
    static const Steinberg::tresult ProcessingError = Steinberg::kInternalError;
    static const Steinberg::tresult HostNotSupported = Steinberg::kNotImplemented;
}

} // namespace vst3

} // namespace plugin
} // namespace vital