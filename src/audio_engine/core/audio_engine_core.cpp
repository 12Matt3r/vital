/*
  ==============================================================================
    audio_engine_core.cpp
    Copyright (c) 2025 Vital Audio Engine Team
    
    Implementation of the core engine infrastructure
  ==============================================================================
*/

#include "audio_engine_core.h"

namespace vital {
namespace audio_engine {
namespace core {

//==============================================================================
// AudioEngineCore Implementation
//==============================================================================

AudioEngineCore::AudioEngineCore(const Config& config)
    : config_(config)
    , state_()
    , internalBuffer_(config.maxChannels, config.bufferSize)
    , memoryPool_(std::make_unique<MemoryPool>())
    , rtBuffer_(std::make_unique<RealtimeBuffer>(config.bufferSize * sizeof(float)))
    , startTime_(std::chrono::steady_clock::now())
{
    jassert(config.sampleRate > 0.0);
    jassert(config.bufferSize > 0);
    jassert(config.maxChannels > 0);
}

AudioEngineCore::~AudioEngineCore()
{
    shutdown();
}

//==============================================================================
// Initialization and Shutdown
//==============================================================================

bool AudioEngineCore::initialize()
{
    if (state_.isInitialized) {
        return true;
    }
    
    try {
        // Validate configuration
        validateConfiguration();
        
        // Setup internal buffer
        ensureBufferSize(config_.maxChannels, config_.bufferSize);
        
        // Initialize memory pool
        memoryPool_ = std::make_unique<MemoryPool>();
        
        // Initialize real-time buffer
        rtBuffer_ = std::make_unique<RealtimeBuffer>(config_.bufferSize * sizeof(float));
        
        // Setup audio device manager if needed
        setupAudioDeviceManager();
        
        // Initialize state
        state_.isInitialized = true;
        state_.isProcessing = true;
        state_.actualSampleRate = config_.sampleRate;
        state_.actualBufferSize = config_.bufferSize;
        state_.lastUpdate = std::chrono::steady_clock::now();
        
        return true;
        
    } catch (const std::exception& e) {
        lastError_ = ErrorInfo{
            true,
            std::string("Initialization failed: ") + e.what(),
            "AudioEngineCore",
            std::chrono::steady_clock::now()
        };
        hasError_ = true;
        return false;
    }
}

void AudioEngineCore::shutdown()
{
    if (!state_.isInitialized && !shutdownRequested_) {
        return;
    }
    
    shutdownRequested_ = true;
    state_.isProcessing = false;
    state_.isInitialized = false;
    
    cleanupResources();
    
    state_ = State{};
    lastError_ = ErrorInfo{};
    hasError_ = false;
}

void AudioEngineCore::reset()
{
    if (!state_.isInitialized) return;
    
    // Clear internal buffer
    clearBuffer(internalBuffer_);
    
    // Reset memory pool
    if (memoryPool_) {
        // Reset memory pool (would need implementation)
    }
    
    // Reset metrics
    resetMetrics();
    
    // Reset state
    state_.isProcessing = true;
    state_.lastUpdate = std::chrono::steady_clock::now();
}

//==============================================================================
// Audio Processing
//==============================================================================

void AudioEngineCore::processBlock(int numSamples)
{
    if (!state_.isInitialized || !state_.isProcessing || state_.isSuspended) {
        return;
    }
    
    const auto startTime = std::chrono::steady_clock::now();
    
    try {
        // Ensure buffer size is adequate
        ensureBufferSize(config_.maxChannels, numSamples);
        
        // Validate realtime constraints
        if (config_.enableRealtime && !checkRealtimeConstraints(numSamples)) {
            // Handle realtime violation
            return;
        }
        
        // Process the block (simplified - actual implementation would be more complex)
        // This would involve calling into synthesis engines, effects, etc.
        
        // Update metrics
        const auto endTime = std::chrono::steady_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime);
        updateMetrics(duration.count() / 1000.0); // Convert to milliseconds
        
    } catch (const std::exception& e) {
        lastError_ = ErrorInfo{
            true,
            std::string("Processing error: ") + e.what(),
            "AudioEngineCore",
            std::chrono::steady_clock::now()
        };
        hasError_ = true;
    }
}

float AudioEngineCore::processSample(float input, int channel)
{
    if (!state_.isInitialized || !state_.isProcessing || state_.isSuspended) {
        return input;
    }
    
    // Single-sample processing would go here
    return input;
}

//==============================================================================
// Buffer Management
//==============================================================================

void AudioEngineCore::setInputBuffer(const juce::AudioBuffer<float>& buffer)
{
    validateBuffer(buffer);
    // Copy input buffer to internal buffer or process directly
}

void AudioEngineCore::setOutputBuffer(juce::AudioBuffer<float>& buffer)
{
    validateBuffer(buffer);
    // Copy internal buffer to output buffer
}

void AudioEngineCore::clearBuffer(juce::AudioBuffer<float>& buffer)
{
    if (buffer.getNumChannels() > 0 && buffer.getNumSamples() > 0) {
        buffer.clear();
    }
}

void AudioEngineCore::copyBuffer(const juce::AudioBuffer<float>& source,
                                juce::AudioBuffer<float>& destination)
{
    if (source.getNumChannels() == destination.getNumChannels() &&
        source.getNumSamples() == destination.getNumSamples()) {
        destination.copyFrom(0, 0, source);
    }
}

void AudioEngineCore::mixBuffers(const juce::AudioBuffer<float>& source,
                                juce::AudioBuffer<float>& destination,
                                float gain)
{
    if (source.getNumChannels() == destination.getNumChannels() &&
        source.getNumSamples() == destination.getNumSamples()) {
        for (int channel = 0; channel < source.getNumChannels(); ++channel) {
            const float* sourceData = source.getReadPointer(channel);
            float* destData = destination.getWritePointer(channel);
            
            for (int sample = 0; sample < source.getNumSamples(); ++sample) {
                destData[sample] += sourceData[sample] * gain;
            }
        }
    }
}

//==============================================================================
// Thread Management
//==============================================================================

void AudioEngineCore::enableRealtimeProcessing(bool enable)
{
    config_.enableRealtime = enable;
}

void AudioEngineCore::setPriority(juce::Thread::Priority priority)
{
    // Set thread priority for realtime audio processing
    // Implementation would use platform-specific APIs
}

bool AudioEngineCore::isRealtimeSafe() const
{
    // Check if current operation is safe for realtime execution
    // This would involve checking for dynamic memory allocation,
    // file I/O, and other non-realtime-safe operations
    return true;
}

void AudioEngineCore::validateRealtimeSafety()
{
    // Validate that all operations are realtime-safe
    // This would involve checking the call stack for prohibited operations
}

//==============================================================================
// Memory Management
//==============================================================================

void AudioEngineCore::setMaxMemoryUsage(size_t maxBytes)
{
    config_.maxBufferMemory = maxBytes;
}

size_t AudioEngineCore::getCurrentMemoryUsage() const
{
    return memoryPool_ ? memoryPool_->getTotalAllocated() : 0;
}

void AudioEngineCore::optimizeMemoryUsage()
{
    // Optimize memory usage by freeing unused blocks
    // and potentially reducing buffer sizes
}

//==============================================================================
// Performance Monitoring
//==============================================================================

AudioEngineCore::PerformanceMetrics AudioEngineCore::getPerformanceMetrics() const
{
    std::lock_guard<std::mutex> lock(metricsMutex_);
    return metrics_;
}

void AudioEngineCore::enableProfiling(bool enable)
{
    config_.enableProfiling = enable;
}

void AudioEngineCore::resetMetrics()
{
    std::lock_guard<std::mutex> lock(metricsMutex_);
    
    metrics_ = PerformanceMetrics{};
    metrics_.totalUptime = std::chrono::duration<double>(
        std::chrono::steady_clock::now() - startTime_
    ).count();
}

//==============================================================================
// Error Handling
//==============================================================================

AudioEngineCore::ErrorInfo AudioEngineCore::getLastError() const
{
    return lastError_;
}

void AudioEngineCore::clearError()
{
    lastError_ = ErrorInfo{};
    hasError_ = false;
}

//==============================================================================
// Internal Implementation Methods
//==============================================================================

void AudioEngineCore::validateConfiguration()
{
    if (config_.sampleRate <= 0.0 || config_.sampleRate > 192000.0) {
        throw std::invalid_argument("Invalid sample rate");
    }
    
    if (config_.bufferSize <= 0 || config_.bufferSize > 8192) {
        throw std::invalid_argument("Invalid buffer size");
    }
    
    if (config_.maxChannels <= 0 || config_.maxChannels > 64) {
        throw std::invalid_argument("Invalid number of channels");
    }
}

void AudioEngineCore::setupAudioDeviceManager()
{
    // Setup JUCE audio device manager if needed
    // This would configure audio input/output devices
}

void AudioEngineCore::cleanupResources()
{
    // Clean up all allocated resources
    memoryPool_.reset();
    rtBuffer_.reset();
    clearBuffer(internalBuffer_);
}

void AudioEngineCore::ensureBufferSize(int numChannels, int numSamples)
{
    if (internalBuffer_.getNumChannels() < numChannels) {
        internalBuffer_.setSize(numChannels, numSamples, true, true);
    } else if (internalBuffer_.getNumSamples() < numSamples) {
        internalBuffer_.setSize(numChannels, numSamples, true, true);
    }
}

void AudioEngineCore::validateBuffer(const juce::AudioBuffer<float>& buffer)
{
    if (buffer.getNumChannels() <= 0 || buffer.getNumSamples() <= 0) {
        throw std::invalid_argument("Invalid audio buffer");
    }
}

bool AudioEngineCore::checkRealtimeConstraints(int numSamples)
{
    // Check if processing can meet realtime constraints
    // This would involve measuring worst-case processing time
    // and comparing it to the available time budget
    
    const double availableTimeMs = (numSamples / config_.sampleRate) * 1000.0;
    const double estimatedProcessingTimeMs = 0.1; // Placeholder
    
    return estimatedProcessingTimeMs <= availableTimeMs;
}

void AudioEngineCore::updateMetrics(double processingTimeMs)
{
    std::lock_guard<std::mutex> lock(metricsMutex_);
    
    // Update CPU usage
    const double bufferTimeMs = (config_.bufferSize / config_.sampleRate) * 1000.0;
    const double cpuLoad = (processingTimeMs / bufferTimeMs) * 100.0;
    
    metrics_.averageCPULoad = (metrics_.averageCPULoad * 0.9) + (cpuLoad * 0.1);
    metrics_.peakCPULoad = std::max(metrics_.peakCPULoad, cpuLoad);
    
    // Update total blocks processed
    metrics_.totalBlocksProcessed++;
    
    // Update latency
    metrics_.averageLatencyMs = (metrics_.averageLatencyMs * 0.95) + (processingTimeMs * 0.05);
}

void AudioEngineCore::trackMemoryAllocation(size_t bytes)
{
    // Track memory allocation for performance monitoring
    // This would be called when allocating memory
}

void AudioEngineCore::trackMemoryDeallocation(size_t bytes)
{
    // Track memory deallocation for performance monitoring
    // This would be called when freeing memory
}

//==============================================================================
// MemoryPool Implementation
//==============================================================================

void* AudioEngineCore::MemoryPool::allocate(size_t size)
{
    std::lock_guard<std::mutex> lock(poolMutex_);
    
    // Find a free block or create a new one
    for (auto& block : blocks_) {
        if (!block.inUse && block.size >= size) {
            block.inUse = true;
            return block.ptr;
        }
    }
    
    // Create new block
    void* ptr = malloc(size);
    if (ptr) {
        blocks_.push_back({ptr, size, true});
    }
    
    return ptr;
}

void AudioEngineCore::MemoryPool::deallocate(void* ptr)
{
    std::lock_guard<std::mutex> lock(poolMutex_);
    
    for (auto& block : blocks_) {
        if (block.ptr == ptr) {
            block.inUse = false;
            return;
        }
    }
    
    // If not found in pool, use standard free
    free(ptr);
}

size_t AudioEngineCore::MemoryPool::getTotalAllocated() const
{
    std::lock_guard<std::mutex> lock(poolMutex_);
    
    size_t total = 0;
    for (const auto& block : blocks_) {
        if (block.inUse) {
            total += block.size;
        }
    }
    return total;
}

size_t AudioEngineCore::MemoryPool::getPeakUsage() const
{
    // Return peak memory usage
    return getTotalAllocated(); // Simplified
}

//==============================================================================
// RealtimeBuffer Implementation
//==============================================================================

AudioEngineCore::RealtimeBuffer::RealtimeBuffer(size_t size)
    : data_(malloc(size))
    , size_(size)
{
    if (data_) {
        memset(data_, 0, size);
    }
}

AudioEngineCore::RealtimeBuffer::~RealtimeBuffer()
{
    if (data_) {
        free(data_);
    }
}

void AudioEngineCore::RealtimeBuffer::resize(size_t newSize)
{
    if (newSize == size_) return;
    
    void* newData = malloc(newSize);
    if (newData) {
        if (data_ && size_ > 0) {
            const size_t copySize = std::min(size_, newSize);
            memcpy(newData, data_, copySize);
        }
        
        if (data_) {
            free(data_);
        }
        
        data_ = newData;
        size_ = newSize;
    }
}

void AudioEngineCore::RealtimeBuffer::clear()
{
    if (data_) {
        memset(data_, 0, size_);
    }
}

} // namespace core
} // namespace audio_engine
} // namespace vital
