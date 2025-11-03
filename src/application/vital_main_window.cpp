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

#include "vital_main_window.h"
#include "vital_application.h"
#include "vital_plugin_processor.h"
#include <iostream>
#include <fstream>
#include <sstream>

namespace vital {

//==============================================================================
VitalMainWindow::VitalMainWindow(const juce::String& name, juce::LookAndFeel& lookAndFeel)
    : juce::DocumentWindow(name, lookAndFeel, DocumentWindow::allButtons) {
    
    try {
        // Set initial properties
        setContentComponent(nullptr);
        setUsingNativeTitleBar(true);
        setHasCloseButton(true);
        setHasMinimizeButton(true);
        setHasMaximizeButton(true);
        setResizable(true, true);
        setVisible(false);
        
        // Initialize components
        initializeComponents();
        initializeMenus();
        initializeThemes();
        initializeMIDI();
        initializeAudio();
        initializeFileManagement();
        
        // Set initial size
        setSize(window_config::DEFAULT_WIDTH, window_config::DEFAULT_HEIGHT);
        
        // Start timers
        startTimer(UI_UPDATE_INTERVAL);
        startTimer(AUTO_SAVE_INTERVAL);
        
        DBG("Main window created");
        
    } catch (const std::exception& e) {
        DBG("Exception creating main window: " << e.what());
    }
}

VitalMainWindow::~VitalMainWindow() {
    try {
        // Stop timers
        stopTimer();
        
        // Save window state
        saveWindowState();
        
        // Clean up components
        custom_title_bar_.reset();
        custom_status_bar_.reset();
        custom_toolbar_.reset();
        
        DBG("Main window destroyed");
        
    } catch (const std::exception& e) {
        DBG("Exception destroying main window: " << e.what());
    }
}

//==============================================================================
void VitalMainWindow::closeButtonPressed() {
    // Save window state before closing
    saveWindowState();
    
    // Check if document needs saving
    if (file_modified_) {
        int result = juce::AlertWindow::showYesNoCancelBox(
            juce::AlertWindow::QuestionIcon,
            "Save Changes?",
            "Do you want to save the changes before closing?",
            "Save",
            "Don't Save",
            "Cancel"
        );
        
        switch (result) {
            case 1: // Save
                onFileSave();
                break;
            case 2: // Don't Save
                break;
            case 0: // Cancel
                return;
        }
    }
    
    // Close the window
    setVisible(false);
    
    // If this is the last window, quit the application
    auto& desktop = juce::Desktop::getInstance();
    if (desktop.getNumOpenWindows() <= 1) {
        juce::JUCEApplication::quit();
    }
}

void VitalMainWindow::minimiseButtonPressed() {
    window_state_ = vital::window_state::WindowState::Minimized;
    juce::DocumentWindow::minimiseButtonPressed();
}

void VitalMainWindow::maximiseButtonPressed() {
    if (isFullScreen()) {
        // Exit fullscreen mode
        exitFullscreen();
    } else if (window_state_ == vital::window_state::WindowState::Maximized) {
        // Return to normal state
        window_state_ = vital::window_state::WindowState::Normal;
        setBounds(last_normal_bounds_);
    } else {
        // Enter maximized state
        window_state_ = vital::window_state::WindowState::Maximized;
        last_normal_bounds_ = getBounds();
        setFullScreen(true);
    }
}

void VitalMainWindow::activeWindowStatusChanged() {
    // Handle window activation
    updateMenus();
    updateTitle();
}

//==============================================================================
void VitalMainWindow::timerCallback() {
    try {
        // Update UI elements
        updatePerformanceDisplay();
        checkAutoSave();
        
        // Update window state
        if (window_state_ == vital::window_state::WindowState::Maximized && !isFullScreen()) {
            window_state_ = vital::window_state::WindowState::Normal;
        }
        
    } catch (const std::exception& e) {
        DBG("Exception in main window timer: " << e.what());
    }
}

//==============================================================================
juce::ApplicationCommandTarget* VitalMainWindow::getNextCommandTarget() {
    return nullptr;
}

void VitalMainWindow::getAllCommands(juce::Array<juce::CommandID>& commands) {
    // File commands
    commands.addArray({
        CommandIDs::FileNew, CommandIDs::FileOpen, CommandIDs::FileSave,
        CommandIDs::FileSaveAs, CommandIDs::FileExportAudio, CommandIDs::FileExit,
        
        CommandIDs::EditUndo, CommandIDs::EditRedo, CommandIDs::EditCut,
        CommandIDs::EditCopy, CommandIDs::EditPaste, CommandIDs::EditSelectAll,
        CommandIDs::EditPreferences,
        
        CommandIDs::ViewZoomIn, CommandIDs::ViewZoomOut, CommandIDs::ViewZoomToFit,
        CommandIDs::ViewFullscreen, CommandIDs::ViewTheme, CommandIDs::ViewPerformanceMonitor,
        CommandIDs::ViewLogWindow,
        
        CommandIDs::TransportPlay, CommandIDs::TransportStop, CommandIDs::TransportRecord,
        CommandIDs::TransportRewind, CommandIDs::TransportForward,
        
        CommandIDs::PresetLoad, CommandIDs::PresetSave, CommandIDs::PresetRandomize,
        CommandIDs::PresetReset,
        
        CommandIDs::PluginLoad, CommandIDs::PluginReload, CommandIDs::PluginInfo,
        
        CommandIDs::HelpAbout, CommandIDs::HelpHelp, CommandIDs::HelpVisitWebsite
    });
}

void VitalMainWindow::getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo& result) {
    switch (commandID) {
        case CommandIDs::FileNew:
            result.setInfo("New", "Create a new document", "File", 0);
            result.addDefaultKeypress('n', juce::ModifierKeys::commandModifier);
            break;
            
        case CommandIDs::FileOpen:
            result.setInfo("Open...", "Open a document", "File", 0);
            result.addDefaultKeypress('o', juce::ModifierKeys::commandModifier);
            break;
            
        case CommandIDs::FileSave:
            result.setInfo("Save", "Save the current document", "File", 0);
            result.addDefaultKeypress('s', juce::ModifierKeys::commandModifier);
            result.setActive(current_file_.exists());
            break;
            
        case CommandIDs::FileSaveAs:
            result.setInfo("Save As...", "Save the document with a new name", "File", 0);
            result.addDefaultKeypress('s', juce::ModifierKeys::commandModifier | juce::ModifierKeys::shiftModifier);
            result.setActive(current_file_.exists());
            break;
            
        case CommandIDs::FileExit:
            result.setInfo("Exit", "Exit the application", "File", 0);
            result.addDefaultKeypress('q', juce::ModifierKeys::commandModifier);
            break;
            
        case CommandIDs::EditUndo:
            result.setInfo("Undo", "Undo the last action", "Edit", 0);
            result.addDefaultKeypress('z', juce::ModifierKeys::commandModifier);
            break;
            
        case CommandIDs::EditRedo:
            result.setInfo("Redo", "Redo the last undone action", "Edit", 0);
            result.addDefaultKeypress('z', juce::ModifierKeys::commandModifier | juce::ModifierKeys::shiftModifier);
            break;
            
        case CommandIDs::ViewFullscreen:
            result.setInfo("Toggle Fullscreen", "Toggle fullscreen mode", "View", 0);
            result.addDefaultKeypress('f', juce::ModifierKeys::commandModifier | juce::ModifierKeys::ctrlModifier);
            result.setTicked(isFullScreen());
            break;
            
        case CommandIDs::TransportPlay:
            result.setInfo("Play", "Start playback", "Transport", 0);
            result.addDefaultKeypress(juce::KeyPress::spaceKey, juce::ModifierKeys::noModifiers);
            break;
            
        case CommandIDs::TransportStop:
            result.setInfo("Stop", "Stop playback", "Transport", 0);
            result.addDefaultKeypress(juce::KeyPress::escapeKey, juce::ModifierKeys::noModifiers);
            break;
            
        // Add more command info as needed
        default:
            break;
    }
}

bool VitalMainWindow::perform(const juce::ApplicationCommandTarget::InvocationInfo& info) {
    switch (info.commandID) {
        case CommandIDs::FileNew:
            onFileNew();
            return true;
            
        case CommandIDs::FileOpen:
            onFileOpen();
            return true;
            
        case CommandIDs::FileSave:
            onFileSave();
            return true;
            
        case CommandIDs::FileSaveAs:
            onFileSaveAs();
            return true;
            
        case CommandIDs::FileExportAudio:
            onFileExportAudio();
            return true;
            
        case CommandIDs::FileExit:
            closeButtonPressed();
            return true;
            
        case CommandIDs::EditUndo:
            onEditUndo();
            return true;
            
        case CommandIDs::EditRedo:
            onEditRedo();
            return true;
            
        case CommandIDs::EditCut:
            onEditCut();
            return true;
            
        case CommandIDs::EditCopy:
            onEditCopy();
            return true;
            
        case CommandIDs::EditPaste:
            onEditPaste();
            return true;
            
        case CommandIDs::EditSelectAll:
            onEditSelectAll();
            return true;
            
        case CommandIDs::EditPreferences:
            onEditPreferences();
            return true;
            
        case CommandIDs::ViewZoomIn:
            onViewZoomIn();
            return true;
            
        case CommandIDs::ViewZoomOut:
            onViewZoomOut();
            return true;
            
        case CommandIDs::ViewZoomToFit:
            onViewZoomToFit();
            return true;
            
        case CommandIDs::ViewFullscreen:
            onViewFullscreen();
            return true;
            
        case CommandIDs::ViewTheme:
            onViewTheme();
            return true;
            
        case CommandIDs::ViewPerformanceMonitor:
            onViewPerformanceMonitor();
            return true;
            
        case CommandIDs::ViewLogWindow:
            onViewLogWindow();
            return true;
            
        case CommandIDs::TransportPlay:
            onTransportPlay();
            return true;
            
        case CommandIDs::TransportStop:
            onTransportStop();
            return true;
            
        case CommandIDs::TransportRecord:
            onTransportRecord();
            return true;
            
        case CommandIDs::TransportRewind:
            onTransportRewind();
            return true;
            
        case CommandIDs::TransportForward:
            onTransportForward();
            return true;
            
        case CommandIDs::PresetLoad:
            onPresetLoad();
            return true;
            
        case CommandIDs::PresetSave:
            onPresetSave();
            return true;
            
        case CommandIDs::PresetRandomize:
            onPresetRandomize();
            return true;
            
        case CommandIDs::PresetReset:
            onPresetReset();
            return true;
            
        case CommandIDs::PluginLoad:
            onPluginLoad();
            return true;
            
        case CommandIDs::PluginReload:
            onPluginReload();
            return true;
            
        case CommandIDs::PluginInfo:
            onPluginInfo();
            return true;
            
        case CommandIDs::HelpAbout:
            onHelpAbout();
            return true;
            
        case CommandIDs::HelpHelp:
            onHelpHelp();
            return true;
            
        case CommandIDs::HelpVisitWebsite:
            onHelpVisitWebsite();
            return true;
            
        default:
            return false;
    }
}

//==============================================================================
bool VitalMainWindow::keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent) {
    // Handle global keyboard shortcuts
    if (key == juce::KeyPress('f', juce::ModifierKeys::commandModifier | juce::ModifierKeys::ctrlModifier, 0)) {
        onViewFullscreen();
        return true;
    }
    
    if (key == juce::KeyPress(juce::KeyPress::F11Key)) {
        onViewFullscreen();
        return true;
    }
    
    if (key == juce::KeyPress(juce::KeyPress::F12Key)) {
        onViewPerformanceMonitor();
        return true;
    }
    
    return false;
}

bool VitalMainWindow::keyStateChanged(bool isKeyDown, juce::Component* originatingComponent) {
    // Handle key state changes
    return false;
}

//==============================================================================
juce::StringArray VitalMainWindow::getMenuBarNames() {
    return {"File", "Edit", "View", "Transport", "Preset", "Plugin", "Help"};
}

juce::PopupMenu VitalMainWindow::getMenuForIndex(int menuIndex, const juce::String& menuName) {
    juce::PopupMenu menu;
    
    switch (menuIndex) {
        case 0: // File
            menu.addCommandItem(getCommandManager(), CommandIDs::FileNew);
            menu.addCommandItem(getCommandManager(), CommandIDs::FileOpen);
            menu.addSeparator();
            menu.addCommandItem(getCommandManager(), CommandIDs::FileSave);
            menu.addCommandItem(getCommandManager(), CommandIDs::FileSaveAs);
            menu.addSeparator();
            menu.addCommandItem(getCommandManager(), CommandIDs::FileExportAudio);
            menu.addSeparator();
            menu.addCommandItem(getCommandManager(), CommandIDs::FileExit);
            break;
            
        case 1: // Edit
            menu.addCommandItem(getCommandManager(), CommandIDs::EditUndo);
            menu.addCommandItem(getCommandManager(), CommandIDs::EditRedo);
            menu.addSeparator();
            menu.addCommandItem(getCommandManager(), CommandIDs::EditCut);
            menu.addCommandItem(getCommandManager(), CommandIDs::EditCopy);
            menu.addCommandItem(getCommandManager(), CommandIDs::EditPaste);
            menu.addCommandItem(getCommandManager(), CommandIDs::EditSelectAll);
            menu.addSeparator();
            menu.addCommandItem(getCommandManager(), CommandIDs::EditPreferences);
            break;
            
        case 2: // View
            menu.addCommandItem(getCommandManager(), CommandIDs::ViewZoomIn);
            menu.addCommandItem(getCommandManager(), CommandIDs::ViewZoomOut);
            menu.addCommandItem(getCommandManager(), CommandIDs::ViewZoomToFit);
            menu.addSeparator();
            menu.addCommandItem(getCommandManager(), CommandIDs::ViewFullscreen);
            menu.addCommandItem(getCommandManager(), CommandIDs::ViewTheme);
            menu.addSeparator();
            menu.addCommandItem(getCommandManager(), CommandIDs::ViewPerformanceMonitor);
            menu.addCommandItem(getCommandManager(), CommandIDs::ViewLogWindow);
            break;
            
        case 3: // Transport
            menu.addCommandItem(getCommandManager(), CommandIDs::TransportPlay);
            menu.addCommandItem(getCommandManager(), CommandIDs::TransportStop);
            menu.addCommandItem(getCommandManager(), CommandIDs::TransportRecord);
            menu.addSeparator();
            menu.addCommandItem(getCommandManager(), CommandIDs::TransportRewind);
            menu.addCommandItem(getCommandManager(), CommandIDs::TransportForward);
            break;
            
        case 4: // Preset
            menu.addCommandItem(getCommandManager(), CommandIDs::PresetLoad);
            menu.addCommandItem(getCommandManager(), CommandIDs::PresetSave);
            menu.addSeparator();
            menu.addCommandItem(getCommandManager(), CommandIDs::PresetRandomize);
            menu.addCommandItem(getCommandManager(), CommandIDs::PresetReset);
            break;
            
        case 5: // Plugin
            menu.addCommandItem(getCommandManager(), CommandIDs::PluginLoad);
            menu.addCommandItem(getCommandManager(), CommandIDs::PluginReload);
            menu.addCommandItem(getCommandManager(), CommandIDs::PluginInfo);
            break;
            
        case 6: // Help
            menu.addCommandItem(getCommandManager(), CommandIDs::HelpAbout);
            menu.addCommandItem(getCommandManager(), CommandIDs::HelpHelp);
            menu.addSeparator();
            menu.addCommandItem(getCommandManager(), CommandIDs::HelpVisitWebsite);
            break;
    }
    
    return menu;
}

void VitalMainWindow::menuItemSelected(int menuItemID, int topLevelMenuIndex) {
    // Handle menu item selection
    // This would be called when a specific menu item is selected
}

//==============================================================================
bool VitalMainWindow::isInterestedInDragSource(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) {
    // Check if we're interested in the drag source
    return dragSourceDetails.description.isA<juce::File>() ||
           dragSourceDetails.description.isA<juce::String>() ||
           dragSourceDetails.description.isA<juce::var>();
}

void VitalMainWindow::itemDropped(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) {
    try {
        if (dragSourceDetails.description.isA<juce::File>()) {
            juce::File droppedFile = dragSourceDetails.description;
            if (droppedFile.existsAsFile()) {
                if (droppedFile.hasFileExtension("vital") || droppedFile.hasFileExtension("xml")) {
                    loadPresetFromFile(droppedFile);
                } else if (droppedFile.hasFileExtension("wav") || droppedFile.hasFileExtension("mp3")) {
                    // Import audio file
                }
            }
        } else if (dragSourceDetails.description.isA<juce::String>()) {
            juce::String droppedText = dragSourceDetails.description;
            // Handle text drops
        }
        
    } catch (const std::exception& e) {
        DBG("Exception handling drop: " << e.what());
    }
}

void VitalMainWindow::itemDragEnter(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) {
    // Handle drag enter
}

void VitalMainWindow::itemDragExit(const juce::DragAndDropTarget::SourceDetails& dragSourceDetails) {
    // Handle drag exit
}

//==============================================================================
bool VitalMainWindow::create() {
    try {
        // Set up command manager
        auto* app = juce::JUCEApplication::getInstance();
        if (app) {
            auto* commandManager = app->getCommandManager();
            setApplicationCommandManagerToWatch(commandManager);
        }
        
        // Set up keyboard focus
        addKeyListener(this);
        
        // Set up drag and drop
        setDropTargetEnabled(true);
        
        // Create main content
        if (!main_content_) {
            main_content_ = std::make_unique<juce::Component>();
            addAndMakeVisible(main_content_.get());
        }
        
        // Set initial layout
        updateLayout();
        
        // Show the window
        setVisible(true);
        toFront(true);
        
        DBG("Main window created successfully");
        return true;
        
    } catch (const std::exception& e) {
        DBG("Exception creating main window: " << e.what());
        return false;
    }
}

void VitalMainWindow::destroy() {
    try {
        // Clean up components
        setApplicationCommandManagerToWatch(nullptr);
        
        removeKeyListener(this);
        setDropTargetEnabled(false);
        
        // Reset UI
        setContentComponent(nullptr);
        
        DBG("Main window destroyed");
        
    } catch (const std::exception& e) {
        DBG("Exception destroying main window: " << e.what());
    }
}

//==============================================================================
void VitalMainWindow::setWindowState(vital::window_state::WindowState state) {
    switch (state) {
        case vital::window_state::WindowState::Normal:
            setFullScreen(false);
            setBounds(last_normal_bounds_);
            break;
            
        case vital::window_state::WindowState::Minimized:
            minimiseButtonPressed();
            break;
            
        case vital::window_state::WindowState::Maximized:
            setFullScreen(true);
            break;
            
        case vital::window_state::WindowState::Fullscreen:
            enterFullscreen();
            break;
            
        case vital::window_state::WindowState::AlwaysOnTop:
            toggleAlwaysOnTop();
            break;
    }
}

vital::window_state::WindowState VitalMainWindow::getWindowState() const {
    return window_state_;
}

void VitalMainWindow::toggleFullscreen() {
    if (isFullScreen()) {
        exitFullscreen();
    } else {
        enterFullscreen();
    }
}

void VitalMainWindow::toggleAlwaysOnTop() {
    setAlwaysOnTop(!isAlwaysOnTop());
}

//==============================================================================
void VitalMainWindow::updateLayout() {
    auto bounds = getBounds();
    
    // Calculate layout areas
    int top_offset = 0;
    
    if (title_bar_visible_) {
        top_offset += window_config::TITLE_BAR_HEIGHT;
    }
    
    if (menu_bar_visible_ && menu_bar_) {
        top_offset += window_config::MENU_BAR_HEIGHT;
        layoutMenuBar();
    }
    
    if (toolbar_visible_ && toolbar_) {
        top_offset += window_config::TOOLBAR_HEIGHT;
        layoutToolbar();
    }
    
    int bottom_offset = 0;
    
    if (status_bar_visible_ && status_bar_) {
        bottom_offset += window_config::STATUS_BAR_HEIGHT;
        layoutStatusBar();
    }
    
    // Main content area
    if (main_content_) {
        juce::Rectangle<int> content_bounds = bounds.reduced(0, top_offset, 0, bottom_offset);
        main_content_->setBounds(content_bounds);
    }
    
    // Notification overlay
    layoutNotificationOverlay();
}

void VitalMainWindow::setTitleBarVisible(bool visible) {
    title_bar_visible_ = visible;
    setUsingNativeTitleBar(visible);
    updateLayout();
}

void VitalMainWindow::setMenuBarVisible(bool visible) {
    menu_bar_visible_ = visible;
    if (menu_bar_) {
        menu_bar_->setVisible(visible);
    }
    updateLayout();
}

void VitalMainWindow::setStatusBarVisible(bool visible) {
    status_bar_visible_ = visible;
    if (status_bar_) {
        status_bar_->setVisible(visible);
    }
    updateLayout();
}

void VitalMainWindow::setToolbarVisible(bool visible) {
    toolbar_visible_ = visible;
    if (toolbar_) {
        toolbar_->setVisible(visible);
    }
    updateLayout();
}

//==============================================================================
void VitalMainWindow::setContent(juce::Component* contentComponent) {
    juce::DocumentWindow::setContentComponent(contentComponent, true);
    updateLayout();
}

juce::Component* VitalMainWindow::getContent() const {
    return getContentComponent();
}

//==============================================================================
void VitalMainWindow::rebuildMenus() {
    if (menu_bar_) {
        // Rebuild menu bar with current items
        // This would recreate the menu structure
    }
    updateMenus();
}

void VitalMainWindow::updateRecentFilesMenu() {
    // Update recent files list
    // Implementation would update the recent files submenu
}

void VitalMainWindow::addRecentFile(const juce::File& file) {
    // Add file to recent files list
    recent_files_.removeString(file.getFullPathName());
    recent_files_.insert(0, file.getFullPathName());
    
    // Limit list size
    while (recent_files_.size() > MAX_RECENT_FILES) {
        recent_files_.remove(recent_files_.size() - 1);
    }
    
    // Save to properties
    auto& properties = juce::PropertiesFile::getDefaultProperties();
    juce::StringArray recentFileStrings = recent_files_;
    properties.setValue("recent_files", recentFileStrings.joinIntoString("|"));
    
    updateRecentFilesMenu();
}

//==============================================================================
void VitalMainWindow::openFile() {
    onFileOpen();
}

void VitalMainWindow::saveFile() {
    onFileSave();
}

void VitalMainWindow::saveFileAs() {
    onFileSaveAs();
}

void VitalMainWindow::exportAudio() {
    onFileExportAudio();
}

//==============================================================================
void VitalMainWindow::loadPreset() {
    onPresetLoad();
}

void VitalMainWindow::savePreset() {
    onPresetSave();
}

void VitalMainWindow::loadPresetFromFile(const juce::File& file) {
    try {
        if (!file.exists()) {
            showErrorDialog("Load Preset", "File does not exist: " + file.getFullPathName());
            return;
        }
        
        // Load preset using XML
        auto xml = juce::XmlDocument::parse(file);
        if (!xml || !xml->hasTagName("VITAL")) {
            showErrorDialog("Load Preset", "Invalid preset file format");
            return;
        }
        
        // Apply preset to processor
        auto app = vital::VitalApplication::getInstance();
        if (app && app->getMainWindow()) {
            // Load preset into processor
        }
        
        // Update file state
        current_file_ = file;
        markFileModified(false);
        addRecentFile(file);
        
        showNotification("Preset Loaded", "Successfully loaded: " + file.getFileNameWithoutExtension());
        
    } catch (const std::exception& e) {
        showErrorDialog("Load Preset", "Error loading preset: " + juce::String(e.what()));
    }
}

void VitalMainWindow::savePresetToFile(const juce::File& file) {
    try {
        // Create preset XML
        auto xml = std::make_unique<juce::XmlElement>("VITAL");
        xml->setAttribute("version", vital::app::VERSION_STRING);
        xml->setAttribute("type", "preset");
        
        // Get application instance
        auto app = vital::VitalApplication::getInstance();
        if (app && app->getMainWindow()) {
            // Save current state to XML
            // This would save all current parameters and settings
        }
        
        // Write to file
        if (xml->writeTo(file)) {
            current_file_ = file;
            markFileModified(false);
            addRecentFile(file);
            showNotification("Preset Saved", "Successfully saved: " + file.getFileNameWithoutExtension());
        } else {
            showErrorDialog("Save Preset", "Failed to write preset file");
        }
        
    } catch (const std::exception& e) {
        showErrorDialog("Save Preset", "Error saving preset: " + juce::String(e.what()));
    }
}

//==============================================================================
void VitalMainWindow::loadPlugin() {
    onPluginLoad();
}

void VitalMainWindow::reloadPlugin() {
    onPluginReload();
}

void VitalMainWindow::showPluginInfo() {
    onPluginInfo();
}

//==============================================================================
void VitalMainWindow::showAbout() {
    onHelpAbout();
}

void VitalMainWindow::showPreferences() {
    onEditPreferences();
}

void VitalMainWindow::showPerformanceMonitor() {
    onViewPerformanceMonitor();
}

void VitalMainWindow::showLogWindow() {
    onViewLogWindow();
}

void VitalMainWindow::showHelp() {
    onHelpHelp();
}

//==============================================================================
void VitalMainWindow::applyTheme(const juce::String& themeName) {
    current_theme_ = themeName;
    applyThemeToWindow();
    
    // Save theme preference
    auto& properties = juce::PropertiesFile::getDefaultProperties();
    properties.setValue("current_theme", themeName);
}

void VitalMainWindow::cycleTheme() {
    // Find current theme index
    int currentIndex = -1;
    for (int i = 0; i < available_themes_.size(); ++i) {
        if (available_themes_[i] == current_theme_) {
            currentIndex = i;
            break;
        }
    }
    
    // Move to next theme
    int nextIndex = (currentIndex + 1) % available_themes_.size();
    applyTheme(available_themes_[nextIndex]);
}

//==============================================================================
void VitalMainWindow::saveWindowState() {
    auto& properties = juce::PropertiesFile::getDefaultProperties();
    
    properties.setValue("window_width", getWidth());
    properties.setValue("window_height", getHeight());
    properties.setValue("window_x", getX());
    properties.setValue("window_y", getY());
    properties.setValue("window_maximized", isFullScreen());
    properties.setValue("window_fullscreen", isFullScreen());
    properties.setValue("window_theme", current_theme_);
    properties.setValue("window_ui_scale", ui_scale_);
    properties.setValue("window_high_dpi", high_dpi_enabled_);
    
    DBG("Window state saved");
}

void VitalMainWindow::restoreWindowState() {
    auto& properties = juce::PropertiesFile::getDefaultProperties();
    
    int x = properties.getValue("window_x", 100);
    int y = properties.getValue("window_y", 100);
    int width = properties.getValue("window_width", window_config::DEFAULT_WIDTH);
    int height = properties.getValue("window_height", window_config::DEFAULT_HEIGHT);
    bool maximized = properties.getValue("window_maximized", false);
    bool fullscreen = properties.getValue("window_fullscreen", false);
    
    current_theme_ = properties.getValue("window_theme", "default");
    ui_scale_ = properties.getValue("window_ui_scale", 1.0f);
    high_dpi_enabled_ = properties.getValue("window_high_dpi", true);
    
    setBounds(x, y, width, height);
    
    if (fullscreen) {
        enterFullscreen();
    } else if (maximized) {
        setFullScreen(true);
    }
    
    applyTheme(current_theme_);
}

//==============================================================================
void VitalMainWindow::enterFullscreen() {
    last_fullscreen_bounds_ = getBounds();
    setFullScreen(true);
    fullscreen_ = true;
    window_state_ = vital::window_state::WindowState::Fullscreen;
    
    // Hide UI elements in fullscreen
    setMenuBarVisible(false);
    setToolbarVisible(false);
    setStatusBarVisible(false);
    setTitleBarVisible(false);
}

void VitalMainWindow::exitFullscreen() {
    setFullScreen(false);
    fullscreen_ = false;
    window_state_ = vital::window_state::WindowState::Normal;
    
    // Restore UI elements
    setMenuBarVisible(true);
    setToolbarVisible(true);
    setStatusBarVisible(true);
    setTitleBarVisible(true);
    
    // Restore previous bounds
    setBounds(last_fullscreen_bounds_);
}

bool VitalMainWindow::isFullscreen() const {
    return fullscreen_;
}

//==============================================================================
void VitalMainWindow::setHighDPIEnabled(bool enabled) {
    high_dpi_enabled_ = enabled;
    if (enabled) {
        juce::Desktop::getInstance().setGlobalScaleFactor(ui_scale_);
    } else {
        juce::Desktop::getInstance().setGlobalScaleFactor(1.0f);
    }
}

void VitalMainWindow::setUIScale(float scale) {
    ui_scale_ = juce::jlimit(0.5f, 2.0f, scale);
    if (high_dpi_enabled_) {
        juce::Desktop::getInstance().setGlobalScaleFactor(ui_scale_);
    }
}

float VitalMainWindow::getUIScale() const {
    return ui_scale_;
}

//==============================================================================
void VitalMainWindow::setAccessibilityEnabled(bool enabled) {
    accessibility_enabled_ = enabled;
    setAccessible(enabled);
}

void VitalMainWindow::setAccessibilityText(const juce::String& text) {
    if (accessibility_enabled_) {
        setDescription(text);
    }
}

//==============================================================================
void VitalMainWindow::updatePerformanceStats(float cpuLoad, float memoryUsage, float latency) {
    current_cpu_load_.store(cpuLoad);
    current_memory_usage_.store(memoryUsage);
    current_latency_.store(latency);
}

void VitalMainWindow::showPerformanceOverlay(bool show) {
    if (performance_overlay_) {
        performance_overlay_->setVisible(show);
    }
}

//==============================================================================
void VitalMainWindow::enableMIDIInput(bool enabled) {
    midi_input_enabled_ = enabled;
    
    if (enabled && selected_midi_device_ >= 0) {
        setupMIDIInput(selected_midi_device_);
    }
}

void VitalMainWindow::selectMIDIInput(int deviceIndex) {
    selected_midi_device_ = deviceIndex;
    if (midi_input_enabled_) {
        setupMIDIInput(deviceIndex);
    }
}

void VitalMainWindow::showMIDILearningDialog() {
    // Show MIDI learning dialog
    // This would open a dialog for MIDI mapping
}

//==============================================================================
void VitalMainWindow::showAudioSettings() {
    // Show audio settings dialog
    // This would open the audio device settings
}

void VitalMainWindow::enableAudioOutput(bool enabled) {
    audio_output_enabled_ = enabled;
}

void VitalMainWindow::setAudioDevice(const juce::String& deviceName) {
    selected_audio_device_ = deviceName;
    if (audio_output_enabled_) {
        setupAudioOutput(deviceName);
    }
}

//==============================================================================
void VitalMainWindow::setCustomTitleBar(juce::Component* titleBar) {
    custom_title_bar_.reset(titleBar);
    // Replace native title bar with custom one
}

void VitalMainWindow::setCustomStatusBar(juce::Component* statusBar) {
    custom_status_bar_.reset(statusBar);
    addAndMakeVisible(custom_status_bar_.get());
    updateLayout();
}

void VitalMainWindow::setCustomToolbar(juce::Component* toolbar) {
    custom_toolbar_.reset(toolbar);
    addAndMakeVisible(custom_toolbar_.get());
    updateLayout();
}

//==============================================================================
// Event handlers implementation
void VitalMainWindow::onFileNew() {
    current_file_ = juce::File();
    markFileModified(false);
    updateTitle();
}

void VitalMainWindow::onFileOpen() {
    juce::FileFilter filter("Vital Presets (*.vital)|*.vital", "", "");
    juce::FileChooser chooser("Open Preset", juce::File::getCurrentWorkingDirectory(), filter.getPatterns().joinIntoString(";"));
    
    if (chooser.browseForFileToOpen()) {
        loadPresetFromFile(chooser.getResult());
    }
}

void VitalMainWindow::onFileSave() {
    if (current_file_.exists()) {
        savePresetToFile(current_file_);
    } else {
        onFileSaveAs();
    }
}

void VitalMainWindow::onFileSaveAs() {
    juce::FileFilter filter("Vital Presets (*.vital)|*.vital", "", "");
    juce::FileChooser chooser("Save Preset As", current_file_.exists() ? current_file_ : juce::File::getCurrentWorkingDirectory(), filter.getPatterns().joinIntoString(";"));
    
    if (chooser.browseForFileToSave(true)) {
        juce::File file = chooser.getResult();
        if (!file.hasFileExtension("vital")) {
            file = file.withFileExtension("vital");
        }
        savePresetToFile(file);
    }
}

void VitalMainWindow::onFileExportAudio() {
    // Show audio export dialog
    showNotification("Export Audio", "Audio export functionality will be implemented");
}

void VitalMainWindow::onFileExit() {
    closeButtonPressed();
}

void VitalMainWindow::onEditUndo() {
    // Undo functionality
}

void VitalMainWindow::onEditRedo() {
    // Redo functionality
}

void VitalMainWindow::onEditCut() {
    // Cut functionality
}

void VitalMainWindow::onEditCopy() {
    // Copy functionality
}

void VitalMainWindow::onEditPaste() {
    // Paste functionality
}

void VitalMainWindow::onEditSelectAll() {
    // Select all functionality
}

void VitalMainWindow::onEditPreferences() {
    showPreferences();
}

void VitalMainWindow::onViewZoomIn() {
    setUIScale(ui_scale_ * 1.1f);
    updateLayout();
}

void VitalMainWindow::onViewZoomOut() {
    setUIScale(ui_scale_ / 1.1f);
    updateLayout();
}

void VitalMainWindow::onViewZoomToFit() {
    setUIScale(1.0f);
    updateLayout();
}

void VitalMainWindow::onViewFullscreen() {
    toggleFullscreen();
}

void VitalMainWindow::onViewTheme() {
    cycleTheme();
}

void VitalMainWindow::onViewPerformanceMonitor() {
    showPerformanceOverlay(true);
}

void VitalMainWindow::onViewLogWindow() {
    showLogWindow();
}

void VitalMainWindow::onTransportPlay() {
    // Start transport
}

void VitalMainWindow::onTransportStop() {
    // Stop transport
}

void VitalMainWindow::onTransportRecord() {
    // Start recording
}

void VitalMainWindow::onTransportRewind() {
    // Rewind transport
}

void VitalMainWindow::onTransportForward() {
    // Fast forward transport
}

void VitalMainWindow::onPresetLoad() {
    loadPreset();
}

void VitalMainWindow::onPresetSave() {
    savePreset();
}

void VitalMainWindow::onPresetRandomize() {
    // Randomize preset parameters
}

void VitalMainWindow::onPresetReset() {
    // Reset to default preset
}

void VitalMainWindow::onPluginLoad() {
    // Load plugin
}

void VitalMainWindow::onPluginReload() {
    // Reload plugin
}

void VitalMainWindow::onPluginInfo() {
    // Show plugin information
}

void VitalMainWindow::onHelpAbout() {
    juce::AlertWindow::showMessageBox(
        juce::AlertWindow::InfoIcon,
        "About Vital Synthesizer",
        "Vital Synthesizer v" + vital::app::VERSION_STRING + "\n\n"
        "Advanced synthesizer with modern features and high performance.\n\n"
        "Copyright (c) 2025 Vital Application Developers",
        "OK"
    );
}

void VitalMainWindow::onHelpHelp() {
    showHelp();
}

void VitalMainWindow::onHelpVisitWebsite() {
    // Open website
    juce::URL("https://vital-synthesizer.com").launchInDefaultBrowser();
}

//==============================================================================
void VitalMainWindow::initializeComponents() {
    // Create menu bar
    menu_bar_ = std::make_unique<juce::MenuBarComponent>(this);
    addAndMakeVisible(menu_bar_.get());
    
    // Create main content area
    main_content_ = std::make_unique<juce::Component>();
    addAndMakeVisible(main_content_.get());
    
    // Create status bar
    status_bar_ = std::make_unique<juce::Component>();
    status_bar_->setOpaque(true);
    addAndMakeVisible(status_bar_.get());
    
    // Create toolbar
    toolbar_ = std::make_unique<juce::Component>();
    addAndMakeVisible(toolbar_.get());
    
    // Create notification overlay
    notification_overlay_ = std::make_unique<juce::Component>();
    notification_overlay_->setOpaque(false);
    notification_overlay_->setVisible(false);
    addAndMakeVisible(notification_overlay_.get());
    
    // Create performance overlay
    createPerformanceOverlay();
}

void VitalMainWindow::initializeMenus() {
    // Set up menu bar
    setMenuBar(this);
}

void VitalMainWindow::initializeThemes() {
    // Load available themes
    available_themes_ = {"default", "dark", "light", "high-contrast"};
    
    // Load current theme
    auto& properties = juce::PropertiesFile::getDefaultProperties();
    current_theme_ = properties.getValue("current_theme", "default");
    
    applyThemeToWindow();
}

void VitalMainWindow::initializeMIDI() {
    refreshMIDIDevices();
}

void VitalMainWindow::initializeAudio() {
    refreshAudioDevices();
}

void VitalMainWindow::initializeFileManagement() {
    // Load recent files
    auto& properties = juce::PropertiesFile::getDefaultProperties();
    juce::String recentFilesString = properties.getValue("recent_files", "");
    recent_files_ = juce::StringArray::fromTokens(recentFilesString, "|", "\"");
}

void VitalMainWindow::layoutMenuBar() {
    if (menu_bar_ && menu_bar_visible_) {
        menu_bar_->setBounds(getBounds().removeFromTop(window_config::MENU_BAR_HEIGHT));
    }
}

void VitalMainWindow::layoutToolbar() {
    if (toolbar_ && toolbar_visible_) {
        auto bounds = getBounds();
        int top_offset = window_config::TITLE_BAR_HEIGHT;
        
        if (menu_bar_visible_) {
            top_offset += window_config::MENU_BAR_HEIGHT;
        }
        
        toolbar_->setBounds(bounds.removeFromTop(window_config::TOOLBAR_HEIGHT).withY(top_offset));
    }
}

void VitalMainWindow::layoutMainContent() {
    if (main_content_) {
        auto bounds = getBounds();
        int top_offset = 0;
        int bottom_offset = 0;
        
        if (menu_bar_visible_) {
            top_offset += window_config::MENU_BAR_HEIGHT;
        }
        
        if (toolbar_visible_) {
            top_offset += window_config::TOOLBAR_HEIGHT;
        }
        
        if (status_bar_visible_) {
            bottom_offset += window_config::STATUS_BAR_HEIGHT;
        }
        
        main_content_->setBounds(bounds.reduced(0, top_offset, 0, bottom_offset));
    }
}

void VitalMainWindow::layoutStatusBar() {
    if (status_bar_ && status_bar_visible_) {
        status_bar_->setBounds(getBounds().removeFromBottom(window_config::STATUS_BAR_HEIGHT));
    }
}

void VitalMainWindow::layoutNotificationOverlay() {
    if (notification_overlay_) {
        notification_overlay_->setBounds(getBounds().removeFromTop(NOTIFICATION_HEIGHT));
    }
}

void VitalMainWindow::updateTitle() {
    juce::String title = "Vital Synthesizer";
    
    if (current_file_.exists()) {
        title += " - " + current_file_.getFileNameWithoutExtension();
    }
    
    if (file_modified_) {
        title += " *";
    }
    
    setName(title);
}

void VitalMainWindow::updateStatusBar() {
    // Update status bar content
    if (status_bar_) {
        // This would update status bar text and indicators
    }
}

void VitalMainWindow::updateToolbar() {
    // Update toolbar content
    if (toolbar_) {
        // This would update toolbar buttons and controls
    }
}

void VitalMainWindow::updateMenus() {
    // Update menu items based on current state
    if (menu_bar_) {
        menu_bar_->refreshMenuBar();
    }
}

void VitalMainWindow::markFileModified(bool modified) {
    file_modified_ = modified;
    updateTitle();
}

void VitalMainWindow::checkAutoSave() {
    // Check if auto-save is needed
    if (file_modified_) {
        auto currentTime = juce::Time::getCurrentTime();
        auto timeSinceModified = currentTime - juce::Time::fromString(file_modified_time_);
        
        if (timeSinceModified.inMinutes() >= 5) {
            performAutoSave();
        }
    }
}

void VitalMainWindow::performAutoSave() {
    if (current_file_.exists() && file_modified_) {
        savePresetToFile(current_file_);
    }
}

void VitalMainWindow::createNotificationOverlay() {
    // Create notification overlay components
    // This would add notification display capabilities
}

void VitalMainWindow::showNotificationInternal(const juce::String& title, const juce::String& message,
                                             const juce::Colour& color, int timeoutMs) {
    // Show notification with timeout
    notification_overlay_->setVisible(true);
    
    // Schedule to hide after timeout
    juce::Timer* hideTimer = new juce::Timer();
    hideTimer->startTimer(timeoutMs);
    hideTimer->onTimer = [this, hideTimer]() {
        notification_overlay_->setVisible(false);
        delete hideTimer;
    };
}

void VitalMainWindow::showNotification(const juce::String& title, const juce::String& message, int timeoutMs) {
    showNotificationInternal(title, message, juce::Colours::green, timeoutMs);
}

void VitalMainWindow::showProgressNotification(const juce::String& title, const juce::String& message, float progress) {
    // Show progress notification
    notification_overlay_->setVisible(true);
    // This would show a progress bar
}

void VitalMainWindow::showErrorDialog(const juce::String& title, const juce::String& message) {
    juce::AlertWindow::showMessageBox(
        juce::AlertWindow::ErrorIcon,
        title,
        message,
        "OK"
    );
}

void VitalMainWindow::showWarningDialog(const juce::String& title, const juce::String& message) {
    juce::AlertWindow::showMessageBox(
        juce::AlertWindow::WarningIcon,
        title,
        message,
        "OK"
    );
}

void VitalMainWindow::showInfoDialog(const juce::String& title, const juce::String& message) {
    juce::AlertWindow::showMessageBox(
        juce::AlertWindow::InfoIcon,
        title,
        message,
        "OK"
    );
}

void VitalMainWindow::createPerformanceOverlay() {
    performance_label_ = std::make_unique<juce::Label>("Performance", "CPU: 0% | Memory: 0 MB | Latency: 0 ms");
    performance_label_->setVisible(false);
    addAndMakeVisible(performance_label_.get());
}

void VitalMainWindow::updatePerformanceDisplay() {
    if (performance_label_ && performance_label_->isVisible()) {
        size_t memoryMB = current_memory_usage_.load() / (1024 * 1024);
        juce::String text = "CPU: " + juce::String(current_cpu_load_.load(), 1) + "% | " +
                           "Memory: " + juce::String(memoryMB) + " MB | " +
                           "Latency: " + juce::String(current_latency_.load(), 1) + " ms";
        performance_label_->setText(text, juce::dontSendNotification);
    }
}

// Continue with remaining implementation methods...
// (Additional methods would be implemented based on the specific requirements)

} // namespace vital
