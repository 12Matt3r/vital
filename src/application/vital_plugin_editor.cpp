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

#include "vital_plugin_editor.h"
#include "vital_plugin_processor.h"

namespace vital {

//==============================================================================
VitalPluginEditor::VitalPluginEditor(VitalPluginProcessor& processor)
    : juce::AudioProcessorEditor(processor),
      processor_(processor),
      custom_look_and_feel_(juce::LookAndFeel_V4::getLightLookAndFeel()) {
    
    try {
        // Set up look and feel
        setLookAndFeel(&custom_look_and_feel_);
        
        // Initialize components
        initializeComponents();
        initializeLayout();
        initializeTheme();
        initializeKeyboardShortcuts();
        initializeAccessibility();
        
        // Start timer for UI updates
        startTimer(1000 / UI_UPDATE_FREQUENCY);
        
        // Set initial size
        setSize(ui_config::DEFAULT_WIDTH, ui_config::DEFAULT_HEIGHT);
        
        DBG("Plugin editor initialized");
        
    } catch (const std::exception& e) {
        DBG("Exception initializing plugin editor: " << e.what());
    }
}

VitalPluginEditor::~VitalPluginEditor() {
    stopTimer();
    setLookAndFeel(nullptr);
    
    // Save UI state
    saveUISTate();
    
    DBG("Plugin editor destroyed");
}

//==============================================================================
void VitalPluginEditor::paint(juce::Graphics& g) {
    try {
        // Clear background
        g.fillAll(juce::Colours::darkgrey);
        
        // Draw background gradient
        auto bounds = getLocalBounds().toFloat();
        juce::ColourGradient gradient(juce::Colour(0xFF1A1A1A), bounds.getTopLeft(),
                                    juce::Colour(0xFF0D0D0D), bounds.getBottomRight());
        g.setGradientFill(gradient);
        g.fillRect(bounds);
        
        // Draw sections
        drawBackground(g);
        drawParameters(g);
        drawPerformanceOverlay(g);
        drawAnimations(g);
        
    } catch (const std::exception& e) {
        DBG("Exception in paint: " << e.what());
    }
}

void VitalPluginEditor::resized() {
    try {
        updateLayout();
        
    } catch (const std::exception& e) {
        DBG("Exception in resized: " << e.what());
    }
}

void VitalPluginEditor::visibilityChanged() {
    if (isVisible()) {
        // Load UI state when becoming visible
        loadUISTate();
        updateLayout();
    }
}

void VitalPluginEditor::focusOfChildComponentChanged(juce::Component::FocusChangeType cause) {
    // Handle focus changes for accessibility
    if (accessibility_enabled_) {
        // Update accessibility descriptions
        updateAccessibilityDescriptions();
    }
}

//==============================================================================
void VitalPluginEditor::timerCallback() {
    try {
        // Update UI elements
        updateParameterDisplay(-1); // Update all parameters
        updateProgramDisplay();
        updatePerformanceDisplay();
        
        // Handle animations
        if (animations_enabled_ && !animations_.empty()) {
            repaint();
        }
        
        // Handle MIDI learn timeout
        if (is_learning_.load()) {
            // Check for MIDI messages
            // This would be implemented with MIDI input processing
        }
        
    } catch (const std::exception& e) {
        DBG("Exception in timerCallback: " << e.what());
    }
}

//==============================================================================
bool VitalPluginEditor::keyPressed(const juce::KeyPress& key, juce::Component* originatingComponent) {
    // Handle keyboard shortcuts
    auto it = shortcuts_.find(key);
    if (it != shortcuts_.end()) {
        it->second();
        return true;
    }
    return false;
}

bool VitalPluginEditor::keyStateChanged(bool isKeyDown, juce::Component* originatingComponent) {
    // Handle key state changes
    return false;
}

//==============================================================================
void VitalPluginEditor::updateParameterDisplay(int parameterIndex) {
    if (parameterIndex >= 0 && parameterIndex < vital::plugin_config::MAX_PARAMETERS) {
        // Update specific parameter
        if (parameterIndex < static_cast<int>(parameter_sliders_.size())) {
            float value = processor_.getParameter(parameterIndex);
            if (parameter_sliders_[parameterIndex]) {
                parameter_sliders_[parameterIndex]->setValue(value, juce::dontSendNotification);
            }
        }
    } else {
        // Update all parameters
        for (int i = 0; i < vital::plugin_config::MAX_PARAMETERS && i < static_cast<int>(parameter_sliders_.size()); ++i) {
            float value = processor_.getParameter(i);
            if (parameter_sliders_[i]) {
                parameter_sliders_[i]->setValue(value, juce::dontSendNotification);
            }
        }
    }
}

void VitalPluginEditor::updateProgramDisplay() {
    if (program_selector_) {
        program_selector_->setSelectedItemIndex(processor_.getCurrentProgram(), juce::dontSendNotification);
    }
}

void VitalPluginEditor::updatePerformanceDisplay() {
    if (cpu_label_) {
        cpu_label_->setText("CPU: " + juce::String(current_cpu_load_.load(), 1) + "%", juce::dontSendNotification);
    }
    
    if (memory_label_) {
        size_t memory_mb = current_memory_usage_.load() / (1024 * 1024);
        memory_label_->setText("Memory: " + juce::String(memory_mb) + " MB", juce::dontSendNotification);
    }
    
    if (latency_label_) {
        latency_label_->setText("Latency: " + juce::String(current_latency_.load(), 1) + " ms", juce::dontSendNotification);
    }
}

//==============================================================================
void VitalPluginEditor::saveUISTate() {
    try {
        // Save editor state using JUCE properties
        auto& properties = juce::PropertiesFile::getDefaultProperties();
        properties.setValue("editor_width", getWidth());
        properties.setValue("editor_height", getHeight());
        properties.setValue("ui_scale", ui_scale_);
        properties.setValue("theme", current_theme_);
        properties.setValue("high_dpi", high_dpi_enabled_);
        properties.setValue("animations", animations_enabled_);
        properties.setValue("tooltips", tooltips_enabled_);
        
        DBG("UI state saved");
        
    } catch (const std::exception& e) {
        DBG("Exception saving UI state: " << e.what());
    }
}

void VitalPluginEditor::loadUISTate() {
    try {
        // Load editor state using JUCE properties
        auto& properties = juce::PropertiesFile::getDefaultProperties();
        
        int width = properties.getValue("editor_width", ui_config::DEFAULT_WIDTH);
        int height = properties.getValue("editor_height", ui_config::DEFAULT_HEIGHT);
        ui_scale_ = properties.getValue("ui_scale", 1.0f);
        current_theme_ = properties.getValue("theme", "default");
        high_dpi_enabled_ = properties.getValue("high_dpi", true);
        animations_enabled_ = properties.getValue("animations", true);
        tooltips_enabled_ = properties.getValue("tooltips", true);
        
        // Apply loaded state
        setEditorSize(width, height);
        setTheme(current_theme_);
        setHighDPIEnabled(high_dpi_enabled_);
        setAnimationsEnabled(animations_enabled_);
        setTooltipsEnabled(tooltips_enabled_);
        
        DBG("UI state loaded");
        
    } catch (const std::exception& e) {
        DBG("Exception loading UI state: " << e.what());
    }
}

void VitalPluginEditor::resetToDefaultLayout() {
    setSize(ui_config::DEFAULT_WIDTH, ui_config::DEFAULT_HEIGHT);
    setTheme("default");
    setHighDPIEnabled(true);
    setAnimationsEnabled(true);
    setTooltipsEnabled(true);
    repaint();
}

//==============================================================================
void VitalPluginEditor::loadPresetByName(const juce::String& presetName) {
    try {
        if (processor_.getPresetManager()) {
            bool success = processor_.getPresetManager()->loadPreset(presetName);
            if (success) {
                updateParameterDisplay(-1);
                announceProgramChange(processor_.getCurrentProgram());
            }
        }
        
    } catch (const std::exception& e) {
        DBG("Exception loading preset: " << e.what());
    }
}

void VitalPluginEditor::savePresetByName(const juce::String& presetName) {
    try {
        if (processor_.getPresetManager()) {
            bool success = processor_.getPresetManager()->savePreset(presetName);
            if (success) {
                updateProgramDisplay();
            }
        }
        
    } catch (const std::exception& e) {
        DBG("Exception saving preset: " << e.what());
    }
}

void VitalPluginEditor::randomizeParameters() {
    try {
        // Randomize all parameters
        for (int i = 0; i < vital::plugin_config::MAX_PARAMETERS; ++i) {
            float randomValue = juce::Random::getSystemRandom().nextFloat();
            processor_.setParameter(i, randomValue);
            if (i < static_cast<int>(parameter_sliders_.size()) && parameter_sliders_[i]) {
                parameter_sliders_[i]->setValue(randomValue, juce::dontSendNotification);
            }
        }
        
        // Announce randomization to accessibility
        if (accessibility_enabled_) {
            announceParameterChange(-1, -1.0f); // Special announcement for randomization
        }
        
    } catch (const std::exception& e) {
        DBG("Exception randomizing parameters: " << e.what());
    }
}

void VitalPluginEditor::resetAllParameters() {
    try {
        // Reset all parameters to default values
        for (int i = 0; i < vital::plugin_config::MAX_PARAMETERS; ++i) {
            processor_.setParameter(i, 0.0f);
            if (i < static_cast<int>(parameter_sliders_.size()) && parameter_sliders_[i]) {
                parameter_sliders_[i]->setValue(0.0f, juce::dontSendNotification);
            }
        }
        
        // Announce reset to accessibility
        if (accessibility_enabled_) {
            announceParameterChange(-2, -1.0f); // Special announcement for reset
        }
        
    } catch (const std::exception& e) {
        DBG("Exception resetting parameters: " << e.what());
    }
}

//==============================================================================
void VitalPluginEditor::initializeComponents() {
    // Create tooltips
    tooltip_window_ = std::make_unique<juce::TooltipWindow>("Vital Synthesizer", 2000);
    
    // Create main panels
    main_panel_ = std::make_unique<juce::Component>();
    parameter_panel_ = std::make_unique<juce::Component>();
    keyboard_panel_ = std::make_unique<juce::Component>();
    status_panel_ = std::make_unique<juce::Component>();
    
    // Add to component hierarchy
    addAndMakeVisible(main_panel_.get());
    addChildComponent(parameter_panel_.get());
    addChildComponent(keyboard_panel_.get());
    addChildComponent(status_panel_.get());
    
    // Create UI components
    createParameterSliders();
    createProgramControls();
    createPerformanceMonitoring();
    createVirtualKeyboard();
    createStatusBar();
}

void VitalPluginEditor::initializeLayout() {
    updateLayout();
}

void VitalPluginEditor::initializeTheme() {
    applyThemeColors();
}

void VitalPluginEditor::initializeKeyboardShortcuts() {
    // Add keyboard shortcuts
    shortcuts_[juce::KeyPress('r', juce::ModifierKeys::ctrlModifier, 0)] = [this]() { randomizeParameters(); };
    shortcuts_[juce::KeyPress('s', juce::ModifierKeys::ctrlModifier, 0)] = [this]() { 
        auto presetName = juce::String("Program " + juce::String(processor_.getCurrentProgram() + 1));
        savePresetByName(presetName);
    };
    shortcuts_[juce::KeyPress('l', juce::ModifierKeys::ctrlModifier, 0)] = [this]() {
        auto presetName = juce::String("Program " + juce::String(processor_.getCurrentProgram() + 1));
        loadPresetByName(presetName);
    };
    shortcuts_[juce::KeyPress('z', juce::ModifierKeys::ctrlModifier, 0)] = [this]() { undo(); };
    shortcuts_[juce::KeyPress('y', juce::ModifierKeys::ctrlModifier, 0)] = [this]() { redo(); };
    shortcuts_[juce::KeyPress(juce::KeyPress::F1Key)] = [this]() { resetToDefaultLayout(); };
}

void VitalPluginEditor::initializeAccessibility() {
    setupAccessibilityLabels();
    updateAccessibilityDescriptions();
}

//==============================================================================
void VitalPluginEditor::layoutMainPanel() {
    if (!main_panel_) return;
    
    auto bounds = getLocalBounds();
    
    // Reserve space for status bar
    int status_height = 30;
    bounds = bounds.removeFromBottom(status_height);
    
    // Main content area
    main_panel_->setBounds(bounds);
}

void VitalPluginEditor::layoutParameterPanel() {
    if (!parameter_panel_) return;
    
    auto bounds = main_panel_->getBounds();
    
    // Divide main panel into sections
    int left_section_width = bounds.getWidth() * 0.6f;
    int right_section_width = bounds.getWidth() * 0.4f;
    
    // Parameter controls on the left
    parameter_panel_->setBounds(bounds.removeFromLeft(left_section_width));
    
    // Position parameter sliders within the panel
    if (!parameter_sliders_.empty()) {
        int margin = ui_config::CONTROL_MARGIN;
        int knob_size = 60;
        int slider_width = 200;
        int slider_height = 40;
        
        int current_x = margin;
        int current_y = margin;
        int knobs_in_row = ui_config::KNOBS_PER_ROW;
        int knob_spacing = slider_width + margin;
        int row_spacing = slider_height + margin;
        
        for (size_t i = 0; i < parameter_sliders_.size() && i < 32; ++i) { // Limit to 32 parameters in editor
            if (parameter_sliders_[i]) {
                int row = i / knobs_in_row;
                int col = i % knobs_in_row;
                
                int x = current_x + col * knob_spacing;
                int y = current_y + row * row_spacing;
                
                if (x + slider_width > parameter_panel_->getWidth()) {
                    break; // Stop if we run out of space
                }
                
                parameter_sliders_[i]->setBounds(x, y, slider_width, slider_height);
                
                if (parameter_labels_[i]) {
                    parameter_labels_[i]->setBounds(x, y + slider_height + 2, slider_width, 20);
                }
            }
        }
    }
}

void VitalPluginEditor::layoutKeyboardPanel() {
    if (!keyboard_panel_ || !virtual_keyboard_) return;
    
    auto bounds = main_panel_->getBounds();
    bounds = bounds.removeFromRight(bounds.getWidth() * 0.4f);
    
    keyboard_panel_->setBounds(bounds);
    
    // Position virtual keyboard
    int keyboard_height = 150;
    virtual_keyboard_->setBounds(bounds.removeFromBottom(keyboard_height));
}

void VitalPluginEditor::layoutStatusPanel() {
    if (!status_panel_) return;
    
    auto bounds = getLocalBounds();
    bounds = bounds.removeFromBottom(30);
    
    status_panel_->setBounds(bounds);
    
    // Position status elements
    if (cpu_label_) {
        cpu_label_->setBounds(10, 5, 100, 20);
    }
    
    if (memory_label_) {
        memory_label_->setBounds(120, 5, 120, 20);
    }
    
    if (latency_label_) {
        latency_label_->setBounds(250, 5, 120, 20);
    }
}

void VitalPluginEditor::updateLayout() {
    layoutMainPanel();
    layoutParameterPanel();
    layoutKeyboardPanel();
    layoutStatusPanel();
}

//==============================================================================
void VitalPluginEditor::createParameterSliders() {
    parameter_sliders_.resize(vital::plugin_config::MAX_PARAMETERS);
    parameter_labels_.resize(vital::plugin_config::MAX_PARAMETERS);
    
    for (int i = 0; i < vital::plugin_config::MAX_PARAMETERS; ++i) {
        // Create slider
        auto slider = std::make_unique<juce::Slider>();
        slider->setSliderStyle(juce::Slider::SliderStyle::LinearVertical);
        slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
        slider->setRange(0.0, 1.0, 0.01);
        slider->setValue(processor_.getParameter(i));
        slider->setEnabled(true);
        
        // Set callback
        slider->onValueChange = [this, i]() {
            onParameterChanged(i, static_cast<float>(parameter_sliders_[i]->getValue()));
        };
        
        parameter_sliders_[i] = std::move(slider);
        
        // Create label
        auto label = std::make_unique<juce::Label>();
        label->setText(processor_.getParameterName(i), juce::dontSendNotification);
        label->setJustification(juce::Justification::centred);
        label->setEditable(false);
        
        parameter_labels_[i] = std::move(label);
        
        // Add to parameter panel
        if (parameter_panel_) {
            parameter_panel_->addAndMakeVisible(parameter_sliders_[i].get());
            parameter_panel_->addAndMakeVisible(parameter_labels_[i].get());
        }
    }
}

void VitalPluginEditor::createProgramControls() {
    // Create program selector
    program_selector_ = std::make_unique<juce::ComboBox>();
    for (int i = 0; i < vital::plugin_config::MAX_PROGRAMS; ++i) {
        program_selector_->addItem(processor_.getProgramName(i), i + 1);
    }
    program_selector_->setSelectedItemIndex(processor_.getCurrentProgram());
    program_selector_->onChange = [this]() {
        onProgramChanged(program_selector_->getSelectedItemIndex());
    };
    
    // Create control buttons
    load_preset_button_ = std::make_unique<juce::TextButton>("Load");
    load_preset_button_->onClick = [this]() {
        auto presetName = juce::String("Program " + juce::String(program_selector_->getSelectedItemIndex() + 1));
        loadPresetByName(presetName);
    };
    
    save_preset_button_ = std::make_unique<juce::TextButton>("Save");
    save_preset_button_->onClick = [this]() {
        auto presetName = juce::String("Program " + juce::String(program_selector_->getSelectedItemIndex() + 1));
        savePresetByName(presetName);
    };
    
    randomize_button_ = std::make_unique<juce::TextButton>("Random");
    randomize_button_->onClick = [this]() { randomizeParameters(); };
    
    reset_button_ = std::makeUnique<juce::TextButton>("Reset");
    reset_button_->onClick = [this]() { resetAllParameters(); };
    
    // Add to main panel
    if (main_panel_) {
        main_panel_->addAndMakeVisible(program_selector_.get());
        main_panel_->addAndMakeVisible(load_preset_button_.get());
        main_panel_->addAndMakeVisible(save_preset_button_.get());
        main_panel_->addAndMakeVisible(randomize_button_.get());
        main_panel_->addAndMakeVisible(reset_button_.get());
    }
}

void VitalPluginEditor::createPerformanceMonitoring() {
    // Create performance labels
    cpu_label_ = std::make_unique<juce::Label>("CPU", "CPU: 0.0%");
    memory_label_ = std::make_unique<juce::Label>("Memory", "Memory: 0 MB");
    latency_label_ = std::make_unique<juce::Label>("Latency", "Latency: 0.0 ms");
    
    // Create performance overlay
    performance_overlay_ = std::make_unique<juce::Component>();
    
    // Add to status panel
    if (status_panel_) {
        status_panel_->addAndMakeVisible(cpu_label_.get());
        status_panel_->addAndMakeVisible(memory_label_.get());
        status_panel_->addAndMakeVisible(latency_label_.get());
    }
}

void VitalPluginEditor::createVirtualKeyboard() {
    // Create MIDI keyboard component
    virtual_keyboard_ = std::make_unique<juce::MidiKeyboardComponent>(
        processor_.getMidiKeyboardState(),
        juce::MidiKeyboardComponent::horizontalKeyboard
    );
    
    virtual_keyboard_->setKeyPressBase octaveBase = 60; // Middle C
    virtual_keyboard_->setVelocity(0.8f);
    virtual_keyboard_->setChannel(1);
    
    // Add to keyboard panel
    if (keyboard_panel_) {
        keyboard_panel_->addAndMakeVisible(virtual_keyboard_.get());
    }
}

void VitalPluginEditor::createStatusBar() {
    // Status bar is already created as status_panel_
    // Add background for status bar
    if (status_panel_) {
        status_panel_->setOpaque(true);
    }
}

//==============================================================================
void VitalPluginEditor::onParameterChanged(int parameterIndex, float newValue) {
    if (parameterIndex >= 0 && parameterIndex < vital::plugin_config::MAX_PARAMETERS) {
        processor_.setParameter(parameterIndex, newValue);
        
        // Push to undo stack
        pushUndoState();
        
        // Animate if enabled
        if (animations_enabled_) {
            animateParameter(parameterIndex, newValue);
        }
        
        // Accessibility announcement
        if (accessibility_enabled_) {
            announceParameterChange(parameterIndex, newValue);
        }
    }
}

void VitalPluginEditor::onProgramChanged(int programIndex) {
    processor_.setCurrentProgram(programIndex);
    
    // Update all parameter displays
    updateParameterDisplay(-1);
    
    // Accessibility announcement
    if (accessibility_enabled_) {
        announceProgramChange(programIndex);
    }
}

//==============================================================================
void VitalPluginEditor::drawBackground(juce::Graphics& g) {
    auto bounds = getLocalBounds().toFloat();
    
    // Draw subtle gradient background
    juce::ColourGradient gradient(juce::Colour(0xFF2A2A2A), bounds.getTopLeft(),
                                juce::Colour(0xFF1A1A1A), bounds.getBottomRight());
    g.setGradientFill(gradient);
    g.fillRect(bounds);
    
    // Draw section borders
    g.setColour(juce::Colour(0xFF404040));
    g.drawRect(main_panel_->getBounds().toFloat(), 1.0f);
    g.drawRect(keyboard_panel_->getBounds().toFloat(), 1.0f);
    g.drawRect(status_panel_->getBounds().toFloat(), 1.0f);
}

void VitalPluginEditor::drawParameters(juce::Graphics& g) {
    // Draw parameter value overlays if animations are running
    if (animations_enabled_ && !animations_.empty()) {
        g.setColour(juce::Colours::yellow.withAlpha(0.5f));
        g.setFont(12.0f);
        
        for (const auto& anim : animations_) {
            if (anim.parameter_index < static_cast<int>(parameter_labels_.size()) && 
                parameter_labels_[anim.parameter_index]) {
                auto label_bounds = parameter_labels_[anim.parameter_index]->getBounds().toFloat();
                g.drawText("Animating...", label_bounds.getRight() + 5, label_bounds.getY(), 
                          80, label_bounds.getHeight(), juce::Justification::left);
            }
        }
    }
}

void VitalPluginEditor::drawPerformanceOverlay(juce::Graphics& g) {
    if (performance_overlay_) {
        auto bounds = performance_overlay_->getBounds().toFloat();
        
        // Draw performance graph background
        g.setColour(juce::Colour(0xFF000000).withAlpha(0.3f));
        g.fillRect(bounds);
        
        // This would draw real-time performance graphs
        // Implementation depends on performance monitoring setup
    }
}

void VitalPluginEditor::drawAnimations(juce::Graphics& g) {
    // Draw active animations
    for (const auto& anim : animations_) {
        float progress = anim.progress;
        int x = static_cast<int>((anim.from_value + (anim.to_value - anim.from_value) * progress) * 100);
        
        // This would draw animation indicators
    }
}

//==============================================================================
void VitalPluginEditor::updatePerformanceStats(float cpuLoad, float memoryUsage, float latency) {
    current_cpu_load_.store(cpuLoad);
    current_memory_usage_.store(memoryUsage);
    current_latency_.store(latency);
    
    updatePerformanceLabels();
}

void VitalPluginEditor::updatePerformanceLabels() {
    updatePerformanceDisplay();
}

void VitalPluginEditor::showPerformanceOverlay(bool show) {
    if (performance_overlay_) {
        performance_overlay_->setVisible(show);
    }
}

//==============================================================================
void VitalPluginEditor::setTheme(const juce::String& themeName) {
    current_theme_ = themeName;
    applyThemeColors();
    repaint();
}

juce::String VitalPluginEditor::getCurrentTheme() const {
    return current_theme_;
}

void VitalPluginEditor::updateTheme() {
    applyThemeColors();
    repaint();
}

void VitalPluginEditor::applyThemeColors() {
    // Apply theme-specific colors to UI elements
    // This would implement different color schemes
    
    if (current_theme_ == "dark") {
        setOpaque(true);
        setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xFF1A1A1A));
    } else if (current_theme_ == "light") {
        setOpaque(true);
        setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(0xFFF0F0F0));
    }
}

juce::Colour VitalPluginEditor::getThemeColor(const juce::String& colorName) const {
    // Return theme-specific colors
    if (current_theme_ == "dark") {
        if (colorName == "background") return juce::Colour(0xFF2A2A2A);
        if (colorName == "text") return juce::Colours::white;
        if (colorName == "accent") return juce::Colour(0xFF4A90E2);
    } else {
        if (colorName == "background") return juce::Colour(0xFFF0F0F0);
        if (colorName == "text") return juce::Colours::black;
        if (colorName == "accent") return juce::Colour(0xFF4A90E2);
    }
    
    return juce::Colours::grey;
}

//==============================================================================
void VitalPluginEditor::setAccessibilityEnabled(bool enabled) {
    accessibility_enabled_ = enabled;
    if (enabled) {
        setupAccessibilityLabels();
        updateAccessibilityDescriptions();
    }
}

void VitalPluginEditor::announceParameterChange(int parameterIndex, float value) {
    // Announce parameter change to screen readers
    if (accessibility_enabled_ && parameterIndex >= 0) {
        juce::String announcement;
        
        if (parameterIndex == -1) {
            announcement = "Parameters randomized";
        } else if (parameterIndex == -2) {
            announcement = "All parameters reset to default";
        } else {
            announcement = processor_.getParameterName(parameterIndex) + 
                         " changed to " + juce::String(value * 100.0f, 1) + " percent";
        }
        
        handleScreenReaderAnnouncement(announcement);
    }
}

void VitalPluginEditor::announceProgramChange(int programIndex) {
    if (accessibility_enabled_ && programIndex >= 0) {
        juce::String announcement = "Program changed to " + processor_.getProgramName(programIndex);
        handleScreenReaderAnnouncement(announcement);
    }
}

void VitalPluginEditor::setupAccessibilityLabels() {
    if (!accessibility_enabled_) return;
    
    // Set up accessibility labels for screen readers
    for (int i = 0; i < vital::plugin_config::MAX_PARAMETERS && i < static_cast<int>(parameter_labels_.size()); ++i) {
        if (parameter_labels_[i]) {
            parameter_labels_[i]->setAccessible(true);
            parameter_labels_[i]->setAccessibilityLabel(processor_.getParameterName(i));
        }
        
        if (parameter_sliders_[i]) {
            parameter_sliders_[i]->setAccessible(true);
            parameter_sliders_[i]->setAccessibilityLabel(processor_.getParameterName(i));
            parameter_sliders_[i]->setDescription("Volume control for " + processor_.getParameterName(i));
        }
    }
}

void VitalPluginEditor::updateAccessibilityDescriptions() {
    if (!accessibility_enabled_) return;
    
    // Update accessibility descriptions based on focus
    // Implementation would depend on which component has focus
}

void VitalPluginEditor::handleScreenReaderAnnouncement(const juce::String& announcement) {
    // This would integrate with the AccessibilityManager to announce to screen readers
    DBG("Screen reader announcement: " << announcement);
}

//==============================================================================
void VitalPluginEditor::setHighDPIEnabled(bool enabled) {
    high_dpi_enabled_ = enabled;
    if (enabled) {
        // Enable high DPI rendering
        juce::Desktop::getInstance().setGlobalScaleFactor(ui_scale_);
    }
}

float VitalPluginEditor::getUIScale() const {
    return ui_scale_;
}

void VitalPluginEditor::setUIScale(float scale) {
    ui_scale_ = juce::jlimit(0.5f, 2.0f, scale);
    if (high_dpi_enabled_) {
        juce::Desktop::getInstance().setGlobalScaleFactor(ui_scale_);
    }
}

//==============================================================================
void VitalPluginEditor::enableAnimations(bool enabled) {
    animations_enabled_ = enabled;
}

void VitalPluginEditor::animateParameter(int parameterIndex, float targetValue) {
    if (!animations_enabled_) return;
    
    // Create animation state
    AnimationState anim;
    anim.parameter_index = parameterIndex;
    anim.from_value = processor_.getParameter(parameterIndex);
    anim.to_value = targetValue;
    anim.progress = 0.0f;
    
    animations_.push_back(anim);
    
    // Start animation timer if needed
    // Implementation would include proper animation timing
}

//==============================================================================
void VitalPluginEditor::pushUndoState() {
    // Create current state snapshot
    std::vector<ParameterState> current_state;
    for (int i = 0; i < vital::plugin_config::MAX_PARAMETERS; ++i) {
        current_state.push_back({i, processor_.getParameter(i)});
    }
    
    // Add to undo stack
    undo_stack_.push_back(current_state);
    
    // Clear redo stack when new action is performed
    clearRedoStack();
    
    // Limit stack size
    if (undo_stack_.size() > 50) {
        undo_stack_.erase(undo_stack_.begin());
    }
}

void VitalPluginEditor::clearUndoStack() {
    undo_stack_.clear();
}

void VitalPluginEditor::clearRedoStack() {
    redo_stack_.clear();
}

void VitalPluginEditor::undo() {
    if (!canUndo()) return;
    
    // Save current state to redo stack
    std::vector<ParameterState> current_state;
    for (int i = 0; i < vital::plugin_config::MAX_PARAMETERS; ++i) {
        current_state.push_back({i, processor_.getParameter(i)});
    }
    redo_stack_.push_back(current_state);
    
    // Restore previous state
    auto previous_state = undo_stack_.back();
    undo_stack_.pop_back();
    
    for (const auto& param : previous_state) {
        processor_.setParameter(param.parameter_index, param.value);
        if (param.parameter_index < static_cast<int>(parameter_sliders_.size())) {
            parameter_sliders_[param.parameter_index]->setValue(param.value, juce::dontSendNotification);
        }
    }
    
    repaint();
}

void VitalPluginEditor::redo() {
    if (!canRedo()) return;
    
    // Save current state to undo stack
    std::vector<ParameterState> current_state;
    for (int i = 0; i < vital::plugin_config::MAX_PARAMETERS; ++i) {
        current_state.push_back({i, processor_.getParameter(i)});
    }
    undo_stack_.push_back(current_state);
    
    // Restore next state
    auto next_state = redo_stack_.back();
    redo_stack_.pop_back();
    
    for (const auto& param : next_state) {
        processor_.setParameter(param.parameter_index, param.value);
        if (param.parameter_index < static_cast<int>(parameter_sliders_.size())) {
            parameter_sliders_[param.parameter_index]->setValue(param.value, juce::dontSendNotification);
        }
    }
    
    repaint();
}

bool VitalPluginEditor::canUndo() const {
    return !undo_stack_.empty();
}

bool VitalPluginEditor::canRedo() const {
    return !redo_stack_.empty();
}

//==============================================================================
void VitalPluginEditor::exportSettings(const juce::File& file) {
    try {
        auto xml = std::make_unique<juce::XmlElement>("VitalSettings");
        xml->setAttribute("version", vital::app::VERSION_STRING);
        xml->setAttribute("theme", current_theme_);
        xml->setAttribute("ui_scale", ui_scale_);
        
        // Export parameters
        auto params = xml->createNewChildElement("Parameters");
        for (int i = 0; i < vital::plugin_config::MAX_PARAMETERS; ++i) {
            auto param = params->createNewChildElement("Parameter");
            param->setAttribute("index", i);
            param->setAttribute("value", processor_.getParameter(i));
            param->setAttribute("name", processor_.getParameterName(i));
        }
        
        // Write to file
        if (xml->writeTo(file)) {
            DBG("Settings exported to: " << file.getFullPathName());
        }
        
    } catch (const std::exception& e) {
        DBG("Exception exporting settings: " << e.what());
    }
}

void VitalPluginEditor::importSettings(const juce::File& file) {
    try {
        auto xml = juce::XmlDocument::parse(file);
        if (!xml || !xml->hasTagName("VitalSettings")) {
            DBG("Invalid settings file");
            return;
        }
        
        // Import settings
        if (xml->hasAttribute("theme")) {
            setTheme(xml->getStringAttribute("theme"));
        }
        
        if (xml->hasAttribute("ui_scale")) {
            setUIScale(xml->getDoubleAttribute("ui_scale"));
        }
        
        // Import parameters
        auto* params = xml->getChildByName("Parameters");
        if (params) {
            for (auto* param : params->getChildElements()) {
                int index = param->getIntAttribute("index", -1);
                float value = param->getDoubleAttribute("value", 0.0);
                
                if (index >= 0 && index < vital::plugin_config::MAX_PARAMETERS) {
                    processor_.setParameter(index, static_cast<float>(value));
                    if (index < static_cast<int>(parameter_sliders_.size()) && parameter_sliders_[index]) {
                        parameter_sliders_[index]->setValue(value, juce::dontSendNotification);
                    }
                }
            }
        }
        
        DBG("Settings imported from: " << file.getFullPathName());
        
    } catch (const std::exception& e) {
        DBG("Exception importing settings: " << e.what());
    }
}

//==============================================================================
void VitalPluginEditor::startMIDILearn(int parameterIndex) {
    is_learning_.store(true);
    midi_learn_parameter_.store(parameterIndex);
    
    // Visual indicator
    if (parameter_sliders_[parameterIndex]) {
        parameter_sliders_[parameterIndex]->setColour(juce::Slider::trackColourId, juce::Colours::red);
    }
    
    DBG("Started MIDI learn for parameter: " << parameterIndex);
}

void VitalPluginEditor::stopMIDILearn() {
    is_learning_.store(false);
    midi_learn_parameter_.store(-1);
    
    // Remove visual indicators
    for (auto& slider : parameter_sliders_) {
        if (slider) {
            slider->setColour(juce::Slider::trackColourId, juce::Colours::grey);
        }
    }
    
    DBG("Stopped MIDI learn");
}

bool VitalPluginEditor::isMIDILearning() const {
    return is_learning_.load();
}

} // namespace vital
