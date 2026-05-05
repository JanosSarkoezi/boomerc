#ifndef SCREENSHOT_H_
#define SCREENSHOT_H_

#include <X11/Xlib.h>
#include <X11/Xutil.h>

typedef struct {
    XImage *image;
} Screenshot;

Screenshot new_screenshot(Display *display, Window window);
void screenshot_destroy(Screenshot *screenshot, Display *display);
void screenshot_refresh(Screenshot *screenshot, Display *display, Window window);

#endif // SCREENSHOT_H_
