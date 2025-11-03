/**
 * @file performance_example.cpp
 * @brief Example demonstrating the use of Vital Performance Optimization Modules
 * @author Vital Development Team
 * @date 2025-11-03
 * 
 * This example demonstrates how to integrate and use the comprehensive
 * performance optimization framework in a real audio application.
 */

#include "performance.h"
#include <iostream>
#include <vector>
#include <chrono>
#include <thread>
#include <random>

using namespace vital::performance;
using namespace vital::performance::simd;
using namespace vital::performance::threading;
using namespace vital::performance::cache;
using namespace vital::performance::branchless;
using namespace vital::performance::realtime;

/**
 * Example 1: Basic SIMD Audio Processing
 */
void example_simd_processing() {
    std::cout << "=== Example 1: SIMD Audio Processing ===" << std::endl;
    
    // Create SIMD audio processor
    AudioBufferProcessor simd_processor;
    
    // Generate test audio data
    const size_t num_samples = 1024;
    const size_t num_channels = 2;
    std::vector<float> input(num_samples * num_channels);
    std::vector<float> output(num_samples * num_channels);
    
    // Fill with sine wave
    for (size_t i = 0; i < num_samples; ++i) {
        float t = static_cast<float>(i) / num_samples;
        float sample = std::sin(2.0f * M_PI * t * 440.0f / 44100.0f);
        input[i * num_channels] = sample;
        input[i * num_channels + 1] = sample * 0.8f; // Second channel slightly quieter
    }
    
    // Process with SIMD optimization
    auto start_time = std::chrono::high_resolution_clock::now();
    simd_processor.process_audio_buffer_simd(input.data(), output.data(), 
                                           num_samples, num_channels);
    auto end_time = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    double throughput = (num_samples * num_channels) / (duration.count() / 1000000.0);
    
    std::cout << "SIMD Processing Time: " << duration.count() << " μs" << std::endl;
    std::cout << "Throughput: " << throughput << " samples/second" << std::endl;
    std::cout << "Peak Output Sample: " << *std::max_element(output.begin(), output.end()) << std::endl;
    std::cout << std::endl;
}

/**
 * Example 2: Cache Optimization
 */
void example_cache_optimization() {
    std::cout << "=== Example 2: Cache Optimization ===" << std::endl;
    
    // Create cache-optimized audio buffer
    auto buffer = create_cache_optimized_audio_buffer<2, 512>();
    
    // Test cache-aware operations
    std::vector<float> test_data(256);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis(-1.0f, 1.0f);
    
    for (float& value : test_data) {
        value = dis(gen);
    }
    
    // Write with cache optimization
    auto start_time = std::chrono::high_resolution_clock::now();
    buffer->write_channel_cache_aware(0, test_data.data(), test_data.size());
    buffer->write_channel_cache_aware(1, test_data.data(), test_data.size());
    auto end_time = std::chrono::high_resolution_clock::now();
    
    auto write_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    // Read with cache optimization
    start_time = std::chrono::high_resolution_clock::now();
    std::vector<float> read_data(256);
    buffer->read_channel_cache_aware(0, read_data.data(), read_data.size());
    end_time = std::chrono::high_resolution_clock::now();
    
    auto read_duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    // Verify data integrity
    bool data_correct = true;
    for (size_t i = 0; i < test_data.size(); ++i) {
        if (std::abs(test_data[i] - read_data[i]) > 1e-6f) {
            data_correct = false;
            break;
        }
    }
    
    std::cout << "Cache-Optimized Write Time: " << write_duration.count() << " μs" << std::endl;
    std::cout << "Cache-Optimized Read Time: " << read_duration.count() << " μs" << std::endl;
    std::cout << "Data Integrity: " << (data_correct ? "PASS" : "FAIL") << std::endl;
    std::cout << std::endl;
}

/**
 * Example 3: Multithreaded Voice Processing
 */
void example_multithreaded_processing() {
    std::cout << "=== Example 3: Multithreaded Voice Processing ===" << std::endl;
    
    // Create voice processor with work stealing scheduler
    auto scheduler = create_optimized_work_scheduler();
    
    // Create voice buffers
    const size_t num_voices = 8;
    const size_t samples_per_voice = 256;
    std::vector<std::vector<float>> voice_inputs(num_voices, std::vector<float>(samples_per_voice));
    std::vector<std::vector<float>> voice_outputs(num_voices, std::vector<float>(samples_per_voice));
    
    // Generate test signals for each voice
    for (size_t voice = 0; voice < num_voices; ++voice) {
        float frequency = 100.0f + voice * 50.0f; // Different frequency per voice
        
        for (size_t i = 0; i < samples_per_voice; ++i) {
            float t = static_cast<float>(i) / 44100.0f;
            voice_inputs[voice][i] = std::sin(2.0f * M_PI * frequency * t);
        }
    }
    
    // Process voices in parallel using work stealing
    auto start_time = std::chrono::high_resolution_clock::now();
    
    for (size_t voice = 0; voice < num_voices; ++voice) {
        auto task = std::make_unique<VoiceProcessingTask>(
            voice_inputs[voice].data(), 
            voice_outputs[voice].data(), 
            samples_per_voice
        );
        scheduler->submit(std::move(task));
    }
    
    // Wait for completion
    scheduler->execute_until_empty();
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
    
    // Verify all voices processed
    bool all_voices_processed = true;
    for (size_t voice = 0; voice < num_voices; ++voice) {
        float max_sample = *std::max_element(voice_outputs[voice].begin(), voice_outputs[voice].end());
        if (max_sample < 0.1f) { // Should have processed signal
            all_voices_processed = false;
        }
    }
    
    auto stats = scheduler->get_statistics();
    
    std::cout << "Parallel Processing Time: " << duration.count() << " ms" << std::endl;
    std::cout << "Tasks Completed: " << stats.total_tasks_completed << std::endl;
    std::cout << "Load Balance Efficiency: " << stats.load_balance_efficiency * 100.0 << "%" << std::endl;
    std::cout << "All Voices Processed: " << (all_voices_processed ? "YES" : "NO") << std::endl;
    std::cout << std::endl;
}

/**
 * Example 4: Branchless Audio Processing
 */
void example_branchless_processing() {
    std::cout << "=== Example 4: Branchless Audio Processing ===" << std::endl;
    
    // Create branchless processor
    auto branchless_proc = create_branchless_processor();
    
    // Test data
    const size_t num_samples = 512;
    std::vector<float> input(num_samples);
    std::vector<float> output(num_samples);
    
    // Generate test signal with dynamics
    for (size_t i = 0; i < num_samples; ++i) {
        float t = static_cast<float>(i) / num_samples;
        float envelope = 0.5f + 0.5f * std::sin(2.0f * M_PI * t * 5.0f);
        input[i] = envelope * std::sin(2.0f * M_PI * t * 440.0f);
    }
    
    // Set parameters for branchless processing
    std::array<float, 3> parameters = {0.8f, 1000.0f, 0.7f};
    
    // Process with branchless optimization
    auto start_time = std::chrono::high_resolution_clock::now();
    branchless_proc->process_audio_branchless(input.data(), output.data(), 
                                            num_samples, parameters.data());
    auto end_time = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    // Test branchless utilities
    float test_value = -0.5f;
    float clamped = branchless_clamp(test_value, -1.0f, 1.0f);
    float abs_val = branchless_abs(test_value);
    float selected = branchless_select(test_value > 0.0f, 1.0f, 0.0f);
    
    std::cout << "Branchless Processing Time: " << duration.count() << " μs" << std::endl;
    std::cout << "Branchless Clamp Test: " << test_value << " -> " << clamped << std::endl;
    std::cout << "Branchless Abs Test: " << test_value << " -> " << abs_val << std::endl;
    std::cout << "Branchless Select Test: " << test_value << " -> " << selected << std::endl;
    std::cout << std::endl;
}

/**
 * Example 5: Real-time Optimization System
 */
void example_realtime_system() {
    std::cout << "=== Example 5: Real-time Optimization System ===" << std::endl;
    
    // Configure real-time system
    RealTimeConfig rt_config;
    rt_config.sample_rate = 44100.0;
    rt_config.buffer_size = 256;
    rt_config.target_latency_ms = 5.0;
    rt_config.max_cpu_usage_percent = 80.0;
    rt_config.adaptive_quality = true;
    
    // Create real-time optimization system
    RealTimeOptimizationSystem rt_system(rt_config);
    
    if (!rt_system.initialize()) {
        std::cout << "Failed to initialize real-time system" << std::endl;
        return;
    }
    
    // Generate test audio
    const size_t num_samples = rt_config.buffer_size;
    std::vector<float> input(num_samples);
    std::vector<float> output(num_samples);
    
    for (size_t i = 0; i < num_samples; ++i) {
        float t = static_cast<float>(i) / num_samples;
        input[i] = std::sin(2.0f * M_PI * t * 440.0f);
    }
    
    // Process with real-time optimization
    auto start_time = std::chrono::high_resolution_clock::now();
    
    bool success = rt_system.process_audio_optimized(input.data(), output.data(), 
                                                   num_samples, 1);
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    // Check real-time safety
    bool is_safe = rt_system.is_real_time_safe();
    
    // Get performance metrics
    auto& monitor = rt_system.get_performance_monitor();
    auto metrics = monitor.get_current_metrics();
    
    std::cout << "Real-time Processing Time: " << duration.count() << " μs" << std::endl;
    std::cout << "Processing Successful: " << (success ? "YES" : "NO") << std::endl;
    std::cout << "Real-time Safe: " << (is_safe ? "YES" : "NO") << std::endl;
    std::cout << "CPU Usage: " << metrics.cpu_usage_percent << "%" << std::endl;
    std::cout << "Audio Latency: " << metrics.audio_latency_ms << " ms" << std::endl;
    std::cout << "Buffer Underruns: " << metrics.buffer_underruns << std::endl;
    std::cout << std::endl;
    
    // Generate comprehensive status report
    std::cout << "=== System Status Report ===" << std::endl;
    std::cout << rt_system.generate_status_report();
}

/**
 * Example 6: Complete Integrated Optimization
 */
void example_integrated_optimization() {
    std::cout << "=== Example 6: Integrated Performance Optimization ===" << std::endl;
    
    // Create performance factory with optimal configuration
    PerformanceConfig config;
    config.enable_simd_optimization = true;
    config.enable_multithreading = true;
    config.enable_cache_optimization = true;
    config.enable_branchless_operations = true;
    config.enable_realtime_optimization = true;
    
    PerformanceFactory factory(config);
    if (!factory.initialize()) {
        std::cout << "Failed to initialize performance factory" << std::endl;
        return;
    }
    
    // Create integrated audio processor
    OptimizedAudioProcessor integrated_processor(config);
    
    // Test processing with different workloads
    std::vector<size_t> test_sizes = {128, 256, 512, 1024};
    
    std::cout << "Performance across different buffer sizes:" << std::endl;
    std::cout << std::setw(10) << "Buffer Size" 
              << std::setw(15) << "Time (μs)"
              << std::setw(15) << "Throughput (S/s)"
              << std::setw(12) << "Real-time"
              << std::endl;
    std::cout << std::string(52, '-') << std::endl;
    
    for (size_t buffer_size : test_sizes) {
        // Generate test signal
        std::vector<float> input(buffer_size);
        std::vector<float> output(buffer_size);
        
        for (size_t i = 0; i < buffer_size; ++i) {
            float t = static_cast<float>(i) / buffer_size;
            input[i] = std::sin(2.0f * M_PI * t * 440.0f);
        }
        
        // Process with integrated optimization
        auto start_time = std::chrono::high_resolution_clock::now();
        
        integrated_processor.process_audio_block(input.data(), output.data(), buffer_size);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        
        double throughput = buffer_size / (duration.count() / 1000000.0);
        bool real_time_safe = integrated_processor.is_real_time_safe();
        
        std::cout << std::setw(10) << buffer_size
                  << std::setw(15) << duration.count()
                  << std::setw(15) << static_cast<size_t>(throughput)
                  << std::setw(12) << (real_time_safe ? "YES" : "NO")
                  << std::endl;
    }
    
    std::cout << std::endl;
    
    // Get detailed performance report
    std::cout << "=== Detailed Performance Report ===" << std::endl;
    std::cout << integrated_processor.get_performance_report();
    
    // Get system capabilities
    auto capabilities = factory.get_capabilities();
    std::cout << "=== System Capabilities ===" << std::endl;
    std::cout << "SSE4.2: " << (capabilities.has_sse42 ? "YES" : "NO") << std::endl;
    std::cout << "AVX2: " << (capabilities.has_avx2 ? "YES" : "NO") << std::endl;
    std::cout << "AVX-512: " << (capabilities.has_avx512 ? "YES" : "NO") << std::endl;
    std::cout << "Multiple Cores: " << (capabilities.has_multiple_cores ? "YES" : "NO") << std::endl;
    std::cout << "Optimal SIMD Width: " << capabilities.optimal_simd_width << std::endl;
    std::cout << "Recommended Threads: " << capabilities.recommended_thread_count << std::endl;
}

/**
 * Main example application
 */
int main() {
    std::cout << "Vital Performance Optimization Module Examples" << std::endl;
    std::cout << "=============================================" << std::endl;
    std::cout << std::endl;
    
    try {
        // Run all examples
        example_simd_processing();
        example_cache_optimization();
        example_multithreaded_processing();
        example_branchless_processing();
        example_realtime_system();
        example_integrated_optimization();
        
        std::cout << "All examples completed successfully!" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error running examples: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

/**
 * Helper class for voice processing task
 */
class VoiceProcessingTask : public WorkStealingTask {
public:
    VoiceProcessingTask(const float* input, float* output, size_t samples)
        : input_(input), output_(output), samples_(samples) {}
    
    void execute() override {
        // Simple voice processing: low-pass filter + gain
        float cutoff_freq = 1000.0f;
        float gain = 0.8f;
        float alpha = cutoff_freq / (cutoff_freq + 44100.0f);
        
        float prev_output = 0.0f;
        
        for (size_t i = 0; i < samples_; ++i) {
            float input_sample = input_[i];
            
            // One-pole low-pass filter
            prev_output = alpha * input_sample + (1.0f - alpha) * prev_output;
            
            // Apply gain
            output_[i] = prev_output * gain;
        }
    }
    
private:
    const float* input_;
    float* output_;
    size_t samples_;
};
