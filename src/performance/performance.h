/**
 * @file performance.h
 * @brief Unified Performance Optimization Framework for Vital Audio Synthesizer
 * @author Vital Development Team
 * @date 2025-11-03
 * 
 * This module provides a comprehensive performance optimization framework that integrates
 * SIMD vectorization, multithreading, cache optimization, branchless programming, and
 * real-time optimization techniques for high-performance audio processing.
 */

#pragma once

#include "simd_vectorization.h"
#include "multithreading.h"
#include "cache_optimization.h"
#include "branchless_programming.h"
#include "real_time_optimization.h"

namespace vital {
namespace performance {

// ============================================================================
// Performance Configuration and Factory
// ============================================================================

/**
 * Global performance configuration
 */
struct PerformanceConfig {
    bool enable_simd_optimization = true;
    bool enable_multithreading = true;
    bool enable_cache_optimization = true;
    bool enable_branchless_operations = true;
    bool enable_realtime_optimization = true;
    
    // SIMD configuration
    int preferred_simd_width = simd::SIMDArchitecture::max_width;
    bool force_simd_architecture = false;
    
    // Multithreading configuration
    size_t num_threads = std::thread::hardware_concurrency();
    bool use_work_stealing = true;
    bool enable_cpu_affinity = true;
    
    // Cache optimization configuration
    size_t cache_line_size = 64;
    bool enable_numa_awareness = true;
    bool enable_hardware_prefetch = true;
    
    // Real-time optimization configuration
    realtime::RealTimeConfig realtime_config;
    double target_performance_level = 1.0;
    bool adaptive_quality_control = true;
};

/**
 * Performance optimization factory for creating optimized components
 */
class PerformanceFactory {
public:
    PerformanceFactory(const PerformanceConfig& config = PerformanceConfig{}) 
        : config_(config), initialized_(false) {
        initialize();
    }
    
    /**
     * Initialize the performance optimization system
     */
    bool initialize() {
        if (initialized_) return true;
        
        try {
            // Initialize cache optimization system
            cache_optimization::CacheOptimizationManager::get_instance().initialize();
            
            // Detect optimal configuration based on hardware
            detect_optimal_configuration();
            
            initialized_ = true;
            return true;
        } catch (const std::exception&) {
            return false;
        }
    }
    
    /**
     * Create optimized audio buffer processor
     */
    std::unique_ptr<simd::AudioBufferProcessor> create_audio_processor() {
        if (!initialized_) initialize();
        return std::make_unique<simd::AudioBufferProcessor>();
    }
    
    /**
     * Create optimized thread pool
     */
    std::unique_ptr<threading::ThreadPool> create_thread_pool() {
        if (!initialized_) initialize();
        
        size_t thread_count = config_.enable_multithreading ? config_.num_threads : 1;
        return std::make_unique<threading::ThreadPool>(thread_count);
    }
    
    /**
     * Create work stealing scheduler
     */
    std::unique_ptr<threading::WorkStealingScheduler> create_work_scheduler() {
        if (!initialized_) initialize();
        
        size_t thread_count = config_.enable_multithreading ? config_.num_threads : 1;
        return std::make_unique<threading::WorkStealingScheduler>(thread_count);
    }
    
    /**
     * Create real-time optimization system
     */
    std::unique_ptr<realtime::RealTimeOptimizationSystem> create_realtime_system() {
        if (!initialized_) initialize();
        return std::make_unique<realtime::RealTimeOptimizationSystem>(config_.realtime_config);
    }
    
    /**
     * Create cache-optimized audio buffer
     */
    template<size_t NumChannels, size_t BufferSize>
    std::unique_ptr<cache::CacheOptimizedAudioBuffer<NumChannels, BufferSize>> 
    create_cache_optimized_buffer() {
        if (!initialized_) initialize();
        return std::make_unique<cache::CacheOptimizedAudioBuffer<NumChannels, BufferSize>>();
    }
    
    /**
     * Create branchless audio processor
     */
    std::unique_ptr<branchless::BranchlessSIMDProcessor> create_branchless_processor() {
        if (!initialized_) initialize();
        return std::make_unique<branchless::BranchlessSIMDProcessor>();
    }
    
    /**
     * Get NUMA-aware memory manager
     */
    cache::NUMAMemoryManager& get_numa_manager() {
        return cache_optimization::CacheOptimizationManager::get_instance().get_numa_manager();
    }
    
    /**
     * Get hardware prefetcher
     */
    cache::HardwarePrefetcher& get_hardware_prefetcher() {
        return cache_optimization::CacheOptimizationManager::get_instance().get_prefetcher();
    }
    
    /**
     * Get performance monitor
     */
    realtime::PerformanceMonitor& get_performance_monitor(
        realtime::RealTimeOptimizationSystem& realtime_system) {
        return realtime_system.get_performance_monitor();
    }
    
    /**
     * Get current configuration
     */
    const PerformanceConfig& get_config() const {
        return config_;
    }
    
    /**
     * Update configuration
     */
    void update_config(const PerformanceConfig& config) {
        config_ = config;
        initialize();
    }
    
    /**
     * Get system capabilities
     */
    struct SystemCapabilities {
        bool has_sse42 = simd::SIMDArchitecture::has_sse42;
        bool has_avx2 = simd::SIMDArchitecture::has_avx2;
        bool has_avx512 = simd::SIMDArchitecture::has_avx512f;
        bool has_neon = simd::SIMDArchitecture::has_neon;
        bool has_multiple_cores = std::thread::hardware_concurrency() > 1;
        bool numa_available = false; // Would be detected in initialize()
        size_t optimal_simd_width = simd::SIMDArchitecture::max_width;
        size_t recommended_thread_count = std::thread::hardware_concurrency();
    };
    
    SystemCapabilities get_capabilities() const {
        SystemCapabilities caps;
        caps.numa_available = cache_optimization::NUMAMemoryManager().get_topology().numa_available;
        return caps;
    }
    
private:
    PerformanceConfig config_;
    bool initialized_;
    
    void detect_optimal_configuration() {
        // Detect SIMD capabilities
        if (config_.enable_simd_optimization) {
            if (simd::SIMDArchitecture::has_avx512f && config_.preferred_simd_width > 16) {
                config_.preferred_simd_width = 16;
            } else if (simd::SIMDArchitecture::has_avx2 && config_.preferred_simd_width > 8) {
                config_.preferred_simd_width = 8;
            } else if (simd::SIMDArchitecture::has_sse42 && config_.preferred_simd_width > 4) {
                config_.preferred_simd_width = 4;
            }
        }
        
        // Detect optimal thread count
        if (config_.enable_multithreading) {
            size_t hw_threads = std::thread::hardware_concurrency();
            config_.num_threads = std::max<size_t>(1, hw_threads - 1); // Reserve one core for system
        }
        
        // Update real-time configuration
        config_.realtime_config.buffer_size = config_.realtime_config.buffer_size;
        config_.realtime_config.max_cpu_usage_percent = config_.target_performance_level * 100.0;
    }
};

// ============================================================================
// Performance Integration Examples
// ============================================================================

/**
 * Example of integrated performance optimization for audio processing
 */
class OptimizedAudioProcessor {
public:
    OptimizedAudioProcessor(const PerformanceConfig& config = PerformanceConfig{}) 
        : factory_(config), realtime_system_(factory_.create_realtime_system()) {
        // Initialize components
        audio_processor_ = factory_.create_audio_processor();
        thread_pool_ = factory_.create_thread_pool();
        cache_buffer_ = factory_.create_cache_optimized_buffer<2, 512>();
        branchless_processor_ = factory_.create_branchless_processor();
        
        // Initialize real-time system
        realtime_system_->initialize();
    }
    
    /**
     * Process audio with full optimization
     */
    void process_audio_block(const float* input, float* output, size_t num_samples,
                           size_t num_channels = 2) {
        // Start performance monitoring
        VITAL_RT_SCOPED_TIMER("AudioProcessing");
        
        // Process with SIMD optimization
        if (num_channels <= 2) {
            audio_processor_->process_audio_buffer_simd(input, output, num_samples, num_channels);
        } else {
            // Fallback to standard processing
            std::copy_n(input, num_samples * num_channels, output);
        }
        
        // Apply branchless processing
        branchless_processor_->process_audio_branchless(output, output, num_samples, parameters_);
        
        // Update real-time system
        realtime_system_->process_audio_optimized(input, output, num_samples, 1);
    }
    
    /**
     * Set processing parameters
     */
    void set_parameters(const std::array<float, 3>& params) {
        parameters_ = params;
    }
    
    /**
     * Get performance statistics
     */
    std::string get_performance_report() const {
        return realtime_system_->generate_status_report();
    }
    
    /**
     * Check if system is optimized for real-time
     */
    bool is_real_time_safe() const {
        return realtime_system_->is_real_time_safe();
    }
    
private:
    PerformanceFactory factory_;
    std::unique_ptr<realtime::RealTimeOptimizationSystem> realtime_system_;
    std::unique_ptr<simd::AudioBufferProcessor> audio_processor_;
    std::unique_ptr<threading::ThreadPool> thread_pool_;
    std::unique_ptr<cache::CacheOptimizedAudioBuffer<2, 512>> cache_buffer_;
    std::unique_ptr<branchless::BranchlessSIMDProcessor> branchless_processor_;
    std::array<float, 3> parameters_{0.8f, 1000.0f, 0.7f};
};

/**
 * Example of voice processing with parallel optimization
 */
class OptimizedVoiceProcessor {
public:
    OptimizedVoiceProcessor(size_t max_voices = 32)
        : max_voices_(max_voices), parallel_processor_(max_voices) {
        // Set up voice buffers
        voice_buffers_.reserve(max_voices);
        for (size_t i = 0; i < max_voices; ++i) {
            voice_buffers_.emplace_back();
        }
    }
    
    /**
     * Process multiple voices in parallel
     */
    template<typename VoiceProcessor>
    void process_voices_parallel(const std::vector<float*>& input_buffers,
                               std::vector<float*>& output_buffers,
                               size_t num_samples,
                               VoiceProcessor&& processor) {
        parallel_processor_.process_voices_parallel(input_buffers, output_buffers, 
                                                  num_samples, std::forward<VoiceProcessor>(processor));
    }
    
    /**
     * Set active voice count
     */
    void set_voice_count(size_t count) {
        parallel_processor_.set_active_voice_count(count);
    }
    
private:
    size_t max_voices_;
    std::vector<cache::CacheOptimizedAudioBuffer<1, 256>> voice_buffers_;
    threading::ParallelDSPProcessor parallel_processor_;
};

// ============================================================================
// Performance Profiling and Analysis
// ============================================================================

/**
 * Performance profiler for comprehensive analysis
 */
class PerformanceProfiler {
public:
    struct ProfilingResult {
        struct SIMDMetrics {
            size_t vector_operations = 0;
            size_t scalar_operations = 0;
            double vectorization_efficiency = 0.0;
            double throughput_msamples_sec = 0.0;
        } simd_metrics;
        
        struct ThreadingMetrics {
            size_t tasks_processed = 0;
            size_t tasks_stolen = 0;
            size_t load_balance_efficiency = 0.0;
            double parallel_speedup = 0.0;
        } threading_metrics;
        
        struct CacheMetrics {
            size_t cache_hits = 0;
            size_t cache_misses = 0;
            double cache_hit_ratio = 0.0;
            size_t prefetch_hits = 0;
        } cache_metrics;
        
        struct BranchlessMetrics {
            size_t branches_eliminated = 0;
            size_t simd_operations_used = 0;
            double branchless_efficiency = 0.0;
        } branchless_metrics;
        
        struct RealTimeMetrics {
            double average_latency_ms = 0.0;
            double max_latency_ms = 0.0;
            size_t overruns = 0;
            double cpu_utilization_percent = 0.0;
        } realtime_metrics;
        
        double overall_performance_score = 0.0;
    };
    
    PerformanceProfiler() = default;
    
    /**
     * Start profiling session
     */
    void start_profiling() {
        session_start_ = std::chrono::high_resolution_clock::now();
        current_result_ = ProfilingResult{};
    }
    
    /**
     * Record SIMD operation
     */
    void record_simd_operation(bool is_vectorized, double throughput) {
        current_result_.simd_metrics.vector_operations += is_vectorized ? 1 : 0;
        current_result_.simd_metrics.scalar_operations += is_vectorized ? 0 : 1;
        current_result_.simd_metrics.throughput_msamples_sec = throughput;
        
        size_t total_ops = current_result_.simd_metrics.vector_operations + 
                          current_result_.simd_metrics.scalar_operations;
        current_result_.simd_metrics.vectorization_efficiency = 
            total_ops > 0 ? static_cast<double>(current_result_.simd_metrics.vector_operations) / total_ops : 0.0;
    }
    
    /**
     * Record threading metrics
     */
    void record_threading_metrics(size_t tasks, size_t stolen, double speedup) {
        current_result_.threading_metrics.tasks_processed = tasks;
        current_result_.threading_metrics.tasks_stolen = stolen;
        current_result_.threading_metrics.parallel_speedup = speedup;
        
        if (tasks > 0) {
            current_result_.threading_metrics.load_balance_efficiency = 
                static_cast<double>(tasks - stolen) / tasks * 100.0;
        }
    }
    
    /**
     * Record cache metrics
     */
    void record_cache_metrics(size_t hits, size_t misses, size_t prefetch_hits) {
        current_result_.cache_metrics.cache_hits = hits;
        current_result_.cache_metrics.cache_misses = misses;
        current_result_.cache_metrics.prefetch_hits = prefetch_hits;
        
        size_t total_accesses = hits + misses;
        current_result_.cache_metrics.cache_hit_ratio = 
            total_accesses > 0 ? static_cast<double>(hits) / total_accesses : 0.0;
    }
    
    /**
     * Record branchless metrics
     */
    void record_branchless_metrics(size_t branches_eliminated, size_t simd_ops) {
        current_result_.branchless_metrics.branches_eliminated = branches_eliminated;
        current_result_.branchless_metrics.simd_operations_used = simd_ops;
        
        // This would need more sophisticated calculation based on total possible branches
        current_result_.branchless_metrics.branchless_efficiency = 0.85; // Placeholder
    }
    
    /**
     * Record real-time metrics
     */
    void record_realtime_metrics(double avg_latency, double max_latency, 
                               size_t overruns, double cpu_utilization) {
        current_result_.realtime_metrics.average_latency_ms = avg_latency;
        current_result_.realtime_metrics.max_latency_ms = max_latency;
        current_result_.realtime_metrics.overruns = overruns;
        current_result_.realtime_metrics.cpu_utilization_percent = cpu_utilization;
    }
    
    /**
     * Generate comprehensive performance report
     */
    ProfilingResult end_profiling() {
        // Calculate overall performance score
        double simd_score = current_result_.simd_metrics.vectorization_efficiency * 100.0;
        double threading_score = std::min(current_result_.threading_metrics.parallel_speedup * 20.0, 100.0);
        double cache_score = current_result_.cache_metrics.cache_hit_ratio * 100.0;
        double branchless_score = current_result_.branchless_metrics.branchless_efficiency * 100.0;
        double realtime_score = std::max(0.0, 100.0 - current_result_.realtime_metrics.overruns * 10.0);
        
        current_result_.overall_performance_score = 
            (simd_score + threading_score + cache_score + branchless_score + realtime_score) / 5.0;
        
        return current_result_;
    }
    
    /**
     * Print performance report
     */
    void print_performance_report(const ProfilingResult& result) {
        std::cout << "=== Performance Profiling Report ===\n\n";
        
        std::cout << "SIMD Optimization:\n";
        std::cout << "  Vector Operations: " << result.simd_metrics.vector_operations << "\n";
        std::cout << "  Vectorization Efficiency: " << result.simd_metrics.vectorization_efficiency * 100.0 << "%\n";
        std::cout << "  Throughput: " << result.simd_metrics.throughput_msamples_sec << " MSamples/s\n\n";
        
        std::cout << "Multithreading:\n";
        std::cout << "  Tasks Processed: " << result.threading_metrics.tasks_processed << "\n";
        std::cout << "  Tasks Stolen: " << result.threading_metrics.tasks_stolen << "\n";
        std::cout << "  Parallel Speedup: " << result.threading_metrics.parallel_speedup << "x\n\n";
        
        std::cout << "Cache Optimization:\n";
        std::cout << "  Cache Hit Ratio: " << result.cache_metrics.cache_hit_ratio * 100.0 << "%\n";
        std::cout << "  Prefetch Hits: " << result.cache_metrics.prefetch_hits << "\n\n";
        
        std::cout << "Branchless Programming:\n";
        std::cout << "  Branches Eliminated: " << result.branchless_metrics.branches_eliminated << "\n";
        std::cout << "  Efficiency: " << result.branchless_metrics.branchless_efficiency * 100.0 << "%\n\n";
        
        std::cout << "Real-Time Performance:\n";
        std::cout << "  Average Latency: " << result.realtime_metrics.average_latency_ms << " ms\n";
        std::cout << "  Max Latency: " << result.realtime_metrics.max_latency_ms << " ms\n";
        std::cout << "  CPU Utilization: " << result.realtime_metrics.cpu_utilization_percent << "%\n";
        std::cout << "  Overruns: " << result.realtime_metrics.overruns << "\n\n";
        
        std::cout << "Overall Performance Score: " << result.overall_performance_score << "/100\n";
    }
    
private:
    std::chrono::high_resolution_clock::time_point session_start_;
    ProfilingResult current_result_;
};

// ============================================================================
// Utility Functions and Macros
// ============================================================================

/**
 * Initialize performance optimization system
 */
inline bool initialize_performance_system(const PerformanceConfig& config = PerformanceConfig{}) {
    static PerformanceFactory factory(config);
    return factory.initialize();
}

/**
 * Get global performance factory instance
 */
inline PerformanceFactory& get_performance_factory() {
    static PerformanceFactory factory;
    return factory;
}

/**
 * Performance-aware audio buffer allocation
 */
template<size_t NumChannels, size_t BufferSize>
inline auto create_optimized_audio_buffer() {
    return get_performance_factory().create_cache_optimized_buffer<NumChannels, BufferSize>();
}

/**
 * Performance-aware thread pool creation
 */
inline auto create_optimized_thread_pool() {
    return get_performance_factory().create_thread_pool();
}

/**
 * Performance-aware work scheduler creation
 */
inline auto create_optimized_work_scheduler() {
    return get_performance_factory().create_work_scheduler();
}

// Performance profiling macros
#define VITAL_PERF_PROFILE_SIMD(is_vectorized, throughput) \
    get_performance_factory().get_profiler().record_simd_operation(is_vectorized, throughput)

#define VITAL_PERF_PROFILE_THREADING(tasks, stolen, speedup) \
    get_performance_factory().get_profiler().record_threading_metrics(tasks, stolen, speedup)

#define VITAL_PERF_PROFILE_CACHE(hits, misses, prefetch_hits) \
    get_performance_factory().get_profiler().record_cache_metrics(hits, misses, prefetch_hits)

#define VITAL_PERF_PROFILE_BRANCHLESS(branches_eliminated, simd_ops) \
    get_performance_factory().get_profiler().record_branchless_metrics(branches_eliminated, simd_ops)

#define VITAL_PERF_PROFILE_REALTIME(avg_latency, max_latency, overruns, cpu_utilization) \
    get_performance_factory().get_profiler().record_realtime_metrics(avg_latency, max_latency, overruns, cpu_utilization)

} // namespace performance
} // namespace vital
