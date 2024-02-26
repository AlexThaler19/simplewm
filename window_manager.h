#ifndef SIMPLEWM_WINDOW_MANAGER_H
#define SIMPLEWM_WINDOW_MANAGER_H

extern "C" {
#include <X11/Xlib.h>
}
#include <memory>
#include <unordered_map>

class WindowManager {
public:
    static ::std::unique_ptr<WindowManager> Create ();
    ~WindowManager();
    void Run();

private:
    WindowManager(Display *display);
    Display *display_;
    const Window root_;

    void Frame(Window w, bool was_created_before_window_manager);
    void Unframe(Window w);
    //void SetBackgroundImage(const char *path);

    static int OnXError(Display *display, XErrorEvent *e);
    static int OnWMDetected(Display *display, XErrorEvent *e);
    static bool wm_detected_;

    //Event Handlers
    void OnCreateNotify(const XCreateWindowEvent& e);
    void OnDestroyNotify(const XDestroyWindowEvent& e);
    void OnReparentNotify(const XReparentEvent& e);
    void OnMapNotify(const XMapEvent& e);
    void OnUnmapNotify(const XUnmapEvent& e);
    void OnConfigureNotify(const XConfigureEvent& e);
    void OnMapRequest(const XMapRequestEvent& e);
    void OnConfigureRequest(const XConfigureRequestEvent& e);
    void OnButtonPress(const XButtonEvent& e);
    void OnButtonRelease(const XButtonEvent& e);
    void OnMotionNotify(const XMotionEvent& e);
    void OnKeyPress(const XKeyEvent& e);
    void OnKeyRelease(const XKeyEvent& e);

    ::std::unordered_map<Window, Window> clients_;
};

#endif