#pragma once

#include <memory>
#include <string>
#include <vector>

namespace vital {

class PluginInterface;
class Settings;
class AudioOutput;
class MidiInput;
class MainWindow;

class Application {
public:
    Application();
    ~Application();

    int run(int argc, char* argv[]);
    void shutdown();

    // Singleton access
    static Application& getInstance();

    // Core subsystems
    PluginInterface* getPluginInterface() const;
    Settings* getSettings() const;
    AudioOutput* getAudioOutput() const;
    MidiInput* getMidiInput() const;
    MainWindow* getMainWindow() const;

    // Configuration
    void loadConfiguration(const std::string& config_path);
    void saveConfiguration(const std::string& config_path);

private:
    class Impl;
    std::unique_ptr<Impl> impl_;

    bool initialize(int argc, char* argv[]);
    void deinitialize();
};

} // namespace vital