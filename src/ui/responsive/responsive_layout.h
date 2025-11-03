#pragma once

#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <JuceHeader.h>
#include "../core/component.h"

namespace vital {
namespace ui {
namespace responsive {

/**
 * @brief Responsive breakpoint definition
 */
struct Breakpoint {
    juce::String name;           // Breakpoint identifier
    float min_width = 0.0f;      // Minimum width in pixels
    float max_width = -1.0f;     // Maximum width (-1 for no limit)
    juce::String description;    // Description of breakpoint
    
    Breakpoint() = default;
    Breakpoint(const juce::String& name, float min, float max, const juce::String& desc = "")
        : name(name), min_width(min), max_width(max), description(desc) {}
};

/**
 * @brief Responsive state information
 */
struct ResponsiveState {
    juce::String current_breakpoint;
    float screen_width = 0.0f;
    float screen_height = 0.0f;
    float scale_factor = 1.0f;
    float device_pixel_ratio = 1.0f;
    bool is_touch_device = false;
    bool is_mobile = false;
    bool is_tablet = false;
    bool is_desktop = false;
    juce::String orientation = "portrait";  // "portrait" or "landscape"
    
    ResponsiveState() = default;
    ResponsiveState(const juce::String& bp, float width, float height, float scale, float dpr)
        : current_breakpoint(bp), screen_width(width), screen_height(height), 
          scale_factor(scale), device_pixel_ratio(dpr) {}
};

/**
 * @brief Layout rule for responsive behavior
 */
struct LayoutRule {
    juce::String breakpoint_name;  // Which breakpoint this rule applies to
    float flex_grow = 1.0f;        // Flex grow factor
    float flex_shrink = 1.0f;      // Flex shrink factor
    juce::String width_rule = "auto";    // "auto", "100%", "fixed", "min", "max"
    float width_value = -1.0f;     // Fixed width value
    juce::String height_rule = "auto";   // "auto", "100%", "fixed"
    float height_value = -1.0f;    // Fixed height value
    juce::String position_rule = "static"; // "static", "relative", "absolute", "fixed"
    juce::Point<float> position_value;    // Position coordinates
    juce::String display_rule = "block";  // "block", "inline", "none", "flex"
    juce::String visibility_rule = "visible"; // "visible", "hidden"
    float margin = 0.0f;           // Margin value
    juce::Rectangle<float> padding; // Padding rectangle
    juce::String z_index;          // Z-index value
    
    LayoutRule() = default;
    LayoutRule(const juce::String& breakpoint)
        : breakpoint_name(breakpoint) {}
};

/**
 * @brief Touch gesture types
 */
enum class TouchGesture {
    Tap,
    DoubleTap,
    LongPress,
    Pan,
    Swipe,
    Pinch,
    Rotate,
    Unknown
};

/**
 * @brief Touch event information
 */
struct TouchEvent {
    int finger_id = -1;
    juce::Point<float> position;
    juce::Point<float> previous_position;
    float pressure = 1.0f;
    float timestamp = 0.0f;
    juce::String phase;  // "began", "moved", "stationary", "ended", "cancelled"
    TouchGesture gesture = TouchGesture::Unknown;
    
    TouchEvent() = default;
    TouchEvent(int id, const juce::Point<float>& pos, const juce::String& ph)
        : finger_id(id), position(pos), phase(ph) {}
};

/**
 * @brief ResponsiveLayoutManager - Manages responsive UI layouts and touch interactions
 * 
 * Provides comprehensive responsive design capabilities:
 * - Breakpoint-based layout system
 * - Touch gesture recognition and handling
 * - Dynamic scaling and DPI awareness
 * - Mobile and tablet optimization
 * - Adaptive component sizing
 * - Touch-friendly interfaces
 */
class VITAL_MODERN_UI_API ResponsiveLayoutManager {
public:
    /**
     * @brief Constructor
     */
    ResponsiveLayoutManager();

    /**
     * @brief Destructor
     */
    ~ResponsiveLayoutManager();

    //==============================================================================
    // Breakpoint Management
    /**
     * @brief Add responsive breakpoint
     * @param breakpoint Breakpoint definition
     */
    void addBreakpoint(const Breakpoint& breakpoint);

    /**
     * @brief Remove breakpoint
     * @param breakpoint_name Name of breakpoint to remove
     */
    void removeBreakpoint(const juce::String& breakpoint_name);

    /**
     * @brief Update breakpoint
     * @param breakpoint_name Name of breakpoint to update
     * @param new_breakpoint New breakpoint definition
     */
    void updateBreakpoint(const juce::String& breakpoint_name, const Breakpoint& new_breakpoint);

    /**
     * @brief Get breakpoint by name
     * @param breakpoint_name Name of breakpoint
     * @return Breakpoint or default if not found
     */
    Breakpoint getBreakpoint(const juce::String& breakpoint_name) const;

    /**
     * @brief Get all breakpoints
     */
    std::vector<Breakpoint> getAllBreakpoints() const;

    /**
     * @brief Check if breakpoint exists
     * @param breakpoint_name Name to check
     */
    bool hasBreakpoint(const juce::String& breakpoint_name) const;

    /**
     * @brief Get current active breakpoint
     */
    const Breakpoint& getCurrentBreakpoint() const;

    /**
     * @brief Determine active breakpoint by width
     * @param width Screen width
     * @param height Screen height
     * @return Active breakpoint
     */
    Breakpoint getActiveBreakpointBySize(float width, float height) const;

    //==============================================================================
    // Responsive State
    /**
     * @ Update responsive state
     * @param state New responsive state
     */
    void setResponsiveState(const ResponsiveState& state);

    /**
     * @brief Get current responsive state
     */
    const ResponsiveState& getResponsiveState() const { return state_; }

    /**
     * @brief Update screen size
     * @param width Screen width
     * @param height Screen height
     * @param pixel_ratio Device pixel ratio
     */
    void updateScreenSize(float width, float height, float pixel_ratio = 1.0f);

    /**
     * @brief Set device type
     * @param is_touch Touch device flag
     * @param is_mobile Mobile device flag
     * @param is_tablet Tablet device flag
     */
    void setDeviceType(bool is_touch, bool is_mobile, bool is_tablet);

    /**
     * @brief Set orientation
     * @param orientation "portrait" or "landscape"
     */
    void setOrientation(const juce::String& orientation);

    /**
     * @brief Get orientation
     */
    juce::String getOrientation() const { return state_.orientation; }

    /**
     * @brief Check if current layout is mobile
     */
    bool isMobileLayout() const { return state_.is_mobile; }

    /**
     * @brief Check if current layout is tablet
     */
    bool isTabletLayout() const { return state_.is_tablet; }

    /**
     * @brief Check if current layout is desktop
     */
    bool isDesktopLayout() const { return state_.is_desktop; }

    //==============================================================================
    // Layout Rules
    /**
     * @brief Add layout rule for component
     * @param component Component to apply rule to
     * @param rule Layout rule to apply
     */
    void addLayoutRule(Component* component, const LayoutRule& rule);

    /**
     * @brief Remove layout rule
     * @param component Component to remove rule from
     * @param breakpoint_name Breakpoint name (empty for all)
     */
    void removeLayoutRule(Component* component, const juce::String& breakpoint_name = "");

    /**
     * @brief Update layout rule
     * @param component Component to update
     * @param breakpoint_name Breakpoint name
     * @param rule New layout rule
     */
    void updateLayoutRule(Component* component, const juce::String& breakpoint_name, 
                         const LayoutRule& rule);

    /**
     * @brief Get layout rule for component
     * @param component Component to query
     * @param breakpoint_name Breakpoint name (empty for current)
     * @return Layout rule or default if not found
     */
    LayoutRule getLayoutRule(Component* component, const juce::String& breakpoint_name = "") const;

    /**
     * @brief Get all layout rules for component
     * @param component Component to query
     */
    std::vector<LayoutRule> getAllLayoutRules(Component* component) const;

    /**
     * @brief Clear all layout rules for component
     * @param component Component to clear rules from
     */
    void clearLayoutRules(Component* component);

    //==============================================================================
    // Component Sizing and Positioning
    /**
     * @brief Calculate responsive dimensions for component
     * @param component Component to calculate for
     * @return Responsive dimensions
     */
    juce::Rectangle<float> calculateResponsiveBounds(Component* component) const;

    /**
     * @brief Apply responsive layout to component
     * @param component Component to apply layout to
     */
    void applyResponsiveLayout(Component* component);

    /**
     * @brief Apply responsive layout to all registered components
     */
    void applyResponsiveLayoutToAll();

    /**
     * @brief Register component for responsive layout
     * @param component Component to register
     */
    void registerComponent(Component* component);

    /**
     * @brief Unregister component from responsive layout
     * @param component Component to unregister
     */
    void unregisterComponent(Component* component);

    /**
     * @brief Get registered components
     */
    std::vector<Component*> getRegisteredComponents() const;

    //==============================================================================
    // Touch Interface Optimization
    /**
     * @ Enable touch optimization
     * @param enabled Whether touch optimization is enabled
     */
    void setTouchOptimizationEnabled(bool enabled);

    /**
     * @brief Check if touch optimization is enabled
     */
    bool isTouchOptimizationEnabled() const { return touch_optimization_enabled_; }

    /**
     * @brief Set minimum touch target size
     * @param size Minimum touch target size in pixels
     */
    void setMinimumTouchTargetSize(float size);

    /**
     * @brief Get minimum touch target size
     */
    float getMinimumTouchTargetSize() const { return min_touch_target_size_; }

    /**
     * @brief Set touch feedback enabled
     * @param enabled Whether touch feedback is enabled
     */
    void setTouchFeedbackEnabled(bool enabled);

    /**
     * @brief Check if touch feedback is enabled
     */
    bool isTouchFeedbackEnabled() const { return touch_feedback_enabled_; }

    /**
     * @brief Set haptic feedback intensity
     * @param intensity Haptic intensity (0.0 to 1.0)
     */
    void setHapticFeedbackIntensity(float intensity);

    /**
     * @brief Get haptic feedback intensity
     */
    float getHapticFeedbackIntensity() const { return haptic_intensity_; }

    //==============================================================================
    // Touch Gesture Recognition
    /**
     * @brief Register touch event
     * @param event Touch event to process
     */
    void processTouchEvent(const TouchEvent& event);

    /**
     * @brief Register touch gesture handler
     * @param gesture_type Gesture type to handle
     * @param handler Gesture handler function
     */
    void registerGestureHandler(TouchGesture gesture_type,
                               std::function<void(const TouchEvent&)> handler);

    /**
     * @brief Unregister gesture handler
     * @param gesture_type Gesture type
     */
    void unregisterGestureHandler(TouchGesture gesture_type);

    /**
     * @brief Get active gestures
     */
    std::vector<TouchEvent> getActiveGestures() const;

    /**
     * @brief Clear all active gestures
     */
    void clearActiveGestures();

    //==============================================================================
    // Dynamic Scaling
    /**
     * @brief Enable dynamic scaling
     * @param enabled Whether dynamic scaling is enabled
     */
    void setDynamicScalingEnabled(bool enabled);

    /**
     * @brief Check if dynamic scaling is enabled
     */
    bool isDynamicScalingEnabled() const { return dynamic_scaling_enabled_; }

    /**
     * @brief Set scale factor
     * @param factor Scale factor
     */
    void setScaleFactor(float factor);

    /**
     * @brief Get scale factor
     */
    float getScaleFactor() const { return state_.scale_factor; }

    /**
     * @brief Calculate scaled value
     * @param value Value to scale
     * @param property Property type for scaling rules
     * @return Scaled value
     */
    float calculateScaledValue(float value, const juce::String& property) const;

    /**
     * @brief Get scaling rule for property
     * @param property Property name
     * @return Scaling rule or default if not found
     */
    juce::String getScalingRule(const juce::String& property) const;

    /**
     * @brief Set scaling rule for property
     * @param property Property name
     * @param rule Scaling rule
     */
    void setScalingRule(const juce::String& property, const juce::String& rule);

    //==============================================================================
    // Mobile and Tablet Optimization
    /**
     * @brief Apply mobile optimizations
     */
    void applyMobileOptimizations();

    /**
     * @brief Apply tablet optimizations
     */
    void applyTabletOptimizations();

    /**
     * @brief Apply desktop optimizations
     */
    void applyDesktopOptimizations();

    /**
     * @brief Get optimized component size for current device
     * @param component Component to optimize
     * @return Optimized size
     */
    juce::Point<float> getOptimizedComponentSize(Component* component) const;

    /**
     * @brief Get optimized spacing for current device
     * @param base_spacing Base spacing value
     * @return Optimized spacing
     */
    float getOptimizedSpacing(float base_spacing) const;

    /**
     * @brief Get optimized font size for current device
     * @param base_font_size Base font size
     * @return Optimized font size
     */
    float getOptimizedFontSize(float base_font_size) const;

    //==============================================================================
    // Performance
    /**
     * @brief Enable performance monitoring
     * @param enabled Whether monitoring is enabled
     */
    void setPerformanceMonitoringEnabled(bool enabled);

    /**
     * @brief Check if performance monitoring is enabled
     */
    bool isPerformanceMonitoringEnabled() const { return performance_monitoring_enabled_; }

    /**
     * @brief Get layout calculation time
     */
    float getLayoutCalculationTime() const { return layout_calculation_time_ms_; }

    /**
     * @brief Get touch processing time
     */
    float getTouchProcessingTime() const { return touch_processing_time_ms_; }

    /**
     * @brief Get optimization cache hit ratio
     */
    float getCacheHitRatio() const { return cache_hit_ratio_; }

    //==============================================================================
    // Lifecycle
    /**
     * @brief Initialize responsive layout manager
     */
    void initialize();

    /**
     * @brief Shutdown responsive layout manager
     */
    void shutdown();

    /**
     * @brief Check if initialized
     */
    bool isInitialized() const { return initialized_; }

    /**
     * @brief Update all responsive layouts (called each frame)
     */
    void update();

private:
    //==============================================================================
    // Private member variables
    std::vector<Breakpoint> breakpoints_;
    Breakpoint current_breakpoint_;
    ResponsiveState state_;
    bool initialized_ = false;

    // Component tracking
    std::vector<Component*> registered_components_;
    std::unordered_map<Component*, std::vector<LayoutRule>> component_rules_;

    // Touch optimization
    bool touch_optimization_enabled_ = true;
    float min_touch_target_size_ = 44.0f;  // Material Design recommendation
    bool touch_feedback_enabled_ = true;
    float haptic_intensity_ = 0.5f;

    // Dynamic scaling
    bool dynamic_scaling_enabled_ = true;
    std::unordered_map<juce::String, juce::String> scaling_rules_;

    // Gesture recognition
    std::unordered_map<TouchGesture, std::function<void(const TouchEvent&)>> gesture_handlers_;
    std::vector<TouchEvent> active_gestures_;
    std::vector<TouchEvent> gesture_history_;
    mutable std::mutex gesture_mutex_;

    // Performance monitoring
    bool performance_monitoring_enabled_ = false;
    float layout_calculation_time_ms_ = 0.0f;
    float touch_processing_time_ms_ = 0.0f;
    float cache_hit_ratio_ = 0.0f;
    
    // Caching
    mutable std::unordered_map<juce::String, juce::Rectangle<float>> layout_cache_;
    mutable std::mutex cache_mutex_;

    //==============================================================================
    // Private methods
    void initializeDefaultBreakpoints();
    void updateCurrentBreakpoint();
    void processResponsiveUpdates();

    // Touch gesture recognition
    void recognizeGesture(const TouchEvent& event);
    void recognizeTapGesture(const TouchEvent& event);
    void recognizeLongPressGesture(const TouchEvent& event);
    void recognizePanGesture(const TouchEvent& event);
    void recognizePinchGesture(const TouchEvent& event);
    void recognizeSwipeGesture(const TouchEvent& event);

    // Layout calculation
    juce::Rectangle<float> calculateLayoutFromRule(Component* component, const LayoutRule& rule) const;
    float calculateWidthFromRule(Component* component, const LayoutRule& rule) const;
    float calculateHeightFromRule(Component* component, const LayoutRule& rule) const;
    juce::Point<float> calculatePositionFromRule(Component* component, const LayoutRule& rule) const;

    // Mobile/tablet optimizations
    void optimizeForMobile(Component* component);
    void optimizeForTablet(Component* component);
    void optimizeForDesktop(Component* component);

    // Scaling calculations
    float applyScalingRule(float value, const juce::String& rule) const;
    juce::String getResponsiveValue(const juce::String& value, const Breakpoint& breakpoint) const;

    // Performance optimization
    void updatePerformanceMetrics();
    void clearLayoutCache();
    bool canUseCachedLayout(Component* component, const juce::String& cache_key) const;
    void cacheLayout(Component* component, const juce::String& cache_key, const juce::Rectangle<float>& bounds);

    // Utility methods
    bool componentExists(Component* component) const;
    Breakpoint findBreakpointForSize(float width, float height) const;
    LayoutRule mergeRules(const std::vector<LayoutRule>& rules) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ResponsiveLayoutManager)
};

} // namespace responsive
} // namespace ui
} // namespace vital
