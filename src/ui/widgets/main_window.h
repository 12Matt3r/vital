#pragma once

#include <memory>
#include <vector>
#include <unordered_map>
#include <functional>
#include <JuceHeader.h>
#include "../core/component.h"
#include "../core/layout_manager.h"
#include "../core/ui_manager.h"

namespace vital {
namespace ui {
namespace widgets {

/**
 * @brief Window state information
 */
struct WindowState {
    bool is_maximized = false;
    bool is_minimized = false;
    bool is_fullscreen = false;
    bool is_docked = false;
    juce::Rectangle<int> last_bounds;
    float opacity = 1.0f;
    bool always_on_top = false;
    bool resizable = true;
    bool has_title_bar = true;
    juce::String title;
    juce::String icon_name;
};

/**
 * @brief Title bar configuration
 */
struct TitleBarConfig {
    bool show_title = true;
    bool show_icon = true;
    bool show_minimize_button = true;
    bool show_maximize_button = true;
    bool show_close_button = true;
    bool show_menu_button = false;
    float height = 40.0f;
    juce::Colour background_color;
    juce::Colour text_color;
    juce::Font font;
    juce::String font_name;
    float font_size = 14.0f;
    
    TitleBarConfig() = default;
};

/**
 * @brief Status bar configuration
 */
struct StatusBarConfig {
    bool show_status_bar = false;
    float height = 24.0f;
    juce::Colour background_color;
    juce::Colour text_color;
    bool show_progress = false;
    bool show_clock = false;
    juce::String status_text;
    
    StatusBarConfig() = default;
};

/**
 * @brief MainWindow - Modern main window with docking and workspace support
 * 
 * Provides a comprehensive main window implementation with:
 * - Material Design 3.0 styling
 * - Docking panel system
 * - Workspace management and persistence
 * - Responsive layout support
 * - Accessibility features
 * - Animation and transitions
 * - Menu bar and toolbars
 * - Status bar and progress indicators
 */
class VITAL_MODERN_UI_API MainWindow : public core::Component {
public:
    /**
     * @brief Window callback types
     */
    using CloseCallback = std::function<bool()>;
    using MinimizeCallback = std::function<void()>;
    using MaximizeCallback = std::function<void(bool)>;
    using ResizeCallback = std::function<void(const juce::Rectangle<int>&)>;
    using MoveCallback = std::function<void(const juce::Point<int>&)>;
    using StateChangeCallback = std::function<void(const WindowState&)>;

    /**
     * @brief Constructor
     * @param title Window title
     * @param initial_bounds Initial window bounds
     */
    MainWindow(const juce::String& title = "Vital Synthesizer",
               const juce::Rectangle<int>& initial_bounds = juce::Rectangle<int>(100, 100, 1200, 800));

    /**
     * @brief Destructor
     */
    ~MainWindow() override;

    //==============================================================================
    // Window State Management
    /**
     * @brief Get current window state
     */
    const WindowState& getWindowState() const { return window_state_; }

    /**
     * @brief Set window title
     * @param title New title
     */
    void setTitle(const juce::String& title);

    /**
     * @brief Get window title
     */
    const juce::String& getTitle() const { return window_state_.title; }

    /**
     * @brief Set window icon
     * @param icon_name Icon name or path
     */
    void setIcon(const juce::String& icon_name);

    /**
     * @brief Get window icon
     */
    const juce::String& getIcon() const { return window_state_.icon_name; }

    /**
     * @brief Set window bounds
     * @param bounds New bounds
     */
    void setBounds(const juce::Rectangle<int>& bounds) override;

    /**
     * @brief Set window position
     * @param position New position
     */
    void setPosition(const juce::Point<int>& position);

    /**
     * @brief Get window position
     */
    juce::Point<int> getPosition() const { return window_state_.last_bounds.getPosition(); }

    /**
     * @brief Set window size
     * @param width Width
     * @param height Height
     */
    void setSize(int width, int height);

    /**
     * @brief Get window size
     */
    juce::Point<int> getSize() const { 
        return juce::Point<int>(window_state_.last_bounds.getWidth(),
                               window_state_.last_bounds.getHeight()); 
    }

    /**
     * @brief Center window on screen
     */
    void centerOnScreen();

    /**
     * @brief Maximize window
     * @param maximized Whether to maximize
     */
    void setMaximized(bool maximized);

    /**
     * @brief Check if window is maximized
     */
    bool isMaximized() const { return window_state_.is_maximized; }

    /**
     * @brief Minimize window
     * @param minimized Whether to minimize
     */
    void setMinimized(bool minimized);

    /**
     * @brief Check if window is minimized
     */
    bool isMinimized() const { return window_state_.is_minimized; }

    /**
     * @brief Set fullscreen mode
     * @param fullscreen Whether to enter fullscreen
     */
    void setFullscreen(bool fullscreen);

    /**
     * @brief Check if window is fullscreen
     */
    bool isFullscreen() const { return window_state_.is_fullscreen; }

    /**
     * @brief Set window opacity
     * @param opacity Opacity (0.0 to 1.0)
     */
    void setOpacity(float opacity);

    /**
     * @brief Get window opacity
     */
    float getOpacity() const { return window_state_.opacity; }

    /**
     * @brief Set always on top
     * @param always_on_top Whether to keep window always on top
     */
    void setAlwaysOnTop(bool always_on_top);

    /**
     * @brief Check if window is always on top
     */
    bool isAlwaysOnTop() const { return window_state_.always_on_top; }

    //==============================================================================
    // Resizability
    /**
     * @brief Set resizable
     * @param resizable Whether window is resizable
     */
    void setResizable(bool resizable);

    /**
     * @brief Check if window is resizable
     */
    bool isResizable() const { return window_state_.resizable; }

    /**
     * @brief Set minimum size
     * @param min_width Minimum width
     * @param min_height Minimum height
     */
    void setMinimumSize(int min_width, int min_height);

    /**
     * @brief Get minimum size
     */
    std::pair<int, int> getMinimumSize() const { return {min_width_, min_height_}; }

    /**
     * @brief Set maximum size
     * @param max_width Maximum width (-1 for no limit)
     * @param max_height Maximum height (-1 for no limit)
     */
    void setMaximumSize(int max_width, int max_height);

    /**
     * @brief Get maximum size
     */
    std::pair<int, int> getMaximumSize() const { return {max_width_, max_height_}; }

    //==============================================================================
    // Title Bar
    /**
     * @brief Configure title bar
     * @param config Title bar configuration
     */
    void setTitleBarConfig(const TitleBarConfig& config);

    /**
     * @brief Get title bar configuration
     */
    const TitleBarConfig& getTitleBarConfig() const { return title_bar_config_; }

    /**
     * @brief Set title bar height
     * @param height Title bar height
     */
    void setTitleBarHeight(float height);

    /**
     * @brief Get title bar height
     */
    float getTitleBarHeight() const { return title_bar_config_.height; }

    /**
     * @brief Enable/disable title bar buttons
     * @param minimize Show minimize button
     * @param maximize Show maximize button
     * @param close Show close button
     */
    void setTitleBarButtons(bool minimize, bool maximize, bool close);

    //==============================================================================
    // Status Bar
    /**
     * @brief Configure status bar
     * @param config Status bar configuration
     */
    void setStatusBarConfig(const StatusBarConfig& config);

    /**
     * @brief Get status bar configuration
     */
    const StatusBarConfig& getStatusBarConfig() const { return status_bar_config_; }

    /**
     * @brief Set status text
     * @param text Status text to display
     */
    void setStatusText(const juce::String& text);

    /**
     * @brief Get current status text
     */
    const juce::String& getStatusText() const { return status_bar_config_.status_text; }

    /**
     * @brief Set progress indicator
     * @param progress Progress value (0.0 to 1.0, -1.0 for indeterminate)
     */
    void setProgress(float progress);

    /**
     * @brief Get current progress
     */
    float getProgress() const { return current_progress_; }

    //==============================================================================
    // Content Management
    /**
     * @brief Set main content component
     * @param content Main content component
     */
    void setContentComponent(core::Component* content);

    /**
     * @brief Get main content component
     */
    core::Component* getContentComponent() const { return content_component_; }

    /**
     * @brief Add component to content area
     * @param component Component to add
     * @param layout_constraint Layout constraint
     */
    void addContentComponent(core::Component* component, 
                            const core::LayoutManager::LayoutConstraint& constraint = 
                            core::LayoutManager::LayoutConstraint());

    /**
     * @brief Remove component from content area
     * @param component Component to remove
     */
    void removeContentComponent(core::Component* component);

    /**
     * @brief Clear all content components
     */
    void clearContentComponents();

    /**
     * @brief Get content layout manager
     */
    core::LayoutManager& getContentLayoutManager();

    //==============================================================================
    // Panel Management
    /**
     * @brief Add docking panel
     * @param panel Panel to add
     * @param position Dock position
     * @param size Panel size
     */
    void addDockingPanel(core::Component* panel, 
                        const juce::String& position,
                        const juce::Point<int>& size = juce::Point<int>(200, 200));

    /**
     * @brief Remove docking panel
     * @param panel Panel to remove
     */
    void removeDockingPanel(core::Component* panel);

    /**
     * @brief Get all docking panels
     */
    std::vector<core::Component*> getDockingPanels() const;

    /**
     * @brief Set panel dock position
     * @param panel Panel to move
     * @param position New dock position
     */
    void setPanelDockPosition(core::Component* panel, const juce::String& position);

    /**
     * @brief Float docking panel
     * @param panel Panel to float
     * @param bounds Bounds for floating panel
     */
    void floatDockingPanel(core::Component* panel, const juce::Rectangle<int>& bounds);

    //==============================================================================
    // Workspace Management
    /**
     * @brief Save workspace
     * @param workspace_name Workspace name
     * @return True if saved successfully
     */
    bool saveWorkspace(const juce::String& workspace_name);

    /**
     * @brief Load workspace
     * @param workspace_name Workspace name
     * @return True if loaded successfully
     */
    bool loadWorkspace(const juce::String& workspace_name);

    /**
     * @brief Get list of saved workspaces
     */
    std::vector<juce::String> getSavedWorkspaces() const;

    /**
     * @brief Delete workspace
     * @param workspace_name Workspace to delete
     * @return True if deleted successfully
     */
    bool deleteWorkspace(const juce::String& workspace_name);

    /**
     * @brief Reset to default workspace
     */
    void resetToDefaultWorkspace();

    /**
     * @brief Set current workspace
     * @param workspace_name Workspace name
     */
    void setCurrentWorkspace(const juce::String& workspace_name);

    /**
     * @brief Get current workspace name
     */
    const juce::String& getCurrentWorkspace() const { return current_workspace_; }

    //==============================================================================
    // Callbacks
    void setCloseCallback(CloseCallback callback);
    void setMinimizeCallback(MinimizeCallback callback);
    void setMaximizeCallback(MaximizeCallback callback);
    void setResizeCallback(ResizeCallback callback);
    void setMoveCallback(MoveCallback callback);
    void setStateChangeCallback(StateChangeCallback callback);

    //==============================================================================
    // Theme Integration
    /**
     * @brief Apply theme to window
     * @param theme_name Theme name
     */
    void applyTheme(const juce::String& theme_name);

    /**
     * @brief Get current theme name
     */
    const juce::String& getCurrentTheme() const { return current_theme_; }

    /**
     * @brief Enable/disable window animations
     */
    void setAnimationsEnabled(bool enabled);

    /**
     * @brief Check if animations are enabled
     */
    bool areAnimationsEnabled() const { return animations_enabled_; }

    //==============================================================================
    // Accessibility
    /**
     * @brief Set window accessibility features
     * @param accessible Whether window is accessible
     * @param label Accessibility label
     */
    void setAccessible(bool accessible, const juce::String& label = "");

    /**
     * @brief Enable/disable keyboard navigation
     */
    void setKeyboardNavigationEnabled(bool enabled);

    /**
     * @brief Check if keyboard navigation is enabled
     */
    bool isKeyboardNavigationEnabled() const { return keyboard_navigation_enabled_; }

    /**
     * @brief Set keyboard shortcuts
     */
    void setKeyboardShortcuts(const std::unordered_map<juce::KeyPress, juce::String>& shortcuts);

    //==============================================================================
    // Event Handling
    bool keyPressed(const juce::KeyPress& key) override;
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;
    void resized() override;
    void childBoundsChanged(juce::Component* child) override;

    //==============================================================================
    // Component Overrides
    void update() override;
    void render(juce::Graphics& g) override;

protected:
    /**
     * @brief Core::Component overrides
     */
    void updateInternal() override;
    void renderInternal(juce::Graphics& g) override;

    void onFocus() override;
    void onFocusLost() override;

private:
    //==============================================================================
    // Member variables
    WindowState window_state_;
    TitleBarConfig title_bar_config_;
    StatusBarConfig status_bar_config_;

    // Content management
    core::Component* content_component_ = nullptr;
    std::vector<core::Component*> content_components_;
    std::unique_ptr<core::LayoutManager> content_layout_manager_;

    // Panel management
    std::vector<core::Component*> docking_panels_;
    std::unordered_map<juce::String, std::vector<core::Component*>> docked_panels_;

    // Size constraints
    int min_width_ = 400;
    int min_height_ = 300;
    int max_width_ = -1;  // No limit
    int max_height_ = -1; // No limit

    // Progress indicator
    float current_progress_ = -1.0f;
    bool show_progress_bar_ = false;

    // Workspace management
    juce::String current_workspace_;
    std::vector<juce::String> saved_workspaces_;
    juce::String workspace_file_path_;

    // Theme
    juce::String current_theme_;
    bool animations_enabled_ = true;

    // Accessibility
    bool keyboard_navigation_enabled_ = true;
    std::unordered_map<juce::KeyPress, juce::String> keyboard_shortcuts_;

    // Drag and resize
    bool is_dragging_ = false;
    bool is_resizing_ = false;
    juce::Point<int> drag_start_position_;
    juce::Rectangle<int> original_bounds_;
    int resize_handle_size_ = 8;

    // Callbacks
    CloseCallback close_callback_;
    MinimizeCallback minimize_callback_;
    MaximizeCallback maximize_callback_;
    ResizeCallback resize_callback_;
    MoveCallback move_callback_;
    StateChangeCallback state_change_callback_;

    //==============================================================================
    // Private methods
    void updateWindowState();
    void updateContentLayout();
    void updatePanelLayouts();
    void updateStatusBar();

    // Title bar rendering
    void renderTitleBar(juce::Graphics& g);
    void renderTitleBarButtons(juce::Graphics& g);
    void renderTitleBarText(juce::Graphics& g);
    bool isPointInTitleBar(const juce::Point<int>& point) const;

    // Status bar rendering
    void renderStatusBar(juce::Graphics& g);
    void renderProgressBar(juce::Graphics& g);
    bool isPointInStatusBar(const juce::Point<int>& point) const;

    // Content area rendering
    juce::Rectangle<int> getContentArea() const;
    juce::Rectangle<int> getTitleBarArea() const;
    juce::Rectangle<int> getStatusBarArea() const;

    // Event handling helpers
    bool handleTitleBarClick(const juce::MouseEvent& e);
    bool handleResizeHandleClick(const juce::MouseEvent& e);
    void handleDragOperation(const juce::MouseEvent& e);
    void handleResizeOperation(const juce::MouseEvent& e);

    // Title bar button clicks
    void handleMinimizeClick();
    void handleMaximizeClick();
    void handleCloseClick();

    // Keyboard shortcuts
    bool handleKeyboardShortcuts(const juce::KeyPress& key);

    // Workspace helpers
    juce::String getWorkspaceFilePath(const juce::String& workspace_name) const;
    bool validateWorkspaceName(const juce::String& workspace_name) const;

    // Utility methods
    bool isValidDockPosition(const juce::String& position) const;
    juce::String getDefaultDockPosition() const;
    void assertContentComponentValid() const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};

} // namespace widgets
} // namespace ui
} // namespace vital
