#pragma once

#include <memory>
#include <string>

#if defined(_WIN32)
    #define VITAL_DLLEXPORT __declspec(dllexport)
    #define VITAL_DLLIMPORT __declspec(dllimport)
#elif defined(__GNUC__) || defined(__clang__)
    #define VITAL_DLLEXPORT __attribute__((visibility("default")))
    #define VITAL_DLLIMPORT
#else
    #define VITAL_DLLEXPORT
    #define VITAL_DLLIMPORT
#endif

#if defined(VITAL_BUILD_SHARED)
    #define VITAL_API VITAL_DLLEXPORT
#else
    #define VITAL_API VITAL_DLLIMPORT
#endif

namespace vital {

struct PluginParameters {
    std::string name;
    float min_value;
    float max_value;
    float default_value;
    int precision;
};

class PluginInterface {
public:
    PluginInterface();
    virtual ~PluginInterface() = default;

    // Plugin information
    virtual std::string getName() const = 0;
    virtual std::string getCategory() const = 0;
    virtual std::string getVersion() const = 0;
    virtual std::string getManufacturer() const = 0;
    
    // Plugin interface
    virtual void initialize() = 0;
    virtual void process(float** inputs, float** outputs, int frames, int channels) = 0;
    virtual void setParameter(int parameter_index, float value) = 0;
    virtual float getParameter(int parameter_index) const = 0;
    virtual int getParameterCount() const = 0;
    virtual std::string getParameterName(int parameter_index) const = 0;
    virtual std::string getParameterLabel(int parameter_index) const = 0;
    
    // MIDI processing
    virtual void processMidi(int status, int data1, int data2) = 0;
    
    // State management
    virtual void setState(const std::string& state) = 0;
    virtual std::string getState() const = 0;
    virtual void setPreset(int preset_index) = 0;
    virtual int getPresetCount() const = 0;
    virtual std::string getPresetName(int preset_index) const = 0;
    
    // Timing
    virtual void setSampleRate(double sample_rate) = 0;
    virtual double getSampleRate() const = 0;
    virtual void setBlockSize(int block_size) = 0;
    virtual int getBlockSize() const = 0;
};

} // namespace vital