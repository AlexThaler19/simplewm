#ifndef SIMPLEWM_STRUCTS_H
#define SIMPLEWM_STRUCTS_H
extern "C" {
#include <X11/Xlib.h>
}

typedef struct {
    Window win;
    Window closeIcon;
    Window maximizeIcon;
    Window minimizeIcon;
} MenuBar;

typedef struct {
    MenuBar topBar;
    Window frame;
    Window w;
} ClientWin;

#endif
