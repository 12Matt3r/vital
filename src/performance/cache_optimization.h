/**
 * @file cache_optimization.h
 * @brief Cache optimization framework for high-performance audio processing
 * @author Vital Development Team
 * @date 2025-11-03
 * 
 * This module provides comprehensive cache optimization techniques including
 * cache-aligned data structures, NUMA-aware memory allocation, hardware
 * prefetching, and cache coherency management.
 */

#pragma once

#include <algorithm>
#include <array>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <limits>
#include <memory>
#include <new>
#include <type_traits>
#include <vector>

#ifdef _WIN32
    #include <windows.h>
    #include <processthreadsapi.h>
    #include <synchapi.h>
#else
    #include <sys/mman.h>
    #include <unistd.h>
    #include <pthread.h>
#endif

namespace vital {
namespace performance {
namespace cache {

// ============================================================================
// Cache Configuration and Constants
// ============================================================================

/**
 * Cache hierarchy configuration
 */
struct CacheConfig {
    static constexpr size_t cache_line_size = 64;        // Typical cache line size
    static constexpr size_t l1_cache_size = 32 * 1024;   // 32KB L1 cache
    static constexpr size_t l2_cache_size = 256 * 1024;  // 256KB L2 cache
    static constexpr size_t l3_cache_size = 8 * 1024 * 1024; // 8MB L3 cache
    static constexpr size_t page_size = 4096;            // Memory page size
    static constexpr size_t prefetch_distance = 16;      // Prefetch horizon
};

/**
 * Cache line alignment utility
 */
template<typename T>
struct alignas(CacheConfig::cache_line_size) CacheLineAligned {
    T data;
};

// ============================================================================
// Cache-Aligned Memory Allocation
// ============================================================================

/**
 * Cache-aligned allocator with custom alignment
 */
template<typename T, size_t Alignment = CacheConfig::cache_line_size>
class CacheAlignedAllocator {
public:
    using value_type = T;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    
    template<typename U>
    struct rebind {
        using other = CacheAlignedAllocator<U, Alignment>;
    };
    
    CacheAlignedAllocator() = default;
    
    template<typename U>
    CacheAlignedAllocator(const CacheAlignedAllocator<U, Alignment>&) noexcept {}
    
    T* allocate(size_type n) {
        if (n == 0) return nullptr;
        if (n > std::numeric_limits<size_type>::max() / sizeof(T)) {
            throw std::bad_array_new_length();
        }
        
        size_type bytes = n * sizeof(T);
        void* ptr = aligned_alloc(bytes, Alignment);
        
        if (ptr == nullptr) {
            throw std::bad_alloc();
        }
        
        return static_cast<T*>(ptr);
    }
    
    void deallocate(T* ptr, size_type) noexcept {
        std::free(ptr);
    }
    
    bool operator==(const CacheAlignedAllocator& other) const noexcept {
        return true;
    }
    
    bool operator!=(const CacheAlignedAllocator& other) const noexcept {
        return false;
    }
    
private:
    void* aligned_alloc(size_type size, size_t alignment) {
        #ifdef _WIN32
            return _aligned_malloc(size, alignment);
        #else
            void* ptr = nullptr;
            if (posix_memalign(&ptr, alignment, size) == 0) {
                return ptr;
            }
            return nullptr;
        #endif
    }
};

/**
 * Cache-aligned container template
 */
template<typename T, size_t Capacity, size_t Alignment = CacheConfig::cache_line_size>
class CacheAlignedContainer {
public:
    using allocator_type = CacheAlignedAllocator<T, Alignment>;
    using value_type = T;
    using size_type = std::size_t;
    using reference = T&;
    using const_reference = const T&;
    using pointer = typename std::allocator_traits<allocator_type>::pointer;
    using const_pointer = typename std::allocator_traits<allocator_type>::const_pointer;
    using iterator = T*;
    using const_iterator = const T*;
    
    CacheAlignedContainer() {
        data_ = allocator_.allocate(Capacity);
        size_ = 0;
    }
    
    ~CacheAlignedContainer() {
        allocator_.deallocate(data_, Capacity);
    }
    
    // Copy and move operations
    CacheAlignedContainer(const CacheAlignedContainer& other) {
        data_ = allocator_.allocate(Capacity);
        size_ = other.size_;
        std::copy_n(other.data_, size_, data_);
    }
    
    CacheAlignedContainer& operator=(const CacheAlignedContainer& other) {
        if (this != &other) {
            size_ = other.size_;
            std::copy_n(other.data_, size_, data_);
        }
        return *this;
    }
    
    CacheAlignedContainer(CacheAlignedContainer&& other) noexcept
        : data_(other.data_), size_(other.size_) {
        other.data_ = nullptr;
        other.size_ = 0;
    }
    
    CacheAlignedContainer& operator=(CacheAlignedContainer&& other) noexcept {
        if (this != &other) {
            std::swap(data_, other.data_);
            std::swap(size_, other.size_);
        }
        return *this;
    }
    
    // Element access
    reference operator[](size_type index) {
        return data_[index];
    }
    
    const_reference operator[](size_type index) const {
        return data_[index];
    }
    
    reference at(size_type index) {
        if (index >= size_) throw std::out_of_range("Index out of range");
        return data_[index];
    }
    
    const_reference at(size_type index) const {
        if (index >= size_) throw std::out_of_range("Index out of range");
        return data_[index];
    }
    
    pointer data() { return data_; }
    const_pointer data() const { return data_; }
    
    // Capacity and size
    size_type size() const { return size_; }
    size_type capacity() const { return Capacity; }
    bool empty() const { return size_ == 0; }
    bool full() const { return size_ == Capacity; }
    
    // Modifiers
    void push_back(const T& value) {
        if (size_ < Capacity) {
            data_[size_++] = value;
        }
    }
    
    template<typename... Args>
    reference emplace_back(Args&&... args) {
        if (size_ < Capacity) {
            return *(new (&data_[size_++]) T(std::forward<Args>(args)...));
        }
        throw std::overflow_error("Container is full");
    }
    
    void pop_back() {
        if (size_ > 0) {
            size_--;
            data_[size_].~T();
        }
    }
    
    void clear() {
        for (size_type i = 0; i < size_; ++i) {
            data_[i].~T();
        }
        size_ = 0;
    }
    
    // Iterators
    iterator begin() { return data_; }
    iterator end() { return data_ + size_; }
    const_iterator begin() const { return data_; }
    const_iterator end() const { return data_ + size_; }
    const_iterator cbegin() const { return data_; }
    const_iterator cend() const { return data_ + size_; }
    
    // Cache-aware operations
    void prefetch_range(size_type start, size_type count) const {
        for (size_type i = start; i < start + count && i < size_; ++i) {
            __builtin_prefetch(&data_[i], 0, 3); // Prefetch for read, high temporal locality
        }
    }
    
    void prefetch_for_write(size_type start, size_type count) const {
        for (size_type i = start; i < start + count && i < size_; ++i) {
            __builtin_prefetch(&data_[i], 1, 3); // Prefetch for write, high temporal locality
        }
    }
    
private:
    pointer data_;
    size_type size_;
    allocator_type allocator_;
};

// ============================================================================
// Hardware Prefetching
// ============================================================================

/**
 * Hardware prefetcher for cache-aware access patterns
 */
class HardwarePrefetcher {
public:
    HardwarePrefetcher() = default;
    
    /**
     * Prefetch data for sequential access
     */
    template<typename T>
    void prefetch_sequential(const T* base, size_t count, size_t stride = sizeof(T)) {
        for (size_t i = 0; i < count; i += CacheConfig::prefetch_distance) {
            const char* addr = reinterpret_cast<const char*>(base) + i * stride;
            __builtin_prefetch(addr, 0, 3); // Prefetch for read
        }
    }
    
    /**
     * Prefetch data for write operations
     */
    template<typename T>
    void prefetch_for_write(const T* base, size_t count, size_t stride = sizeof(T)) {
        for (size_t i = 0; i < count; i += CacheConfig::prefetch_distance) {
            const char* addr = reinterpret_cast<const char*>(base) + i * stride;
            __builtin_prefetch(addr, 1, 3); // Prefetch for write
        }
    }
    
    /**
     * Prefetch audio buffer data
     */
    void prefetch_audio_buffer(const float* buffer, size_t sample_index, size_t count) {
        prefetch_sequential(buffer + sample_index, count, sizeof(float));
    }
    
    /**
     * Prefetch filter coefficients
     */
    void prefetch_filter_coefficients(const float* coefficients, size_t order) {
        prefetch_sequential(coefficients, order, sizeof(float));
    }
    
    /**
     * Prefetch wavetable data
     */
    void prefetch_wavetable(const float* wavetable, size_t table_size) {
        prefetch_sequential(wavetable, table_size, sizeof(float));
    }
    
    /**
     * Prefetch with software pipelining hints
     */
    template<typename Func>
    void prefetch_pipeline(const float* buffer, size_t count, size_t pipeline_depth, Func&& processor) {
        size_t pipeline_states = std::min(count, pipeline_depth * CacheConfig::prefetch_distance);
        
        // Prefetch initial pipeline
        for (size_t i = 0; i < pipeline_states; i += CacheConfig::prefetch_distance) {
            prefetch_sequential(buffer + i, CacheConfig::prefetch_distance);
        }
        
        // Process with continuous prefetching
        for (size_t i = 0; i < count; ++i) {
            size_t prefetch_index = i + pipeline_states;
            if (prefetch_index < count) {
                prefetch_sequential(buffer + prefetch_index, CacheConfig::prefetch_distance);
            }
            
            processor(buffer[i], i);
        }
    }
};

// ============================================================================
// NUMA-Aware Memory Allocation
// ============================================================================

/**
 * NUMA topology information
 */
struct NUMATopology {
    struct Node {
        int node_id;
        size_t memory_size;
        std::vector<int> cpu_cores;
        double bandwidth_gbps;
        double latency_ns;
    };
    
    std::vector<Node> nodes;
    int total_nodes = 0;
    int current_node = -1;
    bool numa_available = false;
};

/**
 * NUMA-aware memory manager
 */
class NUMAMemoryManager {
public:
    NUMAMemoryManager() {
        detect_numa_topology();
    }
    
    ~NUMAMemoryManager() {
        cleanup_allocations();
    }
    
    /**
     * Allocate memory on the specified NUMA node
     */
    void* allocate_on_node(size_t size, int node_id = -1) {
        if (node_id < 0) {
            node_id = topology_.current_node >= 0 ? topology_.current_node : 0;
        }
        
        void* ptr = nullptr;
        
        #ifdef __linux__
        if (topology_.numa_available) {
            // Use numa_alloc_onnode for Linux
            ptr = numa_alloc_onnode(size, node_id);
        } else {
            ptr = aligned_alloc(CacheConfig::cache_line_size, size);
        }
        #elif defined(_WIN32)
        // Windows NUMA support
        if (node_id < topology_.total_nodes) {
            // Use VirtualAllocEx with NUMA hint
            ptr = VirtualAllocExNuma(GetCurrentProcess(), nullptr, size, 
                                    MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE, node_id);
        } else {
            ptr = _aligned_malloc(size, CacheConfig::cache_line_size);
        }
        #else
        // Fallback for non-NUMA systems
        ptr = aligned_alloc(CacheConfig::cache_line_size, size);
        #endif
        
        if (!ptr) {
            throw std::bad_alloc();
        }
        
        // Track allocation
        track_allocation(ptr, size, node_id);
        
        return ptr;
    }
    
    /**
     * Free memory allocated on a specific node
     */
    void free_from_node(void* ptr, int node_id = -1) {
        if (!ptr) return;
        
        #ifdef __linux__
        if (topology_.numa_available && node_id >= 0) {
            numa_free(ptr, get_allocation_size(ptr));
        } else {
            std::free(ptr);
        }
        #elif defined(_WIN32)
        if (node_id < topology_.total_nodes) {
            VirtualFreeEx(GetCurrentProcess(), ptr, 0, MEM_RELEASE);
        } else {
            _aligned_free(ptr);
        }
        #else
        std::free(ptr);
        #endif
        
        untrack_allocation(ptr);
    }
    
    /**
     * Set preferred NUMA node for current thread
     */
    void set_thread_node(int node_id) {
        topology_.current_node = node_id;
        
        #ifdef __linux__
        if (topology_.numa_available) {
            numa_run_on_node(node_id);
        }
        #elif defined(_WIN32)
        // Windows thread-to-node binding
        if (node_id < topology_.total_nodes) {
            SetThreadAffinityMask(GetCurrentThread(), 
                                 1ULL << topology_.nodes[node_id].cpu_cores[0]);
        }
        #endif
    }
    
    /**
     * Get current NUMA node
     */
    int get_current_node() const {
        return topology_.current_node;
    }
    
    /**
     * Get NUMA topology information
     */
    const NUMATopology& get_topology() const {
        return topology_;
    }
    
    /**
     * Migrate allocation to different NUMA node
     */
    void migrate_to_node(void* ptr, int target_node) {
        size_t size = get_allocation_size(ptr);
        int current_node = get_allocation_node(ptr);
        
        if (current_node == target_node) return;
        
        // Allocate new memory on target node
        void* new_ptr = allocate_on_node(size, target_node);
        
        // Copy data
        std::memcpy(new_ptr, ptr, size);
        
        // Free old memory
        free_from_node(ptr, current_node);
        
        // Update tracking (this would need to be implemented based on your allocation tracking)
    }
    
private:
    NUMATopology topology_;
    std::vector<std::pair<void*, size_t>> allocations_;
    std::mutex allocation_mutex_;
    
    void detect_numa_topology() {
        topology_.numa_available = false;
        topology_.total_nodes = 1;
        topology_.current_node = 0;
        
        #ifdef __linux__
        if (numa_available()) {
            topology_.numa_available = true;
            topology_.total_nodes = numa_num_configured_nodes();
            topology_.current_node = numa_node_of_cpu(0);
            
            for (int node = 0; node < topology_.total_nodes; ++node) {
                NUMATopology::Node numa_node;
                numa_node.node_id = node;
                numa_node.memory_size = numa_node_size(node);
                
                // Get CPU cores for this node
                struct bitmask* cpus = numa_alloc_cpumask();
                numa_node_to_cpus(node, cpus);
                for (size_t i = 0; i < cpus->size; ++i) {
                    if (numa_bitmask_isbitset(cpus, i)) {
                        numa_node.cpu_cores.push_back(static_cast<int>(i));
                    }
                }
                numa_bitmask_free(cpus);
                
                // Set default bandwidth and latency
                numa_node.bandwidth_gbps = 25.0; // Typical NUMA bandwidth
                numa_node.latency_ns = 30.0;     // Typical NUMA latency
                
                topology_.nodes.push_back(numa_node);
            }
        }
        #elif defined(_WIN32)
        // Windows NUMA detection
        DWORD numa_count = GetNumaHighestNodeNumber();
        if (numa_count > 0) {
            topology_.numa_available = true;
            topology_.total_nodes = numa_count + 1;
            topology_.current_node = 0;
            
            for (int node = 0; node <= numa_count; ++node) {
                NUMATopology::Node numa_node;
                numa_node.node_id = node;
                
                // Get memory size for node
                ULONGLONG memory_size = GetNumaAvailableMemoryNode(node);
                numa_node.memory_size = static_cast<size_t>(memory_size);
                
                // Get processor group information
                // This is simplified - real implementation would be more complex
                numa_node.cpu_cores.push_back(node * 8); // Simplified core assignment
                
                topology_.nodes.push_back(numa_node);
            }
        }
        #endif
        
        // Initialize nodes for single-node systems
        if (topology_.nodes.empty()) {
            NUMATopology::Node default_node;
            default_node.node_id = 0;
            default_node.memory_size = 8ULL * 1024 * 1024 * 1024; // 8GB default
            default_node.bandwidth_gbps = 25.0;
            default_node.latency_ns = 30.0;
            topology_.nodes.push_back(default_node);
        }
    }
    
    void track_allocation(void* ptr, size_t size, int node_id) {
        std::lock_guard<std::mutex> lock(allocation_mutex_);
        allocations_.push_back({ptr, size});
    }
    
    void untrack_allocation(void* ptr) {
        std::lock_guard<std::mutex> lock(allocation_mutex_);
        allocations_.erase(
            std::remove_if(allocations_.begin(), allocations_.end(),
                [ptr](const auto& alloc) { return alloc.first == ptr; }),
            allocations_.end());
    }
    
    size_t get_allocation_size(void* ptr) {
        std::lock_guard<std::mutex> lock(allocation_mutex_);
        auto it = std::find_if(allocations_.begin(), allocations_.end(),
            [ptr](const auto& alloc) { return alloc.first == ptr; });
        return (it != allocations_.end()) ? it->second : 0;
    }
    
    int get_allocation_node(void* ptr) {
        std::lock_guard<std::mutex> lock(allocation_mutex_);
        // This would need to be implemented based on how you track allocation metadata
        return 0; // Simplified
    }
    
    void cleanup_allocations() {
        std::lock_guard<std::mutex> lock(allocation_mutex_);
        for (auto& allocation : allocations_) {
            free_from_node(allocation.first);
        }
        allocations_.clear();
    }
};

// ============================================================================
// Cache-Friendly Data Structures
// ============================================================================

/**
 * Cache-optimized audio buffer with prefetching
 */
template<size_t NumChannels, size_t BufferSize>
class CacheOptimizedAudioBuffer {
public:
    static constexpr size_t num_channels = NumChannels;
    static constexpr size_t buffer_size = BufferSize;
    static constexpr size_t total_samples = NumChannels * BufferSize;
    
    CacheOptimizedAudioBuffer() {
        // Allocate cache-aligned memory
        buffer_ = CacheAlignedAllocator<float, CacheConfig::cache_line_size>()
                  .allocate(total_samples);
        clear();
    }
    
    ~CacheOptimizedAudioBuffer() {
        if (buffer_) {
            CacheAlignedAllocator<float, CacheConfig::cache_line_size>()
                .deallocate(buffer_, total_samples);
        }
    }
    
    // Copy and move operations
    CacheOptimizedAudioBuffer(const CacheOptimizedAudioBuffer& other) {
        buffer_ = CacheAlignedAllocator<float, CacheConfig::cache_line_size>()
                  .allocate(total_samples);
        std::copy_n(other.buffer_, total_samples, buffer_);
    }
    
    CacheOptimizedAudioBuffer& operator=(const CacheOptimizedAudioBuffer& other) {
        if (this != &other) {
            std::copy_n(other.buffer_, total_samples, buffer_);
        }
        return *this;
    }
    
    // Cache-aware write operations
    void write_channel_cache_aware(size_t channel, const float* data, size_t count) {
        if (channel >= NumChannels || count > BufferSize) return;
        
        size_t base_offset = channel * BufferSize;
        size_t aligned_count = (count + 7) & ~7; // Align to 8 samples for SIMD
        
        // Prefetch destination cache lines
        HardwarePrefetcher prefetcher;
        prefetcher.prefetch_for_write(buffer_ + base_offset, aligned_count);
        
        // Copy data with cache considerations
        for (size_t i = 0; i < count; ++i) {
            buffer_[base_offset + i] = data[i];
        }
    }
    
    // Cache-aware read operations
    bool read_channel_cache_aware(size_t channel, float* data, size_t count) const {
        if (channel >= NumChannels || count > BufferSize) return false;
        
        size_t base_offset = channel * BufferSize;
        size_t aligned_count = (count + 7) & ~7; // Align to 8 samples for SIMD
        
        // Prefetch source cache lines
        HardwarePrefetcher prefetcher;
        prefetcher.prefetch_sequential(buffer_ + base_offset, aligned_count);
        
        // Copy data with cache considerations
        for (size_t i = 0; i < count; ++i) {
            data[i] = buffer_[base_offset + i];
        }
        
        return true;
    }
    
    // Element access with bounds checking
    float& at(size_t channel, size_t sample) {
        if (channel >= NumChannels || sample >= BufferSize) {
            throw std::out_of_range("Audio buffer access out of range");
        }
        return buffer_[channel * BufferSize + sample];
    }
    
    const float& at(size_t channel, size_t sample) const {
        if (channel >= NumChannels || sample >= BufferSize) {
            throw std::out_of_range("Audio buffer access out of range");
        }
        return buffer_[channel * BufferSize + sample];
    }
    
    // Raw pointer access
    float* get_channel_data(size_t channel) {
        return &buffer_[channel * BufferSize];
    }
    
    const float* get_channel_data(size_t channel) const {
        return &buffer_[channel * BufferSize];
    }
    
    // Utility operations
    void clear() {
        std::fill_n(buffer_, total_samples, 0.0f);
    }
    
    void zero_channel(size_t channel) {
        if (channel < NumChannels) {
            size_t base_offset = channel * BufferSize;
            std::fill_n(&buffer_[base_offset], BufferSize, 0.0f);
        }
    }
    
    void copy_channel_to(size_t source_channel, size_t dest_channel) {
        if (source_channel < NumChannels && dest_channel < NumChannels) {
            size_t source_offset = source_channel * BufferSize;
            size_t dest_offset = dest_channel * BufferSize;
            std::copy_n(&buffer_[source_offset], BufferSize, &buffer_[dest_offset]);
        }
    }
    
    // Cache statistics and optimization
    struct CacheStats {
        size_t cache_misses = 0;
        size_t cache_hits = 0;
        size_t prefetch_hits = 0;
        double hit_ratio = 0.0;
    };
    
    void reset_cache_stats() {
        cache_stats_ = CacheStats{};
    }
    
    const CacheStats& get_cache_stats() const {
        return cache_stats_;
    }
    
private:
    float* buffer_;
    CacheStats cache_stats_;
};

// ============================================================================
// Memory Access Pattern Optimization
// ============================================================================

/**
 * Cache-blocked operations for better memory locality
 */
class CacheBlockProcessor {
public:
    struct BlockConfig {
        size_t l1_block_size = CacheConfig::l1_cache_size / sizeof(float); // Samples
        size_t l2_block_size = CacheConfig::l2_cache_size / sizeof(float); // Samples
        size_t l3_block_size = CacheConfig::l3_cache_size / sizeof(float); // Samples
    };
    
    CacheBlockProcessor(const BlockConfig& config = BlockConfig{}) : config_(config) {}
    
    /**
     * Cache-blocked audio buffer copy
     */
    void copy_cache_blocked(const float* source, float* dest, size_t count) {
        // Process in L1 cache-sized blocks
        for (size_t i = 0; i < count; i += config_.l1_block_size) {
            size_t block_size = std::min(config_.l1_block_size, count - i);
            
            // Prefetch next block
            if (i + block_size < count) {
                HardwarePrefetcher prefetcher;
                prefetcher.prefetch_sequential(source + i + block_size, 
                                             std::min(config_.l2_block_size, count - i - block_size));
                prefetcher.prefetch_sequential(dest + i + block_size, 
                                             std::min(config_.l2_block_size, count - i - block_size));
            }
            
            // Copy current block
            std::copy_n(source + i, block_size, dest + i);
        }
    }
    
    /**
     * Cache-blocked FIR filter application
     */
    void apply_fir_filter_cache_blocked(const float* input, const float* coefficients,
                                       float* output, size_t num_samples, size_t filter_length) {
        size_t block_size = std::min(config_.l1_block_size, num_samples / 2);
        
        for (size_t i = 0; i + filter_length < num_samples; i += block_size) {
            size_t current_block_size = std::min(block_size, num_samples - i - filter_length);
            
            // Prefetch input data
            HardwarePrefetcher prefetcher;
            prefetcher.prefetch_sequential(input + i, current_block_size + filter_length);
            prefetcher.prefetch_sequential(coefficients, filter_length);
            
            // Apply filter to current block
            for (size_t j = 0; j < current_block_size; ++j) {
                float sum = 0.0f;
                for (size_t k = 0; k < filter_length; ++k) {
                    sum += input[i + j + k] * coefficients[k];
                }
                output[i + j] = sum;
            }
        }
    }
    
    /**
     * Cache-friendly matrix multiplication for spectral processing
     */
    void matrix_multiply_cache_blocked(const float* A, const float* B, float* C,
                                      size_t m, size_t n, size_t k) {
        size_t block_size = std::min(config_.l2_block_size / n, m);
        
        for (size_t ii = 0; ii < m; ii += block_size) {
            size_t i_block = std::min(block_size, m - ii);
            
            for (size_t kk = 0; kk < k; kk += block_size) {
                size_t k_block = std::min(block_size, k - kk);
                
                for (size_t jj = 0; jj < n; jj += block_size) {
                    size_t j_block = std::min(block_size, n - jj);
                    
                    // Compute block multiplication
                    for (size_t i = 0; i < i_block; ++i) {
                        for (size_t k_idx = 0; k_idx < k_block; ++k_idx) {
                            float a_ik = A[(ii + i) * k + (kk + k_idx)];
                            
                            for (size_t j = 0; j < j_block; ++j) {
                                C[(ii + i) * n + (jj + j)] += 
                                    a_ik * B[(kk + k_idx) * n + (jj + j)];
                            }
                        }
                    }
                }
            }
        }
    }
    
private:
    BlockConfig config_;
};

// ============================================================================
// Performance Monitoring and Statistics
// ============================================================================

/**
 * Cache performance monitor
 */
class CachePerformanceMonitor {
public:
    struct CacheMetrics {
        size_t cache_misses = 0;
        size_t cache_hits = 0;
        size_t prefetch_requests = 0;
        size_t prefetch_hits = 0;
        size_t memory_bandwidth_mbps = 0;
        double l1_hit_ratio = 0.0;
        double l2_hit_ratio = 0.0;
        double l3_hit_ratio = 0.0;
        double avg_memory_latency_ns = 0.0;
    };
    
    void start_monitoring() {
        start_time_ = std::chrono::high_resolution_clock::now();
        metrics_ = CacheMetrics{};
    }
    
    void record_cache_miss(size_t count = 1) {
        metrics_.cache_misses += count;
    }
    
    void record_cache_hit(size_t count = 1) {
        metrics_.cache_hits += count;
    }
    
    void record_prefetch_request(size_t count = 1) {
        metrics_.prefetch_requests += count;
    }
    
    void record_prefetch_hit(size_t count = 1) {
        metrics_.prefetch_hits += count;
    }
    
    void record_memory_bandwidth(double bandwidth_mbps) {
        metrics_.memory_bandwidth_mbps += static_cast<size_t>(bandwidth_mbps);
    }
    
    CacheMetrics get_metrics() {
        update_ratios();
        return metrics_;
    }
    
    void reset_metrics() {
        metrics_ = CacheMetrics{};
        start_monitoring();
    }
    
    double get_hit_ratio() const {
        size_t total_accesses = metrics_.cache_hits + metrics_.cache_misses;
        return total_accesses > 0 ? 
               static_cast<double>(metrics_.cache_hits) / total_accesses : 0.0;
    }
    
private:
    std::chrono::high_resolution_clock::time_point start_time_;
    CacheMetrics metrics_;
    
    void update_ratios() const {
        size_t total_accesses = metrics_.cache_hits + metrics_.cache_misses;
        if (total_accesses > 0) {
            // These would be measured more precisely in a real implementation
            metrics_.l1_hit_ratio = get_hit_ratio() * 0.9;
            metrics_.l2_hit_ratio = get_hit_ratio() * 0.7;
            metrics_.l3_hit_ratio = get_hit_ratio() * 0.4;
        }
    }
};

// ============================================================================
// Global Cache Optimization Manager
// ============================================================================

/**
 * Central coordinator for all cache optimization features
 */
class CacheOptimizationManager {
public:
    static CacheOptimizationManager& get_instance() {
        static CacheOptimizationManager instance;
        return instance;
    }
    
    /**
     * Initialize cache optimization system
     */
    void initialize() {
        if (!initialized_) {
            numa_manager_ = std::make_unique<NUMAMemoryManager>();
            monitor_ = std::make_unique<CachePerformanceMonitor>();
            initialized_ = true;
        }
    }
    
    /**
     * Shutdown cache optimization system
     */
    void shutdown() {
        initialized_ = false;
    }
    
    /**
     * Get NUMA memory manager
     */
    NUMAMemoryManager& get_numa_manager() {
        initialize();
        return *numa_manager_;
    }
    
    /**
     * Get hardware prefetcher
     */
    HardwarePrefetcher& get_prefetcher() {
        initialize();
        return prefetcher_;
    }
    
    /**
     * Get cache performance monitor
     */
    CachePerformanceMonitor& get_monitor() {
        initialize();
        return *monitor_;
    }
    
    /**
     * Get cache block processor
     */
    CacheBlockProcessor& get_block_processor() {
        initialize();
        return block_processor_;
    }
    
    /**
     * Check if optimization is enabled
     */
    bool is_enabled() const {
        return initialized_;
    }
    
    /**
     * Enable/disable specific optimizations
     */
    void set_optimization_flags(bool prefetching, bool numa_aware, bool cache_blocking) {
        optimization_flags_.prefetching = prefetching;
        optimization_flags_.numa_aware = numa_aware;
        optimization_flags_.cache_blocking = cache_blocking;
    }
    
    struct OptimizationFlags {
        bool prefetching = true;
        bool numa_aware = true;
        bool cache_blocking = true;
    } optimization_flags_;
    
private:
    bool initialized_ = false;
    std::unique_ptr<NUMAMemoryManager> numa_manager_;
    std::unique_ptr<CachePerformanceMonitor> monitor_;
    HardwarePrefetcher prefetcher_;
    CacheBlockProcessor block_processor_;
};

// Convenience functions
inline CacheOptimizationManager& get_cache_optimization() {
    return CacheOptimizationManager::get_instance();
}

inline HardwarePrefetcher& get_hardware_prefetcher() {
    return get_cache_optimization().get_prefetcher();
}

} // namespace cache
} // namespace performance
} // namespace vital
