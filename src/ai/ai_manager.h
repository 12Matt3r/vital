#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <thread>
#include <queue>
#include <future>
#include <chrono>

namespace vital {

/**
 * @class AIManager
 * @brief Central AI integration manager for Vital synthesizer
 * 
 * Coordinates all AI features including neural preset generation, style transfer,
 * adaptive modulation, and machine learning capabilities.
 */
class AIManager {
public:
    using Clock = std::chrono::high_resolution_clock;
    using TimePoint = Clock::time_point;
    
    // AI Feature types
    enum class FeatureType {
        NeuralPreset,
        StyleTransfer,
        AdaptiveModulation,
        MachineLearning,
        AudioAnalysis,
        UserLearning,
        IntelligentGeneration
    };
    
    // Performance metrics
    struct PerformanceMetrics {
        std::atomic<uint64_t> total_processed{0};
        std::atomic<uint64_t> active_jobs{0};
        std::atomic<uint64_t> completed_jobs{0};
        std::atomic<uint64_t> failed_jobs{0};
        std::atomic<float> avg_processing_time_ms{0.0f};
        std::atomic<float> cpu_usage_percent{0.0f};
        std::atomic<uint64_t> memory_usage_mb{0};
    };
    
    // AI job structure
    struct AIJob {
        FeatureType type;
        std::function<void()> task;
        Priority priority;
        TimePoint submission_time;
        std::promise<void> promise;
        
        enum class Priority {
            Low = 0,
            Normal = 1,
            High = 2,
            Critical = 3
        };
    };
    
    AIManager();
    ~AIManager();
    
    // Initialization and configuration
    bool initialize(int num_threads = std::thread::hardware_concurrency());
    void shutdown();
    
    // Resource management
    void setCpuUsageLimit(float percentage);
    void setMemoryLimit(size_t max_memory_mb);
    void enableFeature(FeatureType type, bool enable = true);
    bool isFeatureEnabled(FeatureType type) const;
    
    // Job submission
    template<typename Func>
    std::future<void> submitJob(FeatureType type, Func&& task, AIJob::Priority priority = AIJob::Priority::Normal) {
        auto job = std::make_unique<AIJob>();
        job->type = type;
        job->priority = priority;
        job->submission_time = Clock::now();
        
        auto task_wrapper = [task = std::forward<Func>(task), job_ptr = job.get()]() {
            auto start = Clock::now();
            task();
            auto end = Clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
            job_ptr->promise.set_value();
        };
        
        job->task = std::move(task_wrapper);
        
        std::lock_guard<std::mutex> lock(queue_mutex_);
        job_queue_.push(std::move(job));
        queue_cv_.notify_one();
        
        return job->promise.get_future();
    }
    
    // Feature coordination
    template<typename ResultType, typename... Args>
    std::shared_ptr<std::future<ResultType>> callAI(FeatureType type, std::function<ResultType(Args...)> func, Args... args) {
        auto wrapped_func = [func = std::move(func), args...]() -> ResultType {
            return func(args...);
        };
        
        return std::make_shared<std::future<ResultType>>(
            submitJob(type, std::move(wrapped_func))->share()
        );
    }
    
    // Performance monitoring
    PerformanceMetrics getMetrics() const { return metrics_; }
    void startProfiling();
    void stopProfiling();
    std::string getProfilingReport() const;
    
    // Settings persistence
    void saveSettings(const std::string& path);
    bool loadSettings(const std::string& path);
    
    // Statistics and analytics
    void recordEvent(FeatureType type, const std::string& event);
    std::vector<std::string> getRecentEvents(size_t count = 100) const;
    
    // Status reporting
    bool isReady() const { return is_initialized_ && !is_shutting_down_; }
    size_t getJobQueueSize() const { return job_queue_.size(); }
    size_t getActiveJobs() const { return metrics_.active_jobs.load(); }
    
    // Learning system access
    void setUserPreference(FeatureType type, const std::string& parameter, float value);
    float getUserPreference(FeatureType type, const std::string& parameter) const;
    
private:
    void workerThread();
    void processMetrics();
    void updateResourceUsage();
    void cleanup();
    
    // Member variables
    mutable std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    std::queue<std::unique_ptr<AIJob>> job_queue_;
    std::vector<std::thread> worker_threads_;
    
    std::atomic<bool> is_running_{false};
    std::atomic<bool> is_initialized_{false};
    std::atomic<bool> is_shutting_down_{false};
    
    mutable std::mutex settings_mutex_;
    std::unordered_map<FeatureType, bool> enabled_features_;
    std::unordered_map<std::string, float> user_preferences_;
    std::unordered_map<FeatureType, std::unordered_map<std::string, float>> feature_preferences_;
    
    mutable std::mutex events_mutex_;
    std::vector<std::pair<TimePoint, std::string>> recent_events_;
    
    PerformanceMetrics metrics_;
    std::atomic<float> cpu_usage_limit_{100.0f};
    std::atomic<size_t> memory_limit_mb_{1024};
    
    // Profiling
    struct ProfilingData {
        std::atomic<bool> active{false};
        TimePoint start_time;
        std::unordered_map<FeatureType, std::vector<float>> processing_times;
        std::mutex mutex;
    } profiling_;
};

} // namespace vital