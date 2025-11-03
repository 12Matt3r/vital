#pragma once

#include "../core/component.h"
#include <memory>
#include <vector>
#include <functional>

namespace vital {
namespace ui {
namespace visualizations {

/**
 * @brief Modulation visualization system
 * 
 * Displays modulation connections and routing with:
 * - Source/destination mapping
 * - Connection visualization
 * - Modulation depth display
 * - Real-time parameter modulation
 * - Modulation matrix view
 */
class ModulationVisualizer : public Component {
public:
    enum class ViewMode {
        Matrix,         // Matrix view of sources vs destinations
        Network,        // Network/graph view
        List,           // Simple list view
        Custom          // Custom view
    };

    struct ModulationSource {
        std::string id;
        std::string name;
        juce::Drawable* icon = nullptr;
        Color color = Colors::primary;
        bool enabled = true;
        float currentValue = 0.0f;
        float depth = 1.0f;
        std::vector<std::string> destinations;
    };

    struct ModulationDestination {
        std::string id;
        std::string name;
        juce::Drawable* icon = nullptr;
        Color color = Colors::secondary;
        bool enabled = true;
        float currentValue = 0.0f;
        float baseValue = 0.0f;
        float minValue = 0.0f;
        float maxValue = 1.0f;
        std::vector<std::string> sources;
    };

    struct ModulationConnection {
        std::string sourceId;
        std::string destinationId;
        float depth = 1.0f;            // Modulation depth 0-1
        float currentAmount = 0.0f;    // Current modulation amount
        bool enabled = true;
        bool bipolar = false;          // Bipolar modulation
        Color color = Colors::accent;
        bool active = false;           // Currently modulating
    };

    struct ModulationVisualizerSettings {
        ViewMode viewMode = ViewMode::Matrix;
        int maxSources = 8;
        int maxDestinations = 8;
        float connectionOpacity = 0.6f;
        float activeConnectionOpacity = 1.0f;
        float connectionThickness = 2.0f;
        Color backgroundColor = Colors::surface;
        Color gridColor = Colors::outlineVariant;
        Color sourceColor = Colors::primary;
        Color destinationColor = Colors::secondary;
        Color activeColor = Colors::error;
        bool showValues = true;
        bool showDepth = true;
        bool animated = true;
        float animationSpeed = 1.0f;
        bool enableDrag = true;
        bool enableConnections = true;
    };

    using ConnectionCallback = std::function<void(const std::string& sourceId, const std::string& destinationId)>;
    using DisconnectionCallback = std::function<void(const std::string& sourceId, const std::string& destinationId)>;
    using ValueChangedCallback = std::function<void(const std::string& paramId, float value)>;

    ModulationVisualizer(const ModulationVisualizerSettings& settings = {});
    ~ModulationVisualizer() override = default;

    // Component interface
    void paint(juce::Graphics& g) override;
    void resized() override;
    bool mouseDown(const juce::MouseEvent& e) override;
    bool mouseDrag(const juce::MouseEvent& e) override;
    bool mouseUp(const juce::MouseEvent& e) override;
    
    // Settings
    void setSettings(const ModulationVisualizerSettings& settings);
    ModulationVisualizerSettings getSettings() const { return settings_; }
    
    // Source management
    void addSource(const ModulationSource& source);
    void removeSource(const std::string& sourceId);
    void updateSource(const std::string& sourceId, const ModulationSource& source);
    void setSourceValue(const std::string& sourceId, float value);
    void setSourceDepth(const std::string& sourceId, float depth);
    void setSourceEnabled(const std::string& sourceId, bool enabled);
    
    // Destination management
    void addDestination(const ModulationDestination& destination);
    void removeDestination(const std::string& destinationId);
    void updateDestination(const std::string& destinationId, const ModulationDestination& destination);
    void setDestinationValue(const std::string& destinationId, float value);
    void setDestinationBaseValue(const std::string& destinationId, float value);
    void setDestinationRange(const std::string& destinationId, float minValue, float maxValue);
    void setDestinationEnabled(const std::string& destinationId, bool enabled);
    
    // Connection management
    void createConnection(const std::string& sourceId, const std::string& destinationId, float depth = 1.0f);
    void removeConnection(const std::string& sourceId, const std::string& destinationId);
    void setConnectionDepth(const std::string& sourceId, const std::string& destinationId, float depth);
    void setConnectionEnabled(const std::string& sourceId, const std::string& destinationId, bool enabled);
    void setConnectionActive(const std::string& sourceId, const std::string& destinationId, bool active);
    
    // View mode
    void setViewMode(ViewMode mode);
    ViewMode getViewMode() const { return settings_.viewMode; }
    
    // Data queries
    std::vector<ModulationSource> getSources() const;
    std::vector<ModulationDestination> getDestinations() const;
    std::vector<ModulationConnection> getConnections() const;
    ModulationSource* getSource(const std::string& sourceId);
    ModulationDestination* getDestination(const std::string& destinationId);
    ModulationConnection* getConnection(const std::string& sourceId, const std::string& destinationId);
    
    // Callbacks
    void setConnectionCallback(ConnectionCallback callback);
    void setDisconnectionCallback(DisconnectionCallback callback);
    void setValueChangedCallback(ValueChangedCallback callback);
    
    // Animation
    void animateConnections(bool animate);
    void updateAnimation();
    
    // Export
    void exportMatrix(const std::string& filename);
    void exportImage(const std::string& filename, int width = 1920, int height = 1080);
    
    // Accessibility
    juce::String getAccessibilityLabel() const override;

private:
    ModulationVisualizerSettings settings_;
    std::vector<ModulationSource> sources_;
    std::vector<ModulationDestination> destinations_;
    std::vector<ModulationConnection> connections_;
    
    // Animation
    AnimationValue connectionOpacity_;
    std::map<std::string, AnimationValue> connectionAmounts_;
    
    // Drag state
    bool isDragging_ = false;
    juce::Point<float> dragStart_;
    std::string draggingSource_;
    std::string draggingDestination_;
    
    // Callbacks
    ConnectionCallback connectionCallback_;
    DisconnectionCallback disconnectionCallback_;
    ValueChangedCallback valueCallback_;
    
    // Internal helpers
    juce::Rectangle<float> getMatrixArea() const;
    juce::Rectangle<float> getSourceBounds(int index) const;
    juce::Rectangle<float> getDestinationBounds(int index) const;
    juce::Path getConnectionPath(const juce::Rectangle<float>& sourceBounds, const juce::Rectangle<float>& destBounds) const;
    juce::Point<float> getConnectionPoint(const juce::Rectangle<float>& bounds, bool isSource) const;
    void drawMatrixView(juce::Graphics& g);
    void drawNetworkView(juce::Graphics& g);
    void drawListView(juce::Graphics& g);
    void drawSources(juce::Graphics& g);
    void drawDestinations(juce::Graphics& g);
    void drawConnections(juce::Graphics& g);
    void drawConnection(juce::Graphics& g, const ModulationConnection& connection);
    void drawValues(juce::Graphics& g);
    const ModulationSource* getSourceById(const std::string& sourceId) const;
    const ModulationDestination* getDestinationById(const std::string& destId) const;
    const ModulationConnection* getConnectionByIds(const std::string& sourceId, const std::string& destId) const;
    int getSourceIndex(const std::string& sourceId) const;
    int getDestinationIndex(const std::string& destId) const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_CHECK(ModulationVisualizer)
};

} // namespace visualizations
} // namespace ui
} // namespace vital