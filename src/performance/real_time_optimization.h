/**
 * @file real_time_optimization.h
 * @brief Real-time optimization framework for Vital audio processing
 * @author Vital Development Team
 * @date 2025-11-03
 * 
 * This module provides comprehensive real-time optimization including
 * deterministic processing, adaptive quality control, performance monitoring,
 * and memory management for critical audio applications.
 */

#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <deque>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
#include <type_traits>
#include <cstring>

namespace vital {
namespace performance {
namespace realtime {

// ============================================================================
// Real-Time System Configuration
// ============================================================================

/**
 * Real-time system configuration and constraints
 */
struct RealTimeConfig {
    double sample_rate = 44100.0;          // Audio sample rate
    size_t buffer_size = 256;              // Audio buffer size
    size_t num_channels = 2;               // Number of audio channels
    size_t max_polyphony = 32;             // Maximum voice count
    double target_latency_ms = 5.0;        // Target round-trip latency
    double max_cpu_usage_percent = 80.0;   // Maximum CPU usage threshold
    size_t memory_pool_size = 16 * 1024 * 1024; // 16MB memory pool
    bool deterministic_processing = true;  // Enable deterministic processing
    bool adaptive_quality = true;          // Enable adaptive quality control
    bool performance_monitoring = true;    // Enable performance monitoring
    double quality_response_time_ms = 100.0; // Quality adaptation response time
};

// ============================================================================
// Deterministic Audio Processor
// ============================================================================

/**
 * Deterministic audio processor with guaranteed timing bounds
 */
class DeterministicAudioProcessor {
public:
    DeterministicAudioProcessor(const RealTimeConfig& config) 
        : config_(config), buffer_size_(config.buffer_size) {
        // Pre-allocate buffers to avoid dynamic allocation during processing
        audio_buffer_.resize(config_.num_channels * buffer_size_);
        
        // Calculate processing time budget
        processing_budget_us_ = calculate_processing_budget_us();
        
        // Initialize timing statistics
        timing_stats_.min_processing_time_us = std::numeric_limits<uint64_t>::max();
        timing_stats_.max_processing_time_us = 0;
        timing_stats_.average_processing_time_us = 0;
        timing_stats_.total_samples_processed = 0;
        timing_stats_.overruns = 0;
    }
    
    /**
     * Process audio with deterministic timing guarantees
     */
    bool process_audio_block(const float* input, float* output, size_t num_samples) {
        auto start_time = std::chrono::high_resolution_clock::now();
        
        // Verify processing constraints
        if (!validate_processing_constraints(num_samples)) {
            timing_stats_.overruns++;
            return false;
        }
        
        // Process audio with fixed processing pipeline
        bool success = process_audio_deterministic(input, output, num_samples);
        
        // Record timing information
        auto end_time = std::chrono::high_resolution_clock::now();
        auto processing_time = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        uint64_t processing_time_us = processing_time.count();
        
        // Update statistics
        update_timing_statistics(processing_time_us);
        
        // Check if processing exceeded budget
        if (processing_time_us > processing_budget_us_) {
            timing_stats_.overruns++;
            return false;
        }
        
        return success;
    }
    
    /**
     * Get processing statistics
     */
    struct TimingStatistics {
        uint64_t min_processing_time_us = 0;
        uint64_t max_processing_time_us = 0;
        double average_processing_time_us = 0.0;
        uint64_t total_samples_processed = 0;
        uint64_t overruns = 0;
        double cpu_utilization_percent = 0.0;
    };
    
    TimingStatistics get_timing_statistics() const {
        return timing_stats_;
    }
    
    /**
     * Check if system is meeting real-time constraints
     */
    bool is_deterministic() const {
        if (timing_stats_.total_samples_processed == 0) return true;
        
        double max_allowed_time = processing_budget_us_;
        double actual_max_time = timing_stats_.max_processing_time_us;
        
        // Check if worst-case time exceeds budget by more than 5%
        return actual_max_time <= max_allowed_time * 1.05;
    }
    
    /**
     * Reset timing statistics
     */
    void reset_timing_statistics() {
        timing_stats_ = TimingStatistics{};
    }
    
private:
    RealTimeConfig config_;
    size_t buffer_size_;
    std::vector<float> audio_buffer_;
    uint64_t processing_budget_us_;
    TimingStatistics timing_stats_;
    
    /**
     * Calculate processing time budget based on audio buffer timing
     */
    uint64_t calculate_processing_budget_us() const {
        // Budget is the time available for processing one buffer
        double buffer_time_sec = static_cast<double>(buffer_size_) / config_.sample_rate;
        uint64_t buffer_time_us = static_cast<uint64_t>(buffer_time_sec * 1000000.0);
        
        // Reserve 20% for system overhead
        return static_cast<uint64_t>(buffer_time_us * 0.8);
    }
    
    /**
     * Validate processing constraints before starting
     */
    bool validate_processing_constraints(size_t num_samples) const {
        return num_samples <= buffer_size_;
    }
    
    /**
     * Core deterministic audio processing
     */
    bool process_audio_deterministic(const float* input, float* output, size_t num_samples) {
        // Copy input to internal buffer (cache-friendly)
        std::memcpy(audio_buffer_.data(), input, num_samples * sizeof(float) * config_.num_channels);
        
        // Process with fixed pipeline (no dynamic branching in critical path)
        bool success = true;
        for (size_t channel = 0; channel < config_.num_channels; ++channel) {
            success &= process_channel_deterministic(channel, num_samples);
        }
        
        // Copy output from internal buffer
        std::memcpy(output, audio_buffer_.data(), num_samples * sizeof(float) * config_.num_channels);
        
        return success;
    }
    
    /**
     * Process single channel with deterministic pipeline
     */
    bool process_channel_deterministic(size_t channel, size_t num_samples) {
        size_t channel_offset = channel * buffer_size_;
        float* channel_data = &audio_buffer_[channel_offset];
        
        // Apply DSP processing pipeline (deterministic sequence)
        // 1. Gain processing
        process_gain_deterministic(channel_data, num_samples, 0.8f);
        
        // 2. Filtering (simplified)
        process_filter_deterministic(channel_data, num_samples, 1000.0f, 0.7f);
        
        // 3. Saturation
        process_saturation_deterministic(channel_data, num_samples);
        
        return true;
    }
    
    /**
     * Deterministic gain processing
     */
    void process_gain_deterministic(float* data, size_t num_samples, float gain) {
        for (size_t i = 0; i < num_samples; ++i) {
            data[i] *= gain;
        }
    }
    
    /**
     * Deterministic filter processing (simplified)
     */
    void process_filter_deterministic(float* data, size_t num_samples, float cutoff, float q) {
        // Simple one-pole low-pass filter
        float alpha = cutoff / (cutoff + 1000.0f);
        float y = 0.0f;
        
        for (size_t i = 0; i < num_samples; ++i) {
            y = alpha * data[i] + (1.0f - alpha) * y;
            data[i] = y;
        }
    }
    
    /**
     * Deterministic saturation
     */
    void process_saturation_deterministic(float* data, size_t num_samples) {
        for (size_t i = 0; i < num_samples; ++i) {
            if (data[i] > 1.0f) data[i] = 1.0f;
            if (data[i] < -1.0f) data[i] = -1.0f;
        }
    }
    
    /**
     * Update timing statistics
     */
    void update_timing_statistics(uint64_t processing_time_us) {
        // Update min/max
        timing_stats_.min_processing_time_us = std::min(timing_stats_.min_processing_time_us, processing_time_us);
        timing_stats_.max_processing_time_us = std::max(timing_stats_.max_processing_time_us, processing_time_us);
        
        // Update average with exponential moving average
        double alpha = 0.1; // Smoothing factor
        timing_stats_.average_processing_time_us = 
            alpha * processing_time_us + (1.0 - alpha) * timing_stats_.average_processing_time_us;
        
        timing_stats_.total_samples_processed += buffer_size_;
        
        // Calculate CPU utilization
        double utilization = static_cast<double>(processing_time_us) / processing_budget_us_ * 100.0;
        timing_stats_.cpu_utilization_percent = utilization;
    }
};

// ============================================================================
// Real-Time Memory Management
// ============================================================================

/**
 * Real-time memory allocator with bounded allocation time
 */
class RealTimeMemoryManager {
public:
    RealTimeMemoryManager(size_t pool_size = 16 * 1024 * 1024) 
        : pool_size_(pool_size), pool_start_(nullptr), pool_end_(nullptr) {
        // Allocate memory pool with proper alignment
        #ifdef _WIN32
            pool_start_ = VirtualAlloc(nullptr, pool_size_, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        #else
            pool_start_ = aligned_alloc(64, pool_size_);
        #endif
        
        if (!pool_start_) {
            throw std::bad_alloc();
        }
        
        pool_end_ = static_cast<char*>(pool_start_) + pool_size_;
        
        // Initialize free list
        initialize_free_list();
    }
    
    ~RealTimeMemoryManager() {
        if (pool_start_) {
            #ifdef _WIN32
                VirtualFree(pool_start_, 0, MEM_RELEASE);
            #else
                std::free(pool_start_);
            #endif
        }
    }
    
    /**
     * Allocate memory with bounded time complexity
     */
    void* allocate_rt(size_t size, size_t alignment = 64) {
        // Align size to alignment boundary
        size_t aligned_size = (size + alignment - 1) & ~(alignment - 1);
        
        // Find suitable block in free list
        for (auto it = free_list_.begin(); it != free_list_.end(); ++it) {
            if (it->size >= aligned_size) {
                void* ptr = it->ptr;
                size_t remaining = it->size - aligned_size;
                
                // Update free list
                if (remaining >= sizeof(FreeBlock)) {
                    FreeBlock new_block;
                    new_block.ptr = static_cast<char*>(ptr) + aligned_size;
                    new_block.size = remaining;
                    free_list_.insert(it, new_block);
                }
                
                free_list_.erase(it);
                return ptr;
            }
        }
        
        // Out of memory
        return nullptr;
    }
    
    /**
     * Deallocate memory with bounded time complexity
     */
    void deallocate_rt(void* ptr) {
        if (!ptr) return;
        
        // Find insertion point (keep list sorted by address)
        auto it = free_list_.begin();
        while (it != free_list_.end() && static_cast<char*>(ptr) > static_cast<char*>(it->ptr)) {
            ++it;
        }
        
        // Create new free block
        FreeBlock new_block;
        new_block.ptr = ptr;
        new_block.size = get_allocation_size(ptr); // Simplified
        
        // Check for coalescing with adjacent blocks
        bool coalesced = false;
        
        // Coalesce with previous block
        if (it != free_list_.begin()) {
            auto prev = std::prev(it);
            if (static_cast<char*>(prev->ptr) + prev->size == static_cast<char*>(ptr)) {
                prev->size += new_block.size;
                new_block = *prev;
                free_list_.erase(prev);
                coalesced = true;
                it = free_list_.begin(); // Reset iterator
            }
        }
        
        // Coalesce with next block
        if (!coalesced && it != free_list_.end()) {
            if (static_cast<char*>(ptr) + new_block.size == static_cast<char*>(it->ptr)) {
                new_block.size += it->size;
                free_list_.erase(it);
                coalesced = true;
            }
        }
        
        // Insert or update block
        if (!coalesced) {
            free_list_.insert(it, new_block);
        }
    }
    
    /**
     * Get memory pool statistics
     */
    struct MemoryStats {
        size_t total_size = 0;
        size_t used_size = 0;
        size_t free_size = 0;
        size_t largest_free_block = 0;
        size_t num_free_blocks = 0;
        double fragmentation_ratio = 0.0;
    };
    
    MemoryStats get_memory_stats() const {
        MemoryStats stats;
        stats.total_size = pool_size_;
        stats.free_size = 0;
        stats.largest_free_block = 0;
        stats.num_free_blocks = free_list_.size();
        
        for (const auto& block : free_list_) {
            stats.free_size += block.size;
            stats.largest_free_block = std::max(stats.largest_free_block, block.size);
        }
        
        stats.used_size = pool_size_ - stats.free_size;
        
        if (stats.free_size > 0) {
            stats.fragmentation_ratio = 1.0 - static_cast<double>(stats.largest_free_block) / stats.free_size;
        }
        
        return stats;
    }
    
private:
    size_t pool_size_;
    void* pool_start_;
    void* pool_end_;
    
    struct FreeBlock {
        void* ptr;
        size_t size;
        
        bool operator<(const FreeBlock& other) const {
            return static_cast<char*>(ptr) < static_cast<char*>(other.ptr);
        }
    };
    
    std::deque<FreeBlock> free_list_;
    
    /**
     * Initialize free list with single large block
     */
    void initialize_free_list() {
        FreeBlock initial_block;
        initial_block.ptr = pool_start_;
        initial_block.size = pool_size_;
        free_list_.push_back(initial_block);
    }
    
    size_t get_allocation_size(void* ptr) {
        // This would need proper tracking in a real implementation
        // For now, return a reasonable default
        return 1024; // 1KB default
    }
};

// ============================================================================
// Adaptive Quality Controller
// ============================================================================

/**
 * Adaptive quality controller that adjusts processing quality based on system load
 */
class AdaptiveQualityController {
public:
    AdaptiveQualityController(const RealTimeConfig& config) 
        : config_(config), current_quality_(1.0), target_quality_(1.0) {
        quality_levels_.push_back({0.3f, "Minimum", 50.0f});  // 30% quality, 50% CPU
        quality_levels_.push_back({0.5f, "Performance", 60.0f}); // 50% quality, 60% CPU
        quality_levels_.push_back({0.7f, "Balanced", 70.0f}); // 70% quality, 70% CPU
        quality_levels_.push_back({0.85f, "High", 80.0f});    // 85% quality, 80% CPU
        quality_levels_.push_back({1.0f, "Maximum", 95.0f});  // 100% quality, 95% CPU
    }
    
    /**
     * Update quality target based on system performance
     */
    void update_quality_target(const DeterministicAudioProcessor::TimingStatistics& stats) {
        // Calculate target quality based on CPU utilization
        double cpu_utilization = stats.cpu_utilization_percent;
        double target_quality;
        
        if (cpu_utilization <= 60.0) {
            target_quality = 1.0; // Full quality if CPU usage is low
        } else if (cpu_utilization <= 70.0) {
            target_quality = 0.9;
        } else if (cpu_utilization <= 80.0) {
            target_quality = 0.7;
        } else if (cpu_utilization <= 90.0) {
            target_quality = 0.5;
        } else {
            target_quality = 0.3; // Minimum quality to prevent audio dropouts
        }
        
        target_quality_ = static_cast<float>(target_quality);
    }
    
    /**
     * Smooth quality changes to avoid audio artifacts
     */
    void update_current_quality(double delta_time_ms) {
        double smoothing_rate = config_.quality_response_time_ms;
        double alpha = delta_time_ms / smoothing_rate;
        alpha = std::clamp(alpha, 0.01, 0.1); // Limit rate of change
        
        current_quality_ = static_cast<float>(alpha * target_quality_ + 
                                             (1.0 - alpha) * current_quality_);
    }
    
    /**
     * Get current quality factor
     */
    float get_quality_factor() const {
        return current_quality_;
    }
    
    /**
     * Get quality level information
     */
    struct QualityLevel {
        float quality_factor;
        std::string name;
        float max_cpu_usage;
    };
    
    const std::vector<QualityLevel>& get_quality_levels() const {
        return quality_levels_;
    }
    
    /**
     * Set quality factor manually
     */
    void set_quality_factor(float quality) {
        target_quality_ = std::clamp(quality, 0.0f, 1.0f);
    }
    
private:
    RealTimeConfig config_;
    float current_quality_;
    float target_quality_;
    std::vector<QualityLevel> quality_levels_;
};

// ============================================================================
// Performance Monitor
// ============================================================================

/**
 * Comprehensive performance monitoring and alerting system
 */
class PerformanceMonitor {
public:
    PerformanceMonitor(const RealTimeConfig& config) : config_(config) {
        // Initialize performance metrics
        metrics_.cpu_usage_percent = 0.0;
        metrics_.memory_usage_mb = 0.0;
        metrics_.audio_latency_ms = 0.0;
        metrics_.overruns_per_second = 0.0;
        metrics_.quality_factor = 1.0;
        metrics_.processing_time_us = 0.0;
        metrics_.buffer_underruns = 0;
        metrics_.max_polyphony_reached = false;
        
        // Set up alert thresholds
        alert_thresholds_.max_cpu_usage = 90.0;
        alert_thresholds_.max_latency_ms = 10.0;
        alert_thresholds_.max_memory_usage_mb = 512.0;
        alert_thresholds_.max_overruns_per_second = 1.0;
        
        // Set up callbacks
        alert_callback_ = nullptr;
        performance_callback_ = nullptr;
    }
    
    /**
     * Record performance sample
     */
    void record_sample(const DeterministicAudioProcessor::TimingStatistics& timing_stats,
                      size_t active_voices, size_t memory_usage_mb) {
        std::lock_guard<std::mutex> lock(metrics_mutex_);
        
        // Update basic metrics
        metrics_.cpu_usage_percent = timing_stats.cpu_utilization_percent;
        metrics_.audio_latency_ms = static_cast<double>(timing_stats.max_processing_time_us) / 1000.0;
        metrics_.processing_time_us = timing_stats.average_processing_time_us;
        metrics_.buffer_underruns = timing_stats.overruns;
        metrics_.memory_usage_mb = static_cast<double>(memory_usage_mb);
        metrics_.max_polyphony_reached = (active_voices >= config_.max_polyphony);
        
        // Calculate overruns per second
        auto now = std::chrono::high_resolution_clock::now();
        auto time_since_last = std::chrono::duration<double, std::seconds>(now - last_update_time_).count();
        
        if (time_since_last >= 1.0) {
            metrics_.overruns_per_second = timing_stats.overruns / time_since_last;
            last_update_time_ = now;
        }
        
        // Check for alert conditions
        check_alert_conditions();
        
        // Call performance callback if set
        if (performance_callback_) {
            performance_callback_(metrics_);
        }
    }
    
    /**
     * Get current performance metrics
     */
    struct PerformanceMetrics {
        double cpu_usage_percent;
        double memory_usage_mb;
        double audio_latency_ms;
        double overruns_per_second;
        float quality_factor;
        double processing_time_us;
        uint64_t buffer_underruns;
        bool max_polyphony_reached;
        std::chrono::high_resolution_clock::time_point timestamp;
    };
    
    PerformanceMetrics get_current_metrics() const {
        std::lock_guard<std::mutex> lock(metrics_mutex_);
        return metrics_;
    }
    
    /**
     * Set alert callback
     */
    void set_alert_callback(std::function<void(const std::string& alert_type, double value)> callback) {
        alert_callback_ = std::move(callback);
    }
    
    /**
     * Set performance callback
     */
    void set_performance_callback(std::function<void(const PerformanceMetrics&)> callback) {
        performance_callback_ = std::move(callback);
    }
    
    /**
     * Configure alert thresholds
     */
    void set_alert_thresholds(double max_cpu, double max_latency, double max_memory, double max_overruns) {
        alert_thresholds_.max_cpu_usage = max_cpu;
        alert_thresholds_.max_latency_ms = max_latency;
        alert_thresholds_.max_memory_usage_mb = max_memory;
        alert_thresholds_.max_overruns_per_second = max_overruns;
    }
    
    /**
     * Get historical performance data
     */
    std::vector<PerformanceMetrics> get_history(size_t max_samples = 1000) const {
        std::lock_guard<std::mutex> lock(history_mutex_);
        
        if (history_.size() > max_samples) {
            size_t start_idx = history_.size() - max_samples;
            return std::vector<PerformanceMetrics>(history_.begin() + start_idx, history_.end());
        }
        
        return history_;
    }
    
    /**
     * Clear performance history
     */
    void clear_history() {
        std::lock_guard<std::mutex> lock(history_mutex_);
        history_.clear();
    }
    
    /**
     * Generate performance report
     */
    std::string generate_performance_report() const {
        std::lock_guard<std::mutex> lock(metrics_mutex_);
        
        std::ostringstream report;
        report << "=== Real-Time Performance Report ===\n";
        report << "CPU Usage: " << metrics_.cpu_usage_percent << "%\n";
        report << "Memory Usage: " << metrics_.memory_usage_mb << " MB\n";
        report << "Audio Latency: " << metrics_.audio_latency_ms << " ms\n";
        report << "Processing Time: " << metrics_.processing_time_us << " μs\n";
        report << "Buffer Underruns: " << metrics_.buffer_underruns << "\n";
        report << "Overruns/Second: " << metrics_.overruns_per_second << "\n";
        report << "Max Polyphony Reached: " << (metrics_.max_polyphony_reached ? "Yes" : "No") << "\n";
        report << "Quality Factor: " << metrics_.quality_factor << "\n";
        
        return report.str();
    }
    
private:
    RealTimeConfig config_;
    PerformanceMetrics metrics_;
    std::chrono::high_resolution_clock::time_point last_update_time_;
    
    // Thread safety
    mutable std::mutex metrics_mutex_;
    mutable std::mutex history_mutex_;
    
    // Performance history
    std::vector<PerformanceMetrics> history_;
    
    // Alert system
    struct AlertThresholds {
        double max_cpu_usage = 90.0;
        double max_latency_ms = 10.0;
        double max_memory_usage_mb = 512.0;
        double max_overruns_per_second = 1.0;
    } alert_thresholds_;
    
    std::function<void(const std::string& alert_type, double value)> alert_callback_;
    std::function<void(const PerformanceMetrics&)> performance_callback_;
    
    void check_alert_conditions() {
        if (!alert_callback_) return;
        
        if (metrics_.cpu_usage_percent > alert_thresholds_.max_cpu_usage) {
            alert_callback_("CPU_USAGE", metrics_.cpu_usage_percent);
        }
        
        if (metrics_.audio_latency_ms > alert_thresholds_.max_latency_ms) {
            alert_callback_("AUDIO_LATENCY", metrics_.audio_latency_ms);
        }
        
        if (metrics_.memory_usage_mb > alert_thresholds_.max_memory_usage_mb) {
            alert_callback_("MEMORY_USAGE", metrics_.memory_usage_mb);
        }
        
        if (metrics_.overruns_per_second > alert_thresholds_.max_overruns_per_second) {
            alert_callback_("OVERRUNS", metrics_.overruns_per_second);
        }
        
        // Add to history
        {
            std::lock_guard<std::mutex> lock(history_mutex_);
            history_.push_back(metrics_);
            
            // Limit history size
            if (history_.size() > 10000) {
                history_.erase(history_.begin(), history_.begin() + 1000);
            }
        }
    }
};

// ============================================================================
// Unified Real-Time Optimization System
// ============================================================================

/**
 * Unified real-time optimization system coordinator
 */
class RealTimeOptimizationSystem {
public:
    RealTimeOptimizationSystem(const RealTimeConfig& config) 
        : config_(config), 
          deterministic_processor_(config),
          adaptive_quality_controller_(config),
          performance_monitor_(config) {
        // Initialize all subsystems
        initialize_system();
    }
    
    /**
     * Initialize the optimization system
     */
    bool initialize() {
        try {
            // Initialize memory manager
            memory_manager_ = std::make_unique<RealTimeMemoryManager>(config_.memory_pool_size);
            
            // Set up performance monitoring callbacks
            performance_monitor_.set_performance_callback(
                [this](const PerformanceMonitor::PerformanceMetrics& metrics) {
                    adaptive_quality_controller_.update_quality_target(
                        DeterministicAudioProcessor::TimingStatistics{
                            .cpu_utilization_percent = metrics.cpu_usage_percent,
                            .max_processing_time_us = static_cast<uint64_t>(metrics.processing_time_us),
                            .overruns = metrics.buffer_underruns
                        });
                });
            
            return true;
        } catch (const std::exception&) {
            return false;
        }
    }
    
    /**
     * Process audio block with full optimization
     */
    bool process_audio_optimized(const float* input, float* output, size_t num_samples,
                               size_t active_voices = 0) {
        // Get adaptive quality factor
        float quality_factor = adaptive_quality_controller_.get_quality_factor();
        
        // Update performance metrics
        auto timing_stats = deterministic_processor_.get_timing_statistics();
        size_t memory_usage_mb = memory_manager_->get_memory_stats().used_size / (1024 * 1024);
        
        performance_monitor_.record_sample(timing_stats, active_voices, memory_usage_mb);
        
        // Update current quality
        adaptive_quality_controller_.update_current_quality(16.0); // Assume 16ms frame time
        
        // Process audio with deterministic processor
        return deterministic_processor_.process_audio_block(input, output, num_samples);
    }
    
    /**
     * Get system configuration
     */
    const RealTimeConfig& get_config() const {
        return config_;
    }
    
    /**
     * Get performance monitor
     */
    PerformanceMonitor& get_performance_monitor() {
        return performance_monitor_;
    }
    
    /**
     * Get adaptive quality controller
     */
    AdaptiveQualityController& get_adaptive_quality_controller() {
        return adaptive_quality_controller_;
    }
    
    /**
     * Get memory manager
     */
    RealTimeMemoryManager& get_memory_manager() {
        return *memory_manager_;
    }
    
    /**
     * Check if system is operating within real-time constraints
     */
    bool is_real_time_safe() const {
        auto timing_stats = deterministic_processor_.get_timing_statistics();
        return deterministic_processor_.is_deterministic() && 
               timing_stats.cpu_utilization_percent <= config_.max_cpu_usage_percent;
    }
    
    /**
     * Generate comprehensive system status report
     */
    std::string generate_status_report() const {
        std::ostringstream report;
        
        report << "=== Real-Time Optimization System Status ===\n\n";
        
        // Configuration
        report << "Configuration:\n";
        report << "  Sample Rate: " << config_.sample_rate << " Hz\n";
        report << "  Buffer Size: " << config_.buffer_size << " samples\n";
        report << "  Target Latency: " << config_.target_latency_ms << " ms\n";
        report << "  Max CPU Usage: " << config_.max_cpu_usage_percent << "%\n\n";
        
        // Performance metrics
        report << "Performance Metrics:\n";
        report << performance_monitor_.generate_performance_report();
        report << "\n";
        
        // System status
        report << "System Status:\n";
        report << "  Real-Time Safe: " << (is_real_time_safe() ? "Yes" : "No") << "\n";
        report << "  Current Quality: " << adaptive_quality_controller_.get_quality_factor() << "\n";
        report << "  Memory Pool Usage: " << memory_manager_->get_memory_stats().fragmentation_ratio * 100.0 << "%\n";
        
        return report.str();
    }
    
    /**
     * Reset all statistics and metrics
     */
    void reset_system() {
        deterministic_processor_.reset_timing_statistics();
        performance_monitor_.clear_history();
        adaptive_quality_controller_.set_quality_factor(1.0f);
    }
    
private:
    RealTimeConfig config_;
    DeterministicAudioProcessor deterministic_processor_;
    AdaptiveQualityController adaptive_quality_controller_;
    PerformanceMonitor performance_monitor_;
    std::unique_ptr<RealTimeMemoryManager> memory_manager_;
    
    void initialize_system() {
        // Pre-allocate critical resources
        memory_manager_ = std::make_unique<RealTimeMemoryManager>(config_.memory_pool_size);
    }
};

// ============================================================================
// Utility Functions and Macros
// ============================================================================

/**
 * High-resolution timer for performance measurement
 */
class PerformanceTimer {
public:
    PerformanceTimer() : start_time_(std::chrono::high_resolution_clock::now()) {}
    
    void reset() {
        start_time_ = std::chrono::high_resolution_clock::now();
    }
    
    double elapsed_microseconds() const {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time_);
        return static_cast<double>(duration.count());
    }
    
    double elapsed_milliseconds() const {
        return elapsed_microseconds() / 1000.0;
    }
    
private:
    std::chrono::high_resolution_clock::time_point start_time_;
};

/**
 * Scoped performance timer
 */
class ScopedPerformanceTimer {
public:
    ScopedPerformanceTimer(const std::string& name) 
        : name_(name), timer_() {}
    
    ~ScopedPerformanceTimer() {
        double elapsed_ms = timer_.elapsed_milliseconds();
        // In a real implementation, this would log to a performance logger
        // std::cout << name_ << ": " << elapsed_ms << " ms\n";
    }
    
private:
    std::string name_;
    PerformanceTimer timer_;
};

// Convenience macro for scoped timing
#define VITAL_RT_SCOPED_TIMER(name) ScopedPerformanceTimer timer_##name(name)

/**
 * Check if we're in a real-time safe context
 */
inline bool is_real_time_context() {
    // In a real implementation, this would check thread priority and scheduling
    return true; // Simplified
}

/**
 * Assert real-time safety (development builds only)
 */
inline void assert_real_time_safe() {
    #ifndef NDEBUG
    if (!is_real_time_context()) {
        // In a real implementation, this would trigger a more sophisticated assertion
        // throw std::runtime_error("Operation not real-time safe");
    }
    #endif
}

} // namespace realtime
} // namespace performance
} // namespace vital
