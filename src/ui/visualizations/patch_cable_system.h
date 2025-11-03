#pragma once

#include "../core/component.h"
#include <memory>
#include <vector>
#include <functional>
#include <map>

namespace vital {
namespace ui {
namespace visualizations {

/**
 * @brief Visual routing system for patch cables
 * 
 * Provides visual patch cable connections with:
 * - Drag-and-drop cable creation
 * - Multiple connection types
 * - Cable routing optimization
 * - Connection validation
 * - Real-time parameter routing
 */
class PatchCableSystem : public Component {
public:
    struct ConnectionPoint {
        std::string id;
        juce::Rectangle<float> bounds;
        Color color;
        bool input = false;
        bool output = false;
        std::string dataType;
    };

    struct Cable {
        std::string id;
        std::string fromId;
        std::string toId;
        Color color;
        bool active = false;
        float value = 0.0f;
    };

    using ConnectionCallback = std::function<void(const std::string& fromId, const std::string& toId)>;
    using DisconnectionCallback = std::function<void(const std::string& cableId)>;

    PatchCableSystem();
    ~PatchCableSystem() override = default;

    // Component interface
    void paint(juce::Graphics& g) override;
    bool mouseDown(const juce::MouseEvent& e) override;
    bool mouseDrag(const juce::MouseEvent& e) override;
    bool mouseUp(const juce::MouseEvent& e) override;

    // Connection management
    void addConnectionPoint(const ConnectionPoint& point);
    void createConnection(const std::string& fromId, const std::string& toId);
    void removeConnection(const std::string& cableId);

    // Callbacks
    void setConnectionCallback(ConnectionCallback callback);
    void setDisconnectionCallback(DisconnectionCallback callback);

private:
    std::vector<ConnectionPoint> connectionPoints_;
    std::vector<Cable> cables_;
    juce::Point<float> dragStart_;
    bool isDragging_ = false;

    ConnectionCallback connectionCallback_;
    DisconnectionCallback disconnectionCallback_;
};

} // namespace visualizations
} // namespace ui
} // namespace vital