//#include <glog/logging.h>
#include "window_manager.h"
#include "util.h"
#include <X11/Xutil.h>
#include <iostream>

using ::std::unique_ptr;

bool WindowManager::wm_detected_;

unique_ptr<WindowManager> WindowManager::Create() {
    Display *display = XOpenDisplay(nullptr);
    if (display == nullptr) {
        //LOG(ERROR) << "Failed to open X display" << XDisplayName(nullptr);
        ::std::cout << "Error opening X display";
        return nullptr;
    }
    return unique_ptr<WindowManager>(new WindowManager(display));
}

WindowManager::WindowManager(Display *display)
    : display_(display),
      root_(DefaultRootWindow(display)) {
}

WindowManager::~WindowManager() {
    XCloseDisplay(display_);
}

void WindowManager::Run() {
    wm_detected_ = false;
    XSetErrorHandler(&WindowManager::OnWMDetected);
    XSelectInput(
            display_,
            root_,
            SubstructureRedirectMask | SubstructureNotifyMask);
    XSync(display_, false);
    if (wm_detected_) {
        //LOG(ERROR) << "Another window manager is already running" << XDisplayString(display_);
        return;
    }
    XSetErrorHandler(&WindowManager::OnXError);

    XGrabServer(display_);
    Window returned_root, returned_parent;
    Window *top_level_windows;
    unsigned int num_top_level_windows;
    /*CHECK(*/XQueryTree(
            display_,
            root_,
            &returned_root,
            &returned_parent,
            &top_level_windows,
            &num_top_level_windows);//);
    //CHECK_EQ(returned_root, root_);
    for (int i = 0; i < num_top_level_windows; ++i) {
        Frame(top_level_windows[i], true);
    }

    XFree(top_level_windows);
    XUngrabServer(display_);


    //SetBackgroundImage("./resources/LinusTorvalds.png");
    XSetWindowBackground(display_, root_, 0x435975);
    XClearWindow(display_, root_);

    while(true) {
        //Get the next Event
        XEvent e;
        XNextEvent(display_, &e);
        //LOG(INFO) << "Received event: " << ToString(e);

        switch(e.type) {
            case CreateNotify:
                OnCreateNotify(e.xcreatewindow);
                break;
            case DestroyNotify:
                OnDestroyNotify(e.xdestroywindow);
                break;
            case ReparentNotify:
                OnReparentNotify(e.xreparent);
                break;
            case MapNotify:
                OnMapNotify(e.xmap);
                break;
            case UnmapNotify:
                OnUnmapNotify(e.xunmap);
                break;
            case ConfigureNotify:
                OnConfigureNotify(e.xconfigure);
                break;
            case MapRequest:
                OnMapRequest(e.xmaprequest);
                break;
            case ConfigureRequest:
                OnConfigureRequest(e.xconfigurerequest);
                break;
            case ButtonPress:
                OnButtonPress(e.xbutton);
                break;
            case ButtonRelease:
                OnButtonRelease(e.xbutton);
                break;
            case MotionNotify:
                OnMotionNotify(e.xmotion);
                break;
            case KeyPress:
                OnKeyPress(e.xkey);
                break;
            case KeyRelease:
                OnKeyRelease(e.xkey);
                break;
            //default:
                //LOG(WARNING) << "Event not handled";
        }
    }
}

int WindowManager::OnWMDetected(Display *display, XErrorEvent *e) {
    //CHECK_EQ(static_cast<int>(e->error_code), BadAccess);
    wm_detected_ = true;
    return 0;
}

int WindowManager::OnXError(Display *display, XErrorEvent *e) {
    const int MAX_ERROR_TEXT_LENGTH = 1024;
    char error_text[MAX_ERROR_TEXT_LENGTH];
    XGetErrorText(display, e->error_code, error_text, sizeof(error_text));
    /*LOG(ERROR) << "Received X error:\n"
               << "    Request: " << int(e->request_code)
               << " - " << XRequestCodeToString(e->request_code) << "\n"
               << "    Error code: " << int(e->error_code)
               << " - " << error_text << "\n"
               << "    Resource ID: " << e->resourceid;*/
    // The return value is ignored.
    return 0;
}

void WindowManager::Frame(Window w, bool was_created_before_window_manager) {
    const unsigned int BORDER_WIDTH = 3;
    const unsigned int BORDER_COLOR = 0x4287f5;
    const unsigned int BG_COLOR = 0x3b414a;

    //CHECK(!clients_.count(w));

    XWindowAttributes x_window_attrs;
    //CHECK(XGetWindowAttributes(display_, w, &x_window_attrs));

    if (was_created_before_window_manager) {
        if(x_window_attrs.override_redirect || x_window_attrs.map_state != IsViewable) {
            return;
        }
    }

    const Window frame = XCreateSimpleWindow(
            display_,
            root_,
            x_window_attrs.x,
            x_window_attrs.y,
            x_window_attrs.width,
            x_window_attrs.height,
            BORDER_WIDTH,
            BORDER_COLOR,
            BG_COLOR);

    XSelectInput(display_, frame, SubstructureRedirectMask | SubstructureNotifyMask);
    XAddToSaveSet(display_, w);
    XReparentWindow(display_, w, frame, 0, 0);
    XMapWindow(display_, frame);
    clients_[w] = frame;

    //   a. Move windows with alt + left button.
    XGrabButton(
            display_,
            Button1,
            Mod1Mask,
            w,
            false,
            ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
            GrabModeAsync,
            GrabModeAsync,
            None,
            None);
    //   b. Resize windows with alt + right button.
    XGrabButton(
            display_,
            Button3,
            Mod1Mask,
            w,
            false,
            ButtonPressMask | ButtonReleaseMask | ButtonMotionMask,
            GrabModeAsync,
            GrabModeAsync,
            None,
            None);
    //   c. Kill windows with alt + f4.
    XGrabKey(
            display_,
            XKeysymToKeycode(display_, XK_F4),
            Mod1Mask,
            w,
            false,
            GrabModeAsync,
            GrabModeAsync);
    //   d. Switch windows with alt + tab.
    XGrabKey(
            display_,
            XKeysymToKeycode(display_, XK_Tab),
            Mod1Mask,
            w,
            false,
            GrabModeAsync,
            GrabModeAsync);

    //LOG(INFO) << "Framed window " << w << " [" << frame << "]";
}

void WindowManager::Unframe(Window w) {
    const Window frame = clients_[w];
    XUnmapWindow(display_, frame);
    XReparentWindow(
            display_,
            w,
            root_,
            0, 0);
    XRemoveFromSaveSet(display_, w);
    XDestroyWindow(display_, w);
    clients_.erase(w);
    //LOG(INFO) << "Unframed window " << w << " [" << frame << "]";
}


void WindowManager::OnCreateNotify(const XCreateWindowEvent &e) {}
void WindowManager::OnReparentNotify(const XReparentEvent &e) {}
void WindowManager::OnMapNotify(const XMapEvent &e) {}
void WindowManager::OnDestroyNotify(const XDestroyWindowEvent &e) {}
void WindowManager::OnConfigureNotify(const XConfigureEvent &e) {}

void WindowManager::OnConfigureRequest(const XConfigureRequestEvent &e) {
    XWindowChanges changes;
    changes.x = e.x;
    changes.y = e.y;
    changes.width = e.width;
    changes.height = e.height;
    changes.border_width = e.border_width;
    changes.sibling = e.above;
    changes.stack_mode = e.detail;
    if (clients_.count(e.window)) {
        const Window frame =  clients_[e.window];
        XConfigureWindow(display_, frame, e.value_mask, &changes);
        //LOG(INFO) << "Resize [" << frame << "] to " << Size<int>(e.window, e.height);
    }

    XConfigureWindow(display_, e.window, e.value_mask, &changes);
    //LOG(INFO) << "Resize " << e.window << "to " << Size<int>(e.width, e.height);
}

void WindowManager::OnMapRequest(const XMapRequestEvent &e) {
    Frame(e.window, false);
    XMapWindow(display_, e.window);
}

void WindowManager::OnUnmapNotify(const XUnmapEvent &e) {
    if (!clients_.count(e.window)) {
        //LOG(INFO) << "UnmapNotify ignored for non-client window " << e.window;
        return;
    }

    if (e.event == root_) {
        //LOG(INFO) << "UnmapNotify ignored for reparented pre-existing Window " << e.window;
        return;
    }

    Unframe(e.window);
}

void WindowManager::OnButtonPress(const XButtonEvent &e) {
    //CHECK(clients_.count(e.window));
    const Window frame = clients_[e.window];
}
void WindowManager::OnButtonRelease(const XButtonEvent &e) {}
void WindowManager::OnMotionNotify(const XMotionEvent &e) {}
void WindowManager::OnKeyPress(const XKeyEvent &e) {}
void WindowManager::OnKeyRelease(const XKeyEvent &e) {}