#pragma once

#include "../core/component.h"
#include "../theme/theme_manager.h"
#include <memory>
#include <vector>

namespace vital {
namespace ui {
namespace material {

/**
 * @brief Material Design 3.0 card container
 * 
 * A container component that can hold other UI elements with:
 * - Elevation/shadow effects
 * - Rounded corners
 * - Optional header and footer
 * - Click handling
 * - Hover animations
 */
class Card : public Component {
public:
    enum class Style {
        Elevated,       // Elevated card with shadow
        Filled,         // Filled card with subtle background
        Outlined,       // Outlined card with border
        Assist,         // Assistance card style
        Suggestion,     // Suggestion card style
        Custom          // Custom styling
    };

    struct CardStyle {
        Style style = Style::Elevated;
        float cornerRadius = 12.0f;
        float elevation = 8.0f; // Shadow depth
        Color backgroundColor;
        Color borderColor = Colors::outline;
        float borderWidth = 1.0f;
        Color hoverColor;
        bool clickable = false;
        bool showRipple = true;
        bool showHeaderDivider = true;
        bool showFooterDivider = true;
        float headerHeight = 56.0f;
        float footerHeight = 48.0f;
        float padding = 16.0f;
        float spacing = 8.0f;
        std::string title;
        std::string subtitle;
        std::string supportingText;
        std::vector<std::string> actions;
        bool enableStateLayer = true;
        float stateLayerOpacity = 0.08f;
    };

    using CardClickCallback = std::function<void()>;
    using ActionClickCallback = std::function<void(int actionIndex, const std::string& action)>;

    Card(const std::string& title = "", const CardStyle& style = {});
    ~Card() override = default;

    // Component interface
    void paint(juce::Graphics& g) override;
    void resized() override;
    bool mouseDown(const juce::MouseEvent& e) override;
    bool mouseUp(const juce::MouseEvent& e) override;
    void mouseEnter(const juce::MouseEvent& e) override;
    void mouseExit(const juce::MouseEvent& e) override;
    void focusGained() override;
    void focusLost() override;
    
    // Content management
    void addContentComponent(std::unique_ptr<Component> component);
    void removeContentComponent(Component* component);
    void clearContentComponents();
    
    // Style configuration
    void setStyle(const CardStyle& style);
    const CardStyle& getStyle() const { return style_; }
    
    // Card content
    void setTitle(const std::string& title);
    void setSubtitle(const std::string& subtitle);
    void setSupportingText(const std::string& text);
    void setActions(const std::vector<std::string>& actions);
    
    // Appearance
    void setCornerRadius(float radius);
    void setElevation(float elevation);
    void setBackgroundColor(const Color& color);
    void setBorderColor(const Color& color);
    void setBorderWidth(float width);
    
    // State
    void setClickable(bool clickable);
    void setSelected(bool selected);
    bool isSelected() const { return selected_; }
    
    // Callbacks
    void setCardClickCallback(CardClickCallback callback);
    void setActionClickCallback(ActionClickCallback callback);
    
    // Accessibility
    juce::String getAccessibilityLabel() const override;
    juce::String getAccessibilityValue() const override;
    void performAccessibilityAction(const juce::String& actionName) override;

private:
    CardStyle style_;
    std::vector<std::unique_ptr<Component>> contentComponents_;
    
    bool selected_ = false;
    bool isHovered_ = false;
    bool isPressed_ = false;
    
    // Animation values
    AnimationValue elevation_;
    AnimationValue backgroundColor_;
    AnimationValue borderColor_;
    AnimationValue hoverOpacity_;
    
    // Callbacks
    CardClickCallback clickCallback_;
    ActionClickCallback actionCallback_;
    
    // Internal helpers
    void setupAnimations();
    juce::Rectangle<float> getCardBounds() const;
    juce::Rectangle<float> getHeaderBounds() const;
    juce::Rectangle<float> getContentBounds() const;
    juce::Rectangle<float> getFooterBounds() const;
    juce::Rectangle<float> getActionButtonBounds(int index) const;
    void drawHeader(juce::Graphics& g);
    void drawContent(juce::Graphics& g);
    void drawFooter(juce::Graphics& g);
    void drawActions(juce::Graphics& g);
    void handleStateChanges();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_CHECK(Card)
};

} // namespace material
} // namespace ui
} // namespace vital