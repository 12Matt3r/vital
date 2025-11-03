#pragma once

#include <memory>
#include <unordered_map>
#include <vector>
#include <functional>
#include <JuceHeader.h>
#include "../core/component.h"

namespace vital {
namespace ui {
namespace theme {

/**
 * @brief Theme types
 */
enum class ThemeType {
    Light,
    Dark,
    HighContrast,
    Custom
};

/**
 * @brief Material Design 3.0 color roles
 * Following Material You color system
 */
enum class ColorRole {
    // Primary colors
    Primary,
    OnPrimary,
    PrimaryContainer,
    OnPrimaryContainer,
    
    // Secondary colors
    Secondary,
    OnSecondary,
    SecondaryContainer,
    OnSecondaryContainer,
    
    // Tertiary colors
    Tertiary,
    OnTertiary,
    TertiaryContainer,
    OnTertiaryContainer,
    
    // Error colors
    Error,
    OnError,
    ErrorContainer,
    OnErrorContainer,
    
    // Background colors
    Background,
    OnBackground,
    Surface,
    OnSurface,
    SurfaceVariant,
    OnSurfaceVariant,
    
    // Outline and surface variants
    Outline,
    OutlineVariant,
    
    // Inverse colors
    InverseSurface,
    InverseOnSurface,
    InversePrimary,
    
    // Special surfaces
    SurfaceTint,
    SurfaceBright,
    SurfaceDim,
    SurfaceContainerLowest,
    SurfaceContainerLow,
    SurfaceContainer,
    SurfaceContainerHigh,
    SurfaceContainerHighest
};

/**
 * @brief Material Design 3.0 text styles
 */
enum class TextStyle {
    DisplayLarge,
    DisplayMedium,
    DisplaySmall,
    HeadlineLarge,
    HeadlineMedium,
    HeadlineSmall,
    TitleLarge,
    TitleMedium,
    TitleSmall,
    LabelLarge,
    LabelMedium,
    LabelSmall,
    BodyLarge,
    BodyMedium,
    BodySmall
};

/**
 * @brief Elevation levels (0-24)
 */
enum class ElevationLevel {
    Level0 = 0,
    Level1 = 1,
    Level2 = 2,
    Level3 = 3,
    Level4 = 4,
    Level5 = 5,
    Level6 = 6,
    Level7 = 7,
    Level8 = 8,
    Level9 = 9,
    Level10 = 10,
    Level11 = 11,
    Level12 = 12,
    Level13 = 13,
    Level14 = 14,
    Level15 = 15,
    Level16 = 16,
    Level17 = 17,
    Level18 = 18,
    Level19 = 19,
    Level20 = 20,
    Level21 = 21,
    Level22 = 22,
    Level23 = 23,
    Level24 = 24
};

/**
 * @brief Shape corner radius types
 */
enum class ShapeCorner {
    None,
    ExtraSmall,
    Small,
    Medium,
    Large,
    ExtraLarge,
    Full
};

/**
 * @brief Component shape configuration
 */
struct ShapeConfig {
    float top_left = 0.0f;
    float top_right = 0.0f;
    float bottom_right = 0.0f;
    float bottom_left = 0.0f;
    
    ShapeConfig() = default;
    ShapeConfig(float radius) 
        : top_left(radius), top_right(radius), 
          bottom_right(radius), bottom_left(radius) {}
    ShapeConfig(float top, float right, float bottom, float left)
        : top_left(top), top_right(right), 
          bottom_right(bottom), bottom_left(left) {}
};

/**
 * @brief Component state colors (Normal, Hover, Focused, Pressed, Disabled)
 */
struct StateColors {
    juce::Colour normal;
    juce::Colour hover;
    juce::Colour focused;
    juce::Colour pressed;
    juce::Colour disabled;
    
    StateColors() = default;
    StateColors(const juce::Colour& base_color);
};

/**
 * @brief Material Theme - Complete Material Design 3.0 theme definition
 */
struct MaterialTheme {
    // Theme identification
    juce::String name;
    juce::String description;
    ThemeType type = ThemeType::Light;
    juce::String version = "1.0.0";
    
    // Color palette
    std::unordered_map<ColorRole, juce::Colour> colors;
    
    // Typography
    std::unordered_map<TextStyle, juce::Font> fonts;
    std::unordered_map<TextStyle, float> font_sizes;
    std::unordered_map<TextStyle, float> line_heights;
    std::unordered_map<TextStyle, float> letter_spacings;
    
    // Component states
    std::unordered_map<juce::String, StateColors> component_states;
    
    // Spacing scale (8dp grid system)
    float spacing_unit = 8.0f;
    std::unordered_map<juce::String, float> spacing_scale;
    
    // Border radius scale
    std::unordered_map<ShapeCorner, float> corner_radii;
    std::unordered_map<juce::String, ShapeConfig> component_shapes;
    
    // Elevation levels
    std::unordered_map<ElevationLevel, float> elevation_blurs;
    std::unordered_map<ElevationLevel, float> elevation_opacities;
    std::unordered_map<ElevationLevel, juce::Point<float>> elevation_offsets;
    
    // Animation timings
    std::unordered_map<juce::String, float> animation_durations;
    
    // Custom properties
    std::unordered_map<juce::String, juce::var> custom_properties;
    
    MaterialTheme() {
        initializeDefaults();
    }
    
private:
    void initializeDefaults();
};

/**
 * @brief ThemeManager - Material Design 3.0 theme management system
 * 
 * Provides comprehensive theme management including:
 * - Material Design 3.0 color system with semantic colors
 * - Dynamic theme switching with smooth transitions
 * - High contrast mode support
 * - Custom theme creation and editing
 * - WCAG 2.2 AA contrast compliance checking
 * - Accessibility-aware color selection
 */
class VITAL_MODERN_UI_API ThemeManager {
public:
    /**
     * @brief Constructor
     */
    ThemeManager();

    /**
     * @brief Destructor
     */
    ~ThemeManager();

    //==============================================================================
    // Theme Management
    /**
     * @brief Create and register theme
     * @param theme Theme to register
     */
    void registerTheme(const MaterialTheme& theme);

    /**
     * @brief Remove theme
     * @param theme_name Name of theme to remove
     */
    void removeTheme(const juce::String& theme_name);

    /**
     * @brief Get theme by name
     * @param theme_name Name of theme
     * @return Theme or nullptr if not found
     */
    MaterialTheme* getTheme(const juce::String& theme_name) const;

    /**
     * @brief Get all registered themes
     */
    std::vector<juce::String> getAllThemeNames() const;

    /**
     * @brief Set current theme
     * @param theme_name Name of theme to apply
     * @param animate Whether to animate the transition
     */
    void setCurrentTheme(const juce::String& theme_name, bool animate = true);

    /**
     * @brief Get current theme
     */
    MaterialTheme* getCurrentTheme() const;

    /**
     * @brief Get current theme name
     */
    juce::String getCurrentThemeName() const { return current_theme_name_; }

    /**
     * @brief Set theme type
     * @param type Theme type
     */
    void setThemeType(ThemeType type);

    /**
     * @brief Get current theme type
     */
    ThemeType getThemeType() const { return current_theme_type_; }

    //==============================================================================
    // Built-in Themes
    /**
     * @brief Create light theme
     * @param seed_color Optional seed color for theme generation
     * @return Generated light theme
     */
    MaterialTheme createLightTheme(const juce::Colour& seed_color = juce::Colours::blue);

    /**
     * @brief Create dark theme
     * @param seed_color Optional seed color for theme generation
     * @return Generated dark theme
     */
    MaterialTheme createDarkTheme(const juce::Colour& seed_color = juce::Colours::blue);

    /**
     * @brief Create high contrast theme
     * @param type High contrast type
     * @return Generated high contrast theme
     */
    MaterialTheme createHighContrastTheme(ThemeType type = ThemeType::HighContrast);

    /**
     * @brief Initialize default themes
     */
    void initializeDefaultThemes();

    //==============================================================================
    // Color System
    /**
     * @brief Get color for role
     * @param role Color role
     * @return Color for the role
     */
    juce::Colour getColor(ColorRole role) const;

    /**
     * @brief Set color for role
     * @param role Color role
     * @param color Color to set
     */
    void setColor(ColorRole role, const juce::Colour& color);

    /**
     * @brief Generate theme from seed color
     * @param seed_color Seed color for theme generation
     * @param type Theme type to generate
     * @return Generated theme
     */
    MaterialTheme generateThemeFromSeed(const juce::Colour& seed_color, ThemeType type);

    /**
     * @brief Convert HSL to RGB
     * @param hue Hue (0-360)
     * @param saturation Saturation (0-1)
     * @param lightness Lightness (0-1)
     * @return RGB color
     */
    static juce::Colour hslToRgb(float hue, float saturation, float lightness);

    /**
     * @brief Get contrasting color
     * @param color Background color
     * @param preferred_color Preferred foreground color (optional)
     * @return Contrasting color
     */
    juce::Colour getContrastingColor(const juce::Colour& color, 
                                   const juce::Colour& preferred_color = juce::Colours::black) const;

    /**
     * @brief Calculate contrast ratio
     * @param color1 First color
     * @param color2 Second color
     * @return Contrast ratio (1.0 to 21.0)
     */
    static float calculateContrastRatio(const juce::Colour& color1, const juce::Colour& color2);

    /**
     * @brief Check if colors meet WCAG AA contrast requirements
     * @param foreground Foreground color
     * @param background Background color
     * @param is_large_text Whether text is large (AA: 3.0 for large, 4.5 for normal)
     * @return True if contrast meets AA requirements
     */
    bool meetsWCAGAAContrast(const juce::Colour& foreground, 
                           const juce::Colour& background, 
                           bool is_large_text = false) const;

    /**
     * @brief Check if colors meet WCAG AAA contrast requirements
     * @param foreground Foreground color
     * @param background Background color
     * @param is_large_text Whether text is large (AAA: 4.5 for large, 7.0 for normal)
     * @return True if contrast meets AAA requirements
     */
    bool meetsWCAGAAAContrast(const juce::Colour& foreground, 
                            const juce::Colour& background, 
                            bool is_large_text = false) const;

    //==============================================================================
    // Typography System
    /**
     * @brief Get font for text style
     * @param style Text style
     * @return Font for the style
     */
    juce::Font getFont(TextStyle style) const;

    /**
     * @brief Set font for text style
     * @param style Text style
     * @param font Font to set
     */
    void setFont(TextStyle style, const juce::Font& font);

    /**
     * @brief Get font size for text style
     * @param style Text style
     * @return Font size
     */
    float getFontSize(TextStyle style) const;

    /**
     * @brief Set font size for text style
     * @param style Text style
     * @param size Font size
     */
    void setFontSize(TextStyle style, float size);

    /**
     * @brief Get line height for text style
     * @param style Text style
     * @return Line height
     */
    float getLineHeight(TextStyle style) const;

    /**
     * @brief Set line height for text style
     * @param style Text style
     * @param height Line height
     */
    void setLineHeight(TextStyle style, float height);

    /**
     * @brief Get letter spacing for text style
     * @param style Text style
     * @return Letter spacing
     */
    float getLetterSpacing(TextStyle style) const;

    /**
     * @brief Set letter spacing for text style
     * @param style Text style
     * @param spacing Letter spacing
     */
    void setLetterSpacing(TextStyle style, float spacing);

    //==============================================================================
    // Component States
    /**
     * @brief Get state colors for component
     * @param component_name Component name
     * @return State colors for the component
     */
    StateColors getComponentStateColors(const juce::String& component_name) const;

    /**
     * @brief Set state colors for component
     * @param component_name Component name
     * @param colors State colors
     */
    void setComponentStateColors(const juce::String& component_name, const StateColors& colors);

    /**
     * @brief Get color for component state
     * @param component_name Component name
     * @param state Component state
     * @return Color for the state
     */
    juce::Colour getComponentStateColor(const juce::String& component_name, 
                                      core::Component::State state) const;

    //==============================================================================
    // Spacing and Layout
    /**
     * @brief Get spacing value
     * @param spacing_name Spacing name
     * @return Spacing value
     */
    float getSpacing(const juce::String& spacing_name) const;

    /**
     * @brief Set spacing value
     * @param spacing_name Spacing name
     * @param value Spacing value
     */
    void setSpacing(const juce::String& spacing_name, float value);

    /**
     * @brief Get corner radius
     * @param corner Corner type
     * @return Corner radius
     */
    float getCornerRadius(ShapeCorner corner) const;

    /**
     * @brief Set corner radius
     * @param corner Corner type
     * @param radius Corner radius
     */
    void setCornerRadius(ShapeCorner corner, float radius);

    /**
     * @brief Get component shape configuration
     * @param component_name Component name
     * @return Shape configuration
     */
    ShapeConfig getComponentShape(const juce::String& component_name) const;

    /**
     * @brief Set component shape configuration
     * @param component_name Component name
     * @param shape Shape configuration
     */
    void setComponentShape(const juce::String& component_name, const ShapeConfig& shape);

    //==============================================================================
    // Elevation System
    /**
     * @brief Get elevation blur
     * @param level Elevation level
     * @return Blur amount
     */
    float getElevationBlur(ElevationLevel level) const;

    /**
     * @brief Get elevation opacity
     * @param level Elevation level
     * @return Opacity value
     */
    float getElevationOpacity(ElevationLevel level) const;

    /**
     * @brief Get elevation offset
     * @param level Elevation level
     * @return Offset vector
     */
    juce::Point<float> getElevationOffset(ElevationLevel level) const;

    /**
     * @brief Create shadow color for elevation
     * @param level Elevation level
     * @return Shadow color
     */
    juce::Colour createShadowColor(ElevationLevel level) const;

    //==============================================================================
    // Animation
    /**
     * @brief Get animation duration
     * @param animation_name Animation name
     * @return Duration in milliseconds
     */
    float getAnimationDuration(const juce::String& animation_name) const;

    /**
     * @brief Set animation duration
     * @param animation_name Animation name
     * @param duration Duration in milliseconds
     */
    void setAnimationDuration(const juce::String& animation_name, float duration);

    //==============================================================================
    // Theme Animation and Transitions
    /**
     * @ Enable theme transition animation
     * @param enabled Whether animation is enabled
     */
    void setThemeTransitionEnabled(bool enabled);

    /**
     * @brief Check if theme transition is enabled
     */
    bool isThemeTransitionEnabled() const { return theme_transition_enabled_; }

    /**
     * @brief Set theme transition duration
     * @param duration Duration in milliseconds
     */
    void setThemeTransitionDuration(float duration);

    /**
     * @brief Get theme transition duration
     */
    float getThemeTransitionDuration() const { return theme_transition_duration_ms_; }

    //==============================================================================
    // Accessibility Integration
    /**
     * @brief Check if theme is accessibility-friendly
     * @param theme_name Theme name to check
     * @return True if theme meets accessibility requirements
     */
    bool isThemeAccessibilityFriendly(const juce::String& theme_name) const;

    /**
     * @brief Get recommended text color for background
     * @param background_color Background color
     * @param accessibility_level Accessibility level ("AA" or "AAA")
     * @param is_large_text Whether text is large
     * @return Recommended text color
     */
    juce::Colour getRecommendedTextColor(const juce::Colour& background_color,
                                       const juce::String& accessibility_level = "AA",
                                       bool is_large_text = false) const;

    /**
     * @brief Auto-adjust colors for accessibility
     * @param theme Theme to adjust
     * @param accessibility_level Accessibility level
     * @return Adjusted theme
     */
    MaterialTheme autoAdjustForAccessibility(const MaterialTheme& theme,
                                           const juce::String& accessibility_level = "AA") const;

    //==============================================================================
    // Custom Properties
    /**
     * @brief Get custom property value
     * @param property_name Property name
     * @return Property value or default if not found
     */
    juce::var getCustomProperty(const juce::String& property_name) const;

    /**
     * @brief Set custom property value
     * @param property_name Property name
     * @param value Property value
     */
    void setCustomProperty(const juce::String& property_name, const juce::var& value);

    /**
     * @brief Remove custom property
     * @param property_name Property name to remove
     */
    void removeCustomProperty(const juce::String& property_name);

    /**
     * @brief Get all custom property names
     */
    std::vector<juce::String> getAllCustomPropertyNames() const;

    //==============================================================================
    // Theme Callbacks
    /**
     * @brief Register theme change callback
     * @param callback Theme change callback
     * @return Callback ID
     */
    int registerThemeChangeCallback(std::function<void(const juce::String&, const juce::String&)> callback);

    /**
     * @brief Unregister theme change callback
     * @param callback_id Callback ID
     */
    void unregisterThemeChangeCallback(int callback_id);

    /**
     * @brief Notify theme change
     * @param old_theme_name Previous theme name
     * @param new_theme_name New theme name
     */
    void notifyThemeChange(const juce::String& old_theme_name, const juce::String& new_theme_name);

    //==============================================================================
    // Serialization
    /**
     * @brief Save theme to file
     * @param theme_name Theme name
     * @param file_path File path
     * @return True if saved successfully
     */
    bool saveThemeToFile(const juce::String& theme_name, const juce::String& file_path) const;

    /**
     * @brief Load theme from file
     * @param file_path File path
     * @return Loaded theme or default if failed
     */
    MaterialTheme loadThemeFromFile(const juce::String& file_path) const;

    /**
     * @brief Export theme as JSON
     * @param theme_name Theme name
     * @return JSON string
     */
    juce::String exportThemeAsJSON(const juce::String& theme_name) const;

    /**
     * @brief Import theme from JSON
     * @param json_string JSON string
     * @return Imported theme
     */
    MaterialTheme importThemeFromJSON(const juce::String& json_string) const;

private:
    //==============================================================================
    // Private member variables
    std::unordered_map<juce::String, MaterialTheme> themes_;
    juce::String current_theme_name_ = "Light";
    MaterialTheme* current_theme_ = nullptr;
    ThemeType current_theme_type_ = ThemeType::Light;

    // Theme transition
    bool theme_transition_enabled_ = true;
    float theme_transition_duration_ms_ = 300.0f;
    bool is_transitioning_ = false;

    // Callbacks
    std::vector<std::function<void(const juce::String&, const juce::String&)>> theme_change_callbacks_;
    mutable std::mutex callbacks_mutex_;

    //==============================================================================
    // Private methods
    void applyThemeTransition(const MaterialTheme& old_theme, const MaterialTheme& new_theme);
    void updateComponentThemeReferences();
    void validateTheme(const MaterialTheme& theme) const;
    
    // Color generation helpers
    juce::Colour generatePrimaryColor(const juce::Colour& seed_color, ThemeType type) const;
    juce::Colour generateSecondaryColor(const juce::Colour& primary_color) const;
    juce::Colour generateTertiaryColor(const juce::Colour& primary_color) const;
    juce::Colour generateSurfaceColor(ThemeType type) const;
    juce::Colour generateErrorColor() const;

    // Accessibility helpers
    float calculateLuminance(const juce::Colour& color) const;
    bool ensureMinimumContrast(juce::Colour& foreground, const juce::Colour& background, 
                              float min_contrast) const;

    // Theme validation
    bool validateColorContrast(const MaterialTheme& theme) const;
    bool validateAccessibility(const MaterialTheme& theme) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ThemeManager)
};

} // namespace theme
} // namespace ui
} // namespace vital
