#include "card.h"
#include <cmath>
#include <algorithm>
#include "../accessibility/accessibility_manager.h"

namespace vital {
namespace ui {
namespace material {

Card::Card(const std::string& title, const CardStyle& style)
    : Component(), style_(style) {
    setWantsKeyboardFocus(style_.clickable);
    setInterceptsMouseClicks(style_.clickable, style_.clickable);
    
    // Set ARIA role
    setAriaRole("region");
    setAriaLabel(title);
    setAriaDescription(title);
    
    // Initialize animations
    setupAnimations();
    
    // Set title if provided
    if (!title.empty()) {
        setTitle(title);
    }
}

void Card::paint(juce::Graphics& g) {
    Component::paint(g);
    
    auto bounds = getLocalBounds();
    auto card_bounds = getCardBounds();
    
    // Get theme colors
    auto theme = getTheme();
    auto colors = theme->getMaterialColors();
    
    // Calculate animated values
    float current_elevation = elevation_.getCurrentValue();
    float current_hover_opacity = hoverOpacity_.getCurrentValue();
    auto current_bg_color = backgroundColor_.getCurrentColor(colors);
    auto current_border_color = borderColor_.getCurrentColor(colors);
    
    // Draw shadow based on elevation
    if (style_.style == Style::Elevated || style_.style == Style::Assist || style_.style == Style::Suggestion) {
        auto shadow_color = juce::Colours::black.withAlpha(0.1f + current_elevation * 0.02f);
        float shadow_blur = current_elevation * 2.0f;
        float shadow_spread = current_elevation * 0.5f;
        
        // Draw multiple shadow layers for realistic effect
        for (int i = 0; i < 4; ++i) {
            float offset = (i + 1) * shadow_spread * 0.25f;
            float alpha = shadow_color.getFloatAlpha() * (4 - i) * 0.25f;
            g.setColour(shadow_color.withAlpha(alpha));
            g.fillRoundedRectangle(card_bounds.translated(offset, offset), 
                                  style_.cornerRadius + shadow_blur * 0.1f);
        }
    }
    
    // Draw main card background
    if (style_.style == Style::Filled) {
        // Filled card uses surface color with subtle elevation
        g.setColour(current_bg_color);
    } else if (style_.style == Style::Outlined) {
        // Outlined card uses transparent background with border
        g.setColour(colors.surface.withAlpha(0.95f));
    } else {
        // Elevated card
        g.setColour(colors.surface);
    }
    
    g.fillRoundedRectangle(card_bounds, style_.cornerRadius);
    
    // Draw state layer for interactive cards
    if (style_.clickable && style_.enableStateLayer) {
        auto state_color = colors.primary.withAlpha(style_.stateLayerOpacity * current_hover_opacity);
        g.setColour(state_color);
        g.fillRoundedRectangle(card_bounds, style_.cornerRadius);
    }
    
    // Draw border for outlined style
    if (style_.style == Style::Outlined || style_.style == Style::Custom) {
        g.setColour(current_border_color);
        g.drawRoundedRectangle(card_bounds, style_.cornerRadius, style_.borderWidth);
    }
    
    // Draw ripple effect
    if (isPressed_ && style_.showRipple) {
        auto ripple_center = getLastMouseDownPosition().toFloat();
        if (card_bounds.contains(ripple_center)) {
            auto ripple_color = colors.primary.withAlpha(0.12f);
            auto distance = ripple_center.getDistanceFrom(card_bounds.getCentre());
            auto ripple_radius = std::min(distance * 2.0f, card_bounds.getWidth() * 0.8f);
            
            g.setColour(ripple_color);
            g.fillEllipse(ripple_center.getX() - ripple_radius * 0.5f,
                         ripple_center.getY() - ripple_radius * 0.5f,
                         ripple_radius, ripple_radius);
        }
    }
    
    // Draw selected state indicator
    if (selected_) {
        auto selected_color = colors.primary;
        auto indicator_bounds = card_bounds.removeFromLeft(4.0f);
        g.setColour(selected_color);
        g.fillRoundedRectangle(indicator_bounds, 2.0f);
    }
    
    // Draw header
    if (style_.clickable || !style_.title.empty()) {
        drawHeader(g);
    }
    
    // Draw content area
    drawContent(g);
    
    // Draw footer
    if (!style_.actions.empty() || !style_.supportingText.empty()) {
        drawFooter(g);
    }
    
    // Draw actions
    if (!style_.actions.empty()) {
        drawActions(g);
    }
    
    // Draw focus indicator
    if (hasKeyboardFocus() && style_.clickable) {
        auto focus_bounds = card_bounds.reduced(2.0f);
        g.setColour(colors.primary);
        g.drawRoundedRectangle(focus_bounds, style_.cornerRadius - 2.0f, 2.0f);
    }
}

void Card::resized() {
    Component::resized();
    
    auto bounds = getLocalBounds();
    
    // Calculate layout areas
    auto header_bounds = getHeaderBounds();
    auto content_bounds = getContentBounds();
    auto footer_bounds = getFooterBounds();
    
    // Position content components
    float content_y = content_bounds.getY();
    for (auto& component : contentComponents_) {
        auto component_bounds = component->getBounds();
        component_bounds.setY(content_y);
        component->setBounds(component_bounds);
        content_y += component_bounds.getHeight() + style_.spacing;
    }
}

bool Card::mouseDown(const juce::MouseEvent& e) {
    if (!enabled_ || !style_.clickable) return false;
    
    auto position = e.getPosition().toFloat();
    
    // Check if click is in action buttons
    for (size_t i = 0; i < style_.actions.size(); ++i) {
        auto action_bounds = getActionButtonBounds(static_cast<int>(i));
        if (action_bounds.contains(position)) {
            if (actionCallback_) {
                actionCallback_(static_cast<int>(i), style_.actions[i]);
                announceToScreenReader("Action clicked: " + juce::String(style_.actions[i]));
            }
            return true;
        }
    }
    
    // Click on card
    isPressed_ = true;
    setState(State::Pressed);
    
    if (clickCallback_) {
        clickCallback_();
    }
    
    return true;
}

bool Card::mouseUp(const juce::MouseEvent& e) {
    if (!enabled_) return false;
    
    isPressed_ = false;
    handleStateChanges();
    
    return true;
}

void Card::mouseEnter(const juce::MouseEvent& e) {
    if (!enabled_ || !style_.clickable) return;
    
    isHovered_ = true;
    handleStateChanges();
}

void Card::mouseExit(const juce::MouseEvent& e) {
    if (!enabled_) return;
    
    isHovered_ = false;
    handleStateChanges();
}

void Card::focusGained() {
    if (style_.clickable) {
        setState(State::Focused);
        announceToScreenReader("Card focused: " + juce::String(style_.title));
    }
}

void Card::focusLost() {
    setState(State::Normal);
}

void Card::addContentComponent(std::unique_ptr<Component> component) {
    if (component) {
        contentComponents_.push_back(std::move(component));
        setNeedsRedraw();
    }
}

void Card::removeContentComponent(Component* component) {
    contentComponents_.erase(
        std::remove_if(contentComponents_.begin(), contentComponents_.end(),
                      [component](const auto& ptr) { return ptr.get() == component; }),
        contentComponents_.end());
    setNeedsRedraw();
}

void Card::clearContentComponents() {
    contentComponents_.clear();
    setNeedsRedraw();
}

void Card::setStyle(const CardStyle& style) {
    style_ = style;
    setInterceptsMouseClicks(style_.clickable, style_.clickable);
    setWantsKeyboardFocus(style_.clickable);
    setNeedsRedraw();
}

void Card::setTitle(const std::string& title) {
    style_.title = title;
    setAriaLabel(title);
    setNeedsRedraw();
}

void Card::setSubtitle(const std::string& subtitle) {
    style_.subtitle = subtitle;
    setNeedsRedraw();
}

void Card::setSupportingText(const std::string& text) {
    style_.supportingText = text;
    setNeedsRedraw();
}

void Card::setActions(const std::vector<std::string>& actions) {
    style_.actions = actions;
    setNeedsRedraw();
}

void Card::setCornerRadius(float radius) {
    style_.cornerRadius = std::max(0.0f, radius);
    setNeedsRedraw();
}

void Card::setElevation(float elevation) {
    style_.elevation = std::max(0.0f, elevation);
    elevation_.animateTo(style_.elevation);
    setNeedsRedraw();
}

void Card::setBackgroundColor(const Color& color) {
    style_.backgroundColor = color;
    backgroundColor_.animateTo(color);
    setNeedsRedraw();
}

void Card::setBorderColor(const Color& color) {
    style_.borderColor = color;
    borderColor_.animateTo(color);
    setNeedsRedraw();
}

void Card::setBorderWidth(float width) {
    style_.borderWidth = std::max(0.0f, width);
    setNeedsRedraw();
}

void Card::setClickable(bool clickable) {
    style_.clickable = clickable;
    setInterceptsMouseClicks(clickable, clickable);
    setWantsKeyboardFocus(clickable);
    setNeedsRedraw();
}

void Card::setSelected(bool selected) {
    if (selected_ != selected) {
        selected_ = selected;
        setNeedsRedraw();
        
        announceToScreenReader(selected ? "Card selected" : "Card deselected");
    }
}

void Card::setCardClickCallback(CardClickCallback callback) {
    clickCallback_ = callback;
}

void Card::setActionClickCallback(ActionClickCallback callback) {
    actionCallback_ = callback;
}

juce::String Card::getAccessibilityLabel() const {
    return juce::String(style_.title);
}

juce::String Card::getAccessibilityValue() const {
    if (selected_) {
        return "Selected";
    }
    return "Not selected";
}

void Card::performAccessibilityAction(const juce::String& actionName) {
    if (actionName == "click" || actionName == "activate") {
        if (clickCallback_) {
            clickCallback_();
        }
    } else if (actionName == "toggle") {
        setSelected(!selected_);
    }
}

void Card::setupAnimations() {
    elevation_.reset(style_.elevation);
    elevation_.setDuration(200.0f);
    elevation_.setEasing(AnimationEasing::easeInOutCubic);
    
    hoverOpacity_.reset(0.0f);
    hoverOpacity_.setDuration(150.0f);
    hoverOpacity_.setEasing(AnimationEasing::easeOutCubic);
    
    backgroundColor_.reset(style_.backgroundColor);
    backgroundColor_.setDuration(200.0f);
    backgroundColor_.setEasing(AnimationEasing::easeInOutCubic);
    
    borderColor_.reset(style_.borderColor);
    borderColor_.setDuration(200.0f);
    borderColor_.setEasing(AnimationEasing::easeInOutCubic);
}

juce::Rectangle<float> Card::getCardBounds() const {
    auto bounds = getLocalBounds().toFloat();
    return bounds.reduced(style_.padding);
}

juce::Rectangle<float> Card::getHeaderBounds() const {
    if (style_.clickable || !style_.title.empty() || !style_.subtitle.empty()) {
        auto card_bounds = getCardBounds();
        return juce::Rectangle<float>(card_bounds.getX(), card_bounds.getY(),
                                     card_bounds.getWidth(), style_.headerHeight);
    }
    return juce::Rectangle<float>();
}

juce::Rectangle<float> Card::getContentBounds() const {
    auto card_bounds = getCardBounds();
    float header_height = getHeaderBounds().isEmpty() ? 0.0f : style_.headerHeight;
    float footer_height = getFooterBounds().isEmpty() ? 0.0f : style_.footerHeight;
    
    return juce::Rectangle<float>(card_bounds.getX(), 
                                 card_bounds.getY() + header_height + style_.spacing,
                                 card_bounds.getWidth(),
                                 card_bounds.getHeight() - header_height - footer_height - style_.spacing * 2.0f);
}

juce::Rectangle<float> Card::getFooterBounds() const {
    if (!style_.actions.empty() || !style_.supportingText.empty()) {
        auto card_bounds = getCardBounds();
        return juce::Rectangle<float>(card_bounds.getX(),
                                     card_bounds.getBottom() - style_.footerHeight,
                                     card_bounds.getWidth(), style_.footerHeight);
    }
    return juce::Rectangle<float>();
}

juce::Rectangle<float> Card::getActionButtonBounds(int index) const {
    if (index < 0 || index >= static_cast<int>(style_.actions.size())) {
        return juce::Rectangle<float>();
    }
    
    auto footer_bounds = getFooterBounds();
    if (footer_bounds.isEmpty()) {
        return juce::Rectangle<float>();
    }
    
    // Calculate button layout
    float button_width = 48.0f; // Material Design standard
    float button_height = 36.0f;
    float spacing = 8.0f;
    
    float total_width = style_.actions.size() * button_width + 
                       (style_.actions.size() - 1) * spacing;
    float start_x = footer_bounds.getRight() - total_width - 16.0f;
    float button_y = footer_bounds.getCentreY() - button_height * 0.5f;
    
    return juce::Rectangle<float>(start_x + index * (button_width + spacing), 
                                 button_y, button_width, button_height);
}

void Card::drawHeader(juce::Graphics& g) {
    auto header_bounds = getHeaderBounds();
    if (header_bounds.isEmpty()) return;
    
    auto theme = getTheme();
    auto colors = theme->getMaterialColors();
    
    // Draw title
    if (!style_.title.empty()) {
        g.setColour(colors.onSurface);
        g.setFont(juce::Font(20.0f, juce::Font::bold));
        float title_y = header_bounds.getY() + 20.0f;
        g.drawText(style_.title, 
                  header_bounds.getX() + 16.0f, title_y,
                  header_bounds.getWidth() - 32.0f, 24.0f,
                  juce::Justification::left);
    }
    
    // Draw subtitle
    if (!style_.subtitle.empty()) {
        g.setColour(colors.onSurface.withAlpha(0.7f));
        g.setFont(juce::Font(14.0f));
        float subtitle_y = header_bounds.getY() + 40.0f;
        g.drawText(style_.subtitle,
                  header_bounds.getX() + 16.0f, subtitle_y,
                  header_bounds.getWidth() - 32.0f, 16.0f,
                  juce::Justification::left);
    }
    
    // Draw divider
    if (style_.showHeaderDivider && (hasKeyboardFocus() || isHovered_ || isPressed_)) {
        g.setColour(colors.outline.withAlpha(0.2f));
        g.drawLine(header_bounds.getX(), header_bounds.getBottom() - 0.5f,
                  header_bounds.getRight(), header_bounds.getBottom() - 0.5f);
    }
}

void Card::drawContent(juce::Graphics& g) {
    auto content_bounds = getContentBounds();
    if (content_bounds.isEmpty() && contentComponents_.empty()) return;
    
    auto theme = getTheme();
    auto colors = theme->getMaterialColors();
    
    // Draw supporting text
    if (!style_.supportingText.empty()) {
        g.setColour(colors.onSurface.withAlpha(0.8f));
        g.setFont(juce::Font(14.0f));
        g.drawText(style_.supportingText,
                  content_bounds.getX() + 16.0f, content_bounds.getY() + 12.0f,
                  content_bounds.getWidth() - 32.0f, content_bounds.getHeight() - 24.0f,
                  juce::Justification::topLeft);
    }
    
    // Draw content components (they will be painted separately)
    for (auto& component : contentComponents_) {
        if (component->isVisible()) {
            component->paint(g);
        }
    }
}

void Card::drawFooter(juce::Graphics& g) {
    auto footer_bounds = getFooterBounds();
    if (footer_bounds.isEmpty()) return;
    
    auto theme = getTheme();
    auto colors = theme->getMaterialColors();
    
    // Draw divider
    if (style_.showFooterDivider) {
        g.setColour(colors.outline.withAlpha(0.2f));
        g.drawLine(footer_bounds.getX(), footer_bounds.getY() + 0.5f,
                  footer_bounds.getRight(), footer_bounds.getY() + 0.5f);
    }
}

void Card::drawActions(juce::Graphics& g) {
    if (style_.actions.empty()) return;
    
    auto theme = getTheme();
    auto colors = theme->getMaterialColors();
    
    for (size_t i = 0; i < style_.actions.size(); ++i) {
        auto action_bounds = getActionButtonBounds(static_cast<int>(i));
        
        // Draw button background on hover
        if (hover_bounds_.contains(action_bounds.getCentre().toInt())) {
            g.setColour(colors.primary.withAlpha(0.08f));
            g.fillRoundedRectangle(action_bounds.reduced(4.0f), 4.0f);
        }
        
        // Draw action text
        g.setColour(colors.primary);
        g.setFont(juce::Font(14.0f, juce::Font::bold));
        g.drawText(style_.actions[i], action_bounds, juce::Justification::centred);
    }
}

void Card::handleStateChanges() {
    if (!enabled_) {
        setState(State::Disabled);
        hoverOpacity_.animateTo(0.0f);
        return;
    }
    
    if (isPressed_) {
        setState(State::Pressed);
        hoverOpacity_.animateTo(1.0f);
        elevation_.animateTo(std::max(0.0f, style_.elevation * 0.8f));
    } else if (isHovered_) {
        setState(State::Hover);
        hoverOpacity_.animateTo(1.0f);
        elevation_.animateTo(std::min(style_.elevation * 1.2f, style_.elevation + 4.0f));
    } else {
        setState(State::Normal);
        hoverOpacity_.animateTo(0.0f);
        elevation_.animateTo(style_.elevation);
    }
}

void Card::update() {
    Component::update();
    
    // Update animations
    elevation_.update(0.016f);
    hoverOpacity_.update(0.016f);
    backgroundColor_.update(0.016f);
    borderColor_.update(0.016f);
}

juce::String Card::announceToScreenReader(const juce::String& message) {
    auto accessibility_manager = getAccessibilityManager();
    if (accessibility_manager && accessibility_manager->isFeatureEnabled(
        accessibility::AccessibilityFeature::ScreenReader)) {
        accessibility_manager->announceToScreenReader(message);
    }
    return message;
}

} // namespace material
} // namespace ui
} // namespace vital