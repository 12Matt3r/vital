#include "ai_manager.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <numeric>
#include <system_error>

namespace vital {

AIManager::AIManager() {
    // Initialize default settings
    enabled_features_[FeatureType::NeuralPreset] = true;
    enabled_features_[FeatureType::StyleTransfer] = true;
    enabled_features_[FeatureType::AdaptiveModulation] = true;
    enabled_features_[FeatureType::MachineLearning] = true;
    enabled_features_[FeatureType::AudioAnalysis] = true;
    enabled_features_[FeatureType::UserLearning] = true;
    enabled_features_[FeatureType::IntelligentGeneration] = true;
}

AIManager::~AIManager() {
    shutdown();
}

bool AIManager::initialize(int num_threads) {
    if (is_initialized_) return true;
    
    try {
        is_running_ = true;
        is_initialized_ = true;
        
        // Start worker threads
        int actual_threads = std::max(1, std::min(num_threads, std::thread::hardware_concurrency()));
        
        for (int i = 0; i < actual_threads; ++i) {
            worker_threads_.emplace_back([this] { workerThread(); });
        }
        
        // Start metrics collection thread
        std::thread metrics_thread([this] { 
            while (is_running_) {
                processMetrics();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        });
        metrics_thread.detach();
        
        recordEvent(FeatureType::MachineLearning, "AI Manager initialized with " + std::to_string(actual_threads) + " threads");
        
        return true;
    }
    catch (const std::exception& e) {
        // Log error (in real implementation, use proper logging)
        return false;
    }
}

void AIManager::shutdown() {
    if (!is_initialized_ || is_shutting_down_) return;
    
    is_shutting_down_ = true;
    is_running_ = false;
    
    // Notify all waiting threads
    queue_cv_.notify_all();
    
    // Wait for all worker threads to complete
    for (auto& thread : worker_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    cleanup();
    is_initialized_ = false;
    is_shutting_down_ = false;
}

void AIManager::workerThread() {
    while (is_running_) {
        std::unique_ptr<AIJob> job;
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            queue_cv_.wait(lock, [this] { 
                return !job_queue_.empty() || !is_running_; 
            });
            
            if (!is_running_ || job_queue_.empty()) {
                continue;
            }
            
            // Get highest priority job
            job = std::move(const_cast<std::unique_ptr<AIJob>&>(job_queue_.top()));
            job_queue_.pop();
        }
        
        if (job && enabled_features_[job->type]) {
            metrics_.active_jobs++;
            
            try {
                job->task();
                metrics_.completed_jobs++;
                
                // Update processing time metrics
                auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
                    Clock::now() - job->submission_time);
                
                // Update rolling average
                float current_avg = metrics_.avg_processing_time_ms.load();
                float new_avg = (current_avg * 0.9f) + (duration.count() / 1000.0f * 0.1f);
                metrics_.avg_processing_time_ms.store(new_avg);
                
                metrics_.total_processed++;
            }
            catch (const std::exception& e) {
                metrics_.failed_jobs++;
                // Log error
            }
            
            metrics_.active_jobs--;
        }
    }
}

void AIManager::processMetrics() {
    updateResourceUsage();
    
    // Clean up old events
    auto now = Clock::now();
    std::lock_guard<std::mutex> lock(events_mutex_);
    
    recent_events_.erase(
        std::remove_if(recent_events_.begin(), recent_events_.end(),
            [now](const auto& event) {
                return std::chrono::duration_cast<std::chrono::seconds>(now - event.first).count() > 3600;
            }),
        recent_events_.end());
}

void AIManager::updateResourceUsage() {
    // Simulate CPU and memory usage tracking
    // In real implementation, use OS-specific APIs
    metrics_.cpu_usage_percent.store(std::min(100.0f, metrics_.cpu_usage_percent.load() * 0.95f + 5.0f));
    metrics_.memory_usage_mb.store(50 + (metrics_.active_jobs.load() * 5));
}

void AIManager::setCpuUsageLimit(float percentage) {
    cpu_usage_limit_.store(std::clamp(percentage, 0.0f, 100.0f));
    recordEvent(FeatureType::MachineLearning, "CPU limit set to " + std::to_string(percentage) + "%");
}

void AIManager::setMemoryLimit(size_t max_memory_mb) {
    memory_limit_mb_.store(max_memory_mb);
    recordEvent(FeatureType::MachineLearning, "Memory limit set to " + std::to_string(max_memory_mb) + "MB");
}

void AIManager::enableFeature(FeatureType type, bool enable) {
    enabled_features_[type] = enable;
    recordEvent(type, enable ? "Feature enabled" : "Feature disabled");
}

bool AIManager::isFeatureEnabled(FeatureType type) const {
    return enabled_features_.at(type);
}

void AIManager::startProfiling() {
    profiling_.active = true;
    profiling_.start_time = Clock::now();
}

void AIManager::stopProfiling() {
    profiling_.active = false;
}

std::string AIManager::getProfilingReport() const {
    if (!profiling_.active) return "Profiling not active";
    
    std::ostringstream report;
    report << "AI Profiling Report\n";
    report << "==================\n\n";
    
    auto total_time = Clock::now() - profiling_.start_time;
    report << "Total profiling time: " 
           << std::chrono::duration_cast<std::chrono::milliseconds>(total_time).count() 
           << "ms\n\n";
    
    std::lock_guard<std::mutex> lock(profiling_.mutex);
    
    for (const auto& [type, times] : profiling_.processing_times) {
        if (times.empty()) continue;
        
        report << "Feature " << static_cast<int>(type) << ":\n";
        report << "  Calls: " << times.size() << "\n";
        
        float avg_time = std::accumulate(times.begin(), times.end(), 0.0f) / times.size();
        report << "  Avg time: " << avg_time << "ms\n";
        
        float max_time = *std::max_element(times.begin(), times.end());
        report << "  Max time: " << max_time << "ms\n";
        
        float min_time = *std::min_element(times.begin(), times.end());
        report << "  Min time: " << min_time << "ms\n\n";
    }
    
    return report.str();
}

void AIManager::saveSettings(const std::string& path) {
    try {
        std::ofstream file(path);
        if (!file.is_open()) return;
        
        std::lock_guard<std::mutex> lock(settings_mutex_);
        
        file << "# AI Manager Settings\n";
        
        for (const auto& [feature, enabled] : enabled_features_) {
            file << "feature_" << static_cast<int>(feature) << "=" << (enabled ? "true" : "false") << "\n";
        }
        
        for (const auto& [param, value] : user_preferences_) {
            file << "pref_" << param << "=" << value << "\n";
        }
        
        file << "cpu_limit=" << cpu_usage_limit_.load() << "\n";
        file << "memory_limit=" << memory_limit_mb_.load() << "\n";
    }
    catch (const std::exception& e) {
        // Log error
    }
}

bool AIManager::loadSettings(const std::string& path) {
    try {
        std::ifstream file(path);
        if (!file.is_open()) return false;
        
        std::lock_guard<std::mutex> lock(settings_mutex_);
        
        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;
            
            auto delimiter = line.find('=');
            if (delimiter == std::string::npos) continue;
            
            std::string key = line.substr(0, delimiter);
            std::string value = line.substr(delimiter + 1);
            
            if (key.starts_with("feature_")) {
                int feature_id = std::stoi(key.substr(8));
                if (feature_id >= 0 && feature_id < static_cast<int>(FeatureType::IntelligentGeneration)) {
                    enabled_features_[static_cast<FeatureType>(feature_id)] = (value == "true");
                }
            } else if (key.starts_with("pref_")) {
                user_preferences_[key.substr(5)] = std::stof(value);
            } else if (key == "cpu_limit") {
                cpu_usage_limit_.store(std::stof(value));
            } else if (key == "memory_limit") {
                memory_limit_mb_.store(std::stoul(value));
            }
        }
        
        recordEvent(FeatureType::MachineLearning, "Settings loaded from " + path);
        return true;
    }
    catch (const std::exception& e) {
        return false;
    }
}

void AIManager::recordEvent(FeatureType type, const std::string& event) {
    std::lock_guard<std::mutex> lock(events_mutex_);
    recent_events_.emplace_back(Clock::now(), 
        "[" + std::to_string(static_cast<int>(type)) + "] " + event);
}

std::vector<std::string> AIManager::getRecentEvents(size_t count) const {
    std::lock_guard<std::mutex> lock(events_mutex_);
    std::vector<std::string> events;
    
    auto start = recent_events_.size() > count ? 
        recent_events_.end() - count : recent_events_.begin();
    
    for (auto it = start; it != recent_events_.end(); ++it) {
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(Clock::now() - it->first);
        events.push_back("[" + std::to_string(duration.count()) + "s ago] " + it->second);
    }
    
    return events;
}

void AIManager::setUserPreference(FeatureType type, const std::string& parameter, float value) {
    std::lock_guard<std::mutex> lock(settings_mutex_);
    feature_preferences_[type][parameter] = value;
    user_preferences_[parameter] = value;
    
    recordEvent(type, "User preference updated: " + parameter + " = " + std::to_string(value));
}

float AIManager::getUserPreference(FeatureType type, const std::string& parameter) const {
    std::lock_guard<std::mutex> lock(settings_mutex_);
    
    auto type_it = feature_preferences_.find(type);
    if (type_it != feature_preferences_.end()) {
        auto param_it = type_it->second.find(parameter);
        if (param_it != type_it->second.end()) {
            return param_it->second;
        }
    }
    
    auto global_it = user_preferences_.find(parameter);
    return global_it != user_preferences_.end() ? global_it->second : 0.5f;
}

void AIManager::cleanup() {
    // Clear queues
    std::lock_guard<std::mutex> lock(queue_mutex_);
    while (!job_queue_.empty()) {
        job_queue_.pop();
    }
    
    // Reset metrics
    metrics_.total_processed = 0;
    metrics_.completed_jobs = 0;
    metrics_.failed_jobs = 0;
    metrics_.active_jobs = 0;
    metrics_.avg_processing_time_ms = 0.0f;
}

} // namespace vital