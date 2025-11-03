#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>
#include <JuceHeader.h>

namespace vital {
namespace ui {
namespace core {

/**
 * @brief Layout direction
 */
enum class LayoutDirection {
    Row,      // Horizontal layout
    Column,   // Vertical layout
    Grid      // Grid layout
};

/**
 * @brief Alignment options
 */
enum class LayoutAlignment {
    Start,     // Align to start
    Center,    // Center alignment
    End,       // Align to end
    Stretch,   // Stretch to fill
    SpaceAround,  // Space around items
    SpaceBetween, // Space between items
    SpaceEvenly    // Space evenly
};

/**
 * @brief Layout constraint
 */
struct LayoutConstraint {
    float flex = 1.0f;           // Flex grow/shrink factor
    float min_size = 0.0f;       // Minimum size
    float max_size = -1.0f;      // Maximum size (-1 means no limit)
    bool fill_space = false;     // Whether to fill available space
    float aspect_ratio = -1.0f;  // Aspect ratio constraint

    LayoutConstraint() = default;
    LayoutConstraint(float flex_factor) : flex(flex_factor) {}
};

/**
 * @brief Layout item
 */
struct LayoutItem {
    Component* component = nullptr;
    LayoutConstraint constraint;
    LayoutAlignment alignment = LayoutAlignment::Stretch;
    juce::Rectangle<float> bounds;
    juce::Rectangle<float> final_bounds;
    bool dirty = true;

    LayoutItem() = default;
    LayoutItem(Component* comp, const LayoutConstraint& constr)
        : component(comp), constraint(constr) {}
};

/**
 * @brief Layout manager for responsive UI layouts
 */
class VITAL_MODERN_UI_API LayoutManager {
public:
    /**
     * @brief Constructor
     */
    LayoutManager();

    /**
     * @brief Destructor
     */
    ~LayoutManager();

    //==============================================================================
    // Layout Configuration
    /**
     * @brief Set layout direction
     * @param direction Layout direction
     */
    void setDirection(LayoutDirection direction);

    /**
     * @brief Get layout direction
     */
    LayoutDirection getDirection() const { return direction_; }

    /**
     * @brief Set padding around layout
     * @param padding Padding values
     */
    void setPadding(const juce::Rectangle<float>& padding);

    /**
     * @brief Get layout padding
     */
    const juce::Rectangle<float>& getPadding() const { return padding_; }

    /**
     * @brief Set spacing between items
     * @param spacing Spacing value
     */
    void setSpacing(float spacing);

    /**
     * @brief Get spacing between items
     */
    float getSpacing() const { return spacing_; }

    /**
     * @brief Set wrap items to next line/column
     * @param wrap Whether to wrap items
     */
    void setWrap(bool wrap);

    /**
     * @brief Check if wrapping is enabled
     */
    bool getWrap() const { return wrap_; }

    /**
     * @brief Set cross-axis alignment
     * @param alignment Cross-axis alignment
     */
    void setCrossAxisAlignment(LayoutAlignment alignment);

    /**
     * @brief Get cross-axis alignment
     */
    LayoutAlignment getCrossAxisAlignment() const { return cross_axis_alignment_; }

    /**
     * @brief Set main-axis alignment
     * @param alignment Main-axis alignment
     */
    void setMainAxisAlignment(LayoutAlignment alignment);

    /**
     * @brief Get main-axis alignment
     */
    LayoutAlignment getMainAxisAlignment() const { return main_axis_alignment_; }

    //==============================================================================
    // Component Management
    /**
     * @brief Add component to layout
     * @param component Component to add
     * @param constraint Layout constraint
     * @param alignment Component alignment
     */
    void addComponent(Component* component, 
                     const LayoutConstraint& constraint = LayoutConstraint(),
                     LayoutAlignment alignment = LayoutAlignment::Stretch);

    /**
     * @brief Remove component from layout
     * @param component Component to remove
     */
    void removeComponent(Component* component);

    /**
     * @brief Clear all components
     */
    void clearComponents();

    /**
     * @brief Get component at index
     * @param index Component index
     * @return Component pointer or nullptr
     */
    Component* getComponentAt(int index) const;

    /**
     * @brief Get component index
     * @param component Component to find
     * @return Component index or -1
     */
    int getComponentIndex(Component* component) const;

    /**
     * @brief Get component count
     */
    int getComponentCount() const { return static_cast<int>(items_.size()); }

    /**
     * @brief Set component constraint
     * @param component Component to update
     * @param constraint New constraint
     */
    void setComponentConstraint(Component* component, const LayoutConstraint& constraint);

    /**
     * @brief Set component alignment
     * @param component Component to update
     * @param alignment New alignment
     */
    void setComponentAlignment(Component* component, LayoutAlignment alignment);

    //==============================================================================
    // Layout Calculation
    /**
     * @brief Calculate layout bounds
     * @param available_bounds Available space for layout
     */
    void layout(const juce::Rectangle<float>& available_bounds);

    /**
     * @brief Get layout bounds for component
     * @param component Component to query
     * @return Component bounds or empty rectangle if not found
     */
    juce::Rectangle<float> getComponentBounds(Component* component) const;

    /**
     * @brief Check if layout is dirty and needs recalculation
     */
    bool isDirty() const { return dirty_; }

    /**
     * @brief Mark layout as dirty
     */
    void markDirty();

    /**
     * @brief Force layout recalculation
     */
    void invalidate();

    //==============================================================================
    // Responsive Layout
    /**
     * @brief Add responsive breakpoint
     * @param breakpoint Breakpoint name
     * @param min_width Minimum width for this breakpoint
     * @param layout_config Layout configuration for this breakpoint
     */
    void addBreakpoint(const juce::String& breakpoint, float min_width,
                      std::function<void(LayoutManager&)> layout_config);

    /**
     * @brief Remove responsive breakpoint
     * @param breakpoint Breakpoint name
     */
    void removeBreakpoint(const juce::String& breakpoint);

    /**
     * @brief Get current breakpoint
     */
    const juce::String& getCurrentBreakpoint() const { return current_breakpoint_; }

    /**
     * @brief Check if current size matches breakpoint
     * @param breakpoint Breakpoint to check
     * @return True if current size matches
     */
    bool matchesBreakpoint(const juce::String& breakpoint) const;

    /**
     * @brief Set active breakpoint by width
     * @param width Current width
     */
    void setActiveBreakpointByWidth(float width);

    //==============================================================================
    // Grid Layout
    /**
     * @brief Set grid configuration
     * @param columns Number of columns
     * @param rows Number of rows (-1 means auto-calculate)
     * @param column_width Column width constraint
     * @param row_height Row height constraint
     */
    void setGrid(int columns, int rows = -1, 
                const LayoutConstraint& column_width = LayoutConstraint(),
                const LayoutConstraint& row_height = LayoutConstraint());

    /**
     * @brief Set grid cell for component
     * @param component Component to position
     * @param column Column index
     * @param row Row index
     * @param column_span Column span
     * @param row_span Row span
     */
    void setGridPosition(Component* component, int column, int row,
                        int column_span = 1, int row_span = 1);

    /**
     * @brief Get grid position for component
     * @param component Component to query
     * @return Grid position or (-1, -1) if not found
     */
    std::pair<std::pair<int, int>, std::pair<int, int>> getGridPosition(Component* component) const;

    //==============================================================================
    // Performance
    /**
     * @brief Enable layout caching
     * @param enabled Whether caching is enabled
     */
    void setCachingEnabled(bool enabled);

    /**
     * @brief Check if caching is enabled
     */
    bool isCachingEnabled() const { return caching_enabled_; }

    /**
     * @brief Get layout calculation time
     */
    float getLayoutTime() const { return layout_time_ms_; }

    /**
     * @brief Get last layout cache hit ratio
     */
    float getCacheHitRatio() const { return cache_hit_ratio_; }

    //==============================================================================
    // Accessibility
    /**
     * @brief Enable accessibility layout modes
     * @param enabled Whether accessibility layout is enabled
     */
    void setAccessibilityLayoutEnabled(bool enabled);

    /**
     * @brief Set high contrast mode
     * @param high_contrast Whether high contrast mode is enabled
     */
    void setHighContrastMode(bool high_contrast);

    /**
     * @brief Set increased spacing for better touch targets
     * @param increased_spacing Whether increased spacing is enabled
     */
    void setIncreasedTouchSpacing(bool increased_spacing);

private:
    //==============================================================================
    // Private member variables
    LayoutDirection direction_ = LayoutDirection::Row;
    juce::Rectangle<float> padding_{10.0f};
    float spacing_ = 10.0f;
    bool wrap_ = false;
    LayoutAlignment cross_axis_alignment_ = LayoutAlignment::Start;
    LayoutAlignment main_axis_alignment_ = LayoutAlignment::Start;
    bool dirty_ = true;

    // Components
    std::vector<LayoutItem> items_;
    std::unordered_map<Component*, LayoutItem*> item_map_;

    // Grid layout
    int grid_columns_ = 1;
    int grid_rows_ = -1;
    LayoutConstraint grid_column_width_;
    LayoutConstraint grid_row_height_;
    std::unordered_map<Component*, std::pair<std::pair<int, int>, std::pair<int, int>>> grid_positions_;

    // Responsive layout
    struct Breakpoint {
        float min_width;
        std::function<void(LayoutManager&)> config;
    };
    std::unordered_map<juce::String, Breakpoint> breakpoints_;
    juce::String current_breakpoint_ = "default";

    // Performance
    bool caching_enabled_ = true;
    float layout_time_ms_ = 0.0f;
    float cache_hit_ratio_ = 0.0f;
    juce::Rectangle<float> last_layout_bounds_;
    juce::String last_breakpoint_;
    std::chrono::steady_clock::time_point last_layout_time_;

    // Accessibility
    bool accessibility_layout_enabled_ = false;
    bool high_contrast_mode_ = false;
    bool increased_touch_spacing_ = false;

    //==============================================================================
    // Private methods
    void calculateFlexLayout(const juce::Rectangle<float>& bounds);
    void calculateGridLayout(const juce::Rectangle<float>& bounds);
    void calculateWrapLayout(const juce::Rectangle<float>& bounds);

    float calculateAvailableSpace(float container_size, const std::vector<LayoutItem*>& items, bool is_main_axis);
    void distributeSpace(const std::vector<LayoutItem*>& items, float available_space, bool is_main_axis);
    void applyAlignment(const std::vector<LayoutItem*>& items, 
                       const juce::Rectangle<float>& container_bounds, 
                       bool is_main_axis);

    void updateGridPositions();
    void ensureGridCapacity();

    // Utility methods
    void setComponentBounds(Component* component, const juce::Rectangle<float>& bounds);
    bool shouldUseResponsiveLayout(float width) const;
    void applyBreakpointConfiguration(const juce::String& breakpoint);

    // Performance optimization
    bool canUseCachedLayout(const juce::Rectangle<float>& bounds, const juce::String& breakpoint) const;
    void updatePerformanceMetrics();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LayoutManager)
};

} // namespace core
} // namespace ui
} // namespace vital
