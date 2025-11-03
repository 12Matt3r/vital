#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <JuceHeader.h>

namespace vital {
namespace ui {
namespace core {

/**
 * @brief UI Manager coordinates all UI components and systems
 * 
 * Manages:
 * - Component lifecycle and registration
 * - Rendering pipeline
 * - Event distribution
 * - Performance monitoring
 * - Accessibility integration
 */
class VITAL_MODERN_UI_API UIManager {
public:
    /**
     * @brief Constructor
     * @param audio_processor Reference to audio processor
     */
    explicit UIManager(juce::AudioProcessor& audio_processor);

    /**
     * @brief Destructor
     */
    ~UIManager();

    //==============================================================================
    // Component Management
    /**
     * @brief Register component with UI manager
     * @param component Component to register
     */
    void registerComponent(Component* component);

    /**
     * @brief Unregister component from UI manager
     * @param component Component to unregister
     */
    void unregisterComponent(Component* component);

    /**
     * @brief Get all registered components
     */
    std::vector<Component*> getRegisteredComponents() const;

    /**
     * @brief Find component by ID
     * @param id Component ID
     * @return Component pointer or nullptr
     */
    Component* findComponent(const juce::String& id) const;

    /**
     * @brief Find components by type
     * @param type Component type name
     * @return Vector of matching components
     */
    std::vector<Component*> findComponentsByType(const juce::String& type) const;

    //==============================================================================
    // Rendering Pipeline
    /**
     * @brief Begin rendering frame
     */
    void beginFrame();

    /**
     * @brief End rendering frame
     */
    void endFrame();

    /**
     * @brief Render all dirty components
     * @param g Graphics context
     */
    void render(juce::Graphics& g);

    /**
     * @brief Update all components (non-rendering logic)
     */
    void update();

    /**
     * @brief Mark component as needing redraw
     * @param component Component to mark
     */
    void markDirty(Component* component);

    /**
     * @brief Get number of dirty components
     */
    int getDirtyComponentCount() const { return static_cast<int>(dirty_components_.size()); }

    //==============================================================================
    // Event Handling
    /**
     * @brief Broadcast mouse event to all components
     * @param event JUCE mouse event
     * @param component Source component (or nullptr for global)
     */
    void broadcastMouseEvent(const juce::MouseEvent& event, Component* component = nullptr);

    /**
     * @brief Broadcast keyboard event to focused component
     * @param key Key press event
     * @return True if handled
     */
    bool broadcastKeyEvent(const juce::KeyPress& key);

    /**
     * @brief Broadcast touch event
     * @param event Touch event
     * @param component Source component
     */
    void broadcastTouchEvent(const juce::TouchEvent& event, Component* component = nullptr);

    /**
     * @brief Broadcast focus change event
     * @param component Component that gained/lost focus
     * @param gained Whether focus was gained (true) or lost (false)
     */
    void broadcastFocusEvent(Component* component, bool gained);

    //==============================================================================
    // Performance Monitoring
    /**
     * @brief Enable performance monitoring
     * @param enabled Whether monitoring is enabled
     */
    void enablePerformanceMonitoring(bool enabled);

    /**
     * @brief Get current frame rate
     */
    float getFrameRate() const { return frame_rate_.load(); }

    /**
     * @brief Get average frame time (ms)
     */
    float getAverageFrameTime() const { return average_frame_time_.load(); }

    /**
     * @brief Get peak frame time (ms)
     */
    float getPeakFrameTime() const { return peak_frame_time_.load(); }

    /**
     * @brief Get component count
     */
    int getComponentCount() const { return static_cast<int>(components_.size()); }

    /**
     * @brief Get memory usage estimate (bytes)
     */
    size_t getMemoryUsage() const { return memory_usage_.load(); }

    /**
     * @brief Get GPU memory usage (bytes)
     */
    size_t getGpuMemoryUsage() const { return gpu_memory_usage_.load(); }

    //==============================================================================
    // Configuration
    /**
     * @brief Set maximum render time budget (ms)
     * @param budget_ms Maximum time budget
     */
    void setRenderBudgetMs(float budget_ms);

    /**
     * @brief Get maximum render time budget
     */
    float getRenderBudgetMs() const { return render_budget_ms_; }

    /**
     * @brief Set maximum components to render per frame
     * @param max_components Maximum components
     */
    void setMaxRenderComponents(int max_components);

    /**
     * @brief Get maximum components to render per frame
     */
    int getMaxRenderComponents() const { return max_render_components_; }

    /**
     * @brief Enable/disable multi-threading
     * @param enabled Whether multi-threading is enabled
     */
    void enableMultiThreading(bool enabled);

    /**
     * @brief Check if multi-threading is enabled
     */
    bool isMultiThreadingEnabled() const { return multi_threading_enabled_; }

    //==============================================================================
    // Accessibility Integration
    /**
     * @brief Get all focusable components
     * @return Vector of focusable components
     */
    std::vector<Component*> getFocusableComponents() const;

    /**
     * @brief Set focused component
     * @param component Component to focus (or nullptr to clear)
     * @param cause Reason for focus change
     */
    void setFocusedComponent(Component* component, FocusChangeType cause);

    /**
     * @brief Get currently focused component
     */
    Component* getFocusedComponent() const { return focused_component_; }

    /**
     * @brief Move focus to next component
     * @param forward Direction (true = forward, false = backward)
     */
    void moveFocus(bool forward = true);

    /**
     * @brief Move focus to specific component
     * @param component Target component
     */
    void moveFocusToComponent(Component* component);

    //==============================================================================
    // Lifecycle
    /**
     * @brief Initialize UI manager
     */
    void initialize();

    /**
     * @brief Shutdown UI manager
     */
    void shutdown();

    /**
     * @brief Check if initialized
     */
    bool isInitialized() const { return initialized_.load(); }

private:
    //==============================================================================
    // Private member variables
    juce::AudioProcessor& audio_processor_;
    mutable std::mutex components_mutex_;
    mutable std::mutex dirty_mutex_;
    mutable std::mutex focus_mutex_;

    // Component tracking
    std::vector<Component*> components_;
    std::unordered_map<juce::String, Component*> component_id_map_;
    std::vector<Component*> dirty_components_;
    std::vector<Component*> focusable_components_;

    // Focus management
    Component* focused_component_ = nullptr;
    Component* previously_focused_component_ = nullptr;

    // Performance tracking
    mutable std::atomic<float> frame_rate_{60.0f};
    mutable std::atomic<float> average_frame_time_{16.67f};
    mutable std::atomic<float> peak_frame_time_{0.0f};
    mutable std::atomic<size_t> memory_usage_{0};
    mutable std::atomic<size_t> gpu_memory_usage_{0};
    mutable std::atomic<bool> initialized_{false};

    // Configuration
    float render_budget_ms_ = 16.67f;  // ~60 FPS
    int max_render_components_ = 1000;
    bool multi_threading_enabled_ = true;

    // Timing
    juce::uint64 last_frame_time_ = 0;
    juce::uint64 frame_count_ = 0;
    std::chrono::steady_clock::time_point start_time_;

    //==============================================================================
    // Private methods
    void updatePerformanceMetrics();
    void updateMemoryUsage();
    void processDirtyComponents();
    void sortFocusableComponents();
    Component* findNextFocusableComponent(bool forward, Component* current = nullptr);
    
    // Event filtering
    bool shouldComponentReceiveEvent(Component* component, const juce::MouseEvent& event);
    bool shouldComponentReceiveEvent(Component* component, const juce::KeyPress& key);
    
    // Threading helpers
    void parallelComponentUpdate();
    void parallelComponentRender(juce::Graphics& g);
    
    // Cleanup
    void cleanupComponents();
    void updateComponentRegistry();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(UIManager)
};

} // namespace core
} // namespace ui
} // namespace vital
