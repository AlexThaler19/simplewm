/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                           *
 *  Copyright (C) 2013-2017 Chuan Ji <ji@chu4n.com>                          *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the "License");          *
 *  you may not use this file except in compliance with the License.         *
 *  You may obtain a copy of the License at                                  *
 *                                                                           *
 *   http://www.apache.org/licenses/LICENSE-2.0                              *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an "AS IS" BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include "util.h"
#include <algorithm>
#include <sstream>
#include <vector>
#include <png.h>
#include <glog/logging.h>

using ::std::string;
using ::std::vector;
using ::std::pair;
using ::std::ostringstream;

string ToString(const XEvent& e) {
    static const char* const X_EVENT_TYPE_NAMES[] = {
            "",
            "",
            "KeyPress",
            "KeyRelease",
            "ButtonPress",
            "ButtonRelease",
            "MotionNotify",
            "EnterNotify",
            "LeaveNotify",
            "FocusIn",
            "FocusOut",
            "KeymapNotify",
            "Expose",
            "GraphicsExpose",
            "NoExpose",
            "VisibilityNotify",
            "CreateNotify",
            "DestroyNotify",
            "UnmapNotify",
            "MapNotify",
            "MapRequest",
            "ReparentNotify",
            "ConfigureNotify",
            "ConfigureRequest",
            "GravityNotify",
            "ResizeRequest",
            "CirculateNotify",
            "CirculateRequest",
            "PropertyNotify",
            "SelectionClear",
            "SelectionRequest",
            "SelectionNotify",
            "ColormapNotify",
            "ClientMessage",
            "MappingNotify",
            "GeneralEvent",
    };

    if (e.type < 2 || e.type >= LASTEvent) {
        ostringstream out;
        out << "Unknown (" << e.type << ")";
        return out.str();
    }

    // 1. Compile properties we care about.
    vector<pair<string, string>> properties;
    switch (e.type) {
        case CreateNotify:
            properties.emplace_back(
                    "window", ToString(e.xcreatewindow.window));
            properties.emplace_back(
                    "parent", ToString(e.xcreatewindow.parent));
            properties.emplace_back(
                    "size",
                    Size<int>(e.xcreatewindow.width, e.xcreatewindow.height).ToString());
            properties.emplace_back(
                    "position",
                    Position<int>(e.xcreatewindow.x, e.xcreatewindow.y).ToString());
            properties.emplace_back(
                    "border_width",
                    ToString(e.xcreatewindow.border_width));
            properties.emplace_back(
                    "override_redirect",
                    ToString(static_cast<bool>(e.xcreatewindow.override_redirect)));
            break;
        case DestroyNotify:
            properties.emplace_back(
                    "window", ToString(e.xdestroywindow.window));
            break;
        case MapNotify:
            properties.emplace_back(
                    "window", ToString(e.xmap.window));
            properties.emplace_back(
                    "event", ToString(e.xmap.event));
            properties.emplace_back(
                    "override_redirect",
                    ToString(static_cast<bool>(e.xmap.override_redirect)));
            break;
        case UnmapNotify:
            properties.emplace_back(
                    "window", ToString(e.xunmap.window));
            properties.emplace_back(
                    "event", ToString(e.xunmap.event));
            properties.emplace_back(
                    "from_configure",
                    ToString(static_cast<bool>(e.xunmap.from_configure)));
            break;
        case ConfigureNotify:
            properties.emplace_back(
                    "window", ToString(e.xconfigure.window));
            properties.emplace_back(
                    "size",
                    Size<int>(e.xconfigure.width, e.xconfigure.height).ToString());
            properties.emplace_back(
                    "position",
                    Position<int>(e.xconfigure.x, e.xconfigure.y).ToString());
            properties.emplace_back(
                    "border_width",
                    ToString(e.xconfigure.border_width));
            properties.emplace_back(
                    "override_redirect",
                    ToString(static_cast<bool>(e.xconfigure.override_redirect)));
            break;
        case ReparentNotify:
            properties.emplace_back(
                    "window", ToString(e.xreparent.window));
            properties.emplace_back(
                    "parent", ToString(e.xreparent.parent));
            properties.emplace_back(
                    "position",
                    Position<int>(e.xreparent.x, e.xreparent.y).ToString());
            properties.emplace_back(
                    "override_redirect",
                    ToString(static_cast<bool>(e.xreparent.override_redirect)));
            break;
        case MapRequest:
            properties.emplace_back(
                    "window", ToString(e.xmaprequest.window));
            break;
        case ConfigureRequest:
            properties.emplace_back(
                    "window", ToString(e.xconfigurerequest.window));
            properties.emplace_back(
                    "parent", ToString(e.xconfigurerequest.parent));
            properties.emplace_back(
                    "value_mask",
                    XConfigureWindowValueMaskToString(e.xconfigurerequest.value_mask));
            properties.emplace_back(
                    "position",
                    Position<int>(e.xconfigurerequest.x,
                                  e.xconfigurerequest.y).ToString());
            properties.emplace_back(
                    "size",
                    Size<int>(e.xconfigurerequest.width,
                              e.xconfigurerequest.height).ToString());
            properties.emplace_back(
                    "border_width",
                    ToString(e.xconfigurerequest.border_width));
            break;
        case ButtonPress:
        case ButtonRelease:
            properties.emplace_back(
                    "window", ToString(e.xbutton.window));
            properties.emplace_back(
                    "button", ToString(e.xbutton.button));
            properties.emplace_back(
                    "position_root",
                    Position<int>(e.xbutton.x_root, e.xbutton.y_root).ToString());
            break;
        case MotionNotify:
            properties.emplace_back(
                    "window", ToString(e.xmotion.window));
            properties.emplace_back(
                    "position_root",
                    Position<int>(e.xmotion.x_root, e.xmotion.y_root).ToString());
            properties.emplace_back(
                    "state", ToString(e.xmotion.state));
            properties.emplace_back(
                    "time", ToString(e.xmotion.time));
            break;
        case KeyPress:
        case KeyRelease:
            properties.emplace_back(
                    "window", ToString(e.xkey.window));
            properties.emplace_back(
                    "state", ToString(e.xkey.state));
            properties.emplace_back(
                    "keycode", ToString(e.xkey.keycode));
            break;
        default:
            // No properties are printed for unused events.
            break;
    }

    // 2. Build final string.
    const string properties_string = Join(
            properties, ", ", [] (const pair<string, string> &pair) {
                return pair.first + ": " + pair.second;
            });
    ostringstream out;
    out << X_EVENT_TYPE_NAMES[e.type] << " { " << properties_string << " }";
    return out.str();
}

string XConfigureWindowValueMaskToString(unsigned long value_mask) {
    vector<string> masks;
    if (value_mask & CWX) {
        masks.emplace_back("X");
    }
    if (value_mask & CWY) {
        masks.emplace_back("Y");
    }
    if (value_mask & CWWidth) {
        masks.emplace_back("Width");
    }
    if (value_mask & CWHeight) {
        masks.emplace_back("Height");
    }
    if (value_mask & CWBorderWidth) {
        masks.emplace_back("BorderWidth");
    }
    if (value_mask & CWSibling) {
        masks.emplace_back("Sibling");
    }
    if (value_mask & CWStackMode) {
        masks.emplace_back("StackMode");
    }
    return Join(masks, "|");
}

string XRequestCodeToString(unsigned char request_code) {
    static const char* const X_REQUEST_CODE_NAMES[] = {
            "",
            "CreateWindow",
            "ChangeWindowAttributes",
            "GetWindowAttributes",
            "DestroyWindow",
            "DestroySubwindows",
            "ChangeSaveSet",
            "ReparentWindow",
            "MapWindow",
            "MapSubwindows",
            "UnmapWindow",
            "UnmapSubwindows",
            "ConfigureWindow",
            "CirculateWindow",
            "GetGeometry",
            "QueryTree",
            "InternAtom",
            "GetAtomName",
            "ChangeProperty",
            "DeleteProperty",
            "GetProperty",
            "ListProperties",
            "SetSelectionOwner",
            "GetSelectionOwner",
            "ConvertSelection",
            "SendEvent",
            "GrabPointer",
            "UngrabPointer",
            "GrabButton",
            "UngrabButton",
            "ChangeActivePointerGrab",
            "GrabKeyboard",
            "UngrabKeyboard",
            "GrabKey",
            "UngrabKey",
            "AllowEvents",
            "GrabServer",
            "UngrabServer",
            "QueryPointer",
            "GetMotionEvents",
            "TranslateCoords",
            "WarpPointer",
            "SetInputFocus",
            "GetInputFocus",
            "QueryKeymap",
            "OpenFont",
            "CloseFont",
            "QueryFont",
            "QueryTextExtents",
            "ListFonts",
            "ListFontsWithInfo",
            "SetFontPath",
            "GetFontPath",
            "CreatePixmap",
            "FreePixmap",
            "CreateGC",
            "ChangeGC",
            "CopyGC",
            "SetDashes",
            "SetClipRectangles",
            "FreeGC",
            "ClearArea",
            "CopyArea",
            "CopyPlane",
            "PolyPoint",
            "PolyLine",
            "PolySegment",
            "PolyRectangle",
            "PolyArc",
            "FillPoly",
            "PolyFillRectangle",
            "PolyFillArc",
            "PutImage",
            "GetImage",
            "PolyText8",
            "PolyText16",
            "ImageText8",
            "ImageText16",
            "CreateColormap",
            "FreeColormap",
            "CopyColormapAndFree",
            "InstallColormap",
            "UninstallColormap",
            "ListInstalledColormaps",
            "AllocColor",
            "AllocNamedColor",
            "AllocColorCells",
            "AllocColorPlanes",
            "FreeColors",
            "StoreColors",
            "StoreNamedColor",
            "QueryColors",
            "LookupColor",
            "CreateCursor",
            "CreateGlyphCursor",
            "FreeCursor",
            "RecolorCursor",
            "QueryBestSize",
            "QueryExtension",
            "ListExtensions",
            "ChangeKeyboardMapping",
            "GetKeyboardMapping",
            "ChangeKeyboardControl",
            "GetKeyboardControl",
            "Bell",
            "ChangePointerControl",
            "GetPointerControl",
            "SetScreenSaver",
            "GetScreenSaver",
            "ChangeHosts",
            "ListHosts",
            "SetAccessControl",
            "SetCloseDownMode",
            "KillClient",
            "RotateProperties",
            "ForceScreenSaver",
            "SetPointerMapping",
            "GetPointerMapping",
            "SetModifierMapping",
            "GetModifierMapping",
            "NoOperation",
    };
    return X_REQUEST_CODE_NAMES[request_code];
}

void PngErrorHandler(png_structp png_ptr, png_const_charp error_msg) {
    LOG(ERROR) << "PNG Error: " << error_msg;
    throw std::runtime_error(error_msg);
}

Pixmap CreatePixmapFromPNG(Display* display, const char* filename, Window rootWindow) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        throw std::runtime_error("Failed to open PNG file");
    }

    // Initialize libpng structures
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, PngErrorHandler, nullptr);
    if (!png_ptr) {
        fclose(file);
        throw std::runtime_error("png_create_read_struct failed");
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, nullptr, nullptr);
        fclose(file);
        throw std::runtime_error("png_create_info_struct failed");
    }

    // Set custom error handler
    png_set_error_fn(png_ptr, nullptr, PngErrorHandler, nullptr);

    // Initialize PNG IO
    png_init_io(png_ptr, file);
    png_read_info(png_ptr, info_ptr);

    unsigned int width = png_get_image_width(png_ptr, info_ptr);
    unsigned int height = png_get_image_height(png_ptr, info_ptr);
    png_byte color_type = png_get_color_type(png_ptr, info_ptr);
    png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    // Expand paletted images to RGB
    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png_ptr);
    }

    // Expand grayscale images to RGB
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    }

    // Convert transparency to alpha channel
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png_ptr);
    }

    // Convert 16-bit per channel images to 8-bit per channel
    if (bit_depth == 16) {
        png_set_strip_16(png_ptr);
    }

    // Update info after transformations
    png_read_update_info(png_ptr, info_ptr);

    // Allocate memory for image data
    auto* row_pointers = new png_bytep[height];
    for (int y = 0; y < height; y++) {
        row_pointers[y] = new png_byte[png_get_rowbytes(png_ptr, info_ptr)];
    }

    // Read PNG image data
    png_read_image(png_ptr, row_pointers);

    // Create XImage from PNG data
    XImage* image = XCreateImage(display, DefaultVisual(display, DefaultScreen(display)),
                                 DefaultDepth(display, DefaultScreen(display)), ZPixmap,
                                 0, reinterpret_cast<char*>(row_pointers[0]), width, height, 32, 0);

    // Create Pixmap from XImage
    Pixmap pixmap = XCreatePixmap(display, rootWindow, width, height, DefaultDepth(display, DefaultScreen(display)));
    XPutImage(display, pixmap, DefaultGC(display, DefaultScreen(display)), image, 0, 0, 0, 0, width, height);

    // Free memory
    for (int y = 0; y < height; y++) {
        delete[] row_pointers[y];
    }
    delete[] row_pointers;

    // Clean up libpng
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    fclose(file);

    return pixmap;
}


/*
Pixmap CreatePixmapFromPNG(Display* display, const char* filename, Window rootWindow) {
    FILE* file = fopen(filename, "rb");
    if (!file) {
        throw std::runtime_error("Failed to open PNG file");
    }

    // Initialize libpng structures
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (!png_ptr) {
        fclose(file);
        throw std::runtime_error("png_create_read_struct failed");
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        png_destroy_read_struct(&png_ptr, nullptr, nullptr);
        fclose(file);
        throw std::runtime_error("png_create_info_struct failed");
    }

    // Set up error handling
    if (setjmp(png_jmpbuf(png_ptr))) {
        // Error occurred during PNG read
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        fclose(file);
        throw std::runtime_error("Error occurred during PNG read");
    }

    // Set custom error handler
    png_set_error_fn(png_ptr, nullptr, PngErrorHandler, nullptr);

    // Initialize PNG IO
    png_init_io(png_ptr, file);
    png_read_info(png_ptr, info_ptr);

    unsigned int width = png_get_image_width(png_ptr, info_ptr);
    unsigned int height = png_get_image_height(png_ptr, info_ptr);
    png_byte color_type = png_get_color_type(png_ptr, info_ptr);
    png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    // Expand paletted images to RGB
    if (color_type == PNG_COLOR_TYPE_PALETTE) {
        png_set_palette_to_rgb(png_ptr);
    }

    // Expand grayscale images to RGB
    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8) {
        png_set_expand_gray_1_2_4_to_8(png_ptr);
    }

    // Convert transparency to alpha channel
    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(png_ptr);
    }

    // Convert 16-bit per channel images to 8-bit per channel
    if (bit_depth == 16) {
        png_set_strip_16(png_ptr);
    }

    // Update info after transformations
    png_read_update_info(png_ptr, info_ptr);

    // Allocate memory for image data
    png_bytep* row_pointers = new png_bytep[height];
    for (int y = 0; y < height; y++) {
        row_pointers[y] = new png_byte[png_get_rowbytes(png_ptr, info_ptr)];
    }

    // Read PNG image data
    png_read_image(png_ptr, row_pointers);

    // Create XImage from PNG data
    XImage* image = XCreateImage(display, DefaultVisual(display, DefaultScreen(display)),
                                 DefaultDepth(display, DefaultScreen(display)), ZPixmap,
                                 0, reinterpret_cast<char*>(row_pointers[0]), width, height, 32, 0);

    // Create Pixmap from XImage
    Pixmap pixmap = XCreatePixmap(display, rootWindow, width, height, DefaultDepth(display, DefaultScreen(display)));
    XPutImage(display, pixmap, DefaultGC(display, DefaultScreen(display)), image, 0, 0, 0, 0, width, height);

    // Free memory
    for (int y = 0; y < height; y++) {
        delete[] row_pointers[y];
    }
    delete[] row_pointers;

    // Clean up libpng
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
    fclose(file);

    return pixmap;
}
*/