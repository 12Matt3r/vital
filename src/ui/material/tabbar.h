#pragma once

#include "../core/component.h"
#include "../theme/theme_manager.h"
#include <functional>
#include <memory>
#include <vector>

namespace vital {
namespace ui {
namespace material {

/**
 * @brief Material Design 3.0 tab bar
 * 
 * A navigation component for switching between different content sections with:
 * - Fixed or scrollable tabs
 * - Selection indicator with animation
 * - Icons and text support
 * - Overflow handling
 * - Smooth transitions
 */
class TabBar : public Component {
public:
    enum class TabPosition {
        Top,
        Bottom,
        Start,   // Left for LTR, right for RTL
        End      // Right for LTR, left for RTL
    };

    enum class TabStyle {
        Standard,     // Standard Material tabs
        Scrollable,   // Horizontally scrollable
        Pinned,       // Pinned tab bar
        Custom        // Custom styling
    };

    struct Tab {
        std::string id;
        std::string text;
        juce::Drawable* icon = nullptr;
        std::string tooltip;
        bool enabled = true;
        bool selected = false;
        int badgeCount = 0;
        Color indicatorColor = Colors::primary;
        Color textColor;
        Color iconColor;
    };

    struct TabBarStyle {
        TabStyle style = TabStyle::Standard;
        TabPosition position = TabPosition::Top;
        float tabHeight = 48.0f;
        float tabWidth = 120.0f;
        float minTabWidth = 90.0f;
        float maxTabWidth = 200.0f;
        float indicatorHeight = 3.0f;
        Color selectedTextColor = Colors::primary;
        Color unselectedTextColor = Colors::onSurfaceVariant;
        Color selectedIconColor = Colors::primary;
        Color unselectedIconColor = Colors::onSurfaceVariant;
        Color backgroundColor = Colors::surface;
        Color indicatorColor = Colors::primary;
        float cornerRadius = 0.0f;
        bool showDivider = true;
        Color dividerColor = Colors::outlineVariant;
        float dividerOpacity = 0.12f;
        float padding = 16.0f;
        float spacing = 8.0f;
        int maxVisibleTabs = 5;
        bool enableAnimation = true;
        float animationDuration = 0.2f;
    };

    using TabSelectedCallback = std::function<void(const std::string& tabId, int index)>;
    using TabClickedCallback = std::function<void(const std::string& tabId, int index)>;
    using TabCloseCallback = std::function<void(const std::string& tabId, int index)>;

    TabBar(const TabBarStyle& style = {});
    ~TabBar() override = default;

    // Component interface
    void paint(juce::Graphics& g) override;
    void resized() override;
    bool mouseDown(const juce::MouseEvent& e) override;
    bool mouseUp(const juce::MouseEvent& e) override;
    bool mouseDrag(const juce::MouseEvent& e) override;
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;
    
    // Tab management
    void addTab(const Tab& tab);
    void addTab(const std::string& id, const std::string& text, juce::Drawable* icon = nullptr);
    void removeTab(const std::string& id);
    void removeTabAt(int index);
    void clearTabs();
    
    // Tab selection
    void setSelectedTab(const std::string& id, bool sendCallback = true);
    void setSelectedTabIndex(int index, bool sendCallback = true);
    std::string getSelectedTabId() const;
    int getSelectedTabIndex() const;
    
    // Tab updates
    void updateTab(const std::string& id, const Tab& tab);
    void setTabText(const std::string& id, const std::string& text);
    void setTabIcon(const std::string& id, juce::Drawable* icon);
    void setTabEnabled(const std::string& id, bool enabled);
    void setTabBadge(const std::string& id, int badgeCount);
    
    // Style configuration
    void setStyle(const TabBarStyle& style);
    const TabBarStyle& getStyle() const { return style_; }
    
    // Callbacks
    void setTabSelectedCallback(TabSelectedCallback callback);
    void setTabClickedCallback(TabClickedCallback callback);
    void setTabCloseCallback(TabCloseCallback callback);
    
    // Animation
    void animateToTab(const std::string& id, float duration = 0.2f);
    void animateToTabIndex(int index, float duration = 0.2f);
    
    // Scroll support (for scrollable tabs)
    void setScrollPosition(float position);
    float getScrollPosition() const { return scrollPosition_; }
    void scrollToTab(const std::string& id);
    void scrollToTabIndex(int index);
    
    // Accessibility
    juce::String getAccessibilityLabel() const override;
    juce::String getAccessibilityValue() const override;
    void performAccessibilityAction(const juce::String& actionName) override;

private:
    TabBarStyle style_;
    std::vector<Tab> tabs_;
    int selectedIndex_ = 0;
    
    // Scroll state
    float scrollPosition_ = 0.0f;
    float scrollVelocity_ = 0.0f;
    bool isScrolling_ = false;
    juce::Point<float> scrollStart_;
    
    // Animation
    AnimationValue indicatorPosition_;
    AnimationValue scrollAnimation_;
    
    // Callbacks
    TabSelectedCallback selectedCallback_;
    TabClickedCallback clickedCallback_;
    TabCloseCallback closeCallback_;
    
    // Internal helpers
    const Tab* getTabById(const std::string& id) const;
    Tab* getTabById(const std::string& id);
    void layoutTabs();
    float getTabBounds(int index, juce::Rectangle<float>& bounds) const;
    void drawTab(juce::Graphics& g, const Tab& tab, const juce::Rectangle<float>& bounds, bool isSelected);
    void drawIndicator(juce::Graphics& g);
    void drawScrollButtons(juce::Graphics& g);
    bool hitTestScrollButtons(const juce::Point<float>& position);
    void updateIndicatorAnimation();
    void handleTabClick(int index, const juce::MouseEvent& e);
    void updateScrollButtons();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_CHECK(TabBar)
};

} // namespace material
} // namespace ui
} // namespace vital