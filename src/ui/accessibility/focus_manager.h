#pragma once

#include <memory>
#include <vector>
#include <functional>
#include <map>
#include <string>

namespace vital {
namespace ui {
namespace accessibility {

/**
 * @brief Focus management system for keyboard navigation
 * 
 * Manages focus order and navigation for UI components with:
 * - Tab order management
 * - Focus cycling
 * - Custom focus routing
 * - Keyboard shortcuts
 * - Focus indicators
 */
class FocusManager {
public:
    enum class FocusDirection {
        Next,           // Next element
        Previous,       // Previous element
        Up,             // Focus up
        Down,           // Focus down
        Left,           // Focus left
        Right           // Focus right
    };

    struct FocusableElement {
        void* component = nullptr;
        std::string id;
        std::string label;
        juce::Rectangle<float> bounds;
        int tabOrder = 0;
        bool enabled = true;
        bool visible = true;
        bool customFocus = false;
        std::function<bool(FocusDirection)> customHandler;
    };

    using FocusCallback = std::function<void(void* component, const std::string& id)>;

    FocusManager();
    ~FocusManager() = default;

    // Registration
    void registerFocusable(void* component, const std::string& id, int tabOrder = 0);
    void unregisterFocusable(void* component);
    void updateElementBounds(void* component, const juce::Rectangle<float>& bounds);
    void setElementEnabled(void* component, bool enabled);
    void setElementVisible(void* component, bool visible);

    // Focus control
    void setFocus(void* component, bool sendCallback = true);
    void* getFocusedElement() const { return focusedElement_; }
    void clearFocus();
    bool hasFocus(void* component) const;
    
    // Navigation
    void navigateFocus(FocusDirection direction);
    void setFocusToNext();
    void setFocusToPrevious();
    
    // Custom routing
    void setCustomFocusHandler(void* component, std::function<bool(FocusDirection)> handler);
    void enableCustomFocus(void* component, bool enable);
    
    // Callbacks
    void setFocusChangedCallback(FocusCallback callback);
    void setFocusLostCallback(FocusCallback callback);
    
    // Configuration
    void setTabWrapping(bool wrap);
    void setFocusIndicatorEnabled(bool enabled);
    void setFocusIndicatorColor(const juce::Colour& color);
    void setFocusIndicatorThickness(float thickness);

private:
    std::map<void*, FocusableElement> focusableElements_;
    std::vector<void*> tabOrder_;
    void* focusedElement_ = nullptr;
    int focusedIndex_ = -1;
    
    // Settings
    bool tabWrapping_ = true;
    bool focusIndicatorEnabled_ = true;
    juce::Colour focusIndicatorColor_ = juce::Colours::blue;
    float focusIndicatorThickness_ = 2.0f;
    
    // Callbacks
    FocusCallback focusChangedCallback_;
    FocusCallback focusLostCallback_;
    
    void updateTabOrder();
    void sortByTabOrder();
    int getElementIndex(void* component) const;
    void handleFocusChange(void* newElement);
};

} // namespace accessibility
} // namespace ui
} // namespace vital