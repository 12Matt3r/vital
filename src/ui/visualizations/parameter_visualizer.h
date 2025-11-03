#pragma once

#include "../core/component.h"
#include <memory>
#include <vector>
#include <functional>

namespace vital {
namespace ui {
namespace visualizations {

/**
 * @brief Parameter visualization and control system
 * 
 * Visualizes and controls synthesizer parameters with:
 * - Parameter grouping and organization
 * - Real-time value updates
 * - Parameter automation display
 * - Undo/redo history
 * - Preset management
 */
class ParameterVisualizer : public Component {
public:
    struct Parameter {
        std::string id;
        std::string name;
        std::string group;
        float value = 0.0f;
        float minValue = 0.0f;
        float maxValue = 1.0f;
        float defaultValue = 0.0f;
        bool enabled = true;
        bool modulated = false;
        bool automated = false;
        float automationValue = 0.0f;
        std::string unit;
        std::string format;
        juce::Drawable* icon = nullptr;
        Color color = Colors::primary;
    };

    struct ParameterGroup {
        std::string name;
        std::string id;
        Color color = Colors::surfaceVariant;
        bool collapsed = false;
        std::vector<Parameter*> parameters;
    };

    enum class ViewMode {
        List,           // Simple list view
        Grid,           // Grid layout
        Tree,           // Hierarchical tree
        Tabs,           // Tabbed groups
        Custom          // Custom layout
    };

    using ParameterCallback = std::function<void(const std::string& paramId, float value)>;
    using ParameterChangedCallback = std::function<void(const Parameter& parameter)>;
    using GroupCallback = std::function<void(const std::string& groupId)>;

    ParameterVisualizer();
    ~ParameterVisualizer() override = default;

    // Component interface
    void paint(juce::Graphics& g) override;
    void resized() override;
    bool mouseDown(const juce::MouseEvent& e) override;
    
    // Parameter management
    void addParameter(const Parameter& parameter);
    void removeParameter(const std::string& paramId);
    void updateParameter(const std::string& paramId, const Parameter& parameter);
    void setParameterValue(const std::string& paramId, float value, bool sendCallback = true);
    float getParameterValue(const std::string& paramId) const;
    
    // Group management
    void addGroup(const ParameterGroup& group);
    void removeGroup(const std::string& groupId);
    void setGroupCollapsed(const std::string& groupId, bool collapsed);
    
    // View mode
    void setViewMode(ViewMode mode);
    ViewMode getViewMode() const { return viewMode_; }
    
    // Callbacks
    void setParameterCallback(ParameterCallback callback);
    void setParameterChangedCallback(ParameterChangedCallback callback);
    void setGroupCallback(GroupCallback callback);
    
    // Accessibility
    juce::String getAccessibilityLabel() const override;

private:
    ViewMode viewMode_ = ViewMode::List;
    std::vector<Parameter> parameters_;
    std::vector<ParameterGroup> groups_;
    
    ParameterCallback paramCallback_;
    ParameterChangedCallback paramChangedCallback_;
    GroupCallback groupCallback_;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_CHECK(ParameterVisualizer)
};

} // namespace visualizations
} // namespace ui
} // namespace vital