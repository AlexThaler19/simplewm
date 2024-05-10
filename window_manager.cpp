#include "window_manager.h"
extern "C" {
#include <X11/Xutil.h>
#include <X11/extensions/shape.h>
#include <X11/cursorfont.h>
}
#include <cstring>
#include <algorithm>
#include <glog/logging.h>
#include "util.h"
#include <mutex>

using ::std::unique_ptr;
using ::std::max;
using ::std::mutex;
using ::std::string;
using ::std::unique_ptr;

bool WindowManager::wm_detected_;
mutex WindowManager::wm_detected_mutex_;

unique_ptr<WindowManager> WindowManager::Create() {
    Display *display = XOpenDisplay(nullptr);
    if (display == nullptr) {
        LOG(ERROR) << "Failed to open X display" << XDisplayName(nullptr);
        return nullptr;
    }
    return unique_ptr<WindowManager>(new WindowManager(display));
}

WindowManager::WindowManager(Display *display)
    : display_(CHECK_NOTNULL(display)),
      root_(DefaultRootWindow(display)),
      WM_PROTOCOLS(XInternAtom(display_, "WM_PROTOCOLS", false)),
      WM_DELETE_WINDOW(XInternAtom(display_, "WM_DELETE_WINDOW", false)) {
}

WindowManager::~WindowManager() {
    XCloseDisplay(display_);
}

bool isTopBar(::std::vector<ClientWin> clients, Window eWin) {
    for (auto & client : clients) {
        if (client.topBar.win == eWin)
            return true;
    }
    return false;
}

bool isCloseIcon(::std::vector<ClientWin> clients, Window eWin) {
    for (auto & client : clients) {
        if (client.topBar.closeIcon == eWin)
            return true;
    }
    return false;
}

ClientWin getWin(::std::vector<ClientWin> clients, Window frame) {
    for (auto & win : clients) {
        if (win.frame == frame) {
            return win;
        }
    }
    ClientWin ret;
    return ret;
}

void WindowManager::closeWindow(Window win) {
    /*XDestroyWindow(display_, win);
    LOG(INFO) << "Destroyed Window " << win;*/
    Atom* supportedProtocols;
    int numSupportedProtocols;
    if (XGetWMProtocols(display_, win, &supportedProtocols, &numSupportedProtocols) &&
        ::std::find(supportedProtocols, supportedProtocols+numSupportedProtocols, WM_DELETE_WINDOW) !=
        supportedProtocols + numSupportedProtocols) {
        LOG(INFO) << "Gracefully deleting window " << win;

        XEvent msg;
        memset(&msg, 0, sizeof(msg));
        msg.xclient.type = ClientMessage;
        msg.xclient.message_type = WM_PROTOCOLS;
        msg.xclient.window = win;
        msg.xclient.format = 32;
        msg.xclient.data.l[0] = WM_DELETE_WINDOW;
        CHECK(XSendEvent(display_, win, false, 0, &msg));
    } else {
        LOG(INFO) << "Killing Window " << win;
        XDestroyWindow(display_, win);
    }
}

void WindowManager::drawCross(ClientWin win) {
    unsigned long color = 0x3b414a;
    LOG(INFO) << "GC: " << win.topBar.closeGC;
    LOG(INFO) << "Pixmap: " << win.topBar.closePixmap;
    XSetForeground(display_, win.topBar.closeGC, 0xFF0000);
    XFillRectangle(display_, win.topBar.closePixmap, win.topBar.closeGC, 0, 0, 20, 20);
    XSetForeground(display_, win.topBar.closeGC, color);
    XSetLineAttributes(display_, win.topBar.closeGC, 2, 0, 0, 0);
    XDrawLine(display_, win.topBar.closePixmap, win.topBar.closeGC, 3, 3, 17, 17);
    XDrawLine(display_, win.topBar.closePixmap, win.topBar.closeGC, 3, 17, 17, 3);
    XCopyArea(display_, win.topBar.closePixmap, win.topBar.closeIcon, win.topBar.closeGC, 0, 0, 20, 20, 0, 0);
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
        LOG(ERROR) << "Another window manager is already running" << XDisplayString(display_);
        return;
    }
    XSetErrorHandler(&WindowManager::OnXError);

    XGrabServer(display_);
    Window returned_root, returned_parent;
    Window *top_level_windows;
    unsigned int num_top_level_windows;
    CHECK(XQueryTree(
            display_,
            root_,
            &returned_root,
            &returned_parent,
            &top_level_windows,
            &num_top_level_windows));
    CHECK_EQ(returned_root, root_);
    for (int i = 0; i < num_top_level_windows; ++i) {
        Frame(top_level_windows[i], true);
    }

    XFree(top_level_windows);
    XUngrabServer(display_);


    //SetBackgroundImage("./resources/LinusTorvalds.png");
    XSetWindowBackground(display_, root_, 0x435975);
    XClearWindow(display_, root_);
    Cursor c = XCreateFontCursor(display_, XC_arrow);
    XDefineCursor(display_, root_, c);

    while(true) {
        //Get the next Event
        XEvent e;
        XNextEvent(display_, &e);
        LOG(INFO) << "Received event: " << ToString(e);

        switch(e.type) {
            case CreateNotify:
                OnCreateNotify(e.xcreatewindow);
                break;
            case DestroyNotify:
                OnDestroyNotify(e.xdestroywindow);
                for (auto & clientWin : clientWindows) {
                    drawCross(clientWin);
                }
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
                for (auto & clientWin : clientWindows) {
                    drawCross(clientWin);
                }
                break;
            case KeyPress:
                OnKeyPress(e.xkey);
                break;
            case KeyRelease:
                OnKeyRelease(e.xkey);
                break;
            default:
                LOG(WARNING) << "Event not handled";
        }
    }
}

int WindowManager::OnWMDetected(Display *display, XErrorEvent *e) {
    CHECK_EQ(static_cast<int>(e->error_code), BadAccess);
    wm_detected_ = true;
    return 0;
}

int WindowManager::OnXError(Display *display, XErrorEvent *e) {
    const int MAX_ERROR_TEXT_LENGTH = 1024;
    char error_text[MAX_ERROR_TEXT_LENGTH];
    XGetErrorText(display, e->error_code, error_text, sizeof(error_text));
    LOG(ERROR) << "Received X error:\n"
               << "    Request: " << int(e->request_code)
               << " - " << XRequestCodeToString(e->request_code) << "\n"
               << "    Error code: " << int(e->error_code)
               << " - " << error_text << "\n"
               << "    Resource ID: " << e->resourceid;
    // The return value is ignored.
    return 0;
}

void WindowManager::Frame(Window w, bool was_created_before_window_manager) {
    ClientWin client;
    const int BORDERWIDTH = 1;
    const unsigned int BORDERCOLOR = 0xFF0000; //0x7a7a7a;  //TODO Background image
    const unsigned int BGCOLOR = 0x3b414a;

    CHECK(!clients_.count(w));

    client.w = w;
    XWindowAttributes x_window_attrs;
    CHECK(XGetWindowAttributes(display_, w, &x_window_attrs));

    if (was_created_before_window_manager) {
        if(x_window_attrs.override_redirect || x_window_attrs.map_state != IsViewable) {
            LOG(INFO) << "Created before window manager: " << w;
            return;
        }
    }
    client.frame = XCreateSimpleWindow(
            display_,
            root_,
            x_window_attrs.x,
            x_window_attrs.y,
            x_window_attrs.width,
            x_window_attrs.height + 20,
            BORDERWIDTH,
            BORDERCOLOR,
            BGCOLOR);
    //Pixmap pixmap = XCreatePixmap(display_, client.frame, 400, 300, 1);
    //XShapeCombineMask(display_, client.frame, ShapeBounding, 0, 0, pixmap, ShapeSet);     //TODO transparent frame

    XSelectInput(display_, client.frame, SubstructureRedirectMask | SubstructureNotifyMask);
    XAddToSaveSet(display_, w);
    XReparentWindow(display_, w, client.frame, -1, 20);
    XMapWindow(display_, client.frame);

    //Create Titlebar
    client.topBar.win = XCreateSimpleWindow(
            display_,
            client.frame,
            x_window_attrs.x,
            x_window_attrs.y,
            x_window_attrs.width,
            20,
            0,
            0,
            0x646375);
    XSelectInput(display_, client.topBar.win, SubstructureRedirectMask | SubstructureNotifyMask);
    XReparentWindow(display_, client.topBar.win, client.frame, 0, 0);
    XMapWindow(display_, client.topBar.win);

    client.topBar.closeIcon = XCreateSimpleWindow(
            display_,
            client.topBar.win,
            x_window_attrs.x,
            x_window_attrs.y,
            20,
            20,
            0,
            0,
            0xff0000);
    XSelectInput(display_, client.topBar.closeIcon, SubstructureRedirectMask | SubstructureNotifyMask);
    XReparentWindow(display_, client.topBar.closeIcon, client.frame, x_window_attrs.width-20, 0);
    XMapWindow(display_, client.topBar.closeIcon);

    client.topBar.closePixmap = XCreatePixmap(display_, client.topBar.closeIcon,
                                  20, 20,
                                  DefaultDepth(display_, DefaultScreen(display_)));
    client.topBar.closeGC = XCreateGC(display_, client.topBar.closePixmap, 0, None);
    drawCross(client);

    clients_[client.topBar.win] = client.frame;
    clients_[w] = client.frame;
    clients_[client.topBar.closeIcon] = client.frame;
    clientWindows.push_back(client);

    XGrabButton(
            display_,
            Button1,
            AnyModifier,
            client.topBar.closeIcon,
            false,
            ButtonPressMask | ButtonReleaseMask,
            GrabModeAsync,
            GrabModeAsync,
            None,
            None);
    //   a. Move windows with left button.
    XGrabButton(
            display_,
            Button1,
            AnyModifier,
            client.topBar.win,
            false,
            ButtonPressMask | ButtonMotionMask,
            GrabModeAsync,
            GrabModeAsync,
            None,
            None);
    //   b. Resize windows with alt + right button.
    XGrabButton(
            display_,
            Button1,
            AnyModifier,
            w,
            false,
            ButtonPressMask,
            GrabModeAsync,
            GrabModeAsync,
            None,
            None);
    //   c. Kill windows with alt + f4.
    XGrabKey(
            display_,
            XKeysymToKeycode(display_, XK_F4),
            Mod1Mask,
            client.frame,
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

    LOG(INFO) << "Framed window " << w << " [" << client.frame << "]" << " [" << client.topBar.win << "]";
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
    LOG(INFO) << "Unframed window " << w << " [" << frame << "]";
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
        LOG(INFO) << "Resize [" << frame << "] to " << Size<int>(e.window, e.height);
    }

    XConfigureWindow(display_, e.window, e.value_mask, &changes);
    LOG(INFO) << "Resize " << e.window << "to " << Size<int>(e.width, e.height);
}

void WindowManager::OnMapRequest(const XMapRequestEvent &e) {
    Frame(e.window, false);
    XMapWindow(display_, e.window);
}

void WindowManager::OnUnmapNotify(const XUnmapEvent &e) {
    if (!clients_.count(e.window)) {
        LOG(INFO) << "UnmapNotify ignored for non-client window " << e.window;
        return;
    }

    if (e.event == root_) {
        LOG(INFO) << "UnmapNotify ignored for reparented pre-existing Window " << e.window;
        return;
    }

    Unframe(e.window);
}

void WindowManager::OnButtonPress(const XButtonEvent &e) {
    LOG(INFO) << "Button press on " << e.window;
    CHECK(clients_.count(e.window));
    Window frame;
    if (clients_.count(e.window))
        frame = clients_[e.window];
    else
        frame = e.window;

    for(auto & clientWindow : clientWindows) {
        if (isTopBar(clientWindows, e.window)) {
            LOG(INFO) << "Clicked on TopBar";
        } else if (isCloseIcon(clientWindows, e.window)) {
            LOG(INFO) << "Clicked on CloseIcon -> Frame: " << frame;
        }
    }
    startPos = Position<int>(e.x_root, e.y_root);

    Window returned_root;
    int x, y;
    unsigned width, height, borderWidth, depth;
    CHECK(XGetGeometry(display_, frame, &returned_root, &x, &y, &width, &height, &borderWidth, &depth));
    startFramePos = Position<int>(x, y);
    startFrameSize = Position<int>(width, height);
    XRaiseWindow(display_, frame);
}
void WindowManager::OnButtonRelease(const XButtonEvent &e) {
    if (isCloseIcon(clientWindows, e.window))
        closeWindow(clients_[e.window]);
}
void WindowManager::OnMotionNotify(const XMotionEvent &e) {
    CHECK(clients_.count(e.window));
    if (!isTopBar(clientWindows, e.window))
        return;
    Window frame;
    frame = clients_[e.window];
    ClientWin win = getWin(clientWindows, frame);

    const Position<int> currentPos(e.x_root, e.y_root);
    const Vector2D<int> delta = currentPos - startPos;

    if (e.state & Button1Mask) {
        const Position<int> destPos = startFramePos + delta;
        XMoveWindow(display_, frame, destPos.x, destPos.y);
    }
}
void WindowManager::OnKeyPress(const XKeyEvent &e) {
    if ((e.state & Mod1Mask) && e.keycode == XKeysymToKeycode(display_, XK_F4)) {
        closeWindow(e.window);
    }
}
void WindowManager::OnKeyRelease(const XKeyEvent &e) {}