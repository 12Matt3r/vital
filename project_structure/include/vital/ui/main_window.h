#pragma once

#include <memory>
#include <string>
#include <vector>
#include <functional>

#if defined(__linux__)
    #include <X11/Xlib.h>
    typedef Window NativeWindow;
#elif defined(_WIN32)
    #include <windows.h>
    typedef HWND NativeWindow;
#elif defined(__APPLE__)
    #include <Cocoa/Cocoa.h>
    typedef NSView* NativeWindow;
#endif

namespace vital {

class EditorWindow;
class Synthesizer;

class MainWindow {
public:
    MainWindow();
    ~MainWindow();

    bool initialize(Synthesizer* synthesizer);
    void shutdown();

    void show();
    void hide();
    void setFullScreen(bool fullscreen);
    void setAlwaysOnTop(bool always_on_top);
    
    void setSize(int width, int height);
    void setPosition(int x, int y);
    
    int getWidth() const { return width_; }
    int getHeight() const { return height_; }
    int getX() const { return x_; }
    int getY() const { return y_; }
    
    NativeWindow getNativeWindow() const { return native_window_; }
    
    // Event handling
    void processEvents();
    void setCloseCallback(std::function<void()> callback) { close_callback_ = callback; }
    void setResizeCallback(std::function<void(int, int)> callback) { resize_callback_ = callback; }

private:
    void createNativeWindow();
    void destroyNativeWindow();
    void updateLayout();
    
    // Platform-specific implementations
    void createWindowX11();
    void createWindowWindows();
    void createWindowMacOS();
    
    void showX11();
    void showWindows();
    void showMacOS();
    
    void hideX11();
    void hideWindows();
    void hideMacOS();
    
    void processEventsX11();
    void processEventsWindows();
    void processEventsMacOS();

    // State
    bool initialized_;
    bool visible_;
    bool fullscreen_;
    bool always_on_top_;
    
    int width_;
    int height_;
    int x_;
    int y_;
    
    NativeWindow native_window_;
    
    // Callbacks
    std::function<void()> close_callback_;
    std::function<void(int, int)> resize_callback_;
    
    // Child windows
    std::unique_ptr<EditorWindow> editor_window_;
    
    // References
    Synthesizer* synthesizer_;
};

} // namespace vital