/**
 * @file multithreading.h
 * @brief Comprehensive multithreading optimization framework for Vital
 * @author Vital Development Team
 * @date 2025-11-03
 * 
 * This module provides high-performance multithreading capabilities including
 * voice processing parallelization, lock-free data structures, CPU affinity
 * management, and work stealing scheduling.
 */

#pragma once

#include <algorithm>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>
#include <type_traits>

namespace vital {
namespace performance {
namespace threading {

// ============================================================================
// Thread Configuration and Platform Support
// ============================================================================

/**
 * Platform-specific thread utilities
 */
class ThreadPlatform {
public:
    // Set thread CPU affinity for optimal placement
    static bool set_thread_affinity(std::thread::id thread_id, int cpu_core) {
        #ifdef _WIN32
            HANDLE handle = OpenThread(THREAD_ALL_ACCESS, FALSE, 
                                     static_cast<DWORD>(get_native_thread_id(thread_id)));
            if (handle != nullptr) {
                DWORD_PTR mask = 1ULL << cpu_core;
                bool result = SetThreadAffinityMask(handle, mask) != 0;
                CloseHandle(handle);
                return result;
            }
        #elif defined(__linux__)
            cpu_set_t cpuset;
            CPU_ZERO(&cpuset);
            CPU_SET(cpu_core, &cpuset);
            return pthread_setaffinity_np(get_native_thread_id(thread_id), 
                                         sizeof(cpu_set_t), &cpuset) == 0;
        #elif defined(__APPLE__)
            thread_extended_policy_data_t policy_data;
            policy_data.timeshare = false; // Set real-time priority
            return thread_policy_set(mach_thread_self(), 
                                   THREAD_EXTENDED_POLICY, 
                                   (thread_policy_t)&policy_data,
                                   THREAD_EXTENDED_POLICY_COUNT) == KERN_SUCCESS;
        #endif
        return false;
    }
    
    // Set thread priority for real-time audio processing
    static bool set_thread_priority(std::thread::id thread_id, int priority) {
        #ifdef _WIN32
            HANDLE handle = OpenThread(THREAD_ALL_ACCESS, FALSE,
                                     static_cast<DWORD>(get_native_thread_id(thread_id)));
            if (handle != nullptr) {
                bool result = SetThreadPriority(handle, priority) != 0;
                CloseHandle(handle);
                return result;
            }
        #elif defined(__linux__)
            struct sched_param param;
            param.sched_priority = priority;
            return pthread_setschedparam(get_native_thread_id(thread_id), 
                                        SCHED_FIFO, &param) == 0;
        #elif defined(__APPLE__)
            thread_time_constraint_policy_data_t policy_data;
            policy_data.period = 1000000; // 1ms period
            policy_data.computation = 500000; // 0.5ms computation time
            policy_data.constraint = 1000000; // 1ms deadline
            policy_data.preemptible = 1; // Preemptible
            return thread_policy_set(mach_thread_self(),
                                   THREAD_TIME_CONSTRAINT_POLICY,
                                   (thread_policy_t)&policy_data,
                                   THREAD_TIME_CONSTRAINT_POLICY_COUNT) == KERN_SUCCESS;
        #endif
        return false;
    }
    
    // Get optimal CPU core for audio processing
    static std::vector<int> get_audio_optimal_cores(int thread_count) {
        std::vector<int> optimal_cores;
        
        #ifdef _WIN32
            SYSTEM_INFO sysinfo;
            GetSystemInfo(&sysinfo);
            int num_cores = sysinfo.dwNumberOfProcessors;
            // Reserve first core for audio, use others for parallel processing
            for (int i = 1; i < std::min(num_cores, thread_count + 1); ++i) {
                optimal_cores.push_back(i);
            }
        #elif defined(__linux__)
            // Read CPU topology from /sys
            std::ifstream cpuinfo("/proc/cpuinfo");
            std::string line;
            int core_id = 0;
            while (std::getline(cpuinfo, line)) {
                if (line.find("processor") != std::string::npos) {
                    optimal_cores.push_back(core_id++);
                    if (static_cast<int>(optimal_cores.size()) >= thread_count) break;
                }
            }
        #elif defined(__APPLE__)
            // macOS typically has efficiency and performance cores
            // Use performance cores (higher core IDs on Apple Silicon)
            int total_cores = std::thread::hardware_concurrency();
            for (int i = total_cores / 2; i < total_cores && 
                 static_cast<int>(optimal_cores.size()) < thread_count; ++i) {
                optimal_cores.push_back(i);
            }
        #endif
        
        return optimal_cores;
    }
    
private:
    static uintptr_t get_native_thread_id(std::thread::id thread_id) {
        #ifdef _WIN32
            return static_cast<uintptr_t>(*reinterpret_cast<const uint32_t*>(&thread_id));
        #elif defined(__linux__) || defined(__APPLE__)
            return static_cast<uintptr_t>(*reinterpret_cast<const uint64_t*>(&thread_id));
        #else
            return 0;
        #endif
    }
};

// ============================================================================
// Lock-Free Data Structures
// ============================================================================

/**
 * Lock-free single-producer single-consumer ring buffer
 */
template<typename T, size_t Size>
class LockFreeSPSCRingBuffer {
    static_assert((Size & (Size - 1)) == 0, "Size must be a power of 2");
    
public:
    LockFreeSPSCRingBuffer() : head_(0), tail_(0) {}
    
    bool push(const T& item) {
        size_t current_tail = tail_.load(std::memory_order_relaxed);
        size_t next_tail = (current_tail + 1) & (Size - 1);
        
        if (next_tail == head_.load(std::memory_order_acquire)) {
            return false; // Buffer full
        }
        
        buffer_[current_tail] = item;
        tail_.store(next_tail, std::memory_order_release);
        return true;
    }
    
    bool pop(T& item) {
        size_t current_head = head_.load(std::memory_order_relaxed);
        
        if (current_head == tail_.load(std::memory_order_acquire)) {
            return false; // Buffer empty
        }
        
        item = buffer_[current_head];
        head_.store((current_head + 1) & (Size - 1), std::memory_order_release);
        return true;
    }
    
    bool empty() const {
        return head_.load(std::memory_order_acquire) == tail_.load(std::memory_order_acquire);
    }
    
    size_t size() const {
        size_t head = head_.load(std::memory_order_acquire);
        size_t tail = tail_.load(std::memory_order_acquire);
        return (tail - head) & (Size - 1);
    }
    
private:
    alignas(64) T buffer_[Size]; // Cache line alignment
    std::atomic<size_t> head_;
    std::atomic<size_t> tail_;
};

/**
 * Lock-free multi-producer multi-consumer ring buffer
 */
template<typename T, size_t Size>
class LockFreeMPSCRingBuffer {
    static_assert((Size & (Size - 1)) == 0, "Size must be a power of 2");
    
public:
    LockFreeMPSCRingBuffer() : head_(0), tail_(0) {}
    
    bool push(const T& item) {
        size_t current_tail = tail_.fetch_add(1, std::memory_order_relaxed);
        size_t index = current_tail & (Size - 1);
        
        // Wait if buffer is full
        while ((current_tail - head_.load(std::memory_order_acquire)) >= Size) {
            std::this_thread::yield();
        }
        
        buffer_[index] = item;
        return true;
    }
    
    bool pop(T& item) {
        size_t current_head = head_.fetch_add(1, std::memory_order_relaxed);
        size_t index = current_head & (Size - 1);
        
        // Wait if buffer is empty
        while (current_head >= tail_.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }
        
        item = buffer_[index];
        return true;
    }
    
    size_t size() const {
        return tail_.load(std::memory_order_acquire) - head_.load(std::memory_order_acquire);
    }
    
private:
    alignas(64) T buffer_[Size]; // Cache line alignment
    std::atomic<size_t> head_;
    std::atomic<size_t> tail_;
};

// ============================================================================
// Thread-Safe Audio Buffer
// ============================================================================

/**
 * Thread-safe audio buffer with lock-free operations
 */
template<size_t NumChannels, size_t BufferSize>
class ThreadSafeAudioBuffer {
public:
    static constexpr size_t num_channels = NumChannels;
    static constexpr size_t buffer_size = BufferSize;
    static constexpr size_t total_size = NumChannels * BufferSize;
    
    ThreadSafeAudioBuffer() {
        clear();
    }
    
    // Lock-free write for audio thread
    void write_lockfree(const float* data, size_t num_samples, size_t channel) {
        if (channel >= NumChannels || num_samples > BufferSize) return;
        
        size_t write_pos = write_pos_[channel].fetch_add(num_samples) % BufferSize;
        
        for (size_t i = 0; i < num_samples; ++i) {
            size_t buffer_index = (write_pos + i) % BufferSize;
            size_t array_index = channel * BufferSize + buffer_index;
            buffer_[array_index] = data[i];
        }
    }
    
    // Lock-free read for any thread
    bool read_lockfree(float* data, size_t num_samples, size_t channel) const {
        if (channel >= NumChannels || num_samples > BufferSize) return false;
        
        size_t read_pos = read_pos_[channel].load();
        size_t current_write_pos = write_pos_[channel].load();
        
        if (read_pos == current_write_pos) return false; // No data available
        
        size_t available_samples = (current_write_pos - read_pos) % BufferSize;
        size_t samples_to_read = std::min(num_samples, available_samples);
        
        for (size_t i = 0; i < samples_to_read; ++i) {
            size_t buffer_index = (read_pos + i) % BufferSize;
            size_t array_index = channel * BufferSize + buffer_index;
            data[i] = buffer_[array_index];
        }
        
        read_pos_[channel].fetch_add(samples_to_read);
        return true;
    }
    
    // Clear buffer
    void clear() {
        std::fill(std::begin(buffer_), std::end(buffer_), 0.0f);
        for (auto& pos : write_pos_) pos = 0;
        for (auto& pos : read_pos_) pos = 0;
    }
    
    // Get buffer statistics
    struct Statistics {
        size_t total_samples_written = 0;
        size_t total_samples_read = 0;
        float peak_level = 0.0f;
        size_t overruns = 0;
        size_t underruns = 0;
    };
    
    Statistics get_statistics() const {
        Statistics stats;
        for (size_t channel = 0; channel < NumChannels; ++channel) {
            stats.total_samples_written += write_pos_[channel].load();
            stats.total_samples_read += read_pos_[channel].load();
            
            // Calculate peak level
            size_t channel_offset = channel * BufferSize;
            for (size_t i = 0; i < BufferSize; ++i) {
                stats.peak_level = std::max(stats.peak_level, std::abs(buffer_[channel_offset + i]));
            }
        }
        return stats;
    }
    
private:
    alignas(64) float buffer_[total_size];
    std::atomic<size_t> write_pos_[NumChannels];
    std::atomic<size_t> read_pos_[NumChannels];
};

// ============================================================================
// Work Stealing Scheduler
// ============================================================================

/**
 * Work stealing task for parallel processing
 */
class WorkStealingTask {
public:
    virtual ~WorkStealingTask() = default;
    virtual void execute() = 0;
    virtual std::unique_ptr<WorkStealingTask> split() { return nullptr; }
};

/**
 * Work stealing scheduler for dynamic load balancing
 */
class WorkStealingScheduler {
public:
    WorkStealingScheduler(size_t num_threads = std::thread::hardware_concurrency()) 
        : num_threads_(num_threads) {
        // Create thread-local deques
        deques_.reserve(num_threads_);
        for (size_t i = 0; i < num_threads_; ++i) {
            deques_.emplace_back();
        }
        
        // Start worker threads
        for (size_t i = 0; i < num_threads_; ++i) {
            threads_.emplace_back(&WorkStealingScheduler::worker_thread, this, i);
        }
    }
    
    ~WorkStealingScheduler() {
        stop_ = true;
        work_available_cv_.notify_all();
        
        for (auto& thread : threads_) {
            if (thread.joinable()) thread.join();
        }
    }
    
    // Submit work to thread pool
    void submit(std::unique_ptr<WorkStealingTask> task) {
        size_t current_thread_id = get_current_thread_id();
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            deques_[current_thread_id].push_back(std::move(task));
        }
        work_available_cv_.notify_one();
    }
    
    // Execute tasks until queue is empty
    void execute_until_empty() {
        execute_tasks(1); // Use current thread
    }
    
    // Execute tasks with timeout
    void execute_for(std::chrono::milliseconds timeout) {
        auto start = std::chrono::high_resolution_clock::now();
        while (std::chrono::high_resolution_clock::now() - start < timeout) {
            if (!execute_tasks(0)) { // Try to steal work
                std::this_thread::sleep_for(std::chrono::microseconds(100));
            }
        }
    }
    
    // Get scheduler statistics
    struct Statistics {
        size_t total_tasks_completed = 0;
        size_t total_steal_attempts = 0;
        size_t successful_steals = 0;
        std::vector<size_t> tasks_per_thread;
        double load_balance_efficiency = 0.0;
    };
    
    Statistics get_statistics() const {
        Statistics stats;
        stats.tasks_per_thread.resize(num_threads_);
        
        for (size_t i = 0; i < num_threads_; ++i) {
            stats.tasks_per_thread[i] = completed_tasks_[i].load();
        }
        
        stats.total_steal_attempts = total_steal_attempts_.load();
        stats.successful_steals = successful_steals_.load();
        
        if (stats.total_steal_attempts > 0) {
            stats.load_balance_efficiency = 
                static_cast<double>(stats.successful_steals) / stats.total_steal_attempts;
        }
        
        return stats;
    }
    
private:
    size_t num_threads_;
    std::vector<std::deque<std::unique_ptr<WorkStealingTask>>> deques_;
    std::vector<std::thread> threads_;
    std::atomic<bool> stop_{false};
    std::atomic<size_t> current_thread_id_{0};
    
    // Statistics
    std::vector<std::atomic<size_t>> completed_tasks_;
    std::atomic<size_t> total_steal_attempts_{0};
    std::atomic<size_t> successful_steals_{0};
    
    mutable std::mutex queue_mutex_;
    std::condition_variable work_available_cv_;
    
    void worker_thread(size_t thread_id) {
        current_thread_id_.store(thread_id);
        completed_tasks_.resize(num_threads_);
        
        while (!stop_) {
            if (!execute_tasks(thread_id)) {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                work_available_cv_.wait_for(lock, std::chrono::milliseconds(10));
            }
        }
    }
    
    bool execute_tasks(size_t thread_id) {
        bool executed_work = false;
        
        while (true) {
            // Try to pop from local deque (LIFO)
            std::unique_ptr<WorkStealingTask> task;
            {
                std::lock_guard<std::mutex> lock(queue_mutex_);
                if (!deques_[thread_id].empty()) {
                    task = std::move(deques_[thread_id].back());
                    deques_[thread_id].pop_back();
                }
            }
            
            if (task) {
                task->execute();
                executed_work = true;
                completed_tasks_[thread_id]++;
                continue;
            }
            
            // Try to steal from other deques (FIFO)
            bool stole_work = steal_work(thread_id);
            if (!stole_work) break;
        }
        
        return executed_work;
    }
    
    bool steal_work(size_t thread_id) {
        size_t attempts = 0;
        size_t num_victims = num_threads_ - 1;
        
        if (num_victims == 0) return false;
        
        total_steal_attempts_++;
        
        // Try to steal from random victim
        while (attempts < num_victims * 2) {
            size_t victim = (thread_id + 1 + attempts) % num_threads_;
            std::unique_ptr<WorkStealingTask> task;
            
            {
                std::lock_guard<std::mutex> lock(queue_mutex_);
                if (!deques_[victim].empty()) {
                    task = std::move(deques_[victim].front());
                    deques_[victim].pop_front();
                }
            }
            
            if (task) {
                task->execute();
                successful_steals_++;
                completed_tasks_[thread_id]++;
                return true;
            }
            
            attempts++;
        }
        
        return false;
    }
    
    size_t get_current_thread_id() {
        // In a real implementation, this would use thread-local storage
        // For this example, we'll use the atomic counter
        return current_thread_id_.load();
    }
};

// ============================================================================
// Audio Thread Manager
// ============================================================================

/**
 * Specialized manager for audio processing threads
 */
class AudioThreadManager {
public:
    struct AudioThreadConfig {
        int cpu_affinity = -1;           // Specific CPU core (-1 for auto)
        int thread_priority = 80;        // Thread priority (0-100)
        size_t buffer_size = 256;        // Audio buffer size
        double sample_rate = 44100.0;    // Sample rate
        bool realtime_scheduling = true; // Use real-time scheduling
    };
    
    AudioThreadManager() {
        config_.cpu_affinity = -1;
        config_.thread_priority = 80;
        config_.realtime_scheduling = true;
        
        // Detect optimal CPU cores for audio processing
        auto optimal_cores = ThreadPlatform::get_audio_optimal_cores(4);
        if (!optimal_cores.empty()) {
            config_.cpu_affinity = optimal_cores[0];
        }
    }
    
    ~AudioThreadManager() {
        stop_audio_threads();
    }
    
    // Configure audio thread
    void configure_audio_thread(const AudioThreadConfig& config) {
        config_ = config;
        
        // Apply platform-specific optimizations
        if (config_.realtime_scheduling) {
            ThreadPlatform::set_thread_priority(std::this_thread::get_id(), 
                                              config_.thread_priority);
        }
    }
    
    // Start audio processing thread
    void start_audio_thread(std::function<void()> audio_processing_function) {
        stop_flag_ = false;
        
        audio_thread_ = std::thread([this, audio_processing_function = std::move(audio_processing_function)]() {
            // Set up thread for real-time audio processing
            if (config_.cpu_affinity >= 0) {
                ThreadPlatform::set_thread_affinity(std::this_thread::get_id(), 
                                                  config_.cpu_affinity);
            }
            
            ThreadPlatform::set_thread_priority(std::this_thread::get_id(), 
                                              config_.thread_priority);
            
            // Set high-performance scheduling policy if supported
            #ifdef __linux__
            struct sched_param param;
            param.sched_priority = sched_get_priority_max(SCHED_FIFO);
            pthread_setschedparam(pthread_self(), SCHED_FIFO, &param);
            #endif
            
            // Audio processing loop
            while (!stop_flag_.load()) {
                auto start_time = std::chrono::high_resolution_clock::now();
                
                // Execute audio processing
                audio_processing_function();
                
                // Calculate timing
                auto end_time = std::chrono::high_resolution_clock::now();
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
                
                // Check if we're meeting real-time constraints
                size_t max_processing_time_us = static_cast<size_t>((config_.buffer_size / config_.sample_rate) * 1000000.0);
                if (duration.count() > max_processing_time_us) {
                    // Update performance statistics
                    performance_stats_.overruns++;
                }
                
                performance_stats_.total_processing_time_us += duration.count();
                performance_stats_.iterations++;
                
                // Calculate sleep time to maintain consistent buffer timing
                auto sleep_duration = std::chrono::microseconds(max_processing_time_us) - duration;
                if (sleep_duration.count() > 0) {
                    std::this_thread::sleep_for(sleep_duration);
                }
            }
        });
    }
    
    // Stop audio processing threads
    void stop_audio_threads() {
        stop_flag_ = true;
        if (audio_thread_.joinable()) {
            audio_thread_.join();
        }
    }
    
    // Get performance statistics
    struct PerformanceStats {
        size_t iterations = 0;
        size_t total_processing_time_us = 0;
        size_t overruns = 0;
        double average_processing_time_us = 0.0;
        double max_processing_time_us = 0.0;
        double min_processing_time_us = std::numeric_limits<double>::max();
        double cpu_utilization_percent = 0.0;
    };
    
    PerformanceStats get_performance_stats() {
        PerformanceStats stats = performance_stats_;
        
        if (stats.iterations > 0) {
            stats.average_processing_time_us = 
                static_cast<double>(stats.total_processing_time_us) / stats.iterations;
            stats.cpu_utilization_percent = 
                (stats.average_processing_time_us / ((config_.buffer_size / config_.sample_rate) * 1000000.0)) * 100.0;
        }
        
        return stats;
    }
    
    // Reset statistics
    void reset_performance_stats() {
        performance_stats_ = PerformanceStats{};
    }
    
private:
    AudioThreadConfig config_;
    std::thread audio_thread_;
    std::atomic<bool> stop_flag_{false};
    PerformanceStats performance_stats_;
};

// ============================================================================
// Parallel DSP Processing
// ============================================================================

/**
 * Parallel DSP processor for voice synthesis
 */
class ParallelDSPProcessor {
public:
    ParallelDSPProcessor(size_t max_voices = 32) 
        : max_voices_(max_voices) {
        voice_buffers_.reserve(max_voices);
        for (size_t i = 0; i < max_voices; ++i) {
            voice_buffers_.emplace_back();
        }
    }
    
    // Process multiple voice buffers in parallel
    template<typename VoiceProcessor>
    void process_voices_parallel(const std::vector<float*>& input_buffers,
                                std::vector<float*>& output_buffers,
                                size_t num_samples,
                                VoiceProcessor&& processor) {
        size_t num_voices = std::min(input_buffers.size(), output_voices_.load());
        if (num_voices == 0) return;
        
        // Submit voice processing tasks to work stealing scheduler
        for (size_t voice = 0; voice < num_voices; ++voice) {
            auto task = std::make_unique<VoiceProcessingTask>(
                input_buffers[voice], output_buffers[voice], num_samples, 
                std::forward<VoiceProcessor>(processor));
            
            scheduler_.submit(std::move(task));
        }
        
        // Wait for completion
        scheduler_.execute_until_empty();
    }
    
    // Set active voice count
    void set_active_voice_count(size_t count) {
        output_voices_.store(std::min(count, max_voices_));
    }
    
    size_t get_active_voice_count() const {
        return output_voices_.load();
    }
    
private:
    size_t max_voices_;
    std::vector<std::vector<float>> voice_buffers_;
    std::atomic<size_t> output_voices_{0};
    WorkStealingScheduler scheduler_;
    
    class VoiceProcessingTask : public WorkStealingTask {
    public:
        VoiceProcessingTask(float* input, float* output, size_t samples,
                          std::function<void(float*, float*, size_t)> processor)
            : input_(input), output_(output), samples_(samples), 
              processor_(std::move(processor)) {}
        
        void execute() override {
            processor_(input_, output_, samples_);
        }
        
    private:
        float* input_;
        float* output_;
        size_t samples_;
        std::function<void(float*, float*, size_t)> processor_;
    };
};

// ============================================================================
// Thread Pool Utilities
// ============================================================================

/**
 * Simple thread pool for general-purpose parallel tasks
 */
class ThreadPool {
public:
    explicit ThreadPool(size_t thread_count = std::thread::hardware_concurrency()) 
        : stop_(false) {
        for (size_t i = 0; i < thread_count; ++i) {
            workers_.emplace_back([this] { worker_thread(); });
        }
    }
    
    ~ThreadPool() {
        stop_ = true;
        condition_.notify_all();
        
        for (std::thread& worker : workers_) {
            if (worker.joinable()) worker.join();
        }
    }
    
    // Enqueue a task
    template<typename Func, typename... Args>
    auto enqueue(Func&& func, Args&&... args) 
        -> std::future<typename std::result_of<Func(Args...)>::type> {
        using return_type = typename std::result_of<Func(Args...)>::type;
        
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<Func>(func), std::forward<Args>(args)...));
        
        std::future<return_type> result = task->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            if (stop_) {
                throw std::runtime_error("enqueue on stopped ThreadPool");
            }
            tasks_.emplace([task]() { (*task)(); });
        }
        condition_.notify_one();
        return result;
    }
    
    // Get number of threads
    size_t size() const {
        return workers_.size();
    }
    
    // Wait for all tasks to complete
    void wait_all() {
        std::unique_lock<std::mutex> lock(queue_mutex_);
        condition_.wait(lock, [this] { return tasks_.empty(); });
    }
    
private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable condition_;
    bool stop_;
    
    void worker_thread() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                condition_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
                if (stop_ && tasks_.empty()) return;
                
                task = std::move(tasks_.front());
                tasks_.pop();
            }
            
            task();
        }
    }
};

} // namespace threading
} // namespace performance
} // namespace vital
