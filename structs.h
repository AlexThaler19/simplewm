#ifndef SIMPLEWM_STRUCTS_H
#define SIMPLEWM_STRUCTS_H
extern "C" {
#include <X11/Xlib.h>
}

typedef struct {
    Window win;
    Window closeIcon;
    GC closeGC;
    Window maximizeIcon;
    Window minimizeIcon;
} MenuBar;

typedef struct {
    MenuBar topBar;
    Window frame;
    Window w;
} ClientWin;

typedef struct {
    const char *path;
    Pixmap pixmap;
} BackgroundImage;

#endif
