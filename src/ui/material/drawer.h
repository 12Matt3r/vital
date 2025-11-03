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
 * @brief Material Design 3.0 navigation drawer
 * 
 * A sliding navigation panel that can be docked or modal with:
 * - Permanent, persistent, or temporary modes
 * - Header, content, and footer sections
 * - Item selection with icons and badges
 * - Smooth slide animations
 * - Backdrop overlay support
 */
class Drawer : public Component {
public:
    enum class Type {
        Permanent,     // Always visible, docked
        Persistent,    // Sliding, remembers state
        Temporary,     // Modal overlay, temporary
        Custom         // Custom behavior
    };

    enum class Anchor {
        Start,         // Left for LTR, right for RTL
        End,           // Right for LTR, left for RTL
        Custom         // Custom position
    };

    struct DrawerItem {
        std::string id;
        std::string text;
        juce::Drawable* icon = nullptr;
        bool enabled = true;
        bool selected = false;
        int badgeCount = 0;
        bool showBadge = false;
        Color selectedIconColor;
        Color selectedTextColor;
        Color backgroundColor;
    };

    struct DrawerStyle {
        Type type = Type::Persistent;
        Anchor anchor = Anchor::Start;
        float width = 320.0f;
        float minWidth = 280.0f;
        float maxWidth = 400.0f;
        float headerHeight = 72.0f;
        float footerHeight = 56.0f;
        float itemHeight = 48.0f;
        float itemSpacing = 4.0f;
        float itemPadding = 16.0f;
        float iconSize = 24.0f;
        float cornerRadius = 0.0f;
        Color backgroundColor = Colors::surface;
        Color scrimColor = Colors::scrim;
        float scrimOpacity = 0.32f;
        Color dividerColor = Colors::outlineVariant;
        float dividerOpacity = 0.12f;
        Color selectedBackgroundColor = Colors::secondaryContainer;
        Color selectedTextColor = Colors::onSecondaryContainer;
        Color selectedIconColor = Colors::onSecondaryContainer;
        Color textColor = Colors::onSurface;
        Color iconColor = Colors::onSurface;
        bool showDivider = true;
        bool enableAnimation = true;
        float animationDuration = 0.3f;
        bool autoClose = true;
        float dragThreshold = 100.0f;
        bool enableBackdrop = true;
        std::string headerTitle;
        std::string headerSubtitle;
        juce::Drawable* headerImage = nullptr;
    };

    using ItemSelectedCallback = std::function<void(const std::string& itemId, int index)>;
    using ItemClickedCallback = std::function<void(const std::string& itemId, int index)>;
    using DrawerStateCallback = std::function<void(bool isOpen)>;
    using DrawerDismissedCallback = std::function<void()>;

    Drawer(const DrawerStyle& style = {});
    ~Drawer() override = default;

    // Component interface
    void paint(juce::Graphics& g) override;
    void resized() override;
    bool mouseDown(const juce::MouseEvent& e) override;
    bool mouseDrag(const juce::MouseEvent& e) override;
    bool mouseUp(const juce::MouseEvent& e) override;
    void mouseWheelMove(const juce::MouseEvent& e, const juce::MouseWheelDetails& wheel) override;
    
    // State management
    void open();
    void close();
    void toggle();
    void setOpen(bool open, bool animate = true);
    bool isOpen() const { return isOpen_; }
    bool isAnimating() const { return isAnimating_; }
    
    // Item management
    void addItem(const DrawerItem& item);
    void addItem(const std::string& id, const std::string& text, juce::Drawable* icon = nullptr);
    void removeItem(const std::string& id);
    void removeItemAt(int index);
    void clearItems();
    
    // Item selection
    void setSelectedItem(const std::string& id, bool sendCallback = true);
    void setSelectedItemIndex(int index, bool sendCallback = true);
    std::string getSelectedItemId() const;
    int getSelectedItemIndex() const;
    
    // Header content
    void setHeaderContent(const std::string& title, const std::string& subtitle = "", juce::Drawable* image = nullptr);
    void setHeaderTitle(const std::string& title);
    void setHeaderSubtitle(const std::string& subtitle);
    void setHeaderImage(juce::Drawable* image);
    
    // Style configuration
    void setStyle(const DrawerStyle& style);
    const DrawerStyle& getStyle() const { return style_; }
    
    // Callbacks
    void setItemSelectedCallback(ItemSelectedCallback callback);
    void setItemClickedCallback(ItemClickedCallback callback);
    void setDrawerStateCallback(DrawerStateCallback callback);
    void setDrawerDismissedCallback(DrawerDismissedCallback callback);
    
    // Animation control
    void animateToState(bool targetOpen, float duration = 0.3f);
    
    // Drag support
    void setDraggable(bool draggable);
    void setDragConstraints(float minX, float maxX);
    
    // Accessibility
    juce::String getAccessibilityLabel() const override;
    juce::String getAccessibilityValue() const override;
    void performAccessibilityAction(const juce::String& actionName) override;

private:
    DrawerStyle style_;
    std::vector<DrawerItem> items_;
    
    // State
    bool isOpen_ = false;
    bool isAnimating_ = false;
    bool isDragging_ = false;
    float targetX_ = 0.0f;
    float dragStartX_ = 0.0f;
    float dragVelocity_ = 0.0f;
    juce::Point<float> lastDragPosition_;
    
    // Selected item
    int selectedIndex_ = -1;
    
    // Animation values
    AnimationValue xPosition_;
    AnimationValue opacity_;
    AnimationValue scale_;
    AnimationValue scrimOpacity_;
    
    // Callbacks
    ItemSelectedCallback selectedCallback_;
    ItemClickedCallback clickedCallback_;
    StateCallback stateCallback_;
    DismissedCallback dismissedCallback_;
    
    // Internal helpers
    void setupAnimations();
    void updateAnimation();
    void handleGesture();
    juce::Rectangle<float> getDrawerBounds() const;
    juce::Rectangle<float> getScrimBounds() const;
    juce::Rectangle<float> getHeaderBounds() const;
    juce::Rectangle<float> getItemsBounds() const;
    juce::Rectangle<float> getFooterBounds() const;
    juce::Rectangle<float> getItemBounds(int index) const;
    void drawHeader(juce::Graphics& g);
    void drawItems(juce::Graphics& g);
    void drawItem(juce::Graphics& g, const DrawerItem& item, const juce::Rectangle<float>& bounds);
    void drawFooter(juce::Graphics& g);
    void drawScrim(juce::Graphics& g);
    void drawDivider(juce::Graphics& g, const juce::Rectangle<float>& bounds);
    bool hitTestItems(const juce::Point<float>& position, int& itemIndex);
    void handleBackdropClick(const juce::MouseEvent& e);
    void openWithAnimation();
    void closeWithAnimation();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_CHECK(Drawer)
};

} // namespace material
} // namespace ui
} // namespace vital