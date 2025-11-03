#include "theme_manager.h"
#include <algorithm>
#include <cmath>

namespace vital {
namespace ui {
namespace theme {

//==============================================================================
// ThemeManager Implementation
//==============================================================================

ThemeManager::ThemeManager()
    : current_theme_name_("Light"),
      current_theme_(nullptr),
      current_theme_type_(ThemeType::Light),
      theme_transition_enabled_(true),
      theme_transition_duration_ms_(300.0f),
      is_transitioning_(false) {
    
    initializeDefaultThemes();
}

ThemeManager::~ThemeManager() = default;

void ThemeManager::registerTheme(const MaterialTheme& theme) {
    themes_[theme.name] = theme;
    
    // If this is the first theme, make it current
    if (themes_.size() == 1) {
        setCurrentTheme(theme.name);
    }
}

void ThemeManager::removeTheme(const juce::String& theme_name) {
    auto it = themes_.find(theme_name);
    if (it != themes_.end()) {
        themes_.erase(it);
        
        // If we removed the current theme, switch to first available
        if (current_theme_name_ == theme_name && !themes_.empty()) {
            setCurrentTheme(themes_.begin()->first);
        }
    }
}

MaterialTheme* ThemeManager::getTheme(const juce::String& theme_name) const {
    auto it = themes_.find(theme_name);
    return (it != themes_.end()) ? &const_cast<MaterialTheme&>(it->second) : nullptr;
}

std::vector<juce::String> ThemeManager::getAllThemeNames() const {
    std::vector<juce::String> names;
    names.reserve(themes_.size());
    
    for (const auto& pair : themes_) {
        names.push_back(pair.first);
    }
    
    return names;
}

void ThemeManager::setCurrentTheme(const juce::String& theme_name, bool animate) {
    auto new_theme = getTheme(theme_name);
    if (!new_theme) return;
    
    MaterialTheme* old_theme = current_theme_;
    juce::String old_theme_name = current_theme_name_;
    
    current_theme_ = new_theme;
    current_theme_name_ = theme_name;
    current_theme_type_ = new_theme->type;
    
    if (animate && theme_transition_enabled_) {
        is_transitioning_ = true;
        applyThemeTransition(*old_theme, *new_theme);
        is_transitioning_ = false;
    }
    
    notifyThemeChange(old_theme_name, theme_name);
}

MaterialTheme* ThemeManager::getCurrentTheme() const {
    return current_theme_;
}

void ThemeManager::setThemeType(ThemeType type) {
    current_theme_type_ = type;
}

// Built-in themes
MaterialTheme ThemeManager::createLightTheme(const juce::Colour& seed_color) {
    MaterialTheme theme;
    theme.name = "Light_" + seed_color.toString();
    theme.type = ThemeType::Light;
    theme.description = "Light theme with custom seed color";
    
    // Generate color palette from seed
    auto primary = generatePrimaryColor(seed_color, ThemeType::Light);
    auto secondary = generateSecondaryColor(primary);
    auto tertiary = generateTertiaryColor(primary);
    auto surface = generateSurfaceColor(ThemeType::Light);
    auto error = generateErrorColor();
    
    // Set Material Design 3.0 color roles
    theme.colors[ColorRole::Primary] = primary;
    theme.colors[ColorRole::OnPrimary] = getContrastingColor(primary);
    theme.colors[ColorRole::PrimaryContainer] = primary.withAlpha(0.1f);
    theme.colors[ColorRole::OnPrimaryContainer] = primary.withAlpha(0.8f);
    
    theme.colors[ColorRole::Secondary] = secondary;
    theme.colors[ColorRole::OnSecondary] = getContrastingColor(secondary);
    theme.colors[ColorRole::SecondaryContainer] = secondary.withAlpha(0.1f);
    theme.colors[ColorRole::OnSecondaryContainer] = secondary.withAlpha(0.8f);
    
    theme.colors[ColorRole::Tertiary] = tertiary;
    theme.colors[ColorRole::OnTertiary] = getContrastingColor(tertiary);
    theme.colors[ColorRole::TertiaryContainer] = tertiary.withAlpha(0.1f);
    theme.colors[ColorRole::OnTertiaryContainer] = tertiary.withAlpha(0.8f);
    
    theme.colors[ColorRole::Error] = error;
    theme.colors[ColorRole::OnError] = getContrastingColor(error);
    theme.colors[ColorRole::ErrorContainer] = error.withAlpha(0.1f);
    theme.colors[ColorRole::OnErrorContainer] = error.withAlpha(0.8f);
    
    theme.colors[ColorRole::Background] = surface;
    theme.colors[ColorRole::OnBackground] = getContrastingColor(surface);
    theme.colors[ColorRole::Surface] = surface;
    theme.colors[ColorRole::OnSurface] = getContrastingColor(surface);
    theme.colors[ColorRole::SurfaceVariant] = surface.darker(0.1f);
    theme.colors[ColorRole::OnSurfaceVariant] = getContrastingColor(surface.darker(0.1f));
    
    theme.colors[ColorRole::Outline] = surface.darker(0.3f);
    theme.colors[ColorRole::OutlineVariant] = surface.darker(0.2f);
    
    theme.colors[ColorRole::InverseSurface] = surface.darker(0.8f);
    theme.colors[ColorRole::InverseOnSurface] = getContrastingColor(surface.darker(0.8f));
    theme.colors[ColorRole::InversePrimary] = primary.withAlpha(0.8f);
    
    // Initialize typography
    initializeTypography(theme);
    
    // Initialize component states
    initializeComponentStates(theme);
    
    // Initialize spacing and shape systems
    initializeSpacingAndShapes(theme);
    
    // Initialize elevation system
    initializeElevationSystem(theme);
    
    // Initialize animations
    initializeAnimations(theme);
    
    return theme;
}

MaterialTheme ThemeManager::createDarkTheme(const juce::Colour& seed_color) {
    MaterialTheme theme;
    theme.name = "Dark_" + seed_color.toString();
    theme.type = ThemeType::Dark;
    theme.description = "Dark theme with custom seed color";
    
    // Generate color palette from seed
    auto primary = generatePrimaryColor(seed_color, ThemeType::Dark);
    auto secondary = generateSecondaryColor(primary);
    auto tertiary = generateTertiaryColor(primary);
    auto surface = generateSurfaceColor(ThemeType::Dark);
    auto error = generateErrorColor();
    
    // Set Material Design 3.0 color roles for dark theme
    theme.colors[ColorRole::Primary] = primary;
    theme.colors[ColorRole::OnPrimary] = getContrastingColor(primary);
    theme.colors[ColorRole::PrimaryContainer] = primary.brighter(0.3f);
    theme.colors[ColorRole::OnPrimaryContainer] = primary.darker(0.3f);
    
    theme.colors[ColorRole::Secondary] = secondary;
    theme.colors[ColorRole::OnSecondary] = getContrastingColor(secondary);
    theme.colors[ColorRole::SecondaryContainer] = secondary.brighter(0.3f);
    theme.colors[ColorRole::OnSecondaryContainer] = secondary.darker(0.3f);
    
    theme.colors[ColorRole::Tertiary] = tertiary;
    theme.colors[ColorRole::OnTertiary] = getContrastingColor(tertiary);
    theme.colors[ColorRole::TertiaryContainer] = tertiary.brighter(0.3f);
    theme.colors[ColorRole::OnTertiaryContainer] = tertiary.darker(0.3f);
    
    theme.colors[ColorRole::Error] = error;
    theme.colors[ColorRole::OnError] = getContrastingColor(error);
    theme.colors[ColorRole::ErrorContainer] = error.brighter(0.3f);
    theme.colors[ColorRole::OnErrorContainer] = error.darker(0.3f);
    
    theme.colors[ColorRole::Background] = surface;
    theme.colors[ColorRole::OnBackground] = getContrastingColor(surface);
    theme.colors[ColorRole::Surface] = surface.brighter(0.1f);
    theme.colors[ColorRole::OnSurface] = getContrastingColor(surface.brighter(0.1f));
    theme.colors[ColorRole::SurfaceVariant] = surface.brighter(0.2f);
    theme.colors[ColorRole::OnSurfaceVariant] = getContrastingColor(surface.brighter(0.2f));
    
    theme.colors[ColorRole::Outline] = surface.brighter(0.5f);
    theme.colors[ColorRole::OutlineVariant] = surface.brighter(0.4f);
    
    theme.colors[ColorRole::InverseSurface] = surface.brighter(0.8f);
    theme.colors[ColorRole::InverseOnSurface] = getContrastingColor(surface.brighter(0.8f));
    theme.colors[ColorRole::InversePrimary] = primary.withAlpha(0.8f);
    
    // Initialize typography
    initializeTypography(theme);
    
    // Initialize component states
    initializeComponentStates(theme);
    
    // Initialize spacing and shape systems
    initializeSpacingAndShapes(theme);
    
    // Initialize elevation system
    initializeElevationSystem(theme);
    
    // Initialize animations
    initializeAnimations(theme);
    
    return theme;
}

MaterialTheme ThemeManager::createHighContrastTheme(ThemeType type) {
    MaterialTheme theme;
    theme.name = "HighContrast";
    theme.type = type;
    theme.description = "High contrast theme for accessibility";
    
    // Use high contrast colors
    juce::Colour black = juce::Colours::black;
    juce::Colour white = juce::Colours::white;
    juce::Colour red = juce::Colours::red;
    juce::Colour blue = juce::Colours::blue;
    juce::Colour green = juce::Colours::limegreen;
    
    if (type == ThemeType::HighContrast) {
        // Light high contrast
        theme.colors[ColorRole::Primary] = blue;
        theme.colors[ColorRole::OnPrimary] = white;
        theme.colors[ColorRole::Background] = white;
        theme.colors[ColorRole::OnBackground] = black;
        theme.colors[ColorRole::Surface] = white;
        theme.colors[ColorRole::OnSurface] = black;
        theme.colors[ColorRole::Error] = red;
        theme.colors[ColorRole::OnError] = white;
    } else {
        // Dark high contrast
        theme.colors[ColorRole::Primary] = blue;
        theme.colors[ColorRole::OnPrimary] = white;
        theme.colors[ColorRole::Background] = black;
        theme.colors[ColorRole::OnBackground] = white;
        theme.colors[ColorRole::Surface] = black;
        theme.colors[ColorRole::OnSurface] = white;
        theme.colors[ColorRole::Error] = red;
        theme.colors[ColorRole::OnError] = white;
    }
    
    // Initialize typography with larger fonts for accessibility
    initializeTypography(theme, true);
    
    // Initialize component states
    initializeComponentStates(theme);
    
    // Initialize spacing and shape systems
    initializeSpacingAndShapes(theme);
    
    // Initialize elevation system
    initializeElevationSystem(theme);
    
    // Initialize animations
    initializeAnimations(theme);
    
    return theme;
}

void ThemeManager::initializeDefaultThemes() {
    // Create and register default themes
    auto light_blue = createLightTheme(juce::Colours::blue);
    auto dark_blue = createDarkTheme(juce::Colours::blue);
    auto high_contrast = createHighContrastTheme(ThemeType::HighContrast);
    
    registerTheme(light_blue);
    registerTheme(dark_blue);
    registerTheme(high_contrast);
}

// Color system
juce::Colour ThemeManager::getColor(ColorRole role) const {
    if (current_theme_ && current_theme_->colors.count(role)) {
        return current_theme_->colors.at(role);
    }
    
    // Return default colors if theme is not set
    switch (role) {
        case ColorRole::Primary: return juce::Colours::blue;
        case ColorRole::Background: return juce::Colours::white;
        case ColorRole::Surface: return juce::Colours::lightgrey;
        default: return juce::Colours::black;
    }
}

void ThemeManager::setColor(ColorRole role, const juce::Colour& color) {
    if (current_theme_) {
        current_theme_->colors[role] = color;
    }
}

MaterialTheme ThemeManager::generateThemeFromSeed(const juce::Colour& seed_color, ThemeType type) {
    if (type == ThemeType::Light) {
        return createLightTheme(seed_color);
    } else if (type == ThemeType::Dark) {
        return createDarkTheme(seed_color);
    } else {
        return createHighContrastTheme(type);
    }
}

juce::Colour ThemeManager::hslToRgb(float hue, float saturation, float lightness) {
    // HSL to RGB conversion
    float c = (1 - std::fabs(2 * lightness - 1)) * saturation;
    float x = c * (1 - std::fabs(std::fmod(hue / 60.0f, 2) - 1));
    float m = lightness - c / 2.0f;
    
    float r = 0, g = 0, b = 0;
    
    if (0 <= hue && hue < 60) {
        r = c; g = x; b = 0;
    } else if (60 <= hue && hue < 120) {
        r = x; g = c; b = 0;
    } else if (120 <= hue && hue < 180) {
        r = 0; g = c; b = x;
    } else if (180 <= hue && hue < 240) {
        r = 0; g = x; b = c;
    } else if (240 <= hue && hue < 300) {
        r = x; g = 0; b = c;
    } else if (300 <= hue && hue < 360) {
        r = c; g = 0; b = x;
    }
    
    return juce::Colour::fromFloatRGBA(r + m, g + m, b + m, 1.0f);
}

juce::Colour ThemeManager::getContrastingColor(const juce::Colour& color, 
                                               const juce::Colour& preferred_color) const {
    // Calculate contrast ratio with white and black
    float contrast_with_white = calculateContrastRatio(juce::Colours::white, color);
    float contrast_with_black = calculateContrastRatio(juce::Colours::black, color);
    
    // Use preferred color if it has better contrast
    if (preferred_color != juce::Colours::black) {
        float preferred_contrast = calculateContrastRatio(preferred_color, color);
        if (preferred_contrast >= std::max(contrast_with_white, contrast_with_black)) {
            return preferred_color;
        }
    }
    
    // Return the color that provides better contrast
    return (contrast_with_white > contrast_with_black) ? juce::Colours::white : juce::Colours::black;
}

float ThemeManager::calculateContrastRatio(const juce::Colour& color1, const juce::Colour& color2) {
    float lum1 = calculateLuminance(color1);
    float lum2 = calculateLuminance(color2);
    
    float lighter = std::max(lum1, lum2);
    float darker = std::min(lum1, lum2);
    
    return (lighter + 0.05f) / (darker + 0.05f);
}

bool ThemeManager::meetsWCAGAAContrast(const juce::Colour& foreground, 
                                      const juce::Colour& background, 
                                      bool is_large_text) const {
    float ratio = calculateContrastRatio(foreground, background);
    return is_large_text ? (ratio >= 3.0f) : (ratio >= 4.5f);
}

bool ThemeManager::meetsWCAGAAAContrast(const juce::Colour& foreground, 
                                       const juce::Colour& background, 
                                       bool is_large_text) const {
    float ratio = calculateContrastRatio(foreground, background);
    return is_large_text ? (ratio >= 4.5f) : (ratio >= 7.0f);
}

// Typography system
juce::Font ThemeManager::getFont(TextStyle style) const {
    if (current_theme_ && current_theme_->fonts.count(style)) {
        return current_theme_->fonts.at(style);
    }
    
    // Return default font based on style
    switch (style) {
        case TextStyle::DisplayLarge:
        case TextStyle::DisplayMedium:
        case TextStyle::DisplaySmall:
            return juce::Font(24.0f);
        case TextStyle::HeadlineLarge:
        case TextStyle::HeadlineMedium:
        case TextStyle::HeadlineSmall:
            return juce::Font(20.0f);
        case TextStyle::TitleLarge:
        case TextStyle::TitleMedium:
        case TextStyle::TitleSmall:
            return juce::Font(16.0f);
        default:
            return juce::Font(14.0f);
    }
}

void ThemeManager::setFont(TextStyle style, const juce::Font& font) {
    if (current_theme_) {
        current_theme_->fonts[style] = font;
    }
}

float ThemeManager::getFontSize(TextStyle style) const {
    if (current_theme_ && current_theme_->font_sizes.count(style)) {
        return current_theme_->font_sizes.at(style);
    }
    
    // Return default font size based on style
    switch (style) {
        case TextStyle::DisplayLarge: return 57.0f;
        case TextStyle::DisplayMedium: return 45.0f;
        case TextStyle::DisplaySmall: return 36.0f;
        case TextStyle::HeadlineLarge: return 32.0f;
        case TextStyle::HeadlineMedium: return 28.0f;
        case TextStyle::HeadlineSmall: return 24.0f;
        case TextStyle::TitleLarge: return 22.0f;
        case TextStyle::TitleMedium: return 16.0f;
        case TextStyle::TitleSmall: return 14.0f;
        case TextStyle::LabelLarge: return 14.0f;
        case TextStyle::LabelMedium: return 12.0f;
        case TextStyle::LabelSmall: return 11.0f;
        case TextStyle::BodyLarge: return 16.0f;
        case TextStyle::BodyMedium: return 14.0f;
        case TextStyle::BodySmall: return 12.0f;
        default: return 14.0f;
    }
}

void ThemeManager::setFontSize(TextStyle style, float size) {
    if (current_theme_) {
        current_theme_->font_sizes[style] = size;
    }
}

float ThemeManager::getLineHeight(TextStyle style) const {
    if (current_theme_ && current_theme_->line_heights.count(style)) {
        return current_theme_->line_heights.at(style);
    }
    
    // Return default line height based on font size
    return getFontSize(style) * 1.2f;
}

void ThemeManager::setLineHeight(TextStyle style, float height) {
    if (current_theme_) {
        current_theme_->line_heights[style] = height;
    }
}

float ThemeManager::getLetterSpacing(TextStyle style) const {
    if (current_theme_ && current_theme_->letter_spacings.count(style)) {
        return current_theme_->letter_spacings.at(style);
    }
    
    // Default letter spacing
    return 0.0f;
}

void ThemeManager::setLetterSpacing(TextStyle style, float spacing) {
    if (current_theme_) {
        current_theme_->letter_spacings[style] = spacing;
    }
}

// Component states
StateColors ThemeManager::getComponentStateColors(const juce::String& component_name) const {
    if (current_theme_ && current_theme_->component_states.count(component_name)) {
        return current_theme_->component_states.at(component_name);
    }
    
    // Return default state colors
    return StateColors(getColor(ColorRole::Primary));
}

void ThemeManager::setComponentStateColors(const juce::String& component_name, const StateColors& colors) {
    if (current_theme_) {
        current_theme_->component_states[component_name] = colors;
    }
}

juce::Colour ThemeManager::getComponentStateColor(const juce::String& component_name, 
                                                 core::Component::State state) const {
    auto state_colors = getComponentStateColors(component_name);
    
    switch (state) {
        case core::Component::State::Normal:
            return state_colors.normal;
        case core::Component::State::Hover:
            return state_colors.hover;
        case core::Component::State::Focused:
            return state_colors.focused;
        case core::Component::State::Pressed:
            return state_colors.pressed;
        case core::Component::State::Disabled:
            return state_colors.disabled;
        default:
            return state_colors.normal;
    }
}

// Spacing and layout
float ThemeManager::getSpacing(const juce::String& spacing_name) const {
    if (current_theme_ && current_theme_->spacing_scale.count(spacing_name)) {
        return current_theme_->spacing_scale.at(spacing_name);
    }
    
    // Return default spacing based on name
    if (spacing_name == "xs") return 4.0f;
    if (spacing_name == "sm") return 8.0f;
    if (spacing_name == "md") return 16.0f;
    if (spacing_name == "lg") return 24.0f;
    if (spacing_name == "xl") return 32.0f;
    
    return 8.0f; // Default spacing unit
}

void ThemeManager::setSpacing(const juce::String& spacing_name, float value) {
    if (current_theme_) {
        current_theme_->spacing_scale[spacing_name] = value;
    }
}

float ThemeManager::getCornerRadius(ShapeCorner corner) const {
    if (current_theme_ && current_theme_->corner_radii.count(corner)) {
        return current_theme_->corner_radii.at(corner);
    }
    
    // Return default corner radius
    switch (corner) {
        case ShapeCorner::None: return 0.0f;
        case ShapeCorner::ExtraSmall: return 2.0f;
        case ShapeCorner::Small: return 4.0f;
        case ShapeCorner::Medium: return 8.0f;
        case ShapeCorner::Large: return 12.0f;
        case ShapeCorner::ExtraLarge: return 16.0f;
        case ShapeCorner::Full: return 50.0f;
        default: return 4.0f;
    }
}

void ThemeManager::setCornerRadius(ShapeCorner corner, float radius) {
    if (current_theme_) {
        current_theme_->corner_radii[corner] = radius;
    }
}

ShapeConfig ThemeManager::getComponentShape(const juce::String& component_name) const {
    if (current_theme_ && current_theme_->component_shapes.count(component_name)) {
        return current_theme_->component_shapes.at(component_name);
    }
    
    // Return default shape
    return ShapeConfig(getCornerRadius(ShapeCorner::Small));
}

void ThemeManager::setComponentShape(const juce::String& component_name, const ShapeConfig& shape) {
    if (current_theme_) {
        current_theme_->component_shapes[component_name] = shape;
    }
}

// Elevation system
float ThemeManager::getElevationBlur(ElevationLevel level) const {
    if (current_theme_ && current_theme_->elevation_blurs.count(level)) {
        return current_theme_->elevation_blurs.at(level);
    }
    
    // Return default blur amount
    return static_cast<float>(level) * 2.0f;
}

float ThemeManager::getElevationOpacity(ElevationLevel level) const {
    if (current_theme_ && current_theme_->elevation_opacities.count(level)) {
        return current_theme_->elevation_opacities.at(level);
    }
    
    // Return default opacity
    return std::min(0.3f, static_cast<float>(level) * 0.05f);
}

juce::Point<float> ThemeManager::getElevationOffset(ElevationLevel level) const {
    if (current_theme_ && current_theme_->elevation_offsets.count(level)) {
        return current_theme_->elevation_offsets.at(level);
    }
    
    // Return default offset
    return juce::Point<float>(0.0f, static_cast<float>(level) * 0.5f);
}

juce::Colour ThemeManager::createShadowColor(ElevationLevel level) const {
    float opacity = getElevationOpacity(level);
    return juce::Colours::black.withAlpha(opacity);
}

// Animation
float ThemeManager::getAnimationDuration(const juce::String& animation_name) const {
    if (current_theme_ && current_theme_->animation_durations.count(animation_name)) {
        return current_theme_->animation_durations.at(animation_name);
    }
    
    // Return default animation duration
    if (animation_name == "quick") return 150.0f;
    if (animation_name == "standard") return 300.0f;
    if (animation_name == "slow") return 500.0f;
    
    return 300.0f; // Default duration
}

void ThemeManager::setAnimationDuration(const juce::String& animation_name, float duration) {
    if (current_theme_) {
        current_theme_->animation_durations[animation_name] = duration;
    }
}

// Theme transitions
void ThemeManager::setThemeTransitionEnabled(bool enabled) {
    theme_transition_enabled_ = enabled;
}

void ThemeManager::setThemeTransitionDuration(float duration) {
    theme_transition_duration_ms_ = std::max(duration, 0.0f);
}

// Accessibility integration
bool ThemeManager::isThemeAccessibilityFriendly(const juce::String& theme_name) const {
    auto theme = getTheme(theme_name);
    return theme ? validateAccessibility(*theme) : false;
}

juce::Colour ThemeManager::getRecommendedTextColor(const juce::Colour& background_color,
                                                  const juce::String& accessibility_level,
                                                  bool is_large_text) const {
    juce::Colour white = juce::Colours::white;
    juce::Colour black = juce::Colours::black;
    
    // Check which color provides better contrast
    bool white_meets_requirement = (accessibility_level == "AAA") ?
        meetsWCAGAAAContrast(white, background_color, is_large_text) :
        meetsWCAGAAContrast(white, background_color, is_large_text);
        
    bool black_meets_requirement = (accessibility_level == "AAA") ?
        meetsWCAGAAAContrast(black, background_color, is_large_text) :
        meetsWCAGAAContrast(black, background_color, is_large_text);
    
    if (white_meets_requirement && black_meets_requirement) {
        // Both work, choose based on background brightness
        return (background_color.getBrightness() > 0.5f) ? black : white;
    } else if (white_meets_requirement) {
        return white;
    } else if (black_meets_requirement) {
        return black;
    } else {
        // Neither meets the requirement, return the one with better contrast
        float white_contrast = calculateContrastRatio(white, background_color);
        float black_contrast = calculateContrastRatio(black, background_color);
        return (white_contrast > black_contrast) ? white : black;
    }
}

MaterialTheme ThemeManager::autoAdjustForAccessibility(const MaterialTheme& theme,
                                                       const juce::String& accessibility_level) const {
    // This is a simplified implementation
    // In a real implementation, you would adjust colors to meet contrast requirements
    return theme;
}

// Custom properties
juce::var ThemeManager::getCustomProperty(const juce::String& property_name) const {
    if (current_theme_ && current_theme_->custom_properties.count(property_name)) {
        return current_theme_->custom_properties.at(property_name);
    }
    return juce::var();
}

void ThemeManager::setCustomProperty(const juce::String& property_name, const juce::var& value) {
    if (current_theme_) {
        current_theme_->custom_properties[property_name] = value;
    }
}

void ThemeManager::removeCustomProperty(const juce::String& property_name) {
    if (current_theme_) {
        current_theme_->custom_properties.erase(property_name);
    }
}

std::vector<juce::String> ThemeManager::getAllCustomPropertyNames() const {
    std::vector<juce::String> names;
    if (current_theme_) {
        for (const auto& pair : current_theme_->custom_properties) {
            names.push_back(pair.first);
        }
    }
    return names;
}

// Theme callbacks
int ThemeManager::registerThemeChangeCallback(std::function<void(const juce::String&, const juce::String&)> callback) {
    std::lock_guard<std::mutex> lock(callbacks_mutex_);
    theme_change_callbacks_.push_back(callback);
    return static_cast<int>(theme_change_callbacks_.size() - 1);
}

void ThemeManager::unregisterThemeChangeCallback(int callback_id) {
    std::lock_guard<std::mutex> lock(callbacks_mutex_);
    if (callback_id >= 0 && callback_id < static_cast<int>(theme_change_callbacks_.size())) {
        theme_change_callbacks_[callback_id] = nullptr;
    }
}

void ThemeManager::notifyThemeChange(const juce::String& old_theme_name, const juce::String& new_theme_name) {
    std::lock_guard<std::mutex> lock(callbacks_mutex_);
    for (auto& callback : theme_change_callbacks_) {
        if (callback) {
            callback(old_theme_name, new_theme_name);
        }
    }
}

// Serialization
bool ThemeManager::saveThemeToFile(const juce::String& theme_name, const juce::String& file_path) const {
    // Simplified implementation - would use proper JSON serialization
    auto theme = getTheme(theme_name);
    if (!theme) return false;
    
    juce::File file(file_path);
    if (!file.exists()) {
        file.create();
    }
    
    // For now, just save theme name and basic info
    auto content = "Theme: " + theme->name + "\n" +
                   "Type: " + juce::String(static_cast<int>(theme->type)) + "\n" +
                   "Description: " + theme->description;
    
    return file.replaceWithText(content);
}

MaterialTheme ThemeManager::loadThemeFromFile(const juce::String& file_path) const {
    juce::File file(file_path);
    if (!file.exists()) {
        return MaterialTheme(); // Return default theme
    }
    
    auto content = file.loadFileAsString();
    // Simplified parsing - would use proper JSON parsing
    MaterialTheme theme;
    theme.name = "Loaded_" + juce::Time::getCurrentTime().toString();
    theme.type = ThemeType::Light;
    theme.description = "Theme loaded from file";
    
    return theme;
}

juce::String ThemeManager::exportThemeAsJSON(const juce::String& theme_name) const {
    auto theme = getTheme(theme_name);
    if (!theme) return "{}";
    
    // Simplified JSON export
    juce::String json = "{";
    json += "\"name\": \"" + theme->name + "\",";
    json += "\"type\": " + juce::String(static_cast<int>(theme->type)) + ",";
    json += "\"description\": \"" + theme->description + "\"";
    json += "}";
    
    return json;
}

MaterialTheme ThemeManager::importThemeFromJSON(const juce::String& json_string) const {
    // Simplified JSON import - would use proper JSON parsing
    MaterialTheme theme;
    theme.name = "Imported_" + juce::Time::getCurrentTime().toString();
    theme.type = ThemeType::Light;
    theme.description = "Theme imported from JSON";
    
    return theme;
}

// Private methods
void ThemeManager::applyThemeTransition(const MaterialTheme& old_theme, const MaterialTheme& new_theme) {
    // This would implement smooth theme transitions
    // For now, just set the new theme immediately
    current_theme_ = const_cast<MaterialTheme*>(&new_theme);
}

void ThemeManager::updateComponentThemeReferences() {
    // This would update all components to use the new theme
    // For now, it's a placeholder
}

void ThemeManager::validateTheme(const MaterialTheme& theme) const {
    // Validate theme has all required properties
    // This is a simplified validation
}

juce::Colour ThemeManager::generatePrimaryColor(const juce::Colour& seed_color, ThemeType type) const {
    // Generate a harmonious primary color from seed
    float hue = seed_color.getHue();
    float saturation = seed_color.getSaturation();
    float brightness = seed_color.getBrightness();
    
    // Adjust for theme type
    if (type == ThemeType::Dark) {
        saturation = std::min(saturation * 1.2f, 1.0f);
        brightness = std::min(brightness * 0.8f, 1.0f);
    }
    
    return juce::Colour::fromHSV(hue, saturation, brightness, 1.0f);
}

juce::Colour ThemeManager::generateSecondaryColor(const juce::Colour& primary_color) const {
    // Generate a harmonious secondary color
    float hue = primary_color.getHue();
    float saturation = primary_color.getSaturation() * 0.7f;
    float brightness = primary_color.getBrightness() * 0.9f;
    
    return juce::Colour::fromHSV(hue, saturation, brightness, 1.0f);
}

juce::Colour ThemeManager::generateTertiaryColor(const juce::Colour& primary_color) const {
    // Generate a harmonious tertiary color
    float hue = std::fmod(primary_color.getHue() + 120.0f, 360.0f);
    float saturation = primary_color.getSaturation() * 0.8f;
    float brightness = primary_color.getBrightness() * 0.95f;
    
    return juce::Colour::fromHSV(hue, saturation, brightness, 1.0f);
}

juce::Colour ThemeManager::generateSurfaceColor(ThemeType type) const {
    if (type == ThemeType::Light) {
        return juce::Colour::fromRGB(248, 249, 250); // Light gray
    } else if (type == ThemeType::Dark) {
        return juce::Colour::fromRGB(18, 18, 18); // Dark gray
    } else {
        return juce::Colours::white; // High contrast
    }
}

juce::Colour ThemeManager::generateErrorColor() const {
    return juce::Colour::fromRGB(211, 47, 47); // Material Design error red
}

float ThemeManager::calculateLuminance(const juce::Colour& color) const {
    // Calculate relative luminance according to WCAG
    float r = color.getRed() / 255.0f;
    float g = color.getGreen() / 255.0f;
    float b = color.getBlue() / 255.0f;
    
    auto linearize = [](float c) {
        return (c <= 0.03928f) ? (c / 12.92f) : std::pow((c + 0.055f) / 1.055f, 2.4f);
    };
    
    float r_linear = linearize(r);
    float g_linear = linearize(g);
    float b_linear = linearize(b);
    
    return 0.2126f * r_linear + 0.7152f * g_linear + 0.0722f * b_linear;
}

bool ThemeManager::ensureMinimumContrast(juce::Colour& foreground, const juce::Colour& background, 
                                        float min_contrast) const {
    // Ensure foreground color meets minimum contrast ratio
    float current_contrast = calculateContrastRatio(foreground, background);
    if (current_contrast >= min_contrast) {
        return true;
    }
    
    // Adjust foreground color to meet contrast requirement
    // This is a simplified implementation
    juce::Colour white = juce::Colours::white;
    juce::Colour black = juce::Colours::black;
    
    float white_contrast = calculateContrastRatio(white, background);
    float black_contrast = calculateContrastRatio(black, background);
    
    if (white_contrast > black_contrast && white_contrast >= min_contrast) {
        foreground = white;
        return true;
    } else if (black_contrast >= min_contrast) {
        foreground = black;
        return true;
    }
    
    return false;
}

bool ThemeManager::validateColorContrast(const MaterialTheme& theme) const {
    // Validate that all color combinations in the theme meet WCAG requirements
    // This is a simplified validation
    return true;
}

bool ThemeManager::validateAccessibility(const MaterialTheme& theme) const {
    // Check if theme meets accessibility requirements
    // This is a simplified implementation
    return validateColorContrast(theme);
}

// Helper methods for theme initialization
void MaterialTheme::initializeDefaults() {
    // Initialize default values
    spacing_unit = 8.0f;
    corner_radii[ShapeCorner::Small] = 4.0f;
    corner_radii[ShapeCorner::Medium] = 8.0f;
    corner_radii[ShapeCorner::Large] = 12.0f;
    
    // Default colors
    colors[ColorRole::Primary] = juce::Colours::blue;
    colors[ColorRole::Background] = juce::Colours::white;
    colors[ColorRole::Surface] = juce::Colours::lightgrey;
}

void ThemeManager::initializeTypography(MaterialTheme& theme, bool high_contrast) {
    float scale = high_contrast ? 1.2f : 1.0f;
    
    theme.font_sizes[TextStyle::DisplayLarge] = 57.0f * scale;
    theme.font_sizes[TextStyle::DisplayMedium] = 45.0f * scale;
    theme.font_sizes[TextStyle::DisplaySmall] = 36.0f * scale;
    theme.font_sizes[TextStyle::HeadlineLarge] = 32.0f * scale;
    theme.font_sizes[TextStyle::HeadlineMedium] = 28.0f * scale;
    theme.font_sizes[TextStyle::HeadlineSmall] = 24.0f * scale;
    theme.font_sizes[TextStyle::TitleLarge] = 22.0f * scale;
    theme.font_sizes[TextStyle::TitleMedium] = 16.0f * scale;
    theme.font_sizes[TextStyle::TitleSmall] = 14.0f * scale;
    theme.font_sizes[TextStyle::LabelLarge] = 14.0f * scale;
    theme.font_sizes[TextStyle::LabelMedium] = 12.0f * scale;
    theme.font_sizes[TextStyle::LabelSmall] = 11.0f * scale;
    theme.font_sizes[TextStyle::BodyLarge] = 16.0f * scale;
    theme.font_sizes[TextStyle::BodyMedium] = 14.0f * scale;
    theme.font_sizes[TextStyle::BodySmall] = 12.0f * scale;
    
    // Initialize line heights
    for (const auto& pair : theme.font_sizes) {
        theme.line_heights[pair.first] = pair.second * 1.2f;
    }
    
    // Initialize fonts
    for (const auto& pair : theme.font_sizes) {
        theme.fonts[pair.first] = juce::Font(pair.second);
    }
}

void ThemeManager::initializeComponentStates(MaterialTheme& theme) {
    // Initialize default component state colors
    StateColors button_colors;
    button_colors.normal = theme.colors[ColorRole::Primary];
    button_colors.hover = button_colors.normal.brighter(0.1f);
    button_colors.focused = button_colors.normal.brighter(0.2f);
    button_colors.pressed = button_colors.normal.darker(0.2f);
    button_colors.disabled = button_colors.normal.withAlpha(0.5f);
    
    theme.component_states["button"] = button_colors;
}

void ThemeManager::initializeSpacingAndShapes(MaterialTheme& theme) {
    // Initialize spacing scale
    theme.spacing_scale["xs"] = 4.0f;
    theme.spacing_scale["sm"] = 8.0f;
    theme.spacing_scale["md"] = 16.0f;
    theme.spacing_scale["lg"] = 24.0f;
    theme.spacing_scale["xl"] = 32.0f;
    theme.spacing_scale["xxl"] = 48.0f;
    
    // Initialize corner radii
    theme.corner_radii[ShapeCorner::None] = 0.0f;
    theme.corner_radii[ShapeCorner::ExtraSmall] = 2.0f;
    theme.corner_radii[ShapeCorner::Small] = 4.0f;
    theme.corner_radii[ShapeCorner::Medium] = 8.0f;
    theme.corner_radii[ShapeCorner::Large] = 12.0f;
    theme.corner_radii[ShapeCorner::ExtraLarge] = 16.0f;
    theme.corner_radii[ShapeCorner::Full] = 50.0f;
}

void ThemeManager::initializeElevationSystem(MaterialTheme& theme) {
    // Initialize elevation levels
    for (int i = 0; i <= 24; i++) {
        auto level = static_cast<ElevationLevel>(i);
        theme.elevation_blurs[level] = i * 2.0f;
        theme.elevation_opacities[level] = std::min(0.3f, i * 0.05f);
        theme.elevation_offsets[level] = juce::Point<float>(0.0f, i * 0.5f);
    }
}

void ThemeManager::initializeAnimations(MaterialTheme& theme) {
    theme.animation_durations["quick"] = 150.0f;
    theme.animation_durations["standard"] = 300.0f;
    theme.animation_durations["slow"] = 500.0f;
}

// Constructor for StateColors
StateColors::StateColors(const juce::Colour& base_color)
    : normal(base_color),
      hover(base_color.brighter(0.1f)),
      focused(base_color.brighter(0.2f)),
      pressed(base_color.darker(0.2f)),
      disabled(base_color.withAlpha(0.5f)) {
}

} // namespace theme
} // namespace ui
} // namespace vital