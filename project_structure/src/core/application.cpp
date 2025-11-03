#include "application.h"
#include "plugin_interface.h"
#include "settings.h"
#include "audio_output.h"
#include "midi_input.h"
#include "main_window.h"

namespace vital {

class Application::Impl {
public:
    std::unique_ptr<PluginInterface> plugin_interface;
    std::unique_ptr<Settings> settings;
    std::unique_ptr<AudioOutput> audio_output;
    std::unique_ptr<MidiInput> midi_input;
    std::unique_ptr<MainWindow> main_window;
    bool initialized;
};

Application::Application() : impl_(std::make_unique<Impl>()) {
    impl_->initialized = false;
}

Application::~Application() {
    if (impl_->initialized) {
        shutdown();
    }
}

int Application::run(int argc, char* argv[]) {
    if (!initialize(argc, argv)) {
        return 1;
    }
    
    // Main application loop
    while (true) {
        impl_->main_window->processEvents();
        
        // Check for shutdown conditions
        if (!impl_->main_window->isVisible()) {
            break;
        }
        
        // Process audio
        impl_->audio_output->process();
        
        // Add small delay to prevent excessive CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    
    shutdown();
    return 0;
}

void Application::shutdown() {
    if (impl_->initialized) {
        impl_->main_window->shutdown();
        impl_->audio_output->shutdown();
        impl_->midi_input->shutdown();
        impl_->initialized = false;
    }
}

Application& Application::getInstance() {
    static Application instance;
    return instance;
}

PluginInterface* Application::getPluginInterface() const {
    return impl_->plugin_interface.get();
}

Settings* Application::getSettings() const {
    return impl_->settings.get();
}

AudioOutput* Application::getAudioOutput() const {
    return impl_->audio_output.get();
}

MidiInput* Application::getMidiInput() const {
    return impl_->midi_input.get();
}

MainWindow* Application::getMainWindow() const {
    return impl_->main_window.get();
}

bool Application::initialize(int argc, char* argv[]) {
    try {
        impl_->settings = std::make_unique<Settings>();
        impl_->audio_output = std::make_unique<AudioOutput>();
        impl_->midi_input = std::make_unique<MidiInput>();
        impl_->main_window = std::make_unique<MainWindow>();
        
        // Initialize subsystems
        impl_->audio_output->initialize();
        impl_->midi_input->initialize();
        
        // Load configuration
        loadConfiguration("vital_settings.json");
        
        impl_->initialized = true;
        return true;
    } catch (const std::exception& e) {
        // Log error
        return false;
    }
}

void Application::deinitialize() {
    if (impl_->initialized) {
        shutdown();
    }
}

void Application::loadConfiguration(const std::string& config_path) {
    impl_->settings->loadFromFile(config_path);
    
    // Apply settings to subsystems
    const auto& audio_settings = impl_->settings->audio();
    impl_->audio_output->setSampleRate(audio_settings.sample_rate);
    impl_->audio_output->setBufferSize(audio_settings.buffer_size);
    
    const auto& ui_settings = impl_->settings->ui();
    impl_->main_window->setSize(ui_settings.window_width, ui_settings.window_height);
}

void Application::saveConfiguration(const std::string& config_path) {
    // Get current window state
    auto& ui_settings = impl_->settings->ui();
    ui_settings.window_width = impl_->main_window->getWidth();
    ui_settings.window_height = impl_->main_window->getHeight();
    
    impl_->settings->saveToFile(config_path);
}

} // namespace vital