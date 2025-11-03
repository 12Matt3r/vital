#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace vital {

class Settings {
public:
    struct AudioSettings {
        int sample_rate;
        int buffer_size;
        int num_channels;
        bool enable_input_monitoring;
        float input_gain;
        float output_gain;
        bool enable_direct_monitoring;
    };

    struct MidiSettings {
        bool enable_input;
        bool enable_output;
        bool midi_thru;
        int input_channel;
        int output_channel;
        bool enable_midi_clock;
        float midi_clock_tempo;
        std::vector<std::string> input_devices;
        std::vector<std::string> output_devices;
    };

    struct UiSettings {
        int window_width;
        int window_height;
        bool window_maximized;
        bool always_on_top;
        std::string theme;
        int font_size;
        bool show_tooltips;
        bool enable_animations;
        std::string language;
    };

    struct PerformanceSettings {
        int max_voices;
        bool enable_oversampling;
        bool enable_anti_aliasing;
        int audio_thread_priority;
        bool enable_multithreading;
        int max_cpu_usage_percent;
        bool enable_performance_monitoring;
    };

    struct GlobalSettings {
        bool enable_automation;
        bool enable_presets;
        bool enable_midi_recording;
        bool auto_save_presets;
        std::string preset_folder;
        std::string theme_folder;
        bool check_updates;
        std::string update_channel; // "stable", "beta", "dev"
        std::string language;
        bool enable_telemetry;
    };

    Settings();
    ~Settings();

    // Load/Save settings
    void loadFromFile(const std::string& filename);
    void saveToFile(const std::string& filename);
    void loadDefaults();
    
    // Parameter access
    void setValue(const std::string& key, const std::string& value);
    void setValue(const std::string& key, float value);
    void setValue(const std::string& key, int value);
    void setValue(const std::string& key, bool value);
    
    std::string getString(const std::string& key, const std::string& default_value = "") const;
    float getFloat(const std::string& key, float default_value = 0.0f) const;
    int getInt(const std::string& key, int default_value = 0) const;
    bool getBool(const std::string& key, bool default_value = false) const;
    
    // Setting categories
    AudioSettings& audio() { return audio_settings_; }
    MidiSettings& midi() { return midi_settings_; }
    UiSettings& ui() { return ui_settings_; }
    PerformanceSettings& performance() { return performance_settings_; }
    GlobalSettings& global() { return global_settings_; }
    
    const AudioSettings& audio() const { return audio_settings_; }
    const MidiSettings& midi() const { return midi_settings_; }
    const UiSettings& ui() const { return ui_settings_; }
    const PerformanceSettings& performance() const { return performance_settings_; }
    const GlobalSettings& global() const { return global_settings_; }

private:
    void parseValue(const std::string& key, const std::string& value);
    std::string serializeSettings() const;
    void deserializeSettings(const std::string& data);
    
    // String utilities
    std::string trim(const std::string& str) const;
    std::vector<std::string> split(const std::string& str, char delimiter) const;
    bool isNumber(const std::string& str) const;
    
    AudioSettings audio_settings_;
    MidiSettings midi_settings_;
    UiSettings ui_settings_;
    PerformanceSettings performance_settings_;
    GlobalSettings global_settings_;
    
    // Generic parameter storage
    std::unordered_map<std::string, std::string> string_params_;
    std::unordered_map<std::string, float> float_params_;
    std::unordered_map<std::string, int> int_params_;
    std::unordered_map<std::string, bool> bool_params_;
    
    // File management
    std::string settings_file_path_;
    bool dirty_;
    
    // Callbacks
    std::function<void(const std::string& key, const std::string& old_value, const std::string& new_value)> onSettingChanged;
    
    // Auto-save
    bool auto_save_enabled_;
    float auto_save_interval_;
    double last_save_time_;
};

} // namespace vital