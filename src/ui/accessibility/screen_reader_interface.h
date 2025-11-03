#pragma once

#include <memory>
#include <string>
#include <functional>
#include <map>

namespace vital {
namespace ui {
namespace accessibility {

/**
 * @brief Screen reader interface abstraction
 * 
 * Provides unified interface for screen reader functionality with:
 * - Text announcement
 * - Element description
 * - State updates
 * - Live region support
 */
class ScreenReaderInterface {
public:
    struct ElementInfo {
        std::string name;
        std::string role;
        std::string description;
        std::string value;
        std::string state;
        bool enabled = true;
        bool focused = false;
        bool expanded = false;
    };

    using AnnouncementCallback = std::function<void(const std::string& text)>;

    ScreenReaderInterface();
    ~ScreenReaderInterface() = default;

    // Announcements
    void announce(const std::string& text);
    void announcePolite(const std::string& text);
    void announceAssertive(const std::string& text);
    
    // Element management
    void registerElement(void* component, const ElementInfo& info);
    void unregisterElement(void* component);
    void updateElement(void* component, const ElementInfo& info);
    void updateElementValue(void* component, const std::string& value);
    void updateElementState(void* component, const std::string& state);
    
    // Live regions
    void createLiveRegion(void* component, bool polite = true);
    void updateLiveRegion(void* component, const std::string& text);
    void removeLiveRegion(void* component);
    
    // Configuration
    void setEnabled(bool enabled);
    bool isEnabled() const { return enabled_; }
    void setAnnouncementDelay(int delayMs);
    
    // Callbacks
    void setAnnouncementCallback(AnnouncementCallback callback);
    
    // Platform detection
    bool isScreenReaderActive() const;
    std::string getScreenReaderName() const;

private:
    std::map<void*, ElementInfo> elements_;
    std::map<void*, bool> liveRegions_;
    bool enabled_ = true;
    int announcementDelay_ = 100;
    
    AnnouncementCallback announcementCallback_;
    
    // Platform-specific implementations
    void platformAnnounce(const std::string& text, bool assertive);
    void platformRegisterElement(void* component, const ElementInfo& info);
};

} // namespace accessibility
} // namespace ui
} // namespace vital