/*
  ==============================================================================
    audio_engine_core.h
    Copyright (c) 2025 Vital Audio Engine Team
    
    Core engine infrastructure for the VitalAudioEngine
    Provides the fundamental audio processing framework and scheduling
  ==============================================================================
*/

#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>

namespace vital {
namespace audio_engine {
namespace core {

//==============================================================================
/**
 * @class AudioEngineCore
 * @brief Core engine infrastructure providing fundamental audio processing
 * 
 * This class manages the fundamental audio processing infrastructure including:
 * - Audio buffer management and scheduling
 * - Thread synchronization and real-time safety
 * - Memory management and optimization
 * - Performance monitoring and profiling
 * - System resource management
 */
class AudioEngineCore
{
public:
    //==============================================================================
    /** Core configuration */
    struct Config
    {
        double sampleRate = 44100.0;
        int bufferSize = 512;
        int maxChannels = 2;
        bool enableRealtime = true;
        bool enableThreadSafety = true;
        size_t maxBufferMemory = 64 * 1024 * 1024; // 64MB
        int numWorkerThreads = 4;
        bool enableProfiling = false;
    };
    
    //==============================================================================
    /** Engine state */
    struct State
    {
        bool isInitialized = false;
        bool isProcessing = false;
        bool isSuspended = false;
        std::chrono::steady_clock::time_point lastUpdate;
        double actualSampleRate = 0.0;
        int actualBufferSize = 0;
        float cpuUsage = 0.0f;
        size_t memoryUsage = 0;
    };
    
    //==============================================================================
    /** Constructor */
    explicit AudioEngineCore(const Config& config);
    
    /** Destructor */
    ~AudioEngineCore();
    
    //==============================================================================
    /** Initialize the core engine */
    bool initialize();
    
    /** Shutdown and cleanup */
    void shutdown();
    
    /** Reset to initial state */
    void reset();
    
    //==============================================================================
    /** Main processing loop */
    void processBlock(int numSamples);
    
    /** Process single sample */
    float processSample(float input, int channel = 0);
    
    //==============================================================================
    /** Buffer management */
    void setInputBuffer(const juce::AudioBuffer<float>& buffer);
    void setOutputBuffer(juce::AudioBuffer<float>& buffer);
    juce::AudioBuffer<float>& getInternalBuffer() { return internalBuffer_; }
    
    /** Buffer utilities */
    void clearBuffer(juce::AudioBuffer<float>& buffer);
    void copyBuffer(const juce::AudioBuffer<float>& source, 
                    juce::AudioBuffer<float>& destination);
    void mixBuffers(const juce::AudioBuffer<float>& source, 
                    juce::AudioBuffer<float>& destination, 
                    float gain = 1.0f);
    
    //==============================================================================
    /** Thread management */
    void enableRealtimeProcessing(bool enable);
    void setPriority(juce::Thread::Priority priority);
    bool isRealtimeEnabled() const { return config_.enableRealtime; }
    
    /** Lock-free operations */
    bool tryLock() { return lock_.try_lock(); }
    void lock() { lock_.lock(); }
    void unlock() { lock_.unlock(); }
    
    //==============================================================================
    /** Memory management */
    void setMaxMemoryUsage(size_t maxBytes);
    size_t getCurrentMemoryUsage() const;
    void optimizeMemoryUsage();
    
    /** Memory pool management */
    class MemoryPool {
    public:
        void* allocate(size_t size);
        void deallocate(void* ptr);
        size_t getTotalAllocated() const;
        size_t getPeakUsage() const;
        
    private:
        struct Block {
            void* ptr = nullptr;
            size_t size = 0;
            bool inUse = false;
        };
        
        std::vector<Block> blocks_;
        std::mutex poolMutex_;
    };
    
    std::unique_ptr<MemoryPool> memoryPool_;
    
    //==============================================================================
    /** Performance monitoring */
    struct PerformanceMetrics {
        double averageCPULoad = 0.0;
        double peakCPULoad = 0.0;
        size_t peakMemoryUsage = 0;
        int totalBlocksProcessed = 0;
        double totalUptime = 0.0;
        int xruns = 0;
        double averageLatencyMs = 0.0;
    };
    
    PerformanceMetrics getPerformanceMetrics() const;
    void enableProfiling(bool enable);
    void resetMetrics();
    
    //==============================================================================
    /** Real-time safety */
    bool isRealtimeSafe() const;
    void validateRealtimeSafety();
    
    /** Real-time buffer for critical operations */
    class RealtimeBuffer {
    public:
        explicit RealtimeBuffer(size_t size);
        ~RealtimeBuffer();
        
        void resize(size_t newSize);
        void* getData() const { return data_; }
        size_t getSize() const { return size_; }
        void clear();
        
    private:
        void* data_ = nullptr;
        size_t size_ = 0;
        std::atomic<bool> isLocked_{false};
    };
    
    std::unique_ptr<RealtimeBuffer> rtBuffer_;
    
    //==============================================================================
    /** State access */
    State getState() const { return state_; }
    Config getConfig() const { return config_; }
    
    /** Status queries */
    bool isInitialized() const { return state_.isInitialized; }
    bool isProcessing() const { return state_.isProcessing; }
    
    //==============================================================================
    /** Error handling */
    struct ErrorInfo {
        bool hasError = false;
        std::string message;
        std::string component;
        std::chrono::steady_clock::time_point timestamp;
    };
    
    ErrorInfo getLastError() const;
    void clearError();
    
private:
    //==============================================================================
    /** Configuration */
    Config config_;
    
    /** State */
    State state_;
    
    /** Internal audio buffer */
    juce::AudioBuffer<float> internalBuffer_;
    
    /** Thread synchronization */
    std::mutex lock_;
    std::atomic<bool> shutdownRequested_{false};
    
    /** Performance tracking */
    mutable std::mutex metricsMutex_;
    PerformanceMetrics metrics_;
    std::chrono::steady_clock::time_point startTime_;
    std::chrono::steady_clock::time_point lastProcessTime_;
    
    /** Error tracking */
    ErrorInfo lastError_;
    std::atomic<bool> hasError_{false};
    
    //==============================================================================
    /** Internal methods */
    void updateMetrics(double processingTimeMs);
    void validateConfiguration();
    void setupAudioDeviceManager();
    void cleanupResources();
    
    /** Buffer utilities */
    void ensureBufferSize(int numChannels, int numSamples);
    void validateBuffer(const juce::AudioBuffer<float>& buffer);
    
    /** Thread utilities */
    void setThreadName(const std::string& name);
    bool checkRealtimeConstraints(int numSamples);
    
    /** Memory utilities */
    void trackMemoryAllocation(size_t bytes);
    void trackMemoryDeallocation(size_t bytes);
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioEngineCore)
};

} // namespace core
} // namespace audio_engine
} // namespace vital
