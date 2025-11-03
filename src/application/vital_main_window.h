// Copyright (c) 2025 Vital Application Developers
// 
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from
// the use of this software.
// 
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.

#pragma once

#include <JuceHeader.h>
#include <memory>
#include <atomic>
#include <vector>
#include <string>
#include <functional>
#include <optional>

// Forward declarations
namespace vital {
    class VitalApplication;
    class VitalPluginProcessor;
    class VitalPluginEditor;
}

// Window configuration
namespace vital::window_config {
    constexpr int MIN_WIDTH = 1000;
    constexpr int MIN_HEIGHT = 600;
    constexpr int DEFAULT_WIDTH = 1400;
    constexpr int DEFAULT_HEIGHT = 900;
    constexpr int TITLE_BAR_HEIGHT = 30;
    constexpr int STATUS_BAR_HEIGHT = 25;
    constexpr int MENU_BAR_HEIGHT = 25;
    constexpr int TOOLBAR_HEIGHT = 40;
}

// Window state
namespace vital::window_state {
    enum class WindowState {
        Normal,
        Minimized,
        Maximized,
        Fullscreen,
        AlwaysOnTop
    };
}

// Main window class for standalone application
namespace vital {

class VitalMainWindow : public juce::DocumentWindow,
                       public juce::Timer,
                       public juce::ApplicationCommandTarget,
                       public juce::KeyListener,
                       public juce::MenuBarModel,
                       public juce::DragAndDropTarget {
public:
    // Constructor/Destructor
    VitalMainWindow(const juce::String& name, juce::LookAndFeel& lookAndFeel);
    ~VitalMainWindow() override;

    //==============================================================================
    // DocumentWindow overrides
    void closeButtonPressed() override;
    void minimiseButtonPressed() override;
    void maximiseButtonPressed() override;
    void activeWindowStatusChanged() override;
    
    //==============================================================================
    // Timer overrides
    void timerCallback() override;
    
    //==============================================================================
    // ApplicationCommandTarget overrides
    ApplicationCommandTarget* getNextCommandTarget() override;
    void getAllCommands(juce::Array<juce::CommandID>& commands) override;
    void getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo& result) override;
    bool perform(const juce::ApplicationCommandTarget::InvocationInfo& info) override;
    
    //==============================================================================
    // KeyListener overrides
    bool keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent) override;
    bool keyStateChanged(bool isKeyDown, juce::Component* originatingComponent) override;
    
    //==============================================================================
    // MenuBarModel overrides
    juce::StringArray getMenuBarNames() override;
    juce::PopupMenu getMenuForIndex(int menuIndex, const juce::String& menuName) override;
    void menuItemSelected(int menuItemID, int topLevelMenuIndex) override;
    
    //==============================================================================
    // DragAndDropTarget overrides
    bool isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    void itemDropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    void itemDragEnter(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    void itemDragExit(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) override;
    
    //==============================================================================
    // Custom methods
    bool create();
    void destroy();
    
    // Window management
    void setWindowState(vital::window_state::WindowState state);
    vital::window_state::WindowState getWindowState() const;
    void toggleFullscreen();
    void toggleAlwaysOnTop();
    
    // Layout management
    void updateLayout();
    void setTitleBarVisible(bool visible);
    void setMenuBarVisible(bool visible);
    void setStatusBarVisible(bool visible);
    void setToolbarVisible(bool visible);
    
    // Content management
    void setContent(juce::Component* contentComponent);
    juce::Component* getContent() const;
    
    // Menu management
    void rebuildMenus();
    void updateRecentFilesMenu();
    void addRecentFile(const juce::File& file);
    
    // File management
    void openFile();
    void saveFile();
    void saveFileAs();
    void exportAudio();
    
    // Preset management
    void loadPreset();
    void savePreset();
    void loadPresetFromFile(const juce::File& file);
    void savePresetToFile(const juce::File& file);
    
    // Plugin management
    void loadPlugin();
    void reloadPlugin();
    void showPluginInfo();
    
    // View management
    void showAbout();
    void showPreferences();
    void showPerformanceMonitor();
    void showLogWindow();
    void showHelp();
    
    // Theme management
    void applyTheme(const juce::String& themeName);
    void cycleTheme();
    
    // Window state persistence
    void saveWindowState();
    void restoreWindowState();
    
    // Fullscreen support
    void enterFullscreen();
    void exitFullscreen();
    bool isFullscreen() const;
    
    // High DPI support
    void setHighDPIEnabled(bool enabled);
    void setUIScale(float scale);
    float getUIScale() const;
    
    // Accessibility
    void setAccessibilityEnabled(bool enabled);
    void setAccessibilityText(const juce::String& text);
    
    // Performance monitoring
    void updatePerformanceStats(float cpuLoad, float memoryUsage, float latency);
    void showPerformanceOverlay(bool show);
    
    // MIDI management
    void enableMIDIInput(bool enabled);
    void selectMIDIInput(int deviceIndex);
    void showMIDILearningDialog();
    
    // Audio management
    void showAudioSettings();
    void enableAudioOutput(bool enabled);
    void setAudioDevice(const juce::String& deviceName);
    
    // Window customization
    void setCustomTitleBar(juce::Component* titleBar);
    void setCustomStatusBar(juce::Component* statusBar);
    void setCustomToolbar(juce::Component* toolbar);
    
    // Event handling
    void onWindowResized();
    void onWindowMoved();
    void onWindowStateChanged();
    
    // Notification management
    void showNotification(const juce::String& title, const juce::String& message, int timeoutMs = 3000);
    void showProgressNotification(const juce::String& title, const juce::String& message, float progress);
    
    // Error handling
    void showErrorDialog(const juce::String& title, const juce::String& message);
    void showWarningDialog(const juce::String& title, const juce::String& message);
    void showInfoDialog(const juce::String& title, const juce::String& message);

private:
    // Window components
    std::unique_ptr<juce::MenuBarComponent> menu_bar_;
    std::unique_ptr<juce::Component> main_content_;
    std::unique_ptr<juce::Component> status_bar_;
    std::unique_ptr<juce::Component> toolbar_;
    std::unique_ptr<juce::Component> notification_overlay_;
    
    // Custom components
    std::unique_ptr<juce::Component> custom_title_bar_;
    std::unique_ptr<juce::Component> custom_status_bar_;
    std::unique_ptr<juce::Component> custom_toolbar_;
    
    // Window state
    vital::window_state::WindowState window_state_ = vital::window_state::WindowState::Normal;
    bool title_bar_visible_ = true;
    bool menu_bar_visible_ = true;
    bool status_bar_visible_ = true;
    bool toolbar_visible_ = true;
    bool fullscreen_ = false;
    bool high_dpi_enabled_ = true;
    float ui_scale_ = 1.0f;
    bool accessibility_enabled_ = false;
    
    // Layout state
    juce::Rectangle<int> last_normal_bounds_;
    juce::Rectangle<int> last_maximized_bounds_;
    juce::Rectangle<int> last_fullscreen_bounds_;
    
    // Performance monitoring
    std::atomic<float> current_cpu_load_{0.0f};
    std::atomic<size_t> current_memory_usage_{0};
    std::atomic<float> current_latency_{0.0f};
    std::unique_ptr<juce::Component> performance_overlay_;
    std::unique_ptr<juce::Label> performance_label_;
    
    // File management
    juce::StringArray recent_files_;
    juce::File current_file_;
    bool file_modified_ = false;
    juce::String file_modified_time_;
    
    // MIDI management
    bool midi_input_enabled_ = true;
    int selected_midi_device_ = -1;
    std::vector<juce::String> midi_device_names_;
    
    // Audio management
    bool audio_output_enabled_ = true;
    juce::String selected_audio_device_;
    
    // Theme management
    juce::String current_theme_ = "default";
    std::vector<juce::String> available_themes_;
    
    // Recent files
    static constexpr int MAX_RECENT_FILES = 10;
    
    // Window timers
    static constexpr int UI_UPDATE_INTERVAL = 100; // ms
    static constexpr int AUTO_SAVE_INTERVAL = 60000; // ms
    
    // Command IDs
    enum CommandIDs {
        FileNew = 0x1000,
        FileOpen,
        FileSave,
        FileSaveAs,
        FileExportAudio,
        FileExit,
        
        EditUndo,
        EditRedo,
        EditCut,
        EditCopy,
        EditPaste,
        EditSelectAll,
        EditPreferences,
        
        ViewZoomIn,
        ViewZoomOut,
        ViewZoomToFit,
        ViewFullscreen,
        ViewTheme,
        ViewPerformanceMonitor,
        ViewLogWindow,
        
        TransportPlay,
        TransportStop,
        TransportRecord,
        TransportRewind,
        TransportForward,
        
        PresetLoad,
        PresetSave,
        PresetRandomize,
        PresetReset,
        
        PluginLoad,
        PluginReload,
        PluginInfo,
        
        HelpAbout,
        HelpHelp,
        HelpVisitWebsite,
        
        WindowCascade,
        WindowTile,
        WindowMinimizeAll,
        WindowCloseAll
    };
    
    // Initialization methods
    void initializeComponents();
    void initializeMenus();
    void initializeThemes();
    void initializeMIDI();
    void initializeAudio();
    void initializeFileManagement();
    
    // Layout methods
    void layoutMenuBar();
    void layoutToolbar();
    void layoutMainContent();
    void layoutStatusBar();
    void layoutNotificationOverlay();
    
    // Event handlers
    void onFileNew();
    void onFileOpen();
    void onFileSave();
    void onFileSaveAs();
    void onFileExportAudio();
    void onFileExit();
    
    void onEditUndo();
    void onEditRedo();
    void onEditCut();
    void onEditCopy();
    void onEditPaste();
    void onEditSelectAll();
    void onEditPreferences();
    
    void onViewZoomIn();
    void onViewZoomOut();
    void onViewZoomToFit();
    void onViewFullscreen();
    void onViewTheme();
    void onViewPerformanceMonitor();
    void onViewLogWindow();
    
    void onTransportPlay();
    void onTransportStop();
    void onTransportRecord();
    void onTransportRewind();
    void onTransportForward();
    
    void onPresetLoad();
    void onPresetSave();
    void onPresetRandomize();
    void onPresetReset();
    
    void onPluginLoad();
    void onPluginReload();
    void onPluginInfo();
    
    void onHelpAbout();
    void onHelpHelp();
    void onHelpVisitWebsite();
    
    void onWindowCascade();
    void onWindowTile();
    void onWindowMinimizeAll();
    void onWindowCloseAll();
    
    // Utility methods
    void updateTitle();
    void updateStatusBar();
    void updateToolbar();
    void updateMenus();
    void markFileModified(bool modified);
    void checkAutoSave();
    void performAutoSave();
    
    // File dialog helpers
    juce::File showOpenFileDialog(const juce::String& filters);
    juce::File showSaveFileDialog(const juce::String& filters, const juce::String& defaultName);
    
    // Theme helpers
    void loadTheme(const juce::String& themeName);
    void applyThemeToWindow();
    
    // MIDI helpers
    void refreshMIDIDevices();
    void setupMIDIInput(int deviceIndex);
    
    // Audio helpers
    void refreshAudioDevices();
    void setupAudioOutput(const juce::String& deviceName);
    
    // Notification helpers
    void createNotificationOverlay();
    void showNotificationInternal(const juce::String& title, const juce::String& message, 
                                const juce::Colour& color, int timeoutMs);
    
    // Performance helpers
    void createPerformanceOverlay();
    void updatePerformanceDisplay();
    
    // Constants
    static constexpr int NOTIFICATION_HEIGHT = 40;
    static constexpr int NOTIFICATION_MARGIN = 10;
    static constexpr int AUTO_SAVE_INTERVAL_MS = 300000; // 5 minutes
    
    // JUCE leak detection
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VitalMainWindow)
};

} // namespace vital
