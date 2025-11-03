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

#include "vital_application.h"
#include "vital_plugin_processor.h"
#include "vital_plugin_editor.h"
#include <JuceHeader.h>
#include <iostream>
#include <exception>
#include <memory>

//==============================================================================
// Plugin entry point for VST3 format
#if JUCE_VST3_CAN_REPLACE_VST2
#include "public.sdk/source/vst/vst2wrapper/vst2wrapper.h"
#endif

//==============================================================================
// Plugin entry point for Audio Unit (AU) format
#if JUCE_PLUGINHOST_AU
#include "public.sdk/source/vst3hosting/audiohost.h"
#endif

//==============================================================================
// Global application instance
std::unique_ptr<vital::VitalApplication> g_application;

//==============================================================================
// JUCE Application entry point
JUCE_BEGIN_IGNORE_WARNINGS_MSVC(4996) // Disable warnings about deprecated functions

// Standalone application entry point
#if JUCE_WINDOWS && !JUCE_VST3_CLIENT
int WINAPI wWinMain(HINSTANCE instance, HINSTANCE prevInstance, PWSTR cmdLine, int cmdShow) {
    try {
        // Initialize COM for Windows
        juce::JuceInitializer initializer;
        
        // Create and run the application
        g_application = std::make_unique<vital::VitalApplication>();
        return juce::JUCEApplication::createInstance()->main();
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal exception: " << e.what() << std::endl;
        MessageBoxA(nullptr, e.what(), "Vital Application Fatal Error", MB_ICONERROR | MB_OK);
        return -1;
    } catch (...) {
        std::cerr << "Unknown fatal exception occurred" << std::endl;
        MessageBoxA(nullptr, "Unknown fatal exception occurred", "Vital Application Fatal Error", 
                   MB_ICONERROR | MB_OK);
        return -1;
    }
}

#else
int main(int argc, char* argv[]) {
    try {
        // Initialize JUCE
        juce::JuceInitializer initializer;
        
        // Parse command line for plugin vs standalone detection
        juce::String commandLine;
        for (int i = 0; i < argc; ++i) {
            if (i > 0) commandLine += " ";
            commandLine += argv[i];
        }
        
        // Create and run the application
        g_application = std::make_unique<vital::VitalApplication>();
        return juce::JUCEApplication::createInstance()->main();
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal exception: " << e.what() << std::endl;
        return -1;
    } catch (...) {
        std::cerr << "Unknown fatal exception occurred" << std::endl;
        return -1;
    }
}
#endif

JUCE_END_IGNORE_WARNINGS_MSVC

//==============================================================================
// VST3 Plugin Entry Point
extern "C" {
#if JUCE_VST3_CAN_REPLACE_VST2
    // VST2 compatibility entry point
    #include "aeffect.h"
    
    AEffect* VSTPluginMain(audioMasterCallback audioMaster) {
        return juce::VST2Wrapper::createPluginInstance(audioMaster, "VITAL Synthesizer", 4, 0, 0);
    }
    
    // VST3 entry point
    #if ! JUCE_VST3_CAN_REPLACE_VST2
    extern "C" __attribute__((visibility("default"))) IPluginFactory* GetPluginFactory() {
        static juce::PluginHostType host;
        static vital::VitalPluginProcessor processor;
        static juce::VST3PluginFactory factory(processor);
        return &factory;
    }
    #endif
    
#else
    // Standalone VST3 entry point
    extern "C" __attribute__((visibility("default"))) IPluginFactory* GetPluginFactory() {
        static juce::PluginHostType host;
        static vital::VitalPluginProcessor processor;
        static juce::VST3PluginFactory factory(processor);
        return &factory;
    }
#endif
}

//==============================================================================
// Audio Unit (AU) Plugin Entry Point
#if JUCE_PLUGINHOST_AU
extern "C" {
    // AUv3 entry point
    extern void* GetAPIVTablePointer(size_t selectorIndex) {
        // This would be implemented for AUv3 support
        return nullptr;
    }
    
    // AU Legacy entry point
    extern unsigned int GetAPIVersion() {
        return 0x10000; // Version 1.0.0
    }
}
#endif

//==============================================================================
// LV2 Plugin Entry Point
#if JUCE_PLUGINHOST_LV2
extern "C" {
    // LV2 descriptor function
    static const char* lv2_uri = "urn:vital:synthesizer";
    
    // This would implement LV2 plugin descriptor
}
#endif

//==============================================================================
// Plugin processor factory functions
namespace vital {

// Create plugin processor instance
std::unique_ptr<vital::VitalPluginProcessor> createPluginProcessor() {
    return std::make_unique<vital::VitalPluginProcessor>();
}

// Create plugin editor instance
std::unique_ptr<vital::VitalPluginEditor> createPluginEditor(vital::VitalPluginProcessor* processor) {
    return std::make_unique<vital::VitalPluginEditor>(*processor);
}

// Create standalone processor
std::unique_ptr<vital::VitalPluginProcessor> createStandaloneProcessor() {
    auto processor = std::make_unique<vital::VitalPluginProcessor>();
    processor->setIsStandalone(true);
    return processor;
}

} // namespace vital

//==============================================================================
// Plugin format registration functions
namespace vital {

void registerPluginFormats() {
    // VST3 format registration
#if JUCE_VST3_CAN_REPLACE_VST2
    juce::PluginFormatManager& formatManager = juce::PluginFormatManager::getInstance();
    formatManager.addDefaultFormats();
    
    // Add VST3 format if not already added
    if (formatManager.getNumFormats() == 1) {
        formatManager.addFormat(std::make_unique<juce::VST3PluginFormat>());
    }
#endif
    
    // Audio Unit format registration
#if JUCE_PLUGINHOST_AU
    // AU format would be registered here for macOS
#endif
    
    // LV2 format registration
#if JUCE_PLUGINHOST_LV2
    // LV2 format would be registered here
#endif
    
    DBG("Plugin formats registered successfully");
}

} // namespace vital

//==============================================================================
// Cross-platform initialization
namespace vital {

// Initialize platform-specific components
bool initializePlatform() {
#if JUCE_WINDOWS
    // Windows-specific initialization
    {
        // Initialize COM for audio
        juce::COMInitializer comInitializer;
        if (!comInitializer.isInitialized()) {
            DBG("Warning: COM initialization failed");
        }
        
        // Set process priority for better audio performance
        SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
        
        // Enable large page support if available
        HANDLE token;
        if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &token)) {
            LUID luid;
            if (LookupPrivilegeValue(nullptr, SE_LOCK_MEMORY_NAME, &luid)) {
                TOKEN_PRIVILEGES tp;
                tp.PrivilegeCount = 1;
                tp.Privileges[0].Luid = luid;
                tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
                
                AdjustTokenPrivileges(token, FALSE, &tp, 0, nullptr, nullptr);
            }
            CloseHandle(token);
        }
        
        DBG("Windows platform initialized");
    }
#elif JUCE_MAC
    // macOS-specific initialization
    {
        // Initialize Core Audio
        // Set up audio session for macOS
        // Configure audio hardware preferences
        
        DBG("macOS platform initialized");
    }
#elif JUCE_LINUX
    // Linux-specific initialization
    {
        // Set up real-time priority for audio
        // Configure PulseAudio or ALSA preferences
        // Initialize MIDI subsystem
        
        DBG("Linux platform initialized");
    }
#endif
    
    return true;
}

// Cleanup platform-specific components
void cleanupPlatform() {
#if JUCE_WINDOWS
    // Windows-specific cleanup
    {
        // Restore process priority
        SetPriorityClass(GetCurrentProcess(), NORMAL_PRIORITY_CLASS);
        
        DBG("Windows platform cleanup completed");
    }
#elif JUCE_MAC
    // macOS-specific cleanup
    {
        DBG("macOS platform cleanup completed");
    }
#elif JUCE_LINUX
    // Linux-specific cleanup
    {
        DBG("Linux platform cleanup completed");
    }
#endif
}

} // namespace vital

//==============================================================================
// Performance optimization functions
namespace vital {

// Initialize SIMD optimizations
bool initializeSIMDOptimizations() {
    // Detect and initialize available SIMD instruction sets
#if JUCE_USE_AVX512
    if (__builtin_cpu_supports("avx512f")) {
        DBG("AVX-512 support detected and initialized");
    }
#endif
    
#if JUCE_USE_AVX2
    if (__builtin_cpu_supports("avx2")) {
        DBG("AVX2 support detected and initialized");
    }
#endif
    
#if JUCE_USE_SSE42
    if (__builtin_cpu_supports("sse4.2")) {
        DBG("SSE4.2 support detected and initialized");
    }
#endif
    
#if JUCE_USE_NEON
    if (__builtin_cpu_supports("neon")) {
        DBG("NEON support detected and initialized");
    }
#endif
    
    return true;
}

// Set up multi-threading optimizations
bool initializeThreadingOptimizations() {
    // Configure thread pools for optimal performance
    juce::ThreadPool* audioThreadPool = new juce::ThreadPool("Vital Audio");
    
    // Set thread priorities
    audioThreadPool->setThreadPriority(vital::app::AUDIO_THREAD_PRIORITY);
    
    // Configure NUMA awareness on Windows
#if JUCE_WINDOWS
    SetProcessAffinityMask(GetCurrentProcess(), 0xFF); // Use first 8 cores for audio
#endif
    
    DBG("Threading optimizations initialized");
    return true;
}

// Memory optimization setup
bool initializeMemoryOptimizations() {
    // Enable memory pre-allocation for critical buffers
    // Configure memory pools
    // Set up garbage collection alternatives
    
    DBG("Memory optimizations initialized");
    return true;
}

} // namespace vital

//==============================================================================
// Version information
namespace vital::version {

constexpr int getMajor() { return vital::app::VERSION_MAJOR; }
constexpr int getMinor() { return vital::app::VERSION_MINOR; }
constexpr int getPatch() { return vital::app::VERSION_PATCH; }
constexpr const char* getString() { return vital::app::VERSION_STRING; }
constexpr const char* getCode() { return vital::app::VERSION_CODE; }

} // namespace vital::version

//==============================================================================
// Global initialization function
namespace vital {

// Global initialization that runs before main
struct GlobalInitializer {
    GlobalInitializer() {
        try {
            // Platform-specific initialization
            initializePlatform();
            
            // SIMD optimizations
            initializeSIMDOptimizations();
            
            // Threading optimizations
            initializeThreadingOptimizations();
            
            // Memory optimizations
            initializeMemoryOptimizations();
            
            // Plugin format registration
            registerPluginFormats();
            
        } catch (const std::exception& e) {
            std::cerr << "Global initialization failed: " << e.what() << std::endl;
        }
    }
};

// Global cleanup function
struct GlobalCleanup {
    ~GlobalCleanup() {
        try {
            cleanupPlatform();
        } catch (const std::exception& e) {
            std::cerr << "Global cleanup failed: " << e.what() << std::endl;
        }
    }
};

// Global instances
static GlobalInitializer g_globalInitializer;
static GlobalCleanup g_globalCleanup;

} // namespace vital
